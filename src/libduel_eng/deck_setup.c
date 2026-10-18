// @range 0x088E7518-0x088E80CC
// @asm 0x088E77F0-0x088E7D64
// @data 0x08911D88-0x08911DA0
/* Deck setup from the recipes (uids, the list of every card of the duel), saving and restoring the
   piles of a player, removing a card from a zone and putting it in a zone. */
#include "duel.h"
#include "libc.h"

typedef struct DuelPileSave {          /* 0x1B8: hand, deck and zone C of a player as uids */
    unsigned short n_hand;
    unsigned short n_d;
    unsigned short n_c;
    unsigned short uid[217];
} DuelPileSave;

typedef struct DuelDecks {             /* recipes and saved piles of both players */
    unsigned int n_main[2][0x42];      /* RecipeData[2] (0x108 each) */
    DuelPileSave save[2];              /* +0x210 */
} DuelDecks;

typedef struct DuelRecipe {            /* RecipeData */
    unsigned int n_main, n_extra, n_side;
    unsigned short main_ids[80];
    unsigned short extra_ids[15];
    unsigned short side_ids[31];
} DuelRecipe;

void duel_card_swap(DuelCard *a, DuelCard *b);
int duel_rand(int n);
DuelCard *duel_field_card_deb2c(unsigned int p, int zone, int i);
void duel_hand_remove_card(unsigned int p, DuelCard *c);
void duel_zone_f_remove_card(unsigned int p, DuelCard *c);
void duel_grave_remove_card(unsigned int p, DuelCard *c);
int duel_d_remove(unsigned int p, DuelCard *c);
int duel_c_remove(unsigned int p, DuelCard *c);
void duel_effects_drop_card(DuelCard *c);
void duel_effects_clear_src(unsigned int p, int z);
void duel_slot_effects_free(DuelSlot *s);
void duel_088E0890(int uid);
void duel_bzero(void *p, int n);
void duel_hand_add(unsigned int p, DuelCard *c);
void duel_c_push(unsigned int p, DuelCard *c);
void duel_d_push(unsigned int p, DuelCard *c);
void duel_d_push_top(unsigned int p, DuelCard *c);
void duel_grave_add(DuelCard *c);
void duel_zone_f_add(DuelCard *c, int arg);
void duel_place_monster(unsigned int p, int z, DuelCard *c, int a, int b);
void duel_place_st(unsigned int p, int z, DuelCard *c, int a);

/* Reverse the deck of player p. */
void duel_d_reverse(unsigned int p)
{
    int n = duel.players[p & 1].n_d;
    int i;

    for (i = 0; i < n / 2; i++) {
        duel_card_swap(&duel.players[p & 1].zone_d[i], &duel.players[p & 1].zone_d[n - 1 - i]);
    }
}

/* Fill the deck (main) and zone C (side) of player p from recipe r, giving each card a uid. */
void duel_deck_load(unsigned int p, DuelRecipe *r)
{
    unsigned int q = p & 1;
    unsigned int i;
    unsigned int k = 1;

    duel.players[q].n_d = 0;
    for (i = 0; i < r->n_main; i++) {
        DuelCard *c = &duel.players[q].zone_d[i];

        c->id = r->main_ids[i];
        c->owner = p;
        c->uid_hi = k;
        duel.list[CARD_UID(*c)].id = r->main_ids[i];
        duel.players[q].n_d++;
        k++;
    }
    duel.players[q].n_18 = 0;
    for (i = 0; i < r->n_side; i++) {
        DuelCard *c = &duel.players[q].zone_c[i];

        c->id = r->side_ids[i];
        c->owner = p;
        c->uid_hi = k;
        duel.list[CARD_UID(*c)].id = r->side_ids[i];
        duel.players[q].n_18++;
        k++;
    }
    duel.players[q].unk8 = k - 1;
    duel.players[q].unk4 = k;
}

#ifdef NON_MATCHING /* the original keeps the uid and the output pointer as their own induction registers */
/* Uids of the recipe of player p in the save block: shuffled main deck (5 to the hand), side. */
void duel_deck_build_uids(unsigned int p, DuelDecks *d)
{
    int k = duel.players[p & 1].unk8 + 1;
    DuelRecipe *r = (DuelRecipe *)d->n_main[p];
    DuelPileSave *s = &d->save[p];
    unsigned short *u;
    unsigned int i;
    int j;

    u = s->uid;
    s->n_hand = 5;
    s->n_d = r->n_main - 5;
    s->n_c = r->n_side;
    for (i = 0; i < r->n_main; i++) {
        int uid = p + k * 2;

        duel.list[uid].id = r->main_ids[i];
        u[i] = uid;
        k++;
    }
    for (j = r->n_main - 1; j >= 0; j--) {
        int x = duel_rand(j + 1);
        unsigned short t = u[j];

        u[j] = u[x];
        u[x] = t;
    }
    u += r->n_main;
    for (i = 0; i < r->n_side; i++) {
        int uid = p + k * 2;

        duel.list[uid].id = r->side_ids[i];
        *u++ = uid;
        k++;
    }
    duel.players[p & 1].unk8 = k - 1;
    duel.players[p & 1].unk4 = k;
}
#endif

#ifdef NON_MATCHING /* same steps; the original schedules the count loads and stores differently */
/* Swap the hand, deck and zone C of player p with the ones saved in the block. */
void duel_piles_swap(unsigned int p, DuelDecks *d)
{
    unsigned int q = p & 1;
    DuelPileSave t;
    DuelPileSave *s;
    unsigned short *u = t.uid;
    unsigned int i;

    t.n_d = duel.players[q].n_d;
    t.n_hand = duel.players[q].n_hand;
    t.n_c = duel.players[q].n_18;
    for (i = 0; i < duel.players[q].n_hand; i++) {
        *u++ = CARD_UID(duel.players[q].hand[i]);
    }
    for (i = 0; i < (unsigned int)duel.players[q].n_d; i++) {
        *u++ = CARD_UID(duel.players[q].zone_d[i]);
    }
    for (i = 0; i < (unsigned int)duel.players[q].n_18; i++) {
        *u++ = CARD_UID(duel.players[q].zone_c[i]);
    }
    s = &d->save[p];
    u = s->uid;
    duel.players[q].n_hand = s->n_hand;
    duel.players[q].n_d = s->n_d;
    duel.players[q].n_18 = s->n_c;
    for (i = 0; i < duel.players[q].n_hand; i++) {
        unsigned int uid = *u++;

        duel.players[q].hand[i].id = duel.list[uid].id;
        duel.players[q].hand[i].owner = uid;
        duel.players[q].hand[i].uid_hi = (uid & 0x1FF) >> 1;
    }
    for (i = 0; i < (unsigned int)duel.players[q].n_d; i++) {
        unsigned int uid = *u++;

        duel.players[q].zone_d[i].id = duel.list[uid].id;
        duel.players[q].zone_d[i].owner = uid;
        duel.players[q].zone_d[i].uid_hi = (uid & 0x1FF) >> 1;
    }
    for (i = 0; i < (unsigned int)duel.players[q].n_18; i++) {
        unsigned int uid = *u++;

        duel.players[q].zone_c[i].id = duel.list[uid].id;
        duel.players[q].zone_c[i].owner = uid;
        duel.players[q].zone_c[i].uid_hi = (uid & 0x1FF) >> 1;
    }
    memcpy(s, &t, 0x1B8);
}
#endif

void duel_slot_clear(DuelSlot *s)
{
    duel_slot_effects_free(s);
    duel_bzero(s, 0x14);
}

/* Take card i of zone `zone` of player p out of its zone or pile. */
void duel_card_take(unsigned int p, int zone, int i)
{
    DuelCard *c = duel_field_card_deb2c(p, zone, i);

    switch (zone) {
    case 0xC:
        duel_c_remove(p, c);
        break;
    case 0xD:
        duel_d_remove(p, c);
        break;
    case 0xE:
        duel_grave_remove_card(p, c);
        duel_effects_drop_card(c);
        break;
    case 0xF:
        duel_zone_f_remove_card(p, c);
        duel_effects_drop_card(c);
        break;
    case 0xB:
        duel_hand_remove_card(p, c);
        break;
    default: {
        int z = zone + i;

        if (z > 10) {
            duel_slot_clear(p == duel_special.player[0] && zone == duel_special.zone[0] ? &duel_special.slot[0] : &duel_special.slot[1]);
        } else {
            duel_effects_clear_src(p, z);
            duel_slot_clear(&duel.players[p & 1].field[z]);
            duel.zone_off &= ~(1 << (i + (zone + p * 16)));
        }
        break;
    }
    }
}

/* Put card c in zone `zone` of player p (deck: arg 0 top, else bottom). */
void duel_card_put(unsigned int p, int zone, unsigned int arg, DuelCard *c)
{
    switch (zone) {
    case 0xC:
        duel_c_push(p, c);
        break;
    case 0xD:
        if (arg != 0) {
            duel_d_push(p, c);
        } else {
            duel_d_push_top(p, c);
        }
        break;
    case 0xE:
        duel_grave_add(c);
        break;
    case 0xF:
        duel_zone_f_add(c, arg);
        break;
    case 0xB:
        duel_hand_add(p, c);
        break;
    default:
        if (zone < 5) {
            duel_place_monster(p, zone, c, (unsigned char)((arg & 0xFFFF) >> 8), arg & 0xFF);
        } else {
            duel_place_st(p, zone, c, arg & 0xFF);
        }
        break;
    case 0x10:
        break;
    }
    if (c->owner == p && zone != 0xE && zone > 10) {
        duel_effects_drop_card(c);
    }
    switch (zone) {
    case 0xC:
    case 0xD:
    case 0xB:
        duel_088E0890(CARD_UID(*c));
        break;
    }
}
