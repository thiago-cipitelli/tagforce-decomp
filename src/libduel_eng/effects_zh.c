// @range 0x08894158-0x08894A84
// @data 0x089118DC-0x0891190C
// @asm 0x08894500-0x088946B4
/* Card effect handlers (DuelAct *a), after effects_zg.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern unsigned short duel_w_0891968C;
extern unsigned short duel_h_0891968E;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern int duel_w_08919650;
extern int duel_w_08919654;
extern unsigned int duel_turn_player;

int duel_08886B34(DuelAct *a, DuelAct *b);
unsigned int duel_0884E4E4(DuelAct *a);
void duel_088D184C(DuelAct *a, unsigned int mask, int b);
int duel_0884E398(DuelAct *a, int b);
void duel_08832870(unsigned int p, int a);
int duel_0880B17C(unsigned int p, int i);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_08832974();
void duel_088C77EC(unsigned int p, int uid);
int card_is_monster(int id);
int card_get_type(int id);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
int duel_0884E9E8(DuelAct *a, int b);
void duel_088328E8(unsigned int p, unsigned short id, unsigned short loc);
int duel_088D1858(DuelAct *a, unsigned int p, int z);
int duel_hand_find_uid(unsigned int p, int uid);
void duel_088CAB04();
int duel_rand(int n);
void duel_0884AF88(int msg);
void duel_08832AF4(unsigned int p, int z, int a, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
void duel_088C625C(unsigned int p, int a, int b);
int duel_0884E5E0(DuelAct *a);
void duel_0884ACD8(unsigned int p, void *msg);
void duel_088359E8(unsigned int p, int id, int (*fn)(DuelAct *a, unsigned int p, int z));
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_08835AC0(void);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_088D1D58(DuelAct *a, int b);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
int duel_088998D8(DuelAct *a, int b);
int duel_0880B30C(unsigned int p);
void duel_088329D0(unsigned int p, int a, int b);
void duel_088C6728(unsigned int p, int uid, int a);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);

int duel_08894158(DuelAct *a, DuelAct *b)
{
    if (a->w[2] != 0) {
        return duel_08886B34(a, b);
    }
    return 0;
}

int duel_08894180(DuelAct *a)
{
    duel_088D184C(a, duel_0884E4E4(a), 2);
    return 0;
}

int duel_088941B8(DuelAct *a, int b)
{
    int id;
    int ok;

    switch (duel_step) {
    case 0x80:
        if (duel_0884E398(a, b) == 0) {
            return 0;
        }
        duel_08832870(1 - a->player, 0);
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel_w_0891968C;
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = duel_0880B17C(1 - a->player, duel_w_0891968C);
        } else {
            duel_0884ADAC(0x104, 0x1B8, 0x1B9, 0x1BA, 0, 0xF);
        }
        return 0x7E;
    case 0x7E:
        a->w[0] = duel_w_08919624;
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        return 0x7D;
    case 0x7D:
        duel_088C77EC(1 - a->player, CARD_UID(duel.players[1 - a->player & 1].hand[duel_w_089148FC]));
        ok = 0;
        id = duel.players[1 - a->player & 1].hand[duel_w_089148FC].id;
        switch (a->w[0]) {
        case 0:
            ok = card_is_monster(id);
            break;
        case 1:
            ok = card_get_type(id) == 0x16;
            break;
        case 2:
            ok = card_get_type(id) == 0x17;
            break;
        }
        if (ok) {
            duel_088C4604(a, 1 - a->player, 700);
        }
        return 0;
    }
    return 0;
}

int duel_08894408(DuelAct *a, int b)
{
    switch (duel_step) {
    case 0x80:
        return duel_0884E9E8(a, b) != 0 ? 0x7F : 0;
    case 0x7F:
        duel_088328E8(1 - a->player, a->id, 0);
        return 0x7E;
    case 0x7E:
        duel_088D1858(a, duel_w_0891968C & 0xFF, duel_w_0891968C >> 8 & 0xFF);
        return 0;
    }
    return 0;
}

int duel_088944A0(DuelAct *a)
{
    int i = duel_hand_find_uid(1 - a->player, a->src.uid);

    if (i >= 0) {
        duel_088CAB04(1 - a->player, i, 1);
    }
    return 0;
}

#ifdef NON_MATCHING /* duel_step despachado em $a2 (original: $v1) por causa da alocação do caso 0x7D */
int duel_08894500(DuelAct *a, int b)
{
    int n;

    switch (duel_step) {
    case 0x80:
        if (duel_0884E9E8(a, b) == 0) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = duel_rand(2);
        } else {
            duel_0884AF88(0x38);
        }
        return 0x7F;
    case 0x7F:
        duel_08832AF4(a->player, a->zone, 1, duel_w_08919624);
        return 0x7E;
    case 0x7E:
        duel_088C7BA4();
        return 0x7D;
    case 0x7D:
        if (duel_h_0891968E != 0) {
            n = duel.players[1 - a->player & 1].n_grave;
            if (n == 0) {
                return 0x7C;
            }
            duel_088CB400(1 - a->player, &duel.players[1 - a->player & 1].grave[n - 1], 1);
            return 0x7D;
        }
        duel_088C625C(a->player, duel.players[1 - a->player & 1].n_grave, 0);
        return 0x7C;
    case 0x7C:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}
#endif

int duel_088946B4(DuelAct *a)
{
    unsigned int m;
    int i;
    unsigned int q;

    if (a->b3_4 == 0) {
        switch (duel_step) {
        case 0x80:
            if (duel.mode & 1) {
                return 0;
            }
            duel_w_089148FC = 0x10;
            if (duel_0884E5E0(a) != 0) {
                duel_0884ACD8(a->player, (void *)0x7C);
                duel_088359E8(a->player, a->id, duel_0884E440);
                return 0x7F;
            }
            return 0x7D;
        case 0x7F:
            if (duel_08835AC0() != 0) {
                duel_w_089148FC = duel_w_08919650 + duel_w_08919654;
                return 0x7E;
            }
            return 0x80;
        case 0x7E:
            if (duel_w_089148FC < 5) {
                duel_088C87A8(a->player, duel_w_089148FC, a->id, 1, 0);
            }
            return 0x7D;
        case 0x7D:
            m = -1;
            if (duel_w_089148FC != 0x10) {
                m &= ~(1 << (duel_w_089148FC + a->player * 16));
            }
            duel_088D1D58(a, m);
            return 0x7C;
        case 0x7C:
            for (i = 0; i < 2; i++) {
                q = duel_turn_player ^ i;
                if (!duel.players[q & 1].n_hand) {
                    continue;
                }
                duel_088CAE24(q, &duel.players[q & 1].hand[0], 0);
                return 0x7C;
            }
            return 0x7B;
        case 0x7B:
            duel_088C77D4(1 - a->player, a->id, 1, 0);
            return 0;
        }
    }
    return 0;
}

int duel_088948B8(DuelAct *a, int b)
{
    if (a->kind == 2) {
        return duel_088998D8(a, b);
    }
    switch (duel_step) {
    case 0x80:
        if (duel.players[1 - a->player & 1].n_d == 0) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = duel_0880B30C(1 - a->player);
        } else {
            duel_0884ADAC(0x104, 0x1B8, 0x1B9, 0x1BA, 0, 0xF);
        }
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel_w_08919624;
        duel_08832974(1 - a->player, a->id, duel_w_089148FC);
        return 0x7E;
    case 0x7E:
        duel_088329D0(1 - a->player, a->id, duel_w_089148FC);
        return 0x7D;
    case 0x7D:
        if (duel_w_0891968C != 0) {
            duel_088C6728(1 - a->player, duel_h_0891968E, 1);
            return 0x7B;
        }
        duel_0884ACD8(a->player, (void *)0xD);
        return 0x7C;
    case 0x7C:
        duel_088328FC(a->player);
        return 0x7B;
    case 0x7B:
        duel_088C78E0(1 - a->player);
        return 0;
    }
    return 0;
}
