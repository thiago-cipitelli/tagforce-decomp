// @range 0x08895C8C-0x088966D8
/* Card effect handlers (DuelAct *a), after effects_zh.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern unsigned short duel_w_0891968C;
extern int duel_w_08919624;
extern int duel_player_type[2];

int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_088D1858(DuelAct *a, unsigned int p, int z);
int duel_0886CF1C(DuelAct *a, int b);
void duel_08832894(unsigned int p, unsigned short id);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_0886CE38();
int field_get_card(unsigned int p, int z);
int card_is_attribute9(int id);
int card_is_monster(int id);
int duel_088BFF78(unsigned int p, int id, int b);
void duel_0884ACD8(unsigned int p, void *msg);
int duel_count_free_monster_zones(unsigned int p);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
DuelCard *duel_0884CCBC(int k);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
int duel_grave_find_uid(unsigned int p, int uid);
void duel_088CA428(unsigned int p, unsigned int uid, int a);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884C7CC(void);
void duel_08832974();
int duel_088E9150(int id);
void duel_0884AD18(int msg);
void duel_088CB554(unsigned int p, DuelCard *c);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_088C7880(unsigned int p);
void duel_088C7B90(int n);
unsigned int duel_card_locate(unsigned int uid);
void duel_088D147C(DuelAct *a, unsigned int p, int z);

int duel_08895C8C(DuelAct *a, int b)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    int r;

    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel_0884E440(a, p, z) != 0) {
                duel_w_089148FC = duel.players[p & 1].field[z].card.owner;
                duel_088D1858(a, p, z);
                return 0x7F;
            }
        }
        return 0;
    case 0x7F:
        a->opp = a->player != duel_w_089148FC;
        r = duel_0886CF1C(a, b);
        a->opp = 0;
        if (r == 0) {
            return 0;
        }
        duel_08832894(duel_w_089148FC, a->id);
        return 0x7E;
    case 0x7E:
        duel_088263A8(duel_w_089148FC, &duel.players[duel_w_089148FC & 1].hand[duel_w_0891968C], 1, a->id << 16);
        return 0;
    }
    return 0;
}

int duel_08895E4C(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    unsigned short n;

    switch (duel_step) {
    case 0x80:
        if (duel_0886CE38() == 0) {
            return 0;
        }
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel_0884E440(a, p, z) != 0) {
                z = field_get_card(p, z);
                duel_w_089148FC = z;
                if (card_is_attribute9(z)) {
                    return 0;
                }
                if (card_is_monster(z) == 0) {
                    return 0;
                }
                a->w[1] = duel_088BFF78(a->player, a->id, z);
                if ((n = a->w[1]) == 0) {
                    duel_0884ACD8(a->player, (void *)0xD);
                    return 0x6E;
                }
                a->w[1] = MIN(n, duel_count_free_monster_zones(a->player));
                duel_0884ACD8(a->player, (void *)0x11D);
                return 0x7E;
            }
        }
        return 0;
    case 0x7E:
        switch (a->id) {
        case 0x157A:
            duel_0884C588(a->player, MIN(a->w[1] > 2 ? 2 : a->w[1], 0x20) + 0x27, a->id, duel_w_089148FC);
            break;
        case 0x1978:
            duel_0884C588(a->player, MIN(a->w[1], 0x20) + 6, a->id, duel_w_089148FC);
            break;
        }
        return 0x7D;
    case 0x7D:
        a->w[0] = 0;
        a->w[1] = duel_w_08919624;
        duel_088C7BA4();
        return 0x7C;
    case 0x7C:
        if (a->w[0] < a->w[1]) {
            duel_088263A8(a->player, duel_0884CCBC(a->w[0] + 1), 1, a->id << 16);
            a->w[0]++;
            return 0x7C;
        }
        duel_088C7CF0();
        return 100;
    case 0x6E:
        duel_088328FC(1 - a->player);
        return 100;
    case 100:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_08896140(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            duel_088C87A8(p, z, a->id, 3, 0);
        }
    }
    return 0;
}

int duel_088961D0(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            switch (a->w[2]) {
            case 1:
                duel_088C97F0(a, p, z, 0, 1);
                return 0;
            case 2:
                duel_088C87D8(a, p, z, a->id, 1, a->player);
                return 0;
            }
        }
    }
    return 0;
}

int duel_088962A8(DuelAct *a)
{
    unsigned short *src;

    if (a->kind == 0x16) {
        src = (unsigned short *)&a->src;
        if (duel_grave_find_uid(1 - a->player, *src & 0x1FF) >= 0) {
            duel_088CA428(1 - a->player, *src & 0x1FF, 1);
        }
    }
    return 0;
}

int duel_0889632C(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            duel_0884ACD8(a->player, (void *)0xD);
            return 0x78;
        }
        duel_0884ACD8(a->player, (void *)0x5E);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7E;
    case 0x7E:
        duel_08832974(1 - a->player, a->id, duel_0884C7CC()->id);
        return 0x7D;
    case 0x7D:
        if (duel_088E9150(0x159D) != 0) {
            if (duel_player_type[a->player] == 1) {
                duel_w_08919624 = 1;
            } else {
                duel_0884AD18(0x131);
            }
        } else {
            duel_w_08919624 = 0;
        }
        return 0x7C;
    case 0x7C:
        if (duel_w_08919624 != 0) {
            duel_088CB554(a->player, duel_0884C7CC());
        } else {
            duel_088C78E0(a->player);
            duel_cmd((a->player ? 0x8000 : 0) | 0x57, 0xD, CARD_UID(*duel_0884C7CC()), 0);
            duel_088C7880(a->player);
            duel_088C7B90(0x1E);
            duel_088C7880(a->player);
            return 0;
        }
        return 100;
    case 0x78:
        duel_088328FC(1 - a->player);
        return 100;
    case 100:
        duel_088C78E0(a->player);
        return 1;
    }
    return 0;
}

int duel_08896594(DuelAct *a)
{
    unsigned int loc;

    if (a->kind == 0x16) {
        loc = duel_card_locate(duel_special.act[duel_special.act[0].uid == a->src.uid].uid);
        if (((unsigned short)loc >> 8 & 0xFF) < 5) {
            duel_088D147C(a, loc & 0xFF, (unsigned short)loc >> 8 & 0xFF);
        }
    }
    return 0;
}

int duel_08896624(DuelAct *a)
{
    switch (a->w[2]) {
    case 1:
        if (duel.players[a->player & 1].n_d != 0) {
            duel_088C78E0(a->player);
        }
        break;
    case 2:
        if (duel.players[1 - a->player & 1].n_d != 0) {
            duel_088C78E0(1 - a->player);
        }
        break;
    }
    return 0;
}
