// @range 0x08890820-0x088917D4
/* Card effect handlers (DuelAct *a), after effects_zd.c. */
#include "duel.h"

extern int duel_step;
extern unsigned short duel_w_0891968C;
extern unsigned int duel_turn_player;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2();
int duel_088BFF78(unsigned int p, int id, int b);
void duel_0884ACD8(unsigned int p, void *msg);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884C7CC(void);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_count_tributable_except(unsigned int p, unsigned int except);
void duel_088328B8(unsigned int p, unsigned short a, int b);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
int duel_grave_find_uid(unsigned int p, int uid);
void duel_088329BC(unsigned int p, unsigned short id, int b);
void duel_08832950(unsigned int p, unsigned short id);
void duel_088D1920(DuelAct *a, unsigned int mask);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_08832ADC(int a);
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
void duel_088C7CF0(void);
void duel_088C7BA4(void);
int duel_card_fits_zone(unsigned int p, int z, int id);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
int duel_slot_can_act(unsigned int p, unsigned int z, int x, int y);
void duel_088C8DC4(int loc, unsigned short t);
int card_get_special_group(unsigned short id);
int duel_08835AC0(void);
void duel_088D147C(DuelAct *a, unsigned int p, int z);
int duel_0884E5E0(DuelAct *a);
void duel_088359E8(unsigned int p, int id, int (*fn)(DuelAct *a, unsigned int p, int z));
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
unsigned int duel_0884E4E4(DuelAct *a);
void duel_08832B18(unsigned int p, int z);
void duel_088D192C(DuelAct *a, unsigned int p, int z);
void duel_088C78E0(unsigned int p);
int duel_088D14B8(DuelAct *a, unsigned int p, int z);
void duel_088C9344(DuelAct *a, unsigned int p, int z, int b);
void duel_088C4508(unsigned int p, int id);
void duel_088C7B90(int n);
int card_get_type(int id);
int duel_0886AF00(unsigned int p, int z, int id);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_0884F91C(unsigned int a, unsigned short b);
int duel_locks_ok(int p, int monster);
unsigned int duel_act_target2(DuelAct *a, int i);
int duel_f_find_uid(unsigned int p, int uid);
void duel_088CB490(unsigned int p, int uid);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);

int duel_08890820(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            return 0;
        }
        if (a->id == 0x1481) {
            if (duel_count_tributable_except(a->player, (unsigned short)(a->player | a->zone << 8)) == 0) {
                return 0;
            }
            duel_088328B8(a->player, a->player | a->zone << 8, 1);
        }
        return 0x7F;
    case 0x7F:
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, (void *)0x1A);
        return 0x7E;
    case 0x7E:
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7D;
    case 0x7D:
        duel_088263A8(a->player, duel_0884C7CC(), 1, a->id << 16);
        return 0;
    }
    return 0;
}

int duel_0889098C(DuelAct *a)
{
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup != 0) {
        duel_088C87A8(a->player, a->zone, a->id, 0x12, a->w[0] * 300);
    }
    return 0;
}

int duel_08890A64(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            if (a->id == 0x1485) {
                duel_088C7E68(p, z, 0x15, 1);
            }
            duel_088C97F0(a, p, z, 0, 0);
        }
    }
    return 0;
}

int duel_08890B18(DuelAct *a)
{
    unsigned int q;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(1 - a->player, a->id, 0) == 0) {
            return 0;
        }
        if (duel_count_free_monster_zones(1 - a->player) == 0) {
            return 0;
        }
        if (duel_phase_ok2(1 - a->player) == 0) {
            return 0;
        }
        duel_08832950(1 - a->player, a->id);
        return 0x7F;
    case 0x7F:
        if (duel_w_0891968C != 0) {
            duel_088329BC(1 - a->player, a->id, 0);
            return 0x7E;
        }
        return 0;
    case 0x7E:
        q = 1 - a->player;
        duel_088263A8(q, &duel.players[q & 1].grave[duel_grave_find_uid(q, duel_w_0891968C)], 1, 0x20);
        return 0x7D;
    }
    return 0;
}

int duel_08890C90(DuelAct *a)
{
    unsigned int mask;
    int i;
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (a->b4_2) {
        return 0;
    }
    if (a->n7 == 2) {
        mask = 0;
        for (i = 0; i < a->n7; i++) {
            t = duel_act_target(a, i);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel_088A9008(a, i, 0) != 0) {
                mask |= 1 << (z + p * 16);
            }
        }
        duel_088D1920(a, mask);
    }
    return 0;
}

int duel_08890D6C(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    switch (duel_step) {
    case 0x80:
        if (a->b4_2) {
            return 0;
        }
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            duel_cmd((1 - a->player ? 0x8000 : 0) | 0x19, *(unsigned short *)&duel_special.zone[0], z, p);
            return 0x7F;
        }
        return 0;
    case 0x7F:
        duel_08832ADC(1);
        return 0;
    }
    return 0;
}

int duel_08890E3C(DuelAct *a, DuelAct *b)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    int i;
    unsigned int q;
    int sz;

    if (a->b4_2) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        switch (duel_step) {
        case 0x80:
            a->w[1] = 0;
            a->w[2] = 0;
            duel_088C7BA4();
            return 0x7F;
        case 0x7F:
            for (i = 0; i < 2; i++) {
                q = duel_turn_player ^ i;
                for (sz = 5; sz < 10; sz++) {
                    if (duel.players[q & 1].field[sz].card.id != 0 &&
                        duel_card_fits_zone(q, sz, duel.players[q & 1].field[sz].card.id) != 0 &&
                        duel_st_affects(p, z, q, sz) == 0) {
                        if (duel_slot_can_act(q, sz, p, z) != 0) {
                            if (duel.players[q & 1].field[sz].flags >> 1 & 1) {
                                if (q == b->player && sz == b->zone) {
                                    b->w[0] = a->w[0];
                                }
                            } else {
                                duel_088C8DC4((unsigned short)((unsigned char)q | (unsigned char)sz << 8), t);
                                return 0x7F;
                            }
                        } else {
                            a->w[1 + q] |= 1 << sz;
                        }
                    }
                }
            }
            return 0x7E;
        case 0x7E:
            duel_088D13CC(a, a->w[1] | a->w[2] << 16);
            duel_088C7CF0();
            return 0;
        }
    }
    return 0;
}

int duel_08891088(DuelAct *a, DuelAct *b)
{
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (b != 0 && card_get_special_group(b->id) == 1) {
        a->b4_2 = a->b4_1 = 1;
    }
    return 0;
}

int duel_0889117C(DuelAct *a)
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
    if (a->w[0] == 0) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_0884E5E0(a) == 0) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            for (i = 0; i < 2; i++) {
                q = a->player ^ !i;
                for (z = 0; z <= 4; z++) {
                    if (!duel_0884E440(a, q, z)) {
                        continue;
                    }
                    duel_088D147C(a, q, z);
                    return 0;
                }
            }
            return 0;
        }
        duel_0884ACD8(a->player, (void *)0x6D);
        duel_088359E8(a->player, a->id, duel_0884E440);
        return 0x7F;
    case 0x7F:
        if (duel_08835AC0() != 0) {
            duel_088D147C(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return 0;
        }
        return 0x80;
    }
    return 0;
}

int duel_0889138C(DuelAct *a)
{
    duel_088C77D4(a->player, a->id, 4, 0);
    return 0;
}

int duel_088913BC(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_08832B18(a->player, a->zone);
        return 0x7F;
    case 0x7F:
        a->w[0] = duel_w_0891968C;
        duel_088D13CC(a, duel_0884E4E4(a));
        return 0;
    }
    return 0;
}

int duel_08891438(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    unsigned int o;

    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            t = duel.players[p & 1].field[z].card.id;
            if (t != 0 && duel.players[p & 1].field[z].faceup == 0) {
                duel_088C9344(a, p, z, 0);
                duel_088C4508(p, t);
                duel_088C7B90(0x1E);
                if (card_get_type(t) == 0x17 && !a->b4_4) {
                    if (duel_0886AF00(p, z, t) != 0) {
                        duel_088C4448(p, z, CARD_UID(duel.players[p & 1].field[z].card));
                        duel_0884F91C((unsigned short)t | ((p << 31 & 0x80000000) | 0x44000000 | (z << 16 & 0x1F0000)),
                                      CARD_UID(duel.players[p & 1].field[z].card));
                    } else {
                        return 0x7F;
                    }
                } else {
                    duel_088C9344(a, p, z, 0);
                }
            }
        }
        return 100;
    case 0x7F:
        t = duel_act_target(a, 0);
        duel_088D14B8(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF);
        return 100;
    case 100: {
        unsigned char sz = a->zone;
        unsigned int sp = a->player;

        if (duel.players[sp & 1].field[sz].card.id != 0) {
            o = duel.players[sp & 1].field[sz].card.owner;
            duel_088D192C(a, sp, sz);
            duel_088C78E0(o);
        }
        return 0;
    }
    }
    return 0;
}

int duel_088916E8(DuelAct *a)
{
    unsigned int uid;
    unsigned char n;

    if (duel_locks_ok(a->player, 1) == 0) {
        return 0;
    }
    if ((n = a->n7) != 0) {
        uid = duel_act_target2(a, n - 1);
        if (duel_f_find_uid(a->player, uid) >= 0) {
            duel_088CB490(a->player, uid);
        }
        a->n7--;
        return 0x80;
    }
    return 0;
}

int duel_08891784(DuelAct *a)
{
    unsigned int x = 0;

    switch (a->id) {
    case 0x14E2:
        x = a->uid;
    }
    duel_088C77D4(a->player, a->id, 1, x);
    return 0;
}
