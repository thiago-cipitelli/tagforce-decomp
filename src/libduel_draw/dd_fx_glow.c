// @range 0x08831788-0x08831C58
/* Effect on a card shown on screen: a white glow growing behind it (60 frames) with grey rings
   spreading out during the first 32 frames; fades out in 8 frames once the task gets flag 1. */
#define DD_FX
#include "dd_d3.h"

typedef struct DdScrPos {
    int x, y, z;
} DdScrPos;

typedef struct DdFxGlow {              /* 0x20 */
    int id;                            /* +0x00 own task handle */
    int in;                            /* +0x04 */
    int out;                           /* +0x08 */
    DdScrPos pos;                      /* +0x0C */
    unsigned int se;                   /* +0x18 */
    int ring;                          /* +0x1C frames of the rings */
} DdFxGlow;

unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);

int dd_fx_glow_update(int state, DdFxGlow *fx);

int dd_fx_glow_done(int id)
{
    DdFxGlow *fx = dd_task_mem(id);

    if (fx == 0)
        return 1;
    return fx->in >= 60;
}

int dd_fx_glow_start(DdScrPos *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_glow_update, term, arg, sizeof(DdFxGlow));
    DdFxGlow *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->id = id;
        fx->pos = *pos;
    }
    return id;
}

int dd_fx_glow_update(int state, DdFxGlow *fx)
{
    int step = dd_anim_step();
    float in = dd_ease_sin(fx->in, 8, 1);
    float out = dd_ease_cos(fx->out, 8, 1);
    unsigned int *list;
    float a, base;
    int i, w;

    dd_cue_se(0x6229, fx->in, 0, &fx->se, 1);
    list = dl_select_layer(6, 0);
    list = dd_depth_enable(list, 0, 1);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 33);
    base = in * 0.5f + 0.5f;
    w = 128.0f * base;
    list = dl_color(list, ((unsigned int)(in * 255.0f * out) << 24) | 0xFFFFFF);
    list = draw_set_offsets(list, 128, 128, w, w);
    list = draw_sprite(list, fx->pos.x - w / 2, fx->pos.y - w / 2, 0, 0, 0, 0, 0);
    dl_set_list(list);
    if (fx->ring < 32) {
        list = dl_select_layer(6, 0);
        list = dd_depth_enable(list, 0, 1);
        list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
        list = dd_tex_pack(list, 33);
        in *= dd_ease_sin(fx->ring, 16, 1);
        a = 90.0f * (in * dd_ease_cos_out(fx->ring, 16, 32, 1));
        for (i = 0; i < 16; i++) {
            unsigned int c = a * dd_ease_lin_inv(i, 15);

            w = 128.0f * (base + 0.25f * dd_ease_lin(i + 1, 16));
            list = dl_color(list, (c << 24) | 0x646464);
            list = draw_set_offsets(list, 128, 128, w, w);
            list = draw_sprite(list, fx->pos.x - w / 2, fx->pos.y - w / 2, 0, 0, 0, 0, 0);
        }
        fx->ring += step;
        if (fx->ring > 32)
            fx->ring = 32;
        dl_set_list(list);
    }
    if (!dd_task_flag_test(fx->id, 1)) {
        fx->in += step;
        if (fx->in > 60)
            fx->in = 60;
    } else {
        fx->in = 60;
        fx->ring = 32;
        fx->out += step;
        if (fx->out > 8)
            fx->out = 8;
    }
    if (fx->out < 8)
        return 2;
    return 4;
}
