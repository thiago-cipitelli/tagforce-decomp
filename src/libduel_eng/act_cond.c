// @range 0x0886CB44-0x0886D654
// @asm 0x0886CD24-0x0886CD90
// @asm 0x0886D050-0x0886D0C4
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

#ifdef NON_MATCHING /* the found test is computed before the player load in the original */
/* Cards in the hand of a's player besides card a itself. */
int duel_0886CD24(DuelAct *a, unsigned int p)
{
    return duel.players[a->player & 1].n_hand - (duel_hand_find_uid(p, a->uid) < 0 ^ 1);
}
#endif

int duel_0886CD90(DuelAct *a)
{
    return !duel_slot_has_effect(a->player, a->zone, 0xFB6);
}

int duel_0886CDC4(DuelAct *a)
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

int duel_0886CE90(DuelAct *a)
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

int duel_0886CFE0(DuelAct *a)
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
