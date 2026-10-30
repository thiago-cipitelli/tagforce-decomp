// @range 0x088338C8-0x08834418
// @asm 0x08833950-0x08833E80
// @asm 0x08833F08-0x08834418
// @data 0x08876AF0-0x08876B10
/* Two more counter icons over a card on the field with a rolling number: one that throbs (or
   flips through three frames when the value drops) and one that blinks a glow every other
   frame; held for 16 frames and faded out. */
#define DD_FX
#include "dd_d3.h"

typedef struct DdVec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) DdVec4;

typedef struct DdFxIcon {              /* 0x50 */
    int hold;                          /* +0x00 0..16 */
    int in;                            /* +0x04 0..8 */
    int out;                           /* +0x08 0..8 */
    int anim;                          /* +0x0C throb frame (blink: wait 0..4) */
    int anim_len;                      /* +0x10 throb length (blink: frame counter) */
    int from;                          /* +0x14 */
    int to;                            /* +0x18 */
    int roll;                          /* +0x1C 0..4 */
    float pos[3];                      /* +0x20 */
    unsigned int se;                   /* +0x2C */
    int num_x, num_y;                  /* +0x30 */
    int w, h, u, v, x, y;              /* +0x38 sprite */
} DdFxIcon;

float *mat4_transform_vec4(DdVec4 *out, float *m, DdVec4 *v);
void dd_mtx_mvp_cur(float *out);
void dd_roll_number(float t, int x, int y, int from, int to);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);

int dd_fx_throb_update(int state, DdFxIcon *fx);
int dd_fx_blink_update(int state, DdFxIcon *fx);

int dd_fx_throb_start(float *pos, int from, int to, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_throb_update, term, arg, sizeof(DdFxIcon));
    DdFxIcon *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->from = from;
        fx->to = to;
        vec3_copy(fx->pos, pos);
    }
    return id;
}

/* templates of the local vectors of the two updates (kept in asm) */
DdVec4 dd_fx_throb_v4 = { 0.0f, 0.0f, 0.0f, 1.0f };

#ifdef NON_MATCHING /* função grande; mesma estrutura de dd_fx_tally_update */
static void dd_fx_icon_project(DdFxIcon *fx, int *x, int *y)
{
    DdVec4 v = { 0.0f, 0.0f, 0.0f, 1.0f };
    float mvp[16];

    v.x = fx->pos[0];
    v.y = fx->pos[1];
    v.z = fx->pos[2];
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_mtx_mvp_cur(mvp);
    mtx_pop();
    mat4_transform_vec4(&v, mvp, &v);
    *x = v.x * 480.0f / v.w + 2048.0f + 74.0f - 1808.0f;
    *y = v.y * -272.0f / v.w + 2048.0f + -4.0f - 1912.0f;
}

static int dd_fx_icon_tail(DdFxIcon *fx, int step, int wait_len)
{
    if (fx->in < 8) {
        fx->in = dd_step_clamp(fx->in, step, 8);
    } else if (fx->anim < wait_len) {
        fx->anim = dd_step_clamp(fx->anim, step, wait_len);
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

int dd_fx_throb_update(int state, DdFxIcon *fx)
{
    int step = dd_anim_step();
    unsigned int *list;
    float in, out, k;
    int x, y, du;

    if (state == 1) {
        if (fx->from < fx->to)
            fx->anim_len = 8;
        else
            fx->anim_len = 4;
    }
    in = dd_ease_lin(fx->in, 8);
    out = dd_ease_lin_inv(fx->out, 8);
    dd_fx_icon_project(fx, &x, &y);
    list = dl_select_layer(4, 0);
    list = dd_depth_enable(list, 0, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dd_tex_pack(list, 24);
    list = dl_color(list, ((unsigned int)(in * 255.0f * out) << 24) | 0xFFFFFF);
    dl_set_list(list);
    if (state == 1) {
        fx->w = 38;
        fx->h = 30;
        fx->u = 114;
        fx->v = 22;
        fx->x = x - 19;
        fx->y = y - 15;
        fx->num_x = 19;
        fx->num_y = -7;
    }
    list = dl_select_layer(4, 0);
    if (fx->from == fx->to) {
        k = 1.0f;
        du = 0;
    } else if (fx->from < fx->to) {
        k = sinf((float)fx->anim * 3.1415927f / (float)fx->anim_len) + 1.0f;
        du = 0;
    } else {
        k = 1.0f;
        du = fx->out / 2;
        if (du >= 3)
            du = 2;
        du *= fx->w;
    }
    list = draw_set_offsets(list, fx->w, fx->h, (int)(fx->w * k), (int)(fx->h * k));
    list = draw_sprite(list, fx->x - (float)((int)(fx->w * k) - fx->w) / 2.0f,
                       fx->y - (float)((int)(fx->h * k) - fx->h) / 2.0f, 0, fx->u + du, fx->v, 0, 0);
    dl_set_list(list);
    dd_roll_number((float)fx->roll / 4.0f, x + fx->num_x, y + fx->num_y, fx->from, fx->to);
    return dd_fx_icon_tail(fx, step, fx->anim_len);
}
#endif

int dd_fx_blink_start(float *pos, int from, int to, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_blink_update, term, arg, sizeof(DdFxIcon));
    DdFxIcon *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->from = from;
        fx->to = to;
        vec3_copy(fx->pos, pos);
    }
    return id;
}

DdVec4 dd_fx_blink_v4 = { 0.0f, 0.0f, 0.0f, 1.0f };

#ifdef NON_MATCHING /* função grande; mesma estrutura de dd_fx_tally_update */
int dd_fx_blink_update(int state, DdFxIcon *fx)
{
    int step = dd_anim_step();
    unsigned int *list;
    float in, out, k;
    int x, y;

    in = dd_ease_lin(fx->in, 8);
    out = dd_ease_lin_inv(fx->out, 8);
    dd_fx_icon_project(fx, &x, &y);
    y += -8;
    list = dl_select_layer(4, 0);
    list = dd_depth_enable(list, 0, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_tex_pack(list, 24);
    list = dl_color(list, ((unsigned int)(in * 255.0f * out) << 24) | 0xFFFFFF);
    dl_set_list(list);
    if (state == 1) {
        fx->w = 45;
        fx->h = 58;
        fx->u = 99;
        fx->v = 70;
        fx->x = x - 22;
        fx->y = y - 29;
        fx->num_x = 22;
        fx->num_y = 7;
    }
    list = dl_select_layer(4, 0);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = draw_set_offsets(list, fx->w, fx->h, fx->w, fx->h);
    list = draw_sprite(list, fx->x, fx->y, 0, fx->u, fx->v, 0, 0);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    k = (float)(fx->anim_len % 2) * 0.20000005f + 1.0f;
    list = draw_set_offsets(list, fx->w, fx->h, (int)(fx->w * k), (int)(fx->h * k));
    list = draw_sprite(list, fx->x - (float)((int)(fx->w * k) - fx->w) / 2.0f,
                       fx->y - (float)((int)(fx->h * k) - fx->h) / 2.0f, 0, fx->u + fx->w, fx->v, 0, 0);
    dl_set_list(list);
    dd_roll_number((float)fx->roll / 4.0f, x + fx->num_x, y + fx->num_y, fx->from, fx->to);
    fx->anim_len++;
    return dd_fx_icon_tail(fx, step, 4);
}
#endif
