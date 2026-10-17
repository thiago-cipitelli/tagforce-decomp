// @range 0x0886CF50-0x0886E0A4
// @data 0x088F0C94-0x088F0CB0
// @asm 0x0886DAB0-0x0886DE28
/* Extra data files on the Memory Stick: the save folder may hold "YGTF" files with a card
   (.YGC), a deck recipe (.YGR) or a card list (.YGL). extdata_scan lists the files of one kind
   behind a "please wait" message box, extdata_load reads and verifies them (two checksums, a
   header checksum and a byte shuffle seeded from the header) into the caller's array. */
#include "psp.h"
#include "libc.h"
#include "card_db.h"

typedef struct ExtData {               /* 0x2814, heap-allocated */
    void *heap;                        /* +0x000 */
    unsigned char kind : 3;            /* +0x004 0 .YGC, 1 .YGR, 2 .YGL */
    unsigned char state : 4;
    unsigned char pad4 : 1;
    unsigned char pad5;
    unsigned short wait;               /* +0x006 */
    unsigned short index;              /* +0x008 next file to load */
    unsigned short count;              /* +0x00A entries loaded */
    unsigned char busy : 1;            /* +0x00C message box open */
    unsigned char padc : 7;
    unsigned char padd;
    unsigned short text[0x201];        /* +0x00E */
    int nfiles;                        /* +0x410 */
    int size[256];                     /* +0x414 */
    char name[256][32];                /* +0x814 */
} ExtData;

typedef struct ExtCard {               /* 0x44: one .YGC */
    unsigned short name[32];           /* +0x00 file name */
    unsigned short id;                 /* +0x40 */
    unsigned char invalid : 1;         /* +0x42 */
    unsigned char pad42 : 7;
    unsigned char pad43;
} ExtCard;

typedef struct ExtDeck {               /* 0x108 */
    unsigned int main_count;           /* +0x00 */
    unsigned int extra_count;          /* +0x04 */
    unsigned int side_count;           /* +0x08 */
    unsigned short main[80];           /* +0x0C */
    unsigned short extra[15];          /* +0xAC */
    unsigned short side[30];           /* +0xCA */
    unsigned short pad106;
} ExtDeck;

typedef struct ExtRecipe {             /* 0x208: one .YGR */
    unsigned short name[0x80];         /* +0x000 */
    ExtDeck deck;                      /* +0x100 */
} ExtRecipe;

typedef struct ExtList {               /* 0x108: one .YGL */
    unsigned short name[0x80];         /* +0x000 */
    unsigned short count;              /* +0x100 */
    unsigned short pad102;
    unsigned short *ids;               /* +0x104 */
} ExtList;

extern ExtData *extdata;               /* 0x08E34BEC */
extern int rand3_slot;
extern unsigned int rand3_seeds[][3];

signed char ygtf_magic[4] = { 'Y', 'G', 'T', 'F' };

void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
unsigned short *sysmsg_get(int i);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
int msgbox_open(unsigned short *text, void *heap, int type, int flag);
int msgbox_update(void **dl);
int msgbox_close(void);
void se_play(int id);
void sysvar_set_a90(int v);
int cfg_get_language(void);
unsigned short *wcsncpy(unsigned short *dst, unsigned short *src, int n);
void savedata_dir_path(char *out);
int savedata_read_file(const char *name, void *buf, int size);
int memcmp(const void *a, const void *b, int n);

int extdata_verify(int kind, unsigned short *buf, unsigned int size);
void extdata_list_files(void);
int extdata_load_card(char *name, int size, ExtCard *out);
int extdata_load_recipe(char *name, int size, ExtRecipe *out);
int extdata_load_list(char *name, int size, ExtList *out);
void extdata_unscramble(unsigned int seed, unsigned char *buf, int len);
void extdata_get_name(unsigned char *file, unsigned short *out);

/* Exported: allocates the state and shows the "checking" message. */
void extdata_open(void *heap)
{
    extdata = heap_alloc(heap, 0x2814);
    if (extdata != 0) {
        memset(extdata, 0, 0x2814);
        extdata->heap = heap;
        wsprintf(extdata->text, sysmsg_get(1), 0x2122, 0x2122);
        msgbox_open(extdata->text, extdata->heap, 0, 0);
        se_play(0x621A);
        extdata->busy = 1;
    }
}

/* Exported: lists the files of `kind`; returns 1 when done with their number in *count. */
int extdata_scan(int kind, int *count)
{
    if (extdata == 0) {
        *count = 0;
        return 1;
    }
    switch (extdata->state) {
    case 0:
        if (extdata->wait++ > 8) {
            extdata->state++;
            extdata->wait = 0;
            sysvar_set_a90(1);
        }
        break;
    case 1:
        extdata->kind = kind;
        extdata_list_files();
        msgbox_close();
        extdata->state++;
        break;
    case 2:
        if (extdata->busy == 0) {
            extdata->state = 0;
            extdata->wait = 0;
            *count = extdata->nfiles;
            if (extdata->nfiles <= 0) {
                heap_set_level(extdata->heap, extdata);
                extdata = 0;
            }
            sysvar_set_a90(0);
            return 1;
        }
        break;
    }
    return 0;
}

/* Exported: loads up to 8 files per frame into `out` (ExtCard/ExtRecipe/ExtList by kind). */
int extdata_load(void *out, int *count)
{
    int i;

    if (extdata == 0) {
        return 1;
    }
    switch (extdata->state) {
    case 0:
        wsprintf(extdata->text, sysmsg_get(8), 0x2122);
        msgbox_open(extdata->text, extdata->heap, 0, 0);
        se_play(0x621A);
        extdata->busy = 1;
        extdata->state++;
        extdata->wait = 0;
        extdata->index = 0;
        extdata->count = 0;
        sysvar_set_a90(1);
        break;
    case 1:
        i = 0;
        do {
            if (extdata->index >= extdata->nfiles) {
                extdata->state++;
                break;
            }
            switch (extdata->kind) {
            case 0:
                if (extdata_load_card(extdata->name[extdata->index], extdata->size[extdata->index],
                                      (ExtCard *)out + extdata->count) != 0) {
                    ((ExtCard *)out)[extdata->count].invalid = 0;
                } else {
                    ((ExtCard *)out)[extdata->count].invalid = 1;
                }
                extdata->count++;
                break;
            case 1:
                if (extdata_load_recipe(extdata->name[extdata->index], extdata->size[extdata->index],
                                        (ExtRecipe *)out + extdata->count) != 0) {
                    extdata->count++;
                }
                break;
            case 2:
                if (extdata_load_list(extdata->name[extdata->index], extdata->size[extdata->index],
                                      (ExtList *)out + extdata->count) != 0) {
                    extdata->count++;
                }
                break;
            }
            i++;
            extdata->index++;
        } while (i < 8);
        extdata->wait++;
        break;
    case 2:
        if (extdata->wait++ > 8) {
            extdata->state++;
            extdata->wait = 0;
            msgbox_close();
        }
        break;
    case 3:
        if (extdata->busy == 0) {
            *count = extdata->count;
            heap_set_level(extdata->heap, extdata);
            extdata = 0;
            sysvar_set_a90(0);
            return 1;
        }
        break;
    }
    return 0;
}

/* Exported: draws the message box; returns the display list pointer. */
void *extdata_draw(void *dl)
{
    if (extdata == 0) {
        return dl;
    }
    if (extdata->busy && msgbox_update(&dl) != 1) {
        extdata->busy = 0;
    }
    return dl;
}

/* The file's title in the current language: a run of (length, UCS-2 text) per language. */
void extdata_get_name(unsigned char *file, unsigned short *out)
{
    unsigned short *p = (unsigned short *)(file + 0xC);
    int i = 0;

    while (i < cfg_get_language()) {
        i++;
        p += *p + 1;
    }
    wcsncpy(out, p + 1, *p);
}

int extdata_load_card(char *name, int size, ExtCard *out)
{
    unsigned char c;
    unsigned char *buf;
    int i;
    unsigned short *d;

    i = 0;
    d = out->name;
    do {
        c = name[i];
        *d = c;
        if (c == 0) {
            break;
        }
        i++;
        d++;
    } while (i < 0x20);
    buf = heap_alloc(extdata->heap, size);
    if (buf == 0) {
        return 0;
    }
    if (savedata_read_file(name, buf, size) == 0) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    if (extdata_verify(0, (unsigned short *)buf, size) == 0) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    memcpy(&out->id, buf + *(int *)(buf + 8), 2);
    if (card_get_flag31(out->id) == 0) {
        out->id = 0;
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    heap_set_level(extdata->heap, buf);
    return 1;
}

#define RD16(p) ((p)[0] | (p)[1] << 8)

int extdata_load_recipe(char *name, int size, ExtRecipe *out)
{
    unsigned char *buf;
    unsigned char *p;
    ExtDeck *d;
    unsigned short id;
    int n;
    unsigned int i;

    buf = heap_alloc(extdata->heap, size);
    if (buf == 0) {
        return 0;
    }
    if (savedata_read_file(name, buf, size) == 0) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    if (extdata_verify(1, (unsigned short *)buf, size) == 0) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    extdata_get_name(buf, out->name);
    p = buf + *(int *)(buf + 8);
    if (size - *(int *)(buf + 8) > 0x200) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    d = &out->deck;
    memset(d, 0, 0x108);
    d->main_count = (unsigned short)RD16(p + 8);
    p += 10;
    if (d->main_count > 80 || d->main_count < 40) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    n = 0;
    for (i = 0; i < d->main_count; i++) {
        id = RD16(p);
        p += 2;
        if (card_get_flag31(id)) {
            d->main[n++] = id;
        }
    }
    d->main_count = n;
    d->side_count = (unsigned short)RD16(p);
    p += 2;
    if (d->side_count > 30) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    n = 0;
    for (i = 0; i < d->side_count; i++) {
        id = RD16(p);
        p += 2;
        if (card_get_flag31(id)) {
            d->side[n++] = id;
        }
    }
    d->side_count = n;
    d->extra_count = (unsigned short)RD16(p);
    p += 2;
    if (d->extra_count > 15) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    n = 0;
    for (i = 0; i < d->extra_count; i++) {
        id = RD16(p);
        p += 2;
        if (card_get_flag31(id)) {
            d->extra[n++] = id;
        }
    }
    d->extra_count = n;
    heap_set_level(extdata->heap, buf);
    return 1;
}

#ifdef NON_MATCHING /* load_list: the original computes count << 1 before the 0xA28 test; verify: sum and index
                       registers swapped in the header loop, magic compared through an indexed address */
int extdata_load_list(char *name, int size, ExtList *out)
{
    unsigned char *buf;
    unsigned char *p;
    int off;
    int i;

    buf = heap_alloc(extdata->heap, size);
    if (buf == 0) {
        return 0;
    }
    if (savedata_read_file(name, buf, size) == 0) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    if (extdata_verify(2, (unsigned short *)buf, size) == 0) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    extdata_get_name(buf, out->name);
    off = *(int *)(buf + 8);
    p = buf + off;
    out->count = RD16(p);
    p += 2;
    out->ids = heap_alloc(extdata->heap, out->count << 2);
    if ((unsigned int)(size - off) < (unsigned int)(out->count << 2)) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    if (out->count > 0xA28) {
        heap_set_level(extdata->heap, buf);
        return 0;
    }
    if (out->ids != 0) {
        for (i = 0; i < out->count << 1; i++) {
            out->ids[i] = RD16(p);
            p += 2;
        }
    }
    heap_set_level(extdata->heap, buf);
    return 1;
}

int extdata_verify(int kind, unsigned short *buf, unsigned int size)
{
    unsigned char *p;
    unsigned short sum;
    unsigned int i;
    unsigned int n;
    int k;
    int hdr;
    int len;

    if (size < 12) {
        return 0;
    }
    for (k = 0; k < 4; k++) {
        if (ygtf_magic[k] != ((signed char *)buf)[k]) {
            return 0;
        }
    }
    if (buf[3] != (unsigned short)kind) {
        return 0;
    }
    p = (unsigned char *)buf;
    sum = 0x573;
    n = size - 2;
    i = 0;
    if (n != 0) {
        do {
            i++;
            sum += *p;
            sum = ~sum;
            p++;
        } while (i < n);
    }
    if (sum != (unsigned short)RD16(p)) {
        return 0;
    }
    hdr = *(int *)(buf + 4);
    p = (unsigned char *)buf;
    sum = 0x4654;
    n = hdr - 2;
    i = 0;
    if (n != 0) {
        do {
            i++;
            sum += *p;
            p++;
        } while (i < n);
    }
    if (sum != (unsigned short)RD16(p)) {
        return 0;
    }
    p = (unsigned char *)buf + hdr;
    len = size - hdr - 2;
    extdata_unscramble(buf[2], p, len);
    sum = 0x5847;
    n = len - 2;
    i = 0;
    if (n != 0) {
        do {
            k = i & 3;
            i++;
            sum += (unsigned short)(*p >> k);
            p++;
        } while (i < n);
    }
    return sum == (unsigned short)RD16(p);
}
#endif

#define RAND3() (rand3_seeds[rand3_slot][0] *= 0x41C64E6D, rand3_seeds[rand3_slot][0] += 0x3039, \
                 (int)((rand3_seeds[rand3_slot][0] >> 16) & 0x7FFF))

/* Undoes the shuffle: `seed` drives (rand & 0x1FF) random byte swaps. */
void extdata_unscramble(unsigned int seed, unsigned char *buf, int len)
{
    int j;
    int k;
    unsigned char t;
    int n;
    int i;

    rand3_seeds[rand3_slot][0] = seed & 0xFFFF;
    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    n = (rand3_seeds[rand3_slot][0] >> 16) & 0x1FF;
    for (i = 0; i < n; i++) {
        j = RAND3() % len;
        k = RAND3() % len;
        t = buf[j];
        buf[j] = buf[k];
        buf[k] = t;
    }
}

/* Collects up to 256 names (and sizes) of files of the current kind in the save folder. */
void extdata_list_files(void)
{
    SceIoDirent ent;
    char path[64];
    char ext[8];
    int fd;
    int n;

    if (extdata == 0) {
        return;
    }
    memset(&ent, 0, sizeof(ent));
    extdata->nfiles = 0;
    savedata_dir_path(path);
    fd = sceIoDopen(path);
    if (fd < 0) {
        return;
    }
    switch (extdata->kind) {
    case 2:
        strcpy(ext, ".YGL");
        break;
    case 1:
        strcpy(ext, ".YGR");
        break;
    case 0:
        strcpy(ext, ".YGC");
        break;
    }
    while (extdata->nfiles < 256) {
        if (sceIoDread(fd, &ent) <= 0) {
            break;
        }
        n = strlen(ent.d_name);
        if (n > 4 && memcmp(ent.d_name + (n - 4), ext, 4) == 0) {
            extdata->size[extdata->nfiles] = ent.d_stat.st_size;
            strcpy(extdata->name[extdata->nfiles], ent.d_name);
            extdata->nfiles++;
        }
    }
    sceIoDclose(fd);
}
