// @range 0x088F1190-0x088F1E5C
// @asm 0x088F1514-0x088F15E0
/* Cards that can be taken from the graveyard or zone F, the hand (adding, removing, searching,
   counting, shuffling) and the level of a card in the work list after reductions. */
#include "duel.h"
#include "card_db.h"

int duel_count_active(int id);
int duel_count_tributable_type(unsigned int p, unsigned int type);
int duel_has_st_12be(unsigned int p);
int duel_is_cur_card(int id);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_counter(unsigned int p, int z, int id);
int duel_card_check_9148(unsigned int p, int id);
int duel_card_dfac8(unsigned int p, int id, int a);
void duel_c_push(unsigned int p, DuelCard *c);
void duel_card_copy(DuelCard *dst, DuelCard *src);
int duel_card_same(DuelCard *a, DuelCard *b);
void duel_bzero(void *p, int n);
void duel_slot_foreach_effect(unsigned int p, int z, int (*fn)(DuelEffect *e, int *ctx), int *ctx);
int duel_card_e5b88(int id, int a);
int duel_rand(int n);
unsigned int duel_hand_locked(unsigned int p);

/* Card i of the graveyard of player q may be used by player p. */
int duel_grave_card_usable(unsigned int p, unsigned int q, int i)
{
    DuelCard *c = &duel.players[q & 1].grave[i];
    unsigned short id = c->id;

    if (card_is_in_group_1a58(id) == 0) {
        return 0;
    }
    if (card_attribute_icon_in_2_3(id) != 0) {
        if (c->b14 == 0) {
            return 0;
        }
        if (card_is_attribute6(id) && duel_has_st_12be(p) == 0) {
            return 0;
        }
        switch (id) {
        case 0x14FC:
            if (duel_card_check_9148(p, 0x1414) == 0) {
                return 0;
            }
            break;
        case 0x1578:
            if (duel.players[p & 1].flags >> 17 & 1) {
                return 0;
            }
            if (duel_slot_has_effect(p, 11, 0x12F3)) {
                return 0;
            }
            break;
        }
    } else {
        if (duel_card_dfac8(p, id, 0) == 0) {
            return 0;
        }
    }
    switch (id) {
    case 0x14B5:
    case 0x1571:
    case 0x15B1:
    case 0x15B4:
    case 0x15F9:
    case 0x15FA:
    case 0x17C7:
    case 0x17C8:
    case 0x1820:
        return 0;
    case 0x14D5:
        if (duel_count_tributable_type(p, 1) == 0) {
            return 0;
        }
        break;
    }
    return 1;
}

int duel_zone_f_card_usable(unsigned int p, unsigned int q, int i)
{
    DuelCard *c = &duel.players[q & 1].zone_f[i];
    unsigned short id = c->id;

    if (card_is_in_group_1a58(id) == 0) {
        return 0;
    }
    if (card_attribute_icon_in_2_3(id) != 0) {
        if (c->b14 == 0) {
            return 0;
        }
        if (card_is_attribute6(id) && duel_has_st_12be(p) == 0) {
            return 0;
        }
        switch (id) {
        case 0x14FC:
            if (duel_card_check_9148(p, 0x1414) == 0) {
                return 0;
            }
            break;
        case 0x1578:
            if (duel.players[p & 1].flags >> 17 & 1) {
                return 0;
            }
            if (duel_slot_has_effect(p, 11, 0x12F3)) {
                return 0;
            }
            break;
        }
    } else {
        if (duel_card_dfac8(p, id, 0) == 0) {
            return 0;
        }
    }
    return 1;
}

#ifdef NON_MATCHING /* same code; mwcc schedules the multiply and the n_hand pointer later */
/* Puts card c in the hand of player p (attribute-icon 3 cards go to the field instead). */
void duel_hand_add(unsigned int p, DuelCard *c)
{
    DuelCard *dst = &duel.players[p & 1].hand[duel.players[p & 1].n_hand];
    int id = c->id;

    if (id != 0 && card_is_attribute9((unsigned short)id) == 0) {
        if (card_get_attribute_icon(c->id) == 3) {
            duel_c_push(c->owner, c);
        } else {
            duel_card_copy(dst, c);
            duel.players[p & 1].n_hand++;
        }
    }
}
#endif

int duel_hand_remove_card(unsigned int p, DuelCard *c)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < duel.players[p].n_hand; i++) {
        if (duel_card_same(&duel.players[p].hand[i], c)) {
            duel.players[p].n_hand--;
            for (; i < duel.players[p].n_hand; i++) {
                duel_card_copy(&duel.players[p].hand[i], &duel.players[p].hand[i + 1]);
            }
            duel_bzero(&duel.players[p].hand[duel.players[p].n_hand], 4);
            return 1;
        }
    }
    return 0;
}

int duel_level_cb(DuelEffect *e, int *ctx)
{
    if ((e->kind & 0xF) < 6 && e->src == 0x18D9 && (e->arg & 0xFFF) == ctx[0]) {
        ctx[1] = e->arg >> 12;
    }
    return 0;
}

/* Level of card i of the work list, after 0x18D9, 0x150B and 0x15C7. */
int duel_list_card_level(unsigned int p, int i)
{
    int id = duel.list[i].id;
    int ctx[2];
    int n;
    int lvl;
    unsigned int q;

    lvl = card_get_level(id);
    if (lvl == 0) {
        return 0;
    }
    ctx[1] = lvl;
    ctx[0] = i;
    duel_slot_foreach_effect(0, 11, duel_level_cb, ctx);
    lvl = ctx[1];
    if (duel_card_e5b88(id, 3)) {
        for (q = 0; (int)q < 2; q++) {
            if (duel.players[q & 1].field[10].card.id == 0x150B && SLOT_ACTIVE(&duel.players[q & 1].field[10]) &&
                duel_is_cur_card(duel.players[q & 1].field[10].card.id)) {
                lvl--;
                lvl = lvl <= 0 ? 1 : lvl;
            }
        }
    }
    n = duel_slot_counter(p, 11, 0x15C7);
    if (n != 0) {
        lvl -= n * 2;
        lvl = lvl <= 0 ? 1 : lvl;
    }
    return lvl;
}

/* First spell in the hand of player p, -1 if none. */
int duel_hand_first_spell(unsigned int p)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < duel.players[p].n_hand; i++) {
        int id = duel.players[p].hand[i].id;

        if (id != 0 && card_get_type((unsigned short)id) == 0x16) {
            return i;
        }
    }
    return -1;
}

int duel_hand_count_type(unsigned int p, unsigned int type)
{
    unsigned int i;
    int n = 0;

    p &= 1;
    for (i = 0; i < duel.players[p].n_hand; i++) {
        if (type == card_get_type(duel.players[p].hand[i].id)) {
            n++;
        }
    }
    return n;
}

int duel_hand_count_name(unsigned int p, unsigned short id)
{
    unsigned int i;
    int n = 0;

    p &= 1;
    for (i = 0; i < duel.players[p].n_hand; i++) {
        if (card_same_name(duel.players[p].hand[i].id, id)) {
            n++;
        }
    }
    return n;
}

int duel_hand_find_name(unsigned int p, int id)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < duel.players[p].n_hand; i++) {
        if (card_same_name(duel.players[p].hand[i].id, id)) {
            return i;
        }
    }
    return -1;
}

int duel_hand_count_monsters(unsigned int p)
{
    unsigned int i;
    int n;

    p &= 1;
    i = 0;
    n = 0;
    for (; i < duel.players[p].n_hand; i++) {
        if (card_is_monster(duel.players[p].hand[i].id)) {
            n++;
        }
    }
    return n;
}

int duel_hand_count_level(unsigned int p, int lvl)
{
    unsigned int i;
    int n;

    p &= 1;
    i = 0;
    n = 0;
    for (; i < duel.players[p].n_hand; i++) {
        if ((int)card_get_level(duel.players[p].hand[i].id) >= lvl) {
            n++;
        }
    }
    return n;
}

unsigned int duel_hand_locked(unsigned int p)
{
    return duel.players[p & 1].flags >> 1 & 1;
}

/* Effects that let player p see the opponent's hand. */
int duel_hand_revealed(unsigned int p)
{
    if (duel_find_effect_card(1 - p, 0x137B, -1)) {
        return 1;
    }
    if (duel_count_active(0x1340)) {
        return 1;
    }
    if (duel_find_effect_card(1 - p, 0x17E7, -1)) {
        return 1;
    }
    if (p == duel.turn_player && duel_count_active(0x135E)) {
        return 1;
    }
    return 0;
}

/* Shuffles the hand of player p (not while it is locked). */
int duel_hand_shuffle(unsigned int p)
{
    DuelCard t;
    int i;

    if (duel_hand_locked(p)) {
        return 0;
    }
    p &= 1;
    if (duel.players[p].n_hand < 2) {
        return 1;
    }
    for (i = duel.players[p].n_hand - 1; i > 0; i--) {
        int j = duel_rand(i + 1);

        duel_card_copy(&t, &duel.players[p].hand[j]);
        duel_card_copy(&duel.players[p].hand[j], &duel.players[p].hand[i]);
        duel_card_copy(&duel.players[p].hand[i], &t);
    }
    return 1;
}
