// @range 0x088FD690-0x088FDE68
// @asm 0x088FDAF8-0x088FDCF4
/* Card effect handlers (continued). */
#include "duel.h"

extern int duel_player_type[2];
extern int duel_step;
extern int duel_w_08919624;
extern int duel_w_0891964C;
extern int duel_w_08919654;

unsigned int duel_act_target(DuelAct *a, int i);
int duel_088A9008(DuelAct *a, int b, int c);
int duel_0884E440();
int duel_0884E9E8(DuelAct *a, int b);
void duel_0884ACD8(unsigned int p, int msg);
int duel_0887FCA8(int id);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(int id));
void duel_08835978(int (*fn)(int id));
int duel_08835AC0(void);
void duel_0884AF68(int msg, int uid, int a);
int duel_pick_monster_zone();
DuelCard *duel_088E6700(int uid);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, unsigned int d);
int duel_field_atk(unsigned int p, int z);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088CC594(unsigned int p, int z, int a, int id);
extern int duel_w_089148FC;
extern unsigned int duel_turn_player;
int duel_0886CE38(DuelAct *a, int b);
unsigned int duel_088E5F84(int idx);
int card_is_monster(int id);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_0888D5A8(DuelAct *a, int b);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C5424(unsigned int p, int n);
void duel_08832AF4(unsigned int p, int z, int a, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);

int duel_eff_fd690(DuelAct *a, int b)
{
    unsigned int t = duel_act_target(a, 0);
    unsigned int q = t & 0xFF;
    unsigned int z = (unsigned char)((t & 0xFFFF) >> 8);

    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) != 0 && duel_0884E440(a, q, z) != 0 && duel_0884E9E8(a, b) != 0) {
            duel_0884ACD8(a->player, 0x11E);
            return 0x7F;
        }
        return 0;
    case 0x7F:
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, a->id, duel_0887FCA8);
        } else {
            duel_08835978(duel_0887FCA8);
        }
        return 0x7E;
    case 0x7E:
        if (duel_08835AC0() != 0) {
            a->w[2] = CARD_UID(duel.players[duel_w_0891964C & 1].hand[duel_w_08919654]);
            if (duel_player_type[a->player] == 1) {
                duel_w_08919624 = 0;
            } else {
                duel_0884AF68(0xF7, a->w[2], 1);
            }
            return 0x7D;
        }
        return 0x80;
    case 0x7D:
        a->w[3] = duel_pick_monster_zone(q);
        duel_0882619C(q, duel_088E6700(a->w[2]), 1, duel_w_08919624, 0);
        return 0x7C;
    case 0x7C: {
        int atk = duel_field_atk(q, a->w[3]);

        duel_088C87A8(q, z, a->player | a->zone << 8, 0x17, atk > 0xFFFF ? 0xFFFF : atk);
        duel_088C87A8(q, a->w[3], a->id, 3, a->uid);
        return 0;
    }
    }
    return 0;
}

int duel_eff_fd930(DuelAct *a)
{
    if (duel_088A9008(a, 0, 0) != 0) {
        unsigned int t = duel_act_target(a, 0);
        unsigned int q = t & 0xFF;
        unsigned int z = (unsigned char)((t & 0xFFFF) >> 8);

        if (duel_0884E440(a, q, z) != 0) {
            duel_088CC594(q, z, 1, 0x1A90);
        }
    }
    return 0;
}

int duel_eff_fd9b4(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    duel_088CC594(p, z, 1, 0x1A90);
    return 0;
}

int duel_eff_fda70(DuelAct *a)
{
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 4; z++) {
            if (duel_0884E440(a, p, z) != 0) {
                duel_088CC594(p, z, 2, 0x1A90);
            }
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* the original computes the zone before the player and keeps the player in v0 for the 0x7F step */
int duel_eff_fdaf8(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    switch (duel_step) {
    case 0x80: {
        unsigned int t;
        unsigned int q;
        unsigned int i;

        if (duel_0886CE38(a, b) == 0) {
            return 0;
        }
        t = duel_088E5F84(a->idx);
        q = t & 0xFF;
        i = (unsigned short)(t >> 16);
        if ((unsigned char)((t & 0xFFFF) >> 8) == 0xE && card_is_monster(duel.list[a->idx].id) != 0 &&
            duel_grave_card_usable(a->player, q, i) != 0) {
            duel_w_089148FC = duel_pick_monster_zone(a->player);
            duel_088263A8(a->player, &duel.players[q & 1].grave[i], 1, a->id << 16);
            return 0x7F;
        }
        return 0;
    }
    case 0x7F:
        duel_088C87A8(p, duel_w_089148FC, p | z << 8, 7, 0);
        return 0;
    }
    return 0;
}
#endif

int duel_eff_fdcf4(DuelAct *a, int b)
{
    int r = duel_0888D5A8(a, b);

    if (duel_step == 0x7E) {
        a->w[3] = 1;
    }
    if (r == 0 && a->w[3] == 0) {
        duel_088C4604(a, a->player, 2000);
    }
    return r;
}

int duel_eff_fdd6c(DuelAct *a)
{
    unsigned int p;

    p = duel_turn_player;
    p ^= duel_w_089148FC;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        duel_08832AF4(p, p == a->player ? a->zone : -1, 1, 1);
        return 0x7E;
    case 0x7E:
        if (duel.unk3832 != 0) {
            duel_088C5424(p, 2000);
        } else {
            duel_088C4604(a, p, 2000);
        }
        return 0x7D;
    case 0x7D:
        if (++duel_w_089148FC < 2) {
            return 0x7F;
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}
