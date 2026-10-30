// @range 0x08830AAC-0x08831290
// @asm 0x08830B38-0x08831290
/* Effect: a pink orb flying from one point to another in an arc, leaving bursts of fading
   sparks behind it; 24 frames. */
#define DD_FX
#include "libduel_draw.h"

typedef struct DdOrbTrail {            /* 0x54 */
    unsigned int flags;                /* bit 0: alive */
    int age;
    int uv[4];
    float pos[3];
    float dir[4][3];
} DdOrbTrail;

typedef struct DdFxOrb {               /* 0x490 */
    int frame;                         /* +0x000 */
    float from[3];                     /* +0x004 */
    float to[3];                       /* +0x010 */
    DdFxVert orb[6];                   /* +0x01C */
    int next;                          /* +0x0AC */
    DdTexVert spark_uv[8][2];          /* +0x0B0 */
    DdOrbTrail trail[8];               /* +0x1F0 */
} DdFxOrb;

float *vec3_lerp(float *out, float *a, float *b, float t);
void dd_rand_dir(float *out, float len);

int dd_fx_orb_update(int state, DdFxOrb *fx);

int dd_fx_orb_start(float *from, float *to, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_orb_update, term, arg, sizeof(DdFxOrb));
    DdFxOrb *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        vec3_copy(fx->from, from);
        vec3_copy(fx->to, to);
    }
    return id;
}

#ifdef NON_MATCHING /* função grande (1,9 KB) */
int dd_fx_orb_update(int state, DdFxOrb *fx)
{
    DdRandSeed *r = &rand3_seeds[rand3_slot];
    int step = dd_anim_step();
    float in = dd_ease_lin(fx->frame, 4);
    float out = dd_ease_lin_inv(fx->frame - 20, 4);
    float bb[18];
    float p[3], q[3];
    unsigned int *list;
    float e, h;
    int i, k;

    e = dd_ease_sin(fx->frame, 24, 1);
    h = sinf((float)fx->frame * 3.1415927f / 24.0f);
    vec3_lerp(p, fx->from, fx->to, e);
    p[1] += h * 150.0f;
    dd_mtx_billboard_view(bb);
    if (state == 1) {
        static const float ov[6][4] = {
            { 0.5f, 0.5f, 0.0f, 0.0f }, { 0.0f, 0.0f, -75.0f, 75.0f }, { 1.0f, 0.0f, 75.0f, 75.0f },
            { 1.0f, 1.0f, 75.0f, -75.0f }, { 0.0f, 1.0f, -75.0f, -75.0f }, { 0.0f, 0.0f, -75.0f, 75.0f },
        };

        for (i = 0; i < 6; i++) {
            fx->orb[i].u = ov[i][0];
            fx->orb[i].v = ov[i][1];
            fx->orb[i].color = i == 0 ? 0xFFC792CC : 0xC792CC;
            fx->orb[i].x = ov[i][2];
            fx->orb[i].y = ov[i][3];
            fx->orb[i].z = 0.0f;
        }
    }
    list = dl_select_layer(4, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 3);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(p);
    mtx_mult(bb);
    list = dl_ambient_light(list, ((unsigned int)(out * ((float)(fx->frame % 2) * 0.5f + 0.5f) * 255.0f * in) << 24) | 0xFFFFFF);
    list = draw_array(list, 5, 0x19F, 6, 0, fx->orb);
    mtx_pop();
    dl_set_list(list);
    if (state == 1) {
        for (i = 0; i < 8; i++) {
            float u = (float)(i % 4) * 0.25f;
            float tv = (float)(i / 4) * 0.5f;

            fx->spark_uv[i][0].u = u;
            fx->spark_uv[i][0].v = tv;
            fx->spark_uv[i][0].x = -30.0f;
            fx->spark_uv[i][0].y = 30.0f;
            fx->spark_uv[i][0].z = 0.0f;
            fx->spark_uv[i][1].u = u + 0.25f;
            fx->spark_uv[i][1].v = tv + 0.5f;
            fx->spark_uv[i][1].x = 30.0f;
            fx->spark_uv[i][1].y = -30.0f;
            fx->spark_uv[i][1].z = 0.0f;
        }
    }
    for (i = 0; i < step; i++) {
        DdOrbTrail *t = &fx->trail[fx->next];

        t->flags |= 1;
        t->age = 0;
        vec3_copy(t->pos, p);
        for (k = 0; k < 4; k++) {
            t->uv[k] = DD_RAND(r) % 8;
            dd_rand_dir(t->dir[k], 30.0f);
        }
        if (fx->next < 7)
            fx->next++;
        else
            fx->next = 0;
    }
    list = dl_select_layer(4, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 5);
    list = dl_color(list, 0xFFC792CC);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 8; i++) {
        DdOrbTrail *t = &fx->trail[i];

        if (t->flags & 1) {
            float te = dd_ease_lin(t->age, 8);
            float rise = te * 60.0f;

            list = dl_ambient_light(list, ((unsigned int)(out * in * dd_ease_cos(t->age, 8, 1) * 255.0f) << 24) | 0xFFFFFF);
            for (k = 0; k < 4; k++) {
                vec3_scale(q, t->dir[k], te * 0.5f + 0.5f);
                vec3_add(q, q, t->pos);
                q[1] += rise;
                mtx_push();
                mtx_translate(q);
                mtx_mult(bb);
                dd_scale3(1.0f - te, 1.0f - te, 0.0f);
                list = draw_array(list, 6, 0x183, 2, 0, fx->spark_uv[t->uv[k]]);
                mtx_pop();
            }
            t->age += step;
            if (t->age > 7)
                t->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    fx->frame += step;
    if (fx->frame < 24)
        return 2;
    return 4;
}
#endif
