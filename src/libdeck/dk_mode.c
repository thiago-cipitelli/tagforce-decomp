// @range 0x0880A694-0x0880B874
// @asm 0x0880AF34-0x0880B37C
// @asm 0x0880B37C-0x0880B454
// @asm 0x0880B75C-0x0880B81C
// @rodata 0x08822554-0x08822574
// @data 0x08822A24-0x08822BE4
/* Deck check before saving (sizes, forbidden/limited cards), the message popup (deck errors, card
   limits, search/filter result count, quit confirmation), pane layout switching (layouts, modes,
   scroll kept per list), the tab buttons' positions and frames and the background animation. */
#include "libdeck.h"
#include "menu.h"

typedef struct DkMode {                /* dk_modes: pane layouts of a mode */
    short list_a;                      /* pane 0 in list mode */
    short lay_a;                       /* dk_layouts row of pane 0 */
    short list_b;                      /* pane 1, -1 = none */
    short lay_b;
} DkMode;

typedef struct DkTab {                 /* a tab button, 10 bytes */
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
} DkTab;

extern unsigned short dk_popup_arg;            /* 0x08823958 */
extern unsigned short dk_popup_text[0x400];    /* 0x0882395C */
extern MenuWidget dk_popup;                    /* 0x0882415C */

int card_get_limit(int id);
int sysflag_test(unsigned int bit);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
int partner_count_get(void);
void dk_layout_scroll(int keep, int cur, unsigned short view, unsigned short total,
                      unsigned short *top, unsigned short *cx, unsigned short *cy);
void dk_bg_anim_update(void);

/* 0 ok; 1 main deck under 40, 2 over 80, 3 extra deck over 15, 4 side deck over 30, 5 a card over
   its limit (*id) */
int dk_deck_check(unsigned short *id)
{
    int n = 0;
    int main = dk_lists[0].total;
    int extra = dk_lists[2].total;
    int side = dk_lists[1].total;
    unsigned short tabs[3] = { 0, 2, 1 };
    int k;
    unsigned short *t;
    int std;
    DkList *l;
    int i;
    int lim;
    DkEnt *e;
    int copies;

    if (main < 40)
        return 1;
    if (main > 80)
        return 2;
    if (extra > 15)
        return 3;
    if (side > 30)
        return 4;
    if (dk_scene_arg == 2)
        return 0;
    std = sysflag_test(0) != 0;
    for (k = 0, t = tabs; k < 3; k++, t++) {
        l = &dk_lists[*t];
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
                    copies = (unsigned short)dk_list_count_id(&dk_lists[1], e->id, 0);
                    copies += (unsigned short)dk_list_count_id(&dk_lists[0], e->id, 0);
                    if (lim < copies + (unsigned short)dk_list_count_id(&dk_lists[2], e->id, 0)) {
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
unsigned short dk_popup_msgs2[10] = { 0, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0 };
/* list full messages (popup 4) */
unsigned short dk_popup_msgs4[6] = { 0, 0x44, 0x6D, 0x6E, 0x6F, 0 };

/* opens popup id: 2 deck error arg, 3 quit confirmation, 4 list full arg, 5 card limit (card arg),
   6 search/filter result (arg = count), 7 deck operation confirmation */
void dk_popup_open(unsigned short id, unsigned short arg)
{
    MenuWidget *m = &dk_popup;
    MenuItemDef *d = &dk_popup.def;
    unsigned char buttons = 1;
    int lim;
    unsigned short *name;
    int alt;

    dk_popup_arg = arg;
    memset(m, 0, 0x120);
    memset(dk_popup_text, 0, 0x800);
    switch (id) {
    case 0:
    case 1:
        break;
    case 2:
        wcscpy(dk_popup_text, table_get_entry(dk_res_get(0x8033), dk_popup_msgs2[arg]));
        m->cancel_blocked = 1;
        break;
    case 3:
        wcscpy(dk_popup_text, table_get_entry(dk_res_get(0x8033), 0x57));
        m->initial = 4;
        buttons = 2;
        m->cancel_resets = 1;
        break;
    case 4:
        wcscpy(dk_popup_text, table_get_entry(dk_res_get(0x8033), dk_popup_msgs4[arg]));
        m->cancel_blocked = 1;
        break;
    case 5:
        lim = card_get_limit(arg);
        alt = sysflag_test(0) != 0;
        name = card_get_name(arg);
        switch (lim) {
        case 0:
            if (alt > 0)
                wsprintf(dk_popup_text, table_get_entry(dk_res_get(0x8033), 0x88), alt);
            else
                wsprintf(dk_popup_text, table_get_entry(dk_res_get(0x8033), 0x85), name);
            break;
        case 1:
            wsprintf(dk_popup_text, table_get_entry(dk_res_get(0x8033), 0x86), name);
            break;
        case 2:
            wsprintf(dk_popup_text, table_get_entry(dk_res_get(0x8033), 0x87), name);
            break;
        }
        m->cancel_blocked = 1;
        break;
    case 6:
        if (arg <= 0)
            wcscpy(dk_popup_text, table_get_entry(dk_res_get(0x8033), 0x89));
        else
            wsprintf(dk_popup_text, table_get_entry(dk_res_get(0x8033), 0x81), arg);
        d->auto_size = 1;
        m->initial = 1;
        m->cancel_resets = 1;
        break;
    case 7:
        wsprintf(dk_popup_text, table_get_entry(dk_res_get(0x8033), 0x8A), arg + 1,
                 table_get_entry(dk_res_get(0x8033), arg + 0xA3));
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
    d->text = dk_popup_text;
    d->open_anim = 2;
    d->close_anim = 2;
    m->heap = dk_heap;
    m->no_cancel = 1;
    m->buttons = buttons;
    d->style = 0;
    d->pad_b = 0;
    menu_open(m);
    d->x = (0x1E0 - d->w) / 2;
    d->y = (0x110 - d->h) / 2;
    dkv.popup = id;
}

/* help page shown with START on popup 4 */
unsigned short dk_popup_help4[6] = { 0, 1, 1, 3, 4, 0 };

/* the popup's input; 1 when Yes was picked on the quit confirmation */
int dk_popup_update(void)
{
    int done = 0;
    int r = menu_update(&dk_popup);
    unsigned short id;

    if (pad_get()->b.trig & 8) {
        switch (dkv.popup) {
        case 5:
            dk_help_page = 2;
            break;
        case 4:
            dk_help_page = dk_popup_help4[dk_popup_arg];
            break;
        }
    }
    if (r != -1) {
        id = dkv.popup;
        menu_reset(&dk_popup);
        dkv.popup = 0;
        switch (id) {
        case 3:
            if (r == 1)
                done = 1;
            break;
        case 7:
            if (r == 2)
                dk_flags.b7 = 1;
            else
                dk_check_rewards();
            break;
        }
    }
    return done;
}

void *dk_popup_draw(void *list)
{
    menu_draw(&list, &dk_popup);
    return list;
}

DkLayout dk_layouts[5] = {
    { 148, 38, 331, 233, 3, 3, 7, 318, 30, 8, 8, 11, 28, 6, 34 },
    { 148, 38, 215, 233, 3, 3, 6, 200, 30, 8, 8, 7, 28, 6, 34 },
    { 364, 38, 115, 233, 4, 7, 6, 99, 34, 9, 8, 3, 32, 6, 34 },
    { 0, 38, 239, 203, 3, 3, 6, 226, 30, 7, 10, 8, 28, 5, 34 },
    { 240, 38, 239, 203, 3, 3, 6, 226, 30, 7, 10, 8, 28, 5, 34 },
};

DkMode dk_modes[8] = {
    { 1, 0, -1, -1 },
    { 0, 0, -1, -1 },
    { 0, 1, 0, 2 },
    { 0, 1, 1, 2 },
    { 1, 3, 1, 4 },
    { 0, 3, 0, 4 },
    { 1, 3, 0, 4 },
    { 0, 3, 1, 4 },
};

#ifdef NON_MATCHING /* o original usa a base dkv mais o lado para os campos do painel e a alocação dos s difere (como sc_set_mode) */
/* switches the pane layout (dk_modes row), keeping each list's scroll position */
void dk_set_mode(unsigned short mode)
{
    DkMode *md = &dk_modes[mode];
    unsigned short tab_b = dkv.pane[1].list->tab;
    unsigned short tab_a = dkv.pane[0].list->tab;
    int i;
    DkPane *p;
    DkList *l;
    unsigned short grid;
    unsigned short view;
    int tab;

    if (dkv.mode < 2)
        dk_save_panes(0);
    else
        dk_save_panes(2);
    dkv.pane[0].list_mode = md->list_a;
    memcpy(&dkv.pane[0].lay, &dk_layouts[md->lay_a], 0x1E);
    if (md->list_b >= 0)
        dkv.pane[1].list_mode = md->list_b;
    if (md->lay_b >= 0)
        memcpy(&dkv.pane[1].lay, &dk_layouts[md->lay_b], 0x1E);
    dkv.mode = mode;
    for (i = 0; i < 2; i++) {
        p = &dkv.pane[i];
        p->top = p->list->u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        grid = p->list_mode == 0 ? p->lay.cols : 1;
        l = p->list;
        view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        dk_layout_scroll(l->u1d_lo == grid, l->u1a, ((p->list_mode == 0 ? p->lay.cols : 1) & 0xFF) | view << 8,
                         l->count, &p->top, &p->cx, &p->cy);
        if (dkv.mode < 2)
            break;
    }
    if (dkv.mode < 2) {
        tab = -1;
        if (dkv.side != 0) {
            dkv.side = 0;
            if (tab_b == 5)
                tab = 5;
            else if (tab_b == 4)
                tab = 2;
            else if (tab_b == 3)
                tab = 3;
            if (tab >= 0)
                dk_select_tab(0, tab, 2);
        }
    } else if (tab_a > 2) {
        tab = -1;
        if (tab_a == 5)
            tab = 5;
        else if (tab_a == 3)
            tab = 3;
        dk_select_tab(0, 0, 2);
        if (tab >= 0) {
            dk_select_tab(1, tab, 2);
            dkv.side = 1;
        }
    }
    for (i = 0; i < 2; i++) {
        p = &dkv.pane[i];
        p->top = p->list->u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        grid = p->list_mode == 0 ? p->lay.cols : 1;
        l = p->list;
        view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        dk_layout_scroll(l->u1d_lo == grid, l->u1a, ((p->list_mode == 0 ? p->lay.cols : 1) & 0xFF) | view << 8,
                         l->count, &p->top, &p->cx, &p->cy);
        if (dkv.mode < 2)
            break;
    }
    dk_bg_anim_update();
    if (dk_cur_entry(dkv.side) != 0)
        dk_info_update();
}
#endif

#ifdef NON_MATCHING /* o original não encadeia os saltos para o fim (cada grupo termina num b para o epílogo), como sc_mode_for */
/* dk_modes row for a pane count, narrow second pane and the list mode of each pane; -1 none */
int dk_mode_for(short panes, int narrow, short a, short b)
{
    int r = 1;

    if (panes == 1) {
        if (a == 1)
            r = 0;
        else if (a != 0)
            r = -1;
    } else if (narrow == 0) {
        if (a == 0) {
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
    } else if (a == 0) {
        if (b == 1)
            r = 3;
        else if (b == 0)
            r = 2;
        else
            r = -1;
    } else {
        r = -1;
    }
    return r;
}
#endif

/* tab buttons of the three layouts (one pane, two panes, two wide panes) */
DkTab dk_tabs[3][6] = {
    { { 0, 0, 60, 0, 37, 0, 19, 2, 0, 0 }, { 60, 0, 60, 0, 97, 0, 19, 2, 0, 0 }, { 120, 0, 60, 0, 157, 0, 19, 2, 0, 0 },
      { 180, 0, 91, 0, 218, 0, 49, 6, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 271, 0, 60, 0, 316, 0, 11, 1, 0, 0 } },
    { { 0, 0, 60, 0, 37, 0, 19, 2, 0, 0 }, { 60, 0, 60, 0, 97, 0, 19, 2, 0, 0 }, { 120, 0, 60, 0, 157, 0, 19, 2, 0, 0 },
      { 0, 0, 91, 0, 38, 0, 49, 6, 0, 0 }, { 43, 0, 60, 0, 80, 0, 19, 2, 0, 0 }, { 55, 0, 60, 0, 100, 0, 11, 1, 0, 0 } },
    { { 0, 0, 60, 0, 37, 0, 19, 2, 0, 0 }, { 60, 0, 60, 0, 97, 0, 19, 2, 0, 0 }, { 120, 0, 60, 0, 157, 0, 19, 2, 0, 0 },
      { 0, 0, 91, 0, 38, 0, 49, 6, 0, 0 }, { 91, 0, 60, 0, 128, 0, 19, 2, 0, 0 }, { 151, 0, 60, 0, 196, 0, 11, 1, 0, 0 } },
};

/* x and width of tab button i */
unsigned int dk_tab_pos(unsigned short i)
{
    DkTab *e;

    if (dkv.mode < 2)
        e = &dk_tabs[0][i];
    else if (dkv.mode < 4)
        e = &dk_tabs[1][i];
    else
        e = &dk_tabs[2][i];
    return e->x | e->w << 16;
}

/* frame of tab button i (the selected one taller, the others clipped by the selected neighbours)
   and the x of its count */
unsigned short dk_tab_rect(unsigned short i, unsigned short sel, DkRect *r)
{
    int a = -1;
    DkTab *e;
    int b = -1;
    unsigned short x;
    unsigned short end;
    unsigned int p;
    int px;

    if (dkv.mode < 2) {
        e = &dk_tabs[0][i];
    } else if (dkv.mode < 4) {
        e = &dk_tabs[1][i];
        if (sel != i && i >= 3) {
            if (i != 4)
                a = 4;
            b = sel;
        }
    } else {
        e = &dk_tabs[2][i];
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
            p = dk_tab_pos(a);
            px = p & 0xFFFF;
            if (px < e->x && x < (int)(px + (p >> 16 & 0xFFFF)))
                x = px + (p >> 16 & 0xFFFF);
            if (e->x < px && px < end)
                end = p;
        }
        if (b >= 0) {
            p = dk_tab_pos(b);
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

#ifdef NON_MATCHING /* o último teste sai como bnezl com a soma no delay slot; o original tem bnez + nop */
/* frame of bg.ehf for the tabs shown */
int dk_bg_anim(int a, int b)
{
    int r = 0;

    if (dkv.mode < 2) {
        if (partner_count_get() > 0)
            r += 4;
        if (a == 5)
            a--;
        r += a;
    } else {
        if (dkv.mode >= 4)
            r += 24;
        else
            r += 9;
        r += a * 5;
        if (partner_count_get() > 0)
            r += 2;
        if (b >= 3)
            b -= 3;
        r += b;
    }
    return r;
}
#endif

void dk_bg_anim_update(void)
{
    void *e = dk_res_get(0x8022);
    int k = dk_bg_anim(dkv.pane[0].list->tab, dkv.pane[1].list->tab);

    if (e != 0)
        ehf_set_anim(e, k, k + 1);
}
