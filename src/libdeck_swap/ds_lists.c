// @range 0x08804E2C-0x08805E58
// @asm 0x0880556C-0x088056C8
// @data 0x0881A000-0x0881A014
/* The three card lists of the swap screen (main deck, side deck, extra deck), built from the deck
   the duel manager passed, the partners' cards flagged fixed; entries added and removed one copy at
   a time, totals, cards per attribute, reset/clear, and the deck written back (ids, the partners'
   cards, "new" marks seen). */
#include "libdeck_swap.h"

int inv_get_state(unsigned short id);
void inv_set_state2(unsigned short id);
void sysstate_blob_set(const void *src);
void cfg_set_bits_1_0_2(int v);
int ds_mode_for(unsigned short panes, int narrow, unsigned short mode_a, unsigned short mode_b);
void ds_list_sort(DsList *l, unsigned short sort, unsigned short dir);
void ds_layout_scroll(short keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
void ds_list_total(DsList *l);
void ds_attr_count_update(void);
unsigned short ds_list_build_main(DsList *l, RecipeData *d);
unsigned short ds_list_build_side(DsList *l, RecipeData *d);
unsigned short ds_list_build_extra(DsList *l, RecipeData *d);

void ds_list_alloc(DsList *l, unsigned short n)
{
    l->ents = heap_alloc(ds.heap, n * 6);
    l->order = heap_alloc(ds.heap, n * 2);
}

unsigned short (*ds_list_builders[3])(DsList *l, RecipeData *d) = {
    ds_list_build_main, ds_list_build_side, ds_list_build_extra,
};
unsigned short ds_list_caps[4] = { 125, 30, 125, 0 };

void ds_lists_init(void)
{
    DsList *l;
    int i;

    for (i = 0; i < 3; i++) {
        if (ds_list_builders[i] != 0) {
            l = &ds.list[i];
            ds_list_alloc(l, ds_list_caps[i]);
            l->tab = i;
            l->n = ds_list_builders[i](l, ds.deck);
            ds_list_total(l);
            l->count = l->n;
            ds_list_sort(l, 1, 0);
        }
    }
    ds_attr_count_update();
}

/* the cards of one part of the deck; the copies a partner holds (kind of partners[i]) are fixed */
#define DS_LIST_BUILD(name, cnt, ids, kind)                                           \
unsigned short name(DsList *l, RecipeData *d)                                         \
{                                                                                     \
    unsigned short n = d->cnt;                                                        \
    int i;                                                                            \
    int k;                                                                            \
    int j;                                                                            \
    unsigned short id;                                                                \
    unsigned short key;                                                               \
    DsEnt *e;                                                                         \
                                                                                      \
    for (i = 0; i < n; i++) {                                                         \
        id = d->ids[i];                                                               \
        key = ds_sort_key(id, res_get(0x8031));                                       \
        e = &l->ents[i];                                                              \
        e->id = id;                                                                   \
        e->key = key;                                                                 \
        e->count = 1;                                                                 \
        e->marked = 0;                                                                \
        e->fixed = 0;                                                                 \
        l->order[i] = i;                                                              \
    }                                                                                 \
    k = ds.n_partners;                                                                \
    for (i = 0; i < k; i++) {                                                         \
        if (ds.partners[i] == kind) {                                                 \
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

DS_LIST_BUILD(ds_list_build_main, n_main, main_ids, 1)
DS_LIST_BUILD(ds_list_build_side, n_side, side_ids, 3)
DS_LIST_BUILD(ds_list_build_extra, n_extra, extra_ids, 2)

void ds_lists_free(void)
{
    int i;
    DsList *l;

    for (i = 0; i < 3; i++) {
        l = &ds.list[i];
        if (l->order != 0) {
            heap_set_level(ds.heap, l->order);
            l->order = 0;
        }
        if (l->ents != 0) {
            heap_set_level(ds.heap, l->ents);
            l->ents = 0;
        }
    }
}

/* copies of id in l: free ones in the low half, fixed ones in the high half */
unsigned int ds_list_count_id(DsList *l, unsigned short id)
{
    unsigned short free = 0;
    unsigned short fixed = 0;
    int i;
    DsEnt *e;

    for (i = 0; i < l->n; i++) {
        e = &l->ents[i];
        if (e->id == id) {
            if (e->fixed)
                fixed += e->count;
            else
                free += e->count;
        }
    }
    return free | fixed << 16;
}

#ifdef NON_MATCHING /* forma igual; o mwcc troca os registradores s0/s1 do índice do memcpy e do deslocamento i * 6 (como sc_list_remove) */
/* takes one copy of entry i out of l (the whole entry when it was the last one) */
int ds_list_remove(DsList *l, int i)
{
    DsEnt *e;
    int k;
    int count;
    int found;
    unsigned short *o;
    int j;

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
    ds_list_total(l);
    return 1;
}
#endif


void ds_list_add(DsList *l, DsEnt *src)
{
    unsigned short n;
    unsigned short k;

    memcpy(&l->ents[l->n], src, 6);
    n = l->n++;
    k = l->count++;
    l->order[k] = n;
    ds_list_total(l);
}

/* cards of the main deck per attribute icon; the side deck counted in slot 3 */
void ds_attr_count_update(void)
{
    int i;
    int k;
    DsList *l = &ds.list[0];

    memset(ds.attr_count, 0, sizeof(ds.attr_count));
    for (i = 0; i < l->n; i++) {
        k = ds_kind_icon(card_get_attribute_icon(l->ents[i].id));
        if (k >= 0)
            ds.attr_count[k]++;
    }
    l = &ds.list[1];
    ds.attr_count[3] += l->n;
}

/* rebuilds every list from the deck (after a cancel) */
void ds_lists_reset(void)
{
    DsList *l;
    int i;

    for (i = 0; i < 3; i++) {
        if (ds_list_builders[i] != 0) {
            l = &ds.list[i];
            l->tab = i;
            l->n = ds_list_builders[i](l, ds.deck);
            ds_list_total(l);
            l->count = l->n;
            ds_list_sort(l, 1, 0);
            l->u18 = 0;
            l->u1a = 0;
            l->u1c_lo = 0;
            l->u1c_hi = 0;
            l->u1d_lo = 0;
        }
    }
    ds_attr_count_update();
    dsv.pane[0].top = 0;
    dsv.pane[0].cx = 0;
    dsv.pane[0].cy = 0;
    dsv.pane[1].top = 0;
    dsv.pane[1].cx = 0;
    dsv.pane[1].cy = 0;
}

/* moves the main deck into the extra list */
void ds_lists_clear(void)
{
    DsList *src = &ds.list[0];
    DsList *dst = &ds.list[2];
    int i;
    DsEnt *d;
    unsigned short c;
    unsigned short n;
    DsPane *p;
    unsigned short view;
    unsigned short grid;

    for (i = 0; i < src->n; i++) {
        d = &dst->ents[dst->n];
        memcpy(d, &src->ents[i], 6);
        d->count = 1;
        n = dst->n++;
        c = dst->count++;
        dst->order[c] = n;
    }
    src->u1c_lo = 0;
    src->u1c_hi = 0;
    src->n = 0;
    src->total = 0;
    src->u1d_lo = 0;
    src->count = 0;
    src->u18 = 0;
    src->u1a = 0;
    ds_attr_count_update();
    ds_list_total(src);
    ds_list_total(dst);
    p = &dsv.pane[0];
    p->top = 0;
    p->cx = 0;
    p->cy = 0;
    p = &dsv.pane[1];
    view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
    grid = p->list_mode == 0 ? p->lay.cols : 1;
    ds_layout_scroll(1, p->list->u1a, (grid & 0xFF) | (view & 0xFF) << 8, p->list->count, &p->top, &p->cx, &p->cy);
}

void ds_list_total(DsList *l)
{
    int i;

    l->total = 0;
    for (i = 0; i < l->n; i++)
        l->total += l->ents[i].count;
}

/* the words and the layout are saved; with save, the main and extra decks are written back into
   the deck (the partners' cards found again by id, the "new" cards seen) */
void ds_lists_commit(int save)
{
    RecipeData *d = ds.deck;
    DsList *l;
    int i;
    int j;
    DsEnt *e;

    sysstate_blob_set(ds.words);
    cfg_set_bits_1_0_2(ds_mode_for(dsv.mode > 1 ? 2 : 1, dsv.mode < 4, dsv.pane[0].list_mode, dsv.pane[1].list_mode));
    if (save == 0)
        return;
    for (i = 0; i < ds.n_partners; i++) {
        if ((unsigned int)(ds.partners[i] - 1) < 2)
            ds.partners[i] = 0;
    }
    l = &ds.list[0];
    for (i = 0; i < l->n; i++) {
        e = &l->ents[i];
        d->main_ids[i] = e->id;
        if (e->fixed) {
            for (j = 0; j < ds.n_partners; j++) {
                if (ds.partners[j] == 0 && e->id == partner_id_get(j)) {
                    ds.partners[j] = 1;
                    break;
                }
            }
        } else if (inv_get_state(e->id) == 1 && e->marked) {
            inv_set_state2(e->id);
        }
    }
    d->n_main = l->n;
    l = &ds.list[2];
    for (i = 0; i < l->n; i++) {
        e = &l->ents[i];
        d->extra_ids[i] = e->id;
        if (e->fixed) {
            for (j = 0; j < ds.n_partners; j++) {
                if (ds.partners[j] == 0 && e->id == partner_id_get(j)) {
                    ds.partners[j] = 2;
                    break;
                }
            }
        } else if (inv_get_state(e->id) == 1 && e->marked) {
            inv_set_state2(e->id);
        }
    }
    d->n_extra = l->n;
}
