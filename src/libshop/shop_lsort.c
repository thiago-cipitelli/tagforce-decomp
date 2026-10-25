// @range 0x08817DD4-0x08818554
// @asm 0x08818368-0x08818554
// @data 0x088204F4-0x08820504
/* Sorting of the bought list: the sort value of a line for each of the thirteen keys, a stable
   qsort in either direction, and the value shown in the list row for the current key. */
#include "libshop.h"

typedef struct ShopSortEnt {    /* one line while sorting, 8 bytes */
    unsigned short item;        /* index into shop.items */
    unsigned short pos;         /* position before sorting (keeps the sort stable) */
    int value;                  /* -2 = no value, sorted last */
} ShopSortEnt;

/* order of the spell and trap icons (card_get_prop5_hi) under the "type" key */
unsigned char shop_spell_order[8] = { 9, 9, 2, 1, 4, 0, 3, 0 };
unsigned char shop_trap_order[8] = { 0x13, 0x0A, 0x13, 0x13, 0x0B, 0x13, 0x13, 0x00 };

int inv_get_state(unsigned short id);
int inv_get_flag7(unsigned short id);
int card_get_prop6_hi(unsigned short id);
int card_get_sort(unsigned short id);
int card_get_sort_position(unsigned short id);
int card_get_attribute_icon(unsigned short id);
int card_get_limit(unsigned short id);
int card_get_prop5_hi(unsigned short id);
int card_get_level(unsigned short id);
int card_get_atk_q(unsigned short id);
int card_get_def_q(unsigned short id);
int card_get_type(unsigned short id);
int card_get_kind(unsigned short id);

int shop_sort_value(ShopItem *it, unsigned short key);
int shop_sort_cmp_up(const void *pa, const void *pb);
int shop_sort_cmp_down(const void *pa, const void *pb);

void shop_list_sort(unsigned short key, unsigned short dir)
{
    unsigned short n;
    ShopSortEnt *e;
    int i;
    unsigned short id;

    n = shop.n_items;
    e = heap_alloc(shop.heap, n << 3);
    for (i = 0; i < n; i++) {
        id = shop.order[i];
        e[i].item = id;
        e[i].pos = i;
        e[i].value = shop_sort_value(&shop.items[id], key);
    }
    if (dir == 0)
        qsort(e, (int)n, sizeof(ShopSortEnt), shop_sort_cmp_up);
    else if (dir == 1)
        qsort(e, (int)n, sizeof(ShopSortEnt), shop_sort_cmp_down);
    for (i = 0; i < n; i++)
        shop.order[i] = e[i].item;
    if (e != 0)
        heap_set_level(shop.heap, e);
    shop.sort = key;
    shop.order_dir = dir;
}

int shop_sort_cmp_up(const void *pa, const void *pb)
{
    const ShopSortEnt *a = pa;
    const ShopSortEnt *b = pb;

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

int shop_sort_cmp_down(const void *pa, const void *pb)
{
    const ShopSortEnt *a = pa;
    const ShopSortEnt *b = pb;

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

/* 1 pack, 2 new, 3 rarity, 4 marked, 5 card number, 6 attribute, 7 level, 8 ATK, 9 DEF, 10 type,
   11 kind, 12 spell/trap type, 13 limit */
int shop_sort_value(ShopItem *it, unsigned short key)
{
    int v;

    switch (key) {
    case 1:
        v = it->pack;
        break;
    case 2:
        v = inv_get_state(it->id) != 0;
        break;
    case 3:
        v = -card_get_prop6_hi(it->id);
        break;
    case 4:
        v = -inv_get_flag7(it->id);
        break;
    case 5:
        return card_get_sort(it->id);
    case 6:
        v = card_get_attribute_icon(it->id);
        break;
    case 13:
        v = card_get_limit(it->id);
        break;
    default:
        switch (card_get_attribute_icon(it->id)) {
        case 7:
            if (key == 12)
                return shop_spell_order[card_get_prop5_hi(it->id)];
            break;
        case 8:
            if (key != 12)
                break;
            return shop_trap_order[card_get_prop5_hi(it->id)];
        default:
            switch (key) {
            case 7:
                return 0xFFFF - card_get_level(it->id);
            case 8:
                v = card_get_atk_q(it->id);
                if (v == 0xFFFF)
                    return 0x10000;
                return 0xFFFF - v;
            case 9:
                v = card_get_def_q(it->id);
                if (v == 0xFFFF)
                    return 0x10000;
                return 0xFFFF - v;
            case 10:
                return card_get_type(it->id);
            case 11:
                return card_get_kind(it->id);
            }
            break;
        }
        v = -2;
        break;
    }
    return v;
}


#ifdef NON_MATCHING /* mesma lógica; o original guarda as cópias do primeiro lhu de cada caso e repete desvios (492 bytes contra 468) */
/* value shown in the list row for the sort key, -2 = none */
int shop_item_key(ShopItem *it, unsigned short key)
{
    int v;

    switch (key) {
    case 1:
        v = it->pack;
        break;
    case 2:
        v = inv_get_state(it->id) == 0;
        break;
    case 3:
        return card_get_prop6_hi(it->id);
    case 4:
        v = inv_get_flag7(it->id);
        break;
    case 5:
        v = card_get_sort_position(it->id);
        break;
    case 6:
        v = card_get_attribute_icon(it->id);
        break;
    case 13:
        v = card_get_limit(it->id);
        break;
    default:
        switch (card_get_attribute_icon(it->id)) {
        case 7:
        case 8:
            if (key == 12) {
                v = card_get_prop5_hi(it->id);
                if (v == 0)
                    v = 7;
            } else {
                v = -2;
            }
            break;
        default:
            switch (key) {
            case 7:
                v = card_get_level(it->id);
                break;
            case 8:
                v = card_get_atk_q(it->id);
                if (v == 0xFFFF)
                    v = -1;
                break;
            case 9:
                v = card_get_def_q(it->id);
                if (v == 0xFFFF)
                    v = -1;
                break;
            case 10:
                v = card_get_type(it->id);
                break;
            case 11:
                v = card_get_kind(it->id);
                break;
            default:
                v = -2;
                break;
            }
            break;
        }
        break;
    }
    return v;
}
#endif
