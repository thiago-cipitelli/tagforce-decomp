// @range 0x08819F94-0x0881C118
// @asm 0x0881AF78-0x0881B020
// @asm 0x0881A118-0x0881A1E4
// @asm 0x0881A2B4-0x0881A3EC
// @asm 0x0881A3EC-0x0881A554
// @asm 0x0881A7C0-0x0881A960
// @asm 0x0881A960-0x0881AA48
// @asm 0x0881B1E4-0x0881B2C0
/* Conditions the computer player checks before activating a card (continued in ai_cond1.c). */
#include "duel.h"
#include "card_db.h"

int duel_0884E948(DuelAct *a, unsigned int p);
int duel_count_name(unsigned int p, int id);
int duel_count_field_all(unsigned int p);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_d_count_name(unsigned int p, unsigned short id);
int duel_088AA224(DuelAct *a);
int duel_088BFFD8(unsigned int p, int id, int i);
int duel_08809F2C();
int duel_0880B07C(unsigned int p);
int duel_08807AB4(unsigned int p);
int duel_0880ACA0(unsigned int p);
void duel_act_0887E140(DuelAct *a, int d[2]);
int duel_find_st(unsigned int p, int id);
int duel_count_monsters(unsigned int p);
int duel_count_active(int id);
int duel_count_monsters_pos_is(unsigned int p, unsigned int pos);
unsigned short duel_slot_e1854(unsigned int p, int z);
int duel_08836868(unsigned int p);
int duel_088054A8(unsigned int p, int a, int b);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_field_def(unsigned int p, int z);
int duel_phase_ok2(unsigned int p);
unsigned int duel_card_b14(DuelCard *c);
int duel_count_free_monster_zones(unsigned int p);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
int duel_088A6300(int id);
int duel_088A6D2C(int id, int n, void *out);
int duel_08809FC4();
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_0880A17C(unsigned int p, unsigned int q, int a, int b, int c);
int duel_field_atk(unsigned int p, int z);
int duel_count_monsters_pos(unsigned int p, int faceup, int pos0);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
int duel_0886CD24(DuelAct *a, unsigned int p);
unsigned int duel_08807434(unsigned int p, int a);
int duel_count_type(unsigned int p, unsigned int type);
int duel_hand_count_monsters(unsigned int p);
int duel_slot_value_1909(unsigned int p, int z);
int duel_grave_count_monsters(unsigned int p);
int duel_field_level(unsigned int p, int z);
int duel_08816EA4(unsigned int p, int z);
int duel_count_field_cards(unsigned int p, int faceup, int facedown, int fzone);
int duel_pick_st_zone(unsigned int p);
int duel_088154D8(unsigned int p);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_count_tributes();
int duel_can_tribute(unsigned int p, unsigned int q, int z);
int duel_list_card_level(unsigned int p, int i);
int duel_08816A74(unsigned int p, int (*fn)(), int a, int b);
int duel_ai_0881BDAC(void);

extern int duel_w_0891525C[];

extern struct DuelZoneCounts {         /* 0x08913CF4 */
    unsigned char pad0[0x1E8];
    unsigned int n[5];                 /* +0x1E8 by monster zone [?] */
} duel_w_08913CF4;

int duel_ai_08819F94(void)
{
    return 1;
}

int duel_08819F9C(DuelAct *a, int b)
{
    return duel_0884E948(a, 1 - a->player) > 0;
}

int duel_ai_08819FC8(DuelAct *a)
{
    return duel_0884E948(a, a->player) > 0;
}

int duel_08819FEC(DuelAct *a, int b)
{
    return !(duel_count_name(a->player, a->id) - (a->zone < 11) > 0);
}

int duel_0881A038(DuelAct *a, int b)
{
    return duel_088BFFD8(a->player, a->id, duel_088AA224(a)) > 0;
}

int duel_0881A074(DuelAct *a, int b)
{
    return duel_0880B07C(a->player) != 0;
}

int duel_ai_0881A098(DuelAct *a, int b)
{
    if (duel_0880B07C(a->player) != 0) {
        return duel_08819F9C(a, b);
    }
    return 0;
}

int duel_ai_0881A0E4(DuelAct *a, int b)
{
    return !duel_0884E948(a, a->player);
}

int duel_ai_0881A10C(DuelAct *a)
{
    return duel_08807AB4(a->player);
}

#ifdef NON_MATCHING /* the original branches to the shared epilogue for the LP test; mwcc to the instruction after it */
int duel_ai_0881A118(DuelAct *a)
{
    int v[2];
    unsigned int p;
    unsigned int q;
    int n;

    if (duel_find_effect_card(a->player, 0x159B, -1) != 0) {
        return 1;
    }
    duel_act_0887E140(a, v);
    p = a->player;
    q = 1 - p;
    if (v[q] == 0) {
        return 0;
    }
    if (duel.players[q & 1].lp <= v[q]) {
        return 1;
    }
    n = duel_hand_count_name(p, 0x159B);
    return !(n + duel_d_count_name(p, 0x159B));
}
#endif

int duel_ai_0881A1E4(DuelAct *a)
{
    return duel_08809F2C(a->player) > 0;
}

int duel_ai_0881A208(DuelAct *a)
{
    unsigned int p = a->player;

    return duel.players[(1 - p) & 1].lp < duel.players[p & 1].lp + 500;
}

int duel_ai_0881A258(DuelAct *a)
{
    return duel_count_field_all(1 - a->player) > 1;
}

int duel_ai_0881A288(DuelAct *a)
{
    return duel_0880ACA0(1 - a->player) > 0;
}

#ifdef NON_MATCHING /* branch order of the zone owner tests not reproduced */
int duel_ai_0881A2B4(DuelAct *a)
{
    int z;
    unsigned short t;

    if (duel_find_st(a->player, 0x1366) >= 0) {
        return 1;
    }
    if (duel_count_monsters(1 - a->player) == 0 && duel_count_monsters(a->player) != 0 &&
        duel_count_field_all(1 - a->player) > 0) {
        return 1;
    }
    z = duel_find_st(a->player, 0x137D);
    if (z < 0) {
        return 0;
    }
    t = duel_slot_e1854(a->player, z);
    if (t == 0xFFFF) {
        return 1;
    }
    if (duel.players[t & 1].field[t >> 8].card.id == 0x11E4) {
        return 1;
    }
    if (duel.players[t & 1].field[t >> 8].card.id == 0xFD6) {
        return 1;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* branch order of the LP and count tests not reproduced */
int duel_ai_0881A3EC(DuelAct *a)
{
    int n = duel_count_field_all(1 - a->player);
    int v;

    if (n == 0) {
        return 0;
    }
    if (duel_0880ACA0(a->player) < duel_0880ACA0(1 - a->player)) {
        return 1;
    }
    if (n >= 2 && duel_count_field_all(a->player) < n) {
        return 1;
    }
    if (duel_08836868(a->player) != 0) {
        if (duel_count_active(0x140E) == 0 && duel_count_active(0x17A6) == 0) {
            return 0;
        }
        v = duel_088054A8(a->player, -1, 0);
        if (v < duel.players[(1 - a->player) & 1].lp && v < 0x960) {
            return 0;
        }
        if (duel_08809F2C() > 0) {
            return 1;
        }
    }
    return 0;
}
#endif

int duel_ai_0881A554(DuelAct *a)
{
    int z;

    for (z = 0; z <= 4; z++) {
        if (duel_0884E3E4(a, a->player, z) != 0 && duel_w_08913CF4.n[z] > 1 && duel_field_def(a->player, z) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_ai_0881A5F0(DuelAct *a)
{
    return duel_count_monsters_pos_is(1 - a->player, 1) != 0;
}

int duel_ai_0881A61C(DuelAct *a)
{
    int z;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    for (z = 0; z <= 4; z++) {
        if (duel_0884E3E4(a, a->player, z) != 0 &&
            (duel.players[a->player & 1].field[z].flags >> 22 & 1) + (duel.players[a->player & 1].field[z].flags >> 23 & 1) != 0 &&
            card_attribute_icon_in_2_3(duel.players[a->player & 1].field[z].card.id) == 0) {
            return 1;
        }
    }
    return 0;
}

int duel_ai_0881A6F8(DuelAct *a)
{
    int z;

    for (z = 0; z <= 4; z++) {
        if (duel_0884E3E4(a, a->player, z) != 0 &&
            (duel.players[a->player & 1].field[z].flags >> 22 & 1) + (duel.players[a->player & 1].field[z].flags >> 23 & 1) != 0 &&
            (int)card_get_level(duel.players[a->player & 1].field[z].card.id) <= 4) {
            return 1;
        }
    }
    return 0;
}
#ifdef NON_MATCHING /* registers of the card id, uid and count swapped */
int duel_ai_0881A7C0(DuelAct *a)
{
    int z;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    for (z = 0; z <= 4; z++) {
        if (duel_0884E3E4(a, a->player, z) != 0 && duel_card_b14(&duel.players[a->player & 1].field[z].card) != 0 &&
            (duel.players[a->player & 1].field[z].flags >> 22 & 1) + (duel.players[a->player & 1].field[z].flags >> 23 & 1) != 0) {
            unsigned int id;
            int n;
            int uid;

            uid = CARD_UID(duel.players[a->player & 1].field[z].card);
            id = duel.players[a->player & 1].field[z].card.id;
            n = duel_088A6300(id);
            if (!(duel_count_free_monster_zones(a->player) + 1 < n) && n == duel_088BFF78(a->player, a->id, uid) &&
                duel_088A6D2C(id, n, duel_w_0891525C) != 0) {
                return 1;
            }
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* branch order of the grave and LP tests not reproduced */
int duel_ai_0881A960(DuelAct *a)
{
    int r = 1;
    unsigned int p = a->player;

    if (duel.players[(1 - p) & 1].n_grave < 5) {
        if (duel_hand_count_name(p, 0x14F0) == 0 && duel_count_name(a->player, 0x14F0) == 0) {
            return 0;
        }
        r = !(duel.players[a->player & 1].lp < 0x3E9 || duel.players[a->player & 1].n_grave < 0xF);
    }
    return r;
}
#endif

int duel_ai_0881AA48(DuelAct *a)
{
    int x;
    int y;
    DuelStats st;
    int z;

    duel_08809FC4(a->player, &x, &y, 1);
    for (z = 0; z <= 4; z++) {
        unsigned int q = 1 - a->player;

        if (duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].faceup != 0) {
            duel_field_stats(q, z, &st);
            if (duel.players[(1 - a->player) & 1].field[z].pos != 0) {
                if (x < st.def && st.atk < x) {
                    return 1;
                }
            } else if (x < st.atk && st.def < x) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_ai_0881AB94(DuelAct *a, int b)
{
    unsigned int i;
    int r;

    if (duel_08819FEC(a, b) == 0) {
        return 0;
    }
    i = 0;
    if (duel.players[a->player & 1].n_hand != 0) {
        do {
            r = !card_is_attribute6(duel.players[a->player & 1].hand[i].id);
            if (!r) {
                return 1;
            }
            i++;
        } while (i < duel.players[a->player & 1].n_hand);
    }
    return 0;
}

int duel_ai_0881AC74(DuelAct *a)
{
    int v;
    int z;

    if (duel_slot_has_effect(1 - a->player, 11, 0x12CE) == 0) {
        unsigned int p = a->player;

        v = duel_0880A17C(p, p, -1, 1, 0);
        for (z = 0; z < 5; z++) {
            unsigned int q = 1 - a->player;

            if (duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].pos == 0 &&
                duel_field_atk(q, z) < v) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_ai_0881AD74(DuelAct *a, int b)
{
    return duel_08819FEC(a, b) != 0 && duel_count_monsters_pos(1 - a->player, 0, 1) > 0;
}

int duel_ai_0881ADC0(DuelAct *a)
{
    return duel_count_free_monster_zones(1 - a->player) > 1;
}

int duel_ai_0881ADF0(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[(1 - p) & 1].lp <= 500) {
        return 1;
    }
    return !(duel.players[p & 1].lp < 0x5DD);
}

int duel_ai_0881AE54(DuelAct *a, int b)
{
    return duel_08819FEC(a, b) != 0 && duel.players[a->player & 1].n_hand > 5;
}

int duel_ai_0881AEB4(DuelAct *a)
{
    if (duel_count_monsters_fn(a->player, card_is_attribute7) != 0) {
        return !(duel_0886CD24(a, a->player) < 2);
    }
    return 0;
}

int duel_ai_0881AF08(DuelAct *a)
{
    return duel_0886CD24(a, a->player) < 2;
}

int duel_ai_0881AF2C(DuelAct *a, int b)
{
    if (a->b3_4 == 0) {
        return duel_08819FEC(a, b);
    }
    return (int)duel_08807434(a->player, 0) >= 0;
}

#ifdef NON_MATCHING /* operand order of two of the additions differs */
int duel_ai_0881AF78(DuelAct *a)
{
    int n = duel_count_name(a->player, 0x1102);

    n += duel_count_name(a->player, 0x134A);
    n = duel_count_name(a->player, 0x1400) + n;
    n = duel_count_name(a->player, 0x187C) + n;
    if (!(n - (a->zone < 11) > 0)) {
        return duel_08809F2C(a->player) < 0;
    }
    return 0;
}
#endif

int duel_ai_0881B020(DuelAct *a, int b)
{
    return duel_08819FEC(a, b) != 0 && duel_count_type(1 - a->player, 10) > 0;
}

int duel_ai_0881B06C(DuelAct *a)
{
    int z;

    for (z = 0; z < 5; z++) {
        if (card_get_special_group(duel.players[a->player & 1].field[z].card.id) != 0 &&
            duel.players[a->player & 1].field[z].faceup == 0) {
            return 1;
        }
    }
    return 0;
}

int duel_ai_0881B130(DuelAct *a)
{
    if (duel_slot_has_effect(a->player, 11, a->id) != 0) {
        return 0;
    }
    return duel_0886CD24(a, a->player) > 6;
}

int duel_ai_0881B188(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_hand < 3) {
        return 0;
    }
    return duel_hand_count_monsters(p) < 3;
}

#ifdef NON_MATCHING /* the original loads the opponent LP before the multiply; branch order differs */
int duel_ai_0881B1E4(DuelAct *a)
{
    int v;
    int r;

    if (a->b3_4 == 0) {
        return 1;
    }
    v = duel_slot_value_1909(a->player, a->zone);
    r = 1;
    if (v * 1000 < duel.players[(1 - a->player) & 1].lp) {
        if (duel_find_effect_card(a->player, a->id, -1) < 2) {
            r = 0;
        } else if (v < 2) {
            r = 0;
        } else {
            r = 1;
        }
    }
    return r;
}
#endif

int duel_ai_0881B2C0(DuelAct *a)
{
    return duel_grave_count_monsters(1 - a->player) > 1;
}

int duel_ai_0881B2F0(DuelAct *a)
{
    return duel.players[a->player & 1].flags >> 25 & 1;
}

int duel_ai_0881B320(DuelAct *a)
{
    if (duel_count_name(a->player, 0x10BC) != 0) {
        return 0;
    }
    return (int)duel_08807434(a->player, 0) >= 0;
}

int duel_ai_0881B374(DuelAct *a)
{
    int z;

    for (z = 0; z < 5; z++) {
        if (duel_0884E3E4(a, a->player, z) != 0 &&
            duel_088BFFD8(a->player, a->id, duel_field_level(a->player, z)) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_ai_0881B404(DuelAct *a)
{
    int z;

    for (z = 0; z < 5; z++) {
        if (duel.players[a->player & 1].field[z].card.id != 0 && duel_08816EA4(a->player, z) != 0) {
            return 1;
        }
    }
    if (duel_count_monsters(1 - a->player) == 1 && duel_08809F2C(a->player) < 0) {
        return 1;
    }
    return 0;
}

int duel_ai_0881B4DC(DuelAct *a, int b)
{
    int r;

    if (a->b3_4 != 0) {
        if (duel_count_field_cards(1 - a->player, 1, 0, 0) != 0) {
            r = 0;
        } else {
            r = !duel_count_monsters(1 - a->player);
        }
    } else {
        r = duel_08819FEC(a, b);
    }
    return r;
}

int duel_ai_0881B55C(DuelAct *a)
{
    unsigned int i;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (duel_pick_st_zone(a->player) == 0) {
        return 0;
    }
    i = 0;
    if (duel.players[a->player & 1].n_hand != 0) {
        do {
            unsigned int id = duel.players[a->player & 1].hand[i].id;

            if (card_is_monster(id) != 0 && card_is_in_group_16a3(id) != 0) {
                return 1;
            }
            i++;
        } while (i < duel.players[a->player & 1].n_hand);
    }
    return 0;
}

int duel_ai_0881B674(DuelAct *a)
{
    return (int)duel_08807434(a->player, 0) >= 0;
}

int duel_ai_0881B6A0(DuelAct *a, int b)
{
    if (duel_count_monsters_fn(a->player, card_is_attribute0) == 0) {
        return 0;
    }
    if (duel_08809F2C(a->player) < 1) {
        return 0;
    }
    return duel_08819FEC(a, b);
}

int duel_ai_0881B710(DuelAct *a)
{
    return !duel.players[a->player & 1].field[10].card.id;
}

int duel_ai_0881B748(DuelAct *a)
{
    if (a->b3_4 == 0) {
        return duel_ai_0881B710(a);
    }
    return 1;
}

int duel_ai_0881B778(DuelAct *a)
{
    unsigned int id = duel.players[a->player & 1].field[10].card.id;

    if (id == 0x10F4) {
        return 1;
    }
    return !(id != 0);
}

int duel_ai_0881B7C4(DuelAct *a)
{
    if (duel_count_field_all(1 - a->player) != 0) {
        return !duel.players[a->player & 1].field[10].card.id;
    }
    return 0;
}

int duel_ai_0881B834(DuelAct *a)
{
    return duel_count_monsters_pos(1 - a->player, 1, 0) > 0;
}

int duel_ai_0881B864(DuelAct *a)
{
    if (duel_0880B07C(a->player) != 0) {
        return duel_count_monsters_pos(1 - a->player, 1, 0) > 0;
    }
    return 0;
}

int duel_ai_0881B8B4(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].flags >> 17 & 1) {
        return 0;
    }
    if (duel_0884E948(a, 1 - p) == 0) {
        return 0;
    }
    if (duel_088154D8(a->player) != 0) {
        return 1;
    }
    if (duel_count_monsters(a->player) == 0 && duel_count_monsters(1 - a->player) >= 2) {
        return 1;
    }
    if (duel_08809F2C(a->player) < 0) {
        return 1;
    }
    return 0;
}

int duel_ai_0881B990(DuelAct *a, int b)
{
    if (duel_0881A074(a, b) != 0) {
        return duel_ai_0881A0E4(a, b);
    }
    return 0;
}

int duel_ai_0881B9D4(DuelAct *a)
{
    if (duel_088154D8(a->player) != 0) {
        return 1;
    }
    if (duel_08809F2C(a->player) < 0) {
        return 1;
    }
    return duel_0884E948(a, 1 - a->player) > 1;
}

int duel_ai_0881BA44(DuelAct *a)
{
    unsigned int p;
    int best = -1;
    unsigned int bp;
    int z;

    if (duel_08809F2C(a->player) > 0) {
        return 0;
    }
    for (p = 0; (int)p < 2; p++) {
        for (z = 0; z < 5; z++) {
            if (duel_0884E440(a, p, z) != 0) {
                int v = duel_field_atk(p, z);

                if (v > best) {
                    bp = p;
                    best = v;
                }
            }
        }
    }
    return bp == 1 - a->player;
}

int duel_ai_0881BB18(DuelAct *a)
{
    return duel_08809F2C(a->player) < 0;
}

int duel_ai_0881BB3C(DuelAct *a, int b)
{
    unsigned int i;

    if (duel_count_tributes(1 - a->player, 0, 1) > 0 && !(duel.players[a->player & 1].flags >> 17 & 1)) {
        i = 0;
        if (duel.players[a->player & 1].n_hand != 0) do {
            unsigned int id = duel.players[a->player & 1].hand[i].id;

            if (card_is_monster(id) != 0 && card_attribute_icon_in_2_3(id) == 0 &&
                duel_count_free_monster_zones(a->player) != 0) {
                int lv = duel_list_card_level(a->player, CARD_UID(duel.players[a->player & 1].hand[i]));

                if (lv >= 7) {
                    if ((int)duel_08807434(a->player, 1) >= 0) {
                        return 1;
                    }
                } else if (lv >= 5) {
                    return 1;
                }
            }
            i++;
        } while (i < duel.players[a->player & 1].n_hand);
    }
    return 0;
}

int duel_ai_0881BCF4(DuelAct *a, int b)
{
    int z;

    for (z = 0; z < 5; z++) {
        unsigned int p = a->player;
        unsigned int q = 1 - p;

        if (duel.players[q & 1].field[z].card.id != 0 && duel_can_tribute(p, q, z) != 0) {
            return 0;
        }
    }
    return duel_ai_0881BB3C(a, b);
}

int duel_ai_0881BDAC(void)
{
    return 1;
}

int duel_ai_0881BDB4(DuelAct *a)
{
    unsigned int k;
    unsigned int i;

    if (!(duel.players[a->player & 1].flags >> 17 & 1)) {
        k = duel_08816A74(a->player, duel_ai_0881BDAC, 1, 1);
        if ((int)k < 0) {
            return 0;
        }
        i = 0;
        if (duel.players[a->player & 1].n_hand != 0) {
            do {
                unsigned int id = duel.players[a->player & 1].hand[i].id;

                if (i != k) {
                if (card_is_monster(id) != 0 && card_attribute_icon_in_2_3(id) == 0 &&
                    duel_count_free_monster_zones(a->player) != 0) {
                    int lv = duel_list_card_level(a->player, CARD_UID(duel.players[a->player & 1].hand[i]));

                    if (lv >= 7) {
                        if ((int)duel_08807434(a->player, 1) >= 0) {
                            return 1;
                        }
                    } else if (lv >= 5) {
                        return 1;
                    }
                }
                }
                i++;
            } while (i < duel.players[a->player & 1].n_hand);
        }
    }
    return 0;
}

int duel_ai_0881BF94(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int i;

    if (!(duel.players[p & 1].flags >> 17 & 1) && duel_count_free_monster_zones(p) != 0) {
        i = 0;
        if (duel.players[a->player & 1].n_hand != 0) {
            do {
                unsigned int id = duel.players[a->player & 1].hand[i].id;

                if (card_is_monster(id) != 0 && card_attribute_icon_in_2_3(id) == 0 &&
                    duel_list_card_level(a->player, CARD_UID(duel.players[a->player & 1].hand[i])) >= 7) {
                    return 1;
                }
                i++;
            } while (i < duel.players[a->player & 1].n_hand);
        }
    }
    return 0;
}
