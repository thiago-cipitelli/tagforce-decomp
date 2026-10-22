// @range 0x08855908-0x08856860
/* Computer player: more target tests (DuelAct *a, p, z). */
#include "duel.h"

extern unsigned int duel_turn_player;

int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_slot_ef098(unsigned int p, int z, int id);
unsigned int duel_field_type(unsigned int p, int z);
int field_get_card(unsigned int p, int z);
int card_is_in_group_1946(int id);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_field_level(unsigned int p, int z);
int duel_field_def(unsigned int p, int z);
int duel_slot_count_14c0(unsigned int p, int z);
int duel_slot_e5a48(unsigned int p, int z);
unsigned int duel_field_flag9(unsigned int p, int z);
int card_is_in_group_1a6f(int id);

int duel_08855908(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 0) != 0;
}

int duel_088559C0(DuelAct *a, unsigned int p, int z)
{
    if (p == a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 0) != 0;
}

int duel_08855A8C(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_affect(a, p, z, 1) == 0) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}

int duel_08855B40(DuelAct *a, unsigned int p, int z)
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
    return !duel_act_immune(a, p, z);
}

int duel_08855C18(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_slot_ef098(p, z, a->id) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_08855CD4(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (p == a->player) {
        if (duel_slot_ef098(p, z, a->id) == 0) {
            return 0;
        }
    } else {
        if (duel_act_can_affect(a, p, z, 1) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_08855DC8(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_08855E9C(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_08855F54(DuelAct *a, unsigned int p, int z)
{
    unsigned int t;

    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    if (duel_act_can_affect(a, p, z, 0) == 0) {
        return 0;
    }
    t = (unsigned short)duel_field_type(p, z);
    switch (a->id) {
    case 0x1542:
        return 1;
    case 0x1236:
    case 0x166D:
        return duel_slot_count_14c0(p, z) > 0;
    case 0x14E4:
        return duel_field_def(p, z) <= a->w[0];
    case 0x1233:
        return t == 0xF;
    case 0x123A:
        return t == 7;
    case 0x123C:
        return t == 0xA;
    case 0x123D:
        return t == 6;
    case 0x123E:
        return t == 8;
    case 0x1304:
        return t == 0x12;
    case 0x1305:
        return t == 3;
    case 0x161C:
        return t == a->w[0];
    case 0x1945:
        return card_is_in_group_1946(field_get_card(p, z)) != 0
            && (unsigned short)((unsigned char)p | (unsigned char)z << 8) != duel_act_target(a, 0);
    case 0x1977:
        return duel.players[p & 1].field[z].pos != 0 ? 0 : duel_field_level(p, z) == 3;
    }
    return 0;
}

int duel_08856248(DuelAct *a, unsigned int p, int z)
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
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    switch (a->id) {
    case 0x16E1:
    case 0x12CB:
        return 1;
    case 0x15DF:
        return a->w[2] == duel_field_level(p, z);
    case 0x17B6:
        return p == duel_turn_player;
    case 0x17B3:
        return duel.players[p & 1].field[z].pos != 0 ? 0 : duel_slot_e5a48(p, z);
    case 0x1840:
        return field_get_card(p, z) == 0xFE5;
    case 0x1988:
        return card_is_in_group_1a6f(field_get_card(p, z)) != 0 && field_get_card(p, z) != 0x18A7;
    }
    return 0;
}

int duel_0885644C(DuelAct *a, unsigned int p, int z)
{
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 0) != 0;
}

int duel_0885653C(DuelAct *a, unsigned int p, int z)
{
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}

int duel_0885660C(DuelAct *a, unsigned int p, int z)
{
    if (z < 5 && duel_field_flag9(p, z) == 0) {
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
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 0) != 0;
}

int duel_08856754(DuelAct *a, unsigned int p, int z)
{
    if (z < 5 && duel_field_flag9(p, z) == 0) {
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
    return duel_act_can_affect(a, p, z, 0) != 0;
}
