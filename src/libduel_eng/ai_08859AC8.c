// @range 0x08859AC8-0x0885B4B8
/* The computer player: target filters (DuelAct *a, p, z) of the cards it plays. */
#include "duel.h"

int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_field_level(unsigned int p, int z);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_slot_efb84(unsigned int p, int z);
int duel_field_same_name(unsigned int p, int z, int id);
int duel_088559C0(DuelAct *a, unsigned int p, int z);
int duel_0884E698(DuelAct *a, int (*fn)(DuelAct *a, unsigned int p, int z), unsigned int skip);
int field_get_card(unsigned int p, int z);
int card_is_in_group_1a90(int id);
unsigned short duel_field_type(unsigned int p, int z);
int card_is_attribute8(int id);
int card_is_attribute9(int id);
int duel_attack_check(unsigned int p, int z, int arg);
int duel_direct_attack_allowed(unsigned int p, int z);
int duel_field_e5ae0(unsigned int p, int z);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_field_atk(unsigned int p, int z);
int card_is_in_group_1911(int id);
int card_is_in_group_18be(int id);
int card_is_in_group_1a6f(int id);
int duel_field_flag9(unsigned int p, int z);
int card_get_type(int id);
int card_is_in_group_160f(int id);
int card_is_in_group_1a6x(int id);
int duel_slot_ef098(unsigned int p, int z, int id);
void duel_field_stats(unsigned int p, int z, DuelStats *st);

extern unsigned int duel_w_089147E4;

int duel_08859AC8(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (p != 1 - a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    return duel.players[p & 1].field[z].pos != 0;
}

int duel_08859B78(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (z >= 5) {
        return 0;
    }
    if (id == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    if (p != 1 - a->player) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    return duel_field_level(p, z) == 4;
}

int duel_08859C78(DuelAct *a, unsigned int p, int z)
{
    if (a->w[2] != 0) {
        if (duel.players[p & 1].field[z].card.id == 0) {
            return 0;
        }
        if (p == a->player) {
            return 0;
        }
        if (duel_act_immune(a, p, z) != 0) {
            return 0;
        }
        if (duel_act_can_affect(a, p, z, 1) == 0) {
            return 0;
        }
        if (z < 5 && duel_slot_e8170(p, z, 0x1434) != 0) {
            return 0;
        }
    } else {
        if (a->player != p) {
            return 0;
        }
        if (duel.players[p & 1].field[z].card.id == 0) {
            return 0;
        }
        if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        if (z < 5 && duel_slot_e8170(p, z, 0x1434) != 0) {
            return 0;
        }
    }
    return 1;
}

int duel_08859E2C(DuelAct *a, unsigned int p, int z)
{
    if (a->b15) {
        return duel_088559C0(a, p, z);
    }
    if (a->kind == 0x23) {
        return 1;
    }
    if (p != a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel_field_same_name(p, z, 0x1686) == 0) {
        return 0;
    }
    if (duel_slot_efb84(p, z) == 0) {
        return 0;
    }
    return duel_0884E698(a, duel_088559C0, -1) != 0;
}

int duel_08859F10(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (a->player == p) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_special.player[0] == p && duel_special.zone[0] == z) {
        return 0;
    }
    return 1;
}

int duel_0885A004(DuelAct *a, unsigned int p, int z)
{
    if (a->player != p) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel_slot_efb84(p, z) == 0) {
        return 0;
    }
    switch (a->id) {
    case 0x146F:
        return duel_field_same_name(p, z, 0x147A);
    case 0x18A3:
        switch (a->n7) {
        case 0:
            return duel_field_same_name(p, z, 0x189E);
        case 1:
            return duel_field_same_name(p, z, 0x18A0);
        case 2:
            return duel_field_same_name(p, z, 0x189F);
        }
        break;
    case 0x1907:
        return duel_field_same_name(p, z, 0x18AA);
    case 0x1A9A:
        return card_is_in_group_1a90(field_get_card(p, z));
    }
    return 0;
}

int duel_0885A15C(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;
    DuelPlayer *pl = &duel.players[p & 1];
    int i;

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
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    switch (a->id) {
    case 0x1475:
        if (duel_field_same_name(p, z, 0xFBC) != 0) {
            return 1;
        }
        if (duel_field_type(p, z) == 0x13) {
            return 1;
        }
        break;
    case 0x14FC:
        return duel_field_same_name(p, z, 0x1414);
    case 0x15F7:
        return card_is_attribute8(id);
    case 0x15FF:
        return duel_field_type(p, z) == 0x12 && duel_field_level(p, z) >= 7 && duel_attack_check(p, z, 1) != 0;
    case 0x169B:
        return duel_field_same_name(p, z, 0x1691);
    case 0x16A2:
        return card_is_in_group_1911(field_get_card(p, z));
    case 0x175D:
        switch (duel_field_type(p, z)) {
        case 0xF:
        case 7:
            return 1;
        default:
            return 0;
        }
    case 0x179F:
        return duel_field_e5ae0(p, z) != 0 && !card_is_attribute9(id) && duel_tributable(p, p, z) != 0;
    case 0x17B8:
        return duel_field_e5ae0(p, z) != 0 && duel_field_level(p, z) < 3 && !card_is_attribute9(id) && duel_tributable(p, p, z) != 0;
    case 0x17F6:
        return duel_field_same_name(p, z, 0xFF8);
    case 0x17FF:
        return card_is_in_group_18be(field_get_card(p, z));
    case 0x1819:
        return duel.players[p & 1].field[z].pos != 0 ? 0 : duel_field_type(p, z) == 0x12;
    case 0x1889:
        return duel_field_type(p, z) == 0xB;
    case 0x1890:
        for (i = 0; i <= 4; i++) {
            if (i != z && pl->field[i].card.id != 0 && pl->field[i].faceup != 0 && pl->field[i].pos == 0) {
                return 1;
            }
        }
        return 0;
    case 0x1893:
        if (duel_direct_attack_allowed(p, z) == 0) {
            return 0;
        }
        return a->b4_0 ? 1 : duel_field_type(p, z) == 2 && duel_field_atk(p, z) < 2001;
    case 0x18CB:
        return duel_field_level(p, z) < a->w[2];
    case 0x195B:
        return duel_field_same_name(p, z, 0x18A6);
    case 0x19AB:
        return card_is_in_group_1a6f(field_get_card(p, z));
    case 0x19DC:
        return !card_is_attribute9(id);
    case 0x1A32:
        return duel_field_same_name(p, z, 0x1A1C);
    }
    return 0;
}

int duel_0885A740(DuelAct *a, unsigned int p, int z)
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
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_special.player[0] == p && duel_special.zone[0] == z) {
        return 0;
    }
    return 1;
}

int duel_0885A814(DuelAct *a, unsigned int p, int z)
{
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}

int duel_0885A8FC(DuelAct *a, unsigned int p, int z)
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
    return 1;
}

int duel_0885A990(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return a->w[1] != 0 && a->w[1] == duel_field_level(p, z);
    }
    return 0;
}

int duel_0885AA54(DuelAct *a, unsigned int p, int z)
{
    int lv;

    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (a->w[0] != 0) {
        lv = duel_field_level(p, z);
        return a->w[0] == (lv > 6 ? 6 : lv);
    }
    return 1;
}

int duel_0885AB18(DuelAct *a, unsigned int p, int z)
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
    return duel_act_can_target(a, p, z) != 0;
}

int duel_0885ABE0(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (z >= 5) {
        return 0;
    }
    if (id == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return duel.players[p & 1].field[z].flags >> 3 & 1;
}

int duel_0885AC84(DuelAct *a, unsigned int p, int z)
{
    if (z == 10) {
        return 0;
    }
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].flags >> 8 & 1) {
        return 0;
    }
    return a->uid != CARD_UID(duel.players[p & 1].field[z].card);
}

int duel_0885ADAC(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (id == 0) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    if (z < 5 || z > 10) {
        return 0;
    }
    if (!(duel.players[p & 1].field[z].flags >> 8 & 1) && card_get_type(id) == 0x17) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}

int duel_0885AED8(DuelAct *a, unsigned int p, int z)
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
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    if (p == a->player) {
        return card_is_in_group_160f(field_get_card(p, z));
    }
    return 1;
}

int duel_0885AFE0(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_slot_ef098(p, z, a->id) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return duel_field_level(p, z) < 4;
}

int duel_0885B0E4(DuelAct *a, unsigned int p, int z)
{
    DuelStats st;

    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    if (!(duel_special.player[0] == p && duel_special.zone[0] == z) && !(duel_special.player[1] == p && duel_special.zone[1] == z)) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    duel_field_stats(p, z, &st);
    if (MAX(st.atk, st.def) / 100 == 0) {
        return 0;
    }
    return 1;
}

int duel_0885B250(int n, int sum)
{
    int v;

    for (; n >= 0; n--) {
        v = duel_chain.uids[duel_w_089147E4 + n & 0x7F];
        if (v < sum) {
            if (n != 0 && duel_0885B250(n - 1, sum - v) != 0) {
                return 1;
            }
        } else if (v == sum) {
            return 1;
        }
    }
    return !sum;
}

int duel_0885B2F8(DuelAct *a, unsigned int p, int z)
{
    DuelPlayer *pl;
    int lv;
    int i;
    int n;

    if (p != a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    pl = &duel.players[p & 1];
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    switch (a->id) {
    case 0x14C1:
        n = 0;
        lv = duel_field_level(p, z);
        if (lv == 0) {
            return 0;
        }
        for (i = 0; i <= 4; i++) {
            if (z != i && pl->field[i].card.id != 0) {
                duel_chain.uids[duel_w_089147E4 + n & 0x7F] = duel_field_level(p, i);
                n++;
            }
        }
        return duel_0885B250(n - 1, a->w[2] - lv);
    case 0x1A05:
        if (duel.players[p & 1].field[z].faceup != 0) {
            return !card_is_in_group_1a6x(field_get_card(p, z));
        }
        return 1;
    }
    return 1;
}
