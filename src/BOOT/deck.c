// @range 0x08840DC4-0x08841BD0
// @data 0x088EE2F4-0x088EE3B8
// @rodata 0x08875598-0x08875600
/* Stage progress, per-character deck slots, deck/recipe comparison, request flags, region and
   language, and the language-dependent name/message packs. Continues the game.c unit. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "game.h"

unsigned short *table_get_entry(void *table, int i);
void *ehp_find(void *pack, const char *name, int flags);
void recipe_load(int n, RecipeData *out);
void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
int sysflag_test(unsigned int bit);
void sysflag_set(unsigned int bit);
int sysflag_get2(int id);
void sysflag_set2(int id, int value);
int deck_slot_get_level(int i);
static void sort_u16_desc(unsigned short *a, int n);
static int state_noop_update(void);
static int state_noop_exit(void);
void cfg_set_byte0_bits0_2(int v);

/* Enters a stage: records (which, v) in deck_cur, resets the flag records and tracks the max stage. */
void sysflag_apply(unsigned int which, unsigned short v)
{
    SysSave *s = sysstate_get();
    DeckCur *cur = deck_cur_get();

    cur->bit0 = 0;
    cur->which = which;
    cur->val = v;
    sysflag_recs_init();
    if (which == 1) {
        if (sysflag_get2(2) == 2)
            sysflag_set2(2, 1);
        if (sysflag_get2(5) == 2)
            sysflag_set2(5, 1);
    }
    if (s->unk97e8 < (int)which)
        s->unk97e8 = which;
}

void deck_cur_get_u16_0(short *out)
{
    *out = *(short *)deck_cur_get();
}

void deck_cur_set_bit0(void)
{
    DeckCur *c = deck_cur_get();
    c->bit0 = 1;
}

unsigned short *chara_get_name(int i, int j)
{
    char name[32];

    switch (cfg_get_language()) {
    case 0:
        strcpy(name, "chr_name_j.bin");
        break;
    case 1:
        strcpy(name, "chr_name_e.bin");
        break;
    case 2:
        strcpy(name, "chr_name_g.bin");
        break;
    case 3:
        strcpy(name, "chr_name_f.bin");
        break;
    case 4:
        strcpy(name, "chr_name_i.bin");
        break;
    case 5:
        strcpy(name, "chr_name_s.bin");
        break;
    default:
        return 0;
    }
    return table_get_entry(ehp_find(pack_chr_name, name, 0), i + j * 2);
}

int value_to_tier(int v)
{
    if (v >= 2) {
        if (v < 9)
            return 1;
        if (v < 0x29)
            return 2;
        if (v >= 0x29 && v < 0x98)
            return 3;
    }
    return 0;
}

/* Raises character `i`'s deck level to `n` and awards the related flags. */
void deck_slot_set_level(int i, int n)
{
    DeckSlot *d = &deck_slots_get()[i];
    int c;
    int k;

    if (n < 4) {
        if (d->level < n)
            d->level = n;
        if (sysflag_get2(0x19) == 0) {
            n = 0;
            for (i = 2; i < 0x98; i++) {
                if (deck_slot_get_level(i) == 3)
                    n++;
            }
            if (n >= 5)
                sysflag_set2(0x19, 2);
        }
        c = 0;
        for (k = 2; k < 9; k++) {
            if (deck_slot_get_level(k) == 3)
                c++;
        }
        if (sysflag_test(7) == 0 && c >= 1)
            sysflag_set(7);
        if (sysflag_test(8) == 0 && c >= 2)
            sysflag_set(8);
        if (sysflag_test(0) == 0 && c >= 7)
            sysflag_set(0);
    }
}

int deck_slot_get_level(int i)
{
    return deck_slots_get()[i].level;
}

void deck_slot_add_card(int i, unsigned int id)
{
    DeckSlot *d = &deck_slots_get()[i];
    int n = d->count;
    int k;

    if (n < 6) {
        for (k = 0; k < n; k++) {
            if (d->cards[k] == id)
                return;
        }
        d->cards[n] = id;
        d->count++;
    }
}

DeckSlot *deck_slot_get(int i)
{
    return &deck_slots_get()[i];
}

unsigned short deck_slot_get_value(int i)
{
    return deck_slots_get()[i].value;
}

void deck_slot_set_value(int i, int v)
{
    DeckSlot *d = &deck_slots_get()[i];

    if (v < 0)
        d->value = 0;
    else if (v > 8000)
        d->value = 8000;
    else
        d->value = v;
}

void deck_slot_add_value(int i, int delta)
{
    DeckSlot *d = &deck_slots_get()[i];

    int v = d->value + delta;

    if (v < 0) {
        d->value = 0;
    } else {
        d->value = v;
        if (d->value > 8000)
            d->value = 8000;
    }
}

int deck_find_best_slot(void)
{
    DeckSlot *d = deck_slots_get();
    int i, best = 2, bestv = 0;

    for (i = 2; i < 0x98; i++) {
        if (bestv < d[i].value) {
            bestv = d[i].value;
            best = i;
        }
    }
    return best;
}

void deck_name_set(int i, unsigned short c)
{
    DeckName *r = &deck_names_get()[i];

    if (r->set == 0) {
        r->set = 1;
        r->val = c & 0xFF;
    }
}

DeckName *deck_name_get(int i)
{
    return &deck_names_get()[i];
}

static void sort_u16_desc(unsigned short *a, int n)
{
    int i, j;
    unsigned short t;

    for (i = 0; i < n - 1; i++) {
        for (j = i + 1; j < n; j++) {
            if (a[i] < a[j]) {
                t = a[i];
                a[i] = a[j];
                a[j] = t;
            }
        }
    }
}

/* Nonzero when the current deck has the same main and side cards as recipe `n`. */
int deck_equals_recipe(int n)
{
    int equal = 0;
    RecipeData *a;
    RecipeData *b = 0;
    unsigned int i;

    a = heap_alloc(g_game.heap, sizeof(RecipeData));
    if (a != 0) {
        memset(a, 0, sizeof(RecipeData));
        b = heap_alloc(g_game.heap, sizeof(RecipeData));
        if (b != 0) {
            memset(b, 0, sizeof(RecipeData));
            recipe_load(n, a);
            if (a->n_main == deck_count(1) && a->n_side == deck_count(3)) {
                for (i = 0; i < a->n_main; i++)
                    b->main_ids[i] = deck_get_card(1, i);
                for (i = 0; i < a->n_side; i++)
                    b->side_ids[i] = deck_get_card(3, i);
                sort_u16_desc(a->main_ids, a->n_main);
                sort_u16_desc(b->main_ids, a->n_main);
                for (i = 0; i < a->n_main; i++) {
                    if (a->main_ids[i] != b->main_ids[i])
                        goto done;
                }
                sort_u16_desc(a->side_ids, a->n_side);
                sort_u16_desc(b->side_ids, a->n_side);
                for (i = 0; i < a->n_side; i++) {
                    if (a->side_ids[i] != b->side_ids[i])
                        goto done;
                }
                equal = 1;
            }
        }
    }
done:
    if (a != 0)
        heap_set_level(g_game.heap, a);
    if (b != 0)
        heap_set_level(g_game.heap, b);
    return equal;
}

SaveRec *sysstate_rec_get(int i)
{
    return &sysstate_get()->recs[i];
}

void req_flags_clear(void)
{
    memset(g_game.req_flags, 0, sizeof g_game.req_flags);
}

void req_flag_set(int i)
{
    g_game.req_flags[i] = 1;
}

int req_flag_get(int i)
{
    return g_game.req_flags[i];
}

int req_flags_any(void)
{
    int i = 0;

    do {
        if (req_flag_get(i++))
            return 1;
    } while (i <= 0x4A);
    return 0;
}

unsigned char sysstate_get_byte_97d0(int i)
{
    unsigned char *p = (unsigned char *)sysstate_get();
    return p[i + 0x97D0];
}

void sysstate_max_byte_97d0(int i, unsigned char v)
{
    SysSave *s = sysstate_get();

    if (i < 0x18) {
        if (s->byte_97d0[i] < v)
            s->byte_97d0[i] = v;
    }
}

void sys_state_set_default_callbacks(void)
{
    sys_state_set_callbacks(0, state_noop_update, state_noop_exit);
}

static int state_noop_update(void)
{
}

static int state_noop_exit(void)
{
}

int region_get(void)
{
    return 1; /* US build */
}

unsigned short region_pick_value(int flag)
{
    unsigned short v[2] = { 0x2000, 0x4000 };
    unsigned int idx = 0;

    if (region_get())
        idx ^= 1;
    if (flag == 1)
        idx ^= 1;
    return v[idx];
}

int cfg_get_language(void)
{
    return cfg_get()->lang;
}

void lang_apply(void)
{
    switch (region_get()) {
    case 0:
        cfg_set_byte0_bits0_2(0);
        break;
    case 1:
    case 6:
        cfg_set_byte0_bits0_2(1);
        break;
    case 2:
        cfg_set_byte0_bits0_2(2);
        break;
    case 3:
        cfg_set_byte0_bits0_2(3);
        break;
    case 4:
        cfg_set_byte0_bits0_2(4);
        break;
    case 5:
        cfg_set_byte0_bits0_2(5);
        break;
    }
}

void cfg_set_byte0_bits0_2(int v)
{
    SysConfig *c = cfg_get();
    c->lang = v & 7;
}

int region_to_lang_group(void)
{
    switch (region_get()) {
    case 0:
        return 1;
    case 1:
        return 3;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        return 2;
    default:
        return 0;
    }
}

unsigned short *sysmsg_get(int i)
{
    char name[32];

    switch (cfg_get_language()) {
    case 0:
        strcpy(name, "sysmsg_j.bin");
        break;
    case 1:
        strcpy(name, "sysmsg_e.bin");
        break;
    case 2:
        strcpy(name, "sysmsg_g.bin");
        break;
    case 3:
        strcpy(name, "sysmsg_f.bin");
        break;
    case 4:
        strcpy(name, "sysmsg_i.bin");
        break;
    case 5:
        strcpy(name, "sysmsg_s.bin");
        break;
    default:
        return 0;
    }
    return table_get_entry(ehp_find(pack_sysmsg, name, 0), i);
}
