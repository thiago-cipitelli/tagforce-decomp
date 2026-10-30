// @range 0x0882ED38-0x0882F728
// @asm 0x0882EE8C-0x0882F728
/* Effect: a card fading out while sparks spiral up from it and a glowing cylinder of light turns
   around it; 32 frames. */
#define DD_FX
#include "libduel_draw.h"

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

int dd_fx_spiral_start_ex(float s, float t, int card, float *pos, float *rot, DdTaskTermFn term, void *arg)
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

#ifdef NON_MATCHING /* função grande (2,2 KB) */
int dd_fx_spiral_update(int state, DdFxSpiral *fx)
{
    DdRandSeed *r = &rand3_seeds[rand3_slot];
    int step = dd_anim_step();
    float bb[17];
    float sc[3], p[3], m3[9];
    unsigned int *list;
    float fade, e, sz, a;
    int i, j;

    dd_mtx_billboard_view(bb);
    fade = dd_ease_cos(fx->frame, 32, 1);
    dd_cue_se(0x624A, fx->frame, 0, &fx->se, 1);
    dd_card_quad_draw(fx->card, fx->pos, fx->rot, fx->scale,
                      (unsigned int)((1.0f - dd_ease_lin(fx->frame, 32)) * 255.0f), fx->t);
    if (state == 1) {
        for (i = 0; i < 4; i++) {
            float u = (float)(i % 2) * 0.5f;
            float tv = (float)(i / 2) * 0.5f;

            fx->spark_uv[i][0].u = u;
            fx->spark_uv[i][0].v = tv;
            fx->spark_uv[i][0].x = -40.0f;
            fx->spark_uv[i][0].y = 40.0f;
            fx->spark_uv[i][0].z = 0.0f;
            fx->spark_uv[i][1].u = u + 0.5f;
            fx->spark_uv[i][1].v = tv + 0.5f;
            fx->spark_uv[i][1].x = 40.0f;
            fx->spark_uv[i][1].y = -40.0f;
            fx->spark_uv[i][1].z = 0.0f;
        }
    }
    for (i = 0; i < step; i++) {
        for (j = 0; j < 4; j++) {
            DdSpiralSpark *s = &fx->spark[fx->next];
            float sa, ca;

            a = (float)j * 6.2831855f / 4.0f + (float)fx->frame * 0.10471976f;
            sa = sinf(a);
            ca = cosf(a);
            s->age = 0;
            s->uv = DD_RAND(r) % 4;
            s->flags |= 1;
            vec3_set(s->base, sa * 5.0f, 0.0f, ca * 5.0f);
            vec3_set(s->dir, sa * 60.0f, 100.0f, ca * 60.0f);
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
    for (i = 0; i < 128; i++) {
        DdSpiralSpark *s = &fx->spark[i];

        if (s->flags & 1) {
            e = dd_ease_sin(s->age, 32, 1);
            sz = e * 0.9f + 0.1f;
            {
                float v[3] = { sz, sz, sz };

                mat3_identity(m3);
                mat3_from_euler(m3, m3, fx->rot);
                vec3_scale(p, s->dir, e);
                vec3_add(p, p, s->base);
                mat3_transform_vec3(p, m3, p);
                list = dl_ambient_light(list, ((unsigned int)(fade * (1.0f - dd_ease_lin(s->age, 32)) * 255.0f) << 24) | 0xFFFFFF);
                mtx_push();
                mtx_translate(p);
                mtx_mult(bb);
                mtx_scale3(v);
            }
            list = draw_array(list, 6, 0x183, 2, 0, fx->spark_uv[s->uv]);
            mtx_pop();
            s->age += step;
            if (s->age > 31)
                s->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    if (state == 1) {
        for (i = 0; i < 17; i++) {
            DdFxVert *v = fx->ring[i];
            float sa, ca, u;

            a = (float)i * 6.2831855f / 16.0f;
            sa = sinf(a);
            ca = cosf(a);
            u = (float)i * 5.0f / 16.0f;
            v[0].u = u;
            v[0].v = 0.0f;
            v[0].color = 0xAAFF;
            v[0].x = sa * 60.0f;
            v[0].y = 80.0f;
            v[0].z = ca * 60.0f;
            v[1].u = u * 0.2f + (u - 1.0f) * 0.8f;
            v[1].v = 0.8f;
            v[1].color = 0xFF00AAFF;
            v[1].x = sa * 40.0f;
            v[1].y = 16.0f;
            v[1].z = ca * 40.0f;
            v[2].u = u - 1.0f;
            v[2].v = 1.0f;
            v[2].color = 0xAAFF;
            v[2].x = sa * 35.0f;
            v[2].y = 0.0f;
            v[2].z = ca * 35.0f;
            fx->ring_idx[i * 2] = i * 3;
            fx->ring_idx[i * 2 + 1] = i * 3 + 1;
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
    list = dl_ambient_light(list, ((unsigned int)(fade * dd_ease_lin(fx->frame, 8) * 255.0f) << 24) | 0xFFFFFF);
    list = draw_array(list, 4, 0x99F, 34, fx->ring_idx, &fx->ring[0][0]);
    list = draw_array(list, 4, 0x99F, 34, fx->ring_idx, &fx->ring[0][1]);
    mtx_pop();
    dl_set_list(list);
    fx->frame += step;
    if (fx->frame < 32)
        return 2;
    return 4;
}
#endif
