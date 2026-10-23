// @range 0x088099CC-0x0880AD2C
// @asm 0x0880A39C-0x0880A528
/* The card lists of the two panes: built from the caller's ids, the owned cards (trade or
   conversion counts), the rental cards and the partner's rental list; entries added and removed
   one copy at a time, totals, and the result written back (caller's list, sysstate, flags). */
#include "libselect_card.h"

int deck_card_free_count_a(unsigned short id);
int deck_card_free_count_b(unsigned short id);
void sysstate_blob_set(const void *src);
void cfg_set_bits_1_0_2(int v);
void inv_set_state2(unsigned short id);
void partner_count_set(unsigned short v);
void partner_id_set(int i, unsigned short v);
void partner_state_set(int i, unsigned short v);
void sc_list_total(ScList *l);
void sc_list_sort(ScList *l, int reset, int keep);
void sc_list_filter_reset(ScList *l);
int sc_mode_for(unsigned short panes, int narrow, unsigned short mode_a, unsigned short mode_b);
void sc_list_add(ScList *l, ScEnt *src);
unsigned short sc_list_build_ids(ScList *l, int count, unsigned short *ids, int merge);
unsigned short sc_list_build_trade(ScList *l);
unsigned short sc_list_build_conv(ScList *l);
unsigned short sc_list_build_rental(ScList *l);
unsigned short sc_list_build_partner(ScList *l);
void sc_layout_scroll(int grid, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);

void sc_list_alloc(ScList *l, unsigned short n)
{
    l->ents = heap_alloc(sc.heap, n * 6);
    l->order = heap_alloc(sc.heap, n * 2);
}

/* both lists for the kind; the caller's ids go to the second one */
void sc_lists_build(int count, unsigned short *ids)
{
    int i;
    int n;
    int fav = -1;
    ScList *l;
    ScList *q;

    switch (sc.kind) {
    case 0:
        l = &sc.list[0];
        sc_list_alloc(l, 0x993);
        l->n = sc_list_build_rental(l);
        l = &sc.list[1];
        sc_list_alloc(l, 3);
        l->n = sc_list_build_partner(l);
        n = 2;
        break;
    case 1:
        q = &sc.list[1];
        sc_list_alloc(q, 99);
        q->n = sc_list_build_ids(q, count, ids, 0);
        l = &sc.list[0];
        sc_list_alloc(l, 0x993);
        l->n = sc_list_build_conv(l);
        n = 2;
        fav = 0;
        break;
    case 2:
        q = &sc.list[1];
        sc_list_alloc(q, 5);
        q->n = sc_list_build_ids(q, count, ids, 1);
        l = &sc.list[0];
        sc_list_alloc(l, 0x993);
        l->n = sc_list_build_trade(l);
        n = 2;
        fav = 0;
        break;
    }
    for (i = 0; i < n; i++) {
        l = &sc.list[i];
        l->tab = i;
        sc_list_total(l);
        l->count = l->n;
        sc_list_sort(l, 1, 0);
    }
    if (fav >= 0)
        sc_list_filter_reset(&sc.list[fav]);
}

/* the caller's ids: copies of the same card merged unless merge; ids without a sort key dropped
   (the original reuses the last entry looked at as the new slot) */
unsigned short sc_list_build_ids(ScList *l, int count, unsigned short *ids, int merge)
{
    unsigned short n = 0;
    int i;
    int j;
    unsigned short id;
    ScEnt *e;
    unsigned short key;

    if (ids == 0)
        return 0;
    for (i = 0; i < count; i++) {
        id = ids[i];
        if (merge == 0) {
            for (j = n - 1; j >= 0; j--) {
                e = &l->ents[j];
                if (e->id == id) {
                    e->count++;
                    break;
                }
            }
        }
        if (j == -1) {
            key = sc_sort_key(id, res_get(0x8031));
            if (key != 0) {
                e->id = id;
                e->key = key;
                e->count = 1;
                e->marked = 0;
                e->fixed = 0;
                l->order[n] = n;
                n++;
            }
        }
    }
    return n;
}

/* owned cards that can be traded */
unsigned short sc_list_build_trade(ScList *l)
{
    unsigned short n = 0;
    int id;
    int c;
    unsigned short key;
    ScEnt *e;

    id = 0xFA7;
    do {
        c = deck_card_free_count_b(id);
        if (card_get_flag31(id) && c > 0) {
            key = sc_sort_key(id, res_get(0x8031));
            e = &l->ents[n];
            if (key != 0) {
                e->id = id;
                e->key = key;
                e->count = c;
                e->marked = 0;
                e->fixed = 0;
                l->order[n] = n;
                n++;
            }
        }
        id++;
    } while (id < 0x1B2E);
    return n;
}

/* owned cards that can go in the conversion machine */
unsigned short sc_list_build_conv(ScList *l)
{
    unsigned short n = 0;
    int id;
    int c;
    unsigned short key;
    ScEnt *e;

    id = 0xFA7;
    do {
        c = deck_card_free_count_a(id);
        if (card_get_flag31(id) && c > 0) {
            key = sc_sort_key(id, res_get(0x8031));
            e = &l->ents[n];
            if (key != 0) {
                e->id = id;
                e->key = key;
                e->count = c;
                e->marked = 0;
                e->fixed = 0;
                l->order[n] = n;
                n++;
            }
        }
        id++;
    } while (id < 0x1B2E);
    return n;
}

/* every card owned at least once, one copy each, no rank */
unsigned short sc_list_build_rental(ScList *l)
{
    unsigned short n = 0;
    int id;
    unsigned short key;
    ScEnt *e;

    id = 0xFA7;
    do {
        if (card_get_flag31(id) && inv_get_bit0(id) > 0) {
            key = sc_sort_key(id, res_get(0x8031));
            e = &l->ents[n];
            if (key != 0) {
                e->id = id;
                e->key = key;
                e->count = 1;
                e->marked = 0;
                e->fixed = 0;
                e->rank = 4;
                l->order[n] = n;
                n++;
            }
        }
        id++;
    } while (id < 0x1B2E);
    return n;
}

/* the partner's rental cards (sysstate), fixed, with their rank */
unsigned short sc_list_build_partner(ScList *l)
{
    int total = partner_count_get();
    int n = 0;
    int i = 0;
    int st;
    unsigned short id;
    unsigned short key;
    ScEnt *e;

    for (; i < total; i++, n++) {
        st = partner_state_get(i);
        id = partner_id_get(i);
        key = sc_sort_key(id, res_get(0x8031));
        e = &l->ents[i];
        e->id = id;
        e->key = key;
        e->count = 1;
        e->marked = 0;
        e->fixed = 1;
        e->rank = st;
        l->order[i] = n;
    }
    return n;
}

void sc_lists_free(void)
{
    int i;
    ScList *l;

    for (i = 0; i < 2; i++) {
        l = &sc.list[i];
        if (l->order != 0) {
            heap_set_level(sc.heap, l->order);
            l->order = 0;
        }
        if (l->ents != 0) {
            heap_set_level(sc.heap, l->ents);
            l->ents = 0;
        }
    }
}

int sc_list_find(ScList *l, unsigned short id)
{
    int i;
    int v;

    for (i = 0; i < l->n; i++) {
        v = l->ents[i].id;
        if (v == id)
            return i;
    }
    return -1;
}

/* copies of id in l (and of the cards with the same name, when same_name) */
unsigned short sc_list_count_id(ScList *l, unsigned short id, int same_name)
{
    unsigned short c = 0;
    int i;
    ScEnt *e;

    for (i = 0; i < l->n; i++) {
        e = &l->ents[i];
        if (e->id == id)
            c += e->count;
        else if (same_name && card_same_name(id, e->id))
            c += e->count;
    }
    return c;
}

#ifdef NON_MATCHING /* forma igual; o mwcc troca os registradores s0/s1 do índice do memcpy e do deslocamento i * 6 */
/* takes one copy of entry i out of l (the whole entry when it was the last one) */
int sc_list_remove(ScList *l, int i)
{
    ScEnt *e;
    int k;
    int count;
    int found;
    unsigned short *o;
    int j;

    switch (sc.kind) {
    case 0:
        if (l->tab == 0)
            return 0;
        break;
    }
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
    }
    sc_list_total(l);
    return 1;
}
#endif

/* adds the copies of src to l: merged into the same card unless the kind keeps them apart */
void sc_list_add(ScList *l, ScEnt *src)
{
    int sep = 1;
    int i;
    int k;
    unsigned short *o;
    ScEnt *e;
    unsigned short n;

    switch (sc.kind) {
    case 0:
        if (l->tab == 0)
            return;
        break;
    case 1:
        sep = 0;
        break;
    case 2:
        if (l->tab == 0)
            sep = 0;
        break;
    }
    if (!sep) {
        i = sc_list_find(l, src->id);
        if (i >= 0) {
            e = &l->ents[i];
            e->count = e->count + src->count;
            e->marked = e->marked | src->marked;
            sc_list_total(l);
            for (k = 0; k < l->count; k++) {
                if (i == l->order[k])
                    return;
            }
            n = l->count++;
            l->order[n] = i;
            return;
        }
    }
    memcpy(&l->ents[l->n], src, 6);
    n = l->n++;
    k = l->count++;
    l->order[k] = n;
    sc_list_total(l);
}

/* rebuilds the lists from the caller's list (after a cancel) */
void sc_lists_reset(void)
{
    int i;
    int n;
    int fav = -1;
    ScList *l;
    ScList *q;

    switch (sc.kind) {
    case 0:
        l = &sc.list[0];
        l->n = sc_list_build_rental(l);
        l = &sc.list[1];
        l->n = sc_list_build_partner(l);
        n = 2;
        break;
    case 1:
        q = &sc.list[1];
        q->n = sc_list_build_ids(q, sc.n_in, sc.ids_in, 0);
        l = &sc.list[0];
        l->n = sc_list_build_conv(l);
        n = 2;
        fav = 0;
        break;
    case 2:
        q = &sc.list[1];
        q->n = sc_list_build_ids(q, sc.n_in, sc.ids_in, 1);
        l = &sc.list[0];
        l->n = sc_list_build_trade(l);
        n = 2;
        fav = 0;
        break;
    }
    for (i = 0; i < n; i++) {
        l = &sc.list[i];
        l->tab = i;
        sc_list_total(l);
        l->count = l->n;
        sc_list_sort(l, 1, 0);
        l->u18 = 0;
        l->u1a = 0;
        l->u1c_lo = 0;
        l->u1c_hi = 0;
        l->u1d_lo = 0;
    }
    if (fav >= 0)
        sc_list_filter_reset(&sc.list[fav]);
    scv.pane[0].top = 0;
    scv.pane[0].cx = 0;
    scv.pane[0].cy = 0;
    scv.pane[1].top = 0;
    scv.pane[1].cx = 0;
    scv.pane[1].cy = 0;
}

/* moves everything of the second list back into the first one */
void sc_lists_clear(void)
{
    ScList *src;
    ScList *dst;
    int i;
    ScPane *p;
    unsigned short view;
    unsigned short grid;

    switch (sc.kind) {
    case 0:
        src = &sc.list[1];
        dst = &sc.list[0];
        break;
    case 1:
        src = &sc.list[1];
        dst = &sc.list[0];
        break;
    case 2:
        src = &sc.list[1];
        dst = &sc.list[0];
        break;
    default:
        return;
    }
    for (i = 0; i < src->n; i++)
        sc_list_add(dst, &src->ents[i]);
    src->u1c_lo = 0;
    src->u1c_hi = 0;
    src->n = 0;
    src->u1d_lo = 0;
    src->total = 0;
    src->count = 0;
    src->u18 = 0;
    src->u1a = 0;
    sc_list_total(src);
    sc_list_total(dst);
    p = &scv.pane[0];
    p->top = 0;
    p->cx = 0;
    p->cy = 0;
    p = &scv.pane[1];
    view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
    grid = p->list_mode == 0 ? p->lay.cols : 1;
    sc_layout_scroll(1, p->list->u1a, (grid & 0xFF) | (view & 0xFF) << 8, p->list->count, &p->top, &p->cx, &p->cy);
}

void sc_list_total(ScList *l)
{
    int i;

    l->total = 0;
    for (i = 0; i < l->n; i++)
        l->total += l->ents[i].count;
}

/* writes the second list back: the caller's ids (marking the "new" cards as seen), or the
   partner's rental list; saves the blob and the pane layout */
void sc_lists_save(void)
{
    ScList *l;
    int n;
    int i;
    int k;
    ScEnt *e;

    switch (sc.kind) {
    case 0:
        l = &sc.list[1];
        partner_count_set(l->n);
        for (i = 0; i < l->n; i++) {
            e = &l->ents[i];
            partner_id_set(i, e->id);
            partner_state_set(i, e->rank);
        }
        break;
    case 1:
    case 2:
        if (sc.out_count != 0 && sc.out_ids != 0) {
            l = &sc.list[1];
            n = 0;
            for (i = 0; i < l->n; i++) {
                e = &l->ents[i];
                for (k = 0; k < e->count; k++)
                    sc.out_ids[n++] = e->id;
                if (inv_get_state(e->id) == 1 && e->marked)
                    inv_set_state2(e->id);
            }
            *sc.out_count = n;
            l = &sc.list[1];
            for (i = 0; i < l->n; i++) {
                e = &l->ents[i];
                if (inv_get_state(e->id) == 1 && e->marked)
                    inv_set_state2(e->id);
            }
        }
        break;
    }
    sysstate_blob_set(sc.blob);
    cfg_set_bits_1_0_2(sc_mode_for(scv.mode > 1 ? 2 : 1, scv.mode < 4, scv.pane[0].list_mode, scv.pane[1].list_mode));
}
