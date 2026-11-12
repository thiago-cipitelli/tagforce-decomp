// @range 0x0880931C-0x0880A694
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
void dk_word_update(PadBits *pad);
int dk_popup_update(void);
int dk_deck_check(unsigned short *arg);
void dk_popup_open(int id, unsigned short arg);
void dk_set_mode(unsigned short mode);
void dk_sort_menu_open(void);
void dk_fmenu_open(void);
void inv_set_flag7(unsigned short id, int on);

void dk_input_lists(PadBits *pad);
void dk_input_shoulder_l(PadBits *pad);
void dk_input_shoulder_r(PadBits *pad);
void dk_cursor_grid(int side, PadBits *pad);
void dk_cursor_list(int side, PadBits *pad);
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
        return dk_popup_update();
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
        dk_word_update(pad);
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
            switch (r = dk_deck_check(&arg)) {
            case 0:
                dk_popup_open(3, 0);
                se_play(0x621A);
                break;
            case 5:
                dk_popup_open(5, arg);
                se_play(0x621A);
                break;
            default:
                dk_popup_open(4, r);
                se_play(0x621A);
                break;
            }
        } else if (trig & region_pick_value(0)) {
            e = dk_flight_open();
            if (e > 0) {
                dk_popup_open(2, e);
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
                dk_set_mode(m);
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

/* grid cursor: d-pad moves, wraps to the other pane at the left/right edges, scrolls by rows */
void dk_cursor_grid(int side, PadBits *pad)
{
    DkPane *p = &dkv.pane[(unsigned short)side];
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
    rep = pad->rep;
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
        dk_cursor_switch(side, (unsigned short)rows, 1, (unsigned short)cy);
        se_play(0x6212);
    } else if (nx >= cols) {
        dk_cursor_switch(side, (unsigned short)rows, 0, (unsigned short)cy);
        se_play(0x6212);
    } else {
        if (top + cy == 0 && (pad->trig & 0x10)) {
            if (total >= rows)
                top = total - rows;
            else
                top = 0;
            ny = rows - 1;
        } else if (top + cy >= total - 1 && cy == rows - 1 && (pad->trig & 0x40)) {
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

/* 3 palavras (forma de sc/dt_cursor_list, asm idêntico): "if (top <= 0) top = 0" sai blezl e "b; ny = 0" no original */
/* list cursor: d-pad up/down with wrap at the ends, left/right go to the other pane */
void dk_cursor_list(int side, PadBits *pad)
{
    DkPane *p = &dkv.pane[(unsigned short)side];
    int cy = p->cy;
    int rows = p->lay.rows;
    unsigned int rep = pad->rep;
    int top = p->top;
    int total = p->list->count;
    int changed = 0;
    int dy;
    int ny;

    if (rep & 0x80) {
        dk_cursor_switch(side, (unsigned short)rows, 1, (unsigned short)cy);
        se_play(0x6212);
    } else if (rep & 0x20) {
        dk_cursor_switch(side, (unsigned short)rows, 0, (unsigned short)cy);
        se_play(0x6212);
    } else {
        dy = 0;
        if (rep & 0x40)
            dy++;
        if (rep & 0x10)
            dy--;
        if (top + cy == 0 && (pad->trig & 0x10)) {
            if (total >= rows)
                top = total - rows;
            else
                top = 0;
            ny = rows - 1;
        } else if (top + cy >= total - 1 && cy == rows - 1 && (pad->trig & 0x40)) {
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
void dk_cursor_switch(from, rows, dir, cy, to)
unsigned short from;
unsigned short rows;
unsigned short dir;
unsigned short cy;
unsigned short to;
{
    unsigned short cx = 0;
    unsigned short n;
    DkPane *p;

    switch (from) {
    case 0:
        if (dkv.mode > 1)
            to = 1;
        else
            to = 0;
        break;
    case 1:
        to = 0;
        break;
    }
    p = &dkv.pane[to];
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
    dkv.side = to;
    p->cx = cx;
    p->cy = cy;
}

/* L/R: a page up or down */
void dk_page(int up)
{
    DkPane *p = &dkv.pane[dkv.side];
    int total = p->list->count;
    int cols;
    int top;
    int view;

    if (p->list_mode == 0) {
        cols = p->lay.cols;
        top = p->top / cols;
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
void dk_jump(int up)
{
    DkPane *p = &dkv.pane[dkv.side];
    int total = p->list->count;
    int i;
    int cols;
    int row;
    int last;

    if (total == 0)
        return;
    i = dk_find_group(up);
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

 /* laços de busca com o ternário de entrada; alocação diferente */
/* index of the first card of the next (up = 0) or previous sort group from the cursor, -1 none */
int dk_find_group(int up)
{
    int k;
    DkList *l = dkv.pane[dkv.side].list;
    int i = (unsigned short)dk_cur_index(dkv.side);
    unsigned short sort = l->sort;
    int n = l->count;
    DkEnt *e = i >= n ? 0 : &l->ents[l->order[i]];
    int v;
    int w;
    int j;

    if (e == 0)
        return n - 1;
    if (up) {
        if (i <= 0)
            return -1;
        j = i - 1;
        v = dk_sort_value(j >= n ? 0 : &l->ents[l->order[j]], sort);
        if ((sort == 4 || sort == 5) && v > 0)
            v /= 100;
        for (k = j - 1; k >= 0; k--) {
            w = dk_sort_value(DK_ENT(l, k), sort);
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
        v = dk_sort_value(e, sort);
        if ((sort == 4 || sort == 5) && v > 0)
            v /= 100;
        for (k = i + 1; k < n; k++) {
            w = dk_sort_value(DK_ENT(l, k), sort);
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

 /* o original guarda max e max + 0x76C0 em registradores */
void dk_scroll_update(void)
{
    PadState *pad = pad_get();
    int moved;
    int max;
    int lim;
    int v;

    if (dkv.mode < 4) {
        dk_scroll_analog(&dkv.cursor.scroll);
    } else {
        max = dkv.scroll_max;
        if (max > 0) {
            moved = 0;
            lim = max + 0x76C0;
            if (pad->b.analog_x > 0x9E) {
                v = (pad->b.analog_x - 0x9E) * 4;
                if (max < dkv.scroll_pos) {
                    dkv.scroll_pos += v;
                } else {
                    if (max < dkv.scroll_pos + v)
                        dkv.scroll_pos = max;
                    else
                        dkv.scroll_pos += v;
                }
                moved = 1;
            }
            if (pad->b.analog_x < 0x62) {
                v = dkv.scroll_pos + (pad->b.analog_x - 0x62) * 4;
                if (v < 0)
                    dkv.scroll_pos = 0;
                else
                    dkv.scroll_pos = v;
                moved = 1;
            }
            if (!moved && dkv.cursor.timer >= 0x36)
                dkv.scroll_pos += 0x40;
            if (dkv.scroll_pos >= lim)
                dkv.scroll_pos -= lim;
        }
    }
}

/* the focus frame follows the cursor */
void dk_focus_update(void)
{
    DkView *v = &dkv;
    unsigned char side = v->side;
    DkPane *p = &v->pane[side];
    DkLayout *lay = &p->lay;
    DkRect *f = &dk_focus;

    if (p->list_mode == 0) {
        f->x = *(volatile short *)&lay->x + lay->grid_x + p->cx * (unsigned short)lay->cell_w; /* volatile: x's loads first */
        f->y = lay->y + lay->grid_y + p->cy * (unsigned short)lay->cell_h;
        f->w = 0x17;
        f->h = 0x1F;
    } else if (side == 1 && dkv.mode < 4) {
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
