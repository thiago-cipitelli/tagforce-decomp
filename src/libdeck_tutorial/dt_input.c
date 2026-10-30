// @range 0x088100AC-0x08811BB8
// @asm 0x088106B4-0x08810AA0
// @asm 0x088110CC-0x0881137C
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
void dt_cursor_grid(int side, PadBits *pad);
void dt_cursor_list(int side, PadBits *pad);
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

#ifdef NON_MATCHING /* 28 palavras: alocação dos temporários do cálculo inicial (base do painel em v1 no original, cx/cy/top em outra ordem) e o blezl do "if (top <= 0) top = 0" (o mwcc dá bgtz para o fim); mesma lógica de dt_cursor_list */
/* grid cursor: d-pad moves, wraps to the other pane at the left/right edges, scrolls by rows */
void dt_cursor_grid(int side, PadBits *pad)
{
    DtPane *p = &dtv.pane[(unsigned short)side];
    int cols = p->lay.cols;
    int top = (p->top + cols - 1) / cols;
    int cy = p->cy;
    int cx = p->cx;
    int rows = p->lay.grid_rows;
    unsigned int rep = pad->rep;
    int total = (p->list->count + cols - 1) / cols;
    int dx;
    int dy;
    int changed = 0;
    int nx;
    int ny;

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
        dt_cursor_switch(side, (unsigned short)rows, 1, (unsigned short)cy);
        se_play(0x6212);
    } else if (nx >= cols) {
        dt_cursor_switch(side, (unsigned short)rows, 0, (unsigned short)cy);
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
                top += ny;
                ny = 0;
                if (top <= 0)
                    top = 0;
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
#endif

#ifdef NON_MATCHING /* 3 palavras: "top += ny; if (top <= 0) top = 0; ny = 0;" sai no original como blezl para a instrução seguinte e "b; ny = 0"; aqui bgtz direto para o fim (ny = 0 antes do teste é a forma mais próxima) */
/* list cursor: d-pad up/down with wrap at the ends, left/right go to the other pane */
void dt_cursor_list(int side, PadBits *pad)
{
    DtPane *p = &dtv.pane[(unsigned short)side];
    int cy = p->cy;
    int rows = p->lay.rows;
    unsigned int rep = pad->rep;
    int top = p->top;
    int total = p->list->count;
    int changed = 0;
    int dy;
    int ny;

    if (rep & 0x80) {
        dt_cursor_switch(side, (unsigned short)rows, 1, (unsigned short)cy);
        se_play(0x6212);
    } else if (rep & 0x20) {
        dt_cursor_switch(side, (unsigned short)rows, 0, (unsigned short)cy);
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
                top += ny;
                ny = 0;
                if (top <= 0)
                    top = 0;
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
#endif

/* moves the cursor to the other pane (dir 1 = enter from the right), keeping its height; the callers
   pass 4 arguments, so to is whatever the fifth register holds (used only when from is neither 0 nor 1,
   which the callers never pass) */
void dt_cursor_switch(from, rows, dir, cy, to)
unsigned short from;
unsigned short rows;
unsigned short dir;
unsigned short cy;
unsigned short to;
{
    DtPane *p;
    unsigned short cx = 0;
    unsigned short n;

    switch (from) {
    case 0:
        if (dtv.mode > 1)
            to = 1;
        else
            to = 0;
        break;
    case 1:
        to = 0;
        break;
    }
    p = &dtv.pane[to];
    if (p->list_mode == 0) {
        if (dir == 1)
            cx = p->lay.cols - 1;
        n = p->lay.grid_rows;
    } else {
        n = p->lay.rows;
    }
    if (n != rows) {
        cy = (int)(((unsigned int)rows >> 1) + cy * n) / (int)rows;
        if (cy >= n)
            cy = n - 1;
    }
    dtv.side = to;
    p->cx = cx;
    p->cy = cy;
}

/* L/R: a page up or down (the first row shown is rounded down to a row) */
void dt_page(int up)
{
    DtPane *p = &dtv.pane[dtv.side];
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
void dt_jump(int up)
{
    DtPane *p = &dtv.pane[dtv.side];
    int total = p->list->count;
    int i;
    int cols;
    int row;
    int last;

    if (total == 0)
        return;
    i = dt_find_group(up);
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
int dt_find_group(int up)
{
    int k;
    DtList *l = dtv.pane[dtv.side].list;
    int i = (unsigned short)dt_cur_index(dtv.side);
    unsigned short sort = l->sort;
    int n = l->count;
    DtEnt *e = i >= n ? 0 : &l->ents[l->order[i]];
    int v;
    int w;
    int j;

    if (e == 0)
        return n - 1;
    if (up) {
        if (i <= 0)
            return -1;
        j = i - 1;
        v = dt_sort_value(j >= n ? 0 : &l->ents[l->order[j]], sort);
        if ((sort == 4 || sort == 5) && v > 0)
            v /= 100;
        for (k = j - 1; k >= 0; k--) {
            w = dt_sort_value(DK_ENT(l, k), sort);
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
        v = dt_sort_value(e, sort);
        if ((sort == 4 || sort == 5) && v > 0)
            v /= 100;
        for (k = i + 1; k < n; k++) {
            w = dt_sort_value(DK_ENT(l, k), sort);
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

#ifdef NON_MATCHING /* 59 palavras: o original guarda max e max + 0x76C0 em registradores */
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
#endif

#ifdef NON_MATCHING /* 56 palavras: a ordem das cargas no ramo da grade (x, grid_x, cx... no original) e a alocação; tentadas a ordem dos stores, a multiplicação comutada e o ponteiro do layout por dtv */
/* the focus frame follows the cursor */
void dt_focus_update(void)
{
    unsigned char side = dtv.side;
    DtPane *p = &dtv.pane[side];
    DtLayout *lay = &p->lay;
    DtRect *f = &dts.focus;

    if (p->list_mode == 0) {
        f->x = lay->x + lay->grid_x + p->cx * (unsigned short)lay->cell_w;
        f->w = 0x17;
        f->y = lay->y + lay->grid_y + p->cy * (unsigned short)lay->cell_h;
        f->h = 0x1F;
    } else if (side == 1 && dtv.mode < 4) {
        f->x = lay->x + lay->row_x;
        f->w = (unsigned short)lay->row_w - 1;
        f->y = lay->y + lay->row_y + p->cy * (unsigned short)lay->row_h;
        f->h = (unsigned short)lay->row_h - 1;
    } else {
        f->x = lay->x + lay->row_x + 1;
        f->w = (unsigned short)lay->row_w - 3;
        f->y = lay->y + lay->row_y + 1 + p->cy * (unsigned short)lay->row_h;
        f->h = (unsigned short)lay->row_h - 1;
    }
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

