// @range 0x08846498-0x08846584
/* Computer player: destroys the cards whose effects on a zone match an id (all but the first). */
#include "duel.h"

int duel_slot_e8170(unsigned int p, int z, int id);
int duel_088D1438(unsigned int p, int z, int a, int b);

void duel_08846498(unsigned int p, int z, int id, int f)
{
    unsigned int e = duel.players[p & 1].field[z].effects;
    DuelEffect *ef;
    unsigned int sp;
    unsigned int sz;

    while (e != 0) {
        ef = &duel.effects[e];
        e = ef->next;
        if ((ef->kind & 0xF) == 0xA) {
            sp = EFF_PLAYER(ef);
            sz = EFF_ZONE(ef);
            if (duel_slot_e8170(sp, sz, id) != 0) {
                if (f != 0) {
                    duel_088D1438(sp, sz, 0, 0);
                } else {
                    f = 1;
                }
            }
        }
    }
}
