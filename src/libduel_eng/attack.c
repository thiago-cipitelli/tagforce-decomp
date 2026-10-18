// @range 0x088EBF44-0x088EC5D8
/* Whether monster z of player p may declare an attack: restrictions by card, state and effects. */
#include "duel.h"
#include "card_db.h"

int duel_count_monsters(unsigned int p);
int field_get_card(unsigned int p, int z);
int duel_is_cur_card(int id);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_affected_ex(unsigned int p, int z, int id);
unsigned int duel_slot_effect_arg(unsigned int p, int z, int id);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
int duel_slot_counter(unsigned int p, int z, int id);
int duel_slot_count_14c0(unsigned int p, int z);
int duel_grave_count_id(unsigned int p, int id);
int duel_attack_check(unsigned int p, int z, int arg);
int duel_can_direct_attack(unsigned int p, int z);
int duel_direct_attack_kind(unsigned int p, int z, int arg);
int duel_can_attack_target(unsigned int p, int z, unsigned int q, int i, int arg);
int duel_can_attack(unsigned int p, int z, int arg);

/* Monster z of player p may not attack (it attacked already, or a card forbids it). */
int duel_attack_forbidden(unsigned int p, int z)
{
    unsigned int f = duel.players[p & 1].field[z].flags;

    if ((f >> 22 & 1) == 0) {
        return 0;
    }
    if ((f >> 23 & 1) == 0) {
        if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
            switch (field_get_card(p, z)) {
            case 0x174F:
                if (duel_is_cur_card(0x10F4) == 0) {
                    break;
                }
            case 0x1411:
            case 0x170A:
            case 0x1723:
            case 0x18B9:
            case 0x18FC:
            case 0x1A62:
                return 0;
            case 0x14DC:
                if (duel_slot_has_effect(p, z, field_get_card(p, z))) {
                    return 0;
                }
                break;
            case 0x14D5:
                if (duel_count_monsters(1 - p)) {
                    return 0;
                }
                break;
            case 0x19F8:
                if (duel_count_monsters(1 - p) && duel_special.uid == CARD_UID(duel.players[p & 1].field[z].card)) {
                    return 0;
                }
                break;
            }
        }
        if (duel_slot_effect_kind(p, z, 0x16CB, 4)) {
            return 0;
        }
        if (duel_slot_affected_ex(p, z, 0x1661)) {
            return 0;
        }
        if (duel_slot_has_effect(p, z, 0x18CB)) {
            return 0;
        }
        if (duel_slot_has_effect(p, z, 0x19AB)) {
            return 0;
        }
    }
    if (duel_slot_has_effect(p, 11, 0x15FF)) {
        return 0;
    }
    if (field_get_card(p, z) == 0x18A6 && duel_slot_has_effect(p, 11, 0x195B)) {
        return duel_slot_counter(p, z, 0x18A6) >= (int)duel_slot_effect_arg(p, z, 0x195B);
    }
    if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
        switch (field_get_card(p, z)) {
        case 0x1505:
        case 0x1644:
        case 0x1958:
            return duel_slot_effect_arg(p, z, 0x1505) == 0;
        case 0x186C:
            return duel_slot_counter(p, z, 0x186C) >= duel_slot_count_14c0(p, z) + 1;
        case 0x19C1:
            return duel_slot_counter(p, z, 0x19C1) >= duel_grave_count_id(p, 0x19C1) + 1;
        case 0x1A4E:
            if (duel_slot_effect_arg(p, z, 0x1505) == 0) {
                return 1;
            }
            return (unsigned int)duel_slot_counter(p, z, 0x1A4E) >= duel.players[p & 1].field[z].link;
        case 0x1A79:
            if (duel.players[p & 1].field[z].link != 0) {
                return 0;
            }
            break;
        }
    }
    return 1;
}

/* Monster z of player p has at least one attack: direct, or on some monster of the opponent. */
int duel_can_attack_any(unsigned int p, int z, int arg)
{
    int i;

    if (duel_can_attack(p, z, arg) == 0) {
        return 0;
    }
    if (duel_can_direct_attack(p, z)) {
        return 1;
    }
    if (duel_direct_attack_kind(p, z, 1)) {
        return 1;
    }
    for (i = 0; i <= 4; i++) {
        if (duel_can_attack_target(p, z, 1 - p, i, 1)) {
            return 1;
        }
    }
    return 0;
}

/* Monster z of player p may attack: face-up, in attack position (0x1956 and 0x12B4 also in
   defense), not forbidden. */
int duel_can_attack(unsigned int p, int z, int arg)
{
    int id = field_get_card(p, z);

    if (id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].pos != 0) {
        switch (id) {
        case 0x12B4:
            if (duel.players[p & 1].field[z].flags >> 5 & 1) {
                return 0;
            }
            break;
        case 0x1956:
            if (duel.players[p & 1].field[z].flags >> 5 & 1) {
                return 0;
            }
            if (duel_count_monsters(1 - p)) {
                return 0;
            }
            break;
        default:
            return 0;
        }
    }
    if (duel_attack_forbidden(p, z)) {
        return 0;
    }
    return duel_attack_check(p, z, arg);
}
