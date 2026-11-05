// @range 0x08809804-0x0880AB28
// @asm 0x0880A9BC-0x0880AB28
// @rodata 0x08819C7C-0x08819CA0
/* The per-frame input dispatch and the input of the main state: cursor moves in the grid and in
   the list, the help states while Square/Triangle are held (pages, sort groups, marks, the sort and
   filter menus, undo), paging, jumps between sort groups, the card text scroll and the focus frame
   (same source as libselect_card sc_input.c). */
#include "libdeck_swap.h"

void ds_select_tab(unsigned short side, unsigned short tab, int how);
DsEnt *ds_cur_entry(unsigned short side);
void ds_info_update(void);
void ds_set_mode(unsigned short mode);
int ds_mode_for(unsigned short panes, int narrow, unsigned short mode_a, unsigned short mode_b);
void ds_layout_scroll(int grid, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
unsigned short ds_list_filter_reset(DsList *l);
void ds_list_sort(DsList *l, unsigned short sort, unsigned short dir);
void ds_input_sort_menu(void);
void ds_input_action_menu(void);
void ds_sort_menu_open(void);
void ds_sort_menu_update(void);
void ds_fmenu_update(void);
void ds_input_result(void);
void ds_popup_update(void);
void ds_action_open(DsEnt *e);
void ds_popup_open(int id, int arg);
int ds_flight_open(void);
void ds_fmenu_open(void);
int ds_input_move(void);
void ds_input_main(void);
void ds_input_help(void);
void ds_input_help_b(void);
void ds_cursor_grid(int side, PadState *pad);
void ds_cursor_list(int side, PadState *pad);
void ds_cursor_switch();
void ds_page(int up);
void ds_jump(int up);
int ds_find_group(int up);
void ds_scroll_update(void);
void ds_focus_update(void);
unsigned short ds_cur_index(unsigned short side);
int ds_sort_value(DsEnt *e, unsigned short sort);

/* entry at visible position i of list l (0 past the end) */
#define DS_ENT(l, i) ((i) >= (l)->count ? 0 : &(l)->ents[(l)->order[i]])

/* per frame, after the animations: input of the current state, unless a popup or the busy flag */
void ds_update_input(void)
{
    pad_get();
    if (!dsv.busy && !ds.info_open) {
        ds_scroll_update();
        if (dsv.popup != 0) {
            ds_popup_update();
        } else {
            switch (ds.state) {
            case 0:
                break;
            case 1:
            case 7:
            case 8:
                ds_input_main();
                break;
            case 2:
                ds_input_sort_menu();
                break;
            case 3:
                ds_input_action_menu();
                break;
            case 4:
                ds_sort_menu_update();
                break;
            case 5:
                ds_fmenu_update();
                break;
            case 6:
                ds_input_result();
                break;
            }
            ds_focus_update();
        }
    }
}

/* state 1: cursor, Square/Triangle hold the help windows, Circle/Cross/Start/Select actions */
void ds_input_main(void)
{
    PadState *pad = pad_get();
    unsigned int trig;
    unsigned short tab;
    unsigned char side;
    int m;
    unsigned short n;
    int mode;
    unsigned short a;
    unsigned short b;
    unsigned char cur = dsv.side;
    DsPane *p = &dsv.pane[cur];

    if ((pad->b.rep & 0xF0) != 0) {
        if (p->list_mode == 0) {
            ds_cursor_grid(cur, pad);
            return;
        }
        ds_cursor_list(cur, pad);
        return;
    }
    if (pad->b.down & 0x100) {
        ds.state = 7;
        ds_input_help();
        return;
    }
    if (pad->b.down & 0x200) {
        ds.state = 8;
        ds_input_help_b();
        return;
    }
    ds.state = 1;
    trig = pad->b.trig;
    if (trig & region_pick_value(1)) {
        if (ds.list[0].total != ds.deck->n_main) {
            ds_popup_open(3, 0);
            se_play(0x621A);
        } else {
            ds_popup_open(2, 0);
            se_play(0x621A);
        }
    } else if (trig & region_pick_value(0)) {
        if (ds_flight_open())
            se_play(0x6213);
    } else if (trig & 0x1000) {
        ds_action_open(ds_cur_entry(dsv.side));
        se_play(0x6216);
    } else if (trig & 0x8000) {
        side = dsv.side;
        tab = dsv.pane[side].list->tab;
        ds_select_tab(side, tab, 1);
        if (tab != dsv.pane[dsv.side].list->tab)
            se_play(0x621B);
    } else if (trig & 1) {
        m = dsv.mode;
        n = m > 1 ? 2 : 1;
        a = dsv.pane[0].list_mode;
        b = dsv.pane[1].list_mode;
        if (dsv.side == 0)
            a = (a + 1) & 1;
        else
            b = (b + 1) & 1;
        mode = ds_mode_for(n, m < 4, a, b);
        if (mode >= 0) {
            ds_set_mode(mode);
            se_play(0x6213);
        }
    }
}

/* state 7 (Square held): sort order */
void ds_input_help(void)
{
    PadState *pad = pad_get();
    unsigned int rep = pad->b.rep;
    DsList *l;

    if (rep & 0x8000) {
        ds_page(1);
        se_play(0x6212);
    } else if (rep & 0x2000) {
        ds_page(0);
        se_play(0x6212);
    } else if (rep & 0x1000) {
        ds_jump(1);
        se_play(0x6212);
    } else if (rep & 0x4000) {
        ds_jump(0);
        se_play(0x6212);
    } else if (pad->b.trig & 1) {
        l = dsv.pane[dsv.side].list;
        if (l->filter != 0) {
            ds_list_filter_reset(l);
            ds_list_sort(l, l->sort, l->dir);
            se_play(0x6213);
        }
    }
}

/* state 8 (Triangle held): marks and the sort/filter menus */
void ds_input_help_b(void)
{
    PadState *pad = pad_get();
    unsigned int trig = pad->b.trig;
    DsEnt *e;
    unsigned short id;

    if (trig & region_pick_value(0)) {
        if (ds_flight_open())
            se_play(0x6213);
    } else if (trig & region_pick_value(1)) {
        e = ds_cur_entry(dsv.side);
        if (e != 0) {
            id = e->id;
            if (inv_get_flag7(id) != 0)
                inv_set_flag7(id, 0);
            else
                inv_set_flag7(id, 1);
            se_play(0x6213);
        }
    } else if (trig & 0x8000) {
        ds_sort_menu_open();
        se_play(0x6216);
    } else if (trig & 0x1000) {
        ds_fmenu_open();
        se_play(0x6216);
    } else if ((trig & 1) && ds.nhist > 0) {
        ds_input_move();
        se_play(0x6213);
    }
}

/* grid cursor: d-pad moves, wraps to the other pane at the left/right edges, scrolls by rows */
void ds_cursor_grid(int side, PadState *pad)
{
    DsPane *p = &dsv.pane[(unsigned short)side];
    int cols;
    int cy;
    int cx;
    int top;
    int rows;
    unsigned int rep;
    int total;
    int dx;
    int dy;
    int changed = 0;
    int nx;
    int ny;

    cx = p->cx;
    cy = p->cy;
    cols = p->lay.cols;
    top = (p->top + cols - 1) / cols;
    rows = p->lay.grid_rows;
    rep = pad->b.rep;
    total = (p->list->count + cols - 1) / cols;

    dy = 0;
    dx = 0;
    if (rep & 0x40)
        dy++;
    if (rep & 0x10)
        dy--;
    if (rep & 0x20)
        dx++;
    if (rep & 0x80)
        dx--;
    nx = cx + dx;
    if (nx < 0) {
        ds_cursor_switch(side, (unsigned short)rows, 1, (unsigned short)cy);
        se_play(0x6212);
    } else if (nx >= cols) {
        ds_cursor_switch(side, (unsigned short)rows, 0, (unsigned short)cy);
        se_play(0x6212);
    } else {
        if (top + cy == 0 && (pad->b.trig & 0x10)) {
            if (total >= rows)
                top = total - rows;
            else
                top = 0;
            ny = rows - 1;
        } else if (top + cy >= total - 1 && cy == rows - 1 && (pad->b.trig & 0x40)) {
            ny = 0;
            top = 0;
        } else {
            ny = cy + dy;
            if (ny < 0) {
                top = (top + ny > 0) ? top + ny : 0;
                ny = 0;
            } else if (ny >= rows) {
                if (rows < total) {
                    if (top + ny >= total)
                        top = total - rows;
                    else
                        top += ny - (rows - 1);
                }
                ny = rows - 1;
            }
        }
        if (cx != nx) {
            p->cx = nx;
            changed = 1;
        }
        if (p->cy != ny) {
            p->cy = ny;
            changed = 1;
        }
        if (p->top != top * cols) {
            p->top = top * cols;
            changed = 1;
        }
        if (changed)
            se_play(0x6212);
    }
}

/* list cursor: d-pad up/down with wrap at the ends, left/right go to the other pane */
void ds_cursor_list(int side, PadState *pad)
{
    DsPane *p = &dsv.pane[(unsigned short)side];
    int cy = p->cy;
    int rows = p->lay.rows;
    unsigned int rep = pad->b.rep;
    int top = p->top;
    int total = p->list->count;
    int changed = 0;
    int dy;
    int ny;

    if (rep & 0x80) {
        ds_cursor_switch(side, (unsigned short)rows, 1, (unsigned short)cy);
        se_play(0x6212);
    } else if (rep & 0x20) {
        ds_cursor_switch(side, (unsigned short)rows, 0, (unsigned short)cy);
        se_play(0x6212);
    } else {
        dy = 0;
        if (rep & 0x40)
            dy++;
        if (rep & 0x10)
            dy--;
        if (top + cy == 0 && (pad->b.trig & 0x10)) {
            if (total >= rows)
                top = total - rows;
            else
                top = 0;
            ny = rows - 1;
        } else if (top + cy >= total - 1 && cy == rows - 1 && (pad->b.trig & 0x40)) {
            ny = 0;
            top = 0;
        } else {
            ny = cy + dy;
            if (ny < 0) {
                top = (top + ny > 0) ? top + ny : 0;
                ny = 0;
            } else if (ny >= rows) {
                if (total < rows)
                    top = 0;
                else if (top + ny >= total)
                    top = total - rows;
                else
                    top += ny - (rows - 1);
                ny = rows - 1;
            }
        }
        if (cy != ny) {
            p->cy = ny;
            changed = 1;
        }
        if (p->top != top) {
            p->top = top;
            changed = 1;
        }
        if (changed)
            se_play(0x6212);
    }
}

/* moves the cursor to the other pane (dir 1 = enter from the right), keeping its height; K&R so the
   4-argument calls compile: "to" is a dead stack/register value unless from is 0 or 1 (as in the original) */
void ds_cursor_switch(from, rows, dir, cy, to)
unsigned short from;
unsigned short rows;
unsigned short dir;
unsigned short cy;
unsigned short to;
{
    unsigned short cx = 0;
    unsigned short n;
    DsPane *p;

    switch (from) {
    case 0:
        if (dsv.mode > 1)
            to = 1;
        else
            to = 0;
        break;
    case 1:
        to = 0;
        break;
    }
    p = &dsv.pane[to];
    if (p->list_mode == 0) {
        if (dir == 1)
            cx = p->lay.cols - 1;
        n = p->lay.grid_rows;
    } else
        n = p->lay.rows;
    if (n != rows) {
        cy = (int)(((unsigned int)rows >> 1) + cy * n) / (int)rows;
        if (cy >= n)
            cy = n - 1;
    }
    dsv.side = to;
    p->cx = cx;
    p->cy = cy;
}

/* L/R: a page up or down */
void ds_page(int up)
{
    DsPane *p = &dsv.pane[dsv.side];
    int total = p->list->count;
    int cols;
    int top;
    int view;

    if (p->list_mode == 0) {
        cols = p->lay.cols;
        top = (p->top + cols - 1) / cols;
        total = (total + cols - 1) / cols;
        view = p->lay.grid_rows;
    } else {
        view = p->lay.rows;
        top = p->top;
        cols = 1;
    }
    if (total <= view) {
        p->top = 0;
    } else {
        if (up) {
            if (top < view)
                top = 0;
            else
                top -= view;
        } else {
            top += view;
            if (total - view < top)
                top = total - view;
        }
        if (p->list_mode == 0)
            p->top = top * cols;
        else
            p->top = top;
    }
}

/* jumps to the next/previous group of the sort key */
void ds_jump(int up)
{
    DsPane *p = &dsv.pane[dsv.side];
    int total = p->list->count;
    int i;
    int cols;
    int row;
    int last;

    if (total == 0)
        return;
    i = ds_find_group(up);
    if (i < 0)
        return;
    if (p->list_mode == 0) {
        cols = p->lay.cols;
        last = p->lay.grid_rows;
        row = i / cols;
        last = (total + cols - 1) / cols - last;
        if (last < row)
            row = last;
        if (row < 0)
            row = 0;
        p->top = row * cols;
        p->cx = i % cols;
        p->cy = (i - p->top) / cols;
    } else if (i < p->lay.rows) {
        p->top = 0;
        p->cy = i;
    } else {
        last = total - p->lay.rows;
        if (last < i) {
            p->top = last;
            p->cy = i - p->top;
        } else {
            p->top = i;
            p->cy = 0;
        }
    }
}

/* index of the first card of the next (up = 0) or previous sort group from the cursor, -1 none */
int ds_find_group(int up)
{
    int k;
    DsList *l = dsv.pane[dsv.side].list;
    int i = (unsigned short)ds_cur_index(dsv.side);
    unsigned short sort = l->sort;
    int n = l->count;
    DsEnt *e = i >= n ? 0 : &l->ents[l->order[i]];
    int v;
    int w;
    int j;

    if (e == 0)
        return n - 1;
    if (up) {
        if (i <= 0)
            return -1;
        j = i - 1;
        v = ds_sort_value(j >= n ? 0 : &l->ents[l->order[j]], sort);
        if ((sort == 4 || sort == 5) && v > 0)
            v /= 100;
        for (k = j - 1; k >= 0; k--) {
            w = ds_sort_value(DS_ENT(l, k), sort);
            if ((sort == 4 || sort == 5) && w > 0)
                w /= 100;
            if (w != v) {
                k++;
                break;
            }
        }
        if (k < 0)
            k = 0;
    } else {
        v = ds_sort_value(e, sort);
        if ((sort == 4 || sort == 5) && v > 0)
            v /= 100;
        for (k = i + 1; k < n; k++) {
            w = ds_sort_value(DS_ENT(l, k), sort);
            if ((sort == 4 || sort == 5) && w > 0)
                w /= 100;
            if (w != v)
                break;
        }
        if (k >= n)
            return -1;
    }
    return k;
}

/* analog scrolling of the card info text (one pane) or of the wide view */
void ds_scroll_update(void)
{
    PadState *pad = pad_get();
    int moved;
    int max;
    int lim;
    int v;

    if (dsv.mode < 4) {
        ds_scroll_analog(&dsv.cursor.scroll);
    } else {
        max = dsv.scroll_max;
        if (max > 0) {
            moved = 0;
            lim = max + 0x76C0;
            if (pad->b.analog_x > 0x9E) {
                v = (pad->b.analog_x - 0x9E) * 4;
                if (max < dsv.scroll_pos) {
                    dsv.scroll_pos += v;
                } else {
                    if (max < dsv.scroll_pos + v)
                        dsv.scroll_pos = max;
                    else
                        dsv.scroll_pos += v;
                }
                moved = 1;
            }
            if (pad->b.analog_x < 0x62) {
                v = dsv.scroll_pos + (pad->b.analog_x - 0x62) * 4;
                if (v < 0)
                    dsv.scroll_pos = 0;
                else
                    dsv.scroll_pos = v;
                moved = 1;
            }
            if (!moved && dsv.cursor.timer >= 0x36)
                dsv.scroll_pos += 0x40;
            if (dsv.scroll_pos >= lim)
                dsv.scroll_pos -= lim;
        }
    }
}

#ifdef NON_MATCHING
/* NON_MATCHING, 20 palavras: lógica conferida com o asm; no ramo da grade o original soma x + grid_x antes do
   mult e grava w = 23 antes de x (perm_stmts não achou ordem) */
/* the focus frame follows the cursor */
void ds_focus_update(void)
{
    DsView *v = &dsv;
    unsigned char side = v->side;
    DsPane *p = &v->pane[side];
    DsLayout *lay = &p->lay;
    DsRect *f = &ds.focus;

    if (p->list_mode == 0) {
        f->w = 0x17;
        f->x = lay->x + lay->grid_x + p->cx * (unsigned short)lay->cell_w;
        f->y = lay->y + lay->grid_y + p->cy * (unsigned short)lay->cell_h;
        f->h = 0x1F;
    } else if (side == 1 && dsv.mode < 4) {
        f->x = lay->x + lay->row_x;
        f->y = lay->y + lay->row_y + p->cy * (unsigned short)lay->row_h;
        f->w = (unsigned short)lay->row_w - 1;
        f->h = (unsigned short)lay->row_h - 1;
    } else {
        f->x = lay->x + lay->row_x + 1;
        f->y = lay->y + lay->row_y + 1 + p->cy * (unsigned short)lay->row_h;
        f->w = (unsigned short)lay->row_w - 3;
        f->h = (unsigned short)lay->row_h - 1;
    }
}
#endif
