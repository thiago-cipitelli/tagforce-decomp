// @range 0x08832B54-0x08833340
// @asm 0x08832C28-0x08833340
// @data 0x08876AD0-0x08876AE0
/* Counter balloon over a card on the field: a window sprite projected from the 3D position with
   a number that rolls digit by digit from one value to another, a little hand turning before
   each step; fades out 16 frames after reaching the value. */
#define DD_FX
#include "dd_d3.h"

typedef struct DdWVert {               /* 16-bit texture and position, through mode */
    short u, v;
    short x, y, z;
} DdWVert;

typedef struct DdFxCounter {           /* 0x100 */
    int frame;                         /* +0x00 frames after reaching the value */
    int in;                            /* +0x04 0..8 */
    int stage;                         /* +0x08 */
    int tick;                          /* +0x0C picks the vertex copy */
    int from;                          /* +0x10 */
    int to;                            /* +0x14 */
    int cur;                           /* +0x18 */
    int dir;                           /* +0x1C -1 or 1 */
    int roll;                          /* +0x20 frames of the digit roll (0..4) */
    int spin;                          /* +0x24 frames of the hand turn (0..16) */
    float pos[3];                      /* +0x28 */
    int kind;                          /* +0x34 0 or 1 (other window) */
    unsigned int se;                   /* +0x38 */
    int num_x, num_y;                  /* +0x3C */
    DdWVert bg[2];                     /* +0x44 */
    float hand[4][3];                  /* +0x58 */
    DdWVert hand_v[4];                 /* +0x88 */
    DdWVert hand_copy[2][4];           /* +0xB0 */
} DdFxCounter;

float *mat3_rot_z(float *out, float *m, float a);
float *mat4_transform_vec4(FxVec4 *out, float *m, FxVec4 *v);
void *memcpy(void *, const void *, size_t);
void dd_mtx_mvp_cur(float *out);
void dd_roll_number(float t, int x, int y, int from, int to);

int dd_fx_counter_update(int state, DdFxCounter *fx);

int dd_fx_counter_start(float *pos, int from, int to, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_counter_update, term, arg, sizeof(DdFxCounter));
    DdFxCounter *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->from = from;
        fx->to = to;
        vec3_copy(fx->pos, pos);
        fx->kind = 0;
    }
    return id;
}

int dd_fx_counter_start2(float *pos, int from, int to, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_counter_start(pos, from, to, term, arg);

    if (id != -1) {
        DdFxCounter *fx = dd_task_mem(id);
        int one = 1;

        fx->kind = one;
    }
    return id;
}

/* template of the local vector of dd_fx_counter_update (kept in asm) */
FxVec4 dd_fx_counter_v4 = { 0.0f, 0.0f, 0.0f, 1.0f };

#ifdef NON_MATCHING /* função grande (1,8 KB) */
int dd_fx_counter_update(int state, DdFxCounter *fx)
{
    int buf = fx->tick % 2;
    int step = dd_anim_step();
    FxVec4 v = { 0.0f, 0.0f, 0.0f, 1.0f };
    float mvp[16], m3[9], p[3];
    unsigned int *list;
    float in, out, t;
    int x, y, a, b, k, ret;

    if (state == 1) {
        fx->cur = fx->from;
        if (fx->to < fx->from)
            fx->dir = -1;
        else
            fx->dir = 1;
    }
    in = dd_ease_lin(fx->in, 8);
    out = 1.0f - dd_ease_lin(fx->frame - 16, 8);
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
    y = v.y * -272.0f / v.w + 2048.0f + -4.0f - 1912.0f + -8.0f;

    /* the window */
    list = dl_select_layer(4, 0);
    list = dd_depth_enable(list, 0, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dd_tex_pack(list, 24);
    list = dl_color(list, ((unsigned int)(in * 255.0f * out) << 24) | 0xFFFFFF);
    dl_set_list(list);
    if (state == 1) {
        int u0 = 0, u1 = 45;

        if (fx->kind == 1) {
            u0 = 189;
            u1 = 234;
        }
        fx->bg[0].u = u0;
        fx->bg[0].v = 70;
        fx->bg[0].x = x - 22;
        fx->bg[0].y = y - 29;
        fx->bg[0].z = 0;
        fx->bg[1].u = u1;
        fx->bg[1].v = 128;
        fx->bg[1].x = x + 23;
        fx->bg[1].y = y + 29;
        fx->bg[1].z = 0;
        fx->num_x = (u1 - u0) / 2;
        fx->num_y = 7;
    }
    list = dl_select_layer(4, 0);
    list = draw_array(list, 6, 0x800102, 2, 0, fx->bg);
    dl_set_list(list);

    /* the hand */
    if (state == 1) {
        vec3_set(fx->hand[0], -1.5f, -15.0f, 0.0f);
        vec3_set(fx->hand[1], -1.5f, 2.0f, 0.0f);
        vec3_set(fx->hand[2], 1.5f, -15.0f, 0.0f);
        vec3_set(fx->hand[3], 1.5f, 2.0f, 0.0f);
        for (k = 0; k < 4; k++) {
            fx->hand_v[k].u = k < 2 ? 0 : 3;
            fx->hand_v[k].v = k % 2 ? 70 : 53;
            fx->hand_v[k].x = 0;
            fx->hand_v[k].y = 0;
            fx->hand_v[k].z = 0;
        }
    }
    if (fx->from < fx->to)
        k = fx->spin;
    else
        k = 16 - fx->spin;
    mat3_identity(m3);
    mat3_rot_z(m3, m3, (float)k * 6.2831855f / 16.0f);
    for (k = 0; k < 4; k++) {
        mat3_transform_vec3(p, m3, fx->hand[k]);
        fx->hand_v[k].x = p[0] + (float)x + 1.0f;
        fx->hand_v[k].y = p[1] + (float)y + 11.0f;
    }
    memcpy(fx->hand_copy[buf], fx->hand_v, sizeof(fx->hand_v));
    list = dl_select_layer(4, 0);
    list = draw_array(list, 4, 0x800102, 4, 0, fx->hand_copy[buf]);
    dl_set_list(list);
    fx->tick++;

    /* the number */
    ret = 2;
    if (fx->in < 8) {
        fx->in += step;
        if (fx->in > 8)
            fx->in = 8;
        t = 0.0f;
        a = b = fx->from;
    } else if (fx->cur == fx->to) {
        fx->frame += step;
        t = 0.0f;
        a = b = fx->to;
        if (fx->frame > 23)
            ret = 4;
    } else {
        a = b = fx->cur;
        if (fx->stage == 0) {
            fx->spin += step;
            t = 0.0f;
            if (fx->spin > 15) {
                fx->spin = 0;
                fx->stage += step;
            }
        } else if (fx->roll == 0) {
            fx->stage += step;
            t = 0.0f;
            if (fx->stage > 3)
                fx->roll += step;
        } else {
            b = fx->cur + fx->dir;
            t = (float)fx->roll / 4.0f;
            fx->roll += step;
            dd_cue_se(0x6245, fx->roll, 0, &fx->se, 1);
            if (fx->roll >= 4) {
                fx->stage = 0;
                fx->roll = 0;
                fx->cur += fx->dir;
                fx->se &= ~1;
            }
        }
    }
    dd_roll_number(t, x + fx->num_x, y + fx->num_y, a, b);
    return ret;
}
#endif
