// @range 0x088138AC-0x08813C84
/* Map events of the story: a table of events (date range, character level, story flag and
   character flag conditions, kind and position) filtered each time the field asks for the events of
   a kind, plus the 128 bits that mark the events already seen. */
#include "libstory.h"

int sev_done_get(unsigned int n);

void sev_init(void)
{
    memset(&sev, 0, sizeof(SevState));
}

void sev_term(void)
{
    memset(&sev, 0, sizeof(SevState));
}

void sev_set_done(unsigned int *bits)
{
    sev_done = bits;
}

void sev_clear_done(void)
{
    memset(sev_done, 0, 0x10);
}

void sev_set_table(SevEntry *table)
{
    sev.table = table;
}

/* the events of a kind whose conditions hold now and not seen yet: their positions in out (x, y, z),
   at most 9; each one also goes to every free slot */
int sev_collect(int kind, int *out)
{
    SevState *s = &sev;
    SevEntry *table = sev.table;
    int n = 0;
    int i;
    SevEntry *e;
    int j;

    memset(sev.slot, 0, sizeof(sev.slot));
    for (i = 0, e = table;; e++, i++) {
        if (e->kind == 0 && e->x == 0 && e->y == 0 && e->z == 0)
            return n;
        if (kind != e->kind)
            continue;
        if (!story_date_in_range(e->from_month, e->from_day, e->to_month, e->to_day))
            continue;
        if (e->chr != 0 && e->level != st_chr_level(e->chr))
            continue;
        if (e->flag != 0) {
            if (story_flag_get(e->flag) ? 1 : 0) {
                if (e->flag_cond == 4)
                    continue;
            } else if (e->flag_cond == 3)
                continue;
        }
        if (e->chr2 != 0) {
            if (st_chr_flag_get(e->chr2, e->bit) ? 1 : 0) {
                if (e->bit_cond == 4)
                    continue;
            } else if (e->bit_cond == 3)
                continue;
        }
        if (sev_done_get(i + 1))
            continue;
        out[0] = e->x;
        out[1] = e->y;
        out[2] = e->z;
        for (j = 0; j < 8; j++) {
            if (s->slot[j].id == 0) {
                s->slot[j].id = i + 1;
                s->slot[j].x = table[i].x;
                s->slot[j].y = table[i].y;
                s->slot[j].z = table[i].z;
            }
        }
        n++;
        out += 3;
        if (n > 8)
            return n;
    }
}

int sev_find(int x, int y, int z)
{
    SevState *s = &sev;
    int i;

    for (i = 0; i < 8; i++) {
        if (s->slot[i].x == x && s->slot[i].y == y && s->slot[i].z == z)
            return s->slot[i].id;
    }
    return 0;
}

void sev_done_set(unsigned int n, int on)
{
    int w;
    unsigned int *p;

    if (n != 0 && n <= 0x80) {
        w = 0;
        for (;;) {
            if (n <= 0x20)
                break;
            n -= 0x20;
            w++;
        }
        p = &sev_done[w];
        if (on)
            *p |= 1 << (n - 1);
        else
            *p &= ~(1 << (n - 1));
    }
}

int sev_done_get(unsigned int n)
{
    int w;
    unsigned int *p;

    if (n == 0)
        return 0;
    if (n > 0x80)
        return 0;
    w = 0;
    for (;;) {
        if (n <= 0x20)
            break;
        n -= 0x20;
        w++;
    }
    p = &sev_done[w];
    return (*p & (1 << (n - 1))) != 0;
}
