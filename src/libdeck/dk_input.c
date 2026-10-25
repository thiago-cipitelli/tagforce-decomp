// @range 0x0880931C-0x0880A694
// @asm 0x088099CC-0x0880A694
// @rodata 0x08822534-0x08822554
/* Input of the list screen: dispatch by scene state, the main keys (Circle move, Cross quick move or
   info, Triangle action menu, Square switch tab, Select pane layout, L/R pages and sort groups with
   the shoulder keys held), the grid and list cursors (wrapping to the other pane), page and sort
   group jumps, the analog scroll of the card text and the focus frame. */
#include "libdeck.h"

#define DK_ENT(l, i) ((i) >= (l)->count ? 0 : &(l)->ents[(l)->order[i]])

void dk_scroll_analog(DkScroll *s);
int dk_sort_value(DkEnt *e, unsigned short sort);
void dk_list_filter_reset(DkList *l);
void dk_list_sort(DkList *l, int reset, int keep);
int dk_mode_for(unsigned short panes, int narrow, unsigned short mode_a, unsigned short mode_b);
void dk_action_open(DkEnt *e);
void dk_action_update(PadBits *pad);
void dk_sort_menu_update(PadBits *pad);
void dk_fmenu_update(PadBits *pad);
void func_08819460(PadBits *pad);
int func_0880AD9C(void);
int func_0880A694(unsigned short *arg);
void func_0880A8BC(int id, unsigned short arg);
void func_0880AF34(unsigned short mode);
void dk_sort_menu_open(void);
void dk_fmenu_open(void);
void inv_set_flag7(unsigned short id, int on);

void dk_input_lists(PadBits *pad);
void dk_input_shoulder_l(PadBits *pad);
void dk_input_shoulder_r(PadBits *pad);
void dk_cursor_grid(unsigned short side, PadBits *pad);
void dk_cursor_list(unsigned short side, PadBits *pad);
void dk_cursor_switch();
void dk_page(int up);
void dk_jump(int up);
int dk_find_group(int up);
void dk_scroll_update(void);
void dk_focus_update(void);

/* per frame input of the scene (nothing while busy, help or a deck operation is pending) */
int dk_input(PadBits *pad)
{
    if (dkv.busy)
        return 0;
    if (dk_flags.b7)
        return 0;
    if (dk_flags.info)
        return 0;
    if (dk_help_page != 0)
        return 0;
    if (dk_flags.deck_op != 0)
        return 0;
    dk_scroll_update();
    if (dkv.popup != 0)
        return func_0880AD9C();
    switch (dk_state) {
    case 0:
    case 6:
    case 7:
        dk_input_lists(pad);
        break;
    case 1:
        dk_flight_update(pad);
        break;
    case 2:
        dk_action_update(pad);
        break;
    case 3:
        dk_sort_menu_update(pad);
        break;
    case 4:
        dk_fmenu_update(pad);
        break;
    case 5:
        func_08819460(pad);
        break;
    }
    dk_focus_update();
    if ((pad->down & 8) && dk_help_page == 0)
        dk_help_page = 0xFF;
    return 0;
}

/* the lists: d-pad cursor, L/R held for pages and groups, the action keys */
void dk_input_lists(PadBits *pad)
{
    unsigned char side = dkv.side;
    DkPane *p = &dkv.pane[side];
    unsigned int trig;
    unsigned short arg;
    int r;
    unsigned short a;
    unsigned short b;
    int mode;
    unsigned short panes;
    unsigned short e;
    int m;

    if (pad->rep & 0xF0) {
        if (p->list_mode == 0) {
            dk_cursor_grid(side, pad);
            return;
        }
        dk_cursor_list(side, pad);
    } else if (pad->down & 0x100) {
        dk_state = 6;
        dk_input_shoulder_l(pad);
    } else if (pad->down & 0x200) {
        dk_state = 7;
        dk_input_shoulder_r(pad);
    } else {
        dk_state = 0;
        trig = pad->trig;
        if (trig & region_pick_value(1)) {
            switch (r = func_0880A694(&arg)) {
            case 0:
                func_0880A8BC(3, 0);
                se_play(0x621A);
                break;
            case 5:
                func_0880A8BC(5, arg);
                se_play(0x621A);
                break;
            default:
                func_0880A8BC(4, r);
                se_play(0x621A);
                break;
            }
        } else if (trig & region_pick_value(0)) {
            e = dk_flight_open();
            if (e > 0) {
                func_0880A8BC(2, e);
                se_play(0x621A);
            } else {
                se_play(0x6213);
            }
        } else if (trig & 0x1000) {
            dk_action_open(dk_cur_entry(dkv.side));
            se_play(0x6216);
        } else if (trig & 0x8000) {
            side = dkv.side;
            dk_select_tab(side, dkv.pane[side].list->tab, 1);
            se_play(0x621B);
        } else if (trig & 1) {
            mode = dkv.mode;
            panes = mode > 1 ? 2 : 1;
            a = dkv.pane[0].list_mode;
            b = dkv.pane[1].list_mode;
            if (dkv.side == 0)
                a = (a + 1) & 1;
            else
                b = (b + 1) & 1;
            m = dk_mode_for(panes, mode < 4, a, b);
            if (m >= 0) {
                func_0880AF34(m);
                se_play(0x6213);
            }
        }
    }
}

/* L held: d-pad up/down pages, left/right sort groups, Select clears the filter */
void dk_input_shoulder_l(PadBits *pad)
{
    unsigned int rep = pad->rep;
    DkList *l;

    if (rep & 0x8000) {
        dk_page(1);
        se_play(0x6212);
    } else if (rep & 0x2000) {
        dk_page(0);
        se_play(0x6212);
    } else if (rep & 0x1000) {
        dk_jump(1);
        se_play(0x6212);
    } else if (rep & 0x4000) {
        dk_jump(0);
        se_play(0x6212);
    } else if (pad->trig & 1) {
        l = dkv.pane[dkv.side].list;
        if (l->filter != 0) {
            dk_list_filter_reset(l);
            dk_list_sort(l, l->sort, l->dir);
            se_play(0x6213);
        }
    }
}

/* R held: Circle quick move, Cross "new" mark, Triangle/Square the sort and filter menus,
   Select undo */
void dk_input_shoulder_r(PadBits *pad)
{
    unsigned int trig = pad->trig;
    DkEnt *e;
    unsigned short id;

    if (trig & region_pick_value(0)) {
        dk_flight_quick();
        se_play(0x6213);
    } else if (trig & region_pick_value(1)) {
        e = dk_cur_entry(dkv.side);
        if (e != 0) {
            id = e->id;
            if (inv_get_flag7(id) != 0)
                inv_set_flag7(id, 0);
            else
                inv_set_flag7(id, 1);
            se_play(0x6213);
        }
    } else if (trig & 0x8000) {
        dk_sort_menu_open();
        se_play(0x6216);
    } else if (trig & 0x1000) {
        dk_fmenu_open();
        se_play(0x6216);
    } else if ((trig & 1) && dk_undo_count > 0) {
        dk_flight_undo();
        se_play(0x6213);
    }
}

#ifdef NON_MATCHING /* cursores, páginas, grupos, rolagem e foco: mesmas funções que ficaram em asm na libselect_card (sc_input.c); registradores e ordem dos testes diferem */
/* grid cursor: d-pad moves, wraps to the other pane at the left/right edges, scrolls by rows */
void dk_cursor_grid(unsigned short side, PadBits *pad)
{
    unsigned int cols = dkv.pane[side].lay.cols;
    unsigned int cx = dkv.pane[side].cx;
    unsigned int cy = dkv.pane[side].cy;
    int top = (dkv.pane[side].top + cols - 1) / cols;
    unsigned int rows = dkv.pane[side].lay.grid_rows;
    unsigned int rep = pad->rep;
    int total = (dkv.pane[side].list->count + cols - 1) / cols;
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
        dk_cursor_switch(side, rows, 1, cy);
        se_play(0x6212);
    } else if ((int)dx < (int)cols) {
        if (top + cy == 0 && (pad->trig & 0x10)) {
            if (total < (int)rows)
                top = 0;
            else
                top = total - rows;
            dy = rows - 1;
        } else if ((int)(top + cy) < total - 1 || cy != rows - 1 || !(pad->trig & 0x40)) {
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
            dkv.pane[side].cx = dx;
        ocy = dkv.pane[side].cy;
        if (ocy != dy)
            dkv.pane[side].cy = dy;
        ntop = top * cols;
        otop = dkv.pane[side].top;
        if (otop != ntop)
            dkv.pane[side].top = ntop;
        if (otop != ntop || ocy != dy || cx != dx)
            se_play(0x6212);
    } else {
        dk_cursor_switch(side, rows, 0, cy);
        se_play(0x6212);
    }
}

 /* idem dk_cursor_grid */
/* list cursor: d-pad up/down with wrap at the ends, left/right go to the other pane */
void dk_cursor_list(unsigned short side, PadBits *pad)
{
    unsigned int cy = dkv.pane[side].cy;
    unsigned int rows = dkv.pane[side].lay.rows;
    unsigned int rep = pad->rep;
    unsigned int top = dkv.pane[side].top;
    unsigned int total = dkv.pane[side].list->count;
    unsigned int dy;
    unsigned short otop;

    if (rep & 0x80) {
        dk_cursor_switch(side, rows, 1, cy);
        se_play(0x6212);
    } else if (rep & 0x20) {
        dk_cursor_switch(side, rows, 0, cy);
        se_play(0x6212);
    } else {
        dy = (rep & 0x40) != 0;
        if (rep & 0x10)
            dy--;
        if (top + cy == 0 && (pad->trig & 0x10)) {
            if (total < rows)
                top = 0;
            else
                top = total - rows;
            dy = rows - 1;
        } else if ((int)(top + cy) < (int)(total - 1) || cy != rows - 1 || !(pad->trig & 0x40)) {
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
            dkv.pane[side].cy = dy;
        otop = dkv.pane[side].top;
        if (otop != top)
            dkv.pane[side].top = top;
        if (otop != top || cy != dy)
            se_play(0x6212);
    }
}

 /* definição K&R chamada com 4 argumentos; alocação diferente */
/* moves the cursor to the other pane (dir 1 = enter from the right), keeping its height */
void dk_cursor_switch(from, rows, dir, cy, to)
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
        if (dkv.mode < 2)
            to = 0;
        else
            to = 1;
        t = to;
    } else {
        t = to & 0xFFFF;
    }
    if (dkv.pane[t].list_mode == 0) {
        if (dir == 1) {
            cx = dkv.pane[t].lay.cols - 1;
            n = dkv.pane[t].lay.grid_rows;
        } else {
            n = dkv.pane[t].lay.grid_rows;
        }
    } else {
        n = dkv.pane[t].lay.rows;
    }
    if (n != rows) {
        cy = (int)((rows >> 1) + cy * n) / (int)rows;
        if (n <= cy)
            cy = n - 1;
    }
    dkv.side = to;
    dkv.pane[t].cx = cx;
    dkv.pane[t].cy = cy;
}

 /* registradores e ordem dos testes diferem */
/* L/R: a page up or down */
void dk_page(int up)
{
    unsigned int side = dkv.side;
    unsigned int total = dkv.pane[side].list->count;
    unsigned short cols;
    unsigned int top;
    unsigned int view;
    int n;

    if (dkv.pane[side].list_mode == 0) {
        cols = dkv.pane[side].lay.cols;
        top = (dkv.pane[side].top + cols - 1) / cols;
        total = (total + cols - 1) / cols;
        view = dkv.pane[side].lay.grid_rows;
    } else {
        view = dkv.pane[side].lay.rows;
        top = dkv.pane[side].top;
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
        if (dkv.pane[side].list_mode == 0)
            dkv.pane[side].top = n * cols;
        else
            dkv.pane[side].top = n;
    } else {
        dkv.pane[side].top = 0;
    }
}

 /* registradores e ordem dos testes diferem */
/* jumps to the next/previous group of the sort key */
void dk_jump(int up)
{
    unsigned int side = dkv.side;
    unsigned int total = dkv.pane[side].list->count;
    int i;
    unsigned int cols;
    int last;
    int row;
    unsigned short n;

    if (total == 0 || (i = dk_find_group(up)) < 0)
        return;
    if (dkv.pane[side].list_mode == 0) {
        cols = dkv.pane[side].lay.cols;
        last = (int)(total + cols - 1) / (int)cols - dkv.pane[side].lay.grid_rows;
        row = i / (int)cols;
        if (last < i / (int)cols)
            row = last;
        if (row < 0)
            n = 0;
        else
            n = row * dkv.pane[side].lay.cols;
        dkv.pane[side].top = n;
        dkv.pane[side].cx = i % (int)cols;
        dkv.pane[side].cy = (int)(i - dkv.pane[side].top) / (int)cols;
    } else {
        n = i;
        if (i < dkv.pane[side].lay.rows) {
            dkv.pane[side].top = 0;
            dkv.pane[side].cy = n;
        } else {
            last = total - dkv.pane[side].lay.rows;
            if (last < i) {
                dkv.pane[side].top = last;
                dkv.pane[side].cy = n - dkv.pane[side].top;
            } else {
                dkv.pane[side].top = n;
                dkv.pane[side].cy = 0;
            }
        }
    }
}

 /* laços de busca com o ternário de entrada; alocação diferente */
/* index of the first card of the next (up = 0) or previous sort group from the cursor, -1 none */
int dk_find_group(int up)
{
    DkList *l = dkv.pane[dkv.side].list;
    unsigned int i = dk_cur_index(dkv.side);
    unsigned int n = l->count;
    unsigned int sort = l->sort;
    DkEnt *e;
    int v;
    int w;

    e = DK_ENT(l, i);
    if (e == 0) {
        i = n - 1;
    } else if (up == 0) {
        v = dk_sort_value(e, sort);
        if (sort - 4 < 2 && v > 0)
            v /= 100;
        for (i++; (int)i < (int)n; i++) {
            w = dk_sort_value((int)i < (int)l->count ? &l->ents[l->order[i]] : 0, sort);
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
        v = dk_sort_value((int)(i - 1) < (int)n ? &l->ents[l->order[i - 1]] : 0, sort);
        if (sort - 4 < 2 && v > 0)
            v /= 100;
        for (i -= 2; (int)i >= 0; i--) {
            w = dk_sort_value((int)i < (int)l->count ? &l->ents[l->order[i]] : 0, sort);
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

 /* o original guarda max e max + 0x76C0 em registradores */
/* analog scrolling of the card info text (one pane) or of the wide view */
void dk_scroll_update(void)
{
    PadBits *pad = &pad_get()->b;
    int moved;
    int d;

    if (dkv.mode < 4) {
        dk_scroll_analog(&dkv.cursor.scroll);
    } else if (dkv.scroll_max > 0) {
        moved = 0;
        if (pad->analog_x > 0x9E) {
            d = (pad->analog_x - 0x9E) * 4;
            if (dkv.scroll_max < dkv.scroll_pos) {
                dkv.scroll_pos += d;
            } else {
                dkv.scroll_pos += d;
                if (dkv.scroll_max < dkv.scroll_pos)
                    dkv.scroll_pos = dkv.scroll_max;
            }
            moved = 1;
        }
        if (pad->analog_x < 0x62) {
            dkv.scroll_pos += (pad->analog_x - 0x62) * 4;
            if (dkv.scroll_pos < 0)
                dkv.scroll_pos = 0;
            moved = 1;
        }
        if (!moved && dkv.cursor.timer > 0x35)
            dkv.scroll_pos += 0x40;
        if (dkv.scroll_pos >= dkv.scroll_max + 0x76C0)
            dkv.scroll_pos -= dkv.scroll_max + 0x76C0;
    }
}

/* the focus frame follows the cursor */
void dk_focus_update(void)
{
    unsigned int side = dkv.side;
    DkPane *p = &dkv.pane[side];
    short cy;

    if (p->list_mode == 0) {
        dk_focus.x = p->lay.x + p->lay.grid_x + p->cx * p->lay.cell_w;
        dk_focus.w = 0x17;
        dk_focus.y = p->lay.y + p->lay.grid_y + p->cy * p->lay.cell_h;
        dk_focus.h = 0x1F;
        return;
    }
    if (side == 1) {
        cy = dkv.pane[1].cy;
        if (dkv.mode < 4) {
            dk_focus.x = dkv.pane[1].lay.x + dkv.pane[1].lay.row_x;
            dk_focus.y = dkv.pane[1].lay.y + dkv.pane[1].lay.row_y + dkv.pane[1].cy * dkv.pane[1].lay.row_h;
            dk_focus.w = dkv.pane[1].lay.row_w - 1;
            dk_focus.h = dkv.pane[1].lay.row_h - 1;
            return;
        }
    } else {
        cy = p->cy;
    }
    dk_focus.x = p->lay.x + p->lay.row_x + 1;
    dk_focus.w = p->lay.row_w - 3;
    dk_focus.y = p->lay.y + p->lay.row_y + 1 + cy * p->lay.row_h;
    dk_focus.h = p->lay.row_h - 1;
}
#endif
