// @range 0x088EF5EC-0x088EFEE4
// @asm 0x088EF7D0-0x088EFA2C
/* Whether the card being activated may target or affect monster z of player p, and a few counts of
   cards on the field. */
#include "duel.h"
#include "card_db.h"

extern int duel_targeting_off;         /* 0x08914910 */

int duel_count_active(int id);
int field_get_card(unsigned int p, int z);
unsigned int duel_field_type(unsigned int p, int z);
unsigned int duel_field_flag9(unsigned int p, int z);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_affected_ex(unsigned int p, int z, int id);
int duel_slot_e1854(unsigned int p, int z);
int duel_slot_e68a0(unsigned int p, int z);
unsigned int duel_act_e3920(DuelAct *a);
unsigned int duel_dfc10(void);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);

/* Card `a` may target monster z of player p. */
int duel_act_can_target(DuelAct *a, unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 1;
    }
    if (duel_targeting_off) {
        return 1;
    }
    if (z < 5) {
        if (!(duel.players[p & 1].field[z].flags >> 5 & 1)) {
            switch (field_get_card(p, z)) {
            case 0x1386:
            case 4999:
            case 5000:
                return 0;
            }
        }
        if ((duel_field_type(p, z) & 0xFFFF) == 1) {
            if (duel_count_active(0x128B)) {
                return 0;
            }
            if (duel_find_effect_card(1 - a->player, 0x1879, -1)) {
                return 0;
            }
        }
        if (duel_find_effect_card(1 - a->player, 0x17DC, -1)) {
            switch (field_get_card(p, z)) {
            case 0x17D2:
            case 0x17D3:
            case 0x17D4:
                return 0;
            }
        }
    }
    return 1;
}

#ifdef NON_MATCHING /* the original keeps `st` as a 0/1 value in a0 and tests it; mwcc threads the jumps here */
/* Card `a` may affect monster z of player p (arg: also the 0x150A protection). */
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg)
{
    int zone = a->zone;
    int not_mon = 1;
    int st = 0;
    unsigned int r;

    if (zone >= 5 && zone < 11) {
        st = 1;
    }
    if (!st) {
        if (!(!card_is_monster(a->id))) {
            not_mon = 0;
        }
    }
    if (a->id == 0) {
        return 1;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        switch (field_get_card(p, z)) {
        case 0x150C:
            if (!(duel.players[p & 1].field[z].flags >> 5 & 1) && z >= 5 &&
                (unsigned short)duel_slot_e1854(p, z) != 0xFFFF) {
                return 0;
            }
            break;
        case 0x1645:
            if (!(duel.players[p & 1].field[z].flags >> 5 & 1) && z < 5 && not_mon) {
                return 0;
            }
            break;
        }
        if (duel_slot_has_effect(p, z, 0x1A5C)) {
            return 0;
        }
        if (arg != 0 && z < 5 && duel_slot_affected_ex(p, z, 0x150A)) {
            r = duel_act_e3920(a);
            if (duel_field_flag9(p, z) == 0) {
                return 0;
            }
            if (r == 0 || (r & 2)) {
                return 0;
            }
        }
    } else {
        if (z >= 5 && duel_slot_has_effect(0, 11, 0x153C)) {
            return 0;
        }
    }
    return 1;
}
#endif

/* Same for a card `id` of the opponent of p in zone `zone`. */
int duel_card_can_affect(unsigned int p, int z, unsigned short id, unsigned char zone, int arg)
{
    DuelAct a = { 0 };

    a.id = id;
    a.player = 1 - p;
    a.zone = zone;
    a.b3_4 = 0;
    return duel_act_can_affect(&a, p, z, arg);
}

int duel_slot_efacc(unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].faceup != 0) {
        switch (field_get_card(p, z)) {
        case 0x1A8D:
            if (!(duel.players[p & 1].field[z].flags >> 5 & 1) && z < 5) {
                return 0;
            }
            break;
        }
    }
    return 1;
}

int duel_slot_efb84(unsigned int p, int z)
{
    int id = field_get_card(p, z);
    unsigned int f;

    if (id == 0) {
        return 0;
    }
    if (id == 0x16F8 && SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
        return 0;
    }
    if (card_is_attribute9(id)) {
        return 0;
    }
    f = duel_dfc10();
    if (f & 1) {
        return 0;
    }
    if ((f & 2) && card_is_monster(id)) {
        return 0;
    }
    if ((f & 4) && !duel.players[p & 1].field[z].card.owner) {
        return 0;
    }
    if ((f & 8) && duel.players[p & 1].field[z].card.owner == 1) {
        return 0;
    }
    return 1;
}

int duel_sum_e68a0(unsigned int p)
{
    int i;
    int n = 0;

    for (i = 0; i <= 10; i++) {
        if (duel.players[p & 1].field[i].card.id != 0 && duel.players[p & 1].field[i].faceup != 0) {
            n += duel_slot_e68a0(p, i);
        }
    }
    return n;
}

/* Face-up 0x1A90, 0x1A92, 0x1A94, 0x1A96 on the field. */
int duel_count_1a90(void)
{
    unsigned int q;
    int i;
    int n = 0;

    for (q = 0; (int)q < 2; q++) {
        for (i = 0; i <= 4; i++) {
            switch (field_get_card(q, i)) {
            case 0x1A90:
            case 0x1A92:
            case 0x1A94:
            case 0x1A96:
                if (SLOT_ACTIVE(&duel.players[q & 1].field[i])) {
                    n++;
                }
                break;
            }
        }
    }
    return n;
}
