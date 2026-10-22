// @range 0x08858A30-0x088594D0
// @asm 0x08858E2C-0x08859120
/* Computer player: more target tests (DuelAct *a, p, z). */
#include "duel.h"

int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_slot_has_src10(unsigned int p, int z, int q, int k);
int duel_slot_efb84(unsigned int p, int z);
int duel_slot_ef098(unsigned int p, int z, int id);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
unsigned int duel_field_type(unsigned int p, int z);
int card_is_attribute8(int id);
int duel_field_locate(int uid);
int duel_tributable(unsigned int p);
int duel_count_free_monster_zones(int p);
int duel_zone_usable(unsigned int p, unsigned int z);
int duel_field_same_name(unsigned int p, int z, int id);
int duel_field_attr_is(unsigned int p, int z, int attr);
int duel_field_level(unsigned int p, int z);
int duel_088BFFD8(unsigned int p, int id, int i);
int card_get_kind(int id);
int field_get_card(unsigned int p, int z);
int card_is_in_group_1a6x(int id);
int card_is_attribute9(int id);
int duel_field_e5ae0(unsigned int p, int z);

int duel_08858A30(DuelAct *a, unsigned int p, int z)
{
    if (z < 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_slot_has_src10(p, z, a->player, a->zone) == 0) {
        return 0;
    }
    return duel_slot_efb84(p, z) != 0;
}

int duel_08858AE4(DuelAct *a, unsigned int p, int z)
{
    unsigned int t;

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
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_slot_ef098(p, z, a->id) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    t = (unsigned short)duel_field_type(p, z);
    switch (a->id) {
    case 0x13B0:
        return t == 7;
    case 0x14CD:
        return t == 3;
    case 0x14CE:
        return t == 1;
    case 0x15D8:
        return card_is_attribute8(duel.players[p & 1].field[z].card.id);
    }
    return 1;
}

int duel_08858C68(DuelAct *a, unsigned int p, int z)
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
    return duel_slot_effect_kind(p, z, a->id, 4) != 0;
}

int duel_08858D08(DuelAct *a, unsigned int p, int z)
{
    unsigned int t;
    unsigned short u;
    int r;

    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (z < 5) {
        return 0;
    }
    if (duel_act_can_affect(a, p, z, 0) == 0) {
        return 0;
    }
    t = a->idx;
    u = t;
    if (p != (t & 0xFF)) {
        return 0;
    }
    if (z != (u >> 8 & 0xFF)) {
        return 0;
    }
    r = duel_field_locate(a->uid);
    if (r < 0) {
        return 0;
    }
    return duel_slot_has_src10(p, z, r & 0xFF, (unsigned short)r >> 8 & 0xFF) != 0;
}

#ifdef NON_MATCHING /* só o escalonamento do prólogo (lhu de a->kind) difere */
int duel_08858E2C(DuelAct *a, unsigned int p, int z)
{
    unsigned int k = a->kind;
    int id = duel.players[p & 1].field[z].card.id;
    int lv;

    if (k == 0x23) {
        return 1;
    }
    if (id == 0) {
        return 0;
    }
    if (duel_tributable(a->player) == 0) {
        return 0;
    }
    if (a->player != p && duel.players[p & 1].field[z].faceup == 0) {
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
    case 0x13FC:
        return (unsigned short)duel_field_type(p, z) == 0xA;
    case 0x140B:
    case 0x15A3:
        lv = duel_field_level(p, z);
        if (lv == 0) {
            return 0;
        }
        if (duel_088BFFD8(a->player, a->id, lv) != 0) {
            return 1;
        }
        return 0;
    case 0x1713:
        return duel_field_same_name(p, z, 0xFC9);
    case 0x1715:
        return (unsigned short)duel_field_type(p, z) == 0x14;
    case 0x187E:
        return duel_field_same_name(p, z, 0x13C3);
    case 0x1927:
        return duel_field_attr_is(p, z, 5);
    case 0x19B1:
    case 0x19B5:
        return duel_field_same_name(p, z, 0x18F6);
    case 0x1A2B:
    case 0x1A2C:
    case 0x1A2D:
    case 0x1A2E:
        if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        return duel_field_attr_is(p, z, card_get_kind(a->id));
    }
    return 1;
}
#endif

int duel_08859120(DuelAct *a, unsigned int p, int z)
{
    DuelSlot *s;

    if (p != a->player) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    s = &duel.players[p & 1].field[z];
    if (s->card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    switch (a->id) {
    case 0x12CF:
    case 0x153D:
    case 0x15AE:
        return 1;
    case 0x1409:
        return (unsigned short)duel_field_type(p, z) == 7;
    case 0x14DF:
        return (unsigned short)duel_field_type(p, z) == 1 && !(duel_field_level(p, z) < 5);
    case 0x143F:
    case 0x16A3:
        switch (field_get_card(p, z)) {
        case 0x1532:
        case 0x161E:
        case 0x1656:
        case 0x1685:
        case 0x1686:
            return 1;
        }
        break;
    case 0x17AB:
        return duel_field_e5ae0(p, z) != 0 && !card_is_attribute9(s->card.id) && duel_field_level(p, z) == 2;
    case 0x17AC:
        return duel_field_e5ae0(p, z) != 0 && !card_is_attribute9(s->card.id) && duel_field_level(p, z) == 1;
    case 0x17F9:
        return duel_field_attr_is(p, z, 3);
    case 0x1882:
        return (unsigned short)duel_field_type(p, z) == 0xB;
    case 0x18CD:
        switch (field_get_card(p, z)) {
        case 0x185A:
        case 0x18B8:
        case 0x18B9:
            return 1;
        }
        break;
    case 0x18D6:
        return field_get_card(p, z) == 0x18B7;
    case 0x1A60:
        return card_is_in_group_1a6x(field_get_card(p, z));
    }
    return 0;
}

