// @range 0x08864DF0-0x088652F0
// @asm 0x08864FD4-0x088650B8
/* Card effect handlers (descriptor callbacks of the card effect tables). */
#include "duel.h"

extern int duel_w_08914904;

int duel_slot_value_1909(unsigned int p, int z);
int duel_slot_e68a0(unsigned int p, int z);
void duel_088D184C(DuelAct *a, unsigned int mask, int b);
void duel_088D1D64(DuelAct *a, unsigned int mask);
void duel_088CAE80(unsigned int p, int a);
void duel_088CAE34(unsigned int p, int a);
int duel_0886CE90(DuelAct *a, int b);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
void duel_088C7E68(unsigned int p, int z, int a, int b);
void duel_088CC178();
int duel_08866A3C(DuelAct *a, int b);
void duel_088619C4(DuelAct *a, int b);
void duel_0886268C(DuelAct *a, int b);
void duel_088D1CF4(DuelAct *a, unsigned int p, int z);
int duel_088613D8(DuelAct *a);
void duel_088CC284(unsigned int p, int z, int n);
int duel_08862E5C(DuelAct *a, int b);

int duel_08864DF0(DuelAct *a)
{
    if (a->kind == 0x23) {
        return 1;
    }
    switch (duel_w_08914904) {
    case 0: {
        int s = a->player * 16;

        unsigned int m = 0xFFFF << s;

        m &= ~(1 << (a->zone + s));
        duel_088D184C(a, m, 0);
        duel_w_08914904++;
        return 0;
    }
    case 1: {
        unsigned int p = a->player;

        if (duel.players[p & 1].n_hand != 0) {
            duel_088CAE80(p, 0);
            return 0;
        }
        duel_w_08914904++;
        return 0;
    }
    }
    return 1;
}

int duel_08864ED8(DuelAct *a, int b)
{
    if (duel_0886CE90(a, b) != 0) {
        return 1;
    }
    return -1;
}

int duel_08864F00(DuelAct *a)
{
    duel_088C77D4(a->player, a->id, 1, a->uid);
    return 1;
}

int duel_08864F38(DuelAct *a)
{
    a->w[0] = duel.players[a->player & 1].field[a->zone].link;
    duel_088C7E68(a->player, a->zone, 4, 0);
    duel_088CC178(a->player, a->zone, 0);
    return 1;
}

#ifdef NON_MATCHING /* register allocation of the zone mask differs */
int duel_08864FD4(DuelAct *a)
{
    if (a->kind == 0x23) {
        return 1;
    }
    switch (duel_w_08914904) {
    case 0: {
        int s = a->player * 16;

        duel_088D1D64(a, 0xFFFF << s ^ 1 << (a->zone + s));
        duel_w_08914904++;
        return 0;
    }
    case 1: {
        unsigned int p = a->player;

        if (duel.players[p & 1].n_hand == 0) {
            duel_w_08914904 = 2;
            return 0;
        }
        duel_088CAE34(p, 0);
        return 0;
    }
    }
    return 1;
}
#endif

int duel_088650B8(DuelAct *a, int b)
{
    if (a->kind == 0xF) {
        return 1;
    }
    return duel_08866A3C(a, b);
}

void duel_088650F4(DuelAct *a, int b)
{
    if (a->zone < 5) {
        duel_088619C4(a, b);
    } else {
        duel_0886268C(a, b);
    }
}

int duel_08865138(DuelAct *a)
{
    a->w[1] = duel_slot_value_1909(a->player, a->zone);
    duel_088D1CF4(a, a->player, a->zone);
    return 1;
}

int duel_0886518C(DuelAct *a)
{
    duel_088CC284(a->player, a->zone, -duel_088613D8(a));
    return 1;
}

int duel_088651CC(DuelAct *a, int b)
{
    if (duel_08862E5C(a, b) != 0) {
        if (a->b3_4 == 0) {
            duel_088CC284(a->player, a->zone, 3);
        }
        return 1;
    }
    return 0;
}

int duel_08865228(DuelAct *a)
{
    duel_088C77D4(a->player, a->id, 1, 0);
    return 1;
}

int duel_08865258(DuelAct *a, unsigned int p, int z)
{
    if (a->player != p) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    return duel_slot_e68a0(p, z) != 0;
}
