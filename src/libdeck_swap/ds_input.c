// @range 0x08809804-0x0880AB28
// @asm 0x08809E58-0x0880AB28
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
void ds_cursor_grid(unsigned short side, PadState *pad);
void ds_cursor_list(unsigned short side, PadState *pad);
void ds_cursor_switch();
void ds_page(int up);
void ds_jump(int up);
int ds_find_group(int up);
void ds_scroll_update(void);
void ds_focus_update(void);

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

#ifdef NON_MATCHING /* campos do painel por índice no original; registradores e ordem dos testes diferem */
/* grid cursor: d-pad moves, wraps to the other pane at the left/right edges, scrolls by rows */
void ds_cursor_grid(unsigned short side, PadState *pad)
{
    unsigned int cols = dsv.pane[side].lay.cols;
    unsigned int cx = dsv.pane[side].cx;
    unsigned int cy = dsv.pane[side].cy;
    int top = (dsv.pane[side].top + cols - 1) / cols;
    unsigned int rows = dsv.pane[side].lay.grid_rows;
    unsigned int rep = pad->b.rep;
    int total = (dsv.pane[side].list->count + cols - 1) / cols;
    unsigned int dy;
    unsigned int dx;
    unsigned short ocy;
    unsigned short otop;
    unsigned int ntop;

    dy = (rep & 0x40) != 0;
    if (rep & 0x10)
        dy--;
    dx = (rep & 0x20) != 0;
    if (rep & 0x80)
        dx--;
    dx = cx + dx;
    if ((int)dx < 0) {
        ds_cursor_switch(side, rows, 1, cy);
        se_play(0x6212);
    } else if ((int)dx < (int)cols) {
        if (top + cy == 0 && (pad->b.trig & 0x10)) {
            if (total < (int)rows)
                top = 0;
            else
                top = total - rows;
            dy = rows - 1;
        } else if ((int)(top + cy) < total - 1 || cy != rows - 1 || !(pad->b.trig & 0x40)) {
            dy = cy + dy;
            if ((int)dy < 0) {
                top += dy;
                if (top < 1)
                    top = 0;
                dy = 0;
            } else if ((int)rows <= (int)dy) {
                if ((int)rows < total) {
                    if ((int)(top + dy) < total)
                        top += dy - (rows - 1);
                    else
                        top = total - rows;
                    dy = rows - 1;
                } else {
                    dy = rows - 1;
                }
            }
        } else {
            dy = 0;
            top = 0;
        }
        if (cx != dx)
            dsv.pane[side].cx = dx;
        ocy = dsv.pane[side].cy;
        if (ocy != dy)
            dsv.pane[side].cy = dy;
        ntop = top * cols;
        otop = dsv.pane[side].top;
        if (otop != ntop)
            dsv.pane[side].top = ntop;
        if (otop != ntop || ocy != dy || cx != dx)
            se_play(0x6212);
    } else {
        ds_cursor_switch(side, rows, 0, cy);
        se_play(0x6212);
    }
}
#endif

#ifdef NON_MATCHING /* idem ds_cursor_grid */
/* list cursor: d-pad up/down with wrap at the ends, left/right go to the other pane */
void ds_cursor_list(unsigned short side, PadState *pad)
{
    unsigned int cy = dsv.pane[side].cy;
    unsigned int rows = dsv.pane[side].lay.rows;
    unsigned int rep = pad->b.rep;
    unsigned int top = dsv.pane[side].top;
    unsigned int total = dsv.pane[side].list->count;
    unsigned int dy;
    unsigned short otop;

    if (rep & 0x80) {
        ds_cursor_switch(side, rows, 1, cy);
        se_play(0x6212);
    } else if (rep & 0x20) {
        ds_cursor_switch(side, rows, 0, cy);
        se_play(0x6212);
    } else {
        dy = (rep & 0x40) != 0;
        if (rep & 0x10)
            dy--;
        if (top + cy == 0 && (pad->b.trig & 0x10)) {
            if (total < rows)
                top = 0;
            else
                top = total - rows;
            dy = rows - 1;
        } else if ((int)(top + cy) < (int)(total - 1) || cy != rows - 1 || !(pad->b.trig & 0x40)) {
            dy = cy + dy;
            if ((int)dy < 0) {
                top += dy;
                if ((int)top < 1)
                    top = 0;
                dy = 0;
            } else if ((int)rows <= (int)dy) {
                if (total < rows)
                    top = 0;
                else if ((int)(top + dy) < (int)total)
                    top += dy - (rows - 1);
                else
                    top = total - rows;
                dy = rows - 1;
            }
        } else {
            dy = 0;
            top = 0;
        }
        if (cy != dy)
            dsv.pane[side].cy = dy;
        otop = dsv.pane[side].top;
        if (otop != top)
            dsv.pane[side].top = top;
        if (otop != top || cy != dy)
            se_play(0x6212);
    }
}
#endif

#ifdef NON_MATCHING /* definição K&R chamada com 4 argumentos; alocação diferente */
/* moves the cursor to the other pane (dir 1 = enter from the right), keeping its height */
void ds_cursor_switch(from, rows, dir, cy, to)
short from;
unsigned short rows;
short dir;
unsigned short cy;
unsigned int to;
{
    short cx = 0;
    unsigned int t;
    unsigned int n;

    if (from == 1) {
        to = 0;
        t = to;
    } else if (from == 0) {
        if (dsv.mode < 2)
            to = 0;
        else
            to = 1;
        t = to;
    } else {
        t = to & 0xFFFF;
    }
    if (dsv.pane[t].list_mode == 0) {
        if (dir == 1) {
            cx = dsv.pane[t].lay.cols - 1;
            n = dsv.pane[t].lay.grid_rows;
        } else {
            n = dsv.pane[t].lay.grid_rows;
        }
    } else {
        n = dsv.pane[t].lay.rows;
    }
    if (n != rows) {
        cy = (int)((rows >> 1) + cy * n) / (int)rows;
        if (n <= cy)
            cy = n - 1;
    }
    dsv.side = to;
    dsv.pane[t].cx = cx;
    dsv.pane[t].cy = cy;
}
#endif

#ifdef NON_MATCHING /* registradores e ordem dos testes diferem */
/* L/R: a page up or down */
void ds_page(int up)
{
    unsigned int side = dsv.side;
    unsigned int total = dsv.pane[side].list->count;
    unsigned short cols;
    unsigned int top;
    unsigned int view;
    int n;

    if (dsv.pane[side].list_mode == 0) {
        cols = dsv.pane[side].lay.cols;
        top = (dsv.pane[side].top + cols - 1) / cols;
        total = (total + cols - 1) / cols;
        view = dsv.pane[side].lay.grid_rows;
    } else {
        view = dsv.pane[side].lay.rows;
        top = dsv.pane[side].top;
        cols = 1;
    }
    if ((int)view < (int)total) {
        if (up == 0) {
            n = top + view;
            if ((int)(total - view) < (int)(top + view))
                n = total - view;
        } else if ((int)top < (int)view) {
            n = 0;
        } else {
            n = top - view;
        }
        if (dsv.pane[side].list_mode == 0)
            dsv.pane[side].top = n * cols;
        else
            dsv.pane[side].top = n;
    } else {
        dsv.pane[side].top = 0;
    }
}
#endif

#ifdef NON_MATCHING /* registradores e ordem dos testes diferem */
/* jumps to the next/previous group of the sort key */
void ds_jump(int up)
{
    unsigned int side = dsv.side;
    unsigned int total = dsv.pane[side].list->count;
    int i;
    unsigned int cols;
    int last;
    int row;
    unsigned short n;

    if (total == 0 || (i = ds_find_group(up)) < 0)
        return;
    if (dsv.pane[side].list_mode == 0) {
        cols = dsv.pane[side].lay.cols;
        last = (int)(total + cols - 1) / (int)cols - dsv.pane[side].lay.grid_rows;
        row = i / (int)cols;
        if (last < i / (int)cols)
            row = last;
        if (row < 0)
            n = 0;
        else
            n = row * dsv.pane[side].lay.cols;
        dsv.pane[side].top = n;
        dsv.pane[side].cx = i % (int)cols;
        dsv.pane[side].cy = (int)(i - dsv.pane[side].top) / (int)cols;
    } else {
        n = i;
        if (i < dsv.pane[side].lay.rows) {
            dsv.pane[side].top = 0;
            dsv.pane[side].cy = n;
        } else {
            last = total - dsv.pane[side].lay.rows;
            if (last < i) {
                dsv.pane[side].top = last;
                dsv.pane[side].cy = n - dsv.pane[side].top;
            } else {
                dsv.pane[side].top = n;
                dsv.pane[side].cy = 0;
            }
        }
    }
}
#endif

#ifdef NON_MATCHING /* laços de busca com o ternário de entrada; alocação diferente */
/* index of the first card of the next (up = 0) or previous sort group from the cursor, -1 none */
int ds_find_group(int up)
{
    DsList *l = dsv.pane[dsv.side].list;
    unsigned int i = ds_cur_index();
    unsigned int n = l->count;
    unsigned int sort = l->sort;
    DsEnt *e;
    int v;
    int w;

    e = DS_ENT(l, i);
    if (e == 0) {
        i = n - 1;
    } else if (up == 0) {
        v = ds_sort_value(e, sort);
        if (sort - 4 < 2 && v > 0)
            v /= 100;
        for (i++; (int)i < (int)n; i++) {
            w = ds_sort_value((int)i < (int)l->count ? &l->ents[l->order[i]] : 0, sort);
            if (sort - 4 < 2 && w > 0)
                w /= 100;
            if (w != v)
                break;
        }
        if (!((int)i < (int)n))
            i = -1;
    } else if (i == 0) {
        i = -1;
    } else {
        v = ds_sort_value((int)(i - 1) < (int)n ? &l->ents[l->order[i - 1]] : 0, sort);
        if (sort - 4 < 2 && v > 0)
            v /= 100;
        for (i -= 2; (int)i >= 0; i--) {
            w = ds_sort_value((int)i < (int)l->count ? &l->ents[l->order[i]] : 0, sort);
            if (sort - 4 < 2 && w > 0)
                w /= 100;
            if (w != v) {
                i++;
                break;
            }
        }
        if ((int)i < 0)
            i = 0;
    }
    return i;
}
#endif

#ifdef NON_MATCHING /* o original guarda max e max + 0x76C0 em registradores */
/* analog scrolling of the card info text (one pane) or of the wide view */
void ds_scroll_update(void)
{
    PadState *pad = pad_get();
    int moved;
    int d;

    if (dsv.mode < 4) {
        ds_scroll_analog(&dsv.cursor.scroll);
    } else if (dsv.scroll_max > 0) {
        moved = 0;
        if (pad->b.analog_x > 0x9E) {
            d = (pad->b.analog_x - 0x9E) * 4;
            if (dsv.scroll_max < dsv.scroll_pos) {
                dsv.scroll_pos += d;
            } else {
                dsv.scroll_pos += d;
                if (dsv.scroll_max < dsv.scroll_pos)
                    dsv.scroll_pos = dsv.scroll_max;
            }
            moved = 1;
        }
        if (pad->b.analog_x < 0x62) {
            dsv.scroll_pos += (pad->b.analog_x - 0x62) * 4;
            if (dsv.scroll_pos < 0)
                dsv.scroll_pos = 0;
            moved = 1;
        }
        if (!moved && dsv.cursor.timer > 0x35)
            dsv.scroll_pos += 0x40;
        if (dsv.scroll_pos >= dsv.scroll_max + 0x76C0)
            dsv.scroll_pos -= dsv.scroll_max + 0x76C0;
    }
}
#endif

#ifdef NON_MATCHING /* o original indexa o painel pela base dsv; o mwcc atual cria o ponteiro */
/* the focus frame follows the cursor */
void ds_focus_update(void)
{
    unsigned int side = dsv.side;
    DsPane *p = &dsv.pane[side];
    short cy;

    if (p->list_mode == 0) {
        ds.focus.x = p->lay.x + p->lay.grid_x + p->cx * p->lay.cell_w;
        ds.focus.w = 0x17;
        ds.focus.y = p->lay.y + p->lay.grid_y + p->cy * p->lay.cell_h;
        ds.focus.h = 0x1F;
        return;
    }
    if (side == 1) {
        cy = dsv.pane[1].cy;
        if (dsv.mode < 4) {
            ds.focus.x = dsv.pane[1].lay.x + dsv.pane[1].lay.row_x;
            ds.focus.y = dsv.pane[1].lay.y + dsv.pane[1].lay.row_y + dsv.pane[1].cy * dsv.pane[1].lay.row_h;
            ds.focus.w = dsv.pane[1].lay.row_w - 1;
            ds.focus.h = dsv.pane[1].lay.row_h - 1;
            return;
        }
    } else {
        cy = p->cy;
    }
    ds.focus.x = p->lay.x + p->lay.row_x + 1;
    ds.focus.w = p->lay.row_w - 3;
    ds.focus.y = p->lay.y + p->lay.row_y + 1 + cy * p->lay.row_h;
    ds.focus.h = p->lay.row_h - 1;
}
#endif
