// @range 0x08807828-0x08808A54
// @asm 0x08807BE8-0x088080F8
// @data 0x0881A170-0x0881A17C
/* Background of the swap screen: bg.ehf, the window, the row lines of the list panes, the swap
   stream window with how far the main deck is from its size, the pane headers (sort, direction,
   filter and count, scroll bars) and the copy counts of the tabs, the main deck's cards per
   attribute (digits and a split bar) and the time left (same source as libdeck dk_draw.c and
   dk_panes.c). */
#include "libdeck_swap.h"

typedef struct DsLineVtx {             /* dl_draw_array vertex: colour + position, 12 bytes */
    unsigned int color;
    unsigned short x, y, z;
    unsigned short pad;
} DsLineVtx;

int sysvar_get_a98(void);
void *ds_draw_bg_ehf(void *list);
void *ds_draw_window(void *list);
void *ds_draw_grid_lines(void *list, unsigned short mode);
void ds_line(DsLineVtx *v, unsigned short x, unsigned short y, unsigned short w);
void *ds_draw_stream(void *list);
void *ds_draw_headers(void *list);
void *ds_draw_counts(void *list);
void *ds_draw_count(void *list, unsigned short x, DsRect *r, unsigned short ndig, int value);
void *ds_draw_attr_counts(void *list);
void *ds_draw_attr_bars(void *list);
void *ds_draw_attr_bar(void *list, int x, int y, int len, int dir, int i);
void *ds_draw_timer(void *list);
void *ds_draw_sort_icon(void *list, int sort, int x, int y);
void *ds_draw_dir_icon(void *list, int dir, int x, int y);
void *ds_draw_filter_icon(void *list, int filter, int x, int y);
unsigned short ds_tab_rect(unsigned short tab, unsigned short sel, DsRect *r);
void *dl_alloc_inline(void **list, int bytes);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);

/* background: bg.ehf, the window, the row lines, the stream window, the pane headers and the
   attribute counters; the time left when the match has a time limit */
void *ds_draw_bg(void *list)
{
    list = ds_draw_bg_ehf(list);
    list = ds_draw_window(list);
    list = ds_draw_grid_lines(list, dsv.mode);
    list = ds_draw_stream(list);
    list = ds_draw_headers(list);
    list = ds_draw_attr_counts(list);
    list = ds_draw_attr_bars(list);
    if (sysvar_get_a98())
        list = ds_draw_timer(list);
    return list;
}

/* animated background (bg.ehf) */
void *ds_draw_bg_ehf(void *list)
{
    void *e = res_get(0x8021);
    unsigned int opts[24];

    if (e == 0)
        return list;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()[0];
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, opts);
    list = gfx_end_2d(list);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    return dl_blend_func(list, 0, 2, 3, 0, 0);
}

/* the window (d_window0.ehf) */
void *ds_draw_window(void *list)
{
    void *e = res_get(0x8022);
    unsigned int opts[24];

    if (e == 0)
        return list;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()[0];
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, opts);
    return gfx_end_2d(list);
}

/* separator lines under the rows of the list panes */
void *ds_draw_grid_lines(void *list, unsigned short mode)
{
    DsLineVtx *v = dl_alloc_inline(&list, 0x240);
    DsLineVtx *p = v;
    unsigned short n = 0;
    unsigned short y;
    int i;

    if (mode == 0) {
        for (y = 41, i = 0; i < 7; i++, p += 4) {
            ds_line(p, 151, y, 317);
            y += 30;
            n += 4;
        }
    }
    if (mode == 4 || mode == 6) {
        for (y = 41, i = 0; i < 6; i++, p += 4) {
            ds_line(p, 3, y, 225);
            y += 30;
            n += 4;
        }
    }
    if (mode == 4 || mode == 7) {
        for (y = 41, i = 0; i < 6; i++, p += 4) {
            ds_line(p, 243, y, 225);
            y += 30;
            n += 4;
        }
    }
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_disable(list, 9);
    list = dl_draw_array(list, 1, 0x80011C, n, 0, v);
    list = dl_enable(list, 9);
    return list;
}

/* a dark line w wide at (x, y) with a lighter one under it */
void ds_line(DsLineVtx *v, unsigned short x, unsigned short y, unsigned short w)
{
    v[0].x = x;
    v[0].y = y;
    v[0].z = 0;
    v[0].color = 0x33000000;
    v[1].x = x + w;
    v[1].y = y;
    v[1].z = 0;
    v[1].color = 0x33000000;
    v[2].x = x;
    v[2].y = y + 1;
    v[2].z = 0;
    v[2].color = 0x19000000;
    v[3].x = x + w;
    v[3].y = y + 1;
    v[3].z = 0;
    v[3].color = 0x19000000;
}

#ifdef NON_MATCHING /* forma de sc_draw_headers sem a marca de novidade; o original guarda o id 0x8007 na pilha e a alocação dos s difere */
/* header of each pane: sort and direction icons, filter icon with the number of cards, "new"
   scroll bar */
void *ds_draw_headers(void *list)
{
    DsPane *p;
    DsList *l;
    DsLayout *lay;
    int x;
    int y;
    int show;
    int style;
    unsigned short id;
    unsigned short cols;

    p = &dsv.pane[0];
    l = p->list;
    lay = &p->lay;
    x = p->lay.x + 3;
    y = p->lay.y + p->lay.h - 18;
    list = ds_draw_sort_icon(list, l->sort, x, y);
    list = ds_draw_dir_icon(list, l->dir, x + 18, y);
    if (l->filter != 0) {
        show = 1;
        if (dsv.popup == 4 && dsv.side == 0 && (gfx_get_state()[0] >> 2 & 1))
            show = 0;
        if (show) {
            list = ds_draw_filter_icon(list, l->filter, x + 37, y);
            x += 56;
            if (l->count < 10)
                x -= 21;
            else if (l->count < 100)
                x -= 14;
            else if (l->count < 1000)
                x -= 7;
            list = ds_draw_digits(list, PT(x, y + 4), 0, l->count, 4, 0xFFFFFFFF, res_get(0x8009));
        }
    }
    style = dsv.side != 0 ? 1 : 0;
    if (l->tab > 1)
        style += 2;
    if (p->list_mode == 0) {
        id = 0x8007;
        cols = lay->cols;
        list = ds_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top / cols, lay->grid_rows,
                                 (l->count + cols - 1) / cols, style, res_get(id));
    } else {
        id = 0x8007;
        list = ds_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top, lay->rows, l->count, style,
                                 res_get(id));
    }
    if (dsv.mode > 1) {
        p = &dsv.pane[1];
        l = p->list;
        lay = &p->lay;
        x = p->lay.x + 3;
        y = p->lay.y + p->lay.h - 18;
        list = ds_draw_sort_icon(list, l->sort, x, y);
        list = ds_draw_dir_icon(list, l->dir, x + 18, y);
        if (l->filter != 0) {
            show = 1;
            if (dsv.popup == 4 && dsv.side == 1 && (gfx_get_state()[0] >> 2 & 1))
                show = 0;
            if (show) {
                list = ds_draw_filter_icon(list, l->filter, x + 37, y);
                x += 56;
                if (l->count < 10)
                    x -= 21;
                else if (l->count < 100)
                    x -= 14;
                else if (l->count < 1000)
                    x -= 7;
                list = ds_draw_digits(list, PT(x, y + 4), 0, l->count, 4, 0xFFFFFFFF, res_get(0x8009));
            }
        }
        style = dsv.side == 1 ? 2 : 3;
        if (p->list_mode == 0) {
            cols = lay->cols;
            list = ds_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top / cols, lay->grid_rows,
                                     (l->count + cols - 1) / cols, style, res_get(id));
        } else {
            list = ds_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top, lay->rows, l->count,
                                     style, res_get(id));
        }
    }
    return ds_draw_counts(list);
}
#endif

/* digits of the copy count on each tab */
unsigned short ds_count_digits[3] = { 2, 2, 2 };

/* the total of copies on the tab buttons of each pane */
void *ds_draw_counts(void *list)
{
    DsPane *p = &dsv.pane[0];
    unsigned short sel = p->list->tab;
    int i;
    unsigned short x;
    DsRect r;

    if (dsv.mode < 2) {
        for (i = 0; i < 3; i++) {
            x = ds_tab_rect(i, sel, &r);
            x += p->lay.x;
            r.x += p->lay.x;
            list = ds_draw_count(list, x, &r, ds_count_digits[i], ds.list[i].total);
        }
    } else {
        for (i = 0; i < 2; i++) {
            x = ds_tab_rect(i, sel, &r);
            x += p->lay.x;
            r.x += p->lay.x;
            list = ds_draw_count(list, x, &r, ds_count_digits[i], ds.list[i].total);
        }
        p = &dsv.pane[1];
        sel = p->list->tab;
        for (; i < 3; i++) {
            x = ds_tab_rect(i, sel, &r);
            x += p->lay.x;
            r.x += p->lay.x;
            list = ds_draw_count(list, x, &r, ds_count_digits[i], ds.list[i].total);
        }
    }
    return list;
}

void *ds_draw_count(void *list, unsigned short x, DsRect *r, unsigned short ndig, int value)
{
    if (r->w <= 0 || r->h <= 0)
        return list;
    list = dl_scissor(list, r->x, r->y, r->w, r->h);
    list = dl_enable(list, 2);
    font_set_size(11, 12);
    font_set_color(0xFF000000);
    list = ds_draw_num_text(list, (short)x, (short)(r->y + 1), value, 8, ndig, 0);
    return dl_disable(list, 2);
}

/* the deck's cards per attribute icon */
void *ds_draw_attr_counts(void *list)
{
    int i;
    int x;
    int v;
    int y;

    for (i = 0; i < 6; i++) {
        v = ds.attr_count[i];
        if (dsv.mode < 4) {
            x = 122;
            y = i * 19 + 23;
        } else {
            x = i * 21 + 311;
            y = 3;
        }
        if (v < 10)
            x -= 3;
        list = ds_draw_digits(list, PT(x, y), 0, v, 2, 0xFFFFFFFF, res_get(0x8009));
    }
    return list;
}

/* a bar split in the share of each attribute icon */
void *ds_draw_attr_bars(void *list)
{
    int total = 0;
    int i;
    int base;
    int len;
    int sum;
    int pos[7];

    for (i = 0; i < 6; i++)
        total += ds.attr_count[i];
    if (dsv.mode < 4) {
        base = 20;
        len = 111;
    } else {
        base = 310;
        len = 122;
    }
    memset(pos, 0, sizeof(int) * 6);
    if (total > 0) {
        sum = 0;
        pos[0] = base;
        for (i = 0; i < 6; i++) {
            sum += ds.attr_count[i];
            pos[i + 1] = base + len * sum / total;
        }
    }
    for (i = 0; i < 6; i++) {
        if (ds.attr_count[i] == 0)
            continue;
        if (dsv.mode < 4)
            list = ds_draw_attr_bar(list, 141, pos[i], pos[i + 1] - pos[i], 0, i);
        else
            list = ds_draw_attr_bar(list, pos[i], 18, pos[i + 1] - pos[i], 3, i);
    }
    return list;
}

/* the time left of the match (minutes:seconds, the seconds blinking under 10 s) */
void *ds_draw_timer(void *list)
{
    int t = ds.time_limit;
    unsigned short colon[2] = { ':', 0 };

    if (t < 300) {
        if (t >> 2 & 1)
            return list;
        if (t < 0)
            t = 0;
    }
    font_set_size(11, 12);
    font_set_color(0xFFFFFFFF);
    list = ds_draw_num_text(list, 0x1DC, 5, t / 30 % 60, 8, 2, 1);
    font_text_set(list);
    font_draw_text(0x71C0, 0x100, 0x640, colon);
    list = font_text_clear();
    return ds_draw_num_text(list, 0x1C4, 5, t / 30 / 60, 8, 2, 0);
}

/* one share of the attribute bar (dir 0 vertical, 1 / 3 horizontal) */
void *ds_draw_attr_bar(void *list, int x, int y, int len, int dir, int i)
{
    void *gim = res_get(0x800A);
    int w;

    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    switch (dir) {
    case 1:
    case 3:
        w = len;
        len = 3;
        break;
    default:
        w = 3;
        break;
    }
    list = draw_set_offsets(list, 3, 1, w, len);
    return draw_sprite(list, x, y, 0, 0, i + 122, 0, dir);
}

/* the swap stream window and how many cards the main deck is off its size */
void *ds_draw_stream(void *list)
{
    void *e = res_get(0x1010);
    unsigned int opts[24];
    short x;
    int d;

    if (e == 0)
        return list;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()[0];
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, opts);
    list = gfx_end_2d(list);
    d = ds.list[0].total - ds.deck->n_main;
    if (d < 0)
        d = -d;
    if (dsv.mode < 2)
        x = 0xEB;
    else if (dsv.mode < 4)
        x = 0x11D;
    else
        x = 0x93;
    font_set_size(11, 12);
    font_set_color(0xFF000000);
    return ds_draw_num_text(list, x, 4, d, 8, 2, 0);
}
