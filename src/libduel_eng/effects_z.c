// @range 0x0888B904-0x0888C1A8
// @asm 0x0888B904-0x0888BAE0
// @asm 0x0888BBE4-0x0888BDE0
// @asm 0x0888BF08-0x0888C0AC
// @asm 0x0888C10C-0x0888C1A8
/* Card effect handlers (DuelAct *a), after effects_y.c. duel_088C87A8 is called without a prototype here
   (four arguments in 0x0888B904), so these are a separate file. */
#include "duel.h"

extern unsigned int duel_turn_player;
extern int duel_step;
extern int duel_w_089148FC;

int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088C87A8();
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_08832624(unsigned int p, unsigned int n, int a, int b);
int duel_0886CE38();
int duel_grave_find_uid(unsigned int p, int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
int duel_pick_st_zone(unsigned int p);
void duel_088CB728(unsigned int p, int z, int uid, unsigned int q, int qz, int a, int b);
void duel_088C8B70(unsigned int p, unsigned short loc, unsigned short t);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_can_place_st(unsigned int p, int id);
int duel_act_check_08853CE8(DuelAct *a, int x, int y);
int duel_d_count_name(unsigned int p, unsigned short id);
void duel_088C6508(unsigned int p, int id, int a, int b);
void duel_0884ACD8(unsigned int p, int msg);
void duel_088C6A10(unsigned int p, int id, int z);
void duel_0886BCDC(void);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);
void duel_088D192C(DuelAct *a, unsigned int p, int z);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, unsigned short c);

#ifdef NON_MATCHING /* registradores ($t0 vs $v1, ordem das cargas) */
int duel_0888B904(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            duel_088C87A8(p, z, a->id, (unsigned short)((duel_turn_player != a->player) << 4 | 1));
        }
    }
    return 0;
}

int duel_0888B9A8(DuelAct *a)
{
    unsigned int q = 1 - a->player;

    duel_088C77D4(q, a->id, (unsigned char)((duel_turn_player == q) + 1) << 4 | 1, 0);
    return 0;
}

int duel_0888BA00(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        duel_088C87A8(p, z, a->id, (unsigned short)((unsigned char)((duel_turn_player == p) + 1) << 4 | 2), 0);
    }
    return 0;
}
#endif

int duel_0888BAE0(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        if (duel.players[a->player & 1].n_hand != 0) {
            duel_08832624(a->player, 1, 0, 0);
        }
        return 0x7E;
    case 0x7E:
        if (duel.players[1 - a->player & 1].n_hand != 0) {
            duel_08832624(1 - a->player, 1, 0, 1);
        }
        return 0x7D;
    case 0x7D:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* ordem das cargas dos argumentos; registradores dos globais da carta especial */
int duel_0888BBE4(DuelAct *a, int b)
{
    int i;

    if (a->kind != 2) {
        duel_088C77D4(a->player, a->id, (unsigned char)((a->player == duel_turn_player) + 1) << 4 | 1, a->uid);
        return 0;
    }
    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    i = duel_grave_find_uid(a->player, a->uid);
    if (i >= 0 && duel_grave_card_usable(a->player, a->player, i) != 0) {
        duel_0882619C(a->player, &duel.players[a->player & 1].grave[i], 1, 1, a->id << 16 | 0x20);
    }
    return 0;
}

int duel_0888BD08(DuelAct *a)
{
    int sz = duel_special.zone[1];
    unsigned int sp = duel_special.player[1];

    if (duel_special.act[1 - duel_special.player[0]].uid == CARD_UID(duel.players[sp & 1].field[sz].card) &&
        duel.players[sp & 1].field[sz].faceup != 0) {
        duel_088C87A8(sp, sz, a->id, 2, 0);
    }
    return 0;
}
#endif

int duel_0888BDE0(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;
    int s;

    if (duel_grave_find_uid(a->player, a->uid) < 0) {
        return 0;
    }
    if (duel_pick_st_zone(a->player) == -1) {
        return 0;
    }
    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            s = duel_pick_st_zone(a->player);
            duel_088CB728(a->player, 0xE, a->uid, a->player, s, 1, 0);
            duel_088C8B70(a->player, a->player | (s & 0xFF) << 8, (p & 0xFF) | (z & 0xFF) << 8);
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* bnezl no retorno 0x7F */
int duel_0888BF08(DuelAct *a)
{
    unsigned int t = duel_act_target(a, 0);

    switch (duel_step) {
    case 0x80:
        if (duel_can_place_st(a->player, 0x13EA) != 0 && duel_088A9008(a, 0, 0) != 0) {
            if (duel_act_check_08853CE8(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF) != 0) {
                return 0x7F;
            }
        }
        if (duel_d_count_name(a->player, 0x13EA) != 0) {
            duel_088C6508(a->player, 0x13EA, 0, 0);
            return 1;
        }
        duel_0884ACD8(a->player, 0xD);
        return 0x78;
    case 0x7F:
        duel_w_089148FC = duel_pick_st_zone(a->player);
        duel_088C6A10(a->player, 0x13EA, duel_w_089148FC);
        return 0x7E;
    case 0x7E:
        duel_088C8B70(a->player, a->player | (duel_w_089148FC & 0xFF) << 8, t);
        return 0x7D;
    case 0x7D:
        duel_0886BCDC();
        return 1;
    case 0x78:
        duel_088328FC(1 - a->player);
        return 1;
    case 1:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}
#endif

int duel_0888C0AC(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    duel_088D192C(a, a->player, a->zone);
    duel_088C78E0(a->uid & 1);
    return 0;
}

#ifdef NON_MATCHING /* move s0,a0 antes da primeira carga */
int duel_0888C10C(DuelAct *a)
{
    unsigned int t;

    duel_088C77D4(a->player, a->id, 1, 0);
    if (a->b4_2) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        duel_088C87D8(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF, a->id, 1, a->player);
    }
    return 0;
}
#endif
