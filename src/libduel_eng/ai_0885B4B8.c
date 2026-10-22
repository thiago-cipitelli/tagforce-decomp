// @range 0x0885B4B8-0x0885C594
/* The computer player: more target filters (DuelAct *a, p, z). */
#include "duel.h"

int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_slot_efb84(unsigned int p, int z);
int duel_field_flag9(unsigned int p, int z);
int card_get_type(int id);
int duel_field_atk(unsigned int p, int z);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_slot_ef098(unsigned int p, int z, int id);
int duel_08856860(DuelAct *a, unsigned int p, int z);
int duel_08855F54(DuelAct *a, unsigned int p, int z);
int duel_08855214(DuelAct *a, unsigned int p, int z);
int duel_0885A15C(DuelAct *a, unsigned int p, int z);
int duel_088580B8(DuelAct *a, unsigned int p, int z);
int duel_0885F2E4(DuelAct *a, unsigned int p, int z);
int duel_08857D98(DuelAct *a, unsigned int p, int z);
int duel_08859120(DuelAct *a, unsigned int p, int z);
int duel_088578B8(DuelAct *a, unsigned int p, int z);

int field_get_card(unsigned int p, int z);
int card_is_attribute8(unsigned short id);
int duel_st_affects(unsigned int q, int i, unsigned int p, int z);
int duel_08854D4C(DuelAct *a, unsigned int p, int z);
unsigned int duel_act_target(DuelAct *a, int i);

extern unsigned int duel_w_08914920;

int duel_0885B4B8(DuelAct *a, unsigned int p, int z)
{
    if (a->n7 != 0) {
        return duel_08855F54(a, p, z);
    }
    return duel_08856860(a, p, z);
}

int duel_0885B4F0(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (a->player == p) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    return !duel.players[p & 1].field[z].pos;
}

int duel_0885B598(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (id == 0) {
        return 0;
    }
    if (p != a->player) {
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
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}

int duel_0885B70C(DuelAct *a, unsigned int p, int z)
{
    unsigned int i;
    unsigned int q;
    struct DuelActX *x;

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
    case 0x1518:
        if (duel_field_atk(p, z) < 1500) {
            return 0;
        }
    }
    if (a->w[0] == 0) {
        for (i = 0; i < duel_w_08914920; i++) {
            switch (duel_chain.kinds[i]) {
            case 6:
            case 7:
            case 8:
                x = &duel_chain.srcs[i];
                q = x->s_player;
                if (p == q && z == x->s_zone) {
                    return a->player != (q ^ x->s_b16);
                }
            }
        }
    } else {
        return duel_slot_effect_with(p, z, 0x1518, a->uid);
    }
    return 0;
}

int duel_0885B8CC(DuelAct *a, unsigned int p, int z)
{
    if (p == a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return (int)duel.players[p & 1].field[z].card.id > 0;
}

int duel_0885B96C(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    return 1;
}

int duel_0885BA2C(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_0885BB44(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (p == a->player) {
        return 0;
    }
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_0885BC5C(DuelAct *a, unsigned int p, int z)
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
    return duel_act_can_target(a, p, z) != 0;
}

int duel_0885BD04(DuelAct *a, unsigned int p, int z)
{
    if (p != a->player) {
        return 0;
    }
    if (z > 4) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 0) != 0;
}

int duel_0885BDC4(DuelAct *a, unsigned int p, int z)
{
    if (a->b15) {
        switch (a->id) {
        case 0x14C5:
            return duel_08855214(a, p, z);
        case 0x169B:
            return duel_0885A15C(a, p, z);
        case 0x1753:
            return duel_088580B8(a, p, z);
        case 0x1A35:
            return duel_0885F2E4(a, p, z);
        }
    }
    return duel_08856860(a, p, z);
}

int duel_0885BE70(DuelAct *a, unsigned int p, int z)
{
    if (z > 4) {
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

int duel_0885BF1C(DuelAct *a, unsigned int p, int z)
{
    if (a->w[1] != 0) {
        return duel_08857D98(a, p, z);
    }
    return duel_08859120(a, p, z);
}

int duel_0885BF54(DuelAct *a, unsigned int p, int z)
{
    if (a->player != p) {
        return 0;
    }
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    return duel_slot_efb84(p, z) != 0;
}

int duel_0885C050(DuelAct *a, unsigned int p, int z)
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
    return a->uid != CARD_UID(duel.players[p & 1].field[z].card);
}

int duel_0885C0FC(DuelAct *a, unsigned int p, int z)
{
    if (a->w[2] == 2) {
        if (duel_slot_ef098(p, z, a->id) == 0) {
            return 0;
        }
        if (duel_act_immune(a, p, z) != 0) {
            return 0;
        }
    }
    return duel_088578B8(a, p, z);
}

int duel_0885C18C(DuelAct *a, unsigned int p, int z)
{
    int i;
    int j;
    unsigned int t;
    unsigned int q;

    switch (a->w[2]) {
    case 1:
        if (duel_0885A15C(a, p, z) != 0) {
            DuelAct b = {0};

            b.player = p;
            b.zone = z;
            b.b3_4 = 3;
            b.id = field_get_card(p, z);
            b.uid = CARD_UID(duel.players[p & 1].field[z].card);
            for (i = 0; i <= 4; i++) {
                if (i != z && duel_08854D4C(&b, p, i) != 0) {
                    return 1;
                }
            }
        }
        break;
    case 2:
        if (a->player != p) {
            return 0;
        }
        if (z < 5 || z >= 10) {
            return 0;
        }
        j = duel.players[p & 1].field[z].card.id;
        if (j == 0) {
            return 0;
        }
        if (card_is_attribute8(j) == 0) {
            return 0;
        }
        if (duel_act_can_target(a, p, z) == 0) {
            return 0;
        }
        for (i = 0; i < 2; i++) {
            for (j = 0; j <= 4; j++) {
                switch (duel_st_affects(i, j, p, z)) {
                case 10:
                case 0xB:
                    return 1;
                }
            }
        }
        break;
    case 3:
        t = duel_act_target(a, 0);
        q = t & 0xFF;
        i = (unsigned short)t >> 8 & 0xFF;
        if (p == q && z != i && duel.players[p & 1].field[z].card.id != 0) {
            DuelAct b = {0};

            b.player = q;
            b.zone = i;
            b.b3_4 = 3;
            b.id = field_get_card(q, i);
            b.uid = CARD_UID(duel.players[q & 1].field[i].card);
            return duel_08854D4C(&b, p, z);
        }
        break;
    }
    return 0;
}
