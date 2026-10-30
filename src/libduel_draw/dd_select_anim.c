// @range 0x0886AE68-0x0886B25C
/* Short animation of a field selection: switches the draw mode to 10, waits half a step (its
   length follows the duel speed setting), marks the zone, waits again and calls back with the
   chosen side/zone. Also gives the progress of the animation to the field drawing. */
#include "dd_duelui.h"

typedef struct SelAnim {               /* 0x08C5B9C4 */
    int state;                         /* 0 idle, 1 first half, 2 second half */
    int prev_mode;                     /* 0x04 */
    int t;                             /* 0x08 frames in the current half */
    int total;                         /* 0x0C frames since the start */
    int side;                          /* 0x10 */
    int a;                             /* 0x14 */
    int zone;                          /* 0x18 */
    void (*done)(int player, int a, int zone); /* 0x1C */
} SelAnim;

extern SelAnim sel_anim;

int dd_mode_get(void);
void dd_mode_push(int mode);
void dd_mode_set_flag(int mode);
void dd_mode_pop(void);
void dd_cobjs_map(int side, int zone);
void dd_zone_rebuild(int side, int on);
void req_flag_set(int i);
unsigned int cfg_get_bits_1_3_4(void);
int dv_side_to_player(int side);

#define SEL_HALF() ((int)(30.0f / ((float)(int)cfg_get_bits_1_3_4() / 2.0f + 1.0f)) / 2)

int sel_start(int side, int a, int zone, void (*done)(int player, int a, int zone))
{
    sel_anim.side = side;
    sel_anim.a = a;
    sel_anim.zone = zone;
    sel_anim.done = done;
    sel_anim.prev_mode = dd_mode_get();
    dd_mode_push(10);
    dd_mode_set_flag(0);
    req_flag_set(0);
    return 1;
}

void sel_begin(void)
{
    sel_anim.state = 1;
    sel_anim.t = 0;
    sel_anim.total = 0;
}

void sel_end(void)
{
    sel_anim.state = 0;
}

void *sel_draw(void *dl)
{
    return dl;
}

void sel_update(void)
{
    SelAnim *s = &sel_anim;
    int t;

    req_flag_set(0);
    t = sel_anim.t + 1;
    sel_anim.t = t;
    sel_anim.total++;
    switch (sel_anim.state) {
    case 1:
        if (t >= SEL_HALF()) {
            s->state = 2;
            s->t = 0;
            dd_cobjs_map(s->side, s->zone);
            dd_zone_rebuild(s->side, 1);
        }
        break;
    case 2:
        if (t >= SEL_HALF()) {
            dd_mode_pop();
            s->done(dv_side_to_player(s->side), s->a, s->zone);
        }
        break;
    }
}

int sel_active(void)
{
    return sel_anim.state != 0;
}

/* 0..1 while the zone of side is animated (rising then falling) */
float sel_progress(int side)
{
    SelAnim *s = &sel_anim;

    if (side != sel_anim.side)
        return 0.0f;
    switch (s->state) {
    case 1:
        return (float)s->t / (float)SEL_HALF();
    case 2:
        return 1.0f - (float)s->t / (float)SEL_HALF();
    }
    return 0.0f;
}

float sel_progress_total(int side)
{
    SelAnim *s = &sel_anim;

    if (side != sel_anim.side)
        return 0.0f;
    switch (s->state) {
    case 1:
    case 2:
        return (float)s->total / (float)(int)(30.0f / ((float)(int)cfg_get_bits_1_3_4() / 2.0f + 1.0f));
    }
    return 0.0f;
}
