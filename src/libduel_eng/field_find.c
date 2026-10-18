// @range 0x088E8CF4-0x088E94E4
// @asm 0x088E8CF4-0x088E8F88
/* Copies of a card on the field: counts by id or name with conditions, lowest activation order. */
#include "duel.h"
#include "card_db.h"

int field_get_card(unsigned int p, int z);
int duel_field_same_name(unsigned int p, int z, int id);
int duel_088E677C(unsigned int p, int z);

#ifdef NON_MATCHING /* same code; the original gives the zone bit counter and the slot pointer the other $s registers */
/* Active monsters of player p with card id, flag bit 18 set, in enabled zones. */
int duel_count_id_flag18(unsigned int p, int id)
{
    int i;
    int n = 0;
    unsigned int b = p << 4;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++, b++) {
        int c = field_get_card(p, i);

        if (c == id && SLOT_ACTIVE(&pl->field[i]) && (pl->field[i].flags >> 18 & 1) &&
            !(duel.zone_off & 1 << b)) {
            n++;
        }
    }
    return n;
}
#endif

#ifdef NON_MATCHING /* same code; the original gives the zone bit counter and the slot pointer the other $s registers */
/* Lowest activation order (unk4) among the active copies of card id of player p, 0 if none. */
int duel_min_order(unsigned int p, int id)
{
    int m = 0;
    int i;

    if (card_is_monster(id) != 0) {
        DuelPlayer *pl = &duel.players[p & 1];
        unsigned int b;

        for (i = 0, b = p << 4; i < 5; i++, b++) {
            int c = field_get_card(p, i);

            if (c == id && SLOT_ACTIVE(&pl->field[i]) && !(duel.zone_off & 1 << b)) {
                if (m != 0) {
                    if (pl->field[i].unk4 < m) {
                        m = pl->field[i].unk4;
                    }
                } else {
                    m = pl->field[i].unk4;
                }
            }
        }
    } else {
        DuelPlayer *pl = &duel.players[p & 1];

        for (i = 5; i <= 10; i++) {
            int c = field_get_card(p, i);

            if (c == id && SLOT_ACTIVE(&pl->field[i])) {
                if (m != 0) {
                    if (pl->field[i].unk4 < m) {
                        m = pl->field[i].unk4;
                    }
                } else {
                    m = pl->field[i].unk4;
                }
            }
        }
    }
    return m;
}
#endif

int duel_min_order_both(int id)
{
    int a = duel_min_order(0, id);
    int b = duel_min_order(1, id);

    if (a == 0) {
        return b;
    }
    if (b == 0) {
        return a;
    }
    return MIN(a, b);
}

/* Face-up copies of card id of player p, zone skip excluded (monsters by name). */
int duel_count_faceup_skip(unsigned int p, int id, int skip)
{
    int i;
    int n = 0;

    if (card_is_monster(id) != 0) {
        DuelPlayer *pl = &duel.players[p & 1];

        for (i = 0; i < 5; i++) {
            if (field_get_card(p, i) > 0 && i != skip && duel_field_same_name(p, i, id) && pl->field[i].faceup != 0) {
                n++;
            }
        }
    } else {
        DuelPlayer *pl = &duel.players[p & 1];

        for (i = 5; i <= 10; i++) {
            int c = field_get_card(p, i);

            if (c > 0 && i != skip && c == id && pl->field[i].faceup != 0 && !(pl->field[i].flags >> 1 & 1)) {
                n++;
            }
        }
    }
    return n;
}

int duel_card_check_9148(unsigned int p, int id)
{
    return duel_count_faceup_skip(p, id, -1);
}

int duel_088E9150(int id)
{
    return duel_count_faceup_skip(0, id, -1) + duel_count_faceup_skip(1, id, -1);
}

/* Face-up cards of player p with the name of card id. */
int duel_count_faceup_name(unsigned int p, int id)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i <= 10; i++) {
        if (duel_field_same_name(p, i, id) && pl->field[i].faceup != 0) {
            n++;
        }
    }
    return n;
}

int duel_count_card_state(unsigned int p, int id, int x)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 5; i <= 10; i++) {
        int c = field_get_card(p, i);

        if (c > 0 && c == id && SLOT_ACTIVE(&pl->field[i]) && duel_088E677C(p, i) < x) {
            n++;
        }
    }
    return n;
}

int duel_has_effect_attr(unsigned int p, int id, int x)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 5; i <= 10; i++) {
        int c = field_get_card(p, i);

        if (c == id && SLOT_ACTIVE(&pl->field[i]) && x == pl->field[i].link) {
            n++;
        }
    }
    return n;
}

int duel_has_effect_atk(unsigned int p, int id, unsigned int x)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 5; i <= 10; i++) {
        int c = field_get_card(p, i);

        if (c > 0 && c == id && SLOT_ACTIVE(&pl->field[i]) && pl->field[i].link >= x) {
            n++;
        }
    }
    return n;
}
