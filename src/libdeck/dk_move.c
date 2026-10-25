// @range 0x088060B0-0x0880931C
// @asm 0x088060B0-0x08806C18
// @asm 0x08806C18-0x08807100
// @asm 0x08807100-0x08807804
// @asm 0x08807BEC-0x08807F78
// @asm 0x08807F78-0x088083D4
// @asm 0x088083D4-0x08808958
// @asm 0x08808EE4-0x088091E4
// @rodata 0x08822510-0x08822534
/* Moving a card between the lists (Circle in the deck editor, undo with Triangle): the destination
   window offering the lists the card can go to (main, side, extra, trunk) with the full ones greyed
   out, the card flown along a curve from its slot to the slot it lands in, the result checks of the
   deck limits, and the slot positions on screen. */
#include "libdeck.h"
#include "menu.h"

typedef struct DkHist {                /* undo record (dk_hist_push), 10 bytes */
    unsigned char src : 4;             /* source list */
    unsigned char dst : 4;             /* destination list */
    unsigned char pad01;
    DkEnt ent;                         /* +0x02 */
    unsigned short pos;                /* +0x08 position in the destination order */
} DkHist;

typedef struct DkFlight {              /* dk_flight 0x08823950, 0x244 bytes, heap allocated */
    unsigned char step : 4;            /* +0x000 1 fly, 2..5 window, 6 fly to the pick, 7 land, 8 done */
    unsigned char phase : 4;           /*        flying: 0 path, 1 move, 2 land */
    unsigned char timer;               /* +0x001 */
    unsigned short idx;                /* +0x002 entry index in the source list */
    unsigned short pos;                /* +0x004 position in its visible order */
    unsigned short side : 2;           /* +0x006 source pane */
    unsigned short src : 3;            /*        source list */
    unsigned short dside : 2;          /*        destination pane */
    unsigned short n : 3;              /*        destinations offered */
    unsigned short sel : 3;            /*        destination picked */
    unsigned short pad06 : 3;
    unsigned short dst[3];             /* +0x008 destination lists */
    unsigned short full[3];            /* +0x00E destination cannot take the card */
    unsigned short dtab;               /* +0x014 tab shown in the destination pane */
    unsigned short x;                  /* +0x016 destination window */
    unsigned short y;                  /* +0x018 */
    unsigned short w;                  /* +0x01A */
    DkEnt ent;                         /* +0x01C card being moved */
    unsigned short text[0x100];        /* +0x022 window text */
    unsigned char shown : 1;           /* +0x222 card drawn at cur */
    unsigned char frames : 3;          /*        frames per scroll step of the destination pane */
    unsigned char pad222 : 4;
    unsigned char pad223;
    float from[2];                     /* +0x224 */
    float mid[2];                      /* +0x22C */
    float to[2];                       /* +0x234 */
    short cur[2];                      /* +0x23C */
    MenuItemDef *item;                 /* +0x240 destination window */
} DkFlight;

extern DkFlight *dk_flight;            /* 0x08823950 */
extern unsigned short dk_flight_last_side; /* 0x08823954 destination of the last move */
extern unsigned short dk_flight_last_list; /* 0x08823956 */

int sysflag_test(unsigned int bit);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
unsigned short dk_deck_count_forbidden(void);
void dk_hist_push(DkHist *h);
int dk_hist_pop(DkHist *h);
void dk_layout_scroll(int keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
unsigned int func_0880B454(unsigned short tab);
void *dk_draw_shade(void *list, unsigned short side, unsigned short tab, int lit);
void *dk_draw_card_pict(void *list, int x, int y, int z, unsigned int color, DkEnt *e, void *gim, void *gim_alt, void *alt);

void dk_flight_land(void);
void dk_flight_window_input(PadBits *pad);
void dk_flight_take_step(void);
void dk_flight_put_step(void);
void dk_flight_free(void);
void *dk_flight_draw_window(void *list, int card);
void dk_flight_window_pos(void);
void dk_flight_take(void);
void dk_flight_put(void);
unsigned int dk_flight_window_state(void);
void dk_flight_text(void);

/* adds destination k to the window */
#define DK_FLIGHT_ADD(k) (dk_flight->dst[dk_flight->n++] = (k))
/* the next destination offered cannot take the card */
#define DK_FLIGHT_FULL() (dk_flight->full[dk_flight->n] = 1)
/* gives the move up with message r */
#define DK_FLIGHT_FAIL(r)                       \
    do {                                        \
        if (dk_flight != 0) {                   \
            heap_set_level(dk_heap, dk_flight); \
            dk_flight = 0;                      \
        }                                       \
        return r;                               \
    } while (0)

#ifdef NON_MATCHING /* forma igual; a soma das cópias e o limite (int lim/n) ficam em $s diferentes (um $s a mais) */
/* Circle on a card: opens the destination window (0), or returns why it cannot move (2 nothing to
   move to, 4 three copies already, 5 forbidden cards, 6/7 limited / semi-limited, 8 no memory) */
int dk_flight_open(void)
{
    DkEnt *e;
    int i;
    MenuItemDef *it;
    int lim;
    int n;

    dk_flight = heap_alloc(dk_heap, 0x244);
    if (dk_flight == 0)
        return 8;
    memset(dk_flight, 0, 0x244);
    dk_flight->side = dkv.side;
    dk_flight->src = dkv.pane[dk_flight->side].list->tab;
    dk_flight->pos = dk_cur_index(dkv.side);
    dk_flight->idx = dk_lists[dk_flight->src].order[dk_flight->pos];
    e = dk_cur_entry(dkv.side);
    if (e == 0)
        DK_FLIGHT_FAIL(0);
    if (dkv.mode > 1)
        dk_flight->dside = (dk_flight->side + 1) & 1;
    else
        dk_flight->dside = dk_flight->side;
    switch (dk_flight->src) {
    case 0:
        if (e->fixed)
            DK_FLIGHT_ADD(5);
        else
            DK_FLIGHT_ADD(3);
        if ((unsigned int)dk_lists[4].total >= 20)
            DK_FLIGHT_FULL();
        if (dkv.mode > 1)
            DK_FLIGHT_ADD(4);
        else
            DK_FLIGHT_ADD(2);
        break;
    case 2:
        if (e->fixed)
            DK_FLIGHT_ADD(5);
        else
            DK_FLIGHT_ADD(3);
        if (dkv.mode < 2 && (unsigned int)dk_lists[0].total < 0x55)
            DK_FLIGHT_ADD(0);
        break;
    case 1:
        if (e->fixed)
            DK_FLIGHT_ADD(5);
        else
            DK_FLIGHT_ADD(3);
        break;
    case 3:
    case 5:
        if (card_get_attribute_icon(e->id) == 3) {
            if ((unsigned int)dk_lists[1].total >= 0x23)
                DK_FLIGHT_FULL();
            DK_FLIGHT_ADD(1);
        } else {
            if ((unsigned int)dk_lists[0].total >= 0x55)
                DK_FLIGHT_FULL();
            DK_FLIGHT_ADD(0);
            if ((unsigned int)dk_lists[2].total >= 20)
                DK_FLIGHT_FULL();
            DK_FLIGHT_ADD(2);
        }
        n = (unsigned short)dk_list_count_id(&dk_lists[1], e->id, 1);
        n = (unsigned short)(n + dk_list_count_id(&dk_lists[0], e->id, 1));
        if ((unsigned short)(n + dk_list_count_id(&dk_lists[2], e->id, 1)) >= 3)
            DK_FLIGHT_FAIL(4);
        lim = (unsigned short)card_get_limit(e->id);
        n = (unsigned short)dk_list_count_id(&dk_lists[1], e->id, 0);
        n = (unsigned short)(n + dk_list_count_id(&dk_lists[0], e->id, 0));
        if ((unsigned short)(n + dk_list_count_id(&dk_lists[2], e->id, 0)) >= (unsigned short)lim) {
            switch ((unsigned short)lim) {
            case 0:
                lim = sysflag_test(0) != 0;
                if (dk_deck_count_forbidden() >= lim)
                    DK_FLIGHT_FAIL(5);
                break;
            case 1:
                DK_FLIGHT_FAIL(6);
            case 2:
                DK_FLIGHT_FAIL(7);
            }
        }
        break;
    case 4:
        if ((unsigned int)dk_lists[0].total >= 0x55)
            DK_FLIGHT_FULL();
        DK_FLIGHT_ADD(0);
        break;
    }
    memcpy(&dk_flight->ent, e, 6);
    if (dk_flight->n <= 0)
        DK_FLIGHT_FAIL(2);
    if (dk_flight_last_side == dk_flight->dside) {
        for (i = 0; i < dk_flight->n; i++) {
            if (dk_flight->dst[i] == dk_flight_last_list) {
                dk_flight->sel = i;
                break;
            }
        }
    }
    dkv.lock = 1;
    dk_flight_text();
    dk_flight_window_pos();
    dk_flight->dtab = dkv.pane[dk_flight->dside].list->tab;
    dk_flight->item = heap_alloc(dk_heap, 0x78);
    it = dk_flight->item;
    if (it == 0)
        DK_FLIGHT_FAIL(8);
    memset(it, 0, 0x78);
    it->visible = 1;
    it->style = 2;
    it->x = dk_flight->x;
    it->y = dk_flight->y;
    it->w = 0xE0;
    it->h = 0x27;
    it->color = 0xFFFFFFFF;
    if (dk_flight->w <= 0x70)
        it->tab = 3;
    else
        it->tab = 4;
    it->tab_x = dk_flight->w;
    it->text = dk_flight->text;
    it->font_flag4 = 1;
    it->pad_l = 0x1E;
    it->pad_t = 4;
    it->font_size = 0xC;
    it->text_color = 0xFFFFFFFF;
    it->open_anim = 1;
    it->close_anim = 2;
    menu_add_item(&dk_menu, it);
    menu_item_open(&dk_menu, it);
    dk_state = 1;
    dk_flight->cur[1] = -100;
    dk_flight->step = 1;
    return 0;
}
#endif

/* pane showing list tab in the two-pane layouts: the deck lists left, the trunk right */
static inline int dk_tab_side(int tab)
{
    if (tab < 3)
        return 0;
    return 1;
}

#ifdef NON_MATCHING /* o original calcula o lado (src >= 3) com desvio; o mwcc dobra em slti/xori, e os temporários mudam */
/* Triangle: takes the last move back (the card flies from where it landed to its old list) */
int dk_flight_undo(void)
{
    DkHist h;
    int found;
    DkList *l;
    int i;
    int k;

    if (dk_hist_pop(&h) == 0)
        return 0;
    dk_flight = heap_alloc(dk_heap, 0x244);
    if (dk_flight == 0)
        return 0;
    memset(dk_flight, 0, 0x244);
    dk_flight->src = h.dst;
    dk_flight->idx = h.pos;
    DK_FLIGHT_ADD(h.src);
    if (dkv.mode > 1) {
        if (dk_flight->src == 2 && dk_flight->dst[0] < 2)
            dk_flight->src = 4;
        if (dk_flight->dst[0] == 2)
            dk_flight->dst[0] = 4;
        dk_flight->side = dk_tab_side(dk_flight->src);
        dk_flight->dside = (dk_flight->side + 1) & 1;
    } else {
        dk_flight->side = 0;
        dk_flight->dside = 0;
        if (dk_flight->src == 4)
            dk_flight->src = 2;
        if (dk_flight->dst[0] == 4)
            dk_flight->dst[0] = 2;
    }
    dk_flight->dtab = dkv.pane[dk_flight->dside].list->tab;
    memcpy(&dk_flight->ent, &h.ent, 6);
    dk_state = 1;
    dk_flight->step = 7;
    dk_flight->phase = 0;
    l = &dk_lists[dk_flight->src];
    k = -1;
    found = 0;
    if (dk_flight->idx >= l->n) {
        found = 1;
    } else {
        DkEnt *e = &l->ents[dk_flight->idx];

        if (e->id != h.ent.id)
            found = 1;
        else if (e->fixed != h.ent.fixed)
            found = 1;
    }
    if (found) {
        dk_flight->idx = 0xFFFF;
        for (i = l->n - 1; i >= 0; i--) {
            DkEnt *e = &l->ents[i];

            if (e->id == h.ent.id && e->fixed == h.ent.fixed) {
                dk_flight->idx = i;
                break;
            }
        }
        if (dk_flight->idx < 0)
            DK_FLIGHT_FAIL(0);
    }
    for (i = 0; i < l->count; i++) {
        if (dk_flight->idx == l->order[i]) {
            k = i;
            break;
        }
    }
    dk_flight->pos = k;
    dk_slot_pos(dk_flight->from, dk_flight->side, dk_flight->src, dk_flight->pos, 0);
    dk_flight->cur[0] = dk_flight->from[0];
    dk_flight->cur[1] = dk_flight->from[1];
    dk_flight_take();
    return 1;
}
#endif

#ifdef NON_MATCHING /* mesma alocação do limite/soma de dk_flight_open */
/* moves the card under the cursor straight to its first destination (no window); 0 when it cannot */
int dk_flight_quick(void)
{
    DkHist h;
    DkEnt *e;
    int n;
    int lim;

    dk_flight = heap_alloc(dk_heap, 0x244);
    if (dk_flight == 0)
        return 0;
    memset(dk_flight, 0, 0x244);
    dk_flight->side = dkv.side;
    dk_flight->src = dkv.pane[dk_flight->side].list->tab;
    dk_flight->pos = dk_cur_index(dkv.side);
    dk_flight->idx = dk_lists[dk_flight->src].order[dk_flight->pos];
    e = dk_cur_entry(dkv.side);
    if (e == 0)
        DK_FLIGHT_FAIL(0);
    if (dkv.mode > 1)
        dk_flight->dside = (dk_flight->side + 1) & 1;
    else
        dk_flight->dside = dk_flight->side;
    switch (dk_flight->src) {
    case 0:
    case 1:
    case 2:
        if (e->fixed)
            DK_FLIGHT_ADD(5);
        else
            DK_FLIGHT_ADD(3);
        break;
    case 3:
    case 5:
        if (card_get_attribute_icon(e->id) == 3) {
            if ((unsigned int)dk_lists[1].total >= 0x23)
                DK_FLIGHT_FAIL(0);
            DK_FLIGHT_ADD(1);
        } else {
            if ((unsigned int)dk_lists[0].total > 0x54)
                DK_FLIGHT_FAIL(0);
            DK_FLIGHT_ADD(0);
        }
        n = (unsigned short)dk_list_count_id(&dk_lists[1], e->id, 1);
        n = (unsigned short)(n + dk_list_count_id(&dk_lists[0], e->id, 1));
        if ((unsigned short)(n + dk_list_count_id(&dk_lists[2], e->id, 1)) >= 3)
            DK_FLIGHT_FAIL(0);
        lim = (unsigned short)card_get_limit(e->id);
        n = (unsigned short)dk_list_count_id(&dk_lists[1], e->id, 0);
        n = (unsigned short)(n + dk_list_count_id(&dk_lists[0], e->id, 0));
        if ((unsigned short)(n + dk_list_count_id(&dk_lists[2], e->id, 0)) >= (unsigned short)lim && (unsigned short)lim < 3)
            DK_FLIGHT_FAIL(0);
        break;
    case 4:
        if ((unsigned int)dk_lists[0].total >= 0x55)
            DK_FLIGHT_FAIL(0);
        DK_FLIGHT_ADD(0);
        break;
    }
    dk_flight->dtab = dkv.pane[dk_flight->dside].list->tab;
    memcpy(&dk_flight->ent, e, 6);
    h.src = dk_flight->src;
    h.dst = dk_flight->dst[dk_flight->sel];
    h.pos = dk_dst_pos(h.dst, dk_flight->ent.id);
    memcpy(&h.ent, &dk_flight->ent, 6);
    dk_hist_push(&h);
    dk_state = 1;
    dk_flight->step = 7;
    dk_flight->phase = 0;
    dk_slot_pos(dk_flight->from, dk_flight->side, dk_flight->src, dk_flight->pos, 0);
    dk_flight->cur[0] = dk_flight->from[0];
    dk_flight->cur[1] = dk_flight->from[1];
    dk_flight_take();
    return 1;
}
#endif

/* per frame while a card moves; 1 when the move is over */
int dk_flight_update(PadBits *pad)
{
    DkFlight *f;
    DkEnt e;
    DkList *l;

    switch (dk_flight->step) {
    case 0:
        break;
    case 1:
        dk_flight_take_step();
        break;
    case 2:
        if (menu_item_is_open(&dk_menu, dk_flight->item))
            dk_flight->step = 3;
        break;
    case 3:
        dk_flight_window_input(pad);
        break;
    case 6:
        if (++dk_flight->timer >= dk_flight->frames) {
            f = dk_flight;
            if (dk_slot_pos(f->to, f->dside, f->dst[f->sel], dk_dst_pos(f->dst[f->sel], f->ent.id), 1)) {
                menu_item_close(&dk_menu, dk_flight->item);
                dk_flight->phase = 0;
                dk_flight->shown = 1;
                dk_flight->cur[0] = dk_flight->from[0];
                dk_flight->cur[1] = dk_flight->from[1];
                if (dk_flight->item != 0)
                    dk_flight->step = 5;
                else
                    dk_flight->step = 7;
            }
            dk_flight->timer = 0;
        }
        break;
    case 4:
        if (menu_item_is_open(&dk_menu, dk_flight->item)) {
            l = &dk_lists[dk_flight->src];
            memcpy(&e, &dk_flight->ent, 6);
            e.count = 1;
            dk_list_add(l, &e, dk_flight->pos);
            if (dkv.mode < 2 && dk_flight->dst[dk_flight->sel] != dk_flight->dtab)
                dk_select_tab(dk_flight->dside, dk_flight->dtab, 2);
            dk_flight->step = 8;
        }
        break;
    case 5:
        if (menu_item_is_open(&dk_menu, dk_flight->item)) {
            dk_flight->step = 7;
            dk_flight->shown = 0;
        }
    case 7:
        dk_flight_put_step();
        break;
    case 8:
        dk_flight_last_side = dk_flight->dside;
        dk_flight_last_list = dk_flight->dst[dk_flight->sel];
        dk_flight_free();
        dkv.lock = 0;
        dk_state = 0;
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* o original mascara sel com andi 0xff e soma page*rows antes de top; nenhuma grafia testada reproduziu */
/* the destination window: Left/Right pick the list (the destination pane follows), Up/Down scroll
   that pane by a page, Circle flies the card there (unless that list is full), Cross cancels */
void dk_flight_window_input(PadBits *pad)
{
    DkHist h;
    int sel = (unsigned char)dk_flight->sel;
    DkPane *p;
    unsigned short page;
    unsigned short rows;
    unsigned short top;

    if (pad->trig & 0x80)
        dk_flight->sel = (sel + dk_flight->n - 1) % dk_flight->n;
    else if (pad->trig & 0x20)
        dk_flight->sel = (sel + 1) % dk_flight->n;
    if ((unsigned short)sel != dk_flight->sel) {
        dk_flight_window_pos();
        dk_select_tab(dk_flight->dside, dk_flight->dst[dk_flight->sel], 2);
        se_play(0x6212);
    }
    if (pad->rep & 0x50) {
        p = &dkv.pane[dk_flight->dside];
        page = p->list_mode == 0 ? p->lay.cols : 1;
        rows = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        top = p->top;
        if (pad->rep & 0x10) {
            if (top >= page)
                p->top = top - page;
            else
                p->top = 0;
        } else if ((pad->rep & 0x40) && (int)(page * rows) + top < p->list->count) {
            p->top = top + page;
        }
    }
    if (pad->trig & region_pick_value(0)) {
        if (dk_flight->full[dk_flight->sel] != 0) {
            se_play(0x6215);
        } else {
            h.src = dk_flight->src;
            h.dst = dk_flight->dst[dk_flight->sel];
            h.pos = dk_dst_pos(h.dst, dk_flight->ent.id);
            memcpy(&h.ent, &dk_flight->ent, 6);
            dk_hist_push(&h);
            dk_flight->step = 6;
            dk_flight->frames = 0;
            dk_flight->timer = 0;
            se_play(0x6213);
        }
    } else if (pad->trig & region_pick_value(1)) {
        dk_flight->step = 4;
        menu_item_close(&dk_menu, dk_flight->item);
        se_play(0x6214);
    }
}
#endif

#ifdef NON_MATCHING /* comparações float e ordem dos blocos da trajetória diferentes */
/* flight from the slot to the window (phase 0 path, 1 move, 2 open the window) */
void dk_flight_take_step(void)
{
    float t;
    float u;
    float k;

    switch (dk_flight->phase) {
    case 0:
        dk_slot_pos(dk_flight->from, dk_flight->side, dk_flight->src, dk_flight->pos, 0);
        dk_flight->to[0] = dk_flight->x + 0x10;
        dk_flight->to[1] = dk_flight->y + 0x13;
        dk_flight->cur[0] = dk_flight->from[0];
        dk_flight->cur[1] = dk_flight->from[1];
        if (dk_flight->to[1] < dk_flight->from[1] - 10.0f || dk_flight->from[1] + 10.0f < dk_flight->to[1]) {
            if (dk_flight->from[0] - 10.0f <= dk_flight->to[0] && dk_flight->to[0] <= dk_flight->from[0] + 10.0f) {
                if (dk_flight->from[0] <= 240.0f)
                    dk_flight->mid[0] = dk_flight->from[0] + 120.0f;
                else
                    dk_flight->mid[0] = dk_flight->from[0] - 120.0f;
                dk_flight->mid[1] = dk_flight->from[1] + (dk_flight->to[1] - dk_flight->from[1]) / 2.0f;
            } else {
                dk_flight->mid[0] = dk_flight->from[0];
                dk_flight->mid[1] = 0.0f;
            }
        } else {
            dk_flight->mid[0] = dk_flight->from[0] + (dk_flight->to[0] - dk_flight->from[0]) / 2.0f;
            dk_flight->mid[1] = 24.0f;
        }
        dk_flight_take();
        dk_flight->phase++;
        dk_flight->timer = 0;
        break;
    case 1:
        if (++dk_flight->timer > 10) {
            dk_flight->phase++;
            break;
        }
        t = dk_flight->timer / 10.0f;
        u = 1.0f - t;
        k = t * 2.0f * u;
        dk_flight->cur[0] = t * t * dk_flight->to[0] + u * u * dk_flight->from[0] + k * dk_flight->mid[0];
        dk_flight->cur[1] = t * t * dk_flight->to[1] + u * u * dk_flight->from[1] + k * dk_flight->mid[1];
        if (dk_flight->timer == 5)
            dk_flight->shown = 1;
        break;
    case 2:
        if (dk_flight->dtab != dk_flight->dst[dk_flight->sel])
            dk_select_tab(dk_flight->dside, dk_flight->dst[dk_flight->sel], 2);
        dk_flight->from[0] = dk_flight->x + 0x10;
        dk_flight->from[1] = dk_flight->y + 0x13;
        dk_flight->step = 2;
        dk_flight->shown = 0;
        dk_flight->phase = 0;
        dk_flight->timer = 0;
        break;
    }
}
#endif

#ifdef NON_MATCHING /* comparações float e ordem dos blocos da trajetória diferentes */
/* flight from the window (or the slot) to the slot it lands in (phase 0 path, 1 move, 2 land) */
void dk_flight_put_step(void)
{
    DkFlight *f = dk_flight;
    float t;
    float u;
    float k;
    int pos;
    int keep;
    unsigned short view;

    switch (dk_flight->phase) {
    case 2:
        if (dkv.mode < 2 && dk_flight->dst[dk_flight->sel] != dk_flight->dtab)
            dk_select_tab(dk_flight->dside, dk_flight->dtab, 2);
        dk_flight_put();
        dk_attr_count_update();
        pos = dkv.pane[0].cx + dkv.pane[0].top + dkv.pane[0].cy * (dkv.pane[0].list_mode == 0 ? dkv.pane[0].lay.cols : 1);
        keep = 1;
        if (pos > 0 && pos >= dkv.pane[0].list->count) {
            pos = dkv.pane[0].list->count - 1;
            keep = 0;
        }
        view = dkv.pane[0].list_mode == 0 ? dkv.pane[0].lay.grid_rows : dkv.pane[0].lay.rows;
        dk_layout_scroll(keep, pos, (dkv.pane[0].list_mode == 0 ? dkv.pane[0].lay.cols : 1) & 0xFF | view << 8, dkv.pane[0].list->count, &dkv.pane[0].top, &dkv.pane[0].cx, &dkv.pane[0].cy);
        if (dkv.mode > 1) {
            pos = dkv.pane[1].cx + dkv.pane[1].top + dkv.pane[1].cy * (dkv.pane[1].list_mode == 0 ? dkv.pane[1].lay.cols : 1);
            keep = 1;
            if (pos > 0 && pos >= dkv.pane[1].list->count) {
                pos = dkv.pane[1].list->count - 1;
                keep = 0;
            }
            view = dkv.pane[1].list_mode == 0 ? dkv.pane[1].lay.grid_rows : dkv.pane[1].lay.rows;
            dk_layout_scroll(keep, pos, (dkv.pane[1].list_mode == 0 ? dkv.pane[1].lay.cols : 1) & 0xFF | view << 8, dkv.pane[1].list->count, &dkv.pane[1].top, &dkv.pane[1].cx, &dkv.pane[1].cy);
        }
        dk_flight->step = 8;
        dk_flight->phase = 0;
        dk_flight->timer = 0;
        return;
    case 1:
        if (dk_flight->timer++ < 10) {
            t = dk_flight->timer / 10.0f;
            u = 1.0f - t;
            k = t * 2.0f * u;
            dk_flight->cur[0] = t * t * dk_flight->to[0] + u * u * dk_flight->from[0] + k * dk_flight->mid[0];
            dk_flight->cur[1] = t * t * dk_flight->to[1] + u * u * dk_flight->from[1] + k * dk_flight->mid[1];
        } else {
            dk_flight->phase++;
        }
        return;
    case 0:
        break;
    default:
        return;
    }
    dk_slot_pos(f->to, f->dside, f->dst[f->sel], dk_dst_pos(f->dst[f->sel], f->ent.id), 0);
    if (dk_flight->to[1] < dk_flight->from[1] - 10.0f || dk_flight->from[1] + 10.0f < dk_flight->to[1]) {
        if (dk_flight->to[0] < dk_flight->from[0] - 10.0f || dk_flight->from[0] + 10.0f < dk_flight->to[0]) {
            if (dk_flight->from[1] < dk_flight->to[1]) {
                dk_flight->mid[1] = dk_flight->from[1];
                dk_flight->mid[0] = dk_flight->to[0];
            } else {
                dk_flight->mid[1] = dk_flight->to[1];
                dk_flight->mid[0] = dk_flight->from[0];
            }
        } else {
            if (dk_flight->from[0] <= 240.0f)
                dk_flight->mid[0] = dk_flight->from[0] + 120.0f;
            else
                dk_flight->mid[0] = dk_flight->from[0] - 120.0f;
            dk_flight->mid[1] = dk_flight->from[1] + (dk_flight->to[1] - dk_flight->from[1]) / 2.0f;
        }
    } else {
        if (dk_flight->from[1] <= 136.0f)
            dk_flight->mid[1] = dk_flight->from[1] + 68.0f;
        else
            dk_flight->mid[1] = 24.0f;
        dk_flight->mid[0] = dk_flight->from[0] + (dk_flight->to[0] - dk_flight->from[0]) / 2.0f;
    }
    dk_flight->timer = 0;
    dk_flight->phase++;
}
#endif

void dk_flight_free(void)
{
    menu_remove_item(&dk_menu, dk_flight->item);
    if (dk_flight->item != 0) {
        heap_set_level(dk_heap, dk_flight->item);
        dk_flight->item = 0;
    }
    if (dk_flight != 0)
        heap_set_level(dk_heap, dk_flight);
    dk_flight = 0;
}

/* the card in flight (and the window around it) */
void *dk_flight_draw(void *list)
{
    switch (dk_flight->step) {
    case 1:
    case 7:
    case 5:
        if (dk_flight->shown)
            list = dk_flight_draw_window(list, 0);
        list = dk_draw_card_pict(list, dk_flight->cur[0] - 11, dk_flight->cur[1] - 15, 0, 0xFFFFFFFF, &dk_flight->ent,
                                 dk_res_get(0x8001), dk_res_get(0x8002), dk_res_get(0x8032));
        break;
    case 8:
        break;
    default:
        list = dk_flight_draw_window(list, 1);
        break;
    }
    return list;
}

/* the destination window, with the card inside it when card is set */
void *dk_flight_draw_window(void *list, int card)
{
    unsigned int color = 0xFFFFFFFF;
    unsigned int st;

    list = dk_draw_shade(list, dk_flight->dside, dk_flight->dst[dk_flight->sel], 0);
    menu_layout(&list, &dk_menu);
    switch (dk_flight->step) {
    case 2:
        st = dk_flight_window_state();
        if (st < 3)
            return list;
        if (st == 3)
            color = 0x80FFFFFF;
        break;
    case 5:
    case 4:
        if (dk_flight_window_state() < 2) {
            color = 0x80FFFFFF;
            break;
        }
        return list;
    case 6:
    case 3:
        break;
    default:
        return list;
    }
    if (card != 0)
        list = dk_draw_card_pict(list, dk_flight->x + 5, dk_flight->y + 4, 0, color, &dk_flight->ent,
                                 dk_res_get(0x8001), dk_res_get(0x8002), dk_res_get(0x8032));
    return list;
}

/* window above the tab of the destination list */
void dk_flight_window_pos(void)
{
    DkLayout *lay = &dkv.pane[dk_flight->dside].lay;
    unsigned int tp = func_0880B454(dk_flight->dst[dk_flight->sel]);
    unsigned short off = tp;
    unsigned short len = tp >> 16;

    if (lay->w < off + 0xE0)
        dk_flight->x = lay->x + lay->w - 0xE0;
    else
        dk_flight->x = lay->x + off;
    dk_flight->w = (unsigned int)len / 2 + (lay->x + off) - dk_flight->x;
    dk_flight->y = 0x2A;
    if (dk_flight->item != 0) {
        dk_flight->item->x = dk_flight->x;
        dk_flight->item->y = dk_flight->y;
        dk_flight->item->tab = 4;
        dk_flight->item->tab_x = dk_flight->w;
        dk_flight_text();
    }
}

/* the card leaves its list */
void dk_flight_take(void)
{
    dk_list_remove(&dk_lists[dk_flight->src], dk_flight->idx);
}

/* the card lands in its destination list */
void dk_flight_put(void)
{
    DkList *l = &dk_lists[dk_flight->dst[dk_flight->sel]];
    DkEnt e;

    memcpy(&e, &dk_flight->ent, 6);
    e.count = 1;
    e.marked = 1;
    dk_list_add(l, &e, -1);
}

/* state of the destination window in dk_menu */
unsigned int dk_flight_window_state(void)
{
    MenuItem **items = dk_menu.items;
    int i;

    for (i = 0; i < 0x10; i++) {
        if (items[i]->def == dk_flight->item)
            return items[i]->counter;
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores temporários da conta da grade (o original mantém scroll em t0) */
/* screen position of entry pos of list tab in pane side (card center, +11/+15); when the entry is
   scrolled out and scroll is set, scrolls the pane one step toward it and returns 0; tabs not shown
   in the pane give the position of their tab */
int dk_slot_pos(float *out, int side, unsigned int tab, int pos, int scroll)
{
    DkPane *p = &dkv.pane[side];
    DkLayout *lay = &p->lay;
    DkList *l = &dk_lists[tab];
    int off = 0;
    unsigned int mode;
    int count;
    int step;
    int frames;
    unsigned short top;
    unsigned int tp;
    unsigned short len;
    unsigned short x;
    int d;
    int cols;

    if (dkv.pane[side].list->tab != tab)
        off = 1;
    mode = p->list_mode;
    switch (mode) {
    case 0:
        step = lay->cols;
        count = step * lay->grid_rows;
        frames = 6;
        break;
    case 1:
        count = lay->rows;
        step = 1;
        frames = 1;
        break;
    }
    if (!off) {
        top = p->top;
        if (pos < top) {
            if (scroll == 0) {
                off = 1;
            } else if (l->tab == 3) {
                off = 1;
            } else {
                p->top = top - (unsigned short)step;
                dk_flight->frames = frames;
                return 0;
            }
        } else if (top + count <= pos) {
            if (scroll == 0) {
                off = 1;
            } else if (l->tab == 3) {
                off = 1;
            } else {
                p->top = top + (unsigned short)step;
                dk_flight->frames = frames;
                return 0;
            }
        }
    }
    if (off) {
        tp = func_0880B454(tab);
        len = tp >> 16;
        x = tp;
        out[0] = (int)((unsigned int)len / 2 + (lay->x + x));
        out[1] = 32.0f;
        return 1;
    }
    switch (mode) {
    case 0:
        cols = lay->cols;
        d = pos - p->top;
        out[0] = (int)(lay->x + lay->grid_x + (d % cols) * (unsigned short)lay->cell_w);
        out[1] = (int)(lay->y + lay->grid_y + (d / cols) * (unsigned short)lay->cell_h);
        break;
    case 1:
        out[0] = lay->x + lay->row_x;
        out[1] = (int)(lay->y + lay->row_y + (unsigned short)lay->row_h * (pos - p->top));
        if (dkv.mode < 4 && side == 1)
            out[1] += 1.0f;
        break;
    }
    out[0] += 11.0f;
    out[1] += 15.0f;
    return 1;
}
#endif

/* position the card will take in the visible order of list tab (the trunk merges copies) */
int dk_dst_pos(int tab, int id)
{
    DkList *l = &dk_lists[tab];
    int i;
    int k;

    switch (tab) {
    case 3:
        i = dk_list_find(l, id);
        if (i >= 0) {
            for (k = 0; k < l->count; k++) {
                if (i == l->order[k])
                    return k;
            }
        }
        break;
    }
    return l->count;
}

/* window text: "move to <list>" or "<list> is full" */
void dk_flight_text(void)
{
    unsigned short *name;

    if (dk_flight == 0)
        return;
    if (dk_flight->full[dk_flight->sel] != 0) {
        wcscpy(dk_flight->text, table_get_entry(dk_res_get(0x8033), 0x84));
    } else {
        name = dk_list_name(dk_flight->dst[dk_flight->sel]);
        wsprintf(dk_flight->text, table_get_entry(dk_res_get(0x8033), 0x83), name);
    }
}
