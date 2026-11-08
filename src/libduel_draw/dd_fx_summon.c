// @range 0x0882C7A0-0x0882D994
// @data 0x08876A74-0x08876A8C
/* Effect: a card laid on the field (summon) with 16 light pillars turning around it, sparks
   flying out, a coloured ring on the floor and, for the dark kind, music notes rising around it;
   24 frames. */
#include "dd_d3.h"

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

int dd_fx_summon_start_t(int card, float ry, float t, float *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_summon_start(ry, card, pos, term, arg);

    if (id != -1)
        ((DdFxSummon *)dd_task_mem(id))->t = t;
    return id;
}

int dd_fx_summon_start_dark_t(int card, float ry, float t, float *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_summon_start_dark(ry, card, pos, term, arg);

    if (id != -1)
        ((DdFxSummon *)dd_task_mem(id))->t = t;
    return id;
}

/* templates of dd_fx_summon_update: the notes' offset from the center and their scale (x = fade in) */
typedef struct DdSumVec3 { float v[3]; } DdSumVec3;

float dd_fx_summon_ofs[3] = { 0.0f, 0.0f, -29.333334f };
float dd_fx_summon_scale[3] = { 0.0f, 0.0f, 1.0f };

inline static void dd_fx_set_vert(DdFxVert *v, float u, float tv, unsigned int color, float x, float y, float z)
{
    v->u = u;
    v->v = tv;
    v->color = color;
    v->x = x;
    v->y = y;
    v->z = z;
}

#define DD_RAND3(r) ((r) = &rand3_seeds[rand3_slot], (r)->lcg *= 0x41C64E6D, (r)->lcg += 0x3039, \
                     (int)((r)->lcg >> 16 & 0x7FFF))

int dd_fx_summon_update(int state, DdFxSummon *fx)
{
    unsigned int ring_c0;
    unsigned int ring_c1;
    int n_pillar;
    int n_cell;
    int n_burst;
    int n_ring;
    int n_quad;
    int n_new;
    int n_note;
    DdRandSeed *r;
    int step = dd_anim_step();
    unsigned int *list;
    float v[3];
    float bb[16] __attribute__((aligned(16)));
    float grow;
    float fade;
    float rad;
    unsigned int alpha;
    unsigned int c0;
    unsigned int c1;
    int i;
    int j;
    int k;
    float spin;
    float a;

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
        {
            float w = 49.600002f, d = 44.0f;

            dd_fx_set_vert(&fx->pillar[0], 0.0f, 0.0f, c0, -w, 200.0f, -d);
            dd_fx_set_vert(&fx->pillar[1], 0.0f, 0.5f, c1, -w, 0.0f, 0.0f);
            dd_fx_set_vert(&fx->pillar[2], 1.0f, 0.0f, c0, w, 200.0f, -d);
            dd_fx_set_vert(&fx->pillar[3], 1.0f, 0.5f, c1, w, 0.0f, 0.0f);
        }
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
    alpha = (unsigned int)(fade * ((dd_ease_lin(fx->frame, 4) * 0.9f + 0.1f) * 255.0f));
    spin = (float)fx->frame * 0.02617994f;
    list = dl_ambient_light(list, alpha << 24 | 0xFFFFFF);
    for (n_pillar = 0; n_pillar < 16; n_pillar++) {
        a = (float)n_pillar * 6.2831855f / 16.0f + spin;
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
        float h = 10.0f;
        float du = 0.25f, dv = 0.5f;

        for (n_cell = 0; n_cell < 8; n_cell++) {
            float u = du * (float)(n_cell % 4);
            float tv = dv * (float)(n_cell / 4);

            fx->spark_uv[n_cell][0].u = u;
            fx->spark_uv[n_cell][0].v = tv;
            fx->spark_uv[n_cell][0].x = -h;
            fx->spark_uv[n_cell][0].y = -h;
            fx->spark_uv[n_cell][0].z = 0.0f;
            fx->spark_uv[n_cell][1].u = u + du;
            fx->spark_uv[n_cell][1].v = tv + dv;
            fx->spark_uv[n_cell][1].x = h;
            fx->spark_uv[n_cell][1].y = h;
            fx->spark_uv[n_cell][1].z = 0.0f;
        }
    }
    for (n_burst = 0; n_burst < step; n_burst++) {
        float ofs = (float)(DD_RAND3(r) % 32) * 0.19634955f;

        for (j = 0; j < 2; j++) {
            DdFxSpark *s = &fx->spark[fx->next_spark];

            s->flags |= 1;
            s->uv = DD_RAND3(r) % 8;
            s->age = 0;
            a = (float)j * 6.2831855f / 2.0f + ofs;
            rad = grow * (29.333334f * (DD_RAND3(r) / 32767.0f));
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

        if (!(s->flags & 1) != 1) {
            list = dl_ambient_light(list, ((unsigned int)(fade * (dd_ease_cos(s->age, 16, 1) * 255.0f)) << 24) | 0xFFFFFF);
            mtx_push();
            s->pos[1] = (float)s->age * 15.0f;
            mtx_translate(s->pos);
            mtx_mult(bb);
            list = draw_array(list, 6, 0x183, 2, 0, fx->spark_uv[s->uv]);
            mtx_pop();
            s->age += step;
            if (s->age >= 16)
                s->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);

    /* floor ring */
    if (state == 1) {
        if (fx->kind == 0) {
            ring_c0 = 0x64C8FF;
            ring_c1 = 0xC864C8FF;
        } else {
            ring_c0 = 0x6464FF;
            ring_c1 = 0xFF6464FF;
        }
        for (k = 0; k < 17; k++) {
            float sa, ca;

            a = (float)k * 6.2831855f / 16.0f;
            sa = sinf(a);
            ca = cosf(a);
            fx->ring[k][0].color = ring_c0;
            fx->ring[k][0].x = sa * 44.333336f;
            fx->ring[k][0].y = 0.0f;
            fx->ring[k][0].z = ca * 44.333336f;
            fx->ring[k][1].color = ring_c1;
            fx->ring[k][1].x = sa * 29.333334f;
            fx->ring[k][1].y = 0.0f;
            fx->ring[k][1].z = ca * 29.333334f;
            fx->ring[k][2].color = ring_c0;
            fx->ring[k][2].x = sa * 9.333334f;
            fx->ring[k][2].y = 0.0f;
            fx->ring[k][2].z = ca * 9.333334f;
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
    for (n_ring = 0; n_ring < 16; n_ring++) {
        list = draw_array(list, 4, 0x99C, 4, &fx->ring_idx[0], fx->ring[n_ring]);
        list = draw_array(list, 4, 0x99C, 4, &fx->ring_idx[0], &fx->ring[n_ring][1]);
    }
    mtx_pop();
    dl_set_list(list);

    /* music notes of the dark kind */
    if (fx->kind == 1) {
        if (state == 1) {
            float hw = 70.0f, h = 200.0f, q = 0.25f, e9 = 0.001953125f;

            for (n_quad = 0; n_quad < 6; n_quad++) {
                float u = (float)(n_quad % 4) * 0.25f;
                float tv = (float)(n_quad / 4) * 0.25f + 0.5f;

                switch (n_quad / 2) {
                case 0:
                    dd_fx_set_vert(&fx->note_quad[n_quad][0], u, tv + q, 0xFF0064, -hw, 0.0f, -h);
                    dd_fx_set_vert(&fx->note_quad[n_quad][1], u, tv + e9, 0xC8FF0064, -hw, 0.0f, 0.0f);
                    dd_fx_set_vert(&fx->note_quad[n_quad][2], u + q, tv + q, 0xFF0064, hw, 0.0f, -h);
                    dd_fx_set_vert(&fx->note_quad[n_quad][3], u + q, tv + e9, 0xC8FF0064, hw, 0.0f, 0.0f);
                    break;
                case 1:
                    dd_fx_set_vert(&fx->note_quad[n_quad][0], u + q, tv, 0xFF0064, -hw, 0.0f, -h);
                    dd_fx_set_vert(&fx->note_quad[n_quad][1], u, tv, 0xC8FF0064, -hw, 0.0f, 0.0f);
                    dd_fx_set_vert(&fx->note_quad[n_quad][2], u + q, tv + q, 0xFF0064, hw, 0.0f, -h);
                    dd_fx_set_vert(&fx->note_quad[n_quad][3], u, tv + q, 0xC8FF0064, hw, 0.0f, 0.0f);
                    break;
                case 2:
                    dd_fx_set_vert(&fx->note_quad[n_quad][0], u, tv, 0xFF0064, -hw, 0.0f, -h);
                    dd_fx_set_vert(&fx->note_quad[n_quad][1], u, tv + q, 0xC8FF0064, -hw, 0.0f, 0.0f);
                    dd_fx_set_vert(&fx->note_quad[n_quad][2], u + q, tv, 0xFF0064, hw, 0.0f, -h);
                    dd_fx_set_vert(&fx->note_quad[n_quad][3], u + q, tv + q, 0xC8FF0064, hw, 0.0f, 0.0f);
                    break;
                }
            }
        }
        for (n_new = 0; n_new < step; n_new++) {
            for (j = 0; j < 4; j++) {
                DdFxNote *n = &fx->note[fx->next_note];

                n->flags |= 1;
                n->kind = DD_RAND3(r) % 6;
                n->age = 0;
                n->ang = DD_RAND3(r) / 32767.0f * 6.2831855f;
                if (fx->next_note < 31)
                    fx->next_note++;
                else
                    fx->next_note = 0;
            }
        }
        list = dl_select_layer(4, 0);
        {
            DdSumVec3 ofs = *(DdSumVec3 *)dd_fx_summon_ofs;

            list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
            list = dd_depth_enable(list, 1, 1);
            list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
            list = dd_tex_pack(list, 11);
            mtx_set_mode(2);
            mtx_push();
            mtx_identity();
            mtx_translate(fx->pos);
            for (n_note = 0; n_note < 32; n_note++) {
                DdFxNote *n = &fx->note[n_note];

                if (!(n->flags & 1) != 1) {
                    float e = dd_ease_sin(n->age, 8, 1);
                    float s[3];

                    s[0] = dd_fx_summon_scale[0];
                    s[1] = dd_fx_summon_scale[1];
                    s[2] = dd_fx_summon_scale[2];
                    s[0] = e;

                    list = dl_ambient_light(list, alpha);
                    mtx_push();
                    mtx_rotate_y(n->ang);
                    mtx_translate(ofs.v);
                    mtx_rotate_x(1.3962635f);
                    mtx_scale3(s);
                    list = draw_array(list, 4, 0x19F, 4, 0, fx->note_quad[n->kind]);
                    mtx_pop();
                    n->age += step;
                    if (n->age >= 8)
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
