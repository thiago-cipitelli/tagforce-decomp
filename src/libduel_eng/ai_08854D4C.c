// @range 0x08854D4C-0x08855214
/* Computer player: target tests (DuelAct *a, p, z) for the zones it may choose. */
#include "duel.h"

int duel_act_check_08853CE8(DuelAct *a, unsigned int p, int z);
int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_tributable(unsigned int p);
int duel_field_atk(unsigned int p, int z);
int card_get_type(int id);

int duel_08854D4C(DuelAct *a, unsigned int p, int z)
{
    if (duel_act_check_08853CE8(a, p, z) != 1) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}

int duel_08854DAC(DuelAct *a)
{
    return duel_tributable(a->player) != 0;
}

int duel_08854DD0(DuelAct *a, unsigned int p, int z)
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
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return duel.players[p & 1].field[z].pos != 0;
}

int duel_08854EC8(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    if (z > 4) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_08854F74(DuelAct *a, unsigned int p, int z)
{
    if (z > 4) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel_act_can_affect(a, p, z, 1) == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return duel_field_atk(p, z) < 1001;
}

int duel_08855098(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (id == 0) {
        return 0;
    }
    if (z < 5 || z > 10) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0 && !(duel.players[p & 1].field[z].flags >> 8 & 1)
        && card_get_type(id) == 0x17) {
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
