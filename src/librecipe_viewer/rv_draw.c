// @range 0x08804918-0x08805874
// @asm 0x08804918-0x08804AC0
// @asm 0x088057BC-0x08805874
// @data 0x0880E888-0x0880E88C
/* Drawing helpers of the screen: sprite digits, numbers, the 9-slice window frame, sprites from a
   record, card pictures (alternate art from the CIP pack) with their marks (copies, limit, new),
   labelled buttons, scroll arrows and the analog stick scrolling of the card text. Most of it is
   the same code as liblimitlist's ll_draw.c. */
#include "librecipe_viewer.h"
#include "allegrex.h"

#define RV_XY(x, y) ((unsigned short)(short)(x) | (unsigned short)(short)(y) << 16)

typedef struct RvSprite {      /* sprite of a sheet: position, size, source */
    unsigned short x, y;
    unsigned short w, h;
    unsigned short u, v;
} RvSprite;

typedef struct RvLabel {       /* a button sprite with a text to its right */
    void *gim;
    unsigned char u, v, w, h;  /* +0x04 */
    unsigned short *text;      /* +0x08 */
    unsigned int color;        /* +0x0C text color, 0 = black */
} RvLabel;

void *gim_set_texture_frame(void *list, void *g, int frame);
int inv_get_bit3(unsigned short id);
int inv_get_hi(unsigned short id);
int card_get_limit(int id);
int rv_pair_find(unsigned short a, unsigned short b, int *tab);

void *rv_draw_digits(void *list, int pos, int zeros, unsigned int n, unsigned short fmt, unsigned int color, void *gim);
void *rv_draw_sprite(void *list, RvSprite *s, void *gim);
int rv_card_pict(unsigned short *page, unsigned short *u, unsigned short *v, RvEntry *e, int *alt);

#ifdef NON_MATCHING /* forma igual; a largura w e x/y ficam em outros $s em todas as ordens de declaração testadas */
/* n with digit sprites (7 x 9) from the right; fmt = style << 8 | digits */
void *rv_draw_digits(void *list, int pos, int zeros, unsigned int n, unsigned short fmt, unsigned int color, void *gim)
{
    int w;
    short x = pos & 0xFFFF;
    short y = pos >> 16 & 0xFFFF;
    unsigned char style = fmt >> 8 & 0xFF;
    unsigned char digits = fmt & 0xFF;
    int gap = 0;

    int i;

    if (gim == 0)
        return list;
    list = dl_ambient_color(list, color);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    switch (style) {
    case 3:
        gap = -1;
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
    gap = w + gap;
    x += (short)((digits - 1) * gap);
    for (i = 0; i < digits; i++) {
        list = draw_sprite(list, x, y, 0, n % 10 * w, 0, 0, 0);
        x -= (short)gap;
        n /= 10;
        if (!zeros && n == 0)
            break;
    }
    return list;
}
#endif

/* n right-aligned at x (digits from the right, `step` apart); leading zeros when `zeros` */
void *rv_draw_number(void *list, short x, short y, int n, int step, unsigned short digits, int zeros)
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
void *rv_draw_frame(void *list, int x, int y, int w, int h, void *gim, unsigned int flags)
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

void *rv_draw_sprite(void *list, RvSprite *s, void *gim)
{
    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, s->w, s->h, s->w, s->h);
    return draw_sprite(list, (short)s->x, (short)s->y, 0, s->u, s->v, 0, 0);
}

int rv_card_pict(unsigned short *page, unsigned short *u, unsigned short *v, RvEntry *e, int *alt)
{
    if (inv_get_bit3(e->id)) {
        int k = inv_get_hi(e->id);
        if (k > 0) {
            int i = rv_pair_find(e->id, k, alt);
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
void *rv_draw_card(void *list, int x, int y, int shadow, unsigned int color, RvEntry *e, void *gim,
                   void *alt_gim, int *alt)
{
    unsigned short page;
    unsigned short u;
    unsigned short v;

    alt_gim = rv_card_pict(&page, &u, &v, e, alt) ? alt_gim : gim;
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

/* marks over a card picture: copies owned (2..99), the limit icon (or the blinking "new" mark),
   and the "new" or deck-slot (1..3) badge */
void *rv_draw_marks(void *list, int x, int y, RvEntry *e, int show_count, int blink, int alt, unsigned short n,
                    void *digits_gim, void *icons_gim)
{
    RvSprite s;

    if (show_count) {
        int c = e->lines;
        if (c > 1) {
            if (c >= 100)
                c = 99;
            list = rv_draw_digits(list, RV_XY(x + 8, y + 21), 0, c, 2, 0xFFFFFFFF, digits_gim);
        }
    }
    if (!e->flag7) {
        if (blink && (gfx_get_state()->frame >> 3 & 1)) {
            s.x = x;
            s.y = y;
            s.w = 17;
            s.h = 12;
            s.u = 37;
            s.v = 76;
            list = rv_draw_sprite(list, &s, icons_gim);
            goto badge;
        }
    }
    {
        int lim = card_get_limit(e->id);
        if (lim < 3) {
            s.x = x;
            s.y = y;
            s.w = 12;
            s.h = 12;
            s.u = lim * 12;
            s.v = 76;
            list = rv_draw_sprite(list, &s, icons_gim);
        }
    }
badge:
    if (alt && e->flag7) {
        s.x = x + 12;
        s.y = y + 20;
        s.w = 10;
        s.h = 10;
        s.u = 37;
        s.v = 65;
        list = rv_draw_sprite(list, &s, icons_gim);
    } else if (n > 0 && n < 4) {
        s.x = x + 10;
        s.y = y + 18;
        s.w = 12;
        s.h = 12;
        s.u = (n - 1) * 12;
        s.v = 88;
        list = rv_draw_sprite(list, &s, icons_gim);
    }
    return list;
}

void *rv_draw_label(void *list, RvLabel *b, int x, int y)
{
    RvSprite s;
    short tx;

    s.x = x;
    s.y = y;
    s.w = b->w;
    s.h = b->h;
    s.u = b->u;
    s.v = b->v;
    font_text_set(rv_draw_sprite(list, &s, b->gim));
    font_set_size(12, 12);
    if (b->color)
        font_set_color(b->color);
    else
        font_set_color(0xFF000000);
    tx = x + b->w + 4;
    font_draw_text(tx << 6, (short)(y + (b->h - 13) / 2) << 6, (480 - tx) << 6, b->text);
    return font_text_clear();
}

/* scroll arrow (kind bit 0: down, kind >= 2: mirrored) with an optional blinking overlay */
void *rv_draw_arrow(void *list, int x, int y, unsigned char kind, unsigned int period, int blink, int base,
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

#ifdef NON_MATCHING /* o original deixa vazios os atrasos do `b` e do `beqz` e lê `max` antes de `pos`; rv_scroll_pad depende de rv_scroll_add ser static compilada antes (mantém o PadState em $v0 através da chamada) */
static void rv_scroll_add(RvScroll *s, int d)
{
    unsigned int max = s->max;
    unsigned int v;

    if (d >= 0 || s->pos >= (unsigned int)-d) {
        if (max == 0)
            v = 0;
        else {
            v = s->pos + d;
            if (v > max)
                v = max;
        }
    } else {
        v = 0;
    }
    s->pos = v;
}

void rv_scroll_pad(RvScroll *s)
{
    PadState *pad = pad_get();

    if (s) {
        if (pad->b.analog_y > 0x9E)
            rv_scroll_add(s, (pad->b.analog_y - 0x9E) * 4);
        if (pad->b.analog_y < 0x62)
            rv_scroll_add(s, (pad->b.analog_y - 0x62) * 4);
    }
}
#endif
