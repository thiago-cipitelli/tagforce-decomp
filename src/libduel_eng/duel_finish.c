// @range 0x08900C94-0x08901BA0
// @asm 0x08900CD4-0x08901AC8
/* End of a duel: results of the series, the 118 titles, records and bonuses; the character table. */
#include "card_db.h"
#include "libc.h"
#include "duel.h"
#include "duel_setup.h"

typedef struct DuelChar {              /* 0x18, 41 at 0x089135FC sorted by id */
    unsigned short id : 10;
    unsigned short b10 : 3;
    unsigned short n : 2;              /* entries of data */
    unsigned short b15 : 1;
    unsigned char pad2[0xE];
    int data;                          /* +0x10 */
    int pad14;
} DuelChar;

extern DuelChar duel_chars[0x29];      /* 0x089135FC */
extern int duel_view_player;
extern unsigned int duel_turn_player;
extern int duel_w_0891A194;
extern int duel_w_0891A198;
extern int duel_w_0891A19C;

DuelResults *game_state2_get(void);
DeckSlot *deck_slot_get(int i);
int deck_slot_get_level(int i);
int duel_stat_get(int i);
int duel_card_used(int id);
int duel_series_result(void);
int duel_save_once_cc(int i);
int duel_save_once_120(int i);
unsigned int duel_turn_get(void);
int duel_lp_get(unsigned int p);
int duel_setup_224_get(void);
int duel_setup_228_get2(void);
void duel_deck_stats(RecipeData *d, DeckStats *s);
void duel_records_update(void);
void duel_bonus_calc(void);
int sysflag_test(int bit);
void sysstate2_get_triplet(int which, int *out);
int inv_collection_permille(void);
int charmap_lookup(int id, int *out);
short scene_stack_peek_arg(int i);
int sysvar_get_a98(void);
int cfg_get_flag(int which);
void stat_event_record(int id, int kind);
DuelChar *duel_char_find(int id);
int duel_char_data(int id, int k);

int duel_char_known(int id)
{
    int n;

    if (id < 0x29) {
        return duel_char_data(id, 0) > 0;
    }
    n = 0;
    charmap_lookup(id, &n);
    return n > 0;
}

#ifdef NON_MATCHING /* same checks; the original keeps the stats pointer in $s0, has an empty case 117 and assigns other registers */
/* Check the 118 titles not yet earned (save +0x120) and earn the ones whose condition holds. */
void duel_titles_check(void)
{
    unsigned char *rec;
    unsigned char *g;
    unsigned short *p;
    unsigned char *save;
    int slot;
    int i;
    int k;
    unsigned int v;
    int t[3];
    DeckStats *s = &duel_deck_stats_cur;

    game_state2_get();
    rec = (unsigned char *)sysstate2_get_ptr_51c();
    g = (unsigned char *)game_state_get();
    p = (unsigned short *)s;
    save = (unsigned char *)sysstate_get();
    if (sysflag_test(7) == 0) {
        return;
    }
    slot = duel_w_0891A19C * 0x188;
    for (i = 0; i < 0x76; i++, p++) {
        if (rec[i + 0x120] != 0) {
            continue;
        }
        v = 0;
        switch (i) {
        case 0:
            sysstate2_get_triplet(0, t);
            v = t[0] >= 100;
            break;
        case 1:
            v = 1;
            for (k = 2; k < 0x98; k++) {
                if (duel_char_known(k) && (sysstate2_get_triplet(k, t), t[0] < 10)) {
                    v = 0;
                    break;
                }
            }
            break;
        case 2:
            if (duel_stat_get(0x34) > 9) {
                v = 1;
            }
            break;

        case 3:
            v = duel_stat_get(4) >= 10000;
            break;
        case 4:
            v = duel_stat_get(6) >= 4000;
            break;
        case 5:
            v = duel_stat_get(7) >= 20000;
            break;
        case 6:
            v = duel_stat_get(4) + duel_stat_get(6) + duel_stat_get(2) >= 50000;
            break;
        case 7:
            v = duel_stat_get(0xF) >= 20;
            break;
        case 8:
            v = duel_stat_get(0xE) >= 20;
            break;
        case 9:
            v = duel_stat_get(0x27) >= 10;
            break;
        case 10:
            v = duel_stat_get(0x15) >= 10;
            break;
        case 11:
            v = duel_stat_get(0x16) >= 10;
            break;
        case 12:
            v = duel_stat_get(0x23) >= 10;
            break;
        case 13:
            v = duel_stat_get(0x24) >= 10;
            break;
        case 14:
            v = duel_stat_get(0x19) >= 20;
            break;
        case 15:
            v = duel_stat_get(0x1A) >= 10;
            break;
        case 16:
            v = duel_stat_get(0x17) >= 15;
            break;
        case 17:
            v = duel_stat_get(0x30) >= 5;
            break;
        case 18:
            v = duel_stat_get(0xB) >= 8000;
            break;
        case 19:
            v = duel_stat_get(0x1F) >= 20;
            break;
        case 20:
            v = duel_stat_get(0x20) >= 10;
            break;
        case 21:
            v = duel_stat_get(0x21) >= 10;
            break;
        case 22:
            v = duel_stat_get(0x22) >= 10;
            break;
        case 23:
            v = duel_stat_get(0x2D) >= 10;
            break;
        case 24:
            v = duel_stat_get(0x25) >= 5;
            break;
        case 25:
            v = duel_stat_get(0x2F) >= 10;
            break;
        case 26:
            v = duel_stat_get(0x28) >= 20;
            break;
        case 27:
            v = duel_stat_get(0x12) >= 20000;
            break;
        case 28:
            if (g[0x119] == 2) {
                v = duel_series_result() == 1;
            }
            break;
        case 30:
            if (duel_setup.unk228 != 10) {
                v = !(duel_stat_get(0x36) != 0);
            }
            break;
        case 31:
            v = duel_stat_get(9) >= 8000;
            break;
        case 32:
            if (duel_stat_get(1) >= 8000 || duel_stat_get(3) >= 8000 || duel_stat_get(5) >= 8000) {
                v = 1;
            }
            break;
        case 33:
            v = duel_setup.unk228 == 2;
            break;
        case 34:
            v = duel.turn + 1 < 8;
            break;
        case 35:
            v = duel.turn + 1 < 6;
            break;
        case 36:
            v = duel.turn + 1 < 4;
            break;
        case 37:
            if (duel_setup.unk228 != 10) {
                v = duel_turn_player != duel_view_player;
            }
            break;
        case 38:
            v = duel_setup.unk228 == 3;
            break;
        case 39:
            v = duel_setup.unk228 == 4;
            break;
        case 40:
            v = duel_setup.unk228 == 7;
            break;
        case 41:
            v = 0;
            if (g[0x119] == 2) {
                v = duel_setup.unk228 == 8;
            }
            break;
        case 42:
            v = duel_setup.unk228 == 5;
            break;
        case 43:
            v = duel_card_used(0x15B1) && duel_card_used(0x15B4) && duel_card_used(0x15F9) && duel_card_used(0x15FA);
            break;
        case 44:
            v = duel_card_used(0x1954);
            break;
        case 45:
            v = duel_card_used(0x19A3);
            break;
        case 46:
            v = duel_card_used(0x19A4);
            break;
        case 47:
            v = duel_card_used(0x19A5);
            break;
        case 48:
            v = duel_card_used(0x19A3) && duel_card_used(0x19A4) && duel_card_used(0x19A5);
            break;
        case 49:
            v = duel_card_used(0x16CF);
            break;
        case 50:
            v = duel_card_used(0x15EF);
            break;
        case 51:
            v = duel_card_used(0xFE9);
            break;
        case 52:
            v = duel_card_used(0x111C);
            break;
        case 53:
            v = duel_card_used(0x138A);
            break;
        case 54:
            v = duel_card_used(0x146E);
            break;
        case 55:
            v = duel_card_used(0x16A3);
            break;
        case 56:
            v = duel_card_used(0x143F);
            break;
        case 57:
            v = duel_stat_get(0x3F);
            break;
        case 58:
            v = (s->flags & 0x40) == 0;
            break;
        case 59:
            v = (s->levels & 0x1F) >> 1 == 0;
            break;
        case 60:
            v = (s->levels >> 1 & 0xFF0) == 0;
            break;
        case 61:
            v = (s->levels >> 1 & 0xFFE) == 0;
            break;
        case 62:
            v = (s->levels & 0x1FFF) >> 1 == 0xFFF;
            break;
        case 63:
            v = s->atk_min >= 1500;
            break;
        case 64:
            v = s->atk_max <= 1500;
            break;
        case 65:
            v = s->atk_max < 2;
            break;
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
        case 71:
            v = s->n_monster == p[-0x1B];
            break;
        case 72:
        case 73:
        case 74:
        case 75:
        case 76:
        case 77:
        case 78:
        case 79:
        case 80:
        case 81:
        case 82:
        case 83:
        case 84:
        case 85:
        case 86:
        case 87:
        case 88:
        case 89:
        case 90:
        case 91:
            v = s->n_monster == p[-0x36];
            break;
        case 92:
            v = (s->flags & 1) == 0;
            break;
        case 93:
            v = (s->flags & 2) == 0;
            break;
        case 94:
            v = (s->flags & 4) == 0;
            break;
        case 95:
            v = (s->flags & 8) == 0;
            break;
        case 96:
            v = s->n_monster == s->attr[2] + s->attr[3];
            break;
        case 97:
            v = s->n_monster == 0;
            break;
        case 98:
            v = s->attr[14] == 0;
            break;
        case 99:
            v = s->attr[13] == 0;
            break;
        case 100:
            v = s->attr[13] + s->attr[14] == 0;
            break;
        case 101:
            v = *(int *)((unsigned char *)g + slot + 0x1A0) == 80;
            break;
        case 102:
            v = (s->flags & 0x10) == 0;
            break;
        case 103:
            v = (s->flags & 0x20) == 0;
            break;
        case 104:
            v = duel_stat_get(0x18) == 0;
            break;
        case 105:
            v = duel_stat_get(0x19) == 0;
            break;
        case 106:
            v = duel_stat_get(0x15) == 0;
            break;
        case 107:
            v = 1;
            for (k = 2; k < 0x98; k++) {
                if (duel_char_known(k)) {
                    sysstate2_get_triplet(k, t);
                    if (deck_slot_get(k)->count == 0) {
                        v = 0;
                        break;
                    }
                }
            }
            break;
        case 108:
            v = sysflag_test(2);
            break;
        case 109:
            v = sysflag_test(1);
            break;
        case 110:
            v = 1;
            for (k = 2; k < 9; k++) {
                if (deck_slot_get_level(k) != 3) {
                    v = 0;
                    break;
                }
            }
            break;
        case 111:
            v = 1;
            for (k = 2; k < 0x98; k++) {
                if (duel_char_known(k) && deck_slot_get_level(k) != 3) {
                    v = 0;
                    break;
                }
            }
            break;
        case 112:
            v = inv_collection_permille() >= 300;
            break;
        case 113:
            v = inv_collection_permille() >= 500;
            break;
        case 114:
            v = inv_collection_permille() >= 1000;
            break;
        case 115:
            v = 1;
            for (k = 0; k < 0x30; k++) {
                if (save[k + 0x3E38] == 0) {
                    v = 0;
                    break;
                }
            }
            break;
        case 116:
            if (sysflag_test(7)) {
                v = 1;
                for (k = 0; k < 0x2C; k++) {
                    if (save[k + 0x3DF8] == 0) {
                        v = 0;
                        break;
                    }
                }
            }
            break;
        }
        if (v != 0) {
            duel_save_once_120(i);
        }
    }
}
#endif

#ifdef NON_MATCHING /* same steps; the original keeps the game state in $s5 and the results in $s1 */
/* Called when the duel is over: fill the results, then the titles, records and bonuses of the
   single human player. Returns 1 when they were computed. */
int duel_finish(void)
{
    unsigned short scene = scene_stack_peek_arg(6);
    GameState *g = game_state_get();
    DuelResults *r = game_state2_get();
    int *cur = r->cur;
    PlayerSlot *a;
    PlayerSlot *b;
    int n;
    int cnt;
    int i;
    int k;

    sysstate2_get_ptr_51c();
    r->result = duel_setup_224_get();
    r->mode = duel_setup_228_get2();
    r->lp[0] = duel_lp_get(0);
    r->lp[1] = duel_lp_get(1);
    if (((unsigned char *)g)[0x119] == 2) {
        r->duels[r->n_duels++] = r->result;
    }
    if (sysvar_get_a98() == 0 && cfg_get_flag(0) == 1) {
        switch (r->mode) {
        case 3:
            stat_event_record(0xFBB, 0);
            break;
        case 4:
            stat_event_record(0x1468, 0);
            break;
        }
    }
    if (((unsigned char *)g)[0x118] != 1) {
        return 0;
    }
    n = 2;
    if (g->partner_mode == 2) {
        n = 4;
    }
    cnt = 0;
    for (i = 0; i < n; i++) {
        if (g->slot[i].type == 1) {
            cnt++;
            duel_w_0891A19C = i;
        }
    }
    if (cnt != 1) {
        return 0;
    }
    memset(cur, 0, 0x264);
    if (duel_w_0891A19C == 0 || duel_w_0891A19C == 2) {
        a = &g->slot[1];
        b = &g->slot[3];
    } else {
        a = &g->slot[0];
        b = &g->slot[2];
    }
    if (g->partner_mode == 1) {
        duel_record_add(r->result, a->type, -1);
    } else {
        duel_record_add(r->result, a->type, b->type);
    }
    k = scene == 0xD ? 1 : 2;
    switch (r->result) {
    case 2:
        r->bonus[3] = 10;
        duel_save_once_cc(3);
        r->bonus[5] = k * (duel_turn_get() + 1);
        duel_save_once_cc(5);
        return 1;
    case 3:
        r->bonus[4] = 0x32;
        duel_save_once_cc(4);
        r->bonus[5] = k * (duel_turn_get() + 1);
        duel_save_once_cc(5);
        duel_save_once_120(0x1D);
        return 1;
    case 0:
        return 0;
    default:
        duel_deck_stats(&g->slot[duel_w_0891A19C].recipe, &duel_deck_stats_cur);
        if (g->partner_mode == 2) {
            duel_w_0891A198 = (recipe_get_id(a->deck_id) + recipe_get_id(b->deck_id)) / 2;
        } else {
            duel_w_0891A198 = recipe_get_id(a->deck_id);
        }
        duel_w_0891A194 = day_advance(duel_w_0891A198 / (scene == 0xD ? 2 : 1) * 3);
        duel_titles_check();
        duel_records_update();
        duel_bonus_calc();
        return 1;
    }
}
#endif

DuelChar *duel_char_find(int id)
{
    unsigned short key = id;
    int lo = 0;
    int hi = 0x28;
    int mid;

    do {
        DuelChar *c;
        int v;

        mid = (lo + hi) / 2;
        c = &duel_chars[mid];
        v = c->id;
        if (v == key) {
            return c;
        }
        if (v < key) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    } while (lo <= hi);
    return 0;
}

int duel_char_data(int id, int k)
{
    DuelChar *c = duel_char_find(id);
    unsigned short i;

    if (c == 0) {
        return -1;
    }
    i = k;
    if (i > 0 && i >= c->n) {
        return -1;
    }
    return c->data + i;
}
