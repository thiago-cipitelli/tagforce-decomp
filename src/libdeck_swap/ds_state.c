// @range 0x08804608-0x08804E2C
/* Cursor index and entry, the cursor card's info text (measured to scroll it), tab switching with
   the saved scroll of each list, scroll clamping, saving the panes' scroll into their lists, and
   the undo history of moves (same source as libselect_card sc_state.c). */
#include "libdeck_swap.h"
void ds_window_anim(void);

int font_measure_text(unsigned short *text);
unsigned short *card_get_desc(int id);
unsigned short ds_cur_index(unsigned short side);
DsEnt *ds_cur_entry(unsigned short side);
void ds_info_update(void);
void ds_set_panes(unsigned short which);
void ds_layout_scroll(unsigned short keep, int count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);

/* index of the cursor's entry in the visible order of pane side */
unsigned short ds_cur_index(unsigned short side)
{
    DsPane *p = &dsv.pane[side];
    int r;

    if (p->list_mode == 0)
        r = p->cx + (p->top + p->cy * p->lay.cols);
    else
        r = p->top + p->cy;
    return r;
}

DsEnt *ds_cur_entry(unsigned short side)
{
    DsPane *p = &dsv.pane[side];
    unsigned short i = ds_cur_index(side);

    return i >= p->list->count ? 0 : &p->list->ents[p->list->order[i]];
}

/* the cursor moved to another card: reset the timer, measure its text */
void ds_info_update(void)
{
    DsEnt *e;
    unsigned short id;
    int m;

    dsv.cursor.timer = 0;
    e = ds_cur_entry(dsv.side);
    if (e == 0) {
        dsv.cursor.scroll.id = -1;
        dsv.cursor.id = 0;
        dsv.cursor.key = 0;
        dsv.cursor.ent = 0;
        return;
    }
    id = e->id;
    if (id == 0) {
        dsv.cursor.scroll.id = -1;
        dsv.cursor.id = 0;
        dsv.cursor.key = 0;
        dsv.cursor.ent = 0;
        return;
    }
    dsv.cursor.id = id;
    dsv.cursor.key = e->key;
    dsv.cursor.ent = e;
    font_set_size(12, 12);
    m = font_measure_text(card_get_desc(id)) - 0x4F40;
    dsv.scroll_pos = 0;
    if (m <= 0)
        dsv.scroll_max = 0;
    else
        dsv.scroll_max = m;
    if (dsv.mode < 4) {
        dsv.cursor.scroll.id = id;
        dsv.cursor.scroll.pos = 0;
    }
}

/* shows tab (how 0 = previous, 1 = next, else tab itself) in pane side, keeping each list's scroll */
void ds_select_tab(unsigned short side, unsigned short tab, unsigned short how)
{
    unsigned short n = 3;
    int i;
    DsPane *p;
    DsList *l;
    unsigned short keep;
    unsigned short view;
    unsigned short cols;

    if (dsv.mode > 1) {
        switch (side) {
        case 0:
            n = 2;
            break;
        case 1:
            n = 1;
            break;
        }
    }
    switch (how) {
    default:
        i = tab;
        break;
    case 0:
        i = (tab + n - 1) % n;
        break;
    case 1:
        i = (tab + 1) % n;
        break;
    }
    keep = 1;
    p = &dsv.pane[side];
    if (side == 1)
        i += 2;
    l = &ds.list[i];
    if (p->list->tab != l->tab) {
        ds_set_panes(side);
        p->list = l; p->top = p->list->u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        if (p->list->u1d_lo != ((p->list_mode == 0) ? p->lay.cols : 1))
            keep = 0;
        view = (p->list_mode == 0) ? p->lay.grid_rows : p->lay.rows;
        cols = (p->list_mode == 0) ? p->lay.cols : 1;
        ds_layout_scroll(keep, p->list->u1a, (cols & 0xFF) | (view & 0xFF) << 8,
                         p->list->count, &p->top, &p->cx, &p->cy);
        ds_window_anim();
    }
}

/* clamps a pane's scroll (top, cx, cy) to total entries in a view of view & 0xFF columns and
   view >> 8 rows; keep puts the cursor back on the entry it was on */
void ds_layout_scroll(unsigned short keep, int count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy)
{
    int cols = view & 0xFF;
    int rows = (view >> 8) & 0xFF;
    unsigned short area = cols * rows;
    unsigned short otop = *top;
    unsigned short cur = *cx + (otop + *cy * cols);
    int i;
    unsigned short r;

    if (cols <= 0)
        return;
    if (total < 2) {
        *top = 0;
        if (keep == 0) {
            *cy = 0;
            *cx = 0;
        }
        return;
    }
    if (area >= total) {
        *top = 0;
    } else if (otop + area - cols >= total) {
        *top = cols * ((total + cols - 1) / cols - rows);
    }
    if (otop != *top)
        *cy += (unsigned short)((short)((short)otop - (short)*top) / cols);
    if (!(cur < total))
        cur = total - 1;
    i = count;
    if (keep == 1) {
        if (*cx >= cols || *cy >= rows)
            i = cur;
        else
            i = -1;
    }
    if (!(i < total))
        i = total - 1;
    if (i >= 0) {
        r = i / cols;
        if (r < rows)
            *top = 0;
        else
            *top = cols * (r - (rows - 1));
        *cx = i % cols;
        *cy = r - *top / cols;
    }
}

/* stores the scroll of the pane(s) into their lists: which 0 = first, 1 = second, 2 = both */
void ds_set_panes(unsigned short which)
{
    DsPane *p;
    unsigned short n;
    unsigned short v;

    if (which != 1) {
        p = &dsv.pane[0];
        n = p->list_mode == 0 ? p->lay.cols : 1;
        v = p->cx + (p->top + p->cy * n);
        p->list->u18 = p->top;
        p->list->u1c_lo = p->cx;
        p->list->u1c_hi = p->cy;
        p->list->u1a = v;
        p->list->u1d_lo = n;
    }
    if (which != 0) {
        p = &dsv.pane[1];
        n = p->list_mode == 0 ? p->lay.cols : 1;
        v = p->cx + (p->top + p->cy * n);
        p->list->u18 = p->top;
        p->list->u1c_lo = p->cx;
        p->list->u1c_hi = p->cy;
        p->list->u1a = v;
        p->list->u1d_lo = n;
    }
}

/* undo history: last 8 moves */
void ds_hist_push(DsMove *m)
{
    int n = ds.nhist;
    int i;

    if (n >= 8) {
        for (i = 0; i < 7; i++)
            memcpy(&ds.hist[i], &ds.hist[i + 1], 10);
        n--;
    } else {
        ds.nhist++;
    }
    memcpy(&ds.hist[n], m, 10);
}

int ds_hist_pop(DsMove *m)
{
    if (ds.nhist == 0)
        return 0;
    memcpy(m, &ds.hist[--ds.nhist], 10);
    return 1;
}
