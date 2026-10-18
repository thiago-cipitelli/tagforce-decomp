// @range 0x088EAC0C-0x088EB710
/* Cards on the field by face, spell/trap zones (free zones, placement), graveyard counts by type. */
#include "duel.h"
#include "card_db.h"

extern int duel_zone_order_st[5];      /* 0x08911DB4 */

int duel_count_active(int id);
int duel_count_field(unsigned int p);
int duel_zone_usable(unsigned int p, unsigned int z);
unsigned int duel_field_flag9(unsigned int p, int z);
unsigned int duel_field_attrs(unsigned int p, int z);
int duel_field_same_name(unsigned int p, int z, int id);
int field_get_card(unsigned int p, int z);
int duel_count_field_cards(unsigned int p, int faceup, int facedown, int fzone);
int duel_st_zone_free(unsigned int p, int z);
int duel_pick_st_zone(unsigned int p);

int duel_count_field_all(unsigned int p)
{
    return duel_count_field_cards(p, 0, 0, 1);
}

/* Cards of player p on the field (monsters that count, spells/traps, the field zone if `fzone`),
   all of them or only the face-up or face-down ones. */
int duel_count_field_cards(unsigned int p, int faceup, int facedown, int fzone)
{
    int i;
    int n = 0;
    int end;

    end = fzone != 0 ? 11 : 10;
    for (i = 0; i < end; i++) {
        if (duel.players[p & 1].field[i].card.id != 0 && (i >= 5 || duel_field_flag9(p, i))) {
            int ok = 0;

            if (faceup == 0 && facedown == 0) {
                ok = 1;
            }
            if (faceup != 0 && duel.players[p & 1].field[i].faceup != 0) {
                ok = 1;
            }
            if (facedown != 0 && duel.players[p & 1].field[i].faceup == 0) {
                ok = 1;
            }
            if (ok) {
                n++;
            }
        }
    }
    return n;
}

int duel_st_zone_free(unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id != 0) {
        return 0;
    }
    return duel_zone_usable(p, z) != 0;
}

/* First free spell/trap zone in placement order, -1 if none (or 0x16DF limits the field to 5 cards). */
int duel_pick_st_zone(unsigned int p)
{
    int i;

    if (duel_count_active(0x16DF) != 0 && duel_count_field(p) >= 5) {
        return -1;
    }
    for (i = 0; i < 5; i++) {
        int z = duel_zone_order_st[i] + 5;

        if (duel_st_zone_free(p, z)) {
            return z;
        }
    }
    return -1;
}

int duel_count_free_st_zones(unsigned int p)
{
    int i;
    int n = 0;

    for (i = 5; i < 10; i++) {
        if (duel_st_zone_free(p, i)) {
            n++;
        }
    }
    if (duel_count_active(0x16DF) != 0) {
        n = MIN(n, 5 - duel_count_field(p));
        n = MAX(n, 0);
    }
    return n;
}

/* Player p has room for spell/trap `id` (field spells go to zone 10). */
int duel_can_place_st(unsigned int p, int id)
{
    if (card_get_prop5_hi(id) == 2) {
        if (duel_count_active(0x16DF) != 0 && duel.players[p & 1].field[10].card.id == 0 && duel_count_field(p) >= 5) {
            return 0;
        }
        return 1;
    }
    return duel_pick_st_zone(p) >= 0;
}

int duel_has_faceup_attr6(unsigned int p)
{
    int i;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++) {
        if (pl->field[i].card.id != 0 && pl->field[i].faceup != 0 && card_is_attribute6(pl->field[i].card.id)) {
            return 1;
        }
    }
    return 0;
}

int duel_no_faceup_attrs46(unsigned int p)
{
    int i;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++) {
        if (pl->field[i].card.id != 0 && pl->field[i].faceup != 0 && (duel_field_attrs(p, i) & 0x46)) {
            return 0;
        }
    }
    return 1;
}

/* Face-up traps of player p. */
int duel_count_faceup_traps(unsigned int p)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 10; i++) {
        if (pl->field[i].card.id != 0 && pl->field[i].faceup != 0 && (i >= 5 || duel_field_flag9(p, i)) &&
            card_get_type(pl->field[i].card.id) == 0x17) {
            n++;
        }
    }
    return n;
}

/* Face-down spells/traps of player p. */
int duel_count_facedown_st(unsigned int p)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 5; i < 10; i++) {
        if (pl->field[i].card.id != 0 && pl->field[i].faceup == 0) {
            n++;
        }
    }
    return n;
}

int duel_count_grave_type(unsigned int p, unsigned int type)
{
    unsigned int i;
    int n = 0;

    p &= 1;
    for (i = 0; i < duel.players[p].n_grave; i++) {
        if (type == card_get_type(duel.players[p].grave[i].id)) {
            n++;
        }
    }
    return n;
}

int duel_count_grave_kind(unsigned int p, int kind)
{
    unsigned int i;
    int n = 0;

    p &= 1;
    for (i = 0; i < duel.players[p].n_grave; i++) {
        if (kind == card_get_kind(duel.players[p].grave[i].id)) {
            n++;
        }
    }
    return n;
}

/* Spells in player p's graveyard of spell type `st` (card_get_prop5_hi). */
int duel_count_grave_spells(unsigned int p, unsigned int st)
{
    unsigned int i;
    int n = 0;

    p &= 1;
    for (i = 0; i < duel.players[p].n_grave; i++) {
        int id = duel.players[p].grave[i].id;

        if (card_get_type(id) == 0x16 && st == card_get_prop5_hi(id)) {
            n++;
        }
    }
    return n;
}

int duel_count_grave_traps(unsigned int p, unsigned int st)
{
    unsigned int i;
    int n = 0;

    p &= 1;
    for (i = 0; i < duel.players[p].n_grave; i++) {
        int id = duel.players[p].grave[i].id;

        if (card_get_type(id) == 0x17 && st == card_get_prop5_hi(id)) {
            n++;
        }
    }
    return n;
}

int duel_count_zone_f_type(unsigned int p, unsigned int type)
{
    unsigned int i;
    int n = 0;

    p &= 1;
    for (i = 0; i < duel.players[p].n_f; i++) {
        if (type == card_get_type(duel.players[p].zone_f[i].id)) {
            n++;
        }
    }
    return n;
}

/* Other face-up monsters with the same name as monster z of player p. */
int duel_count_same_name_other(unsigned int p, int z)
{
    int id = field_get_card(p, z);
    unsigned int q;
    int i;
    int n;

    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (id == 0) {
        return 0;
    }
    n = 0;
    for (q = 0; (int)q < 2; q++) {
        for (i = 0; i < 5; i++) {
            if ((q != p || i != z) && duel.players[q & 1].field[i].faceup != 0 && duel_field_same_name(q, i, id)) {
                n++;
            }
        }
    }
    return n;
}

/* The card being resolved is `id` (0x150B counts as 0x10F4). */
int duel_is_cur_card(int id)
{
    if (id == 0x10F4 && duel.cur_card == 0x150B) {
        return 1;
    }
    return duel.cur_card == id;
}
