// @range 0x0882C214-0x0882C7A0
/* Effect: a card laid on the field fading in under 24 light rays turning around it (sprite fans
   of PT88_R.gim in four rings), 16 frames. */
#include "libduel_draw.h"

typedef struct DdFxFlare {             /* 0xB8 */
    int frame;                         /* +0x00 */
    int card;                          /* +0x04 */
    float pos[3];                      /* +0x08 */
    float rot[3];                      /* +0x14 */
    unsigned int se;                   /* +0x20 sound cues played */
    float t;                           /* +0x24 texture zoom of the card */
    DdFxVert ray[6];                   /* +0x28 */
} DdFxFlare;

float *vec3_copy(float *out, float *v);
float *vec3_set(float *out, float x, float y, float z);
float sinf(float x);
float cosf(float x);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(float *v);
void mtx_rotate_zyx(float *v);
void mtx_rotate_x(float a);
void mtx_rotate_y(float a);
unsigned int *draw_array(unsigned int *list, int prim, int vtype, int count, void *indices, void *verts);
unsigned int *dd_tex_pack(unsigned int *list, int i);
unsigned int *dd_tex_enable(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v);
unsigned int *dd_depth_enable(unsigned int *list, int on, int mask);
unsigned int *dd_blend_enable(unsigned int *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void dd_card_quad_draw1(int id, float *pos, float *rot, int alpha, float t);
void dd_translate3(float x, float y, float z);
float dd_ease_lin(int t, int n);
float dd_ease_cos(int t, int n, int k);
int dd_anim_step(void);
int dd_cue_se(int id, int now, int at, unsigned int *flags, unsigned int bit);

int dd_fx_flare_update(int state, DdFxFlare *fx);

int dd_fx_flare_start(float ry, int card, float *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_flare_update, term, arg, sizeof(DdFxFlare));
    DdFxFlare *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->card = card;
        vec3_copy(fx->pos, pos);
        vec3_set(fx->rot, 0.0f, ry, 0.0f);
        fx->t = 0.0f;
    }
    return id;
}

int dd_fx_flare_start_t(float ry, float t, int card, float *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_fx_flare_start(ry, card, pos, term, arg);

    if (id != -1)
        ((DdFxFlare *)dd_task_mem(id))->t = t;
    return id;
}

int dd_fx_flare_update(int state, DdFxFlare *fx)
{
    int step = dd_anim_step();
    unsigned int *list;
    float v[3];
    float a, r;
    int i, k;
    float hw, len, back;

    dd_cue_se(0x6225, fx->frame, 0, &fx->se, 1);
    dd_card_quad_draw1(fx->card, fx->pos, fx->rot, (unsigned int)(dd_ease_lin(fx->frame, 16) * 255.0f), fx->t);
    if (state == 1) {
        hw = 50.0f;
        len = 150.0f;
        back = 30.0f;
        fx->ray[0].u = 0.5f;
        fx->ray[0].v = 0.5f;
        fx->ray[0].color = 0xC800AAFF;
        fx->ray[0].x = 0.0f;
        fx->ray[0].y = 0.0f;
        fx->ray[0].z = -back;
        fx->ray[1].u = 0.0f;
        fx->ray[1].v = 0.0f;
        fx->ray[1].color = 0xAAFF;
        fx->ray[1].x = -hw;
        fx->ray[1].y = 0.0f;
        fx->ray[1].z = -len;
        fx->ray[2].u = 1.0f;
        fx->ray[2].v = 0.0f;
        fx->ray[2].color = 0xAAFF;
        fx->ray[2].x = hw;
        fx->ray[2].y = 0.0f;
        fx->ray[2].z = -len;
        fx->ray[3].u = 1.0f;
        fx->ray[3].v = 1.0f;
        fx->ray[3].color = 0xAAFF;
        fx->ray[3].x = hw;
        fx->ray[3].y = 0.0f;
        fx->ray[3].z = 0.0f;
        fx->ray[4].u = 0.0f;
        fx->ray[4].v = 1.0f;
        fx->ray[4].color = 0xAAFF;
        fx->ray[4].x = -hw;
        fx->ray[4].y = 0.0f;
        fx->ray[4].z = 0.0f;
        fx->ray[5].u = 0.0f;
        fx->ray[5].v = 0.0f;
        fx->ray[5].color = 0xAAFF;
        fx->ray[5].x = -hw;
        fx->ray[5].y = 0.0f;
        fx->ray[5].z = -len;
    }
    list = dl_select_layer(4, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 0, 0);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 3);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(fx->pos);
    mtx_rotate_zyx(fx->rot);
    list = dl_ambient_light(list, ((unsigned int)((0.5f + 0.5f * dd_ease_lin(fx->frame, 4)) * 255.0f * dd_ease_cos(fx->frame, 16, 1)) << 24) | 0xFFFFFF);
    for (i = 0; i < 24; i++) {
        k = i % 4;
        a = (float)(i / 4) * 6.2831855f / 6.0f + -0.0872664675f + (float)k * 0.62831855f / 4.0f
            + ((float)(3 - k) * 0.0017453294f + 0.008726646f) * (float)fx->frame;
        r = (float)k * 0.6f / 3.0f + 0.2f;
        vec3_set(v, -31.0f * r * sinf(a), 0.0f, -44.0f * r * cosf(a));
        mtx_push();
        mtx_translate(v);
        mtx_rotate_y(a);
        dd_translate3(0.0f, 0.0f, 19.5f);
        mtx_rotate_x(0.34906587f);
        list = draw_array(list, 5, 0x19F, 6, 0, fx->ray);
        mtx_pop();
    }
    mtx_pop();
    dl_set_list(list);
    fx->frame += step;
    if (fx->frame < 16)
        return 2;
    return 4;
}
