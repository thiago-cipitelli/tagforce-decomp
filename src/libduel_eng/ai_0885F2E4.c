// @range 0x0885F2E4-0x088601AC
/* The computer player: target filters (DuelAct *a, p, z). */
#include "duel.h"

int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int field_get_card(unsigned int p, int z);
int duel_card_e4fd0(int id);
int card_is_monster(int id);
int duel_slot_e68a0(unsigned int p, int z);
int card_get_group_value_1983(int id);
unsigned short duel_field_type(unsigned int p, int z);
int duel_field_level(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_slot_ef098(unsigned int p, int z, int id);
int duel_slot_efb84(unsigned int p, int z);
int duel_slot_counter_1a90(unsigned int p, int z);
int duel_088BFFD8(unsigned int p, int id, int i);
int duel_0884E698(DuelAct *a, int (*fn)(DuelAct *a, unsigned int p, int z), unsigned int skip);
int duel_088594D0(DuelAct *a, unsigned int p, int z);
int duel_088601AC(DuelAct *a, unsigned int p, int z);
int duel_0885FB0C(DuelAct *a, unsigned int p, int z);

extern int duel_targeting_off;         /* 0x08914910 */

int duel_0885F2E4(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_0885F3E4(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885F488(DuelAct *a, unsigned int p, int z)
{
    if (p == a->player) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885F504(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    return !duel.players[p & 1].field[z].faceup;
}

int duel_0885F568(DuelAct *a, unsigned int p, int z)
{
    int card = field_get_card(p, z);

    if (card == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_card_e4fd0(card) == 0) {
        return 0;
    }
    if (card_is_monster(card) != 0) {
        if (z >= 5) {
            return 0;
        }
    } else if (z < 5) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_0885F6A0(DuelAct *a, unsigned int p, int z)
{
    int card = field_get_card(p, z);

    if (card == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].flags >> 5 & 1) {
        return 0;
    }
    return duel_slot_e68a0(p, z) < card_get_group_value_1983(card);
}

int duel_0885F76C(DuelAct *a, unsigned int p, int z)
{
    int card = field_get_card(p, z);

    if (card == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].flags >> 5 & 1) {
        return 0;
    }
    return duel_slot_e68a0(p, z) < card_get_group_value_1983(card);
}

int duel_0885F8B8(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    return !duel.players[p & 1].field[z].faceup;
}

int duel_0885F930(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_0885FA30(DuelAct *a, unsigned int p, int z)
{
    if (p == a->player && z == 10) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    return a->uid != CARD_UID(duel.players[p & 1].field[z].card);
}

int duel_0885FAD0(DuelAct *a, unsigned int p, int z)
{
    if (p == a->player) {
        return duel_088601AC(a, p, z);
    }
    return duel_0885FB0C(a, p, z);
}

int duel_0885FB0C(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].pos != 0) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}

int duel_0885FBD4(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (a->player == p) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_field_type(p, z) != 7) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885FCC8(DuelAct *a, unsigned int p, int z)
{
    if (a->player != p) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_slot_ef098(p, z, a->id) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    if (duel_targeting_off == 0) {
        if (duel.players[p & 1].field[z].faceup == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_0885FDFC(DuelAct *a, unsigned int p, int z)
{
    unsigned int i;
    unsigned int q;

    if (z >= 5) {
        return 0;
    }
    if (a->player == p) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    switch (a->id) {
    case 0x17B0:
        return duel_field_level(p, z) >= 2;
    case 0x192B:
        a->w[2] = duel_field_level(p, z);
        for (i = 0; i < duel.players[(q = a->player) & 1].n_hand; i++) {
            if (!duel_088BFFD8(q, a->id, i)) {
                continue;
            }
            return 1;
        }
        return 0;
    case 0x1A9C:
        if (duel_slot_counter_1a90(p, z) == 0) {
            return 0;
        }
        return duel_slot_ef098(p, z, a->id) != 0;
    }
    return 1;
}

int duel_0885FFD8(DuelAct *a, unsigned int p, int z)
{
    if (a->w[1] != 0) {
        return duel_088594D0(a, p, z);
    }
    if (a->player != p) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel_slot_efb84(p, z) == 0) {
        return 0;
    }
    return duel_0884E698(a, duel_088594D0, (unsigned short)((unsigned char)p | (unsigned char)z << 8)) != 0;
}

int duel_088600F4(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (a->n7 != 0) {
        return a->w[0] == duel_field_atk(p, z);
    }
    return 1;
}
