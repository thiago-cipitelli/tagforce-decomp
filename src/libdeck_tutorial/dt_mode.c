// @range 0x08814460-0x08814D44
// @asm 0x08814460-0x088148A8
// @asm 0x088148A8-0x08814980
// @data 0x08822050-0x088221E4
/* Pane layout switching (the code of libdeck dk_mode.c, second half): layouts, modes, scroll kept per
   list, the tab buttons' positions and frames and the background animation. */
#include "libdeck_tutorial.h"

typedef struct DtMode {                /* dt_modes: pane layouts of a mode */
    short list_a;                      /* pane 0 in list mode */
    short lay_a;                       /* dt_layouts row of pane 0 */
    short list_b;                      /* pane 1, -1 = none */
    short lay_b;
} DtMode;

typedef struct DtTab {                 /* a tab button, 10 bytes */
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
} DtTab;

int card_get_limit(int id);
int sysflag_test(unsigned int bit);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
int partner_count_get(void);
void dt_layout_scroll(int keep, int cur, unsigned short view, unsigned short total,
                      unsigned short *top, unsigned short *cx, unsigned short *cy);
void dt_bg_anim_update(void);

void *dt_eres_get(unsigned short id);
unsigned short dt_cur_index(unsigned short side);
void dt_select_tab(unsigned short side, unsigned short tab, unsigned short how);
void dt_save_panes(unsigned short which);
int ehf_set_anim(void *e, int target, int limit);

DtLayout dt_layouts[5] = {
    { 148, 38, 331, 233, 3, 3, 7, 318, 30, 8, 8, 11, 28, 6, 34 },
    { 148, 38, 215, 233, 3, 3, 6, 200, 30, 8, 8, 7, 28, 6, 34 },
    { 364, 38, 115, 233, 4, 7, 6, 99, 34, 9, 8, 3, 32, 6, 34 },
    { 0, 38, 239, 203, 3, 3, 6, 226, 30, 7, 10, 8, 28, 5, 34 },
    { 240, 38, 239, 203, 3, 3, 6, 226, 30, 7, 10, 8, 28, 5, 34 },
};

DtMode dt_modes[9] = {   /* one zero row more than libdeck */
    { 1, 0, -1, -1 },
    { 0, 0, -1, -1 },
    { 0, 1, 0, 2 },
    { 0, 1, 1, 2 },
    { 1, 3, 1, 4 },
    { 0, 3, 0, 4 },
    { 1, 3, 0, 4 },
    { 0, 3, 1, 4 },
    { 0, 0, 0, 0 },
};

#ifdef NON_MATCHING /* o original usa a base dtv mais o lado para os campos do painel e a alocação dos s difere (como sc_set_mode) */
/* switches the pane layout (dt_modes row), keeping each list's scroll position */
void dt_set_mode(unsigned short mode)
{
    DtMode *md = &dt_modes[mode];
    unsigned short tab_b = dtv.pane[1].list->tab;
    unsigned short tab_a = dtv.pane[0].list->tab;
    int i;
    DtPane *p;
    DtList *l;
    unsigned short grid;
    unsigned short view;
    int tab;

    if (dtv.mode < 2)
        dt_save_panes(0);
    else
        dt_save_panes(2);
    dtv.pane[0].list_mode = md->list_a;
    memcpy(&dtv.pane[0].lay, &dt_layouts[md->lay_a], 0x1E);
    if (md->list_b >= 0)
        dtv.pane[1].list_mode = md->list_b;
    if (md->lay_b >= 0)
        memcpy(&dtv.pane[1].lay, &dt_layouts[md->lay_b], 0x1E);
    dtv.mode = mode;
    for (i = 0; i < 2; i++) {
        p = &dtv.pane[i];
        p->top = p->list->u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        grid = p->list_mode == 0 ? p->lay.cols : 1;
        l = p->list;
        view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        dt_layout_scroll(l->u1d_lo == grid, l->u1a, ((p->list_mode == 0 ? p->lay.cols : 1) & 0xFF) | view << 8,
                         l->count, &p->top, &p->cx, &p->cy);
        if (dtv.mode < 2)
            break;
    }
    if (dtv.mode < 2) {
        tab = -1;
        if (dtv.side != 0) {
            dtv.side = 0;
            if (tab_b == 5)
                tab = 5;
            else if (tab_b == 4)
                tab = 2;
            else if (tab_b == 3)
                tab = 3;
            if (tab >= 0)
                dt_select_tab(0, tab, 2);
        }
    } else if (tab_a > 2) {
        tab = -1;
        if (tab_a == 5)
            tab = 5;
        else if (tab_a == 3)
            tab = 3;
        dt_select_tab(0, 0, 2);
        if (tab >= 0) {
            dt_select_tab(1, tab, 2);
            dtv.side = 1;
        }
    }
    for (i = 0; i < 2; i++) {
        p = &dtv.pane[i];
        p->top = p->list->u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        grid = p->list_mode == 0 ? p->lay.cols : 1;
        l = p->list;
        view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        dt_layout_scroll(l->u1d_lo == grid, l->u1a, ((p->list_mode == 0 ? p->lay.cols : 1) & 0xFF) | view << 8,
                         l->count, &p->top, &p->cx, &p->cy);
        if (dtv.mode < 2)
            break;
    }
    dt_bg_anim_update();
    if (dt_cur_entry(dtv.side) != 0)
        dt_info_update();
}
#endif

#ifdef NON_MATCHING /* 32 palavras: o original não encadeia os saltos para o fim (os "r = -1" dos testes de b caem num bloco b próprio para o epílogo), como sc_mode_for; já com parâmetros u16 e os testes na ordem do asm */
/* dt_modes row for a pane count, narrow second pane and the list mode of each pane; -1 none */
int dt_mode_for(unsigned short panes, int narrow, unsigned short a, unsigned short b)
{
    int r = 1;

    if (panes == 1) {
        if (a == 1)
            r = 0;
        else if (a != 0)
            r = -1;
    } else if (narrow) {
        if (a != 0) {
            r = -1;
        } else {
            if (b == 1)
                r = 3;
            else if (b == 0)
                r = 2;
            else
                r = -1;
        }
    } else if (a == 0) {
        if (b == 1)
            r = 7;
        else if (b == 0)
            r = 5;
        else
            r = -1;
    } else if (a != 1) {
        r = -1;
    } else {
        if (b == 1)
            r = 4;
        else if (b == 0)
            r = 6;
        else
            r = -1;
    }
    return r;
}
#endif

/* tab buttons of the three layouts (one pane, two panes, two wide panes) */
DtTab dt_tabs[3][6] = {
    { { 0, 0, 60, 0, 37, 0, 19, 2, 0, 0 }, { 60, 0, 60, 0, 97, 0, 19, 2, 0, 0 }, { 120, 0, 60, 0, 157, 0, 19, 2, 0, 0 },
      { 180, 0, 91, 0, 218, 0, 49, 6, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 271, 0, 60, 0, 316, 0, 11, 1, 0, 0 } },
    { { 0, 0, 60, 0, 37, 0, 19, 2, 0, 0 }, { 60, 0, 60, 0, 97, 0, 19, 2, 0, 0 }, { 120, 0, 60, 0, 157, 0, 19, 2, 0, 0 },
      { 0, 0, 91, 0, 38, 0, 49, 6, 0, 0 }, { 43, 0, 60, 0, 80, 0, 19, 2, 0, 0 }, { 55, 0, 60, 0, 100, 0, 11, 1, 0, 0 } },
    { { 0, 0, 60, 0, 37, 0, 19, 2, 0, 0 }, { 60, 0, 60, 0, 97, 0, 19, 2, 0, 0 }, { 120, 0, 60, 0, 157, 0, 19, 2, 0, 0 },
      { 0, 0, 91, 0, 38, 0, 49, 6, 0, 0 }, { 91, 0, 60, 0, 128, 0, 19, 2, 0, 0 }, { 151, 0, 60, 0, 196, 0, 11, 1, 0, 0 } },
};

/* x and width of tab button i */
unsigned int dt_tab_pos(unsigned short i)
{
    DtTab *e;

    if (dtv.mode < 2)
        e = &dt_tabs[0][i];
    else if (dtv.mode < 4)
        e = &dt_tabs[1][i];
    else
        e = &dt_tabs[2][i];
    return e->x | e->w << 16;
}

/* frame of tab button i (the selected one taller, the others clipped by the selected neighbours)
   and the x of its count */
unsigned short dt_tab_rect(unsigned short i, unsigned short sel, DtRect *r)
{
    int a = -1;
    DtTab *e;
    int b = -1;
    unsigned short x;
    unsigned short end;
    unsigned int p;
    int px;

    if (dtv.mode < 2) {
        e = &dt_tabs[0][i];
    } else if (dtv.mode < 4) {
        e = &dt_tabs[1][i];
        if (sel != i && i >= 3) {
            if (i != 4)
                a = 4;
            b = sel;
        }
    } else {
        e = &dt_tabs[2][i];
    }
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
        if (a >= 0) {
            p = dt_tab_pos(a);
            px = p & 0xFFFF;
            if (px < e->x && x < (int)(px + (p >> 16 & 0xFFFF)))
                x = px + (p >> 16 & 0xFFFF);
            if (e->x < px && px < end)
                end = p;
        }
        if (b >= 0) {
            p = dt_tab_pos(b);
            px = p & 0xFFFF;
            if (px < e->x && x < (int)(px + (p >> 16 & 0xFFFF)))
                x = px + (p >> 16 & 0xFFFF);
            if (e->x < px && px < end)
                end = p;
        }
        r->x = x;
        if (end <= x)
            r->w = 0;
        else
            r->w = end - x;
    }
    return e->lx + e->lw - ((unsigned int)(unsigned short)(e->lw - e->digits * 8) >> 1) - 1;
}

/* frame of bg.ehf for the tabs shown (no partner list in the lesson) */
static int dt_bg_anim(int a, int b)
{
    int r = 0;

    if (dtv.mode < 2) {
        if (a == 5)
            a--;
        r += a;
    } else {
        if (dtv.mode >= 4)
            r += 24;
        else
            r += 9;
        r += a * 5;
        r += b >= 3 ? b - 3 : b;
    }
    return r;
}

void dt_bg_anim_update(void)
{
    void *e = dt_eres_get(0x8022);
    int k = dt_bg_anim(dtv.pane[0].list->tab, dtv.pane[1].list->tab);

    if (e != 0)
        ehf_set_anim(e, k, k + 1);
}

