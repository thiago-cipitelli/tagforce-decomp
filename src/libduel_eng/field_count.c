// @range 0x088E94E4-0x088EA23C
// @asm 0x088E9D3C-0x088EA098
/* Counting cards on the field: by effective type or attribute, by name, free monster zones. */
#include "duel.h"
#include "card_db.h"

unsigned int duel_field_type(unsigned int p, int z);
int duel_field_attr_is(unsigned int p, int z, int attr);
int duel_field_same_name(unsigned int p, int z, int id);
int duel_field_protected(unsigned int p, int z);
int duel_find_st(unsigned int p, int id);
int field_get_card(unsigned int p, int z);
unsigned int duel_field_flag9(unsigned int p, int z);
unsigned int duel_slot_effect_arg(unsigned int p, int z, int id);
int duel_count_active(int id);
int duel_zone_usable(unsigned int p, unsigned int z);

/* Face-up monsters of player p with effective type `type`, zone `skip` excluded. */
int duel_count_type_skip(unsigned int p, unsigned int type, int skip)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++) {
        if (i != skip && pl->field[i].card.id != 0 && pl->field[i].faceup != 0 && type == (duel_field_type(p, i) & 0xFFFF)) {
            n++;
        }
    }
    return n;
}

int duel_count_type(unsigned int p, unsigned int type)
{
    return duel_count_type_skip(p, type, -1);
}

/* Face-up monsters of player p with attribute `attr`, zone `skip` excluded. */
int duel_count_attr_skip(unsigned int p, int attr, int skip)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++) {
        if (i != skip && pl->field[i].card.id != 0 && pl->field[i].faceup != 0 && duel_field_attr_is(p, i, attr)) {
            n++;
        }
    }
    return n;
}

int duel_count_attr(unsigned int p, int attr)
{
    return duel_count_attr_skip(p, attr, -1);
}

/* Cards of player p on the field with the name of card id. */
int duel_count_name(unsigned int p, int id)
{
    int i;
    int n = 0;

    for (i = 0; i <= 10; i++) {
        if (duel_field_same_name(p, i, id)) {
            n++;
        }
    }
    return n;
}

/* Same, unprotected cards only. */
int duel_count_name_unprotected(unsigned int p, int id)
{
    int i;
    int n = 0;

    for (i = 0; i <= 10; i++) {
        if (duel_field_same_name(p, i, id) && duel_field_protected(p, i) == 0) {
            n++;
        }
    }
    return n;
}

/* Cards owned by p with the name of id: on both fields and in p's graveyard. */
int duel_count_owned_name(unsigned int p, int id)
{
    int q;
    int i;
    int n = 0;

    for (q = 0; q < 2; q++) {
        for (i = 0; i <= 10; i++) {
            if (p == duel.players[q & 1].field[i].card.owner && card_same_name(duel.players[q & 1].field[i].card.id, id)) {
                n++;
            }
        }
    }
    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_same_name(duel.players[p & 1].grave[i].id, id)) {
            n++;
        }
    }
    return n;
}

int duel_has_st_12be(unsigned int p)
{
    return duel_find_st(p, 0x12BE) >= 0;
}

/* Cards of player p on the field, the special cards included. */
int duel_count_field(unsigned int p)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i <= 10; i++) {
        if (pl->field[i].card.id != 0) {
            n++;
        }
    }
    if (duel.mode & 1) {
        if (p == duel_special.player[0] && duel_special.slot[0].card.id != 0) {
            n++;
        }
        if (p == duel_special.player[1] && duel_special.slot[1].card.id != 0) {
            n++;
        }
    }
    return n;
}

int duel_count_monsters(unsigned int p)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++) {
        if (pl->field[i].card.id != 0) {
            n++;
        }
    }
    return n;
}

int duel_count_monsters_special(int p)
{
    int n = duel_count_monsters(p);

    if (duel.mode & 1) {
        if (p == duel_special.player[0] && duel_special.slot[0].card.id != 0) {
            n++;
        }
        if (p == duel_special.player[1] && duel_special.slot[1].card.id != 0) {
            n++;
        }
    }
    return n;
}

/* Face-up monsters of p whose card passes fn. */
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id))
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++) {
        if (pl->field[i].faceup != 0 && fn(field_get_card(p, i))) {
            n++;
        }
    }
    return n;
}

int duel_count_attr9(unsigned int p)
{
    int i;
    int n = 0;

    for (i = 0; i <= 10; i++) {
        if (card_is_attribute9(duel.players[p & 1].field[i].card.id)) {
            n++;
        }
    }
    return n;
}

/* Monsters of p, optionally face up only and/or in position 0 only. */
int duel_count_monsters_pos(unsigned int p, int faceup, int pos0)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++) {
        if (pl->field[i].card.id != 0) {
            int a = 0;
            int b = 0;

            if (faceup == 0 || pl->field[i].faceup != 0) {
                a = 1;
            }
            if (pos0 == 0 || pl->field[i].pos == 0) {
                b = 1;
            }
            if (a && b) {
                n++;
            }
        }
    }
    return n;
}

int duel_count_monsters_state(unsigned int p, unsigned int faceup, unsigned int pos)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++) {
        if (pl->field[i].card.id != 0 && faceup == pl->field[i].faceup && pos == pl->field[i].pos) {
            n++;
        }
    }
    return n;
}

int duel_count_monsters_faceup_is(unsigned int p, unsigned int faceup)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++) {
        if (pl->field[i].card.id != 0 && faceup == pl->field[i].faceup) {
            n++;
        }
    }
    return n;
}

int duel_count_monsters_pos_is(unsigned int p, unsigned int pos)
{
    int i;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (i = 0; i < 5; i++) {
        if (pl->field[i].card.id != 0 && pos == pl->field[i].pos) {
            n++;
        }
    }
    return n;
}

#ifdef NON_MATCHING /* the zone mark nibbles are masked twice and two tests branch to their bodies */
/* Zone z (< 5 monster, else spell/trap) of player p is not locked by an effect. */
int duel_zone_usable(unsigned int p, unsigned int z)
{
    if ((int)z < 5) {
        int q;

        for (q = 0; q < 2; q++) {
            unsigned int e = duel.players[q & 1].effects;
            unsigned int i;

            while (e != 0) {
                DuelEffect *f = &duel.effects[e];

                e = f->next;
                if ((f->kind & 0xF) == 6) {
                    unsigned int sp = EFF_PLAYER(f);
                    unsigned int sz = EFF_ZONE(f);
                    int id = field_get_card(sp, sz);

                    if (id == 0x17EE || id == 0x1432) {
                        sp &= 1;

                        if ((1 << (z + p * 16) & duel.players[sp].field[sz].link) &&
                            SLOT_ACTIVE(&duel.players[sp].field[sz])) {
                            return 0;
                        }
                    }
                }
            }
            for (i = 0; i < duel.players[q & 1].n_marks; i++) {
                unsigned short m = duel.players[q & 1].marks[i];
                unsigned char b = m >> 8;

                if ((m & 0xFF) == 0x40 && p == ((b & 0xF) & 1) && z == (unsigned char)((b & 0xF0) >> 4)) {
                    return 0;
                }
            }
        }
    } else {
        unsigned int e = duel.players[1 - p & 1].effects;
        int q;

        while (e != 0) {
            DuelEffect *f = &duel.effects[e];

            e = f->next;
            if ((f->kind & 0xF) == 6) {
                unsigned int sp = EFF_PLAYER(f) & 1;
                unsigned int sz = EFF_ZONE(f);

                if (duel.players[sp].field[sz].card.id == 0x1AA9 && (1 << (z + 16) & duel.players[sp].field[sz].link) &&
                    SLOT_ACTIVE(&duel.players[sp].field[sz])) {
                    return 0;
                }
            }
        }
        for (q = 0; q < 2; q++) {
            int i;

            for (i = 0; i < 5; i++) {
                if (duel_field_flag9(q, i) && (p & 0xFF | (z & 0xFF) << 8) == duel_slot_effect_arg(q, i, 0x1472)) {
                    return 0;
                }
            }
        }
    }
    return 1;
}
#endif

/* Zone z of player p is empty and usable. */
int duel_zone_free(unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id != 0) {
        return 0;
    }
    if (duel.mode & 1) {
        if (p == duel_special.player[0] && z == duel_special.zone[0] && duel_special.slot[0].card.id != 0) {
            return 0;
        }
        if (p == duel_special.player[1] && z == duel_special.zone[1] && duel_special.slot[1].card.id != 0) {
            return 0;
        }
    }
    return duel_zone_usable(p, z) != 0;
}

/* Free monster zones of player p (at most 5 minus its cards on the field with card 0x16DF out). */
int duel_count_free_monster_zones(unsigned int p)
{
    int i;
    int n = 0;

    for (i = 0; i < 5; i++) {
        if (duel_zone_free(p, i)) {
            n++;
        }
    }
    if (duel_count_active(0x16DF)) {
        n = MIN(n, 5 - duel_count_field(p));
        n = MAX(n, 0);
    }
    return n;
}
