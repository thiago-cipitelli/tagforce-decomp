// @range 0x0886F23C-0x088707BC
// @asm 0x0886F23C-0x0886F390
// @asm 0x0886F3B4-0x0886F65C
// @asm 0x0886F974-0x0886FA90
// @asm 0x0886FAC0-0x0886FD74
// @asm 0x08870348-0x088704BC
// @asm 0x088705DC-0x088706CC
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
unsigned int duel_card_locate(unsigned int uid);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
int card_get_prop5_hi(int id);
int duel_0886CD24(DuelAct *a, unsigned int p);
int duel_grave_count_monsters(unsigned int p);
int duel_0884E690(DuelAct *a, int (*fn)(int id));
int duel_08857148(int id);
int duel_phase_ok2(unsigned int p);
int duel_no_1a1b(unsigned int p);
int duel_08871F70(DuelAct *a, int b);
unsigned short duel_slot_e1854(unsigned int p, int z);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
int duel_hand_first_spell(unsigned int p);
int duel_count_monsters_pos(unsigned int p, int faceup, int pos0);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_field_def(unsigned int p, int z);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_grave_count_id(unsigned int p, int id);

typedef struct DuelListCard {
    unsigned short id : 13;
    unsigned short placed : 2;
    unsigned short b15 : 1;
    unsigned short effects;
} DuelListCard;

extern DuelListCard duel_cards[512];

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

#ifdef NON_MATCHING /* the original reads the +0x14 fields through a pointer */
int duel_0886FAC0(DuelAct *a)
{
    unsigned int t;
    DuelCard *c;

    switch (a->kind) {
    case 0x1B:
    case 0x16:
        break;
    default:
        return 0;
    }
    if (a->src.zone <= 4 && !a->y.y16_7 && a->x.s_zone2 == 0xE) {
        t = duel_card_locate(a->src.uid);
        c = duel_field_card_deb2c(t & 0xFF, (t & 0xFFFF) >> 8, t >> 16);
        if (c->owner == a->player && card_is_monster(c->id)) {
            return 1;
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* the original reads the +0x14 fields through a pointer */
int duel_0886FBA4(DuelAct *a)
{
    int ok;
    int ok2;

    if (a->kind != 0x1B || a->y.y16_2 == a->player || a->src.player != a->player) {
        return 0;
    }
    switch (a->id) {
    case 0x135B:
        ok = 0;
        if (a->src.zone < 5) {
            ok = card_is_monster(duel_cards[a->src.uid].id) != 0;
        }
        if (ok) {
            return a->x.s_zone2 == 0xB;
        }
        return 0;
    case 0x134D:
        ok = 0;
        if (a->src.zone > 4) {
            ok = card_get_type(duel_cards[a->src.uid].id) == 0x17;
        }
        ok2 = 0;
        if (ok) {
            ok2 = !a->y.y16_7;
        }
        if (ok2) {
            return a->x.s_zone2 == 0xE;
        }
        return 0;
    case 0x134C:
        ok = 0;
        if (a->src.zone > 4) {
            ok = card_get_type(duel_cards[a->src.uid].id) != 0x17;
        }
        ok2 = 0;
        if (ok) {
            ok2 = !a->y.y16_7;
        }
        if (!ok2) {
            return 0;
        }
        return a->x.s_zone2 == 0xE;
    }
    return 0;
}
#endif

int duel_0886FD74(DuelAct *a)
{
    return a->kind == 0x18;
}

int duel_0886FD8C(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (b->b3_4 != 0) {
        return 0;
    }
    switch (a->id) {
    case 0x134F:
        return card_get_type(b->id) == 0x16 && card_get_prop5_hi(b->id) == 2;
    case 0x1350:
        return card_get_type(b->id) == 0x16 && card_get_prop5_hi(b->id) == 4;
    case 0x1351:
        return card_get_type(b->id) == 0x17 && card_get_prop5_hi(b->id) == 4;
    }
    return 0;
}

int duel_0886FEA8(DuelAct *a)
{
    switch (a->kind) {
    case 15:
        if (a->y.y16_5 == a->player) {
            return 1;
        }
        break;
    case 16:
        if (a->y.y16_0 == a->player) {
            return 1;
        }
        break;
    }
    return 0;
}

int duel_0886FF1C(DuelAct *a)
{
    if (a->kind == 25) {
        if (a->y.y15_1 == a->player) {
            return 0;
        }
        return duel_phase != 0;
    }
    return 0;
}

int duel_0886FF68(DuelAct *a)
{
    if (a->kind == 29) {
        return a->y.y15_1 == a->player;
    }
    return 0;
}

int duel_0886FFA4(DuelAct *a)
{
    return duel.players[(1 - a->player) & 1].lp < 3001;
}

int duel_0886FFDC(DuelAct *a)
{
    if (duel.players[(1 - a->player) & 1].n_hand < 6) {
        return 0;
    }
    if (duel_0886CD24(a, a->player) > 2) {
        return 0;
    }
    return !(duel.players[a->player & 1].n_d < 5U - duel_0886CD24(a, a->player));
}

int duel_0887009C(DuelAct *a)
{
    if (duel_grave_count_monsters(a->player) < 5) {
        return 0;
    }
    return duel_088BFFD8(a->player, a->id, 0) != 0;
}

/* Some monster is face down in attack position, or the hand holds a card that passes the test. */
int duel_088700F0(DuelAct *a)
{
    unsigned int p;
    int z;

    for (p = 0; (int)p < 2; p++) {
        for (z = 0; z < 5; z++) {
            if (duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup == 0 &&
                duel.players[p & 1].field[z].pos != 0) {
                return 1;
            }
        }
    }
    return duel_0884E690(a, duel_08857148) > 0;
}

int duel_08870188(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel_turn_player == p) {
        return 0;
    }
    if (duel_phase != 3) {
        return 0;
    }
    if (duel_0886CD1C(a, p) < 2) {
        return 0;
    }
    if ((unsigned int)duel.players[a->player & 1].n_d < 2) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    return duel_no_1a1b(a->player) != 0;
}

int duel_08870258(DuelAct *a)
{
    int p;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    for (p = 0; p < 2; p++) {
        if (duel_0886CD1C(a, p) == 0) {
            return 0;
        }
        if (duel_088BFFD8(p, a->id, 0) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_088702E4(DuelAct *a)
{
    unsigned int p;

    if (duel_phase != 1) {
        return 0;
    }
    p = a->player;
    if (p == duel_turn_player) {
        return 0;
    }
    return duel.players[(1 - p) & 1].n_hand != 0;
}

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_08870348(DuelAct *a, int b)
{
    unsigned int src;

    if (a->kind == 2) {
        src = duel_slot_e1854(a->player, a->zone);
        if ((src & 0xFFFF) != 0xFFFF) {
            if (duel_st_affects(src & 0xFF, (unsigned char)((src & 0xFFFF) >> 8), a->player, a->zone) != 10) {
                return 0;
            }
            return 1;
        }
        return 0;
    }
    return duel_08871F70(a, b);
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_088703E0(DuelAct *a)
{
    unsigned int q = (1 - a->player) & 1;
    unsigned int i;

    for (i = 0; i < duel.players[q].n_grave; i++) {
        if (card_get_type(duel.players[q].grave[i].id) == 0x16) {
            return 1;
        }
        q = (1 - a->player) & 1;
    }
    return 0;
}
#endif

int duel_088704BC(DuelAct *a)
{
    return duel_hand_first_spell(1 - a->player) >= 0;
}

int duel_088704EC(DuelAct *a)
{
    return duel_count_monsters_pos(0, 1, 0) + duel_count_monsters_pos(1, 1, 0) != 0;
}

int duel_08870534(DuelAct *a)
{
    switch (a->kind) {
    case 0x13:
    case 0x12:
        break;
    default:
        return 0;
    }
    if (duel_special.player[0] != 1 - a->player) {
        return 0;
    }
    if (duel_act_can_target(a, duel_special.player[0], duel_special.zone[0]) == 0) {
        return 0;
    }
    return !duel_act_immune(a, duel_special.player[0], duel_special.zone[0]);
}

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_088705DC(DuelAct *a)
{
    unsigned int sp = a->src.player;
    unsigned int sz;

    if (sp == a->player) {
        return 0;
    }
    sz = a->src.zone;
    if (duel.players[sp].field[sz].card.id == 0) {
        return 0;
    }
    if (duel.players[sp].field[sz].faceup == 0) {
        return 0;
    }
    return duel_field_def(sp, sz) < 2001;
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_0887068C(DuelAct *a)
{
    return duel.players[a->player & 1].lp <= 1 ? 1 : 2;
}
#endif

int duel_088706CC(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int q = p ^ a->opp;

    if (duel_tributable(q, p, a->zone) == 0) {
        return 0;
    }
    if (duel_0886CD1C(a, q) < (int)((q != a->player) + 2)) {
        return 0;
    }
    if (duel_phase_ok2(q) == 0) {
        return 0;
    }
    if (duel_grave_count_id(q, 0x1288) == 0) {
        return 0;
    }
    if (duel_grave_count_id(q, 0x129B) == 0) {
        return 0;
    }
    if (duel_grave_count_id(q, 0x12B8) == 0) {
        return 0;
    }
    return 2;
}
