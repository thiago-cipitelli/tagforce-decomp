// @range 0x088FC114-0x088FCA90
/* Card effect handlers (continued). */
#include "duel.h"

extern int duel_player_type[2];
extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08919624;
extern int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

unsigned int duel_act_target(DuelAct *a, int i);
int duel_088A9008(DuelAct *a, int b, int c);
int duel_0884E440();
int duel_0884E690(DuelAct *a, int (*fn)(int id));
int duel_08856754(int id);
int duel_0886CE38();
int duel_08861480(unsigned short id, int i);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_08832974(unsigned int p, unsigned short id, int a);
int duel_pick_monster_zone();
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);
void duel_0884ACD8(unsigned int p, int msg);
void duel_088359E8(unsigned int p, int id, int (*fn)(int id));
int duel_08835AC0(void);
void duel_088D147C(DuelAct *a, int b, int c);
void duel_088C452C(unsigned int p, int n);
void duel_088CBAA4(unsigned int p, int z, unsigned int uid);
void duel_088D1858(DuelAct *a, unsigned int p, int z);
int duel_0886CE90(DuelAct *a, int b);
int duel_count_free_monster_zones(unsigned int p);
int duel_088BFF78(unsigned int p, unsigned short id, int a);
void duel_0884C588(unsigned int p, int a, unsigned short id, int b);
DuelCard *duel_0884CCBC(int i);
void duel_088C7CF0(void);
void duel_088C7BA4(void);
int duel_0884E4E4(DuelAct *a);
void duel_088D1D58(DuelAct *a, int b);
int duel_hand_find_uid(unsigned int p, int uid);
void duel_088C7810(unsigned int p, int a);
void duel_08832950(unsigned int p, unsigned short id);
unsigned int duel_card_locate(unsigned int uid);
void duel_088CAF24(DuelAct *a, unsigned int p, DuelCard *c);
int duel_0884E9E8(DuelAct *a, int b);
int duel_0887F8E4(DuelAct *a, int b);

int duel_eff_fc114(DuelAct *a, int b)
{
    unsigned int t = duel_act_target(a, 0);
    unsigned int p = t & 0xFF;
    unsigned int z = (unsigned char)((t & 0xFFFF) >> 8);

    switch (duel_step) {
    case 0x80: {
        unsigned int n;

        if (duel_088A9008(a, 0, 0) == 0) {
            return 0;
        }
        if (duel_0884E440(a, p, z) == 0) {
            return 0;
        }
        n = 1;
        if (duel_0884E690(a, duel_08856754)) {
            n |= 2;
        }
        if (duel_0886CE38(a, b)) {
            n |= 4;
        }
        if (duel_player_type[a->player] == 1) {
            if (n & 4) {
                duel_w_08919624 = 2;
            } else if (n & 2) {
                duel_w_08919624 = 1;
            } else {
                duel_w_08919624 = 0;
            }
        } else {
            duel_0884ADAC(0x103, duel_08861480(a->id, 0), duel_08861480(a->id, 1), duel_08861480(a->id, 2), 0, n);
        }
        return 0x7F;
    }
    case 0x7F:
        a->w[2] = duel_w_08919624 + 1;
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        return 0x7E;
    default:
        switch (a->w[2]) {
        case 1:
            duel_088C87D8(a, p, z, a->id, 3, 0);
            return 0;
        case 2:
            switch (duel_step) {
            case 0x7E:
                duel_0884ACD8(a->player, 0x8E);
                duel_088359E8(a->player, a->id, duel_08856754);
                return 0x7D;
            case 0x7D:
                if (duel_08835AC0()) {
                    duel_088D147C(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
                    return 0;
                }
                return 0x7E;
            }
            return 0;
        case 3:
            switch (duel_step) {
            case 0x7E: {
                DuelCard c = {0};

                c.id = 0x19ED;
                c.owner = a->player;
                duel_w_089148FC = duel_pick_monster_zone();
                duel_088263A8(a->player, &c, 1, a->id << 16 | 1);
                return 0x7D;
            }
            case 0x7D:
                duel_088C87A8(p, z, a->player | (duel_w_089148FC & 0xFF) << 8, 6, 0);
                break;
            }
            return 0;
        }
        break;
    }
    return 0;
}

int duel_eff_fc47c(DuelAct *a)
{
    duel_088C452C(a->player, 3000);
    duel_088C452C(1 - a->player, 3000);
    return 0;
}

int duel_eff_fc4c8(DuelAct *a)
{
    DuelSrc *s = &a->src;

    duel_088CBAA4(s->player, s->zone, s->uid);
    duel_088D1858(a, s->player, s->zone);
    return 0;
}

int duel_eff_fc534(DuelAct *a, int b)
{
    unsigned int q = a->player ^ a->opp;

    switch (duel_step) {
    case 0x80:
        if (duel_0886CE90(a, b) == 0) {
            return 0;
        }
        a->w[1] = MIN(duel_count_free_monster_zones(q), duel_088BFF78(q, a->id, 0));
        if (a->w[1] == 0) {
            return 0;
        }
        duel_0884ACD8(q, 0x11D);
        return 0x7E;
    case 0x7E: {
        int n = a->w[1];

        n = MIN(n, 32);
        duel_0884C588(q, n + 6, a->id, 0);
        a->w[0] = 0;
        duel_088C7BA4();
        return 0x7D;
    }
    case 0x7D:
        if (a->w[0] < a->w[1]) {
            duel_w_089148FC = duel_pick_monster_zone(q);
            duel_088263A8(q, duel_0884CCBC(a->w[0] + 1), 1, a->id << 16);
            return 0x7C;
        }
        duel_088C7CF0();
        return 0;
    case 0x7C:
        duel_088C87A8(q, duel_w_089148FC, a->player | a->zone << 8, 6, 0);
        a->w[0]++;
        return 0x7D;
    }
    return 0;
}

int duel_eff_fc704(DuelAct *a)
{
    duel_088D1D58(a, duel_0884E4E4(a));
    return 0;
}

int duel_eff_fc738(DuelAct *a)
{
    int i = duel_hand_find_uid(a->player, a->src.uid);

    switch (duel_step) {
    case 0x80: {
        unsigned int p = a->player;

        if (duel.players[(1 - p) & 1].n_hand == 0) {
            return 0;
        }
        if (i < 0) {
            duel_088C7810(p, 1);
            return 0x6E;
        }
        duel_088263A8(p, &duel.players[p & 1].hand[i], 1, 0);
        return 0;
    }
    case 0x6E:
        duel_08832950(1 - a->player, a->id);
        return 100;
    case 100:
        duel_088C7810(a->player, 0);
        return 0;
    }
    return 0;
}

int duel_eff_fc850(DuelAct *a)
{
    unsigned int t = duel_card_locate(a->src.uid);
    unsigned int p = t & 0xFF;
    unsigned int i = (unsigned short)(t >> 16);
    unsigned int z = (unsigned char)((t & 0xFFFF) >> 8);

    if (z == 0xE) {
        duel_088CAF24(a, p, &duel.players[p & 1].grave[i]);
    }
    return 0;
}

int duel_eff_fc8cc(DuelAct *a, int b)
{
    int i;

    if (duel_0886CE38(a, b) && (i = duel_hand_find_uid(a->player, a->w[0])) >= 0) {
        unsigned int p = a->player;

        duel_088263A8(p, &duel.players[p & 1].hand[i], 1, a->id << 16);
    }
    return 0;
}

int duel_eff_fc954(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_0884E9E8(a, b) != 0) {
        return duel_0887F8E4(a, b);
    }
    return 0;
}

int duel_eff_fca3c(DuelAct *a)
{
    unsigned int q = 1 - a->player;

    duel_088C452C(q, (duel.players[q & 1].lp + 1) >> 1);
    return 0;
}
