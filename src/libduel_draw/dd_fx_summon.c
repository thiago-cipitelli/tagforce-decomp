// @range 0x0882C7A0-0x0882D994
// @asm 0x0882C920-0x0882D994
// @data 0x08876A74-0x08876A8C
/* Effect: a card laid on the field (summon) with 16 light pillars turning around it, sparks
   flying out, a coloured ring on the floor and, for the dark kind, music notes rising around it;
   24 frames. */
#include "libduel_draw.h"

typedef struct DdTexVert {             /* float texture + float position (0x183) */
    float u, v;
    float x, y, z;
} DdTexVert;

typedef struct DdColVert {             /* 8888 colour + float position (0x99C, 8-bit indices) */
    unsigned int color;
    float x, y, z;
} DdColVert;

typedef struct DdFxSpark {             /* 0x18 */
    unsigned int flags;                /* bit 0: alive */
    int uv;                            /* sprite of the sheet (0..7) */
    int age;
    float pos[3];
} DdFxSpark;

typedef struct DdFxNote {              /* 0x10 */
    unsigned int flags;
    int kind;                          /* note sprite (0..5) */
    int age;
    float ang;
} DdFxNote;

typedef struct DdFxSummon {            /* 0xC4C */
    int frame;                         /* +0x000 */
    int card;                          /* +0x004 */
    float pos[3];                      /* +0x008 */
    float rot[3];                      /* +0x014 */
    int kind;                          /* +0x020 0 light, 1 dark (red, with notes) */
    unsigned int se;                   /* +0x024 */
    float t;                           /* +0x028 */
    DdFxVert pillar[4];                /* +0x02C */
    int next_spark;                    /* +0x08C */
    DdTexVert spark_uv[8][2];          /* +0x090 */
    DdFxSpark spark[32];               /* +0x1D0 */
    DdColVert ring[16][3];             /* +0x4D0 inner, middle, outer radius (17 spokes, the 17th wraps) */
    DdColVert ring_end[3];
    unsigned char ring_idx[8];         /* +0x800 */
    int next_note;                     /* +0x808 */
    DdFxVert note_quad[6][4];          /* +0x80C */
    DdFxNote note[32];                 /* +0xA4C */
} DdFxSummon;

typedef struct DdRandSeed {
    unsigned int lcg;
    unsigned int b, c;
} DdRandSeed;

extern int rand3_slot;
extern DdRandSeed rand3_seeds[];

#define DD_RAND(r) ((r)->lcg *= 0x41C64E6D, (r)->lcg += 0x3039, (int)((r)->lcg >> 16 & 0x7FFF))

float *vec3_copy(float *out, float *v);
float *vec3_set(float *out, float x, float y, float z);
float sinf(float x);
float cosf(float x);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(float *v);
void mtx_rotate_x(float a);
void mtx_rotate_y(float a);
void mtx_scale3(float *v);
void mtx_mult(float *m);
unsigned int *draw_array(unsigned int *list, int prim, int vtype, int count, void *indices, void *verts);
unsigned int *dd_tex_pack(unsigned int *list, int i);
unsigned int *dd_tex_enable(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v);
unsigned int *dd_depth_enable(unsigned int *list, int on, int mask);
unsigned int *dd_blend_enable(unsigned int *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void dd_card_quad_draw1(int id, float *pos, float *rot, int alpha, float t);
void dd_mtx_billboard_view(float *out);
float dd_ease_lin(int t, int n);
float dd_ease_sin(int t, int n, int k);
float dd_ease_cos(int t, int n, int k);
int dd_anim_step(void);
int dd_cue_se(int id, int now, int at, unsigned int *flags, unsigned int bit);

int dd_fx_summon_update(int state, DdFxSummon *fx);

int dd_fx_summon_start(float ry, int card, float *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_summon_update, term, arg, sizeof(DdFxSummon));
    DdFxSummon *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->kind = 0;
        fx->card = card;
        vec3_copy(fx->pos, pos);
        vec3_set(fx->rot, 0.0f, ry, 0.0f);
        fx->t = 0.0f;
    }
    return id;
}

int dd_fx_summon_start_dark(float ry, int card, float *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_summon_start(ry, card, pos, term, arg);

    if (id != -1) {
        DdFxSummon *fx = dd_task_mem(id);
        int one = 1;

        fx->kind = one;
    }
    return id;
}

int dd_fx_summon_start_t(float ry, float t, int card, float *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_summon_start(ry, card, pos, term, arg);

    if (id != -1)
        ((DdFxSummon *)dd_task_mem(id))->t = t;
    return id;
}

int dd_fx_summon_start_dark_t(float ry, float t, int card, float *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_summon_start_dark(ry, card, pos, term, arg);

    if (id != -1)
        ((DdFxSummon *)dd_task_mem(id))->t = t;
    return id;
}

unsigned int dd_fx_summon_tmpl[3] = { 0x00000000, 0x00000000, 0xC1EAAAAB };
#pragma explicit_zero_data on
unsigned int dd_fx_summon_tmpl2[2] = { 0x00000000, 0x00000000 };
#pragma explicit_zero_data reset
unsigned int dd_fx_summon_tmpl3[1] = { 0x3F800000 };

#ifdef NON_MATCHING /* função grande (4 KB); o modelo dos vetores locais fica em dd_fx_summon_tmpl */
static void dd_fx_set_vert(DdFxVert *v, float u, float tv, unsigned int color, float x, float y, float z)
{
    v->u = u;
    v->v = tv;
    v->color = color;
    v->x = x;
    v->y = y;
    v->z = z;
}

int dd_fx_summon_update(int state, DdFxSummon *fx)
{
    DdRandSeed *r = &rand3_seeds[rand3_slot];
    int step = dd_anim_step();
    unsigned int *list;
    float bb[16];
    float v[3];
    float grow, fade, a, rad;
    unsigned int alpha;
    unsigned int c0, c1;
    int i, j, k;

    dd_cue_se(0x622A, fx->frame, 0, &fx->se, 1);
    dd_card_quad_draw1(fx->card, fx->pos, fx->rot, (unsigned int)(dd_ease_lin(fx->frame, 24) * 255.0f), fx->t);

    /* light pillars */
    if (state == 1) {
        if (fx->kind == 0) {
            c0 = 0xAAFF;
            c1 = 0xF000AAFF;
        } else {
            c0 = 0x6464FF;
            c1 = 0xFF6464FF;
        }
        dd_fx_set_vert(&fx->pillar[0], 0.0f, 0.0f, c0, -49.6f, 200.0f, -44.0f);
        dd_fx_set_vert(&fx->pillar[1], 0.0f, 0.5f, c1, -49.6f, 0.0f, 0.0f);
        dd_fx_set_vert(&fx->pillar[2], 1.0f, 0.0f, c0, 49.6f, 200.0f, -44.0f);
        dd_fx_set_vert(&fx->pillar[3], 1.0f, 0.5f, c1, 49.6f, 0.0f, 0.0f);
    }
    grow = (float)fx->frame * 0.03f + 1.0f;
    fade = dd_ease_cos(fx->frame, 24, 1);
    list = dl_select_layer(4, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 3);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    list = dl_ambient_light(list, ((unsigned int)(fade * (dd_ease_lin(fx->frame, 4) * 0.9f + 0.1f) * 255.0f) << 24) | 0xFFFFFF);
    for (i = 0; i < 16; i++) {
        a = (float)i * 6.2831855f / 16.0f + (float)fx->frame * 0.02617994f;
        vec3_set(v, grow * -29.333334f * sinf(a), 0.0f, grow * -29.333334f * cosf(a));
        mtx_push();
        mtx_translate(v);
        mtx_rotate_y(a);
        list = draw_array(list, 4, 0x19F, 4, 0, fx->pillar);
        mtx_pop();
    }
    mtx_pop();
    dl_set_list(list);

    /* sparks: a 4x2 sheet of 0.25x0.5 cells, 20x20 sprites */
    if (state == 1) {
        for (i = 0; i < 8; i++) {
            float u = (float)(i % 4) * 0.25f;
            float tv = (float)(i / 4) * 0.5f;

            fx->spark_uv[i][0].u = u;
            fx->spark_uv[i][0].v = tv;
            fx->spark_uv[i][0].x = -10.0f;
            fx->spark_uv[i][0].y = -10.0f;
            fx->spark_uv[i][0].z = 0.0f;
            fx->spark_uv[i][1].u = u + 0.25f;
            fx->spark_uv[i][1].v = tv + 0.5f;
            fx->spark_uv[i][1].x = 10.0f;
            fx->spark_uv[i][1].y = 10.0f;
            fx->spark_uv[i][1].z = 0.0f;
        }
    }
    for (i = 0; i < step; i++) {
        int base = DD_RAND(r);

        for (j = 0; j < 2; j++) {
            DdFxSpark *s = &fx->spark[fx->next_spark];

            s->flags |= 1;
            s->uv = DD_RAND(r) & 7;
            s->age = 0;
            a = (float)j * 6.2831855f / 2.0f + (float)(base & 0x1F) * 0.19634955f;
            rad = grow * (DD_RAND(r) / 32767.0f) * 29.333334f;
            vec3_set(s->pos, rad * sinf(a), 0.0f, rad * cosf(a));
            if (fx->next_spark < 31)
                fx->next_spark++;
            else
                fx->next_spark = 0;
        }
    }
    list = dl_select_layer(4, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    if (fx->kind == 0)
        list = dd_tex_pack(list, 16);
    else
        list = dd_tex_pack(list, 13);
    if (fx->kind == 0)
        list = dl_color(list, 0x80C8FFFF);
    else
        list = dl_color(list, 0xFF8282FF);
    dd_mtx_billboard_view(bb);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    for (i = 0; i < 32; i++) {
        DdFxSpark *s = &fx->spark[i];

        if (s->flags & 1) {
            list = dl_ambient_light(list, ((unsigned int)(fade * dd_ease_cos(s->age, 16, 1) * 255.0f) << 24) | 0xFFFFFF);
            mtx_push();
            s->pos[1] = (float)s->age * 15.0f;
            mtx_translate(s->pos);
            mtx_mult(bb);
            list = draw_array(list, 6, 0x183, 2, 0, fx->spark_uv[s->uv]);
            mtx_pop();
            s->age += step;
            if (s->age > 15)
                s->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);

    /* floor ring */
    if (state == 1) {
        if (fx->kind == 0) {
            c0 = 0x64C8FF;
            c1 = 0xC864C8FF;
        } else {
            c0 = 0x6464FF;
            c1 = 0xFF6464FF;
        }
        for (i = 0; i < 17; i++) {
            DdColVert *rv = fx->ring[i];

            a = (float)i * 6.2831855f / 16.0f;
            rv[0].color = c0;
            rv[0].x = sinf(a) * 44.333336f;
            rv[0].y = 0.0f;
            rv[0].z = cosf(a) * 44.333336f;
            rv[1].color = c1;
            rv[1].x = sinf(a) * 29.333334f;
            rv[1].y = 0.0f;
            rv[1].z = cosf(a) * 29.333334f;
            rv[2].color = c0;
            rv[2].x = sinf(a) * 9.333334f;
            rv[2].y = 0.0f;
            rv[2].z = cosf(a) * 9.333334f;
        }
        fx->ring_idx[0] = 0;
        fx->ring_idx[1] = 1;
        fx->ring_idx[2] = 3;
        fx->ring_idx[3] = 4;
        fx->ring_idx[4] = 1;
        fx->ring_idx[5] = 2;
        fx->ring_idx[6] = 4;
        fx->ring_idx[7] = 5;
    }
    list = dl_select_layer(4, 0);
    {
        float s[3] = { grow, 0.0f, grow };

        list = dd_tex_enable(list, 0, 1, 1, 0, 1, 1, 1);
        list = dd_depth_enable(list, 1, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(fx->pos);
        mtx_scale3(s);
    }
    alpha = ((unsigned int)(fade * 255.0f) << 24) | 0xFFFFFF;
    list = dl_ambient_light(list, alpha);
    for (i = 0; i < 16; i++) {
        list = draw_array(list, 4, 0x99C, 4, &fx->ring_idx[0], fx->ring[i]);
        list = draw_array(list, 4, 0x99C, 4, &fx->ring_idx[4], fx->ring[i]);
    }
    mtx_pop();
    dl_set_list(list);

    /* music notes of the dark kind */
    if (fx->kind == 1) {
        if (state == 1) {
            for (i = 0; i < 6; i++) {
                float u = (float)(i % 4) * 0.25f;
                float tv = (float)(i / 4) * 0.25f + 0.5f;
                DdFxVert *q = fx->note_quad[i];

                switch (i / 2) {
                case 2:
                    dd_fx_set_vert(&q[0], u, tv, 0xFF0064, -70.0f, 0.0f, -200.0f);
                    dd_fx_set_vert(&q[1], u, tv + 0.25f, 0xC8FF0064, -70.0f, 0.0f, 0.0f);
                    dd_fx_set_vert(&q[2], u + 0.25f, tv, 0xFF0064, 70.0f, 0.0f, -200.0f);
                    dd_fx_set_vert(&q[3], u + 0.25f, tv + 0.25f, 0xC8FF0064, 70.0f, 0.0f, 0.0f);
                    break;
                case 1:
                    dd_fx_set_vert(&q[0], u + 0.25f, tv, 0xFF0064, -70.0f, 0.0f, -200.0f);
                    dd_fx_set_vert(&q[1], u, tv, 0xC8FF0064, -70.0f, 0.0f, 0.0f);
                    dd_fx_set_vert(&q[2], u + 0.25f, tv + 0.25f, 0xFF0064, 70.0f, 0.0f, -200.0f);
                    dd_fx_set_vert(&q[3], u, tv + 0.25f, 0xC8FF0064, 70.0f, 0.0f, 0.0f);
                    break;
                case 0:
                    dd_fx_set_vert(&q[0], u, tv + 0.25f, 0xFF0064, -70.0f, 0.0f, -200.0f);
                    dd_fx_set_vert(&q[1], u, tv + 0.001953125f, 0xC8FF0064, -70.0f, 0.0f, 0.0f);
                    dd_fx_set_vert(&q[2], u + 0.25f, tv + 0.25f, 0xFF0064, 70.0f, 0.0f, -200.0f);
                    dd_fx_set_vert(&q[3], u + 0.25f, tv + 0.001953125f, 0xC8FF0064, 70.0f, 0.0f, 0.0f);
                    break;
                }
            }
        }
        for (i = 0; i < step; i++) {
            for (j = 0; j < 4; j++) {
                DdFxNote *n = &fx->note[fx->next_note];

                n->flags |= 1;
                n->kind = DD_RAND(r) % 6;
                n->age = 0;
                n->ang = DD_RAND(r) / 32767.0f * 6.2831855f;
                if (fx->next_note < 31)
                    fx->next_note++;
                else
                    fx->next_note = 0;
            }
        }
        list = dl_select_layer(4, 0);
        {
            float ofs[3] = { 0.0f, 0.0f, -29.333334f };

            list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
            list = dd_depth_enable(list, 1, 1);
            list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
            list = dd_tex_pack(list, 11);
            mtx_set_mode(2);
            mtx_push();
            mtx_identity();
            mtx_translate(fx->pos);
            for (i = 0; i < 32; i++) {
                DdFxNote *n = &fx->note[i];

                if (n->flags & 1) {
                    float s[3] = { dd_ease_sin(n->age, 8, 1), 0.0f, 1.0f };

                    list = dl_ambient_light(list, alpha);
                    mtx_push();
                    mtx_rotate_y(n->ang);
                    mtx_translate(ofs);
                    mtx_rotate_x(1.3962634f);
                    mtx_scale3(s);
                    list = draw_array(list, 4, 0x19F, 4, 0, fx->note_quad[n->kind]);
                    mtx_pop();
                    n->age += step;
                    if (n->age > 7)
                        n->flags &= ~1;
                }
            }
        }
        mtx_pop();
        dl_set_list(list);
    }
    fx->frame += step;
    if (fx->frame < 24)
        return 2;
    return 4;
}
#endif
