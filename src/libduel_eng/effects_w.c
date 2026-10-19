// @range 0x08886984-0x08887618
// @asm 0x08886C04-0x08886CAC
// @asm 0x08886E88-0x08887034
/* Card effect handlers (DuelAct *a), after effects_v.c. */
#include "duel.h"

extern int duel_step;
extern int duel_player_type[2];
extern unsigned int duel_turn_player;
extern unsigned short duel_w_0891968C;

int duel_slot_has_effect(unsigned int p, int z, int id);
void duel_088D1C70(DuelAct *a, unsigned int p, int z);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088C87A8(unsigned int p, int z, unsigned short id, unsigned short a, unsigned short b);
void duel_088CBAA4(unsigned int p, int z, int uid);
void duel_088D15B4(DuelAct *a, unsigned int p, int z);
int duel_d_find_name(unsigned int p, int id);
int duel_088CB554(unsigned int p, DuelCard *c);
void duel_0884ACD8(unsigned int p, int msg);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_088C7810(unsigned int p, int a);
void duel_088C96EC(unsigned int p, int z, int a);
void duel_088C7B90(int n);
void duel_08835978(int (*fn)(unsigned int p, int z));
int duel_08886C04(DuelAct *a);
void duel_088D1858(DuelAct *a, unsigned int p, int z);
int duel_0888D5A8(DuelAct *a, int b);
void duel_088C89BC(unsigned int p, int z, int id, int c);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_088C7DD4(unsigned int p, int a, int b);
int duel_count_tributable_except(unsigned int p, int src);
void duel_088328B8(unsigned int p, unsigned short a, int b);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int duel_088C5E70(unsigned int p, int a);
void duel_088C5424(unsigned int p, int n);
void duel_088CAB04();
void duel_08832624(unsigned int p, int a, int b, int c);
void duel_08832880();
void duel_088327F8(unsigned int p, int a, int b);
int duel_count_active(int id);
void duel_088C9344(DuelAct *a, unsigned int p, int z, int b);

int duel_08886984(DuelAct *a)
{
    int i;
    unsigned int q;
    int z;

    if (a->b4_2) {
        return 0;
    }
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    for (i = 0; i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z < 5; z++) {
            if (duel_slot_has_effect(q, z, 0x12EA) != 0) {
                duel_088D1C70(a, q, z);
            }
        }
    }
    return 0;
}

int duel_08886AA8(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            duel_088C87A8(p, z, a->id, 1, a->player);
        }
    }
    return 0;
}

int duel_08886B34(DuelAct *a, DuelAct *b)
{
    if (b != 0) {
        duel_088CBAA4(b->player, b->zone, b->uid);
        b->b4_3 = b->b4_2 = b->b4_1 = 1;
        duel_088D15B4(a, b->player, b->zone);
        return 0;
    }
    return duel_08886C04(a);
}

#ifdef NON_MATCHING /* registradores */
int duel_08886C04(DuelAct *a)
{
    if (duel.players[a->x.s_player].field[a->x.s_zone].card.id != 0) {
        duel_088CBAA4(a->x.s_player, a->x.s_zone, a->x.s_uid);
        duel_088D15B4(a, a->x.s_player, a->x.s_zone);
    }
    return 0;
}
#endif

int duel_08886CAC(DuelAct *a)
{
    int i;

    switch (duel_step) {
    case 0x80:
        i = duel_d_find_name(a->player, 0x12E5);
        if (i >= 0 && duel_088CB554(a->player, &duel.players[a->player & 1].zone_d[i]) != 0) {
            return 100;
        }
        duel_0884ACD8(a->player, 0x15);
        return 0x6F;
    case 0x6F:
        duel_088328FC(1 - a->player);
        return 100;
    case 100:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_08886D94(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (a->b4_2) {
        return 0;
    }
    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel.players[p & 1].field[z].pos == 0) {
            duel_088C97F0(a, p, z, 0, 0);
        }
    }
    return 0;
}

int duel_08886E3C(DuelAct *a)
{
    duel_cmd((a->player ? 0x8000 : 0) | 0x1C, 1, 0, 0);
    return 0;
}

static int duel_08886E80(unsigned int p, int z)
{
    return 0;
}

#ifdef NON_MATCHING /* registradores */
int duel_08886E88(DuelAct *a)
{
    int i;
    unsigned int q;
    int z;

    switch (duel_step) {
    case 0x80:
        if (a->id == 0x184D) {
            duel_088C7810(1 - a->player, 1);
        }
        for (i = 0; i < 2; i++) {
            q = a->player ^ i;
            for (z = 0; z <= 10; z++) {
                if (duel_0884E440(a, q, z) != 0) {
                    duel_088C96EC(q, z, 1);
                }
            }
        }
        if (duel_player_type[a->player] == 1) {
            duel_088C7B90(0x1E);
        } else {
            duel_08835978(duel_08886E80);
        }
        return 0x7F;
    case 0x7F:
        for (i = 0; i < 2; i++) {
            q = a->player ^ i;
            for (z = 0; z <= 10; z++) {
                if (duel_0884E440(a, q, z) != 0) {
                    duel_088C96EC(q, z, 0);
                }
            }
        }
        if (a->id == 0x184D) {
            duel_088C7810(1 - a->player, 0);
        }
        return 0;
    }
    return 0;
}
#endif

int duel_08887034(DuelAct *a)
{
    DuelSpecAct *s = &duel_special.act[duel_turn_player];

    if (s->pad14[6] == 0 && s->uid == CARD_UID(duel.players[s->unk0 & 1].field[s->unk4].card)) {
        duel_088D1858(a, s->unk0, s->unk4);
    }
    return 0;
}

int duel_088870E4(DuelAct *a, int b)
{
    if (a->b3_4 != 2 || a->kind == 0x23) {
        duel_088C77D4(a->player, a->id, 1, 0);
        duel_0884ACD8(a->player, 0xA2);
        return 0;
    }
    if (duel_step == 0x80) {
        duel_088C89BC(a->player, 0xB, a->id, 0);
    }
    return duel_0888D5A8(a, b);
}

int duel_088871A0(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    duel_088C7DD4(a->player, 0x17, 1);
    return 0;
}

int duel_088871E4(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    if (duel_count_tributable_except(1 - a->player, -1) == 0) {
        return 0;
    }
    duel_088328B8(1 - a->player, 0xFFFF, 1);
    return 0;
}

int duel_0888725C(DuelAct *a)
{
    int i;
    int z;
    unsigned int q;
    int f;

    if (a->b4_2) {
        return 0;
    }
    for (i = 0; i < 2; i++) {
        if (i != 0) {
            q = 1 - a->player;
        } else {
            q = a->player;
        }
        for (z = 0; z < 5; z++) {
            if (duel.players[q & 1].field[z].card.id != 0) {
                f = duel.players[q & 1].field[z].pos != 0 && !duel.players[q & 1].field[z].faceup;
                if (duel_088C97F0(a, q, z, f, 1) != 0) {
                    duel_088C7E68(q, z, 0x15, 1);
                }
            }
        }
    }
    duel_088C77D4(a->player, a->id, 1, 0);
    return 0;
}

int duel_08887390(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    if (duel_088C5E70(a->player, 1) != 0) {
        duel_088C5424(1 - a->player, 1000);
    }
    return 0;
}

int duel_088873FC(DuelAct *a)
{
    unsigned int q = a->w[0] & 1;
    unsigned int p;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        p = a->player;
        if (q == p) {
            duel_08832624(q, 1, 0, 0);
            return 0;
        }
        if (duel.players[(1 - p) & 1].n_hand == 0) {
            return 0;
        }
        duel_08832880(p, a->id, 1);
        return 0x7F;
    case 0x7F:
        duel_088CAB04(1 - a->player, duel_w_0891968C, 1);
        return 0;
    }
    return 0;
}

int duel_088874E0(DuelAct *a)
{
    unsigned int q = a->w[0] & 1;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        duel_088327F8(q, 1, 1);
        return 0x7F;
    case 0x7F:
        duel_08832624(q, 1, 0, 1);
        return 0x7E;
    }
    return 0;
}

int duel_0888755C(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (a->b4_2) {
        return 0;
    }
    if (duel_count_active(0x135D) != 0) {
        return 0;
    }
    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel.players[p & 1].field[z].faceup != 0) {
            duel_088C9344(a, p, z, 0);
        }
    }
    return 0;
}
