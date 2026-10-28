// @range 0x088156A4-0x08815B54
/* Items lying on the map: a table of records (date window, character level, event flag and character
   flag conditions, place, item) filtered per place, and the bits of the items already taken (one
   word per route). */
#include "libstory.h"

typedef struct StPickupFound {         /* item listed by st_pickup_list */
    int id;                            /* +0x0 record index + 1 */
    int item[3];                       /* +0x4 */
} StPickupFound;

typedef struct StPickup {              /* 0x08ACB2DC, 0x84 */
    unsigned short *table;             /* +0x00 records of 15 u16, ended by a zero place/item */
    StPickupFound found[8];            /* +0x04 */
} StPickup;

extern unsigned int *st_pickup_taken;  /* 0x08ACB2D8 taken bits, one word per route */
extern StPickup st_pickup;

int story_route_index(void);
void inv_adjust_count(unsigned short id, int dir);
void sysflag_set(unsigned int bit);

int st_pickup_is_taken(unsigned int id);
void st_pickup_set_taken(unsigned int id, int on);
int st_pickup_all_taken(void);

void st_pickup_init(void)
{
    memset(&st_pickup, 0, sizeof(StPickup));
}

void st_pickup_term(void)
{
    memset(&st_pickup, 0, sizeof(StPickup));
}

void st_pickup_set_flags(unsigned int *p)
{
    st_pickup_taken = p;
}

void st_pickup_set_table(unsigned short *t)
{
    st_pickup.table = t;
}

/* items of place `place` still lying there: out gets 3 words per item (at most 9); returns the count */
int st_pickup_list(int place, int *out)
{
    StPickup *s = &st_pickup;
    unsigned short *r = st_pickup.table;
    int n = 0;
    int i;
    int j;

    memset(st_pickup.found, 0, sizeof(st_pickup.found));
    i = 0;
    for (;;) {
        if (r[11] == 0 && r[12] == 0 && r[13] == 0 && r[14] == 0)
            return n;
        if (place != r[11])
            goto next;
        if (!story_date_in_range(r[0], r[1], r[2], r[3]))
            goto next;
        if (r[4] != 0 && r[5] != st_chr_level(r[4]))
            goto next;
        if (r[6] != 0) {
            int on = story_flag_get(r[6]) != 0;
            if (on) {
                if (r[7] == 4)
                    goto next;
            } else if (r[7] == 3)
                goto next;
        }
        if (r[8] != 0) {
            int on = st_chr_flag_get(r[8], r[9]) != 0;
            if (on) {
                if (r[10] == 4)
                    goto next;
            } else if (r[10] == 3)
                goto next;
        }
        if (st_pickup_is_taken(i + 1))
            goto next;
        out[0] = r[12];
        out[1] = r[13];
        out[2] = r[14];
        for (j = 0; j < 8; j++) {
            if (s->found[j].id == 0) {
                s->found[j].id = i + 1;
                s->found[j].item[0] = r[12];
                s->found[j].item[1] = r[13];
                s->found[j].item[2] = r[14];
                break;
            }
        }
        n++;
        out += 3;
        if (n > 8)
            return n;
    next:
        r += 15;
        i++;
    }
}

/* record id (index + 1) of a listed item, 0 when not listed */
int st_pickup_find(int a, int b, int c)
{
    StPickup *s = &st_pickup;
    int i;

    for (i = 0; i < 8; i++) {
        if (s->found[i].item[0] == a && s->found[i].item[1] == b && s->found[i].item[2] == c)
            return s->found[i].id;
    }
    return 0;
}

/* picks up (dir 1) or drops (dir -1) an item of the list */
void st_pickup_take(unsigned int item, int dir, int b, int c)
{
    int add;

    switch (dir) {
    case -1:
        add = 0;
        break;
    case 1:
        add = 1;
        break;
    default:
        return;
    }
    inv_adjust_count(item, add);
    if (add) {
        st_pickup_set_taken(st_pickup_find(item, b, c), 1);
        if (st_pickup_all_taken())
            sysflag_set(3);
    }
}

void st_pickup_set_taken(unsigned int id, int on)
{
    unsigned int *w;

    if (id != 0 && id < 0x21) {
        w = &st_pickup_taken[story_route_index()];
        if (on)
            *w |= 1 << (id - 1);
        else
            *w &= ~(1 << (id - 1));
    }
}

int st_pickup_is_taken(unsigned int id)
{
    unsigned int *w;

    if (id == 0)
        return 0;
    if (id > 0x20)
        return 0;
    w = &st_pickup_taken[story_route_index()];
    return (*w & (1 << (id - 1))) != 0;
}

int st_pickup_all_taken(void)
{
    int i;
    unsigned int *p = st_pickup_taken;

    for (i = 0; i < 2; i++, p++) {
        if (*p != -1)
            return 0;
    }
    return 1;
}
