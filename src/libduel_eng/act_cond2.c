// @range 0x0886F23C-0x0887A720
// @asm 0x0886F23C-0x0886F390
// @asm 0x0886F3B4-0x0886F65C
// @asm 0x0886F974-0x0886FA90
// @asm 0x0886FAC0-0x0886FD74
// @asm 0x08871008-0x08871168
// @asm 0x088711BC-0x0887132C
// @asm 0x0887138C-0x08871424
// @asm 0x08870C54-0x08870EF8
// @asm 0x08870F90-0x08871008
// @asm 0x08870848-0x088708E4
// @asm 0x08870968-0x08870A38
// @asm 0x08870A94-0x08870B4C
// @asm 0x08871F70-0x08871FFC
// @asm 0x08871B14-0x08871BFC
// @asm 0x088719DC-0x08871A68
// @asm 0x08871674-0x088717B4
// @asm 0x0887153C-0x08871674
// @asm 0x08871424-0x0887153C
// @asm 0x08870348-0x088704BC
// @asm 0x088705DC-0x088706CC
// @asm 0x088731C0-0x088732B4
// @asm 0x08873368-0x088733C0
// @asm 0x08873B8C-0x08873C84
// @asm 0x088745A8-0x088746D0
// @asm 0x088746D0-0x0887477C
// @asm 0x08874DAC-0x08874E3C
// @asm 0x08875AD0-0x08875BD4
// @asm 0x08876404-0x0887648C
// @asm 0x088766B0-0x088768A8
// @asm 0x08876AEC-0x08876CF0
// @asm 0x088770C0-0x088771D4
// @asm 0x088775A0-0x08877624
// @asm 0x08878538-0x088785E0
// @asm 0x08878FA4-0x08879194
// @asm 0x08879D5C-0x08879E20
// @asm 0x08879EA8-0x08879F60
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
int duel_field_atk(unsigned int p, int z);
int duel_list_has_effect(int uid, int id);
unsigned int duel_grave_find_uid(unsigned int p, int uid);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
unsigned int duel_field_type(unsigned int p, int z);
int duel_card_dfac8(unsigned int p, int id, int field);
int duel_hand_can_discard(unsigned int p, int n, int uid, int monster, int type16);
int duel_is_cur_card(int id, int b);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_0886F89C(DuelAct *b, unsigned int p, int z);
int duel_slot_effect_arg(unsigned int p, int z, int id);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
int card_is_attribute6(int id);
int card_attribute_icon_in_2_3(int id);
int duel_has_st_12be(unsigned int p);
int duel_card_check_9148(unsigned int p, int id);
int duel_0884E948(DuelAct *a, unsigned int p);
int duel_zone_free(unsigned int p, int z);
int duel_pick_st_zone(unsigned int p);
int duel_slot_efb84(unsigned int p, int z);
int duel_08875530(DuelAct *a);
int duel_locks_ok(int p, int monster);
int duel_0886CD90(DuelAct *a, int b);
int duel_card_fits_zone(unsigned int p, int z, int id);
int duel_field_level(unsigned int p, int z);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
int card_is_in_group_160f(int id);
int duel_0886CDC4(DuelAct *a, int b);
int duel_0886CE90(DuelAct *a, int b);
int duel_0886CF1C(DuelAct *a, int b);
int duel_0886CFE0(DuelAct *a, int b);
int duel_slot_has_effect2(unsigned int p, int z, int id);
int field_get_card(unsigned int p, int z);
int duel_count_type(unsigned int p, unsigned int type);
int duel_088770C0(DuelAct *a, int b);
extern unsigned int duel_w_089147E4;
extern unsigned short duel_w_089147E8[];
int duel_list_counter(int uid, int id);
int duel_0884E5E0(DuelAct *a);
int duel_08861360(DuelAct *a);
int card_is_attribute9(int id);
int duel_count_field(unsigned int p);
int duel_slot_value_1909(unsigned int p, int z);
int duel_08873730(DuelAct *a);
int duel_08874A10(DuelAct *a, int b);
int duel_0887307C(DuelAct *a);
int duel_08872D64(DuelAct *a);
int duel_slot_efacc(unsigned int p, int z, int c); /* defined with two parameters; called here with a third 0 */
int duel_count_free_monster_zones(unsigned int p);
int duel_act_e4168(DuelAct *a);
int duel_act_e403c(DuelAct *a);
int duel_088DFCC8(unsigned int uid);
int duel_count_e17a0(unsigned int p, int id);
int duel_088E9150(int id);
DuelCard *duel_card_by_uid(int uid);
int duel_hand_count_monsters(unsigned int p);
int card_get_prop5_hi(int id);
int duel_0886DE0C(DuelAct *a, int b); /* defined with one parameter in act_cond.c; callers here pass b on */
unsigned short duel_slot_src_attr8(unsigned int p, int z, int active);
int duel_088AA224(DuelAct *a);
int duel_zone_usable(unsigned int p, unsigned int z);
int duel_08874098(DuelAct *a);
int duel_08874528(DuelAct *a, int b);
int duel_08836868(unsigned int p);
int duel_088613D8(DuelAct *a);
int duel_slot_e68a0(unsigned int p, int z);
int duel_act_e30c0(DuelAct *a);
int duel_sum_e68a0(unsigned int p);
int duel_first_free_zone(unsigned int p);
int duel_hand_count_name(unsigned int p, unsigned short id);
int card_same_name(int id, int id2);
int duel_act_target2(DuelAct *a, int i);
int duel_act_target_flag9(DuelAct *a);
unsigned int duel_field_flag9(unsigned int p, int z);
int duel_088752D8(DuelAct *a, int b);
int duel_08878260(void);
int duel_count_attr(unsigned int p, int attr);
int duel_count_grave_kind(unsigned int p, int kind);
int duel_count_attack_targets(unsigned int p, int z);
int duel_direct_attack_kind(unsigned int p, int z, int arg);
int duel_count_faceup_skip(unsigned int p, int id, int skip);
int duel_count_free_st_zones(unsigned int p);
int duel_088559C0(DuelAct *a, unsigned int p, int z);
int duel_08857FEC(DuelAct *a, unsigned int p, int z);
int duel_088769A8(DuelAct *a, int b);
int duel_08858704(DuelAct *a, unsigned int p, int z);
int duel_08877ED4(DuelAct *a, int b);
int duel_08877E04(DuelAct *a, int b);
int duel_088734E8(void);
int card_get_atk_q(int id);
int card_attribute_to_group(int id);
int duel_0886DD78(DuelAct *a);
unsigned int duel_field_attrs(unsigned int p, int z);
int duel_bit_count(unsigned int m);
int duel_08875D80(DuelAct *a, int b);
int card_attribute_icon_is3(int id);
int duel_08876904(DuelAct *a);
int duel_0887D4AC(DuelAct *a, int b);
int duel_08876174(DuelAct *a, int b);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
int duel_088750DC(DuelAct *a, int b);

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
    if (b->zone > 10) {
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

int duel_0886F65C(DuelAct *a, int b)
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

int duel_088707BC(DuelAct *a)
{
    unsigned short *src = (unsigned short *)&a->src;

    if (a->kind != 0xF) {
        return 0;
    }
    if (duel_special.player[0] != a->player) {
        return 0;
    }
    if (duel_special.unk8 == 0) {
        return 0;
    }
    if (duel_act_can_target(a, duel_special.player[0], duel_special.zone[0]) == 0) {
        return 0;
    }
    return *src != 0;
}

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_08870848(DuelAct *a)
{
    unsigned int sp = a->src.player;
    unsigned int sz = a->src.zone;

    if (duel.players[sp].field[sz].card.id == 0) {
        return 0;
    }
    if (duel.players[sp].field[sz].faceup == 0) {
        return 0;
    }
    return duel_field_atk(a->src.player, sz) < 1401;
}
#endif

int duel_088708E4(DuelAct *a, int b)
{
    if (duel_list_has_effect(a->uid, a->id)) {
        return 0;
    }
    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    return (int)duel_grave_find_uid(a->player, a->uid) >= 0;
}

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_08870968(DuelAct *a)
{
    unsigned int p;

    if (a->kind != 0xD) {
        return 0;
    }
    if (duel_slot_effect_with(a->player, a->zone, a->id, 0)) {
        return 0;
    }
    p = a->player;
    if (duel_special.player[0] == p && duel_special.zone[0] == a->zone) {
        return 1;
    }
    if (duel_special.player[1] == p) {
        if (duel_special.zone[1] == a->zone) {
            return 1;
        }
        return 0;
    }
    return 0;
}
#endif

int duel_08870A38(DuelAct *a)
{
    if (a->kind == 0x23) {
        switch (a->id) {
        case 0x14E6:
        case 0x16A0:
        case 0x18CA:
            return 0;
        }
        return 1;
    }
    return 1;
}

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_08870A94(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel_special.player[0] == p && duel_special.zone[0] == a->zone) {
        return (short)duel_field_type(duel_special.player[1], duel_special.zone[1]) == 0xF;
    }
    if (duel_special.player[1] == p) {
        if (duel_special.zone[1] == a->zone) {
            return (short)duel_field_type(duel_special.player[0], duel_special.zone[0]) == 0xF;
        }
        return 0;
    }
    return 0;
}
#endif

int duel_08870B4C(DuelAct *a)
{
    return !(a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card));
}

int duel_08870BC4(DuelAct *a, int b)
{
    int r;

    if (a->kind == 0x16) {
        r = 1;
    } else if (a->kind == 2) {
        if (duel_0886CE38(a, b) == 0) {
            r = 0;
        } else {
            r = (int)duel_grave_find_uid(a->player, a->uid) >= 0;
        }
    } else {
        r = 0;
    }
    return r;
}

int duel_08870C40(DuelAct *a)
{
    return duel_phase == 2;
}

#ifdef NON_MATCHING /* branch layout and registers differ */
/* Two or more spells in the hand besides a and a usable monster among them. */
int duel_08870C54(DuelAct *a, int b)
{
    int spells = 0;
    int usable = 0;
    unsigned int p;
    unsigned int i;
    int id;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    p = a->player;
    for (i = 0; i < duel.players[p].n_hand; i++) {
        if (a->uid != CARD_UID(duel.players[p].hand[i])) {
            id = duel.players[p].hand[i].id;
            if (card_is_monster(id) == 0) {
                spells++;
            } else if (duel_card_dfac8(a->player, id, 1)) {
                usable++;
            }
        }
        p = a->player;
    }
    if (usable == 0) {
        return 0;
    }
    return spells > 1;
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_08870DC4(DuelAct *a)
{
    switch (a->kind) {
    case 0x12:
    case 0x13:
    case 0x14:
        break;
    default:
        return 0;
    }
    if (duel.players[duel_special.player[0] & 1].field[duel_special.zone[0]].card.id == 0 ||
        duel.players[duel_special.player[0] & 1].field[duel_special.zone[0]].faceup == 0 ||
        duel_act_can_target(a, duel_special.player[0], duel_special.zone[0]) == 0) {
        return 0;
    }
    return 1;
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_08870E94(DuelAct *a)
{
    if (a->kind == 0x23) {
        return 1;
    }
    if (duel_hand_can_discard(a->player, 1, a->uid, 1, 0) != 0) {
        return 1;
    }
    return 0;
}
#endif

int duel_08870EF8(DuelAct *a, int b)
{
    return duel_is_cur_card(0x10F4, b);
}

int duel_08870F00(DuelAct *a)
{
    if (a->kind != 0x12) {
        return 0;
    }
    if (a->idx == 0) {
        return 0;
    }
    return duel_special.player[0] != a->player;
}

int duel_08870F4C(DuelAct *a)
{
    switch (a->kind) {
    case 6:
    case 7:
    case 8:
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_08870F90(DuelAct *a, int b)
{
    int r;

    if (a->kind == 0x23) {
        if (duel_0886CD1C(a, a->player) < 2) {
            r = 0;
        } else if (duel_phase_ok2(a->player) != 0) {
            r = 2;
        } else {
            r = 0;
        }
    } else {
        r = duel_0886CE38(a, b);
    }
    return r;
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_08871008(DuelAct *a, DuelAct *b)
{
    unsigned int p;

    if (b != 0 && b->id == a->id) {
        return 0;
    }
    if ((unsigned int)(a->kind - 0x12) >= 2) {
        return 0;
    }
    p = a->player;
    if (duel_special.player[0] == 1 - p && duel_special.player[1] == p && duel_special.zone[1] < 5 &&
        duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].card.id != 0) {
        return 1;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_088710D8(DuelAct *a)
{
    unsigned int p = a->player;
    int r;

    if (duel.players[(p ^ a->opp) & 1].n_d == 0) {
        r = 0;
    } else if (duel_phase == 0) {
        r = 1;
    } else if (duel_find_effect_card(1 - p, 0x178B, -1) == 0) {
        r = 1;
    } else {
        r = 0;
    }
    return r;
}
#endif

int duel_08871168(DuelAct *a)
{
    if (duel_phase != 2) {
        return 0;
    }
    return !(duel.players[a->player & 1].flags >> 20 & 1);
}

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_088711BC(DuelAct *a, DuelAct *b)
{
    int z;
    unsigned int p;

    if (b != 0 && duel_act_target_is_monster(b)) {
        if (a->player != (b->player ^ b->opp) && card_is_monster(b->id) == 0 &&
            a->player == (unsigned char)duel_act_target(b, 0) && (duel_act_target(b, 0) & 0xFFFF) >> 8 < 5) {
            for (z = 0; z <= 4; z++) {
                if (duel_act_can_target(a, a->player, z) && duel_0886F89C(b, a->player, z)) {
                    return 2;
                }
            }
        }
    }
    if (a->kind != 0x12) {
        return 0;
    }
    if (duel_special.unk8 == 0) {
        p = a->player;
        if (duel_special.player[0] != p && duel_special.player[1] == p) {
            return 1;
        }
    }
    return 0;
}
#endif

int duel_0887132C(DuelAct *a)
{
    if (a->kind == 0x23) {
        return 0;
    }
    if (duel_count_active(0x13F2) >= 1) {
        return 0;
    }
    return duel_phase_ok2(a->player) != 0;
}

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_0887138C(DuelAct *a)
{
    unsigned int t;
    unsigned int tp;
    unsigned int ti;

    if (a->b3_4 != 2) {
        return 0;
    }
    t = duel_card_locate((unsigned int)a->idx >> 1);
    tp = t & 0xFF;
    ti = t >> 16 & 0xFFFF;
    if ((unsigned char)((t & 0xFFFF) >> 8) != 0xF) {
        return 0;
    }
    if (*(unsigned char *)&duel.players[tp & 1].marks[ti] == 0x40) {
        return 1;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_08871424(DuelAct *a)
{
    unsigned int sp;
    unsigned int sz;

    if ((unsigned int)(a->kind - 8) > 1) {
        return 0;
    }
    sp = a->src.player;
    sz = a->src.zone;
    if (!a->y.y15_6) {
        return 0;
    }
    if (a->y.y15_7) {
        return 0;
    }
    if (sp == a->player) {
        return 0;
    }
    if (duel.players[sp & 1].field[sz].card.id == 0) {
        return 0;
    }
    if (duel.players[sp & 1].field[sz].pos == 0) {
        return 0;
    }
    if (duel.players[sp & 1].field[sz].faceup != 0) {
        return 0;
    }
    if (duel_act_can_target(a, sp, sz) != 0) {
        return 2;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_0887153C(DuelAct *a, int b)
{
    int uid = duel_slot_effect_arg(a->player, 11, 0x1415);
    unsigned int t;
    unsigned int tz;
    DuelCard *c;

    if (duel_0886CE38(a, b) != 0 && uid >= 0) {
        t = duel_card_locate(uid);
        tz = (t & 0xFFFF) >> 8;
        c = duel_field_card_deb2c(t & 0xFF, tz, t >> 16);
        if (tz == 0xE && card_is_monster(c->id) && duel_grave_card_usable(a->player, t & 0xFF, t >> 16)) {
            if (card_is_attribute6(c->id) == 0) {
                return 1;
            }
            if (card_attribute_icon_in_2_3(c->id) == 0 || duel_has_st_12be(a->player)) {
                return 1;
            }
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* branch layout and registers differ */
/* Every other card in the hand belongs to a's player. */
int duel_08871674(DuelAct *a)
{
    unsigned int p;
    unsigned int i;

    if (duel_count_monsters(a->player) == 0) {
        return 0;
    }
    p = a->player;
    for (i = 0; i < duel.players[p].n_hand; i++) {
        p = a->player;
        if (a->uid != CARD_UID(duel.players[p].hand[i]) && p != duel_field_card_deb2c(p, 0xB, i)->owner) {
            return 0;
        }
    }
    return 1;
}
#endif

int duel_088717B4(DuelAct *a)
{
    if (duel_0886CB44(a->player, 1, a->uid, 0, 1)) {
        return 2;
    }
    return a->b3_4 == 0;
}

int duel_08871818(DuelAct *a)
{
    if (a->kind != 0x23 && duel_card_check_9148(a->player, 0xFC9) > 0) {
        return 1;
    }
    return 0;
}

int duel_08871868(DuelAct *a)
{
    if (duel_0884E948(a, 1 - a->player) == 0) {
        return 0;
    }
    if (duel_0884E948(a, a->player) != 0) {
        return 2;
    }
    return 0;
}

int duel_088718C8(DuelAct *a)
{
    int p; int z; int n = 0;

    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 4; z++) {
            if (duel_zone_free(p, z)) {
                n++;
            }
        }
    }
    return n >= 2;
}

int duel_08871940(DuelAct *a)
{
    if (duel_card_check_9148(a->player, 0x1433) || duel_card_check_9148(a->player, 0x1434)) {
        if (duel.players[a->player & 1].flags >> 17 & 1) {
            return 0;
        }
        return !duel_slot_has_effect(a->player, 11, 0x12F3);
    }
    return 0;
}

#ifdef NON_MATCHING /* branch layout and registers differ */
/* Some face-up monster of a's player belongs to the opponent. */
int duel_088719DC(DuelAct *a)
{
    unsigned int p = a->player;
    int z;

    for (z = 0; z <= 4; z++) {
        if (duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup != 0 && p != duel.players[p & 1].field[z].card.owner) {
            return 1;
        }
    }
    return 0;
}
#endif

int duel_08871A68(DuelAct *a)
{
    if (a->kind != 0x23) {
        if (duel_card_check_9148(a->player, 0x1532) == 0) {
            return 0;
        }
        if (duel_card_check_9148(a->player, 0x1686) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_08871ADC(DuelAct *a)
{
    if (duel_list_counter(a->uid, a->id) < 3) {
        return 2;
    }
    return 1;
}

#ifdef NON_MATCHING /* branch layout and registers differ */
int duel_08871B14(DuelAct *a, int b)
{
    unsigned int p;

    if (a->b3_4 != 2) {
        p = a->player;
        if (duel_tributable(p ^ a->opp, p, a->zone) != 0) {
            return 1;
        }
        return 0;
    }
    if (b != 0) {
        return 0;
    }
    if (duel_turn_player != a->player) {
        return 0;
    }
    if (duel.mode & 1) {
        return 0;
    }
    if (duel_slot_has_effect(a->player, 11, a->id) == 0) {
        return 0;
    }
    if (duel_088BFFD8(a->player, a->id, 0) != 0) {
        return 1;
    }
    return 0;
}
#endif

int duel_08871BFC(DuelAct *a)
{
    return duel_count_tributable_except(a->player, -1) > 1;
}

int duel_08871C28(DuelAct *a)
{
    return (unsigned int)duel.players[a->player & 1].n_d > 2;
}

int duel_08871C5C(DuelAct *a)
{
    return duel_0886CD24(a, a->player) >= 2;
}

int duel_08871C84(DuelAct *a)
{
    return !(duel.players[a->player & 1].field[a->zone].pos != 0);
}

int duel_08871CD0(DuelAct *a)
{
    return duel.players[a->player & 1].field[a->zone].pos != 0;
}

int duel_08871D18(DuelAct *a)
{
    if (a->kind == 0x23) {
        return 1;
    }
    return duel.players[1 - a->player & 1].lp > 3000;
}

int duel_08871D74(DuelAct *a)
{
    if (duel.players[1 - a->player & 1].n_hand != 0 && duel_0886CD24(a, a->player) > 0) {
        return 1;
    }
    return 0;
}

int duel_08871DDC(DuelAct *a)
{
    if (a->b3_4 != 2) {
        return 0;
    }
    if (duel_pick_st_zone(a->player) < 0) {
        return 0;
    }
    return (int)duel_grave_find_uid(a->player, a->uid) >= 0;
}

int duel_08871E50(DuelAct *a)
{
    return duel_pick_st_zone(a->player) >= 0;
}

int duel_08871E78(DuelAct *a)
{
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    return duel_slot_efb84(a->player, a->zone) != 0;
}

int duel_08871EC8(DuelAct *a)
{
    if (duel_phase != 2 && duel_phase != 4) {
        return 0;
    }
    return duel_08875530(a);
}

int duel_08871F0C(DuelAct *a)
{
    if (a->kind != 0x23 && duel_phase != 2) {
        return 0;
    }
    return duel_count_monsters_pos(1 - a->player, 1, 0) > 0;
}

#ifdef NON_MATCHING /* a beq with an empty delay slot where mwcc emits beql */
int duel_08871F70(DuelAct *a, int b)
{
    int r;

    if (a->kind == 0x14) {
        if (duel_special.player[0] != duel_special.player[1] && duel.players[a->player & 1].field[a->zone].pos != 0) {
            r = 1;
        } else {
            r = 0;
        }
    } else {
        r = 0;
    }
    return r;
}
#endif

int duel_08871FFC(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_hand == 0) {
        return 0;
    }
    return duel_locks_ok(p, 1) != 0;
}

int duel_08872054(DuelAct *a)
{
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    return duel_088BFFD8(a->player, a->id, 0) != 0;
}

int duel_088720A4(DuelAct *a, int b)
{
    if (duel_088DFE24(a->player, a->player) == 0) {
        return 0;
    }
    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    if (duel_slot_has_effect(a->player, 11, 0x14A0)) {
        return duel_count_monsters(a->player) > 1;
    }
    return duel_088BFFD8(a->player, a->id, 0) > 0;
}

int duel_0887214C(DuelAct *a)
{
    return duel_088BFFD8(a->player, a->id, 0) > 1;
}

/* Some spell/trap on the field could go to a zone of either player. */
int duel_0887217C(void)
{
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 5; z < 10; z++) {
            if (duel_card_fits_zone(p, z, duel.players[p & 1].field[z].card.id)) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_0887221C(DuelAct *a)
{
    int n;
    int p;
    int z;

    if (duel_088DFE24(a->player, a->player)) {
        if (duel_slot_has_effect(a->player, 11, 0x14A0)) {
            n = duel_count_monsters(a->player);
        } else {
            n = duel_grave_count_monsters(a->player);
        }
        if (n != 0) {
            for (p = 0; p < 2; p++) {
                for (z = 0; z < 5; z++) {
                    int lv = duel_field_level(p, z);

                    if (duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup != 0 && lv != 0 && lv <= n) {
                        return 2;
                    }
                }
            }
        }
    }
    return a->b3_4 == 3 ? 0 : 3;
}

int duel_08872360(DuelAct *a)
{
    return duel_grave_count_monsters(a->player) > 0;
}

int duel_08872384(DuelAct *a)
{
    unsigned short *src = (unsigned short *)&a->src;
    int q = 1 - duel_turn_player;
    DuelSpecAct *s = &duel_special.act[q];

    if (a->kind == 0xD && a->player == q && s->unk8 != 0 && src[0] < src[1] &&
        duel_act_immune(a, duel_special.player[0], duel_special.zone[0]) == 0) {
        return 1;
    }
    return 0;
}

int duel_08872434(DuelAct *a)
{
    if (duel_locks_ok(a->player, 1) == 0) {
        return 0;
    }
    if (a->kind != 0x23) {
        if (duel_088BFFD8(a->player, a->id, 0) <= 4) {
            return 0;
        }
    } else {
        if (duel_088BFFD8(a->player, a->id, 0) <= 2) {
            return 0;
        }
    }
    return 1;
}

int duel_088724D8(DuelAct *a, int b)
{
    if (duel_count_monsters_fn(a->player, card_is_in_group_160f) != 0) {
        return duel_0886F65C(a, b);
    }
    return 0;
}

int duel_0887252C(DuelAct *a, int b)
{
    if (duel_0886CE90(a, b) == 0) {
        return 0;
    }
    return duel_0886CDC4(a, b) != 0;
}

int duel_08872578(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[a->zone].card)) {
        return 0;
    }
    return duel_slot_e1854(p, z) != 0xFFFF;
}

/* A face-up monster of group 0x160F is a target of chain link b. */
int duel_0887261C(DuelAct *a, DuelAct *b)
{
    int i;

    if (b == 0) {
        return 0;
    }
    duel_w_089147E8[duel_w_089147E4 & 0x7F] = 0;
    if (duel_0886CF1C(a, (int)b) == 0) {
        return 0;
    }
    for (i = 0; i < b->n_targets; i++) {
        int t = duel_act_target(b, i);
        int z = (unsigned short)t >> 8 & 0xFF;

        if (z < 5) {
            int p = t & 0xFF;

            if (duel.players[p & 1].field[z].faceup != 0 && card_is_in_group_160f(field_get_card(p, z))) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_08872720(DuelAct *a)
{
    if (a->kind == 0x14) {
        if (duel_special.unk8 != 0) {
            return 0;
        }
        if (duel_special.player[0] == a->player &&
            (unsigned short)duel_field_type(duel_special.player[0], duel_special.zone[0]) == 3) {
            return 1;
        }
        if (duel_special.player[1] == a->player &&
            (unsigned short)duel_field_type(duel_special.player[1], duel_special.zone[1]) == 3) {
            return 1;
        }
    }
    return 0;
}

int duel_088727E4(DuelAct *a)
{
    return duel_slot_has_effect2(a->player, 11, 0x14C0);
}

int duel_088727F8(DuelAct *a, int b)
{
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (duel_0886CFE0(a, b) != 0) {
        return 2;
    }
    return 0;
}

int duel_08872858(DuelAct *a)
{
    if (duel_count_type(a->player, 3) != 0) {
        unsigned int p = a->player;

        if (duel.players[p & 1].lp > 500) {
            if (duel_088DFE24(p, 1 - p) && duel_088BFFD8(a->player, a->id, 0)) {
                return 2;
            }
            if (duel_088DFE24(a->player, a->player) && duel_088BFFD8(1 - a->player, a->id, 0)) {
                return 2;
            }
        }
    }
    return a->b3_4 == 0;
}

int duel_08872954(DuelAct *a)
{
    unsigned int p = a->player;

    return duel_tributable(p ^ a->opp, p, a->zone) != 0;
}

/* The two special cards and a's zone share the same link value. */
int duel_08872990(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;
    unsigned int link = duel.players[p & 1].field[z].link;
    unsigned int t;

    if (a->kind != 0xD) {
        return 1;
    }
    if (duel_special.player[0] == p && duel_special.zone[0] == z) {
        t = duel_field_type(duel_special.player[1], duel_special.zone[1]) & 0xFFFF;
        return t != 0 && t == link;
    }
    if (duel_special.player[1] == p && duel_special.zone[1] == z) {
        t = duel_field_type(duel_special.player[0], duel_special.zone[0]) & 0xFFFF;
        return t != 0 && t == link;
    }
    return 0;
}

int duel_08872AB8(void)
{
    return (unsigned int)duel_phase < 4;
}

int duel_08872AC8(DuelAct *a, int b)
{
    unsigned int p;

    if ((unsigned int)(a->kind - 0x12) < 2) {
        p = a->player;
        if (duel_turn_player != p) {
            return 0;
        }
        if (duel_special.player[0] != p || duel_special.zone[0] != a->zone) {
            if (duel_special.player[1] != p || duel_special.zone[1] != a->zone) {
                return 0;
            }
        }
        return duel_088770C0(a, b);
    }
    return 0;
}

int duel_08872B7C(DuelAct *a, int b)
{
    if (duel_phase != 2) {
        return 0;
    }
    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    return duel_0886CDC4(a, b) != 0;
}

/* b: a spell of type 0x17 aimed at one face-up monster of type 1. */
int duel_08872BE4(DuelAct *a, DuelAct *b)
{
    if (duel_0886CDC4(a, (int)b)) {
        if (b != 0 && b->n_targets == 1 && card_is_in_group_147e(b->id) == 0 && card_get_type(b->id) == 0x17) {
            int t = duel_act_target(b, 0);
            int p = t & 0xFF;
            int z = (unsigned short)t >> 8 & 0xFF;

            if (z < 5 && duel.players[p & 1].field[z].faceup != 0 && (unsigned short)duel_field_type(p, z) == 1) {
                return 2;
            }
        }
    }
    return a->b3_4 == 0;
}

int duel_08872CE8(DuelAct *a)
{
    a->w[1] = 1;
    if (duel_0884E5E0(a) == 0) {
        return 0;
    }
    a->w[1] = 0;
    if (duel_0884E5E0(a) != 0) {
        return 2;
    }
    return 0;
}

int duel_08872D3C(DuelAct *a)
{
    if (duel_08872D64(a) != 0) {
        return 2;
    }
    return 0;
}

int duel_08872D64(DuelAct *a)
{
    if (a->kind == 0x23) {
        return 1;
    }
    return duel_count_type(a->player, 1) > 0;
}

int duel_08872DA8(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_d == 0) {
        return 0;
    }
    if (duel.players[(1 - p) & 1].n_hand != 0 && duel_0886CD24(a, p) > 0) {
        return 1;
    }
    return 0;
}

int duel_08872E38(DuelAct *a)
{
    if (a->kind != 0x23) {
        if (duel_hand_can_discard(a->player, duel_08861360(a), a->uid, 0, 1) == 0) {
            return 0;
        }
    }
    return duel_088BFFD8(a->player, a->id, 0) > 0;
}

int duel_08872EC0(DuelAct *a)
{
    unsigned int id;

    if (a->kind != 0x12) {
        return 0;
    }
    if (a->idx == 0) {
        return 0;
    }
    if (duel_special.player[0] != 1 - a->player) {
        return 0;
    }
    id = duel.players[duel_special.player[0] & 1].field[duel_special.zone[0]].card.id;
    if (id == 0) {
        return 0;
    }
    if (card_attribute_icon_in_2_3(id) == 0 && card_is_attribute9(id) == 0 && duel.players[duel_special.player[0] & 1].field[duel_special.zone[0]].card.b14) {
        return 1;
    }
    return 0;
}

int duel_08872FF4(DuelAct *a)
{
    if (a->b3_4 == 1 && a->kind == 5) {
        return 2;
    }
    switch (a->id) {
    case 0x1506:
        return duel_0887307C(a);
    case 0x1694:
        return duel_08873730(a);
    }
    return 1;
}

int duel_0887307C(DuelAct *a)
{
    if (a->kind == 0xF) {
        return a->y.y16_5 != a->player;
    }
    return 0;
}

int duel_088730B8(DuelAct *a)
{
    unsigned int p;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    p = a->player;
    if (duel.players[p & 1].field[a->zone].flags >> 18 & 1) {
        return 0;
    }
    return duel_088BFFD8(p, a->id, 0) > 0;
}

int duel_08873150(void)
{
    int i;

    for (i = 0; i < 2; i++) {
        if (duel_count_field(i) + duel.players[i & 1].n_hand >= 6) {
            return 1;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* case order and registers differ */
int duel_088731C0(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        unsigned int k = a->kind;

        if (k == 0x21) {
            if (duel.players[a->src.player].field[a->src.zone].card.id == 0) {
                return 0;
            }
            return 2;
        }
        if (k == 0x20) {
            return 0;
        }
        if (k == 0x1F) {
            return 0;
        }
    } else if (card_is_in_group_147e(b->id) == 0) {
        return 2;
    }
    return a->b3_4 != 3;
}
#endif

int duel_088732B4(DuelAct *a)
{
    return duel_count_monsters(1 - a->player) != 0;
}

int duel_088732E0(DuelAct *a, DuelAct *b)
{
    if (b != 0 && b->id != a->id) {
        return 0;
    }
    if (a->kind == 0xE) {
        unsigned short *src = (unsigned short *)&a->src;
        unsigned int p = a->player;
        if (a->y.y16_0 != 1 - p) {
            return 0;
        }
        if (a->y.y16_5 != p) {
            return 0;
        }
        return src[0] >= 2000;
    }
    return 0;
}

#ifdef NON_MATCHING /* branch layout differs */
int duel_08873368(DuelAct *a)
{
    if (duel_turn_player != 1 - a->player) {
        return 0;
    }
    if (a->kind == 0x19) {
        return !a->y.y15_2;
    }
    return 0;
}
#endif

int duel_088733C0(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int q = 1 - p;

    if (duel.players[q & 1].n_grave == 0) {
        return 0;
    }
    return duel_088DFE24(p, q) != 0;
}

int duel_08873420(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].lp > 1000) {
        return 0;
    }
    if (duel_turn_player == p) {
        return 0;
    }
    return 2;
}

int duel_08873478(DuelAct *a, int b)
{
    int r;
    if (a->kind == 2) {
        if (duel_0886CE38(a, b) == 0) {
            return 0;
        }
        r = (int)duel_grave_find_uid(a->player, a->uid);
        return r >= 0;
    }
    return 0;
}

int duel_088734E8(void)
{
    if (duel_special.unk8 != 0) {
        return 0;
    }
    if (duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].faceup != 0) {
        return 0;
    }
    return duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].pos != 0;
}

int duel_08873568(DuelAct *a, int b)
{
    if (duel_slot_value_1909(a->player, a->zone) < 2) {
        return 0;
    }
    return duel_08874A10(a, b);
}

int duel_088735C4(DuelAct *a)
{
    if (duel_count_active(0x135D) != 0) {
        return 0;
    }
    if (duel_no_1a1b(a->player) == 0) {
        return 0;
    }
    return !duel_slot_has_effect(a->player, a->zone, 0x152A);
}

int duel_08873630(DuelAct *a)
{
    return duel_slot_has_effect(a->player, 11, 0x1846) != 0;
}

int duel_0887365C(DuelAct *a)
{
    return duel.players[(1 - a->player) & 1].n_d != 0;
}

int duel_08873694(DuelAct *a, int b)
{
    if (duel_088DFE24(a->player, 1 - a->player) == 0) {
        return 0;
    }
    if (duel_088BFFD8(a->player, a->id, 0) == 0) {
        return 0;
    }
    if (duel_088BFFD8(a->player, a->id, 1) != 0) {
        return duel_0886CE38(a, b);
    }
    return 0;
}

int duel_08873730(DuelAct *a)
{
    if (duel_count_active(0x135D) != 0) {
        return 0;
    }
    return duel_no_1a1b(a->player) != 0;
}

int duel_08873774(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel_turn_player != p) {
        return duel.players[p & 1].n_hand < 4;
    }
    return duel_slot_has_effect(p, a->zone, a->id);
}

int duel_088737D8(DuelAct *a, int b)
{
    unsigned int i;
    unsigned int p;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    if (duel_count_monsters(a->player) != 0) {
        return 0;
    }
    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    for (i = 0; i < duel.players[(p = a->player) & 1].n_hand; i++) {
        if (!(!duel_088BFFD8(p, a->id, i))) {
            return 1;
        }
    }
    return 0;
}

int duel_088738D0(DuelAct *a, int b)
{
    if (duel_0886CD24(a, a->player) != 0) {
        return duel_0886F65C(a, b);
    }
    return 0;
}

int duel_0887391C(DuelAct *a)
{
    return duel_count_monsters(1 - a->player) + duel_count_monsters(a->player) > 3;
}

int duel_0887396C(DuelAct *a)
{
    return duel.players[(1 - a->player) & 1].n_hand >= 4;
}

int duel_088739A8(DuelAct *a)
{
    if (duel_slot_has_effect(a->player, a->zone, a->id) != 0) {
        return 0;
    }
    if (duel_special.unk8 != 0) {
        return 0;
    }
    return duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].pos != 0;
}

int duel_08873A38(DuelAct *a, DuelAct *b)
{
    unsigned int p;
    int z;

    if (b != 0 && b->id == a->id) {
        return 0;
    }
    if (a->kind == 0x12 && a->idx != 0) {
        p = duel_special.player[1];
        z = duel_special.zone[1];
        if (p == a->player && duel_special.unk8 == 0 &&
            card_is_attribute6(duel.players[p & 1].field[z].card.id) &&
            duel.players[p & 1].field[z].faceup != 0 && duel_field_level(p, z) < 5 &&
            duel_count_e17a0(p, 0x1318) == 0) {
            return 2;
        }
    }
    return a->b3_4 == 0;
}

#ifdef NON_MATCHING /* loop induction variables differ */
int duel_08873B8C(DuelAct *a)
{
    unsigned int p = a->player;
    DuelPlayer *pl = &duel.players[p & 1];
    int i;

    if ((unsigned int)pl->n_d <= 1) {
        return 0;
    }
    if (a->kind != 0x23) {
        for (i = 0; i <= 10; i++) {
            if (duel.players[a->player].field[i].card.id != 0 && a->uid != CARD_UID(duel.players[a->player].field[i].card)) {
                return duel_slot_efacc(p, i, 0) != 0;
            }
        }
        return duel_0886CD24(a, p) != 0;
    }
    return 1;
}
#endif

int duel_08873C84(DuelAct *a)
{
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (duel_count_free_monster_zones(a->player) < 2) {
        return 0;
    }
    return duel_088BFFD8(a->player, a->id, 0) > 1;
}

int duel_08873CF4(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    return (duel_act_e4168(b) & 2) > 0;
}

int duel_08873D44(DuelAct *a)
{
    int i = duel_grave_find_uid(a->player, a->uid);
    unsigned int p;

    if (i < 0) {
        return 0;
    }
    p = a->player;
    if (!duel.players[p & 1].grave[i].b17) {
        return 0;
    }
    if (duel_slot_effect_with(p, 11, a->id, a->uid) != 0) {
        return 0;
    }
    return duel.players[(1 - a->player) & 1].n_hand != 0;
}

int duel_08873E24(DuelAct *a, int b)
{
    unsigned int i;
    unsigned int p;

    if ((unsigned int)(a->kind - 6) < 2) {
        if (a->src.player == a->player) {
            return 0;
        }
        if (duel_0886CE38(a, b) == 0) {
            return 0;
        }
        for (i = 0; i < duel.players[(p = a->player) & 1].n_hand; i++) {
            if (!(!duel_088BFFD8(p, a->id, i))) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_08873F28(DuelAct *a, int b)
{
    if (duel_0886F65C(a, b) == 0) {
        return 0;
    }
    return duel_0886CF1C(a, b) != 0;
}

int duel_08873F74(DuelAct *a, int b)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_d != 0) {
        return duel_0886CE38(a, b);
    }
    return 0;
}

int duel_08873FC0(DuelAct *a, DuelAct *b)
{
    if (duel_088DFCC8(a->uid) == 0) {
        return 0;
    }
    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    if (card_is_monster(b->id) == 0 && b->b3_4 != 0) {
        return 0;
    }
    return (duel_act_e403c(b) & 1) > 0;
}

int duel_08874078(void)
{
    return duel_088E9150(0x159D) != 0;
}

/* a: a card of the opponent sent from the field to the graveyard (zone 0xE). */
int duel_08874098(DuelAct *a)
{
    struct DuelActX *x;
    DuelCard *c;
    int r;
    unsigned int p;

    if (a->kind == 0x16) {
        x = &a->x;
        c = duel_card_by_uid(x->s_uid);
        if (c == 0) {
            return 0;
        }
        p = a->player;
        if (x->s_player == p) {
            return 0;
        }
        if (c->owner != 1 - p) {
            return 0;
        }
        if (x->s_zone2 != 0xE) {
            return 0;
        }
        r = card_is_monster(c->id);
        return r != 0;
    }
    return 0;
}

int duel_08874164(DuelAct *a)
{
    if (duel_08874098(a) != 0) {
        struct DuelActX *x = &a->x;

        if (a->x.s_b17 == 0) {
            return 0;
        }
        return !x->s_b14;
    }
    return 0;
}

int duel_088741C0(DuelAct *a, int b)
{
    unsigned int p;

    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    p = a->player;
    if (duel.players[p & 1].n_d != 0) {
        return 1;
    }
    return duel.players[(1 - p) & 1].n_d != 0;
}

int duel_08874244(DuelAct *a)
{
    unsigned int p = a->player;

    return duel.players[p & 1].n_d != 0;
}

int duel_08874274(DuelAct *a)
{
    unsigned int p;

    if (a->kind != 0x23 && duel_card_check_9148(a->player, 0x159D) == 0) {
        return 0;
    }
    p = a->player;
    if (duel.players[(1 - p) & 1].n_hand != 0) {
        return 1;
    }
    return duel_hand_count_monsters(p) != 0;
}

int duel_08874314(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_d == 0) {
        return 0;
    }
    if (a->kind != 0x23) {
        if ((duel_0886CD24(a, p) != 0 || duel_count_field(a->player) != 0) && duel_locks_ok(a->player, 1) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_088743C8(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (card_is_monster(b->id) == 0) {
        if (b->b3_4 != 0) {
            return 0;
        }
        if (card_get_prop5_hi(b->id) == 4) {
            return 0;
        }
    }
    if (a->id == 0x15A7 && duel.players[(1 - a->player) & 1].n_hand == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return (duel_act_e4168(b) & 1) > 0;
    }
    return (duel_act_e4168(b) & 2) > 0;
}

int duel_088744D4(DuelAct *a, int b)
{
    if (duel_turn_player != a->player) {
        return 0;
    }
    if (duel_phase != 1) {
        return 0;
    }
    return duel_0886CDC4(a, b);
}

int duel_08874528(DuelAct *a, int b)
{
    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    if (a->zone < 5) {
        return duel_pick_st_zone(a->player) >= 0;
    }
    if (duel_0886CE38(a, b) != 0) {
        return 2;
    }
    return 0;
}

#ifdef NON_MATCHING /* registers differ */
int duel_088745A8(DuelAct *a, int b)
{
    unsigned int p;
    int r;
    struct DuelActX *x;
    int id;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    p = a->player;
    if (duel.players[p & 1].n_d == 0) {
        return 0;
    }
    if (a->kind == 0x1B) {
        x = &a->x;
        if (a->src.player == p && a->src.zone < 5 && !x->s_b23 && a->x.s_b15 && x->s_zone2 == 0xE) {
            id = duel.list[x->s_uid].id;
            if (card_is_attribute9(id) != 0) {
                return 0;
            }
            r = card_is_monster(id);
            return r != 0;
        }
        return 0;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* branch layout differs */
int duel_088746D0(DuelAct *a)
{
    int uid;

    if (a->b3_4 != 2) {
        return 0;
    }
    uid = (unsigned int)a->idx >> 3;
    if (((duel_card_locate(uid) & 0xFFFF) >> 8 & 0xFF) == 0xF) {
        if (uid == a->uid) {
            if (a->id != duel.list[uid].id) {
            return 0;
        }
        return 1;
        }
        return 1;
    }
    return 0;
}
#endif

int duel_0887477C(DuelAct *a, int b)
{
    if (duel_088DFE24(a->player, a->player) == 0) {
        return 0;
    }
    if (duel_088BFFD8(a->player, a->id, 0) < 2) {
        return 0;
    }
    return duel_0886CE38(a, b);
}

int duel_088747F4(DuelAct *a, int b)
{
    if ((unsigned int)(a->kind - 6) < 2) {
        if (a->src.player == a->player) {
            return 0;
        }
        return duel_0886CE38(a, b);
    }
    return 0;
}

int duel_08874850(DuelAct *a, int b)
{
    if (a->zone >= 5 && duel_phase == 2 && duel_count_active(0x13F2) == 0) {
        return 2;
    }
    return duel_08874528(a, b);
}

int duel_088748C8(DuelAct *a, int b)
{
    if (a->kind == 0xF) {
        return 1;
    }
    return duel_0886DE0C(a, b);
}

int duel_08874904(DuelAct *a, int b)
{
    int z = a->zone;

    if (z >= 5 && duel_slot_has_effect(a->player, z, 0x15D3) == 0) {
        return 2;
    }
    return duel_08874528(a, b);
}

int duel_0887496C(DuelAct *a)
{
    if (duel_slot_src_attr8(a->player, a->zone, 0) != 0xFFFF) {
        return 0;
    }
    return duel_pick_st_zone(a->player) >= 0;
}

int duel_088749CC(DuelAct *a)
{
    if (a->kind == 0x16) {
        return a->src.player == 1 - a->player;
    }
    return 0;
}

int duel_08874A10(DuelAct *a, int b)
{
    unsigned int q = a->player ^ a->opp;

    if (duel_phase_ok2(q) == 0) {
        return 0;
    }
    if (duel.players[a->player & 1].n_d == 0) {
        if (duel_088BFFD8(a->player, a->id, duel_088AA224(a)) == 0) {
            return 0;
        }
    }
    if (duel_slot_has_effect(a->player, 11, 0xFA7) != 0) {
        return 0;
    }
    if (duel_tributable(q, a->player, a->zone) == 0) {
        return 0;
    }
    if (duel_0886CD1C(a, q) != 0 || (a->player == q && duel_zone_usable(a->player, a->zone) != 0)) {
        return 1;
    }
    return 0;
}

int duel_08874B4C(DuelAct *a, int b)
{
    if (duel_0886CD90(a, b) != 0) {
        return duel_0886CF1C(a, b);
    }
    return 0;
}

int duel_08874B90(DuelAct *a, int b)
{
    if (duel_phase != 3) {
        return 0;
    }
    return duel_0886CE38(a, b);
}

int duel_08874BC8(DuelAct *a)
{
    unsigned int i;
    unsigned int p;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    for (i = 0; i < duel.players[(p = a->player) & 1].n_hand; i++) {
        if (!(!duel_088BFFD8(p, a->id, i))) {
            return 1;
        }
    }
    return 0;
}

int duel_08874C94(DuelAct *a)
{
    if (duel_card_check_9148(a->player, 0x15CA) == 0) {
        return 0;
    }
    if (duel_card_check_9148(a->player, 0x15CB) == 0) {
        return 0;
    }
    if (duel_card_check_9148(a->player, 0x15CC) == 0) {
        return 0;
    }
    if (duel.players[(1 - a->player) & 1].n_hand != 0) {
        return 2;
    }
    return 1;
}

int duel_08874D48(DuelAct *a)
{
    if (duel_turn_player == a->player) {
        return 0;
    }
    return duel_phase == 1;
}

int duel_08874D7C(DuelAct *a)
{
    if (duel_turn_player != a->player) {
        return 0;
    }
    return (unsigned int)duel_phase < 4;
}

#ifdef NON_MATCHING /* branch layout differs */
int duel_08874DAC(DuelAct *a)
{
    if (duel_count_monsters(1 - a->player) > duel_count_monsters(a->player)) {
        if (a->kind == 6) {
            return a->src.player == a->player;
        }
    }
    return 0;
}
#endif

int duel_08874E3C(DuelAct *a)
{
    return (int)(duel_count_field(1 - a->player) + duel.players[(1 - a->player) & 1].n_hand) > 0;
}

int duel_08874EA8(DuelAct *a, int b)
{
    if (duel_turn_player == a->player) {
        return 0;
    }
    return duel_0886CE38(a, b);
}

int duel_08874EE4(DuelAct *a, int b)
{
    a->w[2] = 1;
    if (duel_pick_st_zone(a->player) >= 0 && duel_0884E5E0(a) != 0) {
        return 2;
    }
    a->w[2] = 2;
    if (duel_0886CE38(a, b) != 0 && duel_0884E5E0(a) != 0) {
        return 2;
    }
    return 0;
}

int duel_08874F7C(DuelAct *a)
{
    if (duel_count_monsters(1 - a->player) == 0) {
        return 0;
    }
    if (duel_phase != 2) {
        return 0;
    }
    return duel_08836868(a->player) != 0;
}

int duel_08874FE4(DuelAct *a)
{
    unsigned int q = a->player ^ a->opp;

    if (duel_phase_ok2(q) == 0) {
        return 0;
    }
    if (duel_slot_e68a0(a->player, a->zone) < duel_088613D8(a)) {
        return 0;
    }
    if (duel_tributable(q, a->player, a->zone) == 0) {
        return 0;
    }
    if (duel_0886CD1C(a, q) != 0 || (a->player == q && duel_zone_usable(a->player, a->zone) != 0)) {
        return 1;
    }
    return 0;
}

int duel_088750DC(DuelAct *a, int b)
{
    return duel_slot_e68a0(a->player, a->zone) >= duel_088613D8(a);
}

int duel_08875124(DuelAct *a, int b)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_d == 0) {
        return 0;
    }
    return duel_088750DC(a, b);
}

int duel_08875178(DuelAct *a, int b)
{
    if (duel_0886CDC4(a, b) != 0) {
        return 2;
    }
    return 0;
}

int duel_088751A0(DuelAct *a)
{
    if (duel_0884E948(a, 1 - a->player) <= 1) {
        return 0;
    }
    if (duel_0884E948(a, a->player) <= 0) {
        return 0;
    }
    return 2;
}

int duel_08875204(DuelAct *a, int b)
{
    if (duel_turn_player == a->player && duel_0886CD90(a, b) != 0 && duel_0884E5E0(a) != 0) {
        return 2;
    }
    if (a->b3_4 == 0) {
        return 3;
    }
    return 0;
}

int duel_0887527C(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_d != 0) {
        return !duel_slot_has_effect(p, 11, a->id);
    }
    return 0;
}

int duel_088752D8(DuelAct *a, int b)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_d == 0) {
        if (duel_088BFFD8(a->player, a->id, duel_088AA224(a)) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_08875354(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    if (card_is_monster(b->id) == 0 && b->b3_4 != 0) {
        return 0;
    }
    return duel_act_e30c0(b);
}

int duel_088753D8(DuelAct *a)
{
    if (a->kind == 0x23) {
        return 1;
    }
    return duel_sum_e68a0(a->player) >= duel_088613D8(a);
}

int duel_08875434(DuelAct *a)
{
    int n = duel_sum_e68a0(a->player);

    return n + duel_sum_e68a0(1 - a->player) > 0;
}

int duel_08875480(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel_turn_player != p) {
        return 0;
    }
    return duel_088BFFD8(p, a->id, 0) >= 2;
}

int duel_088754C8(DuelAct *a, int b)
{
    if (duel_sum_e68a0(a->player) < duel_088613D8(a)) {
        return 0;
    }
    return duel_0886CE90(a, b);
}

int duel_08875530(DuelAct *a)
{
    if (duel_first_free_zone(a->player) < 0) {
        return 0;
    }
    return duel_phase_ok2(a->player) != 0;
}

int duel_08875578(DuelAct *a, int b)
{
    if (duel_0886CDC4(a, b) == 0) {
        return 0;
    }
    return duel_088BFFD8(a->player, a->id, 0) != 0;
}

int duel_088755C0(DuelAct *a)
{
    struct DuelActX *x;
    int r;

    if (a->kind == 0x16) {
        x = &a->x;
        if (x->s_player == a->player) {
            return 0;
        }
        if (!x->s_b14) {
            return 0;
        }
        if (x->s_zone2 != 0xE) {
            return 0;
        }
        r = card_is_monster(duel.list[x->s_uid].id);
        return r != 0;
    }
    return 0;
}

int duel_08875668(DuelAct *a, int b)
{
    if (duel_slot_has_effect(a->player, 11, a->id) != 0) {
        return duel_0886CE38(a, b);
    }
    return 0;
}

int duel_088756BC(DuelAct *a, int b)
{
    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    if (a->kind != 0x23) {
        if (duel_grave_count_id(a->player, 0xFB7) == 0) {
            return 0;
        }
        if (duel_grave_count_id(a->player, 0xFB8) == 0) {
            return 0;
        }
        if (duel_grave_count_id(a->player, 0xFB9) == 0) {
            return 0;
        }
        if (duel_grave_count_id(a->player, 0xFBA) == 0) {
            return 0;
        }
        if (duel_grave_count_id(a->player, 0xFBB) == 0) {
            return 0;
        }
    }
    if (duel_hand_count_name(a->player, 0x1645) == 0) {
        return 0;
    }
    return 1;
}

int duel_088757B0(DuelAct *a)
{
    if (a->kind != 0x23 && duel_hand_can_discard(a->player, duel_08861360(a), a->uid, 0, 0) == 0) {
        return 0;
    }
    return 1;
}

int duel_08875824(DuelAct *a)
{
    if (a->b3_4 != 2) {
        return 0;
    }
    if (((duel_card_locate((int)((unsigned int)a->idx >> 16 & 0xFFFF) >> 3) & 0xFFFF) >> 8 & 0xFF) == 0xF) {
        return 1;
    }
    return ((duel_card_locate((int)((unsigned short *)&a->src)[0] >> 3) & 0xFFFF) >> 8 & 0xFF) == 0xF;
}

int duel_088758A4(DuelAct *a)
{
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    return duel_0886CD1C(a, 1 - a->player) > 2;
}

int duel_088758FC(DuelAct *a)
{
    unsigned int i;
    unsigned int p;

    if (duel_count_type(a->player, 0x12) == 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    for (i = 0; i < duel.players[(p = a->player) & 1].n_hand; i++) {
        if (!(!duel_088BFFD8(p, a->id, i))) {
            return 1;
        }
    }
    return 0;
}

int duel_088759E4(DuelAct *a)
{
    if (duel_grave_count_id(a->player, 0x167A) != 0) {
        return 1;
    }
    return duel_grave_count_id(a->player, 0x167B) != 0;
}

int duel_08875A34(DuelAct *a, int b)
{
    int id;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    if (a->kind != 0x23) {
        switch (a->id) {
        case 0x15B6:
            id = 0x157F;
            break;
        case 0x167E:
            id = 0x129E;
            break;
        case 0x169E:
            id = 0x1532;
            break;
        }
        if (duel_card_check_9148(a->player, id) == 0) {
            return 0;
        }
    }
    return 1;
}

#ifdef NON_MATCHING /* registers and branch layout differ */
int duel_08875AD0(DuelAct *a)
{
    struct DuelActX *x;

    if (duel_088DFCC8(a->uid) != 0) {
        if (a->kind != 0x16 && a->kind != 0x1B) {
            return 0;
        }
        x = &a->x;
        if (a->src.player == a->player && a->src.zone < 5 && !x->s_b23 && a->x.s_b15 && x->s_zone2 == 0xE) {
            return card_same_name(duel.list[x->s_uid].id, 0x1691);
        }
    }
    return 0;
}
#endif

int duel_08875BD4(DuelAct *a)
{
    unsigned int p = a->player;

    return duel.players[p & 1].lp + 2000 <= duel.players[(1 - p) & 1].lp;
}

int duel_08875C28(DuelAct *a)
{
    if (a->id == 0x16A3) {
        if (duel_turn_player != a->player) {
            return 0;
        }
        if ((unsigned int)duel_phase > 3) {
            return 0;
        }
    }
    if (duel_card_check_9148(a->player, 0x1532) == 0) {
        return 0;
    }
    if (duel_card_check_9148(a->player, 0x161E) == 0) {
        return 0;
    }
    if (duel_card_check_9148(a->player, 0x1656) == 0) {
        return 0;
    }
    if (duel_card_check_9148(a->player, 0x1685) == 0) {
        return 0;
    }
    if (duel_card_check_9148(a->player, 0x1686) != 0) {
        return 2;
    }
    return 0;
}

int duel_08875D1C(DuelAct *a)
{
    return !duel_slot_effect_kind(a->player, 11, a->id, 2);
}

int duel_08875D50(DuelAct *a)
{
    return duel_088BFFD8(a->player, a->id, 0) > 1;
}

int duel_08875D80(DuelAct *a, int b)
{
    if (duel_0886CB44(a->player, 1, a->uid, 0, 1) != 0) {
        return duel_0886CD90(a, b);
    }
    return 0;
}

int duel_08875DE4(DuelAct *a)
{
    if (duel_slot_effect_with(a->player, 11, 0x16B8, a->uid) != 0) {
        return 0;
    }
    if (duel_088DFE24(a->player, a->player) == 0) {
        return 0;
    }
    return duel_088BFFD8(a->player, a->id, 0) > 1;
}

int duel_08875E68(DuelAct *a, int b)
{
    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    if (duel_088DFE24(a->player, a->player) == 0) {
        return 0;
    }
    return duel_088BFFD8(a->player, a->id, 0) > 1;
}

int duel_08875ED0(DuelAct *a)
{
    return !duel_slot_effect_with(a->player, 11, a->id, a->uid);
}

int duel_08875F0C(DuelAct *a)
{
    unsigned int i;
    unsigned int p;

    for (i = 0; i < duel.players[(p = a->player) & 1].n_hand; i++) {
        if (!(!duel_088BFFD8(p, a->id, i))) {
            return 1;
        }
    }
    return 0;
}

int duel_08875FC0(DuelAct *a, int b)
{
    unsigned int p;

    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    p = a->player;
    if (duel_088DFE24(p, p) != 0) {
        return duel_0886CFE0(a, b);
    }
    return 0;
}

int duel_08876028(DuelAct *a)
{
    struct DuelActX *x;
    int r;

    if (a->kind == 0x16) {
        x = &a->x;
        if (x->s_player == a->player) {
            return 0;
        }
        if (x->s_zone2 != 0xE) {
            return 0;
        }
        r = card_is_monster(duel.list[x->s_uid].id);
        return r != 0;
    }
    return 0;
}

int duel_088760BC(DuelAct *a)
{
    if (a->kind == 0x16) {
        if (a->src.player == a->player) {
            return 0;
        }
        return 2;
    }
    return 0;
}

int duel_088760FC(DuelAct *a, int b)
{
    unsigned int f = duel.players[a->player & 1].field[a->zone].flags;

    if ((f >> 22 & 1) + (f >> 23 & 1) != 0) {
        return 0;
    }
    return duel_0886CD90(a, b);
}

int duel_08876174(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel_slot_effect_kind(p, z, 0x16CB, 2) != 0) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].flags >> 23 & 1) {
        return 0;
    }
    return duel_0886CD90(a, b);
}

int duel_0887628C(DuelAct *a)
{
    if (a->kind != 0x23) {
        if (duel_card_check_9148(a->player, 0x1681) == 0) {
            return 0;
        }
        if (duel_card_check_9148(a->player, 0x16B3) == 0) {
            return 0;
        }
        if (duel_card_check_9148(a->player, 0x16B4) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_0887631C(DuelAct *a)
{
    unsigned int p;

    if (a->kind == 0x23) {
        return 1;
    }
    p = a->player;
    return duel.players[p & 1].n_f >= 7;
}

int duel_08876370(DuelAct *a)
{
    unsigned int p = a->player;

    if ((unsigned int)duel.players[p & 1].n_d <= 1) {
        return 0;
    }
    if (a->kind == 0x23) {
        return 1;
    }
    return duel.players[p & 1].n_f >= 4 && duel.players[p & 1].n_grave == 0;
}

#ifdef NON_MATCHING /* branch layout differs */
int duel_08876404(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_f <= 1) {
        return 0;
    }
    if (a->kind == 0x23 || (duel_088E9150(0x16CB) != 0 || duel_088E9150(0x16E4) != 0)) {
        return 1;
    }
    return 0;
}
#endif

int duel_0887648C(DuelAct *a, int b)
{
    unsigned int p;

    if (a->kind != 0x23) {
        p = a->player;
        if (duel.players[p & 1].n_grave != 0) {
            return 0;
        }
    }
    return duel_0886CE90(a, b);
}

int duel_088764F8(DuelAct *a)
{
    return duel_0886CD24(a, a->player) > 0;
}

int duel_0887651C(DuelAct *a, DuelAct *b)
{
    int i;

    if (b == 0) {
        return 0;
    }
    if (1 - a->player != (b->player ^ b->opp)) {
        return 0;
    }
    for (i = 0; i < b->n_targets; i++) {
        if (((duel_act_target(b, i) & 0xFFFF) >> 8 & 0xFF) == 0xE &&
            card_is_monster(duel.list[duel_act_target2(b, i)].id) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_08876600(DuelAct *a)
{
    unsigned int n = 1;
    unsigned int p;

    switch (a->id) {
    case 0x14F0:
        n = 0xF;
        break;
    case 0x16DC:
        n = 0x1E;
        break;
    }
    p = a->player;
    return duel.players[p & 1].n_grave >= n;
}

int duel_0887665C(DuelAct *a)
{
    if (duel_count_field(1 - a->player) > 5) {
        return 0;
    }
    return duel_count_field(a->player) < 6;
}

#ifdef NON_MATCHING /* case tests differ */
int duel_088766B0(DuelAct *a)
{
    int i;
    unsigned int j;
    unsigned int p;
    int t;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if ((int)duel_grave_find_uid(a->player, a->uid) < 0) {
        return 0;
    }
    for (i = 0; i < 5; i++) {
        if (duel_slot_efb84(a->player, i) == 0) {
            return 0;
        }
        t = (short)duel_field_type(a->player, i);
        if (t == 0x10) {
            return 1;
        }
        if (t == 0xC || t == 0xB) {
            return 1;
        }
    }
    for (j = 0; j < duel.players[(p = a->player) & 1].n_hand; j++) {
        DuelCard *c = &duel.players[p & 1].hand[j];

        if (duel_088DFCC8(CARD_UID(*c)) == 0) {
            return 0;
        }
        t = card_get_type(c->id);
        if (t == 0x10) {
            return 1;
        }
        if (t == 0xC || t == 0xB) {
            return 1;
        }
    }
    return 0;
}
#endif

int duel_088768A8(DuelAct *a)
{
    if (a->kind == 0x16) {
        return duel_special.act[duel_special.act[1].uid == a->src.uid].pad14[4] == 10;
    }
    return 0;
}

int duel_08876904(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].field[10].faceup == 0) {
        return 0;
    }
    if (duel_slot_efb84(p, 10) != 0) {
        return card_same_name(duel.players[a->player & 1].field[10].card.id, 0x10F4);
    }
    return 0;
}

int duel_088769A8(DuelAct *a, int b)
{
    struct DuelActX *x;
    int r;

    if (a->kind == 0x16) {
        x = &a->x;
        if (x->s_zone2 != 0xE) {
            return 0;
        }
        r = card_is_monster(duel.list[x->s_uid].id);
        return r != 0;
    }
    return 0;
}

int duel_08876A1C(DuelAct *a)
{
    int i;

    for (i = 0; i < 2; i++) {
        if (duel_0886CD1C(a, i) != 0 && duel_phase_ok2(i) != 0 && duel_088BFFD8(i, a->id, 0) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_08876AA0(DuelAct *a, int b)
{
    if (duel_phase_ok2(a->player) != 0) {
        return duel_088752D8(a, b);
    }
    return 0;
}

#ifdef NON_MATCHING /* loop and save/restore layout differ */
int duel_08876AEC(DuelAct *a, DuelAct *b)
{
    int p;
    int z;
    unsigned short save;

    if (b != 0 && duel_act_target_flag9(b) != 0 && 1 - a->player == (b->player ^ b->opp) &&
        card_get_type(b->id) == 0x16 && b->b3_4 == 0) {
        save = duel.players[a->player].field[a->zone].faceup;
        duel.players[a->player].field[a->zone].faceup = 1;
        for (p = 0; p < 2; p++) {
            for (z = 0; z <= 10; z++) {
                if ((z >= 5 || duel_field_flag9(p, z) != 0) && duel_act_can_target(a, p, z) != 0 &&
                    duel_0886F89C(b, p, z) != 0) {
                    duel.players[a->player].field[a->zone].faceup = save;
                    return 1;
                }
            }
        }
        duel.players[a->player].field[a->zone].faceup = save;
    }
    return 0;
}
#endif

int duel_08876CF0(DuelAct *a, int b)
{
    unsigned int i;

    if (a->kind != 0x12) {
        return 0;
    }
    if (a->idx == 0) {
        return 0;
    }
    if (duel_special.player[0] == a->player) {
        return 0;
    }
    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    for (i = 0; i < duel.players[a->player & 1].n_hand; i++) {
        int id = duel.players[a->player & 1].hand[i].id;

        if (card_is_monster(id) != 0 && duel_card_dfac8(a->player, id, 0) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_08876E44(DuelAct *a)
{
    unsigned int p = a->player;

    return duel.players[p & 1].lp + 7000 <= duel.players[(1 - p) & 1].lp;
}

int duel_08876E98(DuelAct *a, int b)
{
    if (duel_0886CD90(a, b) != 0) {
        return duel_088770C0(a, b);
    }
    return 0;
}

int duel_08876EDC(DuelAct *a)
{
    unsigned int p;

    if (duel_phase != 2) {
        return 0;
    }
    p = a->player;
    if (duel_tributable(p, p, a->zone) == 0) {
        return 0;
    }
    p = a->player;
    return duel.players[p & 1].lp > duel.players[(1 - p) & 1].lp;
}

int duel_08876F80(DuelAct *a, int b)
{
    unsigned int p;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    p = a->player;
    if ((duel.players[p & 1].n_d == 0 || a->id == 0x1741) && duel_088BFFD8(a->player, a->id, duel_088AA224(a)) == 0) {
        return 0;
    }
    return 1;
}

int duel_08877024(DuelAct *a, int b)
{
    unsigned int p = a->player;

    if ((unsigned int)duel.players[p & 1].n_d < 3) {
        return 0;
    }
    return duel_088770C0(a, b);
}

int duel_0887707C(DuelAct *a)
{
    if (duel_08878260() == 0) {
        return 0;
    }
    return duel_field_def(duel_special.player[1], duel_special.zone[1]) >= 2000;
}

#ifdef NON_MATCHING /* load order differs */
int duel_088770C0(DuelAct *a, int b)
{
    unsigned int i;
    int p;

    for (i = 0; i < duel.players[(p = a->player) & 1].n_hand; i++) {
        if (CARD_UID(duel.players[p & 1].hand[i]) != a->uid && duel_088BFFD8(p, a->id, i) != 0) {
            return 1;
        }
    }
    return 0;
}
#endif

int duel_088771D4(DuelAct *a)
{
    int id;

    if (a->kind != 0x23) {
        switch (a->id) {
        case 0x175B:
            id = 0xFA7;
            break;
        case 0x17F6:
            id = 0xFF8;
            break;
        }
        if (duel_card_check_9148(a->player, id) == 0) {
            return 0;
        }
        if (duel_slot_has_effect(a->player, 11, id) != 0) {
            return 0;
        }
    }
    return 1;
}

int duel_08877270(DuelAct *a)
{
    unsigned int p;

    if (a->kind != 0x23 && duel_count_tributable_except(a->player, -1) == 0) {
        return 0;
    }
    if (duel.players[a->player & 1].n_d == 0) {
        return 0;
    }
    return 1;
}

int duel_088772F8(DuelAct *a, int b)
{
    if (duel_088E9150(0x175E) == 0) {
        return 0;
    }
    if (duel_088770C0(a, b) != 0) {
        if (duel.players[(1 - a->player) & 1].n_hand != 0) {
            return 2;
        }
        return 1;
    }
    return 0;
}

int duel_08877388(DuelAct *a)
{
    return duel_count_attr(a->player, 1) > 0;
}

int duel_088773B0(DuelAct *a)
{
    if (duel_phase_ok2(a->player) != 0) {
        return duel_0886CD24(a, a->player) + duel.players[a->player & 1].n_d != 0;
    }
    return 0;
}

int duel_08877424(DuelAct *a)
{
    int n = duel_0886CD24(a, a->player);
    int m = duel_count_grave_kind(a->player, 1);

    if (m == 0) {
        return 0;
    }
    if (n != 0) {
        return n <= m;
    }
    return 0;
}

int duel_08877488(void)
{
    return !duel_count_active(0x13F2);
}

int duel_088774AC(DuelAct *a, DuelAct *b)
{
    if (duel_088DFCC8(a->uid) == 0) {
        return 0;
    }
    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    if (card_is_monster(b->id) == 0 && b->b3_4 != 0) {
        return 0;
    }
    return (duel_act_e403c(b) & 2) > 0;
}

int duel_08877564(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_d != 0) {
        return 2;
    }
    return 0;
}

#ifdef NON_MATCHING /* branch layout differs */
int duel_088775A0(DuelAct *a, int b)
{
    int n;

    if (duel_0886F65C(a, b) != 0) {
        n = duel_count_attack_targets(duel_special.player[0], duel_special.zone[0]);
        if (n >= 2) {
            return 1;
        }
        if (n != 0 && duel_direct_attack_kind(duel_special.player[0], duel_special.zone[0], 1) == 1) {
            return 1;
        } else {
            return 0;
        }
    }
    return 0;
}
#endif

int duel_08877624(DuelAct *a, int b)
{
    unsigned int p = a->player;

    if (duel.players[(1 - p) & 1].n_hand == 0) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1846) != 0 || duel_special.unk24 != 0) {
        return duel_088769A8(a, b);
    }
    return 0;
}

int duel_088776BC(DuelAct *a)
{
    if (duel_card_check_9148(a->player, a->id) != 3) {
        return 0;
    }
    return duel_0884E5E0(a) > 1;
}

int duel_08877710(DuelAct *a, int b)
{
    struct DuelActX *x;

    if (a->kind == 0x16) {
        x = &a->x;
        if (a->src.player != 1 - a->player) {
            return 0;
        }
        if (x->s_zone2 != 0xE) {
            return 0;
        }
        if (card_is_monster(duel.list[x->s_uid].id) == 0) {
            return 0;
        }
        if ((x->s_uid & 1) != 1 - a->player) {
            return 0;
        }
        return duel_0886CE38(a, b);
    }
    return 0;
}

int duel_08877810(DuelAct *a)
{
    return duel_088DFCC8(a->uid) != 0;
}

int duel_08877838(DuelAct *a)
{
    return duel_0884E690(a, (int (*)(int))duel_088559C0) > 0;
}

int duel_0887785C(DuelAct *a)
{
    if (duel_count_faceup_skip(a->player, a->id, a->zone) >= 2) {
        if (duel_phase != 0 && duel_find_effect_card(1 - a->player, 0x178B, -1) != 0) {
            return 0;
        }
    }
    return 2;
}

int duel_088778E0(DuelAct *a, int b)
{
    unsigned int p = a->player;

    if ((unsigned int)duel.players[p & 1].n_d < 4) {
        return 0;
    }
    return duel_0886CE38(a, b);
}

int duel_08877938(DuelAct *a)
{
    if (a->kind != 0x23) {
        if (duel_0884E948(a, a->player) != 5) {
            return 0;
        }
    }
    return 2;
}

int duel_08877988(DuelAct *a)
{
    if (a->kind == 5) {
        return duel_count_free_st_zones(a->player) > 0;
    }
    return 1;
}

int duel_088779C4(DuelAct *a)
{
    return duel_turn_player != a->player;
}

int duel_088779E0(DuelAct *a, int b)
{
    unsigned int p;

    if (duel_slot_has_effect(a->player, 11, a->id) == 0) {
        return 0;
    }
    p = a->player;
    if (duel.players[p & 1].n_d != 0) {
        return duel_0886CE38(a, b);
    }
    return 0;
}

int duel_08877A70(DuelAct *a)
{
    return duel_0884E690(a, (int (*)(int))duel_08858704) > 0;
}

int duel_08877A94(DuelAct *a)
{
    unsigned int f = duel.players[a->player & 1].field[a->zone].flags;

    if ((f >> 16 & 1) || (f >> 17 & 1)) {
        return 1;
    }
    return 0;
}

int duel_08877AF8(DuelAct *a, DuelAct *b)
{
    int i;

    if (b == 0) {
        return 0;
    }
    if (card_is_in_group_147e(b->id) != 0) {
        return 0;
    }
    if (card_is_monster(b->id) == 0 || (b->zone >= 5 && b->zone <= 10)) {
        if (b->b3_4 != 0) {
            return 0;
        }
    }
    for (i = 0; i < b->n_targets; i++) {
        unsigned short loc = a->player | a->zone << 8;

        if (loc == duel_act_target(b, i)) {
            return 1;
        }
    }
    return 0;
}

int duel_08877C00(DuelAct *a)
{
    unsigned int p;

    if (duel_088DFE24(a->player, a->player) == 0) {
        return 0;
    }
    if ((int)duel_grave_find_uid(a->player, a->uid) < 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    p = a->player;
    if (duel.players[p & 1].n_d == 0) {
        if (duel_088BFFD8(a->player, a->id, duel_088AA224(a)) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_08877CD8(DuelAct *a)
{
    struct DuelActX *x;
    int r;

    if (a->kind == 0x16) {
        x = &a->x;
        if (!x->s_b14) {
            return 0;
        }
        if (x->s_zone2 != 0xE) {
            return 0;
        }
        r = card_is_monster(duel.list[x->s_uid].id);
        return r != 0;
    }
    return 0;
}

int duel_08877D64(DuelAct *a)
{
    unsigned int p;
    unsigned short *src;
    int r;

    if (a->kind == 0xF) {
        src = (unsigned short *)&a->src;
        p = a->player;
        if (a->y.y16_5 != p) {
            return 0;
        }
        if (src[0] < 3000) {
            return 0;
        }
        if (duel_088DFE24(p, 0) == 0) {
            return 0;
        }
        r = duel_088DFE24(a->player, 1);
        return r != 0;
    }
    return 0;
}

int duel_08877E04(DuelAct *a, int b)
{
    if (duel_slot_has_effect(a->player, a->zone, 0x17D2) != 0) {
        return duel_08877ED4(a, b);
    }
    return 0;
}

int duel_08877E5C(DuelAct *a, int b)
{
    if (a->kind == 2) {
        return duel_08877ED4(a, b);
    }
    return 1;
}

int duel_08877E90(DuelAct *a, int b)
{
    if (a->kind == 5) {
        return duel_08877E04(a, b);
    }
    return duel_088734E8();
}

int duel_08877ED4(DuelAct *a, int b)
{
    unsigned int p;

    if (duel_slot_efb84(a->player, a->zone) == 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    p = a->player;
    if (duel.players[p & 1].n_d == 0) {
        if (duel_088BFFD8(a->player, a->id, duel_088AA224(a)) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_08877F8C(DuelAct *a)
{
    unsigned int i;
    int best = -1;
    int z;

    for (i = 0; i < duel.players[a->player & 1].n_hand; i++) {
        int id = duel.players[a->player & 1].hand[i].id;
        int atk = card_get_atk_q(id);

        if (card_is_monster(id) != 0 && best < atk && atk != 0xFFFF &&
            duel_088DFCC8(CARD_UID(duel.players[a->player & 1].hand[i])) != 0) {
            best = atk;
        }
    }
    if (best < 0) {
        return 0;
    }
    a->w[2] = best;
    for (z = 0; z <= 4; z++) {
        if (!(!duel_0884E3E4(a, 1 - a->player, z))) {
            return 2;
        }
    }
    return 0;
}

int duel_08878138(DuelAct *a)
{
    unsigned int i;
    unsigned int p;

    if (duel_slot_efb84(a->player, a->zone) == 0) {
        return 0;
    }
    if (duel_0886CD1C(a, a->player) == 0 && duel_zone_usable(a->player, a->zone) == 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    for (i = 0; i < duel.players[(p = a->player) & 1].n_hand; i++) {
        if (!(!duel_088BFFD8(p, a->id, i))) {
            return 1;
        }
    }
    return 0;
}

int duel_08878260(void)
{
    if (duel_special.unk8 != 0) {
        return 0;
    }
    if (duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].faceup == 0) {
        return 0;
    }
    return duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].pos != 0;
}

int duel_088782E0(DuelAct *a, int b)
{
    if (duel_count_attr(0, 6) != 0 || duel_count_attr(1, 6) != 0) {
        return duel_08876174(a, b);
    }
    return 0;
}

int duel_08878340(DuelAct *a)
{
    int z;

    for (z = 0; z <= 4; z++) {
        if (!(!duel_zone_free(1 - a->player, z))) {
            return 1;
        }
    }
    return 0;
}

int duel_088783AC(DuelAct *a)
{
    int r = 0;
    unsigned int n = duel.players[(1 - a->player) & 1].n_hand;

    if (n != 0) {
        if (n <= 3) {
            r = 1;
        }
    }
    return r;
}

int duel_088783FC(DuelAct *a)
{
    unsigned int q = (1 - a->player) & 1;
    int r = 1;

    if (duel.players[q].n_hand == 0 && duel.players[q].n_d == 0) {
        r = 0;
    }
    return r;
}

int duel_08878458(DuelAct *a)
{
    if (duel_turn_player != 1 - a->player) {
        return 0;
    }
    return duel_phase == 2;
}

int duel_08878494(DuelAct *a)
{
    return duel.players[(1 - a->player) & 1].n_hand >= 8;
}

int duel_088784D0(DuelAct *a)
{
    return duel_slot_has_effect(a->player, a->zone, a->id) != 0;
}

int duel_08878500(DuelAct *a)
{
    return duel.players[(1 - a->player) & 1].n_grave != 0;
}

#ifdef NON_MATCHING /* trailing branch differs */
int duel_08878538(DuelAct *a)
{
    if (duel_slot_efb84(a->player, a->zone) == 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (duel_count_monsters(1 - a->player) != 0) {
        int n = duel_count_monsters(1 - a->player);
        int r = n <= duel_count_free_monster_zones(a->player) + 1;

        return r;
    }
    return 0;
}
#endif

int duel_088785E0(DuelAct *a)
{
    int n = duel_slot_value_1909(a->player, a->zone);
    unsigned int p;

    if (n == 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    p = a->player;
    if (duel_tributable(p, p, a->zone) == 0) {
        return 0;
    }
    {
        int u = duel_zone_usable(a->player, a->zone);
        int c = duel_0886CD1C(a, a->player);

        return n <= c + u;
    }
}

int duel_088786A4(DuelAct *a, int b)
{
    unsigned int f = duel.players[a->player & 1].field[a->zone].flags;

    if (f >> 16 & 1) {
        return 0;
    }
    if (f >> 18 & 1) {
        return 0;
    }
    if (f >> 17 & 1) {
        return 0;
    }
    if (f >> 19 & 1) {
        return 0;
    }
    return duel_08877ED4(a, b);
}

int duel_08878744(DuelAct *a, int b)
{
    if (duel_slot_has_effect2(a->player, a->zone, 0x1814) != 0) {
        return duel_08877ED4(a, b);
    }
    return 0;
}

int duel_0887879C(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (a->kind == 0x19) {
        return duel_slot_e68a0(p, z) < 5;
    }
    if (duel_slot_has_effect2(p, z, 0x181A) == 0) {
        return 0;
    }
    if (duel_slot_e68a0(a->player, a->zone) < 5) {
        return 0;
    }
    return duel_08877ED4(a, b);
}

int duel_088788AC(DuelAct *a, int b)
{
    unsigned int p;

    if (a->kind != 0x12) {
        return 0;
    }
    if (a->idx == 0) {
        return 0;
    }
    if ((unsigned short)duel_field_type(duel_special.player[0], duel_special.zone[0]) != 0x12) {
        return 0;
    }
    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    p = a->player;
    if (duel.players[p & 1].n_d == 0) {
        return 0;
    }
    return duel_0886CD1C(a, 1 - p) != 0;
}

int duel_0887898C(DuelAct *a)
{
    unsigned int p;
    int z;
    int n = 0;

    if (duel_phase != 2) {
        return 0;
    }
    for (z = 0; z <= 4; z++) {
        p = a->player;
        if (duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup != 0 &&
            duel.players[p & 1].field[z].pos == 0 && (unsigned short)duel_field_type(p, z) == 0x12) {
            n++;
        }
    }
    if (a->kind == 0x23) {
        return n > 0;
    }
    return n >= 2;
}

int duel_08878A88(DuelAct *a, int b)
{
    unsigned int p;

    if (duel_0886CE90(a, b) == 0) {
        return 0;
    }
    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    p = a->player;
    return duel.players[p & 1].n_hand != 0;
}

int duel_08878B08(DuelAct *a, int b)
{
    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    return (int)duel_grave_find_uid(a->player, a->uid) >= 0;
}

int duel_08878B58(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (b->zone > 10) {
        return 0;
    }
    if (b->b3_4 != 0) {
        return 0;
    }
    if (b->player != a->player) {
        return 0;
    }
    if (card_get_type(b->id) != 0x16) {
        return 0;
    }
    if (card_get_prop5_hi(b->id) != 0) {
        return 0;
    }
    if (duel_hand_can_discard(a->player, duel_0886CD24(a, a->player), a->uid, 0, 0) == 0) {
        return 0;
    }
    if (duel_0886CDC4(a, (int)b) != 0) {
        return 2;
    }
    return 0;
}

int duel_08878C6C(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_d == 0 && duel.players[(1 - p) & 1].n_hand == 0) {
        return 0;
    }
    return 1;
}

int duel_08878CDC(DuelAct *a, int b)
{
    unsigned int i;
    unsigned int p;

    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    if (duel_phase != 0 && duel_find_effect_card(1 - a->player, 0x178B, -1) != 0) {
        return 0;
    }
    for (i = 0; i < duel.players[(p = a->player) & 1].n_hand; i++) {
        if (!(!duel_088BFFD8(p, a->id, i))) {
            return 1;
        }
    }
    return 0;
}

int duel_08878DDC(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (card_is_in_group_147e(b->id) != 0) {
        return 0;
    }
    if (card_is_monster(b->id) == 0) {
        return 0;
    }
    if (card_attribute_to_group(b->id) == 0) {
        return 0;
    }
    if (b->zone == 0xD) {
        return 0;
    }
    return duel_0886CDC4(a, (int)b);
}

int duel_08878E80(DuelAct *a)
{
    int n = 0;
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 10; z++) {
            if (duel_0884E3E4(a, p, z) != 0 && a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
                n++;
            }
        }
    }
    return n > 1;
}

int duel_08878F60(DuelAct *a)
{
    if (duel.players[(1 - a->player) & 1].n_hand != 0) {
        return 2;
    }
    return 1;
}

#ifdef NON_MATCHING /* branch layout differs */
int duel_08878FA4(DuelAct *a)
{
    unsigned int p;
    unsigned int q;

    if (duel_count_attr(a->player, 3) == 0 && duel_count_attr(1 - a->player, 3) == 0) {
        return 0;
    }
    if (duel_count_attr(a->player, 4) == 0 && duel_count_attr(1 - a->player, 4) == 0) {
        return 0;
    }
    if (duel_count_attr(a->player, 5) == 0 && duel_count_attr(1 - a->player, 5) == 0) {
        return 0;
    }
    if (duel_count_attr(a->player, 6) == 0 && duel_count_attr(1 - a->player, 6) == 0) {
        return 0;
    }
    if (duel_0884E690(a, (int (*)(int))duel_088559C0) != 0) {
        return 1;
    }
    if (duel_0884E690(a, (int (*)(int))duel_08857FEC) != 0) {
        return 1;
    }
    p = a->player;
    q = 1 - p;
    if (duel.players[q & 1].n_hand != 0) {
        return 1;
    }
    if ((unsigned int)duel.players[p & 1].n_d < 2) {
        return 0;
    }
    if (duel_phase == 0) {
        return 1;
    }
    if (duel_find_effect_card(q, 0x178B, -1) != 0) {
        return 0;
    }
    return 1;
}
#endif

int duel_08879194(DuelAct *a, DuelAct *b)
{
    if (b != 0 && b->id == a->id) {
        return 0;
    }
    if ((unsigned int)(a->kind - 0x12) < 2) {
        if (duel_special.player[1] == a->player && duel_special.unk8 == 0 &&
            duel_count_e17a0(duel_special.player[1], 0x1318) == 0) {
            return 2;
        }
    }
    return a->b3_4 == 0;
}

int duel_08879240(DuelAct *a)
{
    switch (a->kind) {
    case 0x14:
    case 0xD:
        if (duel_special.unk8 == 0 && duel_special.player[0] != a->player && duel_special.player[1] == a->player &&
            duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].card.id != 0 &&
            duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].faceup != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_08879318(DuelAct *a, int b)
{
    if (duel.players[a->player & 1].field[a->zone].pos != 0) {
        return 0;
    }
    return duel_0886CD90(a, b);
}

int duel_08879384(DuelAct *a)
{
    unsigned int p;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    p = a->player;
    return duel_tributable(p, p, a->zone) != 0;
}

int duel_088793D8(DuelAct *a)
{
    if (a->kind == 6) {
        return (int)((unsigned int)(((unsigned char *)&a->src)[2] << 28) >> 30) <= duel_088BFFD8(a->player, a->id, 0);
    }
    return 0;
}

int duel_0887943C(DuelAct *a)
{
    int n = duel_slot_e68a0(a->player, a->zone);

    a->w[2] = n;
    return n > 0;
}

int duel_08879478(DuelAct *a)
{
    unsigned int p;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    p = a->player;
    if (duel_tributable(p, p, a->zone) == 0) {
        return 0;
    }
    if (duel_088BFFD8(a->player, a->id, 0) != 0) {
        return 2;
    }
    return 0;
}

int duel_088794F8(DuelAct *a)
{
    if (duel_slot_efb84(a->player, a->zone) == 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    {
        int u = duel_zone_usable(a->player, a->zone);
        int c = duel_0886CD1C(a, a->player);

        if (c + u < 2) {
            return 0;
        }
    }
    return (unsigned int)duel.players[a->player & 1].n_d > 1;
}

int duel_088795C4(DuelAct *a)
{
    if (a->kind == 2) {
        return 2;
    }
    return duel_0886DD78(a);
}

int duel_08879600(DuelAct *a, int b)
{
    unsigned int i;
    unsigned int p;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    for (i = 0; i < duel.players[(p = a->player) & 1].n_hand; i++) {
        if (!(!duel_088BFFD8(p, a->id, i))) {
            return 1;
        }
    }
    return 0;
}

int duel_088796E0(DuelAct *a, int b)
{
    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    return duel_088770C0(a, b) != 0;
}

int duel_08879744(DuelAct *a)
{
    return duel.players[(1 - a->player) & 1].n_hand == 4;
}

int duel_08879780(DuelAct *a)
{
    if (duel_turn_player != 1 - a->player) {
        return 0;
    }
    return (unsigned int)duel_phase < 4;
}

int duel_088797B8(DuelAct *a)
{
    return duel_grave_count_id(a->player, a->id) + 1U <= (unsigned int)duel.players[a->player & 1].n_d;
}

int duel_0887981C(DuelAct *a)
{
    return duel.players[(1 - a->player) & 1].n_f != 0;
}

int duel_08879854(DuelAct *a)
{
    int p;
    int z;
    unsigned int mask = 0;
    int n = 0;

    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 4; z++) {
            if (duel_tributable(a->player, p, z) != 0 &&
                (a->player == p || duel.players[p & 1].field[z].faceup != 0)) {
                unsigned int m = duel_field_attrs(p, z);

                if (duel_bit_count(m) > 1) {
                    n++;
                } else {
                    mask |= m;
                }
            }
        }
    }
    return n + duel_bit_count(mask & 0x78) >= 4;
}

int duel_08879964(DuelAct *a)
{
    if (duel_phase != 2) {
        return 0;
    }
    return !duel_count_e17a0(1 - a->player, 0x1318);
}

int duel_088799B0(DuelAct *a)
{
    unsigned int i;
    unsigned int p;

    p = a->player;
    if (duel_tributable(p, p, a->zone) == 0) {
        return 0;
    }
    if (duel_0886CD1C(a, a->player) == 0 && duel_zone_usable(a->player, a->zone) == 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    for (i = 0; i < duel.players[(p = a->player) & 1].n_hand; i++) {
        if (!(!duel_088BFFD8(p, a->id, i))) {
            return 1;
        }
    }
    return 0;
}

int duel_08879ADC(DuelAct *a, int b)
{
    if ((unsigned int)(((unsigned char *)&a->src)[2] << 28) >> 30 != 0) {
        return 0;
    }
    if (duel_count_monsters(a->player) > 1) {
        return 0;
    }
    return duel_0886DE0C(a, b);
}

int duel_08879B48(DuelAct *a)
{
    unsigned char save;

    if (duel_slot_efb84(a->player, a->zone) == 0) {
        return 0;
    }
    save = a->n7;
    a->n7 = 0;
    if (duel_0884E5E0(a) == 0) {
        return 0;
    }
    a->n7 = 1;
    if (duel_0884E5E0(a) == 0) {
        return 0;
    }
    a->n7 = 2;
    if (duel_0884E5E0(a) == 0) {
        return 0;
    }
    a->n7 = save;
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (duel.players[a->player & 1].n_d == 0 && duel_088BFFD8(a->player, a->id, duel_088AA224(a)) == 0) {
        return 0;
    }
    return 2;
}

int duel_08879C60(DuelAct *a)
{
    if (duel_slot_efb84(a->player, a->zone) == 0) {
        return 0;
    }
    {
        int c = duel_0886CD1C(a, a->player);
        int u = duel_zone_usable(a->player, a->zone);

        if (c < (!(u != 0)) + 2) {
            return 0;
        }
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (duel_grave_count_id(a->player, 0x189E) == 0) {
        return 0;
    }
    if (duel_grave_count_id(a->player, 0x18A0) == 0) {
        return 0;
    }
    if (duel_grave_count_id(a->player, 0x189F) != 0) {
        return 2;
    }
    return 0;
}

#ifdef NON_MATCHING /* instruction order differs */
int duel_08879D5C(DuelAct *a)
{
    unsigned int z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_0884E948(a, 1 - p) != 0) {
        return 2;
    }
    return 0;
}
#endif

int duel_08879E20(DuelAct *a)
{
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (((duel_card_locate(a->uid) & 0xFFFF) >> 8 & 0xFF) != 0xF) {
        return 0;
    }
    return !duel_slot_effect_with(a->player, 11, 0x16BE, a->uid);
}

#ifdef NON_MATCHING /* instruction order differs */
int duel_08879EA8(DuelAct *a)
{
    unsigned int z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    return duel_0886CD1C(a, p) != 0;
}
#endif

int duel_08879F60(DuelAct *a, int b)
{
    unsigned int p = a->player;

    if (duel_088DFE24(p, p) != 0) {
        return duel_0886CFE0(a, b);
    }
    return 0;
}

int duel_08879FB0(DuelAct *a, int b)
{
    if (duel_count_free_monster_zones(a->player) != 0) {
        return duel_08875D80(a, b);
    }
    return 0;
}

int duel_08879FFC(DuelAct *a, int b)
{
    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    return duel_0886CDC4(a, b) != 0;
}

int duel_0887A048(DuelAct *a, int b)
{
    if (a->kind == 0x23) {
        return 0;
    }
    if (duel_phase != 2) {
        return 0;
    }
    if (duel_088770C0(a, b) != 0) {
        return 2;
    }
    return 0;
}

int duel_0887A0AC(DuelAct *a, int b)
{
    if (duel_0886CD90(a, b) == 0) {
        return 0;
    }
    if (duel_0884E5E0(a) >= 2) {
        return 2;
    }
    return 0;
}

int duel_0887A0FC(DuelAct *a)
{
    struct DuelActX *x;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (duel.players[a->player & 1].n_d == 0 && duel_088BFFD8(a->player, a->id, duel_088AA224(a)) == 0) {
        return 0;
    }
    if (a->kind == 0x16) {
        x = &a->x;
        if (a->src.player == a->player && a->src.zone < 5 && x->s_b16 && x->s_zone2 == 0xE) {
            return 1;
        }
    }
    return 0;
}

int duel_0887A208(DuelAct *a)
{
    if (a->kind == 8) {
        return card_attribute_icon_is3(duel.list[a->src.uid].id);
    }
    return 0;
}

int duel_0887A258(DuelAct *a)
{
    return duel_hand_count_monsters(a->player) > 0;
}

int duel_0887A27C(DuelAct *a)
{
    if (duel_locks_ok(a->player, 1) != 0) {
        return duel.players[(a->player ^ a->opp) & 1].n_d != 0;
    }
    return 0;
}

int duel_0887A2F0(DuelAct *a)
{
    if (a->kind == 8) {
        return card_is_attribute9(duel.list[a->src.uid].id);
    }
    return 0;
}

int duel_0887A340(DuelAct *a)
{
    unsigned int p;

    if (duel_08876904(a) == 0) {
        return 0;
    }
    p = a->player;
    if (duel.players[p & 1].n_hand != 0) {
        return 2;
    }
    if (duel.players[(1 - p) & 1].n_hand != 0) {
        return 2;
    }
    return 1;
}

int duel_0887A3D0(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int n = duel.players[p & 1].field[a->zone].link;

    if (n == 0) {
        return 0;
    }
    if (duel_088DFE24(p, p) == 0) {
        return 0;
    }
    if (duel.players[a->player & 1].n_grave < n) {
        return 0;
    }
    if (duel_0884E5E0(a) < (int)n) {
        return 0;
    }
    return 2;
}

int duel_0887A4B4(DuelAct *a)
{
    unsigned int p = a->player;
    int same;

    if ((unsigned int)duel.players[p & 1].n_d < 2) {
        return 0;
    }
    if (duel_phase != 0 && duel_find_effect_card(1 - p, 0x178B, -1) != 0) {
        return 0;
    }
    p = a->player;
    if (duel.players[p & 1].n_hand != 0) {
        return 0;
    }
    same = !(a->uid != CARD_UID(duel.players[p & 1].field[a->zone].card));
    return same == duel_count_field(p);
}

int duel_0887A5DC(DuelAct *a, int b)
{
    if (duel.players[a->player & 1].n_d == 0) {
        return 0;
    }
    if (duel_0886CE38(a, b) != 0) {
        return duel_08875D80(a, b);
    }
    return 0;
}

int duel_0887A660(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel_turn_player == p) {
        return 0;
    }
    if (duel_phase != 3) {
        return 0;
    }
    return duel_tributable(p, p, a->zone);
}

int duel_0887A6B8(DuelAct *a, int b)
{
    if (duel_hand_can_discard(a->player, duel_08861360(a), a->uid, 0, 0) != 0) {
        return duel_0887D4AC(a, b);
    }
    return 0;
}
