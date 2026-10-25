// @range 0x0880BD30-0x0880C728
// @asm 0x0880BE18-0x0880BEA0
// @asm 0x0880BFB8-0x0880C270
// @asm 0x0880C2D4-0x0880C5F8
// @data 0x08822BE4-0x08822BF4
/* Panes: the card list sprites, the cursor's index and entry, the cursor card's info text, switching
   the list shown in a pane (each list keeps its own scroll), the list names, scroll clamping, saving
   the panes' scroll into their lists, and the undo history of moves. */
#include "libdeck.h"

typedef struct DkCardListCfg {         /* BOOT cardlist_init */
    void *gim;
    void *digits_gim;
    void *frame_ehf;
    void *attr_gim;
    void *kind_gim;
    void *cil_gim;
    void *icon_gim;
    void *cip;
    int frame_view;
} DkCardListCfg;

typedef struct DkMove {                /* a card moved between the lists, 10 bytes */
    unsigned char b[10];
} DkMove;

typedef struct DkBlock {               /* dk_blk 0x095D2330 (= dk_lists), 0x3D4 bytes cleared by dk_init */
    DkList lists[6];
    unsigned char pad0c0[0x316 - 0xC0];
    unsigned short nhist;              /* +0x316 */
    DkMove hist[8];                    /* +0x318 */
} DkBlock;

extern DkBlock dk_blk;

void cardlist_init(void *cfg);
unsigned short *card_get_desc(int id);
int font_measure_text(unsigned short *text);
int partner_count_get(void);

void func_0880B81C(void);
void dk_layout_scroll(int keep, int cur, unsigned short view, unsigned short total,
                      unsigned short *top, unsigned short *cx, unsigned short *cy);

/* the card list sprites, then the info of the card under the cursor */
void dk_cardlist_setup(void)
{
    DkCardListCfg cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.gim = dk_res_get(0x8007);
    cfg.digits_gim = dk_res_get(0x8009);
    cfg.frame_ehf = dk_res_get(0x8023);
    cfg.attr_gim = dk_res_get(0x8011);
    cfg.kind_gim = dk_res_get(0x8010);
    cfg.cip = dk_cip;
    cardlist_init(&cfg);
    dk_info_update();
}

/* index of the cursor's entry in the visible order of pane side */
unsigned short dk_cur_index(unsigned short side)
{
    DkPane *p = &dkv.pane[side];
    int r;

    if (p->list_mode == 0)
        r = p->cx + (p->top + p->cy * p->lay.cols);
    else
        r = p->top + p->cy;
    return r;
}

#ifdef NON_MATCHING /* o original calcula o ponteiro do painel antes da chamada e o guarda em s0; aqui o cálculo desce para depois do jal */
DkEnt *dk_cur_entry(unsigned short side)
{
    DkPane *const p = &dkv.pane[side];
    unsigned short i = dk_cur_index(side);
    DkList *l = p->list;

    if (i >= l->count)
        return 0;
    return &l->ents[l->order[i]];
}
#endif

/* the cursor moved to another card: reset the timer, measure its text */
void dk_info_update(void)
{
    DkEnt *e;
    unsigned short id;
    int m;

    dkv.cursor.timer = 0;
    e = dk_cur_entry(dkv.side);
    if (e == 0) {
        dkv.cursor.scroll.id = -1;
        dkv.cursor.id = 0;
        dkv.cursor.key = 0;
        dkv.cursor.ent = 0;
        return;
    }
    id = e->id;
    if (id == 0) {
        dkv.cursor.scroll.id = -1;
        dkv.cursor.id = 0;
        dkv.cursor.key = 0;
        dkv.cursor.ent = 0;
        return;
    }
    dkv.cursor.id = id;
    dkv.cursor.key = e->key;
    dkv.cursor.ent = e;
    font_set_size(12, 12);
    m = font_measure_text(card_get_desc(id)) - 0x4F40;
    dkv.scroll_pos = 0;
    if (m <= 0)
        dkv.scroll_max = 0;
    else
        dkv.scroll_max = m;
    if (dkv.mode < 4) {
        dkv.cursor.scroll.id = id;
        dkv.cursor.scroll.pos = 0;
    }
}

#ifdef NON_MATCHING /* cópias mortas depois de b (li s5,5 / li v1,1 repetidos, problema aberto) e o li a1,1 do teste de side */
/* shows tab (how 0 = previous, 1 = next, else tab itself) in pane side, keeping each list's scroll;
   the partner list (5 in the wide layout, 2 in the split one) is skipped when there is no partner */
void dk_select_tab(unsigned short side, unsigned short tab, unsigned short how)
{
    unsigned short n;
    int sel;
    DkPane *p;
    DkList *l;
    int keep;
    int grid;
    unsigned short view;

    if (dkv.mode > 1) {
        if (tab >= 3)
            tab -= 3;
        n = 3;
    } else {
        n = 6;
    }
    switch (how) {
    case 0:
        sel = (tab + n - 1) % n;
        break;
    case 1:
        sel = (tab + 1) % n;
        break;
    default:
        sel = tab;
        break;
    }
    if (dkv.mode > 1) {
        if (side == 1 && sel == 2 && partner_count_get() <= 0) {
            if (how == 0)
                sel = 1;
            else
                sel = 0;
        }
    } else {
        if (side != 0)
            return;
        if (sel == 4) {
            if (how == 0)
                sel = 3;
            else
                sel = 5;
        }
        if (sel == 5 && partner_count_get() <= 0) {
            if (how == 0)
                sel = 3;
            else
                sel = 0;
        }
    }
    p = &dkv.pane[side];
    keep = 1;
    if (side == 1)
        sel += 3;
    l = &dk_lists[sel];
    if (p->list->tab != l->tab) {
        dk_save_panes(side);
        p->list = l;
        p->top = l->u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        grid = p->list_mode == 0 ? p->lay.cols : 1;
        if (p->list->u1d_lo != grid)
            keep = 0;
        view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        dk_layout_scroll(keep, p->list->u1a,
                         (unsigned char)(p->list_mode == 0 ? p->lay.cols : 1) | (unsigned char)view << 8,
                         p->list->count, &p->top, &p->cx, &p->cy);
        func_0880B81C();
    }
}
#endif

unsigned short dk_list_name_ids[6] = { 1, 2, 3, 4, 3, 5 };

/* name of list tab (strings 0x8033) */
unsigned short *dk_list_name(unsigned short tab)
{
    if (tab >= 6)
        return (unsigned short *)L"";
    return table_get_entry(dk_res_get(0x8033), dk_list_name_ids[tab]);
}

#ifdef NON_MATCHING /* cópias mortas depois de b (problema aberto) e rows com sra no lugar de ext */
/* clamps a pane's scroll (top, cx, cy) to total entries in a view of view & 0xFF columns and
   view >> 8 rows; keep puts the cursor back on the entry it was on, else on entry cur */
void dk_layout_scroll(int keep, int cur, unsigned short view, unsigned short total,
                      unsigned short *top, unsigned short *cx, unsigned short *cy)
{
    int cols = view & 0xFF;
    int rows = view >> 8;
    unsigned short area = cols * rows;
    unsigned short otop = *top;
    unsigned short at = *cx + (otop + *cy * cols);
    int i;
    unsigned short r;

    if (cols <= 0)
        return;
    if (total < 2) {
        *top = 0;
        if ((unsigned short)keep != 0)
            return;
        *cy = 0;
        *cx = 0;
        return;
    }
    if (area < total) {
        if (otop + area - cols < total)
            goto keep_top;
        *top = cols * ((total + cols - 1) / cols - rows);
    } else {
        *top = 0;
    }
keep_top:
    if (otop != *top)
        *cy += (short)((short)otop - (short)*top) / cols;
    if (total <= at)
        at = total - 1;
    i = cur;
    if ((unsigned short)keep == 1) {
        if (*cx < cols && *cy < rows)
            i = -1;
        else
            i = at;
    }
    if (i >= total)
        i = total - 1;
    if (i >= 0) {
        r = i / cols;
        if (r < rows)
            *top = 0;
        else
            *top = cols * (r - (rows - 1));
        *cx = i % cols;
        *cy = r - *top / cols;
    }
}

/* stores the scroll of the pane(s) into their lists: which 0 = first, 1 = second, 2 = both
   (o escalonador intercala as leituras e gravações de outro jeito, como sc_set_panes) */
void dk_save_panes(unsigned short which)
{
    DkPane *p;
    unsigned short n;
    unsigned short v;

    if (which != 1) {
        p = &dkv.pane[0];
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
        p = &dkv.pane[1];
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
void dk_hist_push(DkMove *m)
{
    int n = dk_blk.nhist;
    int i;

    if (n >= 8) {
        for (i = 0; i < 7; i++)
            memcpy(&dk_blk.hist[i], &dk_blk.hist[i + 1], 10);
        n--;
    } else {
        dk_blk.nhist++;
    }
    memcpy(&dk_blk.hist[n], m, 10);
}

int dk_hist_pop(DkMove *m)
{
    if (dk_blk.nhist == 0)
        return 0;
    memcpy(m, &dk_blk.hist[--dk_blk.nhist], 10);
    return 1;
}
