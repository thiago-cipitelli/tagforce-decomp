// @range 0x088103A0-0x08810CD0
// @asm 0x088107C0-0x08810CD0
/* Small drawing helpers: the card icons (type, attribute, kind, limit, pack) at a position, a
   button icon with its label, the shade over a pane and its list tab, and the bubble beside the
   scroll bar with the sort value of the cursor's card. */
#include "libdeck.h"

#define PT(x, y) ((unsigned short)(x) | (unsigned short)(y) << 16)
#define DK_ENT(l, i) ((i) >= (l)->count ? 0 : &(l)->ents[(l)->order[i]])

typedef struct DkButton {              /* an icon of a gim and a label of strTbl.bin */
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} DkButton;

typedef struct DkIconText {            /* dk_draw_icon_text */
    void *gim;
    unsigned char u, v, w, h;          /* +0x04 */
    unsigned short *text;              /* +0x08 */
    unsigned int color;                /* +0x0C 0 = black */
} DkIconText;

void *dk_draw_icon_a(void *list, int pos, unsigned short id, void *gim);
void *dk_draw_icon_b(void *list, int pos, unsigned short id, void *gim);
void *dk_draw_icon_c(void *list, int pos, unsigned short id, void *gim);
void *dk_draw_icon_d(void *list, int pos, unsigned short id, void *gim);
void *dk_draw_icon_e(void *list, int pos, unsigned short id, void *gim);
void *dk_draw_icon_text(void *list, DkIconText *t, int x, int y);
unsigned int dk_tab_pos(unsigned short tab);

void *dk_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type)
{
    return dk_draw_icon_a(list, x | y << 16, type, dk_res_get(0x8007));
}

void *dk_draw_attr_icon(void *list, unsigned short x, unsigned short y, unsigned short attr)
{
    return dk_draw_icon_b(list, x | y << 16, attr, dk_res_get(0x8007));
}

void *dk_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind)
{
    return dk_draw_icon_c(list, x | y << 16, kind, dk_res_get(0x8007));
}

void *dk_draw_limit_icon(void *list, unsigned short x, unsigned short y, unsigned short n)
{
    return dk_draw_icon_d(list, x | y << 16, n, dk_res_get(0x800A));
}

void *dk_draw_pack_icon(void *list, unsigned short x, unsigned short y, unsigned short n)
{
    return dk_draw_icon_e(list, x | y << 16, n, dk_res_get(0x800A));
}

/* a button icon and its label */
void *dk_draw_button(void *list, DkButton *b, int x, int y, unsigned int color)
{
    DkIconText t;

    memset(&t, 0, sizeof(t));
    t.w = 0x11;
    t.h = 0x11;
    t.u = b->u;
    t.v = b->v;
    t.gim = dk_res_get(b->res);
    t.text = table_get_entry(dk_res_get(0x8033), b->text);
    t.color = color;
    return dk_draw_icon_text(list, &t, x, y);
}

/* darkens pane side and, with show_tab, its list tab */
void *dk_draw_shade(void *list, unsigned short side, unsigned short tab, int show_tab)
{
    DkLayout *lay = &dkv.pane[side].lay;
    unsigned int pos = dk_tab_pos(tab);

    list = dl_disable(list, 9);
    list = dl_tex_func(list, 4, 0);
    list = dl_ambient_color(list, 0x80000000);
    list = draw_set_offsets(list, 0, 0, lay->w, lay->h);
    list = draw_sprite(list, lay->x, lay->y, 0, 0, 0, 0, 0);
    if (show_tab) {
        list = draw_set_offsets(list, 0, 0, pos >> 16 & 0xFFFF, 15);
        list = draw_sprite(list, lay->x + (pos & 0xFFFF), 23, 0, 0, 0, 0, 0);
    }
    list = dl_enable(list, 9);
    list = dl_tex_func(list, 0, 1);
    return dl_ambient_color(list, 0xFFFFFFFF);
}

#ifdef NON_MATCHING /* switch com tamanhos indefinidos e campos por índice; alocação diferente (como sc_draw_detail) */
unsigned int font_sample_glyph_size(void);
unsigned short card_top_get_letter(int v);
int font_measure_text(unsigned short *text);
void font_text_set(void *list);
void *font_text_clear(void);
unsigned int dk_sort_value(DkEnt *e, unsigned short sort);
unsigned short dk_kind_icon(unsigned short kind);
void *dk_draw_box(void *list, int x, int y, int w, int h, void *gim);
void *dk_draw_level(void *list, int pos, unsigned short level, unsigned int color, void *gim);
void *dk_draw_stat(void *list, unsigned int st, int pos, unsigned short size, unsigned int color, void *gim);

/* the value of the sort key of the cursor's card, in a bubble beside the scroll bar */
void *dk_draw_detail(void *list)
{
    unsigned int side = dkv.side;
    DkList *l = dkv.pane[side].list;
    unsigned short i = dk_cur_index(side);
    DkEnt *e = DK_ENT(l, i);
    unsigned char sort = l->sort;
    unsigned int v;
    int w;
    int h;
    unsigned int cols;
    int rows;
    int x;
    int y;
    unsigned int sz;
    int lw;

    if (e == 0)
        return list;
    v = dk_sort_value(e, sort);
    switch (sort) {
    case 3:
        w = 0x27;
        h = 0x14;
        break;
    case 4:
    case 5:
        w = 0x30;
        h = 0x13;
        break;
    case 1:
        font_set_size(14, 14);
        sz = font_sample_glyph_size();
        w = (sz >> 22) + 13;
        h = (sz >> 22) + 11;
        break;
    case 2:
    case 6:
    case 7:
    case 8:
    case 9:
        w = 0x1B;
        h = 0x1B;
        break;
    }
    cols = dkv.pane[side].lay.cols;
    rows = (int)(l->count + cols - 1) / (int)cols;
    x = dkv.pane[side].lay.x + dkv.pane[side].lay.w - 10 - w;
    if (rows == 0)
        y = dkv.pane[side].lay.y + 12 + (dkv.pane[side].lay.h - 0x2B - h) / 2;
    else
        y = dkv.pane[side].lay.y + 12 + (dkv.pane[side].lay.h - 0x2B) * (i / cols) / rows - h / 2;
    list = dk_draw_box(list, x, y, w, h, dk_res_get(0x8013));
    if (v == 0xFFFFFFFE)
        return list;
    switch (sort) {
    case 0:
        break;
    case 1: {
        unsigned short ch[2] = { 0 };

        font_text_set(list);
        font_set_size(14, 14);
        font_set_color(0xFFFFFFFF);
        ch[0] = card_top_get_letter(v);
        lw = font_measure_text(ch);
        font_draw_text((x + 4 + (w - 9) / 2) * 64 - lw / 2, (y + 4) * 64, w << 6, ch);
        list = font_text_clear();
        break;
    }
    case 2:
        list = dk_draw_limit_icon(list, x + 5, y + 5, dk_kind_icon(v));
        break;
    case 9:
        list = dk_draw_pack_icon(list, x + 5, y + 5, v);
        break;
    case 3:
        list = dk_draw_level(list, PT(x + 5, y + 4), v, 0xFFFFFFFF, dk_res_get(0x8009));
        break;
    case 4:
        if ((int)v > 0)
            v = (int)v / 100 * 100;
        else if (v == 0xFFFFFFFF)
            v = 0xFFFF;
        list = dk_draw_stat(list, v << 16 | 2, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, dk_res_get(0x8009));
        break;
    case 5:
        if ((int)v > 0)
            v = (int)v / 100 * 100;
        else if (v == 0xFFFFFFFF)
            v = 0xFFFF;
        list = dk_draw_stat(list, v << 16 | 3, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, dk_res_get(0x8009));
        break;
    case 8:
        list = dk_draw_attr_icon(list, x + 5, y + 5, v);
        break;
    case 6:
        list = dk_draw_type_icon(list, x + 5, y + 5, v);
        break;
    case 7:
        list = dk_draw_kind_icon(list, x + 5, y + 5, v);
        break;
    }
    return list;
}
#endif
