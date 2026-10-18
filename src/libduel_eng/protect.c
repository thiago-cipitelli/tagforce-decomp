// @range 0x088EB710-0x088EBF44
/* Monsters protected by a face-up 0x1368 with the same name, and the checks before a monster of
   player p can be targeted by its controller's action (reason codes in duel_err for the viewer). */
#include "duel.h"
#include "card_db.h"

extern unsigned int duel_view_player;  /* 0x089139F4 */
extern int duel_err;                   /* 0x0891965C */

int duel_count_active(int id);
int field_get_card(unsigned int p, int z);
unsigned int duel_field_type(unsigned int p, int z);
int duel_field_same_name(unsigned int p, int z, int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_affected(unsigned int p, int z, int id);
int duel_slot_affected_ex(unsigned int p, int z, int id);
unsigned int duel_slot_immunity(unsigned int p, int z);
int duel_slot_immune(unsigned int p, int z, int opp);
int duel_slot_ref(unsigned int p, int z);
int duel_effect_applies(int id, unsigned int p, int ref);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_count_card_state(unsigned int p, int id, int state);
int duel_has_st_12be(unsigned int p);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
int duel_phase_ok(unsigned int p);
int duel_phase_ok2(unsigned int p);
int duel_card_check_9148(unsigned int p, int id);

/* Monster z of player p is protected by a face-up 0x1368 naming it whose effect is not its own. */
int duel_field_protected(unsigned int p, int z)
{
    unsigned int q;
    int i;

    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    for (q = 0; (int)q < 2; q++) {
        if (duel_slot_immune(p, z, p != q) == 0) {
            for (i = 5; i < 10; i++) {
                if (duel.players[q & 1].field[i].card.id == 0x1368 && SLOT_ACTIVE(&duel.players[q & 1].field[i]) &&
                    duel_field_same_name(p, z, duel.players[q & 1].field[i].link) &&
                    duel_slot_effect_with(q, i, 0x1368, CARD_UID(duel.players[p & 1].field[z].card)) == 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Same for card id outside the field (uid < 0: any 0x1368 naming it). */
int duel_card_protected(int id, int uid)
{
    unsigned int q;
    int i;

    if (uid >> 1 == 0) {
        return 0;
    }
    for (q = 0; (int)q < 2; q++) {
        for (i = 5; i < 10; i++) {
            if (duel.players[q & 1].field[i].card.id == 0x1368 && SLOT_ACTIVE(&duel.players[q & 1].field[i]) &&
                card_same_name(id, duel.players[q & 1].field[i].link)) {
                if (uid < 0) {
                    return 1;
                }
                if (duel_slot_effect_with(q, i, 0x1368, uid) == 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int duel_monster_can_act(unsigned int p, int z)
{
    unsigned int imm;
    int ref;

    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].flags >> 21 & 1) {
        if (p == duel_view_player) {
            duel_err = 3;
        }
        return 0;
    }
    imm = duel_slot_immunity(p, z);
    ref = duel_slot_ref(p, z);
    if (duel_slot_has_effect(p, 11, 0x12CE) == 0 && duel_slot_has_effect(p, 11, 0x131E) == 0 &&
        duel_slot_has_effect(p, z, 0x17B3) == 0 && duel_slot_has_effect(p, z, 0x1990) == 0 &&
        duel_slot_affected_ex(p, z, 0x1103) == 0 && duel_slot_affected_ex(p, z, 0x1243) == 0 &&
        duel_slot_affected_ex(p, z, 0x14B2) == 0 && duel_slot_affected_ex(p, z, 0x1842) == 0 &&
        duel_slot_affected(p, z, 0x1743) == 0) {
        int self = field_get_card(p, z) == 0x1284;

        if (duel_count_active(0x1284) - self <= 0 &&
            (duel_effect_applies(0x10EF, p, ref) == 0 || duel.players[p & 1].field[z].faceup == 0 ||
             (duel_field_type(p, z) & 0xFFFF) != 1) &&
            (duel_effect_applies(0x15FB, p, ref) == 0 || duel.players[p & 1].field[z].faceup == 0)) {
            if (duel_field_protected(p, z)) {
                if (p == duel_view_player) {
                    duel_err = 0x10;
                }
                return 0;
            }
            if (SLOT_ACTIVE(&duel.players[p & 1].field[z]) == 0 || duel_slot_has_effect(p, z, 0x1419) == 0) {
                if ((int)(imm & 2) > 0 ||
                    (duel_find_effect_card(1 - p, 0x165E, -1) == 0 && duel_count_card_state(1 - p, 0x187C, 2) == 0)) {
                    return 1;
                }
            }
        }
    }
    if (p == duel_view_player) {
        duel_err = 0x12;
    }
    return 0;
}

int duel_facedown_can_act(unsigned int p, int z)
{
    int id;

    if (duel.players[p & 1].field[z].faceup != 0) {
        return 0;
    }
    if (duel_monster_can_act(p, z) == 0) {
        return 0;
    }
    if (duel_phase_ok(p) == 0) {
        return 0;
    }
    if (duel_phase_ok2(p) == 0 || duel_has_st_12be(p) == 0) {
        id = duel.players[p & 1].field[z].card.id;
        if (card_is_attribute6(id) && card_attribute_icon_in_2_3(id)) {
            return 0;
        }
    }
    id = duel.players[p & 1].field[z].card.id;
    switch (id) {
    case 0x158B:
    case 0x160F:
        if (duel_card_check_9148(p, id)) {
            return 0;
        }
        break;
    case 0x164A:
    case 0x164B:
    case 0x164C:
    case 0x164D:
    case 0x164E:
    case 0x164F:
        if (duel_card_check_9148(p, card_id_remap_164a(id)) == 0) {
            return 0;
        }
        break;
    case 0x1691:
        if (duel_count_monsters_fn(p, card_is_in_group_1911) == 0) {
            return 0;
        }
        break;
    }
    return 1;
}
