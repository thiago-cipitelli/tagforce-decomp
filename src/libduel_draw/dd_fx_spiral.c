// @range 0x0882ED38-0x0882F728
/* Effect: a card fading out while sparks spiral up from it and a glowing cylinder of light turns
   around it; 32 frames. */
#define DD_FX
#include "dd_d3.h"

typedef struct DdSpiralSpark {         /* 0x24 */
    int age;
    unsigned int flags;                /* bit 0: alive */
    int uv;
    float base[3];
    float dir[3];
} DdSpiralSpark;

typedef struct DdFxSpiral {            /* 0x17C4 */
    int frame;                         /* +0x000 */
    int card;                          /* +0x004 */
    float pos[3];                      /* +0x008 */
    float rot[3];                      /* +0x014 */
    float scale[3];                    /* +0x020 */
    float t;                           /* +0x02C texture zoom */
    unsigned int se;                   /* +0x030 */
    int next;                          /* +0x034 */
    DdTexVert spark_uv[4][2];          /* +0x038 */
    DdSpiralSpark spark[128];          /* +0x0D8 */
    DdFxVert ring[17][3];              /* +0x12D8 top, middle, bottom of each spoke */
    unsigned char ring_idx[34];        /* +0x17A0 */
} DdFxSpiral;

int dd_fx_spiral_update(int state, DdFxSpiral *fx);

int dd_fx_spiral_start(float ry, int card, float *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_spiral_update, term, arg, sizeof(DdFxSpiral));
    DdFxSpiral *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->card = card;
        vec3_copy(fx->pos, pos);
        vec3_set(fx->rot, 0.0f, ry, 0.0f);
        vec3_set(fx->scale, 1.0f, 1.0f, 1.0f);
        fx->t = 0.0f;
    }
    return id;
}

int dd_fx_spiral_start_ex(int card, float *pos, float s, float t, float *rot, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_spiral_start(0.0f, card, pos, term, arg);
    DdFxSpiral *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        vec3_copy(fx->rot, rot);
        vec3_set(fx->scale, s, s, s);
        fx->t = t;
    }
    return id;
}

#define DD_RAND3(r) ((r) = &rand3_seeds[rand3_slot], (r)->lcg *= 0x41C64E6D, (r)->lcg += 0x3039, \
                     (int)((r)->lcg >> 16 & 0x7FFF))

inline static void dd_tv(DdTexVert *v, float u, float tv, float x, float y)
{
    v->u = u;
    v->v = tv;
    v->x = x;
    v->y = y;
    v->z = 0.0f;
}

int dd_fx_spiral_update(int state, DdFxSpiral *fx)
{
    DdRandSeed *r;
    int step = dd_anim_step();
    float m3[9], p[3], v[3];
    float bb[17];
    unsigned int *list;
    float fade, e, sz, a;
    int i;
    int n, k;

    dd_mtx_billboard_view(bb);
    fade = dd_ease_cos(fx->frame, 32, 1);
    dd_cue_se(0x624A, fx->frame, 0, &fx->se, 1);
    dd_card_quad_draw(fx->card, fx->pos, fx->rot, fx->scale,
                      (unsigned int)((1.0f - dd_ease_lin(fx->frame, 32)) * 255.0f), fx->t);
    if (state == 1) {
        float s = 40.0f;
        float h = 0.5f;

        for (i = 0; i < 4; i++) {
            float u = h * (float)(i % 2);
            float tv = h * (float)(i / 2);

            dd_tv(&fx->spark_uv[i][0], u, tv, -s, s);
            dd_tv(&fx->spark_uv[i][1], u + h, tv + h, s, -s);
        }
    }
    for (n = 0; n < step; n++) {
        int j;

        for (j = 0; j < 4; j++) {
            DdSpiralSpark *sp = &fx->spark[fx->next];
            float sa, ca;

            a = (float)j * 6.2831855f / 4.0f + (float)fx->frame * 0.10471976f;
            sa = sinf(a);
            ca = cosf(a);
            sp->age = 0;
            sp->uv = DD_RAND3(r) % 4;
            sp->flags |= 1;
            vec3_set(sp->base, sa * 5.0f, 0.0f, ca * 5.0f);
            vec3_set(sp->dir, sa * 60.0f, 100.0f, ca * 60.0f);
            if (fx->next < 127)
                fx->next++;
            else
                fx->next = 0;
        }
    }
    list = dl_select_layer(4, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 10);
    list = dl_color(list, 0xC850C8FF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    mtx_scale3(fx->scale);
    for (k = 0; k < 128; k++) {
        DdSpiralSpark *sp = &fx->spark[k];

        if (!(sp->flags & 1) != 1) {
            unsigned int c;

            e = dd_ease_sin(sp->age, 32, 1);
            sz = 0.1f + 0.9f * e;
            c = fade * ((1.0f - dd_ease_lin(sp->age, 32)) * 255.0f);
            {
                unsigned char *d = (unsigned char *)v;
                int k = 12;

                if (d)
                    do {
                        *d++ = 0;
                    } while (--k);
            }
            v[0] = sz;
            v[1] = sz;
            v[2] = sz;
            mat3_identity(m3);
            mat3_from_euler(m3, m3, fx->rot);
            vec3_scale(p, sp->dir, e);
            vec3_add(p, p, sp->base);
            mat3_transform_vec3(p, m3, p);
            list = dl_ambient_light(list, (c << 24) | 0xFFFFFF);
            mtx_push();
            mtx_translate(p);
            mtx_mult(bb);
            mtx_scale3(v);
            list = draw_array(list, 6, 0x183, 2, 0, fx->spark_uv[sp->uv]);
            mtx_pop();
            sp->age += step;
            if (sp->age >= 32)
                sp->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    if (state == 1) {
        int k;

        for (k = 0; k < 17; k++) {
            float sa, ca, u, u1;
            float m = 0.8f, l = 0.2f;

            a = (float)k * 6.2831855f / 16.0f;
            sa = sinf(a);
            ca = cosf(a);
            u = (float)k * 5.0f / 16.0f;
            u1 = u - 1.0f;
            fx->ring[k][0].u = u;
            fx->ring[k][0].v = 0.0f;
            fx->ring[k][0].color = 0xAAFF;
            fx->ring[k][0].x = sa * 60.0f;
            fx->ring[k][0].y = 80.0f;
            fx->ring[k][0].z = ca * 60.0f;
            fx->ring[k][1].u = u * l + u1 * m;
            fx->ring[k][1].v = m;
            fx->ring[k][1].color = 0xFF00AAFF;
            fx->ring[k][1].x = sa * 40.0f;
            fx->ring[k][1].y = 16.0f;
            fx->ring[k][1].z = ca * 40.0f;
            fx->ring[k][2].u = u1;
            fx->ring[k][2].v = 1.0f;
            fx->ring[k][2].color = 0xAAFF;
            fx->ring[k][2].x = sa * 35.0f;
            fx->ring[k][2].y = 0.0f;
            fx->ring[k][2].z = ca * 35.0f;
            fx->ring_idx[k * 2] = k * 3;
            fx->ring_idx[k * 2 + 1] = k * 3 + 1;
        }
    }
    list = dl_select_layer(4, 0);
    list = dd_tex_enable_ofs(list, 1, 1, 1, 0, 0, 0, 0, 0.0f, (float)fx->frame * 0.05f);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 12);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    mtx_rotate_zyx(fx->rot);
    mtx_scale3(fx->scale);
    mtx_rotate_y((float)fx->frame * 0.08726647f);
    list = dl_ambient_light(list, ((unsigned int)(fade * (255.0f * dd_ease_lin(fx->frame, 8))) << 24) | 0xFFFFFF);
    list = draw_array(list, 4, 0x99F, 34, fx->ring_idx, &fx->ring[0][0]);
    list = draw_array(list, 4, 0x99F, 34, fx->ring_idx, &fx->ring[0][1]);
    mtx_pop();
    dl_set_list(list);
    fx->frame += step;
    if (fx->frame < 32)
        return 2;
    return 4;
}
