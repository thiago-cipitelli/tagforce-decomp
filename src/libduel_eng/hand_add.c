// @range 0x088CAF24-0x088CB400
// @asm 0x088CAF24-0x088CB14C
/* Cards added to the hand (from the graveyard, from zone 0xF, from the graveyard of the opponent):
   command 0x3F to the presentation, then the reveal (duel_088C77EC) and the event 0x1A. */
#include "duel.h"

typedef struct DuelMoveEv {            /* event argument of a card moving (see duel_hand.c) */
    unsigned short uid : 9;
    unsigned short player : 1;
    unsigned short b10 : 1;
    unsigned short opp : 1;
    unsigned short zone : 4;
    unsigned short pad;
} DuelMoveEv;

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_grave_pos(unsigned int p, DuelCard *c);
unsigned int duel_f_find_uid(unsigned int p, int uid);
int card_attribute_icon_is3(int id);
int card_is_in_group_1a5c(int id);
void duel_088C77EC(unsigned int p, int uid);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_0884F30C(unsigned int a, unsigned short uid, DuelMoveEv ev);
void duel_088AFD5C(int kind, DuelMoveEv ev);

#ifdef NON_MATCHING /* alocação de registradores (a/c em $s1/$s0 no original) e ordem dos loads iniciais */
/* card c of the graveyard of p to his hand, for the action a */
int duel_088CAF24(DuelAct *a, unsigned int p, DuelCard *c)
{
    int opp;
    unsigned int n;
    int id;
    int uid;
    int k;
    int z;
    DuelMoveEv ev;

    if (duel_grave_pos(p, c) >= 0) {
        opp = a->player != p;
        n = duel.players[p & 1].n_hand;
        id = c->id;
        uid = CARD_UID(*c);
        k = card_attribute_icon_is3(id) ? 0xC : 0xB;
        duel_cmd(0x3F, (uid & 0x3FF) << 6 | ((p & 1) | 0x1C), (n & 0x3FF) << 6 | ((p & 1) | (k & 0x1F) << 1), opp);
        if (!card_attribute_icon_is3(id)) {
            ev.player = p;
            ev.b10 = 1;
            ev.uid = uid;
            duel_088C77EC(p, uid);
            if (card_is_in_group_1a5c(id)) {
                for (z = 0; z < 5; z++) {
                    if (duel_slot_e8170(p, z, 0x1A50)) {
                        duel_0884F30C((p << 31 & 0x80000000) | 0x34200000 | ((unsigned int)z << 16 & 0x1F0000) | 0x1A50,
                                      CARD_UID(duel.players[p & 1].field[z].card), ev);
                    }
                }
            }
            duel_088AFD5C(0x1A, ev);
        }
        return 1;
    }
    return 0;
}
#endif

/* card c of zone 0xF of p to his hand */
int duel_088CB14C(DuelAct *a, unsigned int p, DuelCard *c)
{
    int opp;
    unsigned int n;
    int k;

    if ((int)duel_f_find_uid(p, CARD_UID(*c)) >= 0) {
        int z = 0xF;

        opp = a->player != p;
        n = duel.players[p & 1].n_hand;
        k = card_attribute_icon_is3(c->id) ? 0xC : 0xB;
        duel_cmd(0x3F, (CARD_UID(*c) & 0x3FF) << 6 | ((p & 1) | (z & 0x1F) << 1), (n & 0x3FF) << 6 | ((p & 1) | (k & 0x1F) << 1),
                 opp);
        if (!card_attribute_icon_is3(c->id)) {
            duel_088C77EC(a->player, CARD_UID(*c));
        }
        return 1;
    }
    return 0;
}

/* card c of the graveyard of p to the hand of the other player */
int duel_088CB2AC(DuelAct *a, unsigned int p, DuelCard *c)
{
    unsigned int q;
    int opp;
    unsigned int n;
    int k;

    if (duel_grave_pos(p, c) != 0) {
        int z = 0xE;

        q = 1 - p;
        opp = a->player != q;
        n = duel.players[q & 1].n_hand;
        k = card_attribute_icon_is3(c->id) ? 0xC : 0xB;
        duel_cmd(0x3F, (CARD_UID(*c) & 0x3FF) << 6 | ((p & 1) | (z & 0x1F) << 1), (n & 0x3FF) << 6 | ((q & 1) | (k & 0x1F) << 1),
                 opp);
        if (!card_attribute_icon_is3(c->id)) {
            duel_088C77EC(q, CARD_UID(*c));
        }
        return 1;
    }
    return 0;
}
