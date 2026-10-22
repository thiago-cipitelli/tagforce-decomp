// @range 0x08865F2C-0x08866F7C
// @data 0x089116FC-0x0891171C
// @asm 0x08866608-0x088667B4
/* Card effect handlers (DuelAct *a): step machines on duel_w_08914904 that ask the player (or the
   computer) for a card and apply the result. */
#include "duel.h"

void duel_088328B8(unsigned int p, unsigned short a, int b);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_088D1AC8(unsigned int p, int z);
void duel_08832974(unsigned int p, unsigned short id, int a);
void duel_0884ACD8(unsigned int p, int msg);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)());
void duel_08835978(int (*fn)());
int duel_088619C4(DuelAct *a, int b);
int duel_list_card_level(unsigned int p, int i);
void duel_08832B40(unsigned int p, int a, int b);
void duel_088C4564(unsigned int p, int n);
void duel_088CC178(unsigned int p, int z, unsigned int v);
unsigned int duel_card_locate(unsigned int uid);
void duel_088D2468(int same);
int duel_0887FCA8(int id);
void duel_088CABA8(unsigned int p, int i);
int duel_08835AC0(void);
int card_get_level(int id);
int duel_slot_efb84(unsigned int p, int z);
void duel_088D1D64(DuelAct *a, unsigned int mask);
int duel_08877C00();
int duel_088619FC(DuelAct *a, int b);
int duel_grave_find_uid(unsigned int p, int uid);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
int duel_088BFFD8(unsigned int p, int id, int i);
void duel_0884AD40(int msg, int uid);
void duel_088D1CF4(DuelAct *a, unsigned int p, int z);
void duel_08832950(unsigned int p, unsigned short id);
int card_get_atk_q(int id);
int card_is_monster(int id);
int duel_0884E948(DuelAct *a, unsigned int p);
void duel_088CAE34(unsigned int p, int a);
int card_get_atk(int id);
int duel_08866608(DuelAct *a);
int duel_0886CE38();
int duel_slot_value_1909(unsigned int p, int z);
void duel_088359E8(unsigned int p, int id, int (*fn)(int id));
int duel_088537B0(int id);
int duel_host_call(int a, int b, int c, int d);
int duel_0884E5E0(DuelAct *a);
void duel_0884AD18(int msg);
void duel_088D1B20(unsigned int p, unsigned int mask);
void duel_088C7BA4(void);
void duel_088C7CF0(void);

extern int duel_w_08914904;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern unsigned short duel_w_0891968C;
extern DuelAct *duel_w_089147DC;
extern unsigned int duel_atk_tp;
extern int duel_atk_tz;

/* Picks a monster and lets the opponent choose between its ATK and DEF (w[0], w[1] -> w[2]). */
int duel_08865F2C(DuelAct *a)
{
    DuelStats st;

    switch (duel_w_08914904) {
    case 0:
        duel_088328B8(a->player, 0xFFFF, 0);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_field_stats(duel_w_0891968C & 0xFF, duel_w_0891968C >> 8 & 0xFF, &st);
        a->w[0] = st.atk > 0xFFFF ? 0xFFFF : st.atk;
        a->w[1] = st.def > 0xFFFF ? 0xFFFF : st.def;
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = a->w[0] < a->w[1];
        } else {
            duel_0884ADAC(0x81, 0x198, 0x199, 0, 0, 0xF);
        }
        duel_w_08914904++;
        return 0;
    case 2:
        duel_088D1AC8(duel_w_0891968C & 0xFF, duel_w_0891968C >> 8 & 0xFF);
        duel_w_08914904++;
        return 0;
    case 3:
        switch (duel_w_08919624) {
        case 0:
            a->w[2] = a->w[0];
            break;
        case 1:
            a->w[2] = a->w[1];
            break;
        }
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

/* Picks a card of the hand (duel_0887FCA8 filters) and records its level in w[2]. */
int duel_08866104(DuelAct *a, int b)
{
    unsigned int p;
    int i;

    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(a->player, 0x29);
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, 2, duel_0887FCA8);
        } else {
            duel_08835978(duel_0887FCA8);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            i = duel_w_08919654;
            duel_088CABA8(p, i);
            switch (a->id) {
            case 0x1727:
                duel_088619C4(a, b);
                break;
            case 0x18CB:
                a->w[2] = duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i]));
                break;
            case 0x1895:
                duel_088619C4(a, b);
                a->w[2] = card_get_level(duel.players[p & 1].hand[i].id);
                break;
            }
            return 1;
        }
        duel_w_08914904--;
        return 0;
    }
    return 0;
}

/* Pays duel_w_0891968C * 1000 life points and deals the same damage. */
int duel_088662E8(DuelAct *a)
{
    int n;

    switch (duel_w_08914904) {
    case 0:
        duel_08832B40(a->player, a->id, 0);
        duel_w_08914904++;
        return 0;
    case 1:
        n = duel_w_0891968C * 1000;
        duel_088C4564(a->player, n);
        duel_088CC178(a->player, a->zone, n);
        return 1;
    }
    return 0;
}

int duel_088663A4(DuelAct *a)
{
    int loc = duel_card_locate(a->uid);

    if ((unsigned char)((loc & 0xFFFF) >> 8) < 5) {
        if ((loc >> 16 & 0xFFFF) != 0) {
            duel_088D2468(a->player == duel_atk_tp && a->zone == duel_atk_tz);
        } else {
            duel_088D1AC8(a->player, a->zone);
        }
    }
    return 1;
}

/* Marks the face-up 0x17AD-0x17AF spells/traps of the player; w[2] = all of them pass duel_slot_efb84. */
int duel_08866450(DuelAct *a)
{
    int z;
    int ok;
    unsigned int mask;
    unsigned int p;

    if (a->kind != 5) {
        ok = 1;
        mask = 0;
        for (z = 5; z < 10; z++) {
            p = a->player;
            if (duel.players[p & 1].field[z].faceup != 0) {
                switch (duel.players[p & 1].field[z].card.id) {
                case 0x17AF:
                case 0x17AD:
                case 0x17AE:
                    mask |= 1 << (z + p * 16);
                    ok &= duel_slot_efb84(p, z);
                    break;
                }
            }
        }
        duel_088D1D64(a, mask);
        a->w[2] = ok;
    }
    return 1;
}

int duel_0886655C(DuelAct *a, int b)
{
    int r;
    int i;
    unsigned int p;

    if (duel_08877C00() == 0) {
        return -1;
    }
    r = duel_088619FC(a, b);
    if (r == 1) {
        i = duel_grave_find_uid(a->player, a->uid);
        p = a->player;
        duel_088CB400(p, &duel.players[p & 1].grave[i], 0);
    }
    return r;
}

#ifdef NON_MATCHING /* o retorno 1 do passo 1 passa por um bloco de salto (b; lw ra) antes do caso 2 que nenhuma ordem de case/default reproduziu */
int duel_08866608(DuelAct *a)
{
    unsigned int p;

    switch (duel_w_08914904) {
    case 0:
        p = a->player;
        if (a->uid != CARD_UID(duel.players[p & 1].field[a->zone].card)) {
            return -1;
        }
        if (duel_player_type[p] == 1) {
            duel_w_08919624 = duel_088BFFD8(p, a->id, 0) > 0;
        } else if (duel_088BFFD8(p, a->id, 0) == 0) {
            duel_0884AD40(0x142, a->uid);
        } else {
            duel_0884AD40(0xEC, a->uid);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_w_08919624 != 0) {
            duel_088D1CF4(a, a->player, a->zone);
            break;
        }
        duel_08832950(1 - a->player, 0x10D6);
        duel_w_08914904++;
        return 0;
    default:
        return 1;
    case 2:
        return -1;
    }
    return 1;
}
#endif

int duel_088667B4(DuelAct *a)
{
    switch (a->kind) {
    case 2:
    case 5:
        return duel_08866608(a);
    }
    return 1;
}

/* Selection filter: monsters of the hand whose ATK the opponent can be targeted with (0x800 = valid). */
int duel_088667FC(unsigned int p, int z, int i)
{
    int id;

    if (p != duel_w_089147DC->player) {
        return 0;
    }
    if (z == 0xB) {
        id = duel.players[p & 1].hand[i].id;
        duel_w_089147DC->w[2] = card_get_atk_q(id);
        if (card_is_monster(id) && duel_w_089147DC->w[2] != 0xFFFF && duel_0884E948(duel_w_089147DC, 1 - p)) {
            return 0x800;
        }
    }
    return 0;
}

/* Picks a card of the hand (duel_088667FC) and records its ATK in w[2]. */
int duel_088668D4(DuelAct *a)
{
    unsigned int p;
    int i;

    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(a->player, 0x3A);
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, 2, duel_088667FC);
        } else {
            duel_08835978(duel_088667FC);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            i = duel_w_08919654;
            duel_088CAE34(p, i);
            a->w[2] = card_get_atk(duel.players[p & 1].hand[i].id);
            return 1;
        }
        duel_w_08914904--;
        return 0;
    }
    return 0;
}

int duel_08866A10(DuelAct *a)
{
    duel_088D1CF4(a, a->player, a->zone);
    return 1;
}

int duel_08866A3C(DuelAct *a, int b)
{
    unsigned int p = a->player ^ a->opp;
    unsigned int i;

    if (duel_0886CE38(a, b) == 0) {
        return -1;
    }
    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (!duel_088BFFD8(p, a->id, i)) {
            continue;
        }
        return duel_088619FC(a, b);
    }
    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(p, 0x122);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_08832950(1 - p, 0x10D6);
        break;
    }
    return -1;
}

int duel_08866B68(DuelAct *a)
{
    duel_088D1AC8(a->player, a->zone);
    a->w[0] = duel_slot_value_1909(a->player, a->zone);
    return 1;
}

int duel_08866BB8(DuelAct *a)
{
    switch (a->kind) {
    case 2:
        return duel_08866608(a);
    }
    return 1;
}

/* Lets the player pick cards of the field one by one (up to duel_0884E5E0), marking them in w[0]/w[1]. */
int duel_08866BF4(DuelAct *a, int b)
{
    unsigned int p;
    int z;

    switch (duel_w_08914904) {
    case 0:
        a->w[1] = 0;
        a->w[0] = 0;
        a->w[2] = 0;
        duel_w_08914904++;
        return 0;
    case 1:
        duel_0884ACD8(a->player, 0x12);
        duel_w_08919624 = 1;
        duel_w_08914904++;
        return 0;
    case 2:
    case 5:
        if (duel_w_08919624 == 0) {
            duel_w_08914904 = 7;
            return 0;
        }
        duel_088359E8(a->player, 1, duel_088537B0);
        duel_w_08914904++;
        return 0;
    case 3:
    case 6:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            a->w[p] |= 1 << z;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            a->w[2]++;
            duel_w_08914904++;
            return 0;
        }
        duel_w_08914904 -= 2;
        return 0;
    case 4:
        if (a->w[2] >= duel_0884E5E0(a)) {
            duel_w_08914904 = 7;
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 0;
        } else {
            duel_0884AD18(0x138);
        }
        duel_w_08914904++;
        return 0;
    case 7:
        duel_088D1B20(a->player, a->w[0] | a->w[1] << 16);
        duel_088619C4(a, b);
        return 1;
    }
    return 0;
}

int duel_08866E90(DuelAct *a, DuelAct *b)
{
    unsigned int p;

    switch (duel_w_08914904) {
    case 0:
        duel_088C7BA4();
        duel_w_08914904++;
        return 0;
    case 1:
        p = a->player;
        if (duel.players[p & 1].n_hand != 0) {
            duel_088CABA8(p, 0);
            return 0;
        }
        duel_088C7CF0();
        switch (b->id) {
        case 0x14E0:
            a->w[2] = b->w[2];
            break;
        }
        duel_w_08914904++;
        return 0;
    }
    return 1;
}
