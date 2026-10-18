// @range 0x088E6998-0x088E7518
// @asm 0x088E6CF8-0x088E6D5C
/* Card of a zone as seen by the rules (copy cards take the copied id), and the deck (zone D) and
   zone C piles: push, shuffle, find, remove, draw, counts. */
#include "duel.h"
#include "card_db.h"

unsigned int duel_slot_effect_arg(unsigned int p, int z, int id);
unsigned int duel_card_effect_arg(DuelSlot *s, int id);
int duel_card_protected(int id, int uid);
void duel_card_copy(DuelCard *dst, DuelCard *src);
void duel_card_swap(DuelCard *a, DuelCard *b);
int duel_card_same(DuelCard *a, DuelCard *b);
int duel_rand(int n);
void duel_hand_add(unsigned int p, DuelCard *c);

/* Card id in zone z of player p; 0x1A74 copies another card, 0x19BC/0x19A6 count as 0x18F6/0x18F9
   while active. */
int field_get_card(unsigned int p, int z)
{
    unsigned int q = p & 1;
    unsigned int id = duel.players[q].field[z].card.id;

    if (z < 5) {
        switch (id) {
        case 0x19A6:
            if (SLOT_ACTIVE(&duel.players[q].field[z]) && duel_card_protected(0x18F9, -1) == 0) {
                return 0x18F9;
            }
            break;
        case 0x19BC:
            if (SLOT_ACTIVE(&duel.players[q].field[z]) && duel_card_protected(0x18F6, -1) == 0) {
                return 0x18F6;
            }
            break;
        case 0x1A74:
            id = duel_slot_effect_arg(p, z, id);
            switch (id) {
            case 0x19A6:
                id = 0x18F9;
                break;
            case 0x19BC:
                id = 0x18F6;
                break;
            }
            if (id == -1 || duel_card_protected(id, -1)) {
                return 0x1A74;
            }
            break;
        }
    }
    return id;
}

int duel_special_get_id(DuelSlot *s)
{
    unsigned int id = s->card.id;

    switch (id) {
    case 0x1A74:
        id = duel_card_effect_arg(s, id);
        if (id == -1 || duel_card_protected(id, -1)) {
            return 0x1A74;
        }
        break;
    }
    return id;
}

int duel_field_same_name(unsigned int p, int z, int id)
{
    return card_same_name(field_get_card(p, z), id);
}

/* Put card c on top of the deck of player p (index 0). */
void duel_d_push_top(unsigned int p, DuelCard *c)
{
    int i;

    p &= 1;
    for (i = duel.players[p].n_d; i > 0; i--) {
        duel_card_copy(&duel.players[p].zone_d[i], &duel.players[p].zone_d[i - 1]);
    }
    duel.players[p].n_d++;
    duel_card_copy(&duel.players[p].zone_d[0], c);
}

void duel_d_push(unsigned int p, DuelCard *c)
{
    duel_card_copy(&duel.players[p & 1].zone_d[duel.players[p & 1].n_d], c);
    duel.players[p & 1].n_d++;
}

#ifdef NON_MATCHING /* the original loads the count before computing the player base */
void duel_c_push(unsigned int p, DuelCard *c)
{
    duel_card_copy(&duel.players[p & 1].zone_c[duel.players[p & 1].n_18], c);
    duel.players[p & 1].n_18++;
}
#endif

void duel_d_shuffle(unsigned int p)
{
    int i;

    p &= 1;
    for (i = duel.players[p].n_d - 1; i >= 0; i--) {
        duel_card_swap(&duel.players[p].zone_d[i], &duel.players[p].zone_d[duel_rand(i + 1)]);
    }
}

/* Index of the first card of the deck with the name of id, -1 if none. */
unsigned int duel_d_find_name(unsigned int p, int id)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < (unsigned int)duel.players[p].n_d; i++) {
        if (card_same_name(duel.players[p].zone_d[i].id, id)) {
            return i;
        }
    }
    return -1;
}

unsigned int duel_c_find_name(unsigned int p, int id)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < (unsigned int)duel.players[p].n_18; i++) {
        if (card_same_name(duel.players[p].zone_c[i].id, id)) {
            return i;
        }
    }
    return -1;
}

/* Take card i out of the deck into *out. */
int duel_d_take(unsigned int p, unsigned int i, DuelCard *out)
{
    p &= 1;
    if (i < (unsigned int)duel.players[p].n_d) {
        duel_card_copy(out, &duel.players[p].zone_d[i]);
        duel.players[p].n_d--;
        for (; i < (unsigned int)duel.players[p].n_d; i++) {
            duel_card_copy(&duel.players[p].zone_d[i], &duel.players[p].zone_d[i + 1]);
        }
        return 1;
    }
    return 0;
}

/* Remove card c from the deck. */
int duel_d_remove(unsigned int p, DuelCard *c)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < (unsigned int)duel.players[p].n_d; i++) {
        if (duel_card_same(&duel.players[p].zone_d[i], c)) {
            duel.players[p].n_d--;
            for (; i < (unsigned int)duel.players[p].n_d; i++) {
                duel_card_copy(&duel.players[p].zone_d[i], &duel.players[p].zone_d[i + 1]);
            }
            return 1;
        }
    }
    return 0;
}

/* Draw the top card of the deck. */
void duel_draw(unsigned int p)
{
    DuelCard c;

    if (duel_d_take(p, 0, &c)) {
        duel_hand_add(p, &c);
    }
}

int duel_c_remove(unsigned int p, DuelCard *c)
{
    unsigned int i;

    p &= 1;
    for (i = 0; i < (unsigned int)duel.players[p].n_18; i++) {
        if (duel_card_same(&duel.players[p].zone_c[i], c)) {
            duel.players[p].n_18--;
            for (; i < (unsigned int)duel.players[p].n_18; i++) {
                duel_card_copy(&duel.players[p].zone_c[i], &duel.players[p].zone_c[i + 1]);
            }
            return 1;
        }
    }
    return 0;
}

int duel_d_count_monsters(unsigned int p)
{
    unsigned int i;
    int n;

    p &= 1;
    for (i = 0, n = 0; i < (unsigned int)duel.players[p].n_d; i++) {
        if (card_is_monster(duel.players[p].zone_d[i].id)) {
            n++;
        }
    }
    return n;
}

int duel_d_count_type(unsigned int p, int type)
{
    unsigned int i;
    int n;

    p &= 1;
    for (n = 0, i = 0; i < (unsigned int)duel.players[p].n_d; i++) {
        if (type == card_get_type(duel.players[p].zone_d[i].id)) {
            n++;
        }
    }
    return n;
}

int duel_d_count_name(unsigned int p, unsigned short id)
{
    unsigned int i;
    int n;

    p &= 1;
    for (n = 0, i = 0; i < (unsigned int)duel.players[p].n_d; i++) {
        if (card_same_name(duel.players[p].zone_d[i].id, id)) {
            n++;
        }
    }
    return n;
}

int duel_c_count_name(unsigned int p, unsigned short id)
{
    unsigned int i;
    int n;

    p &= 1;
    for (n = 0, i = 0; i < (unsigned int)duel.players[p].n_18; i++) {
        if (card_same_name(duel.players[p].zone_c[i].id, id)) {
            n++;
        }
    }
    return n;
}
