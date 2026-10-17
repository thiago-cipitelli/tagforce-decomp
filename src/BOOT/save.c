// @range 0x0883E670-0x0883F340
// @asm 0x0883EAB4-0x0883ECB8
// @asm 0x0883ED7C-0x0883EE0C
/* Save state ("sysstate"): new game defaults, checksum, player name, day counter, money and the
   duel record counters. Continues the game.c unit (main.c, main2.c). */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "game.h"
#include "allegrex.h"

int sceUtilityGetSystemParamString(int id, char *out, int size);
int sceOpenPSIDGetOpenPSID(void *psid);
int cfg_get_language(void);
void cfg_set_byte0_bits0_2(int);
void str_to_wide(unsigned short *dst, int max, const char *src);
void recipe_load(int n, RecipeData *out);
void rec3_set_u32(int which, unsigned int v);
void rec3_set_u16(int which, int i, int v);
void inv_adjust_count(int id, int dir);
void inv_set_bits1_2_to_2(int id);
unsigned short *recipe_get_name(int n);
unsigned short *wcscpy(unsigned short *, const unsigned short *);
int sysflag_get2(int id);
void sysflag_set2(int id, int value);
void cfg_defaults_apply(void);
void newgame_init_inventory(void);
int card_is_attribute9(int id);

void newgame_init(void)
{
    SaveExtra *x;             /* declaration order sets $s-registers and stack slots (first = highest) */
    unsigned int i;
    SysSave *s;
    unsigned short *src;
    unsigned short *dst;
    int n;
    int lang;
    RecipeData rec;
    char nick[128];
    unsigned short nick16[32];

    s = sysstate_get();
    x = sysstate2_get_ptr_51c();
    lang = cfg_get_language();
    memset(s, 0, 0xFCA4);
    cfg_set_byte0_bits0_2(lang);
    sceUtilityGetSystemParamString(1, nick, 0x80);
    str_to_wide(nick16, 0x1F, nick);
    n = 0;
    src = nick16;
    dst = s->name;
    do {
        if (*src == 0)
            break;
        if (*src == '$') { /* '$' is the text engine's escape */
            *dst = '$';
            dst++;
        }
        *dst = *src;
        n++;
        src++;
        dst++;
    } while (n < 8);
    recipe_load(0, &rec);
    rec3_set_u32(1, rec.n_main);
    for (i = 0; i < rec.n_main; i++) {
        rec3_set_u16(1, i, rec.main_ids[i]);
        inv_adjust_count(rec.main_ids[i], 1);
        inv_set_bits1_2_to_2(rec.main_ids[i]);
    }
    rec3_set_u32(2, rec.n_extra);
    for (i = 0; i < rec.n_extra; i++) {
        rec3_set_u16(2, i, rec.extra_ids[i]);
        inv_adjust_count(rec.extra_ids[i], 1);
        inv_set_bits1_2_to_2(rec.main_ids[i]); /* sic: original indexes the main list */
    }
    rec3_set_u32(3, rec.n_side);
    for (i = 0; i < rec.n_side; i++) {
        rec3_set_u16(3, i, rec.side_ids[i]);
        inv_adjust_count(rec.side_ids[i], 1);
        inv_set_bits1_2_to_2(rec.main_ids[i]); /* sic */
    }
    wcscpy(s->deck_name, recipe_get_name(0));
    sysflag_set2(0, 2);
    sysflag_set2(1, 2);
    sysflag_set2(3, 2);
    sysflag_set2(4, 2);
    sceOpenPSIDGetOpenPSID(s->psid);
    cfg_defaults_apply();
    newgame_init_inventory();
    x->unk94 = 0x92784;
    x->unk98 = 999;
    s->version = 0x10002;
}

void rand_set_state(unsigned int v)
{
    g_game.rand_state = v;
}

unsigned int rand(void)
{
    g_game.rand_state = g_game.rand_state * 0x343FD + 0x269EC3;
    return ALLEGREX_EXT(g_game.rand_state, 16, 15);
}

unsigned char *syspack_get(void)
{
    return pack_sysgfx;
}

SysSave *sysstate_get(void)
{
    return (SysSave *)&g_game.save;
}

/* Sum of (address low 16 bits XOR halfword) over the save body; returns nonzero when it changed. */
int sysstate_checksum(void)
{
    SysSave *s = sysstate_get();
    unsigned short *p = (unsigned short *)s->psid;
    unsigned int sum = 0;
    unsigned int chk;
    int changed;
    int i;

    for (i = 0; i < 0x7E4C; i++) {
        sum += ((unsigned int)p & 0xFFFF) ^ *p;
        p++;
    }
    chk = ~sum - 0x7E4C;
    changed = g_game.save_checksum != chk;
    g_game.save_checksum = chk;
    return changed;
}

int sysstate_size(void)
{
    return 0xFCA4;
}

void sysstate_set_name(const void *name)
{
    SysSave *s = sysstate_get();
    memcpy(s->name, name, sizeof s->name);
}

unsigned short *sysstate_get_name(void)
{
    return sysstate_get()->name;
}

#ifdef NON_MATCHING
/* Progress needed to finish `day`: 7 on day 1, then each day 1/7 longer than the previous.
   The original is this loop unrolled by 8; prologue branch shapes never matched, kept in asm. */
int day_threshold(int day)
{
    int i = 1;
    int step, total;

    if (day > 99)
        return 0;
    if (day == 1)
        return 7;
    if (day == 0)
        return 0;
    step = total = 7;
    for (; i < day; i++) {
        step += step / 7;
        total += step;
    }
    return total;
}
#endif

int day_next(void)
{
    return sysstate_get()->day + 1;
}

/* Adds progress and returns how many days passed (0 when already at day 99). */
int day_advance(int delta)
{
    SysSave *s = sysstate_get();
    int day = s->day;

    if (day == 99)
        return 0;
    s->day_progress += delta;
    while (day_threshold(s->day + 1) <= s->day_progress)
        s->day++;
    return s->day - day;
}

#ifdef NON_MATCHING
int day_calc(int which) /* kept in asm: case-2 block layout never matched */
{
    SysSave *s = sysstate_get();
    int t = day_threshold(s->day + 1);

    switch (which) {
    case 0:
        return s->day_progress;
    case 1:
        return t;
    case 2:
        if (t == 0)
            return 0;
        return t - s->day_progress;
    }
    return 0;
}
#endif

int sysstate_get_u32_54(int i)
{
    SysSave *s = sysstate_get();
    return (&s->money)[i];
}

void money_add(int delta)
{
    SysSave *s = sysstate_get();

    s->money += delta;
    if (s->money < 0)
        s->money = 0;
    if (s->money > 999999999)
        s->money = 999999999;
    if (delta > 0) {
        s->money_total += delta;
        if (s->money_total > 999999999)
            s->money_total = 999999999;
    }
}

DuelRecords *duel_records_get(void)
{
    return &sysstate_get()->records;
}

void sysstate2_get_triplet(int which, unsigned int *out)
{
    DuelRecords *r = duel_records_get();

    switch (which) {
    case 0:
        out[0] = r->total[0];
        out[1] = r->total[1];
        out[2] = r->total[2];
        break;
    case 1: /* sic: entry 0, not 1 */
        out[0] = r->opp[0].count[0];
        out[1] = r->opp[0].count[1];
        out[2] = r->opp[0].count[2];
        break;
    default:
        out[0] = r->opp[which].count[0];
        out[1] = r->opp[which].count[1];
        out[2] = r->opp[which].count[2];
        break;
    }
}

SaveExtra *sysstate2_get_ptr_51c(void)
{
    return (SaveExtra *)((unsigned char *)duel_records_get() + 0x51C);
}

/* kind 1..3 = result kind; opp1/opp2 = opponent indexes (1 = none). */
void duel_record_add(int kind, int opp1, int opp2)
{
    DuelRecords *r = duel_records_get();

    if (opp1 < 0 || opp1 == 1)
        return;
    switch (kind) {
    case 1:
        if (r->total[0] < 999999)
            r->total[0]++;
        if (r->opp[opp1].count[0] < 999)
            r->opp[opp1].count[0]++;
        if (opp2 == 0 || opp2 > 1) {
            if (r->opp[opp2].count[0] < 999)
                r->opp[opp2].count[0]++;
        }
        if (r->streak < 999999)
            r->streak++;
        if (r->best_streak < r->streak)
            r->best_streak = r->streak;
        r->streak2 = 0;
        if (r->streak >= 10)
            sysflag_set2(0x1D, 2);
        break;
    case 2:
        if (r->total[1] < 999999)
            r->total[1]++;
        if (r->opp[opp1].count[1] < 999)
            r->opp[opp1].count[1]++;
        if (opp2 == 0 || opp2 > 1) {
            if (r->opp[opp2].count[1] < 999)
                r->opp[opp2].count[1]++;
        }
        if (r->streak2 < 999999)
            r->streak2++;
        if (r->best_streak2 < r->streak2)
            r->best_streak2 = r->streak2;
        r->streak = 0;
        if (sysflag_get2(0x1D) == 2)
            sysflag_set2(0x1D, 1);
        if (sysflag_get2(0x1E) == 0 && r->streak2 >= 10)
            sysflag_set2(0x1E, 2);
        break;
    case 3:
        if (r->total[2] < 999999)
            r->total[2]++;
        if (r->opp[opp1].count[2] < 999)
            r->opp[opp1].count[2]++;
        if (opp2 == 0 || opp2 > 1) {
            if (r->opp[opp2].count[2] < 999)
                r->opp[opp2].count[2]++;
        }
        r->streak2 = 0;
        r->streak = 0;
        if (sysflag_get2(0x1D) == 2)
            sysflag_set2(0x1D, 1);
        break;
    }
}

unsigned short sysstate2_rec_get_u16(int which, int i)
{
    DuelRecords *r = duel_records_get();
    unsigned short v = 0;

    switch (which) {
    case 0:
        v = r->opp[i].count[0];
        break;
    case 1:
        v = r->opp[i].count[1];
        break;
    case 2:
        v = r->opp[i].count[2];
        break;
    }
    return v;
}

int card_is_attribute9_alias(int id)
{
    return card_is_attribute9(id);
}
