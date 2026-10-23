// @range 0x08805A00-0x088060C4
// @rodata 0x0880D094-0x0880D09C
// @data 0x0880D2B4-0x0880D5E8
/* Resources of the screen: two packs (deck/all_<l>.ehp with the card art, database/limitlist.ehp
   with the screen's textures, frames and string tables) read asynchronously into the heap and
   mounted, the CIP card picture reader and the BOOT card list; entries looked up by id on demand. */
#include "liblimitlist.h"

typedef struct LlPack {
    unsigned short id;
    void *data;                /* pack in memory */
    const char *name;          /* "%c" = language letter */
} LlPack;

typedef struct LlRes {
    unsigned short id;         /* 0x8000 = entry of the main table, 0x1000 = small table */
    unsigned short pack;       /* LlPack id */
    void *data;                /* found entry, cached */
    const char *name;
} LlRes;

int llres_cmp(const void *a, const void *b);
static int llres_bsearch(unsigned short key, LlRes *tab, int n);
void llres_free_all(void);
void *llres_load_pack(unsigned short id);
int llres_loaded_cb(void *pack, int size);

static const unsigned char llres_lang[8] = "jegfis";   /* language letters by cfg_get_language() */

LlPack llres_packs[2] = {
    { 1, 0, "deck/all_%c.ehp" },
    { 2, 0, "database/limitlist.ehp" },
};

LlRes llres_table[25] = {
    { 0x8001, 1, 0, "cis_all.gim" },
    { 0x8002, 1, 0, "ciss.gim" },
    { 0x8006, 1, 0, "panel.gim" },
    { 0x8007, 1, 0, "icon01.gim" },
    { 0x8008, 1, 0, "icon02.gim" },
    { 0x8009, 1, 0, "icon03.gim" },
    { 0x800A, 1, 0, "icon04.gim" },
    { 0x800B, 1, 0, "icon05.gim" },
    { 0x800C, 1, 0, "icon06.gim" },
    { 0x8010, 1, 0, "spelltrap.gim" },
    { 0x8011, 1, 0, "frame.gim" },
    { 0x8012, 1, 0, "detail_bg.gim" },
    { 0x8013, 1, 0, "s_cut.gim" },
    { 0x8014, 1, 0, "help_menu.gim" },
    { 0x8015, 1, 0, "ocglist.gim" },
    { 0x8021, 1, 0, "bg.ehf" },
    { 0x8022, 1, 0, "d_window0.ehf" },
    { 0x8023, 1, 0, "score-atkdef10.ehf" },
    { 0x8031, 1, 0, "fileTbl.dat" },
    { 0x8032, 1, 0, "sameCardTbl.dat" },
    { 0x8033, 1, 0, "strTbl.bin" },
    { 0x8034, 1, 0, "help_strtbl_contents.bin" },
    { 0x8035, 1, 0, "help_strtbl_label.bin" },
    { 0x8036, 1, 0, "ocg_packname_%c.bin" },
    { 0x8037, 1, 0, "ocg_packlist_%c.bin" },
};

LlRes llres_table2[3] = {
    { 0x1002, 2, 0, "parts.gim" },
    { 0x1010, 2, 0, "window_%c.ehf" },
    { 0x1020, 2, 0, "strTbl_%c.bin" },
};

int llres_cmp(const void *a, const void *b)
{
    return *(const unsigned short *)a - *(const unsigned short *)b;
}

static int llres_bsearch(unsigned short key, LlRes *tab, int n)
{
    int lo = 0;
    int hi = n - 1;
    int mid;
    unsigned short v;

    while (lo <= hi) {
        mid = (lo + hi) / 2;
        v = tab[mid].id;
        if (key == v)
            return mid;
        if (v < key)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}

void llres_init(void)
{
    memset(&llres_flags, 0, 1);
    llres_flags.ready = 1;
    qsort(llres_table, 25, sizeof(LlRes), llres_cmp);
    qsort(llres_table2, 3, sizeof(LlRes), llres_cmp);
}

/* 1 once both packs are mounted, the CIP reader is open and the card list is set up */
int llres_poll(void)
{
    if (llres_flags.ready) {
        switch (llres_flags.step) {
        case 0:
            if (llres_load_pack(1))
                llres_flags.step++;
            break;
        case 1:
            if (llres_load_pack(2))
                llres_flags.step++;
            break;
        case 2:
            llres_cipbuf = heap_alloc(ll.heap, 0x132000);
            ll.cip = cip_open_h(llres_cipbuf, 0x132000, 0);
            llres_flags.step++;
            break;
        case 3:
            if (cip_is_loaded(ll.cip)) {
                CardListCfg cfg;
                memset(&cfg, 0, sizeof(cfg));
                cfg.gim = llres_get(0x8007);
                cfg.digits_gim = llres_get(0x8009);
                cfg.frame_ehf = llres_get(0x8023);
                cfg.attr_gim = llres_get(0x8011);
                cfg.kind_gim = llres_get(0x8010);
                cfg.cip = ll.cip;
                cardlist_init(&cfg);
                ll.scroll.card = -1;
                ll.scroll.pos = 0;
                llres_flags.step++;
            }
            break;
        default:
            llres_flags.step = 0;
            return 1;
        }
    }
    return 0;
}

void llres_free(void)
{
    llres_free_all();
}

void llres_free_all(void)
{
    unsigned int i;

    if (llres_cipbuf) {
        heap_set_level(ll.heap, llres_cipbuf);
        llres_cipbuf = 0;
    }
    for (i = 0; i < 2; i++) {
        if (llres_packs[i].data) {
            heap_set_level(ll.heap, llres_packs[i].data);
            llres_packs[i].data = 0;
        }
    }
}

/* starts the read of pack `id` the first time; the pack once it is in memory */
void *llres_load_pack(unsigned short id)
{
    int found = -1;
    unsigned int i;
    LlPack *p;
    char name[256];
    int size;

    for (i = 0; i < 2; i++) {
        if (id == llres_packs[i].id) {
            found = i;
            break;
        }
    }
    if (found < 0)
        return 0;
    p = &llres_packs[found];
    if (p == 0)
        return 0;
    if (p->data)
        return p->data;
    if (llres_flags.ready) {
        sprintf(name, p->name, llres_lang[cfg_get_language()]);
        llres_flags.ready = 0;
        size = ehfile_get_size(3, name);
        p->data = heap_alloc(ll.heap, size);
        ehfile_read_async_alias(3, name, p->data, size, llres_loaded_cb, p->data);
    }
    return 0;
}

/* entry `id` of the packs (0x8000 ids: main table, 0x1000 ids: small table), cached */
void *llres_get(unsigned short id)
{
    LlRes *tab;
    int n;
    int i;
    LlRes *r;
    void *pack;
    char name[256];

    if (id & 0x8000) {
        tab = llres_table;
        n = 25;
    } else if (id & 0x1000) {
        tab = llres_table2;
        n = 3;
    }
    i = llres_bsearch(id, tab, n);
    if (i < 0)
        return 0;
    r = &tab[i];
    if (r->data)
        return r->data;
    pack = llres_load_pack(r->pack);
    if (pack == 0)
        return 0;
    if (id == 0x8036 || id == 0x8037) {
        char regions[7] = { 'j', 'a', 'g', 'f', 'i', 's', 'b' };   /* letters by region_get() */
        sprintf(name, r->name, regions[region_get()]);
    } else {
        sprintf(name, r->name, llres_lang[cfg_get_language()]);
    }
    return r->data = ehp_find(pack, name, 0);
}

int llres_loaded_cb(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack)
            ehp_mount(0, pack);
        llres_flags.step++;
    }
    llres_flags.ready = 1;
    return ok;
}
