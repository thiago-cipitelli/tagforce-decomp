// @range 0x08835364-0x088357B0
// @asm 0x088353D8-0x088357B0
// @data 0x08876B10-0x08876B30
/* The coin of the toss model flying in from one side of the field, spinning while it grows, then
   playing its spin animation with random clinks before shrinking away. */
#define DD_FX
#include "dd_d3.h"

typedef struct DdFlyDraw {             /* BOOT model_draw options, 0x34 */
    int layer[2];
    int pad[11];
} DdFlyDraw;

typedef struct DdFxCoinFly {           /* 0x90 */
    float frame;                       /* +0x00 */
    int phase;                         /* +0x04 0 grow, 1 spin, 2 shrink */
    float grow;                        /* +0x08 0..8 */
    int side;                          /* +0x0C */
    int height;                        /* +0x10 1..40 flattening */
    unsigned int se;                   /* +0x14 */
    float se_frame;                    /* +0x18 */
    int pad1c;
    FxVec4 from;                       /* +0x20 */
    FxVec4 to;                         /* +0x30 */
    float rot;                         /* +0x40 */
    void *model;                       /* +0x44 */
    DdFlyDraw draw;                    /* +0x48 */
    float anim[4];                     /* +0x7C model_update parameters */
} DdFxCoinFly;

FxVec4 *vec4_copy(FxVec4 *out, FxVec4 *v);
FxVec4 *vec4_set(FxVec4 *out, float x, float y, float z, float w);
FxVec4 *vec3_lerp_w(FxVec4 *out, FxVec4 *a, FxVec4 *b, float t);
void *func_08804268(int id);
void model_update(void *m, float *a);
void model_draw(void *m, DdFlyDraw *c);
float dd_anim_speed(void);
unsigned int *dd_rstate_reset(int layer);
int dd_model_anim_done(void *m);
void dd_model_rewind(void *m);
int dd_rand_hit_se(void);
int dd_cue_se_f(int id, float now, float at, unsigned int *flags, unsigned int bit);

int dd_fx_coinfly_update(int state, DdFxCoinFly *fx);

int dd_fx_coinfly_start(int side, int height, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_coinfly_update, term, arg, sizeof(DdFxCoinFly));
    DdFxCoinFly *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->phase = 0;
        fx->side = side;
        fx->height = height;
    }
    return id;
}

/* templates of the locals of dd_fx_coinfly_update (kept in asm) */
FxVec4 dd_fx_coinfly_from1 = { 315.0f, 0.0f, 240.0f, 1.0f };
FxVec4 dd_fx_coinfly_from2 = { -315.0f, 0.0f, -240.0f, 1.0f };

#ifdef NON_MATCHING /* ramos da máquina de estados e alocação (forma próxima) */
int dd_fx_coinfly_update(int state, DdFxCoinFly *fx)
{
    float speed = dd_anim_speed();
    FxVec4 p;
    float g, rot, h, s;
    int spinning = 0;

    if (state == 1) {
        if (fx->side == 1) {
            FxVec4 a = { 315.0f, 0.0f, 240.0f, 1.0f };

            vec4_copy(&fx->from, &a);
            vec4_set(&fx->to, 0.0f, 0.0f, 0.0f, 1.0f);
            fx->rot = 0.0f;
        } else {
            FxVec4 b = { -315.0f, 0.0f, -240.0f, 1.0f };

            vec4_copy(&fx->from, &b);
            vec4_set(&fx->to, 0.0f, 0.0f, 0.0f, 1.0f);
            fx->rot = 3.1415927f;
        }
        fx->draw.layer[0] = 4;
        fx->draw.layer[1] = 4;
        fx->anim[0] = speed * 2.0f;
        fx->anim[1] = 0.0f;
        fx->anim[2] = 0.0f;
        fx->anim[3] = 0.0f;
        fx->model = func_08804268(0x10020);
        dd_model_rewind(fx->model);
    }
    switch (fx->phase) {
    case 0:
        fx->grow += speed;
        if (fx->grow >= 8.0f) {
            fx->grow = 8.0f;
            fx->phase = 1;
        }
        break;
    case 1:
        if (fx->height < 2) {
            fx->frame += speed;
            if (fx->frame >= 60.0f)
                fx->phase = 2;
        } else {
            dd_cue_se_f(dd_rand_hit_se(), fx->se_frame, 0.0f, &fx->se, 1);
            dd_cue_se_f(dd_rand_hit_se(), fx->se_frame, 10.0f, &fx->se, 2);
            dd_cue_se_f(dd_rand_hit_se(), fx->se_frame, 20.0f, &fx->se, 4);
            dd_cue_se_f(dd_rand_hit_se(), fx->se_frame, 31.0f, &fx->se, 8);
            fx->se_frame += speed;
            spinning = 1;
            if (dd_model_anim_done(fx->model) == 1)
                fx->phase = 2;
        }
        break;
    case 2:
        fx->grow -= speed;
        if (fx->grow <= 0.0f)
            fx->grow = 0.0f;
        break;
    }
    g = fx->grow;
    rot = fx->rot;
    h = (float)fx->height / 40.0f;
    s = g / 8.0f * 2.0f + 1.0f;
    vec3_lerp_w(&p, &fx->from, &fx->to, 40.0f);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_translate3(p.x, p.y, p.z);
    mtx_rotate_y(rot + g / 8.0f * 0.7853982f);
    dd_scale3(s, s, s);
    dd_scale3(1.0f, h, 1.0f);
    model_draw(fx->model, &fx->draw);
    if (spinning)
        model_update(fx->model, fx->anim);
    mtx_pop();
    dd_rstate_reset(4);
    if (fx->phase == 2 && fx->grow == 0.0f)
        return 4;
    return 2;
}
#endif
