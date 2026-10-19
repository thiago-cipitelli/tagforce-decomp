// @range 0x08864DF0-0x08865838
// @asm 0x08864FD4-0x088650B8
// @asm 0x08865658-0x088657D8
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
int duel_08835AC0(void);
void duel_08835A54(unsigned int p, unsigned short id, int (*fn)(DuelAct *a, unsigned int p, int z));
void duel_wfmt_d(char *dst, char *fmt, int n);
void duel_0884ACD8();
int duel_08861360(DuelAct *a);
void duel_08832788(unsigned int p, int id, int a, int b);
void duel_08832710(unsigned int p, int id, int a, int b);
void duel_0886191C(DuelAct *a, int b);
int duel_hand_find_uid(unsigned int p, int uid);
void duel_08868038(DuelAct *a, int b);
void duel_088C77EC(unsigned int p, int i);
void duel_088CA384(unsigned int p, int id, int a);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(int id));
void duel_08835978(int (*fn)(int id));
int duel_0887FCA8(int id);
int duel_088619FC(DuelAct *a, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_08865258(DuelAct *a, unsigned int p, int z);

extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

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

int duel_088652F0(DuelAct *a)
{
    char buf[512];

    if (a->kind == 0x23) {
        return 1;
    }
    switch (duel_w_08914904) {
    case 0:
        duel_wfmt_d(buf, (char *)0x11C, duel_088613D8(a));
        duel_0884ACD8(a->player, buf);
        a->w[2] = duel_088613D8(a);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_08835A54(a->player, a->id, duel_08865258);
        duel_w_08914904++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            duel_088CC284(duel_w_0891964C, duel_w_08919650 + duel_w_08919654, -1);
            if (--a->w[2] == 0) {
                return 1;
            }
        }
        duel_w_08914904--;
        return 0;
    }
    return 0;
}

int duel_08865448(DuelAct *a)
{
    int n = duel_08861360(a);

    if (n > 0) {
        duel_08832788(a->player, n, 0, 0);
    }
    return 1;
}

int duel_08865490(DuelAct *a, int b)
{
    duel_0886191C(a, b);
    duel_088619C4(a, b);
    return 1;
}

int duel_088654D0(DuelAct *a)
{
    int i = duel_hand_find_uid(a->player, a->uid);

    if (i >= 0) {
        duel_088CAE34(a->player, i);
    }
    return 1;
}

int duel_08865524(DuelAct *a, int b)
{
    duel_088619C4(a, b);
    duel_08832710(a->player, 1, 0, 1);
    return 1;
}

void duel_08865564(DuelAct *a, int b)
{
    if (duel_w_08914904 == 0) {
        duel_088C77D4(a->player, 0x16B8, 1, a->uid);
        duel_088619C4(a, b);
    }
    duel_08868038(a, b);
}

int duel_088655D4(DuelAct *a)
{
    duel_088C77D4(a->player, 0x16BE, 1, a->uid);
    return 1;
}

void duel_0886560C(DuelAct *a, int b)
{
    if (duel_w_08914904 == 0) {
        duel_088619C4(a, b);
    }
    duel_08868038(a, b);
}

#ifdef NON_MATCHING /* delay slot filling of the player type test differs */
int duel_08865658(DuelAct *a, int b)
{
    int r;

    switch (duel_w_08914904) {
    default:
        r = duel_088619FC(a, b);
        if (r == 1) {
            duel_w_08914904 = 10;
            r = 0;
        }
        break;
    case 10: {
        unsigned int p;

        duel_0884ACD8(a->player, 0x4C);
        p = a->player;
        if (duel_player_type[p] != 1) {
            duel_08835978(duel_0887FCA8);
        } else {
            duel_0881956C(p, 2, duel_0887FCA8);
        }
        duel_w_08914904++;
        r = 0;
        break;
    }
    case 11:
        if (duel_08835AC0() != 0) {
            unsigned int p = duel_w_0891964C;
            int i = duel_w_08919654;

            duel_088C77EC(p, CARD_UID(duel.players[p & 1].hand[i]));
            duel_088CA384(p, i, 0);
            r = 1;
        } else {
            duel_w_08914904--;
            r = 0;
        }
        break;
    }
    return r;
}
#endif

int duel_088657D8(DuelAct *a, int b)
{
    int r = duel_088619FC(a, b);

    if (r == 1) {
        duel_088C87A8(a->player, a->zone, 0x16CB, 2, 0);
    }
    return r;
}
