// @range 0x08819F94-0x0881AA48
// @asm 0x0881A118-0x0881A1E4
// @asm 0x0881A2B4-0x0881A3EC
// @asm 0x0881A3EC-0x0881A554
// @asm 0x0881A7C0-0x0881A960
// @asm 0x0881A960-0x0881AA48
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

int duel_ai_0881A0E4(DuelAct *a)
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
