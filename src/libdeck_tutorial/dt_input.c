// @range 0x088100AC-0x08811BB8
// @asm 0x088106B4-0x0881137C
// @rodata 0x08820FF0-0x08821030
// @data 0x08821F4C-0x08821F74
/* Input of the editor (the code of libdeck dk_input.c and the popup of dk_mode.c): dispatch by
   editor state, the main keys, the grid and list cursors, page and sort group jumps, the analog
   scroll of the card text and the focus frame; the deck check before saving and the message popup,
   which reads the input the lesson gives the editor. */
#include "libdeck_tutorial.h"

extern unsigned short dt_popup_arg;            /* 0x08823DDC */
extern unsigned short dt_popup_text[0x400];    /* 0x08823DE0 */
extern MenuWidget dt_popup;                    /* 0x088245E0 */

int card_get_limit(int id);
int sysflag_test(unsigned int bit);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
unsigned short *table_get_entry(void *t, unsigned int i);
unsigned short *card_get_name(int id);
void *dt_eres_get(unsigned short id);
int dt_list_count_id(DtList *l, unsigned short id, int same_name);
void dt_check_rewards(void);
unsigned short dt_cur_index(unsigned short side);
DtEnt *dt_cur_entry(unsigned short side);
void dt_select_tab(unsigned short side, unsigned short tab, unsigned short how);
int dt_flight_open(void);
int dt_flight_undo(void);
int dt_flight_quick(void);
int dt_flight_update(PadBits *pad);

#define DK_ENT(l, i) ((i) >= (l)->count ? 0 : &(l)->ents[(l)->order[i]])

void dt_scroll_analog(DtScroll *s);
int dt_sort_value(DtEnt *e, unsigned short sort);
void dt_list_filter_reset(DtList *l);
void dt_list_sort(DtList *l, int reset, int keep);
int dt_mode_for(unsigned short panes, int narrow, unsigned short mode_a, unsigned short mode_b);
void dt_action_open(DtEnt *e);
void dt_action_update(PadBits *pad);
void dt_sort_menu_update(PadBits *pad);
void dt_fmenu_update(PadBits *pad);
void dt_word_update(PadBits *pad);
int dt_popup_update(void);
int dt_deck_check(unsigned short *arg);
void dt_popup_open(unsigned short id, unsigned short arg);
void dt_set_mode(unsigned short mode);
void dt_sort_menu_open(void);
void dt_fmenu_open(void);
void inv_set_flag7(unsigned short id, int on);

void dt_input_lists(PadBits *pad);
void dt_input_shoulder_l(PadBits *pad);
void dt_input_shoulder_r(PadBits *pad);
void dt_cursor_grid(unsigned short side, PadBits *pad);
void dt_cursor_list(unsigned short side, PadBits *pad);
void dt_cursor_switch();
void dt_page(int up);
void dt_jump(int up);
int dt_find_group(int up);
void dt_scroll_update(void);
void dt_focus_update(void);


/* per frame input of the editor */
int dt_input(PadBits *pad)
{
    dt_scroll_update();
    if (dtv.popup != 0)
        return dt_popup_update();
    switch (dts.state) {
    case 0:
    case 6:
    case 7:
        dt_input_lists(pad);
        break;
    case 1:
        dt_flight_update(pad);
        break;
    case 2:
        dt_action_update(pad);
        break;
    case 3:
        dt_sort_menu_update(pad);
        break;
    case 4:
        dt_fmenu_update(pad);
        break;
    case 5:
        dt_word_update(pad);
        break;
    }
    dt_focus_update();
    return 0;
}


/* the lists: d-pad cursor, L/R held for pages and groups, the action keys */
void dt_input_lists(PadBits *pad)
{
    unsigned char side = dtv.side;
    DtPane *p = &dtv.pane[side];
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
            dt_cursor_grid(side, pad);
            return;
        }
        dt_cursor_list(side, pad);
    } else if (pad->down & 0x100) {
        dts.state = 6;
        dt_input_shoulder_l(pad);
    } else if (pad->down & 0x200) {
        dts.state = 7;
        dt_input_shoulder_r(pad);
    } else {
        dts.state = 0;
        trig = pad->trig;
        if (trig & region_pick_value(1)) {
            switch (r = dt_deck_check(&arg)) {
            case 0:
                dt_popup_open(3, 0);
                se_play(0x621A);
                break;
            case 5:
                dt_popup_open(5, arg);
                se_play(0x621A);
                break;
            default:
                dt_popup_open(4, r);
                se_play(0x621A);
                break;
            }
        } else if (trig & region_pick_value(0)) {
            e = dt_flight_open();
            if (e > 0) {
                dt_popup_open(2, e);
                se_play(0x621A);
            } else {
                se_play(0x6213);
            }
        } else if (trig & 0x1000) {
            dt_action_open(dt_cur_entry(dtv.side));
            se_play(0x6216);
        } else if (trig & 0x8000) {
            side = dtv.side;
            dt_select_tab(side, dtv.pane[side].list->tab, 1);
            se_play(0x621B);
        } else if (trig & 1) {
            mode = dtv.mode;
            panes = mode > 1 ? 2 : 1;
            a = dtv.pane[0].list_mode;
            b = dtv.pane[1].list_mode;
            if (dtv.side == 0)
                a = (a + 1) & 1;
            else
                b = (b + 1) & 1;
            m = dt_mode_for(panes, mode < 4, a, b);
            if (m >= 0) {
                dt_set_mode(m);
                se_play(0x6213);
            }
        }
    }
}

/* L held: d-pad up/down pages, left/right sort groups, Select clears the filter */
void dt_input_shoulder_l(PadBits *pad)
{
    unsigned int rep = pad->rep;
    DtList *l;

    if (rep & 0x8000) {
        dt_page(1);
        se_play(0x6212);
    } else if (rep & 0x2000) {
        dt_page(0);
        se_play(0x6212);
    } else if (rep & 0x1000) {
        dt_jump(1);
        se_play(0x6212);
    } else if (rep & 0x4000) {
        dt_jump(0);
        se_play(0x6212);
    } else if (pad->trig & 1) {
        l = dtv.pane[dtv.side].list;
        if (l->filter != 0) {
            dt_list_filter_reset(l);
            dt_list_sort(l, l->sort, l->dir);
            se_play(0x6213);
        }
    }
}

/* R held: Circle quick move, Cross "new" mark, Triangle/Square the sort and filter menus,
   Select undo */
void dt_input_shoulder_r(PadBits *pad)
{
    unsigned int trig = pad->trig;
    DtEnt *e;
    unsigned short id;

    if (trig & region_pick_value(0)) {
        dt_flight_quick();
        se_play(0x6213);
    } else if (trig & region_pick_value(1)) {
        e = dt_cur_entry(dtv.side);
        if (e != 0) {
            id = e->id;
            if (inv_get_flag7(id) != 0)
                inv_set_flag7(id, 0);
            else
                inv_set_flag7(id, 1);
            se_play(0x6213);
        }
    } else if (trig & 0x8000) {
        dt_sort_menu_open();
        se_play(0x6216);
    } else if (trig & 0x1000) {
        dt_fmenu_open();
        se_play(0x6216);
    } else if ((trig & 1) && dt_undo_count > 0) {
        dt_flight_undo();
        se_play(0x6213);
    }
}

#ifdef NON_MATCHING /* cursores, páginas, grupos, rolagem e foco: mesmas funções que ficaram em asm na libselect_card (sc_input.c); registradores e ordem dos testes diferem */
/* grid cursor: d-pad moves, wraps to the other pane at the left/right edges, scrolls by rows */
void dt_cursor_grid(unsigned short side, PadBits *pad)
{
    unsigned int cols = dtv.pane[side].lay.cols;
    unsigned int cx = dtv.pane[side].cx;
    unsigned int cy = dtv.pane[side].cy;
    int top = (dtv.pane[side].top + cols - 1) / cols;
    unsigned int rows = dtv.pane[side].lay.grid_rows;
    unsigned int rep = pad->rep;
    int total = (dtv.pane[side].list->count + cols - 1) / cols;
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
        dt_cursor_switch(side, rows, 1, cy);
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
            dtv.pane[side].cx = dx;
        ocy = dtv.pane[side].cy;
        if (ocy != dy)
            dtv.pane[side].cy = dy;
        ntop = top * cols;
        otop = dtv.pane[side].top;
        if (otop != ntop)
            dtv.pane[side].top = ntop;
        if (otop != ntop || ocy != dy || cx != dx)
            se_play(0x6212);
    } else {
        dt_cursor_switch(side, rows, 0, cy);
        se_play(0x6212);
    }
}

 /* idem dt_cursor_grid */
/* list cursor: d-pad up/down with wrap at the ends, left/right go to the other pane */
void dt_cursor_list(unsigned short side, PadBits *pad)
{
    unsigned int cy = dtv.pane[side].cy;
    unsigned int rows = dtv.pane[side].lay.rows;
    unsigned int rep = pad->rep;
    unsigned int top = dtv.pane[side].top;
    unsigned int total = dtv.pane[side].list->count;
    unsigned int dy;
    unsigned short otop;

    if (rep & 0x80) {
        dt_cursor_switch(side, rows, 1, cy);
        se_play(0x6212);
    } else if (rep & 0x20) {
        dt_cursor_switch(side, rows, 0, cy);
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
            dtv.pane[side].cy = dy;
        otop = dtv.pane[side].top;
        if (otop != top)
            dtv.pane[side].top = top;
        if (otop != top || cy != dy)
            se_play(0x6212);
    }
}

 /* definição K&R chamada com 4 argumentos; alocação diferente */
/* moves the cursor to the other pane (dir 1 = enter from the right), keeping its height */
void dt_cursor_switch(from, rows, dir, cy, to)
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
        if (dtv.mode < 2)
            to = 0;
        else
            to = 1;
        t = to;
    } else {
        t = to & 0xFFFF;
    }
    if (dtv.pane[t].list_mode == 0) {
        if (dir == 1) {
            cx = dtv.pane[t].lay.cols - 1;
            n = dtv.pane[t].lay.grid_rows;
        } else {
            n = dtv.pane[t].lay.grid_rows;
        }
    } else {
        n = dtv.pane[t].lay.rows;
    }
    if (n != rows) {
        cy = (int)((rows >> 1) + cy * n) / (int)rows;
        if (n <= cy)
            cy = n - 1;
    }
    dtv.side = to;
    dtv.pane[t].cx = cx;
    dtv.pane[t].cy = cy;
}

 /* registradores e ordem dos testes diferem */
/* L/R: a page up or down */
void dt_page(int up)
{
    unsigned int side = dtv.side;
    unsigned int total = dtv.pane[side].list->count;
    unsigned short cols;
    unsigned int top;
    unsigned int view;
    int n;

    if (dtv.pane[side].list_mode == 0) {
        cols = dtv.pane[side].lay.cols;
        top = (dtv.pane[side].top + cols - 1) / cols;
        total = (total + cols - 1) / cols;
        view = dtv.pane[side].lay.grid_rows;
    } else {
        view = dtv.pane[side].lay.rows;
        top = dtv.pane[side].top;
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
        if (dtv.pane[side].list_mode == 0)
            dtv.pane[side].top = n * cols;
        else
            dtv.pane[side].top = n;
    } else {
        dtv.pane[side].top = 0;
    }
}

 /* registradores e ordem dos testes diferem */
/* jumps to the next/previous group of the sort key */
void dt_jump(int up)
{
    unsigned int side = dtv.side;
    unsigned int total = dtv.pane[side].list->count;
    int i;
    unsigned int cols;
    int last;
    int row;
    unsigned short n;

    if (total == 0 || (i = dt_find_group(up)) < 0)
        return;
    if (dtv.pane[side].list_mode == 0) {
        cols = dtv.pane[side].lay.cols;
        last = (int)(total + cols - 1) / (int)cols - dtv.pane[side].lay.grid_rows;
        row = i / (int)cols;
        if (last < i / (int)cols)
            row = last;
        if (row < 0)
            n = 0;
        else
            n = row * dtv.pane[side].lay.cols;
        dtv.pane[side].top = n;
        dtv.pane[side].cx = i % (int)cols;
        dtv.pane[side].cy = (int)(i - dtv.pane[side].top) / (int)cols;
    } else {
        n = i;
        if (i < dtv.pane[side].lay.rows) {
            dtv.pane[side].top = 0;
            dtv.pane[side].cy = n;
        } else {
            last = total - dtv.pane[side].lay.rows;
            if (last < i) {
                dtv.pane[side].top = last;
                dtv.pane[side].cy = n - dtv.pane[side].top;
            } else {
                dtv.pane[side].top = n;
                dtv.pane[side].cy = 0;
            }
        }
    }
}

 /* laços de busca com o ternário de entrada; alocação diferente */
/* index of the first card of the next (up = 0) or previous sort group from the cursor, -1 none */
int dt_find_group(int up)
{
    DtList *l = dtv.pane[dtv.side].list;
    unsigned int i = dt_cur_index(dtv.side);
    unsigned int n = l->count;
    unsigned int sort = l->sort;
    DtEnt *e;
    int v;
    int w;

    e = DK_ENT(l, i);
    if (e == 0) {
        i = n - 1;
    } else if (up == 0) {
        v = dt_sort_value(e, sort);
        if (sort - 4 < 2 && v > 0)
            v /= 100;
        for (i++; (int)i < (int)n; i++) {
            w = dt_sort_value((int)i < (int)l->count ? &l->ents[l->order[i]] : 0, sort);
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
        v = dt_sort_value((int)(i - 1) < (int)n ? &l->ents[l->order[i - 1]] : 0, sort);
        if (sort - 4 < 2 && v > 0)
            v /= 100;
        for (i -= 2; (int)i >= 0; i--) {
            w = dt_sort_value((int)i < (int)l->count ? &l->ents[l->order[i]] : 0, sort);
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
void dt_scroll_update(void)
{
    PadBits *pad = &pad_get()->b;
    int moved;
    int d;

    if (dtv.mode < 4) {
        dt_scroll_analog(&dtv.cursor.scroll);
    } else if (dtv.scroll_max > 0) {
        moved = 0;
        if (pad->analog_x > 0x9E) {
            d = (pad->analog_x - 0x9E) * 4;
            if (dtv.scroll_max < dtv.scroll_pos) {
                dtv.scroll_pos += d;
            } else {
                dtv.scroll_pos += d;
                if (dtv.scroll_max < dtv.scroll_pos)
                    dtv.scroll_pos = dtv.scroll_max;
            }
            moved = 1;
        }
        if (pad->analog_x < 0x62) {
            dtv.scroll_pos += (pad->analog_x - 0x62) * 4;
            if (dtv.scroll_pos < 0)
                dtv.scroll_pos = 0;
            moved = 1;
        }
        if (!moved && dtv.cursor.timer > 0x35)
            dtv.scroll_pos += 0x40;
        if (dtv.scroll_pos >= dtv.scroll_max + 0x76C0)
            dtv.scroll_pos -= dtv.scroll_max + 0x76C0;
    }
}

/* the focus frame follows the cursor */
void dt_focus_update(void)
{
    unsigned int side = dtv.side;
    DtPane *p = &dtv.pane[side];
    short cy;

    if (p->list_mode == 0) {
        dts.focus.x = p->lay.x + p->lay.grid_x + p->cx * p->lay.cell_w;
        dts.focus.w = 0x17;
        dts.focus.y = p->lay.y + p->lay.grid_y + p->cy * p->lay.cell_h;
        dts.focus.h = 0x1F;
        return;
    }
    if (side == 1) {
        cy = dtv.pane[1].cy;
        if (dtv.mode < 4) {
            dts.focus.x = dtv.pane[1].lay.x + dtv.pane[1].lay.row_x;
            dts.focus.y = dtv.pane[1].lay.y + dtv.pane[1].lay.row_y + dtv.pane[1].cy * dtv.pane[1].lay.row_h;
            dts.focus.w = dtv.pane[1].lay.row_w - 1;
            dts.focus.h = dtv.pane[1].lay.row_h - 1;
            return;
        }
    } else {
        cy = p->cy;
    }
    dts.focus.x = p->lay.x + p->lay.row_x + 1;
    dts.focus.w = p->lay.row_w - 3;
    dts.focus.y = p->lay.y + p->lay.row_y + 1 + cy * p->lay.row_h;
    dts.focus.h = p->lay.row_h - 1;
}
#endif

/* 0 ok; 1 main deck under 40, 2 over 80, 3 extra deck over 15, 4 side deck over 30, 5 a card over
   its limit (*id) */
int dt_deck_check(unsigned short *id)
{
    int n = 0;
    int main = dts.list[0].total;
    int extra = dts.list[2].total;
    int side = dts.list[1].total;
    unsigned short tabs[3] = { 0, 2, 1 };
    int k;
    unsigned short *t;
    int std;
    DtList *l;
    int i;
    int lim;
    DtEnt *e;
    int copies;

    if (main < 40)
        return 1;
    if (main > 80)
        return 2;
    if (extra > 15)
        return 3;
    if (side > 30)
        return 4;
    if (dt_scene_arg == 2)
        return 0;
    std = sysflag_test(0) != 0;
    for (k = 0, t = tabs; k < 3; k++, t++) {
        l = &dts.list[*t];
        for (i = 0; i < l->n; i++) {
            e = &l->ents[i];
            lim = card_get_limit(e->id);
            if (lim < 3) {
                if (lim == 0) {
                    n++;
                    if (n > std) {
                        *id = e->id;
                        return 5;
                    }
                } else {
                    copies = (unsigned short)dt_list_count_id(&dts.list[1], e->id, 0);
                    copies += (unsigned short)dt_list_count_id(&dts.list[0], e->id, 0);
                    if (lim < copies + (unsigned short)dt_list_count_id(&dts.list[2], e->id, 0)) {
                        *id = e->id;
                        return 5;
                    }
                }
            }
        }
    }
    return 0;
}

/* deck error messages (popup 2) */
unsigned short dt_popup_msgs2[10] = { 0, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0 };
/* list full messages (popup 4) */
unsigned short dt_popup_msgs4[6] = { 0, 0x44, 0x6D, 0x6E, 0x6F, 0 };

/* opens popup id: 2 deck error arg, 3 quit confirmation, 4 list full arg, 5 card limit (card arg),
   6 search/filter result (arg = count), 7 deck operation confirmation */
void dt_popup_open(unsigned short id, unsigned short arg)
{
    MenuWidget *m = &dt_popup;
    MenuItemDef *d = &dt_popup.def;
    unsigned char buttons = 1;
    int lim;
    unsigned short *name;
    int alt;

    dt_popup_arg = arg;
    memset(m, 0, 0x120);
    memset(dt_popup_text, 0, 0x800);
    switch (id) {
    case 0:
    case 1:
        break;
    case 2:
        wcscpy(dt_popup_text, table_get_entry(dt_eres_get(0x8033), dt_popup_msgs2[arg]));
        m->cancel_blocked = 1;
        break;
    case 3:
        wcscpy(dt_popup_text, table_get_entry(dt_eres_get(0x8033), 0x57));
        m->initial = 4;
        buttons = 2;
        m->cancel_resets = 1;
        break;
    case 4:
        wcscpy(dt_popup_text, table_get_entry(dt_eres_get(0x8033), dt_popup_msgs4[arg]));
        m->cancel_blocked = 1;
        break;
    case 5:
        lim = card_get_limit(arg);
        alt = sysflag_test(0) != 0;
        name = card_get_name(arg);
        switch (lim) {
        case 0:
            if (alt > 0)
                wsprintf(dt_popup_text, table_get_entry(dt_eres_get(0x8033), 0x88), alt);
            else
                wsprintf(dt_popup_text, table_get_entry(dt_eres_get(0x8033), 0x85), name);
            break;
        case 1:
            wsprintf(dt_popup_text, table_get_entry(dt_eres_get(0x8033), 0x86), name);
            break;
        case 2:
            wsprintf(dt_popup_text, table_get_entry(dt_eres_get(0x8033), 0x87), name);
            break;
        }
        m->cancel_blocked = 1;
        break;
    case 6:
        if (arg <= 0)
            wcscpy(dt_popup_text, table_get_entry(dt_eres_get(0x8033), 0x89));
        else
            wsprintf(dt_popup_text, table_get_entry(dt_eres_get(0x8033), 0x81), arg);
        d->auto_size = 1;
        m->initial = 1;
        m->cancel_resets = 1;
        break;
    case 7:
        wsprintf(dt_popup_text, table_get_entry(dt_eres_get(0x8033), 0x8A), arg + 1,
                 table_get_entry(dt_eres_get(0x8033), arg + 0xA3));
        m->initial = 3;
        buttons = 3;
        m->cancel_resets = 1;
        break;
    }
    d->visible = 1;
    d->auto_size = 1;
    d->font_flag4 = 1;
    d->max_w = 0xF0;
    d->color = 0xFFFFFFFF;
    d->pad_l = 10;
    d->pad_r = 10;
    d->pad_t = 8;
    d->font_size = 0xC;
    d->text_color = 0xFF000000;
    d->text = dt_popup_text;
    d->open_anim = 2;
    d->close_anim = 2;
    m->heap = dts.heap;
    m->no_cancel = 1;
    m->buttons = buttons;
    m->pad = (PadState *)&dt.pad;
    d->style = 0;
    d->pad_b = 0;
    menu_open(m);
    d->x = (0x1E0 - d->w) / 2;
    d->y = (0x110 - d->h) / 2;
    dtv.popup = id;
}

/* the popup's input; 1 when Yes was picked on the quit confirmation (no help pages in the lesson) */
int dt_popup_update(void)
{
    int done = 0;
    int r = menu_update(&dt_popup);
    unsigned short id;

    if (r != -1) {
        id = dtv.popup;
        menu_reset(&dt_popup);
        dtv.popup = 0;
        switch (id) {
        case 3:
            if (r == 1)
                done = 1;
            break;
        case 7:
            if (r == 2)
                dt_flags.b7 = 1;
            else
                dt_check_rewards();
            break;
        }
    }
    return done;
}

void *dt_popup_draw(void *list)
{
    menu_draw(&list, &dt_popup);
    return list;
}

