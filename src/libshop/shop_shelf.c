// @range 0x0880C238-0x0880D298
// @asm 0x0880C83C-0x0880C904
// @asm 0x0880CA8C-0x0880CB70
// @asm 0x0880CDAC-0x0880CF7C
/* 2D layers of the shop: the counter (clerk, points, total spent, message window, the pack menu
   with completion, price and cards per pack), the shelf with the 48 pack icons in four rows, the
   blinking cursor, and the sold out / price tags of the pack under the cursor. */
#include "libshop.h"

void menu_draw(void **dl, MenuWidget *w);
unsigned short shop_pack_can_buy(int pack);
unsigned short shop_pack_price(int pack);
unsigned short shop_pack_n_cards(unsigned short pack);
unsigned short shop_pack_get_complete(unsigned short pack);
unsigned int *shop_draw_number(void *list, unsigned short x, unsigned short y, int value, unsigned short step,
                               unsigned short digits, int commas);
unsigned short *shop_pack_name(unsigned short pack);

unsigned int *shop_draw_spent(unsigned int *list);
unsigned int *shop_shelf_bg(unsigned int *list);
unsigned int *shop_shelf_icons(unsigned int *list);
unsigned int *shop_shelf_icon(unsigned int *list, int x, int y, int pack);
unsigned int *shop_shelf_pegs(unsigned int *list);
unsigned int *shop_shelf_cursor(unsigned int *list, int pack);
unsigned int *shop_shelf_closed(unsigned int *list);
unsigned int *shop_shelf_slide(unsigned int *list);
unsigned int *shop_shelf_tag(unsigned int *list);

void shop_draw_counter(void)
{
    unsigned int *list;

    list = dl_select_layer(4, 0);
    list = shop_dl_2d(list);
    list = shop_clerk_draw(list);
    list = shop_pts_draw(list);
    list = shop_draw_spent(list);
    shop_msgwin_draw(&list);
    if (shop.state)
        menu_draw((void **)&list, &shop.menu);
    dl_set_list(list);
}

/* points spent this visit, under the counter */
unsigned int *shop_draw_spent(unsigned int *list)
{
    void *gim = shop_res_get(0x2002);

    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x80, 0x40, 0x80, 0x40);
    list = draw_sprite(list, 0x160, 0x88, 0, 0x80, 0xC0, 0, 0);
    font_set_size(10, 12);
    font_set_flag4(0);
    font_set_color(0xFF0080FF);
    list = shop_draw_number(list, 0x1D5, 0x91, shop.spent, 8, 9, 1);
    if (shop.spent) {
        unsigned short minus[2] = { 0 };
        font_text_set(list);
        minus[0] = '-';
        font_draw_text(0x5F80, 0x2440, 0x1880, minus);
        list = font_text_clear();
    }
    font_set_flag4(1);
    return list;
}

/* the pack menu line: name, cards per pack, price in DP, completion in % */
unsigned int *shop_menu_draw(unsigned int *list, int x, int y)
{
    void *gim = shop_res_get(0x2002);
    unsigned short s[128];
    int px;
    int ty;
    int n;

    if (shop_pack_can_buy(shop.pack) == 0)
        return list;
    font_set_size(14, 14);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    s[0] = '%';
    s[1] = 0;
    px = x + 0x1CC;
    px -= font_measure_text(s) >> 6;
    ty = (y + 1) << 6;
    font_draw_text((px + 1) << 6, ty, (480 - (px + 1)) << 6, s);
    font_set_size(12, 14);
    list = font_text_clear();
    list = shop_draw_number(list, px, y + 1, shop_pack_get_complete(shop.pack), 10, 3, 0);
    if (gim) {
        list = gim_set_texture(list, gim);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 12, 16, 12, 16);
        list = draw_sprite(list, px - 0x2B, y - 1, 0, 0x75, 0x91, 0, 0);
    }
    s[0] = 'D';
    s[1] = 'P';
    s[2] = 0;
    px -= 0x30;
    px -= (font_measure_text(s) >> 6) + 1;
    font_set_color(0xFF00FFFF);
    font_text_set(list);
    font_draw_text(px << 6, ty, (480 - px) << 6, s);
    font_set_size(12, 14);
    font_set_color(0xFFFFFFFF);
    list = font_text_clear();
    list = shop_draw_number(list, px, y + 1, shop_pack_price(shop.pack), 10, 5, 0);
    n = shop_pack_n_cards(shop.pack);
    if (n >= 10) {
        list = shop_draw_number(list, px - 0x37, y + 1, n, 10, 2, 0);
        px -= 0x4D;
    } else {
        list = shop_draw_number(list, px - 0x37, y + 1, n, 10, 1, 0);
        px -= 0x43;
    }
    font_set_size(14, 14);
    s[0] = 'x';
    s[1] = 0;
    px -= (font_measure_text(s) >> 6) + 1;
    font_text_set(list);
    font_draw_text(px << 6, ty, (480 - px) << 6, s);
    list = font_text_clear();
    if (gim) {
        list = gim_set_texture(list, gim);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 10, 16, 10, 16);
        list = draw_sprite(list, px - 11, y, 0, 0x65, 0x91, 0, 0);
    }
    if (gim) {
        list = gim_set_texture(list, gim);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 10, 16, 10, 16);
        list = draw_sprite(list, x, y - 1, 0, 0x53, 0x91, 0, 0);
    }
    font_text_set(list);
    font_draw_text((x + 12) << 6, ty, (px - 16 - (x + 12)) << 6, shop_pack_name(shop.pack));
    return font_text_clear();
}

#ifdef NON_MATCHING /* a chamada do cursor no original fica com o atraso vazio (move a0 antes do jal); o mwcc põe o move no atraso */
void shop_draw_shelf(void)
{
    unsigned int *list;

    list = dl_select_layer(1, 0);
    list = shop_dl_2d(list);
    list = shop_shelf_bg(list);
    list = shop_shelf_icons(list);
    list = shop_shelf_pegs(list);
    if (shop.state < 2 || shop.state >= 6)
        list = shop_shelf_cursor(list, shop.pack);
    if (shop_pack_can_buy(shop.pack) == 0) {
        list = shop_shelf_closed(list);
    } else {
        list = shop_shelf_tag(list);
        list = shop_shelf_slide(list);
    }
    dl_set_list(list);
}
#endif

unsigned int *shop_shelf_bg(unsigned int *list)
{
    void *gim = shop_res_get(0x2001);
    unsigned int *p;
    int i;

    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    p = dl_ambient_color(list, 0xFFFFFFFF);
    p = dl_set_tex_filter(p, 0, 0);
    p = draw_set_offsets(p, 0x40, 0x100, 0x40, 0x100);
    for (i = 0; i < 7; i++)
        p = draw_sprite(p, i * 0x40, 0, 0, i * 0x40, 0, 0, 0);
    p = draw_set_offsets(p, 0x20, 0x100, 0x20, 0x100);
    p = draw_sprite(p, i * 0x40, 0, 0, i * 0x40, 0, 0, 0);
    p = draw_set_offsets(p, 0x10, 0x40, 0x40, 0x10);
    for (i = 0; i < 4; i++) {
        p = draw_sprite(p, 0x100 - (i + 1) * 0x40, 0x100, 0, 0x1E0, i * 0x40, 0, 3);
        p = draw_sprite(p, 0x200 - (i + 1) * 0x40, 0x100, 0, 0x1F0, i * 0x40, 0, 3);
    }
    return p;
}

#ifdef NON_MATCHING /* contador do pack e coluna trocam s2/s4 em todas as ordens de declaração testadas */
unsigned int *shop_shelf_icons(unsigned int *list)
{
    int col;
    int row;
    unsigned int pack = 0;

    for (row = 0; row < 4; row++) {
        for (col = 0; col < 12; pack++, col++) {
            switch (shop_pack_can_buy((unsigned short)pack)) {
            case 0:
                continue;
            case 1:
                list = dl_ambient_color(list, 0xFF808080);
                break;
            default:
                list = dl_ambient_color(list, 0xFFFFFFFF);
                break;
            }
            list = shop_shelf_icon(list, 0x56 + col * 0x16, 5 + row * 0x2F, pack);
        }
    }
    return list;
}
#endif

unsigned int *shop_shelf_icon(unsigned int *list, int x, int y, int pack)
{
    void *icons = shop_res_get(0x2006);
    void *panel = shop_res_get(0x2002);

    if (icons == 0 || panel == 0)
        return list;
    list = gim_set_texture(list, panel);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x18, 0x2C, 0x18, 0x2C);
    list = draw_sprite(list, x, y, 0, 0x32, 0xA4, 0, 0);
    list = gim_set_texture(list, icons);
    list = draw_set_offsets(list, 0x14, 0x28, 0x14, 0x28);
    return draw_sprite(list, x, y, 0, pack % 12 * 0x14, pack / 12 * 0x28, 0, 0);
}

unsigned int *shop_shelf_pegs(unsigned int *list)
{
    void *gim = shop_res_get(0x2002);
    unsigned int *p;
    int col;
    int row;

    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    p = dl_ambient_color(list, 0xFFFFFFFF);
    p = dl_set_tex_filter(p, 0, 0);
    p = draw_set_offsets(p, 2, 8, 2, 8);
    for (row = 0; row < 4; row++) {
        for (col = 0; col < 12; col++)
            p = draw_sprite(p, 0x5F + col * 0x16, 4 + row * 0x2F, 0, 0x83, 0x90, 0, 0);
    }
    return p;
}

#ifdef NON_MATCHING /* o original não dobra x + 20 - 12 e aloca o deslocamento do pulso em s4; o mwcc dobra as constantes */
/* four corners around the icon, breathing with the frame counter */
unsigned int *shop_shelf_cursor(unsigned int *list, int pack)
{
    int x = pack % 12 * 0x16 + 0x56;
    int y = pack / 12 * 0x2F + 5;
    int d = gfx_get_state()->frame >> 1 & 7;
    void *gim;
    int x0, y0, x1, y1;

    if (d > 4)
        d = 8 - d;
    gim = shop_res_get(0x2002);
    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 12, 12, 12, 12);
    x0 = x - (d + 2);
    y0 = y - (d + 2);
    list = draw_sprite(list, x0, y0, 0, 0, 0xD6, 0, 0);
    x1 = x + 20 + (d + 2) - 12;
    list = draw_sprite(list, x1, y0, 0, 0, 0xD6, 1, 0);
    y1 = y + 40 + (d + 2) - 12;
    list = draw_sprite(list, x0, y1, 0, 0, 0xD6, 2, 0);
    return draw_sprite(list, x1, y1, 0, 0, 0xD6, 3, 0);
}
#endif

/* "now printing": the pack kind is not on sale yet */
unsigned int *shop_shelf_closed(unsigned int *list)
{
    void *gim = shop_res_get(0x2005);

    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x4A, 0xA8, 0x4A, 0xA8);
    return draw_sprite(list, 1, -17, 0, 0, 0, 0, 0);
}

/* the pack picture sliding in from the left */
unsigned int *shop_shelf_slide(unsigned int *list)
{
    void *gim = shop_res_get(0x2002);
    int w;

    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_set_tex_filter(list, 1, 1);
    w = 0x48;
    if (shop.slide[1] < 0.0f)
        w -= (int)(shop.slide[1] * 3.0f) / 4;
    list = draw_set_offsets(list, 0x18, 0x2C, w, 0x8D);
    return draw_sprite(list, 7, 0x10, 0, 0x32, 0xA4, 0, 0);
}

/* price tag, or "sold out" */
unsigned int *shop_shelf_tag(unsigned int *list)
{
    void *gim = shop_res_get(0x2006);

    if (gim == 0)
        return list;
    switch (shop_pack_can_buy(shop.pack)) {
    case 0:
        break;
    case 1:
        list = gim_set_texture(list, gim);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 0x45, 0x21, 0x45, 0x21);
        list = draw_sprite(list, 3, 0x9B, 0, 0, 0xDF, 0, 0);
        break;
    case 2:
        list = gim_set_texture(list, gim);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 0x45, 0x21, 0x45, 0x21);
        list = draw_sprite(list, 3, 0x9B, 0, 0, 0xBE, 0, 0);
        font_set_size(12, 15);
        font_set_flag4(0);
        font_set_color(0xFF7E0819);
        list = shop_draw_number(list, 0x34, 0xA4, shop_pack_price(shop.pack), 9, 5, 0);
        list = shop_draw_number(list, 0x35, 0xA4, shop_pack_price(shop.pack), 9, 5, 0);
        font_set_flag4(1);
        break;
    }
    return list;
}
