// @range 0x08804408-0x08804D08
// @data 0x0880D6B0-0x0880DA40
/* Resources of the screen: database/cardalbum.ehp is read asynchronously into the heap and mounted;
   textures, strings and the binder models (cover, pages, cursor) are looked up by id on demand. The two
   card picture readers (small and large CIP) are opened here, and a queue of card ids is prefetched. */
#include "libcardalbum.h"

typedef struct CaFile {
    unsigned short id;
    void *data;                /* found entry, cached */
    const char *name;          /* "%c" = language (or region) letter */
    const char *name2;
} CaFile;

typedef struct CaModel {
    unsigned short id;
    Model *model;              /* loaded model, cached */
    const char *tms;
    const char *tmt;           /* animation (or texture animation when tagged TGMT) */
    const char *tgma;
} CaModel;

typedef struct CardListCfg {   /* BOOT cardlist_init configuration, 0x24 */
    void *gim;
    void *digits_gim;
    void *frame_ehf;
    void *attr_gim;
    void *kind_gim;
    void *cil_gim;
    void *icon_gim;
    void *cip;
    int frame_view;
} CardListCfg;

int ca_res_cmp(const void *a, const void *b);
int ca_pack_loaded(void *pack, int size);
int ca_bsearch(unsigned short key, void *base, int n, int size);

CaFile ca_files[18] = {
    { 0x1001, 0, "bg.gim", 0 },
    { 0x1002, 0, "misc.gim", 0 },
    { 0x1003, 0, "hand.gim", 0 },
    { 0x1004, 0, "icon01.gim", 0 },
    { 0x1005, 0, "icon03.gim", 0 },
    { 0x1006, 0, "icon06.gim", 0 },
    { 0x1007, 0, "frame.gim", 0 },
    { 0x1008, 0, "spelltrap_%c.gim", 0 },
    { 0x1009, 0, "detail_bg.gim", 0 },
    { 0x100A, 0, "ocglist.gim", 0 },
    { 0x100B, 0, "dmy_card01.gim", 0 },
    { 0x100C, 0, "dmy_card02.gim", 0 },
    { 0x100D, 0, "dmy_card03.gim", 0 },
    { 0x1010, 0, "skipmenu.ehf", "skipmenu.tmf" },
    { 0x1011, 0, "score-atkdef10.ehf", 0 },
    { 0x1030, 0, "strTbl_%c.bin", 0 },
    { 0x1031, 0, "ocg_packname_%c.bin", 0 },
    { 0x1032, 0, "ocg_packlist_%c.bin", 0 },
};

CaModel ca_models[5] = {
    { 0x1020, 0, "album_cover00.tms", "album_cover00.tmt", 0 },
    { 0x1021, 0, "album_page00.tms", "album_page00.tmt", 0 },
    { 0x1022, 0, "album_page01.tms", "album_page01.tmt", 0 },
    { 0x1023, 0, "album_page02.tms", "album_page02.tmt", 0 },
    { 0x1024, 0, "album_cursor01.tms", "album_cursor01.tmt", 0 },
};

void ca_res_init(void)
{
    memset(&ca_res, 0, 0x26);
    ca_res.ready = 1;
    qsort(ca_files, 18, sizeof(CaFile), ca_res_cmp);
    qsort(ca_models, 5, sizeof(CaModel), ca_res_cmp);
}

/* 1 once the readers are open, the pack is mounted and the card list is set up */
int ca_load_poll(void)
{
    if (ca_res.ready) {
        switch (ca_res.step) {
        case 0:
            ca_cip_h_mem = heap_alloc(ca.heap, 0x6A000);
            ca.cip_h = cip_open_h(ca_cip_h_mem, 0x6A000, 0);
            ca_cip_m_mem = heap_alloc(ca.heap, 0x5E2000);
            ca.cip_m = cip_open_m(ca_cip_m_mem, 0x5E2000, 1);
            ca_pack_load();
            break;
        case 1:
            if (cip_is_loaded(ca.cip_h) && cip_is_loaded(ca.cip_m))
                ca_res.step++;
            break;
        case 2: {
            CardListCfg cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.gim = ca_res_get(0x1004);
            cfg.digits_gim = ca_res_get(0x1005);
            cfg.frame_ehf = ca_res_get(0x1011);
            cfg.attr_gim = ca_res_get(0x1007);
            cfg.kind_gim = ca_res_get(0x1008);
            cfg.cip = ca.cip_h;
            cardlist_init(&cfg);
            ca.panel.id = -1;
            ca.panel.scroll = 0;
            ca_res.step++;
            break;
        }
        default:
            ca_res.step = 0;
            return 1;
        }
    }
    return 0;
}

void ca_res_free(void)
{
    ca_res_free_all();
}

void ca_res_free_all(void)
{
    unsigned int i;
    CaModel *m;

    if (ca_cip_h_mem) {
        heap_set_level(ca.heap, ca_cip_h_mem);
        ca_cip_h_mem = 0;
    }
    if (ca_cip_m_mem) {
        heap_set_level(ca.heap, ca_cip_m_mem);
        ca_cip_m_mem = 0;
    }
    for (i = 0, m = ca_models; i < 5; i++, m++) {
        if (m->model) {
            model_free(m->model);
            m->model = 0;
        }
    }
    if (ca_pack) {
        heap_set_level(ca.heap, ca_pack);
        ca_pack = 0;
    }
}

/* starts the read the first time; the pack once it is in memory */
void *ca_pack_load(void)
{
    int size;

    if (ca_pack)
        return ca_pack;
    if (ca_res.ready) {
        ca_res.ready = 0;
        size = ehfile_get_size(3, "database/cardalbum.ehp");
        ca_pack = heap_alloc(ca.heap, size);
        ehfile_read_async_alias(3, "database/cardalbum.ehp", ca_pack, size, ca_pack_loaded, ca_pack);
    }
    return 0;
}

void *ca_res_get(unsigned short id)
{
    CaFile *f;
    void *pack;
    int i = ca_bsearch(id, ca_files, 18, sizeof(CaFile));

    if (i < 0)
        return 0;
    f = &ca_files[i];
    if (f->data)
        return f->data;
    pack = ca_pack_load();
    {
        char name[256];
        char lang[6] = "jegfis";   /* by cfg_get_language() */
        char reg[7] = "jagfisb";   /* by region_get(): the OCG pack lists */
        if (pack == 0)
            return 0;
        if (id == 0x1031 || id == 0x1032)
            sprintf(name, f->name, reg[region_get()]);
        else
            sprintf(name, f->name, lang[cfg_get_language()]);
        return f->data = ehp_find(pack, name, 0);
    }
}

Model *ca_model_get(unsigned short id)
{
    CaModel *m;
    void *pack;
    void *tms;
    void *tmt;
    void *tgma;
    int i = ca_bsearch(id, ca_models, 5, sizeof(CaModel));

    if (i < 0)
        return 0;
    m = &ca_models[i];
    if (m->model)
        return m->model;
    pack = ca_pack_load();
    tmt = 0;
    tgma = 0;
    if (pack == 0)
        return 0;
    tms = ehp_find(pack, m->tms, 0);
    if (m->tmt)
        tmt = ehp_find(pack, m->tmt, 0);
    if (m->tgma)
        tgma = ehp_find(pack, m->tgma, 0);
    if (tmt == 0) {
        m->model = model_load(tms, 0);
    } else if (*(int *)tmt != 0x544D4754) {    /* not "TGMT": skeletal animations */
        m->model = model_load(tms, tmt);
    } else {
        m->model = model_load(tms, 0);
        model_anim_set(m->model, tmt);
    }
    if (tgma)
        model_set_tgma(m->model, tgma);
    return m->model;
}

int ca_pack_loaded(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack)
            ehp_mount(0, pack);
        ca_res.step++;
    }
    ca_res.ready = 1;
    return ok;
}

int ca_res_cmp(const void *a, const void *b)
{
    return *(const unsigned short *)a - *(const unsigned short *)b;
}

/* binary search over records of `size` bytes keyed by their first u16 */
int ca_bsearch(unsigned short key, void *base, int n, int size)
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

void ca_queue_clear(void)
{
    memset(ca_res.queue, 0, sizeof(ca_res.queue));
}

/* first free slot of the prefetch queue */
void ca_queue_add(unsigned short id)
{
    int i = 0;
    unsigned short *p = (unsigned short *)&ca_res;
    unsigned short *q;

    do {
        q = p + 1;
        if (*q == 0) {
            *q = id;
            return;
        }
        i++;
        p = q;
    } while (i < 18);
}

/* starts loading the first queued picture that is not in memory yet, drops the ones before it */
void ca_queue_prefetch(void)
{
    int i = 0;
    unsigned short *p = (unsigned short *)&ca_res;
    unsigned short id;
    unsigned int *list;
    int loaded;
    int j;

    do {
        id = p[1];
        if (id == 0)
            break;
        if (cip_query(ca.cip_m, id, inv_get_hi(id)) == 1) {
            list = dl_select_layer(0, 0);
            cip_get_image(list, ca.cip_m, id, inv_get_hi(id), &loaded);
            dl_set_list(list);
            break;
        }
        i++;
        p++;
    } while (i < 18);
    if (i > 0) {
        for (j = 0; j < 18; j++) {
            if (i < 18)
                ca_res.queue[j] = ca_res.queue[i++];
            else
                ca_res.queue[j] = 0;
        }
    }
}
