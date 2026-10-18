// @range 0x088EFEE4-0x088F0730
// @asm 0x088EFEE4-0x088F009C
/* Cards fitting a zone, targets of the chain, counters on the field, and adding and removing cards in
   the graveyard and in zone F. */
#include "duel.h"
#include "card_db.h"

int duel_slot_e8170(unsigned int p, int z, int id);
DuelCard *duel_field_card_deb2c(void);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_act_resolve_6a230(DuelAct *a);
void duel_card_copy(DuelCard *dst, DuelCard *src);
int duel_card_same(DuelCard *a, DuelCard *b);
int duel_grave_remove(unsigned int p, unsigned int i);

#ifdef NON_MATCHING /* the equip branch places its `faceup != 0` block after the `return 1` */
/* Card id may be in zone z of player p (monsters in the spell/trap zones, equips, a few others). */
int duel_card_fits_zone(unsigned int p, int z, int id)
{
    if (id == 0) {
        return 0;
    }
    if (z >= 5 && z <= 9 && card_is_monster(id)) {
        return 1;
    }
    if (card_get_prop5_hi(id) == 3) {
        if (z < 5) {
            return 0;
        }
        if (z < 11) {
            return duel.players[p & 1].field[z].faceup != 0;
        }
        return 1;
    }
    switch (id) {
    case 0x1231:
    case 0x1238:
    case 0x13EA:
    case 0x1514:
    case 0x1980:
    case 0x1A18:
    case 0x1A69:
        if (z >= 5 && z < 10 && duel.players[p & 1].field[z].faceup != 0 &&
            !(duel.players[p & 1].field[z].flags >> 1 & 1)) {
            return 1;
        }
        break;
    }
    return 0;
}
#endif

/* A card of `player` in zone `zone` in the chain targets monster z of player p. */
int duel_chain_targets(unsigned int p, int z, unsigned int player, int zone)
{
    int i;
    int j;

    for (i = duel_chain.len - 1; i >= 0; i--) {
        DuelAct *a = &duel_chain.acts[i];

        if (a->player == player && a->zone == zone) {
            for (j = 0; j < a->n_targets; j++) {
                if ((unsigned short)((unsigned char)p | (unsigned char)z << 8) == duel_act_target(a, j)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int duel_field_act(unsigned int player, int zone)
{
    if (zone > 4) {
        DuelCard *c = duel_field_card_deb2c();
        DuelAct a = { 0 };

        a.player = player;
        a.zone = zone;
        a.id = c->id;
        a.uid = CARD_UID(*c);
        a.b3_4 = 0;
        return duel_act_resolve_6a230(&a);
    }
    return 0;
}

/* Adds n to the counter (link) of the monsters (or spells/traps) of player p for card id. */
void duel_add_counters(unsigned int p, int id, int n)
{
    int lo;
    int hi;

    if (card_is_monster(id)) {
        lo = 0;
        hi = 4;
    } else {
        lo = 5;
        hi = 9;
    }
    for (; lo <= hi; lo++) {
        if (duel_slot_e8170(p, lo, id)) {
            duel.players[p & 1].field[lo].link += n;
        }
    }
}

void duel_set_flag4(unsigned int p, int id)
{
    int lo;
    int hi;

    if (card_is_monster(id)) {
        lo = 0;
        hi = 4;
    } else {
        lo = 5;
        hi = 9;
    }
    for (; lo <= hi; lo++) {
        if (duel_slot_e8170(p, lo, id)) {
            duel.players[p & 1].field[lo].flags |= 0x10;
        }
    }
}

/* Puts card c at the top of its owner's graveyard. */
void duel_grave_add(DuelCard *c)
{
    int p = c->owner & 1;
    DuelCard *dst = &duel.players[p].grave[duel.players[p].n_grave];
    int id = c->id;

    if (id != 0 && card_is_attribute9((unsigned short)id) == 0) {
        duel_card_copy(dst, c);
        duel.players[p].n_grave++;
    }
}

void duel_zone_f_add(DuelCard *c, unsigned short mark)
{
    int p = c->owner & 1;
    int i = duel.players[p].n_f;
    DuelCard *dst = &duel.players[p].zone_f[i];
    int id = c->id;

    if (id != 0 && card_is_attribute9((unsigned short)id) == 0) {
        duel_card_copy(dst, c);
        duel.players[p].marks[i] = mark;
        duel.players[p].n_f++;
    }
}

/* Removes card i from the graveyard of player p. */
int duel_grave_remove(unsigned int p, unsigned int i)
{
    unsigned int j;

    p &= 1;
    if (i < duel.players[p].n_grave) {
        duel.players[p].n_grave--;
        for (j = i; j < duel.players[p].n_grave; j++) {
            duel_card_copy(&duel.players[p].grave[j], &duel.players[p].grave[j + 1]);
        }
        return 1;
    }
    return 0;
}

/* Removes the topmost copy of card c from the graveyard of player p. */
int duel_grave_remove_card(unsigned int p, DuelCard *c)
{
    int i;

    for (i = duel.players[p & 1].n_grave - 1; i >= 0; i--) {
        if (duel_card_same(&duel.players[p & 1].grave[i], c)) {
            return duel_grave_remove(p, i);
        }
    }
    return 0;
}
