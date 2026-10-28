// @range 0x08822898-0x08824028
// @asm 0x088234E4-0x08823764
// @asm 0x08823764-0x08823AC4
// @data 0x0883D504-0x0883D530
/* Pieces of the PDA pages: calendar cells and their marks, the date text of a mail, the scroll
   bar and its arrows, a mail of the list and its header, the gauge of a mail and the input of the top
   bar (L/R change the page). */
#include "libfield.h"
#include "allegrex.h"

unsigned char func_08825530(unsigned char month, unsigned char day);
void func_0882435C(void);
void func_088255E4(void);
void func_088257A0(void);

unsigned short pda_mail_faces[14] = { 2, 3, 4, 5, 6, 8, 7, 10, 9, 0x27 };  /* senders with a face icon */
unsigned short pda_tab_states[8] = { 8, 4, 6, 5, 10 };                     /* state of each page */

extern unsigned short pda_month_names[8];      /* 0x0883D4D0 */
extern unsigned short pda_wday_names[8];       /* 0x0883D4E0 */
extern unsigned short pda_wday_names_en[8];    /* 0x0883D4F0 */

/* calendar cell: pos packs x | y << 16; day 0 = empty cell */
void *pda_draw_day(void *list, int pos, int weekday, int today, int day)
{
    int x = pos & 0xFFFF;
    int y = pos >> 16 & 0xFFFF;
    unsigned int color = 0xFFFFFFFF;
    int u;
    int v;

    if (field_pda->gim == 0)
        return list;
    list = gim_set_texture(list, field_pda->gim);
    list = dl_set_tex_filter(list, 0, 0);
    u = 0x67;
    v = 0x4B;
    if (weekday)
        u += 0x1C;
    if (day <= 0)
        color = 0x80FFFFFF;
    if (today)
        v += 0x1A;
    list = dl_ambient_color(list, color);
    list = draw_set_offsets(list, 0x1C, 0x1A, 0x1C, 0x1A);
    list = draw_sprite(list, x, y, 0, u, v, 0, 0);
    if (day <= 0)
        return list;
    if (today)
        list = dl_ambient_color(list, weekday == 0 ? 0xFF00FFFF : 0xFF00FF9E);
    u = (day - 1) % 8 * 0x12 + 0x70;
    v = (day - 1) / 8 * 0x0E + 200;
    list = draw_set_offsets(list, 0x12, 0x0E, 0x12, 0x0E);
    return draw_sprite(list, x, y + 1, 0, u, v, 0, 0);
}

/* event mark of a calendar cell (0 none) */
void *pda_draw_mark(void *list, int pos, int mark)
{
    int x = pos & 0xFFFF;
    int y = pos >> 16 & 0xFFFF;

    if (field_pda->gim == 0)
        return list;
    if (mark == 0)
        return list;
    list = gim_set_texture(list, field_pda->gim);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = draw_set_offsets(list, 0x17, 0x14, 0x17, 0x14);
    return draw_sprite(list, x, y, 0, (mark - 1) * 0x17 + 0x9F, 0x51, 0, 0);
}

/* "<month> <day>(<weekday>)" in the language of the game */
void pda_date_text(unsigned short *buf, int month, int day, int wday)
{
    int n;

    switch (cfg_get_language()) {
    case 0:
        wcscpy(buf, table_get_entry(fld_gim_get(5), pda_month_names[month]));
        n = wcslen(buf);
        if (day >= 10)
            buf[n++] = day / 10 + 0xFF10;
        buf[n++] = day % 10 + 0xFF10;
        buf[n++] = 0;
        wcscat(buf, table_get_entry(fld_gim_get(5), 0x57));
        n = wcslen(buf);
        buf[n++] = '(';
        buf[n++] = 0;
        wcscat(buf, table_get_entry(fld_gim_get(5), pda_wday_names[wday]));
        n = wcslen(buf);
        buf[n++] = ')';
        buf[n++] = 0;
        break;
    case 1:
        n = 0;
        if (day >= 10)
            buf[n++] = day / 10 + '0';
        buf[n++] = day % 10 + '0';
        switch (day % 10) {
        case 1:
            buf[n++] = 's';
            buf[n++] = 't';
            break;
        case 2:
            buf[n++] = 'n';
            buf[n++] = 'd';
            break;
        case 3:
            buf[n++] = 'r';
            buf[n++] = 'd';
            break;
        default:
            buf[n++] = 't';
            buf[n++] = 'h';
            break;
        }
        buf[n++] = ' ';
        buf[n++] = 0;
        wcscat(buf, table_get_entry(fld_gim_get(5), pda_month_names[month]));
        n = wcslen(buf);
        buf[n++] = ' ';
        buf[n++] = '-';
        buf[n++] = ' ';
        buf[n++] = 0;
        wcscat(buf, table_get_entry(fld_gim_get(5), pda_wday_names_en[wday]));
        break;
    default:
        n = 0;
        if (day >= 10)
            buf[n++] = day / 10 + '0';
        buf[n++] = day % 10 + '0';
        buf[n++] = ' ';
        buf[n++] = 0;
        wcscat(buf, table_get_entry(fld_gim_get(5), pda_month_names[month]));
        n = wcslen(buf);
        buf[n++] = ' ';
        buf[n++] = '-';
        buf[n++] = ' ';
        buf[n++] = 0;
        wcscat(buf, table_get_entry(fld_gim_get(5), pda_wday_names_en[wday]));
        break;
    }
}

/* arrow over or under the scroll bar, pulsing while there is more to scroll that way */
void *pda_draw_arrow(void *list, int x, int y, int up, int blink)
{
    unsigned int period = gfx_get_state()[1];
    unsigned int t = gfx_get_state()[0];
    unsigned int half = period >> 1;
    int u;
    int v;
    unsigned int alpha;
    float f;

    if (field_pda->gim == 0)
        return list;
    if (up) {
        u = 0x47;
        v = 0x68;
    } else {
        u = 0x47;
        v = 0x87;
    }
    list = gim_set_texture(list, field_pda->gim);
    list = dl_set_tex_filter(list, 0, 0);
    if (blink) {
        t %= period;
        if (t < half)
            f = (float)t / (float)half;
        else
            f = (float)(period - t) / (float)half;
        if (f < 0.0f)
            f = 0.0f;
        if (f > 1.0f)
            f = 1.0f;
        alpha = 255.0f * f;
    } else {
        alpha = 0xFF;
    }
    list = dl_ambient_color(list, alpha << 24 | 0xFFFFFF);
    list = draw_set_offsets(list, 0x0B, 0x0A, 0x0B, 0x0A);
    return draw_sprite(list, x, y, 0, u, v, 0, 0);
}

/* vertical scroll bar of h pixels: shown of total entries, first at pos */
void *pda_draw_scrollbar(void *list, int x, int y, int h, int pos, int shown, int total)
{
    void *gim = field_pda->gim;
    int range = h - 2;
    int len;
    int at;

    if (gim == 0)
        return list;
    if (total <= shown) {
        len = range;
        at = 1;
    } else {
        len = ALLEGREX_MAX(shown * range / total, 11);
        range -= len;
        at = ALLEGREX_MIN(ALLEGREX_MAX(pos * range / (total - shown), 0), range) + 1;
    }
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 0x0B, 5, 0x0B, 5);
    list = draw_sprite(list, x, y, 0, 0x47, 0x72, 0, 0);
    list = draw_sprite(list, x, y + h - 5, 0, 0x47, 0x82, 0, 0);
    list = draw_set_offsets(list, 0x0B, 5, 0x0B, h - 10);
    list = draw_sprite(list, x, y + 5, 0, 0x47, 0x77, 0, 0);
    list = draw_set_offsets(list, 0x0B, 7, 0x0B, 7);
    list = draw_sprite(list, x, len + (y + at) - 7, 0, 0x57, 0x8A, 0, 0);
    list = draw_sprite(list, x, y + at, 0, 0x57, 0x68, 0, 0);
    if (len - 14 > 0) {
        list = draw_set_offsets(list, 0x0B, 7, 0x0B, len - 14);
        list = draw_sprite(list, x, y + at + 7, 0, 0x57, 0x6F, 0, 0);
    }
    list = pda_draw_arrow(list, x, y - 11, 1, pos > 0);
    return pda_draw_arrow(list, x, y + h + 1, 0, pos < total - shown);
}

void *pda_draw_mail_head(void *list, int x, int y, int w, FieldPdaMail *m, int sel);

/* a mail of the list: frame, sender face, unread and important marks, header */
#ifdef NON_MATCHING /* registrador do índice da face (t0 no original, t1 aqui); faltam 13 palavras */
void *pda_draw_mail_item(void *list, int x, int y, FieldPdaMail *m, int sel)
{
    int i;
    int face;

    if (field_pda->gim == 0)
        return list;
    list = gim_set_texture(list, field_pda->gim);
    if (m == 0)
        list = dl_ambient_color(list, 0xFF808080);
    else
        list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = pda_draw_frame(list, x, y, 0xCB, 0x31, 0x580057);
    list = pda_draw_frame(list, x + 4, y + 4, 0x23, 0x23, 0x580057);
    if (m) {
        list = draw_set_offsets(list, 0x1C, 0x1C, 0x1C, 0x1C);
        face = 0;
        for (i = 0; i < 10; i++) {
            if (m->chara == pda_mail_faces[i]) {
                face = i;
                break;
            }
        }
        list = draw_sprite(list, x + 7, y + 7, 0, face / 9 * 0x1C + 0x1C8, face % 9 * 0x1C, 0, 0);
        if (m->unread && (*gfx_get_state() >> 3 & 1)) {
            list = draw_set_offsets(list, 0x16, 0x0A, 0x16, 0x0A);
            list = draw_sprite(list, x + 5, y + 0x23, 0, 0xD7, 0x10, 0, 0);
        }
        face = 199;
        if (m->important)
            face += 17;
        list = draw_set_offsets(list, 0x11, 0x0C, 0x11, 0x0C);
        list = draw_sprite(list, x + 0x19, y + 0x21, 0, face, 0x21, 0, 0);
    }
    return pda_draw_mail_head(list, x + 0x28, y + 4, 0x9E, m, sel);
}
#endif

/* header of a mail: subject, sender and date */
#ifdef NON_MATCHING /* permutação dos registradores salvos no laço dos rótulos e no fim; faltam 29 palavras */
void *pda_draw_mail_head(void *list, int x, int y, int w, FieldPdaMail *m, int sel)
{
    unsigned short *labels[3];
    unsigned short date[512];
    int u;
    int i;
    int tx;
    int ty;
    int lx;
    int lw;
    int month;
    int day;
    int wday;

    if (field_pda->gim == 0)
        return list;
    list = gim_set_texture(list, field_pda->gim);
    if (m == 0)
        list = dl_ambient_color(list, 0xFF808080);
    else
        list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    u = 0x47;
    if (sel)
        u += 0x10;
    list = draw_set_offsets(list, 2, 0x28, 2, 0x28);
    list = draw_sprite(list, x, y, 0, u, 0x30, 0, 0);
    list = draw_sprite(list, x + w - 2, y, 0, u + 0x0E, 0x30, 0, 0);
    list = draw_set_offsets(list, 2, 0x28, w - 4, 0x28);
    list = draw_sprite(list, x + 2, y, 0, u + 2, 0x30, 0, 0);
    if (m == 0)
        return list;
    font_set_size(0x0C, 0x0C);
    font_text_set(list);
    if (sel)
        font_set_color(0xFFFFFFFF);
    else
        font_set_color(0xFF492B02);
    labels[0] = table_get_entry(fld_gim_get(5), 0x5E);
    labels[1] = table_get_entry(fld_gim_get(5), 0x5F);
    labels[2] = table_get_entry(fld_gim_get(5), 0x60);
    tx = (x + 2) << 6;
    for (i = 0; i < 3; i++) {
        ty = y + 1 + i * 13;
        font_draw_wtext(tx, ty << 6, field_pda->label_w, labels[i]);
        lx = tx + field_pda->label_w;
        font_draw_text(lx, ty << 6, 0x400, table_get_entry(fld_gim_get(5), 0x61));
    }
    lw = field_pda->label_w + font_measure_text(table_get_entry(fld_gim_get(5), 0x61));
    ty = (y + 1) << 6;
    lx = tx + lw;
    month = m->month - 5;
    lw = ((w - 2) << 6) - lw;
    day = m->day;
    wday = func_08825530(month, day) % 7;
    font_draw_wtext(lx, ty, lw, m->title);
    font_draw_wtext(lx, ty + 0x340, lw, chara_get_name(0, m->chara));
    pda_date_text(date, month, day, wday);
    font_draw_wtext(lx, ty + 0x680, lw, date);
    return font_text_clear();
}
#endif

/* gauge of value out of max */
void *pda_draw_bar(void *list, int x, int y, int value, int max)
{
    int w;

    if (field_pda->gim == 0)
        return list;
    list = gim_set_texture(list, field_pda->gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 5, 7, 5, 7);
    list = draw_sprite(list, x, y, 0, 0x9F, 0x3D, 0, 0);
    list = draw_sprite(list, x + 0x6E, y, 0, 0xAA, 0x3D, 0, 0);
    list = draw_set_offsets(list, 4, 7, 0x69, 7);
    list = draw_sprite(list, x + 5, y, 0, 0xA4, 0x3D, 0, 0);
    w = value * 0x65 / max;
    list = draw_set_offsets(list, 1, 7, w, 7);
    list = draw_sprite(list, x + 2, y, 0, 0xAF, 0x3D, 0, 0);
    list = draw_set_offsets(list, 0x0C, 0x0C, 0x0C, 0x0C);
    return draw_sprite(list, x + 2 + w, y - 2, 0, 0x9F, 0x44, 0, 0);
}

/* covers the open mail around the selected text (the window is 0x9E x 0x54) */
void *pda_draw_mail_mask(void *list, int x, int y)
{
    int top;
    int left;
    int h;

    if (field_pda->gim == 0)
        return list;
    list = gim_set_texture(list, field_pda->gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    left = (field_pda->sel_x + 0x20) >> 6;
    h = ((field_pda->sel_y + field_pda->sel_h + 0x20) >> 6) - ((field_pda->sel_y + 0x20) >> 6);
    top = y + ((field_pda->sel_y + 0x20) >> 6);
    list = draw_set_offsets(list, 1, 1, 0x9E - left, h);
    list = draw_sprite(list, x + left, top, 0, 0x4F, 0x60, 0, 0);
    top += h;
    list = draw_set_offsets(list, 1, 1, 0x9E, y + 0x54 - top);
    return draw_sprite(list, x, top, 0, 0x4F, 0x60, 0, 0);
}

/* top bar: cancel leaves, confirm opens the page after a short wait, L/R pick the page */
int pda_input_menu(void)
{
    PadState *pad = pad_get();

    if (field_pda->leaving) {
        if (field_pda->wait++ >= 12) {
            field_pda->leaving = 0;
            field_pda->wait = 0;
            field_pda->cursor = 0;
            switch (field_pda->tab) {
            case 3:
                field_pda->month = field_pda->today_month;
                field_pda->day = func_08825530(field_pda->today_month, field_pda->today);
                break;
            case 5:
                func_088257A0();
                se_play(0x621A);
                return 0;
            case 4:
                func_0882435C();
                func_088255E4();
                break;
            }
            field_pda->state = pda_tab_states[field_pda->tab];
        }
        return 0;
    }
    if (pad->b.trig & (region_pick_value(1) | 0x8000)) {
        field_pda->state = 11;
        se_play(0x6263);
        return 0;
    }
    if (pad->b.trig & region_pick_value(0)) {
        field_pda->leaving = 1;
        field_pda->wait = 0;
        se_play(0x6213);
        return 0;
    }
    if (pad->b.rep & 0x10) {
        field_pda->tab = (field_pda->tab + 5) % 6;
        se_play(0x6212);
    }
    if (pad->b.rep & 0x40) {
        field_pda->tab = (field_pda->tab + 1) % 6;
        se_play(0x6212);
    }
    return 0;
}
