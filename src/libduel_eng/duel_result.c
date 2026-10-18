// @range 0x088FF3FC-0x088FF8C4
/* After the duel: kind of result of a tag series, deck statistics for the rewards, one-time
   flags in the save. */
#include "card_db.h"
#include "libc.h"

typedef struct DuelResults {           /* game_state2_get(), 0x2A0 */
    unsigned char pad0[0x20];
    int result;                        /* +0x20 */
    int mode;                          /* +0x24 8: series of duels */
    unsigned char pad28[0xC];
    unsigned short n_duels;            /* +0x34 */
    unsigned short duels[1];           /* +0x36 result of each duel: 1, 2, 3 */
} DuelResults;

typedef struct DeckStats {             /* 0x68 */
    unsigned short n_monster;
    unsigned short attr[16];           /* +0x02 by card_get_attribute */
    unsigned short type[21];           /* +0x22 monsters by card_get_type */
    unsigned short kind[8];            /* +0x4C monsters by card_get_kind */
    unsigned short levels;             /* +0x5C bit per level */
    unsigned short flags;              /* +0x5E */
    unsigned short atk_max;            /* +0x60 */
    unsigned short atk_min;
    unsigned short def_max;            /* +0x64 */
    unsigned short def_min;
} DeckStats;

DuelResults *game_state2_get(void);
int sysflag_test(int bit);

int duel_series_result(void)
{
    DuelResults *r = game_state2_get();
    unsigned char *g = (unsigned char *)game_state_get();
    int b;
    int a;
    int i;
    int n;

    if (g[0x119] != 2) {
        return r->result;
    }
    if (r->mode == 8) {
        return r->result;
    }
    n = r->n_duels;
    a = 0;
    b = 0;
    for (i = 0; i < n; i++) {
        switch (r->duels[i]) {
        case 1:
            b += 2;
            break;
        case 2:
            a += 2;
            break;
        case 3:
            b++;
            a++;
            break;
        }
    }
    if (b >= 4) {
        return 1;
    }
    if (a >= 4) {
        return 2;
    }
    if (n < 3) {
        return 0;
    }
    return 3;
}

void duel_deck_stats_add(unsigned short id, DeckStats *s)
{
    int v;

    s->attr[card_get_attribute(id)]++;
    if (card_get_limit(id) != 3) {
        s->flags |= 0x10;
    }
    if (card_is_monster(id) != 0) {
        s->n_monster++;
        s->type[card_get_type(id)]++;
        s->kind[card_get_kind(id)]++;
        s->levels |= 1 << card_get_level(id);
        if (card_is_in_group_198c(id) == 0) {
            s->flags |= 1;
        }
        if (card_is_in_group_1a6f(id) == 0) {
            s->flags |= 2;
        }
        if (card_is_attribute7(id) == 0) {
            s->flags |= 4;
        }
        if (card_is_attribute6(id) == 0) {
            s->flags |= 8;
        }
        if (card_attribute_to_group(id) != 0) {
            s->flags |= 0x40;
        }
        v = card_get_atk(id);
        if (v > s->atk_max) {
            s->atk_max = v;
        }
        if (v < s->atk_min) {
            s->atk_min = v;
        }
        v = card_get_def(id);
        if (v > s->def_max) {
            s->def_max = v;
        }
        if (v < s->def_min) {
            s->def_min = v;
        }
    }
}

void duel_deck_stats(RecipeData *d, DeckStats *s)
{
    unsigned int i;
    unsigned int j;

    memset(s, 0, 0x68);
    s->def_min = 0xFFFF;
    s->atk_min = 0xFFFF;
    if (d == 0) {
        return;
    }
    for (i = 0; i < d->n_side; i++) {
        int id = d->side_ids[i];

        duel_deck_stats_add(id, s);
        for (j = i + 1; j < d->n_side; j++) {
            if (id == d->side_ids[j]) {
                s->flags |= 0x20;
            }
        }
    }
    for (i = 0; i < d->n_main; i++) {
        int id = d->main_ids[i];

        duel_deck_stats_add(id, s);
        for (j = i + 1; j < d->n_main; j++) {
            if (id == d->main_ids[j]) {
                s->flags |= 0x20;
            }
        }
    }
}

/* One-time flag i of the save area at +0xCC; returns 1 the first time. */
int duel_save_once_cc(int i)
{
    unsigned char *p = (unsigned char *)((int)sysstate2_get_ptr_51c() + i) + 0xCC;

    if (*p == 0) {
        *p = 1;
        return 1;
    }
    return 0;
}

int duel_save_once_120(int i)
{
    int e = (int)sysstate2_get_ptr_51c();
    int r = (int)game_state2_get() + 0x3C;
    unsigned char *p;

    if (sysflag_test(7) == 0) {
        return 0;
    }
    p = (unsigned char *)(i + e) + 0x120;
    if (*p == 0) {
        *p = 1;
        ((unsigned char *)(i + r))[0x1EC] = 1;
        return 1;
    }
    return 0;
}
