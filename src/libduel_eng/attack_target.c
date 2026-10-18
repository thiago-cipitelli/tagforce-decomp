// @range 0x088ED2E0-0x088EDE6C
/* Monster z of player p may attack monster i of player q: restrictions of the attacker, cards
   that protect or redirect, and (when `arg`) the conditions checked when the attack is declared. */
#include "duel.h"
#include "card_db.h"

int duel_count_monsters(unsigned int p);
int duel_count_monsters_state(unsigned int p, int pos, int state);
int duel_count_type(unsigned int p, unsigned int type);
int duel_count_type_skip(unsigned int p, unsigned int type, int skip);
int duel_has_faceup_attr6(unsigned int p);
int duel_is_cur_card(int id);
int field_get_card(unsigned int p, int z);
unsigned int duel_field_type(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_field_e5ae0(unsigned int p, int z);
int duel_field_level(unsigned int p, int z);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_slot_effect_arg(unsigned int p, int z, int id);
int duel_slot_affected(unsigned int p, int z, int id);
int duel_slot_affected_ex(unsigned int p, int z, int id);
int duel_slot_e12e0(unsigned int p, int z, int id, unsigned int q);
int duel_slot_e1390(unsigned int p, int z, int id, unsigned int q);
unsigned int duel_slot_immunity(unsigned int p, int z);
unsigned int duel_slot_ref(unsigned int p, int z);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_count_e17a0(unsigned int p, int id);
int duel_card_check_9148(unsigned int p, int id);

int duel_can_attack_target(unsigned int p, int z, unsigned int q, int i, int arg)
{
    int id = field_get_card(p, z);
    int tid = field_get_card(q, i);
    unsigned int imm = duel_slot_immunity(p, z);
    unsigned int ref = duel_slot_ref(p, z);
    int j;
    int n;

    if (tid == 0) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x15FF)) {
        if (duel_slot_effect_arg(1 - p, 11, 0x15FF) == 0) {
            return 0;
        }
        if (duel_slot_effect_with(q, i, 0x15FF, 0)) {
            return 0;
        }
    }
    if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
        switch (id) {
        case 0x14FE:
            return 0;
        case 0x1505:
        case 0x1644:
        case 0x1958:
            if (duel_slot_effect_arg(p, z, 0x1505) == 0) {
                return 0;
            }
            if (duel_st_affects(q, i, p, z)) {
                return 0;
            }
            break;
        case 0x1A4E:
            if (duel_slot_effect_arg(p, z, 0x1505) == 0) {
                return 0;
            }
            break;
        case 0x1A79:
            if (duel.players[p & 1].field[z].link != 0 && duel_st_affects(q, i, p, z)) {
                return 0;
            }
            break;
        }
    }
    if (duel_slot_affected_ex(p, z, 0x1561)) {
        return 0;
    }
    if (duel_slot_affected_ex(p, z, 0x1852)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1669)) {
        return duel_slot_has_effect(q, i, 0x1669);
    }
    if ((int)(imm & 2) <= 0 && duel_count_e17a0(q, 0x1318) > 0) {
        return duel_slot_affected(q, i, 0x1318);
    }
    if ((int)(ref & 2) <= 0) {
        n = 0;
        for (j = 0; j <= 4; j++) {
            if (duel.players[q & 1].field[j].card.id != 0 && duel_slot_has_effect(q, j, 0x17FC)) {
                if (j == i) {
                    n = 0;
                    break;
                }
                n++;
            }
        }
        if (n) {
            return 0;
        }
        if (duel_slot_has_effect(p, 11, 0x17FD)) {
            return 0;
        }
    }
    for (j = 0; j <= 4; j++) {
        if (j != i && duel.players[q & 1].field[j].card.id != 0 && duel_slot_e12e0(q, j, 0x15EA, q)) {
            return 0;
        }
    }
    if (SLOT_ACTIVE(&duel.players[q & 1].field[i])) {
        switch (tid) {
        case 0x1753:
            if (duel_count_type(q, 0xD)) {
                return 0;
            }
            break;
        case 0x1744:
        case 0x1755:
        case 0x1756:
        case 0x1863:
            if (duel_count_type_skip(q, card_get_type(tid), i)) {
                return 0;
            }
            break;
        case 0x179D:
            for (j = 0; j <= 4; j++) {
                if (duel.players[q & 1].field[j].card.id != 0 && duel.players[q & 1].field[j].faceup != 0 &&
                    duel_field_e5ae0(q, j) && duel_field_level(q, j) <= 3) {
                    return 0;
                }
            }
            break;
        }
    }
    if (duel.players[q & 1].field[i].faceup != 0) {
        switch ((unsigned short)duel_field_type(q, i)) {
        case 10:
            if (duel_find_effect_card(q, 0x1703, -1)) {
                return 0;
            }
            break;
        }
        if (card_is_in_group_160f(tid) && tid != 0x160F && duel_find_effect_card(q, 0x160F, -1)) {
            return 0;
        }
        if (card_is_in_group_1911(tid) && tid != 0x168C && duel_find_effect_card(q, 0x168C, -1)) {
            return 0;
        }
        if (card_is_in_group_18a4(tid) && tid != 0x18A0 && duel_find_effect_card(q, 0x18A0, -1)) {
            return 0;
        }
        if (duel_slot_e1390(q, i, 0x1980, 1 - p) && (int)(ref & 2) <= 0 && duel_field_atk(p, z) >= 1900) {
            return 0;
        }
    }
    if (arg != 0) {
        if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
            switch (id) {
            case 0x127D:
            case 0x127E:
            case 0x127F:
            case 0x12A5:
            case 0x154A:
                if (duel_has_faceup_attr6(q)) {
                    if (duel.players[q & 1].field[i].faceup == 0 || card_is_attribute6(duel.players[q & 1].field[i].card.id) == 0) {
                        return 0;
                    }
                }
                break;
            }
        }
        if (SLOT_ACTIVE(&duel.players[q & 1].field[i])) {
            switch (tid) {
            case 0x1399:
                if (duel_count_monsters(q) > 1) {
                    return 0;
                }
                break;
            case 0x13CD:
                if (duel_is_cur_card(0x10F4)) {
                    return 0;
                }
                break;
            case 0x164E:
                return 0;
            case 0x18B6:
                if (duel_count_monsters_state(q, 0, 1)) {
                    return 0;
                }
                break;
            }
        }
        if (duel.players[q & 1].field[i].faceup != 0) {
            switch ((unsigned short)duel_field_type(q, i)) {
            case 0x12:
                if (duel_find_effect_card(q, 0x16ED, i)) {
                    return 0;
                }
                break;
            case 0xF:
                if (duel_find_effect_card(q, 0x14C6, i)) {
                    return 0;
                }
                break;
            case 0x13:
                if (tid != 0x1A2F && duel_find_effect_card(q, 0x1A2F, i)) {
                    return 0;
                }
                break;
            }
            if (tid != 0x1A1E && card_is_in_group_183f(tid) && duel_find_effect_card(q, 0x1A1E, i)) {
                return 0;
            }
            if (tid != 0x1A56 && duel_find_effect_card(q, 0x1A56, i)) {
                return 0;
            }
        }
        if ((int)(imm & 2) <= 0 && duel_find_effect_card(q, 0x1777, -1) && duel_card_check_9148(q, 6000)) {
            if (duel.players[q & 1].field[i].faceup == 0) {
                return 0;
            }
            if (tid != 6000) {
                return 0;
            }
        }
        if (duel_find_effect_card(q, 0x18B1, -1)) {
            if (duel.players[q & 1].field[i].faceup == 0 && duel.players[q & 1].field[i].pos != 0) {
                return 0;
            }
        }
        for (j = 0; j < 5; j++) {
            if (j != i && field_get_card(q, j) == 0x19A4 && SLOT_ACTIVE(&duel.players[q & 1].field[j]) &&
                duel.players[q & 1].field[j].pos != 0) {
                return 0;
            }
        }
        if ((int)(ref & 2) <= 0 && duel_slot_has_effect(p, 11, 0x14D4) && duel.players[q & 1].field[i].faceup == 0) {
            return 0;
        }
    }
    return 1;
}

/* Monsters of the opponent that monster z of player p may attack. */
int duel_count_attack_targets(unsigned int p, int z)
{
    int i;
    int n = 0;

    for (i = 0; i <= 4; i++) {
        if (duel_can_attack_target(p, z, 1 - p, i, 1)) {
            n++;
        }
    }
    return n;
}
