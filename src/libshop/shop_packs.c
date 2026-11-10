// @range 0x0880A960-0x0880C238
// @asm 0x0880AD08-0x0880B5FC
/* The 48 pack kinds: which are on sale (unlocked by story progress, days, decks and collection),
   their prices and messages, the completion of each, and the opening of a pack: the rarity of the
   best card is drawn from a 20-slot cycle saved per pack (14 rare, 4 super, 2 ultra), the other cards
   are commons, and three late packs favour cards the player does not own yet. */
#include "libshop.h"

int sysflag_get2(int id);
void sysflag_set2(int id, int v);
int sysflag_test(unsigned int bit);
unsigned char sysflag_rec_get_byte0(int id);
void sysflag_rec_set_byte0(int id, unsigned char v);
unsigned short *sysflag_rec_get_data(int id);
void sysflag_rec_set_u16(int id, int i, unsigned short v);
int inv_get_state(unsigned short id);
int inv_get_count(unsigned short id);
int card_get_flag31(int id);
void deck_cur_get_u16_0(short *out);
unsigned short deck_slot_get_value(int i);
unsigned short inv_collection_permille(void);
int day_next(void);
unsigned char *sysstate2_get_ptr_51c(void);
int value_to_tier(int v);
unsigned int rand_next_alias(void);

typedef struct ShopFlagEnt {    /* entry of a recipe/character table: bit 0 counts */
    unsigned char on;
    unsigned char pad;
} ShopFlagEnt;

extern ShopFlagEnt shop_tab_a[40];     /* 0x088200C8 */
extern ShopFlagEnt shop_tab_b[83];     /* 0x08820118 */
extern ShopFlagEnt shop_tab_c[118];    /* 0x088201C0 */
extern short shop_rarity_slots[4];     /* 0x088202AC: 0, 14, 4, 2 */

void shop_packs_unlock(void);
unsigned short shop_pack_complete(ShopCardList *l);
int shop_card_in(unsigned short id, unsigned short *list, unsigned short n);

/* refreshes the state, completion and stock of every pack kind */
void shop_packs_refresh(void)
{
    int i;

    shop_packs_unlock();
    for (i = 0; i < 48; i++) {
        shop_pack_defs[i].state = sysflag_get2(i);
        shop_pack_defs[i].complete = shop_pack_complete(shop_pack_defs[i].lists);
        shop.stock[i] = sysflag_rec_get_byte0(i);
    }
}

/* writes the stock back to the save */
void shop_stock_save(void)
{
    int i;

    for (i = 0; i < 48; i++)
        sysflag_rec_set_byte0(i, shop.stock[i]);
}

/* name of pack `pack` */
unsigned short *shop_pack_name(unsigned short pack)
{
    int *t = (int *)shop_str_get(0xC002);

    return (unsigned short *)((char *)t + t[0] + t[pack * 3 + 1]);
}

/* the clerk's line about pack `pack` (two clerks) */

unsigned short *shop_pack_msg(unsigned short pack, unsigned short alt)
{
    int *t = (int *)shop_str_get(0xC002);
    int *e = t + 1;
    char *base = (char *)t + t[0];
    int i = pack * 3;

    i++;
    if (alt == 1)
        i++;
    return (unsigned short *)(base + e[i]);
}

int shop_pack_can_buy(unsigned short pack)
{
    return shop_pack_defs[pack].state;
}

unsigned short shop_pack_price(unsigned short pack)
{
    return shop_pack_defs[pack].price;
}

int shop_pack_n_cards(unsigned short pack)
{
    return shop_pack_defs[pack].n_cards;
}

unsigned short shop_pack_rank(unsigned short pack)
{
    return shop_pack_defs[pack].rank;
}

unsigned char shop_pack_get_complete(unsigned short pack)
{
    return shop_pack_defs[pack].complete;
}

/* percent of the cards of the four lists the player owns */
unsigned short shop_pack_complete(ShopCardList *lists)
{
    unsigned short own;
    unsigned short all;
    int k;
    int i;
    ShopCardList *l;

    own = 0;
    all = 0;
    l = lists;
    for (k = 0; k < 4; k++, l++) {
        unsigned short *id = l->ids;
        int n = l->n;
        if (id) {
            for (i = 0; i < n; i++, id++) {
                if (inv_get_state(*id))
                    own++;
            }
            all += n;
        }
    }
    if (all <= 0)
        return 0;
    return own * 100 / all;
}

/* packs bought this visit */
int shop_bought_total(void)
{
    int n = 0;
    int i;

    for (i = 0; i < 48; i++)
        n += shop.bought[i];
    return n;
}

#ifdef NON_MATCHING /* 39 palavras: contadores do 1º laço (a2/a3), $s de c/j nos laços das não possuídas, pool/cnt e j do fim */
/* the cards of one pack of kind `pack` into `out` */
void shop_pack_open(int pack, unsigned short *out)
{
    ShopPackDef *d = &shop_pack_defs[pack];
    ShopCardList *own = 0;
    int n = 0;
    unsigned short left[4];
    unsigned short slots[20];
    unsigned short *save;
    int i, k, m;
    int rarity;
    ShopCardList *l;
    int cnt;
    unsigned short *ids;
    int r;

    left[0] = shop_rarity_slots[0];
    left[1] = shop_rarity_slots[1];
    left[2] = shop_rarity_slots[2];
    left[3] = shop_rarity_slots[3];
    save = sysflag_rec_get_data(pack);
    for (k = 0, i = 0; k < 20; i++, k++) {
        if (save[k] <= 0 || save[k] >= 4)
            break;
        left[save[k]]--;
    }
    if (i >= 20) {
        memset(save, 0, 40);
        left[1] = 14;
        left[2] = 4;
        left[3] = 2;
    }
    memset(slots, 0, sizeof(slots));
    m = 0;
    for (i = 0; i < (unsigned short)left[3]; i++)
        slots[m++] = 3;
    for (i = 0; i < (unsigned short)left[2]; i++)
        slots[m++] = 2;
    for (i = 0; i < (unsigned short)left[1]; i++)
        slots[m++] = 1;
    rarity = slots[(int)rand_next_alias() % m];
    sysflag_rec_set_u16(pack, 20 - m, rarity);
    switch (pack) {
    case 15: case 28: case 47:
      own = heap_alloc(shop.heap, 0x20);
      if (own != 0) {
        for (k = 0; k < 4; k++) {
            if (d->lists[k].n > 0)
                own[k].ids = heap_alloc(shop.heap, d->lists[k].n * 2);
        }
        for (k = 0; k < 4; k++) {
            own[k].n = 0;
            if (own[k].ids) {
                int j;
                int c = 0;
                for (j = 0; j < d->lists[k].n; j++) {
                    unsigned short id = d->lists[k].ids[j];
                    if (inv_get_count(id) <= 0)
                        own[k].ids[c++] = id;
                }
                own[k].n = c;
            }
        }
        for (k = 0; k < 4; k++) {
            if (own[k].ids && own[k].n <= 0) {
                int j;
                int c = 0;
                for (j = 0; j < d->lists[k].n; j++) {
                    unsigned short id = d->lists[k].ids[j];
                    if (inv_get_count(id) < 3)
                        own[k].ids[c++] = id;
                }
                own[k].n = c;
            }
        }
      }
    }
    for (; rarity >= 0; rarity--) {
        if (own && own[rarity].ids && own[rarity].n > 0 && (int)rand_next_alias() % 10 < 5) {
            ids = own[rarity].ids;
            cnt = own[rarity].n;
        } else {
            ids = d->lists[rarity].ids;
            cnt = d->lists[rarity].n;
        }
        if (cnt > 0) {
            out[0] = ids[(int)rand_next_alias() % cnt];
            n++;
            break;
        }
    }
    l = d->lists;
    ids = l->ids;
    cnt = l->n;
    k = 0;
    if (ids == 0 || cnt == 0) {
        int j;
        for (j = 1; 4 > j; j++) {
            ids = l[j].ids;
            cnt = l[j].n;
            if (ids && cnt > 0) {
                k = j;
                break;
            }
        }
    }
    while (n < d->n_cards) {
        unsigned short id;
        if (own && own[k].ids && own[k].n > 0 && (int)rand_next_alias() % 10 < 3)
            id = own[k].ids[(int)rand_next_alias() % own[k].n];
        else
            id = ids[(int)rand_next_alias() % cnt];
        if (card_get_flag31(id) && !shop_card_in(id, out, n))
            out[n++] = id;
    }
    r = out[0];
    out[0] = out[n - 1];
    out[n - 1] = r;
    if (own) {
        for (k = 0; k < 4; k++) {
            if (own[k].ids)
                heap_set_level(shop.heap, own[k].ids);
        }
        heap_set_level(shop.heap, own);
    }
}
#endif

/* 1 if `id` is among the first n of `list` */
int shop_card_in(unsigned short id, unsigned short *list, unsigned short n)
{
    int i;

    for (i = 0; i < n; i++, list++) {
        if (id == *list)
            return 1;
    }
    return 0;
}

typedef struct ShopDeckFlags {         /* deck_cur_get_u16_0 (DeckFlags of libtitle.h) */
    unsigned short b0 : 1;
    unsigned short kind : 3;
    unsigned short chara : 8;
    unsigned short rest : 4;
} ShopDeckFlags;

/* unlocks the pack kinds whose condition is met */
void shop_packs_unlock(void)
{
    ShopDeckFlags deck;
    int best;
    int k;
    int i;

    if (sysflag_get2(6) != 2 || sysflag_get2(7) != 2 || sysflag_get2(8) != 2) {
        unsigned short own = 0, all = 0;
        for (i = 3; i <= 5; i++) {
            int j;
            ShopCardList *l;
            for (k = 0, l = shop_pack_defs[i].lists; k < 4; k++, l++) {
                unsigned short *id = l->ids;
                int n = l->n;
                if (id) {
                    for (j = 0; j < n; j++, id++) {
                        if (inv_get_state(*id))
                            own++;
                    }
                    all += n;
                }
            }
        }
        if (own >= all * 80 / 100) {
            sysflag_set2(6, 2);
            sysflag_set2(7, 2);
            sysflag_set2(8, 2);
        }
    }
    deck_cur_get_u16_0((short *)&deck);
    if (deck.kind == 1 && shop.arg == 1)
        sysflag_set2(9, 2);
    else
        sysflag_set2(9, 1);
    if (deck.kind == 1 && shop.arg == 2)
        sysflag_set2(10, 2);
    else
        sysflag_set2(10, 1);
    if (deck.kind == 1 && shop.arg == 3)
        sysflag_set2(11, 2);
    else
        sysflag_set2(11, 1);
    if (deck.kind == 1 && shop.arg == 4)
        sysflag_set2(12, 2);
    else
        sysflag_set2(12, 1);
    if (deck.kind == 1 && shop.arg == 5)
        sysflag_set2(13, 2);
    else
        sysflag_set2(13, 1);
    if (deck.kind == 1 && shop.arg == 6)
        sysflag_set2(14, 2);
    else
        sysflag_set2(14, 1);
    if (day_next() >= 5)
        sysflag_set2(16, 2);
    if (day_next() >= 10)
        sysflag_set2(17, 2);
    if (day_next() >= 15)
        sysflag_set2(18, 2);
    if (day_next() >= 20)
        sysflag_set2(19, 2);
    if (day_next() >= 25)
        sysflag_set2(20, 2);
    if (day_next() >= 30)
        sysflag_set2(21, 2);
    if (day_next() >= 35)
        sysflag_set2(22, 2);
    if (day_next() >= 40)
        sysflag_set2(23, 2);
    if (day_next() >= 45)
        sysflag_set2(24, 2);
    if (day_next() >= 50)
        sysflag_set2(0x1C, 2);
    if (sysflag_get2(0x1A) != 2) {
        unsigned char *x = sysstate2_get_ptr_51c();
        unsigned short got = 0, all = 0;
        for (i = 0; i < 40; i++) {
            if (shop_tab_a[i].on & 1) {
                all++;
                got++;
            }
        }
        for (i = 0; i < 83; i++) {
            if (shop_tab_b[i].on & 1) {
                all++;
                if (x[0xCC + i] > 0)
                    got++;
            }
        }
        for (i = 0; i < 118; i++) {
            if (shop_tab_c[i].on & 1) {
                all++;
                if (x[0x120 + i] > 0)
                    got++;
            }
        }
        if (got >= all * 50 / 100)
            sysflag_set2(0x1A, 2);
    }
    if (deck.kind == 1) {
        best = 0;
        for (i = 2; i <= 8; i++) {
            if (deck_slot_get_value(i) > best)
                best = deck_slot_get_value(i);
        }
    }
    if (sysflag_get2(31))
        sysflag_set2(31, 1);
    if (deck.kind == 1) {
        if (best == deck_slot_get_value(2) && best > 0)
            sysflag_set2(31, 2);
    } else if (deck.chara == 2) {
        sysflag_set2(31, 2);
    }
    if (sysflag_get2(32))
        sysflag_set2(32, 1);
    if (deck.kind == 1) {
        if (best == deck_slot_get_value(3) && best > 0)
            sysflag_set2(32, 2);
    } else if (deck.chara == 3) {
        sysflag_set2(32, 2);
    }
    if (sysflag_get2(33))
        sysflag_set2(33, 1);
    if (deck.kind == 1) {
        if (best == deck_slot_get_value(4) && best > 0)
            sysflag_set2(33, 2);
    } else if (deck.chara == 4) {
        sysflag_set2(33, 2);
    }
    if (sysflag_get2(34))
        sysflag_set2(34, 1);
    if (deck.kind == 1) {
        if (best == deck_slot_get_value(5) && best > 0)
            sysflag_set2(34, 2);
    } else if (deck.chara == 5) {
        sysflag_set2(34, 2);
    }
    if (sysflag_get2(35))
        sysflag_set2(35, 1);
    if (deck.kind == 1) {
        if (best == deck_slot_get_value(6) && best > 0)
            sysflag_set2(35, 2);
    } else if (deck.chara == 6) {
        sysflag_set2(35, 2);
    }
    if (sysflag_get2(36))
        sysflag_set2(36, 1);
    if (deck.kind == 1) {
        if (best == deck_slot_get_value(7) && best > 0)
            sysflag_set2(36, 2);
    } else if (deck.chara == 7) {
        sysflag_set2(36, 2);
    }
    if (sysflag_get2(37))
        sysflag_set2(37, 1);
    if (deck.kind == 1) {
        if (best == deck_slot_get_value(8) && best > 0)
            sysflag_set2(37, 2);
    } else if (deck.chara == 8) {
        sysflag_set2(37, 2);
    }
    if (sysflag_get2(0x26))
        sysflag_set2(0x26, 1);
    if (deck.kind > 1 && value_to_tier(deck.chara) == 2)
        sysflag_set2(0x26, 2);
    if (sysflag_get2(0x27))
        sysflag_set2(0x27, 1);
    if (deck.kind > 1 && value_to_tier(deck.chara) == 3)
        sysflag_set2(0x27, 2);
    if (sysflag_get2(0x28))
        sysflag_set2(0x28, 1);
    if (deck.kind - 2 < 2u)
        sysflag_set2(0x28, 2);
    if (sysflag_get2(0x2B)) {
        sysflag_set2(0x2B, 1);
        if (shop_clerk.alt == 0)
            sysflag_set2(0x2B, 2);
    }
    if (sysflag_get2(0x2C)) {
        sysflag_set2(0x2C, 1);
        if (shop_clerk.alt == 1)
            sysflag_set2(0x2C, 2);
    }
    if (shop.code_done)
        sysflag_set2(0x2D, 2);
    if (sysflag_test(1))
        sysflag_set2(0x2E, 2);
    if (inv_collection_permille() >= 900)
        sysflag_set2(0x2F, 2);
}
