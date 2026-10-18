// @range 0x088FF3FC-0x088FFFDC
// @asm 0x088FF8C4-0x088FFFDC
/* After the duel: kind of result of a tag series, deck statistics for the rewards, one-time
   flags in the save. */
#include "card_db.h"
#include "libc.h"
#include "duel.h"
#include "duel_setup.h"

typedef struct DuelResults {           /* game_state2_get(), 0x2A0 */
    unsigned char pad0[0x10];
    unsigned int frames;               /* +0x10 duel time in frames */
    int unk14;
    int lp_left[2];                    /* +0x18 */
    int result;                        /* +0x20 */
    int mode;                          /* +0x24 8: series of duels */
    unsigned char pad28[0xC];
    unsigned short n_duels;            /* +0x34 */
    unsigned short duels[3];           /* +0x36 result of each duel: 1, 2, 3 */
    int cur[0x28];                     /* +0x3C records set by this duel */
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
int duel_stat_get(int i);
extern int duel_view_player;

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

#ifdef NON_MATCHING /* same steps; the original keeps the duel results in $s0 and the save records in $s5, and gives the
                        record pointer $s1 */
#define REC_MAX(e)                                                             \
    v = (e);                                                                   \
    if (*best < v) {                                                           \
        *best = v;                                                             \
        *cur = v;                                                              \
    }                                                                          \
    break

/* Update the 40 duel records of the save (maximums, two minimums) after a duel and mark the
   records broken by this duel. */
void duel_records_update(void)
{
    DuelResults *r;
    int v;
    int *rec;
    int *best;
    int i;
    int *cur;

    r = game_state2_get();
    cur = (int *)((unsigned char *)r + 0x3C);
    rec = (int *)sysstate2_get_ptr_51c();
    best = rec;

    for (i = 0; i < 0x28; i++, cur++, best++) {
        *cur = 0;
        switch (i) {
        case 0:
            REC_MAX(duel_stat_get(0x13));
        case 1:
            REC_MAX(duel_stat_get(0x14));
        case 3:
            REC_MAX(duel_stat_get(0x2E));
        case 4:
            REC_MAX(duel_stat_get(0x2F));
        case 5:
            REC_MAX(duel_stat_get(4) + duel_stat_get(6) + duel_stat_get(2));
        case 6:
            REC_MAX(duel_stat_get(4));
        case 7:
            REC_MAX(duel_stat_get(6));
        case 8:
            REC_MAX(duel_stat_get(7));
        case 9: {
            int a = duel_stat_get(3);
            int b = duel_stat_get(5);

            b = MAX(b, a);
            REC_MAX(MAX(b, duel_stat_get(1)));
        }
        case 10:
            REC_MAX(duel_stat_get(0xF) + duel_stat_get(0xE));
        case 11:
            REC_MAX(duel_stat_get(0xF));
        case 12:
            REC_MAX(duel_stat_get(0xE));
        case 13:
            REC_MAX(duel_stat_get(0x27));
        case 14:
            REC_MAX(duel_stat_get(0x15) + duel_stat_get(0x16) + duel_stat_get(0x18) + duel_stat_get(0x23) +
                    duel_stat_get(0x24) + duel_stat_get(0x19));
        case 15:
            REC_MAX(duel_stat_get(0x15));
        case 16:
            REC_MAX(duel_stat_get(0x16));
        case 17:
            REC_MAX(duel_stat_get(0x18));
        case 18:
            REC_MAX(duel_stat_get(0x23));
        case 19:
            REC_MAX(duel_stat_get(0x24));
        case 20:
            REC_MAX(duel_stat_get(0x19));
        case 21:
            REC_MAX(duel_stat_get(0x17));
        case 22:
            REC_MAX(duel_stat_get(0));
        case 23:
            REC_MAX(duel_stat_get(0x30));
        case 24:
            REC_MAX(duel_stat_get(0x1F));
        case 25:
            REC_MAX(duel_stat_get(0x21));
        case 26:
            REC_MAX(duel_stat_get(0x22));
        case 27:
            REC_MAX(duel_stat_get(0x2D));
        case 28:
            REC_MAX(duel_stat_get(0xB));
        case 30:
            REC_MAX(duel_stat_get(0x34));
        case 31:
            REC_MAX(duel_stat_get(0x25));
        case 32:
            REC_MAX(duel_stat_get(0x26));
        case 33:
            REC_MAX(duel_stat_get(0x28));
        case 34:
            REC_MAX(duel_stat_get(0x29));
        case 35:
            REC_MAX(duel_stat_get(0x2B));
        case 36:
            REC_MAX(r->lp_left[duel_view_player ^ 1]);
        case 37:
            if (duel_setup.unk228 == 10) {
                break;
            }
            if (duel_setup.unk228 == 11) {
                break;
            }
            v = (unsigned int)r->frames / 30 + 1;
            if (v < *best) {
                *best = v;
                *cur = v;
            }
            break;
        case 38:
            if (duel_setup.unk228 == 10) {
                break;
            }
            if (duel_setup.unk228 == 11) {
                break;
            }
            v = duel.turn + 1;
            if (v < *best) {
                *best = v;
                *cur = v;
            }
            break;
        case 39:
            v = duel.turn + 1;
            if (*best < v) {
                *best = v;
                *cur = v;
            }
        }
        if (*cur != 0) {
            ((unsigned char *)rec)[i + 0xA0] = 1;
        }
    }
}
#endif
