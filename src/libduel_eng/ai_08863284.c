// @range 0x08863284-0x08863780
/* The computer player: handlers of the cards it activates. */
#include "duel.h"

void duel_08832710(unsigned int p, int id, int a, int b);
int duel_088717B4(DuelAct *a, int b);
void duel_0884AD18(int msg);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088CABA8(unsigned int p, int i);
void duel_088C89BC(unsigned int p, int z, int id, int c);
void duel_088D1AC8(unsigned int p, int z);
void duel_088328B8(unsigned int p, unsigned short a, int b);
void duel_088C6264(unsigned int p, int a, int b);
int duel_0886CD24(DuelAct *a, unsigned int p);
void duel_08832870(unsigned int p, int a);
void duel_0884ACD8(unsigned int p, int msg);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
void duel_088359E8(unsigned int p, int id, int (*fn)(DuelAct *a, unsigned int p, int z));
int duel_08835AC0(void);
void duel_088D1CF4(DuelAct *a, unsigned int p, int z);
void duel_088327F8(unsigned int p, int a, int b);

extern int duel_w_08914904;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

int duel_08863284(DuelAct *a)
{
    if (a->kind != 0x23) {
        duel_08832710(a->player, 1, 1, 0);
    }
    return 1;
}

int duel_088632CC(DuelAct *a, int b)
{
    switch (duel_w_08914904) {
    case 0:
        if (a->b3_4 == 3) {
            duel_w_08919624 = 1;
        } else if (duel_088717B4(a, b) == 2) {
            if (duel_player_type[a->player] == 1) {
                duel_w_08919624 = 0;
            } else {
                duel_0884AD18(0x10E);
            }
        } else {
            duel_w_08919624 = 0;
        }
        duel_w_08914904++;
        return 0;
    case 1:
        a->w[0] = duel_w_08919624;
        if (duel_w_08919624 == 0) {
            return 1;
        }
        duel_08832710(a->player, 1, 0, 1);
        return 1;
    }
    return 0;
}

int duel_088633E0(DuelAct *a)
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
        if (duel.players[(p = a->player) & 1].n_hand != 0) {
            duel_088CABA8(p, 0);
            return 0;
        }
        duel_088C7CF0();
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

int duel_088634B4(DuelAct *a)
{
    if (a->b3_4 != 2) {
        duel_088D1AC8(a->player, a->zone);
    } else {
        duel_088C89BC(a->player, 11, a->id, 0);
    }
    return 1;
}

int duel_08863514(DuelAct *a)
{
    duel_088328B8(a->player, 0xFFFF, 2);
    return 1;
}

int duel_08863540(DuelAct *a)
{
    duel_088C6264(a->player, 3, 0);
    return 1;
}

int duel_0886356C(DuelAct *a)
{
    if (duel_0886CD24(a, a->player) < 2) {
        return -1;
    }
    duel_08832870(a->player, 2);
    return 1;
}

int duel_088635BC(DuelAct *a)
{
    unsigned int p;
    int z;

    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(a->player, 0x3A);
        duel_088359E8(a->player, a->id, duel_0884E3E4);
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            duel_w_08914904++;
            return 0;
        }
        duel_w_08914904--;
        return 0;
    case 2:
        p = duel_w_0891964C;
        z = duel_w_08919650 + duel_w_08919654;
        duel_088C7BA4();
        duel_088D1CF4(a, p, z);
        duel_088D1CF4(a, a->player, a->zone);
        duel_088C7CF0();
        return 1;
    }
    return 1;
}

int duel_088636F0(DuelAct *a)
{
    duel_088327F8(a->player, 0, 1);
    return 1;
}

int duel_0886371C(DuelAct *a, unsigned int p, int z)
{
    if (a->player != p) {
        return 0;
    }
    if (z > 4) {
        return 0;
    }
    return duel.players[p & 1].field[z].card.id != 0;
}
