// @range 0x08855214-0x08855908
/* Computer player: target test by card id for a face-up monster of the field. */
#include "duel.h"

int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
unsigned int duel_field_type(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_field_def(unsigned int p, int z);
int duel_field_base_atk(unsigned int p, int z);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_field_same_name(unsigned int p, int z, int id);
int duel_field_attr_is(unsigned int p, int z, int attr);
int duel_count_free_monster_zones(unsigned int p);
unsigned short duel_slot_src_attr8(unsigned int p, int z, int active);
int field_get_card(unsigned int p, int z);
int card_get_atk(int id);
int card_is_in_group_1911(int id);
int card_attribute_to_group(int id);

int duel_08855214(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;
    unsigned int t;
    unsigned int q;
    unsigned char k;
    int x;
    int r;
    DuelStats st;

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
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    t = (unsigned short)duel_field_type(p, z);
    switch (a->id) {
    case 0x119A:
        return t == 1;
    case 0x14B9:
        return a->uid == CARD_UID(duel.players[p & 1].field[z].card) ? 0 : t == 3;
    case 0x14C5:
        k = a->zone;
        q = a->player;
        if (a->uid != CARD_UID(duel.players[q & 1].field[k].card)) {
            x = card_get_atk(duel.list[a->uid].id);
        } else {
            x = duel_field_atk(q, k);
        }
        return !(x < duel_field_def(p, z));
    case 0x14D3:
        return t == 0xF || t == 0x12;
    case 0x14E0:
        return duel_field_def(p, z) < 801;
    case 0x15E4:
        r = 0;
        if ((duel.players[p & 1].field[z].flags >> 22 & 1) + (duel.players[p & 1].field[z].flags >> 23 & 1) != 0
            && duel_slot_src_attr8(p, z, 0) != 0xFFFF) {
            r = 1;
        }
        return r;
    case 0x1690:
        return card_is_in_group_1911(field_get_card(p, z));
    case 0x16C0:
        k = a->zone;
        q = a->player;
        if (a->uid == CARD_UID(duel.players[q & 1].field[k].card) && duel.players[q & 1].field[k].faceup != 0) {
            x = duel_field_atk(p, z);
            return duel_field_atk(q, k) < x;
        }
        break;
    case 0x16CE:
        return t == 0xB || t == 0xC;
    case 0x186A:
        return duel_field_atk(p, z) <= a->w[2] * 700;
    case 0x18C9:
        k = a->zone;
        q = a->player;
        if (a->uid == CARD_UID(duel.players[q & 1].field[k].card) && duel.players[q & 1].field[k].faceup != 0) {
            x = duel_field_base_atk(p, z);
            return x < duel_field_atk(q, k);
        }
        break;
    case 0x190E:
        return duel_field_atk(p, z) != 2000;
    case 0x1945:
        return duel_field_same_name(p, z, 0x17EE);
    case 0x19A9:
        k = a->zone;
        q = a->player;
        if (a->uid == CARD_UID(duel.players[q & 1].field[k].card) && duel.players[q & 1].field[k].faceup != 0) {
            x = duel_field_atk(q, k);
            duel_field_stats(p, z, &st);
            return st.atk >= x || st.def >= x;
        }
        break;
    case 0x19F1:
        return duel_field_attr_is(p, z, 5);
    case 0x1A6D:
        return a->uid == CARD_UID(duel.players[p & 1].field[z].card) ? 0 : card_attribute_to_group(id);
    case 0x1A7C:
        return duel_count_free_monster_zones(p) != 0;
    }
    return 0;
}
