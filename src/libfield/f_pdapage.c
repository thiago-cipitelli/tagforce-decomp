// @range 0x0881FFEC-0x08822898
// @asm 0x0881FFEC-0x08820A04
// @asm 0x08820BD0-0x088213DC
// @asm 0x088213DC-0x088216B0
// @asm 0x088216B0-0x08821D24
// @asm 0x088220D8-0x088223AC
// @asm 0x0882244C-0x08822690
// @data 0x0883D44C-0x0883D504
/* The pages of the PDA: the player's status (days, duels won/lost/drawn, win rate, cards), the
   options, the page titles, the calendar of the month, the mail list and an open mail; plus the
   frame, cursor, gauge and number pieces they share. */
#include "libfield.h"

unsigned char func_08824028(int row);
void *func_08825A94(int i);
void *func_08825A48(int i);
unsigned char func_088254D0(unsigned char month, unsigned char row, unsigned char col);
unsigned char func_08825584(unsigned char month, unsigned char day);
void *func_0881FBCC(void *list, int x, int y, unsigned short *title);
void *func_0882AD34(void *list, int face, int x, int y);

extern unsigned char pda_event_marks[16];      /* 0x0883D410 */

unsigned short pda_times_text[2] = { 'x', 0 };                                    /* "x" before the card count */
unsigned char pda_opt_rows[16] = { 0x12, 0x29, 0x52, 0x65, 0x78, 0xA3, 0xB6, 0xC9, 0xDC };      /* y of each option */
unsigned char pda_opt_rows_en[16] = { 0x12, 0x29, 0x52, 0x58, 0x71, 0xA3, 0xB6, 0xC9, 0xDC };
unsigned short pda_opt_names[16] = { 1, 2, 4, 5, 6, 8, 9, 0x0B, 0x12 };
unsigned short pda_speed_names[8] = { 0x0F, 0x10, 0x11 };
unsigned short pda_volume_names[8] = { 0x0E, 0x0F, 0x10, 0x11 };
unsigned short pda_tab_names[8] = { 0x4A, 0x4B };
unsigned short pda_page_titles[8] = { 0x48, 0x49 };
unsigned short pda_month_names[8] = { 0x4C, 0x4D, 0x4E, 0x4F };
unsigned short pda_wday_names[8] = { 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D };
unsigned short pda_wday_names_en[8] = { 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56 };

#define FONT_BROWN 0xFF492B02
#define PDA_XY(x, y) ((x) & 0xFFFF | ((y) & 0xFFFF) << 16)   /* packed position or uv */

/* status page: days played, duel record and win rate, and the card count when the PDA has one */
#ifdef NON_MATCHING /* regalloc */
void *pda_draw_status(void *list, int x, int y)
{
    unsigned short dot[2];
    unsigned short unit[2];
    unsigned int rec[3];
    void *gim = field_pda->gim;
    int cards = -1;
    int total;
    int w;
    int cx;
    int lx;
    int lw;
    int rx;
    int rw;
    int ty;
    int sy;
    int pct;
    int fy;

    if (gim == 0)
        return list;
    if (field_pda_count != 0 && 999 < (cards = field_pda_count()))
        cards = 999;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    fy = y + 0x13;
    list = pda_draw_frame(list, x + 3, fy, 0x25, 0x25, 0x580047);
    list = pda_draw_frame(list, x + 0x29, fy, 0xB1, 0x25, 0x580047);
    list = pda_draw_frame(list, x + 3, fy + 0x26, 0xD7, 0x42, 0x580047);
    list = pda_draw_frame(list, x + 3, fy + 0x69, 0xD7, 0x25, 0x580047);
    list = pda_draw_frame(list, x + 3, fy + 0x8F, 0xD7, 0x25, 0x580047);
    list = pda_draw_frame(list, x + 3, fy + 0xB5, 0xD7, 0x16, 0x580047);
    if (cards >= 0)
        list = pda_draw_frame(list, x + 3, fy + 0xCC, 0xD7, 0x28, 0x580047);
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x1E, 0x1E, 0x1E, 0x1E);
    list = draw_sprite(list, x + 6, y + 0x16, 0, 0x47, 0xB0, 0, 0);
    if (cfg_get_language()) {
        list = draw_set_offsets(list, 0x0E, 0x0D, 0x0E, 0x0D);
        list = draw_sprite(list, x + 0x0D, y + 0xB4, 0, 0xAD, 0x44, 0, 0);
        list = draw_sprite(list, x + 0x51, y + 0xB4, 0, 0xBB, 0x44, 0, 0);
        list = draw_sprite(list, x + 0x93, y + 0xB4, 0, 0xC9, 0x44, 0, 0);
    }
    if (cards >= 0) {
        cx = x + 0xAD;
        if (cards >= 10)
            cx -= 10;
        if (cards >= 100)
            cx -= 10;
        list = draw_set_offsets(list, 0x11, 0x1E, 0x11, 0x1E);
        list = draw_sprite(list, cx, y + 0xE4, 0, 0x100, 0x10, 0, 0);
    }
    ty = y + 0x17;
    font_text_set(list);
    font_set_size(0x0D, 0x0D);
    font_set_color(FONT_BROWN);
    /* name */
    font_draw_text((x + 0x2E) << 6, ty << 6, (480 - (x + 0x2E)) << 6, table_get_entry(fld_gim_get(5), 0x3E));
    w = font_measure_text(table_get_entry(fld_gim_get(5), 0x3E));
    cx = x + 0x2E + ((w >> 6) + 2);
    font_draw_text(cx << 6, ty << 6, (480 - cx) << 6, sysstate_get_name());
    /* day */
    font_draw_text((x + 0x2E) << 6, (ty + 0xF) << 6, (480 - (x + 0x2E)) << 6, table_get_entry(fld_gim_get(5), 0x3F));
    pda_draw_number(0, x + 0xCF, ty + 0xF, day_next(), 9, 3);
    /* days */
    lx = (x + 9) << 6;
    lw = (480 - (x + 9)) << 6;
    font_draw_text(lx, (ty + 0x26) << 6, lw, table_get_entry(fld_gim_get(5), 0x40));
    pda_draw_number(0, x + 0xB5, ty + 0x35, day_calc(0), 10, 9);
    rx = (x + 0xB9) << 6;
    rw = (480 - (x + 0xB9)) << 6;
    font_draw_text(rx, (ty + 0x35) << 6, rw, table_get_entry(fld_gim_get(5), 0x41));
    font_draw_text(lx, (ty + 0x44) << 6, lw, table_get_entry(fld_gim_get(5), 0x42));
    pda_draw_number(0, x + 0xB5, ty + 0x53, day_calc(2), 10, 9);
    font_draw_text(rx, (ty + 0x53) << 6, rw, table_get_entry(fld_gim_get(5), 0x41));
    font_draw_text(lx, (ty + 0x69) << 6, lw, table_get_entry(fld_gim_get(5), 0x43));
    pda_draw_number(0, x + 0xB5, ty + 0x78, sysstate_get_u32_54(0), 10, 9);
    font_draw_text(rx, (ty + 0x78) << 6, rw, table_get_entry(fld_gim_get(5), 0x30));
    /* duel record */
    font_draw_text(lx, (ty + 0x8F) << 6, lw, table_get_entry(fld_gim_get(5), 0x44));
    sysstate2_get_triplet(0, rec);
    total = rec[2] + (rec[0] + rec[1]);
    if (cfg_get_language() == 0) {
        unsigned short c[2] = { 0 };

        pda_draw_number(0, x + 0x3E, ty + 0x9E, rec[0], 10, 5);
        c[0] = 0x52DD;
        sy = (ty + 0x9E) << 6;
        font_draw_text((x + 0x40) << 6, sy, (480 - (x + 0x40)) << 6, c);
        pda_draw_number(0, x + 0x82, ty + 0x9E, rec[1], 10, 5);
        c[0] = 0x6557;
        font_draw_text((x + 0x84) << 6, sy, (480 - (x + 0x84)) << 6, c);
        pda_draw_number(0, x + 0xC4, ty + 0x9E, rec[2], 10, 5);
        c[0] = 0x5206;
        font_draw_text((x + 0xC6) << 6, sy, (480 - (x + 0xC6)) << 6, c);
    } else {
        pda_draw_number(0, x + 0x4E, ty + 0x9E, rec[0], 10, 5);
        pda_draw_number(0, x + 0x92, ty + 0x9E, rec[1], 10, 5);
        pda_draw_number(0, x + 0xD4, ty + 0x9E, rec[2], 10, 5);
    }
    /* win rate */
    sy = (ty + 0xB5) << 6;
    font_draw_text(lx, sy, lw, table_get_entry(fld_gim_get(5), 0x45));
    if (rec[0] == 0) {
        pda_draw_number(0, x + 0xC0, ty + 0xB5, 0, 10, 1);
    } else if (rec[0] == total) {
        pda_draw_number(0, x + 0xC0, ty + 0xB5, 100, 10, 3);
    } else {
        unsigned short d[2] = { 0 };

        pct = rec[0] * 1000 / total;
        if (pct == 0)
            pct = 1;
        pda_draw_number(0, x + 0xAC, ty + 0xB5, pct / 10, 10, 2);
        d[0] = '.';
        font_draw_text((x + 0xAE) << 6, sy, 0x200, d);
        pda_draw_number(0, x + 0xC0, ty + 0xB5, pct % 10, 10, 1);
    }
    font_draw_text((x + 0xC2) << 6, sy, (480 - (x + 0xC2)) << 6, table_get_entry(fld_gim_get(5), 0x46));
    if (cards >= 0) {
        font_draw_text(lx, (ty + 0xCC) << 6, lw, table_get_entry(fld_gim_get(5), 0x47));
        pda_draw_number(0, x + 0xD2, ty + 0xDD, cards, 10, 3);
        cx = x + 0xBF;
        if (cards >= 10)
            cx -= 10;
        if (cards >= 100)
            cx -= 10;
        font_draw_text(cx << 6, (ty + 0xDD) << 6, (480 - cx) << 6, pda_times_text);
    }
    return font_text_clear();
}
#endif

/* widest value text of the text speed and volume options (pixels) */
unsigned char pda_opt_value_width(void)
{
    int n = func_08824028(1);
    unsigned int max = 0;
    unsigned int w;
    int i;

    font_set_size(0x0D, 0x0D);
    for (i = 0; i < n; i++) {
        w = font_measure_text(table_get_entry(func_08825A94(0), pda_speed_names[i])) + 0x40;
        if (max < w)
            max = w;
    }
    n = func_08824028(5);
    for (i = 0; i < n; i++) {
        w = font_measure_text(table_get_entry(func_08825A94(0), pda_volume_names[i])) + 0x40;
        if (max < w)
            max = w;
    }
    return max >> 6;
}

/* widest of the "From/Subject/Date" labels of a mail (1/64 px, at most 48 px) */
unsigned int pda_mail_label_width(void)
{
    unsigned short *labels[3];
    unsigned int max = 0;
    unsigned int w;
    int i;

    font_set_size(0x0C, 0x0C);
    labels[0] = table_get_entry(fld_gim_get(5), 0x5E);
    labels[1] = table_get_entry(fld_gim_get(5), 0x5F);
    labels[2] = table_get_entry(fld_gim_get(5), 0x60);
    for (i = 0; i < 3; i++) {
        w = font_measure_text(labels[i]) + 0x40;
        if (max < w)
            max = w;
    }
    if (max > 0xC00)
        max = 0xC00;
    return max;
}

/* options page: nine rows, on/off switches, the speed and volume values and the three gauges */
#ifdef NON_MATCHING /* regalloc */
void *pda_draw_options(void *list, int x, int y)
{
    void *gim = field_pda->gim;
    int shift = 0;
    unsigned char *rows = pda_opt_rows;
    unsigned short *names;
    unsigned short *v;
    int i;
    int tx;
    int vx;
    int w;
    unsigned char cur;

    if (gim == 0)
        return list;
    if (cfg_get_language()) {
        shift = -3;
        rows = pda_opt_rows_en;
    }
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = pda_draw_frame(list, x + 3, y + 0x12, 0xD8, 0x16, 0x580047);
    list = pda_draw_frame(list, x + 3, y + 0x29, 0xD8, 0x16, 0x580047);
    list = pda_draw_frame(list, x + 3, y + 0x40, 0xD8, 0x50, 0x580047);
    list = pda_draw_frame(list, x + 3, y + 0x91, 0xD8, 0x63, 0x580047);
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 1, 1, 0xD2, 0x11);
    list = draw_sprite(list, x + 5, y + rows[field_pda->cursor] + 2, 0, 0xB4, 0x20, 0, 0);
    list = pda_draw_cursor(list, x + 3, y + rows[field_pda->cursor], 0xD7, 0x16);
    if (cfg_get_language() == 0)
        list = pda_draw_gauge(list, x + 100, y + rows[2] + 5, field_pda->opt[2]);
    list = pda_draw_gauge(list, x + 100, y + rows[3] + 5, field_pda->opt[3]);
    list = pda_draw_gauge(list, x + 100, y + rows[4] + 5, field_pda->opt[4]);
    for (i = 0; i < 9; i++) {
        if (i == 8 || i == 7 || i == 6 || i == 0) {
            int on = field_pda->opt[i];

            list = draw_set_offsets(list, 0x20, 0x0F, 0x20, 0x0F);
            list = draw_sprite(list, x + 0x90 + (on != 0) * 0x24, y + rows[i] + 3, 0, 0x47, 0xA0, 0, 0);
        }
    }
    if ((*gfx_get_state() >> 3) % 3 != 0 && (field_pda->cursor == 5 || field_pda->cursor == 1)) {
        cur = field_pda->opt[field_pda->cursor];
        list = draw_set_offsets(list, 8, 0x0C, 8, 0x0C);
        if (cur != 0)
            list = draw_sprite(list, x + 0xA3 + shift - (field_pda->opt_w >> 1), y + rows[field_pda->cursor] + 5, 0, 0xB7, 0x30, 0, 0);
        if (cur < func_08824028(field_pda->cursor))
            list = draw_sprite(list, (field_pda->opt_w >> 1) + x + 0xBA + shift, y + rows[field_pda->cursor] + 5, 0, 0xBF, 0x30, 0, 0);
    }
    font_text_set(list);
    font_set_size(0x0D, 0x0D);
    font_set_color(FONT_BROWN);
    font_draw_text((x + 9) << 6, (y + 0x44) << 6, 0x32C0, table_get_entry(func_08825A94(0), 3));
    font_draw_text((x + 9) << 6, (y + 0x95) << 6, 0x32C0, table_get_entry(func_08825A94(0), 7));
    names = pda_opt_names;
    vx = x + 0xB2 + shift;
    for (i = 0; i < 9; i++, names++) {
        tx = x + 9;
        if (cfg_get_language() != 0 && i == 2)
            continue;
        if (i == field_pda->cursor)
            font_set_color(0xFFFFFFFF);
        else
            font_set_color(FONT_BROWN);
        if (i > 1)
            tx = x + 0x10;
        font_draw_text(tx << 6, (y + rows[i] + 4) << 6, 0x2000, table_get_entry(func_08825A94(0), *names));
        switch (i) {
        case 0:
        case 6:
        case 7:
        case 8:
            w = font_measure_text(table_get_entry(func_08825A94(0), 0x0C));
            font_draw_text((x + 0xA0 - (w >> 7)) << 6, (y + rows[i] + 4) << 6, 0xA00, table_get_entry(func_08825A94(0), 0x0C));
            w = font_measure_text(table_get_entry(func_08825A94(0), 0x0D));
            font_draw_text((x + 0xC4 - (w >> 7)) << 6, (y + rows[i] + 4) << 6, 0xA00, table_get_entry(func_08825A94(0), 0x0D));
            break;
        case 1:
            v = &pda_speed_names[field_pda->opt[i]];
            font_set_size(font_measure_wtext(0xF00, table_get_entry(func_08825A94(0), *v)), 0x0D);
            w = font_measure_text(table_get_entry(func_08825A94(0), *v));
            font_draw_wtext((vx - (w >> 7)) << 6, (y + rows[i] + 4) << 6, 0xF00, table_get_entry(func_08825A94(0), *v));
            font_set_size(0x0D, 0x0D);
            break;
        case 5:
            v = &pda_volume_names[field_pda->opt[i]];
            font_set_size(font_measure_wtext(0xF00, table_get_entry(func_08825A94(0), *v)), 0x0D);
            w = font_measure_text(table_get_entry(func_08825A94(0), *v));
            font_draw_wtext((vx - (w >> 7)) << 6, (y + rows[i] + 4) << 6, 0xF00, table_get_entry(func_08825A94(0), *v));
            font_set_size(0x0D, 0x0D);
            break;
        }
    }
    return font_text_clear();
}
#endif

/* the page buttons; n < 4: the buttons slide in (n = frame), else the cursor is on the page */
#ifdef NON_MATCHING /* regalloc */
void *pda_draw_tabs(void *list, int x, int y, int n)
{
    unsigned short *title = 0;
    int sliding = 0;
    int last = 1;
    unsigned short *name;
    unsigned char alpha;
    int hl;
    int sel;
    int i;
    int ty;
    int tv;
    int d;

    if (n < 4) {
        sliding = 1;
        last = (n - 1) / 2;
    } else {
        title = table_get_entry(fld_gim_get(5), pda_page_titles[field_pda->cursor]);
    }
    list = func_0881FBCC(list, x, y, title);
    for (i = last, ty = y + 0x11 + last * 0x20, name = &pda_tab_names[last], tv = last * 0x18 + 0x90; i >= 0;
         i--, ty -= 0x20, name--, tv -= 0x18) {
        int py = ty;

        hl = 0;
        sel = 0;
        alpha = 0xFF;
        if (!sliding && field_pda->cursor == i) {
            if (!field_pda->leaving || (field_pda->wait >> 1 & 1) != 1)
                hl = 1;
            sel = 1;
        }
        if (hl) {
            list = pda_draw_tab(list, PDA_XY(x, ty), PDA_XY(0x16E, tv), 0xFF, table_get_entry(fld_gim_get(5), *name), 1, 1);
        } else {
            title = table_get_entry(fld_gim_get(5), *name);
            if (sliding && i == last && n <= i * 2 + 2) {
                d = i * 2 + 2 - n;
                if (d != 0) {
                    py -= d * 0x20 / 2;
                    title = 0;
                }
                alpha -= (unsigned char)((d + 1) * 0x60);
            }
            list = pda_draw_tab(list, PDA_XY(x, py), PDA_XY(0x16E, tv), alpha, title, 0, sel);
        }
    }
    return list;
}
#endif

/* calendar of the month shown: weekday header, the days with their marks, and the date of the cursor */
#ifdef NON_MATCHING /* regalloc */
void *pda_draw_calendar(void *list, int x, int y)
{
    unsigned short date[512];
    unsigned char month;
    unsigned char day;
    unsigned char kind;
    int row;
    int col;
    int cx;
    int cy;
    int w;
    int tx;
    int ty;

    if (field_pda->gim == 0)
        return list;
    list = gim_set_texture(list, field_pda->gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = pda_draw_frame(list, x + 3, y + 0x21, 0xD8, 0xE6, 0x580047);
    list = pda_draw_frame(list, x + 9, y + 0xD5, 0xCC, 0x2D, 0x580047);
    list = draw_set_offsets(list, 0x38, 0x14, 0x38, 0x14);
    for (row = 0, cx = x + 3; row < 4; row++, cx += 0x36)
        list = draw_sprite(list, cx, y + 0x12, 0, row != field_pda->month ? 0x9F : 0x67, (row + 1) / 2 * 0x14 + 0x7F, 0, 0);
    list = draw_set_offsets(list, 0x1C, 0x0E, 0x1C, 0x0E);
    list = draw_sprite(list, x + 0x0A, y + 0x25, 0, 0x67, 0x3D, 0, 0);
    for (col = 1, cx = x + 0x27; col < 7; col++, cx += 0x1D)
        list = draw_sprite(list, cx, y + 0x25, 0, 0x83, 0x3D, 0, 0);
    for (row = 0, cy = y + 0x33; row < 6; row++, cy += 0x1B) {
        for (col = 0, cx = x + 0x0A; col < 7; col++, cx += 0x1D) {
            day = func_088254D0(field_pda->month, row, col);
            list = pda_draw_day(list, (cx & 0xFFFF) | cy << 16, col,
                                field_pda->month == field_pda->today_month && day == field_pda->today, day);
        }
    }
    list = pda_draw_cursor(list, x + 7 + field_pda->day % 7 * 0x1D, y + 0x30 + field_pda->day / 7 * 0x1B, 0x21, 0x1F);
    for (row = 0, cy = y + 0x3B; row < 6; row++, cy += 0x1B) {
        for (col = 0, cx = x + 0x14; col < 7; col++, cx += 0x1D) {
            day = func_088254D0(field_pda->month, row, col);
            kind = 0;
            if (day > 0) {
                day = func_08825584(field_pda->month, day);
                if (day != 0 && day < 12)
                    kind = pda_event_marks[day];
            }
            list = pda_draw_mark(list, (cx & 0xFFFF) | cy << 16, kind);
        }
    }
    list = dl_scissor(list, x + 6, y + 0x17, 0xD1, 10);
    list = dl_enable(list, 2);
    font_text_set(list);
    font_set_size(0x0C, 0x0C);
    font_set_color(0xFFFFFFFF);
    for (row = 0, cx = x; row < 4; row++, cx += 0x36) {
        unsigned short *t = table_get_entry(fld_gim_get(5), pda_month_names[row]);

        tx = cx + 6;
        ty = y + 0x17;
        if (field_pda->month == row)
            ty = y + 0x16;
        w = (font_measure_text(t) >> 6) + 1;
        if (w <= 0x30)
            tx += (0x30 - w) / 2;
        font_draw_wtext(tx << 6, ty << 6, 0xC00, t);
    }
    list = font_text_clear();
    list = dl_disable(list, 2);
    font_text_set(list);
    font_set_size(0x0C, 0x0C);
    font_set_color(0xFFFFFFFF);
    for (col = 0, cx = x; col < 7; col++, cx += 0x1D) {
        unsigned short *t = table_get_entry(fld_gim_get(5), pda_wday_names[col]);

        tx = cx + 0x0B;
        w = (font_measure_text(t) >> 6) + 1;
        if (w <= 0x19)
            tx += (0x19 - w) / 2;
        font_draw_wtext(tx << 6, (y + 0x26) << 6, 0x640, t);
    }
    font_set_color(FONT_BROWN);
    font_set_size(0x0E, 0x0E);
    month = field_pda->month;
    col = field_pda->day % 7;
    day = func_088254D0(month, field_pda->day / 7, col);
    pda_date_text(date, month, day, col);
    tx = (x + 0x0D) << 6;
    font_draw_wtext(tx, (y + 0xD9) << 6, 0x3100, date);
    if (cfg_get_language() == 0)
        font_set_flag8000(1);
    kind = func_08825584(month, day);
    if (kind != 0 && kind < 12)
        font_draw_wtext(tx, (y + 0xE8) << 6, 0x3100, table_get_entry(func_08825A48(0), kind));
    if (cfg_get_language() == 0)
        font_set_flag8000(0);
    return font_text_clear();
}
#endif

/* mail list: five mails from the top one, the scroll bar and the cursor */
void *pda_draw_mail_list(void *list, int x, int y)
{
    int i;
    int ty;
    int n;
    int sel;
    FieldPdaMail *m;

    for (i = 0, ty = y + 0x12; i < 5; i++, ty += 0x31) {
        n = field_pda->top + i;
        sel = 0;
        if (n >= field_pda->n_mails) {
            m = 0;
        } else {
            if (i == field_pda->sel)
                sel = 1;
            m = &field_pda->mails[n];
        }
        list = pda_draw_mail_item(list, x + 3, ty, m, sel);
    }
    list = pda_draw_scrollbar(list, x + 0xCF, y + 0x1D, 0xDF, field_pda->top, 5, field_pda->n_mails);
    if (field_pda->n_mails > 0)
        list = pda_draw_cursor(list, x + 3, y + 0x12 + field_pda->sel * 0x31, 0xCA, 0x30);
    return list;
}

/* an open mail: sender face, header, gauge and the text scrolled inside its window */
void *pda_draw_mail(void *list, int x0, int y0)
{
    int x = x0 + 0x0E;
    int y = y0 + 0x17;
    int i;
    int ty;

    if (field_pda->gim == 0)
        return list;
    list = gim_set_texture(list, field_pda->gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = pda_draw_frame(list, x, y, 0xB5, 0xEB, 0x2F00D7);
    list = pda_draw_frame(list, x + 0x29, y + 8, 99, 0x47, 0x2F00D7);
    list = pda_draw_frame(list, x + 6, y + 0x86, 0xA9, 0x5F, 0x580047);
    list = func_0882AD34(list, field_pda->face, x + 0x2C, y + 0x0B);
    list = pda_draw_bar(list, x + 0x21, y + 0x52, field_pda->bar_pos, field_pda->bar_max);
    list = pda_draw_mail_head(list, x + 7, y + 0x5D, 0xA6, &field_pda->mails[field_pda->top + field_pda->sel], 1);
    list = dl_scissor(list, x + 0x0B, y + 0x8B, 0x9E, 0x54);
    list = dl_enable(list, 2);
    font_set_size(0x0E, 0x0E);
    font_set_color(FONT_BROWN);
    font_set_line_extra(0x80);
    if (cfg_get_language() == 0)
        font_set_flag8000(1);
    font_text_set(list);
    ty = ((y + 0x8B) << 6) - field_pda->scroll;
    for (i = 0; i < field_pda->n_lines; i++) {
        font_draw_wtext_ex((x + 0x0B) << 6, ty, 0x2780, field_pda->text_h + 0x1500, field_pda->lines[i].text, -1);
        ty = font_get_cursor_y();
    }
    if (cfg_get_language() == 0)
        font_set_flag8000(0);
    font_set_line_extra(0);
    list = font_text_clear();
    list = pda_draw_mail_mask(list, x + 0x0B, y + 0x8B);
    return dl_disable(list, 2);
}

/* a page button: pos and uv pack x | y << 16, alpha 0..255, title 0 = none */
#ifdef NON_MATCHING /* regalloc */
void *pda_draw_tab(void *list, int pos, int uv, unsigned char alpha, unsigned short *title, int sel, int blink)
{
    int iu = uv & 0xFFFF;
    void *gim = field_pda->gim;
    int x = pos & 0xFFFF;
    int y = pos >> 16 & 0xFFFF;
    int iv = uv >> 16 & 0xFFFF;
    int frame = 0;
    int u;
    unsigned int color;
    int h;

    if (gim == 0)
        return list;
    if (blink)
        frame = *gfx_get_state() / 5 % 3;
    list = gim_set_texture(list, gim);
    color = alpha << 24 | 0xFFFFFF;
    list = dl_ambient_color(list, color);
    list = dl_set_tex_filter(list, 0, 0);
    u = 0x47;
    if (sel)
        u += 0x40;
    list = draw_set_offsets(list, 0x28, 0x20, 0x28, 0x20);
    list = draw_sprite(list, x + 3, y, 0, u, 0x10, 0, 0);
    list = draw_set_offsets(list, 1, 0x20, 0x98, 0x20);
    list = draw_sprite(list, x + 0x2B, y, 0, u + 0x28, 0x10, 0, 0);
    list = draw_set_offsets(list, 0x18, 0x20, 0x18, 0x20);
    list = draw_sprite(list, x + 0xC3, y, 0, u + 0x28, 0x10, 0, 0);
    list = draw_set_offsets(list, 0x1E, 0x18, 0x1E, 0x18);
    list = draw_sprite(list, x + 6, y + 3, 0, iu + frame * 0x1E, iv, 0, 0);
    if (title) {
        font_text_set(list);
        font_set_size(0x0E, 0x0E);
        if (sel)
            font_set_color(color);
        else
            font_set_color(alpha << 24 | 0x492B02);
        h = 0x1E - (font_line_height() >> 6);
        font_draw_text((x + 0x2D) << 6, (y + h / 2) << 6, (480 - (x + 0x2D)) << 6, title);
        list = font_text_clear();
    }
    if (sel)
        list = pda_draw_cursor(list, x + 3, y, 0xD8, 0x1F);
    return list;
}
#endif

/* yellow cursor frame */
void *pda_draw_cursor(void *list, int x, int y, int w, int h)
{
    if (field_pda->gim == 0)
        return list;
    list = gim_set_texture(list, field_pda->gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    return pda_draw_frame(list, x, y, w, h, 0x1000C7);
}

/* nine-slice frame from the 8x8 corners at uv (u | v << 16) */
#ifdef NON_MATCHING /* args */
void *pda_draw_frame(void *list, int x, int y, int w, int h, int uv)
{
    int u = uv & 0xFFFF;
    int v = uv >> 16 & 0xFFFF;
    int by;
    int rx;

    list = draw_set_offsets(list, 8, 8, 8, 8);
    list = draw_sprite(list, x, y, 0, u, v, 0, 0);
    rx = x + w - 8;
    list = draw_sprite(list, rx, y, 0, u + 8, v, 0, 0);
    by = y + h - 8;
    list = draw_sprite(list, x, by, 0, u, v + 8, 0, 0);
    list = draw_sprite(list, rx, by, 0, u + 8, v + 8, 0, 0);
    if (w > 0x10) {
        int u8 = u + 8;

        list = draw_set_offsets(list, 1, 8, w - 0x10, 8);
        list = draw_sprite(list, x + 8, y, 0, u8, v, 0, 0);
        list = draw_sprite(list, x + 8, by, 0, u8, v + 8, 0, 0);
    }
    if (h > 0x10) {
        int v8 = v + 8;

        list = draw_set_offsets(list, 8, 1, 8, h - 0x10);
        list = draw_sprite(list, x, y + 8, 0, u, v8, 0, 0);
        list = draw_sprite(list, rx, y + 8, 0, u + 8, v8, 0, 0);
    }
    list = draw_set_offsets(list, 1, 1, w - 0x10, h - 0x10);
    return draw_sprite(list, x + 8, y + 8, 0, u + 8, v + 8, 0, 0);
}
#endif

/* 15-step gauge, level steps lit */
void *pda_draw_gauge(void *list, int x, int y, int level)
{
    int i;
    int u;

    if (field_pda->gim == 0)
        return list;
    list = gim_set_texture(list, field_pda->gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 7, 0x0C, 7, 0x0C);
    for (i = 0; i < 15; i++, x += 7) {
        u = 0xD6;
        if (i >= level)
            u += 7;
        list = draw_sprite(list, x, y, 0, u, 0x44, 0, 0);
    }
    return list;
}

/* right-aligned number ending at x, step pixels per digit; the first digit is always drawn */
void *pda_draw_number(void *list, short x0, short y, unsigned int value, unsigned short step, unsigned short digits)
{
    short s[2] = { '0', 0 };
    unsigned short x;
    int last;
    int i;

    x = x0;
    if (list)
        font_text_set(list);
    for (i = last = digits - 1; i >= 0; i--) {
        int d = value % 10;
        x -= step;
        if (i == last || value != 0) {
            s[0] = d + '0';
            font_draw_text(x << 6, y << 6, (480 - x) << 6, (unsigned short *)s);
        }
        value /= 10;
        if (value == 0)
            break;
    }
    if (list)
        list = font_text_clear();
    return list;
}
