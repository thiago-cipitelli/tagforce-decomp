// @range 0x08804608-0x08804E2C
// @asm 0x08804674-0x088046FC
// @asm 0x08804814-0x08804CFC
/* Cursor index and entry, the cursor card's info text (measured to scroll it), tab switching with
   the saved scroll of each list, scroll clamping, saving the panes' scroll into their lists, and
   the undo history of moves (same source as libselect_card sc_state.c). */
#include "libdeck_swap.h"

int font_measure_text(unsigned short *text);
unsigned short *card_get_desc(int id);
unsigned short ds_cur_index(unsigned short side);
DsEnt *ds_cur_entry(unsigned short side);
void ds_info_update(void);
void ds_set_panes(unsigned short which);
void ds_layout_scroll(short keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);

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

#ifdef NON_MATCHING /* o original calcula o ponteiro do painel antes da chamada e o guarda em s0 */
DsEnt *ds_cur_entry(unsigned short side)
{
    DsPane *p = &dsv.pane[side];
    unsigned short i = ds_cur_index(side);
    DsList *l = p->list;

    return DS_ENT(l, i);
}
#endif

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

#ifdef NON_MATCHING /* campos do painel por índice; alocação diferente */
/* shows tab (how 0 = previous, 1 = next, else tab itself) in pane side, keeping each list's scroll */
void ds_select_tab(unsigned short side, unsigned short tab, short how)
{
    unsigned int n = 1;
    DsPane *p;
    DsList *l;
    unsigned short grid;
    short view;

    if (dsv.mode < 2) {
        switch (ds.kind) {
        case 0:
            n = 2;
            break;
        case 1:
            n = 2;
            break;
        case 2:
            n = 2;
            break;
        }
    }
    if (how == 1)
        n = (tab + 1) % n;
    else if (how == 0)
        n = (int)(tab + n - 1) % (int)n;
    else
        n = tab;
    p = &dsv.pane[side];
    if (side == 1)
        n++;
    if (p->list->tab != ds.list[n].tab) {
        ds_set_panes(side);
        p->list = &ds.list[n];
        p->top = ds.list[n].u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        grid = p->list_mode == 0 ? p->lay.cols : 1;
        l = p->list;
        view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        ds_layout_scroll(l->u1d_lo == grid, l->u1a, ((p->list_mode == 0 ? p->lay.cols : 1) & 0xFF) | view << 8, l->count,
                         &p->top, &p->cx, &p->cy);
        ds_window_anim();
    }
}
#endif

#ifdef NON_MATCHING /* aritmética u16 em registradores diferentes */
/* clamps a pane's scroll (top, cx, cy) to total entries in a view of view & 0xFF columns and
   view >> 8 rows; keep puts the cursor back on the entry it was on */
void ds_layout_scroll(short keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy)
{
    unsigned int cols = view & 0xFF;
    unsigned int rows = view >> 8;
    unsigned short otop = *top;
    unsigned int t = otop;
    unsigned int cur = (unsigned short)(*cx + t + *cy * cols);
    int i;
    short r;

    if (cols == 0)
        return;
    if (total < 2) {
        *top = 0;
        if (keep != 0)
            return;
        *cy = 0;
        *cx = 0;
        return;
    }
    if (cols * rows < total) {
        if ((int)(t + cols * rows - cols) < (int)total)
            goto keep_top;
        *top = cols * ((short)((int)(total + cols - 1) / (int)cols) - (unsigned char)(view >> 8));
    } else {
        *top = 0;
    }
keep_top:
    if (t != *top)
        *cy += (short)(otop - *top) / (int)cols;
    if (total <= cur)
        cur = (unsigned short)(total - 1);
    i = count;
    if (keep == 1) {
        i = cur;
        if (*cx < cols && *cy < rows)
            i = -1;
    }
    if (i >= (int)total)
        i = total - 1;
    if (i >= 0) {
        r = i / (int)cols;
        if ((unsigned short)(i / (int)cols) < rows)
            *top = 0;
        else
            *top = cols * (r - ((unsigned char)(view >> 8) - 1));
        *cx = i % (int)cols;
        *cy = r - *top / cols;
    }
}
#endif

#ifdef NON_MATCHING /* o escalonador intercala as leituras e gravações de outro jeito */
/* stores the scroll of the pane(s) into their lists: which 0 = first, 1 = second, 2 = both */
void ds_set_panes(unsigned short which)
{
    DsPane *p;
    unsigned short n;
    unsigned short v;

    if (which != 1) {
        p = &dsv.pane[0];
        n = 1;
        if (p->list_mode == 0)
            n = p->lay.cols;
        v = p->cx + (p->top + p->cy * n);
        p->list->u18 = p->top;
        p->list->u1c_lo = p->cx;
        p->list->u1c_hi = p->cy;
        p->list->u1a = v;
        p->list->u1d_lo = n;
    }
    if (which != 0) {
        p = &dsv.pane[1];
        n = p->lay.cols;
        if (p->list_mode)
            n = 1;
        v = p->cx + (p->top + p->cy * n);
        p->list->u18 = p->top;
        p->list->u1c_lo = p->cx;
        p->list->u1c_hi = p->cy;
        p->list->u1a = v;
        p->list->u1d_lo = n;
    }
}
#endif

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
