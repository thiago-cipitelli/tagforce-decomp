// @range 0x0880CDC8-0x088100AC
// @asm 0x0880CDC8-0x0880D96C
// @asm 0x0880D96C-0x0880DE54
// @asm 0x0880DE54-0x0880E558
// @asm 0x0880E940-0x0880ECCC
// @asm 0x0880ECCC-0x0880F128
// @asm 0x0880F128-0x0880F6E8
// @asm 0x0880FC74-0x0880FF74
// @rodata 0x08820FCC-0x08820FF0
/* Moving a card between the lists (Circle in the deck editor, undo with Triangle): the destination
   window offering the lists the card can go to (main, side, extra, trunk) with the full ones greyed
   out, the card flown along a curve from its slot to the slot it lands in, the result checks of the
   deck limits, and the slot positions on screen. */
#include "libdeck_tutorial.h"

unsigned short dt_cur_index(unsigned short side);
DtEnt *dt_cur_entry(unsigned short side);
void dt_select_tab(unsigned short side, unsigned short tab, unsigned short how);
unsigned short *dt_list_name(unsigned short tab);
void *dt_eres_get(unsigned short id);
int dt_list_find(DtList *l, int id);
int dt_list_count_id(DtList *l, unsigned short id, int same_name);
int dt_list_remove(DtList *l, int i);
void dt_list_add(DtList *l, DtEnt *src, int pos);
void dt_list_total(DtList *l);
void dt_attr_count_update(void);

typedef struct DtHist {                /* undo record (dt_hist_push), 10 bytes */
    unsigned char src : 4;             /* source list */
    unsigned char dst : 4;             /* destination list */
    unsigned char pad01;
    DtEnt ent;                         /* +0x02 */
    unsigned short pos;                /* +0x08 position in the destination order */
} DtHist;

typedef struct DtFlight {              /* dt_flight 0x08823DD4, 0x244 bytes, heap allocated */
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
    DtEnt ent;                         /* +0x01C card being moved */
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
} DtFlight;

extern DtFlight *dt_flight;            /* 0x08823DD4 */
extern unsigned short dt_flight_last_side; /* 0x08823954 destination of the last move */
extern unsigned short dt_flight_last_list; /* 0x08823956 */

int sysflag_test(unsigned int bit);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
unsigned short dt_deck_count_forbidden(void);
void dt_hist_push(DtHist *h);
int dt_hist_pop(DtHist *h);
void dt_layout_scroll(int keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
unsigned int dt_tab_pos(unsigned short tab);
void *dt_draw_shade(void *list, unsigned short side, unsigned short tab, int lit);
void *dt_draw_card_pict(void *list, int x, int y, int z, unsigned int color, DtEnt *e, void *gim, void *gim_alt, void *alt);

void dt_flight_land(void);
void dt_flight_window_input(PadBits *pad);
void dt_flight_take_step(void);
void dt_flight_put_step(void);
void dt_flight_free(void);
void *dt_flight_draw_window(void *list, int card);
void dt_flight_window_pos(void);
void dt_flight_take(void);
void dt_flight_put(void);
unsigned int dt_flight_window_state(void);
void dt_flight_text(void);

/* adds destination k to the window */
#define DK_FLIGHT_ADD(k) (dt_flight->dst[dt_flight->n++] = (k))
/* the next destination offered cannot take the card */
#define DK_FLIGHT_FULL() (dt_flight->full[dt_flight->n] = 1)
/* gives the move up with message r */
#define DK_FLIGHT_FAIL(r)                       \
    do {                                        \
        if (dt_flight != 0) {                   \
            heap_set_level(dts.heap, dt_flight); \
            dt_flight = 0;                      \
        }                                       \
        return r;                               \
    } while (0)

#ifdef NON_MATCHING /* cópia de dk_flight_open (libdeck, mesma alocação de $s diferente) mais o contador da lição no fim */
/* Circle on a card: opens the destination window (0), or returns why it cannot move (2 nothing to
   move to, 4 three copies already, 5 forbidden cards, 6/7 limited / semi-limited, 8 no memory) */
int dt_flight_open(void)
{
    DtEnt *e;
    int i;
    MenuItemDef *it;
    int lim;
    int n;

    dt_flight = heap_alloc(dts.heap, 0x244);
    if (dt_flight == 0)
        return 8;
    memset(dt_flight, 0, 0x244);
    dt_flight->side = dtv.side;
    dt_flight->src = dtv.pane[dt_flight->side].list->tab;
    dt_flight->pos = dt_cur_index(dtv.side);
    dt_flight->idx = dts.list[dt_flight->src].order[dt_flight->pos];
    e = dt_cur_entry(dtv.side);
    if (e == 0)
        DK_FLIGHT_FAIL(0);
    if (dtv.mode > 1)
        dt_flight->dside = (dt_flight->side + 1) & 1;
    else
        dt_flight->dside = dt_flight->side;
    switch (dt_flight->src) {
    case 0:
        if (e->fixed)
            DK_FLIGHT_ADD(5);
        else
            DK_FLIGHT_ADD(3);
        if ((unsigned int)dts.list[4].total >= 20)
            DK_FLIGHT_FULL();
        if (dtv.mode > 1)
            DK_FLIGHT_ADD(4);
        else
            DK_FLIGHT_ADD(2);
        break;
    case 2:
        if (e->fixed)
            DK_FLIGHT_ADD(5);
        else
            DK_FLIGHT_ADD(3);
        if (dtv.mode < 2 && (unsigned int)dts.list[0].total < 0x55)
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
            if ((unsigned int)dts.list[1].total >= 0x23)
                DK_FLIGHT_FULL();
            DK_FLIGHT_ADD(1);
        } else {
            if ((unsigned int)dts.list[0].total >= 0x55)
                DK_FLIGHT_FULL();
            DK_FLIGHT_ADD(0);
            if ((unsigned int)dts.list[2].total >= 20)
                DK_FLIGHT_FULL();
            DK_FLIGHT_ADD(2);
        }
        n = (unsigned short)dt_list_count_id(&dts.list[1], e->id, 1);
        n = (unsigned short)(n + dt_list_count_id(&dts.list[0], e->id, 1));
        if ((unsigned short)(n + dt_list_count_id(&dts.list[2], e->id, 1)) >= 3)
            DK_FLIGHT_FAIL(4);
        lim = (unsigned short)card_get_limit(e->id);
        n = (unsigned short)dt_list_count_id(&dts.list[1], e->id, 0);
        n = (unsigned short)(n + dt_list_count_id(&dts.list[0], e->id, 0));
        if ((unsigned short)(n + dt_list_count_id(&dts.list[2], e->id, 0)) >= (unsigned short)lim) {
            switch ((unsigned short)lim) {
            case 0:
                lim = sysflag_test(0) != 0;
                if (dt_deck_count_forbidden() >= lim)
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
        if ((unsigned int)dts.list[0].total >= 0x55)
            DK_FLIGHT_FULL();
        DK_FLIGHT_ADD(0);
        break;
    }
    memcpy(&dt_flight->ent, e, 6);
    if (dt_flight->n <= 0)
        DK_FLIGHT_FAIL(2);
    if (dt_flight_last_side == dt_flight->dside) {
        for (i = 0; i < dt_flight->n; i++) {
            if (dt_flight->dst[i] == dt_flight_last_list) {
                dt_flight->sel = i;
                break;
            }
        }
    }
    dtv.lock = 1;
    dt_flight_text();
    dt_flight_window_pos();
    dt_flight->dtab = dtv.pane[dt_flight->dside].list->tab;
    dt_flight->item = heap_alloc(dts.heap, 0x78);
    it = dt_flight->item;
    if (it == 0)
        DK_FLIGHT_FAIL(8);
    memset(it, 0, 0x78);
    it->visible = 1;
    it->style = 2;
    it->x = dt_flight->x;
    it->y = dt_flight->y;
    it->w = 0xE0;
    it->h = 0x27;
    it->color = 0xFFFFFFFF;
    if (dt_flight->w <= 0x70)
        it->tab = 3;
    else
        it->tab = 4;
    it->tab_x = dt_flight->w;
    it->text = dt_flight->text;
    it->font_flag4 = 1;
    it->pad_l = 0x1E;
    it->pad_t = 4;
    it->font_size = 0xC;
    it->text_color = 0xFFFFFFFF;
    it->open_anim = 1;
    it->close_anim = 2;
    menu_add_item(&dt_menu, it);
    menu_item_open(&dt_menu, it);
    dts.state = 1;
    dt_flight->cur[1] = -100;
    dt_flight->step = 1;
    if (dt.step == 10 && dt.goal == 3)   /* lesson: a move was opened */
        dt.done++;
    return 0;
}
#endif

/* pane showing list tab in the two-pane layouts: the deck lists left, the trunk right */
static inline int dt_tab_side(int tab)
{
    if (tab < 3)
        return 0;
    return 1;
}

#ifdef NON_MATCHING /* o original calcula o lado (src >= 3) com desvio; o mwcc dobra em slti/xori, e os temporários mudam */
/* Triangle: takes the last move back (the card flies from where it landed to its old list) */
int dt_flight_undo(void)
{
    DtHist h;
    int found;
    DtList *l;
    int i;
    int k;

    if (dt_hist_pop(&h) == 0)
        return 0;
    dt_flight = heap_alloc(dts.heap, 0x244);
    if (dt_flight == 0)
        return 0;
    memset(dt_flight, 0, 0x244);
    dt_flight->src = h.dst;
    dt_flight->idx = h.pos;
    DK_FLIGHT_ADD(h.src);
    if (dtv.mode > 1) {
        if (dt_flight->src == 2 && dt_flight->dst[0] < 2)
            dt_flight->src = 4;
        if (dt_flight->dst[0] == 2)
            dt_flight->dst[0] = 4;
        dt_flight->side = dt_tab_side(dt_flight->src);
        dt_flight->dside = (dt_flight->side + 1) & 1;
    } else {
        dt_flight->side = 0;
        dt_flight->dside = 0;
        if (dt_flight->src == 4)
            dt_flight->src = 2;
        if (dt_flight->dst[0] == 4)
            dt_flight->dst[0] = 2;
    }
    dt_flight->dtab = dtv.pane[dt_flight->dside].list->tab;
    memcpy(&dt_flight->ent, &h.ent, 6);
    dts.state = 1;
    dt_flight->step = 7;
    dt_flight->phase = 0;
    l = &dts.list[dt_flight->src];
    k = -1;
    found = 0;
    if (dt_flight->idx >= l->n) {
        found = 1;
    } else {
        DtEnt *e = &l->ents[dt_flight->idx];

        if (e->id != h.ent.id)
            found = 1;
        else if (e->fixed != h.ent.fixed)
            found = 1;
    }
    if (found) {
        dt_flight->idx = 0xFFFF;
        for (i = l->n - 1; i >= 0; i--) {
            DtEnt *e = &l->ents[i];

            if (e->id == h.ent.id && e->fixed == h.ent.fixed) {
                dt_flight->idx = i;
                break;
            }
        }
        if (dt_flight->idx < 0)
            DK_FLIGHT_FAIL(0);
    }
    for (i = 0; i < l->count; i++) {
        if (dt_flight->idx == l->order[i]) {
            k = i;
            break;
        }
    }
    dt_flight->pos = k;
    dt_slot_pos(dt_flight->from, dt_flight->side, dt_flight->src, dt_flight->pos, 0);
    dt_flight->cur[0] = dt_flight->from[0];
    dt_flight->cur[1] = dt_flight->from[1];
    dt_flight_take();
    return 1;
}
#endif

#ifdef NON_MATCHING /* mesma alocação do limite/soma de dt_flight_open */
/* moves the card under the cursor straight to its first destination (no window); 0 when it cannot */
int dt_flight_quick(void)
{
    DtHist h;
    DtEnt *e;
    int n;
    int lim;

    dt_flight = heap_alloc(dts.heap, 0x244);
    if (dt_flight == 0)
        return 0;
    memset(dt_flight, 0, 0x244);
    dt_flight->side = dtv.side;
    dt_flight->src = dtv.pane[dt_flight->side].list->tab;
    dt_flight->pos = dt_cur_index(dtv.side);
    dt_flight->idx = dts.list[dt_flight->src].order[dt_flight->pos];
    e = dt_cur_entry(dtv.side);
    if (e == 0)
        DK_FLIGHT_FAIL(0);
    if (dtv.mode > 1)
        dt_flight->dside = (dt_flight->side + 1) & 1;
    else
        dt_flight->dside = dt_flight->side;
    switch (dt_flight->src) {
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
            if ((unsigned int)dts.list[1].total >= 0x23)
                DK_FLIGHT_FAIL(0);
            DK_FLIGHT_ADD(1);
        } else {
            if ((unsigned int)dts.list[0].total > 0x54)
                DK_FLIGHT_FAIL(0);
            DK_FLIGHT_ADD(0);
        }
        n = (unsigned short)dt_list_count_id(&dts.list[1], e->id, 1);
        n = (unsigned short)(n + dt_list_count_id(&dts.list[0], e->id, 1));
        if ((unsigned short)(n + dt_list_count_id(&dts.list[2], e->id, 1)) >= 3)
            DK_FLIGHT_FAIL(0);
        lim = (unsigned short)card_get_limit(e->id);
        n = (unsigned short)dt_list_count_id(&dts.list[1], e->id, 0);
        n = (unsigned short)(n + dt_list_count_id(&dts.list[0], e->id, 0));
        if ((unsigned short)(n + dt_list_count_id(&dts.list[2], e->id, 0)) >= (unsigned short)lim && (unsigned short)lim < 3)
            DK_FLIGHT_FAIL(0);
        break;
    case 4:
        if ((unsigned int)dts.list[0].total >= 0x55)
            DK_FLIGHT_FAIL(0);
        DK_FLIGHT_ADD(0);
        break;
    }
    dt_flight->dtab = dtv.pane[dt_flight->dside].list->tab;
    memcpy(&dt_flight->ent, e, 6);
    h.src = dt_flight->src;
    h.dst = dt_flight->dst[dt_flight->sel];
    h.pos = dt_dst_pos(h.dst, dt_flight->ent.id);
    memcpy(&h.ent, &dt_flight->ent, 6);
    dt_hist_push(&h);
    dts.state = 1;
    dt_flight->step = 7;
    dt_flight->phase = 0;
    dt_slot_pos(dt_flight->from, dt_flight->side, dt_flight->src, dt_flight->pos, 0);
    dt_flight->cur[0] = dt_flight->from[0];
    dt_flight->cur[1] = dt_flight->from[1];
    dt_flight_take();
    return 1;
}
#endif

/* per frame while a card moves; 1 when the move is over */
int dt_flight_update(PadBits *pad)
{
    DtFlight *f;
    DtEnt e;
    DtList *l;

    switch (dt_flight->step) {
    case 0:
        break;
    case 1:
        dt_flight_take_step();
        break;
    case 2:
        if (menu_item_is_open(&dt_menu, dt_flight->item))
            dt_flight->step = 3;
        break;
    case 3:
        dt_flight_window_input(pad);
        break;
    case 6:
        if (++dt_flight->timer >= dt_flight->frames) {
            f = dt_flight;
            if (dt_slot_pos(f->to, f->dside, f->dst[f->sel], dt_dst_pos(f->dst[f->sel], f->ent.id), 1)) {
                menu_item_close(&dt_menu, dt_flight->item);
                dt_flight->phase = 0;
                dt_flight->shown = 1;
                dt_flight->cur[0] = dt_flight->from[0];
                dt_flight->cur[1] = dt_flight->from[1];
                if (dt_flight->item != 0)
                    dt_flight->step = 5;
                else
                    dt_flight->step = 7;
            }
            dt_flight->timer = 0;
        }
        break;
    case 4:
        if (menu_item_is_open(&dt_menu, dt_flight->item)) {
            l = &dts.list[dt_flight->src];
            memcpy(&e, &dt_flight->ent, 6);
            e.count = 1;
            dt_list_add(l, &e, dt_flight->pos);
            if (dtv.mode < 2 && dt_flight->dst[dt_flight->sel] != dt_flight->dtab)
                dt_select_tab(dt_flight->dside, dt_flight->dtab, 2);
            dt_flight->step = 8;
        }
        break;
    case 5:
        if (menu_item_is_open(&dt_menu, dt_flight->item)) {
            dt_flight->step = 7;
            dt_flight->shown = 0;
        }
    case 7:
        dt_flight_put_step();
        break;
    case 8:
        dt_flight_last_side = dt_flight->dside;
        dt_flight_last_list = dt_flight->dst[dt_flight->sel];
        dt_flight_free();
        dtv.lock = 0;
        dts.state = 0;
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* o original mascara sel com andi 0xff e soma page*rows antes de top; nenhuma grafia testada reproduziu */
/* the destination window: Left/Right pick the list (the destination pane follows), Up/Down scroll
   that pane by a page, Circle flies the card there (unless that list is full), Cross cancels */
void dt_flight_window_input(PadBits *pad)
{
    DtHist h;
    int sel = (unsigned char)dt_flight->sel;
    DtPane *p;
    unsigned short page;
    unsigned short rows;
    unsigned short top;

    if (pad->trig & 0x80)
        dt_flight->sel = (sel + dt_flight->n - 1) % dt_flight->n;
    else if (pad->trig & 0x20)
        dt_flight->sel = (sel + 1) % dt_flight->n;
    if ((unsigned short)sel != dt_flight->sel) {
        dt_flight_window_pos();
        dt_select_tab(dt_flight->dside, dt_flight->dst[dt_flight->sel], 2);
        se_play(0x6212);
    }
    if (pad->rep & 0x50) {
        p = &dtv.pane[dt_flight->dside];
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
        if (dt_flight->full[dt_flight->sel] != 0) {
            se_play(0x6215);
        } else {
            h.src = dt_flight->src;
            h.dst = dt_flight->dst[dt_flight->sel];
            h.pos = dt_dst_pos(h.dst, dt_flight->ent.id);
            memcpy(&h.ent, &dt_flight->ent, 6);
            dt_hist_push(&h);
            dt_flight->step = 6;
            dt_flight->frames = 0;
            dt_flight->timer = 0;
            se_play(0x6213);
        }
    } else if (pad->trig & region_pick_value(1)) {
        dt_flight->step = 4;
        menu_item_close(&dt_menu, dt_flight->item);
        se_play(0x6214);
    }
}
#endif

#ifdef NON_MATCHING /* comparações float e ordem dos blocos da trajetória diferentes */
/* flight from the slot to the window (phase 0 path, 1 move, 2 open the window) */
void dt_flight_take_step(void)
{
    float t;
    float u;
    float k;

    switch (dt_flight->phase) {
    case 0:
        dt_slot_pos(dt_flight->from, dt_flight->side, dt_flight->src, dt_flight->pos, 0);
        dt_flight->to[0] = dt_flight->x + 0x10;
        dt_flight->to[1] = dt_flight->y + 0x13;
        dt_flight->cur[0] = dt_flight->from[0];
        dt_flight->cur[1] = dt_flight->from[1];
        if (dt_flight->to[1] < dt_flight->from[1] - 10.0f || dt_flight->from[1] + 10.0f < dt_flight->to[1]) {
            if (dt_flight->from[0] - 10.0f <= dt_flight->to[0] && dt_flight->to[0] <= dt_flight->from[0] + 10.0f) {
                if (dt_flight->from[0] <= 240.0f)
                    dt_flight->mid[0] = dt_flight->from[0] + 120.0f;
                else
                    dt_flight->mid[0] = dt_flight->from[0] - 120.0f;
                dt_flight->mid[1] = dt_flight->from[1] + (dt_flight->to[1] - dt_flight->from[1]) / 2.0f;
            } else {
                dt_flight->mid[0] = dt_flight->from[0];
                dt_flight->mid[1] = 0.0f;
            }
        } else {
            dt_flight->mid[0] = dt_flight->from[0] + (dt_flight->to[0] - dt_flight->from[0]) / 2.0f;
            dt_flight->mid[1] = 24.0f;
        }
        dt_flight_take();
        dt_flight->phase++;
        dt_flight->timer = 0;
        break;
    case 1:
        if (++dt_flight->timer > 10) {
            dt_flight->phase++;
            break;
        }
        t = dt_flight->timer / 10.0f;
        u = 1.0f - t;
        k = t * 2.0f * u;
        dt_flight->cur[0] = t * t * dt_flight->to[0] + u * u * dt_flight->from[0] + k * dt_flight->mid[0];
        dt_flight->cur[1] = t * t * dt_flight->to[1] + u * u * dt_flight->from[1] + k * dt_flight->mid[1];
        if (dt_flight->timer == 5)
            dt_flight->shown = 1;
        break;
    case 2:
        if (dt_flight->dtab != dt_flight->dst[dt_flight->sel])
            dt_select_tab(dt_flight->dside, dt_flight->dst[dt_flight->sel], 2);
        dt_flight->from[0] = dt_flight->x + 0x10;
        dt_flight->from[1] = dt_flight->y + 0x13;
        dt_flight->step = 2;
        dt_flight->shown = 0;
        dt_flight->phase = 0;
        dt_flight->timer = 0;
        break;
    }
}
#endif

#ifdef NON_MATCHING /* comparações float e ordem dos blocos da trajetória diferentes */
/* flight from the window (or the slot) to the slot it lands in (phase 0 path, 1 move, 2 land) */
void dt_flight_put_step(void)
{
    DtFlight *f = dt_flight;
    float t;
    float u;
    float k;
    int pos;
    int keep;
    unsigned short view;

    switch (dt_flight->phase) {
    case 2:
        if (dtv.mode < 2 && dt_flight->dst[dt_flight->sel] != dt_flight->dtab)
            dt_select_tab(dt_flight->dside, dt_flight->dtab, 2);
        dt_flight_put();
        dt_attr_count_update();
        pos = dtv.pane[0].cx + dtv.pane[0].top + dtv.pane[0].cy * (dtv.pane[0].list_mode == 0 ? dtv.pane[0].lay.cols : 1);
        keep = 1;
        if (pos > 0 && pos >= dtv.pane[0].list->count) {
            pos = dtv.pane[0].list->count - 1;
            keep = 0;
        }
        view = dtv.pane[0].list_mode == 0 ? dtv.pane[0].lay.grid_rows : dtv.pane[0].lay.rows;
        dt_layout_scroll(keep, pos, (dtv.pane[0].list_mode == 0 ? dtv.pane[0].lay.cols : 1) & 0xFF | view << 8, dtv.pane[0].list->count, &dtv.pane[0].top, &dtv.pane[0].cx, &dtv.pane[0].cy);
        if (dtv.mode > 1) {
            pos = dtv.pane[1].cx + dtv.pane[1].top + dtv.pane[1].cy * (dtv.pane[1].list_mode == 0 ? dtv.pane[1].lay.cols : 1);
            keep = 1;
            if (pos > 0 && pos >= dtv.pane[1].list->count) {
                pos = dtv.pane[1].list->count - 1;
                keep = 0;
            }
            view = dtv.pane[1].list_mode == 0 ? dtv.pane[1].lay.grid_rows : dtv.pane[1].lay.rows;
            dt_layout_scroll(keep, pos, (dtv.pane[1].list_mode == 0 ? dtv.pane[1].lay.cols : 1) & 0xFF | view << 8, dtv.pane[1].list->count, &dtv.pane[1].top, &dtv.pane[1].cx, &dtv.pane[1].cy);
        }
        dt_flight->step = 8;
        dt_flight->phase = 0;
        dt_flight->timer = 0;
        return;
    case 1:
        if (dt_flight->timer++ < 10) {
            t = dt_flight->timer / 10.0f;
            u = 1.0f - t;
            k = t * 2.0f * u;
            dt_flight->cur[0] = t * t * dt_flight->to[0] + u * u * dt_flight->from[0] + k * dt_flight->mid[0];
            dt_flight->cur[1] = t * t * dt_flight->to[1] + u * u * dt_flight->from[1] + k * dt_flight->mid[1];
        } else {
            dt_flight->phase++;
        }
        return;
    case 0:
        break;
    default:
        return;
    }
    dt_slot_pos(f->to, f->dside, f->dst[f->sel], dt_dst_pos(f->dst[f->sel], f->ent.id), 0);
    if (dt_flight->to[1] < dt_flight->from[1] - 10.0f || dt_flight->from[1] + 10.0f < dt_flight->to[1]) {
        if (dt_flight->to[0] < dt_flight->from[0] - 10.0f || dt_flight->from[0] + 10.0f < dt_flight->to[0]) {
            if (dt_flight->from[1] < dt_flight->to[1]) {
                dt_flight->mid[1] = dt_flight->from[1];
                dt_flight->mid[0] = dt_flight->to[0];
            } else {
                dt_flight->mid[1] = dt_flight->to[1];
                dt_flight->mid[0] = dt_flight->from[0];
            }
        } else {
            if (dt_flight->from[0] <= 240.0f)
                dt_flight->mid[0] = dt_flight->from[0] + 120.0f;
            else
                dt_flight->mid[0] = dt_flight->from[0] - 120.0f;
            dt_flight->mid[1] = dt_flight->from[1] + (dt_flight->to[1] - dt_flight->from[1]) / 2.0f;
        }
    } else {
        if (dt_flight->from[1] <= 136.0f)
            dt_flight->mid[1] = dt_flight->from[1] + 68.0f;
        else
            dt_flight->mid[1] = 24.0f;
        dt_flight->mid[0] = dt_flight->from[0] + (dt_flight->to[0] - dt_flight->from[0]) / 2.0f;
    }
    dt_flight->timer = 0;
    dt_flight->phase++;
    if (dt.step == 10 && dt.goal == 4)   /* lesson: a card was put */
        dt.done++;
}
#endif

void dt_flight_free(void)
{
    menu_remove_item(&dt_menu, dt_flight->item);
    if (dt_flight->item != 0) {
        heap_set_level(dts.heap, dt_flight->item);
        dt_flight->item = 0;
    }
    if (dt_flight != 0)
        heap_set_level(dts.heap, dt_flight);
    dt_flight = 0;
}

/* the card in flight (and the window around it) */
void *dt_flight_draw(void *list)
{
    switch (dt_flight->step) {
    case 1:
    case 7:
    case 5:
        if (dt_flight->shown)
            list = dt_flight_draw_window(list, 0);
        list = dt_draw_card_pict(list, dt_flight->cur[0] - 11, dt_flight->cur[1] - 15, 0, 0xFFFFFFFF, &dt_flight->ent,
                                 dt_eres_get(0x8001), dt_eres_get(0x8002), dt_eres_get(0x8032));
        break;
    case 8:
        break;
    default:
        list = dt_flight_draw_window(list, 1);
        break;
    }
    return list;
}

/* the destination window, with the card inside it when card is set */
void *dt_flight_draw_window(void *list, int card)
{
    unsigned int color = 0xFFFFFFFF;
    unsigned int st;

    list = dt_draw_shade(list, dt_flight->dside, dt_flight->dst[dt_flight->sel], 0);
    menu_layout(&list, &dt_menu);
    switch (dt_flight->step) {
    case 2:
        st = dt_flight_window_state();
        if (st < 3)
            return list;
        if (st == 3)
            color = 0x80FFFFFF;
        break;
    case 5:
    case 4:
        if (dt_flight_window_state() < 2) {
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
        list = dt_draw_card_pict(list, dt_flight->x + 5, dt_flight->y + 4, 0, color, &dt_flight->ent,
                                 dt_eres_get(0x8001), dt_eres_get(0x8002), dt_eres_get(0x8032));
    return list;
}

/* window above the tab of the destination list */
void dt_flight_window_pos(void)
{
    DtLayout *lay = &dtv.pane[dt_flight->dside].lay;
    unsigned int tp = dt_tab_pos(dt_flight->dst[dt_flight->sel]);
    unsigned short off = tp;
    unsigned short len = tp >> 16;

    if (lay->w < off + 0xE0)
        dt_flight->x = lay->x + lay->w - 0xE0;
    else
        dt_flight->x = lay->x + off;
    dt_flight->w = (unsigned int)len / 2 + (lay->x + off) - dt_flight->x;
    dt_flight->y = 0x2A;
    if (dt_flight->item != 0) {
        dt_flight->item->x = dt_flight->x;
        dt_flight->item->y = dt_flight->y;
        dt_flight->item->tab = 4;
        dt_flight->item->tab_x = dt_flight->w;
        dt_flight_text();
    }
}

/* the card leaves its list */
void dt_flight_take(void)
{
    dt_list_remove(&dts.list[dt_flight->src], dt_flight->idx);
}

/* the card lands in its destination list */
void dt_flight_put(void)
{
    DtList *l = &dts.list[dt_flight->dst[dt_flight->sel]];
    DtEnt e;

    memcpy(&e, &dt_flight->ent, 6);
    e.count = 1;
    e.marked = 1;
    dt_list_add(l, &e, -1);
}

/* state of the destination window in dt_menu */
unsigned int dt_flight_window_state(void)
{
    MenuItem **items = dt_menu.items;
    int i;

    for (i = 0; i < 0x10; i++) {
        if (items[i]->def == dt_flight->item)
            return items[i]->counter;
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores temporários da conta da grade (o original mantém scroll em t0) */
/* screen position of entry pos of list tab in pane side (card center, +11/+15); when the entry is
   scrolled out and scroll is set, scrolls the pane one step toward it and returns 0; tabs not shown
   in the pane give the position of their tab */
int dt_slot_pos(float *out, int side, unsigned int tab, int pos, int scroll)
{
    DtPane *p = &dtv.pane[side];
    DtLayout *lay = &p->lay;
    DtList *l = &dts.list[tab];
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

    if (dtv.pane[side].list->tab != tab)
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
                dt_flight->frames = frames;
                return 0;
            }
        } else if (top + count <= pos) {
            if (scroll == 0) {
                off = 1;
            } else if (l->tab == 3) {
                off = 1;
            } else {
                p->top = top + (unsigned short)step;
                dt_flight->frames = frames;
                return 0;
            }
        }
    }
    if (off) {
        tp = dt_tab_pos(tab);
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
        if (dtv.mode < 4 && side == 1)
            out[1] += 1.0f;
        break;
    }
    out[0] += 11.0f;
    out[1] += 15.0f;
    return 1;
}
#endif

/* position the card will take in the visible order of list tab (the trunk merges copies) */
int dt_dst_pos(int tab, int id)
{
    DtList *l = &dts.list[tab];
    int i;
    int k;

    switch (tab) {
    case 3:
        i = dt_list_find(l, id);
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
void dt_flight_text(void)
{
    unsigned short *name;

    if (dt_flight == 0)
        return;
    if (dt_flight->full[dt_flight->sel] != 0) {
        wcscpy(dt_flight->text, table_get_entry(dt_eres_get(0x8033), 0x84));
    } else {
        name = dt_list_name(dt_flight->dst[dt_flight->sel]);
        wsprintf(dt_flight->text, table_get_entry(dt_eres_get(0x8033), 0x83), name);
    }
}

