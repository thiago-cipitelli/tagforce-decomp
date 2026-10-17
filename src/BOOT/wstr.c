// @range 0x08841BD0-0x088420B4
// @data 0x088EE3B8-0x088EE8BC
/* Text character map (game text code <-> font glyph), partner table lookup, collection
   percentage, and the UCS-2 string primitives used by the text layer. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "game.h"

int card_get_flag31(int id);
int card_is_attribute9(int id);
int card_is_excluded(unsigned short id);

typedef struct CharmapEntry {
    unsigned char ch;              /* text code 0x29..0x97, sorted */
    unsigned char kind;            /* low nibble */
    unsigned short code;           /* glyph/table code */
    unsigned char x;               /* low nibble */
    unsigned char pad;
} CharmapEntry;

typedef struct CharmapRevEntry {
    unsigned short code;           /* sorted */
    unsigned char a;
    unsigned char b;
    unsigned char c : 4;
    unsigned char hi : 4;
    unsigned char pad;
} CharmapRevEntry;

typedef struct CharmapPos {
    unsigned char a;
    unsigned char b;
    unsigned char c : 4;
    unsigned char hi : 4;
} CharmapPos;

static CharmapEntry charmap_table[111] = {     { 0x29, 4, 0x3E8, 0, 0 }, { 0x2A, 4, 0x3E8, 0, 0 }, { 0x2B, 4, 0x3E8, 1, 0 }, { 0x2C, 4, 0x3E8, 0, 0 }, { 0x2D, 4, 0x3E8, 2, 0 }, { 0x2E, 4, 0x3E8, 1, 0 },
    { 0x2F, 0, 0x074, 0, 0 }, { 0x30, 0, 0x07E, 0, 0 }, { 0x31, 0, 0x089, 0, 0 }, { 0x32, 1, 0x094, 2, 0 }, { 0x33, 1, 0x064, 1, 0 }, { 0x34, 1, 0x06E, 3, 0 },
    { 0x35, 1, 0x078, 2, 0 }, { 0x36, 1, 0x082, 3, 0 }, { 0x37, 1, 0x08C, 1, 0 }, { 0x38, 1, 0x079, 6, 0 }, { 0x39, 1, 0x083, 10, 0 }, { 0x3A, 1, 0x06F, 7, 0 },
    { 0x3B, 1, 0x07A, 4, 0 }, { 0x3C, 1, 0x085, 2, 0 }, { 0x3D, 1, 0x08D, 12, 0 }, { 0x3E, 1, 0x065, 14, 0 }, { 0x3F, 1, 0x070, 10, 0 }, { 0x40, 1, 0x07B, 14, 0 },
    { 0x41, 1, 0x084, 1, 0 }, { 0x42, 1, 0x08E, 4, 0 }, { 0x43, 1, 0x066, 3, 0 }, { 0x44, 1, 0x071, 12, 0 }, { 0x45, 1, 0x07C, 1, 0 }, { 0x46, 1, 0x086, 13, 0 },
    { 0x47, 1, 0x08F, 11, 0 }, { 0x48, 1, 0x06D, 14, 0 }, { 0x49, 1, 0x076, 9, 0 }, { 0x4A, 1, 0x07F, 12, 0 }, { 0x4B, 1, 0x088, 0, 0 }, { 0x4C, 1, 0x091, 10, 0 },
    { 0x4D, 2, 0x0C8, 11, 0 }, { 0x4E, 2, 0x0D2, 5, 0 }, { 0x4F, 2, 0x0DC, 7, 0 }, { 0x50, 2, 0x0E6, 10, 0 }, { 0x51, 2, 0x0F0, 8, 0 }, { 0x52, 2, 0x0D7, 7, 0 },
    { 0x53, 2, 0x0E1, 10, 0 }, { 0x54, 2, 0x0EB, 4, 0 }, { 0x55, 2, 0x0F5, 13, 0 }, { 0x56, 2, 0x0CD, 2, 0 }, { 0x57, 2, 0x0D8, 2, 0 }, { 0x58, 2, 0x0E2, 13, 0 },
    { 0x59, 0, 0x0EC, 0, 0 }, { 0x5A, 0, 0x0F6, 0, 0 }, { 0x5B, 0, 0x0DD, 0, 0 }, { 0x5C, 0, 0x0E7, 0, 0 }, { 0x5D, 0, 0x0F1, 0, 0 }, { 0x5E, 0, 0x0C9, 0, 0 },
    { 0x5F, 0, 0x0CB, 0, 0 }, { 0x60, 0, 0x067, 0, 0 }, { 0x61, 0, 0x073, 0, 0 }, { 0x62, 0, 0x07D, 0, 0 }, { 0x63, 0, 0x087, 0, 0 }, { 0x64, 0, 0x091, 0, 0 },
    { 0x65, 0, 0x068, 0, 0 }, { 0x66, 3, 0x133, 11, 0 }, { 0x67, 3, 0x13D, 10, 0 }, { 0x68, 3, 0x147, 3, 0 }, { 0x69, 3, 0x151, 9, 0 }, { 0x6A, 3, 0x15B, 3, 0 },
    { 0x6B, 3, 0x12F, 3, 0 }, { 0x6C, 0, 0x139, 0, 0 }, { 0x6D, 3, 0x143, 12, 0 }, { 0x6E, 3, 0x14D, 10, 0 }, { 0x6F, 3, 0x157, 3, 0 }, { 0x70, 3, 0x130, 5, 0 },
    { 0x71, 3, 0x13B, 8, 0 }, { 0x72, 3, 0x132, 11, 0 }, { 0x73, 3, 0x13C, 8, 0 }, { 0x74, 0, 0x146, 0, 0 }, { 0x75, 3, 0x150, 7, 0 }, { 0x76, 3, 0x15A, 6, 0 },
    { 0x77, 3, 0x194, 0, 0 }, { 0x78, 3, 0x19F, 1, 0 }, { 0x79, 3, 0x1AA, 4, 0 }, { 0x7A, 0, 0x1B5, 0, 0 }, { 0x7B, 3, 0x1C0, 8, 0 }, { 0x7C, 0, 0x199, 0, 0 },
    { 0x7D, 3, 0x1A3, 3, 0 }, { 0x7E, 3, 0x1AD, 12, 0 }, { 0x7F, 3, 0x1B7, 2, 0 }, { 0x80, 0, 0x1C0, 0, 0 }, { 0x81, 3, 0x192, 10, 0 }, { 0x82, 0, 0x1A2, 0, 0 },
    { 0x83, 3, 0x1AB, 2, 0 }, { 0x84, 0, 0x1B4, 0, 0 }, { 0x85, 0, 0x1BD, 0, 0 }, { 0x86, 0, 0x198, 0, 0 }, { 0x87, 0, 0x1AC, 0, 0 }, { 0x88, 3, 0x1C1, 4, 0 },
    { 0x89, 6, 0x3EF, 0, 0 }, { 0x8A, 6, 0x3F0, 0, 0 }, { 0x8B, 6, 0x3F1, 0, 0 }, { 0x8C, 6, 0x3F2, 0, 0 }, { 0x8D, 6, 0x3F3, 0, 0 }, { 0x8E, 6, 0x3F4, 0, 0 },
    { 0x8F, 0, 0x3E9, 0, 0 }, { 0x90, 0, 0x3EA, 0, 0 }, { 0x91, 0, 0x3EB, 0, 0 }, { 0x92, 0, 0x3EC, 0, 0 }, { 0x93, 0, 0x3ED, 0, 0 }, { 0x94, 0, 0x3EE, 0, 0 },
    { 0x95, 0, 0x3EE, 0, 0 }, { 0x96, 0, 0x3EE, 0, 0 }, { 0x97, 0, 0x3EE, 0, 0 }, 
};
static CharmapRevEntry charmap_rev_table[101] = {     { 0x064, 0, 0, 0, 0 }, { 0x065, 0, 0, 1, 0 }, { 0x066, 0, 0, 2, 0 }, { 0x067, 0, 0, 3, 0 }, { 0x068, 0, 0, 4, 0 }, { 0x06D, 0, 0, 9, 0 },
    { 0x06E, 1, 0, 0, 0 }, { 0x06F, 1, 0, 1, 0 }, { 0x070, 1, 0, 2, 0 }, { 0x071, 1, 0, 3, 0 }, { 0x073, 1, 0, 5, 0 }, { 0x074, 1, 0, 6, 0 },
    { 0x076, 1, 0, 8, 0 }, { 0x078, 2, 0, 0, 0 }, { 0x079, 2, 0, 1, 0 }, { 0x07A, 2, 0, 2, 0 }, { 0x07B, 2, 0, 3, 0 }, { 0x07C, 2, 0, 4, 0 },
    { 0x07D, 2, 0, 5, 0 }, { 0x07E, 2, 0, 6, 0 }, { 0x07F, 2, 0, 7, 0 }, { 0x082, 3, 0, 0, 0 }, { 0x083, 3, 0, 1, 0 }, { 0x084, 3, 0, 2, 0 },
    { 0x085, 3, 0, 3, 0 }, { 0x086, 3, 0, 4, 0 }, { 0x087, 3, 0, 5, 0 }, { 0x088, 3, 0, 6, 0 }, { 0x089, 3, 0, 7, 0 }, { 0x08C, 4, 0, 0, 0 },
    { 0x08D, 4, 0, 1, 0 }, { 0x08E, 4, 0, 2, 0 }, { 0x08F, 4, 0, 3, 0 }, { 0x091, 4, 0, 5, 0 }, { 0x094, 4, 0, 8, 0 }, { 0x0C8, 0, 1, 0, 0 },
    { 0x0C9, 0, 1, 1, 0 }, { 0x0CB, 0, 1, 3, 0 }, { 0x0CD, 0, 1, 5, 0 }, { 0x0D2, 1, 1, 0, 0 }, { 0x0D7, 1, 1, 5, 0 }, { 0x0D8, 1, 1, 6, 0 },
    { 0x0DC, 2, 1, 0, 0 }, { 0x0DD, 2, 1, 1, 0 }, { 0x0E1, 2, 1, 5, 0 }, { 0x0E2, 2, 1, 6, 0 }, { 0x0E6, 3, 1, 0, 0 }, { 0x0E7, 3, 1, 1, 0 },
    { 0x0EB, 3, 1, 5, 0 }, { 0x0EC, 3, 1, 6, 0 }, { 0x0F0, 4, 1, 0, 0 }, { 0x0F1, 4, 1, 1, 0 }, { 0x0F5, 4, 1, 5, 0 }, { 0x0F6, 4, 1, 6, 0 },
    { 0x12F, 0, 2, 3, 0 }, { 0x130, 0, 2, 4, 0 }, { 0x132, 0, 2, 6, 0 }, { 0x133, 0, 2, 7, 0 }, { 0x139, 1, 2, 3, 0 }, { 0x13B, 1, 2, 5, 0 },
    { 0x13C, 1, 2, 6, 0 }, { 0x13D, 1, 2, 7, 0 }, { 0x143, 2, 2, 3, 0 }, { 0x146, 2, 2, 6, 0 }, { 0x147, 2, 2, 7, 0 }, { 0x14D, 3, 2, 3, 0 },
    { 0x150, 3, 2, 6, 0 }, { 0x151, 3, 2, 7, 0 }, { 0x157, 4, 2, 3, 0 }, { 0x15A, 4, 2, 6, 0 }, { 0x15B, 4, 2, 7, 0 }, { 0x192, 5, 3, 2, 0 },
    { 0x194, 5, 3, 4, 0 }, { 0x198, 5, 3, 8, 0 }, { 0x199, 5, 3, 9, 0 }, { 0x19F, 6, 3, 5, 0 }, { 0x1A2, 6, 3, 8, 0 }, { 0x1A3, 6, 3, 9, 0 },
    { 0x1AA, 7, 3, 6, 0 }, { 0x1AB, 7, 3, 7, 0 }, { 0x1AC, 7, 3, 8, 0 }, { 0x1AD, 7, 3, 9, 0 }, { 0x1B4, 8, 3, 6, 0 }, { 0x1B5, 8, 3, 7, 0 },
    { 0x1B7, 8, 3, 9, 0 }, { 0x1BD, 9, 3, 5, 0 }, { 0x1C0, 9, 3, 8, 0 }, { 0x1C1, 9, 3, 9, 0 }, { 0x3E8, 0, 0, 0, 0 }, { 0x3E9, 0, 0, 0, 0 },
    { 0x3EA, 0, 0, 0, 0 }, { 0x3EB, 0, 0, 0, 0 }, { 0x3EC, 0, 0, 0, 0 }, { 0x3ED, 0, 0, 0, 0 }, { 0x3EE, 0, 0, 0, 0 }, { 0x3EF, 0, 0, 0, 0 },
    { 0x3F0, 0, 0, 0, 0 }, { 0x3F1, 0, 0, 0, 0 }, { 0x3F2, 0, 0, 0, 0 }, { 0x3F3, 0, 0, 0, 0 }, { 0x3F4, 0, 0, 0, 0 }, 
};
static unsigned short excluded_card_ids[3] = { 0x143D, 0x1445, 0x1446 };

/* Binary search of `ch`; returns the glyph code (and the kind nibble) or -1. */
int charmap_lookup(int ch, int *kind_out)
{
    int lo = 0;
    int hi = 110;
    int mid;
    CharmapEntry *e;

    do {
        mid = (lo + hi) / 2;
        e = &charmap_table[mid];
        if (e->ch == ch) {
            if (kind_out != 0)
                *kind_out = e->kind & 0xF;
            return e->code;
        }
        if (e->ch < ch)
            lo = mid + 1;
        else
            hi = mid - 1;
    } while (lo <= hi);
    return -1;
}

int charmap_lookup3(int ch)
{
    int lo = 0;
    int hi = 110;
    int mid;
    CharmapEntry *e;

    do {
        mid = (lo + hi) / 2;
        e = &charmap_table[mid];
        if (e->ch == ch)
            return e->x & 0xF;
        if (e->ch < ch)
            lo = mid + 1;
        else
            hi = mid - 1;
    } while (lo <= hi);
    return -1;
}

int charmap_lookup2(int code, CharmapPos *out)
{
    int lo = 0;
    int hi = 100;
    int mid;
    CharmapRevEntry *e;

    do {
        mid = (lo + hi) / 2;
        e = &charmap_rev_table[mid];
        if (e->code == code) {
            out->a = e->a;
            out->b = e->b;
            out->c = e->c;
            return 1;
        }
        if (e->code < code)
            lo = mid + 1;
        else
            hi = mid - 1;
    } while (lo <= hi);
    return 0;
}

unsigned short partner_table_lookup(int flag, unsigned short mode, unsigned short idx, unsigned short *table)
{
    unsigned short off;

    if (table == 0)
        return 0;
    if (mode >= 3)
        return 0;
    if (flag) {
        if (idx >= 0x90)
            return 0;
        off = idx + mode * 0x90;
    } else {
        if (idx >= 0x35)
            return 0;
        off = idx + mode * 0x35;
    }
    return table[off];
}

int card_is_excluded(unsigned short id)
{
    int lo = 0;
    int hi = 2;
    int mid;

    do {
        mid = (lo + hi) / 2;
        if (id == excluded_card_ids[mid])
            return 1;
        if (excluded_card_ids[mid] < id)
            lo = mid + 1;
        else
            hi = mid - 1;
    } while (lo <= hi);
    return 0;
}

/* Per-mille of the collectable cards owned (excluding attribute-9 cards and three fixed ids). */
unsigned short inv_collection_permille(void)
{
    unsigned short owned = 0;
    unsigned short total = 0;
    int id;

    for (id = 0xFA7; id < 0x1B2E; id++) {
        if (card_get_flag31(id) == 0)
            continue;
        if (card_is_attribute9(id))
            continue;
        if (card_is_excluded(id))
            continue;
        if (inv_get_state(id))
            owned++;
        total++;
    }
    return owned * 1000 / total;
}

int wcscmp(const unsigned short *a, const unsigned short *b)
{
    while (*a == *b) {
        if (*a == 0)
            return 0;
        a++;
        b++;
    }
    return *a - *b;
}

int wcslen(const unsigned short *s)
{
    int n = 0;

    while (*s != 0) {
        s++;
        n++;
    }
    return n;
}

int wcschr_index(const unsigned short *s, unsigned short c)
{
    int i = 0;
    const unsigned short *p = s;

    while (*p) {
        if (c == *p)
            return i;
        p++;
        i++;
    }
    return -1;
}

unsigned short *wcscpy(unsigned short *d, const unsigned short *s)
{
    unsigned short *p = d;

    while ((*p++ = *s++) != 0)
        ;
    return d;
}

unsigned short *wcsncpy(unsigned short *d, const unsigned short *s, unsigned int n)
{
    unsigned short *p = d;
    unsigned int i;

    for (i = 0; i < n; i++) {
        if (*s == 0) {
            *p = 0;
            p++;
        } else {
            *p = *s;
            s++;
            p++;
        }
    }
    return d;
}

unsigned short *wcscat(unsigned short *d, const unsigned short *s)
{
    wcscpy(d + wcslen(d), s);
    return d;
}

unsigned short *wcsncat(unsigned short *d, const unsigned short *s, unsigned int n)
{
    wcsncpy(d + wcslen(d), s, n);
    return d;
}
