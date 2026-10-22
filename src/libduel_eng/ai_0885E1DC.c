// @range 0x0885E1DC-0x0885F2E4
/* The computer player: target filters (DuelAct *a, p, z). */
#include "duel.h"

int duel_slot_efb84(unsigned int p, int z);
int duel_field_same_name(unsigned int p, int z, int id);
int duel_field_attr_is(unsigned int p, int z, int attr);
unsigned short duel_field_type(unsigned int p, int z);
int duel_act_check_08853CE8(DuelAct *a, unsigned int p, int z);
int field_get_card(unsigned int p, int z);
int card_get_type(int id);
int card_is_in_group_1a5c(int id);
int card_is_1a6c(int id);
int duel_field_flag9(unsigned int p, int z);
int card_get_prop5_hi(int id);
int card_get_kind(int id);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_slot_ef098(unsigned int p, int z, int id);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_08853B98(DuelAct *a, unsigned int p, int z);
int duel_088A6300(int id);
int duel_088A6360(int id, int i);
int duel_088A62AC(int k, unsigned int p, int z);
int duel_0885CD48(DuelAct *a, unsigned int p, int z);
int duel_0885D284(DuelAct *a, unsigned int p, int z);
int card_attribute_icon_is3(unsigned short id);

int duel_0885E1DC(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (a->player != p) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    switch (a->id) {
    case 0x17E1:
        if (duel_slot_efb84(p, z) == 0) {
            return 0;
        }
        return a->uid == CARD_UID(duel.players[p & 1].field[z].card) ? 0 : duel_field_attr_is(p, z, 4);
    case 0x180E:
        return duel_slot_efb84(p, z) ? duel_field_same_name(p, z, 0x1806) : 0;
    case 0x1938:
        if (duel_field_type(p, z) != 0xF) {
            return 0;
        }
        if (a->w[0] != 0) {
            DuelAct b = {0};

            b.player = p;
            b.zone = 0xB;
            b.id = a->w[0];
            b.uid = a->w[1];
            b.b3_4 = 0;
            if (duel_act_check_08853CE8(&b, p, z) != 1) {
                return 0;
            }
        }
        return 1;
    case 0x1979:
        if (a->n7 != 0) {
            DuelAct b = {0};

            b.player = p;
            b.zone = 0xB;
            b.id = duel.list[a->w[1]].id;
            b.uid = a->w[1];
            b.b3_4 = 0;
            if (duel_act_check_08853CE8(&b, p, z) != 1) {
                return 0;
            }
        }
        return 1;
    case 0x1A5B:
        return card_get_type(duel.players[p & 1].field[z].card.id) == 7 && card_is_in_group_1a5c(field_get_card(p, z)) != 0;
    case 0x1A76:
        return card_is_1a6c(field_get_card(p, z));
    }
    return 1;
}

int duel_0885E5D4(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (card_get_type(id) != 0x17) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (card_get_prop5_hi(id) != 4) {
        return 0;
    }
    return !(duel.players[p & 1].field[z].flags >> 8 & 1);
}

int duel_0885E6D0(DuelAct *a, unsigned int p, int z)
{
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_0885E7A0(DuelAct *a, unsigned int p, int z)
{
    int r;

    a->id = a[-1].id;
    r = duel_0884E3E4(a, p, z);
    a->id = 0x183E;
    return r;
}

int duel_0885E7D4(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (card_get_type(id) != 0x16) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (card_get_prop5_hi(id) != 4) {
        return 0;
    }
    return !(duel.players[p & 1].field[z].flags >> 8 & 1);
}

int duel_0885E8D0(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (a->player != p) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_special.player[1] == p && duel_special.zone[1] == z) {
        return 0;
    }
    return 1;
}

int duel_0885E9A0(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == duel.players[p & 1].field[z].card.owner) {
        return 0;
    }
    if (duel_slot_ef098(p, z, a->id) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885EA74(DuelAct *a, unsigned int p, int z)
{
    if (duel_tributable(a->player, p, z) == 0) {
        return 0;
    }
    return a->uid != CARD_UID(duel.players[p & 1].field[z].card);
}

int duel_0885EB24(DuelAct *a, unsigned int p, int z)
{
    if (duel_08853B98(a, p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    return duel_field_attr_is(p, z, card_get_kind(a->id)) != 0;
}

int duel_0885EBC8(DuelAct *a, unsigned int p, int z)
{
    int mask;
    int n;
    int i;
    int id;

    if (duel_tributable(a->player, p, z) != 0) {
        if (a->player == p || duel.players[p & 1].field[z].faceup != 0) {
            id = a->w[0];
            mask = a->w[1];
            n = duel_088A6300(id);
            for (i = 0; i < n; i++) {
                if (!(mask & 1 << i) && duel_088A62AC(duel_088A6360(id, i), p, z) != 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int duel_0885ECE4(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (z >= 5) {
        return 0;
    }
    if (p != 1 - a->player) {
        return 0;
    }
    if (id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0 && duel.players[p & 1].field[z].pos != 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return 1;
}

int duel_0885EDBC(DuelAct *a, unsigned int p, int z)
{
    int i;

    if (a->w[1] != 0) {
        return duel_0885D284(a, p, z);
    }
    if (duel_0885CD48(a, p, z) != 0) {
        for (i = 0; i <= 4; i++) {
            if (z != i && duel_0885D284(a, p, i) != 0) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_0885EE64(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p != a->player) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].pos == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885EF30(DuelAct *a, unsigned int p, int z)
{
    int id;

    if (z >= 5) {
        return 0;
    }
    id = duel.players[p & 1].field[z].card.id;
    if (id == 0) {
        return 0;
    }
    if (card_attribute_icon_is3(id) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885F018(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}

int duel_0885F0B0(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == a->player) {
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

int duel_0885F188(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885F244(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}
