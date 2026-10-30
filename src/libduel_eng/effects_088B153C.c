// @range 0x088B153C-0x088B1EAC
// @asm 0x088B1854-0x088B1EAC
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
int duel_pick_monster_zone(unsigned int p);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_zone_usable(unsigned int p, unsigned int z);
int duel_088B1EAC(int *v, int n, int need, int max);
extern int duel_w_089148FC;
extern int duel_w_08914900;

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

#ifdef NON_MATCHING /* alocação de registradores no laço da mão ($s0/$s1/$s2 trocados) */
/* can the card (p, z, i) be the next tribute of the ritual a? It must not be picked yet, and the
   levels of the other tributes still available must complete the level to pay (duel_088B1EAC) */
int duel_088B1854(DuelAct *a, unsigned int p, int z, unsigned int i)
{
    int uid;
    int id;
    int k;
    int zz;
    int n;
    int q;

    int need = duel_w_089148FC;
    int rest;
    int max = duel_w_08914900;
    int monster = duel_rituals[a->w[0]].monster;
    int lv;
    int levels[42];

    if (z == 0xB) {
        id = duel.players[p & 1].hand[i].id;
        uid = CARD_UID(duel.players[p & 1].hand[i]);
        lv = duel_list_card_level(p, uid);
        if (p != a->player) {
            return 0;
        }
        if (a->w[1] == 0 && duel_pick_monster_zone((int)a->player) < 0) {
            return 0;
        }
        if (!card_is_monster(id)) {
            return 0;
        }
        if (id == monster && duel_hand_count_name(p, id) < 2) {
            return 0;
        }
    } else {
        id = z + i;
        if (id < 5) {
            uid = CARD_UID(duel.players[p & 1].field[id].card);
            lv = duel_field_level(p, id);
            if (!duel_tributable(a->player, p, id)) {
                return 0;
            }
            if (duel_slot_immune(p, id, 0)) {
                return 0;
            }
            if (p != a->player && duel.players[p & 1].field[id].faceup == 0) {
                return 0;
            }
            if (a->w[1] == 0 && duel_pick_monster_zone((int)a->player) < 0) {
                if (p != a->player || !duel_zone_usable(p, id)) {
                    return 0;
                }
            }
        } else {
            return 0;
        }
    }
    if (lv == 0) {
        return 0;
    }
    for (k = 0; k < a->w[1]; k++) {
        if (duel_chain.uids[(duel_chain.uids_head + k) & 0x7F] == uid) {
            return 0;
        }
    }
    switch (a->id) {
    case 0x1698:
    case 0x1716:
    case 0x19D9:
        max = 1;
    }
    if (lv >= need && lv < need + max) {
        return 1;
    }
    max = MIN(max, lv);
    rest = need - lv;
    /* levels of the other monsters of the hand and of the field that can still be tributed */
    n = 0;
    for (q = 0; q < duel.players[a->player & 1].n_hand; q++) {
        id = duel.players[a->player & 1].hand[q].id;
        if (z == 0xB && i == q) {
            continue;
        }
        if (!card_is_monster(id)) {
            continue;
        }
        if (id == monster) {
            monster = -1;
            continue;
        }
        uid = CARD_UID(duel.players[a->player & 1].hand[q]);
        for (k = 0; k < a->w[1]; k++) {
            if (duel_chain.uids[(duel_chain.uids_head + k) & 0x7F] == uid) {
                uid = 0;
                break;
            }
        }
        if (uid != 0 && n < 42) {
            levels[n++] = duel_list_card_level((int)a->player, uid);
        }
    }
    for (q = 0; q < 2; q++) {
        for (zz = 0; zz < 5; zz++) {
            if (!duel_tributable(a->player, q, zz)) {
                continue;
            }
            if (duel_slot_immune(q, zz, 0)) {
                continue;
            }
            if (p == q && zz == z + i) {
                continue;
            }
            if (p != q && duel.players[q & 1].field[zz].faceup == 0) {
                continue;
            }
            uid = CARD_UID(duel.players[q & 1].field[zz].card);
            for (k = 0; k < a->w[1]; k++) {
                if (duel_chain.uids[(duel_chain.uids_head + k) & 0x7F] == uid) {
                    uid = 0;
                    break;
                }
            }
            if (uid != 0 && n < 42) {
                levels[n++] = duel_field_level(q, zz);
            }
        }
    }
    return duel_088B1EAC(levels, n - 1, rest, max);
}
#endif
