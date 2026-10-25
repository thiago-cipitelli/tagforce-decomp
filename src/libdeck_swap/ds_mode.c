// @range 0x08805E58-0x08806654
// @asm 0x08805E58-0x08806324
// @asm 0x088064F8-0x08806534
// @asm 0x0880658C-0x08806654
// @data 0x0881A014-0x0881A170
/* Pane layouts (one or two panes, grid or list each) and the tab positions of the three lists;
   the window and stream animations that follow the layout and the deck size (same source as
   libdeck_swap ds_input.c). */
#include "libdeck_swap.h"

typedef struct DsMode {                /* ds_modes: pane layouts of a mode */
    short list_a;                      /* pane 0 in list mode */
    short lay_a;                       /* ds_layouts row of pane 0 */
    short list_b;                      /* pane 1, -1 = none */
    short lay_b;
} DsMode;

typedef struct DsTab {                 /* a tab button, 10 bytes */
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
} DsTab;

void ds_set_panes(unsigned short which);
void ds_select_tab(unsigned short side, unsigned short tab, int how);
DsEnt *ds_cur_entry(unsigned short side);
void ds_info_update(void);
void ds_layout_scroll(int grid, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
void ds_window_anim(void);
void ds_stream_anim(void);

DsLayout ds_layouts[5] = {
    { 148, 38, 331, 233, 3, 3, 7, 318, 30, 8, 8, 11, 28, 6, 34 },
    { 148, 38, 215, 233, 3, 3, 6, 200, 30, 8, 8, 7, 28, 6, 34 },
    { 364, 38, 115, 233, 4, 7, 6, 99, 34, 9, 8, 3, 32, 6, 34 },
    { 0, 38, 239, 203, 3, 3, 6, 226, 30, 7, 10, 8, 28, 5, 34 },
    { 240, 38, 239, 203, 3, 3, 6, 226, 30, 7, 10, 8, 28, 5, 34 },
};

DsMode ds_modes[8] = {
    { 1, 0, -1, -1 },
    { 0, 0, -1, -1 },
    { 0, 1, 0, 2 },
    { 0, 1, 1, 2 },
    { 1, 3, 1, 4 },
    { 0, 3, 0, 4 },
    { 1, 3, 0, 4 },
    { 0, 3, 1, 4 },
};

#ifdef NON_MATCHING /* o original usa a base dsv mais o lado para os campos do painel e a alocação dos s difere */
/* switches the pane layout (ds_modes row), keeping each list's scroll position */
void ds_set_mode(unsigned short mode)
{
    DsMode *m = &ds_modes[mode];
    unsigned short tab_b = dsv.pane[1].list->tab;
    unsigned short tab_a = dsv.pane[0].list->tab;
    int i;
    DsPane *p;
    DsList *l;
    unsigned short grid;
    unsigned short view;

    if (dsv.mode < 2)
        ds_set_panes(0);
    else
        ds_set_panes(2);
    dsv.pane[0].list_mode = m->list_a;
    memcpy(&dsv.pane[0].lay, &ds_layouts[m->lay_a], 0x1E);
    if (m->list_b >= 0)
        dsv.pane[1].list_mode = m->list_b;
    if (m->lay_b >= 0)
        memcpy(&dsv.pane[1].lay, &ds_layouts[m->lay_b], 0x1E);
    dsv.mode = mode;
    for (i = 0; i < 2; i++) {
        p = &dsv.pane[i];
        p->top = p->list->u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        grid = p->list_mode == 0 ? p->lay.cols : 1;
        l = p->list;
        view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        ds_layout_scroll(l->u1d_lo == grid, l->u1a, ((p->list_mode == 0 ? p->lay.cols : 1) & 0xFF) | (view & 0xFF) << 8,
                         l->count, &p->top, &p->cx, &p->cy);
        if (dsv.mode < 2)
            break;
    }
    if (dsv.mode < 2) {
        if (dsv.side != 0) {
            dsv.side = 0;
            ds_select_tab(0, tab_b, 2);
        }
    } else if (tab_a != 0) {
        ds_select_tab(1, tab_a - 1, 2);
        dsv.side = 1;
        ds_select_tab(0, 0, 2);
    }
    for (i = 0; i < 2; i++) {
        p = &dsv.pane[i];
        p->top = p->list->u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        grid = p->list_mode == 0 ? p->lay.cols : 1;
        l = p->list;
        view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        ds_layout_scroll(l->u1d_lo == grid, l->u1a, ((p->list_mode == 0 ? p->lay.cols : 1) & 0xFF) | (view & 0xFF) << 8,
                         l->count, &p->top, &p->cx, &p->cy);
        if (dsv.mode < 2)
            break;
    }
    ds_window_anim();
    if (ds_cur_entry(dsv.side) != 0)
        ds_info_update();
}
#endif

#ifdef NON_MATCHING /* o original não encadeia os saltos para o fim (cada grupo termina num b para o epílogo) */
/* ds_modes row for a pane count, narrow second pane and the list mode of each pane; -1 none */
int ds_mode_for(unsigned short panes, int narrow, unsigned short a, unsigned short b)
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

/* tab buttons of the three layouts (one pane, two panes, two wide panes) */
DsTab ds_tabs[3][3] __attribute__((aligned(16))) = {
    { { 0, 0, 60, 0, 37, 0, 19, 2, 0, 0 }, { 60, 0, 60, 0, 97, 0, 19, 2, 0, 0 }, { 120, 0, 60, 0, 157, 0, 19, 2, 0, 0 } },
    { { 0, 0, 60, 0, 37, 0, 19, 2, 0, 0 }, { 60, 0, 60, 0, 97, 0, 19, 2, 0, 0 }, { 0, 0, 60, 0, 37, 0, 19, 2, 0, 0 } },
    { { 0, 0, 60, 0, 37, 0, 19, 2, 0, 0 }, { 60, 0, 60, 0, 97, 0, 19, 2, 0, 0 }, { 0, 0, 60, 0, 37, 0, 19, 2, 0, 0 } },
};

/* x and width of tab button i */
unsigned int ds_tab_pos(unsigned short i)
{
    DsTab *e;

    if (dsv.mode < 2)
        e = &ds_tabs[0][i];
    else if (dsv.mode < 4)
        e = &ds_tabs[1][i];
    else
        e = &ds_tabs[2][i];
    return e->x | e->w << 16;
}

/* frame of tab button i (the selected one taller) and the x of its count */
unsigned short ds_tab_rect(unsigned short i, unsigned short sel, DsRect *r)
{
    DsTab *e;
    unsigned short x;
    unsigned short end;

    if (dsv.mode < 2)
        e = &ds_tabs[0][i];
    else if (dsv.mode < 4)
        e = &ds_tabs[1][i];
    else
        e = &ds_tabs[2][i];
    r->x = e->lx;
    r->w = e->lw;
    if (i == sel) {
        r->y = 0x19;
        r->h = 0xD;
    } else {
        x = r->x;
        end = x + r->w;
        r->y = 0x1B;
        r->h = 0xB;
        r->x = x;
        if (end <= x)
            r->w = 0;
        else
            r->w = end - x;
    }
    return e->lx + e->lw - ((unsigned int)(unsigned short)(e->lw - e->digits * 8) >> 1) - 1;
}

#ifdef NON_MATCHING /* o original soma 39 a partir de um registrador no retorno antecipado; o mwcc atual dobra a constante */
/* window animation of the pane with tab i */
int ds_tab_anim(int tab)
{
    int r = 0x27;

    if (dsv.mode < 2)
        return r + tab;
    if (dsv.mode >= 4)
        r += 5;
    else
        r += 3;
    return r + tab;
}
#endif



void ds_window_anim(void)
{
    int a = ds_tab_anim(dsv.pane[0].list->tab);
    void *e = res_get(0x8022);

    if (e != 0)
        ehf_set_anim(e, a, a + 1);
    ds_stream_anim();
}

/* frames of swapstream.ehf: layout column, deck short (0), over (1) or under (2) its size */
unsigned short ds_stream_anims[3][3][2] __attribute__((aligned(4))) = {
    { { 0x34, 0x35 }, { 0x00, 0x1A }, { 0x1A, 0x34 } },
    { { 0x9A, 0x9B }, { 0x3C, 0x6B }, { 0x6B, 0x9A } },
    { { 0x104, 0x105 }, { 0xA0, 0xD2 }, { 0xD2, 0x104 } },
};

#ifdef NON_MATCHING /* registradores de total/tamanho e do modo trocados (a0/v1 contra a1/a0) */
void ds_stream_anim(void)
{
    int total = ds.list[0].total;
    int size = ds.deck->n_main;
    int col = 0;
    int row = 0;
    void *e;
    unsigned short *a;

    if (dsv.mode >= 2) {
        if (dsv.mode < 4)
            col = 1;
        else
            col = 2;
    }
    if (size < total)
        row = 1;
    else if (total < size)
        row = 2;
    e = res_get(0x1010);
    if (e != 0) {
        a = ds_stream_anims[col][row];
        ehf_set_anim(e, a[0], a[1]);
    }
}
#endif

