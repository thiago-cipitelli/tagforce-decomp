// @range 0x08834AA0-0x08835364
// @asm 0x08834C34-0x08834E48
// @asm 0x088350B8-0x08835364
/* Coin toss and die roll animations on layer 6: the coin or die model plays its throw animation
   once per result (up to two), the model turned so the result faces up near the end, with the
   throw and landing sounds. */
#define DD_FX
#include "dd_d3.h"

typedef struct DdTossDraw {            /* BOOT model_draw options, 0x34 */
    int layer[2];
    int pad[11];
} DdTossDraw;

typedef struct DdTossAnim {            /* model_update parameters */
    float speed;
    void (*bone)(void *m, float *mtx, struct DdTossAnim *a);
    int face;                          /* result shown (0: none) */
    int pad;
} DdTossAnim;

typedef struct DdFxToss {              /* 0x68 */
    float frame;                       /* +0x00 */
    int count;                         /* +0x04 */
    int cur;                           /* +0x08 */
    int result[2];                     /* +0x0C */
    int kind;                          /* +0x14 die: 1 or other colour */
    unsigned int se;                   /* +0x18 */
    void *model;                       /* +0x1C */
    DdTossDraw draw;                   /* +0x20 */
    DdTossAnim anim;                   /* +0x54 */
    void *shadow;                      /* +0x64 */
} DdFxToss;

float *mat4_identity(float *out);
float *mat4_mul(float *out, float *a, float *b);
float *mat4_mul_rot_x(float *out, float *m, float a);
float *mat4_mul_rot_y(float *out, float *m, float a);
float *mat4_mul_rot_z(float *out, float *m, float a);
void *dd_model_get(int id);           /* model of the duel resources */
void *dd_bin_get(int id);
int model_anim_play(void *m, int n, unsigned int flags);
void model_update(void *m, DdTossAnim *a);
void model_draw(void *m, DdTossDraw *c);
float dd_anim_speed(void);
void dd_mtx_push_identity(void);
void dd_mtx_pop_all(void);
void dd_set_offset(int layer, int x, int y);
void dd_clear_stencil(int layer, unsigned int s);
unsigned int *dd_rstate_reset(int layer);
void dd_layer_draw_f(int layer, int arg, float f);
int dd_model_anim_done(void *m);
void dd_model_rewind(void *m);
int dd_cue_se_f(int id, float now, float at, unsigned int *flags, unsigned int bit);

int dd_fx_coin_update(int state, DdFxToss *fx);
int dd_fx_die_update(int state, DdFxToss *fx);

void dd_fx_coin_bone(void *m, float *mtx, DdTossAnim *a)
{
    float r[16];

    if (a->face == 1) {
        mat4_identity(r);
        mat4_mul_rot_y(r, r, 3.1415927f);
        mat4_mul(mtx, mtx, r);
    }
}

int dd_fx_coin_start(int count, int *result, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_coin_update, term, arg, sizeof(DdFxToss));
    DdFxToss *fx;
    int i;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->count = count;
        for (i = 0; i < count; i++)
            fx->result[i] = result[i];
    }
    return id;
}

#ifdef NON_MATCHING /* ordem dos testes do índice e do reinício da animação */
int dd_fx_coin_update(int state, DdFxToss *fx)
{
    float speed = dd_anim_speed();
    int last;
    int restart;

    dd_mtx_push_identity();
    dd_set_offset(6, 0x6C6, 0x77C);
    if (state == 1) {
        fx->draw.layer[0] = 6;
        fx->draw.layer[1] = 6;
        fx->anim.speed = speed;
        fx->anim.bone = dd_fx_coin_bone;
        fx->anim.face = 0;
        fx->anim.pad = 0;
        fx->model = dd_model_get(0x1001D);
        dd_model_rewind(fx->model);
        fx->shadow = dd_bin_get(0x80014);
    }
    last = fx->count - 1;
    if (fx->cur <= last)
        last = fx->cur;
    if (fx->result[last] < 0) {
        fx->cur = fx->count;
    } else if (fx->result[last] < 2) {
        dd_layer_draw_f(6, (int)fx->shadow, fx->frame);
        dd_clear_stencil(6, 0);
        model_draw(fx->model, &fx->draw);
        restart = 0;
        if (!dd_model_anim_done(fx->model)) {
            dd_cue_se_f(0x6247, fx->frame, 38.0f, &fx->se, 1 << fx->cur);
            fx->frame += speed;
        } else if (fx->cur < fx->count) {
            fx->cur++;
            fx->frame = 0.0f;
            restart = 1;
        }
        if (restart) {
            fx->anim.face = 0;
            model_anim_play(fx->model, 0, 0);
        }
        if (fx->frame > 54.0f)
            fx->anim.face = fx->result[last];
        else
            fx->anim.face = 0;
        model_update(fx->model, &fx->anim);
    } else {
        fx->cur = fx->count;
    }
    dd_mtx_pop_all();
    dd_set_offset(6, 0x710, 0x778);
    dd_rstate_reset(4);
    if (fx->cur < fx->count)
        return 2;
    return 4;
}
#endif

void dd_fx_die_bone(void *m, float *mtx, DdTossAnim *a)
{
    float r[16];

    mat4_identity(r);
    switch (a->face) {
    case 2:
        mat4_mul_rot_x(r, r, -1.5707964f);
        break;
    case 3:
        mat4_mul_rot_z(r, r, 1.5707964f);
        break;
    case 4:
        mat4_mul_rot_z(r, r, -1.5707964f);
        break;
    case 5:
        mat4_mul_rot_x(r, r, 1.5707964f);
        break;
    case 6:
        mat4_mul_rot_y(r, r, 1.5707964f);
        mat4_mul_rot_z(r, r, 3.1415927f);
        break;
    }
    mat4_mul(mtx, mtx, r);
}

int dd_fx_die_start(int count, int *result, int kind, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_die_update, term, arg, sizeof(DdFxToss));
    DdFxToss *fx;
    int i;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->count = count;
        for (i = 0; i < count; i++)
            fx->result[i] = result[i];
        fx->kind = kind;
    }
    return id;
}

#ifdef NON_MATCHING /* ordem dos testes do índice e do reinício da animação */
int dd_fx_die_update(int state, DdFxToss *fx)
{
    float speed = dd_anim_speed();
    int last;
    int restart;
    int model, shadow;

    dd_mtx_push_identity();
    dd_set_offset(6, 0x6C6, 0x77C);
    if (state == 1) {
        fx->draw.layer[0] = 6;
        fx->draw.layer[1] = 6;
        fx->anim.speed = speed;
        fx->anim.bone = dd_fx_die_bone;
        fx->anim.face = 0;
        fx->anim.pad = 0;
        if (fx->kind == 1) {
            model = 0x1001E;
            shadow = 0x80015;
        } else {
            model = 0x1001F;
            shadow = 0x80016;
        }
        fx->model = dd_model_get(model);
        dd_model_rewind(fx->model);
        fx->shadow = dd_bin_get(shadow);
    }
    last = fx->count - 1;
    if (fx->cur <= last)
        last = fx->cur;
    if (fx->result[last] < 1) {
        fx->cur = fx->count;
    } else if (fx->result[last] < 7) {
        dd_layer_draw_f(6, (int)fx->shadow, fx->frame);
        dd_clear_stencil(6, 0);
        model_draw(fx->model, &fx->draw);
        restart = 0;
        if (!dd_model_anim_done(fx->model)) {
            dd_cue_se_f(0x624F, fx->frame, 19.0f, &fx->se, 1);
            dd_cue_se_f(0x6250, fx->frame, 28.0f, &fx->se, 2);
            dd_cue_se_f(0x6251, fx->frame, 36.0f, &fx->se, 4);
            dd_cue_se_f(0x6252, fx->frame, 45.0f, &fx->se, 8);
            dd_cue_se_f(0x6245, fx->frame, 80.0f, &fx->se, 0x10);
            fx->frame += speed;
        } else if (fx->cur < fx->count) {
            fx->cur++;
            fx->frame = 0.0f;
            fx->se = 0;
            restart = 1;
        }
        if (restart) {
            fx->anim.face = 1;
            model_anim_play(fx->model, 0, 0);
        }
        if (fx->frame > 25.0f)
            fx->anim.face = fx->result[last];
        else
            fx->anim.face = 1;
        model_update(fx->model, &fx->anim);
    } else {
        fx->cur = fx->count;
    }
    dd_mtx_pop_all();
    dd_set_offset(6, 0x710, 0x778);
    dd_rstate_reset(4);
    if (fx->cur < fx->count)
        return 2;
    return 4;
}
#endif
