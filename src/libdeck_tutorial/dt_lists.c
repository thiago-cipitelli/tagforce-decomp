// @range 0x0880B704-0x0880CDC8
// @asm 0x0880C2B0-0x0880C4E8
// @data 0x08821F28-0x08821F4C
/* The card lists of the editor (the code of libdeck dk_lists.c): the deck being built (main, side,
   extra), the owned cards (trunk) and the cards rented to the partner, allocated from the editor heap
   with the capacity of each tab; entries added and removed one copy at a time, visible order, totals
   and the per-attribute counters. */
#include "libdeck_tutorial.h"

int deck_card_free_count_a(unsigned short id);
int deck_count(int which);
unsigned short deck_get_card(int which, int i);
int partner_count_get(void);
unsigned short partner_id_get(int i);
int partner_state_get(int i);
int card_get_limit(int id);
int card_get_attribute_icon(int id);
int card_get_flag31(int id);
int card_same_name(int a, int b);
int inv_get_count(unsigned short id);
void *heap_alloc(void *h, int n);
int heap_set_level(void *h, void *p);
void *memset(void *, int, unsigned int);

unsigned short dt_sort_key(unsigned short id, void *tab);
void dt_list_sort(DtList *l, int reset, int keep);
void dt_list_filter_reset(DtList *l);
int dt_kind_icon(unsigned short icon);
void *dt_eres_get(unsigned short id);
void dt_list_order_insert(DtList *l, int i, int pos);
void dt_list_total(DtList *l);
void dt_attr_count_update(void);
int dt_list_find(DtList *l, int id);
void dt_layout_scroll(int keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);

unsigned short dt_list_build_deck1(DtList *l);
unsigned short dt_list_build_deck2(DtList *l);
unsigned short dt_list_build_deck3(DtList *l);
unsigned short dt_list_build_owned(DtList *l);
unsigned short dt_list_build_rented(DtList *l);
void dt_list_copy(DtList *dst, DtList *src, int alloc);



unsigned short (*dt_list_builders[6])(DtList *l) = {
    dt_list_build_deck1, dt_list_build_deck3, dt_list_build_deck2, dt_list_build_owned, 0, dt_list_build_rented,
};

unsigned short dt_list_caps[6] = { 0x55, 0x23, 0x14, 0x993, 0, 3 };

void dt_list_alloc(DtList *l, unsigned short tab)
{
    unsigned short n = dt_list_caps[tab];

    if (n > 0) {
        l->ents = heap_alloc(dts.heap, n * 6);
        l->order = heap_alloc(dts.heap, n << 1);
    }
}

/* owned cards (free copies), counting the kinds owned at least once */
unsigned short dt_list_build_owned(DtList *l)
{
    unsigned short n = 0;
    int id;
    int c;
    DtEnt *e;

    dts.n_owned = 0;
    id = 0xFA7;
    do {
        c = deck_card_free_count_a(id);
        if (card_get_flag31(id) && c > 0) {
            unsigned short key = dt_sort_key(id, dt_eres_get(0x8031));
            e = &l->ents[n];
            e->id = id;
            e->key = key;
            e->count = c;
            e->marked = 0;
            e->fixed = 0;
            l->order[n] = n;
            n++;
        }
        if (inv_get_count(id) > 0)
            dts.n_owned++;
        id++;
    } while (id < 0x1B2E);
    return n;
}

#define DK_LIST_BUILD_DECK(name, which)                                              \
unsigned short name(DtList *l)                                                        \
{                                                                                     \
    unsigned short n = deck_count(which);                                             \
    int i;                                                                            \
    int k;                                                                            \
    int j;                                                                            \
    unsigned short id;                                                                \
    unsigned short key;                                                               \
    DtEnt *e;                                                                         \
                                                                                      \
    for (i = 0; i < n; i++) {                                                         \
        id = deck_get_card(which, i);                                                 \
        key = dt_sort_key(id, dt_eres_get(0x8031));                               \
        e = &l->ents[i];                                                              \
        e->id = id;                                                                   \
        e->key = key;                                                                 \
        e->count = 1;                                                                 \
        e->marked = 0;                                                                \
        e->fixed = 0;                                                                 \
        l->order[i] = i;                                                              \
    }                                                                                 \
    k = partner_count_get();                                                          \
    for (i = 0; i < k; i++) {                                                         \
        if (partner_state_get(i) == which) {                                          \
            id = partner_id_get(i);                                                   \
            for (j = n - 1; j >= 0; j--) {                                            \
                e = &l->ents[j];                                                      \
                if (e->id == id && !e->fixed) {                                       \
                    e->fixed = 1;                                                     \
                    break;                                                            \
                }                                                                     \
            }                                                                         \
        }                                                                             \
    }                                                                                 \
    return n;                                                                         \
}

/* saved deck <which>, the copies rented to the partner flagged fixed */
DK_LIST_BUILD_DECK(dt_list_build_deck1, 1)
DK_LIST_BUILD_DECK(dt_list_build_deck2, 2)
DK_LIST_BUILD_DECK(dt_list_build_deck3, 3)

/* the partner's rental cards that are not in one of the three decks */
unsigned short dt_list_build_rented(DtList *l)
{
    int total = partner_count_get();
    int n = 0;
    int i;
    unsigned short id;
    unsigned short key;
    DtEnt *e;

    for (i = 0; i < total; i++) {
        switch (partner_state_get(i)) {
        case 1:
        case 2:
        case 3:
            break;
        default:
            id = partner_id_get(i);
            key = dt_sort_key(id, dt_eres_get(0x8031));
            e = &l->ents[n];
            e->id = id;
            e->key = key;
            e->count = 1;
            e->marked = 0;
            e->fixed = 1;
            l->order[n] = n;
            n++;
            break;
        }
    }
    return n;
}

void dt_list_copy(DtList *dst, DtList *src, int alloc)
{
    if (alloc) {
        if (dst->order != 0)
            heap_set_level(dts.heap, dst->order);
        dst->order = heap_alloc(dts.heap, 0x28);
    }
    dst->ents = src->ents;
    dst->n = src->n;
    dst->count = src->count;
    memcpy(dst->order, src->order, src->count << 1);
}

void dt_lists_free(void)
{
    int i;
    DtList *l;

    for (i = 0, l = dts.list; i < 6; i++, l++) {
        if (l->order != 0) {
            heap_set_level(dts.heap, l->order);
            l->order = 0;
        }
        if (i != 4 && l->ents != 0) {
            heap_set_level(dts.heap, l->ents);
            l->ents = 0;
        }
    }
}

int dt_list_find(DtList *l, int id)
{
    int i;
    int v;

    for (i = 0; i < l->n; i++) {
        v = l->ents[i].id;
        if (v == (unsigned short)id)
            return i;
    }
    return -1;
}

/* copies of id in l (and of the cards with the same name, when same_name) */
int dt_list_count_id(DtList *l, unsigned short id, int same_name)
{
    int c = 0;
    int i;
    DtEnt *e;

    for (i = 0; i < l->n; i++) {
        e = &l->ents[i];
        if (e->id == id)
            c = (unsigned short)(c + e->count);
        else if (same_name && card_same_name(id, e->id))
            c = (unsigned short)(c + e->count);
    }
    return c;
}

/* copies of id in l into *count; 1 when one of them is rented (fixed) */
int dt_list_count_fixed(DtList *l, unsigned short id, unsigned short *count)
{
    unsigned short c = 0;
    int fixed = 0;
    int i;
    DtEnt *e;

    for (i = 0; i < l->n; i++) {
        e = &l->ents[i];
        if (e->id == id) {
            c += e->count;
            if (e->fixed)
                fixed = 1;
        }
    }
    *count = c;
    return fixed;
}

/* forbidden cards (limit 0) in the deck */
unsigned short dt_deck_count_forbidden(void)
{
    unsigned short n = 0;
    int i;
    DtList *l;

    l = &dts.list[0];
    for (i = 0; i < l->n; i++) {
        if (card_get_limit(l->ents[i].id) == 0)
            n++;
    }
    l = &dts.list[2];
    for (i = 0; i < l->n; i++) {
        if (card_get_limit(l->ents[i].id) == 0)
            n++;
    }
    l = &dts.list[1];
    for (i = 0; i < l->n; i++) {
        if (card_get_limit(l->ents[i].id) == 0)
            n++;
    }
    return n;
}

/* the list shown alongside l: the extra deck and its copy in tab 4 */
#define DK_LIST_TWIN(l) ((l)->tab == 4 ? &dts.list[2] : (l)->tab == 2 ? &dts.list[4] : 0)

#ifdef NON_MATCHING /* forma igual; o mwcc troca os registradores s0/s1 do índice do memcpy e do deslocamento i * 6 (como sc_list_remove) */
/* takes one copy of entry i out of l (the whole entry when it was the last one) */
int dt_list_remove(DtList *l, int i)
{
    DtEnt *e;
    int k;
    int count;
    int found;
    int j;

    unsigned short *o;
    DtList *p;

    if (i < 0 || i >= l->n)
        return 0;
    e = &l->ents[i];
    if (e->count > 1) {
        e->count--;
    } else {
        count = l->count;
        found = 0;
        for (k = 0; k < count; k++) {
            o = &l->order[k];
            if (i == *o) {
                found = 1;
                count--;
            }
            if (found)
                *o = o[1];
            if (i < l->order[k])
                l->order[k]--;
        }
        if (found)
            l->count = count;
        l->n--;
        for (j = i; j < l->n; j++)
            memcpy(&l->ents[j], &l->ents[j + 1], 6);
        p = 0;
        switch (l->tab) {
        case 2:
            p = &dts.list[4];
            break;
        case 4:
            p = &dts.list[2];
            break;
        }
        if (p != 0) {
            count = p->count;
            found = 0;
            for (k = 0; k < count; k++) {
                o = &p->order[k];
                if (i == *o) {
                    found = 1;
                    count--;
                }
                if (found)
                    *o = o[1];
                if (i < p->order[k])
                    p->order[k]--;
                if (found)
                    p->count = count;
            }
            p->n--;
            dt_list_total(p);
        }
    }
    dt_list_total(l);
    return 1;
}
#endif

/* adds the copies of src to l at visible position pos (-1: the end); the trunk merges copies */
void dt_list_add(DtList *l, DtEnt *src, int pos)
{
    int i;
    DtEnt *e;
    DtList *p;
    unsigned short n;
    unsigned short k;

    switch (l->tab) {
    case 3:
        i = dt_list_find(l, src->id);
        if (i >= 0) {
            e = &l->ents[i];
            e->count = e->count + src->count;
            e->marked = e->marked | src->marked;
            dt_list_total(l);
            dt_list_order_insert(l, i, pos);
            return;
        }
        break;
    }
    memcpy(&l->ents[l->n], src, 6);
    dt_list_order_insert(l, l->n++, pos);
    dt_list_total(l);
    p = 0;
    switch (l->tab) {
    case 2:
        p = &dts.list[4];
        break;
    case 4:
        p = &dts.list[2];
        break;
    }
    if (p != 0) {
        n = p->n++;
        k = p->count++;
        p->order[k] = n;
        dt_list_total(p);
    }
}

/* puts entry i at visible position pos of l (moves it when already visible; pos out of range keeps
   it where it is, or appends it) */
void dt_list_order_insert(DtList *l, int i, int pos)
{
    unsigned short *tmp;
    int n;
    int keep;
    int done;
    int k;
    unsigned short v;

    tmp = heap_alloc(dts.heap, (l->count + 1) * 2);
    n = 0;
    keep = 0;
    done = 0;
    memset(tmp, 0, (l->count + 1) * 2);
    if (pos < 0 || l->count >= pos)
        keep = 1;
    for (k = 0; k < l->count; k++) {
        v = l->order[k];
        if (i == v) {
            if (keep) {
                tmp[n++] = v;
                done = 1;
            }
        } else {
            if (n == pos) {
                tmp[n++] = i;
                done = 1;
            }
            tmp[n++] = l->order[k];
        }
    }
    if (!done)
        tmp[n++] = i;
    memcpy(l->order, tmp, n << 1);
    l->count = n;
    heap_set_level(dts.heap, tmp);
}

/* cards of the deck per attribute icon; the side deck counted in slot 3 */
void dt_attr_count_update(void)
{
    int i;
    int k;
    DtList *l = &dts.list[0];

    memset(dts.n_type, 0, sizeof(dts.n_type));
    for (i = 0; i < l->n; i++) {
        k = dt_kind_icon(card_get_attribute_icon(l->ents[i].id));
        if (k >= 0)
            dts.n_type[k]++;
    }
    l = &dts.list[1];
    dts.n_type[3] += l->n;
}

/* rebuilds every list from sysstate (after a cancel) */
void dt_lists_reset(void)
{
    int i;
    unsigned short (**b)(DtList *);
    DtList *l;

    for (i = 0, b = dt_list_builders, l = dts.list; i < 6; i++, b++, l++) {
        if (*b != 0) {
            l->tab = i;
            l->n = (*b)(l);
            dt_list_total(l);
            l->count = l->n;
            dt_list_sort(l, 1, 0);
            l->u18 = 0;
            l->u1a = 0;
            l->u1c_lo = 0;
            l->u1c_hi = 0;
            l->u1d_lo = 0;
        }
    }
    dt_list_filter_reset(&dts.list[3]);
    l = &dts.list[4];
    dt_list_copy(l, &dts.list[2], 1);
    dt_list_total(l);
    l->tab = 4;
    dt_list_sort(l, 1, 0);
    l->u1c_lo = 0;
    l->u1c_hi = 0;
    l->u1d_lo = 0;
    l->u18 = 0;
    l->u1a = 0;
    dt_attr_count_update();
    dtv.pane[0].top = 0;
    dtv.pane[0].cx = 0;
    dtv.pane[0].cy = 0;
    dtv.pane[1].top = 0;
    dtv.pane[1].cx = 0;
    dtv.pane[1].cy = 0;
}

/* moves the deck (main, side, extra) back into the trunk; rented copies go to the rented list */
void dt_deck_clear(void)
{
    int k;
    int i;
    DtEnt *e;
    DtList *l;
    DtList *dst;
    int j;
    DtEnt *d;
    unsigned short n;
    unsigned short c;
    DtPane *p;
    unsigned short view;
    unsigned short grid;

    for (k = 0, l = dts.list; k < 3; k++, l++) {
        for (i = 0; i < l->n; i++) {
            e = &l->ents[i];
            if (e->fixed) {
                dst = &dts.list[5];
                memcpy(&dst->ents[dst->n], e, 6);
                n = dst->n++;
                c = dst->count++;
                dst->order[c] = n;
            } else {
                dst = &dts.list[3];
                j = dt_list_find(dst, e->id);
                if (j >= 0) {
                    d = &dst->ents[j];
                    d->count = d->count + e->count;
                    d->marked = e->marked | d->marked;
                } else {
                    d = &dst->ents[dst->n];
                    memcpy(d, e, 6);
                    d->count = 1;
                    n = dst->n++;
                    c = dst->count++;
                    dst->order[c] = n;
                }
            }
        }
        l->n = 0;
        l->total = 0;
        l->count = 0;
        l->u18 = 0;
        l->u1a = 0;
        l->u1c_lo = 0;
        l->u1c_hi = 0;
        l->u1d_lo = 0;
    }
    l = &dts.list[4];
    l->n = 0;
    l->u1c_lo = 0;
    l->u1d_lo = 0;
    l->total = 0;
    l->count = 0;
    l->u1c_hi = 0;
    l->u18 = 0;
    l->u1a = 0;
    dt_attr_count_update();
    dt_list_total(&dts.list[3]);
    dt_list_total(&dts.list[5]);
    p = &dtv.pane[0];
    p->top = 0;
    p->cx = 0;
    p->cy = 0;
    p = &dtv.pane[1];
    view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
    grid = p->list_mode == 0 ? p->lay.cols : 1;
    dt_layout_scroll(1, p->list->u1a, (grid & 0xFF) | (view & 0xFF) << 8, p->list->count, &p->top, &p->cx, &p->cy);
}

void dt_list_total(DtList *l)
{
    int i;

    l->total = 0;
    for (i = 0; i < l->n; i++)
        l->total += l->ents[i].count;
}

/* writes the deck into saved deck slot (0..49): ids, name and the time */
