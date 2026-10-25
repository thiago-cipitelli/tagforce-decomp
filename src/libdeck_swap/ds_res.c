// @range 0x08804000-0x08804608
// @rodata 0x08819C00-0x08819C08
// @data 0x08819CFC-0x0881A000
/* Resources: the deck/all_%c.ehp and deck/deckswap.ehp packs (read asynchronously, language letter
   in the names) and the resources looked up in them by id through tables sorted at startup; the
   load steps and the setup of the screen once the packs are in (same source as libselect_card
   sc_main.c). */
#include "libdeck_swap.h"

typedef struct DsCardListCfg {         /* BOOT cardlist_init */
    void *gim;
    void *digits_gim;
    void *frame_ehf;
    void *attr_gim;
    void *kind_gim;
    void *cil_gim;
    void *icon_gim;
    void *cip;
    int frame_view;
} DsCardListCfg;

int cfg_get_language(void);
int region_get(void);
int sprintf(char *buf, const char *fmt, ...);
int cfg_get_bits_1_0_2(void);
void sysstate_blob_get(void *dst);
void cardlist_init(void *cfg);
void ds_lists_init(void);
void ds_set_mode(unsigned short mode);
void ds_tabs_init(void);
void ds_setup(void);
void ds_res_free(void);
void ds_packs_free(void);
int ds_pack_loaded(void *pack, int size);

const unsigned char ds_lang[] = "jegfis";

static int res_cmp(const void *a, const void *b)
{
    return ((DsRes *)a)->id - ((DsRes *)b)->id;
}

static int res_find(unsigned short id, DsRes *t, int n)
{
    int lo = 0, hi = n - 1, mid;

    while (lo <= hi) {
        mid = (lo + hi) / 2;
        if (id == t[mid].id)
            return mid;
        if (t[mid].id < id)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}

DsPack ds_packs[2] = {
    { 1, 0, "deck/all_%c.ehp" },
    { 2, 0, "deck/deckswap.ehp" },
};

DsRes ds_res[25] = {
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

/* the stream animation of the swap screen */
DsRes ds_res_swap[1] = { { 0x1010, 2, 0, "swapstream_%c.ehf" } };

void ds_res_init(void)
{
    memset(&ds_load, 0, sizeof(ds_load));
    ds_load.ready = 1;
    qsort(ds_res, 25, sizeof(DsRes), res_cmp);
    qsort(ds_res_swap, 1, sizeof(DsRes), res_cmp);
}

void *ds_pack_load(unsigned short id);

/* state 0: the two packs; 1 when the screen is set up */
int ds_load_step(void)
{
    if (ds_load.ready) {
        switch (ds_load.step) {
        case 0:
            ds_pack_load(1);
            break;
        case 1:
            ds_pack_load(2);
            break;
        default:
            ds_setup();
            ds_load.step = 0;
            return 1;
        }
    }
    return 0;
}

void ds_res_free(void)
{
    ds_packs_free();
}

void ds_packs_free(void)
{
    unsigned int i;
    DsPack *p;

    for (i = 0, p = ds_packs; i < 2; i++, p++) {
        if (p->data != 0) {
            heap_set_level(ds.heap, p->data);
            p->data = 0;
        }
    }
}

/* the pack once loaded; otherwise starts the read (if none is in flight) and returns 0 */
void *ds_pack_load(unsigned short id)
{
    int found = -1;
    unsigned int i;
    DsPack *p;
    char name[256];
    int size;

    for (i = 0; i < 2; i++) {
        if (id == ds_packs[i].id) {
            found = i;
            break;
        }
    }
    if (found < 0)
        return 0;
    p = &ds_packs[found];
    if (p->data != 0)
        return p->data;
    if (ds_load.ready) {
        sprintf(name, p->name, ds_lang[cfg_get_language()]);
        ds_load.ready = 0;
        size = ehfile_get_size(3, name);
        p->data = heap_alloc(ds.heap, size);
        ehfile_read_async_alias(3, name, p->data, size, ds_pack_loaded, p->data);
    }
    return 0;
}

/* resource by id: 0x8000 = pack 1 resources, 0x1000 = the swap screen's own */
void *res_get(unsigned short id)
{
    DsRes *t;
    int n;
    int i;
    DsRes *r;
    void *pack;
    char name[256];

    if (id & 0x8000) {
        t = ds_res;
        n = 25;
    } else if (id & 0x1000) {
        t = ds_res_swap;
        n = 1;
    }
    i = res_find(id, t, n);
    if (i < 0)
        return 0;
    r = &t[i];
    if (r->data != 0)
        return r->data;
    pack = ds_pack_load(r->pack);
    if (pack == 0)
        return 0;
    if (id - 0x8036 < 2u) {
        char lang[7] = "jagfisb";

        sprintf(name, r->name, lang[region_get()]);
    } else {
        sprintf(name, r->name, ds_lang[cfg_get_language()]);
    }
    r->data = ehp_find(pack, name, 0);
    return r->data;
}

int ds_pack_loaded(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack != 0)
            ehp_mount(0, pack);
        ds_load.step++;
    }
    ds_load.ready = 1;
    return ok;
}

/* the lists, the panes (the deck on the left, the side deck on the right), the card list sprites */
void ds_setup(void)
{
    DsCardListCfg cfg;

    ds_lists_init();
    sysstate_blob_get(ds.words);
    dsv.side = 1;
    dsv.pane[0].list = &ds.list[0];
    dsv.pane[1].list = &ds.list[2];
    ds_set_mode(cfg_get_bits_1_0_2());
    ds_tabs_init();
    memset(&cfg, 0, sizeof(cfg));
    cfg.gim = res_get(0x8007);
    cfg.digits_gim = res_get(0x8009);
    cfg.frame_ehf = res_get(0x8023);
    cfg.attr_gim = res_get(0x8011);
    cfg.kind_gim = res_get(0x8010);
    cfg.cip = ds.cip;
    cardlist_init(&cfg);
    dsv.cursor.scroll.id = -1;
    dsv.cursor.id = 0;
    dsv.cursor.key = 0;
    dsv.cursor.ent = 0;
}
