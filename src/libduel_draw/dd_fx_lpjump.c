// @range 0x08834418-0x08834AA0
// @asm 0x08834628-0x08834AA0
/* Damage/gain number popping on screen: the digits of a value (and a sign) jump one after the
   other, optionally sliding to another position, in a given colour; then fade out. */
#define DD_FX
#include "dd_d3.h"

typedef struct DdScrPos {
    int x, y, z;
} DdScrPos;

typedef struct DdFxLpJump {            /* 0x50 */
    int phase;                         /* +0x00 0 jumping, then 1..12 */
    int slide;                         /* +0x04 0..10 */
    int in;                            /* +0x08 0..4 */
    int out;                           /* +0x0C 0..4 */
    int tick;                          /* +0x10 */
    int value;                         /* +0x14 sign gives the sound and the sign sprite */
    DdScrPos pos;                      /* +0x18 */
    unsigned int r, g, b, a;           /* +0x24 colour */
    int moving;                        /* +0x34 */
    DdScrPos to;                       /* +0x38 */
    unsigned int se;                   /* +0x44 */
    int ox, oy;                        /* +0x48 centring of the digits */
} DdFxLpJump;

int int_abs(int x);
void dd_roll_digit(float t, int layer, int x, int y, int digit, int down);

int dd_fx_lpjump_update(int state, DdFxLpJump *fx);

int dd_fx_lpjump_start(DdScrPos *pos, int value, unsigned int r, unsigned int g, unsigned int b, unsigned int a, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_lpjump_update, term, arg, sizeof(DdFxLpJump));
    DdFxLpJump *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->value = value;
        fx->pos = *pos;
        fx->r = r;
        fx->g = g;
        fx->b = b;
        fx->a = a;
        fx->moving = 0;
    }
    return id;
}

int dd_fx_lpjump_start_move(DdScrPos *pos, int value, unsigned int r, unsigned int g, unsigned int b, unsigned int a, DdScrPos *to, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_lpjump_start(pos, value, r, g, b, a, term, arg);

    if (id != -1) {
        DdFxLpJump *fx = dd_task_mem(id);
        int one = 1;

        fx->moving = one;
        fx->to = *to;
    }
    return id;
}

float dd_fx_lpjump_digit(float ang, int digit, int i, int x, int y)
{
    float a = ang - (float)i * 1.0471976f;

    if (a < 0.0f)
        a = 0.0f;
    else if (a > 3.1415927f)
        a = 3.1415927f;
    dd_roll_digit(1.0f, 6, x, y + (int)(-11.0f * sinf(a)), digit, 1);
    return a;
}

#ifdef NON_MATCHING /* função grande */
int dd_fx_lpjump_update(int state, DdFxLpJump *fx)
{
    int step = dd_anim_step();
    float in = dd_ease_lin(fx->in, 4);
    float out = dd_ease_lin_inv(fx->out, 4);
    unsigned int *list;
    float ang, last, k;
    int v, w, x, y, i;

    list = dl_select_layer(6, 0);
    list = dd_depth_enable(list, 0, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_tex_pack(list, 24);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dl_color(list, fx->r | fx->g << 8 | (unsigned int)(out * (float)fx->a * in) << 24 | fx->b << 16);
    dl_set_list(list);
    dd_cue_se(fx->value > 0 ? 0x6245 : 0x6244, fx->phase, 0, &fx->se, 1);
    if (state == 1) {
        v = int_abs(fx->value);
        w = v == 0 ? 0 : 19;
        while (v >= 10) {
            v /= 10;
            w += 19;
        }
        w += 19;
        fx->ox = w / 2 - 19;
        fx->oy = -11;
    }
    v = int_abs(fx->value);
    ang = (((float)(step - 1) * 10.0f + 30.0f) * 3.1415927f / 180.0f) * (float)fx->tick;
    if (fx->moving == 0) {
        x = fx->ox + fx->pos.x;
        y = fx->oy + fx->pos.y;
    } else {
        k = dd_ease_cos_inv(fx->slide, 10, 1);
        x = fx->ox + (int)((float)fx->pos.x + k * (float)(fx->to.x - fx->pos.x));
        y = fx->oy + (int)((float)fx->pos.y + k * (float)(fx->to.y - fx->pos.y));
    }
    i = 0;
    do {
        last = dd_fx_lpjump_digit(ang, v % 10, i, x, y);
        x -= 19;
        v /= 10;
        i++;
    } while (v != 0);
    if (fx->value != 0)
        last = dd_fx_lpjump_digit(ang, fx->value > 0 ? 11 : 12, i, x, y);
    if (last == 3.1415927f && fx->phase == 0)
        fx->phase = 1;
    fx->tick++;
    if (fx->in < 4) {
        fx->in = dd_step_clamp(fx->in, step, 4);
    } else if (fx->phase >= 1) {
        if (fx->phase < 12) {
            fx->phase = dd_step_clamp(fx->phase, step, 12);
        } else {
            if (fx->slide < 10)
                fx->slide = dd_step_clamp(fx->slide, step, 10);
            if (fx->slide > 6 && fx->out < 4)
                fx->out = dd_step_clamp(fx->out, step, 4);
        }
    }
    if (fx->out < 4)
        return 2;
    return 4;
}
#endif
