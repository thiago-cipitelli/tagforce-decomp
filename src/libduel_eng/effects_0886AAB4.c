// @range 0x0886AAB4-0x0886CB44
// @asm 0x0886B1F4-0x0886B4E8
// @asm 0x0886B6B8-0x0886BCDC
// @asm 0x0886BCDC-0x0886C2E0
/* Card effect handlers and activation checks (DuelAct *a). */
#include "duel.h"

int duel_act_resolve_6a230(DuelAct *a);
int card_get_type_class(int id);
int card_is_monster(int id);
int duel_08860A88(DuelAct *a);
int duel_count_active(int id);
int card_get_prop5_hi(int id);
int duel_pick_st_zone(unsigned int p);
DuelCard *duel_card_by_uid(int uid);
int duel_0884E9E8(DuelAct *a, DuelAct *b);
void duel_bzero(void *p, int n);
int duel_0884ECD4(DuelAct *a);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_min_order_both(int id);
int field_get_card(unsigned int p, int z);
int duel_field_protected(unsigned int p, int z);
int duel_slot_count_src(unsigned int p, int z, int id);
int duel_count_monsters(unsigned int p);
int duel_slot_affected(unsigned int p, int z, int id);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_card_check_9148(unsigned int p, int id);
void duel_0886B6B8(int *f);
int card_is_in_group_147e(int id);
int card_attribute_to_group(int id);
int card_get_special_group(unsigned short id);
int card_get_type(int id);
unsigned int duel_field_flag9(unsigned int p, int z);
int duel_card_fits_zone(unsigned int p, int z, int id);
int duel_count_monster_active_atk(unsigned int p, int id);
int duel_slot_has_src(unsigned int p, unsigned int z, unsigned int q, int sz);
unsigned int duel_field_type(unsigned int p, int z);
void duel_088CBB64(unsigned int p, int z, int a);
unsigned int duel_card_locate(unsigned int uid);
int duel_special_has_effect(int p, int z, int id);
int duel_zone_faceup(unsigned int p, int z, int i);
int duel_zone_pos(unsigned int p, int z, int i);
int duel_special_affected(unsigned int p, int z, int id);
int duel_special_count_src(unsigned int p, int z, int id);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
int duel_count_monsters_special(int p);
int duel_act_e50ec(DuelAct *a);
int duel_0886981C(DuelAct *a, DuelAct *b);
int duel_08869ACC(DuelAct *a, DuelAct *b);

extern unsigned int duel_w_08914920;
extern unsigned int duel_w_08917DB8;
extern int duel_err;
extern unsigned int duel_view_player;

/* Whether a can be activated now (b: the action it chains to, or 0): cost, then the activation itself
   with the card temporarily placed face up. */
int duel_0886AAB4(DuelAct *a, DuelAct *b)
{
    unsigned int p = a->player ^ a->opp;
    int n;
    unsigned char z;
    int r;

    if (duel_act_resolve_6a230(a)) {
        return 0;
    }
    if (b != 0) {
        if (card_get_type_class(a->id) < card_get_type_class(b->id)) {
            return 0;
        }
        switch (b->id) {
        case 0x19A3:
            if (!card_is_monster(a->id)) {
                return 0;
            }
            break;
        }
    }
    n = duel_08860A88(a);
    if (a->zone == 0xB) {
        n += duel_count_active(0x132C) * 500;
    }
    if (duel.players[p & 1].lp <= n) {
        if (p == duel_view_player) {
            duel_err = 0x17;
        }
        return 0;
    }
    if (a->b3_4 == 0 && !card_is_monster(a->id) && card_get_prop5_hi(a->id) != 2) {
        z = a->zone;
        if (z == 0xB) {
            n = duel_pick_st_zone(a->player);
            duel.players[a->player & 1].field[n].faceup = 1;
            duel.players[a->player & 1].field[n].card = *duel_card_by_uid(a->uid);
            a->zone = n;
            r = duel_0884E9E8(a, b);
            a->zone = 0xB;
            duel_bzero(&duel.players[a->player & 1].field[n], 0x14);
            return r;
        }
        if (z < 10) {
            duel.players[a->player & 1].field[z].faceup = 1;
            r = duel_0884E9E8(a, b);
            duel.players[a->player & 1].field[a->zone].faceup = 0;
            return r;
        }
    }
    return duel_0884E9E8(a, b);
}

/* Whether the face-up card id in zone z of player p can be activated now. */
int duel_0886AE08(unsigned int p, int z, int id)
{
    DuelAct act = { 0 };

    act.id = id;
    act.player = p;
    act.zone = z;
    act.uid = CARD_UID(duel.players[p & 1].field[z].card);
    act.kind = 0;
    return duel_0886981C(&act, 0);
}

/* Same for a set card. */
int duel_0886AF00(unsigned int p, int z, int id)
{
    DuelAct act = { 0 };

    act.id = id;
    act.player = p;
    act.zone = z;
    act.uid = CARD_UID(duel.players[p & 1].field[z].card);
    act.kind = 0;
    return duel_08869ACC(&act, 0);
}

/* Same for card i of the hand. */
int duel_0886AFF8(unsigned int p, int i, int id)
{
    DuelAct act = { 0 };

    act.id = id;
    act.player = p;
    act.zone = 0xB;
    act.uid = CARD_UID(duel.players[p & 1].hand[i]);
    act.kind = 0;
    return duel_08869ACC(&act, 0);
}

/* Same for an action packed in a word: id | zone << 16 | b3_4 << 21 | opp << 24 | kind << 25 | player << 31. */
int duel_0886B0E4(unsigned int w, int uid, int x)
{
    DuelAct act = { 0 };

    act.idx = x;
    act.player = (w & 0x80000000) >> 31;
    act.opp = (w & 0x1000000) >> 24;
    act.b3_4 = (w & 0xE00000) >> 21;
    act.zone = (w & 0x1F0000) >> 16;
    act.id = w;
    act.kind = (w & 0x7E000000) >> 25;
    act.uid = uid;
    return duel_0886AAB4(&act, 0);
}

#ifdef NON_MATCHING /* só o primeiro teste da lista final de ids sai beql (com o li 1 no atraso) em vez de beq+nop; demais palavras iguais */
/* Whether the card of a may be activated again (spells and traps that stay on the field). */
int duel_0886B1F4(DuelAct *a)
{
    if (a->b4_1) {
        return 0;
    }
    if (duel_0884ECD4(a) == 0) {
        return 0;
    }
    if (a->b4_2) {
        switch (card_get_prop5_hi(a->id)) {
        case 2:
        case 3:
            if (a->b3_4 != 0 && a->id != 0x1909) {
                return 0;
            }
            break;
        default:
            switch (a->id) {
            case 0x13F0:
            case 0x1103:
            case 0x1243:
            case 0x1432:
            case 0x14B2:
            case 0x16A2:
            case 0x180F:
            case 0x184B:
            case 0x18D3:
            case 0xFEE:
            case 0x1238:
            case 0x1231:
            case 0x1514:
            case 0x12DE:
            case 0x13EB:
            case 0x12D2:
            case 0x1710:
            case 0x173F:
            case 0x17F6:
            case 0x1973:
            case 0x166C:
                if (a->b3_4 != 0) {
                    return 0;
                }
                break;
            default:
                return 0;
            }
            break;
        }
    }
    switch (card_get_prop5_hi(a->id)) {
    case 2:
    case 3:
    case 4:
        break;
    default:
        return 1;
    }
    switch (a->id) {
    case 0x146F:
    case 0x150D:
    case 0x15EE:
    case 0x17AF:
    case 0x1232:
    case 0x19D8:
        break;
    default:
        if (a->b3_4 != 2 && a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
            return 0;
        }
    }
    return 1;
}
#endif

/* Whether no pending chain entry of the same kind already comes from the card that caused a. */
int duel_0886B4E8(DuelAct *a)
{
    unsigned int k = a->kind & 0xFFFF;
    unsigned int uid;
    unsigned int i;

    switch (k) {
    case 6:
    case 7:
    case 8:
    case 11:
    case 12:
    case 22:
    case 27:
    case 25:
    case 26:
    case 29:
    case 28:
    case 30:
        uid = a->idx & 0x1FF;
        break;
    default:
        return 0;
    }
    for (i = 0; i < duel_w_08914920; i++) {
        if (duel_chain.kinds[i] == k) {
            switch (duel_chain.kinds[i]) {
            case 6:
            case 7:
            case 8:
            case 11:
            case 12:
            case 22:
            case 27:
            case 25:
            case 26:
            case 29:
            case 28:
            case 30:
                if (uid == (*(int *)&duel_chain.srcs[i] & 0x1FF)) {
                    return 0;
                }
                break;
            }
        }
    }
    return 1;
}

#ifdef NON_MATCHING /* forma quase igual (diff estrutural só de ordem); alocação dos oito $s diferente */
/* Fills f[12] with the restrictions on activations in force (per player: f[0..1] spells/traps,
   f[2..3], f[4..5] min order, f[6], f[7..11] single cards on the field). */
void duel_0886B6B8(int *f)
{
    int best;
    int q;
    int z;
    int side;
    int other;
    int bits;
    int id;
    int m;
    DuelPlayer *pl;
    int *g;

    f[6] = 0;
    f[0] = duel_slot_effect_with(0, 0xB, 0x1679, 2) | duel_slot_has_effect(1, 0xB, 0x1840);
    f[1] = duel_slot_effect_with(1, 0xB, 0x1679, 2) | duel_slot_has_effect(0, 0xB, 0x1840);
    f[2] = duel_slot_effect_with(0, 0xB, 0x1679, 2);
    f[3] = duel_slot_effect_with(1, 0xB, 0x1679, 2);
    f[4] = duel_slot_effect_with(0, 0xB, 0x1679, 1);
    f[5] = duel_slot_effect_with(1, 0xB, 0x1679, 1);
    f[7] = duel_slot_has_effect(0, 0xB, 0x1351);
    f[8] = duel_slot_has_effect(0, 0xB, 0x1350);
    f[9] = duel_slot_has_effect(0, 0xB, 0x134F);
    f[10] = duel_slot_has_effect(0, 0xB, 0x134E);
    f[11] = duel_slot_has_effect(0, 0xB, 0x12BF);
    if (duel_count_active(0x166C)) {
        m = duel_min_order_both(0x166C);
        if (f[4] == 0) {
            f[4] = m;
        }
        if (f[5] == 0) {
            f[5] = m;
        }
    }
    best = 0;
    for (q = 0, bits = 0; q < 2; q++, bits += 16) {
        pl = &duel.players[q & 1];
        other = 1 - q;
        for (z = 0; z <= 4; z++) {
            if (field_get_card(q, z) != 0x1955) {
                continue;
            }
            if (pl->field[z].faceup == 0 || (pl->field[z].flags >> 1 & 1) || (duel_w_08917DB8 & 1 << (z + bits))) {
                continue;
            }
            if (f[11] && pl->field[z].pos != 0) {
                continue;
            }
            if (duel_field_protected(q, z) || duel_slot_count_src(q, z, 0x1A6D) || duel_count_monsters(other) != 3) {
                continue;
            }
            if (best == 0 || pl->field[z].unk4 < best) {
                best = pl->field[z].unk4;
                side = other;
            }
        }
    }
    if (best != 0) {
        if (f[side + 4] == 0 || best < f[side + 4]) {
            f[side] = f[side + 2] = f[side + 4] = 1;
        }
    }
    for (q = 0, bits = 0; q < 2; q++, bits += 16) {
        if (f[q + 4] != 0) {
            continue;
        }
        pl = &duel.players[q & 1];
        g = &f[1 - q];
        for (z = 0; z <= 4; z++) {
            id = field_get_card(q, z);
            if (id == 0) {
                continue;
            }
            if (pl->field[z].faceup == 0 || (pl->field[z].flags >> 1 & 1) || (duel_w_08917DB8 & 1 << (z + bits))) {
                continue;
            }
            if (f[11] && pl->field[z].pos != 0) {
                continue;
            }
            if (duel_field_protected(q, z) || duel_slot_count_src(q, z, 0x1A6D) || duel_slot_has_effect(q, z, 0x1944)) {
                continue;
            }
            switch (id) {
            case 0x1296:
                if (duel_slot_affected(q, z, 0x12D3)) {
                    *g = 1;
                } else {
                    f[0] = f[1] = 1;
                }
                break;
            case 0x14DA:
                f[6] = 1;
                break;
            case 0x15DA:
            case 0x1816:
                f[2] = f[3] = 1;
                break;
            case 0x19C6:
                f[7] = f[8] = 1;
                break;
            }
        }
    }
    if (f[8] == 0) {
        for (q = 0; q < 2; q++) {
            if (f[q + 2] == 0 && duel_find_effect_card(q, 0x1760, -1)) {
                f[0] = f[1] = 1;
            }
        }
    }
    if (f[7] == 0) {
        for (q = 0; q < 2; q++) {
            if (f[q] == 0 && duel_card_check_9148(q, 0x1302)) {
                if (f[0] == 0) {
                    f[0] = 2;
                }
                if (f[1] == 0) {
                    f[1] = 2;
                }
            }
        }
        for (q = 0; q < 2; q++) {
            if (f[q] != 0) {
                continue;
            }
            if (duel_card_check_9148(q, 0x1360)) {
                f[2] = f[3] = 1;
            }
            if (duel_card_check_9148(q, 0x1765)) {
                f[2] = f[3] = 1;
            }
            if (duel_card_check_9148(q, 0x1495)) {
                f[10] = 1;
            }
            if (duel_card_check_9148(q, 0x148E)) {
                f[6] = 1;
            }
        }
    }
}
#endif

#ifdef NON_MATCHING /* usa $fp e guarda f[]/g na pilha; o mwcc distribui os $s de outro jeito (tamanho quase igual) */
/* Recomputes the "negated" flag (bit 5) of every face-up card on the field from the restrictions of
   duel_0886B6B8 and the cards that negate others; returns whether any flag changed. */
int duel_0886BCDC(void)
{
    int changed = 0;
    int *g;
    int q;
    int z;
    int id;
    int neg;
    int r;
    int y;
    int cnt;
    DuelPlayer *pl;
    DuelPlayer *rp;
    int f[12];

    duel_0886B6B8(f);
    g = f;
    for (q = 0; q < 2; q++, g++) {
        pl = &duel.players[q & 1];
        for (z = 0; z < 11; z++) {
            id = field_get_card(q, z);
            neg = 0;
            if (id == 0 || pl->field[z].faceup == 0 || (pl->field[z].flags >> 1 & 1) || card_is_in_group_147e(id)) {
                goto check;
            }
            if (z < 5 && card_attribute_to_group(id) && !duel_slot_has_effect(q, z, 0x1A5C)) {
                switch (id) {
                case 0x1437:
                case 0x194E:
                case 0x19C3:
                    if (g[4] != 0) {
                        neg = g[4] < pl->field[z].unk4;
                    }
                    break;
                default:
                    if (g[4] != 0) {
                        neg = 1;
                    }
                    if (f[11] && pl->field[z].pos != 0) {
                        neg = 1;
                    }
                    break;
                }
                if (duel_field_protected(q, z)) {
                    neg = 1;
                }
                if (pl->field[z].card.b21) {
                    neg = 1;
                }
                if (card_get_special_group(id) == 1) {
                    if (f[6]) {
                        neg = 1;
                    }
                    if (duel_slot_has_effect(q, z, 0x1433)) {
                        neg = 1;
                    }
                }
                if (duel_slot_has_effect(q, z, 0x1944)) {
                    neg = 1;
                }
                if (duel_slot_count_src(q, z, 0x1A6D)) {
                    neg = 1;
                }
                if (card_get_type(id) == 9 && duel_slot_affected(q, z, 0x1A9B)) {
                    neg = 1;
                }
            }
            if (z >= 5 || duel_field_flag9(q, z)) {
                if (card_get_type(id) == 0x17 && !(pl->field[z].flags >> 8 & 1)) {
                    if (*g == 2) {
                        if (id != 0x1302) {
                            neg = 1;
                        }
                    } else if (*g == 1) {
                        neg = 1;
                    }
                    if (f[7] && card_get_prop5_hi(id) == 4) {
                        neg = 1;
                    }
                } else {
                    if (g[2]) {
                        neg = 1;
                    }
                    if (f[8] && card_get_prop5_hi(id) == 4) {
                        neg = 1;
                    }
                }
                if (f[10] && duel_card_fits_zone(q, z, id)) {
                    neg = 1;
                }
                if (f[9] && z == 10) {
                    neg = 1;
                }
                if (duel_slot_has_effect(q, z, 0x17C2)) {
                    neg = 1;
                }
                if (neg == 0) {
                    for (r = 0; r < 2; r++) {
                        cnt = duel_count_monster_active_atk(r, 0x1A07);
                        rp = &duel.players[r & 1];
                        for (y = 0; y <= 4; y++) {
                            switch (field_get_card(r, y)) {
                            case 0x1812:
                                if (r != q && card_get_type(id) != 0x17 && SLOT_ACTIVE(&rp->field[y]) && duel_slot_has_src(q, z, r, y)) {
                                    neg = 1;
                                }
                                break;
                            case 0x172E:
                                if (r != q && SLOT_ACTIVE(&rp->field[y]) && duel_slot_has_src(q, z, r, y)) {
                                    neg = 1;
                                }
                                break;
                            case 0x1A1D:
                                if (rp->field[y].link == 0x1A2F && SLOT_ACTIVE(&rp->field[y]) && duel_slot_has_src(q, z, r, y)) {
                                    neg = 1;
                                }
                                break;
                            }
                            if (cnt && rp->field[y].card.id != 0 && (unsigned short)duel_field_type(r, y) == 7 && duel_slot_has_src(q, z, r, y)) {
                                neg = 1;
                            }
                        }
                    }
                }
            }
            if (pl->field[z].flags >> 6 & 1) {
                neg = 1;
            }
        check:
            if (neg != (pl->field[z].flags >> 5 & 1)) {
                duel_088CBB64(q, z, neg);
                changed = 1;
            }
        }
    }
    return changed;
}
#endif

/* Whether the restrictions of duel_0886B6B8 forbid player q to activate card id. */
int duel_0886C2E0(int q, int id)
{
    int f[12];

    duel_0886B6B8(f);
    if (card_is_in_group_147e(id)) {
        return 0;
    }
    if (card_get_type(id) == 0x17) {
        if (f[q] == 2 && id != 0x1302) {
            return 1;
        }
        if (f[q] == 1) {
            return 1;
        }
        if (f[7] && card_get_prop5_hi(id) == 4) {
            return 1;
        }
    } else {
        if (f[q + 2]) {
            return 1;
        }
        if (f[8] && card_get_prop5_hi(id) == 4) {
            return 1;
        }
        if (f[10] && (card_get_prop5_hi(id) == 3 || card_is_monster(id))) {
            return 1;
        }
        if (f[9] && card_get_prop5_hi(id) == 2) {
            return 1;
        }
    }
    return 0;
}

/* Whether the activation of a is negated by a card on the field (1). */
int duel_0886C448(DuelAct *a)
{
    int loc;
    int q;
    int z;
    int i;
    int m;

    if (card_is_monster(a->id) && card_attribute_to_group(a->id)) {
        loc = duel_card_locate(a->uid);
        if (card_get_special_group(a->id) == 1) {
            if (duel_count_active(0x148E)) {
                return 1;
            }
            if (duel_count_active(0x14DA)) {
                return 1;
            }
            if ((unsigned char)((loc & 0xFFFF) >> 8) < 5 && a->b3_4 == 1) {
                q = loc >> 16 & 0xFFFF;
                if (q != 0) {
                    if (duel_special_has_effect(a->player, a->zone, 0x1433)) {
                        return 1;
                    }
                }
                if ((unsigned short)q == 0) {
                    if (duel_slot_has_effect(a->player, a->zone, 0x1433)) {
                        return 1;
                    }
                }
            }
        }
        if (a->b3_4 != 2) {
            z = (unsigned char)((loc & 0xFFFF) >> 8);
            if (z < 5) {
                i = loc >> 16 & 0xFFFF;
                q = loc & 0xFF;
                if (duel_zone_faceup(q, z, i)) {
                    if (duel_count_active(0x166C)) {
                        return 1;
                    }
                    if (duel_slot_has_effect(0, 0xB, 0x12BF) && duel_zone_pos(q, z, i)) {
                        return 1;
                    }
                    if ((unsigned short)i != 0) {
                        if (duel_special_has_effect(q, z, 0x1944)) {
                            return 1;
                        }
                        if (duel_special_affected(q, z, 0x1A9B)) {
                            return 1;
                        }
                        if (duel_special_count_src(q, z, 0x1A6D)) {
                            return 1;
                        }
                    } else {
                        if (duel_slot_has_effect(q, z, 0x1944)) {
                            return 1;
                        }
                        if (duel_slot_affected(q, z, 0x1A9B)) {
                            return 1;
                        }
                        if (duel_slot_count_src(q, z, 0x1A6D)) {
                            return 1;
                        }
                    }
                }
            }
        }
        if (a->zone <= 4 || a->zone > 10) {
            if (duel_slot_effect_with(a->player, 0xB, 0x1679, 1)) {
                return 1;
            }
        }
        if (loc != 0x1000) {
            if (duel_field_card_deb2c(loc & 0xFF, (unsigned char)((loc & 0xFFFF) >> 8), loc >> 16 & 0xFFFF)->b21) {
                return 1;
            }
        }
    } else if (card_get_type(a->id) == 0x17) {
        loc = duel_card_locate(a->uid);
        if (loc != 0x1000) {
            if (duel_field_card_deb2c(loc & 0xFF, (unsigned char)((loc & 0xFFFF) >> 8), loc >> 16 & 0xFFFF)->b21) {
                return 1;
            }
        }
    }
    if (a->zone < 11) {
        if (duel_count_monsters_special(a->player) == 3 && duel_find_effect_card(1 - a->player, 0x1955, -1)) {
            return 1;
        }
    }
    if (!card_is_monster(a->id)) {
        if (duel_slot_effect_with(a->player, 0xB, 0x1679, 2)) {
            return 1;
        }
    }
    if ((a->zone >= 5 && a->zone <= 10) || a->b3_4 == 0) {
        if (duel_0886C2E0(a->player, a->id)) {
            return 1;
        }
    }
    if (duel_count_active(0x159D) && a->id != 0x15AC) {
        m = duel_act_e50ec(a);
        if (m & 1) {
            if (duel_find_effect_card(a->player, 0x158B, -1) == 0) {
                return 1;
            }
        }
        if (m & 2) {
            if (duel_find_effect_card(1 - a->player, 0x158B, -1) == 0) {
                return 1;
            }
        }
    }
    if (a->b3_4 == 2) {
        if (a->zone == 0xE && duel_count_active(0x17B9)) {
            return 1;
        }
    }
    return 0;
}

/* Whether player p has fewer than n cards in hand usable for mode (0: any, 7: type 0x16, 8: type 0x17,
   other: monsters), not counting the card uid. */
int duel_0886C99C(unsigned int p, int n, int uid, int mode)
{
    unsigned int i;
    int ok;
    int id;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        id = duel.players[p & 1].hand[i].id;
        ok = id > 0;
        if (uid == CARD_UID(duel.players[p & 1].hand[i])) {
            ok = 0;
        }
        switch (mode) {
        case 0:
            break;
        case 7:
            if (card_get_type(id) != 0x16) {
                ok = 0;
            }
            break;
        case 8:
            if (card_get_type(id) != 0x17) {
                ok = 0;
            }
            break;
        default:
            if (!card_is_monster(id)) {
                ok = 0;
            }
            break;
        }
        if (duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].hand[i]))) {
            ok = 0;
        }
        if (ok) {
            n--;
        }
    }
    return !(n > 0);
}
