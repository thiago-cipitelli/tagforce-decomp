// @range 0x0887DB24-0x0887E140
// @asm 0x0887DB24-0x0887DBB4
// @asm 0x0887DC94-0x0887DDB0
/* Card effect handlers (DuelAct *a): what a's card does when it resolves. */
#include "duel.h"

int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440();
int duel_088D14B8(DuelAct *a, unsigned int p, int z);
void duel_088D13CC(DuelAct *a, unsigned int mask);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2();
int duel_act_target2(DuelAct *a, int i);
unsigned int duel_card_locate(unsigned int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_0884E4E4(DuelAct *a);
int duel_act_check_08853CE8(DuelAct *a, int x, int y);
int card_get_type(int id);
void duel_088C8B70(unsigned int p, unsigned short loc, unsigned short t);
void duel_088CC178();
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088CC718(int id);
int duel_0887DB24(DuelAct *a);

#ifdef NON_MATCHING /* branch layout differs */
int duel_0887DB24(DuelAct *a)
{
    int t;
    int p;
    int z;

    if (duel_088A9008(a, 0, (unsigned int *)&t) != 0) {
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0 && duel_088D14B8(a, p, z) != 0) {
            return 1;
        }
        return 0;
    }
    return 0;
}
#endif

void duel_0887DBB4(DuelAct *a)
{
    unsigned int mask = 0;
    int i;
    int t;

    for (i = 0; i < a->n_targets; i++) {
        if (duel_088A9008(a, i, (unsigned int *)&t) != 0) {
            int p = t & 0xFF;
            int z = (unsigned short)t >> 8 & 0xFF;

            if (duel_0884E440(a, p, z) != 0 && duel_act_can_affect(a, p, z, 1) != 0) {
                mask |= 1 << (z + p * 16);
            }
        }
    }
    duel_088D13CC(a, mask);
}

#ifdef NON_MATCHING /* branch layout differs */
int duel_0887DC94(DuelAct *a)
{
    unsigned int q = a->player ^ a->opp;

    if (duel_count_free_monster_zones(q) == 0) {
        return 0;
    }
    if (duel_phase_ok2(q) == 0) {
        return 0;
    }
    if (a->n7 == 1) {
        unsigned int loc = duel_card_locate(duel_act_target2(a, 0));

        if ((loc & 0xFFFF) >> 8 == 0xE && duel_grave_card_usable(q, loc & 0xFF, loc >> 16) != 0) {
            duel_088263A8(q, &duel.players[loc & 1].grave[loc >> 16], 1, a->id << 16);
            return 1;
        }
        return 0;
    }
    return 0;
}
#endif

int duel_0887DDB0(DuelAct *a)
{
    duel_0887DB24(a);
    return 0;
}

int duel_0887DDD0(DuelAct *a)
{
    duel_088D13CC(a, duel_0884E4E4(a));
    return 0;
}

int duel_0887DE04(DuelAct *a)
{
    int t;
    unsigned int p;

    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel_088A9008(a, 0, (unsigned int *)&t) == 0 ||
        duel_act_check_08853CE8(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF) == 0) {
        a->b4_3 = 1;
        return 0;
    }
    if (card_get_type(a->id) == 0x17 && a->b4_2) {
        a->b4_3 = 1;
        return 0;
    }
    p = a->player;
    duel_088C8B70(p, p | a->zone << 8, (unsigned short)t);
    switch (a->id) {
    case 0x12C6:
    case 0x145B:
        duel_088CC178(a->player, a->zone, a->w[1]);
        break;
    }
    return 0;
}

int duel_0887DF94(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int q = 1 - p;
    unsigned int id = duel.players[q & 1].field[10].card.id;
    int i;
    int z;

    if (a->uid != CARD_UID(duel.players[p & 1].field[a->zone].card)) {
        return 0;
    }
    if (id != 0 && duel.players[q & 1].field[10].faceup != 0) {
        duel_088D1438(q, 10, 0, 0);
    }
    for (i = 0; i < 2; i++) {
        for (z = 0; z <= 4; z++) {
            if (duel.players[i & 1].field[z].card.id != 0 && duel.players[i & 1].field[z].faceup != 0) {
                duel_088C87A8(i, z, a->player | a->zone << 8, 9, 0);
            }
        }
    }
    if (a->id != id) {
        duel_088CC718(a->id);
    }
    return 0;
}
