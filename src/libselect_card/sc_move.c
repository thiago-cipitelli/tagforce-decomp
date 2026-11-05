// @range 0x08815168-0x08816DD4
// @asm 0x08815408-0x088158DC
// @asm 0x088158DC-0x08815CD0
// @asm 0x08816550-0x08816874
// @data 0x0881B51C-0x0881B528
/* The keyboard frame update, moving a card between the panes (Circle, undo with Triangle): the
   entry is taken out, flown along a curve from its slot to the slot it lands in and added to the
   other list; and the message popup (results, limits, quit confirmation). */
#include "libselect_card.h"
#include "menu.h"

typedef struct ScOskData {             /* SceUtilityOskData */
    int unk00;
    int unk04;
    int language;
    int unk12;
    int inputtype;
    int lines;
    int unk24;
    unsigned short *desc;
    unsigned short *intext;
    int outtextlength;
    unsigned short *outtext;
    int result;
    int outtextlimit;
} ScOskData;

typedef struct ScOskParams {           /* SceUtilityOskParams */
    int size;
    int language;
    int button_swap;
    int graphics_thread;
    int access_thread;
    int font_thread;
    int sound_thread;
    int result;
    int reserved[4];
    int datacount;
    ScOskData *data;
    int state;
    int unk60;
} ScOskParams;

typedef struct ScOsk {                 /* sc_osk 0x0881B9A4, 0x1B0 bytes */
    unsigned char state;               /* +0x00 0 idle, 1 fade in, 2 open, 3 fade out, 4 done */
    unsigned char pad1[3];
    ScOskParams params;                /* +0x04 */
    ScOskData data;                    /* +0x44 */
    unsigned short intext[24];         /* +0x78 */
    unsigned short outtext[128];       /* +0xA8 */
    unsigned short *target;            /* +0x1A8 */
    void (*done)(unsigned short *text, int r); /* +0x1AC */
} ScOsk;

typedef struct ScSel {
    unsigned short side : 2;           /* source pane */
    unsigned short src : 3;            /* source list */
    unsigned short dside : 2;          /* destination pane */
    unsigned short dst : 3;            /* destination list */
    unsigned short pad : 6;
} ScSel;

typedef struct ScHist {                /* undo record (sc_hist_push), 10 bytes */
    unsigned char src : 4;             /* source list */
    unsigned char dst : 4;             /* destination list */
    unsigned char pad01;
    ScEnt ent;                         /* +0x02 */
    unsigned short pos;                /* +0x08 position in the destination order */
} ScHist;

typedef struct ScFlight {              /* sc_flight 0x0881BB54, 0x34 bytes, heap allocated */
    unsigned char step : 4;            /* 0 start, 1 scroll wait, 2 flying, 3 done */
    unsigned char phase : 4;           /*   flying: 0 path, 1 move, 2 land */
    unsigned char timer;               /* +0x01 */
    unsigned short idx;                /* +0x02 entry index in the source list */
    unsigned short pos;                /* +0x04 position in its visible order */
    ScSel sel;                         /* +0x06 */
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
void sc_layout_scroll(int keep, int count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
unsigned int sc_tab_pos(unsigned short i);
void sc_hist_push(ScMove *m);
void sc_popup_open(int id, int arg);
int sc_hist_pop(ScMove *m);
void sc_flight_take(void);

static inline unsigned short sc_lay_pack(unsigned char cols, unsigned char rows)
{
    return cols | rows << 8;
}

/* keeps the cursor of pane p inside its list after a move (last entry when it fell off the end) */
#define SC_PANE_REFIT(p)                                                                                   \
    do {                                                                                                   \
        pos = (p)->cx + ((p)->top + (p)->cy * ((p)->list_mode == 0 ? (p)->lay.cols : 1));                  \
        keep = 1;                                                                                          \
        if (pos > 0 && pos >= (p)->list->count) {                                                          \
            pos = (p)->list->count - 1;                                                                    \
            keep = 0;                                                                                      \
        }                                                                                                  \
        view = (p)->list_mode == 0 ? (p)->lay.grid_rows : (p)->lay.rows;                                   \
        sc_layout_scroll(keep, pos, sc_lay_pack((p)->list_mode == 0 ? (p)->lay.cols : 1, view),            \
                         (p)->list->count, &(p)->top, &(p)->cx, &(p)->cy);                                 \
    } while (0)

void sc_flight_land(void);
void sc_flight_step(void);
void sc_flight_free(void);
int sc_slot_pos(float *out, int side, unsigned int tab, int pos, int scroll);
int sc_dst_pos(int tab, unsigned short id);

/* per frame while the keyboard runs: fades, utility status; 1 when closed */
int sc_popup_done(int layer)
{
    switch (sc_osk.state) {
    case 1:
    case 2:
    case 3: {
        void *dl = dl_select_layer(layer, 0);
        if (fade_apply(&dl)) {
            if (sc_osk.state == 1)
                sc_osk.state = 2;
            if (sc_osk.state == 3)
                sc_osk.state = 4;
        }
        dl_set_list(dl);
    }
    }
    sceGuSync(0, 0);
    switch (sc_osk.state) {
    case 0:
        sysvar_set_a90(0);
        return 1;
    case 1:
    case 2:
        switch (sceUtilityOskGetStatus()) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            if (sceUtilityOskUpdate(2) == 0)
                break;
        case 3: {
            int r = sceUtilityOskShutdownStart();
            if (sc_osk.params.result == 0) {
                unsigned int i;
                int k;
                k = 0;
                i = 0;
                if (wcslen(sc_osk.outtext)) {
                    for (;;) {
                        unsigned short c = sc_osk.outtext[i];
                        if (c == 0)
                            break;
                        if (c == '$')
                            sc_osk.target[k++] = c;
                        sc_osk.target[k++] = c;
                        i++;
                        if (i >= wcslen(sc_osk.outtext))
                            break;
                    }
                }
                sc_osk.target[k] = 0;
            }
            if (sc_osk.done)
                sc_osk.done(sc_osk.outtext, r);
            fade_start(2, 0xA0000000, 8);
            sc_osk.state = 3;
            break;
        }
        case 4:
            break;
        }
        break;
    case 3:
        break;
    case 4:
        sysvar_set_a90(0);
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* lógica conferida com o asm (229 palavras): o original grava os nibbles de mv.b[0] um a um
                       e relê sc_flight a cada campo; bitfields do voo em outra ordem */
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
    f->sel.side = scv.side;
    f->sel.src = tab;
    f->pos = sc_cur_index(scv.side);
    f->idx = sc.list[tab].order[f->pos];
    if (scv.mode < 2)
        f->sel.dside = f->sel.side;
    else
        f->sel.dside = (f->sel.side + 1) & 1;
    f->sel.dst = to;
    f->dtab = scv.pane[to].list->tab;
    memcpy(&f->ent, e, 6);
    mv.b[0] = f->sel.src | f->sel.dst << 4;
    *(unsigned short *)&mv.b[8] = sc_dst_pos(f->sel.dst, f->ent.id);
    memcpy(&mv.b[2], &f->ent, 6);
    sc_hist_push(&mv);
    sc.state = 2;
    f->phase = 0;
    sc_slot_pos(f->from, f->sel.side, f->sel.src, f->pos, 0);
    if (sc_slot_pos(f->to, f->sel.dside, f->sel.dst, sc_dst_pos(f->sel.dst, f->ent.id), 1) == 0)
        f->step = 1;
    else
        f->step = 2;
    f->x = (int)f->from[0];
    f->y = (int)f->from[1];
    sc_flight_take();
    return 1;
}
#endif

#ifdef NON_MATCHING /* lógica conferida com o asm (faltava o teste morto de idx < 0 com sc_flight_free);
                       laços de busca e bitfields com outra alocação */
/* Triangle: undoes the last move */
int sc_input_move(void)
{
    ScHist h;
    ScList *l;
    int i;
    int k;
    int found;
    int side;

    if (sc_hist_pop((ScMove *)&h) == 0)
        return 0;
    sc_flight = heap_alloc(sc.heap, 0x34);
    if (sc_flight == 0)
        return 0;
    memset(sc_flight, 0, 0x34);
    sc_flight->sel.src = h.dst;
    sc_flight->idx = h.pos;
    if (scv.mode > 1) {
        switch (h.dst) {
        case 0:
            side = 0;
            break;
        default:
            side = 1;
            break;
        }
        sc_flight->sel.side = side;
        sc_flight->sel.dside = (sc_flight->sel.side + 1) & 1;
    } else {
        sc_flight->sel.side = 0;
        sc_flight->sel.dside = 0;
    }
    sc_flight->sel.dst = h.src;
    sc_flight->dtab = scv.pane[sc_flight->sel.dside].list->tab;
    memcpy(&sc_flight->ent, &h.ent, 6);
    sc.state = 2;
    sc_flight->step = 2;
    sc_flight->phase = 0;
    l = &sc.list[sc_flight->sel.src];
    k = -1;
    found = 0;
    if (sc_flight->idx >= l->n) {
        found = 1;
    } else {
        ScEnt *e = &l->ents[sc_flight->idx];

        if (e->id != h.ent.id)
            found = 1;
        else if (e->fixed != h.ent.fixed)
            found = 1;
    }
    if (found) {
        sc_flight->idx = 0xFFFF;
        for (i = l->n - 1; i >= 0; i--) {
            ScEnt *e = &l->ents[i];

            if (e->id == h.ent.id && e->fixed == h.ent.fixed) {
                sc_flight->idx = i;
                break;
            }
        }
        if (sc_flight->idx < 0) {
            sc_flight_free();
            return 0;
        }
    }
    for (i = 0; i < l->count; i++) {
        if (sc_flight->idx == l->order[i]) {
            k = i;
            break;
        }
    }
    sc_flight->pos = k;
    sc_slot_pos(sc_flight->from, sc_flight->sel.side, sc_flight->sel.src, sc_flight->pos, 0);
    sc_flight->x = sc_flight->from[0];
    sc_flight->y = sc_flight->from[1];
    sc_flight_take();
    return 1;
}
#endif

/* state 2 */
void sc_input_sort_menu(void)
{
    ScFlight *f = sc_flight;
    ScSel *s;

    switch (f->step) {
    case 1:
        if (++f->timer >= f->frames) {
            f = sc_flight;
            s = &f->sel;
            if (sc_slot_pos(f->to, s->dside, s->dst, sc_dst_pos(s->dst, f->ent.id), 1)) {
                sc_flight->phase = 0;
                sc_flight->x = sc_flight->from[0];
                sc_flight->y = sc_flight->from[1];
                sc_flight->step = 2;
            }
            sc_flight->timer = 0;
        }
        break;
    case 2:
        sc_flight_step();
        break;
    case 3:
        sc_flight_free();
        sc.state = 1;
        break;
    }
}

void sc_flight_take(void)
{
    sc_list_remove(&sc.list[sc_flight->sel.src], sc_flight->idx);
}

void sc_flight_land(void)
{
    ScFlight *f = sc_flight;
    ScList *l = &sc.list[f->sel.dst];
    ScEnt e;

    memcpy(&e, &f->ent, 6);
    e.count = 1;
    e.marked = 1;
    sc_list_add(l, &e);
}

/* point at t of the quadratic curve a, m, b of a flight */
static inline float sc_bez(float t, float a, float m, float b)
{
    float u = 1.0f - t;
    float k = 2.0f * t * u;
    u = u * u;
    t = t * t;
    return t * b + (u * a + k * m);
}

/* the flight: path through a control point, 10 frames of quadratic curve, landing */
void sc_flight_step(void)
{
    ScFlight *f = sc_flight;
    float t;
    float u;
    float k;
    float uu;
    float tt;
    ScPane *p;
    ScSel *s;
    int pos;
    unsigned short keep;
    unsigned short view;

    switch (f->phase) {
    case 0:
        s = &f->sel;
        sc_slot_pos(f->to, s->dside, s->dst, sc_dst_pos(s->dst, f->ent.id), 0);
        if (sc_flight->from[1] - 10.0f <= sc_flight->to[1] && sc_flight->from[1] + 10.0f >= sc_flight->to[1]) {
            if (sc_flight->from[1] <= 136.0f)
                sc_flight->mid[1] = sc_flight->from[1] + 68.0f;
            else
                sc_flight->mid[1] = 24.0f;
            sc_flight->mid[0] = sc_flight->from[0] + (sc_flight->to[0] - sc_flight->from[0]) / 2.0f;
        } else if (sc_flight->from[0] - 10.0f <= sc_flight->to[0] && sc_flight->from[0] + 10.0f >= sc_flight->to[0]) {
            if (sc_flight->from[0] <= 240.0f)
                sc_flight->mid[0] = sc_flight->from[0] + 120.0f;
            else
                sc_flight->mid[0] = sc_flight->from[0] - 120.0f;
            sc_flight->mid[1] = sc_flight->from[1] + (sc_flight->to[1] - sc_flight->from[1]) / 2.0f;
        } else if (sc_flight->from[1] < sc_flight->to[1]) {
            sc_flight->mid[1] = sc_flight->from[1];
            sc_flight->mid[0] = sc_flight->to[0];
        } else {
            sc_flight->mid[1] = sc_flight->to[1];
            sc_flight->mid[0] = sc_flight->from[0];
        }
        sc_flight->timer = 0;
        sc_flight->phase++;
        break;
    case 1:
        if (f->timer++ < 10) {
            t = sc_flight->timer / 10.0f;
            u = 1.0f - t;
            uu = u * u;
            k = 2.0f * t * u;
            tt = t * t;
            sc_flight->x = tt * sc_flight->to[0] + (uu * sc_flight->from[0] + k * sc_flight->mid[0]);
            sc_flight->y = tt * sc_flight->to[1] + (uu * sc_flight->from[1] + k * sc_flight->mid[1]);
        } else {
            sc_flight->phase++;
        }
        break;
    case 2:
        if (scv.mode < 2 && (s = &f->sel, s->dst != f->dtab))
            sc_select_tab(s->dside, f->dtab, 2);
        sc_flight_land();
        p = &scv.pane[0];
        SC_PANE_REFIT(p);
        if (scv.mode > 1) {
            p = &scv.pane[1];
            SC_PANE_REFIT(p);
        }
        sc_flight->step = 3;
        sc_flight->phase = 0;
        sc_flight->timer = 0;
        break;
    }
}

void sc_flight_free(void)
{
    if (sc_flight != 0) {
        heap_set_level(sc.heap, sc_flight);
        sc_flight = 0;
    }
}

void *sc_draw_sort_menu(void *list)
{
    ScFlight *f = sc_flight;

    switch (f->step) {
    case 2:
        list = sc_draw_card_pict(list, f->x - 11, f->y - 15, 0, 0xFFFFFFFF, &f->ent, res_get(0x8001), res_get(0x8002), res_get(0x8032));
        break;
    }
    return list;
}

#ifdef NON_MATCHING /* forma de dk_slot_pos (o rascunho dava frames = step: são 6 na grade e 1 na lista; tab u32); 8 palavras: v0/v1 trocados no resto da coluna da grade, como em dk_slot_pos */
/* screen position of position pos of list tab shown in pane side; scroll the pane towards it when
   asked (returns 0 then) */
int sc_slot_pos(float *out, int side, unsigned int tab, int pos, int scroll)
{
    ScPane *p = &scv.pane[side];
    ScLayout *lay = &p->lay;
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

    if (scv.pane[side].list->tab != tab)
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
    if (scroll) {
        switch (sc.kind) {
        case 0:
            if (tab == 0)
                scroll = 0;
            break;
        case 1:
            if (tab == 0)
                scroll = 0;
            break;
        case 2:
            if (tab == 0)
                scroll = 0;
            break;
        }
    }
    if (!off) {
        top = p->top;
        if (pos < top) {
            if (scroll == 0) {
                off = 1;
            } else {
                p->top = top - (unsigned short)step;
                sc_flight->frames = frames;
                return 0;
            }
        } else if (top + count <= pos) {
            if (scroll == 0) {
                off = 1;
            } else {
                p->top = top + (unsigned short)step;
                sc_flight->frames = frames;
                return 0;
            }
        }
    }
    if (off) {
        tp = sc_tab_pos(tab);
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
        out[0] = (int)(lay->x + lay->grid_x + (d % cols) * (unsigned short)lay->cell_w);
        out[1] = (int)(lay->y + lay->grid_y + (d / cols) * (unsigned short)lay->cell_h);
        break;
    case 1:
        out[0] = lay->x + lay->row_x;
        cols = lay->y + lay->row_y;
        out[1] = (int)(cols + (unsigned short)lay->row_h * (pos - p->top));
        if (scv.mode < 4 && side == 1)
            out[1] += 1.0f;
        break;
    }
    out[0] += 11.0f;
    out[1] += 15.0f;
    return 1;
}
#endif

/* position the card id lands at in list tab */
int sc_dst_pos(int tab, unsigned short id)
{
    ScList *l = &sc.list[tab];
    int sep = 1;
    int i;
    int k;

    switch (sc.kind) {
    case 0:
        if (tab == 0)
            sep = 0;
        break;
    case 1:
        sep = 0;
        break;
    case 2:
        if (tab == 0)
            sep = 0;
        break;
    }
    if (!sep) {
        i = sc_list_find(l, id);
        if (i >= 0) {
            for (k = 0; k < l->count; k++) {
                if (i == l->order[k])
                    return k;
            }
        }
    }
    return l->count;
}

/* messages of popup 2 */
unsigned short sc_popup_msgs[6] = { 0, 0x22, 0x23, 0x84, 0x29, 0 };

/* opens popup id: 4 search/filter result (arg = count), 3 quit confirmation, 2 message arg */
void sc_popup_open(int id, int arg)
{
    MenuWidget *m = &sc_popup;
    MenuItemDef *d = &sc_popup.def;
    unsigned char buttons = 1;

    memset(m, 0, 0x120);
    memset(sc_popup_text, 0, 0x800);
    switch ((unsigned short)id) {
    case 2:
        wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), sc_popup_msgs[(unsigned short)arg]));
        m->cancel_blocked = 1;
        break;
    case 3:
        switch (sc.kind) {
        case 0:
            wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), 0x97));
            break;
        case 1:
            if ((unsigned int)sc.list[1].total < 10)
                wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), 0x9A));
            else
                wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), 0x98));
            break;
        case 2:
            wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), 0x99));
            break;
        }
        m->initial = 4;
        buttons = 2;
        m->cancel_resets = 1;
        break;
    case 4:
        if ((unsigned short)arg <= 0)
            wcscpy(sc_popup_text, table_get_entry(res_get(0x8033), 0x89));
        else
            wsprintf(sc_popup_text, table_get_entry(res_get(0x8033), 0x81), (unsigned short)arg);
        d->auto_size = 1;
        m->initial = 1;
        d->max_h = 0x50;
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
    d->text = sc_popup_text;
    d->open_anim = 2;
    d->close_anim = 2;
    m->heap = sc.heap;
    m->no_cancel = 1;
    m->buttons = buttons;
    d->style = 0;
    d->pad_b = 0;
    menu_open(m);
    d->x = (0x1E0 - d->w) / 2;
    d->y = (0x110 - d->h) / 2;
    scv.popup = id;
}


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
