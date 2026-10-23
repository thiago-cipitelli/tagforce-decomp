// @range 0x08869190-0x0886AAB4
// @asm 0x0886981C-0x08869ACC
/* Card effect handlers (DuelAct *a): step machines on duel_w_08914904. */
#include "duel.h"

int duel_088619FC(DuelAct *a, int b);
void duel_088D1AC8(unsigned int p, int z);
unsigned int duel_slot_link_1a0c(unsigned int p, int z);
void duel_0884ACD8(unsigned int p, int msg);
void duel_088359E8(unsigned int p, int id, int (*fn)());
int duel_088537B0(int id);
int duel_08835AC0(void);
int duel_field_level(unsigned int p, int z);
int duel_0884E690(DuelAct *a, int (*fn)());
void duel_088D1D64(DuelAct *a, unsigned int mask);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
DuelCard *duel_0884CCBC(int i);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
int duel_grave_find_uid(unsigned int p, int uid);
int field_get_card(unsigned int p, int z);
int duel_0886981C(DuelAct *a, int b);
int duel_08869ACC(DuelAct *a, int b);
int duel_08869F5C();
int duel_0886AAB4(DuelAct *a, int b);
int duel_0884E340(unsigned short id);
int card_is_in_group_1517(int id);
int card_is_monster(int id);
int card_is_attribute8(int id);
unsigned short duel_slot_e1854(unsigned int p, int z);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
int card_is_in_group_1a60(int id);
int card_get_type(int id);
int duel_can_place_st(unsigned int p, int id);
int duel_count_active(int id);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_find_effect_card(unsigned int p, int id, int z);
int card_get_prop5_hi(int id);
int card_get_type_class(int id);
int duel_card_zone_ok(int id, int zone);
int duel_card_e3070(int id);
int duel_field_protected(unsigned int p, int z);
int duel_card_protected(int id, int uid);
int card_is_in_group_147e(int id);
int duel_special_has_effect(int p, int z, int id);
int card_get_special_group(unsigned short id);
int duel_count_monster_active_both(int id);
int duel_08860FDC(DuelAct *a);
int duel_card_e4de8(int id);
int duel_slot_has_effect12_arg(unsigned int p, int z, int id, int arg);
int duel_slot_affected(unsigned int p, int z, int id);
int duel_slot_effect_arg(unsigned int p, int z, int id);
unsigned int duel_card_locate(unsigned int uid);

extern int duel_w_08914904;
extern int duel_phase;
extern unsigned int duel_w_08917DB4;
extern int duel_err;
extern unsigned int duel_view_player;
extern unsigned int duel_w_08914920;
extern unsigned int duel_turn_player;
extern unsigned int duel_atk_tp;
extern int duel_atk_tz;
extern unsigned int duel_w_08919624;
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern unsigned int duel_w_089147E0;

int duel_08869190(DuelAct *a, int b)
{
    if (a->idx != 0) {
        return 1;
    }
    return duel_088619FC(a, b);
}

int duel_088691C0(DuelAct *a)
{
    duel_088D1AC8(a->player, a->zone);
    a->w[0] = duel_slot_link_1a0c(a->player, a->zone);
    return 1;
}

/* Picks field cards while duel_0884E690 finds more; w[2] = sum of their levels. */
int duel_08869210(DuelAct *a)
{
    unsigned int p;
    int z;

    switch (duel_w_08914904) {
    case 0:
        a->w[1] = 0;
        a->w[0] = 0;
        a->w[2] = 0;
        duel_0884ACD8(a->player, 0x3A);
        duel_w_08914904++;
    case 1:
        duel_088359E8(a->player, a->id, duel_088537B0);
        duel_w_08914904++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            a->w[2] += (unsigned short)duel_field_level(p, z);
            a->w[p] |= 1 << z;
            if (duel_0884E690(a, duel_088537B0) == 0) {
                duel_w_08914904++;
                return 0;
            }
            duel_w_08914904--;
            return 0;
        }
        if (a->w[2] != 0) {
            duel_w_08914904++;
            return 0;
        }
        duel_w_08914904 = 0;
        return 0;
    case 3:
        duel_088D1D64(a, a->w[0] | a->w[1] << 16);
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

int duel_088693E8(DuelAct *a)
{
    unsigned int i;

    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(a->player, 0x56);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_0884C588(a->player, 0x47, a->id, 0);
        duel_w_08914904++;
        return 0;
    case 2:
        duel_088C7BA4();
        for (i = 0; i < duel_w_08919624; i++) {
            duel_088CB400(a->player, duel_0884CCBC(i + 1), 0);
        }
        duel_088C7CF0();
        a->w[0] = duel_w_08919624;
        return 1;
    }
    return 1;
}

int duel_088694F8(DuelAct *a)
{
    int k = duel_grave_find_uid(a->player, a->uid);
    unsigned int p = a->player;

    duel_088CB400(p, &duel.players[p & 1].grave[k], 0);
    return 1;
}

/* Whether the card uid is in one of the actions queued by the computer player. */
int duel_0886956C(int uid)
{
    unsigned int i;

    for (i = 0; i < duel_w_089147E0; i++) {
        if (uid == duel_chain.pending[i].uid) {
            return 1;
        }
    }
    return 0;
}

/* Whether player p can activate the card in zone z of player q (x: passed to the checks). */
int duel_088695B8(unsigned int p, unsigned int q, int z, int x)
{
    DuelAct act = { 0 };
    int id = field_get_card(q, z);

    if (id == 0) {
        return 0;
    }
    act.id = id;
    act.player = q;
    act.zone = z;
    act.opp = p != q;
    act.uid = CARD_UID(duel.players[q & 1].field[z].card);
    if (duel.players[q & 1].field[z].faceup != 0) {
        return duel_0886981C(&act, x);
    }
    return duel_08869ACC(&act, x);
}

/* Whether player p can activate card i of the hand. */
int duel_08869728(unsigned int p, int i, int x)
{
    DuelAct act = { 0 };

    act.id = duel.players[p & 1].hand[i].id;
    act.player = p;
    act.zone = 0xB;
    act.opp = 0;
    act.uid = CARD_UID(duel.players[p & 1].hand[i]);
    return duel_08869ACC(&act, x);
}

#ifdef NON_MATCHING /* só o teste inicial difere: o original carrega opp antes de duel_view_player e usa bnel; nenhuma grafia da comparação reproduziu */
/* Activation check of a face-up card on the field. */
int duel_0886981C(DuelAct *a, int b)
{
    unsigned short t;
    unsigned int i;

    if ((a->player ^ a->opp) == duel_view_player) {
        duel_err = 4;
    }
    if (a->id == 0) {
        return 0;
    }
    a->b3_4 = 3;
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    if (duel_0884E340(a->id) == 0) {
        return 0;
    }
    if (a->opp) {
        if (card_is_in_group_1517(a->id) == 0) {
            return 0;
        }
    }
    if (card_is_monster(a->id)) {
        if (a->zone >= 5) {
            if (card_is_attribute8(a->id) == 0) {
                return 0;
            }
            t = duel_slot_e1854(a->player, a->zone);
            if (t == 0xFFFF) {
                return 0;
            }
            if (duel_st_affects(t & 0xFF, (unsigned char)(t >> 8), a->player, a->zone) != 11) {
                return 0;
            }
        }
    } else if (a->zone < 5) {
        return 0;
    }
    if (duel_w_08914920 == 0) {
        if (duel_08869F5C(a, b) && duel_0886AAB4(a, b)) {
            return 1;
        }
    } else {
        for (i = 0; i < duel_w_08914920; i++) {
            a->kind = duel_chain.kinds[i];
            a->idx = *(int *)&duel_chain.srcs[i];
            if (duel_08869F5C(a, b) && duel_0886AAB4(a, b)) {
                return 1;
            }
        }
    }
    return 0;
}
#endif

/* Activation check of a card of the hand or set on the field. */
int duel_08869ACC(DuelAct *a, int b)
{
    int ok;
    unsigned int i;
    unsigned int j;
    unsigned char z;

    if (a->id == 0) {
        return 0;
    }
    if (a->player == duel_view_player) {
        duel_err = 4;
    }
    a->b3_4 = 0;
    if (a->opp) {
        return 0;
    }
    if (a->zone == 0xB) {
        if (card_is_monster(a->id)) {
            if (card_is_in_group_1a60(a->id) == 0) {
                return 0;
            }
            if (a->id != 0xFEE) {
                a->b3_4 = 2;
            }
        } else if (card_get_type(a->id) == 0x16) {
            if (duel_can_place_st(a->player, a->id) == 0) {
                return 0;
            }
            if (duel_turn_player != a->player) {
                return 0;
            }
            if (duel_count_active(0x1390) != 0) {
                return 0;
            }
        } else {
            ok = 0;
            if (duel_can_place_st(a->player, a->id) == 0) {
                return 0;
            }
            if (duel_slot_has_effect(a->player, 0xB, 0x14A5)) {
                ok = 1;
            }
            if (duel_slot_has_effect(a->player, 0xB, 0x198A) && duel_slot_effect_with(a->player, 0xB, 0x198A, 1) == 0) {
                ok = 1;
            }
            if (duel_atk_tp == a->player) {
                for (i = 0; i < duel_w_08914920; i++) {
                    switch (duel_chain.kinds[i]) {
                    case 0x12:
                        if (duel_slot_e8170(duel_atk_tp, duel_atk_tz, 0x18F5)) {
                            ok = 1;
                        }
                        break;
                    }
                }
            }
            if (ok == 0) {
                return 0;
            }
        }
    } else {
        if (card_is_monster(a->id)) {
            return 0;
        }
        z = a->zone;
        if (z < 5) {
            return 0;
        }
        if (duel.players[a->player & 1].field[z].faceup != 0) {
            return 0;
        }
        if (duel.players[a->player & 1].field[z].flags >> 20 & 1) {
            if (card_get_type(a->id) == 0x17 && duel_find_effect_card(a->player, 0x146F, -1) == 0) {
                if (a->player == duel_view_player) {
                    duel_err = 1;
                }
                return 0;
            }
            if (card_get_prop5_hi(a->id) == 5) {
                if (a->player == duel_view_player) {
                    duel_err = 1;
                }
                return 0;
            }
        }
    }
    if (duel_w_08914920 == 0) {
        if (duel_08869F5C(a, b) && duel_0886AAB4(a, b)) {
            return 1;
        }
    } else {
        for (j = 0; j < duel_w_08914920; j++) {
            a->kind = duel_chain.kinds[j];
            a->idx = *(int *)&duel_chain.srcs[j];
            if (duel_08869F5C(a, b) && duel_0886AAB4(a, b)) {
                return 1;
            }
        }
    }
    return 0;
}

/* Timing check of an activation (b: the action it would chain to, or 0). */
int duel_08869F5C(DuelAct *a, DuelAct *b)
{
    unsigned int p = a->player ^ a->opp;
    int ok;
    unsigned char z;

    if (b != 0) {
        if (card_get_type_class(a->id) < card_get_type_class(b->id)) {
            return 0;
        }
        if (card_get_type_class(a->id) < 2) {
            return 0;
        }
        z = a->zone;
        if (z >= 5 && z < 11 && duel.players[a->player & 1].field[z].flags >> 1 & 1) {
            return 0;
        }
    } else if (card_get_type_class(a->id) < 2 && a->id != 0x14D7 && a->id != 0x131E) {
        if (a->kind > 5) {
            ok = 0;
            if (a->zone < 5) {
                switch (a->kind) {
                case 6:
                case 7:
                case 8:
                    ok = 1;
                    break;
                }
            }
            if (ok == 0) {
                if (p == duel_view_player) {
                    duel_err = 0x16;
                }
                return 0;
            }
        }
        if (duel_turn_player != p || (duel_phase != 2 && duel_phase != 4)) {
            if (p == duel_view_player) {
                duel_err = 0x16;
            }
            return 0;
        }
    }
    if (duel_w_08917DB4 & 1) {
        if (duel_card_zone_ok(a->id, a->kind) == 0) {
            if (p == duel_view_player) {
                duel_err = 0x15;
            }
            return 0;
        }
    }
    switch (a->kind) {
    case 0x1F:
    case 0x20:
    case 0x21:
        if (b == 0 && duel_card_e3070(a->id) == 0) {
            return 0;
        }
        break;
    }
    return 1;
}

/* Whether the activation of a is prevented by a card on the field (1, with duel_err 0x14). */
int duel_act_resolve_6a230(DuelAct *a)
{
    unsigned int p = a->player ^ a->opp;
    int m;
    int loc;
    int q;
    int z;
    unsigned int i;
    int k;

    if (a->zone < 5) {
        if (duel_field_protected(a->player, a->zone)) {
            goto blocked;
        }
    } else if (duel_card_protected(a->id, a->uid)) {
        goto blocked;
    }
    if (!card_is_monster(a->id) || (a->zone >= 5 && a->zone < 11)) {
        if (duel_slot_has_effect(0, 0xB, 0x1407)) {
            goto blocked;
        }
        if (a->zone < 11 && duel_slot_has_effect(a->player, a->zone, 0x19AE)) {
            goto blocked;
        }
    }
    if (card_is_in_group_147e(a->id)) {
        return 0;
    }
    if (a->zone < 5) {
        loc = duel_card_locate(a->uid);
        if ((loc >> 16 & 0xFFFF) != 0) {
            if (duel_special_has_effect(loc & 0xFF, (unsigned char)((loc & 0xFFFF) >> 8), 0x1944)) {
                goto blocked;
            }
        } else if (duel_slot_has_effect(loc & 0xFF, (unsigned char)((loc & 0xFFFF) >> 8), 0x1944)) {
            goto blocked;
        }
    }
    if (duel_slot_effect_with(duel_turn_player, 0xB, 0x15FF, a->uid)) {
        goto blocked;
    }
    if (card_get_special_group(a->id) == 1) {
        if (duel_count_active(0x148E)) {
            goto blocked;
        }
    }
    if (duel_count_monster_active_both(0x168A)) {
        if (duel_08860FDC(a)) {
            goto blocked;
        }
    }
    if (!card_is_monster(a->id) || (a->zone >= 5 && a->zone < 11)) {
        if (duel_slot_has_effect(0, 0xB, 0x1684)) {
            goto blocked;
        }
        if (duel_slot_effect_with(p, 0xB, 0x1679, 2)) {
            goto blocked;
        }
    } else if (duel_slot_effect_with(p, 0xB, 0x1679, 1)) {
        goto blocked;
    }
    k = a->b3_4;
    if (k == 0 || k == 3) {
        m = duel_card_e4de8(a->id);
        if (duel_phase != 0) {
            if (m & 1) {
                if (duel_find_effect_card(1 - p, 0x178B, -1)) {
                    goto blocked;
                }
            }
            if (m & 2) {
                if (duel_find_effect_card(p, 0x178B, -1)) {
                    goto blocked;
                }
            }
        } else if (duel_slot_has_effect12_arg(duel_turn_player, 0xB, 0x1885, 0)) {
            if (m & 1) {
                if (p == duel_turn_player) {
                    goto blocked;
                }
            }
            if (m & 2) {
                if (p != duel_turn_player) {
                    goto blocked;
                }
            }
        }
    }
    if (a->zone <= 10 || a->b3_4 == 0) {
        if (card_get_type(a->id) == 0x17) {
            for (q = 0; q < 2; q++) {
                for (z = 0; z <= 4; z++) {
                    if (duel_slot_e8170(q, z, 0x1296)) {
                        if (a->player != q) {
                            goto blocked;
                        }
                        if (duel_slot_affected(q, z, 0x12D3) == 0) {
                            goto blocked;
                        }
                    }
                }
            }
            if (duel_count_active(0x1760)) {
                goto blocked;
            }
            if (duel_slot_has_effect(1 - a->player, 0xB, 0x1840)) {
                goto blocked;
            }
        } else if (!card_is_monster(a->id)) {
            if (a->zone > 4 || a->b3_4 == 0) {
                if (duel_count_active(0x15DA)) {
                    goto blocked;
                }
                if (duel_count_active(0x1765)) {
                    goto blocked;
                }
            }
        }
    }
    if (a->b3_4 == 0) {
        if (!card_is_monster(a->id)) {
            if (duel_slot_has_effect(a->player, 0xB, 0x18AB)) {
                loc = duel_card_locate(duel_slot_effect_arg(a->player, 0xB, 0x18AB));
                if ((unsigned char)((loc & 0xFFFF) >> 8) <= 10) {
                    goto blocked;
                }
            }
            if (duel_slot_effect_with(a->player, 0xB, 0x1885, 1)) {
                goto blocked;
            }
            if (duel_slot_has_effect(a->player, 0xB, 0x1A2A)) {
                goto blocked;
            }
        }
    }
    if (a->b3_4 == 0) {
        if (card_get_type(a->id) == 0x16) {
            if (duel_slot_has_effect(a->player, a->zone, 0x1390)) {
                goto blocked;
            }
            if (duel_slot_has_effect(a->player, 0xB, 0x13BD)) {
                goto blocked;
            }
            q = 1 - a->player;
            if (duel.players[q & 1].n_hand == 0) {
                if (duel_find_effect_card(q, 0x1910, -1)) {
                    goto blocked;
                }
            }
            if (card_get_prop5_hi(a->id) == 5) {
                if (duel_find_effect_card(1 - a->player, 0x1722, -1)) {
                    goto blocked;
                }
            }
        }
    }
    if (a->b3_4 == 0) {
        if (card_get_type(a->id) == 0x17) {
            for (i = 0; i < duel_w_08914920; i++) {
                if (duel_chain.kinds[i] == 6) {
                    switch (duel.list[((DuelSrc *)&duel_chain.srcs[i])->uid].id) {
                    case 0x182F:
                    case 0x1830:
                    case 0x1831:
                    case 0x1832:
                        goto blocked;
                    }
                }
            }
            if (duel_slot_has_effect(0, 0xB, 0x188E)) {
                goto blocked;
            }
            if (duel_phase == 3) {
                if (duel_find_effect_card(1 - a->player, 0x1833, -1)) {
                    goto blocked;
                }
                if (duel_find_effect_card(1 - a->player, 0x1834, -1)) {
                    goto blocked;
                }
                if (duel_slot_has_effect(0, 0xB, 0x19BB)) {
                    goto blocked;
                }
            }
        }
    }
    if (a->b3_4 == 0) {
        if (a->zone < 11) {
            if (duel_slot_affected(a->player, a->zone, 0x184A)) {
                goto blocked;
            }
        }
    }
    k = a->b3_4;
    if (k == 0 || k == 2) {
        if (!card_is_monster(a->id)) {
            if (duel_slot_effect_with(p, 0xB, 0x1664, a->uid)) {
                goto blocked;
            }
            if (duel_slot_effect_with(p, 0xB, 0x16DD, a->id)) {
                goto blocked;
            }
        }
    }
    return 0;
blocked:
    if (p == duel_view_player) {
        duel_err = 0x14;
    }
    return 1;
}
