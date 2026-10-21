// @range 0x08895C8C-0x08895E4C
/* Card effect handlers (DuelAct *a), after effects_zh.c. */
#include "duel.h"

extern int duel_step;
extern unsigned int duel_w_089148FC;
extern unsigned short duel_w_0891968C;

int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_088D1858(DuelAct *a, unsigned int p, int z);
int duel_0886CF1C(DuelAct *a, int b);
void duel_08832894(unsigned int p, unsigned short id);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);

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
