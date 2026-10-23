// @range 0x08804000-0x08804B50
// @rodata 0x0880C5F0-0x0880C620
// @data 0x0880C684-0x0880CBBC
/* Entry points of the convert machine (init / per-frame update), its release, and the resources: two
   packs (deck/all_<lang>.ehp with the card art, labo/conv_machine.ehp with the machine), textures and
   strings looked up by id, the two machine models, and the loading of libselect_card.prx. */
#include "libconv_machine.h"

typedef struct CmPack {
    unsigned short id;
    void *data;
    const char *name;
} CmPack;

typedef struct CmFile {         /* deck/all pack entries (ids 0x8000+) */
    unsigned short id;
    unsigned short pad;
    void *data;
    const char *name;
} CmFile;

typedef struct CmFile2 {        /* conv_machine pack entries (ids 0x1000+) */
    unsigned short id;
    void *data;
    const char *name;
    const char *name2;
} CmFile2;

typedef struct CmModel {
    unsigned short id;
    Model *model;
    const char *tms;
    const char *tmt;
    const char *tgma;
} CmModel;

int cm_res_cmp12(const void *a, const void *b);
int cm_res_cmp(const void *a, const void *b);
static int cm_bsearch12(unsigned short key, void *base, int n);
int cm_bsearch(unsigned short key, void *base, int n, int size);
int cm_pack_loaded(void *pack, int size);
int cm_module_loaded(void *arg, int modid);

int module_entry(void)
{
    return 0;
}

/* 0x08804008: export */
void conv_init(void)
{
    memset(&cm, 0, 0xCE4);
    cm.heap = heap_create(cm_heap_mem, 0x600000);
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    cm_gfx_init();
    cm_res_init();
    snd_cmd_fefa();
    bgm_fade(336);
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
}

/* 0x088040BC: export; 1 once left and released */
int conv_update(void)
{
    switch (cm.state) {
    case 0:
        if (cm_load_poll()) {
            cm.state = 1;
            fade_start(1, 0xFF000000, 16);
            bgm_play(8);
            gfx_overlay_enable(0);
        }
        break;
    case 1:
        cm_state_intro();
        break;
    case 2:
        cm_state_select();
        break;
    case 3:
        cm_state_insert();
        break;
    case 4:
        cm_state_convert();
        break;
    case 5:
        cm_state_result();
        break;
    case 6:
    case 7:
        cm_state_ask();
        break;
    case 8:
        if (fade_is_done())
            cm.state = 9;
        break;
    case 9:
        if (cm_term())
            return 1;
        break;
    }
    return 0;
}

/* language letters by cfg_get_language() */
static const unsigned char cm_lang[] = "jegfis";

/* waits for the sounds, closes the card reader, unloads libselect_card, frees the heap */
int cm_term(void)
{
    switch (cm.term) {
    case 0:
        if (snd_count_se() == 0) {
            bgm_cmd3(336);
            cm.term++;
        }
        break;
    case 1:
        cardlist_clear();
        if (cm.cip) {
            cip_close(cm.cip);
            cm.term++;
        }
        if (cm.modid > 0) {
            sceKernelStopModule(cm.modid, 0, 0, 0, 0);
            sceKernelUnloadModule(cm.modid);
        }
        heap_free_block(cm.heap);
        break;
    case 2:
        if (cip_is_idle(cm.cip))
            return 1;
        break;
    }
    return 0;
}

int cm_res_cmp12(const void *a, const void *b)
{
    return *(const unsigned short *)a - *(const unsigned short *)b;
}

/* binary search over the 12-byte records of cm_files */
static int cm_bsearch12(unsigned short key, void *base, int n)
{
    int lo = 0;
    int hi = n - 1;
    int mid;
    unsigned short v;

    while (lo <= hi) {
        mid = (lo + hi) / 2;
        v = ((CmFile *)base)[mid].id;
        if (key == v)
            return mid;
        if (v < key)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}

CmPack cm_packs[2] = {
    { 1, 0, "deck/all_%c.ehp" },
    { 2, 0, "labo/conv_machine.ehp" },
};

CmFile cm_files[25] = {
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

CmFile2 cm_files2[12] = {
    { 0x1001, 0, "bg.gim", 0 },
    { 0x1002, 0, "cover.gim", 0 },
    { 0x1010, 0, "insert.ehf", "insert.tmf" },
    { 0x1011, 0, "out1.ehf", "out1.tmf" },
    { 0x1012, 0, "out2.ehf", 0 },
    { 0x1013, 0, "nixi.ehf", "nixi.tmf" },
    { 0x1014, 0, "efx_glass.ehf", "efx_glass.tmf" },
    { 0x1015, 0, "glass_light.ehf", "glass_light.tmf" },
    { 0x1016, 0, "efx_bombs.ehf", "efx_bombs.tmf" },
    { 0x1017, 0, "efx_bombl.ehf", "efx_bombl.tmf" },
    { 0x1018, 0, "efx_exit.ehf", "efx_exit.tmf" },
    { 0x1030, 0, "strTbl_%c.bin", 0 },
};

CmModel cm_models[2] = {
    { 0x1020, 0, "cc_card_in.tms", "cc_card_in.tmt", 0 },
    { 0x1021, 0, "cc_card_out.tms", "cc_card_out.tmt", 0 },
};

void cm_res_init(void)
{
    memset(&cm_res, 0, 1);
    cm_res.ready = 1;
    qsort(cm_files, 25, sizeof(CmFile), cm_res_cmp12);
    qsort(cm_files2, 12, sizeof(CmFile2), cm_res_cmp);
    qsort(cm_models, 2, sizeof(CmModel), cm_res_cmp);
}

/* card reader, machine pack, card pack, libselect_card; 1 when everything is in */
int cm_load_poll(void)
{
    if (cm_res.ready) {
        switch (cm_res.step) {
        case 0:
            cm.cip = cip_open_h(cm_cip_mem, 0x6A000, 0);
            cm_pack_get(2);
            break;
        case 1:
            cm_pack_get(1);
            break;
        case 2:
            if (cip_is_loaded(cm.cip))
                cm_res.step++;
            break;
        case 3:
            ehfile_request_module(2, "libselect_card.prx", cm_module_loaded, 0);
            cm_res.step++;
            break;
        default:
            cm_lid_close();
            cm_res.step = 0;
            return 1;
        }
    }
    return 0;
}

/* pack `id`; starts reading it the first time, 0 until it is in */
void *cm_pack_get(unsigned short id)
{
    int k = -1;
    unsigned int i;
    CmPack *p;
    char name[256];
    int size;

    for (i = 0; i < 2; i++) {
        if (id == cm_packs[i].id) {
            k = i;
            break;
        }
    }
    if (k < 0)
        return 0;
    p = &cm_packs[k];
    if (p->data)
        return p->data;
    if (cm_res.ready) {
        sprintf(name, p->name, cm_lang[cfg_get_language()]);
        cm_res.ready = 0;
        size = ehfile_get_size(3, name);
        p->data = heap_alloc(cm.heap, size);
        ehfile_read_async_alias(3, name, p->data, size, cm_pack_loaded, p->data);
    }
    return 0;
}

/* resource `id`: 0x8000+ from the card pack (pack lists by region), 0x1000+ from the machine pack */
void *cm_res_get(unsigned short id)
{
    int i;
    CmFile *f;
    CmFile2 *f2;
    void *pack;

    if (id & 0x8000) {
        i = cm_bsearch12(id, cm_files, 25);
        if (i < 0)
            return 0;
        f = &cm_files[i];
        if (f->data)
            return f->data;
        pack = cm_pack_get(1);
        if (pack == 0)
            return 0;
        {
            char name[256];
            if (id == 0x8036 || id == 0x8037) {
                char reg[7] = "jagfisb";
                sprintf(name, f->name, reg[region_get()]);
            } else {
                sprintf(name, f->name, cm_lang[cfg_get_language()]);
            }
            return f->data = ehp_find(pack, name, 0);
        }
    }
    if (id & 0x1000) {
        i = cm_bsearch(id, cm_files2, 12, sizeof(CmFile2));
        if (i < 0)
            return 0;
        f2 = &cm_files2[i];
        if (f2->data)
            return f2->data;
        pack = cm_pack_get(2);
        if (pack == 0)
            return 0;
        {
            char name[256];
            sprintf(name, f2->name, cm_lang[cfg_get_language()]);
            return f2->data = ehp_find(pack, name, 0);
        }
    }
    return 0;
}

Model *cm_model_get(unsigned short id)
{
    CmModel *m;
    void *pack;
    void *tms;
    void *tmt;
    void *tgma;
    int i = cm_bsearch(id, cm_models, 2, sizeof(CmModel));

    if (i < 0)
        return 0;
    m = &cm_models[i];
    if (m->model)
        return m->model;
    if (id & 0x1000) {
        pack = cm_pack_get(2);
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
        } else if (*(int *)tmt != 0x544D4754) {
            m->model = model_load(tms, tmt);
        } else {
            m->model = model_load(tms, 0);
            model_anim_set(m->model, tmt);
        }
        if (tgma)
            model_set_tgma(m->model, tgma);
        return m->model;
    }
    return 0;
}

int cm_pack_loaded(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack)
            ehp_mount(0, pack);
        cm_res.step++;
    }
    cm_res.ready = 1;
    return ok;
}

int cm_res_cmp(const void *a, const void *b)
{
    return *(const unsigned short *)a - *(const unsigned short *)b;
}

/* binary search over records of `size` bytes keyed by their first u16 */
int cm_bsearch(unsigned short key, void *base, int n, int size)
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

/* libselect_card.prx loaded: start it */
int cm_module_loaded(void *arg, int modid)
{
    int ok = modid > 0;

    if (ok) {
        int one = 1;
        sceKernelStartModule(modid, 4, &one, 0, 0);
        cm.modid = modid;
    }
    return ok;
}
