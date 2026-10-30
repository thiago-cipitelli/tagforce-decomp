// @range 0x08816FD0-0x088175E4
// @data 0x08875AC4-0x08875AD4
/* The full-screen EHF cut-in player: one animation plays (with an optional caption and the end
   callback), a second request waits in the queue; the speed follows the configured animation
   setting and the loop part repeats until the player skips. */
#include "dd_field_ui.h"

extern DdCutin dd_cutin;               /* 0x08C546D0 */

/* frames advanced per update by the animation-speed setting */
int dd_cutin_speed_tab[4] = { 1, 1, 2, 2 };

int dd_scene_flags(int a, int b);
unsigned short *dd_text(int id);
int *dd_pad(void);
int dd_cutin_speed(void);
void dd_cutin_play(DdCutinDesc *d);

void dd_cutin_init(void)
{
    memset(&dd_cutin, 0, sizeof(DdCutin));
}

void dd_cutin_term(void)
{
    memset(&dd_cutin, 0, sizeof(DdCutin));
}

unsigned int *dd_cutin_draw(unsigned int *list)
{
    DdCutin *c = &dd_cutin;
    int layer = -1;
    void *e;
    unsigned int opts[24];
    unsigned short *text;
    int w, maxw;
    unsigned int sz;
    int tw, th;

    if (dd_cutin.active == 0)
        return list;
    if (c->cur.layer > 0) {
        layer = dl_get_layer();
        dl_set_list(list);
        list = dl_select_layer(c->cur.layer, 0);
        list = gfx_begin_2d(list);
    }
    e = c->cur.ehf;
    memset(opts, 0, sizeof(opts));
    if (ehf_get_frame_count(e) == 0)
        opts[0] = gfx_get_state()[0];
    else
        opts[0] = ehf_get_cur_frame(e);
    opts[3] = 0x10000;
    ((float *)opts)[5] = c->cur.x;
    ((float *)opts)[6] = c->cur.y;
    list = ehf_draw(list, e, opts);
    if (c->queued != 0 && c->next.flag > 0)
        req_flag_set(c->next.flag);
    if (c->cur.always != 0 || cfg_get_bits_1_3_4() < 2)
        req_flag_set(c->cur.flag);
    if (c->cur.text_frame > 0 && c->cur.text_frame < opts[0]) {
        text = dd_text(c->cur.text);
        font_text_set(list);
        font_set_size(0xC, 0xC);
        font_set_color(0xFFFFFFFF);
        if (cfg_get_language() == 0) {
            font_set_flag1(1);
            font_set_flag8(0);
        } else {
            font_set_flag1(0);
            font_set_flag8(1);
        }
        w = (font_measure_text(text) + 0x3F) >> 6;
        maxw = (0x1E0 - c->cur.text_x) * 160 / 100;
        w = ALLEGREX_MIN(w, maxw);
        sz = font_measure_wtext2((w + 1) << 6, 0x4400, text);
        tw = ((sz >> 16) + 0x3F) >> 6;
        th = ((sz & 0xFFFF) + 0x3F) >> 6;
        font_draw_wtext_ex((c->cur.text_x - tw / 2) << 6, (c->cur.text_y - th / 2) << 6, tw << 6, th << 6, text, -1);
        list = font_text_clear();
    }
    if (layer >= 0) {
        dl_set_list(gfx_end_2d(list));
        list = dl_select_layer(layer, 0);
    }
    return list;
}

void dd_cutin_update(void)
{
    DdCutin *c = &dd_cutin;
    int *sys = dd_pad();
    void *e;
    int cnt, f, shown;
    int target, limit;

    if (dd_cutin.active == 0)
        return;
    e = c->cur.ehf;
    cnt = ehf_get_frame_count(e);
    if (dd_cutin_speed() <= 1 || c->looping != 0)
        f = cnt + 1;
    else
        f = cnt + 2;
    if (f >= ehf_get_data_ptr(e) - 1)
        f = ehf_get_data_ptr(e) - 1;
    shown = f;
    ehf_get_size(e, &target, &limit);
    if (f >= limit) {
        c->loops++;
        shown = target;
    }
    ehf_set_frame_alias(e, shown);
    if (f >= c->cur.loop_from && c->cur.loop_from != c->cur.loop_to && c->cur.loop_to != 0 && c->ending == 0 &&
        c->looping == 0) {
        c->looping = 1;
        c->loops = 0;
        ehf_set_anim(c->cur.ehf, c->cur.loop_from, c->cur.loop_to);
    }
    if ((f >= c->cur.end - 1 && c->cur.end >= 0) || f >= ehf_get_data_ptr(e) - 1) {
        c->active = 0;
        if (c->cur.done != 0)
            c->cur.done(c->cur.done_arg);
        if (c->queued != 0) {
            c->queued = 0;
            dd_cutin_play(&c->next);
        }
    }
    if (c->looping != 0 &&
        ((sysvar_get_a98() == 0 && sys[1] != 0) || (sysvar_get_a98() != 0 && c->loops >= 5))) {
        c->ending = 1;
        c->looping = 0;
        ehf_set_anim(e, c->cur.loop_to, c->cur.end);
    }
}

/* starts a cut-in, or queues it behind the one playing */
void dd_cutin_play(DdCutinDesc *d)
{
    DdCutin *c = &dd_cutin;

    if (dd_cutin.active != 0) {
        c->queued = 1;
        c->next = *d;
        req_flag_set(c->next.flag);
        return;
    }
    c->cur = *d;
    c->ending = 0;
    c->looping = 0;
    c->active = 1;
    ehf_set_anim(d->ehf, d->start, d->end);
    ehf_set_frame_alias(d->ehf, d->start);
    if (d->se != 0 && dd_scene_flags(8, -1) == 0)
        se_play(d->se);
}

int dd_cutin_looping(void)
{
    return dd_cutin.looping;
}

int dd_cutin_speed(void)
{
    return dd_cutin_speed_tab[cfg_get_bits_1_3_4()];
}
