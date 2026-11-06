// @range 0x088357B0-0x08837098
// @asm 0x08835954-0x08837098
// @data 0x08876B30-0x08876B54
/* Fusion: the material cards circle around the centre of the screen and spin into a whirlpool
   (a 33x33 polar mesh textured with what was drawn into VRAM) that closes, flashes and opens on
   the resulting card, with sparks drawn inwards; five stages of 8, 60, 60, 16 and 8 frames. */
#define DD_FX
#include "dd_d3.h"

typedef struct DdFusionSpark {         /* 0x48 */
    unsigned int flags;
    int age;
    struct {
        int uv;
        float dir[3];
    } p[4];
} DdFusionSpark;

typedef struct DdFxFusion {            /* 0xB5E8 + 4 * cards */
    int frame;                         /* +0x0000 */
    int tick;                          /* +0x0004 */
    int stage;                         /* +0x0008 0..4 */
    int card;                          /* +0x000C result */
    unsigned int se;                   /* +0x0010 */
    DdTexVert mesh[2][33][33];         /* +0x0014 double buffered */
    unsigned char idx[66];             /* +0xAA3C strip of two rings */
    unsigned char pad[2];
    DdFxVert flash[6];                 /* +0xAA80 behind the whirlpool */
    DdFxVert flash2[6];                /* +0xAB10 stage 2 */
    int next;                          /* +0xABA0 */
    DdTexVert spark_uv[8][2];          /* +0xABA4 */
    DdFusionSpark spark[32];           /* +0xACE4 */
    int n_cards;                       /* +0xB5E4 */
    int cards[1];                      /* +0xB5E8 */
} DdFxFusion;

void *memcpy(void *, const void *, size_t);
float *mat3_rot_z(float *out, float *m, float a);
float *mat4_transform_vec4(FxVec4 *out, float *m, FxVec4 *v);
unsigned int *gfx_set_draw_target(unsigned int *list, int base, unsigned int fbp, int w, int h, int psm);
unsigned int *gfx_set_default_target(unsigned int *list);
unsigned int *card_draw(void *list, int x, int y, unsigned int id);
unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_clear(unsigned int *list, unsigned int flags);
unsigned int *dl_stencil_op(unsigned int *list, int fail, int zfail, int zpass);
unsigned int *dl_stencil_func(unsigned int *list, int func, int ref, int mask);
unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle);
unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr);
unsigned int *dl_tex_flush(unsigned int *list);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
void dd_cam_default(int layer, int ox, int oy);
void dd_cam_end(int layer);
void dd_mtx_mvp_cur(float *out);
void *dd_vram_addr(int offset);
float dd_ease_cos_inv(int t, int n, int k);

int dd_fx_fusion_update(int state, DdFxFusion *fx);

int dd_fx_fusion_start(int n, int *cards, int result, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_fusion_update, term, arg, n * 4 + 0xB5E8);
    DdFxFusion *fx;
    int i;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->n_cards = n;
        for (i = 0; i < n; i++)
            fx->cards[i] = cards[i];
        fx->card = result;
    }
    return id;
}

/* templates of the locals of dd_fx_fusion_update (kept in asm) */
FxVec4 dd_fx_fusion_v4 = { 0.0f, 0.0f, 0.0f, 1.0f };
int dd_fx_fusion_stage_len[5] = { 8, 60, 60, 16, 8 };

#ifdef NON_MATCHING /* função grande (5,8 KB): chamadas na ordem do original (jal batendo, 2026-11-06); falta a forma
   da malha polar (pilha: mvp em sp+112, o FxVec4 copiado do template depois de rad/ang, v calculado antes de u) e
   registradores; ~440 linhas de diferença de estrutura */
inline static void dd_fv(DdFxVert *v, float u, float tv, unsigned int c, float x, float y)
{
    v->u = u;
    v->v = tv;
    v->color = c;
    v->x = x;
    v->y = y;
    v->z = 0.0f;
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

int dd_fx_fusion_update(int state, DdFxFusion *fx)
{
    DdRandSeed *r;
    int buf = fx->tick % 2;
    int step = dd_anim_step();
    float mvp[16], m3[9], p[3];
    unsigned int *list;
    unsigned int a;
    float in, out, k, rad, ang, twist, sc;
    int i, j, n;

    dd_cam_default(6, 0x13A, 0x84);
    in = fx->stage == 0 ? dd_ease_lin(fx->frame, 8) : 1.0f;
    out = fx->stage == 4 ? dd_ease_lin_inv(fx->frame, 8) : 1.0f;
    dd_cue_se(0x6246, fx->frame, 0, &fx->se, 1);

    /* glow behind */
    if (state == 1) {
        float w = 500.0f;

        dd_fv(&fx->flash[0], 0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
        dd_fv(&fx->flash[1], 1.0f, 0.0f, 0xFFFFFF, -w, w);
        dd_fv(&fx->flash[2], 0.0f, 0.0f, 0xFFFFFF, w, w);
        dd_fv(&fx->flash[3], 0.0f, 1.0f, 0xFFFFFF, w, -w);
        dd_fv(&fx->flash[4], 1.0f, 1.0f, 0xFFFFFF, -w, -w);
        dd_fv(&fx->flash[5], 1.0f, 0.0f, 0xFFFFFF, -w, w);
    }
    if (fx->stage < 3) {
        list = dl_select_layer(6, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
        list = dd_tex_pack(list, 18);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_rotate_z((float)fx->tick * -0.043633234f * (float)step);
        a = (unsigned int)(in * 255.0f);
        if (fx->stage == 2)
            a = (unsigned int)((float)a * dd_ease_lin_inv(fx->frame, 60));
        list = dl_ambient_light(list, (a << 24) | 0xFFFFFF);
        list = draw_array(list, 5, 0x19F, 6, 0, fx->flash);
        mtx_pop();
        dl_set_list(list);
    }

    /* the polar mesh: texture coordinates from the screen projection of each vertex */
    if (state == 1) {
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        dd_mtx_mvp_cur(mvp);
        for (i = 0; i < 33; i++) {
            rad = (float)i * 345.0f / 32.0f;
            for (j = 0; j < 33; j++) {
                FxVec4 v = { 0.0f, 0.0f, 0.0f, 1.0f };
                DdTexVert *m = &fx->mesh[0][i][j];

                ang = (float)j * 6.2831855f / 32.0f;
                v.x = rad * sinf(ang);
                v.y = rad * cosf(ang);
                mat4_transform_vec4(&v, mvp, &v);
                m->u = (v.x * 240.0f / v.w + 2048.0f - 240.0f + 314.0f - 1808.0f) / 512.0f;
                m->v = (v.y * -136.0f / v.w + 2048.0f - 136.0f + 132.0f - 1912.0f) / 512.0f;
                m->x = 0.0f;
                m->y = 0.0f;
                m->z = 0.0f;
            }
        }
        memcpy(fx->mesh[1], fx->mesh[0], sizeof(fx->mesh[0]));
        mtx_pop();
        for (j = 0; j < 33; j++) {
            fx->idx[j * 2] = j;
            fx->idx[j * 2 + 1] = j + 33;
        }
    }

    /* render the cards (or the result) into VRAM at 0x174000 */
    list = dl_select_layer(6, 0);
    list = gfx_set_draw_target(list, 0x174000, 0xFFFFFFFF, 512, 512, 3);
    list = dl_clear_color(list, 0);
    list = dl_clear_depth(list, 0);
    list = dl_clear(list, 0x13);
    list = dd_depth_enable(list, 0, 1);
    list = dl_enable(list, 3);
    list = dl_stencil_op(list, 1, 0xFF, 0xFF);
    list = dl_stencil_func(list, 2, 2, 2);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dl_ambient_light(list, 0xFFFFFFFF);
    list = dl_color(list, 0xFFFFFFFF);
    list = draw_set_offsets(list, 64, 64, 64, 90);
    ang = fx->n_cards < 3 ? 1.5707964f : 3.1415927f;
    if (fx->stage == 1)
        rad = fx->n_cards == 1 ? 0.0f : dd_ease_cos(fx->frame, 60, 1) * 75.0f;
    else if (fx->stage == 0)
        rad = fx->n_cards == 1 ? 0.0f : 75.0f;
    else
        rad = 0.0f;
    if (fx->stage < 2) {
        for (i = 0; i < fx->n_cards; i++) {
            float an = ang + -6.2831855f * (float)i / (float)fx->n_cards;
            int x = 314.0f + rad * sinf(an) - 32.0f;
            int y = 132.0f + rad * cosf(an) - 45.419353f;

            list = dd_tex_card(list, fx->cards[i], 1);
            list = draw_sprite(list, x, y, 0, 0, 0, 0, 0);
        }
    } else {
        list = card_draw(list, 0x104, 0x37, fx->card);
    }
    list = dl_disable(list, 3);
    list = gfx_set_default_target(list);

    /* the whirlpool */
    {
        void *vram = dd_vram_addr(0x174000);

        list = dl_tex_flush(list);
        list = dl_tex_mode(list, 3, 0, 0, 0);
        list = dl_tex_image(list, 0, 512, 512, 512, vram);
    }
    list = dd_depth_enable(list, 0, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dd_tex_enable(list, 1, 1, 1, 4, 1, 1, 1);
    switch (fx->stage) {
    case 1:
        k = dd_ease_cos_inv(fx->frame, 60, 1);
        break;
    case 2:
        k = dd_ease_sin_inv(fx->frame, 60, 1);
        break;
    default:
        k = 0.0f;
        break;
    }
    a = (unsigned int)(k * 255.0f);
    list = dl_color(list, a | a << 16 | 0xFF000000 | a << 8);
    for (i = 0; i < 33; i++) {
        rad = (float)i * 345.0f / 32.0f;
        switch (fx->stage) {
        case 1:
            twist = (0.13962634f + 0.17453294f * (float)(32 - i) / 32.0f) * (float)fx->frame
                    * dd_ease_cos_inv(fx->frame, 60, 1);
            break;
        case 2:
            twist = 60.0f * (0.17453294f * (float)(32 - i) / 32.0f);
            twist += (18.849556f - twist) * dd_ease_sin(fx->frame, 60, 1);
            break;
        default:
            twist = 0.0f;
            break;
        }
        for (j = 0; j < 33; j++) {
            DdTexVert *m = &fx->mesh[buf][i][j];

            ang = (float)j * 6.2831855f / 32.0f + twist;
            m->x = rad * sinf(ang);
            m->y = rad * cosf(ang);
        }
    }
    list = dl_ambient_light(list, ((unsigned int)(in * 255.0f * out) << 24) | 0xFFFFFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    switch (fx->stage) {
    case 1:
        sc = 0.3f + 0.7f * dd_ease_cos(fx->frame, 60, 3);
        break;
    case 2:
        sc = dd_ease_sin(fx->frame, 60, 3);
        break;
    default:
        sc = 1.0f;
        break;
    }
    dd_scale3(sc, sc, 0.0f);
    for (i = 0; i < 32; i++)
        list = draw_array(list, 4, 0x983, 66, fx->idx, fx->mesh[buf][i]);
    mtx_pop();
    dl_set_list(list);

    /* the flash of stage 2 */
    if (state == 1) {
        float w = 400.0f;

        dd_fv(&fx->flash2[0], 0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
        dd_fv(&fx->flash2[1], 1.0f, 0.0f, 0xFFFFFF, -w, -w);
        dd_fv(&fx->flash2[2], 0.0f, 0.0f, 0xFFFFFF, w, -w);
        dd_fv(&fx->flash2[3], 0.0f, 1.0f, 0xFFFFFF, w, w);
        dd_fv(&fx->flash2[4], 1.0f, 1.0f, 0xFFFFFF, -w, w);
        dd_fv(&fx->flash2[5], 1.0f, 0.0f, 0xFFFFFF, -w, -w);
    }
    if (fx->stage == 2) {
        list = dl_select_layer(6, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
        list = dd_tex_pack(list, 3);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        list = dl_ambient_light(list, ((unsigned int)(dd_ease_cos(fx->frame, 32, 1) * 255.0f) << 24) | 0xFFFFFF);
        list = draw_array(list, 5, 0x19F, 6, 0, fx->flash2);
        mtx_pop();
        dl_set_list(list);
    }

    /* sparks drawn in */
    if (state == 1) {
        float h = 30.0f;
        float du = 0.25f, dv = 0.5f;

        for (i = 0; i < 8; i++) {
            float u = (float)(i % 4) * du;
            float tv = (float)(i / 4) * dv;

            dd_tv(&fx->spark_uv[i][0], u, tv, -h, h);
            dd_tv(&fx->spark_uv[i][1], u + du, tv + dv, h, -h);
        }
    }
    if (fx->stage == 0 || (fx->stage == 1 && fx->frame < 44)) {
        for (i = 0; i < step; i++) {
            DdFusionSpark *s = &fx->spark[fx->next];

            s->flags |= 1;
            s->age = 0;
            for (j = 0; j < 4; j++) {
                float an = 6.2831855f * (DD_RAND3(r) / 32767.0f);

                s->p[j].uv = DD_RAND3(r) % 8;
                vec3_set(s->p[j].dir, 400.0f * sinf(an), 400.0f * cosf(an), 0.0f);
            }
            if (fx->next < 31)
                fx->next++;
            else
                fx->next = 0;
        }
    }
    if (fx->stage < 2) {
        list = dl_select_layer(6, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
        list = dd_tex_pack(list, 13);
        list = dl_color(list, 0xFFFFFFFF);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        if (fx->stage != 0)
            in *= dd_ease_lin_inv(fx->frame - 44, 16);
        for (i = 0; i < 32; i++) {
            DdFusionSpark *s = &fx->spark[i];

            if (s->flags & 1) {
                float e = dd_ease_sin_inv(s->age, 32, 2);

                k = in * dd_ease_sin(s->age, 8, 1);
                mat3_identity(m3);
                mat3_rot_z(m3, m3, (float)s->age * -0.08726647f);
                list = dl_ambient_light(list, ((unsigned int)(k * 255.0f) << 24) | 0xFFFFFF);
                for (j = 0; j < 4; j++) {
                    mtx_push();
                    vec3_scale(p, s->p[j].dir, e);
                    mat3_transform_vec3(p, m3, p);
                    mtx_translate(p);
                    list = draw_array(list, 6, 0x183, 2, 0, fx->spark_uv[s->p[j].uv]);
                    mtx_pop();
                }
                s->age += step;
                if (s->age > 31)
                    s->flags &= ~1;
            }
        }
        mtx_pop();
        dl_set_list(list);
    }
    dd_cam_end(6);
    fx->tick++;
    {
        int len[5] = { 8, 60, 60, 16, 8 };
        int last = len[fx->stage] - 1;

        if (fx->frame < last) {
            fx->frame = dd_step_clamp(fx->frame, step, last);
        } else {
            if (fx->stage >= 4)
                return 4;
            fx->stage++;
            fx->frame = 0;
        }
    }
    return 2;
}
#endif
