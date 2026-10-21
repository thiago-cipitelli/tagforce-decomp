// @range 0x0886CB44-0x0886E784
// @asm 0x0886D050-0x0886D0C4
// @asm 0x0886D828-0x0886DBC4
// @asm 0x0886DE70-0x0886E074
// @asm 0x0886E590-0x0886E784
/* Activation conditions of cards (DuelAct *a): enough cards to pay or tribute, free zones, phase and
   turn, cards in the piles. */
#include "duel.h"

/* a field of duel.players[p] addressed from the field of player 0 (offset folded into the address) */
#define PL_FIELD(T, f, p) (*(T *)((char *)&duel.players[0].f + (p) * sizeof(DuelPlayer)))

extern unsigned int duel_turn_player;
extern int duel_phase;

int duel_0886C99C(unsigned int p, int n, int uid, int mode);
int duel_count_free_monster_zones(unsigned int p);
int card_is_monster(int id);
int card_get_type(int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_088DFCC8(unsigned int uid);
int duel_hand_find_uid(unsigned int p, int uid);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_08860FDC(DuelAct *a);
int duel_phase_ok2(unsigned int p);
int duel_088AA224(DuelAct *a);
int duel_088BFFD8(unsigned int p, int id, int i);
void duel_act_0887E140(DuelAct *a, int *n);
int duel_pick_st_zone(unsigned int p);
int duel_count_tributable_except(unsigned int p, int src);
int duel_088E9150(int id);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_0884E5E0(DuelAct *a);
int duel_find_st(unsigned int p, int id);
unsigned short duel_slot_e1854(unsigned int p, int z);
int duel_count_free_zones(unsigned int p);
int duel_zone_usable(unsigned int p, unsigned int z);
unsigned int duel_act_target(DuelAct *a, int i);
void duel_0886AF00(unsigned int p, int z, int id);
unsigned short duel_slot_src13(unsigned int p, int z);
int duel_slot_has_effect2(unsigned int p, int z, int id);
int duel_08871F70(DuelAct *a, int b);
int duel_count_monsters(unsigned int p);
unsigned int duel_card_locate(unsigned int uid);
int duel_pick_monster_zone(unsigned int p);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
int card_is_attribute6(int id);
int card_attribute_icon_in_2_3(int id);
int duel_has_st_12be(unsigned int p);
int duel_0884E948(DuelAct *a, unsigned int p);
int duel_count_active(int id);
int duel_088DFE24(int p, unsigned int q);
int duel_08827D98(unsigned int p, int uid);

extern unsigned int duel_chain2_n;     /* 0x08914920 */
extern unsigned char duel_chain2_kind[]; /* 0x08914928 */
extern struct { unsigned int s_uid : 9, s_player : 1, s_zone : 4, s_b14 : 1, s_b15 : 1, s_b16 : 1, s_b17 : 1, s_b18 : 1, s_zone2 : 4, s_b23 : 9; } duel_chain2_src[]; /* 0x08914948 */

int duel_0886CB44(unsigned int p, int n, int uid, int monster, int all)
{
    int mode = 0;

    if (monster) {
        mode = 1;
    }
    if (all) {
        mode = 7;
    }
    return duel_0886C99C(p, n, uid, mode);
}

/* Player p can discard n cards from the hand besides the card with uid (monsters only, or cards of
   type 0x16 only). */
int duel_hand_can_discard(unsigned int p, int n, int uid, int monster, int type16)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        int id = duel.players[p & 1].hand[i].id;
        int ok = id > 0;

        if (uid == CARD_UID(duel.players[p & 1].hand[i])) {
            ok = 0;
        }
        if (monster && card_is_monster(id) == 0) {
            ok = 0;
        }
        if (type16 && card_get_type(id) != 0x16) {
            ok = 0;
        }
        if (duel_slot_effect_with(p, 11, 0x12A1, CARD_UID(duel.players[p & 1].hand[i]))) {
            ok = 0;
        }
        if (duel_088DFCC8(CARD_UID(duel.players[p & 1].hand[i])) == 0) {
            ok = 0;
        }
        if (ok) {
            n--;
        }
    }
    return !(n > 0);
}

int duel_0886CD1C(DuelAct *a, unsigned int p)
{
    return duel_count_free_monster_zones(p);
}

/* Cards in the hand of a's player besides card a itself. */
int duel_0886CD24(DuelAct *a, unsigned int p)
{
    int f = duel_hand_find_uid(p, a->uid) >= 0;

    return duel.players[a->player & 1].n_hand - f;
}

int duel_0886CD90(DuelAct *a, int b)
{
    return !duel_slot_has_effect(a->player, a->zone, 0xFB6);
}

int duel_0886CDC4(DuelAct *a, int b)
{
    int n;

    if (a->kind != 0x23) {
        n = duel_08860FDC(a);
        if (!duel_0886CB44(a->player, n, a->uid, 0, 0)) {
            return 0;
        }
    }
    return 1;
}

int duel_0886CE38(DuelAct *a, int b)
{
    unsigned int p = a->player ^ a->opp;

    if (duel_0886CD1C(a, p) == 0) {
        return 0;
    }
    return duel_phase_ok2(p) != 0;
}

int duel_0886CE90(DuelAct *a, int b)
{
    unsigned int p = a->player ^ a->opp;

    if (duel_0886CD1C(a, p) == 0) {
        return 0;
    }
    if (duel_phase_ok2(p) == 0) {
        return 0;
    }
    return duel_088BFFD8(p, a->id, duel_088AA224(a)) != 0;
}

int duel_0886CF1C(DuelAct *a, int b)
{
    unsigned int p = a->player ^ a->opp;
    unsigned int i;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (duel_088BFFD8(p, a->id, i)) {
            return 1;
        }
    }
    return 0;
}

int duel_0886CFE0(DuelAct *a, int b)
{
    return duel_088BFFD8(a->player, a->id, duel_088AA224(a)) != 0;
}

int duel_0886D01C(DuelAct *a)
{
    int n[2];

    duel_act_0887E140(a, n);
    return n[0] != 0 || n[1] != 0;
}

#ifdef NON_MATCHING /* the last test is not shared with the other returns */
int duel_0886D050(DuelAct *a, int b)
{
    unsigned int p;

    if (b != 0) {
        return 0;
    }
    p = a->player;
    if (p == duel_turn_player) {
        return 0;
    }
    if (duel_phase != 3) {
        return 0;
    }
    if (a->kind == 0xE) {
        return a->y.y16_5 == p;
    }
    return 0;
}
#endif

int duel_0886D0C4(DuelAct *a)
{
    if (duel.players[a->player & 1].flags >> 17 & 1) {
        return 0;
    }
    if (duel_count_free_monster_zones(a->player) == 0) {
        return 0;
    }
    if (a->zone > 10 && duel_pick_st_zone(a->player) < 0) {
        return 0;
    }
    return 1;
}

int duel_0886D168(DuelAct *a)
{
    return duel_count_tributable_except(a->player, -1) > 0;
}

int duel_0886D190(DuelAct *a)
{
    return duel.players[(a->player ^ a->opp) & 1].n_d != 0;
}

int duel_0886D1D0(DuelAct *a)
{
    unsigned int p = a->player;

    return duel_count_tributable_except(p, (unsigned short)(p | a->zone << 8)) > 0;
}

int duel_0886D208(DuelAct *a)
{
    return duel_count_tributable_except(a->player, -1) != 0;
}

int duel_0886D230(DuelAct *a, int b)
{
    if (a->kind != 0x23 && duel_088E9150(0xFE4) == 0) {
        return 0;
    }
    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    if (duel.players[a->player & 1].n_d == 0 && duel_088BFFD8(a->player, a->id, 0) == 0) {
        return 0;
    }
    return 1;
}

int duel_0886D2F0(DuelAct *a)
{
    unsigned int p;

    if (a->kind != 0xD) {
        return 0;
    }
    if (duel_slot_effect_kind(a->player, a->zone, 0x1119, 3)) {
        return 0;
    }
    p = a->player;
    if (duel_special.player[0] == 1 - p) {
        if (duel_special.player[1] == p && duel_special.zone[1] == a->zone &&
            duel_act_can_target(a, duel_special.player[0], duel_special.zone[0])) {
            return 1;
        }
    }
    return 0;
}

int duel_0886D3C0(DuelAct *a)
{
    unsigned int p = a->player;

    if (p != duel_turn_player) {
        return 0;
    }
    if (duel_phase != 1) {
        return 0;
    }
    if (duel.players[p & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    if (duel_tributable(p, p, a->zone) == 0) {
        return 0;
    }
    if (duel_0884E5E0(a) >= 2) {
        return 2;
    }
    return 0;
}

int duel_0886D494(DuelAct *a)
{
    return duel.players[a->player & 1].n_18 != 0;
}

int duel_0886D4C4(DuelAct *a)
{
    if (a->kind == 2) {
        return 1;
    }
    return duel_0886CD1C(a, 1 - a->player) != 0;
}

int duel_0886D50C(DuelAct *a)
{
    return duel_find_st(1 - a->player, 0x1102) >= 0;
}

int duel_0886D540(DuelAct *a)
{
    if (duel_0884E5E0(a) < 2) {
        return 0;
    }
    return 2;
}

int duel_0886D56C(DuelAct *a)
{
    return duel_phase != 3;
}

int duel_0886D580(DuelAct *a)
{
    unsigned int p;

    if ((unsigned int)(a->kind - 0x12) < 2) {
        p = duel_special.player[0];
        if (duel.players[p & 1].field[duel_special.zone[0]].card.id != 0 &&
            duel.players[p & 1].field[duel_special.zone[0]].pos == 0 && duel_act_can_target(a, p, duel_special.zone[0]) &&
            a->player != p) {
            return 2;
        }
    }
    return 1;
}

int duel_0886D654(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int q = p ^ a->opp;
    unsigned int src = duel_slot_e1854(p, a->zone);
    unsigned int sp;
    unsigned int sz;

    if ((src & 0xFFFF) == 0xFFFF) {
        return 0;
    }
    sz = (unsigned char)((src & 0xFFFF) >> 8);
    sp = src & 0xFF;
    if (duel_tributable(q, sp, sz) == 0) {
        return 0;
    }
    if (duel_count_free_zones(q) == 0 && (sp != q || duel_zone_usable(sp, sz) == 0)) {
        return 0;
    }
    if (duel_phase_ok2(q) == 0) {
        return 0;
    }
    return duel.players[q & 1].n_d != 0;
}

int duel_0886D754(DuelAct *a)
{
    int need = 0;

    switch (a->id) {
    case 0x12D2:
        need = 4;
        break;
    case 0x1710:
        need = 2;
        break;
    }
    if (a->kind != 0x23 && (duel.players[a->player & 1].flags >> 16 & 1)) {
        return 0;
    }
    if (duel_0886CD1C(a, a->player) < need) {
        return 0;
    }
    return duel_phase_ok2(a->player) != 0;
}

#ifdef NON_MATCHING /* the original reuses the registers of a and b for the target fields */
/* Activation b (the chain link a answers) targets or concerns a trap card other than a's own. */
int duel_0886D828(DuelAct *a, DuelAct *b)
{
    unsigned int t;
    unsigned int tp;
    unsigned int tz;
    int id;
    unsigned int z;
    int i;

    if (b == 0) {
        return 0;
    }
    if (a->player == (b->player ^ b->opp)) {
        return 0;
    }
    switch (b->id) {
    case 0xFFA:
    case 0x1086:
    case 0x12E6:
    case 0x132D:
    case 0x134C:
    case 0x134D:
    case 0x16B8:
    case 0x1364:
        t = duel_act_target(b, 0);
        tp = t & 0xFF;
        tz = (unsigned char)((t & 0xFFFF) >> 8);
        id = duel.players[tp & 1].field[tz].card.id;
        if (id == 0) {
            return 0;
        }
        if (tp == a->player && tz == a->zone) {
            return 0;
        }
        if (card_get_type((unsigned short)id) == 0x17) {
            return 1;
        }
        return 0;
    case 0x149B:
        t = duel_act_target(b, 0);
        tp = t & 0xFF;
        tz = (unsigned char)((t & 0xFFFF) >> 8);
        id = duel.players[tp & 1].field[tz].card.id;
        if (id == 0) {
            return 0;
        }
        if (tp == a->player && tz == a->zone) {
            return 0;
        }
        if (card_get_type((unsigned short)id) == 0x17) {
            return 1;
        }
        duel_0886AF00(tp, tz, id);
        return 0;
    case 0x11F0:
        for (i = 0; i < 2; i++) {
            t = duel_act_target(b, i);
            tp = t & 0xFF;
            tz = (unsigned char)((t & 0xFFFF) >> 8);
            id = duel.players[tp & 1].field[tz].card.id;
            if (id != 0 && (tp != a->player || tz != a->zone) && card_get_type((unsigned short)id) == 0x17) {
                return 1;
            }
        }
        return 0;
    case 0x1246:
    case 0x131B:
    case 0x131C:
    case 0x1321:
        for (z = 5; (int)z < 10; z++) {
            id = duel.players[a->player].field[z].card.id;
            if (id != 0 && z != a->zone && card_get_type((unsigned short)id) == 0x17) {
                return 1;
            }
        }
        return 0;
    }
    return 0;
}
#endif

int duel_0886DBC4(DuelAct *a, int b)
{
    unsigned int p = a->player;

    if (duel_slot_src13(p, a->zone) != 0xFFFF) {
        return 0;
    }
    if (duel_pick_st_zone(p) == -1) {
        return 0;
    }
    return duel_0886CD90(a, b);
}

int duel_0886DC4C(DuelAct *a)
{
    if (a->kind == 2) {
        unsigned char z = a->zone;
        unsigned int p = a->player;

        if (!(duel.players[p & 1].field[z].flags >> 4 & 1)) {
            return 0;
        }
        if (duel_tributable(p, p, z) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_0886DCEC(DuelAct *a, int b)
{
    if (a->kind == 2) {
        return duel_slot_has_effect2(a->player, a->zone, a->id);
    }
    return duel_08871F70(a, b);
}

int duel_0886DD40(DuelAct *a, int b)
{
    if (duel_phase != 2) {
        return 0;
    }
    return duel_0886CD90(a, b);
}

int duel_0886DD78(DuelAct *a)
{
    unsigned int z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    return duel.players[p & 1].field[z].faceup != 0;
}

int duel_0886DE0C(DuelAct *a)
{
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    return duel.players[a->player & 1].n_hand != 0;
}

#ifdef NON_MATCHING /* draft */
/* The only chain link that brings a monster back from the graveyard is this one, and its monster can
   come back. */
int duel_0886DE70(DuelAct *a)
{
    int n = 0;
    unsigned int i;
    unsigned int t;
    DuelCard *c;

    if (a->kind != 0x16 || duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    for (i = 0; i < duel_chain2_n; i++) {
        if (duel_chain2_kind[i] == 0x16 && duel_chain2_src[i].s_b16 && duel_chain2_src[i].s_b15 &&
            duel_chain2_src[i].s_zone2 == 0xE) {
            n++;
        }
    }
    if (n != 1) {
        return 0;
    }
    if (!a->y.y16_0 || !a->y.y15_7 || a->x.s_zone2 != 0xE) {
        return 0;
    }
    t = duel_card_locate(a->src.uid);
    if (duel_pick_monster_zone(a->src.player) < 0) {
        return 0;
    }
    if ((t & 0xFFFF) >> 8 != 0xE) {
        return 0;
    }
    c = duel_field_card_deb2c(t & 0xFF, 0xE, t >> 16);
    if (card_is_monster(c->id) && duel_grave_card_usable(a->player, t & 0xFF, t >> 16)) {
        if (card_is_attribute6(c->id) == 0) {
            return 1;
        }
        if (card_attribute_icon_in_2_3(c->id) == 0 || duel_has_st_12be(a->player)) {
            return 1;
        }
    }
    return 0;
}
#endif

int duel_0886E074(DuelAct *a)
{
    if (a->kind != 0x12) {
        return 0;
    }
    if (a->idx == 0) {
        return 0;
    }
    if (duel_special.player[0] != 1 - a->player) {
        return 0;
    }
    return duel_act_can_target(a, duel_special.player[0], duel_special.zone[0]) != 0;
}

int duel_0886E0E8(DuelAct *a)
{
    return duel_phase_ok2(a->player) != 0;
}

int duel_0886E10C(DuelAct *a)
{
    return duel.players[(1 - a->player) & 1].n_hand != 0;
}

int duel_0886E144(DuelAct *a, int b)
{
    if (a->kind != 0x23 && duel_088E9150(0x128B) == 0) {
        return 0;
    }
    return duel_0886CF1C(a, b);
}

int duel_0886E1A8(DuelAct *a)
{
    unsigned int n = duel_0886CD24(a, a->player);
    unsigned int p = a->player;

    if (duel.players[p & 1].n_d < n) {
        return 0;
    }
    if (n == 0 && duel.players[(1 - p) & 1].n_hand == 0) {
        return 0;
    }
    return 1;
}

int duel_0886E250(DuelAct *a)
{
    if (a->kind == 0xB) {
        if (a->y.y15_1 != 1 - a->player) {
            return 0;
        }
        if (a->y.y15_7 == 0) {
            return 0;
        }
        return a->y.y15_6 != 0;
    }
    return 0;
}

int duel_0886E2B8(DuelAct *a, int b)
{
    unsigned int p;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    p = a->player;
    if (duel.players[p & 1].n_d == 0) {
        return 0;
    }
    if (a->kind != 0x23 && (duel.players[p & 1].flags >> 16 & 1)) {
        return 0;
    }
    return 1;
}

int duel_0886E35C(DuelAct *a)
{
    unsigned int p;

    if (a->kind != 0x12) {
        return 0;
    }
    if (a->idx == 0) {
        return 0;
    }
    p = a->player;
    if (duel_special.player[0] == p) {
        return 0;
    }
    if (duel_count_monsters(p) == 0) {
        return 0;
    }
    return duel_0886CD1C(a, a->player) != 0;
}

int duel_0886E3F0(DuelAct *a)
{
    switch (a->kind) {
    case 7:
    case 6:
        return 1;
    }
    return 0;
}

int duel_0886E428(DuelAct *a)
{
    if (duel_0884E948(a, a->player) < 2) {
        return 0;
    }
    if (duel_0884E948(a, 1 - a->player) < 1) {
        return 0;
    }
    return 2;
}

int duel_0886E48C(DuelAct *a, int b)
{
    if (duel_count_active(0x12FB) >= 1) {
        return 0;
    }
    return duel_0886CE90(a, b);
}

int duel_0886E4D8(DuelAct *a)
{
    unsigned int need;

    switch (a->id) {
    case 0x12CC:
        need = 3;
        break;
    case 0x12EC:
        need = 2;
        break;
    default:
        need = 1;
        break;
    }
    return !(duel.players[(a->player ^ a->opp) & 1].n_d < need);
}

int duel_0886E53C(DuelAct *a)
{
    if (duel_088DFE24(a->player, 1 - a->player) != 0) {
        return duel_088BFFD8(a->player, a->id, 0) > 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* the phase tests and the hand loop are laid out differently */
/* Paying 500 LP to activate is allowed when another card in the hand could be used now and the chain
   does not already hold all of them. */
int duel_0886E590(DuelAct *a)
{
    unsigned int p = a->player;
    int ok;
    int n;
    unsigned int i;
    int k;

    if (duel_turn_player == p) {
        ok = 1;
        if (duel_phase != 2 && duel_phase != 4) {
            ok = 0;
        }
    } else {
        ok = duel_phase == 3;
    }
    if (ok && duel.players[p].lp > 500) {
        n = 0;
        for (i = 0; i < duel.players[p].n_hand; i++) {
            if (duel_08827D98(p, CARD_UID(duel.players[p].hand[i]))) {
                n++;
            }
            p = a->player;
        }
        if (n != 0) {
            for (k = duel_chain.len - 1; k >= 0; k--) {
                if (a->uid != duel_chain.acts[k].uid) {
                    break;
                }
                n--;
            }
            if (n != 0) {
                return 2;
            }
        }
    }
    return a->b3_4 != 3;
}
#endif
