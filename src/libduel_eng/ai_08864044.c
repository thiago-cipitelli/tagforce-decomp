// @range 0x08864044-0x08864DF0
/* The computer player: handlers of the cards it activates. */
#include "duel.h"

void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088CABA8(unsigned int p, int i);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
int duel_0884C824(void);
DuelCard *duel_0884C7EC(int i);
int duel_0886CE38();
void duel_0884ACD8(unsigned int p, int msg);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
void duel_08835F34(unsigned int p, int z, int i, int d);
int duel_088537B0(int id);
void duel_088359E8(unsigned int p, int id, int (*fn)(int id));
int duel_08835AC0(void);
int duel_0884E690(DuelAct *a, int (*fn)(int id));
void duel_088D1D64(DuelAct *a, unsigned int mask);
int duel_08861360(DuelAct *a);
void duel_08832788(unsigned int p, int id, int a, int b);
unsigned int duel_0884E5E0(DuelAct *a);
int duel_host_call(int a, int b, int c, int d);
void duel_088D1B20(unsigned int p, unsigned int mask);
int duel_088619FC(DuelAct *a, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_0886B4E8();
void duel_08832974(unsigned int p, unsigned short id, int a);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_088C4564(unsigned int p, int n);
int duel_0884E948(DuelAct *a, unsigned int p);
int duel_count_monsters(unsigned int p);
int duel_08861480(int id, int n);

extern int duel_w_08914904;
extern int duel_w_08919624;
extern unsigned int duel_turn_player;
extern int duel_phase;
extern int duel_player_type[2];
extern unsigned int duel_ai_player;
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

int duel_08864044(DuelAct *a)
{
    unsigned int p;
    int i;

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
    case 2:
        duel_088BFF78(a->player, a->id, 0);
        for (i = 0; i < duel_0884C824(); i++) {
            a->w[i] = CARD_UID(*duel_0884C7EC(i));
        }
        a->n7 = duel_0884C824();
        break;
    }
    return 1;

}

int duel_088641B0(void)
{
    if (duel_0886CE38() != 0) {
        return 1;
    }
    return -1;
}

int duel_088641D8(DuelAct *a, DuelAct *b)
{
    unsigned int p;
    int z;

    switch (duel_w_08914904) {
    case 0:
        a->w[1] = 0;
        a->w[0] = 0;
        a->w[2] = 0;
        duel_0884ACD8(a->player, 0x3A);
        duel_w_08914904++;
    case 1:
        if (duel_player_type[a->player] == 1) {
            if (b->player == a->player && duel_0884E3E4(a, b->player, b->zone) != 0) {
                duel_ai_player = a->player;
                duel_08835F34(b->player, b->zone, 0, 11);
                duel_w_08914904++;
                return 0;
            }
        }
        duel_088359E8(a->player, a->id, duel_088537B0);
        duel_w_08914904++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            a->w[2]++;
            a->w[p] |= 1 << z;
            if (duel_player_type[a->player] == 1) {
                duel_w_08914904++;
                return 0;
            }
            if (duel_0884E690(a, duel_088537B0) == 0) {
                duel_w_08914904++;
                return 0;
            }
            duel_w_08914904--;
            return 0;
        }
        if (a->w[2] != 0) {
            duel_w_08914904++;
            return 0;
        }
        duel_w_08914904 = 0;
        return 0;
    case 3:
        duel_088D1D64(a, a->w[0] | a->w[1] << 16);
        a->w[0] = a->w[2];
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

int duel_08864478(DuelAct *a)
{
    int n;

    if (a->kind != 0x23 && (n = duel_08861360(a)) > 0) {
        duel_08832788(a->player, n, 0, 1);
    }
    return 1;
}

int duel_088644D8(DuelAct *a)
{
    unsigned int r = duel_0884E5E0(a);
    unsigned int p = a->player;
    int n = duel.players[(1 - p) & 1].n_d;
    unsigned int q;
    int z;

    if (r < n) {
        n = r;
    }
    switch (duel_w_08914904) {
    case 0:
        a->w[1] = 0;
        a->w[0] = 0;
        a->w[2] = 0;
        if (n == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, 0x74);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_088359E8(p, a->id, duel_088537B0);
        duel_w_08914904++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            q = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_host_call(0x28, q, z, CARD_UID(duel.players[q & 1].field[z].card));
            a->w[q] |= 1 << z;
            if (++a->w[2] >= n) {
                duel_w_08914904++;
            } else {
                duel_w_08914904--;
            }
            return 0;
        }
        if (a->w[2] == 0) {
            duel_host_call(0x29, 0, 0, 0);
            duel_w_08914904 = 0;
            return 0;
        }
        duel_w_08914904++;
        return 0;
    case 3:
        duel_088D1B20(p, a->w[0] | a->w[1] << 16);
        return 1;
    }
    return 0;
}

int duel_08864744(DuelAct *a, int b)
{
    if (a->b3_4 == 1) {
        if (a->kind == 5) {
            return 1;
        }
    }
    switch (a->id) {
    case 0x1503:
        return duel_088619FC(a, b);
    }
    return 1;
}

int duel_088647B0(DuelAct *a)
{
    duel_088C87A8(a->player, a->zone, 0x152A, 1, 0);
    return 1;
}

int duel_088647E8(DuelAct *a)
{
    unsigned int m;
    unsigned int q;

    switch (duel_w_08914904) {
    case 0:
        if (duel_0886B4E8(a) != 0) {
            duel_0884ACD8(a->player, 0x107);
            return -1;
        }
        m = 4;
        switch (a->id) {
        case 5000:
            if (duel_0884E5E0(a) != 0) {
                if (duel.players[a->player & 1].lp > 1000) {
                    m |= 1;
                }
            }
            if (duel.players[a->player & 1].lp > 1) {
                m |= 2;
            }
            break;
        case 0x1532:
            q = (1 - a->player) & 1;
            if (duel.players[q].n_hand != 0) {
                m |= 1;
            }
            if (duel.players[q].n_d != 0) {
                m |= 2;
            }
            break;
        case 0x161E:
            if (duel_0884E5E0(a) != 0) {
                m |= 1;
            }
            if (duel.players[(1 - a->player) & 1].n_d != 0) {
                m |= 2;
            }
            break;
        case 0x1656:
            if (duel_0884E5E0(a) != 0) {
                m |= 1;
            }
            if (duel.players[(1 - a->player) & 1].n_d != 0) {
                m |= 2;
            }
            break;
        case 0x1685:
            if (duel_0884E5E0(a) != 0) {
                m |= 1;
            }
            if (duel.players[(1 - a->player) & 1].n_d != 0) {
                m |= 2;
            }
            break;
        case 0x1686:
            if (duel.players[a->player & 1].n_d != 0) {
                m |= 1;
            }
            if (duel_088BFFD8(a->player, a->id, 0) != 0) {
                m |= 2;
            }
            break;
        }
        q = a->player;
        if (duel_player_type[q] == 1) {
            duel_w_08919624 = 2;
            switch (a->id) {
            case 0x1686:
                if (duel_088BFFD8(q, a->id, 1) != 0) {
                    duel_w_08919624 = 0;
                } else if (m & 2) {
                    duel_w_08919624 = 1;
                }
                break;
            case 0x1532:
                if (m & 1) {
                    duel_w_08919624 = 0;
                } else if (m & 2) {
                    duel_w_08919624 = 1;
                }
                break;
            case 5000:
                if ((m & 2) && duel_turn_player == q && (unsigned int)duel_phase < 4 && duel_count_monsters(1 - q) == 0) {
                    duel_w_08919624 = 1;
                } else if ((m & 1) && duel_0884E948(a, 1 - a->player) != 0) {
                    duel_w_08919624 = 0;
                }
                break;
            case 0x161E:
            case 0x1685:
                if ((m & 1) && duel_0884E948(a, 1 - a->player) != 0) {
                    duel_w_08919624 = 0;
                } else if (m & 2) {
                    duel_w_08919624 = 1;
                }
                break;
            case 0x1656:
                if (duel_0884E948(a, 1 - q) != 0) {
                    duel_w_08919624 = 0;
                }
                break;
            }
        } else {
            duel_0884ADAC(0x103, duel_08861480(a->id, 0), duel_08861480(a->id, 1), 0x1AC, 0, m);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        a->w[2] = duel_w_08919624 + 1;
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        duel_w_08914904++;
        return 0;
    case 2:
        if (a->w[2] == 3) {
            return -1;
        }
        switch (a->id) {
        case 5000:
            if (a->w[2] == 1) {
                duel_088C4564(a->player, 1000);
            }
            if (a->w[2] == 2) {
                a->w[0] = duel.players[a->player & 1].lp - 1;
                q = a->player;
                duel_088C4564(q, duel.players[q & 1].lp - 1);
            }
            break;
        }
        return 1;
    }
    return 0;
}
