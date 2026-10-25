// @range 0x08807CAC-0x08808B44
// @rodata 0x0881C140-0x0881C15C
/* Shop setup (GE state, fonts, music, the message window, the menu), the reset after a pack was
   opened, the model allocator, the frame draw of every layer, and the clerk over the counter: her two
   EHF (eyes and mouth) animated by a small state machine, and her voice lines. */
#include "libshop.h"

void font_set_flag1(int on);
void font_set_flag2(int on);
void font_set_flag4(int on);
void font_set_flag8(int on);
void font_set_line_extra(int v);
void snd_cmd_fefa(void);
void bgm_fade(int x);
unsigned int bgm_get_status(int id);
void se_play(int id);
int rand_next_alias(void);
int fade_apply(void **list);
unsigned int *view_init(unsigned int *list);
unsigned int *dl_material_colors(unsigned int *list, unsigned int emissive, unsigned int ambient, unsigned int diffuse, unsigned int specular);
unsigned int *dl_color_material(unsigned int *list, int comps);
unsigned int *dl_specular_power(float p, unsigned int *list);
unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_tex_offset(unsigned int *list, float u, float v);
unsigned int *dl_tex_scale(float u, float v, unsigned int *list);
unsigned int *dl_bone_matrix(unsigned int *list, int index, float *m);
Mat4 *mat4_identity(Mat4 *out);
GfxState *gfx_get_state(void);
unsigned int *gfx_begin_2d(unsigned int *list);
unsigned int *gfx_end_2d(unsigned int *list);
unsigned int *ehf_draw(unsigned int *list, void *e, EhfOpts *o);
int ehf_get_frame_count(void *e);
int ehf_set_anim(void *e, int target, int limit);
void menu_open(MenuWidget *w);

void shop_gfx_init(void);
void *shop_model_alloc(unsigned int size);
void shop_model_free(void *p);
void shop_clerk_init(int alt);
void shop_clerk_set(int state);
int shop_clerk_voice_busy(void);
void shop_code_entered(void);
void shop_msgwin_init(void *heap, int a1, int a2, int a3, int a4, int a5);
void shop_tabs_setup(int x, int y, int w, int h);
void *shop_menu_draw(void *dl, int x, int y, int w, int h);
int shop_pack_can_buy(int pack);
unsigned short shop_pack_rank(unsigned short pack);
int shop_points_left(void);
void shop_draw_shelf(void);
void shop_draw_shelf_ui(void);
void shop_draw_counter(void);
void shop_draw_open(void);
void shop_draw_list(void);
unsigned int *shop_dialog_draw(unsigned int *list);

void shop_setup(int alt)
{
    MenuWidget *w;
    MenuItemDef *d;

    shop_gfx_init();
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
    font_set_line_extra(0);
    snd_cmd_fefa();
    bgm_fade(0x14F);
    shop_clerk_init(alt);
    shop_code_entered();
    shop_msgwin_init(shop.heap, 2, 0x1DC, 14, 0, 0);
    shop_msgwin_open();
    shop_tabs_setup(480, 2, 0x16A, 2);
    w = &shop.menu;
    d = &w->def;
    memset(w, 0, sizeof(MenuWidget));
    d->visible = 1;
    d->font_flag4 = 1;
    d->style = 2;
    d->x = 2;
    d->pad_l = 2;
    d->open_anim = 2;
    d->close_anim = 2;
    d->y = 0xBE;
    d->w = 0x1DC;
    d->h = 0x18;
    d->color = -1;
    w->heap = shop.heap;
    w->no_input = 1;
    w->style = 2;
    w->draw_all = (MenuDrawRectFn)shop_menu_draw;
    d->pad_t = 0;
    menu_open(w);
}

/* back from the pack opening: frees the card list, back to the shelf */
void shop_after_open(void)
{
    if (shop.items) {
        heap_set_level(shop.heap, shop.items);
        shop.items = 0;
    }
    if (shop.order) {
        heap_set_level(shop.heap, shop.order);
        shop.order = 0;
    }
    shop_code_entered();
    shop.pack = 0;
    shop.top = 0;
    shop.cursor = 0;
    shop.n_items = 0;
    memset(shop.bought, 0, sizeof(shop.bought));
    shop.spent = 0;
    shop_model_set_box(0x8002, shop.pack);
    shop_model_set_box(0x8001, shop.pack);
    shop_clerk_load();
    shop_msgwin_open();
    shop.state = 0;
}

/* GE state of the whole screen, the two lights, projection and the bone matrices */
void shop_gfx_init(void)
{
    unsigned int *list;
    Mat4 m;
    int i;

    list = dl_select_layer(0, 0);
    list = dl_offset(list, 0x710, 0x778);
    list = dl_viewport(list, 0x800, 0x800, 480, 272);
    list = dl_depth_range(list, 0xFFFF, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 4);
    list = dl_scissor(list, 0, 0, 480, 272);
    list = dl_enable(list, 2);
    list = dl_alpha_test(list, 6, 0, 0xFF);
    list = dl_disable(list, 0);
    list = dl_depth_func(list, 7);
    list = dl_enable(list, 1);
    list = dl_front_face(list, 1);
    list = dl_enable(list, 5);
    list = dl_disable(list, 9);
    list = dl_shade_model(list, 1);
    list = dl_enable(list, 8);
    list = dl_disable(list, 3);
    list = dl_disable(list, 6);
    list = dl_disable(list, 0x12);
    list = dl_disable(list, 0x11);
    list = dl_disable(list, 0x15);
    list = dl_color(list, 0xFF0000FF);
    list = view_init(list);
    model_set_allocator(shop_model_alloc, shop_model_free);
    list = lighting_enable(list);
    list = light_enable(list, 0);
    list = light_enable(list, 1);
    list = light_set_ambient(list, 0xFF202020);
    list = dl_material_colors(list, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
    list = dl_color_material(list, 7);
    list = dl_specular_power(10.0f, list);
    list = dl_fog(50.0f, 100.0f, list, 0x404040);
    list = dl_disable(list, 7);
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(0.0f, 1.7647059f, 5.0f, 300.0f);
    mtx_set_mode(1);
    mtx_identity();
    mtx_set_mode(2);
    mtx_identity();
    list = dl_clear_color(list, 0xFF770000);
    list = dl_clear_stencil(list, 0);
    list = dl_clear_depth(list, 0);
    list = dl_set_tex_filter(list, 1, 1);
    list = dl_tex_offset(list, 0.0f, 0.0f);
    list = dl_tex_scale(1.0f, -1.0f, list);
    list = dl_enable(list, 9);
    mat4_identity(&m);
    for (i = 0; i < 8; i++)
        list = dl_bone_matrix(list, i, m.m);
    dl_set_list(list);
}

void *shop_model_alloc(unsigned int size)
{
    return heap_alloc(shop.heap, size);
}

void shop_model_free(void *p)
{
    heap_set_level(shop.heap, p);
}

/* frame draw: clear, the scene of the state, the dialog, the fade */
void shop_draw_all(void)
{
    unsigned int *list;

    list = dl_select_layer(0, 0);
    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (shop.state == 15)
        return;
    if (shop.state <= 6 || shop.state == 7) {
        shop_draw_shelf();
        shop_draw_shelf_ui();
        shop_draw_counter();
    } else if (shop.state == 8) {
        shop_draw_open();
    } else if (shop.state >= 9 && shop.state < 15) {
        list = dl_select_layer(0, 0);
        list = dl_clear_color(list, 0xFF000000);
        list = dl_clear_depth(list, 0);
        list = dl_clear_stencil(list, 0);
        list = dl_clear(list, 0x17);
        dl_set_list(list);
        shop_draw_list();
    }
    if (shop.hud) {
        list = dl_select_layer(4, 0);
        list = shop_dialog_draw(list);
        dl_set_list(list);
    }
    list = dl_select_layer(5, 0);
    list = dl_disable(list, 1);
    fade_apply((void **)&list);
    dl_set_list(list);
}

void shop_clerk_init(int alt)
{
    memset(&shop_clerk, 0, sizeof(shop_clerk));
    shop_clerk.alt = alt;
    shop_clerk_set(0);
    if ((unsigned short)alt == 0)
        bgm_fade(0x149);
    else
        bgm_fade(0x148);
}

void shop_clerk_load(void)
{
    if (shop_clerk.alt == 0) {
        shop_clerk.eyes = shop_ehf_get(0x4001);
        shop_clerk.mouth = shop_ehf_get(0x4002);
    } else {
        shop_clerk.eyes = shop_ehf_get(0x4003);
        shop_clerk.mouth = shop_ehf_get(0x4004);
    }
    shop_clerk_set(1);
}

void shop_clerk_update(void)
{
    switch (shop_clerk.state) {
    case 1:
        if (shop_clerk.slide > 13)
            shop_clerk.slide -= 13;
        else
            shop_clerk_set(2);
        break;
    case 3:
        if (shop_msg_typing() == 0 && ehf_get_frame_count(shop_clerk.mouth) == 12) {
            shop_clerk.state = 2;
            ehf_set_anim(shop_clerk.mouth, 0, 1);
        }
    case 2:
        if (shop_clerk.count >= shop_clerk.timeout) {
            if (ehf_get_frame_count(shop_clerk.eyes) == 12) {
                shop_clerk.count = 0;
                shop_clerk.timeout = rand_next_alias() % 210 + 90;
                ehf_set_anim(shop_clerk.eyes, 0, 1);
            }
        } else {
            shop_clerk.count++;
            if (shop_clerk.count >= shop_clerk.timeout)
                ehf_set_anim(shop_clerk.eyes, 6, 13);
        }
        break;
    case 4:
        if (shop_clerk.count++ >= shop_clerk.timeout)
            shop_clerk_set(2);
        break;
    case 5:
        shop_clerk.slide += 13;
        if (shop_clerk.slide < 124)
            shop_clerk.state = 0;
        break;
    }
}

void shop_clerk_draw(unsigned int *list)
{
    EhfOpts o;

    memset(&o, 0, sizeof(o));
    o.time = gfx_get_state()->frame;
    if (shop_clerk.slide) {
        o.flags |= 0x10000;
        o.x = shop_clerk.slide;
    }
    list = gfx_begin_2d(list);
    list = ehf_draw(list, shop_clerk.eyes, &o);
    list = ehf_draw(list, shop_clerk.mouth, &o);
    gfx_end_2d(list);
}

void shop_clerk_set(int state)
{
    switch (state) {
    case 1:
        shop_clerk.slide = 124;
        shop_clerk.count = 0;
        shop_clerk.timeout = 9;
        ehf_set_anim(shop_clerk.eyes, 0, 1);
        ehf_set_anim(shop_clerk.mouth, 0, 1);
        break;
    case 2:
        shop_clerk.slide = 0;
        shop_clerk.count = 0;
        shop_clerk.timeout = rand_next_alias() % 210 + 90;
        ehf_set_anim(shop_clerk.eyes, 0, 1);
        ehf_set_anim(shop_clerk.mouth, 0, 1);
        break;
    case 3:
        shop_clerk.slide = 0;
        ehf_set_anim(shop_clerk.mouth, 6, 13);
        break;
    case 4:
        shop_clerk.slide = 0;
        shop_clerk.count = 0;
        shop_clerk.timeout = 32;
        ehf_set_anim(shop_clerk.eyes, 15, 16);
        ehf_set_anim(shop_clerk.mouth, 15, 16);
        break;
    case 5:
        shop_clerk.slide = 0;
        shop_clerk.count = 0;
        shop_clerk.timeout = 9;
        ehf_set_anim(shop_clerk.eyes, 0, 1);
        ehf_set_anim(shop_clerk.mouth, 0, 1);
        break;
    }
    shop_clerk.state = state;
}

/* voice line of the clerk for an event (Japanese only) */
void shop_clerk_voice(unsigned short what)
{
    int pack = shop.pack;
    int se = -1;
    int r;

    if (cfg_get_language() != 0)
        return;
    if ((unsigned short)shop_pack_can_buy(shop.pack) != 2)
        pack = se;
    switch (what) {
    case 0:
        if (rand_next_alias() % 2)
            se = 0;
        else
            se = 1;
        break;
    case 1:
        se = 2;
        break;
    case 2:
        if (pack < 0)
            return;
        r = shop_pack_rank(pack);
        if (r >= 5)
            return;
        se = r + 3;
        break;
    case 3:
        r = shop_points_left();
        if (r > 0 && r % 20 == 0)
            se = 11;
        else if (rand_next_alias() % 2)
            se = 8;
        else
            se = 9;
        break;
    case 4:
        se = 13;
        break;
    case 5:
        if (shop.mode == 0)
            se = 14;
        break;
    case 6:
        se = 10;
        break;
    }
    if (se >= 0 && shop_clerk_voice_busy() == 0) {
        if (shop_clerk.alt == 0)
            se += 0x61FF;
        else
            se += 0x61EC;
        se_play(se);
    }
}

/* 1 while one of the clerk's voice lines plays */
int shop_clerk_voice_busy(void)
{
    int base;
    int i;

    if (shop_clerk.alt == 0)
        base = 0x61FF;
    else
        base = 0x61EC;
    for (i = 0; i <= 14; i++) {
        if (!(bgm_get_status(base + i) & 0x21) == 0)
            return 1;
    }
    return 0;
}
