// @range 0x08831C58-0x08832138
// @asm 0x08831D8C-0x08832138
/* Effect: a pulsing icon (one of 8 in spel_mark01.gim, kind 6 in red) lying at a point of the
   field, for a number of frames or until the task gets flag 1 (dur -1). */
#define DD_FX
#include "libduel_draw.h"

typedef struct DdFxMark {              /* 0x94 */
    int id;                            /* +0x00 own task handle */
    int layer;                         /* +0x04 */
    int count;                         /* +0x08 frames shown */
    int dur;                           /* +0x0C -1: until flag 1 */
    int in;                            /* +0x10 0..4 */
    int out;                           /* +0x14 0..8 */
    int pulse;                         /* +0x18 */
    int kind;                          /* +0x1C icon */
    float pos[3];                      /* +0x20 */
    float rot[3];                      /* +0x2C */
    float scale[3];                    /* +0x38 */
    DdTexVert quad[4];                 /* +0x44 */
} DdFxMark;

int dd_fx_mark_update(int state, DdFxMark *fx);

int dd_fx_mark_start(float s, int kind, float *pos, float *rot, int dur, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_mark_update, term, arg, sizeof(DdFxMark));
    DdFxMark *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->id = id;
        fx->layer = 4;
        fx->dur = dur;
        fx->kind = kind;
        vec3_copy(fx->pos, pos);
        vec3_copy(fx->rot, rot);
        vec3_set(fx->scale, s, s, s);
    }
    return id;
}

int dd_fx_mark_start_layer(int layer, float s, int kind, float *pos, float *rot, int dur, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_mark_start(s, kind, pos, rot, dur, term, arg);

    if (id != -1)
        ((DdFxMark *)dd_task_mem(id))->layer = layer;
    return id;
}

#ifdef NON_MATCHING /* ordem dos operandos de ponto flutuante e constantes guardadas em registrador (forma igual) */
int dd_fx_mark_update(int state, DdFxMark *fx)
{
    float in = (float)fx->in / 4.0f;
    float fo = 1.0f - (float)fx->out / 8.0f;
    unsigned int *list;

    if (state == 1) {
        float u = (float)(fx->kind % 4) * 0.25f;
        float v = (float)(fx->kind / 4) * 0.5f;
        float h = h;

        fx->quad[0].u = u;
        fx->quad[0].v = v;
        fx->quad[0].x = -h;
        fx->quad[0].y = 0.0f;
        fx->quad[0].z = -h;
        fx->quad[1].u = u;
        fx->quad[1].v = v + 0.5f;
        fx->quad[1].x = -h;
        fx->quad[1].y = 0.0f;
        fx->quad[1].z = h;
        fx->quad[2].u = u + 0.25f;
        fx->quad[2].v = v;
        fx->quad[2].x = h;
        fx->quad[2].y = 0.0f;
        fx->quad[2].z = -h;
        fx->quad[3].u = u + 0.25f;
        fx->quad[3].v = v + 0.5f;
        fx->quad[3].x = h;
        fx->quad[3].y = 0.0f;
        fx->quad[3].z = h;
    }
    list = dl_select_layer(fx->layer, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dd_tex_pack(list, 22);
    if (fx->kind == 6)
        list = dl_color(list, 0xFF0000FF);
    else
        list = dl_color(list, 0xFFFFFFFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    mtx_rotate_zyx(fx->rot);
    mtx_scale3(fx->scale);
    list = dl_ambient_light(list, ((unsigned int)(fo
                                                  * (cosf((float)(fx->pulse % 32) * 6.2831855f / 32.0f) * 0.3f + 0.7f)
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
