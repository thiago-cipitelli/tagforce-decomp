// @range 0x088C134C-0x088C2F80
// @asm 0x088C1B88-0x088C1D80
// @asm 0x088C28A4-0x088C2A08
// @asm 0x088C2BA0-0x088C2CA8
/* Effects resolved at a phase change (after effects_088C004C.c): one handler per card id, each scanning
   both fields for the cards it affects. */
#include "duel.h"

int duel_count_active(int id);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_ref_immune(unsigned int p, int z, int opp);
void duel_088D1BEC(unsigned int p, int z, int a, int b);
void duel_088C448C(int p, int a);
void duel_088C4448(unsigned int p, int a, unsigned int uid);
int duel_088C4730(unsigned int p, int n, int f, int id);
void duel_088C4564(unsigned int p, int n);
void duel_088CC178(unsigned int p, int z, unsigned int v);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_slot_affected(unsigned int p, int z, int id);
int duel_count_same_name_other(unsigned int p, int z);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
unsigned int duel_088D13D8(unsigned int p, unsigned short id, unsigned int mask);
void duel_088CC1A0(unsigned int p, int z, unsigned int v);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_field_same_name(unsigned int p, int z, int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
void duel_088C89BC(unsigned int p, int z, int id, int c);
int duel_field_attr_is(unsigned int p, int z, int attr);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088C5424(unsigned int p, int n);
unsigned int duel_field_flag9(unsigned int p, int z);
unsigned int duel_slot_effect_arg(unsigned int p, int z, int id);
void duel_088D147C(DuelAct *a, unsigned int p, int z);
void duel_088D1DA4(DuelAct *a, unsigned int p, int z, unsigned int q, int y);
int card_get_prop5_hi(int id);
int duel_pick_st_zone(unsigned int p);
int duel_card_check_9148(unsigned int p, int id);
int duel_count_faceup_name(unsigned int p, int id);
unsigned short duel_slot_e1854(unsigned int p, int z);
void duel_088C88B8(unsigned int p, int z, int v, int c);
int duel_slot_value_1909(unsigned int p, int z);
void duel_088CC1D0(unsigned int p, int z, int a);
int duel_hand_find_uid(unsigned int p, int uid);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
void duel_088C44E4(unsigned int p, int id);
void duel_088C7478(unsigned int p, int i, DuelCard *c, int a);
void duel_088C8AD0(unsigned int p, int z, int id, int v);
extern unsigned int duel_turn_player;
extern unsigned int duel_w_08917DB4;
extern unsigned int duel_w_0891960C;

int duel_088C134C(void)
{
    unsigned int mask;
    int p;
    int z;
    unsigned int q;
    int n;
    unsigned int b;
    int k;

    if (duel_count_active(0x12FB)) {
        mask = 0;
        for (p = 0; p < 2; p++) {
            for (z = 0; z < 5; z++) {
                if (duel_slot_has_effect(p, z, 0x12EA) && !duel_slot_ref_immune(p, z, duel_find_effect_card(1 - p, 0x12FB, -1) > 0)) {
                    mask |= 1 << (z + p * 16);
                }
            }
        }
        if (mask != 0) {
            duel_088C448C(-1, 0x12FB);
            for (k = 0; k < 2; k++) {
                q = duel_turn_player ^ k;
                n = duel_find_effect_card(1 - q, 0x12FB, -1) > 0;
                for (z = 0, b = q << 4; z < 5; z++, b++) {
                    if (mask & 1 << b) {
                        duel_088D1BEC(q, z, n, 0x12FB);
                    }
                }
            }
            return 1;
        }
    }
    return 0;
}

int duel_088C14C0(void)
{
    int r = 0;
    unsigned int q;
    int z;
    int k;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 5; z < 10; z++) {
            if (duel.players[q & 1].field[z].card.id == 0x1306 && duel.players[q & 1].field[z].link != 0) {
                duel_088CC178(q, z, 0);
                if (SLOT_ACTIVE(&duel.players[q & 1].field[z])) {
                    duel_088C4448(q, z, CARD_UID(duel.players[q & 1].field[z].card));
                    duel_088C4730(1 - q, duel.players[q & 1].field[z].link * 500, 1, 0x1306);
                    r = 1;
                }
            }
        }
    }
    return r;
}

void duel_088C1618(unsigned int p)
{
    int n = duel_count_active(0x132C);
    int i;

    if (n > 0) {
        duel_088C448C(-1, 0x132C);
        for (i = 0; i < n; i++) {
            duel_088C4564(p, 500);
        }
    }
}

int duel_088C168C(void)
{
    int k;
    unsigned int q;
    int z;
    unsigned int mask;
    int p;
    int i;

    if (duel_w_08917DB4 & 1 && duel_w_0891960C < 9) {
        return 0;
    }
    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 0; z < 5; z++) {
            if (!duel_slot_e8170(q, z, 0x1343)) {
                continue;
            }
            {
                mask = 0;
                for (p = 0; p < 2; p++) {
                    for (i = 0; i < 5; i++) {
                        if (duel.players[p & 1].field[i].card.id != 0 && duel.players[p & 1].field[i].faceup != 0 &&
                            !(duel.players[p & 1].field[i].flags >> 1 & 1) && !duel_slot_affected(p, i, 0x1343)) {
                            if (duel_count_same_name_other(p, i)) {
                                mask |= 1 << (i + p * 16);
                            } else {
                                duel_088C87A8(p, i, (unsigned char)q | (unsigned char)z << 8, 9, 0);
                            }
                        }
                    }
                }
                if (mask != 0) {
                    p = duel_turn_player ^ (duel_find_effect_card(duel_turn_player, 0x1343, -1) == 0);
                    duel_088C448C(-1, 0x1343);
                    duel_088D13D8(p, 0x1343, mask);
                    return 1;
                }
                return 0;
            }
        }
    }
    return 0;
}

void duel_088C1884(unsigned int p)
{
    int z;
    int id;

    if (duel_find_effect_card(p, 0x1354, -1)) {
        for (z = 5; z <= 10; z++) {
            id = duel.players[p & 1].field[z].card.id;
            if (id == 0x1354 && SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
                if (!(duel.players[p & 1].field[z].flags >> 4 & 1)) {
                    duel_088C7E68(p, z, 4, 1);
                    duel_088CC178(p, z, 0);
                    duel_0884F30C((id & 0xFFFF) | ((p << 31 & 0x80000000) | 0x3A200000 | ((unsigned int)z << 16 & 0x1F0000)),
                                  CARD_UID(duel.players[p & 1].field[z].card), p & 0xFF);
                } else {
                    duel_088CC1A0(p, z, 1);
                }
            }
        }
    }
}

int duel_088C19F0(void)
{
    int r = 0;
    unsigned int q;
    int z;
    int k;
    unsigned int t;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 5; z < 10; z++) {
            if (duel.players[q & 1].field[z].card.id == 0x1361 && duel.players[q & 1].field[z].link != 0) {
                duel_088CC178(q, z, 0);
                if (SLOT_ACTIVE(&duel.players[q & 1].field[z])) {
                    t = duel_turn_player;
                    duel_088C4448(q, z, CARD_UID(duel.players[q & 1].field[z].card));
                    duel_088C4730(t, (duel.players[q & 1].field[z].link >> (t << 3) & 0xFF) * 300, t != q, 0x1361);
                    t ^= 1;
                    duel_088C4730(t, (duel.players[q & 1].field[z].link >> (t << 3) & 0xFF) * 300, t != q, 0x1361);
                    r = 1;
                }
            }
        }
    }
    return r;
}

#ifdef NON_MATCHING /* no laço do deck o contador e o ponteiro saem com $s2/$s3 trocados; nenhuma ordem de declaração resolve */
void duel_088C1B88(unsigned int p, int z)
{
    int q;
    int i;
    int id;
    int uid;

    if (duel.players[p & 1].field[z].card.id == 0x1368) {
        id = duel.players[p & 1].field[z].link;
        duel_088C89BC(p, z, 0x1368, 1);
        for (q = 0; q < 2; q++) {
            for (i = 0; i <= 10; i++) {
                if (duel_field_same_name(q, i, id)) {
                    duel_088C87A8(p, z, 0x1368, 3, CARD_UID(duel.players[q & 1].field[i].card));
                }
            }
        }
        if (id == 0x12A1) {
            for (q = 0; q < 2; q++) {
                for (i = 0; i < (unsigned int)duel.players[q & 1].n_d; i++) {
                    uid = CARD_UID(duel.players[q & 1].zone_d[i]);
                    if (duel_slot_effect_with(q, 0xB, 0x12A1, uid)) {
                        duel_088C87A8(p, z, 0x1368, 3, uid);
                    }
                }
            }
        }
    }
}
#endif

int duel_088C1D80(unsigned int p)
{
    int z;

    for (z = 0; z <= 4; z++) {
        if (duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup != 0 && !duel_field_attr_is(p, z, 1)) {
            return 1;
        }
    }
    return 0;
}

int duel_088C1E20(void)
{
    unsigned int q;
    int z;
    int k;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 0; z <= 4; z++) {
            if (duel_slot_e8170(q, z, 0x13A4) && duel_088C1D80(q)) {
                duel_088D1438(q, z, 0, 0);
                return 1;
            }
        }
    }
    return 0;
}

int duel_088C1EC8(void)
{
    int r = 0;
    int k;
    unsigned int q;
    int z;
    int id;
    unsigned int link;
    DuelSrc s;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 5; z < 10; z++) {
            id = duel.players[q & 1].field[z].card.id;
            link = duel.players[q & 1].field[z].link;
            if (id == 0x1403 && link != 0) {
                duel_088CC178(q, z, 0);
                if (SLOT_ACTIVE(&duel.players[q & 1].field[z])) {
                    s.player = q;
                    s.uid = link;
                    duel_0884F30C((id & 0xFFFF) | ((q << 31 & 0x80000000) | 0x10200000 | ((unsigned int)z << 16 & 0x1F0000)),
                                  CARD_UID(duel.players[q & 1].field[z].card), *(unsigned int *)&s);
                    r = 1;
                }
            }
        }
    }
    return r;
}

int duel_088C205C(void)
{
    int r = 0;
    unsigned int q;
    int z;
    int k;
    int n;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 5; z < 10; z++) {
            if (duel.players[q & 1].field[z].card.id == 0x1405 && (n = duel.players[q & 1].field[z].link) != 0) {
                duel_088CC178(q, z, 0);
                if (SLOT_ACTIVE(&duel.players[q & 1].field[z])) {
                    duel_088C4448(q, z, CARD_UID(duel.players[q & 1].field[z].card));
                    duel_088C5424(q, n * 500);
                    r = 1;
                }
            }
        }
    }
    return r;
}

int duel_088C21A0(void)
{
    int r = 0;
    unsigned int q;
    int z;
    int k;
    int n;
    int id;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 0; z <= 4; z++) {
            id = field_get_card(q, z);
            n = duel.players[q & 1].field[z].link;
            if (id == 0x144D && n != 0) {
                duel_088CC178(q, z, 0);
                if (SLOT_ACTIVE(&duel.players[q & 1].field[z])) {
                    duel_088C4448(q, z, CARD_UID(duel.players[q & 1].field[z].card));
                    for (k = 0; k < n; k++) { /* the original reuses the player counter here */
                        duel_088C4730(1 - q, 500, 1, 0x144D);
                    }
                    r = 1;
                }
            }
        }
    }
    return r;
}

int duel_088C2300(void)
{
    int k;
    unsigned int q;
    int z;
    int id;
    int t;
    unsigned int v;

    DuelAct a;
    char *c;
    int n;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 0; z <= 4; z++) {
            id = duel.players[q & 1].field[z].card.id;
            if (duel_field_flag9(q, z)) {
                v = duel_slot_effect_arg(q, z, 0x1472);
                if ((v & 0xFF) == q) {
                    t = (unsigned short)v >> 8 & 0xFF;
                    c = (char *)&a;
                    n = 0x18;
                    if (c != 0) {
                        do {
                            *c++ = 0;
                        } while (--n);
                    }
                    a.uid = CARD_UID(duel.players[q & 1].field[z].card);
                    a.player = q;
                    a.zone = z;
                    a.id = id;
                    a.b3_4 = 1;
                    if (t < 0 || duel.players[q & 1].field[t].card.id != 0) {
                        duel_088D147C(&a, q, z);
                        return 1;
                    }
                    if (duel.players[q & 1].field[z].faceup == 0) {
                        duel_088D1DA4(&a, q, z, q, t);
                        duel_088C7E68(q, t, 0x14, 1);
                        return 1;
                    }
                    if (duel.players[q & 1].field[z].flags >> 5 & 1) {
                        if (card_get_prop5_hi(id) == 4) {
                            duel_088D1DA4(&a, q, z, q, t);
                        } else {
                            duel_088D1BEC(q, z, 0, 0);
                        }
                        return 1;
                    }
                } else {
                    duel_088C89BC(q, z, 0x1472, 1);
                    duel_088C87A8(q, z, 0x1472, 5, (unsigned char)q | (unsigned char)(duel_pick_st_zone(q) < 0 ? 0 : duel_pick_st_zone(q)) << 8);
                }
            }
        }
    }
    return 0;
}

int duel_088C25C0(void)
{
    unsigned int q;
    int z;
    int k;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 0; z <= 4; z++) {
            if (duel_slot_e8170(q, z, 0x147A) && !duel_card_check_9148(q, 0x146F)) {
                duel_088D1438(q, z, 0, 0);
                return 1;
            }
        }
    }
    return 0;
}

int duel_088C266C(void)
{
    unsigned int q;
    int z;
    int k;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 0; z <= 4; z++) {
            if (duel_slot_e8170(q, z, 0x1914) && !duel_count_faceup_name(0, 0x1784) && !duel_count_faceup_name(1, 0x1784)) {
                duel_088D1438(q, z, 0, 0);
                return 1;
            }
        }
    }
    return 0;
}

int duel_088C272C(int id)
{
    int k;
    unsigned int q;
    int z;
    int i;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 0; z <= 4; z++) {
            if (duel_slot_e8170(q, z, id)) {
                for (i = 0; i < 5; i++) {
                    if (i != z && id == field_get_card(q, i) && duel.players[q & 1].field[i].faceup != 0) {
                        if (duel.players[q & 1].field[z].unk4 > duel.players[q & 1].field[i].unk4) {
                            duel_088D1438(q, z, 0, 0);
                            return 1;
                        }
                        duel_088D1438(q, i, 0, 0);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* mesmo beqz para o corpo + b fim do teste do bit 5 que em duel_088C07FC; o y do alvo sai com srl em vez de ext */
void duel_088C28A4(unsigned int p, int z, int c)
{
    unsigned short v = duel_slot_e1854(p, z);
    unsigned int tp;
    unsigned int tz;

    if (v != 0xFFFF) {
        tp = v & 0xFF;
        tz = (v & 0xFFFF) >> 8 & 0xFF;
        duel_088CC1D0(p, z, 1);
        if (duel_slot_value_1909(p, z) + 1 >= 3) {
            duel_088C88B8(tp, tz, (unsigned short)((unsigned char)p | (unsigned char)z << 8), 1);
            if (c == 0 && !(duel.players[p & 1].field[z].flags >> 5 & 1)) {
                duel_088C4448(p, z, CARD_UID(duel.players[p & 1].field[z].card));
                duel_088D1438(tp, tz, tp != p, field_get_card(p, z));
            }
        }
    }
}
#endif

int duel_088C2A08(void)
{
    int r = 0;
    unsigned int q;
    int z;
    int k;
    unsigned int t;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 5; z < 10; z++) {
            if (duel.players[q & 1].field[z].card.id == 0x14BF && duel.players[q & 1].field[z].link != 0) {
                duel_088CC178(q, z, 0);
                if (SLOT_ACTIVE(&duel.players[q & 1].field[z])) {
                    t = duel_turn_player;
                    duel_088C4448(q, z, CARD_UID(duel.players[q & 1].field[z].card));
                    duel_088C4730(t, (duel.players[q & 1].field[z].link >> (t << 3) & 0xFF) * 500, t != q, 0x1717);
                    t ^= 1;
                    duel_088C4730(t, (duel.players[q & 1].field[z].link >> (t << 3) & 0xFF) * 500, t != q, 0x1717);
                    r = 1;
                }
            }
        }
    }
    return r;
}

#ifdef NON_MATCHING /* o endereço da zona do alvo sai com os registradores e a ordem do cálculo trocados */
int duel_088C2BA0(void)
{
    unsigned int q;
    int z;
    int k;
    unsigned short v;
    unsigned int tp;
    unsigned int tz;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = 0; z <= 4; z++) {
            if (duel_slot_e8170(q, z, 0x14FC)) {
                v = duel_slot_e1854(q, z);
                if (v != 0xFFFF) {
                    tz = v & 0xFFFF;
                    tp = v & 0xFF;
                    tz = tz >> 8 & 0xFF;
                    if (duel.players[tp & 1].field[tz].card.id != 0 && duel.players[tp & 1].field[tz].faceup == 0) {
                        duel_088D1438(q, z, 0, 0);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}
#endif

int duel_088C2CA8(void)
{
    int r = 0;
    int k;
    unsigned int q;
    unsigned int e;
    int i;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        if (duel_slot_effect_kind(q, 0xB, 0x123B, 2)) {
            e = duel.players[q & 1].effects;
            while (e != 0) {
                DuelEffect *ef = &duel.effects[e];

                e = ef->next;
                if ((ef->kind & 0xF) == 2 && ef->src == 0x123B) {
                    i = duel_hand_find_uid(q, ef->arg);
                    if (i >= 0) {
                        if (r == 0) {
                            duel_088C44E4(1 - q, 0x123B);
                        }
                        duel_088C7478(q, i, &duel.players[q & 1].hand[i], 1);
                        r = 1;
                    }
                    duel_088C8AD0(q, 0xB, 0x123B, ef->arg);
                }
            }
        }
    }
    return r;
}

int duel_088C2E14(void)
{
    int r = 0;
    int k;
    unsigned int q;
    unsigned int e;
    int i;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        if (duel_slot_effect_kind(q, 0xB, 0x188C, 2)) {
            e = duel.players[q & 1].effects;
            while (e != 0) {
                DuelEffect *ef = &duel.effects[e];

                e = ef->next;
                if ((ef->kind & 0xF) == 2 && ef->src == 0x188C) {
                    i = duel_hand_find_uid(q, ef->arg);
                    if (i >= 0) {
                        if (r == 0) {
                            duel_088C44E4(1 - q, 0x188C);
                        }
                        duel_088C7478(q, i, &duel.players[q & 1].hand[i], 1);
                        r = 1;
                    }
                    duel_088C8AD0(q, 0xB, 0x188C, ef->arg);
                }
            }
        }
    }
    return r;
}
