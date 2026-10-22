// @range 0x088601AC-0x08860A88
/* The computer player: target filters (DuelAct *a, p, z). */
#include "duel.h"

int duel_act_can_target(DuelAct *a, unsigned int p, int z);
unsigned short duel_field_type(unsigned int p, int z);
int duel_field_level(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_field_same_name(unsigned int p, int z, int id);
int field_get_card(unsigned int p, int z);
int card_get_attribute_icon(int id);
int duel_is_cur_card(int id);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_slot_efb84(unsigned int p, int z);
int card_get_type(unsigned short id);
int duel_0884E948(DuelAct *a, unsigned int p);
int duel_088580B8(DuelAct *a, unsigned int p, int z);
int duel_0885A004(DuelAct *a, unsigned int p, int z);

int duel_088601AC(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;
    int card;

    if (p != a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    switch (a->id) {
    case 0x157A:
        return duel_field_type(p, z) == 7 && duel_field_atk(p, z) < 501;
    case 0x1669:
        if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
            switch (field_get_card(p, z)) {
            case 0x13CD:
                if (duel_is_cur_card(0x10F4) != 0) {
                    return 0;
                }
                break;
            case 0x164E:
                return 0;
            }
        }
        return 1;
    case 0x1908:
        return duel.players[p & 1].field[z].pos != 0 ? 0 : duel_field_same_name(p, z, 0x18F9);
    case 0x1978:
        return card_get_attribute_icon(id) != 0 ? 0 : duel_field_level(p, z) < 3;
    case 0x1402:
        if (field_get_card(p, z) != 0x13C7) {
            return 0;
        }
        if (duel_special.player[1] == p && duel_special.zone[1] == z) {
            return 0;
        }
        return 1;
    case 0x143B:
        if (field_get_card(p, z) != 0x1439) {
            return 0;
        }
        if (duel_special.player[1] == p && duel_special.zone[1] == z) {
            return 0;
        }
        return 1;
    case 0x1AA6:
        if (duel_field_type(p, z) != 9) {
            return 0;
        }
        card = duel_field_level(p, z) >= 5;
        return card;
    }
    return 1;
}

int duel_08860534(DuelAct *a, unsigned int p, int z)
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
    if (duel.players[p & 1].field[z].pos != 0) {
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

int duel_0886065C(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (z < 5 || z > 10) {
        return 0;
    }
    if (!(duel.players[p & 1].field[z].flags >> 8 & 1) && card_get_type(id) == 0x17) {
        return 0;
    }
    if (duel_act_can_affect(a, p, z, 0) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_08860788(DuelAct *a, unsigned int p, int z)
{
    if (a->player != p) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0886083C(DuelAct *a, unsigned int p, int z)
{
    int lv;

    if (a->player != p) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_slot_efb84(p, z) == 0) {
        return 0;
    }
    lv = duel_field_level(p, z);
    if (lv == 0) {
        return 0;
    }
    return lv + a->w[2] < 13;
}

int duel_08860914(DuelAct *a, unsigned int p, int z)
{
    if (a->b15) {
        return duel_088580B8(a, p, z);
    }
    return duel_0885A004(a, p, z);
}

int duel_08860954(DuelAct *a, unsigned int p, int z)
{
    int r;

    if (a->w[2] != 0) {
        if (z >= 5) {
            return 0;
        }
        if (a->player == p) {
            return 0;
        }
        if (duel.players[p & 1].field[z].card.id != 0) {
            return !duel.players[p & 1].field[z].faceup || duel_field_level(p, z) < a->w[2];
        }
        return 0;
    }
    if (duel_088601AC(a, p, z) != 0) {
        a->w[2] = duel_field_level(p, z);
        r = duel_0884E948(a, 1 - p);
        a->w[2] = 0;
        return r > 0;
    }
    return 0;
}
