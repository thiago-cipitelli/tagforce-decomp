// @range 0x088EA23C-0x088EAC0C
// @asm 0x088EA784-0x088EAA1C
/* Free monster zones in placement order, and the monsters that can be tributed. */
#include "duel.h"
#include "card_db.h"

extern int duel_zone_order[5];         /* 0x08911DA0 center first */

int duel_zone_free(unsigned int p, int z);
int duel_count_free_monster_zones(unsigned int p);
int field_get_card(unsigned int p, int z);
int duel_count_active(int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_field_attr_is(unsigned int p, int z, int attr);
int duel_field_atk(unsigned int p, int z);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
unsigned int duel_field_type(unsigned int p, int z);
int duel_first_free_zone(unsigned int p);
int duel_tributable(unsigned int p, unsigned int q, int z);

int duel_count_free_zones(unsigned int p)
{
    int i;
    int n = 0;

    for (i = 0; i < 5; i++) {
        if (duel_zone_free(p, i)) {
            n++;
        }
    }
    return n;
}

int duel_pick_monster_zone(unsigned int p)
{
    int z;

    if (duel_count_free_monster_zones(p) != 0) {
        z = duel_first_free_zone(p);
    } else {
        z = -1;
    }
    return z;
}

/* First free monster zone in placement order (center first). */
int duel_first_free_zone(unsigned int p)
{
    int i;

    for (i = 0; i < 5; i++) {
        int z = duel_zone_order[i];

        if (duel_zone_free(p, z)) {
            return z;
        }
    }
    return -1;
}

/* Player p can tribute monster z of player q (cards that can never be tributed excluded). */
int duel_can_tribute(unsigned int p, unsigned int q, int z)
{
    int id = field_get_card(q, z);

    switch (id) {
    case 0x14F8:
    case 0x14F9:
    case 0x1422:
    case 0x1670:
    case 0x154F:
    case 0x1550:
    case 0x1551:
    case 0x1671:
    case 0x1672:
        return 0;
    case 0x19EE:
        if (duel.players[q & 1].field[z].flags >> 18 & 1) {
            return 0;
        }
        break;
    case 0x1836:
        if (SLOT_ACTIVE(&duel.players[q & 1].field[z])) {
            return 0;
        }
        break;
    }
    return duel_tributable(p, q, z);
}

int duel_tributable(unsigned int p, unsigned int q, int z)
{
    int id = field_get_card(q, z);

    if (id == 0) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel_count_active(0x13F2) > 0) {
        return 0;
    }
    if (p != q && duel_slot_effect_with(q, z, 0x13EB, p) == 0) {
        return 0;
    }
    if (duel_slot_has_effect(q, z, 0x16A4)) {
        return 0;
    }
    if (duel_slot_has_effect(q, z, 0x12D1)) {
        return 0;
    }
    switch (id) {
    case 0x1900:
    case 0x1AAD:
        if (SLOT_ACTIVE(&duel.players[q & 1].field[z])) {
            return 0;
        }
    }
    return 1;
}

/* Monster z of player q counts as two tributes for card id of player p. */
int duel_double_tribute(unsigned int p, int id, unsigned int q, int z)
{
    if ((duel.players[q & 1].field[z].flags >> 5 & 1) == 0 && (q == p || duel.players[q & 1].field[z].faceup != 0)) {
        switch (field_get_card(q, z)) {
        case 0x1521:
        case 0x1798:
        case 0x17E8:
        case 0x17E9:
        case 0x1873:
        case 0x1874:
            return card_get_kind(id) == card_get_kind(field_get_card(q, z));
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* same flow; mwcc keeps copies of the first instruction of two blocks it also put in delay slots */
/* Tributes player p can offer for card id. */
int duel_count_tributes(unsigned int p, int id, int flag)
{
    int q;
    int z;
    int n = 0;

    if (duel_count_active(0x13F2) > 0) {
        return 0;
    }
    for (q = 0; q < 2; q++) {
        for (z = 0; z < 5; z++) {
            if (duel.players[q & 1].field[z].card.id != 0 && duel_can_tribute(p, q, z)) {
                int ok = 1;

                switch (id) {
                case 0x164A:
                case 0x164B:
                case 0x164C:
                case 0x164D:
                case 0x164E:
                case 0x164F:
                    if (flag != 0) {
                        int s;

                        ok = 0;
                        for (s = 5; s < 10; s++) {
                            if (duel.players[p & 1].field[s].card.id == card_id_remap_164a(id) &&
                                duel.players[p & 1].field[s].faceup != 0 && duel_st_affects(q, z, p, s) == 0) {
                                ok = 1;
                            }
                        }
                    }
                    break;
                case 0x194C:
                    ok = q == p || duel.players[q & 1].field[z].faceup != 0;
                    if (ok) {
                        ok = duel_field_atk(q, z) >= 2000;
                    }
                    break;
                case 0x1A40:
                    ok = q == p || duel.players[q & 1].field[z].faceup != 0;
                    if (ok) {
                        ok = duel_field_attr_is(q, z, card_get_kind(id)) != 0;
                    }
                    break;
                }
                if (ok) {
                    n += duel_double_tribute(p, id, q, z) + 1;
                }
            }
        }
    }
    return n;
}
#endif

/* Tributable monsters for player p, the card at `except` (player | zone << 8) excluded. */
int duel_count_tributable_except(unsigned int p, unsigned int except)
{
    int q;
    int z;
    int n = 0;

    if (duel_count_active(0x13F2) > 0) {
        return 0;
    }
    for (q = 0; q < 2; q++) {
        for (z = 0; z < 5; z++) {
            if (duel.players[q & 1].field[z].card.id != 0 && except != (unsigned short)(q & 0xFF | (z & 0xFF) << 8)) {
                n += duel_tributable(p, q, z);
            }
        }
    }
    return n;
}

/* Tributable monsters for player p of effective type `type`. */
int duel_count_tributable_type(unsigned int p, unsigned int type)
{
    int q;
    int z;
    int n = 0;

    if (duel_count_active(0x13F2) > 0) {
        return 0;
    }
    for (q = 0; q < 2; q++) {
        for (z = 0; z < 5; z++) {
            if (duel.players[q & 1].field[z].card.id != 0 && (q == p || duel.players[q & 1].field[z].faceup != 0) &&
                type == (duel_field_type(q, z) & 0xFFFF)) {
                n += duel_tributable(p, q, z);
            }
        }
    }
    return n;
}
