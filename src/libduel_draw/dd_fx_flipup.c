// @range 0x08831290-0x08831788
// @asm 0x0883134C-0x08831788
// @data 0x08876A9C-0x08876AD0
/* Effect on a card shown on screen: the card rises from lying flat to facing the camera (16
   frames) and stays until the task gets flag 1, then fades out in 8 frames. */
#define DD_FX
#include "dd_d3.h"

typedef struct DdVec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) DdVec4;

typedef struct DdScrPos {
    int x, y, z;
} DdScrPos;

typedef struct DdFxFlipUp {            /* 0x6C */
    int id;                            /* +0x00 own task handle */
    int in;                            /* +0x04 frames of the rise (stops at 16) */
    int out;                           /* +0x08 frames of the fade */
    int card;                          /* +0x0C */
    DdScrPos pos;                      /* +0x10 */
    DdTexVert quad[4];                 /* +0x1C */
} DdFxFlipUp;

void mtx_get34(float *m);
float *mat4_identity(float *out);
float *mat4_mul(float *out, float *a, float *b);
float *mat4_transform_vec4(DdVec4 *out, float *m, DdVec4 *v);
void dd_cam_begin(int layer, int ox, int oy, float fovy, float aspect, float near, float far, float *eye, float *target, float *up);
void dd_cam_end(int layer);

int dd_fx_flipup_update(int state, DdFxFlipUp *fx);


int dd_fx_flipup_risen(int id)
{
    DdFxFlipUp *fx = dd_task_mem(id);

    if (fx == 0)
        return 1;
    return fx->in >= 16;
}

int dd_fx_flipup_start(int card, DdScrPos *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_flipup_update, term, arg, sizeof(DdFxFlipUp));
    DdFxFlipUp *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->id = id;
        fx->card = card;
        fx->pos = *pos;
    }
    return id;
}

/* templates of the locals of dd_fx_flipup_update (kept in asm) */
float dd_flipup_eye[3] = { 0.0f, 0.0f, 500.0f };
float dd_flipup_up[3] = { 0.0f, 1.0f, 0.0f };
DdVec4 dd_flipup_corner = { 1.0f, 1.0f, 0.0f, 1.0f };

#ifdef NON_MATCHING /* alocação de pilha e de registradores de ponto flutuante */
int dd_fx_flipup_update(int state, DdFxFlipUp *fx)
{
    int step = dd_anim_step();
    float eye[3] = { 0.0f, 0.0f, 500.0f };
    float target[3] = { 0 };
    float up[3] = { 0.0f, 1.0f, 0.0f };   /* dd_flipup_eye, dd_flipup_up */
    float proj[16], view[16], model[16], m[16];
    DdVec4 v;
    unsigned int *list;
    float e, f;

    dd_cam_begin(6, fx->pos.x, 77.0f + fx->pos.y, 0.38397247f, 1.7647059f, 1.0f, 1000.0f, eye, target, up);
    e = dd_ease_sin(fx->in, 16, 2);
    f = dd_ease_lin_inv(fx->out, 8);
    if (state == 1) {
        float hw = 54.0f, hh = 77.0f;

        v = dd_flipup_corner;
        mtx_set_mode(0);
        mtx_get34(proj);
        mtx_set_mode(1);
        mtx_get34(view);
        mat4_identity(model);
        mat4_mul(m, view, model);
        mat4_mul(m, proj, m);
        mat4_transform_vec4(&v, m, &v);
        v.x = 240.0f * v.x / v.w;
        v.y = 136.0f * v.y / v.w;
        hh = 2.0f * (hh / v.y);
        hw = hw / v.x;
        fx->quad[0].u = 0.0f;
        fx->quad[0].v = 0.0f;
        fx->quad[0].x = -hw;
        fx->quad[0].y = hh;
        fx->quad[0].z = 0.0f;
        fx->quad[1].u = 0.0f;
        fx->quad[1].v = 0.6015625f;
        fx->quad[1].x = -hw;
        fx->quad[1].y = 0.0f;
        fx->quad[1].z = 0.0f;
        fx->quad[2].u = 0.84375f;
        fx->quad[2].v = 0.0f;
        fx->quad[2].x = hw;
        fx->quad[2].y = hh;
        fx->quad[2].z = 0.0f;
        fx->quad[3].u = 0.84375f;
        fx->quad[3].v = 0.6015625f;
        fx->quad[3].x = hw;
        fx->quad[3].y = 0.0f;
        fx->quad[3].z = 0.0f;
    }
    list = dl_select_layer(6, 0);
    list = dl_enable(list, 5);
    list = dd_depth_enable(list, 0, 1);
    list = dd_tex_card(list, fx->card, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0xFFFFFFFF);
    list = dl_color(list, 0xFFFFFFFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    list = dl_ambient_light(list, ((unsigned int)(255.0f * e * f) << 24) | 0xFFFFFF);
    mtx_rotate_x(1.5707964f - 1.5707964f * e);
    list = draw_array(list, 4, 0x183, 4, 0, fx->quad);
    mtx_pop();
    list = dl_disable(list, 5);
    dl_set_list(list);
    dd_cam_end(6);
    if (dd_task_flag_test(fx->id, 1)) {
        fx->in = 16;
        fx->out += step;
        if (fx->out < 8)
            return 2;
        return 4;
    }
    fx->in += step;
    if (fx->in > 16)
        fx->in = 16;
    return 2;
}
#endif
