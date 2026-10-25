// @range 0x08804000-0x088060B0
// @asm 0x08804C9C-0x08804ED4
// @asm 0x08805900-0x08805CF0
// @data 0x08822A00-0x08822A24
/* The card lists of the editor: the deck being built (main, side, extra), the owned cards (trunk) and
   the cards rented to the partner, allocated from dk_heap with the capacity of each tab; entries added
   and removed one copy at a time, visible order, totals, the per-attribute counters, and the deck
   saved to / loaded from a sysstate record. */
#include "libdeck.h"

int deck_card_free_count_a(unsigned short id);
int deck_count(int which);
unsigned short deck_get_card(int which, int i);
int partner_count_get(void);
unsigned short partner_id_get(int i);
int partner_state_get(int i);

unsigned short dk_sort_key(unsigned short id, void *tab);
void dk_list_sort(DkList *l, int reset, int keep);
void dk_list_filter_reset(DkList *l);
int dk_kind_icon(unsigned short icon);
void dk_layout_scroll(int keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
int sceRtcGetCurrentClockLocalTime(void *time);
void deck_clear(void);
void deck_set_card(int which, int i, unsigned short id);
void deck_set_count(int which, unsigned int n);
void partner_id_set(int i, unsigned short v);
void partner_state_set(int i, unsigned short v);
void partner_count_set(unsigned short v);
int inv_get_state(unsigned short id);
void inv_set_state2(unsigned short id);
void deck_name_copy_in(const void *src);
void sysstate_blob_set(const void *src);
void cfg_set_bits_1_0_2(int v);
int dk_mode_for(unsigned short panes, int narrow, unsigned short mode_a, unsigned short mode_b);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
void dk_list_order_insert(DkList *l, int i, int pos);

unsigned short dk_list_build_deck1(DkList *l);
unsigned short dk_list_build_deck2(DkList *l);
unsigned short dk_list_build_deck3(DkList *l);
unsigned short dk_list_build_owned(DkList *l);
unsigned short dk_list_build_rented(DkList *l);
void dk_list_copy(DkList *dst, DkList *src, int alloc);


unsigned short (*dk_list_builders[6])(DkList *l) = {
    dk_list_build_deck1, dk_list_build_deck3, dk_list_build_deck2, dk_list_build_owned, 0, dk_list_build_rented,
};

unsigned short dk_list_caps[6] = { 0x55, 0x23, 0x14, 0x993, 0, 3 };

void dk_list_alloc(DkList *l, unsigned short tab)
{
    unsigned short n = dk_list_caps[tab];

    if (n > 0) {
        l->ents = heap_alloc(dk_heap, n * 6);
        l->order = heap_alloc(dk_heap, n << 1);
    }
}

void dk_lists_init(void)
{
    int i;
    unsigned short (**b)(DkList *);
    DkList *l;

    for (i = 0, b = dk_list_builders, l = dk_lists; i < 6; i++, b++, l++) {
        if (*b != 0) {
            dk_list_alloc(l, i);
            l->tab = i;
            l->n = (*b)(l);
            dk_list_total(l);
            l->count = l->n;
            dk_list_sort(l, 1, 0);
        }
    }
    l = &dk_lists[4];
    dk_list_copy(l, &dk_lists[2], 1);
    dk_list_total(l);
    l->tab = 4;
    dk_list_sort(l, 1, 0);
    dk_list_filter_reset(&dk_lists[3]);
    dk_attr_count_update();
}

/* owned cards (free copies), counting the kinds owned at least once */
unsigned short dk_list_build_owned(DkList *l)
{
    unsigned short n = 0;
    int id;
    int c;
    DkEnt *e;

    dk_deck.owned_kinds = 0;
    id = 0xFA7;
    do {
        c = deck_card_free_count_a(id);
        if (card_get_flag31(id) && c > 0) {
            unsigned short key = dk_sort_key(id, dk_res_get(0x8031));
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
            dk_deck.owned_kinds++;
        id++;
    } while (id < 0x1B2E);
    return n;
}

#define DK_LIST_BUILD_DECK(name, which)                                              \
unsigned short name(DkList *l)                                                        \
{                                                                                     \
    unsigned short n = deck_count(which);                                             \
    int i;                                                                            \
    int k;                                                                            \
    int j;                                                                            \
    unsigned short id;                                                                \
    unsigned short key;                                                               \
    DkEnt *e;                                                                         \
                                                                                      \
    for (i = 0; i < n; i++) {                                                         \
        id = deck_get_card(which, i);                                                 \
        key = dk_sort_key(id, dk_res_get(0x8031));                               \
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
DK_LIST_BUILD_DECK(dk_list_build_deck1, 1)
DK_LIST_BUILD_DECK(dk_list_build_deck2, 2)
DK_LIST_BUILD_DECK(dk_list_build_deck3, 3)

/* the partner's rental cards that are not in one of the three decks */
unsigned short dk_list_build_rented(DkList *l)
{
    int total = partner_count_get();
    int n = 0;
    int i;
    unsigned short id;
    unsigned short key;
    DkEnt *e;

    for (i = 0; i < total; i++) {
        switch (partner_state_get(i)) {
        case 1:
        case 2:
        case 3:
            break;
        default:
            id = partner_id_get(i);
            key = dk_sort_key(id, dk_res_get(0x8031));
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

void dk_list_copy(DkList *dst, DkList *src, int alloc)
{
    if (alloc) {
        if (dst->order != 0)
            heap_set_level(dk_heap, dst->order);
        dst->order = heap_alloc(dk_heap, 0x28);
    }
    dst->ents = src->ents;
    dst->n = src->n;
    dst->count = src->count;
    memcpy(dst->order, src->order, src->count << 1);
}

void dk_lists_free(void)
{
    int i;
    DkList *l;

    for (i = 0, l = dk_lists; i < 6; i++, l++) {
        if (l->order != 0) {
            heap_set_level(dk_heap, l->order);
            l->order = 0;
        }
        if (i != 4 && l->ents != 0) {
            heap_set_level(dk_heap, l->ents);
            l->ents = 0;
        }
    }
}

int dk_list_find(DkList *l, int id)
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
int dk_list_count_id(DkList *l, unsigned short id, int same_name)
{
    int c = 0;
    int i;
    DkEnt *e;

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
int dk_list_count_fixed(DkList *l, unsigned short id, unsigned short *count)
{
    unsigned short c = 0;
    int fixed = 0;
    int i;
    DkEnt *e;

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
unsigned short dk_deck_count_forbidden(void)
{
    unsigned short n = 0;
    int i;
    DkList *l;

    l = &dk_lists[0];
    for (i = 0; i < l->n; i++) {
        if (card_get_limit(l->ents[i].id) == 0)
            n++;
    }
    l = &dk_lists[2];
    for (i = 0; i < l->n; i++) {
        if (card_get_limit(l->ents[i].id) == 0)
            n++;
    }
    l = &dk_lists[1];
    for (i = 0; i < l->n; i++) {
        if (card_get_limit(l->ents[i].id) == 0)
            n++;
    }
    return n;
}

/* the list shown alongside l: the extra deck and its copy in tab 4 */
#define DK_LIST_TWIN(l) ((l)->tab == 4 ? &dk_lists[2] : (l)->tab == 2 ? &dk_lists[4] : 0)

#ifdef NON_MATCHING /* forma igual; o mwcc troca os registradores s0/s1 do índice do memcpy e do deslocamento i * 6 (como sc_list_remove) */
/* takes one copy of entry i out of l (the whole entry when it was the last one) */
int dk_list_remove(DkList *l, int i)
{
    DkEnt *e;
    int k;
    int count;
    int found;
    int j;

    unsigned short *o;
    DkList *p;

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
            p = &dk_lists[4];
            break;
        case 4:
            p = &dk_lists[2];
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
            dk_list_total(p);
        }
    }
    dk_list_total(l);
    return 1;
}
#endif

/* adds the copies of src to l at visible position pos (-1: the end); the trunk merges copies */
void dk_list_add(DkList *l, DkEnt *src, int pos)
{
    int i;
    DkEnt *e;
    DkList *p;
    unsigned short n;
    unsigned short k;

    switch (l->tab) {
    case 3:
        i = dk_list_find(l, src->id);
        if (i >= 0) {
            e = &l->ents[i];
            e->count = e->count + src->count;
            e->marked = e->marked | src->marked;
            dk_list_total(l);
            dk_list_order_insert(l, i, pos);
            return;
        }
        break;
    }
    memcpy(&l->ents[l->n], src, 6);
    dk_list_order_insert(l, l->n++, pos);
    dk_list_total(l);
    p = 0;
    switch (l->tab) {
    case 2:
        p = &dk_lists[4];
        break;
    case 4:
        p = &dk_lists[2];
        break;
    }
    if (p != 0) {
        n = p->n++;
        k = p->count++;
        p->order[k] = n;
        dk_list_total(p);
    }
}

/* puts entry i at visible position pos of l (moves it when already visible; pos out of range keeps
   it where it is, or appends it) */
void dk_list_order_insert(DkList *l, int i, int pos)
{
    unsigned short *tmp;
    int n;
    int keep;
    int done;
    int k;
    unsigned short v;

    tmp = heap_alloc(dk_heap, (l->count + 1) * 2);
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
    heap_set_level(dk_heap, tmp);
}

/* cards of the deck per attribute icon; the side deck counted in slot 3 */
void dk_attr_count_update(void)
{
    int i;
    int k;
    DkList *l = &dk_lists[0];

    memset(dk_deck.attr_count, 0, sizeof(dk_deck.attr_count));
    for (i = 0; i < l->n; i++) {
        k = dk_kind_icon(card_get_attribute_icon(l->ents[i].id));
        if (k >= 0)
            dk_deck.attr_count[k]++;
    }
    l = &dk_lists[1];
    dk_deck.attr_count[3] += l->n;
}

/* rebuilds every list from sysstate (after a cancel) */
void dk_lists_reset(void)
{
    int i;
    unsigned short (**b)(DkList *);
    DkList *l;

    for (i = 0, b = dk_list_builders, l = dk_lists; i < 6; i++, b++, l++) {
        if (*b != 0) {
            l->tab = i;
            l->n = (*b)(l);
            dk_list_total(l);
            l->count = l->n;
            dk_list_sort(l, 1, 0);
            l->u18 = 0;
            l->u1a = 0;
            l->u1c_lo = 0;
            l->u1c_hi = 0;
            l->u1d_lo = 0;
        }
    }
    dk_list_filter_reset(&dk_lists[3]);
    l = &dk_lists[4];
    dk_list_copy(l, &dk_lists[2], 1);
    dk_list_total(l);
    l->tab = 4;
    dk_list_sort(l, 1, 0);
    l->u1c_lo = 0;
    l->u1c_hi = 0;
    l->u1d_lo = 0;
    l->u18 = 0;
    l->u1a = 0;
    dk_attr_count_update();
    dkv.pane[0].top = 0;
    dkv.pane[0].cx = 0;
    dkv.pane[0].cy = 0;
    dkv.pane[1].top = 0;
    dkv.pane[1].cx = 0;
    dkv.pane[1].cy = 0;
}

/* moves the deck (main, side, extra) back into the trunk; rented copies go to the rented list */
void dk_deck_clear(void)
{
    int k;
    int i;
    DkEnt *e;
    DkList *l;
    DkList *dst;
    int j;
    DkEnt *d;
    unsigned short n;
    unsigned short c;
    DkPane *p;
    unsigned short view;
    unsigned short grid;

    for (k = 0, l = dk_lists; k < 3; k++, l++) {
        for (i = 0; i < l->n; i++) {
            e = &l->ents[i];
            if (e->fixed) {
                dst = &dk_lists[5];
                memcpy(&dst->ents[dst->n], e, 6);
                n = dst->n++;
                c = dst->count++;
                dst->order[c] = n;
            } else {
                dst = &dk_lists[3];
                j = dk_list_find(dst, e->id);
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
    l = &dk_lists[4];
    l->n = 0;
    l->u1c_lo = 0;
    l->u1d_lo = 0;
    l->total = 0;
    l->count = 0;
    l->u1c_hi = 0;
    l->u18 = 0;
    l->u1a = 0;
    dk_attr_count_update();
    dk_list_total(&dk_lists[3]);
    dk_list_total(&dk_lists[5]);
    p = &dkv.pane[0];
    p->top = 0;
    p->cx = 0;
    p->cy = 0;
    p = &dkv.pane[1];
    view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
    grid = p->list_mode == 0 ? p->lay.cols : 1;
    dk_layout_scroll(1, p->list->u1a, (grid & 0xFF) | (view & 0xFF) << 8, p->list->count, &p->top, &p->cx, &p->cy);
}

void dk_list_total(DkList *l)
{
    int i;

    l->total = 0;
    for (i = 0; i < l->n; i++)
        l->total += l->ents[i].count;
}

/* writes the deck into saved deck slot (0..49): ids, name and the time */
void dk_deck_save(int slot)
{
    SaveRec *r;
    RecipeData *d;
    DkList *l;
    int i;

    if (slot < 0 || slot >= 50)
        return;
    r = sysstate_rec_get(slot);
    d = &r->recipe;
    l = &dk_lists[0];
    for (i = 0; i < l->n; i++)
        d->main_ids[i] = l->ents[i].id;
    d->n_main = l->n;
    l = &dk_lists[2];
    for (i = 0; i < l->n; i++)
        d->extra_ids[i] = l->ents[i].id;
    d->n_extra = l->n;
    l = &dk_lists[1];
    for (i = 0; i < l->n; i++)
        d->side_ids[i] = l->ents[i].id;
    d->n_side = l->n;
    wcscpy(r->name, (unsigned short *)dk_deck_name);
    sceRtcGetCurrentClockLocalTime(r->unk44);
    r->used = 1;
}

#ifdef NON_MATCHING /* forma igual; nos laços do extra e do side o mwcc troca os registradores do índice e do ponteiro dos ids (s1/s3) */
/* one card of a loaded deck into dst: taken from the trunk, or else from the rented list (marked
   fixed) */
static inline void dk_deck_take(DkList *dst, unsigned short id)
{
    DkEnt e;
    DkList *l;
    int k;

    memset(&e, 0, 6);
    e.id = id;
    e.key = dk_sort_key(id, dk_res_get(0x8031));
    e.count = 1;
    l = (DkList *)(char *)&dk_lists[3];
    k = dk_list_find(l, id);
    if (k >= 0) {
        dk_list_remove(l, k);
        dk_list_add(dst, &e, -1);
    } else {
        e.fixed = 1;
        l = (DkList *)(char *)&dk_lists[5];
        k = dk_list_find(l, id);
        if (k >= 0) {
            dk_list_remove(l, k);
            dk_list_add(dst, &e, -1);
        }
    }
}

/* replaces the deck with recipe d (name, or none) */
void dk_deck_load(RecipeData *d, unsigned short *name)
{
    if (name != 0)
        wcscpy((unsigned short *)dk_deck_name, name);
    else
        memset(dk_deck_name, 0, 0x40);
    dk_deck_clear();
    {
        DkList *dst = &dk_lists[0];
        unsigned int i;
        unsigned short id;

        for (i = 0; i < d->n_main; i++) {
            id = d->main_ids[i];
            dk_deck_take(dst, id);
        }
        dk_list_filter_reset(dst);
        dk_list_sort(dst, 1, 0);
    }
    {
        unsigned short id;
        unsigned int i;
        DkList *dst = &dk_lists[2];

        for (i = 0; i < d->n_extra; i++) {
            id = d->extra_ids[i];
            dk_deck_take(dst, id);
        }
        dk_list_filter_reset(dst);
        dk_list_sort(dst, 1, 0);
    }
    {
        DkList *dst = &dk_lists[4];

        dk_list_filter_reset(dst);
        dk_list_sort(dst, 1, 0);
    }
    {
        unsigned short id;
        unsigned int i;
        DkList *dst = &dk_lists[1];

        for (i = 0; i < d->n_side; i++) {
            id = d->side_ids[i];
            dk_deck_take(dst, id);
        }
        dk_list_filter_reset(dst);
        dk_list_sort(dst, 1, 0);
    }
    dk_undo_count = 0;
    dk_attr_count_update();
}
#endif

/* one list written back as saved deck <which>: rented copies go back to the partner (state which),
   the "new" marks of the cards seen are cleared */
#define DK_DECK_STORE(which, ent)                                                    \
    n = 0;                                                                           \
    for (i = 0; i < l->n; i++) {                                                     \
        e = ent;                                                                     \
        deck_set_card(which, n, e->id);                                              \
        n++;                                                                         \
        if (e->fixed) {                                                              \
            partner_id_set(k, e->id);                                                \
            partner_state_set(k, which);                                             \
            k++;                                                                     \
        } else if (inv_get_state(e->id) == 1 && e->marked) {                         \
            inv_set_state2(e->id);                                                   \
        }                                                                            \
    }                                                                                \
    deck_set_count(which, n)

/* writes the lists back to sysstate on exit: the three decks (main, extra, side in its shown
   order), the partner's rental cards, the "new" marks, the deck name and the pane layout */
void dk_deck_store(void)
{
    DkList *l;
    int k;
    int i;
    int n;
    DkEnt *e;

    l = &dk_lists[5];
    k = l->total;
    for (i = 0; i < k; i++) {
        partner_id_set(i, l->ents[i].id);
        partner_state_set(i, 4);
    }
    deck_clear();
    l = &dk_lists[0];
    DK_DECK_STORE(1, &l->ents[i]);
    l = &dk_lists[2];
    DK_DECK_STORE(2, &l->ents[i]);
    l = &dk_lists[1];
    if (l->filter != 0)
        dk_list_filter_reset(l);
    dk_list_sort(l, 1, 0);
    DK_DECK_STORE(3, &l->ents[l->order[i]]);
    partner_count_set(k);
    l = &dk_lists[3];
    for (i = 0; i < l->n; i++) {
        e = &l->ents[i];
        if (inv_get_state(e->id) == 1 && e->marked)
            inv_set_state2(e->id);
    }
    deck_name_copy_in(dk_deck_name);
    sysstate_blob_set(dk_profile);
    cfg_set_bits_1_0_2(dk_mode_for(dkv.mode > 1 ? 2 : 1, dkv.mode < 4, dkv.pane[0].list_mode, dkv.pane[1].list_mode));
}
