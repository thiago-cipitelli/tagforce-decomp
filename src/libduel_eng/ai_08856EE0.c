// @range 0x08856EE0-0x08857B34
/* Computer player: more target tests (DuelAct *a, p, z). */
#include "duel.h"

extern unsigned int duel_turn_player;
extern unsigned int duel_w_08914920;

int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_tributable(unsigned int p);
int duel_field_atk(unsigned int p, int z);
int duel_slot_e5a48(unsigned int p, int z);
int card_is_attribute9(int id);
unsigned int duel_field_type(unsigned int p, int z);
int duel_slot_ef098(unsigned int p, int z, int id);
int duel_slot_has_src10(unsigned int p, int z, int q, int k);
int duel_slot_can_act(unsigned int p, unsigned int z, int x, int y);
int duel_0885C594(DuelAct *a, unsigned int p, int z);
int duel_field_def(unsigned int p, int z);

int duel_08856EE0(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].pos == 0) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}

int duel_08856F70(DuelAct *a, unsigned int p, int z)
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
    if (a->n7 != 0) {
        return a->w[0] == duel_field_atk(p, z);
    }
    return 1;
}

int duel_0885705C(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}

int duel_088570D0(DuelAct *a, unsigned int p, int z)
{
    if (duel_tributable(duel_turn_player) == 0) {
        return 0;
    }
    return duel.players[p & 1].field[z].faceup != 0;
}

int duel_08857148(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    return duel_slot_e5a48(p, z) != 0;
}

int duel_088571DC(DuelAct *a, unsigned int p, int z)
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
    if (card_is_attribute9(id) != 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_088572D0(DuelAct *a, unsigned int p, int z)
{
    unsigned int i;

    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    switch (a->id) {
    case 0x12CD:
        for (i = 0; i < duel_w_08914920; i++) {
            switch (duel_chain.kinds[i]) {
            case 6:
            case 7:
            case 8:
                if (p == duel_chain.srcs[i].s_player && z == duel_chain.srcs[i].s_zone) {
                    return duel_field_atk(p, z) < 2001;
                }
            }
        }
        break;
    case 0x184B:
        return (unsigned short)duel_field_type(p, z) == 7;
    }
    return 0;
}

int duel_0885746C(DuelAct *a, unsigned int p, int z)
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
    if (duel_slot_ef098(p, z, a->id) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885759C(DuelAct *a, unsigned int p, int z)
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
    if (a->n7 != 0) {
        return a->w[0] == duel_field_atk(p, z);
    }
    return 1;
}

int duel_08857668(DuelAct *a, unsigned int p, int z)
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
    return duel_act_can_target(a, p, z) != 0;
}

int duel_08857710(DuelAct *a, unsigned int p, int z)
{
    int q;
    int k;

    if (z < 5 || z >= 10) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (a->w[2] == 2) {
        for (q = 0; q < 2; q++) {
            for (k = 0; k <= 4; k++) {
                if (duel.players[q & 1].field[k].card.id != 0 && duel_slot_has_src10(p, z, q, k) != 0) {
                    if (a->player != q || a->zone != k) {
                        if (duel_slot_can_act(p, z, a->player, a->zone) != 0) {
                            return 1;
                        }
                    }
                    return 0;
                }
            }
        }
        return 0;
    }
    return duel_0885C594(a, p, z);
}

int duel_088578B8(DuelAct *a, unsigned int p, int z)
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
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_088579A4(DuelAct *a, unsigned int p, int z)
{
    if (p == a->player) {
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
    if (a->kind == 6 || a->kind == 7) {
        if (p != a->x.s_player) {
            return 0;
        }
        if (z != a->x.s_zone) {
            return 0;
        }
        switch (a->id) {
        case 0x12E4:
            return duel_field_atk(p, z) > 999;
        case 0x124F:
            return duel_field_def(p, z) < 501;
        case 0x1250:
            return duel_field_atk(p, z) < 501;
        }
    }
    return 0;
}
