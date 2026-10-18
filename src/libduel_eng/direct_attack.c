// @range 0x088EDE6C-0x088EE93C
/* Direct attacks: allowed for monster z of player p, and blocked while the opponent has a monster
   that can be attacked. */
#include "duel.h"
#include "card_db.h"

int duel_has_faceup_attr6(unsigned int p);
int duel_is_cur_card(int id);
int field_get_card(unsigned int p, int z);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_effect_arg(unsigned int p, int z, int id);
unsigned int duel_slot_ref(unsigned int p, int z);
int duel_has_attack_target(unsigned int p);
int duel_has_st_12be(unsigned int p);
int duel_no_faceup_attrs46(unsigned int p);
int duel_count_monsters_pos(unsigned int p, int faceup, int pos0);
int duel_count_field_cards(unsigned int p, int faceup, int facedown, int fzone);
int duel_field_atk(unsigned int p, int z);
int duel_slot_affected_ex(unsigned int p, int z, int id);

/* Monster z of player p is not stopped from attacking directly. */
int duel_direct_attack_allowed(unsigned int p, int z)
{
    unsigned int ref = duel_slot_ref(p, z);
    int j;

    if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
        switch (field_get_card(p, z)) {
        case 0x147D:
            return 0;
        case 0x1505:
        case 0x1644:
        case 0x1958:
        case 0x1A4E:
            if (duel.players[p & 1].field[z].flags >> 22 & 1) {
                return 0;
            }
            if (duel_slot_effect_arg(p, z, 0x1505) != -1) {
                return 0;
            }
            break;
        case 0x127D:
        case 0x127E:
        case 0x127F:
        case 0x12A5:
        case 0x154A:
            if (duel_has_faceup_attr6(1 - p)) {
                return 0;
            }
            break;
        case 0x1A79:
            if (duel.players[p & 1].field[z].link != 0 && duel_slot_effect_arg(p, z, 0x1505) == 1) {
                return 0;
            }
            break;
        }
    }
    if (duel_slot_has_effect(p, 11, 0x15FF) && duel_slot_effect_arg(1 - p, 11, 0x15FF) != -1) {
        return 0;
    }
    if ((int)(ref & 2) <= 0) {
        for (j = 0; j <= 4; j++) {
            if (duel.players[(1 - p) & 1].field[j].card.id != 0 && duel_slot_has_effect(1 - p, j, 0x17FC)) {
                return 0;
            }
        }
    }
    if (duel_slot_has_effect(p, z, 0x1619)) {
        return 0;
    }
    if (duel_slot_has_effect(p, z, 0x1890)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1669)) {
        return 0;
    }
    return !duel_slot_has_effect(p, 11, 0x195B);
}

/* Player p has a monster that can be attacked. */
int duel_has_attack_target(unsigned int p)
{
    int i;

    for (i = 0; i < 5; i++) {
        int id = field_get_card(p, i);

        if (id != 0) {
            if (SLOT_ACTIVE(&duel.players[p & 1].field[i])) {
                switch (id) {
                case 0x13CD:
                    if (duel_is_cur_card(0x10F4)) {
                        continue;
                    }
                    break;
                case 0x164E:
                    continue;
                }
            }
            return 1;
        }
    }
    return 0;
}

int duel_can_direct_attack(unsigned int p, int z)
{
    if (duel_direct_attack_allowed(p, z) == 0) {
        return 0;
    }
    return !duel_has_attack_target(1 - p);
}

/* How monster z of player p may attack directly: 0 not at all, 1 yes (card effects that allow it
   even with monsters on the other side, checked when `arg`), 2 forbidden by an effect. */
int duel_direct_attack_kind(unsigned int p, int z, int arg)
{
    unsigned int ref = duel_slot_ref(p, z);
    int i;
    int ok;

    if (duel_direct_attack_allowed(p, z) == 0) {
        return 0;
    }
    if (duel_slot_affected_ex(p, z, 0x1561)) {
        return 2;
    }
    if (duel_slot_affected_ex(p, z, 0x1852)) {
        return 2;
    }
    if ((int)(ref & 2) <= 0 && duel_slot_has_effect(p, 11, 0x17FD)) {
        return 2;
    }
    if (arg != 0 && SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
        switch (field_get_card(p, z)) {
        case 0x14FE:
            return 2;
        case 0x1129:
        case 0x1133:
        case 0x1134:
        case 0x114C:
        case 0x118E:
        case 0x1221:
        case 0x13B3:
        case 0x13B2:
        case 0x1597:
        case 0x1598:
        case 0x182D:
        case 0x1A52:
            return 1;
        case 0x1563:
        case 0x1564:
        case 0x1565:
        case 0x1566:
            if (duel_has_st_12be(p) == 0) {
                break;
            }
        case 0x127D:
        case 0x127E:
        case 0x127F:
        case 0x12A5:
        case 0x154A:
            if (duel_has_faceup_attr6(1 - p) == 0) {
                return 1;
            }
            break;
        case 0x1295:
            if (duel_slot_has_effect(p, z, 0x1295)) {
                return 1;
            }
            break;
        case 0x138B:
            if (duel_no_faceup_attrs46(1 - p)) {
                return 1;
            }
            break;
        case 0x15BA:
            ok = 1;
            for (i = 5; i <= 10; i++) {
                if (!(!duel.players[(1 - p) & 1].field[i].card.id)) {
                    ok = 0;
                    break;
                }
            }
            if (ok) {
                int j;
                int all;

                all = 1;
                for (j = 0; j <= 4; j++) {
                    if (duel.players[(1 - p) & 1].field[j].card.id != 0 &&
                        (duel.players[(1 - p) & 1].field[j].faceup == 0 || duel_field_atk(1 - p, j) < 1600)) {
                        all = 0;
                        break;
                    }
                }
                if (all) {
                    return 1;
                }
            }
            break;
        case 0x1705:
            if (duel_is_cur_card(0x10F4)) {
                return 1;
            }
            break;
        case 0x1701:
        case 0x1747:
        {
            int j;
            int all;

            all = 1;
            for (j = 5; j <= 10; j++) {
                if (!(!duel.players[(1 - p) & 1].field[j].card.id)) {
                    all = 0;
                    break;
                }
            }
            if (all == 0) {
                break;
            }
        }
        case 0x186D:
            if (duel_count_monsters_pos(1 - p, 0, 1) == 0) {
                return 1;
            }
            break;
        case 0x1A01:
            if (duel_count_field_cards(p, 0, 1, 1)) {
                return 1;
            }
            break;
        case 0x1A08:
        {
            int j;
            int all;

            all = 1;
            for (j = 0; j <= 4; j++) {
                if (duel.players[(1 - p) & 1].field[j].card.id != 0 &&
                    (duel.players[(1 - p) & 1].field[j].faceup == 0 || duel_field_atk(1 - p, j) <= duel_field_atk(p, z))) {
                    all = 0;
                    break;
                }
            }
            if (all) {
                return 1;
            }
            break;
        }
        case 0x1A41:
        {
            int j;
            int all;

            all = 1;
            for (j = 5; j < 10; j++) {
                if (!(!duel.players[(1 - p) & 1].field[j].card.id)) {
                    all = 0;
                    break;
                }
            }
            if (all) {
                return 1;
            }
            break;
        }
        }
    }
    if (duel_slot_has_effect(p, z, 0x13AB)) {
        return 1;
    }
    if (duel_slot_has_effect(p, z, 0x15CF)) {
        return 1;
    }
    if (duel_slot_has_effect(p, z, 0x169B)) {
        return 1;
    }
    if (duel_slot_has_effect(p, z, 0x16A3)) {
        return 1;
    }
    if (duel_slot_has_effect(p, z, 0x17AA)) {
        return 1;
    }
    if (duel_slot_has_effect(p, z, 0x1893)) {
        return 1;
    }
    if (duel_slot_has_effect(p, z, 0x1A13)) {
        return 1;
    }
    if (duel_slot_affected_ex(p, z, 0x1759)) {
        return 1;
    }
    return duel_slot_affected_ex(p, z, 0x165D) != 0;
}
