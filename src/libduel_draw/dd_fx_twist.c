// @range 0x0882F728-0x08830468
// @asm 0x0882F82C-0x08830468
/* Effect: a card mesh (17x25 grid) twisted into a vortex that turns faster towards the middle,
   with a flash under it and sparks drawn in towards the centre; 32 frames. */
#define DD_FX
#include "libduel_draw.h"

typedef struct DdTwistSpark {          /* 0x24 */
    int age;
    unsigned int flags;
    int uv;
    float from[3];
    float to[3];
} DdTwistSpark;

typedef struct DdFxTwist {             /* 0x7050 */
    int frame;                         /* +0x0000 */
    int tick;                          /* +0x0004 picks one of the two vertex copies */
    int card;                          /* +0x0008 */
    float pos[3];                      /* +0x000C */
    float rot[3];                      /* +0x0018 */
    float scale[3];                    /* +0x0024 */
    float t;                           /* +0x0030 texture zoom */
    unsigned int se;                   /* +0x0034 */
    DdTexVert grid[25][17];            /* +0x0038 */
    DdTexVert copy[2][25][17];         /* +0x216C copy handed to the GE (double buffered) */
    unsigned char idx[34];             /* +0x63D4 strip of two rows */
    float ang[25][17];                 /* +0x63F8 turn per frame of each vertex */
    DdFxVert flash[6];                 /* +0x6A9C */
    int next;                          /* +0x6B2C */
    DdTexVert spark_uv[4][2];          /* +0x6B30 */
    DdTwistSpark spark[32];            /* +0x6BD0 */
} DdFxTwist;

float *mat3_rot_y(float *out, float *m, float a);
void *memcpy(void *, const void *, size_t);

int dd_fx_twist_update(int state, DdFxTwist *fx);

int dd_fx_twist_start(float s, int card, float *pos, float *rot, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_twist_update, term, arg, sizeof(DdFxTwist));
    DdFxTwist *fx;

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

int dd_fx_twist_start_t(float s, float t, int card, float *pos, float *rot, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_twist_start(s, card, pos, rot, term, arg);

    if (id != -1)
        ((DdFxTwist *)dd_task_mem(id))->t = t;
    return id;
}

#ifdef NON_MATCHING /* função grande (3,1 KB) */
int dd_fx_twist_update(int state, DdFxTwist *fx)
{
    DdRandSeed *r = &rand3_seeds[rand3_slot];
    int buf = fx->tick % 2;
    int step = dd_anim_step();
    float bb[18];
    float a[3], b[3], v[3], m3[9];
    unsigned int *list;
    float fade, s, e, d2;
    int i, j;

    dd_mtx_billboard_view(bb);
    fade = dd_ease_cos(fx->frame, 32, 3);
    dd_cue_se(0x6246, fx->frame, 0, &fx->se, 1);
    dd_cue_se(0x624A, fx->frame, 0, &fx->se, 2);
    if (state == 1) {
        for (i = 0; i < 25; i++) {
            for (j = 0; j < 17; j++) {
                DdTexVert *g = &fx->grid[i][j];

                g->u = (float)j * 0.984375f / 16.0f - 0.4921875f;
                g->v = (float)i * 0.984375f / 24.0f - 0.4921875f;
                g->x = (float)j * 62.0f / 16.0f + -31.0f;
                g->y = 0.0f;
                g->z = (float)i * 88.0f / 24.0f + -44.0f;
            }
        }
        for (j = 0; j < 17; j++) {
            fx->idx[j * 2] = j;
            fx->idx[j * 2 + 1] = j + 17;
        }
        for (i = 0; i < 25; i++) {
            for (j = 0; j < 17; j++) {
                float p[3] = { fx->grid[i][j].x, 0.0f, fx->grid[i][j].z };

                d2 = p[0] * p[0] + p[2] * p[2];
                if (d2 > 2897.0f)
                    d2 = 2897.0f;
                fx->ang[i][j] = (float)step * ((2897.0f - d2) * -0.13962634f / 2897.0f + -0.13962634f);
            }
        }
    }

    /* the twisted card */
    list = dl_select_layer(4, 0);
    list = dd_depth_enable(list, 1, 1);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    mtx_rotate_zyx(fx->rot);
    mtx_scale3(fx->scale);
    list = dl_ambient_light(list, ((unsigned int)(fade * 255.0f) << 24) | 0xFFFFFF);
    list = dd_tex_card(list, fx->card, 1);
    s = fx->t / 64.0f + 1.0f;
    list = dd_tex_enable_uv(list, 1, 1, 1, 0, 0, 1, 1, 0.5f, 0.5f, s, s);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dl_color(list, 0xFFFFFFFF);
    memcpy(fx->copy[buf], fx->grid, sizeof(fx->grid));
    for (i = 0; i < 24; i++)
        list = draw_array(list, 4, 0x983, 34, fx->idx, fx->copy[buf][i]);
    for (i = 0; i < 25; i++) {
        for (j = 0; j < 17; j++) {
            DdTexVert *g = &fx->grid[i][j];

            vec3_set(v, g->x, 0.0f, g->z);
            mat3_identity(m3);
            mat3_rot_y(m3, m3, fx->ang[i][j]);
            mat3_transform_vec3(v, m3, v);
            g->x = v[0];
            g->z = v[2];
        }
    }
    mtx_pop();
    dl_set_list(list);

    /* the flash */
    if (state == 1) {
        static const float fv[6][5] = {
            { 0.5f, 0.5f, 0.0f, 0.0f }, { 1.0f, 0.0f, -80.0f, -80.0f }, { 0.0f, 0.0f, 80.0f, -80.0f },
            { 0.0f, 1.0f, 80.0f, 80.0f }, { 1.0f, 1.0f, -80.0f, 80.0f }, { 1.0f, 0.0f, -80.0f, -80.0f },
        };

        for (i = 0; i < 6; i++) {
            fx->flash[i].u = fv[i][0];
            fx->flash[i].v = fv[i][1];
            fx->flash[i].color = i == 0 ? 0xFFFFFFFF : 0xFFFFFF;
            fx->flash[i].x = fv[i][2];
            fx->flash[i].y = 0.0f;
            fx->flash[i].z = fv[i][3];
        }
    }
    list = dl_select_layer(4, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 18);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    mtx_rotate_zyx(fx->rot);
    mtx_scale3(fx->scale);
    mtx_rotate_y((float)fx->frame * -0.08726647f);
    list = dl_ambient_light(list, ((unsigned int)(fade * dd_ease_lin(fx->frame, 8) * 255.0f) << 24) | 0xFFFFFF);
    list = draw_array(list, 5, 0x19F, 6, 0, fx->flash);
    mtx_pop();
    dl_set_list(list);

    /* sparks drawn in */
    if (state == 1) {
        for (i = 0; i < 4; i++) {
            float u = (float)(i % 4) * 0.25f;
            float tv = (float)(i / 4) * 0.5f;

            fx->spark_uv[i][0].u = u;
            fx->spark_uv[i][0].v = tv;
            fx->spark_uv[i][0].x = -10.0f;
            fx->spark_uv[i][0].y = 10.0f;
            fx->spark_uv[i][0].z = 0.0f;
            fx->spark_uv[i][1].u = u + 0.25f;
            fx->spark_uv[i][1].v = tv + 0.5f;
            fx->spark_uv[i][1].x = 10.0f;
            fx->spark_uv[i][1].y = -10.0f;
            fx->spark_uv[i][1].z = 0.0f;
        }
    }
    if (fx->frame < 16) {
        for (i = 0; i < step; i++) {
            for (j = 0; j < 2; j++) {
                DdTwistSpark *sp = &fx->spark[fx->next];
                float an = DD_RAND(r) / 32767.0f * 6.2831855f;
                float sa = sinf(an), ca = cosf(an);

                sp->age = 0;
                sp->uv = DD_RAND(r) % 4;
                sp->flags |= 1;
                vec3_set(sp->from, sa * 130.0f, 10.0f, ca * 130.0f);
                vec3_set(sp->to, sa, 0.0f, ca);
                if (fx->next < 31)
                    fx->next++;
                else
                    fx->next = 0;
            }
        }
    }
    list = dl_select_layer(4, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 13);
    list = dl_color(list, 0xFFFFFFFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    mtx_scale3(fx->scale);
    for (i = 0; i < 32; i++) {
        DdTwistSpark *sp = &fx->spark[i];

        if (sp->flags & 1) {
            e = dd_ease_lin(sp->age, 16);
            s = fade * dd_ease_sin(sp->age, 16, 1);
            mat3_identity(m3);
            mat3_rot_y(m3, m3, (float)sp->age * -0.08726647f);
            mat3_from_euler(m3, m3, fx->rot);
            mat3_transform_vec3(a, m3, sp->from);
            mat3_transform_vec3(b, m3, sp->to);
            vec3_sub(b, b, a);
            vec3_scale(b, b, e);
            vec3_add(b, b, a);
            list = dl_ambient_light(list, ((unsigned int)(s * 255.0f) << 24) | 0xFFFFFF);
            mtx_push();
            mtx_translate(b);
            mtx_mult(bb);
            list = draw_array(list, 6, 0x183, 2, 0, fx->spark_uv[sp->uv]);
            mtx_pop();
            sp->age += step;
            if (sp->age > 15)
                sp->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    fx->tick++;
    fx->frame += step;
    if (fx->frame < 32)
        return 2;
    return 4;
}
#endif
