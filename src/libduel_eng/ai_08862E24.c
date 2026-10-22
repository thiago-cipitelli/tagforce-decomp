// @range 0x08862E24-0x08863284
// @asm 0x08862E5C-0x08863018
/* The computer player: handlers of the cards it activates. */
#include "duel.h"

void duel_088C8AD0(unsigned int p, int z, int id, int v);
int duel_0884E398(DuelAct *a, int b);
void duel_0884AD18(int msg);
int duel_0886191C(DuelAct *a, int b);
int duel_08861964(DuelAct *a, int b);
int duel_088619C4(DuelAct *a, int b);
int duel_088619FC(DuelAct *a, int b);
int duel_08861D44(DuelAct *a, int b);
void duel_088328B8(unsigned int p, unsigned short a, int b);
int duel_0886CE38();
void duel_088D1AC8(unsigned int p, int z);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_0884ACD8(unsigned int p, int msg);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
void duel_088359E8(unsigned int p, int id, int (*fn)(DuelAct *a, unsigned int p, int z));
int duel_08835AC0(void);
void duel_088D1CF4(DuelAct *a, unsigned int p, int z);

extern int duel_w_08914904;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

int duel_08862E24(DuelAct *a)
{
    duel_088C8AD0(a->player, 11, 0x12A1, a->uid);
    return 1;
}

#ifdef NON_MATCHING /* igual ao original exceto um b para a instrução seguinte (b fim; nop) depois do último case do switch interno, que o mwcc não emite */
int duel_08862E5C(DuelAct *a, int b)
{
    int r = 1;

    switch (duel_w_08914904) {
    case 0:
        if (a->b3_4 == 0) {
            if (duel_0884E398(a, b) == 2) {
                if (a->id == 0x1517) {
                    duel_w_08919624 = 1;
                } else if (duel_player_type[a->player] == 1) {
                    duel_w_08919624 = 1;
                } else {
                    switch (a->id) {
                    case 0x1A1A:
                        duel_0884AD18(0x2F);
                        break;
                    default:
                        duel_0884AD18(0x10E);
                    }
                }
                duel_w_08914904++;
                return 0;
            }
            duel_w_08919624 = 0;
        } else {
            duel_w_08919624 = 1;
        }
        duel_w_08914904++;
        return 0;
    case 1:
        a->w[2] = duel_w_08919624;
        if (duel_w_08919624 == 0) {
            return r;
        }
        switch (a->id) {
        case 0x12F3:
        case 0x14C2:
        case 0x1517:
            r = duel_0886191C(a, b);
            break;
        case 0x14DE:
            r = duel_08861964(a, b);
            break;
        case 0x1624:
            r = duel_088619C4(a, b);
            break;
        }
        break;
    }
    return r;
}
#endif

int duel_08863018(DuelAct *a)
{
    duel_088328B8(a->player, a->src.player | a->src.zone << 8, 1);
    return 1;
}

int duel_08863060(DuelAct *a, int b)
{
    if (duel_0886CE38() != 0) {
        return duel_088619FC(a, b);
    }
    return -1;
}

int duel_088630A4(DuelAct *a)
{
    duel_088D1AC8(a->player, a->zone);
    return 1;
}

int duel_088630D0(DuelAct *a, int b)
{
    duel_088C87A8(a->player, a->zone, a->id, 4, 0);
    return duel_0886191C(a, b);
}

int duel_08863128(DuelAct *a)
{
    unsigned int p;
    int z;

    if (a->kind == 0x23) {
        return 1;
    }
    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(a->player, 0x3A);
        duel_088359E8(a->player, a->id, duel_0884E3E4);
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            a->w[1] = duel.players[p & 1].field[z].card.id;
            duel_088D1CF4(a, p, z);
            return 1;
        }
        duel_w_08914904--;
        return 0;
    }
    return 1;
}

int duel_08863250(DuelAct *a, int b)
{
    if (a->kind == 0x16) {
        return duel_08861D44(a, b);
    }
    return 1;
}
