// @range 0x088EF170-0x088EF5EC
/* Effect immunity of monster z of player p: bit 1 against its controller's effects, bit 0 against
   the opponent's (cards with built-in immunity, 0x1693, 0x1667, 0x17A1), and the same for the
   effects that target it (0x19C3, 0x194E, 0x1437). */
#include "duel.h"
#include "card_db.h"

typedef struct DuelAct {               /* card being activated */
    unsigned short id;
    unsigned char player;              /* +0x02 bit 0 */
} DuelAct;

extern int duel_effects_off;           /* 0x08914914 */

int duel_count_active(int id);
int duel_is_cur_card(int id);
int field_get_card(unsigned int p, int z);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_slot_e1390(unsigned int p, int z, int id, unsigned int q);
int duel_slot_e5a48(unsigned int p, int z);
unsigned int duel_slot_immunity(unsigned int p, int z);
unsigned int duel_slot_ref(unsigned int p, int z);

unsigned int duel_slot_immunity(unsigned int p, int z)
{
    int id = field_get_card(p, z);
    unsigned int r = 0;
    unsigned int q;

    if (id == 0) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel_effects_off) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (!(duel.players[p & 1].field[z].flags >> 5 & 1) && !(duel.players[p & 1].field[z].flags >> 1 & 1)) {
        switch (id) {
        case 0x13CD:
        case 0x12A8:
        case 0x1706:
        case 0x1709:
            if (duel_is_cur_card(0x10F4) && !(duel.players[p & 1].field[z].flags >> 5 & 1)) {
                return 3;
            }
            break;
        case 0x164E:
        case 0x1742:
        case 0x17D1:
        case 0x17D3:
            return 3;
        case 0x1814:
        case 0x181A:
            r |= 2;
            break;
        }
    }
    if (duel_slot_e1390(p, z, 0x1693, p)) {
        r |= 2;
    }
    q = 1 - p;
    if (duel_slot_e1390(p, z, 0x1693, q)) {
        r |= 1;
    }
    if (duel_slot_e5a48(p, z) == 0) {
        if (duel_count_active(0x1667)) {
            return 3;
        }
        if (duel_find_effect_card(p, 0x17A1, -1)) {
            r |= 2;
        }
        if (duel_find_effect_card(q, 0x17A1, -1)) {
            r |= 1;
        }
    }
    return r;
}

int duel_slot_immune(unsigned int p, int z, int opp)
{
    return (int)(duel_slot_immunity(p, z) & (opp + 1)) > 0;
}

unsigned int duel_slot_ref(unsigned int p, int z)
{
    int id = field_get_card(p, z);

    if (id == 0) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel_effects_off) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (!(duel.players[p & 1].field[z].flags >> 5 & 1) && !(duel.players[p & 1].field[z].flags >> 1 & 1)) {
        switch (id) {
        case 0x1437:
        case 0x194E:
        case 0x19C3:
            return 3;
        }
    }
    return 0;
}

int duel_slot_ref_immune(unsigned int p, int z, int opp)
{
    return (int)(duel_slot_ref(p, z) & (opp + 1)) > 0;
}

/* Monster z of player p is immune to card `a` (traps by the targeting immunity, spells and monster
   effects in the spell/trap zones by the effect immunity). */
int duel_act_immune(DuelAct *a, unsigned int p, int z)
{
    if (card_get_type(a->id) == 0x17) {
        return duel_slot_ref_immune(p, z, p != (a->player & 1));
    }
    if (card_get_type(a->id) == 0x16 || (z >= 5 && z < 10 && card_is_monster(a->id))) {
        return duel_slot_immune(p, z, p != (a->player & 1));
    }
    return 0;
}
