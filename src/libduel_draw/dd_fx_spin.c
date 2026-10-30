// @range 0x08832138-0x0883264C
// @asm 0x08832284-0x0883264C
/* Effect: a pulsing disc (k02_256b.gim or, mirrored, k02_256y.gim) lying on the field and
   turning slowly, for a number of frames or until the task gets flag 1 (dur -1). */
#define DD_FX
#include "dd_d3.h"

typedef struct DdFxSpin {              /* 0x90 */
    int id;                            /* +0x00 own task handle */
    int side;                          /* +0x04 0: blue disc, else yellow mirrored */
    int count;                         /* +0x08 frames shown, also the turn */
    int dur;                           /* +0x0C -1: until flag 1 */
    int in;                            /* +0x10 0..4 */
    int out;                           /* +0x14 0..8 */
    int pulse;                         /* +0x18 */
    float pos[3];                      /* +0x1C */
    float rot[3];                      /* +0x28 */
    float scale[3];                    /* +0x34 */
    DdTexVert quad[4];                 /* +0x40 */
} DdFxSpin;

int dd_fx_spin_update(int state, DdFxSpin *fx);

int dd_fx_spin_start(float s, int side, float *pos, float *rot, int dur, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_spin_update, term, arg, sizeof(DdFxSpin));
    DdFxSpin *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->id = id;
        fx->side = side;
        fx->dur = dur;
        vec3_copy(fx->pos, pos);
        vec3_copy(fx->rot, rot);
        vec3_set(fx->scale, s, s, s);
    }
    return id;
}

int dd_fx_spin_move(int id, float *pos, float *rot, float s)
{
    DdFxSpin *fx = dd_task_mem(id);

    if (fx == 0)
        return 0;
    vec3_copy(fx->pos, pos);
    vec3_copy(fx->rot, rot);
    vec3_set(fx->scale, s, s, s);
    return 1;
}

#ifdef NON_MATCHING /* mesma forma de dd_fx_mark_update */
int dd_fx_spin_update(int state, DdFxSpin *fx)
{
    float in = (float)fx->in / 4.0f;
    float fo = 1.0f - (float)fx->out / 8.0f;
    unsigned int *list;
    float u0, u1;

    if (state == 1) {
        if (fx->side == 0) {
            u0 = 1.0f;
            u1 = 0.0f;
        } else {
            u0 = 0.0f;
            u1 = 1.0f;
        }
        fx->quad[0].u = u0;
        fx->quad[0].v = 0.0f;
        fx->quad[0].x = -48.0f;
        fx->quad[0].y = 0.0f;
        fx->quad[0].z = -48.0f;
        fx->quad[1].u = u0;
        fx->quad[1].v = 1.0f;
        fx->quad[1].x = -48.0f;
        fx->quad[1].y = 0.0f;
        fx->quad[1].z = 48.0f;
        fx->quad[2].u = u1;
        fx->quad[2].v = 0.0f;
        fx->quad[2].x = 48.0f;
        fx->quad[2].y = 0.0f;
        fx->quad[2].z = -48.0f;
        fx->quad[3].u = u1;
        fx->quad[3].v = 1.0f;
        fx->quad[3].x = 48.0f;
        fx->quad[3].y = 0.0f;
        fx->quad[3].z = 48.0f;
    }
    list = dl_select_layer(4, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    if (fx->side == 0)
        list = dd_tex_pack(list, 26);
    else
        list = dd_tex_pack(list, 28);
    list = dl_color(list, 0xFFFFFFFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    mtx_rotate_zyx(fx->rot);
    mtx_scale3(fx->scale);
    mtx_rotate_y((float)fx->count * -0.017453292f);
    list = dl_ambient_light(list, ((unsigned int)(fo * (cosf((float)(fx->pulse % 32) * 6.2831855f / 32.0f) * 0.3f + 0.7f)
                                                  * 255.0f * in) << 24) | 0xFFFFFF);
    list = draw_array(list, 4, 0x183, 4, 0, fx->quad);
    mtx_pop();
    dl_set_list(list);
    fx->count++;
    if (fx->out == 0) {
        if (fx->in < 4)
            fx->in++;
        else
            fx->pulse++;
    }
    if (fx->dur == -1) {
        if (dd_task_flag_test(fx->id, 1) == 1) {
            fx->out++;
            if (fx->out < 8)
                return 2;
            return 4;
        }
        return 2;
    }
    if (fx->count > fx->dur - 8)
        fx->out++;
    if (fx->count < fx->dur)
        return 2;
    return 4;
}
#endif
