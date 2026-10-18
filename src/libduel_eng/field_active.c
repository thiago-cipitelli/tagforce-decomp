// @range 0x088E80CC-0x088E8CF4
// @asm 0x088E83C8-0x088E8884
// @asm 0x088E8920-0x088E8AD4
// @asm 0x088E8B14-0x088E8CF4
/* Active copies of a card on the field (face up, not negated, zone enabled), including the two
   cards held outside the zones in the special mode. */
#include "duel.h"
#include "card_db.h"

int field_get_card(unsigned int p, int z);
int duel_special_get_id(DuelSlot *s);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_card_protected(int id, int uid);

/* Face-up spell/trap zone of player p holding card id, -1 if none. */
int duel_find_st(unsigned int p, int id)
{
    int i;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 5; i <= 10; i++) {
        int c = field_get_card(p, i);

        if (c > 0 && pl->field[i].faceup != 0 && c == id) {
            return i;
        }
    }
    return -1;
}

/* Zone z of player p holds card id, active, in an enabled zone. */
int duel_slot_e8170(unsigned int p, int z, int id)
{
    int r = 0;
    int ok = 0;

    if (id == field_get_card(p, z) && SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
        ok = 1;
    }
    if (ok && !(duel.zone_off & 1 << (z + p * 16)) != 0) {
        r = 1;
    }
    return r;
}

int duel_slot_faceup_live(unsigned int p, int z)
{
    int r = 0;

    if (duel.players[p & 1].field[z].faceup != 0 && !(duel.players[p & 1].field[z].flags >> 1 & 1) != 0) {
        r = 1;
    }
    return r;
}

/* The card of a special activation still applies. */
int duel_spec_act_ok(DuelSpecAct *a)
{
    if (card_attribute_to_group(a->id) != 0) {
        if (a->unk34 != 0) {
            return 0;
        }
        if (duel_count_active(0x166C) != 0) {
            return 0;
        }
        if (a->unk8 != 0 && duel_slot_has_effect(0, 0xB, 0x12BF)) {
            return 0;
        }
        if (card_get_special_group(a->id) == 1) {
            if (duel_count_active(0x148E) != 0) {
                return 0;
            }
            if (duel_count_active(0x14DA) != 0) {
                return 0;
            }
        }
        if (duel_card_protected(a->id, a->uid) != 0) {
            return 0;
        }
    }
    return 1;
}

#ifdef NON_MATCHING /* the original tests monsters first and gives the zone bit counter its own $s register */
/* Active copies of card id of player p, zone skip excluded. */
int duel_find_effect_card(unsigned int p, int id, int skip)
{
    int n = 0;
    int i;

    if (card_is_monster(id) == 0) {
        if (card_get_prop5_hi(id) == 2) {
            if (id == field_get_card(p, 10) && SLOT_ACTIVE(&duel.players[p & 1].field[10]) &&
                !(duel.zone_off & 1 << (p * 16 + 10))) {
                n = 1;
            }
        } else {
            DuelPlayer *pl = &duel.players[p & 1];

            for (i = 5; i < 10; i++) {
                if (id == field_get_card(p, i) && i != skip && SLOT_ACTIVE(&pl->field[i]) &&
                    !(duel.zone_off & 1 << (p * 16 + i))) {
                    n++;
                }
            }
        }
    } else {
        DuelPlayer *pl = &duel.players[p & 1];

        for (i = 0; i < 5; i++) {
            if (id == field_get_card(p, i) && i != skip && SLOT_ACTIVE(&pl->field[i]) &&
                !(duel.zone_off & 1 << (p * 16 + i))) {
                n++;
            }
        }
    }
    return n;
}
#endif

#ifdef NON_MATCHING /* same; zone bit counters and slot pointers in other registers */
/* Active copies of card id on both fields. */
int duel_count_active(int id)
{
    int n = 0;
    unsigned int p;
    int i;

    if (card_is_monster(id) == 0) {
        if (card_get_prop5_hi(id) == 2) {
            for (p = 0; (int)p < 2; p++) {
                if (id == field_get_card(p, 10) && SLOT_ACTIVE(&duel.players[p & 1].field[10]) &&
                    !(duel.zone_off & 1 << (p * 16 + 10))) {
                    n++;
                }
            }
        } else {
            for (p = 0; (int)p < 2; p++) {
                DuelPlayer *pl = &duel.players[p & 1];

                for (i = 5; i < 10; i++) {
                    if (id == field_get_card(p, i) && SLOT_ACTIVE(&pl->field[i]) && !(duel.zone_off & 1 << (i + p * 16))) {
                        n++;
                    }
                }
            }
        }
    } else {
        for (p = 0; (int)p < 2; p++) {
            DuelPlayer *pl = &duel.players[p & 1];

            for (i = 0; i < 5; i++) {
                if (id == field_get_card(p, i) && SLOT_ACTIVE(&pl->field[i]) && !(duel.zone_off & 1 << (i + p * 16))) {
                    n++;
                }
            }
        }
    }
    return n;
}
#endif

/* Copies of card id whose effect applies to player p: both fields (ref 0) or the sides not
   excluded by ref (bit 0: p, bit 1: the opponent). */
int duel_effect_applies(int id, unsigned int p, int ref)
{
    int n;

    if (ref != 0) {
        n = 0;
        if ((ref & 1) <= 0) {
            n += duel_find_effect_card(p, id, -1);
        }
        if ((ref & 2) <= 0) {
            n += duel_find_effect_card(1 - p, id, -1);
        }
        return n;
    }
    return duel_count_active(id);
}

#ifdef NON_MATCHING /* same code; the zone bit counter and the slot pointer swap $s0/$s1 */
/* Active monsters of player p with card id, special cards included. */
int duel_count_monster_active(unsigned int p, int id)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];
    unsigned int b;

    for (i = 0, b = p << 4; i < 5; i++, b++) {
        int c = field_get_card(p, i);

        if (c == id && SLOT_ACTIVE(&pl->field[i]) && !(duel.zone_off & 1 << b)) {
            n++;
        }
    }
    if (duel.mode & 1) {
        if (p == duel_special.player[0] && id == duel_special_get_id(&duel_special.slot[0]) &&
            duel_spec_act_ok(&duel_special.act[duel_special.player[0]])) {
            n++;
        }
        if (p == duel_special.player[1] && id == duel_special_get_id(&duel_special.slot[1]) &&
            duel_spec_act_ok(&duel_special.act[1 - duel_special.player[0]])) {
            n++;
        }
    }
    return n;
}
#endif

int duel_count_monster_active_both(int id)
{
    return duel_count_monster_active(0, id) + duel_count_monster_active(1, id);
}

#ifdef NON_MATCHING /* same as duel_count_monster_active */
/* Same, face-up attack position only. */
int duel_count_monster_active_atk(unsigned int p, int id)
{
    int n = 0;
    int i;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++) {
        if (field_get_card(p, i) == id && pl->field[i].pos == 0 && SLOT_ACTIVE(&pl->field[i]) &&
            !(duel.zone_off & 1 << (p * 16 + i))) {
            n++;
        }
    }
    if (duel.mode & 1) {
        if (p == duel_special.player[0] && id == duel_special_get_id(&duel_special.slot[0]) &&
            duel_special.slot[0].pos == 0 && duel_spec_act_ok(&duel_special.act[duel_special.player[0]])) {
            n++;
        }
        if (p == duel_special.player[1] && id == duel_special_get_id(&duel_special.slot[1]) &&
            duel_special.slot[1].pos == 0 && duel_spec_act_ok(&duel_special.act[1 - duel_special.player[0]])) {
            n++;
        }
    }
    return n;
}
#endif
