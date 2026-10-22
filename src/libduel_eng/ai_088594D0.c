// @range 0x088594D0-0x08859AC8
/* Computer player: more target tests (DuelAct *a, p, z). */
#include "duel.h"

extern unsigned int duel_w_08914920;

int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
int duel_can_attack_target(unsigned int p, int z, unsigned int q, int i, int arg);
int card_get_type(unsigned short id);
int card_attribute_icon_is3(int id);

int duel_088594D0(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (p != a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    switch (a->id) {
    case 0x1420:
        if (a->player != duel.players[p & 1].field[z].card.owner) {
            return 0;
        }
        break;
    case 0x1957:
        if (duel_st_affects(p, z, a->player, a->zone) != 0) {
            return 0;
        }
        break;
    }
    return 1;
}

int duel_0885961C(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (a->player != p) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_special.player[1] == p && duel_special.zone[1] == z) {
        return 0;
    }
    return duel_can_attack_target(duel_special.player[0], duel_special.zone[0], p, z, 0);
}

int duel_088596CC(DuelAct *a, unsigned int p, int z)
{
    int id;

    if (z < 5 || z > 10) {
        return 0;
    }
    p &= 1;
    id = duel.players[p].field[z].card.id;
    if (id == 0) {
        return 0;
    }
    if (duel.players[p].field[z].faceup == 0) {
        return 0;
    }
    if (card_get_type(id) == 0x17) {
        return 0;
    }
    return a->uid != CARD_UID(duel.players[p].field[z].card);
}

int duel_088597D4(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (z >= 5) {
        return 0;
    }
    if (id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return card_attribute_icon_is3(id) != 0;
}

int duel_08859888(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    if (p != a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    return id != 0 ? 1 : 0;
}

int duel_0885994C(DuelAct *a, unsigned int p, int z)
{
    unsigned int i;

    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].pos == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    for (i = 0; i < duel_w_08914920; i++) {
        if ((duel_chain.kinds[i] == 8 || duel_chain.kinds[i] == 9) && p == duel_chain.srcs[i].s_player
            && z == duel_chain.srcs[i].s_zone && !duel_chain.srcs[i].s_b15 && duel_chain.srcs[i].s_b14) {
            return 1;
        }
    }
    return 0;
}
