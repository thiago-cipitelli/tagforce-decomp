// @range 0x0884D050-0x0884D690
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
