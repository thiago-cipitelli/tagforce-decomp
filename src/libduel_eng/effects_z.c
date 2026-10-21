// @range 0x0888B904-0x0888BAE0
// @asm 0x0888B904-0x0888BAE0
/* Card effect handlers (DuelAct *a), after effects_y.c. duel_088C87A8 is called without a prototype here
   (four arguments in 0x0888B904), so these are a separate file. */
#include "duel.h"

extern unsigned int duel_turn_player;

int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088C87A8();
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);

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
