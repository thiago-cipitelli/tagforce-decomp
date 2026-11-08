// @range 0x08806F0C-0x08807828
// @rodata 0x08819C2C-0x08819C7C
/* The icon wrappers (type, attribute, kind, limit, pack), a button icon with its label, the pane
   shade and the detail window shown with Square (same source as libdeck_swap ds_info.c). */
#include "libdeck_swap.h"

typedef struct DsButton {              /* ds_draw_button */
    unsigned short res;                /* resource id of the icon */
    unsigned char u, v;                /* +0x02 */
    unsigned short text;               /* +0x04 strTbl.bin entry */
} DsButton;


void font_set_flag4000(int on);
unsigned short *card_get_desc(int id);
unsigned short *card_info_fill(int id);
void *cardlist_build(void *list, DsScroll *s);
unsigned int font_sample_glyph_size(void);
unsigned short ds_cur_index(unsigned short side);
int ds_sort_value(DsEnt *e, unsigned short sort);
unsigned int ds_tab_pos(unsigned short i);
void *ds_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type);
void *ds_draw_attr_icon(void *list, unsigned short x, unsigned short y, unsigned short attr);
void *ds_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind);
void *ds_draw_limit_icon(void *list, unsigned short x, unsigned short y, unsigned short n);
void *ds_draw_pack_icon(void *list, unsigned short x, unsigned short y, unsigned short n);

void *ds_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type)
{
    return ds_draw_icon_a(list, x | y << 16, type, res_get(0x8007));
}

void *ds_draw_attr_icon(void *list, unsigned short x, unsigned short y, unsigned short attr)
{
    return ds_draw_icon_b(list, x | y << 16, attr, res_get(0x8007));
}

void *ds_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind)
{
    return ds_draw_icon_c(list, x | y << 16, kind, res_get(0x8007));
}

void *ds_draw_limit_icon(void *list, unsigned short x, unsigned short y, unsigned short n)
{
    return ds_draw_icon_d(list, x | y << 16, n, res_get(0x800A));
}

void *ds_draw_pack_icon(void *list, unsigned short x, unsigned short y, unsigned short n)
{
    return ds_draw_icon_e(list, x | y << 16, n, res_get(0x800A));
}

/* a button icon and its label */
void *ds_draw_button(void *list, DsButton *b, int x, int y, unsigned int color)
{
    DsIconText t;

    memset(&t, 0, sizeof(t));
    t.w = 0x11;
    t.h = 0x11;
    t.u = b->u;
    t.v = b->v;
    t.gim = res_get(b->res);
    t.text = table_get_entry(res_get(0x8033), b->text);
    t.color = color;
    return ds_draw_icon_text(list, &t, x, y);
}

/* darkens pane side and tab button tab */
void *ds_draw_shade(void *list, unsigned short side, unsigned short tab)
{
    DsLayout *lay = &dsv.pane[side].lay;
    unsigned int pos = ds_tab_pos(tab);

    list = dl_disable(list, 9);
    list = dl_tex_func(list, 4, 0);
    list = dl_ambient_color(list, 0x80000000);
    list = draw_set_offsets(list, 0, 0, lay->w, lay->h);
    list = draw_sprite(list, lay->x, lay->y, 0, 0, 0, 0, 0);
    list = draw_set_offsets(list, 0, 0, (pos >> 16) & 0xFFFF, 0xF);
    list = draw_sprite(list, lay->x + (pos & 0xFFFF), 0x17, 0, 0, 0, 0, 0);
    list = dl_enable(list, 9);
    list = dl_tex_func(list, 0, 1);
    return dl_ambient_color(list, 0xFFFFFFFF);
}

/* the value of the sort key of the cursor's card, in a bubble beside the scroll bar */
void *ds_draw_detail(void *list)
{
    DsPane *p = &dsv.pane[dsv.side];
    DsList *l = p->list;
    unsigned short i = ds_cur_index(dsv.side);
    DsEnt *e = DS_ENT(l, i);
    int sort = l->sort;
    unsigned int v;
    unsigned int sz;
    int x;
    int w;
    int r;
    int h;
    int cols;
    int hh;
    int y;
    int rows;
    int lw;

    if (e == 0)
        return list;
    v = ds_sort_value(e, sort);
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
        sz = (sz >> 16 & 0xFFFF) >> 6;
        w = sz + 13;
        h = sz + 11;
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
    hh = p->lay.h - 0x2B;
    y = p->lay.y + 12;
    cols = p->lay.cols;
    rows = (l->count + (cols - 1)) / cols;
    r = i / cols;
    x = p->lay.x + p->lay.w - 10 - w;
    if (rows == 0)
        y += (hh - h) / 2;
    else
        y = y + hh * r / rows - h / 2;
    list = ds_draw_box(list, x, y, w, h, res_get(0x8013));
    if (v == 0xFFFFFFFE)
        return list;
    switch (sort) {
    case 0:
        break;
    case 1: {
        unsigned short ch[2];

        font_text_set(list);
        font_set_size(14, 14);
        font_set_color(0xFFFFFFFF);
        {
            unsigned char *d = (unsigned char *)ch;
            int n = 4;

            if (d)
                do {
                    *d++ = 0;
                } while (--n);
        }
        ch[0] = card_top_get_letter(v);
        lw = font_measure_text(ch);
        font_draw_text((x + 4 + (w - 9) / 2) * 64 - lw / 2, (y + 4) * 64, w << 6, ch);
        list = font_text_clear();
        break;
    }
    case 2:
        list = ds_draw_limit_icon(list, x + 5, y + 5, ds_kind_icon(v));
        break;
    case 9:
        list = ds_draw_pack_icon(list, x + 5, y + 5, v);
        break;
    case 3:
        list = ds_draw_level(list, PT(x + 5, y + 4), v, 0xFFFFFFFF, res_get(0x8009));
        break;
    case 4:
        if ((int)v > 0)
            v = (int)v / 100 * 100;
        else if (v == 0xFFFFFFFF)
            v = 0xFFFF;
        list = ds_draw_stat(list, (unsigned short)v << 16 | 2, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, res_get(0x8009));
        break;
    case 5:
        if ((int)v > 0)
            v = (int)v / 100 * 100;
        else if (v == 0xFFFFFFFF)
            v = 0xFFFF;
        list = ds_draw_stat(list, (unsigned short)v << 16 | 3, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, res_get(0x8009));
        break;
    case 8:
        list = ds_draw_attr_icon(list, x + 5, y + 5, v);
        break;
    case 6:
        list = ds_draw_type_icon(list, x + 5, y + 5, v);
        break;
    case 7:
        list = ds_draw_kind_icon(list, x + 5, y + 5, v);
        break;
    }
    return list;
}

