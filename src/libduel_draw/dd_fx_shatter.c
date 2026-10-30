// @range 0x0882D994-0x0882ED38
// @asm 0x0882DA98-0x0882ED38
// @data 0x08876A8C-0x08876A9C
/* Effect: a card on the field shattering into 48 triangles (front and back faces) that fly out
   and spin while the whole falls, with a flash and 16 sparks; 24 frames. */
#include "libduel_draw.h"

typedef struct DdTexVert {             /* float texture + float position (0x183) */
    float u, v;
    float x, y, z;
} DdTexVert;

typedef struct DdShard {               /* two triangles of a 6x4 grid cell, 0x168 */
    DdTexVert tri[4][3];               /* front 0,1; back 2,3 (u mirrored) */
    float pos[2][3];
    float pos0[2][3];
    float vel[2][3];
    float rot[2][3];
    float spin[2][3];
} DdShard;

typedef struct DdShatterSpark {        /* 0x1C */
    int uv;
    float base[3];
    float dir[3];
} DdShatterSpark;

typedef struct DdFxShatter {           /* 0x2508 */
    int frame;                         /* +0x000 */
    int card;                          /* +0x004 */
    float pos[3];                      /* +0x008 */
    float rot[3];                      /* +0x014 */
    float scale[3];                    /* +0x020 */
    unsigned int se;                   /* +0x02C */
    float t;                           /* +0x030 texture zoom */
    DdShard shard[6][4];               /* +0x034 */
    float ofs[3];                      /* +0x21F4 fall of the whole */
    float vel[3];                      /* +0x2200 */
    float acc[3];                      /* +0x220C */
    DdFxVert flash[6];                 /* +0x2218 */
    DdShatterSpark spark[16];          /* +0x22A8 */
    DdTexVert spark_uv[4][2];          /* +0x2468 */
} DdFxShatter;

typedef struct DdRandSeed {
    unsigned int lcg;
    unsigned int b, c;
} DdRandSeed;

extern int rand3_slot;
extern DdRandSeed rand3_seeds[];

#define DD_RAND(r) ((r)->lcg *= 0x41C64E6D, (r)->lcg += 0x3039, (int)((r)->lcg >> 16 & 0x7FFF))

float *vec3_copy(float *out, float *v);
float *vec3_set(float *out, float x, float y, float z);
float *vec3_add(float *out, float *a, float *b);
float *vec3_mul(float *out, float *a, float *b);
float *vec3_scale(float *out, float *v, float s);
float *vec3_normalize(float *out, float *v);
float *mat3_identity(float *out);
float *mat3_from_euler(float *out, float *m, float *angles);
float *mat3_transform_vec3(float *out, float *m, float *v);
float sinf(float x);
float cosf(float x);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(float *v);
void mtx_rotate_zyx(float *v);
void mtx_scale3(float *v);
void mtx_mult(float *m);
unsigned int *draw_array(unsigned int *list, int prim, int vtype, int count, void *indices, void *verts);
unsigned int *dd_tex_pack(unsigned int *list, int i);
unsigned int *dd_tex_card(unsigned int *list, int id, int mode);
unsigned int *dd_tex_enable(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v);
unsigned int *dd_tex_enable_uv(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v, float u, float v, float su, float sv);
unsigned int *dd_depth_enable(unsigned int *list, int on, int mask);
unsigned int *dd_blend_enable(unsigned int *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void dd_mtx_billboard_view(float *out);
float dd_ease_sin(int t, int n, int k);
float dd_ease_cos(int t, int n, int k);
int dd_anim_step(void);
int dd_cue_se(int id, int now, int at, unsigned int *flags, unsigned int bit);

int dd_fx_shatter_update(int state, DdFxShatter *fx);

int dd_fx_shatter_start(float s, int card, float *pos, float *rot, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_shatter_update, term, arg, sizeof(DdFxShatter));
    DdFxShatter *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->card = card;
        vec3_copy(fx->pos, pos);
        vec3_copy(fx->rot, rot);
        vec3_set(fx->scale, s, s, s);
        fx->t = 0.0f;
    }
    return id;
}

int dd_fx_shatter_start_t(float s, float t, int card, float *pos, float *rot, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_shatter_start(s, card, pos, rot, term, arg);

    if (id != -1)
        ((DdFxShatter *)dd_task_mem(id))->t = t;
    return id;
}

/* colour template of the tint (only the alpha word is used) */
#pragma explicit_zero_data on
int dd_fx_shatter_tint[3] = { 0, 0, 0 };
#pragma explicit_zero_data reset
int dd_fx_shatter_tint_a = 0xFF;

#ifdef NON_MATCHING /* função grande (4,7 KB) */
static void dd_rand_vec(DdRandSeed *r, float *out, float s)
{
    vec3_set(out, DD_RAND(r) / 32767.0f * s * (DD_RAND(r) % 2 * 2 - 1),
             DD_RAND(r) / 32767.0f * s * (DD_RAND(r) % 2 * 2 - 1),
             DD_RAND(r) / 32767.0f * s * (DD_RAND(r) % 2 * 2 - 1));
}

static void dd_tv(DdTexVert *v, float u, float tv, float x, float z)
{
    v->u = u;
    v->v = tv;
    v->x = x;
    v->y = 0.0f;
    v->z = z;
}

static void dd_fv(DdFxVert *v, float u, float tv, unsigned int c, float x, float y)
{
    v->u = u;
    v->v = tv;
    v->color = c;
    v->x = x;
    v->y = y;
    v->z = 0.0f;
}

int dd_fx_shatter_update(int state, DdFxShatter *fx)
{
    DdRandSeed *r = &rand3_seeds[rand3_slot];
    int step = dd_anim_step();
    float bb[18];
    float ofs[3], d[3], p[3], m3[9];
    unsigned int *list;
    unsigned int alpha, tint;
    float fade, e, dark, s, a;
    int i, j, k;

    dd_mtx_billboard_view(bb);
    dd_cue_se(0x6226, fx->frame, 0, &fx->se, 1);
    if (state == 1) {
        vec3_set(fx->acc, 0.0f, -0.5f, 0.0f);
        for (i = 0; i < 6; i++) {
            float v0 = (float)i * 0.1640625f - 0.4921875f;
            float v1 = v0 + 0.1640625f;
            float cz = (float)i * 14.666667f + -36.666668f;

            for (j = 0; j < 4; j++) {
                DdShard *sh = &fx->shard[i][j];
                float u0 = (float)j * 0.24609375f - 0.4921875f;
                float u1 = u0 + 0.24609375f;
                float cx = (float)j * 15.5f + -23.25f;

                dd_tv(&sh->tri[0][0], u0, v0, -5.1666667f, -4.888889f);
                dd_tv(&sh->tri[0][1], u0, v1, -5.1666667f, 9.777777f);
                dd_tv(&sh->tri[0][2], u1, v0, 10.333333f, -4.888889f);
                dd_tv(&sh->tri[1][0], u0, v1, -10.333333f, 4.888889f);
                dd_tv(&sh->tri[1][1], u1, v1, 5.166667f, 4.888889f);
                dd_tv(&sh->tri[1][2], u1, v0, 5.166667f, -9.777777f);
                dd_tv(&sh->tri[2][0], -u0, v0, -5.1666667f, -4.888889f);
                dd_tv(&sh->tri[2][1], -u0 - 0.24609375f, v0, 10.333333f, -4.888889f);
                dd_tv(&sh->tri[2][2], -u0, v1, -5.1666667f, 9.777777f);
                dd_tv(&sh->tri[3][0], -u0, v1, -10.333333f, 4.888889f);
                dd_tv(&sh->tri[3][1], -u0 - 0.24609375f, v0, 5.166667f, -9.777777f);
                dd_tv(&sh->tri[3][2], -u0 - 0.24609375f, v1, 5.166667f, 4.888889f);
                vec3_set(sh->pos[0], cx - 2.5833333f, 0.0f, cz - 2.4444444f);
                vec3_set(sh->pos[1], cx + 2.5833333f, 0.0f, cz + 2.4444444f);
                for (k = 0; k < 2; k++) {
                    vec3_copy(sh->pos0[k], sh->pos[k]);
                    vec3_normalize(d, sh->pos0[k]);
                    vec3_scale(d, d, 90.0f);
                    dd_rand_vec(r, p, 35.0f);
                    vec3_add(sh->vel[k], d, p);
                    dd_rand_vec(r, sh->spin[k], (float)step * 0.34906587f);
                }
            }
        }
    }

    /* the shards */
    fade = dd_ease_cos(fx->frame, 24, 1);
    list = dl_select_layer(4, 0);
    list = dl_enable(list, 5);
    list = dd_depth_enable(list, 1, 1);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    vec3_mul(ofs, fx->ofs, fx->scale);
    for (i = 0; i < step; i++) {
        vec3_add(fx->ofs, fx->ofs, fx->vel);
        vec3_add(fx->vel, fx->vel, fx->acc);
    }
    mtx_translate(ofs);
    mtx_rotate_zyx(fx->rot);
    mtx_scale3(fx->scale);
    e = dd_ease_sin(fx->frame, 24, 1);
    dark = (1.0f - dd_ease_sin(fx->frame, 24, 1)) * 0.7f;
    alpha = ((unsigned int)(fade * 255.0f) << 24) | 0xFFFFFF;
    list = dl_ambient_light(list, alpha);
    list = dd_tex_card(list, fx->card, 1);
    s = fx->t / 64.0f + 1.0f;
    list = dd_tex_enable_uv(list, 1, 1, 1, 4, 0, 1, 1, 0.5f, 0.5f, s, s);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    tint = (unsigned int)(dark * 255.0f) | (unsigned int)(dark * 220.0f) << 8 | dd_fx_shatter_tint_a << 24
           | (unsigned int)(dark * 130.0f) << 16;
    list = dl_color(list, tint);
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 4; j++) {
            DdShard *sh = &fx->shard[i][j];

            for (k = 0; k < 2; k++) {
                vec3_scale(d, sh->vel[k], e);
                vec3_add(sh->pos[k], sh->pos0[k], d);
                mtx_push();
                mtx_translate(sh->pos[k]);
                mtx_rotate_zyx(sh->rot[k]);
                list = draw_array(list, 3, 0x183, 3, 0, sh->tri[k]);
                mtx_pop();
            }
        }
    }
    list = dd_tex_card(list, -1, 1);
    s = fx->t / 64.0f + 1.0f;
    list = dd_tex_enable_uv(list, 1, 1, 1, 4, 0, 1, 1, 0.5f, 0.5f, s, s);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dl_color(list, tint);
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 4; j++) {
            DdShard *sh = &fx->shard[i][j];

            for (k = 0; k < 2; k++) {
                mtx_push();
                mtx_translate(sh->pos[k]);
                mtx_rotate_zyx(sh->rot[k]);
                list = draw_array(list, 3, 0x183, 3, 0, sh->tri[2 + k]);
                mtx_pop();
                vec3_add(sh->rot[k], sh->rot[k], sh->spin[k]);
            }
        }
    }
    mtx_pop();
    list = dl_disable(list, 5);
    dl_set_list(list);

    /* the flash */
    if (state == 1) {
        dd_fv(&fx->flash[0], 0.5f, 0.5f, 0xFFC8FFFF, 0.0f, 0.0f);
        dd_fv(&fx->flash[1], 0.0f, 0.0f, 0xC8FFFF, -150.0f, 150.0f);
        dd_fv(&fx->flash[2], 1.0f, 0.0f, 0xC8FFFF, 150.0f, 150.0f);
        dd_fv(&fx->flash[3], 1.0f, 1.0f, 0xC8FFFF, 150.0f, -150.0f);
        dd_fv(&fx->flash[4], 0.0f, 1.0f, 0xC8FFFF, -150.0f, -150.0f);
        dd_fv(&fx->flash[5], 0.0f, 0.0f, 0xC8FFFF, -150.0f, 150.0f);
    }
    list = dl_select_layer(4, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 3);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    mtx_scale3(fx->scale);
    mtx_mult(bb);
    list = dl_ambient_light(list, alpha);
    list = draw_array(list, 5, 0x19F, 6, 0, fx->flash);
    mtx_pop();
    dl_set_list(list);

    /* the sparks */
    if (state == 1) {
        mat3_identity(m3);
        mat3_from_euler(m3, m3, fx->rot);
        for (i = 0; i < 16; i++) {
            DdShatterSpark *sp = &fx->spark[i];

            a = (float)i * 6.2831855f / 16.0f;
            sp->uv = DD_RAND(r) % 4;
            vec3_set(p, sinf(a) * 30.0f, 0.0f, cosf(a) * 30.0f);
            mat3_transform_vec3(sp->base, m3, p);
            vec3_set(p, sinf(a) * 40.0f, 0.0f, cosf(a) * 40.0f);
            dd_rand_vec(r, d, 35.0f);
            vec3_add(p, p, d);
            mat3_transform_vec3(sp->dir, m3, p);
        }
        for (i = 0; i < 4; i++) {
            float u = (float)(i % 2) * 0.5f;
            float tv = (float)(i / 2) * 0.5f;

            fx->spark_uv[i][0].u = u;
            fx->spark_uv[i][0].v = tv;
            fx->spark_uv[i][0].x = -20.0f;
            fx->spark_uv[i][0].y = 20.0f;
            fx->spark_uv[i][0].z = 0.0f;
            fx->spark_uv[i][1].u = u + 0.5f;
            fx->spark_uv[i][1].v = tv + 0.5f;
            fx->spark_uv[i][1].x = 20.0f;
            fx->spark_uv[i][1].y = -20.0f;
            fx->spark_uv[i][1].z = 0.0f;
        }
    }
    list = dl_select_layer(4, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 10);
    list = dl_color(list, 0x8082DCFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    mtx_translate(ofs);
    mtx_scale3(fx->scale);
    e = dd_ease_sin(fx->frame, 24, 1);
    list = dl_ambient_light(list, alpha);
    for (i = 0; i < 16; i++) {
        DdShatterSpark *sp = &fx->spark[i];

        vec3_scale(p, sp->dir, e);
        vec3_add(p, p, sp->base);
        mtx_push();
        mtx_translate(p);
        mtx_mult(bb);
        list = draw_array(list, 6, 0x183, 2, 0, fx->spark_uv[sp->uv]);
        mtx_pop();
    }
    mtx_pop();
    dl_set_list(list);
    fx->frame += step;
    if (fx->frame < 24)
        return 2;
    return 4;
}
#endif
