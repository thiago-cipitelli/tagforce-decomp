// @range 0x0880AD2C-0x0880C940
// @asm 0x0880AD2C-0x0880B120
// @asm 0x0880B120-0x0880B1F8
// @asm 0x0880B35C-0x0880B574
// @asm 0x0880B754-0x0880BA10
// @asm 0x0880BC70-0x0880BE98
// @asm 0x0880BE98-0x0880C05C
// @asm 0x0880C380-0x0880C690
// @asm 0x0880C690-0x0880C7D4
// @asm 0x0880C7D4-0x0880C940
// @rodata 0x0881A960-0x0881A984
// @data 0x0881AE58-0x0881B378
/* Pane layouts (one or two panes, grid or list each) and the tab positions per kind and language;
   the per-frame input dispatch and the input of the main state: cursor moves in the grid and in
   the list, paging, jumps between sort groups, moving cards between the panes; focus frame. */
#include "libselect_card.h"

typedef struct ScMode {                /* sc_modes: pane layouts of a mode */
    short list_a;                      /* pane 0 in list mode */
    short lay_a;                       /* sc_layouts row of pane 0 */
    short list_b;                      /* pane 1, -1 = none */
    short lay_b;
} ScMode;

typedef struct ScTab {                 /* a tab button, 10 bytes */
    unsigned short x : 9;              /* +0x00 */
    unsigned short f00 : 7;
    unsigned short w : 9;              /* +0x02 */
    unsigned short f02 : 7;
    unsigned short lx : 9;             /* +0x04 frame of the count */
    unsigned short f04 : 7;
    unsigned short lw : 9;             /* +0x06 */
    unsigned short digits : 5;         /*       digits of the count */
    unsigned short f06 : 2;
    unsigned short pad08;
} ScTab;

void sc_set_panes(unsigned short which);
void sc_select_tab(unsigned short side, unsigned short tab, int how);
ScEnt *sc_cur_entry(unsigned short side);
void sc_info_update(void);
void sc_layout_scroll(int grid, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
unsigned short sc_cur_index(unsigned short side);
int sc_sort_value(ScEnt *e, unsigned int sort);
void sc_input_sort_menu(void);
void sc_input_action_menu(void);
void sc_input_search(void);
void sc_input_confirm(void);
void sc_input_result(void);
void sc_popup_update(void);
void sc_action_open(ScEnt *e);
void sc_popup_open(int id, int arg);
int sc_cancel_request(void);
void sc_input_sort(void);
void sc_input_filter(void);
void sc_input_move(void);
void sc_input_main(void);
void sc_input_help(void);
void sc_input_help_b(void);
void sc_cursor_grid(unsigned short side, PadState *pad);
void sc_cursor_list(unsigned short side, PadState *pad);
void sc_cursor_switch();
void sc_page(int up);
void sc_jump(int up);
int sc_find_group(int up);
void sc_scroll_update(void);
void sc_focus_update(void);
void sc_window_anim(void);
int sc_mode_for(unsigned short panes, int narrow, unsigned short mode_a, unsigned short mode_b);

ScLayout sc_layouts[5] = {
    { 148, 38, 331, 233, 3, 3, 7, 318, 30, 8, 8, 11, 28, 6, 34 },
    { 148, 38, 215, 233, 3, 3, 6, 200, 30, 8, 8, 7, 28, 6, 34 },
    { 364, 38, 115, 233, 4, 7, 6, 99, 34, 9, 8, 3, 32, 6, 34 },
    { 0, 38, 239, 203, 3, 3, 6, 226, 30, 7, 10, 8, 28, 5, 34 },
    { 240, 38, 239, 203, 3, 3, 6, 226, 30, 7, 10, 8, 28, 5, 34 },
};

ScMode sc_modes[8] = {
    { 1, 0, -1, -1 },
    { 0, 0, -1, -1 },
    { 0, 1, 0, 2 },
    { 0, 1, 1, 2 },
    { 1, 3, 1, 4 },
    { 0, 3, 0, 4 },
    { 1, 3, 0, 4 },
    { 0, 3, 1, 4 },
};

/* tab buttons per kind and language: one pane, two panes, two wide panes; x, w and the label
   width packed in 9-bit fields */
ScTab sc_tabs_rental[6][6] __attribute__((aligned(16))) = {
#define T0 { { 0x0, 0, 0x4D, 0, 0x26, 0, 0x23, 4, 0, 0x0 }, { 0x4D, 0, 0x3C, 0, 0x7A, 0, 0xB, 1, 0, 0x0 }, { 0x0, 0, 0x4D, 0, 0x26, 0, 0x23, 4, 0, 0x0 }, { 0x0, 0, 0x3C, 0, 0x2D, 0, 0xB, 1, 0, 0x0 }, { 0x0, 0, 0x4D, 0, 0x26, 0, 0x23, 4, 0, 0x0 }, { 0x0, 0, 0x3C, 0, 0x2D, 0, 0xB, 1, 0, 0x0 } }
    T0, T0, T0, T0, T0, T0,
#undef T0
};

ScTab sc_tabs_conv[6][6] __attribute__((aligned(16))) = {
    { { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x5B, 0, 0x3D, 0, 0x81, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3D, 0, 0x26, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3C, 0, 0x26, 0, 0x13, 2, 0, 0x0 } },
    { { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x5B, 0, 0x3D, 0, 0x9D, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x59, 0, 0x42, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3C, 0, 0x42, 0, 0x13, 2, 0, 0x0 } },
    { { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x5B, 0, 0x3D, 0, 0x81, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3D, 0, 0x26, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3C, 0, 0x26, 0, 0x13, 2, 0, 0x0 } },
    { { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x5B, 0, 0x3D, 0, 0x81, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3D, 0, 0x26, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3C, 0, 0x26, 0, 0x13, 2, 0, 0x0 } },
    { { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x5B, 0, 0x3D, 0, 0x81, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3D, 0, 0x26, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3C, 0, 0x26, 0, 0x13, 2, 0, 0x0 } },
    { { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x5B, 0, 0x3D, 0, 0x81, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3D, 0, 0x26, 0, 0x13, 2, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3C, 0, 0x26, 0, 0x13, 2, 0, 0x0 } },
};

ScTab sc_tabs_trade[6][6] __attribute__((aligned(16))) = {
#define T2 { { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x5B, 0, 0x3C, 0, 0x88, 0, 0xB, 1, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3C, 0, 0x2D, 0, 0xB, 1, 0, 0x0 }, { 0x0, 0, 0x5B, 0, 0x26, 0, 0x31, 6, 0, 0x0 }, { 0x0, 0, 0x3C, 0, 0x2D, 0, 0xB, 1, 0, 0x0 } }
    T2, T2, T2, T2, T2, T2,
#undef T2
};

#ifdef NON_MATCHING /* o original usa a base scv mais o lado para os campos do painel e a alocação dos s difere */
/* switches the pane layout (sc_modes row), keeping each list's scroll position */
void sc_set_mode(unsigned short mode)
{
    ScMode *m = &sc_modes[mode];
    unsigned short tab_b = scv.pane[1].list->tab;
    unsigned short tab_a = scv.pane[0].list->tab;
    int i;
    ScPane *p;
    ScList *l;
    unsigned short grid;
    unsigned short view;

    if (scv.mode < 2)
        sc_set_panes(0);
    else
        sc_set_panes(2);
    scv.pane[0].list_mode = m->list_a;
    memcpy(&scv.pane[0].lay, &sc_layouts[m->lay_a], 0x1E);
    if (m->list_b >= 0)
        scv.pane[1].list_mode = m->list_b;
    if (m->lay_b >= 0)
        memcpy(&scv.pane[1].lay, &sc_layouts[m->lay_b], 0x1E);
    scv.mode = mode;
    for (i = 0; i < 2; i++) {
        p = &scv.pane[i];
        p->top = p->list->u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        grid = p->list_mode == 0 ? p->lay.cols : 1;
        l = p->list;
        view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        sc_layout_scroll(l->u1d_lo == grid, l->u1a, ((p->list_mode == 0 ? p->lay.cols : 1) & 0xFF) | (view & 0xFF) << 8,
                         l->count, &p->top, &p->cx, &p->cy);
        if (scv.mode < 2)
            break;
    }
    if (scv.mode < 2) {
        if (scv.side != 0) {
            scv.side = 0;
            sc_select_tab(0, tab_b, 2);
        }
    } else if (tab_a != 0) {
        sc_select_tab(1, tab_a - 1, 2);
        scv.side = 1;
        sc_select_tab(0, 0, 2);
    }
    for (i = 0; i < 2; i++) {
        p = &scv.pane[i];
        p->top = p->list->u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        grid = p->list_mode == 0 ? p->lay.cols : 1;
        l = p->list;
        view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        sc_layout_scroll(l->u1d_lo == grid, l->u1a, ((p->list_mode == 0 ? p->lay.cols : 1) & 0xFF) | (view & 0xFF) << 8,
                         l->count, &p->top, &p->cx, &p->cy);
        if (scv.mode < 2)
            break;
    }
    sc_window_anim();
    if (sc_cur_entry(scv.side) != 0)
        sc_info_update();
}
#endif

#ifdef NON_MATCHING /* o original não encadeia os saltos para o fim (cada grupo termina num b para o epílogo) */
/* sc_modes row for a pane count, narrow second pane and the list mode of each pane; -1 none */
int sc_mode_for(unsigned short panes, int narrow, unsigned short a, unsigned short b)
{
    int r = 1;

    if (panes == 1) {
        if (a == 1)
            r = 0;
        else if (a == 0)
            r = 1;
        else
            r = -1;
    } else if (narrow != 0) {
        if (a == 0) {
            if (b == 1)
                r = 3;
            else if (b == 0)
                r = 2;
            else
                r = -1;
        } else {
            r = -1;
        }
    } else if (a == 0) {
        if (b == 1)
            r = 7;
        else if (b == 0)
            r = 5;
        else
            r = -1;
    } else if (a == 1) {
        if (b == 1)
            r = 4;
        else if (b == 0)
            r = 6;
        else
            r = -1;
    } else {
        r = -1;
    }
    return r;
}
#endif

/* x and width of tab button i */
unsigned int sc_tab_pos(unsigned short i)
{
    ScTab *t;
    int n;
    ScTab *e;

    switch (sc.kind) {
    case 0:
        t = sc_tabs_rental[cfg_get_language()];
        n = 2;
        break;
    case 1:
        t = sc_tabs_conv[cfg_get_language()];
        n = 2;
        break;
    case 2:
        t = sc_tabs_trade[cfg_get_language()];
        n = 2;
        break;
    default:
        return 0;
    }
    if (scv.mode < 2)
        e = &t[i];
    else if (scv.mode < 4)
        e = &(t + n)[i];
    else
        e = &(t + n * 2)[i];
    return e->x | e->w << 16;
}

#ifdef NON_MATCHING /* o original lê a largura antes das gravações de y/h; o escalonador atual as intercala */
/* frame of tab button i (the selected one taller) and the x of its count */
unsigned short sc_tab_rect(unsigned short i, unsigned short sel, ScRect *r)
{
    ScTab *t;
    int n;
    ScTab *e;
    short x;
    unsigned short ux;
    unsigned short end;

    switch (sc.kind) {
    case 0:
        t = sc_tabs_rental[cfg_get_language()];
        n = 2;
        break;
    case 1:
        t = sc_tabs_conv[cfg_get_language()];
        n = 2;
        break;
    case 2:
        t = sc_tabs_trade[cfg_get_language()];
        n = 2;
        break;
    default:
        return 0;
    }
    if (scv.mode < 2)
        e = &t[i];
    else if (scv.mode < 4)
        e = &(t + n)[i];
    else
        e = &(t + n * 2)[i];
    r->x = e->lx;
    r->w = e->lw;
    if (i == sel) {
        r->y = 0x19;
        r->h = 0xD;
    } else {
        x = r->x;
        r->y = 0x1B;
        ux = x;
        end = ux + r->w;
        r->h = 0xB;
        r->x = x;
        if (end <= ux)
            r->w = 0;
        else
            r->w = end - ux;
    }
    return e->lx + e->lw - ((unsigned int)(unsigned short)(e->lw - e->digits * 8) >> 1) - 1;
}
#endif

/* window animation of the pane with tab i */
static int sc_tab_anim(int tab)
{
    int r = 0;

    if (scv.mode < 2)
        r += tab;
    else if (scv.mode >= 4)
        r += 3;
    else
        r += 2;
    return r;
}

void sc_window_anim(void)
{
    void *e = 0;
    int a = sc_tab_anim(scv.pane[0].list->tab);

    switch (sc.kind) {
    case 0:
        e = res_get(0x1010);
        break;
    case 1:
        e = res_get(0x2010);
        break;
    case 2:
        e = res_get(0x4010);
        break;
    }
    if (e != 0)
        ehf_set_anim(e, a, a + 1);
}

/* per frame, after the animations: input of the current state, unless a popup or the busy flag */
void sc_update_input(void)
{
    pad_get();
    if (!scv.busy && !sc.info_open) {
        sc_scroll_update();
        if (scv.popup != 0) {
            sc_popup_update();
        } else {
            switch (sc.state) {
            case 0:
                break;
            case 1:
            case 7:
            case 8:
                sc_input_main();
                break;
            case 2:
                sc_input_sort_menu();
                break;
            case 3:
                sc_input_action_menu();
                break;
            case 4:
                sc_input_search();
                break;
            case 5:
                sc_input_confirm();
                break;
            case 6:
                sc_input_result();
                break;
            }
            sc_focus_update();
        }
    }
}

#ifdef NON_MATCHING /* o lado é lido para um u8 local e o painel por base; desvios e alocação diferentes */
/* state 1: cursor, Square/Triangle hold the help windows, Circle/Cross/Start/Select actions */
void sc_input_main(void)
{
    PadState *pad = pad_get();
    unsigned int trig;
    short tab;
    unsigned int side;
    int mode;
    unsigned char a;
    unsigned char b;

    if ((pad->b.rep & 0xF0) != 0) {
        if (scv.pane[scv.side].list_mode == 0)
            sc_cursor_grid(scv.side, pad);
        else
            sc_cursor_list(scv.side, pad);
        return;
    }
    if (pad->b.down & 0x100) {
        sc.state = 7;
        sc_input_help();
        return;
    }
    if (pad->b.down & 0x200) {
        sc.state = 8;
        sc_input_help_b();
        return;
    }
    sc.state = 1;
    trig = pad->b.trig;
    if (trig & region_pick_value(1)) {
        sc_popup_open(3, 0);
        se_play(0x621A);
    } else if (trig & region_pick_value(0)) {
        if (sc_cancel_request())
            se_play(0x6213);
    } else if (trig & 0x1000) {
        sc_action_open(sc_cur_entry(scv.side));
        se_play(0x6216);
    } else if (trig & 0x8000) {
        side = scv.side;
        tab = scv.pane[side].list->tab;
        sc_select_tab(side, tab, 1);
        if (tab != scv.pane[scv.side].list->tab)
            se_play(0x621B);
    } else if (trig & 1) {
        a = scv.pane[0].list_mode;
        b = scv.pane[1].list_mode;
        if (scv.side == 0)
            a = (a + 1) & 1;
        else
            b = (b + 1) & 1;
        mode = sc_mode_for(scv.mode < 2 ? 1 : 2, scv.mode < 4, a, b);
        if (mode >= 0) {
            sc_set_mode(mode);
            se_play(0x6213);
        }
    }
}
#endif

/* state 7 (Square held): sort order */
void sc_input_help(void)
{
    PadState *pad = pad_get();
    unsigned int rep = pad->b.rep;
    ScList *l;

    if (rep & 0x8000) {
        sc_page(1);
        se_play(0x6212);
    } else if (rep & 0x2000) {
        sc_page(0);
        se_play(0x6212);
    } else if (rep & 0x1000) {
        sc_jump(1);
        se_play(0x6212);
    } else if (rep & 0x4000) {
        sc_jump(0);
        se_play(0x6212);
    } else if (pad->b.trig & 1) {
        l = scv.pane[scv.side].list;
        if (l->filter != 0) {
            sc_list_filter_reset(l);
            sc_list_sort(l, l->sort, l->dir);
            se_play(0x6213);
        }
    }
}

/* state 8 (Triangle held): marks and the sort/filter menus */
void sc_input_help_b(void)
{
    PadState *pad = pad_get();
    unsigned int trig = pad->b.trig;
    ScEnt *e;
    unsigned short id;

    if (trig & region_pick_value(0)) {
        if (sc_cancel_request())
            se_play(0x6213);
    } else if (trig & region_pick_value(1)) {
        e = sc_cur_entry(scv.side);
        if (e != 0) {
            id = e->id;
            if (inv_get_flag7(id) != 0)
                inv_set_flag7(id, 0);
            else
                inv_set_flag7(id, 1);
            se_play(0x6213);
        }
    } else if (trig & 0x8000) {
        sc_input_sort();
        se_play(0x6216);
    } else if (trig & 0x1000) {
        sc_input_filter();
        se_play(0x6216);
    } else if ((trig & 1) && sc.nhist > 0) {
        sc_input_move();
        se_play(0x6213);
    }
}

#ifdef NON_MATCHING /* campos do painel por índice no original; registradores e ordem dos testes diferem */
/* grid cursor: d-pad moves, wraps to the other pane at the left/right edges, scrolls by rows */
void sc_cursor_grid(unsigned short side, PadState *pad)
{
    ScPane *p = &scv.pane[side];
    int cols = p->lay.cols;
    int top = (p->top + cols - 1) / cols;
    int cx = p->cx;
    int cy = p->cy;
    int rows = p->lay.grid_rows;
    unsigned int rep = pad->b.rep;
    int total = (p->list->count + cols - 1) / cols;
    int changed = 0;
    int dy = 0;
    int dx = 0;
    int ny;

    if (rep & 0x40)
        dy++;
    if (rep & 0x10)
        dy--;
    if (rep & 0x20)
        dx++;
    if (rep & 0x80)
        dx--;
    dx = cx + dx;
    if (dx < 0) {
        sc_cursor_switch(side, (unsigned short)rows, 1, (unsigned short)cy);
        se_play(0x6212);
        return;
    }
    if (dx >= cols) {
        sc_cursor_switch(side, (unsigned short)rows, 0, (unsigned short)cy);
        se_play(0x6212);
        return;
    }
    if (top + cy == 0 && (pad->b.trig & 0x10)) {
        if (total < rows)
            top = 0;
        else
            top = total - rows;
        ny = rows - 1;
    } else if (top + cy >= total - 1 && cy == rows - 1 && (pad->b.trig & 0x40)) {
        ny = 0;
        top = 0;
    } else {
        ny = cy + dy;
        if (ny < 0) {
            top += ny;
            if (top <= 0)
                top = 0;
            ny = 0;
        } else if (ny >= rows) {
            if (rows < total) {
                if (top + ny < total)
                    top += ny - (rows - 1);
                else
                    top = total - rows;
            }
            ny = rows - 1;
        }
    }
    if (cx != dx) {
        p->cx = dx;
        changed = 1;
    }
    if (p->cy != ny) {
        p->cy = ny;
        changed = 1;
    }
    top *= cols;
    if (p->top != top) {
        p->top = top;
        changed = 1;
    }
    if (changed)
        se_play(0x6212);
}
#endif

#ifdef NON_MATCHING /* lógica conferida com o asm; 78 palavras: o original lê cy/rows/top pela base scv antes
                       de formar o ponteiro do painel; registradores diferentes */
/* list cursor: d-pad up/down with wrap at the ends, left/right go to the other pane */
void sc_cursor_list(unsigned short side, PadState *pad)
{
    ScPane *p = &scv.pane[side];
    int cy = p->cy;
    int rows = p->lay.rows;
    unsigned int rep = pad->b.rep;
    int top = p->top;
    int total = p->list->count;
    int changed = 0;
    int d;
    int ny;

    if (rep & 0x80) {
        sc_cursor_switch(side, (unsigned short)rows, 1, (unsigned short)cy);
        se_play(0x6212);
        return;
    }
    if (rep & 0x20) {
        sc_cursor_switch(side, (unsigned short)rows, 0, (unsigned short)cy);
        se_play(0x6212);
        return;
    }
    d = 0;
    if (rep & 0x40)
        d++;
    if (rep & 0x10)
        d--;
    if (top + cy == 0 && (pad->b.trig & 0x10)) {
        if (total < rows)
            top = 0;
        else
            top = total - rows;
        ny = rows - 1;
    } else if (top + cy >= total - 1 && cy == rows - 1 && (pad->b.trig & 0x40)) {
        ny = 0;
        top = 0;
    } else {
        ny = cy + d;
        if (ny < 0) {
            top += ny;
            if (top <= 0)
                top = 0;
            ny = 0;
        } else if (ny >= rows) {
            if (total < rows)
                top = 0;
            else if (top + ny < total)
                top += ny - (rows - 1);
            else
                top = total - rows;
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
#endif

/* moves the cursor to the other pane (dir 1 = enter from the right), keeping its height; K&R because
   some callers pass only 4 arguments (to is then whatever is in t0) */
void sc_cursor_switch(from, rows, dir, cy, to)
unsigned short from;
unsigned short rows;
unsigned short dir;
unsigned short cy;
unsigned short to;
{
    unsigned short cx = 0;
    ScPane *p;
    unsigned short n;

    switch (from) {
    case 0:
        if (scv.mode > 1)
            to = 1;
        else
            to = 0;
        break;
    case 1:
        to = 0;
        break;
    }
    p = &scv.pane[to];
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
    scv.side = to;
    p->cx = cx;
    p->cy = cy;
}

/* L/R: a page up or down */
void sc_page(int up)
{
    ScPane *p = &scv.pane[scv.side];
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
        return;
    }
    if (up != 0) {
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

/* jumps to the next/previous group of the sort key */
void sc_jump(int up)
{
    ScPane *p = &scv.pane[scv.side];
    int total = p->list->count;
    int i;
    int cols;
    int last;
    int row;

    if (total == 0)
        return;
    i = sc_find_group(up);
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

#ifdef NON_MATCHING /* lógica conferida com o asm (o rascunho chamava sc_cur_index sem o lado); 114 palavras:
                       up e i trocam de registrador salvo e o teste da chave sai fora do laço */
/* sort value of a card, ATK/DEF grouped by hundreds */
static inline int sc_group_value(ScEnt *e, unsigned short sort)
{
    int v = sc_sort_value(e, sort);

    if ((unsigned int)(sort - 4) < 2 && v > 0)
        v /= 100;
    return v;
}

/* index of the first card of the next (up = 0) or previous sort group from the cursor, -1 none */
int sc_find_group(int up)
{
    unsigned char side = scv.side;
    ScList *l = scv.pane[side].list;
    unsigned short i = sc_cur_index(side);
    int n = l->count;
    int sort = l->sort;
    ScEnt *e;
    int v;
    int k;

    e = SC_ENT(l, i);
    if (e == 0)
        return n - 1;
    if (up != 0) {
        if (i <= 0)
            return -1;
        k = i - 1;
        v = sc_group_value(SC_ENT(l, k), sort);
        for (k--; k >= 0; k--) {
            if (sc_group_value(SC_ENT(l, k), sort) != v) {
                k++;
                break;
            }
        }
        if (k >= 0)
            return k;
        k = 0;
    } else {
        v = sc_group_value(e, sort);
        for (k = i + 1; k < n; k++) {
            if (sc_group_value(SC_ENT(l, k), sort) != v)
                break;
        }
        if (!(k < n))
            return -1;
    }
    return k;
}
#endif

#ifdef NON_MATCHING /* o original guarda max e max + 0x76C0 em registradores */
/* analog scrolling of the card info text (one pane) or of the wide view */
void sc_scroll_update(void)
{
    PadState *pad = pad_get();
    int moved;
    int d;

    if (scv.mode < 4) {
        sc_scroll_analog(&scv.cursor.scroll);
    } else if (scv.scroll_max > 0) {
        moved = 0;
        if (pad->b.analog_x > 0x9E) {
            d = (pad->b.analog_x - 0x9E) * 4;
            if (scv.scroll_max < scv.scroll_pos) {
                scv.scroll_pos += d;
            } else {
                scv.scroll_pos += d;
                if (scv.scroll_max < scv.scroll_pos)
                    scv.scroll_pos = scv.scroll_max;
            }
            moved = 1;
        }
        if (pad->b.analog_x < 0x62) {
            scv.scroll_pos += (pad->b.analog_x - 0x62) * 4;
            if (scv.scroll_pos < 0)
                scv.scroll_pos = 0;
            moved = 1;
        }
        if (!moved && scv.cursor.timer > 0x35)
            scv.scroll_pos += 0x40;
        if (scv.scroll_pos >= scv.scroll_max + 0x76C0)
            scv.scroll_pos -= scv.scroll_max + 0x76C0;
    }
}
#endif

#ifdef NON_MATCHING /* lógica conferida com o asm; 65 palavras: o original faz todas as leituras do
                       layout antes das gravações em sc.focus; o mwcc intercala */
/* the focus frame follows the cursor */
void sc_focus_update(void)
{
    unsigned char side = scv.side;
    ScPane *p = &scv.pane[side];
    ScRect *f = &sc.focus;
    ScLayout *ly = &p->lay;

    if (p->list_mode == 0) {
        f->w = 0x17;
        f->x = ly->x + ly->grid_x + p->cx * (unsigned short)ly->cell_w;
        f->y = ly->y + ly->grid_y + p->cy * (unsigned short)ly->cell_h;
        f->h = 0x1F;
    } else if (side == 1 && scv.mode < 4) {
        f->x = ly->x + ly->row_x;
        f->w = (unsigned short)ly->row_w - 1;
        f->y = ly->y + ly->row_y + p->cy * (unsigned short)ly->row_h;
        f->h = (unsigned short)ly->row_h - 1;
    } else {
        f->x = ly->x + ly->row_x + 1;
        f->w = (unsigned short)ly->row_w - 3;
        f->y = ly->y + ly->row_y + 1 + p->cy * (unsigned short)ly->row_h;
        f->h = (unsigned short)ly->row_h - 1;
    }
}
#endif
