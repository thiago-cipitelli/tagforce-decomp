// @range 0x08827618-0x08827DA4
/* The computer player's summon checks: tributes for the cards that need named or typed monsters,
   and the start of the check of the effects that answer a summon. */
#include "duel.h"

typedef struct DuelAtkRec {            /* 0x08914A14, 0x1C: the attack being resolved (sent as is to the peer) */
    unsigned int player : 1;           /* attacking card */
    unsigned int zone : 4;
    unsigned int b5 : 1;               /* the attack goes on (not cancelled) [?] */
    unsigned int kind : 2;             /* bits 6-7 */
    unsigned int id : 15;
    unsigned int uid : 9;
    unsigned short h18;                /* +0x04 duel_h_08914A18 */
    unsigned short atk_id;             /* +0x06 duel_h_08914A1A */
    unsigned char n;                   /* +0x08 number of targets */
    unsigned char count;               /* +0x09 distinct targets */
    unsigned short uids[3];            /* +0x0A */
    unsigned short locs[3];            /* +0x10 player | zone << 8 */
    unsigned char f0 : 1;              /* +0x16 duel_w_08914A2A */
    unsigned char f1 : 1;
    unsigned char owner : 1;           /* bit 0 of the uid of the attacking card */
    unsigned char f3 : 1;
    unsigned char f4 : 2;
    unsigned char f6 : 1;
    unsigned char f7 : 1;
    unsigned char mode : 3;            /* +0x17 1 attack, 2 direct attack, 3 summon, 4 special summon [?] */
    unsigned char g3 : 5;
} DuelAtkRec;

extern DuelAtkRec duel_atk_rec;      /* duel_summon (duel.h) seen as the attack/summon record */
extern unsigned short duel_h_08914A1A;

int duel_08827544(int id);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_phase_ok2(unsigned int p);
int duel_088DFE24(unsigned int p, unsigned int q);
int duel_count_free_monster_zones(unsigned int p);
int duel_count_free_zones(unsigned int p);
int duel_088BFFD8(unsigned int p, int id, int i);
unsigned int duel_field_type(unsigned int p, int z);
int card_get_type(int id);
int card_get_kind(int id);
int duel_field_attr_is(unsigned int p, int z, int attr);
int duel_field_kind(unsigned int p, int z);
int duel_count_active(int id);
int duel_can_tribute(unsigned int p, unsigned int q, int z);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_double_tribute(unsigned int p, int id, unsigned int q, int z);
int duel_zone_usable(unsigned int p, unsigned int z);
int duel_pick_monster_zone(unsigned int p);
int duel_0882926C(int q, int z, int b);
int duel_list_card_level(unsigned int p, int i);
int duel_grave_count_id_free(unsigned int p, unsigned int id);
int duel_has_effect_atk(unsigned int p, int id, int atk);
int duel_count_card_state(unsigned int p, int id, int state);
int duel_08827DA4(unsigned int p, int uid, int a, int b);

/* Whether p can summon the card id by tributing monsters of its type or attribute (n tributes
   from duel_08827544); with the effect 0x14A0 the tributes must be on the field. */
int duel_08827618(unsigned int p, unsigned short id)
{
    int n = duel_08827544(id);
    int fx = duel_slot_has_effect(p, 0xB, 0x14A0);
    int z;
    int ok;

    if (duel_phase_ok2(p) == 0) {
        return 0;
    }
    if (duel_088DFE24(p, p) == 0) {
        return 0;
    }
    if (duel_count_free_monster_zones(p) == 0 && fx == 0) {
        return 0;
    }
    if (fx) {
        for (z = 0; z < 5; z++) {
            if (duel.players[p & 1].field[z].card.id != 0) {
                switch (id) {
                case 0x1466:
                case 0x18B4:
                case 0x19CA:
                    ok = (unsigned short)duel_field_type(p, z) == card_get_type(id);
                    break;
                default:
                    ok = duel_field_attr_is(p, z, card_get_kind(id));
                    break;
                }
                if (ok && --n == 0) {
                    return 1;
                }
            }
        }
        return 0;
    } else {
        return duel_088BFFD8(p, id, 0) >= n;
    }
}

/* Whether p can summon the card id with the effect 0x14A0 (needs monsters of kinds 1 and 2) or from
   the deck [?]. */
int duel_088277BC(unsigned int p, int id)
{
    int z;
    int a;
    int b;

    if (duel_phase_ok2(p) == 0) {
        return 0;
    }
    if (duel_088DFE24(p, p) == 0) {
        return 0;
    }
    if (duel_slot_has_effect(p, 0xB, 0x14A0) != 0) {
        a = 0;
        b = 0;
        for (z = 0; z < 5; z++) {
            if (duel.players[p & 1].field[z].card.id != 0) {
                switch (duel_field_kind(p, z)) {
                case 1:
                    a = 1;
                    break;
                case 2:
                    b = 1;
                    break;
                }
            }
        }
        return a && b;
    } else {
        if (duel_count_free_monster_zones(p) != 0) {
            return duel_088BFFD8(p, id, 0) > 0 && duel_088BFFD8(p, id, 1) > 0;
        }
        return 0;
    }
}

/* Whether p has three tributes (double tributes count twice) of the type of the card id. */
int duel_08827934(unsigned int p, int id)
{
    int z;
    int n = 0;
    int n2 = 0;
    int ok = 0;

    if (duel_count_active(0x13F2) > 0) {
        return 0;
    }
    for (z = 0; z < 5; z++) {
        if (duel.players[p & 1].field[z].card.id != 0 && duel_can_tribute(p, p, z) != 0 &&
            (unsigned short)duel_field_type(p, z) == card_get_type(id)) {
            n++;
            if (duel_double_tribute(p, id, p, z) != 0) {
                n2++;
            }
            if (duel_zone_usable(p, z) != 0) {
                ok = 1;
            }
        }
    }
    if (!ok && duel_pick_monster_zone(p) == 0) {
        return 0;
    }
    return n + n2 >= 3;
}

int duel_08827A90(unsigned int p, int id)
{
    int q;
    int z;
    int r;

    duel_atk_rec.player = p;
    duel_atk_rec.id = id;
    duel_h_08914A1A = id;
    duel_atk_rec.f1 = 0;
    duel_atk_rec.n = 0;
    for (q = 0; q < 2; q++) {
        for (z = 0; z <= 4; z++) {
            r = !duel_0882926C(q, z, 0);
            if (!r) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_08827B80(unsigned int p, int i)
{
    if (duel_count_free_monster_zones(p) != 0 && duel_list_card_level(p, i) >= 5 &&
        duel_grave_count_id_free(p, 0x197F) != 0) {
        return 1;
    }
    return 0;
}

int duel_08827BF0(unsigned int p, int i)
{
    int lv;

    if (duel_count_active(0x13F2) > 0) {
        return 0;
    }
    if (duel_count_free_zones(p) != 0 && (lv = duel_list_card_level(p, i)) >= 5) {
        return duel_has_effect_atk(p, 0x19B2, (lv >= 7) + 1) > 0;
    }
    return 0;
}

int duel_08827C80(unsigned int p, int i)
{
    if (duel_count_free_monster_zones(p) != 0 && duel_list_card_level(p, i) >= 5) {
        return duel_count_card_state(p, 0x1AA3, 3) > 0;
    }
    return 0;
}

/* Whether p has a free monster zone or a monster it can tribute in a usable zone. */
int duel_08827CEC(unsigned int p, int b)
{
    int z;
    int r;

    if (duel_pick_monster_zone(p) >= 0) {
        return 1;
    }
    for (z = 0; z <= 4; z++) {
        if (b > 0) {
            r = duel_tributable(p, p, z);
        } else {
            r = duel_can_tribute(p, p, z);
        }
        if (r != 0 && duel_zone_usable(p, z) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_08827D98(unsigned int p, int uid)
{
    return duel_08827DA4(p, uid, 1, 0);
}
