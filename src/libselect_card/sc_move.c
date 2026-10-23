// @range 0x08815168-0x08816DD4
// @asm 0x08815168-0x08815408
// @asm 0x08815408-0x088158DC
// @asm 0x088158DC-0x08815CD0
// @asm 0x08815CD0-0x08815E48
// @asm 0x08815E78-0x08815F04
// @asm 0x08815F04-0x08816468
// @asm 0x088164A0-0x08816550
// @asm 0x08816550-0x08816874
// @asm 0x08816874-0x08816938
// @asm 0x08816938-0x08816CD8
// @data 0x0881B51C-0x0881B528
/* The keyboard frame update, moving a card between the panes (Circle, undo with Triangle): the
   entry is taken out, flown along a curve from its slot to the slot it lands in and added to the
   other list; and the message popup (results, limits, quit confirmation). */
#include "libselect_card.h"
#include "menu.h"

typedef struct ScOsk {                 /* see sc_word.c */
    int state;
    unsigned int base[12];
    int datacount;
    unsigned int *data;
    int pstate;
    int unk40;
    unsigned int odata[13];
    unsigned short in[0x18];
    unsigned short out[0x80];
    unsigned short *target;
    void (*done)(unsigned short *text, int r);
} ScOsk;

typedef struct ScFlight {              /* sc_flight 0x0881BB54, 0x34 bytes, heap allocated */
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
    ScEnt ent;                         /* +0x0E card being moved */
    unsigned char frames : 3;          /* +0x14 frames of the scroll wait */
    unsigned char pad14 : 5;
    unsigned char pad15[3];
    float from[2];                     /* +0x18 */
    float mid[2];                      /* +0x20 */
    float to[2];                       /* +0x28 */
    short x, y;                        /* +0x30 */
} ScFlight;

extern ScOsk sc_osk;
extern ScFlight *sc_flight;
extern unsigned short sc_popup_text[0x400];
extern MenuWidget sc_popup;

void sysvar_set_a90(int v);
int sceGuSync(int mode, int what);
int sceUtilityOskGetStatus(void);
int sceUtilityOskUpdate(int n);
int sceUtilityOskShutdownStart(void);
int wcslen(const unsigned short *s);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
void bgm_cmd(int arg);
ScEnt *sc_cur_entry(unsigned short side);
unsigned short sc_cur_index(unsigned short side);
int sc_list_find(ScList *l, unsigned short id);
int sc_list_remove(ScList *l, int i);
void sc_list_add(ScList *l, ScEnt *src);
void sc_lists_save(void);
void sc_select_tab(unsigned short side, unsigned short tab, short how);
void sc_layout_scroll(short keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
unsigned int sc_tab_pos(unsigned short i);
void sc_hist_push(ScMove *m);
void sc_popup_open(int id, int arg);
int sc_hist_pop(ScMove *m);
void sc_flight_take(void);
void sc_flight_land(void);
void sc_flight_step(void);
void sc_flight_free(void);
int sc_slot_pos(float *out, int side, unsigned short tab, int pos, int scroll);
unsigned short sc_dst_pos(unsigned short tab, unsigned short id);

#ifdef NON_MATCHING /* laço da cópia com '$' dobrado e alocação diferentes */
/* per frame while the keyboard runs: fades, utility status; 1 when closed */
int sc_popup_done(int layer)
{
    void *list;
    unsigned short *d;
    unsigned int i;
    int n;
    unsigned short c;
    int r;
    unsigned short *p;

    if (sc_osk.state == 3 || sc_osk.state == 2 || sc_osk.state == 1) {
        list = dl_select_layer(layer, 0);
        if (fade_apply(&list)) {
            if (sc_osk.state == 1)
                sc_osk.state = 2;
            if (sc_osk.state == 3)
                sc_osk.state = 4;
        }
        dl_set_list(list);
    }
    sceGuSync(0, 0);
    if (sc_osk.state == 4) {
        sysvar_set_a90(0);
        return 1;
    }
    if (sc_osk.state == 3)
        return 0;
    if (sc_osk.state == 2 || sc_osk.state == 1) {
        r = sceUtilityOskGetStatus();
        if (r != 4 && (r == 3 || (r == 2 && sceUtilityOskUpdate(1) != 0))) {
            r = sceUtilityOskShutdownStart();
            if (sc_osk.base[7] == 0) {
                d = sc_osk.target;
                n = 0;
                i = 0;
                if (wcslen(sc_osk.out) != 0) {
                    for (p = sc_osk.out, c = *p; c != 0; c = *++p) {
                        if (c == '$')
                            d[n++] = '$';
                        d[n++] = c;
                        i++;
                        if (i >= (unsigned int)wcslen(sc_osk.out))
                            break;
                    }
                }
                d[n] = 0;
            }
            if (sc_osk.done != 0)
                sc_osk.done(sc_osk.out, r);
            fade_start(2, 0xA0000000, 8);
            sc_osk.state = 3;
        }
    } else if (sc_osk.state == 0) {
        sysvar_set_a90(0);
        return 1;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* bitfields do voo gravados em outra ordem */
/* Circle: moves the cursor's card to the other list (if the kind allows it) */
int sc_cancel_request(void)
{
    unsigned int tab = scv.pane[scv.side].list->tab;
    unsigned int to = (tab + 1) & 1;
    ScEnt *e = sc_cur_entry(scv.side);
    ScMove mv;
    ScFlight *f;

    if (e == 0)
        return 0;
    switch (sc.kind) {
    case 2:
        if (to == 1 && sc.list[1].n > 4) {
            sc_popup_open(2, 3);
            se_play(0x621A);
            return 0;
        }
        break;
    case 1:
        if (to == 1 && sc.list[1].total > 0x62) {
            sc_popup_open(2, 3);
            se_play(0x621A);
            return 0;
        }
        break;
    case 0:
        if (to == 1 && sc.list[1].n > 2) {
            sc_popup_open(2, 3);
            se_play(0x621A);
            return 0;
        }
        if (e->rank != 4)
            return 0;
        break;
    }
    sc_flight = heap_alloc(sc.heap, 0x34);
    memset(sc_flight, 0, 0x34);
    f = sc_flight;
    f->side = scv.side;
    f->src = tab;
    f->pos = sc_cur_index(scv.side);
    f->idx = sc.list[tab].order[f->pos];
    if (scv.mode < 2)
        f->dside = f->side;
    else
        f->dside = (f->side + 1) & 1;
    f->dst = to;
    f->dtab = scv.pane[to].list->tab;
    memcpy(&f->ent, e, 6);
    mv.b[0] = f->src | f->dst << 4;
    *(unsigned short *)&mv.b[8] = sc_dst_pos(f->dst, f->ent.id);
    memcpy(&mv.b[2], &f->ent, 6);
    sc_hist_push(&mv);
    sc.state = 2;
    f->phase = 0;
    sc_slot_pos(f->from, f->side, f->src, f->pos, 0);
    if (sc_slot_pos(f->to, f->dside, f->dst, sc_dst_pos(f->dst, f->ent.id), 1) == 0)
        f->step = 1;
    else
        f->step = 2;
    f->x = (int)f->from[0];
    f->y = (int)f->from[1];
    sc_flight_take();
    return 1;
}
#endif

#ifdef NON_MATCHING /* laços de busca e bitfields com outra alocação */
/* Triangle: undoes the last move */
int sc_input_move(void)
{
    ScMove mv;
    ScFlight *f;
    unsigned int src;
    int changed;
    ScEnt *e;
    int i;
    ScList *l;

    if (sc_hist_pop(&mv) == 0)
        return 0;
    sc_flight = heap_alloc(sc.heap, 0x34);
    if (sc_flight == 0)
        return 0;
    f = sc_flight;
    memset(f, 0, 0x34);
    f->src = (mv.b[0] & 0x7F) >> 4;
    f->idx = *(unsigned short *)&mv.b[8];
    if (scv.mode < 2) {
        f->side = 0;
        f->dside = 0;
    } else {
        f->side = mv.b[0] >> 4 != 0;
        f->dside = (f->side + 1) & 1;
    }
    f->dst = mv.b[0] & 7;
    f->dtab = scv.pane[f->dside].list->tab;
    memcpy(&f->ent, &mv.b[2], 6);
    sc.state = 2;
    f->step = 2;
    f->phase = 0;
    src = f->src;
    l = &sc.list[src];
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
    sc_slot_pos(f->from, f->side, f->src, i, 0);
    f->x = (int)f->from[0];
    f->y = (int)f->from[1];
    sc_flight_take();
    return 1;
}
#endif

#ifdef NON_MATCHING /* testes do passo e releituras de sc_flight diferentes */
/* state 2 */
void sc_input_sort_menu(void)
{
    ScFlight *f = sc_flight;

    switch (f->step) {
    case 3:
        sc_flight_free();
        sc.state = 1;
        break;
    case 2:
        sc_flight_step();
        break;
    case 1:
        if (++f->timer >= f->frames) {
            if (sc_slot_pos(sc_flight->to, sc_flight->dside, sc_flight->dst, sc_dst_pos(sc_flight->dst, sc_flight->ent.id), 1)) {
                sc_flight->phase = 0;
                sc_flight->x = (int)sc_flight->from[0];
                sc_flight->y = (int)sc_flight->from[1];
                sc_flight->step = 2;
            }
            sc_flight->timer = 0;
        }
        break;
    }
}
#endif

void sc_flight_take(void)
{
    sc_list_remove(&sc.list[sc_flight->src], sc_flight->idx);
}

#ifdef NON_MATCHING /* o original indexa sc.list pelo campo de 16 bits com a base calculada antes */
void sc_flight_land(void)
{
    unsigned int bits = *(unsigned short *)((char *)sc_flight + 6);
    ScEnt e;

    memcpy(&e, &sc_flight->ent, 6);
    e.count = 1;
    e.marked = 1;
    sc_list_add(&sc.list[(bits & 0x3FF) >> 7], &e);
}
#endif

#ifdef NON_MATCHING /* contas em float com outra alocação de $f */
/* the flight: path through a control point, 10 frames of quadratic curve, landing */
void sc_flight_step(void)
{
    ScFlight *f = sc_flight;
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
        if (scv.mode < 2 && f->dst != f->dtab)
            sc_select_tab(f->dside, f->dst, 2);
        sc_flight_land();
        i = scv.pane[0].cx + scv.pane[0].top + scv.pane[0].cy * (scv.pane[0].list_mode ? 1 : scv.pane[0].lay.cols);
        keep = 1;
        if (i > 0 && i >= scv.pane[0].list->count) {
            i = scv.pane[0].list->count - 1;
            keep = 0;
        }
        sc_layout_scroll(keep, i, ((scv.pane[0].list_mode ? 1 : scv.pane[0].lay.cols) & 0xFF) |
                         (scv.pane[0].list_mode ? scv.pane[0].lay.rows : scv.pane[0].lay.grid_rows) << 8,
                         scv.pane[0].list->count, &scv.pane[0].top, &scv.pane[0].cx, &scv.pane[0].cy);
        if (scv.mode > 1) {
            i = scv.pane[1].cx + scv.pane[1].top + scv.pane[1].cy * (scv.pane[1].list_mode ? 1 : scv.pane[1].lay.cols);
            keep = 1;
            if (i > 0 && i >= scv.pane[1].list->count) {
                i = scv.pane[1].list->count - 1;
                keep = 0;
            }
            sc_layout_scroll(keep, i, ((scv.pane[1].list_mode ? 1 : scv.pane[1].lay.cols) & 0xFF) |
                             (scv.pane[1].list_mode ? scv.pane[1].lay.rows : scv.pane[1].lay.grid_rows) << 8,
                             scv.pane[1].list->count, &scv.pane[1].top, &scv.pane[1].cx, &scv.pane[1].cy);
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
        sc_slot_pos(f->to, f->dside, f->dst, sc_dst_pos(f->dst, f->ent.id), 0);
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

void sc_flight_free(void)
{
    if (sc_flight != 0) {
        heap_set_level(sc.heap, sc_flight);
        sc_flight = 0;
    }
}

#ifdef NON_MATCHING /* o original lê os campos de f depois dos recursos */
void *sc_draw_sort_menu(void *list)
{
    ScFlight *f = sc_flight;

    if (f->step != 2)
        return list;
    else
        list = sc_draw_card_pict(list, f->x - 11, f->y - 15, 0, 0xFFFFFFFF, &f->ent, res_get(0x8001), res_get(0x8002), res_get(0x8032));
    return list;
}
#endif

#ifdef NON_MATCHING /* registradores herdados de testes anteriores (v1) no original */
/* screen position of position pos of list tab shown in pane side; scroll the pane towards it when
   asked (returns 0 then) */
int sc_slot_pos(float *out, int side, unsigned short tab, int pos, int scroll)
{
    ScPane *p = &scv.pane[side];
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
    if (scroll) {
        switch (sc.kind) {
        case 2:
        case 1:
        case 0:
            if (tab == 0)
                scroll = 0;
            break;
        }
    }
    if (!other) {
        if (pos < p->top) {
            if (scroll) {
                p->top -= step;
                sc_flight->frames = step;
                return 0;
            }
            other = 1;
        } else if (pos >= (int)(p->top + view)) {
            if (scroll) {
                p->top += step;
                sc_flight->frames = step;
                return 0;
            }
            other = 1;
        }
    }
    if (other) {
        t = sc_tab_pos(tab);
        out[0] = (float)(int)((t >> 17) + p->lay.x + (t & 0xFFFF));
        out[1] = 32.0f;
        return 1;
    }
    if (p->list_mode == 1) {
        out[0] = (float)(p->lay.x + p->lay.row_x);
        out[1] = (float)(int)(p->lay.y + p->lay.row_y + p->lay.row_h * (pos - p->top));
        if (scv.mode < 4 && side == 1)
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

#ifdef NON_MATCHING /* cadeia de testes do tipo com outra ordem */
/* position the card id lands at in list tab */
unsigned short sc_dst_pos(unsigned short tab, unsigned short id)
{
    int sep = 1;
    int i;
    unsigned int k;

    switch (sc.kind) {
    case 2:
        if (tab == 0)
            sep = 0;
        break;
    case 1:
        sep = 0;
        break;
    case 0:
        if (tab == 0)
            sep = 0;
        break;
    }
    if (!sep) {
        i = sc_list_find(&sc.list[tab], id);
        if (i >= 0) {
            for (k = 0; k < sc.list[tab].count; k++) {
                if (i == sc.list[tab].order[k])
                    return k;
            }
        }
    }
    return sc.list[tab].count;
}
#endif

/* messages of popup 2 */
unsigned short sc_popup_msgs[6] = { 0, 0x22, 0x23, 0x84, 0x29, 0 };

#ifdef NON_MATCHING /* gravações dos campos do menu em outra ordem */
/* opens popup id: 4 search/filter result (arg = count), 3 quit confirmation, 2 message arg */
void sc_popup_open(int id, int arg)
{
    unsigned char buttons = 1;

    memset(&sc_popup, 0, 0x120);
    memset(sc_popup_text, 0, 0x800);
    if (id == 4) {
        if (arg == 0)
            wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), 0x89));
        else
            wsprintf(sc_popup_text, table_get_entry(res_get(0x8033), 0x81), arg);
        sc_popup.initial = 1;
        sc_popup.def.max_h = 0x50;
        sc_popup.cancel_resets = 1;
    } else if (id == 3) {
        switch (sc.kind) {
        case 2:
            wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), 0x99));
            break;
        case 1:
            if (sc.list[1].total < 10)
                wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), 0x9A));
            else
                wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), 0x98));
            break;
        case 0:
            wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), 0x97));
            break;
        }
        sc_popup.initial = 4;
        sc_popup.cancel_resets = 1;
        buttons = 2;
    } else if (id == 2) {
        wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), sc_popup_msgs[arg]));
        sc_popup.cancel_blocked = 1;
    }
    sc_popup.def.visible = 1;
    sc_popup.def.auto_size = 1;
    sc_popup.def.font_flag4 = 1;
    sc_popup.def.max_w = 0xF0;
    sc_popup.def.color = 0xFFFFFFFF;
    sc_popup.def.pad_l = 10;
    sc_popup.def.pad_r = 10;
    sc_popup.def.pad_t = 8;
    sc_popup.def.font_size = 0xC;
    sc_popup.def.text_color = 0xFF000000;
    sc_popup.def.text = sc_popup_text;
    sc_popup.def.open_anim = 2;
    sc_popup.def.close_anim = 2;
    sc_popup.heap = sc.heap;
    sc_popup.no_cancel = 1;
    sc_popup.buttons = buttons;
    sc_popup.def.style = 0;
    sc_popup.def.pad_b = 0;
    menu_open(&sc_popup);
    sc_popup.def.x = (0x1E0 - sc_popup.def.w) / 2;
    sc_popup.def.y = (0x110 - sc_popup.def.h) / 2;
    scv.popup = id;
}
#endif

/* the popup's input; Yes on the quit confirmation saves and leaves */
void sc_popup_update(void)
{
    int r = menu_update(&sc_popup);

    if (r == -1)
        return;
    menu_reset(&sc_popup);
    switch (scv.popup) {
    case 3:
        if (r != 1)
            break;
        sc_lists_save();
        fade_start_in(1, 0xFF000000, 16);
        sc.state = 9;
        if (sc.kind == 0)
            bgm_cmd(600);
        break;
    }
    scv.popup = 0;
}

void *sc_draw_popup(void *list)
{
    menu_draw(&list, &sc_popup);
    return list;
}
