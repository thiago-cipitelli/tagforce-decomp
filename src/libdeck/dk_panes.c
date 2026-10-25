// @range 0x0880E4D0-0x0880F1E4
// @asm 0x0880E4D0-0x0880EAA0
// @data 0x08822EE4-0x08822EF0
/* Pane headers and counters: the sort, direction and filter icons of each pane with the number of
   cards shown, the "new" mark of the trunk, the scroll bars, the copy counts on the list tabs, and
   the deck's cards per attribute icon (digits and a bar split by attribute). */
#include "libdeck.h"

#define PT(x, y) ((unsigned short)(x) | (unsigned short)(y) << 16)

unsigned char cfg_get_byte0_hi2(void);
int partner_count_get(void);
void *dk_draw_sort_icon(void *list, int sort, int x, int y);
void *dk_draw_dir_icon(void *list, int dir, int x, int y);
void *dk_draw_filter_icon(void *list, int filter, int x, int y);
void *dk_draw_digits(void *list, int pos, int pad, unsigned int value, unsigned short fmt, unsigned int color, void *gim);
void *dk_draw_flag_icon(void *list, int pos, unsigned short kind, void *gim);
void *dk_draw_scrollbar(void *list, int x, int y, int h, int pos, int first, int total, int frame, void *gim);
void *dk_draw_num_text(void *list, int x, int y, int value, int step, unsigned short n, int pad);
unsigned short func_0880B4F0(unsigned short tab, unsigned short sel, DkRect *r);
void *dk_draw_counts(void *list);
void *dk_draw_count(void *list, unsigned short x, DkRect *r, unsigned short ndig, int value);
void *dk_draw_attr_bar(void *list, int x, int y, int len, int dir, int i);

#ifdef NON_MATCHING /* mesma forma; o original guarda o id 0x8007 num u16 da pilha para o segundo painel e distribui os s de outro jeito (como sc_draw_headers) */
/* header of each pane: sort and direction icons, filter icon with the number of cards, "new"
   mark of the trunk, scroll bar */
void *dk_draw_headers(void *list)
{
    DkPane *p;
    DkList *l;
    DkLayout *lay;
    int x;
    int y;
    int show;
    int mark;
    int style;
    unsigned short id;
    unsigned short cols;

    p = &dkv.pane[0];
    l = p->list;
    lay = &p->lay;
    x = p->lay.x + 3;
    y = p->lay.y + p->lay.h - 18;
    list = dk_draw_sort_icon(list, l->sort, x, y);
    list = dk_draw_dir_icon(list, l->dir, x + 18, y);
    if (l->filter != 0) {
        show = 1;
        if (dkv.popup == 6 && dkv.side == 0 && (gfx_get_state()[0] >> 2 & 1))
            show = 0;
        if (show) {
            list = dk_draw_filter_icon(list, l->filter, x + 37, y);
            x += 56;
            if (l->count < 10)
                x -= 21;
            else if (l->count < 100)
                x -= 14;
            else if (l->count < 1000)
                x -= 7;
            list = dk_draw_digits(list, PT(x, y + 4), 0, l->count, 4, 0xFFFFFFFF, dk_res_get(0x8009));
        }
    }
    x = lay->x + lay->w - 20;
    if (l->tab == 3 && (mark = cfg_get_byte0_hi2()) != 0)
        list = dk_draw_flag_icon(list, PT(x, y + 3), mark, dk_res_get(0x800A));
    if (dkv.side == 0)
        style = 0;
    else
        style = 1;
    if (l->tab >= 3)
        style += 2;
    if (p->list_mode == 0) {
        id = 0x8007;
        cols = lay->cols;
        list = dk_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top / cols, lay->grid_rows,
                                 (l->count + cols - 1) / cols, style, dk_res_get(id));
    } else {
        id = 0x8007;
        list = dk_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top, lay->rows, l->count, style,
                                 dk_res_get(id));
    }
    if (dkv.mode > 1) {
        p = &dkv.pane[1];
        l = p->list;
        lay = &p->lay;
        x = p->lay.x + 3;
        y = p->lay.y + p->lay.h - 18;
        list = dk_draw_sort_icon(list, l->sort, x, y);
        list = dk_draw_dir_icon(list, l->dir, x + 18, y);
        if (l->filter != 0) {
            show = 1;
            if (dkv.popup == 6 && dkv.side == 1 && (gfx_get_state()[0] >> 2 & 1))
                show = 0;
            if (show) {
                list = dk_draw_filter_icon(list, l->filter, x + 37, y);
                x += 56;
                if (l->count < 10)
                    x -= 21;
                else if (l->count < 100)
                    x -= 14;
                else if (l->count < 1000)
                    x -= 7;
                list = dk_draw_digits(list, PT(x, y + 4), 0, l->count, 4, 0xFFFFFFFF, dk_res_get(0x8009));
            }
        }
        x = lay->x + lay->w - 20;
        if (l->tab == 3 && (mark = cfg_get_byte0_hi2()) != 0)
            list = dk_draw_flag_icon(list, PT(x, y + 3), mark, dk_res_get(0x800A));
        style = dkv.side == 1 ? 2 : 3;
        if (p->list_mode == 0) {
            cols = lay->cols;
            list = dk_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top / cols, lay->grid_rows,
                                     (l->count + cols - 1) / cols, style, dk_res_get(id));
        } else {
            list = dk_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top, lay->rows, l->count,
                                     style, dk_res_get(id));
        }
    }
    return dk_draw_counts(list);
}
#endif

/* digits of the copy count on each list tab */
unsigned short dk_count_digits[6] = { 2, 2, 2, 6, 2, 1 };

/* the total of copies on the tab buttons of each pane (the partner list only when there is one) */
void *dk_draw_counts(void *list)
{
    DkPane *p = &dkv.pane[0];
    unsigned short sel = p->list->tab;
    int n = 6;
    int i;
    unsigned short x;
    DkRect r;

    if (partner_count_get() <= 0)
        n--;
    if (dkv.mode < 2) {
        for (i = 0; i < n; i++) {
            if (i == 4)
                continue;
            x = func_0880B4F0(i, sel, &r);
            x += p->lay.x;
            r.x += p->lay.x;
            list = dk_draw_count(list, x, &r, dk_count_digits[i], dk_lists[i].total);
        }
    } else {
        for (i = 0; i < 3; i++) {
            x = func_0880B4F0(i, sel, &r);
            x += p->lay.x;
            r.x += p->lay.x;
            list = dk_draw_count(list, x, &r, dk_count_digits[i], dk_lists[i].total);
        }
        p = &dkv.pane[1];
        sel = p->list->tab;
        for (; i < n; i++) {
            x = func_0880B4F0(i, sel, &r);
            x += p->lay.x;
            r.x += p->lay.x;
            list = dk_draw_count(list, x, &r, dk_count_digits[i], dk_lists[i].total);
        }
    }
    return list;
}

void *dk_draw_count(void *list, unsigned short x, DkRect *r, unsigned short ndig, int value)
{
    if (r->w <= 0 || r->h <= 0)
        return list;
    list = dl_scissor(list, r->x, r->y, r->w, r->h);
    list = dl_enable(list, 2);
    font_set_size(11, 12);
    font_set_color(0xFF000000);
    list = dk_draw_num_text(list, (short)x, (short)(r->y + 1), value, 8, ndig, 0);
    return dl_disable(list, 2);
}

/* the deck's cards per attribute icon */
void *dk_draw_attr_counts(void *list)
{
    int i;
    int x;
    int v;
    int y;

    for (i = 0; i < 6; i++) {
        v = dk_deck.attr_count[i];
        if (dkv.mode < 4) {
            x = 122;
            y = i * 19 + 23;
        } else {
            x = i * 21 + 24;
            y = 3;
        }
        if (v < 10)
            x -= 3;
        list = dk_draw_digits(list, PT(x, y), 0, v, 2, 0xFFFFFFFF, dk_res_get(0x8009));
    }
    return list;
}

/* a bar split in the share of each attribute icon */
void *dk_draw_attr_bars(void *list)
{
    int total = 0;
    int i;
    int base;
    int len;
    int sum;
    int pos[7];

    for (i = 0; i < 6; i++)
        total += dk_deck.attr_count[i];
    if (dkv.mode < 4) {
        base = 20;
        len = 111;
    } else {
        base = 23;
        len = 122;
    }
    memset(pos, 0, sizeof(int) * 6);
    if (total > 0) {
        sum = 0;
        pos[0] = base;
        for (i = 0; i < 6; i++) {
            sum += dk_deck.attr_count[i];
            pos[i + 1] = base + len * sum / total;
        }
    }
    for (i = 0; i < 6; i++) {
        if (dk_deck.attr_count[i] == 0)
            continue;
        if (dkv.mode < 4)
            list = dk_draw_attr_bar(list, 141, pos[i], pos[i + 1] - pos[i], 0, i);
        else
            list = dk_draw_attr_bar(list, pos[i], 18, pos[i + 1] - pos[i], 3, i);
    }
    return list;
}

/* one share of the attribute bar (dir 0 vertical, 1 / 3 horizontal) */
void *dk_draw_attr_bar(void *list, int x, int y, int len, int dir, int i)
{
    void *gim = dk_res_get(0x800A);
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
