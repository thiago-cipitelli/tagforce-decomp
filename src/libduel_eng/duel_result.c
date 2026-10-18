// @range 0x088FF3FC-0x08900C94
// @asm 0x088FF8C4-0x08900C94
/* After the duel: kind of result of a tag series, deck statistics for the rewards, one-time
   flags in the save. */
#include "card_db.h"
#include "libc.h"
#include "duel.h"
#include "duel_setup.h"


DuelResults *game_state2_get(void);
int sysflag_test(int bit);
int duel_stat_get(int i);
extern int duel_view_player;
extern unsigned int duel_turn_player;
extern int duel_w_0891A194;
extern int duel_w_0891A198;
extern int duel_w_0891A19C;
int deck_slot_get_level(int i);
int day_next(void);
int sysvar_get_a98(void);
unsigned int duel_turn_get(void);
int duel_tag_slot(int side);
int duel_card_used(int id);

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

#ifdef NON_MATCHING /* same steps; the original assigns the $s registers of the pointers and the index differently */
/* Bonuses of the duel (r->bonus[i]); a bonus counts only the first time it is earned (save +0xCC).
   With any new bonus, bonus 0x52 too, and the total is 30 per new bonus. */
void duel_bonus_calc(void)
{
    DuelResults *r = game_state2_get();
    int *cur = r->cur;
    unsigned char *rec = (unsigned char *)sysstate2_get_ptr_51c();
    unsigned char *g = (unsigned char *)game_state_get();
    int n = 0;
    int i;
    int *b;
    int k;

    for (i = 0, b = cur; i < 0x53; i++, b++) {
        b[0x28] = 0;
        switch (i) {
        case 0:
            for (k = 2; k < 0x98; k++) {
                if (deck_slot_get_level(k) == 3) {
                    b[0x28] += 2;
                }
            }
            break;
        case 1:
            b[0x28] = day_next() * 2;
            break;
        case 2:
            if (sysflag_test(7)) {
                for (k = 0; k < 0x76; k++) {
                    if (rec[k + 0x120] != 0) {
                        b[0x28] += 2;
                    }
                }
            }
            break;
        case 5:
            b[0x28] = (duel_turn_get() + 1) * 2;
            break;
        case 6:
            if (g[0x11A] == 2) {
                b[0x28] = 0x32;
            }
            break;
        case 7:
            if (sysvar_get_a98()) {
                b[0x28] = 0x32;
            }
            break;
        case 8:
            if (g[0x119] == 2 && duel_series_result() == 1) {
                b[0x28] = 0x32;
            }
            break;
        case 9:
            if (duel_w_0891A194 != 0) {
                b[0x28] = 400;
            }
            break;
        case 10:
            b[0x28] = duel_w_0891A198 * 25;
            break;
        case 11:
            if (duel_setup.unk228 == 2) {
                b[0x28] = 0x14;
            }
            break;
        case 12:
            if (duel_setup.unk228 != 10 && duel_setup.unk228 != 11 && duel.turn + 1 < 6) {
                b[0x28] = 10;
            }
            break;
        case 13:
            if (r->unk14 != 0) {
                b[0x28] = 10;
            }
            break;
        case 14:
            if (duel_turn_player != duel_view_player) {
                b[0x28] = 0x14;
            }
            break;
        case 15:
            if (g[0x11A] == 2 && duel_turn_player == duel_view_player && duel_w_0891A19C != duel_tag_slot(duel_view_player)) {
                b[0x28] = 10;
            }
            break;
        case 16:
            if (r->lp[duel_view_player] <= 1000) {
                b[0x28] = 0x14;
            }
            break;
        case 17:
            if (r->lp[duel_view_player] <= 100) {
                b[0x28] = 100;
            }
            break;
        case 18:
            if (duel_setup.unk228 != 10 && duel_setup.unk228 != 11 && duel_stat_get(0x36) == 0) {
                b[0x28] = 10;
            }
            break;
        case 19:
            if (r->lp[duel_view_player] >= 20000) {
                b[0x28] = 0x14;
            }
            break;
        case 20:
            if (r->lp[duel_view_player] == 0x1662) {
                b[0x28] = 0x23D;
            }
            break;
        case 21:
            if ((unsigned int)duel.players[duel_view_player & 1].n_d < 6) {
                b[0x28] = 0x14;
            }
            break;
        case 22:
            if (duel.players[duel_view_player & 1].n_d == 0) {
                b[0x28] = 100;
            }
            break;
        case 23:
            k = duel_stat_get(0x1F);
            if (k > 9) {
                b[0x28] = k * 2;
            }
            break;
        case 24:
            k = duel_stat_get(0x21);
            if (k > 9) {
                b[0x28] = k * 2;
            }
            break;
        case 25:
            if (duel_setup.unk228 != 10 && duel_setup.unk228 != 11 && duel_stat_get(0x1F) == 0) {
                b[0x28] = 0xF;
            }
            break;
        case 26:
            if (duel_setup.unk228 != 10 && duel_setup.unk228 != 11 && duel_stat_get(0x21) == 0) {
                b[0x28] = 0xF;
            }
            break;
        case 27:
            b[0x28] = duel_stat_get(0x23) * 4;
            break;
        case 28:
            b[0x28] = duel_stat_get(0x24) * 4;
            break;
        case 29:
            b[0x28] = duel_stat_get(0x18) * 4;
            break;
        case 30:
            if (duel_setup.unk228 != 10 && duel_setup.unk228 != 11 && duel_stat_get(0x19) == 0) {
                b[0x28] = 10;
            }
            break;
        case 31:
            k = duel_stat_get(0x25);
            if (k > 2) {
                b[0x28] = k * 2;
            }
            break;
        case 32:
            k = duel_stat_get(0);
            if (k > 2999) {
                b[0x28] = k / 500;
            }
            break;
        case 33: {
            int a = duel_stat_get(3);

            k = duel_stat_get(5);
            k = MAX(k, a);
            a = duel_stat_get(1);
            k = MAX(a, k);
            if (k > 2999) {
                b[0x28] = k / 250;
            }
            break;
        }
        case 34:
            k = duel_stat_get(0x14);
            if (k > 4999) {
                b[0x28] = k / 250;
            }
            break;
        case 35:
            k = duel_stat_get(5);
            if (k > 1499) {
                b[0x28] = k / 50;
            }
            break;
        case 36:
            if (duel_stat_get(0x37) != 0) {
                b[0x28] = 0x14;
            }
            break;
        case 37:
            if (r->mode == 1 && duel_stat_get(1) == 0 && (duel_stat_get(3) != 0 || duel_stat_get(5) != 0)) {
                b[0x28] = 10;
            }
            break;
        case 38:
            if (r->mode == 1 && duel_stat_get(3) == 0 && duel_stat_get(5) == 0 && duel_stat_get(1) != 0) {
                b[0x28] = 0x14;
            }
            break;
        case 39:
            k = duel_stat_get(0xF);
            if (k > 7) {
                b[0x28] = k * 4;
            }
            break;
        case 40:
            k = duel_stat_get(0xD);
            if (k > 7) {
                b[0x28] = k * 4;
            }
            break;
        case 41:
            k = duel_stat_get(0x26);
            if (k > 4) {
                b[0x28] = k * 4;
            }
            break;
        case 42:
            k = duel_stat_get(0x28);
            if (k > 4) {
                b[0x28] = k * 4;
            }
            break;
        case 43:
            k = duel_stat_get(0x29);
            if (k > 4) {
                b[0x28] = k * 4;
            }
            break;
        case 44:
            k = duel_stat_get(0x2B);
            if (k > 4) {
                b[0x28] = k * 4;
            }
            break;
        case 45:
            k = duel_stat_get(0x34);
            if (k > 2) {
                b[0x28] = k * 4;
            }
            break;
        case 46:
            k = duel_stat_get(0x2E);
            if (k > 4) {
                b[0x28] = k * 4;
            }
            break;
        case 47:
            k = duel_stat_get(0x30);
            if (k > 2) {
                b[0x28] = k * 6;
            }
            break;
        case 48:
            if (duel_stat_get(0x3B) != 0) {
                b[0x28] = 10;
            }
            break;
        case 49:
            if (duel_stat_get(0x3C) != 0) {
                b[0x28] = 10;
            }
            break;
        case 50:
            if (duel_stat_get(0x3D) != 0) {
                b[0x28] = 0x3C;
            }
            break;
        case 51:
            b[0x28] = r->lp_left[duel_view_player ^ 1] * 20;
            break;
        case 52:
            if (duel_setup.unk228 == 3) {
                b[0x28] = 0x32;
            }
            break;
        case 53:
            if (duel_setup.unk228 == 4) {
                b[0x28] = 100;
            }
            break;
        case 54:
            if (duel_setup.unk228 == 7) {
                b[0x28] = 0x32;
            }
            break;
        case 55:
            if (duel_setup.unk228 == 6) {
                b[0x28] = 0x32;
            }
            break;
        case 56:
            if (duel_stat_get(0x3F) != 0) {
                b[0x28] = 1;
            }
            break;
        case 57:
            if (duel_card_used(0xFE9) != 0) {
                b[0x28] = 0x1E;
            }
            break;
        case 58:
            if (duel_card_used(0x1117) != 0) {
                b[0x28] = 10;
            }
            break;
        case 59:
            if (duel_card_used(0x111C) != 0) {
                b[0x28] = 0x1E;
            }
            break;
        case 60:
            if (duel_card_used(0x1122) != 0) {
                b[0x28] = 0x14;
            }
            break;
        case 61:
            if (duel_card_used(0x112E) != 0) {
                b[0x28] = 10;
            }
            break;
        case 62:
            if (duel_card_used(0x128C) != 0) {
                b[0x28] = 10;
            }
            break;
        case 63:
            if (duel_card_used(0x138A) != 0) {
                b[0x28] = 0x1E;
            }
            break;
        case 64:
            if (duel_card_used(0x146E) != 0) {
                b[0x28] = 0x1E;
            }
            break;
        case 65:
            if (duel_card_used(0x15B4) != 0) {
                b[0x28] = 0x14;
            }
            break;
        case 66:
            if (duel_card_used(0x1645) != 0) {
                b[0x28] = 0x1E;
            }
            break;
        case 67:
            if (duel_card_used(0x17EE) != 0) {
                b[0x28] = 0x14;
            }
            break;
        case 68:
            if (duel_card_used(0x183D) != 0) {
                b[0x28] = 0x14;
            }
            break;
        case 69:
            if (duel_card_used(0x1905) != 0) {
                b[0x28] = 0x14;
            }
            break;
        case 70:
            if (duel_card_used(0x1A4E) != 0) {
                b[0x28] = 0x14;
            }
            break;
        case 71:
            if (duel_card_used(0x162D) != 0) {
                b[0x28] = 5;
            }
            break;
        case 72:
            if (duel_stat_get(0x40) != 0) {
                b[0x28] = 5;
            }
            break;
        case 73:
            if (duel_card_used(0x16A3) != 0) {
                b[0x28] = 10;
            }
            break;
        case 74:
            if (duel_card_used(0x16CF) != 0) {
                b[0x28] = 10;
            }
            break;
        case 75:
            if (duel_card_used(0x16DC) != 0) {
                b[0x28] = 5;
            }
            break;
        case 76:
            if (duel_card_used(0x17A8) != 0) {
                b[0x28] = 10;
            }
            break;
        case 77:
            if (duel_card_used(0x17CA) != 0) {
                b[0x28] = 5;
            }
            break;
        case 78:
            if (duel_card_used(0x184E) != 0) {
                b[0x28] = 5;
            }
            break;
        case 79:
            if (duel_card_used(0x188D) != 0) {
                b[0x28] = 10;
            }
            break;
        case 80:
            if (duel_card_used(0x143F) != 0) {
                b[0x28] = 0x1E;
            }
            break;
        case 81:
            if (duel_card_used(0x143D) != 0) {
                b[0x28] = 10;
            }
            break;
        }
        if (b[0x28] != 0 && duel_save_once_cc(i)) {
            n++;
        }
    }
    if (n != 0) {
        if (duel_save_once_cc(0x52)) {
            n++;
        }
        cur[0x28 + 0x52] = n * 30;
    }
}
#endif
