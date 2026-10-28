// @range 0x088171F4-0x08817500
/* The morning plan: a list of dates (with an optional story flag) giving the script to run that
   morning in the ESM script table, the two message packs of the morning and two words of state. */
#include "libstory.h"

typedef struct StPlanDay {             /* 8 bytes, ended by month 0 and day 0 */
    unsigned short month;
    unsigned short day;
    unsigned short flag;               /* story flag that must be set (0: none) */
    unsigned short script;             /* id in the ESM script table (0xFFFF: 0) */
} StPlanDay;

typedef struct StMorning {             /* 0x08ACBAD4, 0x10 */
    StPlanDay *plan;                   /* +0x0 */
    StoryCmd *esm;                     /* +0x4 scripts, each starting with its id */
    void *text_a;                      /* +0x8 */
    void *text_b;                      /* +0xC */
} StMorning;

extern int *st_ev2;                    /* 0x08ACBAD0 2 words */
extern StMorning st_morning;

int scal_get_month(void);
int scal_get_day(void);
int story_flag_get(unsigned int id);
int scal_get_wday(void);
int scal_get_cal_month(void);

void st_init_8171f4(void)
{
    memset(&st_morning, 0, sizeof(StMorning));
}

void st_term_817208(void)
{
    memset(&st_morning, 0, sizeof(StMorning));
}

void st_ev2_attach(void *p)
{
    st_ev2 = p;
}

void st_ev2_clear(void)
{
    memset(st_ev2, 0, 8);
}

void st_morning_set_plan(void *p)
{
    st_morning.plan = p;
}

void st_morning_set_esm(void *p)
{
    st_morning.esm = p;
}

void st_morning_set_text(void *a, void *b)
{
    st_morning.text_a = a;
    st_morning.text_b = b;
}

void st_ev2_set_0(int v)
{
    st_ev2[0] = v;
}

int st_ev2_get_0(void)
{
    return st_ev2[0];
}

void *st_morning_text_b(void)
{
    return st_morning.text_b;
}

void *st_morning_text_a(void)
{
    return st_morning.text_a;
}

/* there is a morning script for today */
int st_ev2_today(void)
{
    StMorning *s = &st_morning;
    int month = scal_get_month();
    int day = scal_get_day();
    int i;

    for (i = 0;; i++) {
        StPlanDay *d = s->plan + i;

        if (d->month == 0 && d->day == 0)
            return 0;
        if (d->month == month && d->day == day) {
            if (d->flag == 0)
                return 1;
            if (story_flag_get(d->flag))
                return 1;
        }
    }
}

void st_ev2_set_4(int v)
{
    st_ev2[1] = v;
}

int st_ev2_get_4(void)
{
    return st_ev2[1];
}

/* the morning script of today, or the one of the last date of the plan */
StoryCmd *st_ev_lesson_script(void)
{
    StMorning *s = &st_morning;
    int month = scal_get_month();
    int day = scal_get_day();
    int n;
    int i;
    int id;

    for (n = 0, i = 0;; i++, n++) {
        StPlanDay *d = s->plan + i;

        if (d->month == 0 && d->day == 0) {
            n--;
            d = s->plan + n;
            id = d->script;
            break;
        }
        if (d->month == month && d->day == day) {
            if (d->flag == 0) {
                id = d->script;
                break;
            }
            if (story_flag_get(d->flag)) {
                id = d->script;
                break;
            }
        }
    }
    if (id == -1)
        id = 0;
    for (i = 0;; i++) {
        StoryCmd *p = s->esm + i;

        if (p->op == id)
            return &s->esm[i];
    }
}

StoryCmd *st_ev_script_174ac(int id)
{
    StMorning *s = &st_morning;
    int i;

    for (i = 0;; i++) {
        StoryCmd *p = s->esm + i;

        if (p->op == id)
            return &s->esm[i];
    }
}

int st_morning_wday(void)
{
    return scal_get_wday();
}

int st_morning_month(void)
{
    return scal_get_cal_month();
}
