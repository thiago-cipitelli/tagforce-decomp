// @range 0x08804000-0x08804778
// @rodata 0x0880E4D4-0x0880E4E8
// @data 0x0880E564-0x0880E888
/* Resources of the screen: two packs (deck/all_<l>.ehp with the card art, unless the caller
   already has it in memory, and deck/recipe_view_<l>.ehp with the screen's textures and frames)
   read asynchronously into the heap and mounted, the CIP card picture reader and the BOOT card
   list; entries looked up by id on demand. Same code as liblimitlist's ll_res.c. */
#include "librecipe_viewer.h"

typedef struct RvPack {
    unsigned short id;
    void *data;                /* pack in memory */
    const char *name;          /* "%c" = language letter */
} RvPack;

typedef struct RvRes {
    unsigned short id;         /* 0x8000 = entry of the main table, 0x1000 = small table */
    unsigned short pack;       /* RvPack id */
    void *data;                /* found entry, cached */
    const char *name;
} RvRes;

int rvres_cmp(const void *a, const void *b);
static int rvres_bsearch(unsigned short key, RvRes *tab, int n);
void rvres_free_all(void);
void *rvres_load_pack(unsigned short id);
int rvres_loaded_cb(void *pack, int size);

const int rvres_rodata_pad[3] = { 0, 0, 0 };             /* start of the module's read-only data */
static const unsigned char rvres_lang[8] = "jegfis";   /* language letters by cfg_get_language() */

RvPack rvres_packs[2] = {
    { 1, 0, "deck/all_%c.ehp" },
    { 2, 0, "deck/recipe_view_%c.ehp" },
};

RvRes rvres_table[25] = {
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

RvRes rvres_table2[3] = {
    { 0x1001, 2, 0, "panel.gim" },
    { 0x1010, 2, 0, "recipe01.ehf" },
    { 0x1011, 2, 0, "title.ehf" },
};

int rvres_cmp(const void *a, const void *b)
{
    return *(const unsigned short *)a - *(const unsigned short *)b;
}

static int rvres_bsearch(unsigned short key, RvRes *tab, int n)
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

void rvres_init(void)
{
    memset(&rvres_flags, 0, 1);
    rvres_flags.ready = 1;
    qsort(rvres_table, 25, sizeof(RvRes), rvres_cmp);
    qsort(rvres_table2, 3, sizeof(RvRes), rvres_cmp);
}

/* 1 once both packs are mounted, the CIP reader is open and the card list is set up */
int rvres_poll(void)
{
    if (rvres_flags.ready) {
        unsigned char step = rvres_flags.step;

        switch (step) {
        case 0:
            if (rv.deck_pack) {
                unsigned int i;

                for (i = 0; i < 2; i++) {
                    if (rvres_packs[i].id == 1) {
                        rvres_packs[i].data = rv.deck_pack;
                        break;
                    }
                }
                rvres_flags.step++;
            } else if (rvres_load_pack(1)) {
                rvres_flags.step++;
            }
            break;
        case 1:
            if (!rvres_load_pack(2))
                break;
            rvres_flags.step++;
        case 2:
            if (rv.cip == 0) {
                rvres_cipbuf = heap_alloc(rv.heap, 0x132000);
                rv.cip = cip_open_h(rvres_cipbuf, 0x132000, 0);
                rvres_flags.step++;
            } else {
                rv.scroll.card = -1;
                rv.scroll.pos = 0;
                rvres_flags.step = 0;
                return 1;
            }
            break;
        case 3:
            if (cip_is_loaded(rv.cip)) {
                CardListCfg cfg;
                memset(&cfg, 0, sizeof(cfg));
                cfg.gim = rvres_get(0x8007);
                cfg.digits_gim = rvres_get(0x8009);
                cfg.frame_ehf = rvres_get(0x8023);
                cfg.attr_gim = rvres_get(0x8011);
                cfg.kind_gim = rvres_get(0x8010);
                cfg.cip = rv.cip;
                cardlist_init(&cfg);
                rv.scroll.card = -1;
                rv.scroll.pos = 0;
                rvres_flags.step++;
            }
            break;
        default:
            rvres_flags.step = 0;
            return 1;
        }
    }
    return 0;
}

void rvres_free(void)
{
    rvres_free_all();
}

void rvres_free_all(void)
{
    unsigned int i;

    if (rvres_cipbuf) {
        heap_set_level(rv.heap, rvres_cipbuf);
        rvres_cipbuf = 0;
    }
    for (i = 0; i < 2; i++) {
        if (rvres_packs[i].data) {
            heap_set_level(rv.heap, rvres_packs[i].data);
            rvres_packs[i].data = 0;
        }
    }
}

/* starts the read of pack `id` the first time; the pack once it is in memory */
void *rvres_load_pack(unsigned short id)
{
    int found = -1;
    unsigned int i;
    RvPack *p;
    char name[256];
    int size;

    for (i = 0; i < 2; i++) {
        if (id == rvres_packs[i].id) {
            found = i;
            break;
        }
    }
    if (found < 0)
        return 0;
    p = &rvres_packs[found];
    if (p->data)
        return p->data;
    if (rvres_flags.ready) {
        sprintf(name, p->name, rvres_lang[cfg_get_language()]);
        rvres_flags.ready = 0;
        size = ehfile_get_size(3, name);
        p->data = heap_alloc(rv.heap, size);
        ehfile_read_async_alias(3, name, p->data, size, rvres_loaded_cb, p->data);
    }
    return 0;
}

/* entry `id` of the packs (0x1000 ids: small table, others: main table), cached */
void *rvres_get(unsigned short id)
{
    RvRes *tab;
    int n;
    int i;
    RvRes *r;
    void *pack;
    char name[256];

    if (id & 0x1000) {
        tab = rvres_table2;
        n = 3;
    } else {
        tab = rvres_table;
        n = 25;
    }
    i = rvres_bsearch(id, tab, n);
    if (i < 0)
        return 0;
    r = &tab[i];
    if (r->data)
        return r->data;
    pack = rvres_load_pack(r->pack);
    if (pack == 0)
        return 0;
    if (id == 0x8036 || id == 0x8037) {
        char regions[7] = { 'j', 'a', 'g', 'f', 'i', 's', 'b' };   /* letters by region_get() */
        sprintf(name, r->name, regions[region_get()]);
    } else {
        sprintf(name, r->name, rvres_lang[cfg_get_language()]);
    }
    return r->data = ehp_find(pack, name, 0);
}

int rvres_loaded_cb(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack)
            ehp_mount(0, pack);
        rvres_flags.step++;
    }
    rvres_flags.ready = 1;
    return ok;
}
