// @range 0x08849060-0x0884A86C
// @asm 0x088492F4-0x088494D0
// @asm 0x088494D0-0x0884A86C
/* Card effects "hit sound" (effects 204-376: only the hit sounds and the character's reaction)
   and "body glow" (effect 18): orange glows (orang_light.gim) projected on the character's
   bones, pulsing with a random phase each, faded in and out. */
#include "libduel_draw.h"

void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_color(void *list, unsigned int color);
void *dl_ambient_light(void *list, unsigned int color);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(FxVec3 *v);
void mtx_mult(FxMat4 *m);
void mtx_get34(FxMat4 *m);
void mat4_mul(FxMat4 *r, FxMat4 *a, FxMat4 *b);
FxMat4 *mat4_copy(FxMat4 *out, FxMat4 *m);
FxVec4 *mat4_transform_vec4(FxVec4 *out, FxMat4 *m, FxVec4 *v);
FxVec4 *vec4_set(FxVec4 *out, float x, float y, float z, float w);
FxVec3 *vec3_lerp(FxVec3 *out, FxVec3 *a, FxVec3 *b, float t);
float sinf(float x);

float func_088279A4(int t, int len);
void func_08827738(void);
void func_08827668(void);
void func_08827258(FxMat4 *out);
void func_08827ED4(int bone, FxVec3 *out);
int func_08827F68(int se, int t, int at, unsigned int *flags, unsigned int bit);
int func_0882623C(void *update, void *a, void *b, int size);
void *func_08826440(int task);
void *func_0882698C(void *list, int on, int mask);
void *func_088269D8(void *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *func_08826A64(void *list, int tex);
void *func_088266D8(void *list, int on, int min, int mag, int func, int comp, int wrapu, int wrapv);
FxMat4 *dd_chara_view(void);
FxMat4 *dd_chara_proj(void);
void dd_chara_set_08813FB4(int arg);
void dd_fx_hit2_times(int id, int *delay, int *hit, int *len);

typedef struct RandSeed {              /* BOOT rand3_seeds entry, 12 bytes */
    unsigned int lcg;
    unsigned int b, c;
} RandSeed;
extern int rand3_slot;                 /* BOOT variable imports, 0 in the relocated ELF */
extern RandSeed rand3_seeds[];

#define RAND15() (rand3_seeds[rand3_slot].lcg *= 0x41C64E6D, rand3_seeds[rand3_slot].lcg += 0x3039, \
                  (int)(rand3_seeds[rand3_slot].lcg >> 16 & 0x7FFF))
#define RAND_ANGLE() (RAND15() / 32767.0f * 6.2831855f)

typedef struct DdFxHitSe {             /* 0x24 */
    int wait;                          /* +0x00 */
    int delay;                         /* +0x04 */
    int hit;                           /* +0x08 */
    int t;                             /* +0x0C */
    int len;                           /* +0x10 */
    int hit_done;                      /* +0x14 */
    DdFxArg *arg;                      /* +0x18 */
    unsigned int se_done;              /* +0x1C */
    int se;                            /* +0x20 */
} DdFxHitSe;

typedef struct DdFxGlow {              /* 0x200 */
    int wait;                          /* +0x000 */
    int delay;                         /* +0x004 */
    int t;                             /* +0x008 */
    int len;                           /* +0x00C < 0: until stopped */
    int fade_in;                       /* +0x010 */
    int frame;                         /* +0x014 */
    DdFxArg *arg;                      /* +0x018 */
    int pad1c;
    FxVec4 rect[5][2];                 /* +0x020 corners of the sprites of size 5..25 */
    float ph_limb[4][2][8];            /* +0x0C0 pulse phases */
    float ph_spine[8];                 /* +0x1C0 */
    float ph_head[2];                  /* +0x1E0 */
    float ph_arm[2][2];                /* +0x1E8 */
    float ph_core;                     /* +0x1F8 */
} DdFxGlow;

int dd_fx_hit_se_update(int phase, DdFxHitSe *w);
int dd_fx_glow_update(int phase, DdFxGlow *w);

int dd_fx_hit_se_start(DdFxArg *arg, void *a, void *b)
{
    int task = func_0882623C(dd_fx_hit_se_update, a, b, sizeof(DdFxHitSe));

    if (task != -1)
        ((DdFxHitSe *)func_08826440(task))->arg = arg;
    return task;
}

int dd_fx_hit_se_update(int phase, DdFxHitSe *w)
{
    DdFxArg *arg = w->arg;
    int run = arg->run;

    if (phase == 1) {
        dd_fx_hit2_times(arg->id, &w->delay, &w->hit, &w->len);
        switch (((int *)arg)[2]) {
        case 12:
            w->se = 0x6239;
            break;
        default:
            w->se = 0x623A;
            break;
        }
    }
    func_08827F68(w->se, w->wait, w->delay, &w->se_done, 1);
    if (w->wait == w->hit && w->hit_done == 0) {
        w->hit_done = 1;
        dd_chara_set_08813FB4(0);
    }
    if (run == 1) {
        if (w->wait < w->hit)
            w->wait++;
        else
            w->t++;
    }
    if (w->t < w->len)
        return 2;
    return 4;
}

/* Times of the glow: effect 18 on duelist 0, by the frame of the duel animation. */
void dd_fx_glow_times(int id, int who, int f, int *delay, int *len, int *fade_in)
{
    switch (id) {
    case 18:
        if (who == 0) {
            if (f >= 247 && f <= 289) {
                *delay = 120;
                *len = -1;
                *fade_in = 1;
            } else if ((f >= 204 && f <= 246) || (f >= 290 && f <= 333) || (f >= 334 && f <= 376)) {
                *delay = 0;
                *len = 30;
                *fade_in = 0;
            } else {
                *delay = 0;
                *len = -1;
                *fade_in = 0;
            }
        } else {
            *delay = 0;
            *len = 0;
            *fade_in = 0;
        }
        break;
    default:
        *delay = 0;
        *len = 0;
        *fade_in = 0;
        break;
    }
}

int dd_fx_glow_start(DdFxArg *arg, void *a, void *b)
{
    int task = func_0882623C(dd_fx_glow_update, a, b, sizeof(DdFxGlow));

    if (task != -1)
        ((DdFxGlow *)func_08826440(task))->arg = arg;
    return task;
}

#ifdef NON_MATCHING /* só o fim difere: o original testa x[0] < x[1] com slt at/beqzl direto ao epílogo e y com bnezl; as formas testadas invertem um dos dois */
/* Draws the glow sprite `rect` at `pos` in screen space (skipped when off screen). */
void *dd_fx_glow_sprite(void *list, FxVec3 *pos, int alpha, FxVec4 *rect, FxMat4 *cam, FxMat4 *vp)
{
    FxMat4 m;
    FxVec4 c[2];
    int x[2];
    int y[2];
    void *r;
    int i;

    r = dl_color(list, alpha << 24 | 0x9BC8FF);
    mtx_push();
    mtx_translate(pos);
    mtx_mult(cam);
    mtx_get34(&m);
    mat4_mul(&m, vp, &m);
    mtx_pop();
    for (i = 0; i < 2; i++) {
        mat4_transform_vec4(&c[i], &m, &rect[i]);
        x[i] = c[i].x * 240.0f / c[i].w + 2048.0f;
        y[i] = c[i].y * -136.0f / c[i].w + 2048.0f;
        if (x[i] < 0 || x[i] > 4096 || y[i] < 0 || y[i] > 4096)
            return r;
        x[i] -= 0x710;
        y[i] -= 0x778;
    }
    if (x[0] >= x[1])
        return r;
    if (y[0] >= y[1])
        return r;
    r = draw_set_offsets(r, 16, 16, x[1] - x[0], y[1] - y[0]);
    return draw_sprite(r, x[0], y[0], 0, 40, 8, 0, 0);
}
#endif

#ifdef NON_MATCHING /* função de 5 KB com seis grupos de ossos; escalonamento e alocação de registradores diferentes (1159 palavras); C equivalente */
int dd_fx_glow_update(int phase, DdFxGlow *w)
{
    DdFxArg *arg = w->arg;
    int run = arg->run;
    void *list;
    float in;
    float out;
    float rot;
    unsigned int a150;
    unsigned int a255;
    FxVec4 *rect;
    FxMat4 cam;
    FxMat4 view;
    FxMat4 proj;
    FxMat4 vp;
    FxVec3 q;
    int i;
    int j;
    int k;

    func_08827738();
    if (phase == 1)
        dd_fx_glow_times(((int *)arg)[2], ((int *)arg)[3], arg->id, &w->delay, &w->len, &w->fade_in);
    if (w->fade_in == 1)
        in = func_088279A4(w->t, 8);
    else
        in = 1.0f;
    if (w->len < 0)
        out = 1.0f;
    else
        out = 1.0f - func_088279A4(w->t - (w->len - 16), 16);
    func_08827258(&cam);
    mat4_copy(&proj, dd_chara_proj());
    mat4_copy(&view, dd_chara_view());
    mat4_mul(&vp, &view, &proj);
    if (phase == 1) {
        vec4_set(&w->rect[0][0], -5.0f, 5.0f, 0.0f, 1.0f);
        vec4_set(&w->rect[0][1], 5.0f, -5.0f, 0.0f, 1.0f);
        vec4_set(&w->rect[1][0], -10.0f, 10.0f, 0.0f, 1.0f);
        vec4_set(&w->rect[1][1], 10.0f, -10.0f, 0.0f, 1.0f);
        vec4_set(&w->rect[2][0], -15.0f, 15.0f, 0.0f, 1.0f);
        vec4_set(&w->rect[2][1], 15.0f, -15.0f, 0.0f, 1.0f);
        vec4_set(&w->rect[3][0], -20.0f, 20.0f, 0.0f, 1.0f);
        vec4_set(&w->rect[3][1], 20.0f, -20.0f, 0.0f, 1.0f);
        vec4_set(&w->rect[4][0], -25.0f, 25.0f, 0.0f, 1.0f);
        vec4_set(&w->rect[4][1], 25.0f, -25.0f, 0.0f, 1.0f);
        for (i = 0; i < 4; i++)
            for (j = 0; j < 2; j++)
                for (k = 0; k < 8; k++)
                    w->ph_limb[i][j][k] = RAND_ANGLE();
        for (i = 0; i < 8; i++)
            w->ph_spine[i] = RAND_ANGLE();
        for (i = 0; i < 2; i++)
            w->ph_head[i] = RAND_ANGLE();
        for (i = 0; i < 2; i++)
            for (j = 0; j < 2; j++)
                w->ph_arm[i][j] = RAND_ANGLE();
        w->ph_core = RAND_ANGLE();
    }
    if (w->wait == w->delay) {
        list = dl_select_layer(7, 0);
        a150 = in * 150.0f * out;
        a255 = in * 255.0f * out;
        rot = (float)w->frame * 0.08726647f;
        list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
        list = func_0882698C(list, 1, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
        list = func_08826A64(list, 15);
        list = dl_ambient_light(list, 0xFFFFFFFF);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        {
            int limb_bones[4][3] = { { 14, 15, 16 }, { 17, 18, 19 }, { 20, 21, 22 }, { 23, 24, 25 } };
            FxVec3 limb[4][3];

            for (i = 0; i < 4; i++)
                for (j = 0; j < 3; j++)
                    func_08827ED4(limb_bones[i][j], &limb[i][j]);
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 2; j++) {
                    if (i < 2)
                        rect = j < 1 ? w->rect[2] : w->rect[3];
                    else
                        rect = w->rect[1];
                    for (k = 0; k < 8; k++) {
                        vec3_lerp(&q, &limb[i][j], &limb[i][j + 1], (float)k / 8.0f);
                        list = dd_fx_glow_sprite(list, &q,
                                                 (float)a150 * (sinf(w->ph_limb[i][j][k] + rot) * 0.2f + 0.8f),
                                                 rect, &cam, &vp);
                    }
                }
            }
        }
        {
            int spine_bones[4] = { 22, 25, 16, 19 };
            FxVec3 spine[4];

            for (i = 0; i < 4; i++)
                func_08827ED4(spine_bones[i], &spine[i]);
            vec3_lerp(&spine[0], &spine[0], &spine[1], 0.5f);
            vec3_lerp(&spine[2], &spine[2], &spine[3], 0.5f);
            for (i = 0; i < 8; i++) {
                vec3_lerp(&q, &spine[0], &spine[2], (float)i / 7.0f);
                list = dd_fx_glow_sprite(list, &q, (float)a150 * (sinf(w->ph_spine[i] + rot) * 0.2f + 0.8f),
                                         w->rect[4], &cam, &vp);
            }
        }
        {
            int head_bones[2] = { 22, 25 };
            FxVec3 head[2];

            for (i = 0; i < 2; i++)
                func_08827ED4(head_bones[i], &head[i]);
            for (i = 0; i < 2; i++)
                list = dd_fx_glow_sprite(list, &head[i], (float)a255 * (sinf(w->ph_head[i] + rot) * 0.2f + 0.8f),
                                         w->rect[2], &cam, &vp);
        }
        {
            int arm_bones[2][2] = { { 21, 20 }, { 23, 0 } };
            FxVec3 arm[2][2];
            float f;

            for (i = 0; i < 2; i++)
                for (j = 0; j < 2; j++)
                    func_08827ED4(arm_bones[i][j], &arm[i][j]);
            for (i = 0; i < 2; i++) {
                for (j = 0; j < 2; j++) {
                    if (i == 0)
                        f = (float)(j + 1) * 0.1f / 2.0f + 1.0f;
                    else
                        f = (float)(j + 1) * 0.5f / 2.0f;
                    vec3_lerp(&q, &arm[i][0], &arm[i][1], f);
                    list = dd_fx_glow_sprite(list, &q, (float)a150 * (sinf(w->ph_arm[i][j] + rot) * 0.2f + 0.8f),
                                             w->rect[0], &cam, &vp);
                }
            }
        }
        {
            int core_bones[3] = { 4, 22, 25 };
            FxVec3 core[3];
            FxVec3 c;

            for (i = 0; i < 3; i++)
                func_08827ED4(core_bones[i], &core[i]);
            vec3_lerp(&core[1], &core[1], &core[2], 0.5f);
            vec3_lerp(&c, &core[1], &core[0], 1.1f);
            list = func_088269D8(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
            list = dd_fx_glow_sprite(list, &c, (float)a255 * 0.8f * (sinf(w->ph_core + rot) * 0.2f + 0.8f),
                                     w->rect[3], &cam, &vp);
        }
        mtx_pop();
        dl_set_list(list);
    }
    func_08827668();
    w->frame++;
    if (w->len < 0) {
        if (run == 1) {
            if (w->wait < w->delay)
                w->wait++;
            else if (w->t < 8)
                w->t++;
        }
        return 2;
    }
    if (run == 1) {
        if (w->wait < w->delay)
            w->wait++;
        else
            w->t++;
    }
    if (w->t < w->len)
        return 2;
    return 4;
}
#endif
