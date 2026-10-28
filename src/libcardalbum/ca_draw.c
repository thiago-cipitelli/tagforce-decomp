// @range 0x08805CA4-0x0880681C
// @asm 0x08805CA4-0x08805F30
// @data 0x0880DBA0-0x0880DC50
/* 2D part of the screen and the per-frame draw export: background, title bar with the completion
   percentage, the skip menu with the page number, the pointing hand over the cursor and the digits. */
#include "libcardalbum.h"

typedef struct CaPoint {       /* screen position of the hand (stored as two ints) */
    unsigned short x, pad0;
    unsigned short y, pad1;
} CaPoint;

unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
unsigned int *dl_clear(unsigned int *list, unsigned int flags);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
unsigned int *gfx_begin_2d(unsigned int *list);
unsigned int *gfx_end_2d(unsigned int *list);
unsigned int *ehf_draw(unsigned int *list, void *ehf, void *opts);
GfxState *gfx_get_state(void);
void mtx_set_mode(int mode);
void cardinfo_draw(void);

void ca_gfx_begin3d(void);
void ca_book_draw(void);
void ca_pages_draw(void);
unsigned int *ca_draw_bg(unsigned int *list);
unsigned int *ca_draw_hand(unsigned int *list);
unsigned int *ca_draw_header(unsigned int *list);
unsigned int *ca_draw_footer(unsigned int *list);
unsigned int *ca_draw_number(unsigned int *list, int x, short y, int n, int step);
unsigned int *ca_index_draw(unsigned int *list);
CaPage *ca_cur_page(void);

#ifdef NON_MATCHING /* o original deixa a cópia morta da primeira instrução do ramo depois do `b` (beqzl; b; <cópia>), problema aberto (ver docs/build.md) */
/* 0x08805CA4: export, once per frame */
void cardalbum_draw(void)
{
    unsigned int *list;

    list = dl_select_layer(0, 0);
    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (ca.state != 8 && ca.state != 0) {
        if (ca.detail) {
            cardinfo_draw();
        } else {
            list = dl_select_layer(0, 0);
            list = dl_enable(list, 9);
            list = dl_enable(list, 4);
            list = dl_tex_func(list, 0, 1);
            list = dl_set_tex_filter(list, 0, 0);
            list = dl_blend_func(list, 0, 2, 3, 0, 0);
            list = dl_disable(list, 1);
            list = ca_draw_bg(list);
            dl_set_list(list);
            ca_gfx_begin3d();
            mtx_set_mode(2);
            ca_book_draw();
            ca_pages_draw();
            list = dl_select_layer(4, 0);
            list = dl_enable(list, 9);
            list = dl_enable(list, 4);
            list = dl_tex_func(list, 0, 1);
            list = dl_set_tex_filter(list, 0, 0);
            list = dl_blend_func(list, 0, 2, 3, 0, 0);
            list = dl_disable(list, 1);
            list = ca_draw_header(list);
            list = ca_draw_footer(list);
            if (ca.state == 3)
                list = ca_draw_hand(list);
            list = cardlist_build(list, &ca.panel);
            if (ca.state == 5)
                list = ca_index_draw(list);
            dl_set_list(list);
            list = dl_select_layer(5, 0);
            list = dl_disable(list, 1);
            fade_apply((void **)&list);
            dl_set_list(list);
        }
    }
}
#endif

/* 480x272 background in 64-pixel strips; the bottom 16 lines come from the right edge, rotated */
unsigned int *ca_draw_bg(unsigned int *list)
{
    void *gim = ca_res_get(CA_BG);
    int i;
    int x;
    int v;
    int j;

    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 64, 256, 64, 256);
    for (i = 0, x = 0; i < 7; i++, x += 64)
        list = draw_sprite(list, x, 0, 0, x, 0, 0, 0);
    list = draw_set_offsets(list, 32, 256, 32, 256);
    list = draw_sprite(list, i * 64, 0, 0, i * 64, 0, 0, 0);
    list = draw_set_offsets(list, 16, 64, 64, 16);
    for (j = 0, v = 0; j < 4; j++, v += 64) {
        list = draw_sprite(list, 256 - (j + 1) * 64, 256, 0, 480, v, 0, 3);
        list = draw_sprite(list, 512 - (j + 1) * 64, 256, 0, 496, v, 0, 3);
    }
    return list;
}

/* where the hand points: the 18 card cells, then the menu entries */
CaPoint ca_hand_cells[18] = {
    { 180, 0, 57, 0 }, { 223, 0, 57, 0 }, { 266, 0, 57, 0 },
    { 349, 0, 57, 0 }, { 393, 0, 57, 0 }, { 436, 0, 57, 0 },
    { 178, 0, 113, 0 }, { 222, 0, 113, 0 }, { 266, 0, 113, 0 },
    { 350, 0, 113, 0 }, { 395, 0, 113, 0 }, { 440, 0, 113, 0 },
    { 175, 0, 171, 0 }, { 221, 0, 171, 0 }, { 266, 0, 171, 0 },
    { 352, 0, 171, 0 }, { 397, 0, 171, 0 }, { 442, 0, 171, 0 },
};
unsigned short ca_hand_menu[6] = { 173, 212, 251, 369, 408, 447 };

/* the pointing hand, tapping (frames 1 every other 4 frames) */
unsigned int *ca_draw_hand(unsigned int *list)
{
    void *gim = ca_res_get(CA_HAND);
    unsigned short x;
    unsigned short y;
    unsigned short v;
    int flip = 0;

    if (gim == 0)
        return list;
    v = 0x80;
    switch (ca.cur.area) {
    case 0:
        x = ca_hand_cells[ca.cur.pos].x;
        y = ca_hand_cells[ca.cur.pos].y;
        break;
    case 1:
        if (ca.cur.side == 0) {
            flip = 1;
            x = 0x96;
        } else {
            x = 0x1B8;
        }
        y = 200;
        v = 0xA0;
        break;
    case 2:
        y = 0xF0;
        x = ca_hand_menu[ca.cur.item];
        break;
    }
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 32, 32, 32, 32);
    {
        unsigned char frames[10] = { 0, 0, 0, 0, 0, 1, 0, 1, 0, 1 };
        return draw_sprite(list, x, y, 0, frames[(gfx_get_state()->frame >> 2) % 10] * 32 + 64, v, flip, 0);
    }
}

/* title bar: name of the screen, the "collected" label and the per mille as "99.9%" */
unsigned int *ca_draw_header(unsigned int *list)
{
    unsigned short ch[2] = { 0 };
    void *gim = ca_res_get(CA_MISC);
    int w;

    if (gim) {
        list = gim_set_texture(list, gim);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 12, 25, 12, 25);
        list = draw_sprite(list, 0x91, 0, 0, 0, 20, 0, 0);
        list = draw_set_offsets(list, 1, 25, 0x137, 25);
        list = draw_sprite(list, 0x9D, 0, 0, 6, 20, 0, 0);
        list = draw_set_offsets(list, 12, 25, 12, 25);
        list = draw_sprite(list, 0x1D4, 0, 0, 13, 20, 0, 0);
    }
    font_set_size(14, 14);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    font_draw_text(0x2580, 0x100, 0x2D00, table_get_entry(ca_res_get(CA_STRINGS), 1));
    w = font_measure_text(table_get_entry(ca_res_get(CA_STRINGS), 2));
    font_draw_text(0x68C0 - w, 0x100, w + 0x40, table_get_entry(ca_res_get(CA_STRINGS), 2));
    ch[0] = '%';
    font_draw_text(0x7380, 0x100, 0x480, ch);
    if (ca.permille > 999) {
        list = font_text_clear();
        font_set_size(12, 14);
        return ca_draw_number(list, 0x1CC, 4, 100, 9);
    } else {
        ch[0] = '.';
        font_draw_text(0x6FC0, 0x100, 0x840, ch);
        list = font_text_clear();
        font_set_size(12, 14);
        list = ca_draw_number(list, 0x1BF, 4, ca.permille / 10 % 100, 9);
        return ca_draw_number(list, 0x1CC, 4, ca.permille % 10, 9);
    }
}

/* skip menu (animated) with "page/pages", or the label of the extra pages */
unsigned int *ca_draw_footer(unsigned int *list)
{
    void *ehf = ca_res_get(CA_SKIPMENU);
    CaPage *p;
    int w;
    unsigned int opts[24] __attribute__((aligned(16)));  /* 16-byte aligned like a VFPU block */

    if (ehf) {
        memset(opts, 0, sizeof(opts));
        opts[0] = gfx_get_state()->frame;
        list = gfx_begin_2d(list);
        list = ehf_draw(list, ehf, opts);
        list = gfx_end_2d(list);
    }
    font_set_size(12, 14);
    font_set_color(0xFFFFFFFF);
    p = ca_cur_page();
    if (p) {
        if (p->page >= CA_PAGES(ca.n_cards)) {
            w = font_measure_text(table_get_entry(ca_res_get(CA_STRINGS), 4));
            font_text_set(list);
            font_draw_text(0x4E40 - w / 2, 0x3DC0, 0xF00, table_get_entry(ca_res_get(CA_STRINGS), 4));
            list = font_text_clear();
        } else {
            unsigned short slash[2] = { '/', 0 };
            list = ca_draw_number(list, 0x135, 0xF7, (p->page + 1) % 1000, 9);
            list = ca_draw_number(list, 0x157, 0xF7, CA_PAGES(ca.n_cards) % 1000, 9);
            font_text_set(list);
            font_draw_text(0x4D80, 0x3DC0, 0x2A80, slash);
            list = font_text_clear();
        }
    }
    return list;
}

/* n right-aligned with its last digit at x, digits `step` pixels apart */
unsigned int *ca_draw_number(unsigned int *list, int x, short y, int n, int step)
{
    unsigned short ch[2] = { '0', 0 };
    unsigned int cx = (unsigned short)x;

    font_text_set(list);
    do {
        cx = (unsigned short)(cx - step);
        ch[0] = n % 10 + '0';
        font_draw_text(cx * 64, y * 64, (480 - cx) * 64, ch);
        n /= 10;
    } while (n > 0);
    return font_text_clear();
}
