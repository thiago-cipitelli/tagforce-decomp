// @range 0x0883F340-0x08840478
/* Card inventory records (one 4-byte bitfield per card index), the current deck, tag partner slots
   and the player name inside the save state. Continues the game.c unit. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "game.h"

int card_index_from_id2(int id);
int card_is_attribute9(int id);
int card_attribute_icon_is3(int id);
unsigned short *wcsncpy(unsigned short *, const unsigned short *, int);
unsigned short *sysmsg_get(int i);

static InvRec *inv_rec(int id)
{
    SysSave *s = sysstate_get();
    int idx = card_index_from_id2(id);

    if (idx == 0 || card_is_attribute9(id))
        return 0;
    return &s->inv[idx];
}

int inv_get_count(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r)
        return r->count;
    return 0;
}

void inv_set_count(unsigned short id, int n)
{
    InvRec *r = inv_rec(id);

    if (r == 0)
        return;
    if (n > 99) {
        if (n < 0) /* sic: unreachable */
            return;
    }
    if (r->count == 0) {
        if (n)
            r->state = 1;
        else
            r->state = 0;
    } else {
        r->state = 2;
    }
    r->count = n;
}

/* dir 0: one less (not below 0); otherwise one more (up to 99) and mark as seen */
void inv_adjust_count(unsigned short id, int dir)
{
    InvRec *r = inv_rec(id);

    if (r == 0)
        return;
    if (dir != 0) {
        if (r->state == 0)
            r->state = 1;
        if (r->count < 99)
            r->count = r->count + 1;
    } else {
        if (r->count != 0)
            r->count--;
    }
}

void inv_set_state2(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r != 0)
        r->state = 2;
}

int inv_state_is1(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r)
        return r->state == 1;
    return 0;
}

int inv_get_state(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r)
        return r->state;
    return 0;
}

void inv_set_bit7(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r != 0)
        r->bit7 = 1;
}

int inv_get_bit7_default1(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r)
        return r->bit7;
    return 1;
}

void inv_set_flag7(unsigned short id, int on)
{
    InvRec *r = inv_rec(id);

    if (r == 0)
        return;
    if (on)
        r->flag7b = 1;
    else
        r->flag7b = 0;
}

int inv_get_flag7(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r)
        return r->flag7b;
    return 0;
}

void inv_set_bit0(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r != 0)
        r->flag0 = 1;
}

int inv_get_bit0(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r)
        return r->flag0;
    return 0;
}

void inv_set_bit3(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r != 0)
        r->flag3 = 1;
}

int inv_get_bit3(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r)
        return r->flag3;
    return 0;
}

void inv_set_hi(unsigned short id, int v)
{
    InvRec *r = inv_rec(id);

    if (r != 0 && v >= 0 && v < 8)
        r->hi = v;
}

int inv_get_hi(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r)
        return r->hi;
    return 0;
}

void sysstate_set_player_name(const unsigned short *name)
{
    SysSave *s = sysstate_get();

    if (s == 0)
        return;
    memset(s->player_name, 0, sizeof s->player_name);
    wcsncpy(s->player_name, name, 0x3F);
}

unsigned short *sysstate_get_player_name_ptr(void)
{
    SysSave *s = sysstate_get();

    if (s)
        return s->player_name;
    return 0;
}

void inv_set_limit(unsigned short id, unsigned short v)
{
    InvRec *r = inv_rec(id);

    if (r == 0)
        return;
    if (v >= 0 && v < 4)
        r->limit = v;
}

int inv_get_limit(unsigned short id)
{
    InvRec *r = inv_rec(id);

    if (r)
        return r->limit;
    return 3;
}

void newgame_init_inventory(void)
{
    int i;

    for (i = 4007; i < 0x1B2E; i++)
        inv_set_limit(i, 3);
    for (i = 0; i < sizeof(card_limit_table) / sizeof(card_limit_table[0]); i += 2)
        inv_set_limit(card_limit_table[i], card_limit_table[i + 1]);
    sysstate_set_player_name(sysmsg_get(0x17));
}

unsigned short card_limit_default(unsigned short id)
{
    unsigned int i;

    for (i = 0; i < 0xA6; i += 2) {
        if (id == card_limit_table[i])
            return card_limit_table[i + 1];
    }
    return 3;
}

RecipeData *deck_get(void)
{
    return &sysstate_get()->deck;
}

void deck_copy_out(RecipeData *dst)
{
    memcpy(dst, deck_get(), sizeof(RecipeData));
}

void deck_name_copy_out(void *dst)
{
    SysSave *s = sysstate_get();
    memcpy(dst, s->deck_name, sizeof s->deck_name);
}

void deck_name_copy_in(const void *src)
{
    SysSave *s = sysstate_get();
    memcpy(s->deck_name, src, sizeof s->deck_name);
}

void deck_clear(void)
{
    memset(deck_get(), 0, sizeof(RecipeData));
}

int deck_count(int which)
{
    RecipeData *d = deck_get();

    switch (which) {
    case 1:
        return d->n_main;
    case 2:
        return d->n_extra;
    case 3:
        return d->n_side;
    }
    return 0;
}

void deck_set_count(int which, unsigned int n)
{
    RecipeData *d = deck_get();

    switch (which) {
    case 1:
        d->n_main = n;
        break;
    case 2:
        d->n_extra = n;
        break;
    case 3:
        d->n_side = n;
        break;
    }
}

unsigned short deck_get_card(int which, int i)
{
    RecipeData *d = deck_get();

    if (i < 0)
        return 0;
    switch (which) {
    case 1:
        if (i < 80)
            return d->main_ids[i];
        break;
    case 2:
        if (i < 15)
            return d->extra_ids[i];
        break;
    case 3:
        if (i < 30)
            return d->side_ids[i];
        break;
    }
    return 0;
}

void deck_set_card(int which, int i, unsigned short id)
{
    RecipeData *d = deck_get();

    if (i < 0)
        return;
    switch (which) {
    case 1:
        if (i < 80)
            d->main_ids[i] = id;
        break;
    case 2:
        if (i < 15)
            d->extra_ids[i] = id;
        break;
    case 3:
        if (i < 30)
            d->side_ids[i] = id;
        break;
    }
}

void deck_add_card(int which, unsigned short id)
{
    RecipeData *d = deck_get();
    unsigned int n;

    switch (which) {
    case 1:
        n = d->n_main;
        if (n < 0x4F) {
            d->n_main = n + 1;
            d->main_ids[n] = id;
        }
        break;
    case 2:
        n = d->n_extra;
        if (n < 0xE) {
            d->n_extra = n + 1;
            d->extra_ids[n] = id;
        }
        break;
    case 3:
        n = d->n_side;
        if (n < 0x1D) {
            d->n_side = n + 1;
            d->side_ids[n] = id;
        }
        break;
    }
}

int partner_count_get(void)
{
    return sysstate_get()->partner_count;
}

void partner_count_set(unsigned short v)
{
    SysSave *s = sysstate_get();

    if (v >= 0 && v < 4)
        s->partner_count = v;
}

unsigned short partner_id_get(int i)
{
    SysSave *s = sysstate_get();

    if (i < 0 || i >= 3)
        return 0;
    return s->partner_ids[i];
}

void partner_id_set(int i, unsigned short v)
{
    SysSave *s = sysstate_get();

    if (i < 0 || i >= 3)
        return;
    s->partner_ids[i] = v;
}

int partner_state_get(int i)
{
    SysSave *s = sysstate_get();

    if (i >= 0) {
        if (i < 3)
            return s->partner_state[i];
    }
    return 0;
}

void partner_state_set(int i, unsigned short v)
{
    SysSave *s = sysstate_get();

    if (i >= 0 && i < 3)
        s->partner_state[i] = v;
}

/* Partners in state 1 or 3, plus state 2 when `include2` */
int partner_count_active(int include2)
{
    int i;
    int n = 0;

    for (i = 0; i < partner_count_get(); i++) {
        switch (partner_state_get(i)) {
        case 1:
        case 3:
            n++;
            break;
        case 2:
            if (include2)
                n++;
            break;
        }
    }
    return n;
}

#define DECK_FREE_COUNT(NAME)                                                   \
int NAME(unsigned short id)                                                     \
{                                                                               \
    int in_deck = 0;                                                            \
    int in_partner = 0;                                                         \
    int have = inv_get_count(id);                                               \
    int i;                                                                      \
    int st;                                                                     \
                                                                                \
    deck_get();                                                                 \
    if (card_attribute_icon_is3(id)) {                                          \
        for (i = 0; i < deck_count(3); i++)                                     \
            if (id == deck_get_card(3, i))                                      \
                in_deck++;                                                      \
    } else {                                                                    \
        for (i = 0; i < deck_count(1); i++)                                     \
            if (id == deck_get_card(1, i))                                      \
                in_deck++;                                                      \
        for (i = 0; i < deck_count(2); i++)                                     \
            if (id == deck_get_card(2, i))                                      \
                in_deck++;                                                      \
    }                                                                           \
    for (i = 0; i < partner_count_get(); i++) {                                 \
        if (id == partner_id_get(i)) {                                          \
            st = partner_state_get(i);                                          \
            switch (st) {                                                       \
            case 1:                                                             \
            case 2:                                                             \
            case 3:                                                             \
                in_partner++;                                                   \
                break;                                                          \
            }                                                                   \
        }                                                                       \
    }                                                                           \
    return have - (in_deck - in_partner);                                       \
}

/* Copies owned minus copies already in the deck (partner cards count as available). Two identical
   exported copies exist in the original. */
DECK_FREE_COUNT(deck_card_free_count_b)
DECK_FREE_COUNT(deck_card_free_count_a)

void sysstate_blob_get(void *dst)
{
    SysSave *s = sysstate_get();
    memcpy(dst, s->blob, sizeof s->blob);
}

void sysstate_blob_set(const void *src)
{
    SysSave *s = sysstate_get();
    memcpy(s->blob, src, sizeof s->blob);
}

FlagRec *flagrec_get(int id)
{
    SysSave *s = sysstate_get();

    if (id >= 0x30)
        return 0;
    return &s->flagrec[id];
}
