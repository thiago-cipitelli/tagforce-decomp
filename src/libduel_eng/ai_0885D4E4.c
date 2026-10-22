// @range 0x0885D4E4-0x0885E1DC
/* The computer player: target filters (DuelAct *a, p, z). */
#include "duel.h"

int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_field_flag9(unsigned int p, int z);
int duel_field_e5ae0(unsigned int p, int z);
int duel_field_level(unsigned int p, int z);
int duel_field_same_name(unsigned int p, int z, int id);
int duel_tributable(unsigned int p, unsigned int q, int z);
int field_get_card(unsigned int p, int z);
int card_is_attribute9(int id);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_field_atk(unsigned int p, int z);
int duel_slot_efb84(unsigned int p, int z);
int card_is_in_group_198c(int id);
int card_id_remap_1a6c(int id, int *out);
int duel_count_free_monster_zones(unsigned int p);
int duel_zone_usable(unsigned int p, unsigned int z);
int duel_0885DD6C(DuelAct *a, unsigned int p, int z);

typedef struct AiLoc {                 /* +0x14 of a DuelAct as the location of a card */
    unsigned int pad : 16;
    unsigned int player : 1;           /* bit 16 */
    unsigned int zone : 4;             /* bits 17-20 */
} AiLoc;

int duel_0885D4E4(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (id == 0) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (a->player == p) {
        return 0;
    }
    if (card_is_attribute9(id) != 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return duel.players[p & 1].field[z].faceup != 0;
}

int duel_0885D5F8(DuelAct *a, unsigned int p, int z)
{
    if (a->player == p) {
        return 0;
    }
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    return 1;
}

int duel_0885D6A0(DuelAct *a, unsigned int p, int z)
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
    if (duel.players[p & 1].field[z].pos != 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885D758(DuelAct *a, unsigned int p, int z)
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
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return a->id != field_get_card(p, z);
}

int duel_0885D830(DuelAct *a, unsigned int p, int z)
{
    if (card_is_attribute9(duel.players[p & 1].field[z].card.id) != 0) {
        return 0;
    }
    if (duel_field_e5ae0(p, z) == 0) {
        return 0;
    }
    if (duel_tributable(a->player, p, z) != 0) {
        if (a->player == p || duel.players[p & 1].field[z].faceup != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_0885D92C(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].faceup != 0) {
        return a->id == field_get_card(p, z);
    }
    return 0;
}

int duel_0885D99C(DuelAct *a, unsigned int p, int z)
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
    if (duel_field_e5ae0(p, z) == 0) {
        return 0;
    }
    return duel_field_level(p, z) < 3;
}

int duel_0885DA60(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;
    DuelPlayer *pl = &duel.players[p & 1];
    int card = field_get_card(p, z);
    int i;
    int n;

    if (id == 0) {
        return 0;
    }
    if (p != a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_field_e5ae0(p, z) == 0) {
        return 0;
    }
    if (card_is_attribute9(id) != 0) {
        return 0;
    }
    n = 0;
    for (i = 0; i <= 4; i++) {
        if (pl->field[i].faceup != 0 && duel_field_same_name(p, i, card) != 0) {
            n++;
        }
    }
    return n == 3;
}

int duel_0885DBCC(DuelAct *a, unsigned int p, int z)
{
    if (p != a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0 && duel.players[p & 1].field[z].pos != 0) {
        return 0;
    }
    return 1;
}

int duel_0885DC7C(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (a->kind == 0xF && p == ((AiLoc *)&a->idx)->player && z == ((AiLoc *)&a->idx)->zone) {
        return 1;
    }
    return 0;
}

int duel_0885DD18(DuelAct *a, unsigned int p, int z)
{
    if (duel_act_can_target(a, p, z) != 0) {
        return duel_0885DD6C(a, p, z);
    }
    return 0;
}

int duel_0885DD6C(DuelAct *a, unsigned int p, int z)
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
    if (duel_act_can_affect(a, p, z, 1) == 0) {
        return 0;
    }
    return duel_field_atk(p, z) <= a->w[2];
}

int duel_0885DE58(DuelAct *a, unsigned int p, int z)
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
    if (duel.players[p & 1].field[z].pos != 0) {
        return 0;
    }
    if (a->n7 != 0) {
        return a->w[0] == duel_field_atk(p, z);
    }
    return 1;
}

int duel_0885DF30(DuelAct *a, unsigned int p, int z)
{
    int card = field_get_card(p, z);

    if (a->kind == 0x23) {
        return 1;
    }
    if (card == 0) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (a->player != p) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (card_is_in_group_198c(card) == 0) {
        return 0;
    }
    if (card_id_remap_1a6c(card, 0) == 0) {
        return 0;
    }
    if (duel_slot_efb84(p, z) == 0) {
        return 0;
    }
    if (duel_count_free_monster_zones(a->player) == 0) {
        if (duel_zone_usable(p, z) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_0885E088(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p != a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885E154(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p != a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}
