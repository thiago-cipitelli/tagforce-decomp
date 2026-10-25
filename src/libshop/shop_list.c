// @range 0x0880D298-0x0880EF74
// @asm 0x0880D3E0-0x0880D828
// @asm 0x0880DFE4-0x0880E598
// @asm 0x0880E860-0x0880EF74
// @data 0x088202B4-0x088202E4
/* 3D part of the shelf (the pack model over the counter, the shelf of the chosen kind with its
   packs and the one flying to the counter), and the list of the cards that came out: five rows with
   picture, marks, name, icons, ATK/DEF and counters, the cursor frame, the header and the bubble that
   shows the sort key of the line under the cursor on the scroll bar. */
#include "libshop.h"

void mtx_translate(float *v);
void mtx_rotate_x(float a);
void mtx_rotate_y(float a);
void mtx_load(Mat4 *m);
void mtx_get34(Mat4 *m);
Mat4 *mat4_identity(Mat4 *out);
void mat4_mul(Mat4 *r, Mat4 *a, Mat4 *b);
Mat4 *mat4_translate(Mat4 *out, Mat4 *m, Vec4 *v);
void model_update(Model *m, float *dt);
void model_draw(Model *m, ModelDraw *c);
void cardinfo_draw(void);
void *cardlist_build(void *list, void *p);
int inv_get_flag7(unsigned short id);
int inv_get_state(unsigned short id);
unsigned short region_pick_value(int flag);
unsigned int *gfx_begin_2d(unsigned int *list);
unsigned int *gfx_end_2d(unsigned int *list);
unsigned int *ehf_draw(unsigned int *list, void *e, EhfOpts *o);
unsigned short *card_get_name(unsigned short id);
int card_get_attribute_icon(unsigned short id);
int card_get_kind(unsigned short id);
int card_is_monster(unsigned short id);
int card_get_type(unsigned short id);
int card_get_level(unsigned short id);
int card_get_atk_q(unsigned short id);
int card_get_def_q(unsigned short id);
int card_get_prop5_hi(unsigned short id);
int card_get_prop6_hi(unsigned short id);
unsigned short card_top_get_letter(unsigned short id);
void font_set_flag4000(int on);
void font_set_flag8(int on);
int font_measure_wtext(int w, unsigned short *text);
unsigned short shop_pack_can_buy(int pack);
unsigned short *shop_pack_name(unsigned short pack);
unsigned short shop_items_count(unsigned short id);
int shop_item_key(ShopItem *it, int key);
unsigned int *shop_draw_sort_tab(unsigned int *list, int sort, int x, int y);
unsigned int *shop_draw_order_tab(unsigned int *list, int order, int x, int y);
unsigned int *shop_sort_draw(unsigned int *list);
unsigned int *shop_detail_draw(unsigned int *list);

void *shop_ui_draw_num_text(void *list, int x, int y, int value, int step, unsigned short n, int pad);
void *shop_ui_draw_frame(void *list, int x, int y, int w, int h, void *gim, int style);
void *shop_ui_draw_icon_a(void *list, UiPt pos, unsigned short id, void *gim);
void *shop_ui_draw_icon_b(void *list, UiPt pos, unsigned short id, void *gim);
void *shop_ui_draw_icon_c(void *list, UiPt pos, unsigned short id, void *gim);
void *shop_ui_draw_icon_d(void *list, UiPt pos, unsigned short id, void *gim);
void *shop_ui_draw_icon_e(void *list, UiPt pos, unsigned short id, void *gim);
void *shop_ui_draw_level(void *list, UiPt pos, unsigned short level, unsigned int color, void *gim);
void *shop_ui_draw_stat(void *list, unsigned int st, UiPt pos, unsigned short size, unsigned int color, void *gim);
void *shop_ui_draw_card_pict(void *list, int x, int y, int shadow, unsigned int color, UiEnt *e, void *gim, void *gim_alt, UiIdList *alt);
void *shop_ui_draw_card_marks(void *list, int x, int y, UiEnt *e, int show_count, int blink, int show_fixed, unsigned short rank, void *gim_digits, void *gim);
void *shop_ui_draw_flag_icon(void *list, UiPt pos, unsigned short kind, void *gim);
void *shop_ui_draw_counters(void *list, UiPt pos, UiCounter *c, void *gim_strip, void *gim);
void *shop_ui_draw_scrollbar(void *list, int x, int y, int h, int pos, int first, int total, int frame, void *gim);
void *shop_ui_draw_box(void *list, int x, int y, int w, int h, void *gim);
void *shop_ui_draw_help(void *list, int x, int y, int alt, unsigned int flags, void *gim, void *strtbl);
int shop_ui_kind_icon(unsigned short kind);

void shop_draw_shelf_pack(void);
void shop_draw_shelf_3d(void);
unsigned int *shop_dl_fade(unsigned int *list, int alpha);
unsigned int *shop_list_bg(unsigned int *list);
unsigned int *shop_list_header(unsigned int *list);
unsigned int *shop_list_rows(unsigned int *list);
unsigned int *shop_list_row(unsigned int *list, ShopItem *it, int row);
unsigned int *shop_draw_kind_frame(unsigned int *list, int x, int y, int kind);
unsigned int *shop_draw_rarity(unsigned int *list, int x, int y, int rarity);
unsigned int *shop_list_cursor(unsigned int *list);
unsigned int *shop_list_key_bubble(unsigned int *list);

void shop_draw_shelf_ui(void)
{
    shop_gfx_shelf();
    mtx_set_mode(2);
    if (shop_pack_can_buy(shop.pack) != 0)
        shop_draw_shelf_pack();
    shop_draw_shelf_3d();
}

/* the pack of the kind under the cursor, turning in when it changes */
void shop_draw_shelf_pack(void)
{
    float pos[3];
    ModelDraw o;
    Model *m;

    memset(&o, 0, sizeof(o));
    o.layer[0] = 2;
    o.layer[1] = 2;
    mtx_identity();
    pos[0] = -18.5f;
    pos[1] = -1.2f;
    pos[2] = -4.0f;
    m = shop_model_get(0x8001);
    mtx_translate(pos);
    if (shop.slide[0] < 0.0f)
        mtx_rotate_x(3.1415927f * shop.slide[0] / 180.0f);
    if (shop.slide[1] < 0.0f)
        mtx_rotate_y(3.1415927f * shop.slide[1] / 180.0f);
    model_update(m, 0);
    model_draw(m, &o);
}

#ifdef NON_MATCHING /* laços das duas fileiras com a matriz salva; alocação diferente */
/* the shelf of the chosen kind: the box, the packs left in both rows, the one flying to the counter */
void shop_draw_shelf_3d(void)
{
    Mat4 fly;
    Mat4 base;
    ModelDraw o;
    float pos[3];
    Model *m;
    int side;
    int i;

    mtx_identity();
    if (shop.state == 4 || shop.state == 3) {
        dl_set_list(shop_dl_fade(dl_select_layer(2, 0), 0xA0));
        pos[1] = -3.0f;
        pos[2] = -8.0f;
        pos[0] = 0.0f;
        mtx_translate(pos);
        mtx_rotate_x(0.7853982f);
        mtx_rotate_y(3.1415927f * shop.rot / 180.0f);
    } else if (shop.state == 5 || shop.state == 2) {
        dl_set_list(shop_dl_fade(dl_select_layer(2, 0), (unsigned char)(shop.timer * 0xA0 / 18)));
        pos[0] = 0.0f;
        pos[1] = -20.0f - shop.timer * -17.0f / 18.0f;
        pos[2] = -200.0f - shop.timer * -192.0f / 18.0f;
        mtx_translate(pos);
        mtx_rotate_x(0.7853982f);
        mtx_rotate_y(3.1415927f * (shop.rot + shop.timer * 360.0f / 18.0f) / 180.0f);
    } else {
        return;
    }
    mtx_get34(&base);
    memset(&o, 0, sizeof(o));
    o.layer[0] = 2;
    o.layer[1] = 3;
    m = shop_model_get(0x8002);
    model_update(m, 0);
    model_draw(m, &o);
    m = shop_model_get(0x8001);
    model_update(m, 0);
    for (side = 0; side < 2; side++) {
        pos[2] = -1.4f;
        pos[0] = -3.0f + 6.0f * side;
        for (i = 0; i < (shop.stock[shop.pack] >> (side * 4) & 0xF); i++) {
            mtx_load(&base);
            pos[1] = shop.lift[side][i];
            mtx_translate(pos);
            model_draw(m, &o);
            pos[2] += 0.3f;
        }
    }
    if (shop.taken) {
        mat4_identity(&fly);
        mat4_mul(&fly, &fly, &shop.rotm);
        mat4_translate(&fly, &fly, &shop.pos);
        mtx_load(&fly);
        model_draw(m, &o);
    }
}
#endif

/* darkens the screen behind the 3D shelf */
unsigned int *shop_dl_fade(unsigned int *list, int alpha)
{
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 1);
    list = dl_disable(list, 9);
    list = dl_tex_func(list, 4, 0);
    list = dl_ambient_color(list, (unsigned char)alpha << 24);
    list = draw_set_offsets(list, 0, 0, 480, 272);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    list = dl_enable(list, 9);
    list = dl_tex_func(list, 0, 1);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_enable(list, 1);
    return dl_disable(list, 4);
}

void shop_draw_list(void)
{
    unsigned int *list;
    ShopItem *it;
    int y;

    if (shop.info_open) {
        cardinfo_draw();
        return;
    }
    list = dl_select_layer(4, 0);
    list = shop_dl_2d(list);
    list = shop_list_bg(list);
    list = shop_list_header(list);
    list = cardlist_build(list, &shop.sel_id);
    if (shop_item_id_at(shop.top + shop.cursor) != 0) {
        if (inv_get_flag7(shop_item_id_at(shop.top + shop.cursor)))
            list = shop_ui_draw_flag_icon(list, PT(0x7B, 0x87), 2, shop_res_get(0x2026));
    }
    list = shop_list_rows(list);
    list = shop_list_cursor(list);
    switch (shop.state) {
    case 10: {
        int y = shop.cursor >= 2 ? 0x36 : 0xA2;
        list = shop_ui_draw_help(list, 0xD0, y, 0, 0xF000, shop_res_get(0x202A), shop_str_get(0xC005));
        list = shop_list_key_bubble(list);
        break;
    }
    case 11: {
        int y = shop.cursor >= 2 ? 0x36 : 0xA2;
        list = shop_ui_draw_help(list, 0xD0, y, 1, region_pick_value(1) | 0x8000, shop_res_get(0x202A),
                                 shop_str_get(0xC005));
        break;
    }
    case 13:
        list = shop_detail_draw(list);
        break;
    case 12:
        list = shop_sort_draw(list);
        break;
    }
    dl_set_list(list);
}

unsigned int *shop_list_bg(unsigned int *list)
{
    void *e = shop_ehf_get(0x4005);
    EhfOpts o;

    if (e == 0)
        return list;
    memset(&o, 0, sizeof(o));
    o.time = gfx_get_state()->frame;
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, &o);
    list = gfx_end_2d(list);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    return dl_blend_func(list, 0, 2, 3, 0, 0);
}

/* x of the card count by language */
short shop_count_x[6] = { 0x1CD, 0x1D7, 0x1D7, 0x1D7, 0x1D7, 0x1D7 };

/* tabs, sort and order, card count, page and scroll bar */
unsigned int *shop_list_header(unsigned int *list)
{
    void *gim = shop_res_get(0x2003);

    list = dl_set_tex_filter(list, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    if (gim) {
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 0xDC, 0x100, 0xDC, 0x100);
        list = draw_sprite(list, 0x94, 0x10, 0, 0, 0, 0, 0);
        list = draw_set_offsets(list, 0x4C, 0x100, 0x4C, 0x100);
        list = draw_sprite(list, 0x170, 0x10, 0, 0x3C, 0, 0, 0);
        list = draw_set_offsets(list, 0x24, 0x100, 0x24, 0x100);
        list = draw_sprite(list, 0x1BC, 0x10, 0, 0xDC, 0, 0, 0);
    }
    gim = shop_res_get(0x2004);
    if (gim) {
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 0x100, 0x20, 0x100, 0x20);
        list = draw_sprite(list, 0x94, 0, 0, 0, 0, 0, 0);
    }
    list = shop_draw_sort_tab(list, shop.sort, 0x97, 0xFD);
    list = shop_draw_order_tab(list, shop.order_dir, 0xA9, 0xFD);
    gim = shop_res_get(0x2002);
    if (gim) {
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 0x7C, 0x18, 0x7C, 0x18);
        list = draw_sprite(list, 0x166, 0, 0, 0x86, 0x90, 0, 0);
    }
    font_set_size(10, 12);
    font_set_color(0xFF000000);
    list = shop_ui_draw_num_text(list, shop_count_x[cfg_get_language()], 5, shop.n_items, 8, 4, 0);
    font_set_size(11, 12);
    list = shop_ui_draw_num_text(list, 0x116, 12, shop.n_owned, 8, 6, 0);
    return shop_ui_draw_scrollbar(list, 0x1D5, 0x23, 0xCF, shop.top, 5, shop.n_items, 2, shop_res_get(0x2023));
}

unsigned int *shop_list_rows(unsigned int *list)
{
    int i;

    for (i = 0; i < 5; i++) {
        if (shop.top + i >= shop.n_items)
            break;
        list = shop_list_row(list, shop_item_at(shop.top + i), i);
    }
    return list;
}

/* counters of a row: copies that came out, copies owned (red over 99) */
UiCounter shop_row_counters[3] = {
    { 0, { 0 }, 0, 0 }, { 5, { 0 }, 0xFF000000, 0 }, { 6, { 0 }, 0xFF000000, 0 },
};

#ifdef NON_MATCHING /* muitos valores da carta guardados em s e na pilha; alocação diferente */
unsigned int *shop_list_row(unsigned int *list, ShopItem *it, int row)
{
    UiEnt e;
    UiCounter c[3];
    unsigned short *name;
    unsigned short *pack;
    int attr, kind, type, level, atk, def, rarity;
    int y, ty;
    int total, owned;

    memset(&e, 0, sizeof(e));
    e.id = it->id;
    e.key = it->key;
    e.count = it->count;
    name = card_get_name(it->id);
    pack = shop_pack_name(it->pack);
    attr = card_get_attribute_icon(it->id);
    kind = card_get_kind(it->id);
    if (card_is_monster(it->id)) {
        type = card_get_type(it->id);
        level = card_get_level(it->id);
        atk = card_get_atk_q(it->id) & 0xFFFF;
        def = card_get_def_q(it->id) & 0xFFFF;
    } else {
        type = card_get_prop5_hi(it->id);
        level = 0;
        atk = 0;
        def = 0;
    }
    rarity = card_get_prop6_hi(it->id);
    y = 0x1A + row * 0x2D;
    list = shop_ui_draw_card_pict(list, 0x97, y, 1, 0xFFFFFFFF, &e, shop_res_get(0x2020), shop_res_get(0x2021),
                                  shop_str_get(0xC004));
    list = shop_ui_draw_card_marks(list, 0x97, y, &e, 1, inv_get_state(e.id) == 0, 0, 0, shop_res_get(0x2025),
                                   shop_res_get(0x2026));
    list = shop_draw_rarity(list, 0x98, y + 0x20, rarity);
    list = shop_draw_kind_frame(list, 0xAE, y, attr);
    font_set_size(12, 12);
    font_set_color(0xFF000000);
    font_text_set(list);
    if (cfg_get_language()) {
        font_set_flag4000(1);
        font_set_flag8(0);
    }
    font_draw_text(0x2D00, (y + 2) << 6, 0x4100, name);
    if (cfg_get_language()) {
        font_set_flag4000(0);
        font_set_flag8(1);
    }
    list = font_text_clear();
    if (inv_get_flag7(it->id))
        list = shop_ui_draw_flag_icon(list, PT(0x1BC, y + 2), 2, shop_res_get(0x2026));
    ty = y + 0xE;
    list = shop_ui_draw_icon_c(list, PT(0xB3, ty), kind, shop_res_get(0x2023));
    if (card_is_monster(it->id)) {
        list = shop_ui_draw_icon_a(list, PT(0xC4, ty), type, shop_res_get(0x2023));
        list = shop_ui_draw_level(list, PT(0xD7, ty + 2), level, 0xFF000000, shop_res_get(0x2025));
        list = shop_ui_draw_stat(list, atk << 16, PT(0xFB, ty + 2), 0xC0A, 0xFF000000, 0);
        list = shop_ui_draw_stat(list, def << 16 | 1, PT(0x137, ty + 2), 0xC0A, 0xFF000000, 0);
    } else {
        list = shop_ui_draw_icon_b(list, PT(0xC4, ty), type, shop_res_get(0x2023));
    }
    memcpy(c, shop_row_counters, sizeof(c));
    c[0].value = it->partners;
    c[2].value = it->owned;
    total = shop_items_count(it->id);
    owned = it->owned;
    if (owned + total > 99) {
        c[2].color = 0xFF0000FF;
        if (owned > 99)
            c[2].value = 99;
    }
    list = shop_ui_draw_counters(list, PT(0x173, ty), c, shop_res_get(0x2022), shop_res_get(0x2026));
    if ((name = shop_res_get(0x2002)) != 0) {
        list = gim_set_texture(list, name);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 10, 16, 10, 16);
        list = draw_sprite(list, 0xD7, y + 0x1D, 0, 0x53, 0x91, 0, 0);
    }
    font_set_size(12, 12);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    font_draw_text(0x38C0, (y + 0x20) << 6, 0x3A80, pack);
    return font_text_clear();
}
#endif

/* the frame of the card kind behind a row */
unsigned int *shop_draw_kind_frame(unsigned int *list, int x, int y, int kind)
{
    void *gim = shop_res_get(0x2002);
    int k;
    int u, v;

    if (gim == 0)
        return list;
    k = shop_ui_kind_icon(kind);
    if (k < 0)
        return list;
    u = k / 3 * 0x80;
    v = k % 3 * 0x30;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x67, 0x30, 0x67, 0x30);
    list = draw_sprite(list, x, y, 0, u, v, 0, 0);
    list = draw_set_offsets(list, 0x62, 0x30, 0x62, 0x30);
    list = draw_sprite(list, x + 0x67, y, 0, u + 5, v, 0, 0);
    list = draw_set_offsets(list, 0x58, 0x30, 0x58, 0x30);
    list = draw_sprite(list, x + 0xC9, y, 0, u + 5, v, 0, 0);
    list = draw_set_offsets(list, 6, 0x30, 6, 0x30);
    return draw_sprite(list, x + 0x121, y, 0, u + 0x67, v, 0, 0);
}

/* rarity mark: ultra, super, rare */
unsigned int *shop_draw_rarity(unsigned int *list, int x, int y, int rarity)
{
    void *gim;
    int k;

    switch (rarity) {
    case 1:
        k = 2;
        break;
    case 3:
        k = 1;
        break;
    case 4:
        k = 0;
        break;
    default:
        return list;
    }
    gim = shop_res_get(0x2027);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x18, 12, 0x18, 12);
    return draw_sprite(list, x, y, 0, k * 0x18, 0x61, 0, 0);
}

#ifdef NON_MATCHING /* o original carrega o 26 da linha antes de chamar shop_res_get; registradores diferentes */
unsigned int *shop_list_cursor(unsigned int *list)
{
    int y = 0x1A + shop.cursor * 0x2D;
    void *gim = shop_res_get(0x2022);

    return shop_ui_draw_frame(list, 0x97, y, 0x13E, 0x2D, gim, gfx_get_state()->frame);
}
#endif

#ifdef NON_MATCHING /* switch duplo com tabelas de saltos; o original mantém o tamanho da bolha em s0/s4 sem inicializar no caso 0 */
/* the bubble next to the scroll bar with the sort key of the line under the cursor */
unsigned int *shop_list_key_bubble(unsigned int *list)
{
    unsigned short sort = shop.sort;
    unsigned int line = shop.top + shop.cursor;
    unsigned int key;
    int w, h;
    int x, y;
    int fw;
    unsigned char size;
    unsigned short ch[2];

    if (line >= shop.n_items)
        return list;
    key = shop_item_key(shop_item_at(line), sort);
    switch (sort) {
    case 1:
        w = 0x4E;
        h = 0x14;
        break;
    case 2:
    case 4:
        w = 0x1A;
        h = 0x14;
        break;
    case 3:
        w = 0x1F;
        h = 0x16;
        break;
    case 5:
        font_set_size(14, 14);
        fw = font_measure_text(L"W");
        w = (fw >> 6) + 13;
        h = (fw >> 6) + 11;
        break;
    case 6:
    case 10:
    case 11:
    case 12:
    case 13:
        w = 0x1B;
        h = 0x1B;
        break;
    case 7:
        w = 0x27;
        h = 0x14;
        break;
    case 8:
    case 9:
        w = 0x30;
        h = 0x13;
        break;
    }
    x = 0x1D5 - w;
    y = line * 0xCF / shop.n_items + 0x23 - h / 2;
    list = shop_ui_draw_box(list, x, y, w, h, shop_res_get(0x202A));
    if (key == 0xFFFFFFFE)
        return list;
    switch (sort) {
    case 0:
        break;
    case 1:
        font_text_set(list);
        font_set_size(12, 12);
        font_set_color(0xFFFFFFFF);
        size = font_measure_wtext((w - 11) << 6, shop_pack_name(key));
        if (size < 8)
            size = 8;
        font_set_size(size, 12);
        font_draw_text((x + 5) << 6, (y + 4) << 6, (w - 11) << 6, shop_pack_name(key));
        list = font_text_clear();
        break;
    case 2:
        if (key) {
            void *gim = shop_res_get(0x2026);
            list = dl_ambient_color(list, 0xFFFFFFFF);
            list = dl_set_tex_filter(list, 0, 0);
            list = gim_set_texture(list, gim);
            list = draw_set_offsets(list, 0x11, 12, 0x11, 12);
            list = draw_sprite(list, x + 4, y + 5, 0, 0x25, 0x4C, 0, 0);
        }
        break;
    case 3:
        list = shop_draw_rarity(list, x + 5, y + 5, key);
        break;
    case 4:
        if (key)
            list = shop_ui_draw_flag_icon(list, PT(x + 6, y + 5), 2, shop_res_get(0x2026));
        break;
    case 5:
        font_text_set(list);
        font_set_size(14, 14);
        font_set_color(0xFFFFFFFF);
        ch[0] = 0;
        ch[1] = 0;
        ch[0] = card_top_get_letter(key);
        fw = font_measure_text(ch);
        font_draw_text(((x + 4 + (w - 9) / 2) << 6) - fw / 2, (y + 4) << 6, w << 6, ch);
        list = font_text_clear();
        break;
    case 6:
        list = shop_ui_draw_icon_d(list, PT(x + 5, y + 5), shop_ui_kind_icon(key), shop_res_get(0x2026));
        break;
    case 7:
        list = shop_ui_draw_level(list, PT(x + 5, y + 4), key, 0xFFFFFFFF, shop_res_get(0x2025));
        break;
    case 8:
        if ((int)key > 0)
            key = (int)key / 100 * 100;
        else if (key == 0xFFFFFFFF)
            key = 0xFFFF;
        list = shop_ui_draw_stat(list, key << 16 | 2, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, shop_res_get(0x2025));
        break;
    case 9:
        if ((int)key > 0)
            key = (int)key / 100 * 100;
        else if (key == 0xFFFFFFFF)
            key = 0xFFFF;
        list = shop_ui_draw_stat(list, key << 16 | 3, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, shop_res_get(0x2025));
        break;
    case 10:
        list = shop_ui_draw_icon_a(list, PT(x + 5, y + 5), key, shop_res_get(0x2023));
        break;
    case 11:
        list = shop_ui_draw_icon_c(list, PT(x + 5, y + 5), key, shop_res_get(0x2023));
        break;
    case 12:
        list = shop_ui_draw_icon_b(list, PT(x + 5, y + 5), key, shop_res_get(0x2023));
        break;
    case 13:
        list = shop_ui_draw_icon_e(list, PT(x + 5, y + 5), key, shop_res_get(0x2026));
        break;
    }
    return list;
}
#endif
