// @range 0x088A49F8-0x088A5B68
/* Card effect handlers (DuelAct *a), after effects_088A3430.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08919624;

int duel_0886CE38(DuelAct *a, int b);
int duel_grave_count_id(unsigned int p, int id);
int duel_0886CD1C(DuelAct *a, unsigned int p);
int duel_088AA224(DuelAct *a);
int duel_088BFF78(unsigned int p, int id, int b);
void duel_0884ACD8(unsigned int p, int msg);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
DuelCard *duel_0884CCBC(int k);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_088C7810(unsigned int p, int a);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);
unsigned int duel_0884E4E4(DuelAct *a);
void duel_088D184C(DuelAct *a, unsigned int mask, int b);
int duel_088DFE24(unsigned int p, unsigned int q);
int card_is_monster(int id);
int card_get_type(int id);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
extern int duel_player_type[2];
extern int duel_w_08919654;
extern unsigned short duel_w_0891968C;
extern unsigned short duel_h_0891968E;
int duel_0884E9E8(DuelAct *a, int b);
void duel_08832894(unsigned int p, unsigned short id);
void duel_088CA384(unsigned int p, int i, int f);
void duel_08832B28(unsigned int p, int z, int a);
void duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_08832880(unsigned int p, int id, int a);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088D1970(DuelAct *a, unsigned int p, int z);
int duel_field_level(unsigned int p, int z);
int duel_0887FCA8(int id);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(int id));
void duel_08835978(int (*fn)(int id));
int duel_08835AC0(void);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_zone_faceup(unsigned int p, int z, int i);
void duel_088C7880(unsigned int p);
void duel_088CB980(unsigned int p);
int card_is_group_b(int id);
int duel_card_dfac8(unsigned int p, int id, int field);
int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2(unsigned int p);
void duel_088C625C(unsigned int p, int a, int b);
int duel_088CB55C(unsigned int p, DuelCard *c, int f);
void duel_088C7DD4(unsigned int p, int a, int b);
int duel_0887AC5C(DuelAct *a);
int duel_rand(int n);
void duel_0884AF88(int msg);
void duel_08832AF4(unsigned int p, int z, int a, int b);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
int duel_field_atk(unsigned int p, int z);
int duel_field_def(unsigned int p, int z);
DuelCard *duel_0884C7CC(void);
void duel_088CB95C(unsigned int p, int z, int uid);
unsigned short *card_get_name(int id);
void duel_wfmt_s(unsigned short *dst, int fmt, unsigned short *arg);
void duel_088359E8(unsigned int p, int id, int (*fn)());
void duel_088CB728(unsigned int p, int z, int uid, unsigned int q, int qz, int a, int b);
int duel_pick_st_zone(unsigned int p);
void duel_088C8B70(unsigned int p, unsigned short loc, unsigned short t);
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
void duel_088C5E70(unsigned int p, int n);
int duel_act_target2(DuelAct *a, int i);
int duel_grave_find_uid(unsigned int p, int uid);
unsigned int duel_card_b14(DuelCard *c);
unsigned int duel_0884E538(DuelAct *a, int (*fn)());
int duel_08855F54();
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
int duel_bit_count(unsigned int m);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, unsigned short c);

int duel_088A49F8(DuelAct *a, int b)
{
    unsigned int p = a->player ^ a->opp;

    switch (duel_step) {
    case 0x80:
        if (duel_0886CE38(a, b) == 0) {
            return 0;
        }
        duel_w_089148FC = duel_grave_count_id(p, 0x1919);
        duel_w_089148FC = MIN(duel_w_089148FC, duel_0886CD1C(a, p));
        if (duel_w_089148FC == 0) {
            return 0;
        }
        if (duel_088BFF78(p, a->id, duel_088AA224(a)) == 0) {
            duel_0884ACD8(p, 0x122);
            return 0x78;
        }
        duel_0884ACD8(p, 0x11D);
        return 0x7F;
    case 0x7F:
        duel_0884C588(p, MIN(duel_w_089148FC, 0x20) + 0x27, a->id, duel_088AA224(a));
        return 0x7E;
    case 0x7E:
        duel_088C7BA4();
        a->w[0] = duel_w_08919624;
        duel_w_089148FC = 0;
        return 0x7D;
    case 0x7D:
        if (duel_w_089148FC < a->w[0]) {
            duel_088263A8(p, duel_0884CCBC(duel_w_089148FC + 1), 1, a->id << 16);
            duel_w_089148FC++;
            return 0x7D;
        }
        duel_088C7CF0();
        return 0x64;
    case 0x78:
        duel_088C7810(p, 1);
        return 0x77;
    case 0x77:
        duel_088328FC(1 - p);
        return 0x76;
    case 0x76:
        duel_088C7810(p, 0);
        return 0x64;
    case 0x64:
        duel_088C78E0(p);
        return 0;
    }
    return 0;
}

int duel_088A4C50(DuelAct *a)
{
    int i;
    int id;

    switch (duel_step) {
    case 0x80:
        duel_088C7BA4();
        duel_088D184C(a, duel_0884E4E4(a), 0);
        return 0x7F;
    case 0x7F:
        if (duel_088DFE24(a->player, 1 - a->player) != 0) {
            for (i = duel.players[(1 - a->player) & 1].n_grave - 1; i >= 0; i--) {
                id = duel.players[(1 - a->player) & 1].grave[i].id;
                if (card_is_monster(id) && card_get_type(id) == 7) {
                    duel_088CB400(1 - a->player, &duel.players[(1 - a->player) & 1].grave[i], 1);
                    return 0x7F;
                }
            }
        }
        return 0x7E;
    case 0x7E:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_088A4DFC(DuelAct *a, int b)
{
    switch (duel_step) {
    case 0x80:
        if (duel_0884E9E8(a, b) == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, 0x1F);
        duel_w_089148FC = 0;
        a->w[1] = 0;
        a->w[0] = 0;
        return 0x7F;
    case 0x7F:
        if (duel.players[a->player & 1].n_hand == 0) {
            return 0x7D;
        }
        duel_08832894(a->player, a->id);
        return 0x7E;
    case 0x7E:
        duel_088CA384(a->player, duel_w_0891968C, 0);
        a->w[duel_h_0891968E & 1]++;
        return ++duel_w_089148FC < 2 ? 0x7F : 0x7D;
    case 0x7D:
        if (a->w[a->player] != 0) {
            duel_088C78E0(a->player);
        }
        if (a->w[1 - a->player] != 0) {
            duel_088C78E0(1 - a->player);
        }
        return 0;
    }
    return 0;
}

int duel_088A4F90(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_08832B28(a->player, a->zone, 3);
        return 0x7F;
    case 0x7F:
        duel_088C4604(a, 1 - a->player, (duel_w_0891968C + duel_h_0891968E + duel.unk3832[1]) * 100);
        return 0;
    }
    return 0;
}

int duel_088A5030(DuelAct *a)
{
    unsigned int q;

    switch (duel_step) {
    case 0x80:
        if (duel.players[(1 - a->player) & 1].n_hand == 0) {
            return 0;
        }
        duel_08832880(a->player, a->id, 1);
        return 0x7F;
    case 0x7F:
        q = 1 - a->player;
        duel_088CAE24(q, &duel.players[q & 1].hand[duel_w_0891968C], 1);
        return 0;
    }
    return 0;
}

int duel_088A50FC(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            duel_088D1970(a, p, z);
        }
    }
    return 0;
}

int duel_088A5184(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel_0884E440(a, p, z) != 0) {
                a->w[2] = duel_field_level(p, z);
                return 0x7F;
            }
        }
        return 0;
    case 0x7F:
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, a->id, duel_0887FCA8);
        } else {
            duel_0884ACD8(a->player, 0x11E);
            duel_08835978(duel_0887FCA8);
        }
        return 0x7E;
    case 0x7E:
        if (duel_08835AC0() != 0) {
            duel_088263A8(a->player, &duel.players[a->player & 1].hand[duel_w_08919654], 1, a->id << 16);
            return 0;
        }
        return 0x7F;
    }
    return 0;
}

int duel_088A5300(DuelAct *a)
{
    int i;
    unsigned int m = 0;
    unsigned int t;
    unsigned int p;
    unsigned int z;

    for (i = 0; i < a->n7; i++) {
        if (duel_088A9008(a, i, &t)) {
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel.players[p & 1].field[z].card.id != 0 && duel_act_can_affect(a, p, z, 1)) {
                m |= 1 << (z + p * 16);
            }
        }
    }
    duel_088D184C(a, m, 2);
    return 0;
}

int duel_088A540C(DuelAct *a)
{
    unsigned int q = 1 - a->player;
    int id;

    if (duel.players[q & 1].n_d != 0) {
        id = duel.players[q & 1].zone_d[0].id;
        if (duel_zone_faceup(q, 0xD, 0) == 0) {
            duel_088C7880(1 - a->player);
        }
        duel_088CB980(1 - a->player);
        if (card_is_monster(id) && card_is_group_b(id) && duel_card_dfac8(a->player, id, 0)) {
            if (duel_count_free_monster_zones(a->player) != 0 && duel_phase_ok2(a->player) != 0) {
                duel_088263A8(a->player, duel.players[(1 - a->player) & 1].zone_d, 1, 0);
            } else {
                duel_088C625C(1 - a->player, 1, 1);
            }
        } else {
            duel_088CB55C(1 - a->player, duel.players[(1 - a->player) & 1].zone_d, 0);
            duel_088C7DD4(1 - a->player, 2, 1);
        }
    }
    return 0;
}

int duel_088A55D4(DuelAct *a)
{
    int d;

    switch (duel_step) {
    case 0x80:
        if (duel_0887AC5C(a) == 0) {
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
        if (duel_h_0891968E != 0) {
            duel_088C97F0(a, duel_special.player[1], duel_special.zone[1], 0, 0);
            return 0;
        }
        d = duel_field_atk(duel_special.player[0], duel_special.zone[0]) - duel_field_def(duel_special.player[1], duel_special.zone[1]);
        duel_088C4604(a, a->player, d > 0 ? d : 0);
        return 0;
    }
    return 0;
}

int duel_088A5738(DuelAct *a, int b)
{
    unsigned short buf[256];
    unsigned int p;
    int z;
    int k;

    switch (duel_step) {
    case 0x80:
        if (duel_0884E9E8(a, b) == 0) {
            return 0;
        }
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        if (duel_0884E9E8(a, b) != 0) {
            duel_0884ACD8(a->player, 0x139);
            return 0x7E;
        }
        return 0x64;
    case 0x7E:
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7D;
    case 0x7D:
        a->w[0] = duel_0884C7CC()->id;
        a->w[1] = CARD_UID(*duel_0884C7CC());
        duel_088CB95C(a->player, 0xE, a->w[1]);
        return 0x7C;
    case 0x7C:
        duel_wfmt_s(buf, 0x45, card_get_name(a->w[0]));
        duel_0884ACD8(a->player, (int)buf);
        duel_088359E8(a->player, a->id, duel_0884E440);
        return 0x7B;
    case 0x7B:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            k = duel_pick_st_zone(a->player);
            duel_088CB728(a->player, 0xE, a->w[1], a->player, k, 1, 0);
            duel_088C8B70(a->player, a->player | (unsigned char)k << 8, (unsigned char)p | (unsigned char)z << 8);
            return 0x7F;
        }
        return 0x7C;
    case 0x64:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_088A5984(DuelAct *a)
{
    unsigned int p = a->player ^ a->opp;
    int i;
    DuelCard *c;

    duel_088C5E70(1 - a->player, 2);
    if (duel_count_free_monster_zones(p) == 0) {
        return 0;
    }
    if (duel_phase_ok2(p) == 0) {
        return 0;
    }
    if (a->n7 == 1) {
        i = duel_grave_find_uid(p, duel_act_target2(a, 0));
        if (i >= 0) {
            c = &duel.players[p & 1].grave[i];
            if (duel_card_b14(c) == 0) {
                return 0;
            }
            duel_088263A8(p, c, 1, a->id << 16);
        }
    }
    return 0;
}

int duel_088A5A90(DuelAct *a)
{
    unsigned int m = duel_088D13CC(a, duel_0884E538(a, duel_08855F54));
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            duel_088C87D8(a, p, z, a->id, 3, duel_bit_count(m) * 1000);
        }
    }
    return 0;
}
