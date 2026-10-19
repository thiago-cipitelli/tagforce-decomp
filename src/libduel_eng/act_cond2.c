// @range 0x0886F23C-0x08874078
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
int duel_08875530(void);
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
    return duel_08875530();
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
