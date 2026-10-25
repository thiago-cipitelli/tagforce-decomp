// @range 0x08804000-0x088052F8
// @asm 0x08804C50-0x08804DC4
// @rodata 0x0881C0E0-0x0881C0E8
// @data 0x0881C2B8-0x0881C890
/* Entry points of the shop (scene 7 and the exports used by libstory), its release, the resources
   (two packs, shop/all_<lang>.ehp and deck/all_<lang>.ehp, with textures, EHF effects, string tables
   and the pack/box models, all looked up by id with a binary search), the pack textures picked by the
   pack number, a number printer, the 2D state and the shelf camera. */
#include "libshop.h"

typedef struct ShopRes {        /* resource looked up by id: the pack it lives in and its file name */
    unsigned short id;
    unsigned short pack;
    void *data;
    const char *name;
} ShopRes;

typedef struct ShopModelRes {
    unsigned short id;
    unsigned short pack;
    Model *model;
    const char *tms;
    const char *tmt;
    const char *tma;
} ShopModelRes;

extern unsigned char shop_scene_data[16];
extern Vec4 shop_cam1_pos, shop_cam1_target, shop_cam1_up;

int shop_pack_loaded(void *pack, int size);
void *shop_box_tex_get(const char *name, int pack);
void shop_scene_main(void);
void shop_scene_exit(void);
void shop_cb_draw(void);
void shop_cb_exit(void);



static const unsigned char shop_lang[] = "jegfis";

ShopRes shop_packs[2] = {
    { 0x1001, 0x1001, 0, "shop/all_%c.ehp" },
    { 0x1002, 0x1002, 0, "deck/all_%c.ehp" },
};

ShopRes shop_files[25] = {
    { 0x2001, 0x1001, 0, "bg1.gim" },
    { 0x2002, 0x1001, 0, "panel.gim" },
    { 0x2003, 0x1001, 0, "tab_base.gim" },
    { 0x2004, 0x1001, 0, "tab_title.gim" },
    { 0x2005, 0x1001, 0, "now_print.gim" },
    { 0x2006, 0x1001, 0, "s_pack.gim" },
    { 0x2007, 0x1001, 0, "system.gim" },
    { 0x2020, 0x1002, 0, "cis_all.gim" },
    { 0x2021, 0x1002, 0, "ciss.gim" },
    { 0x2022, 0x1002, 0, "panel.gim" },
    { 0x2023, 0x1002, 0, "icon01.gim" },
    { 0x2024, 0x1002, 0, "icon02.gim" },
    { 0x2025, 0x1002, 0, "icon03.gim" },
    { 0x2026, 0x1002, 0, "icon04.gim" },
    { 0x2027, 0x1002, 0, "icon05.gim" },
    { 0x2028, 0x1002, 0, "icon06.gim" },
    { 0x2029, 0x1002, 0, "detail_bg.gim" },
    { 0x202A, 0x1002, 0, "s_cut.gim" },
    { 0x2030, 0x1002, 0, "spelltrap.gim" },
    { 0x2031, 0x1002, 0, "frame.gim" },
    { 0x2032, 0x1002, 0, "ocglist.gim" },
    { 0x2040, 0x1001, 0, "eff_blu_light_g.gim" },
    { 0x2041, 0x1001, 0, "eff_lens_aP8A.gim" },
    { 0x2042, 0x1001, 0, "eff_m530snd.gim" },
    { 0x2043, 0x1001, 0, "eff_sen_07.gim" },
};

ShopRes shop_ehfs[6] = {
    { 0x4001, 0x1001, 0, "pc_tome_e.ehf" },
    { 0x4002, 0x1001, 0, "pc_tome_m.ehf" },
    { 0x4003, 0x1001, 0, "pc_sei_e.ehf" },
    { 0x4004, 0x1001, 0, "pc_sei_m.ehf" },
    { 0x4005, 0x1002, 0, "bg.ehf" },
    { 0x4010, 0x1002, 0, "score-atkdef10.ehf" },
};

ShopModelRes shop_models[5] = {
    { 0x8001, 0x1001, 0, "pack11.tms", "pack11.tmt", 0 },
    { 0x8002, 0x1001, 0, "box11.tms", "box11.tmt", 0 },
    { 0x8003, 0x1001, 0, "pack_in_card00.tms", "pack_in_card00.tmt", 0 },
    { 0x8004, 0x1001, 0, "pack_open_mov01.tms", "pack_open_mov01.tmt", 0 },
    { 0x8005, 0x1001, 0, "duel-bg-fmagic0000-base.tms", "duel-bg-fmagic0000-base.tmt",
      "duel-bg-fmagic0000-base.tma" },
};

ShopRes shop_strs[7] = {
    { 0xC001, 0x1001, 0, "strTbl_%c.bin" },
    { 0xC002, 0x1001, 0, "pack_str_%c.bin" },
    { 0xC003, 0x1002, 0, "fileTbl.dat" },
    { 0xC004, 0x1002, 0, "sameCardTbl.dat" },
    { 0xC005, 0x1002, 0, "strTbl.bin" },
    { 0xC006, 0x1002, 0, "ocg_packname_%c.bin" },
    { 0xC007, 0x1002, 0, "ocg_packlist_%c.bin" },
};

/* textures of a pack model, numbered by the pack */
const char *shop_box_names[3] = { "box_tex%02d.iff", "box_side%02d.iff", "pack_tex%02d.iff" };

/* export: starts the shop as a sub-screen of libstory */
void shop_init(int mode, int arg)
{
    memset(&shop, 0, 0x450);
    shop.heap = heap_create(shop_heap_mem, 0x600000);
    shop.arg = arg;
    shop_setup(mode);
    shop_res_init();
}

/* export: one frame; 1 once the shop is done */
int shop_update(void)
{
    shop_main_update();
    return shop_main_input();
}

/* export */
void shop_draw(void)
{
    shop_draw_all();
}

/* export: waits for the sounds, frees everything; 1 once released */
int shop_term(void)
{
    switch (shop.term) {
    case 0:
        if (snd_count_se() == 0) {
            if (shop_clerk.alt == 0)
                bgm_cmd3(0x149);
            else
                bgm_cmd3(0x148);
            shop.term++;
        }
        break;
    case 1:
        menu_reset(&shop.menu);
        shop_msgwin_free();
        shop_menu_free();
        cardlist_clear();
        shop_res_free();
        view_term();
        model_set_allocator(0, 0);
        if (shop.cip)
            cip_close(shop.cip);
        shop.term++;
        break;
    case 2:
        if (cip_is_idle(shop.cip)) {
            heap_free_block(shop.heap);
            shop.term = 0;
            return 1;
        }
        break;
    }
    return 0;
}

int module_start(int args)
{
    if (args == 0)
        ehscript_register((int (*)(void))shop_scene_main, (int (*)(void))shop_scene_exit, shop_scene_data);
    rand_seed_from_rtc();
    return 0;
}

void shop_scene_init(void)
{
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    memset(&shop, 0, 0x450);
    shop.heap = heap_create(shop_heap_mem, 0x600000);
    shop_setup(1);
    shop_res_init();
    while (!shop_load_poll())
        sys_frame_wait();
}

/* scene main: runs the shop until it is done, then returns to the field */
void shop_scene_main(void)
{
    shop_scene_init();
    gfx_overlay_enable(0);
    sys_state_set_callbacks(7, (int (*)(void))shop_cb_draw, (int (*)(void))shop_cb_exit);
    for (;;) {
        if (shop_update())
            break;
        sys_frame_wait();
    }
    sys_frame_wait();
    sys_state_set_callbacks(0, 0, 0);
    gfx_overlay_enable(1);
    sys_frame_wait();
    scene_pop_to(7, 0);
}

void shop_scene_exit(void)
{
    menu_reset(&shop.menu);
    shop_msgwin_free();
    shop_menu_free();
    cardlist_clear();
    if (shop_clerk.alt == 0)
        bgm_cmd3(0x149);
    else
        bgm_cmd3(0x148);
    bgm_cmd3(0x14F);
    if (shop.cip) {
        cip_close(shop.cip);
        while (!cip_is_idle(shop.cip))
            sys_frame_wait();
    }
    heap_free_block(shop.heap);
}

void shop_cb_draw(void)
{
    shop_draw_all();
}

void shop_cb_exit(void)
{
    shop_res_free();
    view_term();
    model_set_allocator(0, 0);
}

void shop_res_init(void)
{
    memset(shop_box_tex, 0, sizeof(shop_box_tex));
    memset(&shop_res, 0, 4);
    shop_res.ready = 1;
    memset(shop_cip_mem, 0, 0x16E000);
}

/* card reader, shop pack, then the textures of the first screen; 1 when everything is in */
int shop_load_poll(void)
{
    if (shop_res.ready) {
        switch (shop_res.step) {
        case 0:
            shop.cip = cip_open_h(shop_cip_mem, 0x16E000, 0);
            shop_pack_get(0x1001);
            break;
        default:
            shop_clerk_load();
            shop_menu_init(shop_res_get(0x2007));
            shop_tabs_init();
            shop_model_set_box(0x8002, shop.pack);
            shop_model_set_box(0x8001, shop.pack);
            fade_start(1, 0xFF000000, 16);
            bgm_play(7);
            shop_res.step = 0;
            return 1;
        }
    }
    return 0;
}

void shop_res_free(void)
{
    shop_packs_free();
}

void shop_models_free(void)
{
    unsigned int i;
    ShopModelRes *m;

    for (i = 0, m = shop_models; i < 5; i++, m++) {
        if (m->model) {
            model_free(m->model);
            m->model = 0;
        }
    }
}

void shop_packs_free(void)
{
    unsigned int i;
    ShopRes *p;

    shop_models_free();
    for (i = 0, p = shop_packs; i < 2; i++, p++) {
        if (p->data) {
            heap_set_level(shop.heap, p->data);
            p->data = 0;
        }
    }
}

/* the deck pack (card pictures); 1 once it is in */
int shop_load_deck_pack(void)
{
    if (shop_res.ready) {
        switch (shop_res.step) {
        case 0:
            shop_pack_get(0x1002);
            break;
        default:
            shop_res.step = 0;
            return 1;
        }
    }
    return 0;
}

/* pack `id`; starts reading it the first time, 0 until it is in */
void *shop_pack_get(unsigned short id)
{
    int i = shop_bsearch(id, shop_packs, 2, sizeof(ShopRes));
    ShopRes *p;
    char name[256];
    int size;

    if (i < 0)
        return 0;
    p = &shop_packs[i];
    if (p->data)
        return p->data;
    if (shop_res.ready) {
        sprintf(name, p->name, shop_lang[cfg_get_language()]);
        shop_res.ready = 0;
        size = ehfile_get_size(3, name);
        p->data = heap_alloc(shop.heap, size);
        ehfile_read_async_alias(3, name, p->data, size, shop_pack_loaded, p->data);
    }
    return 0;
}

void *shop_res_get(unsigned short id)
{
    int i = shop_bsearch(id, shop_files, 25, sizeof(ShopRes));
    ShopRes *f;
    void *pack;
    char name[256];

    if (i < 0)
        return 0;
    f = &shop_files[i];
    if (f->data)
        return f->data;
    pack = shop_pack_get(f->pack);
    sprintf(name, f->name, shop_lang[cfg_get_language()]);
    if (pack == 0)
        return 0;
    return f->data = ehp_find(pack, name, 0);
}

void *shop_ehf_get(unsigned short id)
{
    int i = shop_bsearch(id, shop_ehfs, 6, sizeof(ShopRes));
    ShopRes *f;
    void *pack;

    if (i < 0)
        return 0;
    f = &shop_ehfs[i];
    if (f->data)
        return f->data;
    pack = shop_pack_get(f->pack);
    if (pack == 0)
        return 0;
    return f->data = ehp_find(pack, f->name, 0);
}

/* string tables; the OCG pack names and lists are picked by region */
void *shop_str_get(unsigned short id)
{
    int i = shop_bsearch(id, shop_strs, 7, sizeof(ShopRes));
    ShopRes *f;
    void *pack;
    char name[256];

    if (i < 0)
        return 0;
    f = &shop_strs[i];
    if (f->data)
        return f->data;
    pack = shop_pack_get(f->pack);
    if (pack == 0)
        return 0;
    if (id == 0xC006 || id == 0xC007) {
        char reg[7] = "jagfisb";
        sprintf(name, f->name, reg[region_get()]);
    } else {
        sprintf(name, f->name, shop_lang[cfg_get_language()]);
    }
    return f->data = ehp_find(pack, name, 0);
}

Model *shop_model_get(unsigned short id)
{
    int i = shop_bsearch(id, shop_models, 5, sizeof(ShopModelRes));
    ShopModelRes *m;
    void *pack;
    void *tms;
    void *tmt;
    void *tma;

    if (i < 0)
        return 0;
    m = &shop_models[i];
    if (m->model)
        return m->model;
    pack = shop_pack_get(m->pack);
    tmt = 0;
    tma = 0;
    if (pack == 0)
        return 0;
    tms = ehp_find(pack, m->tms, 0);
    if (m->tmt)
        tmt = ehp_find(pack, m->tmt, 0);
    if (m->tma)
        tma = ehp_find(pack, m->tma, 0);
    if (tmt == 0) {
        m->model = model_load(tms, 0);
    } else if (*(int *)tmt != 0x544D4754) {
        m->model = model_load(tms, tmt);
    } else {
        m->model = model_load(tms, 0);
        model_anim_set(m->model, tmt);
    }
    if (tma)
        model_set_tgma(m->model, tma);
    if (id == 0x8005)
        model_anim_play(m->model, 0, 2);
    return m->model;
}

/* ".gim": file extension of the pack pictures (the .iff names of the TMS are mapped to it) */
char shop_gim_ext[] = ".gim";

#ifdef NON_MATCHING /* o original mantém a linha da tabela em s0 e o índice já multiplicado por 4 em s1 até depois do ehp_find; o mwcc troca os dois registradores */
/* texture for the texture slot `name` of a pack model: the box/pack pictures are numbered by pack */
void *shop_box_tex_get(const char *name, int pack)
{
    void *data = shop_pack_get(0x1001);
    int i;
    char buf[128];
    char *ext;

    for (i = 0; i < 3; i++) {
        sprintf(buf, shop_box_names[i], 1);
        if (strcmp(buf, name) == 0) {
            if (shop_box_tex[pack][i])
                return shop_box_tex[pack][i];
            if (data == 0)
                return 0;
            sprintf(buf, shop_box_names[i], pack);
            ext = strrchr_str(buf, '.');
            if (ext)
                strcpy(ext, shop_gim_ext);
            return shop_box_tex[pack][i] = ehp_find(data, buf, 0);
        }
    }
    if (data == 0)
        return 0;
    strcpy(buf, name);
    ext = strrchr_str(buf, '.');
    if (ext)
        strcpy(ext, shop_gim_ext);
    return ehp_find(data, buf, 0);
}
#endif

/* binds the textures of model `id` to the pictures of pack `pack` */
void shop_model_set_box(unsigned short id, int pack)
{
    Model *m = shop_model_get(id);
    Tms *t;
    int i;

    if (m && (t = m->tms) != 0) {
        for (i = 0; i < t->ntexs; i++) {
            ModelTex *x = &t->texs[i];
            x->tex = shop_box_tex_get(x->name, pack);
        }
    }
}

/* binary search over records of `size` bytes keyed by their first u16 */
int shop_bsearch(unsigned short key, void *base, int n, int size)
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

int shop_pack_loaded(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack)
            ehp_mount(0, pack);
        shop_res.step++;
    }
    shop_res.ready = 1;
    return ok;
}

/* right-aligned number ending at x, `digits` places, with thousands commas if `commas` */
unsigned int *shop_draw_number(void *list, int x, unsigned short y, int value, unsigned short step,
                               unsigned short digits, int commas)
{
    unsigned short ch[2] = { '0', 0 };
    unsigned short cx = x;
    int last;
    int i;
    int d;

    font_text_set(list);
    last = digits - 1;
    for (i = last; i >= 0; i--) {
        d = value % 10;
        cx -= step;
        if (i == last || value != 0) {
            ch[0] = d + '0';
            font_draw_text(cx << 6, y << 6, (480 - cx) << 6, ch);
        }
        value /= 10;
        if (value == 0)
            break;
        if (commas && i > 0 && (last - i) % 3 == 2) {
            cx -= step - 3;
            ch[0] = ',';
            font_draw_text(cx << 6, y << 6, (480 - cx) << 6, ch);
        }
    }
    return font_text_clear();
}

/* 2D sprites: textured, blended, no depth test */
unsigned int *shop_dl_2d(unsigned int *list)
{
    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    return dl_disable(list, 1);
}

/* unlit 3D layer: shelf camera */
void shop_gfx_shelf(void)
{
    unsigned int *list;

    list = dl_select_layer(2, 0);
    list = lighting_disable(list);
    list = shop_ge_state(list);
    list = dl_fog(50.0f, 100.0f, list, 0x404040);
    list = dl_disable(list, 7);
    list = dl_set_tex_filter(list, 1, 1);
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(0.69813174f, 1.7647059f, 5.0f, 300.0f);
    mtx_set_mode(1);
    mtx_identity();
    mtx_set_mode(2);
    mtx_identity();
    mtx_set_mode(1);
    {
        Vec4 pos, target, up;
        up = shop_cam1_up;
        target = shop_cam1_target;
        pos = shop_cam1_pos;
        camera_set(&pos, &target, &up);
    }
    camera_load_view();
    list = dl_set_tex_filter(list, 1, 1);
    dl_set_list(list);
}

Vec4 shop_cam1_pos = { 0.0f, 0.0f, 30.0f, 1.0f };
Vec4 shop_cam1_target = { 0.0f, 0.0f, 0.0f, 1.0f };
Vec4 shop_cam1_up = { 0.0f, 1.0f, 0.0f, 0.0f };

