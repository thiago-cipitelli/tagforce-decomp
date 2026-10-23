// @range 0x08804128-0x08805948
// @data 0x0880D230-0x0880D2B4
/* Drawing helpers of the screen: numbers, the 9-slice window frame, sprites from a record,
   attribute/type/level icons, ATK/DEF values, card pictures (alternate art from the CIP pack),
   card kind banners, the limit counters, scroll arrows and the scrollbar. */
#include "liblimitlist.h"
#include "allegrex.h"

typedef struct LlSprite {      /* sprite of a sheet: position, size, source */
    unsigned short x, y;
    unsigned short w, h;
    unsigned short u, v;
} LlSprite;

typedef struct LlCounter {     /* one of the three counters (12 bytes) */
    unsigned char icon;        /* ll_counter_icons index, 0 = hidden */
    unsigned char pad1[3];
    unsigned int color;        /* +0x04 */
    unsigned char value;       /* +0x08 */
    unsigned char pad9[3];
} LlCounter;

void *gim_set_texture_frame(void *list, void *g, int frame);
int inv_get_bit3(unsigned short id);
int inv_get_hi(unsigned short id);
int ll_pair_find(unsigned short a, unsigned short b, int *tab);
int ll_kind_icon(unsigned short kind);

void *ll_draw_sprite(void *list, LlSprite *s, void *gim);
int ll_card_pict(unsigned short *page, unsigned short *u, unsigned short *v, LlEntry *e, int *alt);
void *ll_draw_arrow(void *list, int x, int y, unsigned char kind, unsigned int period, int blink, int base,
                    unsigned int time, void *gim);

/* n right-aligned at x (digits from the right, `step` apart); leading zeros when `zeros` */
void *ll_draw_number(void *list, short x, short y, int n, int step, unsigned short digits, int zeros)
{
    short txt[2] = { '0', 0 };
    int px = (unsigned short)x;
    int i;

    font_text_set(list);
    for (i = digits - 1; i >= 0; i--) {
        px = (unsigned short)(px - step);
        txt[0] = n % 10 + '0';
        font_draw_text(px << 6, y << 6, (480 - px) << 6, (unsigned short *)txt);
        n /= 10;
        if (!zeros && n == 0)
            break;
    }
    return font_text_clear();
}

/* 9-slice window frame around (x, y, w, h); flags bit 3 widens it by one pixel per side */
void *ll_draw_frame(void *list, int x, int y, int w, int h, void *gim, unsigned int flags)
{
    int e;
    int d;
    int y0;
    int x0;
    int y1;
    int x1;
    int m;

    if (gim == 0)
        return list;
    if (w <= 0 || h <= 0)
        return list;
    e = (flags >> 3 & 1) * 2;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 6, 6, 6, 6);
    d = e / 2;
    y0 = y - 4 - d;
    x0 = x - 4 - d;
    list = draw_sprite(list, x0, y0, 0, 213, 122, 0, 0);
    x1 = d + (x + w - 2);
    list = draw_sprite(list, x1, y0, 0, 250, 122, 0, 0);
    y1 = d + (y + h - 2);
    list = draw_sprite(list, x0, y1, 0, 213, 159, 0, 0);
    list = draw_sprite(list, x1, y1, 0, 250, 159, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, h + e - 4);
    m = y + 2 - d;
    list = draw_sprite(list, x0, m, 0, 213, 128, 0, 0);
    list = draw_sprite(list, x1, m, 0, 250, 128, 0, 0);
    list = draw_set_offsets(list, 1, 6, w + e - 4, 6);
    m = x + 2 - d;
    list = draw_sprite(list, m, y0, 0, 219, 122, 0, 0);
    return draw_sprite(list, m, y1, 0, 219, 159, 0, 0);
}

void *ll_draw_sprite(void *list, LlSprite *s, void *gim)
{
    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, s->w, s->h, s->w, s->h);
    return draw_sprite(list, (short)s->x, (short)s->y, 0, s->u, s->v, 0, 0);
}

/* attribute icon (1-based, 7 per row of 17 x 17) */
void *ll_draw_attr(void *list, int pos, unsigned short icon, void *gim)
{
    LlSprite s = { pos, pos >> 16, 17, 17, 0, 0 };

    if (icon == 0)
        return list;
    s.u = (unsigned short)(icon - 1) % 7 * 17;
    s.v = (unsigned short)(icon - 1) / 7 * 17;
    return ll_draw_sprite(list, &s, gim);
}

/* spell/trap type icon (row at v 51) */
void *ll_draw_type(void *list, int pos, unsigned short icon, void *gim)
{
    LlSprite s = { pos, pos >> 16, 17, 17, 0, 0 };

    if (icon == 0)
        return list;
    s.u = (unsigned short)(icon - 1) * 17;
    s.v = 51;
    return ll_draw_sprite(list, &s, gim);
}

/* monster type icon (rows from v 68); 9 and 8 share the icons of 13 and 7 */
void *ll_draw_race(void *list, int pos, unsigned short icon, void *gim)
{
    LlSprite s = { pos, pos >> 16, 17, 17, 0, 0 };

    if (icon == 0)
        return list;
    switch (icon) {
    case 8:
        icon = 7;
        break;
    case 9:
        icon = 13;
        break;
    }
    s.u = (unsigned short)(icon - 1) % 7 * 17;
    s.v = (unsigned short)(icon - 1) / 7 * 17 + 68;
    return ll_draw_sprite(list, &s, gim);
}

/* limit badge: frame and the number n (1-based cell of 19 x 11, 6 per row) */
void *ll_draw_badge(void *list, int pos, unsigned short n, unsigned int color, void *gim)
{
    short x = pos & 0xFFFF;
    short y = pos >> 16 & 0xFFFF;
    unsigned short i;

    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 9, 9, 9, 9);
    list = draw_sprite(list, x, y + 2, 0, 18, 83, 0, 0);
    list = dl_ambient_color(list, color);
    list = draw_set_offsets(list, 19, 11, 19, 11);
    i = n - 1;
    return draw_sprite(list, x + 10, y + 1, 0, (unsigned short)(i % 6 * 19), (unsigned short)(i / 6 * 11 + 45), 0, 0);
}

unsigned short ll_atkdef[2][5] = {
    { 'A', 'T', 'K', '/', 0 },
    { 'D', 'E', 'F', '/', 0 },
};

/* "ATK/" or "DEF/" or the level/rank star, then a 4-digit value ("?" for 0xFFFF) */
void *ll_draw_stat(void *list, int stat, int pos, unsigned short size, unsigned int color, void *gim)
{
    short px = pos & 0xFFFF;
    short py = pos >> 16 & 0xFFFF;
    unsigned short kind = stat & 0xFFFF;
    unsigned short value = stat >> 16 & 0xFFFF;
    short txt[2] = { '0', 0 };
    int h = size >> 8 & 0xFF;
    int w;
    int x;

    font_set_size(size & 0xFF, h);
    font_set_color(color);
    switch (kind) {
    case 0:
    case 1: {
        unsigned short *s;
        font_text_set(list);
        s = ll_atkdef[kind];
        w = font_measure_text(s) >> 6;
        font_draw_text(px << 6, py << 6, (480 - px) << 6, s);
        list = font_text_clear();
        break;
    }
    case 2: {
        LlSprite s = { px, py, 9, 9, 0, 83 };
        s.y += (unsigned short)((h - 8) / 2);
        list = ll_draw_sprite(list, &s, gim);
        w = 10;
        break;
    }
    case 3: {
        LlSprite s = { px, py, 9, 9, 9, 83 };
        s.y += (unsigned short)((h - 8) / 2);
        list = ll_draw_sprite(list, &s, gim);
        w = 10;
        break;
    }
    }
    x = px + w;
    font_text_set(list);
    if (value == 0xFFFF) {
        txt[0] = '?';
        x += ((font_measure_text((unsigned short *)txt) >> 6) + 1) * 3;
        font_draw_text(x << 6, py << 6, (480 - x) << 6, (unsigned short *)txt);
    } else {
        int cw = (font_measure_text((unsigned short *)txt) >> 6) + 1;
        int i;

        x += cw * 3;
        for (i = 3; i >= 0; i--) {
            int d = value % 10;
            if (i == 3 || value != 0) {
                txt[0] = d + '0';
                font_draw_text(x << 6, py << 6, (480 - x) << 6, (unsigned short *)txt);
            }
            value /= 10;
            x -= cw;
        }
    }
    return font_text_clear();
}

/* picture of the card: its cell in the deck/all sheets, or the alternate art (page 0) when the
   player owns one; 1 for the alternate art */
int ll_card_pict(unsigned short *page, unsigned short *u, unsigned short *v, LlEntry *e, int *alt)
{
    if (inv_get_bit3(e->id)) {
        int k = inv_get_hi(e->id);
        if (k > 0) {
            int i = ll_pair_find(e->id, k, alt);
            if (i >= 0) {
                *page = 0;
                *u = i % 23 * 22;
                *v = i / 23 * 30;
                return 1;
            }
        }
    }
    {
        int f = e->file;
        unsigned short r;
        *page = f / 391;
        r = f % 391;
        *u = r % 23 * 22;
        *v = r / 23 * 30;
    }
    return 0;
}

/* card picture (22 x 30) with an optional drop shadow */
void *ll_draw_card(void *list, int x, int y, int shadow, unsigned int color, LlEntry *e, void *gim,
                   void *alt_gim, int *alt)
{
    unsigned short page;
    unsigned short u;
    unsigned short v;

    alt_gim = ll_card_pict(&page, &u, &v, e, alt) ? alt_gim : gim;
    if (alt_gim == 0)
        return list;
    list = gim_set_texture_frame(list, alt_gim, page);
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

/* card kind banner, in three widths */
void *ll_draw_kind(void *list, int pos, unsigned short width, unsigned short kind, void *gim)
{
    short x = pos & 0xFFFF;
    short y = pos >> 16 & 0xFFFF;
    int i;

    if (gim == 0)
        return list;
    i = ll_kind_icon(kind);
    if (i < 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    if (width >= 250) {
        list = draw_set_offsets(list, 104, 31, 104, 31);
        list = draw_sprite(list, x, y, 0, 0, i << 5, 0, 0);
        list = draw_set_offsets(list, 191, 31, 191, 31);
        list = draw_sprite(list, x + 104, y, 0, 12, i << 5, 0, 0);
    } else if (width < 150) {
        list = draw_set_offsets(list, 73, 13, 73, 13);
        list = draw_sprite(list, x, y, 0, i / 3 * 80, i % 3 * 14 + 192, 0, 0);
    } else {
        list = draw_set_offsets(list, 203, 31, 203, 31);
        list = draw_sprite(list, x, y, 0, 0, i << 5, 0, 0);
    }
    return list;
}

static unsigned char ll_counter_icons[7][4] = {   /* u, v, w, h in the icon sheet */
    { 0, 0, 0, 0 },
    { 0, 88, 12, 12 },
    { 12, 88, 12, 12 },
    { 24, 88, 12, 12 },
    { 36, 88, 12, 12 },
    { 36, 64, 12, 12 },
    { 48, 52, 12, 12 },
};

/* the three counters (icon, bar segment, number) */
void *ll_draw_counters(void *list, int pos, LlCounter *c, void *bar_gim, void *icon_gim)
{
    unsigned short bx;
    int i;
    short x = pos & 0xFFFF;
    unsigned short offs[3] = { 0x17, 0x30, 0x50 };
    short y = pos >> 16 & 0xFFFF;
    LlCounter *p;
    unsigned short widths[3] = { 0x18, 0x1A, 0x14 };

    bx = 0;
    if (bar_gim == 0 || icon_gim == 0)
        return list;
    p = c;
    for (i = 0; i < 3; i++) {
        if (p->icon != 0) {
            unsigned char *q;

            list = dl_ambient_color(list, 0xFFFFFFFF);
            list = dl_set_tex_filter(list, 0, 0);
            q = ll_counter_icons[p->icon];
            list = gim_set_texture(list, icon_gim);
            list = draw_set_offsets(list, q[2], q[3], q[2], q[3]);
            list = draw_sprite(list, x + i * 25, y, 0, q[0], q[1], 0, 0);
            list = gim_set_texture(list, bar_gim);
            list = draw_set_offsets(list, widths[i], 14, widths[i], 14);
            list = draw_sprite(list, x + 12 + bx, y, 0, bx, 234, 0, 0);
            font_set_size(10, 12);
            font_set_color(p->color);
            if (i == 2)
                list = ll_draw_number(list, x + offs[i], y + 1, p->value, 8, 2, 0);
            else
                list = ll_draw_number(list, x + offs[i], y + 1, p->value, 8, 1, 0);
        }
        bx += widths[i];
        p++;
    }
    return list;
}

/* scroll arrow (kind bit 0: down, kind >= 2: mirrored) with an optional blinking overlay */
void *ll_draw_arrow(void *list, int x, int y, unsigned char kind, unsigned int period, int blink, int base,
                    unsigned int time, void *gim)
{
    int v;
    unsigned int half = period >> 1;
    int flip;
    float a;

    if (kind & 1)
        v = 8;
    else
        v = 0;
    flip = kind >= 2;
    list = gim_set_texture(list, gim);
    list = dl_set_tex_filter(list, 0, 0);
    if (base) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = draw_set_offsets(list, 8, 8, 8, 8);
        list = draw_sprite(list, x, y, 0, 119, v + 108, 0, flip);
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
    return draw_sprite(list, x, y, 0, 119, v + 60, 0, flip);
}

/* scrollbar with its two arrows; style picks the knob color, odd styles never blink */
void *ll_draw_scrollbar(void *list, int x, int y, int h, int pos, int page, int total, int style, void *gim)
{
    int track = h - 2;
    int knob;
    int u;
    int k;
    int odd;
    int blink;
    int period;

    if (gim == 0)
        return list;
    if (total <= page) {
        knob = track;
        track = 0;
    } else {
        knob = ALLEGREX_MIN(ALLEGREX_MAX(track - total * 4 / 5, 10), track - 2);
        track = ALLEGREX_MIN(ALLEGREX_MAX(pos * (track - knob) / (total - page), 0), track - knob);
    }
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 8, 5, 8, 5);
    list = draw_sprite(list, x, y, 0, 119, 0, 0, 0);
    list = draw_sprite(list, x, y + h - 5, 0, 119, 15, 0, 0);
    list = draw_set_offsets(list, 8, 5, 8, h - 10);
    list = draw_sprite(list, x, y + 5, 0, 119, 5, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, 5);
    k = style / 2;
    u = style % 2 * 4 + 119;
    list = draw_sprite(list, x + 2, y + track + 2, 0, u, (k + 1) * 20, 0, 0);
    list = draw_sprite(list, x + 2, knob + (y + track - 3), 0, u, k * 20 + 35, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, knob - 10);
    list = draw_sprite(list, x + 2, y + track + 7, 0, u, k * 20 + 25, 0, 0);
    odd = style & 1;
    blink = 0;
    if (!odd && pos > 0)
        blink = 1;
    period = gfx_get_state()->fps;
    list = ll_draw_arrow(list, x, y - 7, 0, period, blink, 1, gfx_get_state()->frame, gim);
    blink = odd == 0 && pos < total - page;
    period = gfx_get_state()->fps;
    return ll_draw_arrow(list, x, y + h + 2, 1, period, blink, 1, gfx_get_state()->frame, gim);
}
