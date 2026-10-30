// @range 0x08830468-0x08830AAC
/* 2D effects on a card shown on screen (layer 6): a pulsing aura of 16 growing copies of the card,
   and a static noise flash over it; 24 frames. */
#define DD_FX
#include "dd_d3.h"

typedef struct DdScrPos {
    int x, y, z;
} DdScrPos;

typedef struct DdFxAura {              /* 0x18 */
    int frame;
    int card;
    DdScrPos pos;
    unsigned int se;
} DdFxAura;

typedef struct DdFxNoise {             /* 0x14 */
    int frame;
    DdScrPos pos;
    unsigned int se;
} DdFxNoise;

unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);

int dd_fx_aura_update(int state, DdFxAura *fx);
int dd_fx_noise_update(int state, DdFxNoise *fx);

int dd_fx_aura_start(int card, DdScrPos *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_aura_update, term, arg, sizeof(DdFxAura));
    DdFxAura *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->card = card;
        fx->pos = *pos;
    }
    return id;
}

int dd_fx_aura_update(int state, DdFxAura *fx)
{
    int step = dd_anim_step();
    unsigned int *list;
    float in, out, k;
    int i, w, h;

    dd_cue_se(0x6229, fx->frame, 0, &fx->se, 1);
    in = dd_ease_lin(fx->frame, 8);
    out = dd_ease_cos(fx->frame, 24, 1);
    list = dl_select_layer(6, 0);
    list = dd_depth_enable(list, 0, 1);
    list = dd_tex_card(list, fx->card, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    for (i = 0; i < 16; i++) {
        k = (float)i * 0.25f / 16.0f + 1.0f;
        w = k * 108.0f;
        h = k * 154.0f;
        list = dl_color(list, ((unsigned int)(in * out * (float)((16 - i) * 90 / 16)) << 24) | 0x5064C8);
        list = draw_set_offsets(list, 108, 154, w, h);
        list = draw_sprite(list, fx->pos.x - w / 2, fx->pos.y - h / 2, 0, 0, 0, 0, 0);
    }
    dl_set_list(list);
    fx->frame += step;
    if (fx->frame < 24)
        return 2;
    return 4;
}

int dd_fx_noise_start(DdScrPos *pos, DdTaskTermFn term, void *arg)
{
    int id = dd_task_create((DdTaskUpdateFn)dd_fx_noise_update, term, arg, sizeof(DdFxNoise));
    DdFxNoise *fx;

    if (id != -1) {
        fx = dd_task_mem(id);
        fx->pos = *pos;
    }
    return id;
}

int dd_fx_noise_update(int state, DdFxNoise *fx)
{
    DdRandSeed *r;
    int step = dd_anim_step();
    unsigned int *list;
    float in = dd_ease_lin(fx->frame, 4);
    float out = dd_ease_lin_out(fx->frame, 4, 24);
    unsigned int alpha;

    dd_cue_se(0x624E, fx->frame, 0, &fx->se, 1);
    list = dl_select_layer(6, 0);
    list = dd_depth_enable(list, 0, 1);
    list = dd_tex_enable(list, 0, 1, 1, 0, 0, 0, 0);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    alpha = (unsigned int)(128.0f * in * out) << 24;
    list = dl_color(list, alpha);
    list = draw_set_offsets(list, 0, 0, 108, 154);
    list = draw_sprite(list, fx->pos.x - 54, fx->pos.y - 77, 0, 0, 0, 0, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
    list = dd_blend_enable(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 23);
    list = dl_color(list, alpha | 0xFFFFFF);
    list = draw_set_offsets(list, 85, 121, 108, 154);
    r = &rand3_seeds[rand3_slot];
    list = draw_sprite(list, fx->pos.x - 54, fx->pos.y - 77, 0, DD_RAND(r) % 128, DD_RAND(r) % 128, 0, 0);
    mtx_pop();
    dl_set_list(list);
    fx->frame += step;
    if (fx->frame < 24)
        return 2;
    return 4;
}
