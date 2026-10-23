// @range 0x08867F54-0x08869190
// @asm 0x08868910-0x08868A54
// @asm 0x08868F80-0x08869190
// @data 0x08911760-0x08911770
/* Card effect handlers (DuelAct *a): step machines on duel_w_08914904 (draws, checks by card id). */
#include "duel.h"

int card_get_attribute_icon(int id);
int card_is_monster(int id);
void duel_08832734(unsigned int p, int id, int a);
int duel_088619C4(DuelAct *a, int b);
void duel_0884ACD8(unsigned int p, int msg);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
DuelCard *duel_0884CCBC(int i);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
int duel_count_free_st_zones(unsigned int p);
int duel_0884E5E0(DuelAct *a);
int duel_0887A828(DuelAct *a, int b);
int duel_0887B2B8(DuelAct *a);
int duel_0886CFE0(DuelAct *a, int b);
int duel_0887CCA8();
int duel_088619FC(DuelAct *a, int b);
void duel_088CC4B4(unsigned int p, int z, int n);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);

int duel_08835AC0(void);
void duel_088359E8(unsigned int p, int id, int (*fn)());
int duel_088537B0(int id);
int duel_host_call(int a, int b, int c, int d);
void duel_088D1B20(unsigned int p, unsigned int mask);
int duel_088657D8(DuelAct *a, int b);
int duel_0886CD1C(DuelAct *a, unsigned int p);
int duel_phase_ok2(unsigned int p);
int duel_grave_find_name(unsigned int p, int id);
int duel_0884E690(DuelAct *a, int (*fn)());
int duel_0885660C(DuelAct *a, unsigned int p, int z);
int duel_0886CE38();
int duel_08861480(unsigned short id, int i);
void duel_08832974(unsigned int p, unsigned short id, int a);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
int duel_hand_find_uid(unsigned int p, int uid);
void duel_088CAE34(unsigned int p, int a);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(int id));
void duel_08835978(int (*fn)(int id));
int duel_0887FCA8(int id);
unsigned int duel_field_type(unsigned int p, int z);
void duel_088C44E4(unsigned int p, int id);
unsigned int duel_card_locate(unsigned int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
int duel_0886CF1C(DuelAct *a, int b);
void duel_0884AD40(int msg, int uid);
int duel_088BFFD8(unsigned int p, int id, int i);
void duel_088C77EC(unsigned int p, int uid);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
int duel_08827DA4(unsigned int p, int uid, int a, int b);
int duel_0884B010(int a, unsigned short b, unsigned int c, unsigned int d, unsigned int e);
void duel_088C4564(unsigned int p, int n);

extern int duel_w_08914904;
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern int duel_player_type[2];
extern unsigned int duel_w_08919624;

int duel_08867F54(DuelAct *a, DuelAct *b)
{
    int n = card_get_attribute_icon(b->id);

    if (card_is_monster(b->id) && b->zone >= 5 && b->zone < 11) {
        n = 7;
    }
    switch (duel_w_08914904) {
    case 0:
        duel_08832734(a->player, 1, n);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_088619C4(a, (int)b);
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

/* Draws 2 cards (for 0x18F8: the counter of its zone, at most 0x20). */
int duel_08868038(DuelAct *a)
{
    int n;
    unsigned int i;

    n = 2;
    switch (a->id) {
    case 0x18F8:
        n = duel.players[a->player & 1].field[a->zone].link;
        break;
    }
    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(a->player, 0x56);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_0884C588(a->player, MIN(n, 0x20) + 6, a->id, 0);
        duel_w_08914904++;
        return 0;
    case 2:
        duel_088C7BA4();
        for (i = 0; i < duel_w_08919624; i++) {
            duel_088CB400(a->player, duel_0884CCBC(i + 1), 0);
        }
        duel_088C7CF0();
        return 1;
    }
    return 1;
}

int duel_088681A8(DuelAct *a, int b)
{
    switch (a->id) {
    case 0x13E3:
        if (duel_count_free_st_zones(a->player) == 0) {
            return -1;
        }
        if (duel_0884E5E0(a) == 0) {
            return -1;
        }
        break;
    case 0x1445:
        if (duel.players[a->player & 1].n_d == 0) {
            return -1;
        }
        break;
    case 0x190A:
        if (duel_0887A828(a, b) == 0) {
            return -1;
        }
        break;
    case 0x1951:
        if (duel_0887B2B8(a) == 0) {
            return -1;
        }
        break;
    case 0x197C:
        if (duel_0886CFE0(a, b) == 0) {
            return -1;
        }
        break;
    case 0x1A28:
        if (duel_0887CCA8(a) == 0) {
            return -1;
        }
        break;
    }
    return duel_088619FC(a, b);
}

int duel_088682F4(DuelAct *a, int b)
{
    duel_088619C4(a, b);
    duel_088CC4B4(a->player, a->zone, -1);
    return 1;
}

int duel_08868334(DuelAct *a)
{
    unsigned int p;

    if (a->kind == 0x23) {
        return 1;
    }
    switch (duel_w_08914904) {
    case 0:
        duel_088C7BA4();
        duel_w_08914904++;
        return 0;
    case 1:
        p = a->player;
        if (duel.players[p & 1].n_hand != 0) {
            duel_088CAE24(p, duel.players[p & 1].hand, 0);
            return 0;
        }
        duel_088C7CF0();
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

/* Picks 2 cards of the field (3 for 0x195C), marked in w[0]/w[1]. */
int duel_08868418(DuelAct *a)
{
    int n;
    unsigned int p;
    int z;

    n = 2;
    if (a->kind == 0x23) {
        return 1;
    }
    switch (a->id) {
    case 0x195C:
        n = 3;
        break;
    }
    switch (duel_w_08914904) {
    case 0:
        a->w[1] = 0;
        a->w[0] = 0;
        a->w[2] = 0;
        duel_0884ACD8(a->player, 0x12);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_088359E8(a->player, 1, duel_088537B0);
        duel_w_08914904++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            a->w[p] |= 1 << z;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            if (++a->w[2] < n) {
                duel_w_08914904--;
                return 0;
            }
            duel_088D1B20(a->player, a->w[0] | a->w[1] << 16);
            return 1;
        }
        duel_w_08914904 = 0;
        duel_host_call(0x29, 0, 0, 0);
        return 0;
    }
    return 0;
}

int duel_08868614(DuelAct *a, int b)
{
    if (a->kind == 0xF) {
        return duel_088657D8(a, b);
    }
    return 1;
}

int duel_08868648(DuelAct *a)
{
    unsigned int p = a->player ^ a->opp;

    if (duel_0886CD1C(a, p) < 2) {
        return -1;
    }
    if (duel_phase_ok2(p)) {
        return 1;
    }
    return -1;
}

static const int ids_088686B0[4] = { 0x1A0F, 0x1A10, 0x1A11, 0x1A12 };

/* Adds to the hand one copy of each of 0x1A0F-0x1A12 from the graveyard. */
int duel_088686B0(DuelAct *a)
{
    unsigned int i;
    int k;
    unsigned int p;

    if (a->kind == 0x23) {
        return 1;
    }
    duel_088C7BA4();
    for (i = 0; i < 4; i++) {
        k = duel_grave_find_name(a->player, ids_088686B0[i]);
        if (k >= 0) {
            p = a->player;
            duel_088CB400(p, &duel.players[p & 1].grave[k], 0);
        }
    }
    duel_088C7CF0();
    return 1;
}

int duel_0886877C(DuelAct *a, int b)
{
    int m;
    int x;
    int y;

    switch (duel_w_08914904) {
    case 0:
        m = 1;
        if (duel_0884E690(a, duel_0885660C)) {
            m |= 2;
        }
        if (duel_0886CE38(a, b)) {
            m |= 4;
        }
        if (duel_player_type[a->player] == 1) {
            if (m & 4) {
                duel_w_08919624 = 2;
            } else if (m & 2) {
                duel_w_08919624 = 1;
            } else {
                duel_w_08919624 = 0;
            }
        } else {
            x = duel_08861480(a->id, 0);
            y = duel_08861480(a->id, 1);
            duel_0884ADAC(0x103, x, y, duel_08861480(a->id, 2), 0, m);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        a->w[2] = duel_w_08919624 + 1;
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        duel_w_08914904++;
        return 0;
    case 2:
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* o original carrega duel_w_08919654 antes de duel_w_0891964C para a chamada final; nenhuma grafia inverteu a ordem */
int duel_08868910(DuelAct *a)
{
    int k;

    switch (duel_w_08914904) {
    case 0:
        k = duel_hand_find_uid(a->player, a->uid);
        if (k >= 0) {
            duel_088CAE34(a->player, k);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        duel_0884ACD8(a->player, 0x3A);
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, 2, duel_0887FCA8);
        } else {
            duel_08835978(duel_0887FCA8);
        }
        duel_w_08914904++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            k = duel_w_08919654;
            duel_088CAE34(duel_w_0891964C, k);
            return 1;
        }
        duel_w_08914904--;
        return 0;
    }
    return 0;
}
#endif

/* w[0] = number of monster zones of type 0x11 after the effect. */
int duel_08868A54(DuelAct *a, int b)
{
    int r;
    int z;
    int n;

    if (duel_0887CCA8() == 0) {
        return -1;
    }
    r = duel_088619FC(a, b);
    if (r == 1) {
        n = 0;
        duel_088C44E4(a->player, a->id);
        for (z = 0; z < 5; z++) {
            if ((unsigned short)duel_field_type(a->player, z) == 0x11) {
                n++;
            }
        }
        duel_088D1B20(a->player, 0x1F << (a->player * 16));
        a->w[0] = n;
    }
    return r;
}

int duel_08868B28(DuelAct *a, int b)
{
    DuelSrc *s;
    int loc;
    int q;
    int i;
    int r;

    if (duel_0886CE38(a, b) == 0) {
        return -1;
    }
    s = &a->src;
    loc = duel_card_locate(s->uid);
    q = loc & 0xFF;
    i = loc >> 16 & 0xFFFF;
    if ((unsigned char)((loc & 0xFFFF) >> 8) == 0xE) {
        if (card_is_monster(duel.list[s->uid].id) && duel_grave_card_usable(a->player, q, i)) {
            r = duel_088619FC(a, b);
            if (r == 1) {
                duel_088619C4(a, b);
            }
            return r;
        }
    }
    return -1;
}

/* Picks a card of the hand accepted by duel_088BFFD8 (directly when there is only one). */
int duel_08868C28(DuelAct *a, int b)
{
    unsigned int i;
    int last;
    int n;
    unsigned int p;
    unsigned int q;

    switch (duel_w_08914904) {
    case 0:
        if (duel_0886CF1C(a, b) == 0) {
            return -1;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_0884AD40(0xFA, a->uid);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_w_08919624 == 0) {
            return -1;
        }
        n = 0;
        for (i = 0; i < duel.players[(q = a->player) & 1].n_hand; i++) {
            if (duel_088BFFD8(q, a->id, i)) {
                last = i;
                n++;
            }
        }
        if (n == 0) {
            return -1;
        }
        if (n == 1) {
            a->w[0] = CARD_UID(duel.players[(unsigned char)q & 1].hand[last]);
            duel_w_08914904 = 4;
            return 0;
        }
        duel_w_08914904++;
        return 0;
    case 2:
        duel_0884ACD8(a->player, 0x11E);
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, a->id, duel_0887FCA8);
        } else {
            duel_08835978(duel_0887FCA8);
        }
        duel_w_08914904++;
        return 0;
    case 3:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            a->w[0] = CARD_UID(duel.players[p & 1].hand[duel_w_08919654]);
            duel_w_08914904++;
            return 0;
        }
        duel_w_08914904--;
        return 0;
    case 4:
        duel_088C77EC(a->player, a->w[0]);
        duel_088C77D4(a->player, a->id, 5, a->uid);
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* o original guarda pl e o valor em pilha e usa s8; o mwcc aloca tudo em registradores (forma igual) */
/* Pays 1000, 2000 or 3000 life points: k * 1000 for the k that the cards of the hand allow. */
int duel_08868F80(DuelAct *a)
{
    unsigned int p = a->player ^ a->opp;
    DuelPlayer *pl;
    int v;
    unsigned int mask;
    int lo;
    int hi;
    int k;
    unsigned int i;

    switch (duel_w_08914904) {
    case 0:
        v = 1000;
        mask = 0;
        lo = 0;
        hi = 0;
        pl = &duel.players[p & 1];
        for (k = 1; k < 4; k++, v += 1000) {
            if (!(v < pl->lp)) {
                break;
            }
            for (i = 0; i < pl->n_hand; i++) {
                if (duel_08827DA4(p, CARD_UID(pl->hand[i]), 1, k)) {
                    mask |= 1 << k;
                    if (lo == 0) {
                        lo = k;
                    }
                    hi = k;
                }
            }
        }
        if (mask == 0) {
            return 1;
        }
        if (hi == lo || duel_player_type[p] == 1) {
            duel_w_08919624 = hi;
        } else {
            duel_0884B010(0x12D, 0x12E, lo, hi, (mask == 0xA) + 1);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        a->w[0] = duel_w_08919624;
        duel_088C4564(p, a->w[0] * 1000);
        return 1;
    }
    return 1;
}
#endif
