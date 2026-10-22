// @range 0x0886191C-0x08861E7C
// @asm 0x088619FC-0x08861BF8
/* The computer player: handlers of the cards it activates (damage, messages). */
#include "duel.h"

int duel_08860A88(DuelAct *a);
int duel_08860FDC(DuelAct *a);
void duel_088C4564(unsigned int p, int n);
void duel_08832710(unsigned int p, int id, int a, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_08832950(unsigned int p, unsigned short id);
int duel_0886B4E8();
int duel_0884E9E8(DuelAct *a, int b);
int duel_08807C7C(DuelAct *a);
void duel_0884AD40(int msg, int uid);
void duel_0884ACD8(unsigned int p, int msg);
int duel_hand_find_uid(unsigned int p, int uid);
void duel_088CABA8(unsigned int p, int i);
void duel_088C7DD4(unsigned int p, int a, int b);
void duel_088328B8(unsigned int p, unsigned short a, int b);
int duel_088630A4();

extern int duel_w_08914904;
extern int duel_w_08919624;
extern unsigned int duel_w_0891964C;
extern int duel_player_type[2];

int duel_0886191C(DuelAct *a)
{
    duel_088C4564(a->player ^ a->opp, duel_08860A88(a));
    return 1;
}

int duel_08861964(DuelAct *a)
{
    int n;

    if (a->kind != 0x23 && (n = duel_08860FDC(a)) > 0) {
        duel_08832710(a->player, n, 0, 0);
    }
    return 1;
}

int duel_088619C4(DuelAct *a)
{
    duel_088C87A8(a->player, a->zone, 0xFB6, 2, 0);
    return 1;
}

#ifdef NON_MATCHING /* o original testa os passos 2 e 1 com beq sem likely e adia o move s2,a0 para o delay slot; o mwcc usa beql e move o a0 antes */
int duel_088619FC(DuelAct *a, int b)
{
    unsigned int q = a->player ^ a->opp;
    int m;

    switch (duel_w_08914904) {
    case 0:
        if (duel_0886B4E8() != 0) {
            duel_0884ACD8(q, 0x107);
            return -1;
        }
        if (duel_0884E9E8(a, b) == 0) {
            return -1;
        }
        if (duel_player_type[q] == 1) {
            duel_w_08919624 = duel_08807C7C(a);
        } else {
            m = 0xFA;
            switch (a->id) {
            case 0x10D6:
                m = 0x17;
                break;
            case 0x10DF:
                m = 0x18;
                break;
            case 0x10E7:
                m = 0x18;
                break;
            case 0x1294:
                m = 0x2F;
                break;
            }
            duel_0884AD40(m, a->uid);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_w_08919624 != 0) {
            return 1;
        }
        switch (a->id) {
        case 0x17C9:
        case 0x17CC:
        case 0x190A:
        case 0x1A28:
        case 0x18EC:
            break;
        default:
            duel_08832950(1 - q, 0x10D6);
        }
        duel_w_08914904++;
        return 0;
    case 2:
        break;
    default:
        return 1;
    }
    return -1;
}
#endif

int duel_08861BF8(DuelAct *a)
{
    int i = duel_hand_find_uid(a->player, a->uid);

    if (i >= 0) {
        duel_088CABA8(a->player, i);
    }
    return 1;
}

int duel_08861C4C(DuelAct *a)
{
    duel_088C7DD4(duel_w_0891964C, 0x11, 1);
    return 1;
}

int duel_08861C78(DuelAct *a)
{
    unsigned int p = a->player;

    duel_088328B8(p, p | a->zone << 8, 1);
    return 1;
}

int duel_08861CB4(DuelAct *a, int b)
{
    int r;

    switch (duel_w_08914904) {
    default:
        r = duel_088619FC(a, b);
        if (r == 1) {
            duel_w_08914904 = 10;
            return 0;
        }
        return r;
    case 10:
        duel_088328B8(a->player, 0xFFFF, 1);
        duel_w_08914904++;
        return 0;
    case 11:
        return 1;
    }
}

int duel_08861D44(DuelAct *a, int b)
{
    unsigned int q = a->player ^ a->opp;
    int n = duel_08860A88(a);
    int r;

    if (duel.players[q & 1].lp <= n) {
        return -1;
    }
    r = duel_088619FC(a, b);
    if (r == 1) {
        duel_088C4564(q, n);
    }
    return r;
}

int duel_08861DFC(DuelAct *a)
{
    duel_088C87A8(a->player, a->zone, 0x1119, 3, 0);
    return 1;
}

int duel_08861E34(DuelAct *a, int b)
{
    int r = duel_088619FC(a, b);

    if (r == 1) {
        return duel_088630A4(a, b);
    }
    return r;

}
