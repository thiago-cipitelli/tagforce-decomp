// @range 0x0886F23C-0x0886FAC0
// @asm 0x0886F23C-0x0886F390
// @asm 0x0886F3B4-0x0886F65C
// @asm 0x0886F974-0x0886FA90
/* More activation conditions of cards (DuelAct *a; b: the chain link a answers). */
#include "duel.h"

extern unsigned int duel_turn_player;
extern int duel_phase;
extern int duel_effects_off;

int duel_088DFE24(int p, unsigned int q);
int card_is_monster(int id);
int card_get_type(int id);
int card_is_in_group_147e(int id);
int duel_0886CD1C(DuelAct *a, unsigned int p);
int duel_0886CE38(DuelAct *a, int b);
int duel_0886CB44(unsigned int p, int n, int uid, int monster, int all);
int duel_count_tributable_except(unsigned int p, int src);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_hand_find_uid(unsigned int p, int uid);
int duel_count_monsters(unsigned int p);
int duel_count_active(int id);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_act_target_is_monster(DuelAct *a);
int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_088BFFD8(unsigned int p, int id, int i);

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_0886F23C(DuelAct *a)
{
    if (duel_088DFE24(a->player, 0) && duel.players[0].n_grave != 0) {
        return 1;
    }
    if (duel_088DFE24(a->player, 1) == 0) {
        return 0;
    }
    if (duel.players[1].n_grave == 0) {
        return 0;
    }
    return 1;
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_0886F2C0(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (!(!card_is_monster(duel.players[p & 1].hand[i].id) != 0)) {
            return 1;
        }
        p = a->player;
    }
    return 0;
}
#endif

int duel_0886F390(DuelAct *a)
{
    return duel_0886CD1C(a, a->player) != 0;
}

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_0886F3B4(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        switch (a->kind) {
        case 0x21:
        case 0x20:
        case 0x1F:
            break;
        default:
            return 0;
        }
        return duel.players[a->src.player].field[a->src.zone].card.id != 0;
    }
    if (card_is_monster(b->id)) {
        return 0;
    }
    if (b->zone >= 11) {
        return 0;
    }
    if (b->b3_4 != 0) {
        return 0;
    }
    return card_is_in_group_147e(b->id) == 0;
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_0886F4CC(DuelAct *a, int b)
{
    if (b != 0) {
        return 0;
    }
    switch (a->kind) {
    case 0x21:
    case 0x20:
    case 0x1F:
        break;
    default:
        return 0;
    }
    if (duel.players[a->src.player].field[a->src.zone].card.id == 0) {
        return 0;
    }
    return duel_count_tributable_except(a->player, a->src.player | a->src.zone << 8) != 0;
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_0886F59C(DuelAct *a, int b)
{
    if (b != 0) {
        return 0;
    }
    if (duel_turn_player != a->player) {
        return 0;
    }
    if (duel.mode & 1) {
        return 0;
    }
    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    if (!(duel.players[a->player & 1].flags >> 25 & 1)) {
        return 0;
    }
    return duel_slot_has_effect(a->player, 11, a->id) != 0;
}
#endif

int duel_0886F65C(DuelAct *a)
{
    if (a->kind != 0x12) {
        return 0;
    }
    if (a->idx == 0) {
        return 0;
    }
    return duel_special.player[0] != a->player;
}

int duel_0886F6A8(DuelAct *a)
{
    if (a->kind != 0x23 && duel_count_tributable_except(a->player, -1) == 0) {
        return 0;
    }
    if (!duel_count_tributable_except(1 - a->player, -1)) {
        return 0;
    }
    return 1;
}

int duel_0886F720(DuelAct *a)
{
    unsigned int p;

    if (duel_phase != 1) {
        return 0;
    }
    p = a->player;
    if (duel_turn_player != p) {
        return 0;
    }
    if (a->kind > 5) {
        return 0;
    }
    if (duel_hand_find_uid(p, a->uid) >= 0) {
        return 0;
    }
    if (duel_count_monsters(a->player) >= 1) {
        return 1;
    }
    return duel_count_monsters(1 - a->player) > 0;
}

int duel_0886F7DC(DuelAct *a)
{
    return duel_0886CB44(a->player, 5, a->uid, 0, 0) != 0;
}

int duel_0886F818(DuelAct *a)
{
    if (duel_count_active(0x135D)) {
        return 0;
    }
    if (a->kind != 0x23 && duel_0886CB44(a->player, 2, a->uid, 0, 0) == 0) {
        return 0;
    }
    return 1;
}

/* The monster in zone z of p would be affected by b, which does not target it already. */
int duel_0886F89C(DuelAct *b, unsigned int p, int z)
{
    int r;

    if (b == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if ((unsigned short)((unsigned char)p | (unsigned char)z << 8) == duel_act_target(b, 0)) {
        return 0;
    }
    duel_effects_off = 1;
    r = duel_0884E3E4(b, p, z);
    duel_effects_off = 0;
    return r;
}

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_0886F974(DuelAct *a, DuelAct *b)
{
    int p;
    int z;

    if (b == 0 || b->b3_4 != 0 || duel_act_target_is_monster(b) == 0 ||
        1 - a->player != (b->player ^ b->opp) || card_get_type(b->id) != 0x16) {
        return 0;
    }
    for (p = 0; p < 2; p++) {
        for (z = 0; z < 5; z++) {
            if (duel_act_can_target(a, p, z) && duel_0886F89C(b, p, z)) {
                return 1;
            }
        }
    }
    return 0;
}
#endif

int duel_0886FA90(DuelAct *a)
{
    return duel_088BFFD8(a->player, a->id, 0) > 4;
}
