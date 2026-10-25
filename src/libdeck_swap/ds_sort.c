// @range 0x0880B750-0x0880BE1C
// @asm 0x0880BC8C-0x0880BE1C
// @data 0x0881A1E8-0x0881A1F8
/* Sorting a pane's visible order by one of the nine keys (stable, "no value" last), the key value
   of a card and the value shown for the key (same source as libselect_card sc_filter.c). */
#include "libdeck_swap.h"

typedef struct DsKey {                 /* qsort record */
    unsigned short idx;                /* entry index */
    unsigned short pos;                /* position before sorting */
    int value;                         /* +0x04 ds_sort_key_value, -2 = none */
} DsKey;

int ds_sort_key_value(DsEnt *e, unsigned short sort);
int ds_cmp_up(const void *pa, const void *pb);
int ds_cmp_down(const void *pa, const void *pb);

/* sorts the visible order of l by key sort, dir 0 up, 1 down */
void ds_list_sort(DsList *l, unsigned short sort, unsigned short dir)
{
    unsigned short n;
    DsKey *k;
    int i;
    unsigned short id;

    n = l->count;
    k = heap_alloc(ds.heap, n << 3);
    for (i = 0; i < n; i++) {
        id = l->order[i];
        k[i].idx = id;
        k[i].pos = i;
        k[i].value = ds_sort_key_value(&l->ents[id], sort);
    }
    if (dir == 0)
        qsort(k, (int)n, sizeof(DsKey), ds_cmp_up);
    else if (dir == 1)
        qsort(k, (int)n, sizeof(DsKey), ds_cmp_down);
    for (i = 0; i < n; i++)
        l->order[i] = k[i].idx;
    if (k != 0)
        heap_set_level(ds.heap, k);
    l->sort = sort;
    l->dir = dir;
}

int ds_cmp_up(const void *pa, const void *pb)
{
    const DsKey *a = pa;
    const DsKey *b = pb;

    if (a->value == b->value) {
        if (a->pos < b->pos)
            return -1;
        if (b->pos < a->pos)
            return 1;
    } else {
        if (a->value == -2)
            return 1;
        if (b->value == -2)
            return -1;
        if (a->value < b->value)
            return -1;
        if (b->value < a->value)
            return 1;
    }
    return 0;
}

int ds_cmp_down(const void *pa, const void *pb)
{
    const DsKey *a = pa;
    const DsKey *b = pb;

    if (a->value == b->value) {
        if (a->pos > b->pos)
            return -1;
        if (a->pos < b->pos)
            return 1;
    } else {
        if (a->value == -2)
            return 1;
        if (b->value == -2)
            return -1;
        if (a->value < b->value)
            return 1;
        if (b->value < a->value)
            return -1;
    }
    return 0;
}

/* order of the spell and trap icons (card_get_prop5_hi) under the "type" key */
unsigned char ds_spell_order[8] = { 9, 9, 2, 1, 4, 0, 3, 0 };
unsigned char ds_trap_order[8] = { 0x13, 0x0A, 0x13, 0x13, 0x0B, 0x13, 0x13, 0 };

/* value of key sort for a card (smaller sorts first), -2 none */
int ds_sort_key_value(DsEnt *e, unsigned short sort)
{
    int v;

    switch (sort) {
    case 1:
        return card_get_sort(e->id);
    case 2:
        v = card_get_attribute_icon(e->id);
        break;
    case 9:
        v = card_get_limit(e->id);
        break;
    default:
        switch (card_get_attribute_icon(e->id)) {
        case 7:
            if (sort == 8)
                return ds_spell_order[card_get_prop5_hi(e->id)];
            break;
        case 8:
            if (sort != 8)
                break;
            return ds_trap_order[card_get_prop5_hi(e->id)];
        default:
            switch (sort) {
            case 3:
                return 0xFFFF - card_get_level(e->id);
            case 4:
                v = card_get_atk_q(e->id);
                if (v == 0xFFFF)
                    return 0x10000;
                return 0xFFFF - v;
            case 5:
                v = card_get_def_q(e->id);
                if (v == 0xFFFF)
                    return 0x10000;
                return 0xFFFF - v;
            case 6:
                return card_get_type(e->id);
            case 7:
                return card_get_kind(e->id);
            }
            break;
        }
        v = -2;
        break;
    }
    return v;
}

#ifdef NON_MATCHING /* o original mantém os desvios até o fim de cada caso (b para outro b) e repete o lhu do id; a mesma função da libshop também não bateu */
/* the value shown for key sort (-1 = "?", -2 none) */
int ds_sort_value(DsEnt *e, unsigned short sort)
{
    int v;

    switch (sort) {
    case 1:
        return card_get_sort_position(e->id);
    case 2:
        v = card_get_attribute_icon(e->id);
        break;
    case 9:
        v = card_get_limit(e->id);
        break;
    default:
        switch (card_get_attribute_icon(e->id)) {
        case 7:
        case 8:
            if (sort == 8) {
                v = card_get_prop5_hi(e->id);
                if (v == 0)
                    v = 7;
                return v;
            }
            return -2;
        default:
            switch (sort) {
            case 3:
                return card_get_level(e->id);
            case 4:
                v = card_get_atk_q(e->id);
                if (v == 0xFFFF)
                    v = -1;
                return v;
            case 5:
                v = card_get_def_q(e->id);
                if (v == 0xFFFF)
                    v = -1;
                return v;
            case 6:
                return card_get_type(e->id);
            case 7:
                return card_get_kind(e->id);
            }
            break;
        }
        v = -2;
        break;
    }
    return v;
}
#endif
