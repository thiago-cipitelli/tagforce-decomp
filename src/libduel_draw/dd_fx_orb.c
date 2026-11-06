// @range 0x08830AAC-0x08831290
/* Effect: a pink orb flying from one point to another in an arc, leaving bursts of fading
   sparks behind it; 24 frames. */
#define DD_FX
#include "dd_d3.h"

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
void dd_rand_dir(float len, float *out);

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

#define DD_RAND3(r) ((r) = &rand3_seeds[rand3_slot], (r)->lcg *= 0x41C64E6D, (r)->lcg += 0x3039, \
                     (int)((r)->lcg >> 16 & 0x7FFF))

inline static void dd_fv(DdFxVert *v, float u, float tv, unsigned int c, float x, float y)
{
    v->u = u;
    v->v = tv;
    v->color = c;
    v->x = x;
    v->y = y;
    v->z = 0.0f;
}

inline static void dd_flash_init(DdFxVert *f, unsigned int c, float r)
{
    dd_fv(&f[0], 0.5f, 0.5f, c | 0xFF000000, 0.0f, 0.0f);
    dd_fv(&f[1], 0.0f, 0.0f, c, -r, r);
    dd_fv(&f[2], 1.0f, 0.0f, c, r, r);
    dd_fv(&f[3], 1.0f, 1.0f, c, r, -r);
    dd_fv(&f[4], 0.0f, 1.0f, c, -r, -r);
    dd_fv(&f[5], 0.0f, 0.0f, c, -r, r);
}

inline static void dd_tv(DdTexVert *v, float u, float tv, float x, float y)
{
    v->u = u;
    v->v = tv;
    v->x = x;
    v->y = y;
    v->z = 0.0f;
}

int dd_fx_orb_update(int state, DdFxOrb *fx)
{
    DdRandSeed *r;
    int step = dd_anim_step();
    float in = dd_ease_lin(fx->frame, 4);
    float out = dd_ease_lin_inv(fx->frame - 20, 4);
    float e = dd_ease_sin(fx->frame, 24, 1);
    float h = sinf((float)fx->frame * 3.1415927f / 24.0f);
    float p[3], q[3];
    float bb[18];
    unsigned int *list;
    int i, k;
    int n;

    vec3_lerp(p, fx->from, fx->to, e);
    p[1] += h * 150.0f;
    dd_mtx_billboard_view(bb);
    if (state == 1) {
        float r = 75.0f;

        dd_fv(&fx->orb[0], 0.5f, 0.5f, 0xFFC792CC, 0.0f, 0.0f);
        dd_fv(&fx->orb[1], 0.0f, 0.0f, 0xC792CC, -r, r);
        dd_fv(&fx->orb[2], 1.0f, 0.0f, 0xC792CC, r, r);
        dd_fv(&fx->orb[3], 1.0f, 1.0f, 0xC792CC, r, -r);
        dd_fv(&fx->orb[4], 0.0f, 1.0f, 0xC792CC, -r, -r);
        dd_fv(&fx->orb[5], 0.0f, 0.0f, 0xC792CC, -r, r);
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
    list = dl_ambient_light(list, ((unsigned int)(out * (255.0f * (0.5f + 0.5f * (float)(fx->frame % 2)) * in)) << 24) | 0xFFFFFF);
    list = draw_array(list, 5, 0x19F, 6, 0, fx->orb);
    mtx_pop();
    dl_set_list(list);
    if (state == 1) {
        float s = 30.0f;
        float du = 0.25f, dv = 0.5f;

        for (i = 0; i < 8; i++) {
            float u = (float)(i % 4) * du;
            float tv = (float)(i / 4) * dv;

            dd_tv(&fx->spark_uv[i][0], u, tv, -s, s);
            dd_tv(&fx->spark_uv[i][1], u + du, tv + dv, s, -s);
        }
    }
    for (n = 0; n < step; n++) {
        DdOrbTrail *t = &fx->trail[fx->next];
        int j;

        t->flags |= 1;
        t->age = 0;
        vec3_copy(t->pos, p);
        for (j = 0; j < 4; j++) {
            t->uv[j] = DD_RAND3(r) % 8;
            dd_rand_dir(30.0f, t->dir[j]);
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

        if (!(t->flags & 1) != 1) {
            float te = dd_ease_lin(t->age, 8);
            float sc = 0.5f + 0.5f * te;
            float rise = 60.0f * te;
            float w = 1.0f - te;

            list = dl_ambient_light(list, ((unsigned int)(out * (in * (255.0f * dd_ease_cos(t->age, 8, 1)))) << 24) | 0xFFFFFF);
            for (k = 0; k < 4; k++) {
                vec3_scale(q, t->dir[k], sc);
                vec3_add(q, q, t->pos);
                q[1] += rise;
                mtx_push();
                mtx_translate(q);
                mtx_mult(bb);
                dd_scale3(w, w, 0.0f);
                list = draw_array(list, 6, 0x183, 2, 0, fx->spark_uv[t->uv[k]]);
                mtx_pop();
            }
            t->age += step;
            if (t->age >= 8)
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
