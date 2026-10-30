// @range 0x0880C940-0x0880D54C
// @asm 0x0880D0F8-0x0880D2AC
/* Screen state: the cursor's card (info text measured after a while), the card info screen, the
   first setup after loading (lists, saved layout, card list sprites), cursor index and entry, tab
   switching with the saved scroll of each list, scroll clamping, and the undo history of moves. */
#include "libselect_card.h"

typedef struct ScCardListCfg {         /* BOOT cardlist_init */
    void *gim;
    void *digits_gim;
    void *frame_ehf;
    void *attr_gim;
    void *kind_gim;
    void *cil_gim;
    void *icon_gim;
    void *cip;
    int frame_view;
} ScCardListCfg;

int cfg_get_bits_1_0_2(void);
void sysstate_blob_get(void *dst);
void cardlist_init(void *cfg);
void cardinfo_open(int id, void *gim, void *bg);
int cardinfo_update(void);
void cardinfo_set_lists(void *gim, void *names, void *list);
int cardinfo_has_alt_pict(void);
unsigned short *card_get_desc(int id);
void sc_lists_build(int count, unsigned short *ids);
void sc_set_mode(unsigned short mode);
void sc_window_anim(void);
void sc_info_anim(void);
unsigned short sc_cur_index(unsigned short side);
ScEnt *sc_cur_entry(unsigned short side);
void sc_info_update(void);
void sc_set_panes(unsigned short which);
void sc_layout_scroll(unsigned short keep, int count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);

/* per frame: cursor timer ("new" mark cleared after 16 frames), info text when the card changes */
void sc_update_anim(void)
{
    ScEnt *e;

    if (sc.info_open) {
        sc_info_anim();
    } else {
        if (++scv.cursor.timer >= 0x10) {
            if (scv.cursor.ent != 0)
                scv.cursor.ent->marked = 1;
            if (scv.cursor.timer > 0x8000)
                scv.cursor.timer = 0x100;
        }
        if (!scv.lock && (e = sc_cur_entry(scv.side)) != 0 && e->id != scv.cursor.id)
            sc_info_update();
    }
}

/* the card info screen: fade out, open, run until closed, fade back */
void sc_info_anim(void)
{
    ScEnt *e;

    switch (sc.info_step) {
    case 0:
        fade_start_in(1, 0xFF000000, 16);
        sc.info_step++;
    case 1:
        if (fade_is_done() == 0)
            return;
        e = sc_cur_entry(scv.side);
        cardinfo_open(e->id, res_get(0x800C), res_get(0x8012));
        cardinfo_set_lists(res_get(0x8015), res_get(0x8036), res_get(0x8037));
        cardinfo_has_alt_pict();
        scv.info = 1;
        sc.info_step++;
        return;
    case 2:
        if (cardinfo_update() == 0)
            return;
        scv.info = 0;
        fade_start(1, 0xFF000000, 16);
        sc.info_step++;
    case 3:
        if (fade_is_done()) {
            sc.info_step = 0;
            sc.info_open = 0;
        }
        return;
    default:
        return;
    }
}

/* after loading: the lists, the saved blob and layout, the card list sprites */
void sc_setup(void)
{
    ScCardListCfg cfg;

    if (sc.out_count == 0)
        sc_lists_build(0, 0);
    else
        sc_lists_build(*sc.out_count, sc.out_ids);
    sysstate_blob_get(sc.words);
    scv.side = 0;
    scv.pane[0].list = &sc.list[0];
    scv.pane[1].list = &sc.list[1];
    sc_set_mode(cfg_get_bits_1_0_2());
    memset(&cfg, 0, sizeof(cfg));
    cfg.gim = res_get(0x8007);
    cfg.digits_gim = res_get(0x8009);
    cfg.frame_ehf = res_get(0x8023);
    cfg.attr_gim = res_get(0x8011);
    cfg.kind_gim = res_get(0x8010);
    cfg.cip = sc.cip;
    cardlist_init(&cfg);
    scv.cursor.scroll.id = -1;
    scv.cursor.id = 0;
    scv.cursor.key = 0;
    scv.cursor.ent = 0;
}

/* index of the cursor's entry in the visible order of pane side */
unsigned short sc_cur_index(unsigned short side)
{
    ScPane *p = &scv.pane[side];
    int r;

    if (p->list_mode == 0)
        r = p->cx + (p->top + p->cy * p->lay.cols);
    else
        r = p->top + p->cy;
    return r;
}

ScEnt *sc_cur_entry(unsigned short side)
{
    ScPane *p = &scv.pane[side];
    unsigned short i = sc_cur_index(side);

    return SC_ENT(p->list, i);
}

/* the cursor moved to another card: reset the timer, measure its text */
void sc_info_update(void)
{
    ScEnt *e;
    unsigned short id;
    int m;

    scv.cursor.timer = 0;
    e = sc_cur_entry(scv.side);
    if (e == 0) {
        scv.cursor.scroll.id = -1;
        scv.cursor.id = 0;
        scv.cursor.key = 0;
        scv.cursor.ent = 0;
        return;
    }
    id = e->id;
    if (id == 0) {
        scv.cursor.scroll.id = -1;
        scv.cursor.id = 0;
        scv.cursor.key = 0;
        scv.cursor.ent = 0;
        return;
    }
    scv.cursor.id = id;
    scv.cursor.key = e->key;
    scv.cursor.ent = e;
    font_set_size(12, 12);
    m = font_measure_text(card_get_desc(id)) - 0x4F40;
    scv.scroll_pos = 0;
    if (m <= 0)
        scv.scroll_max = 0;
    else
        scv.scroll_max = m;
    if (scv.mode < 4) {
        scv.cursor.scroll.id = id;
        scv.cursor.scroll.pos = 0;
    }
}

/* shows tab (how 0 = previous, 1 = next, else tab itself) in pane side, keeping each list's scroll */
void sc_select_tab(unsigned short side, unsigned short tab, unsigned short how)
{
    unsigned short n = 1;
    int i;
    ScPane *p;
    ScList *l;
    unsigned short keep;
    unsigned short view;
    unsigned short cols;

    if (scv.mode < 2) {
        switch (sc.kind) {
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
    switch (how) {
    default:
        i = tab;
        break;
    case 0:
        i = (tab + n - 1) % n;
        break;
    case 1:
        i = (tab + 1) % n;
        break;
    }
    keep = 1;
    p = &scv.pane[side];
    if (side == 1)
        i++;
    l = &sc.list[i];
    if (p->list->tab != l->tab) {
        sc_set_panes(side);
        p->list = l; p->top = p->list->u18;
        p->cx = p->list->u1c_lo;
        p->cy = p->list->u1c_hi;
        if (p->list->u1d_lo != ((p->list_mode == 0) ? p->lay.cols : 1))
            keep = 0;
        view = (p->list_mode == 0) ? p->lay.grid_rows : p->lay.rows;
        cols = (p->list_mode == 0) ? p->lay.cols : 1;
        sc_layout_scroll(keep, p->list->u1a, (cols & 0xFF) | (view & 0xFF) << 8,
                         p->list->count, &p->top, &p->cx, &p->cy);
        sc_window_anim();
    }
}

#ifdef NON_MATCHING /* lógica conferida com o asm (cols/rows int, keep u16, count int); 79 palavras: o original
                       tem 4 cópias mortas do destino depois de um `b` (não-corrigível) que deslocam o resto */
/* clamps a pane's scroll (top, cx, cy) to total entries in a view of view & 0xFF columns and
   view >> 8 rows; keep puts the cursor back on the entry it was on */
void sc_layout_scroll(unsigned short keep, int count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy)
{
    int cols = view & 0xFF;
    int rows = (view >> 8) & 0xFF;
    unsigned short area = cols * rows;
    unsigned short otop = *top;
    unsigned short cur = *cx + (otop + *cy * cols);
    int i;
    unsigned short r;

    if (cols <= 0)
        return;
    if (total < 2) {
        *top = 0;
        if (keep != 0)
            return;
        *cy = 0;
        *cx = 0;
        return;
    }
    if (area >= total) {
        *top = 0;
    } else if (otop + area - cols >= total) {
        *top = cols * ((total + cols - 1) / cols - rows);
    }
    if (otop != *top)
        *cy += (unsigned short)((short)((short)otop - (short)*top) / cols);
    if (!(cur < total))
        cur = total - 1;
    if (keep == 1) {
        if (*cx < cols) {
            if (*cy < rows)
                i = -1;
            else
                i = cur;
        } else {
            i = cur;
        }
    } else {
        i = count;
    }
    if (!(i < total))
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
#endif

/* stores the scroll of the pane(s) into their lists: which 0 = first, 1 = second, 2 = both */
void sc_set_panes(unsigned short which)
{
    if (which != 1) {
        ScPane *p = &scv.pane[0];
        unsigned short n;
        unsigned short v;

        n = (p->list_mode == 0) ? p->lay.cols : 1;
        v = p->cx + (p->top + p->cy * n);
        p->list->u18 = p->top;
        p->list->u1c_lo = p->cx;
        p->list->u1c_hi = p->cy;
        p->list->u1a = v;
        p->list->u1d_lo = n;
    }
    if (which != 0) {
        ScPane *p = &scv.pane[1];
        unsigned short n;
        unsigned short v;

        n = (p->list_mode == 0) ? p->lay.cols : 1;
        v = p->cx + (p->top + p->cy * n);
        p->list->u18 = p->top;
        p->list->u1c_lo = p->cx;
        p->list->u1c_hi = p->cy;
        p->list->u1a = v;
        p->list->u1d_lo = n;
    }
}

/* undo history: last 8 moves */
void sc_hist_push(ScMove *m)
{
    int n = sc.nhist;
    int i;

    if (n >= 8) {
        for (i = 0; i < 7; i++)
            memcpy(&sc.hist[i], &sc.hist[i + 1], 10);
        n--;
    } else {
        sc.nhist++;
    }
    memcpy(&sc.hist[n], m, 10);
}

int sc_hist_pop(ScMove *m)
{
    if (sc.nhist == 0)
        return 0;
    memcpy(m, &sc.hist[--sc.nhist], 10);
    return 1;
}
