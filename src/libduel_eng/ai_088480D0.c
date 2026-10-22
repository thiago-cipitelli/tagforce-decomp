// @range 0x088480D0-0x088482F8
/* Computer player: records of the special cards (duel_special +0x9C) answering a target. */
#include "duel.h"


int duel_08846584(unsigned int p, int b);
int duel_slot_has_effect(unsigned int p, int z, int id);

int duel_088480D0(unsigned int p)
{
    return duel_08846584(p, 0);
}

void duel_088480D8(int *a, int *b, int c)
{
    int i = duel_special.recs[0].w0 != 0;
    int found = 0;
    int j;

    duel_special.recs[i].w0 = c;
    duel_special.recs[i].a[0] = a[0];
    duel_special.recs[i].a[1] = a[1];
    if (b == 0) {
        duel_special.recs[i].b[0] = 1 - a[0];
        duel_special.recs[i].b[1] = 5;
    } else {
        duel_special.recs[i].b[0] = b[0];
        duel_special.recs[i].b[1] = b[1];
    }
    for (j = 0; j < 2; j++) {
        if (found) {
            continue;
        }
        if (duel_special.recs[i].b[0] != duel_special.act[j].unk0) {
            continue;
        }
        switch (duel_special.act[j].id) {
        case 0x14A4:
        case 0x1639:
            if (duel.players[duel_special.act[j].unk0 & 1].field[duel_special.act[j].unk4].flags >> 5 & 1) {
                break;
            }
            if (a[0] != duel_special.act[j].unk0) {
                duel_special.recs[i].b[1] = a[1];
            } else {
                duel_special.recs[i].b[1] = 5;
            }
            found = 1;
            duel_special.recs[i].b[0] ^= 1;
            break;
        }
    }
    if (found) {
        return;
    }
    if (duel_slot_has_effect(duel_special.recs[i].b[0], 0xB, 0x1930) != 0) {
        if (a[0] != duel_special.act[j].unk0) {
            duel_special.recs[i].b[1] = a[1];
        } else {
            duel_special.recs[i].b[1] = 5;
        }
        duel_special.recs[i].b[0] ^= 1;
    }
}
