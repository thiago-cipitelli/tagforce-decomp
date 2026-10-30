// @range 0x08833340-0x088338C8
// @asm 0x08833448-0x088338C8
// @data 0x08876AE0-0x08876AF0
/* Counter icon over a card on the field: a 38x30 icon (or a fixed one) dropping in, then a number
   rolling from one value to the next, held for 16 frames and fading out. */
#define DD_FX
#include "dd_d3.h"

typedef struct DdVec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) DdVec4;

typedef struct DdFxTally {             /* 0x54 */
    int icon;                          /* +0x00 */
    int hold;                          /* +0x04 0..16 */
    int in;                            /* +0x08 0..8 */
    int out;                           /* +0x0C 0..8 */
    int wait;                          /* +0x10 0..4 */
    int from;                          /* +0x14 */
    int to;                            /* +0x18 */
    int roll;                          /* +0x1C 0..4 */
    float pos[3];                      /* +0x20 */
    int fixed;                         /* +0x2C use icon slot 2 */
    unsigned int se;                   /* +0x30 */
    int num_x, num_y;                  /* +0x34 */
    int w, h, u, v, x, y;              /* +0x3C sprite */
} DdFxTally;

float *mat4_transform_vec4(DdVec4 *out, float *m, DdVec4 *v);
void dd_mtx_mvp_cur(float *out);
void dd_roll_number(float t, int x, int y, int from, int to);
float dd_ease_sin_inv(int t, int n, int k);
float dd_ease_cos_inv(int t, int n, int k);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);

int dd_fx_tally_update(int state, DdFxTally *fx);

int dd_fx_tally_start(int icon, float *pos, int from, int to, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_tally_update, term, arg, sizeof(DdFxTally));
    DdFxTally *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->icon = icon;
        fx->from = from;
        fx->to = to;
        vec3_copy(fx->pos, pos);
        fx->fixed = 0;
    }
    return id;
}

int dd_fx_tally_start_fixed(float *pos, int from, int to, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_tally_start(0, pos, from, to, term, arg);

    if (id != -1) {
        DdFxTally *fx = dd_task_mem(id);
        int one = 1;

        fx->fixed = one;
    }
    return id;
}

/* template of the local vector of dd_fx_tally_update (kept in asm) */
DdVec4 dd_fx_tally_v4 = { 0.0f, 0.0f, 0.0f, 1.0f };

#ifdef NON_MATCHING /* alocação de registradores (lista e deslocamento trocados entre s2 e s3) */
int dd_fx_tally_update(int state, DdFxTally *fx)
{
    int step = dd_anim_step();
    float in = dd_ease_lin(fx->in, 8);
    float out = dd_ease_lin_inv(fx->out, 8);
    DdVec4 v = { 0.0f, 0.0f, 0.0f, 1.0f };
    float mvp[16];
    unsigned int *list;
    int dy;
    int x, y;

    v.x = fx->pos[0];
    v.y = fx->pos[1];
    v.z = fx->pos[2];
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_mtx_mvp_cur(mvp);
    mtx_pop();
    mat4_transform_vec4(&v, mvp, &v);
    x = v.x * 480.0f / v.w + 2048.0f + 74.0f - 1808.0f;
    y = v.y * -272.0f / v.w + 2048.0f + -4.0f - 1912.0f;
    list = dl_select_layer(4, 0);
    list = dd_depth_enable(list, 0, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dd_tex_pack(list, 24);
    list = dl_color(list, ((unsigned int)(in * 255.0f * out) << 24) | 0xFFFFFF);
    dl_set_list(list);
    if (state == 1) {
        int sx = x - 19;
        int sy = y - 15;

        fx->w = 38;
        fx->h = 30;
        if (fx->fixed == 0)
            fx->u = fx->icon * 38;
        else
            fx->u = 76;
        fx->v = 22;
        fx->x = sx;
        fx->y = sy;
        fx->num_x = 19;
        fx->num_y = -7;
    }
    list = dl_select_layer(4, 0);
    if (fx->from == fx->to)
        dy = 0;
    else if (fx->from < fx->to)
        dy = dd_ease_sin_inv(fx->in, 8, 1) * -64.0f;
    else
        dy = dd_ease_cos_inv(fx->out, 8, 1) * -64.0f;
    list = draw_set_offsets(list, fx->w, fx->h, fx->w, fx->h);
    list = draw_sprite(list, fx->x, fx->y + dy, 0, fx->u, fx->v, 0, 0);
    dl_set_list(list);
    dd_roll_number((float)fx->roll / 4.0f, x + fx->num_x, y + fx->num_y, fx->from, fx->to);
    if (fx->in < 8) {
        fx->in = dd_step_clamp(fx->in, step, 8);
    } else if (fx->wait < 4) {
        fx->wait = dd_step_clamp(fx->wait, step, 4);
    } else {
        dd_cue_se(0x6245, fx->roll, 0, &fx->se, 1);
        if (fx->roll < 4)
            fx->roll = dd_step_clamp(fx->roll, step, 4);
        else if (fx->hold < 16)
            fx->hold = dd_step_clamp(fx->hold, step, 16);
        else if (fx->out < 8)
            fx->out = dd_step_clamp(fx->out, step, 8);
    }
    if (fx->out < 8)
        return 2;
    return 4;
}
#endif
