// @range 0x08857B34-0x08858A30
// @asm 0x08858440-0x0885861C
/* Computer player: more target tests (DuelAct *a, p, z). */
#include "duel.h"

int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
unsigned int duel_field_flag9(unsigned int p, int z);
int card_get_type(int id);
int duel_card_fits_zone(unsigned int p, int z, int id);
unsigned short duel_slot_e1854(unsigned int p, int z);
int duel_slot_can_act(unsigned int p, unsigned int z, int x, int y);
int duel_tributable(unsigned int p);
int duel_count_free_monster_zones(unsigned int p);
int duel_zone_usable(unsigned int p, unsigned int z);
int duel_field_same_name(unsigned int p, int z, int id);
int field_get_card(unsigned int p, int z);
int card_is_in_group_18be(int id);
int card_is_attribute9(int id);
int duel_slot_has_effect(unsigned int p, int z, int id);

int duel_08857B34(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (card_get_type(id) != 0x17) {
        return 0;
    }
    if (duel.players[p & 1].field[z].flags >> 8 & 1) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_08857C7C(DuelAct *a, unsigned int p, int z)
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
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return !duel.players[p & 1].field[z].pos;
}

int duel_08857D98(DuelAct *a, unsigned int p, int z)
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
    return duel_act_can_affect(a, p, z, 0) != 0;
}

int duel_08857E84(DuelAct *a, unsigned int p, int z)
{
    unsigned short s;
    int id;
    int q;
    int k;

    if (z < 5 || z > 10) {
        return 0;
    }
    id = duel.players[p & 1].field[z].card.id;
    if (id == 0) {
        return 0;
    }
    if (duel_card_fits_zone(p, z, id) == 0) {
        return 0;
    }
    s = duel_slot_e1854(p, z);
    if (s != 0xFFFF) {
        for (q = 0; q < 2; q++) {
            for (k = 0; k < 5; k++) {
                if (s != (unsigned short)((unsigned char)q | (unsigned char)k << 8)
                    && duel_act_can_target(a, q, k) != 0 && duel_slot_can_act(p, z, q, k) != 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int duel_08857FEC(DuelAct *a, unsigned int p, int z)
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
    return duel_act_can_affect(a, p, z, 0) != 0;
}

int duel_088580B8(DuelAct *a, unsigned int p, int z)
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
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_088581C0(DuelAct *a, unsigned int p, int z)
{
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    return a->uid != CARD_UID(duel.players[p & 1].field[z].card);
}

int duel_08858290(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    return !duel.players[p & 1].field[z].faceup;
}

int duel_08858304(DuelAct *a, unsigned int p, int z)
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
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_act_can_affect(a, p, z, 1) == 0) {
        return 0;
    }
    return !duel.players[p & 1].field[z].faceup;
}

#ifdef NON_MATCHING /* só a ordem do lhu de a->kind no prólogo difere */
int duel_08858440(DuelAct *a, unsigned int p, int z)
{
    if (a->kind == 0x23) {
        return 1;
    }
    if (duel_tributable(a->player) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
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
    switch (a->id) {
    case 0x12C5:
        return duel_field_same_name(p, z, 0xFE0);
    case 0x167D:
        return duel_field_same_name(p, z, 0xFC9);
    case 0x1768:
        return card_is_in_group_18be(field_get_card(p, z));
    }
    return 1;
}
#endif

int duel_0885861C(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (p != a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (id == 0) {
        return 0;
    }
    if (card_is_attribute9(id) != 0) {
        return 0;
    }
    return !duel_slot_has_effect(p, z, a->id);
}

int duel_08858704(DuelAct *a, unsigned int p, int z)
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
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_088587EC(DuelAct *a, unsigned int p, int z)
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
    return !duel_act_immune(a, p, z);
}

int duel_08858898(DuelAct *a, unsigned int p, int z)
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
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_0885898C(DuelAct *a, unsigned int p, int z)
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
    return duel.players[p & 1].field[z].faceup != 0;
}
