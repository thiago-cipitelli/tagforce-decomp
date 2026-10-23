// @range 0x0886AAB4-0x0886BCDC
// @asm 0x0886B1F4-0x0886B4E8
// @asm 0x0886B6B8-0x0886BCDC
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
