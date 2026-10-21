// @range 0x088B153C-0x088B1854
/* Ritual summon helpers: the ritual table at 0x0891291C (monster and spell of each ritual). */
#include "duel.h"

typedef struct DuelRitual {             /* 4 */
    unsigned int monster : 13;
    unsigned int spell : 13;
    unsigned int b26 : 6;
} DuelRitual;

extern DuelRitual duel_rituals[];       /* 0x0891291C */

int duel_card_e5b88(int id, int kind);
int duel_card_protected(int id, int uid);
int card_is_monster(int id);
int duel_list_card_level(unsigned int p, int i);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_slot_immune(unsigned int p, int z, int opp);
int duel_field_level(unsigned int p, int z);

int duel_088B153C(unsigned int id, int i)
{
    DuelRitual *r = &duel_rituals[i];

    if (id == r->spell) {
        return 1;
    }
    switch (id) {
    case 0x1698:
        return duel_card_e5b88(r->monster, 2);
    case 0x1716:
        return duel_card_e5b88(r->monster, 5);
    }
    return 0;
}

int duel_088B15B8(unsigned int p, int i)
{
    unsigned int k;

    for (k = 0; k < duel.players[p & 1].n_hand; k++) {
        if (duel.players[p & 1].hand[k].id == duel_rituals[i].monster &&
            duel_card_protected(duel.players[p & 1].hand[k].id, -1) == 0) {
            return 1;
        }
    }
    return 0;
}

int duel_088B167C(unsigned int p, unsigned int id)
{
    int sum = 0;
    unsigned int k;

    for (k = 0; k < duel.players[p & 1].n_hand; k++) {
        unsigned int c = duel.players[p & 1].hand[k].id;

        if (card_is_monster(c)) {
            if (c == id) {
                id = -1;
            } else {
                sum += duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[k]));
            }
        }
    }
    return sum;
}

int duel_088B1774(unsigned int p)
{
    int q;
    int z;
    int sum = 0;

    for (q = 0; q < 2; q++) {
        for (z = 0; z < 5; z++) {
            if (duel_tributable(p, q, z) != 0 && duel_slot_immune(q, z, 0) == 0 &&
                (p == q || duel.players[q & 1].field[z].faceup != 0)) {
                sum += duel_field_level(q, z);
            }
        }
    }
    return sum;
}
