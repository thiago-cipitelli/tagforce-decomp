// @range 0x08804690-0x088060A4
// @asm 0x08804690-0x088049DC
// @data 0x0880A5CC-0x0880A5D4
/* Drawing of the duel record screen: title bar with the completion percentage, tab strip, list
   lines (number, text, value column, "new" mark) and the scroll bar with its blinking arrows. */
#include "libc.h"
#include "allegrex.h"
#include "font.h"
#include "libduelrecord.h"

void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);

void *drec_clear_bg(void *list);
void *drec_draw_header(void *list);
void *drec_draw_list_frame(void *list, int x, int y, int w, int h);
void *drec_draw_entry(void *list, int x, int y, DrecEntry *e);
void *drec_draw_new_mark(void *list, int x, int y, int w, int h);
void drec_draw_number(int x, int y, int value, unsigned short digits, unsigned short step, int pad);
void *drec_draw_value(void *list, int x, int y, DrecEntry *e);
void *drec_draw_bar_frame(void *list, int x, int y, unsigned char down, unsigned int period, int blink,
                          int show, unsigned int t, void *gim);
void *drec_draw_scrollbar(void *list, int x, int y, int h, int pos, int page, int total);

#ifdef NON_MATCHING /* forma igual (mesmo inline de drec_tab_rows); o original deixa uma cópia morta do sll no alvo dos dois bnezl do inline (8 bytes a mais, que deslocam o resto) */
void duelrecord_draw(void)
{
    void *list;
    int i;
    int total;
    int above;
    int y;
    DrecEntry *e;
    unsigned char tab;
    int n;

    list = dl_select_layer(0, 0);
    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (drec.state != 4 && drec.state != 0) {
        list = dl_select_layer(4, 0);
        list = dl_enable(list, 9);
        list = dl_enable(list, 4);
        list = dl_tex_func(list, 0, 1);
        list = dl_set_tex_filter(list, 0, 0);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_disable(list, 1);
        list = drec_clear_bg(list);
        list = drec_draw_header(list);
        list = drec_draw_list_frame(list, 0, 0x17, 0x1E0, 0xF9);

        total = 0;
        above = 0;
        for (i = 0; i < drec_tab_rows(drec.tab); i++) {
            tab = drec.tab;
            e = drec_entry(tab, i);
            if (e != 0) {
                n = e->lines;
                total += n;
                if (i < drec.scroll[tab])
                    above += n;
            }
        }
        list = drec_draw_scrollbar(list, 0x1D4, 0x32, 0xD1, above, 9, total);

        list = dl_scissor(list, 0, 0x29, 0x1E0, 0xE3);
        list = dl_enable(list, 2);
        i = 0;
        y = 0;
        do {
            tab = drec.tab;
            n = drec.scroll[tab] + i;
            if (n >= drec.count[tab])
                break;
            e = drec_entry(tab, n);
            if (e != 0) {
                list = drec_draw_entry(list, 4, y * 24 + 0x30, e);
                y += e->lines;
            }
            i++;
        } while (y < 9);
        list = dl_disable(list, 2);
        dl_set_list(list);

        list = dl_select_layer(5, 0);
        list = dl_disable(list, 1);
        fade_apply(&list);
        dl_set_list(list);
    }
}

#endif

void *drec_clear_bg(void *list)
{
    list = dl_clear_color(list, 0xFF603B60);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    return dl_clear(list, 0x17);
}

void *drec_draw_header(void *list)
{
    void *gim;
    int w;
    int x;
    int end;

    gim = drec_get_file(0x1003);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0xC, 0x17, 0xC, 0x17);
    list = draw_sprite(list, 0, 0, 0, 0, 0x14, 0, 0);
    list = draw_sprite(list, 0x1D4, 0, 0, 0xD, 0x14, 0, 0);
    list = draw_set_offsets(list, 1, 0x17, 0x1C8, 0x17);
    list = draw_sprite(list, 0xC, 0, 0, 0xC, 0x14, 0, 0);

    font_set_size(0xE, 0xE);
    w = (font_measure_text(table_get_entry(drec_get_file(0x1020), 0xFC)) >> 6) + 0x2E;
    x = 0x1DB - w;
    list = draw_set_offsets(list, 4, 0x10, 4, 0x10);
    list = draw_sprite(list, x, 3, 0, 0x40, 0, 0, 0);
    end = x + w;
    list = draw_sprite(list, end - 4, 3, 0, 0x4C, 0, 0, 0);
    list = draw_set_offsets(list, 1, 0x10, w - 8, 0x10);
    list = draw_sprite(list, x + 4, 3, 0, 0x44, 0, 0, 0);

    font_set_size(0xE, 0xE);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    font_draw_text(8 << 6, 4 << 6, 0x1D8 << 6, table_get_entry(drec_get_file(0x1020), 1));
    {
        short pct[2] = { '%', 0 };

        font_draw_text((end - 0xD) << 6, 4 << 6, (0x1E0 - (end - 0xD)) << 6, (unsigned short *)pct);
    }
    font_set_size(0xC, 0xE);
    drec_draw_number(end - 0x16, 4, drec.percent, 3, 9, 0);
    font_set_size(0xE, 0xE);
    font_draw_wtext((x + 1) << 6, 4 << 6, (w - 0x2A) << 6, table_get_entry(drec_get_file(0x1020), 0xFC));
    return font_text_clear();
}

void *drec_draw_list_frame(void *list, int x, int y, int w, int h)
{
    void *gim;
    int y0;
    int v;
    int y1;
    int x1;
    int u0;
    int t;
    int tw;
    int u1;
    int dy;
    void *dl; /* list carried across the tabs */

    gim = drec_get_file(0x1002);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    v = drec.tab * 32;
    y0 = y + 0xF;
    list = draw_set_offsets(list, 6, 6, 6, 6);
    list = draw_sprite(list, x, y0, 0, 0x40, v, 0, 0);
    y1 = y + h - 6;
    list = draw_sprite(list, x, y1, 0, 0x40, v + 0xB, 0, 0);
    list = draw_set_offsets(list, 0xE, 6, 0xE, 6);
    x1 = x + w - 0xE;
    list = draw_sprite(list, x1, y0, 0, 0x62, v, 0, 0);
    list = draw_sprite(list, x1, y1, 0, 0x62, v + 0x1A, 0, 0);
    list = draw_set_offsets(list, 1, 6, w - 0x14, 6);
    list = draw_sprite(list, x + 6, y0, 0, 0x46, v, 0, 0);
    list = draw_sprite(list, x + 6, y1, 0, 0x46, v + 0xB, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, h - 0x1B);
    list = draw_sprite(list, x, y0 + 6, 0, 0x40, v + 6, 0, 0);
    list = draw_set_offsets(list, 0xE, 1, 0xE, h - 0x1B);
    list = draw_sprite(list, x1, y0 + 6, 0, 0x62, v + 6, 0, 0);
    list = draw_set_offsets(list, 1, 1, w - 0x14, h - 0x1B);
    list = draw_sprite(list, x + 6, y0 + 6, 0, 0x46, v + 6, 0, 0);

    font_set_size(0xD, 0xD);
    dl = list;
    t = 0;
    if (drec.n_tabs > 0) {
        v = 0;
        do {
            tw = w / (drec.n_tabs - t);
            dy = 0;
            w -= tw;
            if (t == 0)
                u0 = 0;
            else
                u0 = 0x10;
            if (t == drec.n_tabs - 1)
                u1 = 0x30;
            else
                u1 = 0x20;
            if (t == drec.tab) {
                list = dl_ambient_color(dl, 0xFFFFFFFF);
                font_set_color(0xFFFFFFFF);
            } else {
                u0 += 8;
                u1 += 8;
                dy = 2;
                list = dl_ambient_color(dl, 0x80FFFFFF);
                font_set_color(0xFF808080);
            }
            list = gim_set_texture(list, gim);
            list = dl_set_tex_filter(list, 0, 0);
            list = draw_set_offsets(list, 8, 0x12, 8, 0x12);
            list = draw_sprite(list, x, y, 0, u0, v, 0, 0);
            list = draw_sprite(list, x + tw - 8, y, 0, u1, v, 0, 0);
            list = draw_set_offsets(list, 1, 0x12, tw - 0x10, 0x12);
            list = draw_sprite(list, x + 8, y, 0, u0 + 4, v, 0, 0);
            if (t != drec.tab) {
                list = dl_scissor(list, x, y + dy, tw - 8, 0xD);
                list = dl_enable(list, 2);
            }
            font_text_set(list);
            font_draw_text((x + 4) << 6, (y + 3 + dy) << 6, (tw - 8) << 6,
                           table_get_entry(drec_get_file(0x1020), t + 0x10D));
            dl = dl_disable(font_text_clear(), 2);
            x += tw;
            t++;
            v += 32;
        } while (t < drec.n_tabs);
    }
    return dl;
}

void *drec_draw_entry(void *list, int x, int y, DrecEntry *e)
{
    void *gim;
    int h;
    int w;

    gim = drec_get_file(0x1003);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0xC, 0x15, 0xC, 0x15);
    list = draw_sprite(list, x, y + 1, 0, 0, 0x2B, 0, 0);
    list = draw_sprite(list, x + 0x1C, y + 1, 0, 0xD, 0x2B, 0, 0);
    list = draw_set_offsets(list, 1, 0x15, 0x10, 0x15);
    list = draw_sprite(list, x + 0xC, y + 1, 0, 0xC, 0x2B, 0, 0);
    h = e->lines * 24;
    list = drec_draw_new_mark(list, x + 0x2A, y, 0x1A3, h - 1);
    if (cfg_get_language() == 0)
        font_set_line_extra(0x280);
    else
        font_set_line_extra(0x180);
    font_text_set(list);
    font_set_size(0xC, 0xE);
    font_set_color(0xFFFFFFFF);
    drec_draw_number(x + 0x19, y + 4, e->number + 1, 3, 8, 1);
    font_set_size(0xE, 0xE);
    font_set_color(0xFF000000);
    if (e->has_value)
        w = 0x14A;
    else
        w = 0x198;
    font_draw_wtext_ex((x + 0x2F) << 6, (y + 4) << 6, w << 6, (h - 7) << 6,
                       table_get_entry(drec_get_file(0x1020), drec_text_index(drec.tab, e->index)), -1);
    font_set_line_extra(0);
    list = font_text_clear();
    if (e->has_value)
        list = drec_draw_value(list, x + 0x17F, y + 5, e);
    if (e->is_new && (*gfx_get_state() >> 3 & 1)) {
        list = gim_set_texture(list, gim);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 0x16, 0xA, 0x16, 0xA);
        list = draw_sprite(list, x + 0x1B1, y + 2, 0, 0xD, 0, 0, 0);
    }
    return list;
}

void *drec_draw_new_mark(void *list, int x, int y, int w, int h)
{
    void *gim;
    int y1;
    int x1;

    gim = drec_get_file(0x1003);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 6, 6, 6, 6);
    list = draw_sprite(list, x, y, 0, 0x19, 0x2F, 0, 0);
    y1 = y + h - 6;
    list = draw_sprite(list, x, y1, 0, 0x19, 0x3A, 0, 0);
    list = draw_set_offsets(list, 6, 6, 6, 6);
    x1 = x + w - 6;
    list = draw_sprite(list, x1, y, 0, 0x24, 0x2F, 0, 0);
    list = draw_sprite(list, x1, y1, 0, 0x24, 0x3A, 0, 0);
    list = draw_set_offsets(list, 1, 6, w - 0xC, 6);
    list = draw_sprite(list, x + 6, y, 0, 0x1F, 0x2F, 0, 0);
    list = draw_sprite(list, x + 6, y1, 0, 0x1F, 0x3A, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, h - 0xC);
    list = draw_sprite(list, x, y + 6, 0, 0x19, 0x35, 0, 0);
    list = draw_sprite(list, x1, y + 6, 0, 0x24, 0x35, 0, 0);
    list = draw_set_offsets(list, 1, 1, w - 0xC, h - 0xC);
    list = draw_sprite(list, x + 6, y + 6, 0, 0x1F, 0x35, 0, 0);
    return list;
}

void drec_draw_number(int x, int y, int value, unsigned short digits, unsigned short step, int pad)
{
    unsigned short buf[2] = { 0 };
    int i;

    for (i = 0; i < digits; i++) {
        buf[0] = value % 10 + '0';
        font_draw_text(x << 6, y << 6, (0x1E0 - x) << 6, buf);
        x -= step;
        value /= 10;
        if (!pad && value == 0)
            return;
    }
}

void *drec_draw_value(void *list, int x, int y, DrecEntry *e)
{
    void *gim;

    gim = drec_get_file(0x1003);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0xC, 0xC, 0xC, 0xC);
    list = draw_sprite(list, x, y + 1, 0, 0, 0, 0, 0);
    list = draw_set_offsets(list, 4, 0xD, 4, 0xD);
    list = draw_sprite(list, x + 0xD, y, 0, 0x40, 0x10, 0, 0);
    list = draw_sprite(list, x + 0x43, y, 0, 0x4C, 0x10, 0, 0);
    list = draw_set_offsets(list, 1, 0xD, 0x32, 0xD);
    list = draw_sprite(list, x + 0x11, y, 0, 0x44, 0x10, 0, 0);
    font_text_set(list);
    font_set_color(0xFFFFFFFF);
    if ((int)e->value > 999999999)
        e->value = 999999999;
    if (drec.tab == 0 && e->index == 0x25) {
        /* play time: minutes:seconds */
        short colon[2] = { ':', 0 };
        int sec = (int)e->value % 60;

        font_set_size(0xC, 0xE);
        drec_draw_number(x + 0x3D, y, sec, 2, 9, 1);
        font_draw_text((x + 0x2E) << 6, y << 6, (0x1E0 - (x + 0x2E)) << 6, (unsigned short *)colon);
        drec_draw_number(x + 0x22, y, (int)e->value / 60, 2, 9, 0);
    } else if ((int)e->value > 999999) {
        font_set_size(8, 0xE);
        drec_draw_number(x + 0x40, y, e->value, 9, 6, 0);
    } else {
        font_set_size(0xC, 0xE);
        drec_draw_number(x + 0x3D, y, e->value, 6, 9, 0);
    }
    return font_text_clear();
}

void *drec_draw_bar_frame(void *list, int x, int y, unsigned char down, unsigned int period, int blink,
                          int show, unsigned int t, void *gim)
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
    if (show) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = draw_set_offsets(list, 8, 8, 8, 8);
        list = draw_sprite(list, x, y, 0, 0x2E, v + 0x30, 0, 0);
    }
    if (!blink)
        return list;
    t %= period;
    if (t < half)
        a = (float)t / (float)half;
    else
        a = (float)(period - t) / (float)half;
    if (a < 0.0f)
        a = 0.0f;
    if (a > 1.0f)
        a = 1.0f;
    list = dl_ambient_color(list, (unsigned int)(255.0f * a) << 24 | 0xFFFFFF);
    list = draw_set_offsets(list, 8, 8, 8, 8);
    list = draw_sprite(list, x, y, 0, 0x2E, v, 0, 0);
    return list;
}

void *drec_draw_scrollbar(void *list, int x, int y, int h, int pos, int page, int total)
{
    void *gim;
    int at = h - 2; /* track length, then thumb offset */
    unsigned int period_up;
    int len;  /* thumb length */
    unsigned int period_down;

    gim = drec_get_file(0x1003);
    if (gim == 0)
        return list;
    if (total <= page) {
        len = at;
        at = 0;
    } else {
        len = ALLEGREX_MAX(page * at / total, 10);
        at = ALLEGREX_MIN(ALLEGREX_MAX(pos * (at - len) / total, 0), h - 2 - len);
    }
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 8, 5, 8, 5);
    list = draw_sprite(list, x, y, 0, 0x37, 0, 0, 0);
    list = draw_sprite(list, x, y + h - 5, 0, 0x37, 0xF, 0, 0);
    list = draw_set_offsets(list, 8, 5, 8, h - 10);
    list = draw_sprite(list, x, y + 5, 0, 0x37, 5, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, 5);
    list = draw_sprite(list, x + 2, y + at + 2, 0, 0x37, 0x14, 0, 0);
    list = draw_sprite(list, x + 2, y + at - 3 + len, 0, 0x37, 0x23, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, len - 10);
    list = draw_sprite(list, x + 2, y + at + 7, 0, 0x37, 0x19, 0, 0);
    period_up = gfx_get_state()[1];
    list = drec_draw_bar_frame(list, x, y - 7, 0, period_up, pos > 0, 1, gfx_get_state()[0], gim);
    period_down = gfx_get_state()[1];
    list = drec_draw_bar_frame(list, x, y + h + 2, 1, period_down, pos < total, 1, gfx_get_state()[0],
                               gim);
    return list;
}

