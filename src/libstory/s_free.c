// @range 0x088165F8-0x08816890
/* The free duel calendar: a list of (month, day, entry) telling which free duel entry applies on a
   date, the table of entries (records of 5 u16 found by id) and three words of free duel state. */
#include "libstory.h"

typedef struct StFreeDay {             /* 6 bytes, ended by month 0 and day 0 */
    unsigned short month;
    unsigned short day;
    unsigned short entry;              /* id in the entry table */
} StFreeDay;

typedef struct StFreeEntry {           /* 10 bytes */
    unsigned short id;
    unsigned short data[4];
} StFreeEntry;

typedef struct StFree {                /* 0x08ACBAB0, 0x10 */
    StFreeDay *days;                   /* +0x0 */
    StFreeEntry *entries;              /* +0x4 */
    int x8;                            /* +0x8 */
    int xc;                            /* +0xC */
} StFree;

extern int *st_free_state;             /* 0x08ACBAAC 3 words */
extern StFree st_free;

int scal_get_month(void);
int scal_get_day(void);

void st_init_8165f8(void)
{
    memset(&st_free, 0, sizeof(StFree));
}

void st_term_81660c(void)
{
    memset(&st_free, 0, sizeof(StFree));
}

void st_free_set_state(int *p)
{
    st_free_state = p;
}

void st_free_clear_state(void)
{
    memset(st_free_state, 0, 12);
}

void st_lesson_set_plan(StFreeDay *d)
{
    st_free.days = d;
}

void st_lesson_set_esm(StFreeEntry *e)
{
    st_free.entries = e;
}

void st_lesson_set_text(int a, int b)
{
    st_free.x8 = a;
    st_free.xc = b;
}

void st_flag666c_set(int v)
{
    st_free_state[0] = v;
}

int st_free_state0(void)
{
    return st_free_state[0];
}

void st_free_state1_set(int v)
{
    st_free_state[1] = v;
}

int st_free_state1(void)
{
    return st_free_state[1];
}

int st_free_xc(void)
{
    return st_free.xc;
}

int st_free_x8(void)
{
    return st_free.x8;
}

/* there is a free duel entry for today */
int st_free_duel_ok(void)
{
    StFree *s = &st_free;
    int month = scal_get_month();
    int day = scal_get_day();
    int i;

    for (i = 0;; i++) {
        StFreeDay *d = s->days + i;

        if (d->month == 0 && d->day == 0)
            return 0;
        if (d->month == month && d->day == day)
            return 1;
    }
}

StFreeEntry *st_free_entry(int id)
{
    StFree *s = &st_free;
    int i;

    for (i = 0;; i++) {
        StFreeEntry *p = s->entries + i;

        if (p->id == id)
            return &s->entries[i];
    }
}

/* entry of today, or of the last date of the list */
StFreeEntry *st_free_today(void)
{
    StFree *const s = &st_free;
    int month = scal_get_month();
    int day = scal_get_day();
    StFreeDay *days = s->days;
    int i;
    StFreeDay *d;
    int id;
    StFreeEntry *t;
    StFreeEntry *p;

    for (i = 0, d = days;; d++, i++) {
        if (d->month == 0 && d->day == 0) {
            i--;
            d = days + i;
            id = d->entry;
            break;
        }
        if (d->month == month && d->day == day) {
            id = d->entry;
            break;
        }
    }
    if (id == -1)
        id = 0;
    t = s->entries;
    for (i = 0, p = t;; p++, i++) {
        if (p->id == id)
            return &t[i];
    }
}

void st_free_state2_set(int v)
{
    st_free_state[2] = v;
}

int st_free_state2(void)
{
    return st_free_state[2];
}
