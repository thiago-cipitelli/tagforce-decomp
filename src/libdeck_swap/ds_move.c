// @range 0x0880BE1C-0x0880D23C
// @asm 0x0880BE1C-0x0880C7C0
// @asm 0x0880C87C-0x0880CDF0
// @asm 0x0880CE28-0x0880D198
// @asm 0x0880D198-0x0880D1B0
/* Moving a card between the main and the extra deck (Circle, undo with Triangle): the entry is
   taken out, flown along a curve from its slot to the slot it lands in and added to the other list;
   the position of a slot on screen, and the filter reset of a list (same source as libselect_card
   sc_move.c). */
#include "libdeck_swap.h"

typedef struct DsFlight {              /* ds_flight 0x0881BB54, 0x34 bytes, heap allocated */
    unsigned char step : 4;            /* 0 start, 1 scroll wait, 2 flying, 3 done */
    unsigned char phase : 4;           /*   flying: 0 path, 1 move, 2 land */
    unsigned char timer;               /* +0x01 */
    unsigned short idx;                /* +0x02 entry index in the source list */
    unsigned short pos;                /* +0x04 position in its visible order */
    unsigned short side : 2;           /* +0x06 source pane */
    unsigned short src : 3;            /*       source list */
    unsigned short dside : 2;          /*       destination pane */
    unsigned short dst : 3;            /*       destination list */
    unsigned short pad06 : 6;
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
void ds_layout_scroll(short keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
unsigned int ds_tab_pos(unsigned short i);
void ds_hist_push(DsMove *m);
int ds_hist_pop(DsMove *m);
void ds_flight_take(void);
void ds_flight_land(void);
void ds_flight_step(void);
void ds_flight_free(void);
int ds_slot_pos(float *out, int side, unsigned short tab, int pos, int scroll);
unsigned short ds_dst_pos(int tab, unsigned short id);

#ifdef NON_MATCHING /* bitfields do voo gravados em outra ordem (como sc_cancel_request) */
/* Circle: moves the cursor's card between the main and the extra deck (the side deck stays) */
int ds_flight_open(void)
{
    unsigned short tab = dsv.pane[dsv.side].list->tab;
    DsEnt *e = ds_cur_entry(dsv.side);
    DsMove mv;
    DsFlight *f;

    if (e == 0)
        return 0;
    if (tab == 1)
        return 0;
    ds_flight = heap_alloc(ds.heap, 0x34);
    memset(ds_flight, 0, 0x34);
    ds_flight->side = dsv.side;
    ds_flight->src = tab;
    ds_flight->pos = ds_cur_index(dsv.side);
    ds_flight->idx = ds.list[tab].order[ds_flight->pos];
    if (dsv.mode < 2)
        ds_flight->dside = ds_flight->side;
    else
        ds_flight->dside = (ds_flight->side + 1) & 1;
    switch (tab) {
    case 2:
        ds_flight->dst = 0;
        break;
    case 0:
        ds_flight->dst = 2;
        break;
    default:
        ds_flight_free();
        return 0;
    }
    ds_flight->dtab = dsv.pane[(tab + 1) & 1].list->tab;
    memcpy(&ds_flight->ent, e, 6);
    f = ds_flight;
    mv.b[0] = f->src | f->dst << 4;
    *(unsigned short *)&mv.b[8] = ds_dst_pos(f->dst, f->ent.id);
    memcpy(&mv.b[2], &f->ent, 6);
    ds_hist_push(&mv);
    ds.state = 2;
    ds_flight->phase = 0;
    ds_slot_pos(ds_flight->from, ds_flight->side, ds_flight->src, ds_flight->pos, 0);
    f = ds_flight;
    if (ds_slot_pos(f->to, f->dside, f->dst, ds_dst_pos(f->dst, f->ent.id), 1) == 0)
        ds_flight->step = 1;
    else
        ds_flight->step = 2;
    ds_flight->x = (int)ds_flight->from[0];
    ds_flight->y = (int)ds_flight->from[1];
    ds_flight_take();
    return 1;
}
#endif

#ifdef NON_MATCHING /* laços de busca e bitfields com outra alocação */
/* Triangle: undoes the last move */
int ds_input_move(void)
{
    DsMove mv;
    DsFlight *f;
    unsigned int src;
    int changed;
    DsEnt *e;
    int i;
    DsList *l;

    if (ds_hist_pop(&mv) == 0)
        return 0;
    ds_flight = heap_alloc(ds.heap, 0x34);
    if (ds_flight == 0)
        return 0;
    f = ds_flight;
    memset(f, 0, 0x34);
    f->src = (mv.b[0] & 0x7F) >> 4;
    f->idx = *(unsigned short *)&mv.b[8];
    if (dsv.mode < 2) {
        f->side = 0;
        f->dside = 0;
    } else {
        f->side = mv.b[0] >> 4 != 0;
        f->dside = (f->side + 1) & 1;
    }
    f->dst = mv.b[0] & 7;
    f->dtab = dsv.pane[f->dside].list->tab;
    memcpy(&f->ent, &mv.b[2], 6);
    ds.state = 2;
    f->step = 2;
    f->phase = 0;
    src = f->src;
    l = &ds.list[src];
    changed = 0;
    if (f->idx < l->n) {
        e = &l->ents[f->idx];
        if (e->id != f->ent.id || e->fixed != f->ent.fixed)
            changed = 1;
    } else {
        changed = 1;
    }
    if (changed) {
        f->idx = 0xFFFF;
        for (i = l->n - 1; i >= 0; i--) {
            e = &l->ents[i];
            if (e->id == f->ent.id && e->fixed == f->ent.fixed) {
                f->idx = i;
                break;
            }
        }
    }
    for (i = 0; i < l->count; i++) {
        if (f->idx == l->order[i])
            break;
    }
    if (i >= l->count)
        i = -1;
    f->pos = i;
    ds_slot_pos(f->from, f->side, f->src, i, 0);
    f->x = (int)f->from[0];
    f->y = (int)f->from[1];
    ds_flight_take();
    return 1;
}
#endif

#ifdef NON_MATCHING /* testes do passo e releituras de ds_flight diferentes */
/* state 2 */
void ds_input_sort_menu(void)
{
    DsFlight *f = ds_flight;

    switch (f->step) {
    case 3:
        ds_flight_free();
        ds.state = 1;
        break;
    case 2:
        ds_flight_step();
        break;
    case 1:
        if (++f->timer >= f->frames) {
            if (ds_slot_pos(ds_flight->to, ds_flight->dside, ds_flight->dst, ds_dst_pos(ds_flight->dst, ds_flight->ent.id), 1)) {
                ds_flight->phase = 0;
                ds_flight->x = (int)ds_flight->from[0];
                ds_flight->y = (int)ds_flight->from[1];
                ds_flight->step = 2;
            }
            ds_flight->timer = 0;
        }
        break;
    }
}
#endif

void ds_flight_take(void)
{
    ds_list_remove(&ds.list[ds_flight->src], ds_flight->idx);
}

void ds_flight_land(void)
{
    DsList *l = &ds.list[ds_flight->dst];
    DsEnt e;

    memcpy(&e, &ds_flight->ent, 6);
    e.count = 1;
    e.marked = 1;
    ds_list_add(l, &e);
}

#ifdef NON_MATCHING /* contas em float com outra alocação de $f */
/* the flight: path through a control point, 10 frames of quadratic curve, landing */
void ds_flight_step(void)
{
    DsFlight *f = ds_flight;
    int i;
    int keep;
    float t;
    float u;
    float k;
    float y0;
    float y1;
    float x0;

    switch (f->phase) {
    case 2:
        if (dsv.mode < 2 && f->dst != f->dtab)
            ds_select_tab(f->dside, f->dst, 2);
        ds_flight_land();
        i = dsv.pane[0].cx + dsv.pane[0].top + dsv.pane[0].cy * (dsv.pane[0].list_mode ? 1 : dsv.pane[0].lay.cols);
        keep = 1;
        if (i > 0 && i >= dsv.pane[0].list->count) {
            i = dsv.pane[0].list->count - 1;
            keep = 0;
        }
        ds_layout_scroll(keep, i, ((dsv.pane[0].list_mode ? 1 : dsv.pane[0].lay.cols) & 0xFF) |
                         (dsv.pane[0].list_mode ? dsv.pane[0].lay.rows : dsv.pane[0].lay.grid_rows) << 8,
                         dsv.pane[0].list->count, &dsv.pane[0].top, &dsv.pane[0].cx, &dsv.pane[0].cy);
        if (dsv.mode > 1) {
            i = dsv.pane[1].cx + dsv.pane[1].top + dsv.pane[1].cy * (dsv.pane[1].list_mode ? 1 : dsv.pane[1].lay.cols);
            keep = 1;
            if (i > 0 && i >= dsv.pane[1].list->count) {
                i = dsv.pane[1].list->count - 1;
                keep = 0;
            }
            ds_layout_scroll(keep, i, ((dsv.pane[1].list_mode ? 1 : dsv.pane[1].lay.cols) & 0xFF) |
                             (dsv.pane[1].list_mode ? dsv.pane[1].lay.rows : dsv.pane[1].lay.grid_rows) << 8,
                             dsv.pane[1].list->count, &dsv.pane[1].top, &dsv.pane[1].cx, &dsv.pane[1].cy);
        }
        f->step = 3;
        f->phase = 0;
        f->timer = 0;
        break;
    case 1:
        if (f->timer++ < 10) {
            t = f->timer / 10.0f;
            u = 1.0f - t;
            k = t * 2.0f * u;
            f->x = (int)(t * t * f->to[0] + u * u * f->from[0] + k * f->mid[0]);
            f->y = (int)(t * t * f->to[1] + u * u * f->from[1] + k * f->mid[1]);
        } else {
            f->phase++;
        }
        break;
    case 0:
        ds_slot_pos(f->to, f->dside, f->dst, ds_dst_pos(f->dst, f->ent.id), 0);
        y0 = f->from[1];
        y1 = f->to[1];
        if (y1 < y0 - 10.0f || y0 + 10.0f < y1) {
            x0 = f->from[0];
            if (f->to[0] < x0 - 10.0f || x0 + 10.0f < f->to[0]) {
                if (y0 < y1) {
                    f->mid[1] = y0;
                    f->mid[0] = f->to[0];
                } else {
                    f->mid[1] = y1;
                    f->mid[0] = f->from[0];
                }
            } else {
                if (x0 <= 240.0f)
                    f->mid[0] = x0 + 120.0f;
                else
                    f->mid[0] = x0 - 120.0f;
                f->mid[1] = f->from[1] + (f->to[1] - f->from[1]) / 2.0f;
            }
        } else {
            if (y0 <= 136.0f)
                f->mid[1] = y0 + 68.0f;
            else
                f->mid[1] = 24.0f;
            f->mid[0] = f->from[0] + (f->to[0] - f->from[0]) / 2.0f;
        }
        f->timer = 0;
        f->phase++;
        break;
    }
}
#endif

void ds_flight_free(void)
{
    if (ds_flight != 0) {
        heap_set_level(ds.heap, ds_flight);
        ds_flight = 0;
    }
}

#ifdef NON_MATCHING /* o original lê os campos de f depois dos recursos */
void *ds_draw_sort_menu(void *list)
{
    DsFlight *f = ds_flight;

    if (f->step != 2)
        return list;
    else
        list = ds_draw_card_pict(list, f->x - 11, f->y - 15, 0, 0xFFFFFFFF, &f->ent, res_get(0x8001), res_get(0x8002), res_get(0x8032));
    return list;
}
#endif

#ifdef NON_MATCHING /* registradores herdados de testes anteriores (v1) no original */
/* screen position of position pos of list tab shown in pane side; scroll the pane towards it when
   asked (returns 0 then) */
int ds_slot_pos(float *out, int side, unsigned short tab, int pos, int scroll)
{
    DsPane *p = &dsv.pane[side];
    int other = p->list->tab != tab;
    unsigned int view;
    int step;
    unsigned int t;

    if (p->list_mode == 1) {
        view = p->lay.rows;
        step = 1;
    } else {
        view = p->lay.cols * p->lay.grid_rows;
        step = p->lay.cols;
    }
    if (!other) {
        if (pos < p->top) {
            if (scroll) {
                p->top -= step;
                ds_flight->frames = step;
                return 0;
            }
            other = 1;
        } else if (pos >= (int)(p->top + view)) {
            if (scroll) {
                p->top += step;
                ds_flight->frames = step;
                return 0;
            }
            other = 1;
        }
    }
    if (other) {
        t = ds_tab_pos(tab);
        out[0] = (float)(int)((t >> 17) + p->lay.x + (t & 0xFFFF));
        out[1] = 32.0f;
        return 1;
    }
    if (p->list_mode == 1) {
        out[0] = (float)(p->lay.x + p->lay.row_x);
        out[1] = (float)(int)(p->lay.y + p->lay.row_y + p->lay.row_h * (pos - p->top));
        if (dsv.mode < 4 && side == 1)
            out[1] = out[1] + 1.0f;
    } else {
        out[0] = (float)(int)(p->lay.x + p->lay.grid_x + (pos - p->top) % (int)p->lay.cols * p->lay.cell_w);
        out[1] = (float)(int)(p->lay.y + p->lay.grid_y + (pos - p->top) / (int)p->lay.cols * p->lay.cell_h);
    }
    out[0] = out[0] + 11.0f;
    out[1] = out[1] + 15.0f;
    return 1;
}
#endif

#ifdef NON_MATCHING /* o original soma o deslocamento do campo no lhu; o mwcc atual o dobra no addiu da base */
/* position the card lands at in list tab: the end of its visible order */
unsigned short ds_dst_pos(int tab, unsigned short id)
{
    return ds.list[tab].count;
}
#endif


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
