// @range 0x088B2D0C-0x088B2F34
/* Candidate lists: cards of a pile offered for selection (duel_088BFC64(p, zone, i)). */
#include "duel.h"

void duel_088BFC64(unsigned int p, int z, int i);
int card_is_monster(int id);
int card_get_atk_q(int id);
unsigned int card_get_type(int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);

void duel_088B2D0C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_is_monster(duel.players[p & 1].zone_d[i].id) &&
            card_get_atk_q(duel.players[p & 1].zone_d[i].id) < 0x5DD &&
            duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i])) == 0) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B2E04(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0x17) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B2EB0(unsigned int p)
{
    int i;

    for (i = 0; i < (unsigned int)duel.players[p & 1].n_d && i < 5; i++) {
        duel_088BFC64(p, 0xD, i);
    }
}

