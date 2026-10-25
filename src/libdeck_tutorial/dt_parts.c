// @range 0x08808C6C-0x0880B704
// @asm 0x08808DE0-0x08808F88
// @asm 0x0880A340-0x0880A5BC
// @asm 0x0880B010-0x0880B704
// @rodata 0x08820F78-0x08820F9C
// @data 0x08821E70-0x08821F28
/* Small lookups (sort key, copies and position of an id in a sorted list, icon row of a card kind)
   and the drawing pieces shared by the panes: digit strips, framed boxes, card icons (rarity,
   attribute, type, limit marks), the ATK/DEF value, a card's picture tile, the kind frame of a list
   row, the per-card counters, the scroll bar with its blinking arrows and the button help window.
   The same code as libselect_card's sc_parts.c. */
#include "libdeck_tutorial.h"
#include "allegrex.h"

unsigned short dt_sort_key(unsigned short id, unsigned short *tab);
int dt_id_find(unsigned short id, unsigned short sub, ScIdList *l);
int dt_kind_icon(unsigned short kind);
void *dt_draw_digits(void *list, ScPt pos, int pad, unsigned int value, unsigned short fmt, unsigned int color, void *gim);
void *dt_draw_num_text(void *list, int x, int y, int value, int step, unsigned short n, int pad);
void *dt_draw_frame(void *list, int x, int y, int w, int h, void *gim, int style);
void *dt_draw_sprite(void *list, ScSprite *s, void *gim);
void *dt_draw_icon_a(void *list, ScPt pos, unsigned short id, void *gim);
void *dt_draw_icon_b(void *list, ScPt pos, unsigned short id, void *gim);
void *dt_draw_icon_c(void *list, ScPt pos, unsigned short id, void *gim);
void *dt_draw_icon_d(void *list, ScPt pos, unsigned short id, void *gim);
void *dt_draw_icon_e(void *list, ScPt pos, unsigned short id, void *gim);
void *dt_draw_level(void *list, ScPt pos, unsigned short level, unsigned int color, void *gim);
void *dt_draw_stat(void *list, unsigned int st, ScPt pos, unsigned short size, unsigned int color, void *gim);
void *dt_draw_card_pict(void *list, int x, int y, int shadow, unsigned int color, ScEnt *e, void *gim, void *gim_alt, ScIdList *alt);
void *dt_draw_card_marks(void *list, int x, int y, ScEnt *e, int show_count, int blink, int show_fixed, unsigned short rank, void *gim_digits, void *gim);
void *dt_draw_row_frame(void *list, ScPt pos, unsigned short w, unsigned short kind, void *gim);
void *dt_draw_flag_icon(void *list, ScPt pos, unsigned short kind, void *gim);
void *dt_draw_counters(void *list, ScPt pos, ScCounter *c, void *gim_strip, void *gim);
void *dt_draw_icon_text(void *list, ScIconText *t, int x, int y);
void *dt_draw_arrow(void *list, int x, int y, unsigned char dir, unsigned int period, int blink, int base, unsigned int t, void *gim);
void *dt_draw_scrollbar(void *list, int x, int y, int h, int pos, int first, int total, int frame, void *gim);
void dt_scroll_analog(ScScroll *s);
void *dt_draw_box(void *list, int x, int y, int w, int h, void *gim);
void *dt_draw_help(void *list, int x, int y, int alt, unsigned int flags, void *gim, void *strtbl);



unsigned short dt_sort_key(unsigned short id, unsigned short *tab)
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
unsigned short dt_id_count(unsigned short id, ScIdList *l)
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

int dt_id_find(unsigned short id, unsigned short sub, ScIdList *l)
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
int dt_kind_icon(unsigned short kind)
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

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifdef NON_MATCHING /* forma igual; o mwcc troca os registradores s4-s8 de value, pad, x, w e y */
/* digits right to left from pos, w x 9 each: n = fmt & 0xFF digits, style = fmt >> 8 */
void *dt_draw_digits(void *list, ScPt pos, int pad, unsigned int value, unsigned short fmt, unsigned int color, void *gim)
{
    short px = pos & 0xFFFF;
    short py = pos >> 16 & 0xFFFF;
    unsigned char style = fmt >> 8;
    unsigned char n = fmt & 0xFF;
    int step = 0;
    int w;
    short x;
    int i;

    if (gim == 0)
        return list;
    list = dl_ambient_color(list, color);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    switch (style) {
    case 3:
        step = -1;
        w = 7;
        break;
    case 0:
        w = 7;
        break;
    case 1:
        break;
    case 2:
        break;
    }
    list = draw_set_offsets(list, w, 9, w, 9);
    step = w + step;
    x = px + (short)((n - 1) * step);
    for (i = 0; i < n; i++) {
        list = draw_sprite(list, x, py, 0, value % 10 * w, 0, 0, 0);
        x -= (short)step;
        value /= 10;
        if (!pad && value == 0)
            break;
    }
    return list;
}
#endif

/* font digits right to left, n at most (all of them with pad) */
void *dt_draw_num_text(void *list, int x, int y, int value, int step, unsigned short n, int pad)
{
    short digit[2] = { '0', 0 };
    int px = x & 0xFFFF;
    int i;

    font_text_set(list);
    for (i = n - 1; i >= 0; i--) {
        px = (px - step) & 0xFFFF;
        digit[0] = value % 10 + '0';
        font_draw_text(px << 6, (short)y << 6, (480 - px) << 6, (unsigned short *)digit);
        value /= 10;
        if (!pad && value == 0)
            break;
    }
    return font_text_clear();
}

/* rounded frame around (x, y, w, h); bit 3 of style makes it one pixel larger */
void *dt_draw_frame(void *list, int x, int y, int w, int h, void *gim, int style)
{
    int grow;
    int half;
    int top;
    int left;
    int bottom;
    int right;
    int mid;

    if (gim == 0)
        return list;
    if (w <= 0 || h <= 0)
        return list;
    grow = (style >> 3 & 1) * 2;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 6, 6, 6, 6);
    half = grow / 2;
    top = y - 4 - half;
    left = x - 4 - half;
    list = draw_sprite(list, left, top, 0, 213, 122, 0, 0);
    right = half + (x + w - 2);
    list = draw_sprite(list, right, top, 0, 250, 122, 0, 0);
    bottom = half + (y + h - 2);
    list = draw_sprite(list, left, bottom, 0, 213, 159, 0, 0);
    list = draw_sprite(list, right, bottom, 0, 250, 159, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, h + grow - 4);
    mid = y + 2 - half;
    list = draw_sprite(list, left, mid, 0, 213, 128, 0, 0);
    list = draw_sprite(list, right, mid, 0, 250, 128, 0, 0);
    list = draw_set_offsets(list, 1, 6, w + grow - 4, 6);
    mid = x + 2 - half;
    list = draw_sprite(list, mid, top, 0, 219, 122, 0, 0);
    list = draw_sprite(list, mid, bottom, 0, 219, 159, 0, 0);
    return list;
}

void *dt_draw_sprite(void *list, ScSprite *s, void *gim)
{
    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, s->w, s->h, s->w, s->h);
    return draw_sprite(list, (short)s->x, (short)s->y, 0, s->u, s->v, 0, 0);
}

/* 17x17 icons, 7 per row */
void *dt_draw_icon_a(void *list, ScPt pos, unsigned short id, void *gim)
{
    ScSprite s = { pos, pos >> 16, 17, 17, 0, 0 };
    int i;

    if (id == 0)
        return list;
    i = (unsigned short)(id - 1);
    s.u = i % 7 * 17;
    s.v = i / 7 * 17;
    return dt_draw_sprite(list, &s, gim);
}

void *dt_draw_icon_b(void *list, ScPt pos, unsigned short id, void *gim)
{
    ScSprite s = { pos, pos >> 16, 17, 17, 0, 0 };

    if (id == 0)
        return list;
    s.u = (unsigned short)(id - 1) * 17;
    s.v = 51;
    return dt_draw_sprite(list, &s, gim);
}

void *dt_draw_icon_c(void *list, ScPt pos, unsigned short id, void *gim)
{
    ScSprite s = { pos, pos >> 16, 17, 17, 0, 0 };
    int i;

    if (id == 0)
        return list;
    switch (id) {
    case 8:
        id = 7;
        break;
    case 9:
        id = 13;
        break;
    }
    i = (unsigned short)(id - 1);
    s.u = i % 7 * 17;
    s.v = i / 7 * 17 + 68;
    return dt_draw_sprite(list, &s, gim);
}

void *dt_draw_icon_d(void *list, ScPt pos, unsigned short id, void *gim)
{
    ScSprite s = { pos, pos >> 16, 17, 17, 0, 17 };

    s.u = id * 17;
    return dt_draw_sprite(list, &s, gim);
}

void *dt_draw_icon_e(void *list, ScPt pos, unsigned short id, void *gim)
{
    ScSprite s = { pos, pos >> 16, 17, 17, 0, 0 };

    s.u = id * 17 + 51;
    return dt_draw_sprite(list, &s, gim);
}

/* level stars: a star and the count from the 19x11 digit strip */
void *dt_draw_level(void *list, ScPt pos, unsigned short level, unsigned int color, void *gim)
{
    short x;
    short y;
    unsigned short i;

    x = pos & 0xFFFF;
    y = pos >> 16 & 0xFFFF;
    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 9, 9, 9, 9);
    list = draw_sprite(list, x, y + 2, 0, 18, 83, 0, 0);
    list = dl_ambient_color(list, color);
    list = draw_set_offsets(list, 19, 11, 19, 11);
    i = level - 1;
    return draw_sprite(list, x + 10, y + 1, 0, (unsigned short)(i % 6 * 19), (unsigned short)(i / 6 * 11 + 45), 0, 0);
}

unsigned short dt_stat_labels[2][5] = { { 'A', 'T', 'K', '/', 0 }, { 'D', 'E', 'F', '/', 0 } };

/* "ATK/" or "DEF/" (kind 0/1) or an icon (2/3), then the value right-aligned in 4 digits ("?" = 0xFFFF) */
void *dt_draw_stat(void *list, unsigned int st, ScPt pos, unsigned short size, unsigned int color, void *gim)
{
    unsigned short kind = st & 0xFFFF;
    unsigned short value = st >> 16 & 0xFFFF;
    short x = pos & 0xFFFF;
    short y = pos >> 16 & 0xFFFF;
    short digit[2] = { '0', 0 };
    int w;
    int h;
    int tx;
    int cw;
    int dx;
    int i;
    int d;
    unsigned short *label;

    h = size >> 8 & 0xFF;
    font_set_size(size & 0xFF, h);
    font_set_color(color);
    switch (kind) {
    case 0:
    case 1:
        font_text_set(list);
        label = dt_stat_labels[kind];
        w = font_measure_text(label) >> 6;
        font_draw_text(x << 6, y << 6, (480 - x) << 6, label);
        list = font_text_clear();
        break;
    case 2: {
        ScSprite s = { x, y, 9, 9, 0, 0x53 };

        s.y += (unsigned short)((h - 8) / 2);
        list = dt_draw_sprite(list, &s, gim);
        w = 10;
        break;
    }
    case 3: {
        ScSprite s = { x, y, 9, 9, 9, 0x53 };

        s.y += (unsigned short)((h - 8) / 2);
        list = dt_draw_sprite(list, &s, gim);
        w = 10;
        break;
    }
    }
    tx = x + w;
    font_text_set(list);
    if (value == 0xFFFF) {
        digit[0] = '?';
        dx = tx + ((font_measure_text((unsigned short *)digit) >> 6) + 1) * 3;
        font_draw_text(dx << 6, y << 6, (480 - dx) << 6, (unsigned short *)digit);
    } else {
        cw = (font_measure_text((unsigned short *)digit) >> 6) + 1;
        tx += cw * 3;
        for (i = 3; i >= 0; i--) {
            d = value % 10;
            if (i == 3 || value != 0) {
                digit[0] = d + '0';
                font_draw_text(tx << 6, y << 6, (480 - tx) << 6, (unsigned short *)digit);
            }
            tx -= cw;
            value /= 10;
        }
    }
    return font_text_clear();
}

/* tile of a card picture in cis_all.gim (or the alternate picture list): page and u/v; 1 = alternate */
int dt_card_tile(unsigned short *page, unsigned short *u, unsigned short *v, ScEnt *e, ScIdList *alt)
{
    int n;
    int i;
    unsigned short k;
    unsigned short key;

    if (inv_get_bit3(e->id)) {
        n = inv_get_hi(e->id);
        if (n > 0 && (i = dt_id_find(e->id, n, alt)) >= 0) {
            *page = 0;
            *u = i % 23 * 22;
            *v = i / 23 * 30;
            return 1;
        }
    }
    key = e->key;
    *page = key / 391;
    k = key % 391;
    *u = k % 23 * 22;
    *v = k / 23 * 30;
    return 0;
}

/* the 22x30 picture of a card, with a shadow when asked */
void *dt_draw_card_pict(void *list, int x, int y, int shadow, unsigned int color, ScEnt *e, void *gim, void *gim_alt, ScIdList *alt)
{
    unsigned short page;
    unsigned short u;
    unsigned short v;

    gim_alt = dt_card_tile(&page, &u, &v, e, alt) ? gim_alt : gim;
    if (gim_alt == 0)
        return list;
    list = gim_set_texture_frame(list, gim_alt, page);
    list = dl_set_tex_filter(list, 0, 0);
    if (shadow) {
        if ((color >> 24 & 0xFF) > 160)
            list = dl_ambient_color(list, color >> 1 & 0x7F000000);
        else
            list = dl_ambient_color(list, 0);
        list = draw_set_offsets(list, 22, 30, 22, 30);
        list = draw_sprite(list, x + 1, y + 1, 0, u, v, 0, 0);
    }
    list = dl_ambient_color(list, color);
    list = draw_set_offsets(list, 22, 30, 22, 30);
    return draw_sprite(list, x, y, 0, u, v, 0, 0);
}

/* marks over a picture: copies, the limit (or "new" blinking), and the rental rank */
void *dt_draw_card_marks(void *list, int x, int y, ScEnt *e, int show_count, int blink, int show_fixed, unsigned short rank, void *gim_digits, void *gim)
{
    int count;
    int lim;
    ScSprite s;

    if (show_count) {
        count = e->count;
        if (count > 1) {
            if (count >= 100)
                count = 99;
            list = dt_draw_digits(list, PT((short)(x + 8), (short)(y + 21)), 0, count, 2, 0xFFFFFFFF, gim_digits);
        }
    }
    if (!e->fixed && blink && (*gfx_get_state() >> 3 & 1)) {
        s.w = 17;
        s.h = 12;
        s.u = 37;
        s.v = 76;
        s.x = x;
        s.y = y;
        list = dt_draw_sprite(list, &s, gim);
    } else {
        lim = card_get_limit(e->id);
        if (lim < 3) {
            s.w = 12;
            s.h = 12;
            s.u = lim * 12;
            s.v = 76;
            s.x = x;
            s.y = y;
            list = dt_draw_sprite(list, &s, gim);
        }
    }
    if (show_fixed && e->fixed) {
        s.x = x + 12;
        s.y = y + 20;
        s.w = 10;
        s.h = 10;
        s.u = 37;
        s.v = 65;
        list = dt_draw_sprite(list, &s, gim);
    } else if (rank > 0 && rank < 4) {
        s.x = x + 10;
        s.y = y + 18;
        s.w = 12;
        s.h = 12;
        s.u = (rank - 1) * 12;
        s.v = 88;
        list = dt_draw_sprite(list, &s, gim);
    }
    return list;
}

/* the frame of a list row in the colour of the card kind, w wide */
void *dt_draw_row_frame(void *list, ScPt pos, unsigned short w, unsigned short kind, void *gim)
{
    short x = pos & 0xFFFF;
    short y = pos >> 16 & 0xFFFF;
    int k;

    if (gim == 0)
        return list;
    k = dt_kind_icon(kind);
    if (k < 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    if (w >= 250) {
        list = draw_set_offsets(list, 104, 31, 104, 31);
        list = draw_sprite(list, x, y, 0, 0, k << 5, 0, 0);
        list = draw_set_offsets(list, 191, 31, 191, 31);
        list = draw_sprite(list, x + 104, y, 0, 12, k << 5, 0, 0);
    } else if (w < 150) {
        list = draw_set_offsets(list, 73, 13, 73, 13);
        list = draw_sprite(list, x, y, 0, k / 3 * 80, k % 3 * 14 + 192, 0, 0);
    } else {
        list = draw_set_offsets(list, 203, 31, 203, 31);
        list = draw_sprite(list, x, y, 0, 0, k << 5, 0, 0);
    }
    return list;
}

/* "new" (0) / "favourite" (1) / other mark, 17x17 */
void *dt_draw_flag_icon(void *list, ScPt pos, unsigned short kind, void *gim)
{
    short x = pos & 0xFFFF;
    short y = pos >> 16 & 0xFFFF;
    unsigned short u = 0;

    switch (kind) {
    case 0:
        u += 17;
        break;
    case 1:
        u += 34;
        break;
    }
    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 17, 17, 17, 17);
    return draw_sprite(list, x - 1, y - 3, 0, u, 34, 0, 0);
}

unsigned char dt_count_icons[7][4] = {   /* u, v, w, h */
    { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x58, 0x0C, 0x0C }, { 0x0C, 0x58, 0x0C, 0x0C }, { 0x18, 0x58, 0x0C, 0x0C },
    { 0x24, 0x58, 0x0C, 0x0C }, { 0x24, 0x40, 0x0C, 0x0C }, { 0x30, 0x34, 0x0C, 0x0C },
};

/* label x offsets and strip widths of the counters (the templates of dt_draw_counters' locals) */
short dt_counter_xs[3] = { 0x17, 0x30, 0x50 };
unsigned short dt_counter_ws[3] = { 0x18, 0x1A, 0x14 };

#ifdef NON_MATCHING /* o original guarda os dois gim na pilha e c/x em outros registradores s; a forma muda pouco */
/* the three counters under a card (icon, label strip, number) */
void *dt_draw_counters(void *list, ScPt pos, ScCounter *c, void *gim_strip, void *gim)
{
    short y = pos >> 16 & 0xFFFF;
    int xx = pos & 0xFFFF;
    short *px = dt_counter_xs;
    unsigned short *pw = dt_counter_ws;
    unsigned short off = 0;
    short x;
    int i;
    int ix;
    unsigned char *ic;

    if (gim_strip == 0 || gim == 0)
        return list;
    x = xx;
    for (i = 0, ix = x; i < 3; i++, c++, ix += 25, pw++, px++) {
        if (c->icon != 0) {
            list = dl_ambient_color(list, 0xFFFFFFFF);
            list = dl_set_tex_filter(list, 0, 0);
            ic = dt_count_icons[c->icon];
            list = gim_set_texture(list, gim);
            list = draw_set_offsets(list, ic[2], ic[3], ic[2], ic[3]);
            list = draw_sprite(list, ix, y, 0, ic[0], ic[1], 0, 0);
            list = gim_set_texture(list, gim_strip);
            list = draw_set_offsets(list, *pw, 14, *pw, 14);
            list = draw_sprite(list, x + 12 + off, y, 0, off, 234, 0, 0);
            font_set_size(10, 12);
            font_set_color(c->color);
            if (i == 2)
                list = dt_draw_num_text(list, (short)(x + *px), (short)(y + 1), c->value, 8, 2, 0);
            else
                list = dt_draw_num_text(list, (short)(x + *px), (short)(y + 1), c->value, 8, 1, 0);
        }
        off += *pw;
    }
    return list;
}
#endif

/* an icon with a text on its right */
void *dt_draw_icon_text(void *list, ScIconText *t, int x, int y)
{
    ScSprite s;
    short tx;
    short ty;

    s.x = x;
    s.y = y;
    s.w = t->w;
    s.h = t->h;
    s.u = t->u;
    s.v = t->v;
    font_text_set(dt_draw_sprite(list, &s, t->gim));
    font_set_size(12, 12);
    if (t->color != 0)
        font_set_color(t->color);
    else
        font_set_color(0xFF000000);
    tx = x + t->w + 4;
    ty = y + (t->h - 13) / 2;
    font_draw_text(tx << 6, ty << 6, (480 - tx) << 6, t->text);
    return font_text_clear();
}

/* scroll arrow (dir bit 0 = down, >= 2 flipped), its blinking highlight fades over period frames */
void *dt_draw_arrow(void *list, int x, int y, unsigned char dir, unsigned int period, int blink, int base, unsigned int t, void *gim)
{
    unsigned int half = period >> 1;
    int v;
    int flip;
    float a;

    if (dir & 1)
        v = 8;
    else
        v = 0;
    flip = !(dir < 2);
    list = gim_set_texture(list, gim);
    list = dl_set_tex_filter(list, 0, 0);
    if (base) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = draw_set_offsets(list, 8, 8, 8, 8);
        list = draw_sprite(list, x, y, 0, 119, v + 108, 0, flip);
    }
    if (blink == 0)
        return list;
    {
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
        list = draw_sprite(list, x, y, 0, 119, v + 60, 0, flip);
    }
    return list;
}

/* vertical scroll bar of h pixels: knob for pos out of total - first, arrows above and below */
void *dt_draw_scrollbar(void *list, int x, int y, int h, int pos, int first, int total, int frame, void *gim)
{
    int track = h - 2;
    int knob;
    int off;
    int range;
    int u;
    int bottom;
    int up;
    int down;
    int half;
    unsigned int fps;
    int ky;

    if (gim == 0)
        return list;
    if (total <= first) {
        knob = track;
        off = 0;
    } else {
        knob = track - total * 4 / 5;
        knob = ALLEGREX_MIN(ALLEGREX_MAX(knob, 10), track - 2);
        range = track - knob;
        off = pos * range / (total - first);
        off = ALLEGREX_MIN(ALLEGREX_MAX(off, 0), range);
    }
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 8, 5, 8, 5);
    list = draw_sprite(list, x, y, 0, 119, 0, 0, 0);
    bottom = y + h;
    list = draw_sprite(list, x, bottom - 5, 0, 119, 15, 0, 0);
    list = draw_set_offsets(list, 8, 5, 8, h - 10);
    list = draw_sprite(list, x, y + 5, 0, 119, 5, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, 5);
    half = frame / 2;
    u = frame % 2 * 4 + 119;
    ky = y + off;
    list = draw_sprite(list, x + 2, ky + 2, 0, u, (half + 1) * 20, 0, 0);
    list = draw_sprite(list, x + 2, knob + (ky - 3), 0, u, half * 20 + 35, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, knob - 10);
    list = draw_sprite(list, x + 2, ky + 7, 0, u, half * 20 + 25, 0, 0);
    up = 0;
    if (!(frame & 1) && pos > 0)
        up = 1;
    fps = gfx_get_state()[1];
    list = dt_draw_arrow(list, x, y - 7, 0, fps, up, 1, gfx_get_state()[0], gim);
    down = (frame & 1) == 0 && pos < total - first;
    fps = gfx_get_state()[1];
    return dt_draw_arrow(list, x, bottom + 2, 1, fps, down, 1, gfx_get_state()[0], gim);
}

/* adds d to s->pos, clamped to 0..s->max (0 when max is 0) */
#ifdef __MWERKS__
static asm void dt_scroll_add(ScScroll *s, int d)
{
    lw a3, 12(a0)
    bgez a1, @pos
    lw a2, 8(a0)
    negu v1, a1
    sltu v1, a2, v1
    bnezl v1, @store
    move v1, zero
@pos:
    bnezl a3, @clamp
    addu v1, a2, a1
    move v1, zero
    b @store
    nop
@clamp:
    sltu at, a3, v1
    beqz at, @store
    nop
    move v1, a3
@store:
    sw v1, 8(a0)
    jr ra
    nop
}
#else
static void dt_scroll_add(ScScroll *s, int d)
{
    unsigned int max = s->max;
    unsigned int pos = s->pos;
    unsigned int v;

    if (d < 0 && pos < -d)
        v = 0;
    else if (max != 0) {
        v = pos + d;
        if (v > max)
            v = max;
    } else
        v = 0;
    s->pos = v;
}
#endif

/* the analog stick scrolls s */
void dt_scroll_analog(ScScroll *s)
{
    PadState *pad = pad_get();

    if (s != 0) {
        if (pad->b.analog_y > 158)
            dt_scroll_add(s, (pad->b.analog_y - 158) * 4);
        if (pad->b.analog_y < 98)
            dt_scroll_add(s, (pad->b.analog_y - 98) * 4);
    }
}

/* 9-piece box */
void *dt_draw_box(void *list, int x, int y, int w, int h, void *gim)
{
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 4, 4, 4, 4);
    list = draw_sprite(list, x, y, 0, 0, 54, 0, 0);
    list = draw_set_offsets(list, 1, 4, w - 9, 4);
    list = draw_sprite(list, x + 4, y, 0, 4, 54, 0, 0);
    list = draw_set_offsets(list, 5, 4, 5, 4);
    list = draw_sprite(list, x + w - 5, y, 0, 5, 54, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, 5);
    list = draw_sprite(list, x, y + h - 5, 0, 0, 59, 0, 0);
    list = draw_set_offsets(list, 1, 5, w - 9, 5);
    list = draw_sprite(list, x + 4, y + h - 5, 0, 4, 59, 0, 0);
    list = draw_set_offsets(list, 5, 5, 5, 5);
    list = draw_sprite(list, x + w - 5, y + h - 5, 0, 5, 59, 0, 0);
    list = draw_set_offsets(list, 4, 1, 4, h - 9);
    list = draw_sprite(list, x, y + 4, 0, 0, 58, 0, 0);
    list = draw_set_offsets(list, 5, 1, 5, h - 9);
    list = draw_sprite(list, x + w - 5, y + 4, 0, 5, 58, 0, 0);
    list = draw_set_offsets(list, 1, 1, w - 9, h - 9);
    return draw_sprite(list, x + 4, y + 4, 0, 4, 58, 0, 0);
}

/* button help string ids (strTbl.bin) for the two help windows */
unsigned short dt_help_text[2][6] = {
    { 0x7A, 0x74, 0x71, 0x72, 0x73, 0 },
    { 0x79, 0x77, 0x75, 0x78, 0x76, 0 },
};

unsigned short dt_help_plus[2] = { 0xFF0B, 0 };   /* "+" (template of a local of dt_draw_help) */

#ifdef NON_MATCHING /* (y + 7 + lh) + 12 do original não é reassociado, e o mwcc atual dobra as constantes */
/* the button help window: flags select the lines (bit 0 title, 0x1000/0x2000/0x4000/0x8000 buttons) */
void *dt_draw_help(void *list, int x, int y, int alt, unsigned int flags, void *gim, void *strtbl)
{
    unsigned short *ids;
    int lh;
    int h;
    unsigned short *text;
    int tx;
    int ty;
    unsigned char fit;

    if (alt != 0)
        ids = dt_help_text[1];
    else
        ids = dt_help_text[0];
    font_set_color(0xFFFFFFFF);
    font_set_size(12, 12);
    font_set_flag4(0);
    lh = font_line_height() >> 6;
    h = 0x58;
    if (!(flags & 1))
        h = 0x58 - lh;
    list = dt_draw_box(list, x, y, 0xD4, h, gim);
    if (gim != 0) {
        list = gim_set_texture(list, gim);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 0x2F, 0x26, 0x2F, 0x26);
        list = draw_sprite(list, x + 0x52, y + 7 + lh, 0, 0, 0, 0, 0);
    }
    if ((flags & 1) && (text = table_get_entry(strtbl, ids[0])) != 0) {
        font_set_size(12, 12);
        tx = x + ((unsigned int)(0xD4 - (font_measure_text(text) >> 6) - 0x1C) >> 1);
        ty = y + 0x51 - lh;
        if (gim != 0) {
            list = draw_set_offsets(list, 0x1A, 0xB, 0x1A, 0xB);
            list = draw_sprite(list, tx, ty + 2, 0, 0x5E, 0xB, 0, 0);
        }
        font_text_set(list);
        tx += 0x1C;
        font_draw_text(tx << 6, (ty + 1) << 6, (480 - tx) << 6, text);
        list = font_text_clear();
    }
    font_text_set(list);
    if (flags & 0x1000) {
        text = table_get_entry(strtbl, ids[4]);
        font_set_size(12, 12);
        if (text != 0) {
            tx = x + ((0xD4u - (font_measure_text(text) >> 6)) >> 1);
            font_draw_text(tx << 6, (y + 5) << 6, (480 - tx) << 6, text);
        }
    }
    if (flags & 0x4000) {
        if (alt != 0 && region_pick_value(0) != 0x4000)
            text = table_get_entry(strtbl, ids[3]);
        else
            text = table_get_entry(strtbl, ids[1]);
        font_set_size(12, 12);
        if (text != 0) {
            tx = x + ((0xD4u - (font_measure_text(text) >> 6)) >> 1);
            font_draw_text(tx << 6, (y + 0x50 - lh * 2) << 6, (480 - tx) << 6, text);
        }
    }
    if (flags & 0x8000) {
        text = table_get_entry(strtbl, ids[2]);
        font_set_size(12, 12);
        if (text != 0) {
            fit = font_measure_wtext(0x1340, text);
            if (fit < 12) {
                font_draw_wtext_ex((x + 5) << 6, (y + 7 + lh + 6) << 6, 0x1340, 0xC00, text, -1);
            } else {
                font_set_size(fit, 12);
                tx = x + 5 + ((0x4Du - (font_measure_text(text) >> 6)) >> 1);
                font_draw_text(tx << 6, (y + 7 + lh + 12) << 6, (480 - tx) << 6, text);
            }
        }
    }
    if (flags & 0x2000) {
        if (alt != 0 && region_pick_value(0) != 0x4000)
            text = table_get_entry(strtbl, ids[1]);
        else
            text = table_get_entry(strtbl, ids[3]);
        font_set_size(12, 12);
        if (text != 0) {
            fit = font_measure_wtext(0x1340, text);
            if (fit < 12) {
                font_draw_wtext_ex((x + 0x81) << 6, (y + 7 + lh + 6) << 6, 0x1340, 0xC00, text, -1);
            } else {
                font_set_size(fit, 12);
                tx = x + 0x81 + ((0x4Du - (font_measure_text(text) >> 6)) >> 1);
                font_draw_text(tx << 6, (y + 7 + lh + 12) << 6, (480 - tx) << 6, text);
            }
        }
    }
    list = font_text_clear();
    font_set_flag4(1);
    if (gim != 0) {
        unsigned short *plus = dt_help_plus;

        list = gim_set_texture(list, gim);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 4, 0x10, 4, 0x10);
        list = draw_sprite(list, x, y - 0xE, 0, 0, 0x26, 0, 0);
        list = draw_set_offsets(list, 1, 0x10, 0x24, 0x10);
        list = draw_sprite(list, x + 4, y - 0xE, 0, 4, 0x26, 0, 0);
        list = draw_set_offsets(list, 9, 0x10, 9, 0x10);
        list = draw_sprite(list, x + 0x28, y - 0xE, 0, 5, 0x26, 0, 0);
        list = draw_set_offsets(list, 0x17, 0xB, 0x17, 0xB);
        if (alt == 0)
            list = draw_sprite(list, x + 5, y - 0xB, 0, 0x5E, 0x16, 0, 0);
        else
            list = draw_sprite(list, x + 5, y - 0xB, 0, 0x5E, 0x21, 0, 0);
        font_text_set(list);
        font_set_size(12, 12);
        font_draw_text((x + 0x1D) << 6, (y - 0xB) << 6, (480 - (x + 0x1D)) << 6, plus);
        list = font_text_clear();
    }
    return list;
}
#endif
