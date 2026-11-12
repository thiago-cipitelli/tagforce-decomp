// @range 0x08864DF0-0x08865F2C
// @asm 0x08865C10-0x08865E14
/* Card effect handlers (descriptor callbacks of the card effect tables). */
#include "duel.h"

extern int duel_w_08914904;

int duel_slot_value_1909(unsigned int p, int z);
int duel_slot_e68a0(unsigned int, int);
void duel_088D184C(DuelAct *a, unsigned int mask, int b);
void duel_088D1D64(DuelAct *a, unsigned int mask);
void duel_088CAE80(unsigned int p, int a);
void duel_088CAE34(unsigned int p, int a);
int duel_0886CE90(DuelAct *a, int b);
void duel_088C77D4(unsigned int p, unsigned short id, unsigned short a, unsigned short b);
void duel_088C7E68(unsigned int, int, int, int);
void duel_088CC178();
int duel_08866A3C(DuelAct *a, int b);
int duel_088619C4(DuelAct *a, int b);
void duel_0886268C(DuelAct *a, int b);
int duel_088D1CF4(DuelAct*, unsigned int, int);
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
int duel_0886191C(DuelAct *a, int b);
int duel_hand_find_uid(unsigned int, int);
int duel_08868038(DuelAct *a, int b);
void duel_088C77EC(unsigned int p, int i);
void duel_088CA384(unsigned int p, int id, int a);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(int id));
void duel_08835978(int (*fn)(int id));
int duel_0887FCA8(int id);
int duel_088619FC(DuelAct *a, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, unsigned short a, unsigned short b);
int duel_08865258(DuelAct *a, unsigned int p, int z);
int duel_0884E9E8(DuelAct *a, int b);
int card_get_attribute_icon(int id);
unsigned int card_get_type(int);
void duel_0884AD40(int msg, int uid);
int duel_0886CFE0(DuelAct *a, int b);
int duel_slot_efb84(unsigned int p, int z);
unsigned int duel_field_type(unsigned int p, int z);
int duel_088DFCC8(unsigned int uid);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
int duel_card_check_9148(unsigned int p, int id);
unsigned int duel_08865ABC(unsigned int p, int z, int i);

extern int duel_player_type[2];
extern int duel_w_08919624;
extern DuelAct *duel_w_089147DC;
extern DuelCard duel_cards[512];
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

int duel_08864FD4(DuelAct *a)
{
    if (a->kind != 0x23) {
        switch (duel_w_08914904) {
        case 0: {
            int s = a->player * 16;

            duel_088D1D64(a, 0xFFFF << s ^ 1 << (a->zone + s));
            duel_w_08914904++;
            return 0;
        }
        case 1: {
            unsigned int p = a->player;

            if (duel.players[p & 1].n_hand != 0) {
                duel_088CAE34(p, 0);
                return 0;
            }
            duel_w_08914904++;
            return 0;
        }
        }
    }
    return 1;
}

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

int duel_08865658(DuelAct *a, int b)
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
        duel_0884ACD8(a->player, 0x4C);
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, 2, duel_0887FCA8);
        } else {
            duel_08835978(duel_0887FCA8);
        }
        duel_w_08914904++;
        return 0;
    case 11:
        if (duel_08835AC0() != 0) {
            int p;
            int i;

            p = duel_w_0891964C;
            i = duel_w_08919654;
            duel_088C77EC(p, CARD_UID(duel.players[p & 1].hand[i]));
            duel_088CA384(p, i, 0);
            return 1;
        }
        duel_w_08914904--;
        return 0;
    }
}

int duel_088657D8(DuelAct *a, int b)
{
    int r = duel_088619FC(a, b);

    if (r == 1) {
        duel_088C87A8(a->player, a->zone, 0x16CB, 2, 0);
    }
    return r;
}

/* x = &srcs[i] no bloco do teste de kinds e usado também no teste do jogador: o mwcc o guarda em $s0 */
int duel_08865838(DuelAct *a, int b)
{
    unsigned int i;

    switch (duel_w_08914904) {
    case 0:
        duel_088C7E68(a->player, a->zone, 4, 0);
        if (duel_0884E9E8(a, b) == 0) {
            return -1;
        }
        for (i = 0; i < duel_chain.n_kinds; i++) {
            if (duel_chain.kinds[i] == 0x19) {
                struct DuelActX *x = &duel_chain.srcs[i];

                if (x->s_player == a->player && card_get_attribute_icon(duel.list[x->s_uid].id) == 0 && duel_hand_find_uid(a->player, x->s_uid) >= 0) {
                    a->w[0] = x->s_uid;
                    duel_w_08914904++;
                    return 0;
                }
            }
        }
        return -1;
    case 1:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_0884AD40(0xFA, a->uid);
        }
        duel_w_08914904++;
        return 0;
    case 2:
        if (duel_w_08919624 == 0) {
            return -1;
        }
        duel_088C77EC(a->player, a->w[0]);
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

int duel_08865A48(DuelAct *a)
{
    duel_08832710(a->player, 1, 0, 1);
    return 1;
}

int duel_08865A78(DuelAct *a, int b)
{
    if (duel_0886CFE0(a, b) != 0) {
        return duel_088619FC(a, b);
    }
    return -1;
}

unsigned int duel_08865ABC(unsigned int p, int z, int i)
{
    if (p != duel_w_089147DC->player) {
        return 0;
    }
    if (z < 5) {
        if (duel_slot_efb84(p, z + i) == 0) {
            return 0;
        }
        switch ((unsigned short)duel_field_type(p, z + i)) {
        case 0xB:
        case 0xC:
        case 0x10:
            return 0x800;
        default:
            goto none;
        }
    }
    if (z == 0xB) {
        if (duel_088DFCC8(CARD_UID(duel.players[p & 1].hand[i])) == 0) {
            return 0;
        }
        switch (card_get_type(duel.players[p & 1].hand[i].id)) {
        case 0xB:
        case 0xC:
        case 0x10:
            return 0x800;
        }
    }
none:
    return 0;
}

#ifdef NON_MATCHING /* 51 palavras (era 91): case 3 com o corpo dentro de if (duel_08835AC0() != 0); falta o registrador do valor do switch (v1 no original, carregado antes do move s0,a0) e o b/nop do default */
int duel_08865C10(DuelAct *a)
{
    switch (duel_w_08914904) {
    case 0:
        if (duel_player_type[a->player] == 1) {
            if (duel_card_check_9148(a->player, a->id) != 0) {
                duel_w_08919624 = 0;
            } else {
                duel_w_08919624 = 1;
            }
        } else {
            duel_0884AD40(0xFA, a->uid);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_w_08919624 != 0) {
            duel_w_08914904++;
            return 0;
        }
        return -1;
    case 2:
        duel_0884ACD8(a->player, 0x3A);
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, a->id, (int (*)(int))duel_08865ABC);
        } else {
            duel_08835978((int (*)(int))duel_08865ABC);
        }
        duel_w_08914904++;
        return 0;
    case 3:
        if (duel_08835AC0() != 0) {
            if (duel_w_08919650 == 0xB) {
                duel_088CAE24(duel_w_0891964C, &duel.players[duel_w_0891964C & 1].hand[duel_w_08919654], 0);
            } else {
                duel_088D1CF4(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            }
            return 1;
        }
        duel_w_08914904--;
        return 0;
    }
    /* no return in the original: the default case returns the v0 = 1 left by the switch tests (port: return 1) */
}
#endif

int duel_08865E14(DuelAct *a)
{
    duel_088D1CF4(a, a->player, 10);
    return 1;
}

int duel_08865E3C(DuelAct *a)
{
    switch (duel_w_08914904) {
    case 0: {
        unsigned int p;

        duel_0884ACD8(a->player, 0x3A);
        p = a->player;
        if (duel_player_type[p] == 1) {
            duel_0881956C(p, 2, duel_0887FCA8);
        } else {
            duel_08835978(duel_0887FCA8);
        }
        duel_w_08914904++;
        return 0;
    }
    case 1:
        if (duel_08835AC0() != 0) {
            unsigned int i = duel_w_08919654;

            duel_088CAE34(duel_w_0891964C, i);
            return 1;
        }
        duel_w_08914904--;
        return 0;
    }
    return 0;
}

