// @range 0x0884D050-0x0884E228
// @asm 0x0884D72C-0x0884D874
// @asm 0x0884D874-0x0884D9B8
// @asm 0x0884DBAC-0x0884DD94
// @asm 0x0884DD94-0x0884DF2C
// @asm 0x0884E02C-0x0884E1D8
/* End of the duel: the reason each player wins or loses (LP, deck out, Exodia and the other
   special victories), the end-of-duel presentation, replaying the input log. */
#include "duel.h"

typedef struct DuelLogEnt {
    unsigned short v[4];
} DuelLogEnt;

extern unsigned int duel_view_player;  /* 0x089139F4 */
extern unsigned int duel_turn_player;
extern unsigned int duel_turn;
extern int duel_player_type[2];
extern int duel_w_089195EC;
extern int duel_w_089195D8;
extern int duel_w_08917DC4;
extern int duel_w_089195E4;
extern int duel_w_08919600;
extern int duel_busy_08915238;
extern unsigned char duel_msg_state;

int duel_0884CF30(unsigned int p);
int duel_0884CFBC(unsigned int p);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
int duel_count_monsters(unsigned int p);
int duel_0884B08C(void);
int duel_0884B050(void);
void duel_msg_send3(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_log_at_end(void);
DuelLogEnt *duel_log_cur(void);
void duel_log_next(void);
int duel_088D4CD8();
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_0884356C(void);
int duel_0884DFB0(void);
int duel_088321B4(void);
int duel_08824D34(void);
int duel_0884E1D8(void);
int duel_088535C0(void);
void duel_0883447C(void);
int duel_088F7418(void);
int duel_088D557C(void);
int duel_08827544(void);
int duel_card_used_e66ac(unsigned int id);
unsigned int card_get_type(int id);
int card_get_prop5_hi(int id);
int field_get_card(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_zone_usable(unsigned int p, unsigned int z);
int card_is_attribute9(int id);
int card_same_name(int id, int id2);
int duel_req_flag(int f);
int duel_host_call(int a, int b, int c, int d);
void duel_0884E02C(void);

extern int duel_w_08919630;
extern int duel_w_08919634;
extern int duel_w_08919638;
extern int duel_w_0891963C;

extern int (*duel_end_steps[])(void);  /* 0x089125F0, ends with 0 */
extern int (*duel_end_steps2[])(void); /* 0x0891260C, ends with 0 */
extern int duel_w_089195FC;
extern int duel_w_089195F8;
extern int duel_w_089195F4;
extern int duel_w_08913C14;
extern int duel_w_08913C18;
extern int duel_stats[0x35];           /* 0x08913C1C counters of the duel record */
extern unsigned int duel_stat_bits[];  /* 0x08913CF0 flags from index 0x36 */

/* Why player p wins (0: not yet). */
int duel_0884D050(unsigned int p)
{
    unsigned int q = (1 - p) & 1;

    if (duel.players[q].unk20[0] != 0) {
        return 2;
    }
    if (duel.players[p & 1].unk20[0] != 0) {
        return 0;
    }
    if (duel_0884CF30(p) != 0) {
        return 3;
    }
    if (duel_0884CFBC(p) != 0) {
        return 4;
    }
    if (duel_slot_effect_kind(duel_turn_player, 11, 0x151E, 5) != 0) {
        duel_w_08917DC4 = 1;
        if (duel_count_monsters(p) > 0) {
            return 6;
        }
        return 0;
    }
    if (duel_slot_effect_kind(p, 11, 0x169C, 1) != 0) {
        return 7;
    }
    if (duel.players[p & 1].unk20[2] != 0) {
        return 8;
    }
    if (duel.players[q].lp == 0) {
        return 1;
    }
    if (duel_turn >= duel_w_089195D8 + 4U) {
        if (duel.players[q].lp < duel.players[p & 1].lp) {
            return 9;
        }
    }
    return 0;
}

int duel_0884D1E0(void)
{
    int a;
    int b;
    unsigned int q;

    if (duel_w_089195EC != 0 && duel_player_type[duel_turn_player] == 2) {
        return 0;
    }
    a = duel_0884D050(duel_view_player);
    b = duel_0884D050(duel_view_player ^ 1);
    if (a != 0 || b != 0 || duel_w_08917DC4 != 0) {
        q = duel_view_player ^ 1;
        duel.players[duel_view_player & 1].unk20[3] = a;
        duel.players[q & 1].unk20[3] = b;
        if (a != 0 && b == 0) {
            duel_w_089195E4 = 1;
        } else if (a == 0 && b != 0) {
            duel_w_089195E4 = 2;
        } else {
            duel_w_089195E4 = 3;
        }
        duel_w_08917DC4 = 1;
        return 1;
    }
    return 0;
}

int duel_0884D310(void)
{
    unsigned int v = duel_view_player;
    unsigned int u;

    if (duel_0884B08C() != 0) {
        return 0;
    }
    switch (duel_w_08919600) {
    case 0:
        if (duel_w_089195EC != 0 && duel_player_type[duel_turn_player] == 2) {
            duel_w_08919600 = 2;
            return 0;
        }
        if (duel.players[0].unk20[3] == 3 || duel.players[1].unk20[3] == 3) {
            u = duel.players[v & 1].unk20[3] == 3 ? v : 1 - v;
            duel_cmd((u ? 0x8000 : 0) | 6, 0, 0, 0);
        }
        if (duel.players[0].unk20[3] == 4 || duel.players[1].unk20[3] == 4) {
            u = duel.players[v & 1].unk20[3] == 4 ? v : 1 - v;
            duel_cmd((u ? 0x8000 : 0) | 7, 0, 0, 0);
        }
        if (duel.players[0].unk20[3] == 7 || duel.players[1].unk20[3] == 7) {
            u = duel.players[v & 1].unk20[3] == 7 ? v : 1 - v;
            duel_cmd((u ? 0x8000 : 0) | 8, 0, 0, 0);
        }
        duel_cmd((v ? 0x8000 : 0) | 5, duel_w_089195E4, duel.players[0].unk20[3], duel.players[1].unk20[3]);
        duel_w_08919600++;
        break;
    case 1:
        if (duel_busy_08915238 == 0) {
            if (duel_w_089195EC != 0) {
                duel_msg_send3(2, duel_w_089195E4, 0, 0);
            }
            if (duel_player_type[v] == 0) {
                duel_0884B050();
            }
            duel_w_08919600++;
        }
        break;
    case 2:
        if (duel_w_089195EC == 0 || (duel_msg_state & 1)) {
            return 1;
        }
        break;
    }
    return 0;
}

void duel_0884D618(int all)
{
    while (!duel_log_at_end() != 0) {
        DuelLogEnt *e = duel_log_cur();

        duel_088D4CD8(e->v[0], e->v[1], e->v[2], e->v[3]);
        duel_log_next();
        if (all == 0) {
            break;
        }
    }
}

int duel_0884D690(void)
{
    if (duel_slot_has_effect(duel_turn_player, 11, 0x151E) != 0 && duel_0884356C() == 0) {
        return 0;
    }
    if (duel_end_steps[duel_w_089195FC] != 0) {
        if (duel_end_steps[duel_w_089195FC]() != 0) {
            duel_w_08919600 = 0;
            duel_w_089195FC++;
        }
        return 0;
    }
    return 1;
}

#ifdef NON_MATCHING /* branch layout of the end checks not reproduced */
int duel_0884D72C(void)
{
    unsigned int t = duel_turn_player;
    int r;

    if (duel_player_type[0] == 3) {
        return duel_log_at_end();
    }
    if (duel_w_089195EC == 0 || !(duel_msg_state & 1)) {
        if (duel_w_08917DC4 == 0) {
            r = duel_0884B08C();
            if (r == 0) {
                r = duel_0884DFB0();
            }
            if (r == 0) {
                r = duel_088321B4();
            }
            if (r == 0) {
                r = duel_08824D34();
            }
            if (r == 0) {
                r = duel_0884E1D8();
            }
            if (r == 0) {
                r = duel_088535C0();
            }
            duel_w_089195F4 = !(r != 0);
            if (r != 0) {
                return 0;
            }
            if (duel_0884D1E0() != 0) {
                return 1;
            }
            if (duel_player_type[t] == 2) {
                duel_0883447C();
            } else {
                duel_0884D690();
            }
            return 0;
        }
        return 1;
    }
    return 1;
}
#endif

#ifdef NON_MATCHING /* branch layout of the end checks not reproduced */
int duel_0884D874(void)
{
    if (duel_end_steps2[duel_w_089195F8] == 0) {
        duel_w_08913C14 = duel_w_089195E4;
        if (duel_w_089195E4 == 2) {
            duel_w_08913C18 = duel.players[(duel_view_player ^ 1) & 1].unk20[3];
        } else {
            duel_w_08913C18 = duel.players[duel_view_player & 1].unk20[3];
        }
        return 1;
    }
    if (duel_w_089195EC != 0 && duel_088F7418() == 0) {
        return 1;
    }
    if (duel_088D557C() != 0) {
        return 0;
    }
    if (duel_end_steps2[duel_w_089195F8]() != 0) {
        duel_w_08919600 = 0;
        duel_w_089195F8++;
    }
    return 0;
}
#endif

unsigned int duel_stat_get(int i)
{
    int j;

    if (i < 0x35) {
        return duel_stats[i];
    }
    j = i - 0x36;
    return duel_stat_bits[j / 32] >> (j % 32) & 1;
}

void duel_0884DA24(int i, int v)
{
    int j;

    if (i < 0x35) {
        duel_stats[i] = v;
    } else {
        j = i - 0x36;
        if (v != 0) {
            duel_stat_bits[j / 32] |= 1 << (j % 32);
        } else {
            duel_stat_bits[j / 32] &= ~(1 << (j % 32));
        }
    }
}

void duel_0884DAF0(int i, int v)
{
    if ((int)duel_stat_get(i) < v) {
        duel_0884DA24(i, v);
    }
}

void duel_0884DB38(int i, int v)
{
    duel_0884DA24(i, v + duel_stat_get(i));
}

int duel_card_used(unsigned int id)
{
    return duel_card_used_e66ac(id);
}

void duel_0884DB7C(void)
{
    if (duel_08827544() != 0) {
        duel_0884DB38(0x1A, 1);
    }
}

#ifdef NON_MATCHING /* branch layout of the end checks not reproduced */
void duel_0884DBAC(DuelAct *a, int i)
{
    int f;

    if (duel_view_player != (a->player ^ a->opp)) {
        return;
    }
    if (card_get_type(a->id) == 0x17) {
        duel_0884DA24(0x3A, 1);
        if (a->b3_4 != 0) {
            return;
        }
        duel_0884DB38(0x21, 1);
        if (card_get_prop5_hi(a->id) != 1) {
            return;
        }
        f = 0;
        if (i >= 1 && a[-1].player != a->player) {
            f = 1;
        } else {
            if (a->id == 0x12C4) {
                f = duel_turn_player != a->player;
            }
            if (a->kind == 0x21 || a->kind == 0x20 || a->kind == 0x1F) {
                f = a->y.y15_1 != a->player;
            }
        }
        if (f) {
            duel_0884DB38(0x22, 1);
        }
        return;
    }
    if (card_get_type(a->id) != 0x16) {
        return;
    }
    duel_0884DA24(0x39, 1);
    if (a->b3_4 != 0) {
        return;
    }
    duel_0884DB38(0x1F, 1);
    if (duel_turn_player != (duel_view_player ^ 1)) {
        return;
    }
    if (card_get_prop5_hi(a->id) != 5) {
        return;
    }
    duel_0884DB38(0x20, 1);
}
#endif

#ifdef NON_MATCHING /* register allocation of the two loops not reproduced */
void duel_0884DD94(void)
{
    int ids[5];
    int n = 0;
    int k = 0;
    int z;
    int *pp = ids;

    for (z = 0; z <= 4; z++) {
        int id = field_get_card(duel_view_player, z);

        if (id != 0) {
            *pp++ = id;
            n++;
            duel_0884DAF0(0, duel_field_atk(duel_view_player, z));
        }
        if (duel_zone_usable(duel_view_player ^ 1, z) == 0) {
            k++;
        }
    }
    if (n >= 3) {
        int i;

        for (i = 0; i <= n - 3; i++) {
            if (card_is_attribute9(ids[i]) == 0) {
                int j;
                int c = 0;

                for (j = i + 1; j < n; j++) {
                    if (card_same_name(ids[i], ids[j]) != 0) {
                        c++;
                    }
                }
                if (c > 1) {
                    duel_0884DA24(0x3B, 1);
                }
            }
        }
    }
    if (n == 5) {
        duel_0884DA24(0x3C, 1);
    }
    if (k == 5) {
        duel_0884DA24(0x3D, 1);
    }
}
#endif

int duel_0884DF2C(int (*fn)(unsigned int p, int z, int a))
{
    int n = 0;
    unsigned int p;
    int z;

    for (p = 0; (int)p < 2; p++) {
        for (z = 0; z <= 10; z++) {
            if (fn(p, z, 0) != 0) {
                n++;
            }
        }
    }
    return n;
}

int duel_0884DFB0(void)
{
    if (duel_req_flag(3) != 0) {
        return 1;
    }
    if (duel_w_08919630 != 0) {
        if (!duel_w_08919634) {
            duel_w_08919634 = 1;
            duel_host_call(3, 0, 0, 0);
        }
        return 1;
    }
    duel_w_08919634 = 0;
    return 0;
}

int duel_0884E1D8(void)
{
    if (duel_w_0891963C == 0) {
        return 0;
    }
    duel_0884E02C();
    if (duel_w_08919638 == 0) {
        duel_w_0891963C = 0;
    }
    return 1;
}
