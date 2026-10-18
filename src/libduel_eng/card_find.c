// @range 0x088E5F84-0x088E6998
// @asm 0x088E6314-0x088E6378
/* Where a card is: location of a card uid (player | zone << 8 | index << 16), index of a uid in
   each pile, values kept in the zone of some cards. */
#include "duel.h"
#include "card_db.h"

int field_get_card(unsigned int p, int z);
int duel_slot_e1854(unsigned int p, int z);
int duel_slot_counter(unsigned int p, int z, int id);
void duel_slot_foreach_effect(unsigned int p, int z, int (*fn)(DuelEffect *e, int *ctx), int *ctx);
DuelCard *duel_field_card_deb2c(unsigned int p, int zone, int i);

/* Location of the card with uid: field (p | z << 8), piles (p | zone << 8 | index << 16, zone
   0xB..0xF), special cards (0xFFFF0000 | p | z << 8); 0x1000 if the uid is nowhere. */
unsigned int duel_card_locate(int uid)
{
    unsigned int p;
    unsigned int i;

    for (p = 0; (int)p < 2; p++) {
        for (i = 0; (int)i <= 10; i++) {
            if (duel.players[p & 1].field[i].card.id != 0 && uid == CARD_UID(duel.players[p & 1].field[i].card)) {
                return (unsigned short)((p & 0xFF) | (i & 0xFF) << 8);
            }
        }
    }
    for (p = 0; (int)p < 2; p++) {
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            if (uid == CARD_UID(duel.players[p & 1].grave[i])) {
                return (unsigned short)((p & 0xFF) | 0xE00) | (unsigned short)i << 16;
            }
        }
        for (i = 0; i < duel.players[p & 1].n_f; i++) {
            if (uid == CARD_UID(duel.players[p & 1].zone_f[i])) {
                return (unsigned short)((p & 0xFF) | 0xF00) | (unsigned short)i << 16;
            }
        }
        for (i = 0; i < (unsigned int)duel.players[p & 1].n_d; i++) {
            if (uid == CARD_UID(duel.players[p & 1].zone_d[i])) {
                return (unsigned short)((p & 0xFF) | 0xD00) | (unsigned short)i << 16;
            }
        }
        for (i = 0; i < (unsigned int)duel.players[p & 1].n_18; i++) {
            if (uid == CARD_UID(duel.players[p & 1].zone_c[i])) {
                return (unsigned short)((p & 0xFF) | 0xC00) | (unsigned short)i << 16;
            }
        }
    }
    for (p = 0; (int)p < 2; p++) {
        for (i = 0; i < duel.players[p & 1].n_hand; i++) {
            if (uid == CARD_UID(duel.players[p & 1].hand[i])) {
                return (unsigned short)((p & 0xFF) | 0xB00) | (unsigned short)i << 16;
            }
        }
    }
    for (i = 0; (int)i < 2; i++) {
        if (uid == CARD_UID(duel_special.slot[i].card)) {
            unsigned int q = i == 0 ? duel_special.player[0] : duel_special.player[1];
            unsigned int z = i == 0 ? duel_special.zone[0] : duel_special.zone[1];

            return (unsigned short)((q & 0xFF) | (z & 0xFF) << 8) | 0xFFFF0000;
        }
    }
    return 0x1000;
}

#ifdef NON_MATCHING /* the original keeps the list index q + 2 * i in its own register */
/* Uid of a card id of player p (the list alternates players), checking p then the opponent. */
int duel_list_find_id(unsigned int p, unsigned int id)
{
    unsigned int k;

    for (k = 0; (int)k < 2; k++) {
        unsigned int q = p ^ k;
        int i;

        for (i = 0; i < 0x100; i++) {
            if (duel.list[q + i * 2].id == id) {
                return q + i * 2;
            }
        }
    }
    return 0;
}
#endif

/* Field location of the card with uid, -1 if it is not on the field. */
unsigned int duel_field_locate(int uid)
{
    unsigned int p;
    unsigned int z;

    for (p = 0; (int)p < 2; p++) {
        for (z = 0; (int)z <= 10; z++) {
            if (duel.players[p & 1].field[z].card.id != 0 && uid == CARD_UID(duel.players[p & 1].field[z].card)) {
                return (unsigned short)((p & 0xFF) | (z & 0xFF) << 8);
            }
        }
    }
    return -1;
}

unsigned int duel_d_find_uid(unsigned int p, int uid)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < (unsigned int)duel.players[p].n_d; i++) {
        if (uid == CARD_UID(duel.players[p].zone_d[i])) {
            return i;
        }
    }
    return -1;
}

unsigned int duel_c_find_uid(unsigned int p, int uid)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < (unsigned int)duel.players[p].n_18; i++) {
        if (uid == CARD_UID(duel.players[p].zone_c[i])) {
            return i;
        }
    }
    return -1;
}

unsigned int duel_grave_find_uid(unsigned int p, int uid)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < duel.players[p].n_grave; i++) {
        if (uid == CARD_UID(duel.players[p].grave[i])) {
            return i;
        }
    }
    return -1;
}

unsigned int duel_f_find_uid(unsigned int p, int uid)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < duel.players[p].n_f; i++) {
        if (uid == CARD_UID(duel.players[p].zone_f[i])) {
            return i;
        }
    }
    return -1;
}

unsigned int duel_hand_find_uid(unsigned int p, int uid)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < duel.players[p].n_hand; i++) {
        if (uid == CARD_UID(duel.players[p].hand[i])) {
            return i;
        }
    }
    return -1;
}

/* Card id was used in this duel (flag b15 of its list entry). */
int duel_card_used_e66ac(unsigned int id)
{
    int i;

    for (i = 1; i < 0x200; i++) {
        if (duel.list[i].id == id && duel.list[i].b15) {
            return 1;
        }
    }
    return 0;
}

DuelCard *duel_card_by_uid(int uid)
{
    unsigned int t = duel_card_locate(uid);

    if (t != 0x1000) {
        return duel_field_card_deb2c(t & 0xFF, (unsigned char)((t & 0xFFFF) >> 8), (unsigned short)(t >> 16));
    }
    return 0;
}

unsigned int duel_card_b14(DuelCard *c)
{
    if (card_attribute_icon_in_2_3(c->id) != 0) {
        return c->b14;
    }
    return 1;
}

/* Value kept in the link field by cards of group 0x1909. */
unsigned short duel_slot_value_1909(unsigned int p, int z)
{
    if (card_is_in_group_1909(field_get_card(p, z)) != 0) {
        switch (field_get_card(p, z)) {
        case 0x14AC:
            if (z > 4) {
                return 0;
            }
            if ((unsigned short)duel_slot_e1854(p, z) == 0xFFFF) {
                return 0;
            }
            break;
        }
        return duel.players[p & 1].field[z].link;
    }
    return 0;
}

static int duel_min_1130_cb(DuelEffect *e, int *best)
{
    if (e->src == 0x1130) {
        *best = MIN(*best, e->arg);
    }
    return 0;
}

unsigned int duel_slot_min_1130(unsigned int p, int z)
{
    int best = 0x10000;

    duel_slot_foreach_effect(p, z, duel_min_1130_cb, &best);
    return best & 0xFFFF;
}

unsigned int duel_slot_e68a0(unsigned int p, int z)
{
    if (card_get_group_value_1983(field_get_card(p, z)) != 0) {
        return duel.players[p & 1].field[z].link;
    }
    return 0;
}

unsigned int duel_slot_link_1a0c(unsigned int p, int z)
{
    if (card_is_in_group_1a0c(field_get_card(p, z)) != 0) {
        return duel.players[p & 1].field[z].link;
    }
    return 0;
}

int duel_slot_counter_1513(unsigned int p, int z)
{
    return duel_slot_counter(p, z, 0x1513);
}

int duel_slot_counter_1a90(unsigned int p, int z)
{
    return duel_slot_counter(p, z, 0x1A90);
}

int duel_slot_counter_1439(unsigned int p, int z)
{
    return duel_slot_counter(p, z, 0x1439);
}
