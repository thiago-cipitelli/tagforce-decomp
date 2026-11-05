// @range 0x0880BE1C-0x0880D23C
// @asm 0x0880C254-0x0880C648
/* Moving a card between the main and the extra deck (Circle, undo with Triangle): the entry is
   taken out, flown along a curve from its slot to the slot it lands in and added to the other list;
   the position of a slot on screen, and the filter reset of a list (same source as libselect_card
   sc_move.c). */
#include "libdeck_swap.h"

typedef struct DsSel {
    unsigned short side : 2;           /* source pane */
    unsigned short src : 3;            /* source list */
    unsigned short dside : 2;          /* destination pane */
    unsigned short dst : 3;            /* destination list */
    unsigned short pad : 6;
} DsSel;

typedef struct DsHist {                /* undo record (ds_hist_push), 10 bytes */
    unsigned char src : 4;             /* source list */
    unsigned char dst : 4;             /* destination list */
    unsigned char pad01;
    DsEnt ent;                         /* +0x02 */
    unsigned short pos;                /* +0x08 position in the destination order */
} DsHist;

typedef struct DsFlight {              /* ds_flight 0x0881BB54, 0x34 bytes, heap allocated */
    unsigned char step : 4;            /* 0 start, 1 scroll wait, 2 flying, 3 done */
    unsigned char phase : 4;           /*   flying: 0 path, 1 move, 2 land */
    unsigned char timer;               /* +0x01 */
    unsigned short idx;                /* +0x02 entry index in the source list */
    unsigned short pos;                /* +0x04 position in its visible order */
    DsSel sel;                         /* +0x06 */
    unsigned short dtab;               /* +0x08 tab shown in the destination pane */
    unsigned short pad0a[2];
    DsEnt ent;                         /* +0x0E card being moved */
    unsigned char frames : 3;          /* +0x14 frames of the scroll wait */
    unsigned char pad14 : 5;
    unsigned char pad15[3];
    float from[2];                     /* +0x18 */
    float mid[2];                      /* +0x20 */
    float to[2];                       /* +0x28 */
    short x, y;                        /* +0x30 */
} DsFlight;

extern DsFlight *ds_flight;

DsEnt *ds_cur_entry(unsigned short side);
unsigned short ds_cur_index(unsigned short side);
int ds_list_remove(DsList *l, int i);
void ds_list_add(DsList *l, DsEnt *src);
void ds_list_sort(DsList *l, unsigned short sort, unsigned short dir);
void ds_select_tab(unsigned short side, unsigned short tab, short how);
void ds_layout_scroll(int keep, int count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
unsigned int ds_tab_pos(unsigned short i);
void ds_hist_push(DsMove *m);
int ds_hist_pop(DsMove *m);
void ds_flight_take(void);
void ds_flight_land(void);
void ds_flight_step(void);
void ds_flight_free(void);
void ds_attr_count_update(void);
void ds_stream_anim(void);
int ds_slot_pos(float *out, int side, unsigned int tab, int pos, int scroll);
int ds_dst_pos(int tab, unsigned short id);

/* Circle: moves the cursor's card between the main and the extra deck (the side deck stays) */
int ds_flight_open(void)
{
    unsigned short tab = dsv.pane[dsv.side].list->tab;
    unsigned short other = (tab + 1) & 1;
    DsEnt *e = ds_cur_entry(dsv.side);
    DsHist h;
    DsFlight *f;
    DsSel *s;

    if (e == 0)
        return 0;
    if (tab == 1)
        return 0;
    ds_flight = heap_alloc(ds.heap, 0x34);
    memset(ds_flight, 0, 0x34);
    ds_flight->sel.side = dsv.side;
    ds_flight->sel.src = tab;
    ds_flight->pos = ds_cur_index(dsv.side);
    ds_flight->idx = ds.list[tab].order[ds_flight->pos];
    if (dsv.mode > 1)
        ds_flight->sel.dside = (ds_flight->sel.side + 1) & 1;
    else
        ds_flight->sel.dside = ds_flight->sel.side;
    switch (tab) {
    case 0:
        ds_flight->sel.dst = 2;
        break;
    case 2:
        ds_flight->sel.dst = 0;
        break;
    default:
        ds_flight_free();
        return 0;
    }
    ds_flight->dtab = dsv.pane[other].list->tab;
    memcpy(&ds_flight->ent, e, 6);
    f = ds_flight;
    h.src = f->sel.src;
    h.dst = f->sel.dst;
    h.pos = ds_dst_pos(h.dst, f->ent.id);
    memcpy(&h.ent, &ds_flight->ent, 6);
    ds_hist_push((DsMove *)&h);
    ds.state = 2;
    ds_flight->phase = 0;
    ds_slot_pos(ds_flight->from, ds_flight->sel.side, ds_flight->sel.src, ds_flight->pos, 0);
    f = ds_flight;
    s = &f->sel;
    if (ds_slot_pos(f->to, s->dside, s->dst, ds_dst_pos(f->sel.dst, f->ent.id), 1))
        ds_flight->step = 2;
    else
        ds_flight->step = 1;
    ds_flight->x = ds_flight->from[0];
    ds_flight->y = ds_flight->from[1];
    ds_flight_take();
    return 1;
}

#ifdef NON_MATCHING /* 3 palavras: só o lado (switch em h.dst), que o original calcula com bnezl + li 1 no atraso e move zero (aqui beqzl); o slot_pos recebe k (não o pos u16 relido) */
/* Triangle: undoes the last move */
int ds_input_move(void)
{
    DsHist h;
    DsList *l;
    int i;
    int k;
    int found;
    int side;

    if (ds_hist_pop((DsMove *)&h) == 0)
        return 0;
    ds_flight = heap_alloc(ds.heap, 0x34);
    if (ds_flight == 0)
        return 0;
    memset(ds_flight, 0, 0x34);
    ds_flight->sel.src = h.dst;
    ds_flight->idx = h.pos;
    if (dsv.mode > 1) {
        switch (h.dst) {
        case 0:
            side = 0;
            break;
        default:
            side = 1;
            break;
        }
        ds_flight->sel.side = side;
        ds_flight->sel.dside = (ds_flight->sel.side + 1) & 1;
    } else {
        ds_flight->sel.side = 0;
        ds_flight->sel.dside = 0;
    }
    ds_flight->sel.dst = h.src;
    ds_flight->dtab = dsv.pane[ds_flight->sel.dside].list->tab;
    memcpy(&ds_flight->ent, &h.ent, 6);
    ds.state = 2;
    ds_flight->step = 2;
    ds_flight->phase = 0;
    l = &ds.list[ds_flight->sel.src];
    found = 0;
    k = -1;
    if (ds_flight->idx >= l->n) {
        found = 1;
    } else {
        DsEnt *e = &l->ents[ds_flight->idx];

        if (e->id != h.ent.id)
            found = 1;
        else if (e->fixed != h.ent.fixed)
            found = 1;
    }
    if (found) {
        ds_flight->idx = 0xFFFF;
        for (i = l->n - 1; i >= 0; i--) {
            DsEnt *e = &l->ents[i];

            if (e->id == h.ent.id && e->fixed == h.ent.fixed) {
                ds_flight->idx = i;
                break;
            }
        }
        if (ds_flight->idx < 0) {
            ds_flight_free();
            return 0;
        }
    }
    for (i = 0; i < l->count; i++) {
        if (ds_flight->idx == l->order[i]) {
            k = i;
            break;
        }
    }
    ds_flight->pos = k;
    ds_slot_pos(ds_flight->from, ds_flight->sel.side, ds_flight->sel.src, k, 0);
    ds_flight->x = ds_flight->from[0];
    ds_flight->y = ds_flight->from[1];
    ds_flight_take();
    return 1;
}
#endif

/* state 2 */
void ds_input_sort_menu(void)
{
    DsFlight *f = ds_flight;
    DsSel *s;

    switch (f->step) {
    case 1:
        if (++f->timer >= f->frames) {
            f = ds_flight;
            s = &f->sel;
            if (ds_slot_pos(f->to, s->dside, s->dst, ds_dst_pos(s->dst, f->ent.id), 1)) {
                ds_flight->phase = 0;
                ds_flight->x = ds_flight->from[0];
                ds_flight->y = ds_flight->from[1];
                ds_flight->step = 2;
            }
            ds_flight->timer = 0;
        }
        break;
    case 2:
        ds_flight_step();
        break;
    case 3:
        ds_flight_free();
        ds.state = 1;
        break;
    }
}

void ds_flight_take(void)
{
    ds_list_remove(&ds.list[ds_flight->sel.src], ds_flight->idx);
}

void ds_flight_land(void)
{
    DsList *l = &ds.list[ds_flight->sel.dst];
    DsEnt e;

    memcpy(&e, &ds_flight->ent, 6);
    e.count = 1;
    e.marked = 1;
    ds_list_add(l, &e);
}

/* the view argument of ds_layout_scroll: columns in the low byte, visible rows in the high one */
static inline unsigned short ds_lay_pack(unsigned char cols, unsigned char rows)
{
    return cols | rows << 8;
}

/* keeps the cursor of pane p inside its list after a move (last entry when it fell off the end) */
#define DS_PANE_REFIT(p)                                                                                   \
    do {                                                                                                   \
        pos = (p)->cx + ((p)->top + (p)->cy * ((p)->list_mode == 0 ? (p)->lay.cols : 1));                  \
        keep = 1;                                                                                          \
        if (pos > 0 && pos >= (p)->list->count) {                                                          \
            pos = (p)->list->count - 1;                                                                    \
            keep = 0;                                                                                      \
        }                                                                                                  \
        view = (p)->list_mode == 0 ? (p)->lay.grid_rows : (p)->lay.rows;                                   \
        ds_layout_scroll(keep, pos, ds_lay_pack((p)->list_mode == 0 ? (p)->lay.cols : 1, view),            \
                         (p)->list->count, &(p)->top, &(p)->cx, &(p)->cy);                                 \
    } while (0)

/* the flight: path through a control point, 10 frames of quadratic curve, landing */
void ds_flight_step(void)
{
    DsFlight *f = ds_flight;
    float t;
    float u;
    float k;
    float uu;
    float tt;
    DsPane *p;
    DsSel *s;
    int pos;
    unsigned short keep;
    unsigned short view;

    switch (f->phase) {
    case 0:
        s = &f->sel;
        ds_slot_pos(f->to, s->dside, s->dst, ds_dst_pos(s->dst, f->ent.id), 0);
        if (ds_flight->from[1] - 10.0f <= ds_flight->to[1] && ds_flight->from[1] + 10.0f >= ds_flight->to[1]) {
            if (ds_flight->from[1] <= 136.0f)
                ds_flight->mid[1] = ds_flight->from[1] + 68.0f;
            else
                ds_flight->mid[1] = 24.0f;
            ds_flight->mid[0] = ds_flight->from[0] + (ds_flight->to[0] - ds_flight->from[0]) / 2.0f;
        } else if (ds_flight->from[0] - 10.0f <= ds_flight->to[0] && ds_flight->from[0] + 10.0f >= ds_flight->to[0]) {
            if (ds_flight->from[0] <= 240.0f)
                ds_flight->mid[0] = ds_flight->from[0] + 120.0f;
            else
                ds_flight->mid[0] = ds_flight->from[0] - 120.0f;
            ds_flight->mid[1] = ds_flight->from[1] + (ds_flight->to[1] - ds_flight->from[1]) / 2.0f;
        } else if (ds_flight->from[1] < ds_flight->to[1]) {
            ds_flight->mid[1] = ds_flight->from[1];
            ds_flight->mid[0] = ds_flight->to[0];
        } else {
            ds_flight->mid[1] = ds_flight->to[1];
            ds_flight->mid[0] = ds_flight->from[0];
        }
        ds_flight->timer = 0;
        ds_flight->phase++;
        break;
    case 1:
        if (f->timer++ < 10) {
            t = ds_flight->timer / 10.0f;
            u = 1.0f - t;
            uu = u * u;
            k = 2.0f * t * u;
            tt = t * t;
            ds_flight->x = tt * ds_flight->to[0] + (uu * ds_flight->from[0] + k * ds_flight->mid[0]);
            ds_flight->y = tt * ds_flight->to[1] + (uu * ds_flight->from[1] + k * ds_flight->mid[1]);
        } else {
            ds_flight->phase++;
        }
        break;
    case 2:
        if (dsv.mode < 2 && (s = &f->sel, s->dst != f->dtab))
            ds_select_tab(s->dside, f->dtab, 2);
        ds_flight_land();
        ds_attr_count_update();
        ds_stream_anim();
        p = &dsv.pane[0];
        DS_PANE_REFIT(p);
        if (dsv.mode > 1) {
            p = &dsv.pane[1];
            DS_PANE_REFIT(p);
        }
        ds_flight->step = 3;
        ds_flight->phase = 0;
        ds_flight->timer = 0;
        break;
    }
}

void ds_flight_free(void)
{
    if (ds_flight != 0) {
        heap_set_level(ds.heap, ds_flight);
        ds_flight = 0;
    }
}

void *ds_draw_sort_menu(void *list)
{
    DsFlight *f = ds_flight;

    switch (f->step) {
    case 2:
        list = ds_draw_card_pict(list, f->x - 11, f->y - 15, 0, 0xFFFFFFFF, &f->ent, res_get(0x8001), res_get(0x8002), res_get(0x8032));
        break;
    }
    return list;
}

/* screen position of position pos of list tab shown in pane side (card center, +11/+15); when the
   entry is scrolled out and scroll is set, scrolls the pane one step toward it and returns 0; tabs
   not shown in the pane give the position of their tab */
int ds_slot_pos(float *out, int side, unsigned int tab, int pos, int scroll)
{
    DsPane *p = &dsv.pane[side];
    DsLayout *lay = &p->lay;
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

    if (dsv.pane[side].list->tab != tab)
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
            } else {
                p->top = top - (unsigned short)step;
                ds_flight->frames = frames;
                return 0;
            }
        } else if (top + count <= pos) {
            if (scroll == 0) {
                off = 1;
            } else {
                p->top = top + (unsigned short)step;
                ds_flight->frames = frames;
                return 0;
            }
        }
    }
    if (off) {
        tp = ds_tab_pos(tab);
        len = tp >> 16;
        x = tp;
        out[0] = (int)((unsigned int)len / 2 + (lay->x + x));
        out[1] = 32.0f;
        return 1;
    }
    switch (mode) {
    case 0:
        d = pos - p->top;
        cols = lay->cols;
        {
            int m = d % cols;
            out[0] = (int)(lay->x + lay->grid_x + m * (unsigned short)lay->cell_w);
            m = d / cols;
            out[1] = (int)(lay->y + lay->grid_y + m * (unsigned short)lay->cell_h);
        }
        break;
    case 1:
        out[0] = lay->x + lay->row_x;
        cols = lay->y + lay->row_y;
        out[1] = (int)(cols + (unsigned short)lay->row_h * (pos - p->top));
        if (dsv.mode < 4 && side == 1)
            out[1] += 1.0f;
        break;
    }
    out[0] += 11.0f;
    out[1] += 15.0f;
    return 1;
}

/* position the card lands at in list tab: the end of its visible order */
int ds_dst_pos(int tab, unsigned short id)
{
    DsState *d = &ds;
    DsList *l = &d->list[tab];

    return l->count;
}


/* clears the filter of l, re-sorted by key 1 */
unsigned short ds_list_filter_reset(DsList *l)
{
    int i;
    int n = 0;

    for (i = 0; i < l->n; i++) {
        n++;
        l->order[i] = i;
    }
    l->count = n;
    l->filter = 0;
    ds_list_sort(l, 1, 0);
    return l->count;
}
