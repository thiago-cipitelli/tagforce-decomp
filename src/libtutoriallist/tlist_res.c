// @range 0x08805B0C-0x08805F10
// @data 0x0880A0F4-0x0880A174
/* Resources of the screen: database/tutoriallist.ehp is read asynchronously into the heap and
   mounted; the textures, background and per-language string table are looked up by id on demand. */
#include "libtutoriallist.h"

typedef struct TlRes {
    unsigned short id;
    void *data;                /* found entry, cached */
    const char *name;          /* "%c" = language letter */
} TlRes;

int tlres_cmp(const void *a, const void *b);
void tlres_free_pack(void);
int tlres_loaded_cb(void *pack, int size);
int tlres_bsearch(unsigned short key, void *base, int n, int size);

TlRes tlres_table[4] = {
    { 0x1002, 0, "tab.gim" },
    { 0x1003, 0, "parts.gim" },
    { 0x1011, 0, "bg.ehf" },
    { 0x1020, 0, "strTbl_%c.bin" },
};

void tlres_init(void)
{
    memset(&tlres_flags, 0, 1);
    tlres_flags.ready = 1;
    qsort(tlres_table, 4, sizeof(TlRes), tlres_cmp);
}

/* 1 once the pack is mounted */
int tlres_poll(void)
{
    if (tlres_flags.ready) {
        switch (tlres_flags.step) {
        case 0:
            if (tlres_load())
                tlres_flags.step++;
            break;
        default:
            tlres_flags.step = 0;
            return 1;
        }
    }
    return 0;
}

void tlres_free(void)
{
    tlres_free_pack();
}

void tlres_free_pack(void)
{
    if (tlres_pack) {
        heap_set_level(tl.heap, tlres_pack);
        tlres_pack = 0;
    }
}

/* starts the read the first time; the pack once it is in memory */
void *tlres_load(void)
{
    int size;

    if (tlres_pack)
        return tlres_pack;
    if (tlres_flags.ready) {
        tlres_flags.ready = 0;
        size = ehfile_get_size(3, "database/tutoriallist.ehp");
        tlres_pack = heap_alloc(tl.heap, size);
        ehfile_read_async_alias(3, "database/tutoriallist.ehp", tlres_pack, size, tlres_loaded_cb, tlres_pack);
    }
    return 0;
}

void *tlres_get(unsigned short id)
{
    TlRes *r;
    void *pack;
    int i = tlres_bsearch(id, tlres_table, 4, sizeof(TlRes));

    if (i < 0)
        return 0;
    r = &tlres_table[i];
    if (r->data)
        return r->data;
    pack = tlres_load();
    {
        char name[256];
        char codes[6] = "jegfis";   /* language letters by cfg_get_language() */
        if (pack == 0)
            return 0;
        sprintf(name, r->name, codes[cfg_get_language()]);
        return r->data = ehp_find(pack, name, 0);
    }
}

int tlres_loaded_cb(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack)
            ehp_mount(0, pack);
        tlres_flags.step++;
    }
    tlres_flags.ready = 1;
    return ok;
}

int tlres_cmp(const void *a, const void *b)
{
    return *(const unsigned short *)a - *(const unsigned short *)b;
}

/* binary search over records of `size` bytes keyed by their first u16 */
int tlres_bsearch(unsigned short key, void *base, int n, int size)
{
    int half = size / 2;
    int lo = 0;
    int hi = n - 1;
    int mid;
    unsigned short v;

    while (lo <= hi) {
        mid = (lo + hi) / 2;
        v = ((unsigned short *)base)[mid * half];
        if (key == v)
            return mid;
        if (v < key)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}
