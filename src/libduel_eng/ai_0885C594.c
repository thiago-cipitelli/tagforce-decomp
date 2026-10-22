// @range 0x0885C594-0x0885D4E4
/* The computer player: target filters (DuelAct *a, p, z), tributes and zone checks. */
#include "duel.h"

int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_slot_efb84(unsigned int p, int z);
int duel_slot_has_src10(unsigned int p, unsigned int z, unsigned int q, int sz);
int field_get_card(unsigned int p, int z);
int card_is_attribute7(unsigned short id);
int card_is_attribute9(unsigned short id);
int card_is_in_group_158d(int id);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_count_free_monster_zones(unsigned int p);
int duel_zone_usable(unsigned int p, unsigned int z);
int duel_0884E698(DuelAct *a, int (*fn)(DuelAct *a, unsigned int p, int z), unsigned int skip);
int duel_08853A28(DuelAct *a, unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_field_def(unsigned int p, int z);
int duel_field_flag9(unsigned int p, int z);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_0885FA30(DuelAct *a, unsigned int p, int z);
int duel_08855DC8(DuelAct *a, unsigned int p, int z);
int duel_088580B8(DuelAct *a, unsigned int p, int z);
int duel_08856860(DuelAct *a, unsigned int p, int z);

extern unsigned int duel_w_08914920;

int duel_0885C594(DuelAct *a, unsigned int p, unsigned int z)
{
    DuelPlayer *pl;
    int q;
    int i;

    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    for (q = 0; q < 2; q++) {
        pl = &duel.players[q & 1];
        for (i = 0; i <= 4; i++) {
            if (pl->field[i].card.id != 0 && duel_slot_has_src10(p, z, q, i) != 0) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_0885C694(DuelAct *a, unsigned int p, int z)
{
    if (a->player == p) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_special.player[0] != p || duel_special.zone[0] != z) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885C7A8(DuelAct *a, unsigned int p, int z)
{
    if (z > 4) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].pos == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_act_can_affect(a, p, z, 1) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885C8A0(DuelAct *a, unsigned int p, int z)
{
    int id;

    if (a->player == p) {
    int id;

        return 0;
    }
    if (z > 4) {
    int id;

        return 0;
    }
    id = duel.players[p & 1].field[z].card.id;
    if (id == 0) {
    int id;

        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
    int id;

        return 0;
    }
    return card_is_attribute7(id) != 0;
}

int duel_0885C948(DuelAct *a, unsigned int p, int z)
{
    int id = field_get_card(p, z);

    if (z > 4) {
        return 0;
    }
    if (card_is_in_group_158d(id) == 0) {
        return 0;
    }
    if (id == 0x158C) {
        return 0;
    }
    if (duel_tributable(a->player, p, z) != 0) {
        if (a->player == p || duel.players[p & 1].field[z].faceup != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_0885CA40(DuelAct *a, unsigned int p, int z)
{
    if (a->kind == 0x23) {
        return 1;
    }
    if (duel_tributable(a->player, p, z) == 0) {
        return 0;
    }
    if (duel_count_free_monster_zones(a->player) == 0) {
        if (a->player != p) {
            return 0;
        }
        if (duel_zone_usable(p, z) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_0885CAF4(DuelAct *a, unsigned int p, int z)
{
    unsigned int q;

    if (a->w[1] != 0) {
        return duel_08853A28(a, p, z);
    }
    q = a->player;
    if (p != q) {
        return 0;
    }
    if (z < 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_slot_has_src10(p, z, q, a->zone) == 0) {
        return 0;
    }
    if (duel_slot_efb84(p, z) == 0) {
        return 0;
    }
    return duel_0884E698(a, duel_08853A28, (unsigned short)((unsigned char)p | (unsigned char)z << 8)) != 0;
}

int duel_0885CC08(DuelAct *a, unsigned int p, int z)
{
    if (duel_tributable(a->player, p, z) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    if (duel_count_free_monster_zones(a->player) == 0) {
        if (a->player != p) {
            return 0;
        }
        if (duel_zone_usable(p, z) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_0885CCBC(DuelAct *a, unsigned int p, int z)
{
    if (z > 4) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    return a->uid != CARD_UID(duel.players[p & 1].field[z].card);
}

int duel_0885CD48(DuelAct *a, unsigned int p, int z)
{
    int id;

    if (a->player != p) {
    int id;

        return 0;
    }
    if (z >= 5) {
    int id;

        return 0;
    }
    id = duel.players[p & 1].field[z].card.id;
    if (id == 0) {
    int id;

        return 0;
    }
    if (card_is_attribute9(id) != 0) {
    int id;

        return 1;
    }
    return duel_slot_efb84(p, z) != 0;
}

int duel_0885CE00(DuelAct *a, unsigned int p, int z)
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
    return !duel_act_immune(a, p, z);
}

int duel_0885CE8C(DuelAct *a, unsigned int p, int z)
{
    unsigned int i;
    struct DuelActX *x;

    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_field_atk(p, z) > 1000) {
        return 0;
    }
    if (a->w[0] == 0) {
        for (i = 0; i < duel_w_08914920; i++) {
            switch (duel_chain.kinds[i]) {
            case 6:
            case 7:
            case 8:
                x = &duel_chain.srcs[i];
                if (p == x->s_player && z == x->s_zone) {
                    return 1;
                }
            }
        }
    } else {
        return duel_slot_effect_with(p, z, 0x1518, a->uid);
    }
    return 0;
}

int duel_0885D014(DuelAct *a, unsigned int p, int z)
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
    if (duel.players[p & 1].field[z].pos == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885D0C0(DuelAct *a, unsigned int p, int z)
{
    if (duel_0885FA30(a, p, z) == 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 0) != 0;
}

int duel_0885D120(DuelAct *a, unsigned int p, int z)
{
    switch (a->w[1]) {
    case 0x1706:
        return duel_08855DC8(a, p, z);
    case 0x1709:
        return duel_088580B8(a, p, z);
    }
    return duel_08856860(a, p, z);
}

int duel_0885D180(DuelAct *a, unsigned int p, int z)
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
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_0885D284(DuelAct *a, unsigned int p, int z)
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
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885D370(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (id == 0) {
        return 0;
    }
    if (card_is_attribute9(id) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_0885D418(DuelAct *a, unsigned int p, int z)
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
        return a->w[0] == duel_field_def(p, z);
    }
    return 1;
}
