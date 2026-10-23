// @range 0x08804000-0x08804D0C
// @rodata 0x0881A910-0x0881A93C
// @data 0x0881AA10-0x0881AD70
/* Card picker entry points (init, per-frame update, unload), the EHP packs and the resources looked
   up in them by id (read asynchronously, language letter in the file names), and small id lookups. */
#include "libselect_card.h"

int sceKernelChangeCurrentThreadAttr(unsigned int clear, unsigned int set);
int region_get(void);
int cfg_get_language(void);
void sc_update_input(void);
void sc_update_anim(void);
void sc_unload_hook(void);
void sc_lists_free(void);
void sc_setup(void);
int sc_load_step(void);
int sc_unload(void);
void sc_res_init(void);

const unsigned char sc_lang[] = "jegfis";

int module_entry(void)
{
    return 0;
}

/* export: kind = SC_KIND_*, cfg = heap/cip/packs to share (or 0), *count/ids = the list to edit */
void libselect_card_08804008(unsigned short kind, ScConfig *cfg, int *count, unsigned short *ids)
{
    memset(&sc, 0, sizeof(sc));
    memset(&scv, 0, sizeof(scv));
    if (cfg != 0) {
        sc.heap = cfg->heap;
        sc.cip = cfg->cip;
        sc.pack_a = cfg->pack_a;
        sc.pack_b = cfg->pack_b;
    }
    sc.kind = kind;
    if (sc.heap == 0) {
        sc.fpl = sceKernelCreateFpl("", 2, 0, 0x600000, 1, 0);
        sceKernelAllocateFpl(sc.fpl, &sc.fpl_mem, 0);
        sc.heap = heap_create(sc.fpl_mem, 0x600000);
    }
    sc.out_count = count;
    sc.out_ids = ids;
    if (count != 0)
        sc.n_in = *count;
    if (ids != 0)
        memcpy(sc.ids_in, ids, sc.n_in << 1);
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    sc_res_init();
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
    sc.state = 0;
}

ScPack sc_packs[4] = {
    { 1, 0, "deck/all_%c.ehp" },
    { 2, 0, "labo/rental.ehp" },
    { 3, 0, "labo/conv_machine.ehp" },
    { 4, 0, "trade.ehp" },
};

ScRes sc_res[25] = {
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

/* the window of each kind, one table each */
ScRes sc_res_rental[1] = { { 0x1010, 2, 0, "window_%c.ehf" } };
ScRes sc_res_conv[1] = { { 0x2010, 3, 0, "window_%c.ehf" } };
ScRes sc_res_trade[1] = { { 0x4010, 4, 0, "t_window.ehf" } };

/* export: once per frame; 1 when the screen is done */
int libselect_card_088041C4(void)
{
    switch (sc.state) {
    case 0:
        if (sc_load_step() != 0) {
            gfx_overlay_enable(0);
            fade_start(1, 0xFF000000, 16);
            if (sc.kind == 0) {
                snd_cmd_fefa();
                bgm_play(8);
            }
            sc.state = 1;
        }
        return 0;
    case 9:
        if (fade_is_done())
            sc.state = 10;
        break;
    case 10:
        return sc_unload() != 0;
    }
    sc_update_anim();
    sc_update_input();
    return 0;
}

int sc_unload(void)
{
    switch (sc.unload_step) {
    case 0:
        if (sc.own_cip) {
            cip_close(sc.cip);
            sc.unload_step++;
        } else {
            sc.unload_step = 2;
        }
        break;
    case 1:
        if (cip_is_idle(sc.cip))
            sc.unload_step++;
        break;
    case 2:
        sc_unload_hook();
        sc_lists_free();
        if (sc.fpl > 0) {
            sceKernelFreeFpl(sc.fpl, sc.fpl_mem);
            sceKernelDeleteFpl(sc.fpl);
        }
        return 1;
    }
    return 0;
}

static int res_cmp(const void *a, const void *b)
{
    return ((ScRes *)a)->id - ((ScRes *)b)->id;
}

static int res_find(unsigned short id, ScRes *t, int n)
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

void sc_res_init(void)
{
    memset(&sc_load, 0, sizeof(sc_load));
    sc_load.ready = 1;
    qsort(sc_res, 25, sizeof(ScRes), res_cmp);
    qsort(sc_res_rental, 1, sizeof(ScRes), res_cmp);
    qsort(sc_res_conv, 1, sizeof(ScRes), res_cmp);
    qsort(sc_res_trade, 1, sizeof(ScRes), res_cmp);
}

ScPack *sc_pack_find(unsigned short id);
void *sc_pack_load(unsigned short id);

/* state 0: the two packs (or the caller's), then the card image reader */
int sc_load_step(void)
{
    int id;

    if (sc_load.ready) {
        switch (sc_load.step) {
        case 0:
            if (sc.pack_a != 0) {
                ScPack *p = sc_pack_find(1);
                p->data = sc.pack_a;
            } else if (sc_pack_load(1) == 0)
                break;
            sc_load.step++;
        case 1:
            switch (sc.kind) {
            case 0:
                id = 2;
                break;
            case 1:
                id = 3;
                break;
            case 2:
                id = 4;
                break;
            default:
                return 1;
            }
            if (sc.pack_b != 0) {
                ScPack *p = sc_pack_find(id);
                p->data = sc.pack_b;
            } else if (sc_pack_load(id) == 0)
                break;
            sc_load.step++;
        case 2:
            if (sc.cip == 0) {
                sc_cip_mem = heap_alloc(sc.heap, 0x132000);
                sc.cip = cip_open_h(sc_cip_mem, 0x132000, 0);
                sc.own_cip = 1;
                sc_load.step++;
            } else {
                sc_load.step = 4;
            }
            break;
        case 3:
            if (cip_is_loaded(sc.cip))
                sc_load.step++;
            break;
        default:
            sc_setup();
            sc_load.step = 0;
            return 1;
        }
    }
    return 0;
}

void sc_unload_hook(void)
{
}

/* export: frees what the screen allocated (the packs the caller did not pass) */
void select_card_release(void)
{
    unsigned int i;
    ScPack *p;
    void *ext;

    if (sc_cip_mem != 0) {
        heap_set_level(sc.heap, sc_cip_mem);
        sc_cip_mem = 0;
    }
    for (i = 0, p = sc_packs; i < 4; i++, p++) {
        ext = p->id == 1 ? sc.pack_a : sc.pack_b;
        if (ext == 0 && p->data != 0) {
            heap_set_level(sc.heap, p->data);
            p->data = 0;
        }
    }
}

ScPack *sc_pack_find(unsigned short id)
{
    int found = -1;
    unsigned int i;

    for (i = 0; i < 4; i++) {
        if (id == sc_packs[i].id) {
            found = i;
            break;
        }
    }
    if (found < 0)
        return 0;
    return &sc_packs[found];
}

int sc_pack_loaded(void *pack, int size);

/* the pack once loaded; otherwise starts the read (if none is in flight) and returns 0 */
void *sc_pack_load(unsigned short id)
{
    ScPack *p;
    char name[256];
    int size;

    p = sc_pack_find(id);
    if (p == 0)
        return 0;
    if (p->data != 0)
        return p->data;
    if (sc_load.ready) {
        sprintf(name, p->name, sc_lang[cfg_get_language()]);
        sc_load.ready = 0;
        size = ehfile_get_size(3, name);
        p->data = heap_alloc(sc.heap, size);
        ehfile_read_async_alias(3, name, p->data, size, sc_pack_loaded, p->data);
    }
    return 0;
}

/* resource by id: 0x8000 = pack 1 resources, 0x1000/0x2000/0x4000 = the window of each kind */
void *res_get(unsigned short id)
{
    ScRes *t;
    int n;
    int i;
    ScRes *r;
    void *pack;
    char name[256];

    if (id & 0x8000) {
        t = sc_res;
        n = 25;
    } else if (id & 0x1000) {
        t = sc_res_rental;
        n = 1;
    } else if (id & 0x2000) {
        t = sc_res_conv;
        n = 1;
    } else if (id & 0x4000) {
        t = sc_res_trade;
        n = 1;
    }
    i = res_find(id, t, n);
    if (i < 0)
        return 0;
    r = &t[i];
    if (r->data != 0)
        return r->data;
    pack = sc_pack_load(r->pack);
    if (pack == 0)
        return 0;
    if (id - 0x8036 < 2u) {
        char lang[7] = "jagfisb";

        sprintf(name, r->name, lang[region_get()]);
    } else {
        sprintf(name, r->name, sc_lang[cfg_get_language()]);
    }
    r->data = ehp_find(pack, name, 0);
    return r->data;
}

int sc_pack_loaded(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack != 0)
            ehp_mount(0, pack);
        sc_load.step++;
    }
    sc_load.ready = 1;
    return ok;
}

/* row of id in a sorted id table (count, ids...), 0 when absent */
unsigned short sc_sort_key(unsigned short id, unsigned short *tab)
{
    unsigned short r = 0;
    unsigned short *t;
    int lo, hi, mid;

    if (tab == 0)
        return r;
    t = tab + 1;
    lo = 0;
    hi = tab[0] - 1;
    while (lo <= hi) {
        mid = (lo + hi) / 2;
        if (id == t[mid]) {
            r = mid;
            break;
        }
        if (t[mid] < id)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return r;
}

/* copies of id in a sorted list */
unsigned short sc_id_count(unsigned short id, ScIdList *l)
{
    int n = l->n;
    int c = 0;
    int i;

    for (i = 0; i < n; i++) {
        if (id == (unsigned short)l->e[i])
            c++;
        if (id < (unsigned short)l->e[i])
            break;
    }
    return (unsigned short)c;
}

int sc_id_find(unsigned short id, unsigned short sub, ScIdList *l)
{
    unsigned int key = id | sub << 16;
    int n = l->n;
    int i;

    for (i = 0; i < n; i++) {
        if (key == l->e[i])
            return i;
    }
    return -1;
}

/* icon row of a card kind, -1 for none */
int sc_kind_icon(unsigned short kind)
{
    switch (kind) {
    case 0:
        return 0;
    case 1:
        return 1;
    case 2:
        return 2;
    case 3:
        return 3;
    case 7:
        return 4;
    case 8:
        return 5;
    case 4:
    case 5:
    case 6:
    default:
        return -1;
    }
}
