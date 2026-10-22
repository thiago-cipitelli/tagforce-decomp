// @range 0x088042F4-0x08805B0C
// @asm 0x088042F4-0x088046E8
/* Drawing of the screen: animated background (bg.ehf), title bar, the two tabs, the lesson rows
   (number + title in a frame), the cursor box and the scroll bar with its blinking arrows. */
#include "libtutoriallist.h"
#include "allegrex.h"

typedef struct GfxState {
    unsigned int frame;        /* frame counter */
    int fps;                   /* +0x04 30 or 60 */
} GfxState;

GfxState *gfx_get_state(void);
unsigned int *gfx_begin_2d(unsigned int *list);
unsigned int *gfx_end_2d(unsigned int *list);
unsigned int *ehf_draw(unsigned int *list, void *ehf, void *opts);
unsigned int *gim_set_texture(unsigned int *list, void *gim);
unsigned int *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(unsigned int *list);
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
unsigned int *dl_scissor(unsigned int *list, int x, int y, int w, int h);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);

/* lesson at row i of a tab, TL_NONE past the end */
#define TL_ENTRY(tab, i) ((i) >= tl.count[tab] ? TL_NONE : tl.list[tab][i])

#ifdef NON_MATCHING /* o original mantém a cópia morta da primeira instrução do bloco destino depois de cada `b` das três buscas na lista (bnezl; b; li 24; <cópia>), que o mwcc atual remove; o resto da forma é igual */
/* 0x088042f4: export, once per frame */
void tlist_draw(void)
{
    unsigned int *list;
    int i;
    int y;
    int total;
    int above_top;
    int above_cursor;
    int id;

    list = dl_select_layer(0, 0);
    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (tl.state != 4 && tl.state != 0) {
        list = dl_select_layer(4, 0);
        list = dl_enable(list, 9);
        list = dl_enable(list, 4);
        list = dl_tex_func(list, 0, 1);
        list = dl_set_tex_filter(list, 0, 0);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_disable(list, 1);
        list = tlist_draw_bg(list);
        list = tlist_draw_header(list);
        list = tlist_draw_tabs(list, 75, 39, 331, 219);
        total = 0;
        above_top = 0;
        above_cursor = 0;
        for (i = 0; i < tl.count[tl.tab]; i++) {
            y = tl.lines[TL_ENTRY(tl.tab, i)];
            total += y;
            if (i < tl.top[tl.tab] + tl.cursor[tl.tab])
                above_cursor += y;
            if (i < tl.top[tl.tab])
                above_top += y;
        }
        list = tlist_draw_scrollbar(list, 394, 66, 179, above_top, TL_ROWS, total);
        i = 0;
        y = 0;
        do {
            if (tl.top[tl.tab] + i >= tl.count[tl.tab])
                break;
            id = TL_ENTRY(tl.tab, tl.top[tl.tab] + i);
            list = tlist_draw_entry(list, 81, y * 24 + 60, id);
            y += tl.lines[id];
            i++;
        } while (y < TL_ROWS);
        if (tl.count[tl.tab] > 0) {
            list = tlist_draw_cursor(list, 110, (above_cursor - above_top) * 24 + 62, 279,
                                     tl.lines[TL_ENTRY(tl.tab, tl.top[tl.tab] + tl.cursor[tl.tab])] * 24 - 5);
        }
        dl_set_list(list);
        list = dl_select_layer(5, 0);
        list = dl_disable(list, 1);
        fade_apply((void **)&list);
        dl_set_list(list);
    }
}
#endif

unsigned int *tlist_draw_bg(unsigned int *list)
{
    void *ehf = tlres_get(0x1011);
    unsigned int opts[24];

    if (ehf == 0)
        return list;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()->frame;
    list = gfx_begin_2d(list);
    list = ehf_draw(list, ehf, opts);
    list = gfx_end_2d(list);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    return list;
}

unsigned int *tlist_draw_header(unsigned int *list)
{
    void *parts = tlres_get(0x1003);

    if (parts == 0)
        return list;
    list = gim_set_texture(list, parts);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 12, 23, 12, 23);
    list = draw_sprite(list, 0, 0, 0, 0, 20, 0, 0);
    list = draw_sprite(list, 468, 0, 0, 13, 20, 0, 0);
    list = draw_set_offsets(list, 1, 23, 456, 23);
    list = draw_sprite(list, 12, 0, 0, 12, 20, 0, 0);
    font_set_size(14, 14);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    font_draw_text(0x200, 0x100, 0x7600, table_get_entry(tlres_get(0x1020), 1));
    list = font_text_clear();
    return list;
}

/* the list window with the two tabs on top; the current tab is lit */
unsigned int *tlist_draw_tabs(unsigned int *list, int x, int y, int w, int h)
{
    void *tab = tlres_get(0x1002);
    void *parts = tlres_get(0x1003);
    int y0;
    int v;
    int y1;
    int x1;
    int u0;
    int dy;
    int u1;
    int i;
    int tw;
    int icon_v;

    if (tab == 0)
        return list;
    list = gim_set_texture(list, tab);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    v = tl.tab * 32;
    y0 = y + 15;
    list = draw_set_offsets(list, 6, 6, 6, 6);
    list = draw_sprite(list, x, y0, 0, 64, v, 0, 0);
    y1 = y + h - 6;
    list = draw_sprite(list, x, y1, 0, 64, v + 11, 0, 0);
    list = draw_set_offsets(list, 14, 6, 14, 6);
    x1 = x + w - 14;
    list = draw_sprite(list, x1, y0, 0, 98, v, 0, 0);
    list = draw_sprite(list, x1, y1, 0, 98, v + 26, 0, 0);
    list = draw_set_offsets(list, 1, 6, w - 20, 6);
    list = draw_sprite(list, x + 6, y0, 0, 70, v, 0, 0);
    list = draw_sprite(list, x + 6, y1, 0, 70, v + 11, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, h - 27);
    list = draw_sprite(list, x, y0 + 6, 0, 64, v + 6, 0, 0);
    list = draw_set_offsets(list, 14, 1, 14, h - 27);
    list = draw_sprite(list, x1, y0 + 6, 0, 98, v + 6, 0, 0);
    list = draw_set_offsets(list, 1, 1, w - 20, h - 27);
    list = draw_sprite(list, x + 6, y0 + 6, 0, 70, v + 6, 0, 0);
    font_set_size(13, 13);
    i = 0;
    v = 0;
    icon_v = 0;
    do {
        tw = w / (2 - i);
        dy = 0;
        w -= tw;
        if (i == 0)
            u0 = 0;
        else
            u0 = 16;
        u1 = i == 1 ? 48 : 32;
        if (i == tl.tab) {
            list = dl_ambient_color(list, 0xFFFFFFFF);
            font_set_color(0xFFFFFFFF);
        } else {
            u0 += 8;
            u1 += 8;
            dy = 2;
            list = dl_ambient_color(list, 0x80FFFFFF);
            font_set_color(0xFF808080);
        }
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, tab);
        list = draw_set_offsets(list, 8, 18, 8, 18);
        list = draw_sprite(list, x, y, 0, u0, v, 0, 0);
        list = draw_sprite(list, x + tw - 8, y, 0, u1, v, 0, 0);
        list = draw_set_offsets(list, 1, 18, tw - 16, 18);
        list = draw_sprite(list, x + 8, y, 0, u0 + 4, v, 0, 0);
        if (i != tl.tab) {
            list = dl_scissor(list, x, y + dy, tw - 8, 13);
            list = dl_enable(list, 2);
        }
        if (parts) {
            list = gim_set_texture(list, parts);
            list = draw_set_offsets(list, 16, 16, 16, 16);
            list = draw_sprite(list, x + 4, y + 2 + dy, 0, icon_v, 0, 0, 0);
        }
        font_text_set(list);
        font_draw_text((x + 20) * 64, (y + 4 + dy) * 64, (tw - 8) * 64, table_get_entry(tlres_get(0x1020), i + 2));
        list = font_text_clear();
        list = dl_disable(list, 2);
        i++;
        x += tw;
        icon_v += 16;
        v += 32;
    } while (i < 2);
    return list;
}

/* one lesson row: number badge, frame and title */
unsigned int *tlist_draw_entry(unsigned int *list, int x, int y, int id)
{
    void *parts = tlres_get(0x1003);
    int lines;

    if (parts == 0)
        return list;
    list = gim_set_texture(list, parts);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 25, 21, 25, 21);
    list = draw_sprite(list, x, y + 1, 0, 0, 43, 0, 0);
    lines = tl.lines[id];
    list = tlist_draw_frame(list, x + 27, y, 283, lines * 24 - 1);
    if (cfg_get_language() == 0)
        font_set_line_extra(0x280);
    else
        font_set_line_extra(0x180);
    font_text_set(list);
    font_set_size(12, 14);
    font_set_color(0xFFFFFFFF);
    tlist_draw_number(x + 4, y + 4, (id >= 8 ? id - 8 : id) + 1);
    font_set_size(14, 14);
    font_set_color(0xFF000000);
    font_draw_wtext_ex((x + 32) * 64, (y + 5) * 64, 0x4400, (lines * 24 - 7) * 64,
                       table_get_entry(tlres_get(0x1020), id + 4), -1);
    font_set_line_extra(0);
    list = font_text_clear();
    return list;
}

/* 9-slice frame around a title */
unsigned int *tlist_draw_frame(unsigned int *list, int x, int y, int w, int h)
{
    void *parts = tlres_get(0x1003);
    int y1;
    int x1;

    if (parts == 0)
        return list;
    list = gim_set_texture(list, parts);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 6, 6, 6, 6);
    list = draw_sprite(list, x, y, 0, 25, 47, 0, 0);
    y1 = y + h - 6;
    list = draw_sprite(list, x, y1, 0, 25, 58, 0, 0);
    list = draw_set_offsets(list, 6, 6, 6, 6);
    x1 = x + w - 6;
    list = draw_sprite(list, x1, y, 0, 36, 47, 0, 0);
    list = draw_sprite(list, x1, y1, 0, 36, 58, 0, 0);
    list = draw_set_offsets(list, 1, 6, w - 12, 6);
    list = draw_sprite(list, x + 6, y, 0, 31, 47, 0, 0);
    list = draw_sprite(list, x + 6, y1, 0, 31, 58, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, h - 12);
    list = draw_sprite(list, x, y + 6, 0, 25, 53, 0, 0);
    list = draw_sprite(list, x1, y + 6, 0, 36, 53, 0, 0);
    list = draw_set_offsets(list, 1, 1, w - 12, h - 12);
    list = draw_sprite(list, x + 6, y + 6, 0, 31, 53, 0, 0);
    return list;
}

/* cursor box; pulses by 2 pixels every 8 frames */
unsigned int *tlist_draw_cursor(unsigned int *list, int x, int y, int w, int h)
{
    void *parts = tlres_get(0x1003);
    int grow;
    int d;
    int y0;
    int x0;
    int y1;
    int x1;
    int ym;

    if (parts == 0)
        return list;
    grow = ((gfx_get_state()->frame >> 3) & 1) * 2;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, parts);
    list = draw_set_offsets(list, 6, 6, 6, 6);
    d = grow / 2;
    y0 = y - 4 - d;
    x0 = x - 4 - d;
    list = draw_sprite(list, x0, y0, 0, 25, 20, 0, 0);
    x1 = x + w - 2 + d;
    list = draw_sprite(list, x1, y0, 0, 40, 20, 0, 0);
    y1 = y + h - 2 + d;
    list = draw_sprite(list, x0, y1, 0, 25, 41, 0, 0);
    list = draw_sprite(list, x1, y1, 0, 40, 41, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, h + grow - 4);
    ym = y + 2 - d;
    list = draw_sprite(list, x0, ym, 0, 25, 26, 0, 0);
    list = draw_sprite(list, x1, ym, 0, 40, 26, 0, 0);
    list = draw_set_offsets(list, 1, 6, w + grow - 4, 6);
    d = x + 2 - d;
    list = draw_sprite(list, d, y0, 0, 31, 20, 0, 0);
    list = draw_sprite(list, d, y1, 0, 31, 41, 0, 0);
    return list;
}

/* scroll arrow (down = 1 for the lower one), blinking once per `period` frames */
unsigned int *tlist_draw_arrow(unsigned int *list, int x, int y, unsigned char down, unsigned int period,
                               int blink, int base, unsigned int time, void *gim)
{
    unsigned int half = period >> 1;
    int v;
    float a;

    if (down == 1)
        v = 8;
    else
        v = 0;
    list = gim_set_texture(list, gim);
    list = dl_set_tex_filter(list, 0, 0);
    if (base) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = draw_set_offsets(list, 8, 8, 8, 8);
        list = draw_sprite(list, x, y, 0, 46, v + 48, 0, 0);
    }
    if (!blink)
        return list;
    time %= period;
    if (time < half)
        a = (float)time / (float)half;
    else
        a = (float)(period - time) / (float)half;
    if (a < 0.0f)
        a = 0.0f;
    if (a > 1.0f)
        a = 1.0f;
    list = dl_ambient_color(list, ((unsigned int)(255.0f * a) << 24) | 0xFFFFFF);
    list = draw_set_offsets(list, 8, 8, 8, 8);
    return draw_sprite(list, x, y, 0, 46, v, 0, 0);
}

/* scroll bar: knob size and position from the rows above the window, the window and the total */
unsigned int *tlist_draw_scrollbar(unsigned int *list, int x, int y, int h, int pos, int page, int total)
{
    int track = h - 2;         /* becomes the knob offset */
    void *parts = tlres_get(0x1003);
    int knob;
    int period_up;             /* blink period of each arrow (one second) */
    int period_down;

    if (parts == 0)
        return list;
    if (total <= page) {
        knob = track;
        track = 0;
    } else {
        knob = ALLEGREX_MAX(page * track / total, 10);
        /* `h - 2 - knob`, not `track - knob`: the original recomputes it (move; subu) */
        track = ALLEGREX_MIN(ALLEGREX_MAX(pos * (track - knob) / (total - page), 0), h - 2 - knob);
    }
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, parts);
    list = draw_set_offsets(list, 8, 5, 8, 5);
    list = draw_sprite(list, x, y, 0, 55, 0, 0, 0);
    list = draw_sprite(list, x, y + h - 5, 0, 55, 15, 0, 0);
    list = draw_set_offsets(list, 8, 5, 8, h - 10);
    list = draw_sprite(list, x, y + 5, 0, 55, 5, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, 5);
    list = draw_sprite(list, x + 2, y + track + 2, 0, 55, 40, 0, 0);
    list = draw_sprite(list, x + 2, knob + (y + track - 3), 0, 55, 55, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, knob - 10);
    list = draw_sprite(list, x + 2, y + track + 7, 0, 55, 45, 0, 0);
    period_up = gfx_get_state()->fps;
    list = tlist_draw_arrow(list, x, y - 7, 0, period_up, pos > 0, 1, gfx_get_state()->frame, parts);
    period_down = gfx_get_state()->fps;
    return tlist_draw_arrow(list, x, y + h + 2, 1, period_down, pos < total - page, 1, gfx_get_state()->frame,
                            parts);
}

/* lesson number, right-aligned, drawn digit by digit from the right */
void tlist_draw_number(int x, int y, int n)
{
    unsigned short ch[2] = {0};

    if (n < 10)
        x += 4;
    else
        x += 8;
    while (n > 0) {
        ch[0] = n % 10 + '0';
        font_draw_text(x * 64, y * 64, (480 - x) * 64, ch);
        n /= 10;
        x -= 8;
    }
}
