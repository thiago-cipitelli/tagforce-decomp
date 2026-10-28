// @range 0x088062FC-0x08806A18
// @asm 0x08806480-0x0880650C
// @asm 0x088067EC-0x08806854
// @asm 0x08806928-0x088069B0
/* Story player state (save data): room and position, the current event ids, the point counters,
   the counts of the 20 items and 50 cards held, and the bit set of cards ever obtained. */
#include "libstory.h"

int sev_find(int x, int y, int z);
void sev_done_set(unsigned int n, int on);
int st_pl_room_prev_get(void);
void st_pl_room_prev_set(int room);
void st_pl_card_bit_set(unsigned int n, int on);
int st_pl_card_bits_all(void);
int st_pl_points_get(void);

void st_pl_init(void)
{
}

void st_pl_term(void)
{
}

void st_pl_attach(StoryPlayer *pl, unsigned int *bits)
{
    st_pl = pl;
    st_pl_bits = bits;
}

void st_pl_clear(void)
{
    memset(st_pl, 0, sizeof(StoryPlayer));
}

void st_pl_room_set(int room)
{
    StoryPlayer *pl = st_pl;

    st_pl_room_prev_set(pl->room);
    pl->room = room;
}

int st_pl_room_get(void)
{
    int room = st_pl->room;

    if (room == 0)
        room = 1;
    return room;
}

void st_pl_room_prev_set(int room)
{
    st_pl->prev_room = room;
}

int st_pl_room_prev_get(void)
{
    return st_pl->prev_room;
}

void st_pl_pos_set(float x, float z, int dir)
{
    StoryPlayer *pl = st_pl;

    pl->x = x;
    pl->z = z;
    pl->dir = dir;
}

float st_pl_x_get(void)
{
    return st_pl->x;
}

float st_pl_z_get(void)
{
    return st_pl->z;
}

int st_pl_dir_get(void)
{
    return st_pl->dir;
}

int *st_pl_items_get(void)
{
    return st_pl->items;
}

void st_pl_item_inc(int i)
{
    StoryPlayer *pl = st_pl;
    int n = pl->items[i] + 1;

    if (n < 100)
        pl->items[i] = n;
}

void st_pl_item_dec(int i)
{
    StoryPlayer *pl = st_pl;
    int n = pl->items[i] - 1;

    if (n >= 0)
        pl->items[i] = n;
}

int st_pl_item_has(int i)
{
    return st_pl->items[i] != 0;
}

#ifdef NON_MATCHING /* os dois addu saem com os operandos trocados (base + i*4, *p + n) */
void st_pl_item_add(int i, int n, int y, int z)
{
    StoryPlayer *pl = st_pl;
    int *p;

    if (i <= 0 || i >= 21)
        return;
    if (n <= 0 || n > 99)
        return;
    p = &pl->items[i];
    n += *p;
    if (n < 100) {
        *p = n;
        sev_done_set(sev_find(i, y, z), 1);
    }
}
#endif

void st_pl_x18_set(int v)
{
    st_pl->x18 = v;
}

int st_pl_x18_get(void)
{
    return st_pl->x18;
}

void st_pl_x1c_set(int v)
{
    st_pl->x1c = v;
}

int st_pl_x1c_is(int v)
{
    return st_pl->x1c == v;
}

void st_pl_x20_set(int v)
{
    st_pl->x20 = v;
}

int st_pl_x20_get(void)
{
    return st_pl->x20;
}

int st_pl_x20_is(int v)
{
    return st_pl->x20 == v;
}

void st_pl_card_last_set(int n)
{
    StoryPlayer *pl = st_pl;

    st_pl_card_bit_set(n, 1);
    if (st_pl_card_bits_all())
        sysflag_set(2);
    pl->x24 = n;
}

int st_pl_card_last_get(void)
{
    return st_pl->x24;
}

int st_pl_card_last_is(int n)
{
    return st_pl->x24 == n;
}

int *st_pl_cards_get(void)
{
    return st_pl->cards;
}

void st_pl_card_dec(int i)
{
    StoryPlayer *pl = st_pl;
    int n = pl->cards[i] - 1;

    if (n >= 0)
        pl->cards[i] = n;
}

int st_pl_card_has(int i)
{
    return st_pl->cards[i] != 0;
}

void st_pl_card_add(int i, int n)
{
    StoryPlayer *pl = st_pl;
    int k;
    int sum = 0;

    if (i <= 0 || i >= 51)
        return;
    if (n == 0)
        return;
    for (k = 1; k < 51; k++) {
        sum += pl->cards[k];
        if (sum + n > 10)
            return;
    }
    if (n > 0)
        st_pl_card_last_set(i);
    pl->cards[i] += n;
}

void st_pl_x150_set(int v)
{
    st_pl->x150 = v;
}

int st_pl_x150_get(void)
{
    return st_pl->x150;
}

int st_pl_cards_full(void)
{
    StoryPlayer *pl = st_pl;
    int sum = 0;
    int k;

    for (k = 1; k < 51; k++) {
        sum += pl->cards[k];
        if (sum >= 10)
            return 1;
    }
    return 0;
}

int *st_pl_x2c_ptr(void)
{
    return &st_pl->x2c;
}

void st_pl_x2c_inc(void)
{
    StoryPlayer *pl = st_pl;

    if (++st_pl->x2c > 999)
        pl->x2c = 999;
}

void st_pl_x2c_dec(void)
{
    StoryPlayer *pl = st_pl;

    if (--st_pl->x2c < 0)
        pl->x2c = 0;
}

int st_pl_points_get(void)
{
    return st_pl->points;
}

#ifdef NON_MATCHING /* o original testa 3, 2, 4, 1 e põe o corpo de 1 fora da linha; nenhuma forma de switch/if reproduz */
int st_pl_points(void)
{
    switch (story_mode_get()) {
    case 1:
        return -1;
    case 4:
        return -1;
    case 2:
    case 3:
        break;
    }
    return st_pl_points_get();
}
#endif

void st_pl_points_add(int d)
{
    StoryPlayer *pl = st_pl;

    if ((pl->points += d) < 0)
        pl->points = 0;
    if (pl->points > 999)
        pl->points = 999;
}

void st_pl_x14_set(int v)
{
    st_pl->x14 = v;
}

int st_pl_x14_get(void)
{
    return st_pl->x14;
}

int st_pl_x14_is(int v)
{
    return st_pl->x14 == v;
}

void st_pl_x154_set(unsigned short v)
{
    st_pl->x154 = v;
}

void st_pl_x156_set(unsigned short v)
{
    st_pl->x156 = v;
}

void st_pl_x158_set(int v)
{
    st_pl->x158 = v;
}

unsigned short st_pl_x154_get(void)
{
    return st_pl->x154;
}

unsigned short st_pl_x156_get(void)
{
    return st_pl->x156;
}

int st_pl_x158_get(void)
{
    return st_pl->x158;
}

#ifdef NON_MATCHING /* o desvio para o laço sai bnezl com a primeira instrução no delay slot; o original tem bnez + nop */
void st_pl_card_bit_set(unsigned int n, int on)
{
    unsigned int *bits = st_pl_bits;
    int w = 0;

    if (n == 0 || n >= 51)
        return;
loop:
    if (n > 32) {
        n -= 32;
        w++;
        goto loop;
    }
    bits += w;
    if (on)
        *bits |= 1 << (n - 1);
    else
        *bits &= ~(1 << (n - 1));
}
#endif

int st_pl_card_bits_all(void)
{
    unsigned int *bits = st_pl_bits;
    unsigned int i = 1;
    unsigned int n;
    int w;
    unsigned int *p;

    do {
        n = i;
        w = 0;
    loop:
        if (n > 32) {
            n -= 32;
            w++;
            goto loop;
        }
        p = &bits[w];
        if ((*p & (1 << (n - 1))) == 0)
            return 0;
    } while (++i < 51);
    return 1;
}
