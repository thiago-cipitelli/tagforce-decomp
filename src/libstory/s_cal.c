// @range 0x0880B3E4-0x0880C728
// @rodata 0x0881C16C-0x0881C29C
// @asm 0x0880B65C-0x0880B6B4
// @asm 0x0880BF94-0x0880C07C
// @asm 0x0880C4C8-0x0880C598
// @data 0x0881D1E0-0x0881D32C
/* The story calendar (scal): the date kept in the save (month 1..8 of the story = May..December,
   day, 73 time slots of 15 minutes from 5:45), the derived fields (hour, minute, week day, days
   left), the story flags 1..64 / 129..192, the chapter of each date and the text/branch tables
   (stt) set by the script loader. */
#include "libstory.h"

int st_chr_level(int chr);
int st_chr_flag_get(int chr, unsigned int bit);

void scal_set_month(int month);
void scal_set_day(int day);
void scal_set_slot(int slot);
void scal_refresh(void);
void scal_update(ScalDate *d);
void scal_from_tick(ScalDate *d);
void scal_calc_cal_month(ScalDate *d);
void scal_calc_month(ScalDate *d);
void scal_calc_hour(ScalDate *d);
void scal_calc_minute(ScalDate *d);
void scal_calc_wday(ScalDate *d);
void scal_calc_days_left(ScalDate *d);
int scal_period_a(ScalDate *d);
int scal_period_b(ScalDate *d);
int scal_daynum_of(ScalDate *d);
int scal_get_month(void);
int scal_get_day(void);

/* chapter of the story by date: the first entry with the month and (day 0 or) the day */
ScalPeriod scal_periods[19] = {
    { 1, 1, 1, 1 },   { 1, 7, 2, 0 },   { 1, 0x11, 3, 2 }, { 1, 0x17, 4, 0 }, { 1, 0, -1, 0 },
    { 2, 6, 5, 3 },   { 2, 0xE, 6, 4 }, { 2, 0x17, 7, 5 }, { 2, 0, -1, 0 },   { 3, 6, 9, 7 },
    { 3, 0xC, 0xA, 8 }, { 3, 0x1F, 0xC, 0xA }, { 3, 0, -1, 0 }, { 4, 1, -1, 0xB }, { 4, 0, -1, 0 },
    { 5, 0, -1, 0 },  { 6, 0, -1, 0 },  { 7, 0, -1, 0 },   { 8, 0, -1, 0 },
};

void scal_init(void)
{
    memset(&stt, 0, sizeof(stt));
}

void scal_term(void)
{
    memset(&stt, 0, sizeof(stt));
}

void scal_bind(ScalDate *date, int *state, unsigned int *flags)
{
    scal_date = date;
    scal_state = state;
    scal_flags = flags;
}

void scal_clear(void)
{
    ScalDate *d = scal_date;

    memset(scal_state, 0, 0xC);
    memset(d, 0, sizeof(ScalDate));
}

/* date at the start of a story mode */
void scal_start(int mode)
{
    ScalDate *d = scal_date;

    d->tick = 1;
    d->month = 1;
    d->day = 1;
    d->slot = 1;
    switch (mode) {
    case 1:
        scal_set_month(1);
        scal_set_day(1);
        scal_set_slot(8);
        break;
    case 2:
        scal_set_month(4);
        scal_set_day(1);
        scal_set_slot(1);
        break;
    case 3:
        scal_set_month(4);
        scal_set_day(3);
        scal_set_slot(1);
        break;
    case 4:
        scal_set_month(3);
        scal_set_day(1);
        scal_set_slot(1);
        break;
    }
}

int sdate_get_0c(void)
{
    return scal_date->slot;
}

int scal_get_month(void)
{
    return scal_date->month;
}

int scal_get_cal_month(void)
{
    return scal_date->cal_month;
}

int scal_get_day(void)
{
    return scal_date->day;
}

void scal_tick_next(void)
{
    scal_date->tick++;
    scal_refresh();
}

void scal_tick_add(int n)
{
    scal_date->tick += n;
    scal_refresh();
}

/* chapter n days after today */
int sdate_check(int days)
{
    ScalDate d;

    d.tick = scal_date->tick + days * 73;
    d.month = scal_date->month;
    d.day = scal_date->day;
    d.slot = scal_date->slot;
    scal_from_tick(&d);
    return scal_period_a(&d);
}

int scal_period_of(int cal_month, int day)
{
    ScalDate d;

    d.cal_month = cal_month;
    scal_calc_month(&d);
    d.day = day;
    return scal_period_b(&d);
}

int scal_period_now(void)
{
    return scal_period_b(scal_date);
}

#ifdef NON_MATCHING /* o original guarda uma cópia morta do `li v1,2` no alvo do bnel */
/* before the 24th of month 2 */
int sdate_is_daytime(void)
{
    ScalDate *d = scal_date;
    int r = 1;

    if (d->month == 1) {
        if (d->day >= 24)
            r = 0;
    } else if (d->month == 2) {
        if (d->day < 24)
            r = 0;
    }
    return r;
}
#endif

/* from the 24th of month 2 on */
int st_room_variant(void)
{
    ScalDate *d = scal_date;
    int r = 0;

    if (d->month == 2) {
        if (d->day > 23)
            r = 1;
    } else if (d->month > 2) {
        r = 1;
    }
    return r;
}

int scal_after_m1_10(void)
{
    ScalDate *d = scal_date;
    int r = d->month >= 1;

    if (r)
        r = d->day > 9;
    return r;
}

/* 1 when the day n days after today is a Sunday */
int sdate_weekday_flag(int n)
{
    int sunday[7] = { 0, 0, 0, 0, 0, 0, 1 };
    int day = scal_daynum_of(scal_date);

    return sunday[(day + n - 1) % 7];
}

void scal_flag_set(unsigned int id, int on)
{
    unsigned int *p = scal_flags;
    int *st = scal_state;

    if (id == 0)
        return;
    if (id != 0 && id <= 32)
        p = (unsigned int *)&st[1];
    else if (id > 32 && id <= 64)
        p = (unsigned int *)&st[2];
    else if (id > 128 && id <= 160)
        p = scal_flags;
    else if (id > 160 && id <= 192)
        p++;
    else
        return;
    if (on)
        *p |= 1 << (id & 0x1F);
    else
        *p &= ~(1 << (id & 0x1F));
}

int story_flag_get(unsigned int id)
{
    unsigned int *f = scal_flags;
    int *st = scal_state;
    unsigned int w;

    if (id == 0)
        return 0;
    if (id != 0 && id <= 32)
        w = st[1];
    else if (id > 32 && id <= 64)
        w = st[2];
    else if (id > 128 && id <= 160)
        w = f[0];
    else if (id > 160 && id <= 192)
        w = f[1];
    else
        return 0;
    return (w & (1 << (id & 0x1F))) != 0;
}

void scal_flag2_set(unsigned int id, int on)
{
    unsigned int *f = scal_flags;
    unsigned int *p;

    if (id != 0 && id <= 32) {
        p = &f[2];
        if (on)
            *p |= 1 << (id - 1);
        else
            *p &= ~(1 << (id - 1));
    }
}

int scal_flag2_get(unsigned int id)
{
    unsigned int *f = scal_flags;

    if (id == 0)
        return 0;
    if (id > 32)
        return 0;
    return (f[2] & (1 << (id - 1))) != 0;
}

void scal_set_month(int month)
{
    ScalDate *d = scal_date;

    d->month = month;
    scal_update(d);
}

void scal_set_day(int day)
{
    ScalDate *d = scal_date;
    int max = 0;

    switch (d->month) {
    case 1: max = 31; break;
    case 2: max = 30; break;
    case 3: max = 31; break;
    case 4: max = 31; break;
    case 5: max = 30; break;
    case 6: max = 31; break;
    case 7: max = 30; break;
    case 8: max = 31; break;
    default: day = 31; break;
    }
    if (max < day)
        day = max;
    d->day = day;
    scal_update(d);
}

void scal_set_slot(int slot)
{
    ScalDate *d = scal_date;

    if (slot < 1)
        slot = 1;
    if (slot > 73)
        slot = 73;
    d->slot = slot;
    scal_update(d);
}

void scal_next_month(void)
{
    ScalDate *d = scal_date;

    switch (d->month) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        d->month++;
        goto update;
    case 8:
        return;
    case 0:
        break;
    }
    return;
update:
    scal_update(d);
}

void scal_next_day(void)
{
    ScalDate *d = scal_date;

    d->day++;
    switch (d->month) {
    case 1:
        if (d->day > 31) {
            d->month = 2;
            d->day = 1;
        }
        break;
    case 2:
        if (d->day > 30) {
            d->month = 3;
            d->day = 1;
        }
        break;
    case 3:
        if (d->day > 31) {
            d->month = 4;
            d->day = 1;
        }
        break;
    case 4:
        if (d->day > 31) {
            d->month = 5;
            d->day = 1;
        }
        break;
    case 5:
        if (d->day > 30) {
            d->month = 6;
            d->day = 1;
        }
        break;
    case 6:
        if (d->day > 31) {
            d->month = 7;
            d->day = 1;
        }
        break;
    case 7:
        if (d->day > 30) {
            d->month = 8;
            d->day = 1;
        }
        break;
    case 8:
        if (d->day > 31)
            d->day = 31;
        break;
    }
    scal_update(d);
}

void stt_set_c(unsigned short *base, int *off)
{
    stt.base_c = base;
    stt.off_c = off;
}

unsigned short *stext_get(int i)
{
    return stt.base_c + stt.off_c[i - 1];
}

void stt_set_a(int count, unsigned short *base, int *off)
{
    stt.count_a = count;
    stt.base_a = base;
    stt.off_a = off;
}

unsigned short *stt_get_a(int i)
{
    return stt.base_a + stt.off_a[i];
}

int stt_count_a(void)
{
    return stt.count_a;
}

void stt_set_b(int count, unsigned short *base, int *off)
{
    stt.count_b = count;
    stt.base_b = base;
    stt.off_b = off;
}

unsigned short *stt_get_b(int i)
{
    return stt.base_b + stt.off_b[i];
}

int stt_count_b(void)
{
    return stt.count_b;
}

void stt_set_branch(unsigned short *tab, int x)
{
    stt.branch = tab;
    stt.branch_x = x;
}

int stt_branch_x(void)
{
    return stt.branch_x;
}

/* result of the first branch record of id whose date, character level and flag conditions hold */
unsigned short stt_branch_find(unsigned int id)
{
    unsigned short *tab = stt.branch;
    ScalDate *d = scal_date;
    int i = 0;
    unsigned short *e = tab;
    int id0, mfrom, dfrom, mto, dto;
    int on;

    for (;; e += 13, i++) {
        id0 = e[0];
        mfrom = e[2];
        dfrom = e[3];
        mto = e[4];
        dto = e[5];
        if (id0 == 0 || id != id0)
            continue;
        if (mfrom != 0 && d->month < mfrom)
            continue;
        if (dfrom != 0 && d->day < dfrom)
            continue;
        if (mto != 0 && d->month > mto)
            continue;
        if (dto != 0 && d->day > dto)
            continue;
        if (e[6] != 0 && e[7] != st_chr_level(e[6]))
            continue;
        if (e[8] != 0) {
            on = story_flag_get(e[8]) != 0;
            if (on) {
                if (e[9] == 4)
                    continue;
            } else if (e[9] == 3) {
                continue;
            }
        }
        if (e[10] != 0) {
            on = st_chr_flag_get(e[10], e[11]) != 0;
            if (on) {
                if (e[12] == 4)
                    continue;
            } else if (e[12] == 3) {
                continue;
            }
        }
        return tab[i * 13 + 1];
    }
}

void scal_get_ptrs(int **out)
{
    ScalDate *d = scal_date;

    out[0] = &d->cal_month;
    out[1] = &d->day;
    out[2] = &d->wday;
    out[3] = &d->hour;
    out[4] = &d->minute;
    out[5] = &d->days_left;
}

int sdate_weekday(void)
{
    return scal_daynum_of(scal_date);
}

#ifdef NON_MATCHING /* ordem dos operandos e agendamento das somas *100 diferem */
/* today between month m0 day d0 and month m1 day d1 (all 0: always) */
int story_date_in_range(int m0, int d0, int m1, int d1)
{
    int month = scal_get_month();
    int day = scal_get_day();
    int r;

    if (m0 == 0 && d0 == 0 && m1 == 0 && d1 == 0) {
        r = 1;
    } else {
        day = month * 100 + day;
        if (day < m0 * 100 + d0)
            r = 0;
        else if (m1 * 100 + d1 < day)
            r = 0;
        else
            r = 1;
    }
    return r;
}
#endif

/* 1 (and remembers it) when the day changed since the last call */
int scal_day_changed(void)
{
    int *st = scal_state;
    int n = scal_daynum_of(scal_date);

    if (*st != n) {
        *st = n;
        return 1;
    }
    return 0;
}

int scal_get_wday(void)
{
    return scal_date->wday;
}

/* tick and derived fields from month/day/slot */
void scal_update(ScalDate *d)
{
    int slot = d->slot;

    d->tick = slot + (scal_daynum_of(d) - 1) * 73;
    scal_calc_cal_month(d);
    scal_calc_hour(d);
    scal_calc_minute(d);
    scal_calc_wday(d);
    scal_calc_days_left(d);
}

/* month/day/slot and derived fields from the tick */
void scal_from_tick(ScalDate *d)
{
    int tick = d->tick;
    int slot = (tick - 1) % 73;
    int month = 1;
    int days = tick - (slot + 1);
    int len = 0;

    if (days > 0)
        days /= 73;
    for (;;) {
        switch (month) {
        case 1: len = 31; break;
        case 2: len = 30; break;
        case 3: len = 31; break;
        case 4: len = 31; break;
        case 5: len = 30; break;
        case 6: len = 31; break;
        case 7: len = 30; break;
        case 8: len = 31; break;
        }
        if (days < len)
            break;
        days -= len;
        month++;
    }
    d->month = month;
    d->day = days + 1;
    d->slot = slot + 1;
    scal_calc_cal_month(d);
    scal_calc_hour(d);
    scal_calc_minute(d);
    scal_calc_wday(d);
    scal_calc_days_left(d);
}

void scal_refresh(void)
{
    scal_from_tick(scal_date);
}

void scal_calc_cal_month(ScalDate *d)
{
    int m;

    switch (d->month) {
    case 1: m = 5; break;
    case 2: m = 6; break;
    case 3: m = 7; break;
    case 4: m = 8; break;
    case 5: m = 9; break;
    case 6: m = 10; break;
    case 7: m = 11; break;
    case 8: m = 12; break;
    }
    d->cal_month = m;
}

void scal_calc_month(ScalDate *d)
{
    int m;

    switch (d->cal_month) {
    case 5: m = 1; break;
    case 6: m = 2; break;
    case 7: m = 3; break;
    case 8: m = 4; break;
    case 9: m = 5; break;
    case 10: m = 6; break;
    case 11: m = 7; break;
    case 12: m = 8; break;
    }
    d->month = m;
}

void scal_calc_hour(ScalDate *d)
{
    int min = (d->slot - 1) * 15 + 345;

    if (min != 0)
        min = min / 60;
    d->hour = min;
}

void scal_calc_minute(ScalDate *d)
{
    int m;

    switch ((d->slot - 1) % 4) {
    case 0: m = 45; break;
    case 1: m = 0; break;
    case 2: m = 15; break;
    case 3: m = 30; break;
    }
    d->minute = m;
}

void scal_calc_wday(ScalDate *d)
{
    int w;

    switch ((scal_daynum_of(d) - 1) % 7) {
    case 0: w = 1; break;
    case 1: w = 2; break;
    case 2: w = 3; break;
    case 3: w = 4; break;
    case 4: w = 5; break;
    case 5: w = 6; break;
    case 6: w = 0; break;
    default: w = 1; break;
    }
    d->wday = w;
}

void scal_calc_days_left(ScalDate *d)
{
    int n = scal_daynum_of(d);

    d->days_left = 92 - (n - 1);
    if (d->days_left < 0)
        d->days_left = 0;
}

#ifdef NON_MATCHING /* o original sai do laço por blocos `b fim` separados; nenhuma forma do laço os reproduz */
int scal_period_a(ScalDate *d)
{
    int i = 0;
    ScalPeriod *p = scal_periods;

    while (d->month != p->month || (d->day != p->day && p->day != 0)) {
        p++;
        i++;
    }
    return scal_periods[i].a;
}
#endif

#ifdef NON_MATCHING /* mesmo laço de scal_period_a */
int scal_period_b(ScalDate *d)
{
    int i = 0;
    ScalPeriod *p = scal_periods;

    while (d->month != p->month || (d->day != p->day && p->day != 0)) {
        p++;
        i++;
    }
    return scal_periods[i].b;
}
#endif

/* day number (1 = month 1 day 1) of a date */
int scal_daynum_of(ScalDate *d)
{
    int month = d->month;
    int day = d->day;
    int days = 0;
    int m;
    int i = 0;

    for (;;) {
        switch (i) {
        case 0: m = 1; break;
        case 1: m = 2; days += 31; break;
        case 2: m = 3; days += 30; break;
        case 3: m = 4; days += 31; break;
        case 4: m = 5; days += 31; break;
        case 5: m = 6; days += 30; break;
        case 6: m = 7; days += 31; break;
        case 7: m = 8; days += 30; break;
        }
        if (month == m)
            break;
        i++;
    }
    return days + day;
}

int scal_daynum_cal(ScalDate *d)
{
    int month = d->cal_month;
    int day = d->day;
    int days = 0;
    int m;
    int i = 0;

    for (;;) {
        switch (i) {
        case 0: m = 5; break;
        case 1: m = 6; days += 31; break;
        case 2: m = 7; days += 30; break;
        case 3: m = 8; days += 31; break;
        case 4: m = 9; days += 31; break;
        case 5: m = 10; days += 30; break;
        case 6: m = 11; days += 31; break;
        case 7: m = 12; days += 30; break;
        }
        if (month == m)
            break;
        i++;
    }
    return days + day;
}
