// @range 0x0883B62C-0x08842824
// @rodata 0x088707FC-0x08870820
// @asm 0x0883E6F8-0x0883EF6C
/* Effects 32-37 and 52-53: nine styles of full-screen burst picked by dd_fx_table.arg0 (style 0-8),
   each a task with its own particles, drawn through a stencil mask of the field except for effect 33,
   and the shared helpers (timings, depth state, the mask). */
#include "dd_fx.h"

/* BOOT */
void *dl_clear_depth(void *list, unsigned int z);
void *dl_clear(void *list, unsigned int flags);
void *dl_depth_func(void *list, int func);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_stencil_op(void *list, int fail, int zfail, int zpass);
void *dl_stencil_func(void *list, int func, int ref, int mask);

int dd_fx32_s0_start(DdFxSlot *s, int a, int b);
int dd_fx32_s1_start(DdFxSlot *s, int a, int b);
int dd_fx32_s2_start(DdFxSlot *s, int a, int b);
int dd_fx32_s3_start(DdFxSlot *s, int a, int b);
int dd_fx32_s4_start(DdFxSlot *s, int a, int b);
int dd_fx32_s5_start(DdFxSlot *s, int a, int b);
int dd_fx32_s6_start(DdFxSlot *s, int a, int b);
int dd_fx32_s7_start(DdFxSlot *s, int a, int b);
int dd_fx32_s8_start(DdFxSlot *s, int a, int b);

/* effects 32-37 / 52-53: style = dd_fx_table.arg0 */
int dd_fx32_start(DdFxSlot *s, int style, int a, int b)
{
    switch (style) {
    case 0:
        return dd_fx32_s0_start(s, a, b);
    case 1:
        return dd_fx32_s1_start(s, a, b);
    case 2:
        return dd_fx32_s2_start(s, a, b);
    case 3:
        return dd_fx32_s3_start(s, a, b);
    case 4:
        return dd_fx32_s4_start(s, a, b);
    case 5:
        return dd_fx32_s5_start(s, a, b);
    case 6:
        return dd_fx32_s6_start(s, a, b);
    case 7:
        return dd_fx32_s7_start(s, a, b);
    case 8:
        return dd_fx32_s8_start(s, a, b);
    default:
        return -1;
    }
}

/* grow time and length by effect id */
void dd_fx32_kind(int id, int *grow, int *end)
{
    switch (id) {
    case 32: case 33:
        *grow = 10;
        *end = 50;
        return;
    case 34: case 52: case 53:
        *grow = 20;
        break;
    case 35:
        *grow = 10;
        break;
    case 37:
        *grow = 10;
        *end = 45;
        return;
    case 36:
        *grow = 25;
        break;
    default:
        *grow = 0;
        *end = 0;
        return;
    }
    *end = 60;
}

/* depth test off, depth writes on (the id is not used) */
void *dd_fx32_depth(void *list, int id)
{
    return dd_depth_enable(list, 0, 1);
}

/* except for effect 33, draws only over the pixels the field left empty: marks them in the stencil */
void dd_fx32_mask_begin(int id)
{
    void *list;

    if (id != 33) {
        list = dl_select_layer(7, 0);
        list = dl_clear_depth(list, 0);
        list = dl_clear(list, 0x12);
        list = dd_tex_enable(list, 0, 1, 1, 0, 0, 0, 0);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dl_color(list, 0xFF000000);
        list = dd_depth_enable(list, 1, 1);
        list = dl_depth_func(list, 2);
        list = dl_enable(list, 3);
        list = dl_stencil_op(list, 1, 0xFF, 0xFF);
        list = dl_stencil_func(list, 0, 0, 2);
        list = draw_set_offsets(list, 0, 0, 480, 272);
        list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
        list = dl_depth_func(list, 7);
        list = dl_stencil_op(list, 2, 0xFF, 0xFF);
        list = dl_stencil_func(list, 0, 0, 0);
        dl_set_list(list);
    }
}

void dd_fx32_mask_end(int id)
{
    void *list;

    if (id != 33) {
        list = dl_select_layer(7, 0);
        list = dl_disable(list, 3);
        dl_set_list(list);
    }
}

/* ---- style 0: blades and motes rushing at the camera, then a cyan flash over the screen ---- */

int dd_fx32_s0_update(int phase, DdFx32S0 *d);

int dd_fx32_s0_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_fx32_s0_update, a, b, sizeof(DdFx32S0));
    DdFx32S0 *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

int dd_fx32_s0_update(int phase, DdFx32S0 *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    ScePspFMatrix4 bill;
    DdRandSeed *r;
    DdFx32Blades *bl;
    DdFx32Motes *m;
    DdRingVtx *v;
    void *list;
    float in;
    float out;
    float ang;
    float z;
    float tv;
    float hl;
    float hw;
    float cw;
    float ch;
    float cu;
    float cv;
    float f;
    float sn;
    float cs;
    int col;
    int i;
    int j;
    int y;
    int w;
    int h;
    int j3;
    int i3;
    int j2;
    int i2;
    DdFx32Motes * m_0;
    DdFx32Blades * bl_0;
    int j_0;
    int y_0;

    dd_mtx_push_saved();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = dd_ease_sin(d->t, 8, 1);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 8), 8);
    dd_mtx_billboard_cam(&bill);
    dd_cue_se(0x6236, d->grow, d->grow_end, &d->se_done, 1);

    /* the blades: 8 frames of a 4 x 2 sheet stretched along z */
    if (phase == 1) {
        hl = 800.0f;
        hw = 20.0f;
        cw = 0.25f;
        ch = 0.5f;
        for (i = 0; i < 8; i++) {
            cu = i % 4 * cw;
            cv = i / 4 * ch;
            for (j = 0; j < 9; j++) {
                f = j / 8.0f;
                z = -hl + f * 1600.0f;
                tv = cv + ch * f;
                d->blade_v[i][j][0].u = cu;
                d->blade_v[i][j][0].v = tv;
                d->blade_v[i][j][0].x = -hw;
                d->blade_v[i][j][0].y = 0.0f;
                d->blade_v[i][j][0].z = z;
                d->blade_v[i][j][1].u = cu + cw;
                d->blade_v[i][j][1].v = tv;
                d->blade_v[i][j][1].x = hw;
                d->blade_v[i][j][1].y = 0.0f;
                d->blade_v[i][j][1].z = z;
            }
        }
        for (i = 0; i < 9; i++) {
            d->idx[i * 2] = i * 2;
            d->idx[i * 2 + 1] = i * 2 + 1;
        }
    }
    if (d->grow == d->grow_end) {
        bl = &d->blade[d->blade_next];
        bl->t = 0;
        bl->flags |= 1;
        for (i2 = 0; i2 < 12; i2++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            bl->frame[i2] = DD_RAND(r) % 8;
            vec3_set(&bl->pos[i2], sinf(ang) * 220.0f, cosf(ang) * 220.0f + 120.0f, 160.0f);
            bl->speed[i2] = (DD_RAND(r) / 32767.0f * 60.0f + 240.0f) * -1.0f;
            bl->ang[i2] = -ang;
        }
        if (d->blade_next < 23)
            d->blade_next++;
        else
            d->blade_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_fx32_depth(list, s->id);
    list = dd_blend_enable(list, 1, 0, 6, 10, 0, -1);
    list = dd_tex_pack(list, 14);
    list = dl_color(list, 0xFFA0DCFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        int jb;
        bl_0 = &d->blade[i];
        if (!(bl_0->flags & 1) == 1)
            continue;
        list = dl_ambient_light(list, (unsigned int)(out * (dd_ease_cos(bl_0->t, 24, 1) * 255.0f)) << 24 | 0xFFFFFF);
        for (jb = 0; jb < 12; jb++) {
            mtx_push();
            mtx_translate(&bl_0->pos[jb]);
            mtx_rotate_z(bl_0->ang[jb]);
            list = draw_array(list, 4, 0x983, 18, d->idx, d->blade_v[bl_0->frame[jb]]);
            mtx_pop();
            bl_0->pos[jb].z += bl_0->speed[jb];
        }
        if (bl_0->t < 23)
            bl_0->t++;
        else
            bl_0->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);

    /* the motes: 4 frames of a 2 x 2 sheet */
    if (phase == 1) {
        hw = 80.0f;
        ch = 0.5f;
        for (j2 = 0; j2 < 4; j2++) {
            cu = ch * (j2 % 2);
            cv = ch * (j2 / 2);
            d->mote_v[j2][0].u = cu;
            d->mote_v[j2][0].v = cv;
            d->mote_v[j2][0].x = -hw;
            d->mote_v[j2][0].y = hw;
            d->mote_v[j2][0].z = 0.0f;
            d->mote_v[j2][1].u = cu + ch;
            d->mote_v[j2][1].v = cv + ch;
            d->mote_v[j2][1].x = hw;
            d->mote_v[j2][1].y = -hw;
            d->mote_v[j2][1].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        m = &d->mote[d->mote_next];
        m->t = 0;
        m->flags |= 1;
        for (i3 = 0; i3 < 4; i3++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            sn = sinf(ang);
            cs = cosf(ang);
            m->frame[i3] = DD_RAND(r) % 4;
            m->speed[i3] = (DD_RAND(r) / 32767.0f * 60.0f + 120.0f) * -1.0f;
            vec3_set(&m->pos[i3], sn * 220.0f, cs * 220.0f + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
        }
        if (d->mote_next < 23)
            d->mote_next++;
        else
            d->mote_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_fx32_depth(list, s->id);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
    list = dd_tex_pack(list, 10);
    list = dl_color(list, 0xC8A0DCFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        m_0 = &d->mote[i];
        if (!(m_0->flags & 1) == 1)
            continue;
        dd_ease_lin(m_0->t, 24);
        list = dl_ambient_light(list, (unsigned int)(out * (dd_ease_cos(m_0->t, 24, 1) * 255.0f)) << 24 | 0xFFFFFF);
        for (j_0 = 0; j_0 < 4; j_0++) {
            mtx_push();
            mtx_translate(&m_0->pos[j_0]);
            mtx_mult(&bill);
            list = draw_array(list, 6, 0x183, 2, 0, d->mote_v[m_0->frame[j_0]]);
            mtx_pop();
            m_0->pos[j_0].z += m_0->speed[j_0];
        }
        if (m_0->t < 23)
            m_0->t++;
        else
            m_0->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    dd_fx32_mask_end(s->id);

    /* flash: the screen copied tile by tile (64 x 32) and added back tinted */
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
        list = dd_depth_enable(list, 0, 0);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dl_color(list, (unsigned int)(in * 200.0f * out) << 24 | 0xA0DCFF);
        for (i = 0, y_0 = 0; i < 9; i++, y_0 += 32) {
            int x;
            h = i < 8 ? 32 : 272 - y_0;
            for (col = 0, x = 0; col < 8; col++, x += 64) {
                w = col < 7 ? 64 : 480 - x;
                list = dd_tex_fb_rect(list, x, y_0, 64, 32);
                list = draw_set_offsets(list, w, h, w, h);
                list = draw_sprite(list, x, y_0, 0, 0, 0, 0, 0);
            }
        }
        dl_set_list(list);
    }
    dd_mtx_pop_all();
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}

/* ---- style 1: a wavy tube of light going into the screen, motes and spinning sparks ---- */

int dd_fx32_s1_update(int phase, DdFx32S1 *d);

int dd_fx32_s1_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_fx32_s1_update, a, b, sizeof(DdFx32S1));
    DdFx32S1 *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

int dd_fx32_s1_update(int phase, DdFx32S1 *d)
{
    int k5;
    int k4;
    int k3;
    int k2;
    int k1;
    int k0;
    float rd;
    DdFxSlot *s = d->slot;
    int run = s->param;
    ScePspFMatrix4 bill;
    int buf;
    DdRandSeed *r;
    DdFx32Motes *m;
    DdFx32Spin *sp;
    DdTCVtx *v;
    DdRingVtx *q;
    void *list;
    unsigned int alpha;
    float in;
    float out;
    float ang;
    float fr;
    float z;
    float rad;
    float tu;
    float tv;
    int sign;
    float cw;
    float ch;
    float hw;
    float cu;
    float cv;
    float tvv;
    float sn;
    float cs;
    unsigned int c;
    int i;
    int j;
    int j3;
    int i3;
    int j2;
    DdFx32Motes *m_0;
    DdFx32Spin *sp_0;

    dd_mtx_push_saved();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = dd_ease_sin(d->t, 8, 1);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 8), 8);
    dd_mtx_billboard_cam(&bill);
    buf = d->frame % 2;
    dd_cue_se(0x6236, d->grow, d->grow_end, &d->se_done, 1);

    /* motes */
    if (phase == 1) {
        hw = 120.0f;
        ch = 0.5f;
        for (k0 = 0; k0 < 4; k0++) {
            cu = ch * (k0 % 2);
            cv = ch * (k0 / 2);
            d->mote_v[k0][0].u = cu;
            d->mote_v[k0][0].v = cv;
            d->mote_v[k0][0].x = -hw;
            d->mote_v[k0][0].y = hw;
            d->mote_v[k0][0].z = 0.0f;
            d->mote_v[k0][1].u = cu + ch;
            d->mote_v[k0][1].v = cv + ch;
            d->mote_v[k0][1].x = hw;
            d->mote_v[k0][1].y = -hw;
            d->mote_v[k0][1].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        m = &d->mote[d->mote_next];
        m->t = 0;
        m->flags |= 1;
        for (k1 = 0; k1 < 4; k1++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            sn = sinf(ang);
            cs = cosf(ang);
            m->frame[k1] = DD_RAND(r) % 4;
            m->speed[k1] = (DD_RAND(r) / 32767.0f * 60.0f + 120.0f) * -1.0f;
            vec3_set(&m->pos[k1], sn * 270.0f, cs * 270.0f + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
        }
        if (d->mote_next < 23)
            d->mote_next++;
        else
            d->mote_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_fx32_depth(list, s->id);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
    list = dd_tex_pack(list, 10);
    list = dl_color(list, 0xFF0000FF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (k2 = 0; k2 < 24; k2++) {
        int j_0;
        m_0 = &d->mote[k2];
        if (!(m_0->flags & 1) == 1)
            continue;
        dd_ease_lin(m_0->t, 24);
        list = dl_ambient_light(list, (unsigned int)(out * (dd_ease_cos(m_0->t, 24, 1) * 255.0f)) << 24 | 0xFFFFFF);
        for (j_0 = 0; j_0 < 4; j_0++) {
            mtx_push();
            mtx_translate(&m_0->pos[j_0]);
            mtx_mult(&bill);
            list = draw_array(list, 6, 0x183, 2, 0, d->mote_v[m_0->frame[j_0]]);
            mtx_pop();
            m_0->pos[j_0].z += m_0->speed[j_0];
        }
        if (m_0->t < 23)
            m_0->t++;
        else
            m_0->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);

    /* the tube: 49 rings of 33 vertices, radius waving per vertex */
    if (phase == 1) {
        vec3_set(&d->center, 0.0f, 120.0f, 160.0f);
        for (i = 0; i < 49; i++) {
            fr = i / 48.0f;
            z = fr * -6000.0f;
            tvv = fr * 4.0f;
            alpha = (unsigned int)((1.0f - fr) * 220.0f);
            c = alpha << 24 | 0xFF;
            for (j = 0; j < 33; j++) {
                ang = j / 32.0f * 6.2831855f;
                d->tube[i][j].u = j / 32.0f * 2.0f;
                d->tube[i][j].v = tvv;
                d->tube[i][j].color = c;
                d->tube[i][j].x = sinf(ang) * 220.0f;
                d->tube[i][j].y = cosf(ang) * 220.0f;
                d->tube[i][j].z = z;
            }
        }
        for (i = 0; i < 48; i++) {
            for (j = 0; j < 33; j++) {
                d->idx[i][j * 2] = j + i * 33;
                d->idx[i][j * 2 + 1] = j + (i + 1) * 33;
                d->phase[i][j] = DD_RAND(r) / 32767.0f * 6.2831855f;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_fx32_depth(list, s->id);
        list = dd_blend_enable(list, 1, 0, 6, 3, 0, 0);
        list = dd_tex_pack(list, 9);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->center);
        alpha = (unsigned int)(in * 255.0f * out);
        for (k5 = 0; k5 < 49; k5++) {
            for (k4 = 0; k4 < 32; k4++) {
                ang = k4 / 32.0f * 6.2831855f;
                rad = (sinf(d->phase[k5][k4] + d->frame * 0.17453294f) * 0.12f + 1.0f) * 220.0f;
                d->tube[k5][k4].x = rad * sinf(ang);
                d->tube[k5][k4].y = rad * cosf(ang);
            }
            d->tube[k5][32].x = d->tube[k5][0].x;
            d->tube[k5][32].y = d->tube[k5][0].y;
        }
        memcpy(d->tube_buf[buf], d->tube, sizeof(d->tube));
        list = dl_tex_offset(d->tex_u, d->tex_v, list);
        list = dl_ambient_light(list, alpha << 24 | 0xFFFFFF);
        for (i = 0; i < 48; i++)
            list = draw_array(list, 4, 0x119F, 66, d->idx[i], d->tube_buf[buf]);
        d->tex_u = d->tex_u + 0.01f;
        d->tex_v = d->tex_v - 0.05f;
        mtx_pop();
        dl_set_list(list);
    }
    dd_fx32_mask_end(s->id);

    /* spinning sparks: 8 frames of a 4 x 2 sheet */
    if (phase == 1) {
        cw = 0.25f;
        ch = 0.5f;
        hw = 25.0f;
        for (j2 = 0; j2 < 8; j2++) {
            tu = cw * (j2 % 4);
            tv = ch * (j2 / 4);
            d->spin_v[j2][0].u = tu;
            d->spin_v[j2][0].v = tv;
            d->spin_v[j2][0].x = -hw;
            d->spin_v[j2][0].y = hw;
            d->spin_v[j2][0].z = 0.0f;
            d->spin_v[j2][1].u = tu;
            d->spin_v[j2][1].v = tv + ch;
            d->spin_v[j2][1].x = -hw;
            d->spin_v[j2][1].y = -hw;
            d->spin_v[j2][1].z = 0.0f;
            d->spin_v[j2][2].u = tu + cw;
            d->spin_v[j2][2].v = tv;
            d->spin_v[j2][2].x = hw;
            d->spin_v[j2][2].y = hw;
            d->spin_v[j2][2].z = 0.0f;
            d->spin_v[j2][3].u = tu + cw;
            d->spin_v[j2][3].v = tv + ch;
            d->spin_v[j2][3].x = hw;
            d->spin_v[j2][3].y = -hw;
            d->spin_v[j2][3].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end && d->t < d->end) {
        sp = &d->spin[d->spin_next];
        sp->t = 0;
        sp->flags |= 1;
        for (i3 = 0; i3 < 8; i3++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            rd = DD_RAND(r) / 32767.0f * 100.0f;
            sn = sinf(ang);
            cs = cosf(ang);
            sp->frame[i3] = DD_RAND(r) % 8;
            sp->speed[i3] = DD_RAND(r) / 32767.0f * 10.0f + 20.0f;
            vec3_set(&sp->pos[i3], rd * sn, rd * cs + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
            sp->ang[i3] = 0.0f;
#ifdef __MWERKS__
            sp->spin[i3] = (DD_RAND(r) % 2 * 2 - 1) * (DD_RAND(r) / 32767.0f * 0.34906587f);
#else /* port: sign drawn first, as mwcc does; in C the order is unspecified */
            { int sg = DD_RAND(r) % 2 * 2 - 1; sp->spin[i3] = sg * (DD_RAND(r) / 32767.0f * 0.34906587f); }
#endif
        }
        if (d->spin_next < 23)
            d->spin_next++;
        else
            d->spin_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 2, 2, 10, 0, -1);
    list = dd_tex_pack(list, 5);
    list = dl_color(list, 0x80FFFFFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (j3 = 0; j3 < 24; j3++) {
        int jd;
        sp_0 = &d->spin[j3];
        if (!(sp_0->flags & 1) == 1)
            continue;
        dd_ease_lin(sp_0->t, 24);
        list = dl_ambient_light(list, (unsigned int)(out * (dd_ease_cos(sp_0->t, 24, 1) * 255.0f)) << 24
                                          | 0xFFFFFF);
        for (jd = 0; jd < 8; jd++) {
            mtx_push();
            mtx_translate(&sp_0->pos[jd]);
            mtx_mult(&bill);
            mtx_rotate_z(sp_0->ang[jd]);
            list = draw_array(list, 4, 0x183, 4, 0, d->spin_v[sp_0->frame[jd]]);
            mtx_pop();
            sp_0->pos[jd].z -= sp_0->speed[jd];
            sp_0->ang[jd] += sp_0->spin[jd];
        }
        if (sp_0->t < 23)
            sp_0->t++;
        else
            sp_0->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    dd_mtx_pop_all();
    d->frame++;
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}

/* ---- style 2: a white tube of light scrolling fast into the screen and spinning sparks ---- */

int dd_fx32_s2_update(int phase, DdFx32S2 *d);

int dd_fx32_s2_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_fx32_s2_update, a, b, sizeof(DdFx32S2));
    DdFx32S2 *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

int dd_fx32_s2_update(int phase, DdFx32S2 *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    ScePspFMatrix4 bill;
    DdRandSeed *r;
    DdFx32Spin *sp;
    DdTCVtx *v;
    DdRingVtx *q;
    void *list;
    unsigned int alpha;
    float in;
    float out;
    float ang;
    float fr;
    float z;
    float rad;
    float tu;
    float tv;
    int sign;
    float cw;
    float ch;
    float hw;
    float tvv;
    float sn;
    float cs;
    unsigned int c;
    int i;
    int j;
    int j3;
    int i3;
    int j2;
    int i2;
    DdFx32Spin * sp_0;

    dd_mtx_push_saved();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = dd_ease_sin(d->t, 8, 1);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 8), 8);
    dd_mtx_billboard_cam(&bill);
    dd_cue_se(0x6236, d->grow, d->grow_end, &d->se_done, 1);

    /* the tube: 49 rings of 33 vertices */
    if (phase == 1) {
        vec3_set(&d->center, 0.0f, 120.0f, 160.0f);
        for (i = 0; i < 49; i++) {
            fr = i / 48.0f;
            z = fr * -6000.0f;
            tvv = fr * 4.0f;
            alpha = (unsigned int)((1.0f - fr) * 255.0f);
            c = alpha << 24 | 0xFFFFFF;
            for (j = 0; j < 33; j++) {
                ang = j / 32.0f * 6.2831855f;
                d->tube[i][j].u = j / 32.0f * 2.0f;
                d->tube[i][j].v = tvv;
                d->tube[i][j].color = c;
                d->tube[i][j].x = sinf(ang) * 220.0f;
                d->tube[i][j].y = cosf(ang) * 220.0f;
                d->tube[i][j].z = z;
            }
        }
        for (i = 0; i < 48; i++) {
            for (j = 0; j < 33; j++) {
                d->idx[i][j * 2] = j + i * 33;
                d->idx[i][j * 2 + 1] = j + (i + 1) * 33;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_fx32_depth(list, s->id);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dd_tex_pack(list, 12);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->center);
        list = dl_tex_offset(d->tex_u, d->tex_v, list);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i2 = 0; i2 < 48; i2++)
            list = draw_array(list, 4, 0x119F, 66, d->idx[i2], d->tube);
        d->tex_u = d->tex_u + 0.05f;
        d->tex_v = d->tex_v - 0.2f;
        mtx_pop();
        dl_set_list(list);
    }
    dd_fx32_mask_end(s->id);

    /* spinning sparks: 8 frames of a 4 x 2 sheet */
    if (phase == 1) {
        cw = 0.25f;
        ch = 0.5f;
        hw = 18.0f;
        for (j2 = 0; j2 < 8; j2++) {
            tu = cw * (j2 % 4);
            tv = ch * (j2 / 4);
            d->spin_v[j2][0].u = tu;
            d->spin_v[j2][0].v = tv;
            d->spin_v[j2][0].x = -hw;
            d->spin_v[j2][0].y = hw;
            d->spin_v[j2][0].z = 0.0f;
            d->spin_v[j2][1].u = tu;
            d->spin_v[j2][1].v = tv + ch;
            d->spin_v[j2][1].x = -hw;
            d->spin_v[j2][1].y = -hw;
            d->spin_v[j2][1].z = 0.0f;
            d->spin_v[j2][2].u = tu + cw;
            d->spin_v[j2][2].v = tv;
            d->spin_v[j2][2].x = hw;
            d->spin_v[j2][2].y = hw;
            d->spin_v[j2][2].z = 0.0f;
            d->spin_v[j2][3].u = tu + cw;
            d->spin_v[j2][3].v = tv + ch;
            d->spin_v[j2][3].x = hw;
            d->spin_v[j2][3].y = -hw;
            d->spin_v[j2][3].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end && d->t < d->end) {
        sp = &d->spin[d->spin_next];
        sp->t = 0;
        sp->flags |= 1;
        for (i3 = 0; i3 < 8; i3++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            rad = DD_RAND(r) / 32767.0f * 80.0f;
            sn = sinf(ang);
            cs = cosf(ang);
            sp->frame[i3] = DD_RAND(r) % 8;
            sp->speed[i3] = DD_RAND(r) / 32767.0f * 10.0f + 20.0f;
            vec3_set(&sp->pos[i3], rad * sn, rad * cs + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
            sp->ang[i3] = 0.0f;
#ifdef __MWERKS__
            sp->spin[i3] = (DD_RAND(r) % 2 * 2 - 1) * (DD_RAND(r) / 32767.0f * 0.34906587f);
#else /* port: sign drawn first, as mwcc does; in C the order is unspecified */
            { int sg = DD_RAND(r) % 2 * 2 - 1; sp->spin[i3] = sg * (DD_RAND(r) / 32767.0f * 0.34906587f); }
#endif
        }
        if (d->spin_next < 23)
            d->spin_next++;
        else
            d->spin_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dd_tex_pack(list, 5);
    list = dl_color(list, 0xFFFFFFFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (j3 = 0; j3 < 24; j3++) {
        int jd;
        sp_0 = &d->spin[j3];
        if (!(sp_0->flags & 1) == 1)
            continue;
        dd_ease_lin(sp_0->t, 24);
        list = dl_ambient_light(list, (unsigned int)(out * (dd_ease_cos(sp_0->t, 24, 1) * 255.0f)) << 24
                                          | 0xFFFFFF);
        for (jd = 0; jd < 8; jd++) {
            mtx_push();
            mtx_translate(&sp_0->pos[jd]);
            mtx_mult(&bill);
            mtx_rotate_z(sp_0->ang[jd]);
            list = draw_array(list, 4, 0x183, 4, 0, d->spin_v[sp_0->frame[jd]]);
            mtx_pop();
            sp_0->pos[jd].z -= sp_0->speed[jd];
            sp_0->ang[jd] += sp_0->spin[jd];
        }
        if (sp_0->t < 23)
            sp_0->t++;
        else
            sp_0->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    dd_mtx_pop_all();
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}

/* ---- style 3: comets with long fading tails rushing at the camera ---- */

int dd_fx32_s3_update(int phase, DdFx32S3 *d);

int dd_fx32_s3_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_fx32_s3_update, a, b, sizeof(DdFx32S3));
    DdFx32S3 *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* 12 palavras: no laço do rastro o -1,0 é içado antes do 700 (no original, depois do -80) e f17-f19 rodados; ordem de declaração e forma de (j + -1.0f) * 700.0f * 0.2f não mudam */
int dd_fx32_s3_update(int phase, DdFx32S3 *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    ScePspFMatrix4 bill;
    DdRandSeed *r;
    DdFx32Comets *c;
    DdTCVtx *v;
    void *list;
    unsigned int color;
    float out;
    float tu;
    float tv;
    float k;
    float z;
    float ang;
    float cw;
    float ch;
    float hw;
    float hh;
    float tv2;
    int i;
    int j;
    int i3;
    int i2;
    DdFx32Comets * c_0;

    dd_mtx_push_saved();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    dd_ease_sin(d->t, 8, 1);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 8), 8);
    dd_mtx_billboard_cam(&bill);
    dd_cue_se(0x6236, d->grow, d->grow_end, &d->se_done, 1);
    if (phase == 1) {
        cw = 0.25f;
        ch = 0.5f;
        hw = 30.0f;
        hh = 80.0f;
        for (i = 0; i < 8; i++) {
            tu = i % 4 * cw;
            tv = i / 4 * ch;
            /* the head of the tail, opaque */
            for (j = 0; j < 2; j++) {
                v = d->tail_v[i][j];
                z = (j + -1.0f) * 700.0f * 0.2f;
                v[0].u = tu;
                v[0].v = tv + j * 0.25f;
                v[0].color = 0xFF9B9BFF;
                v[0].x = -hw;
                v[0].y = 0.0f;
                v[0].z = z;
                v[1].u = tu + cw;
                v[1].v = tv + j * 0.25f;
                v[1].color = 0xFF9B9BFF;
                v[1].x = hw;
                v[1].y = 0.0f;
                v[1].z = z;
            }
            /* the rest fades out over 1400 units */
            for (j = 2; j < 9; j++) {
                k = (j - 1) / 7.0f;
                z = k * 1400.0f;
                tv2 = tv + (k * 0.5f + 0.5f) * 0.5f;
                color = (unsigned int)((1.0f - k) * 255.0f) << 24 | 0x9B9BFF;
                d->tail_v[i][j][0].u = tu;
                d->tail_v[i][j][0].v = tv2;
                d->tail_v[i][j][0].color = color;
                d->tail_v[i][j][0].x = -hw;
                d->tail_v[i][j][0].y = 0.0f;
                d->tail_v[i][j][0].z = z;
                d->tail_v[i][j][1].u = tu + cw;
                d->tail_v[i][j][1].v = tv2;
                d->tail_v[i][j][1].color = color;
                d->tail_v[i][j][1].x = hw;
                d->tail_v[i][j][1].y = 0.0f;
                d->tail_v[i][j][1].z = z;
            }
            v = d->head_v[i];
            v[0].u = tu;
            v[0].v = tv;
            v[0].color = 0xFF9B9BFF;
            v[0].x = -hh;
            v[0].y = hh;
            v[0].z = 0.0f;
            v[1].u = tu + cw;
            v[1].v = tv + ch;
            v[1].color = 0xFF9B9BFF;
            v[1].x = hh;
            v[1].y = -hh;
            v[1].z = 0.0f;
        }
        for (i = 0; i < 9; i++) {
            d->idx[i * 2] = i * 2;
            d->idx[i * 2 + 1] = i * 2 + 1;
        }
    }
    if (d->grow == d->grow_end) {
        c = &d->comet[d->comet_next];
        c->t = 0;
        c->flags |= 1;
        for (i2 = 0; i2 < 6; i2++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            c->frame[i2] = DD_RAND(r) % 8;
            vec3_set(&c->pos[i2], sinf(ang) * 220.0f, cosf(ang) * 220.0f + 120.0f,
                     DD_RAND(r) / 32767.0f * 160.0f + 20.0f);
            c->speed[i2] = (DD_RAND(r) / 32767.0f * 60.0f + 180.0f) * -1.0f;
            c->ang[i2] = -ang;
        }
        if (d->comet_next < 31)
            d->comet_next++;
        else
            d->comet_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_fx32_depth(list, s->id);
    list = dd_blend_enable(list, 1, 0, 6, 10, 0, -1);
    list = dd_tex_pack(list, 15);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 32; i++) {
        int j2;
        c_0 = &d->comet[i];
        if (!(c_0->flags & 1) == 1)
            continue;
        list = dl_ambient_light(list, (unsigned int)(out * (dd_ease_cos(c_0->t, 32, 1) * 255.0f)) << 24 | 0xFFFFFF);
        for (j2 = 0; j2 < 6; j2++) {
            mtx_push();
            mtx_translate(&c_0->pos[j2]);
            mtx_push();
            mtx_rotate_z(c_0->ang[j2]);
            list = draw_array(list, 4, 0x99F, 18, d->idx, d->tail_v[c_0->frame[j2]]);
            mtx_pop();
            mtx_mult(&bill);
            list = draw_array(list, 6, 0x19F, 2, 0, d->head_v[c_0->frame[j2]]);
            mtx_pop();
            c_0->pos[j2].z += c_0->speed[j2];
        }
        if (c_0->t < 31)
            c_0->t++;
        else
            c_0->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    dd_fx32_mask_end(s->id);
    dd_mtx_pop_all();
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}
#endif

/* ---- style 4: a red tube of light, red and white spinning sparks and a wobbling screen ---- */

int dd_fx32_s4_update(int phase, DdFx32S4 *d);

int dd_fx32_s4_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_fx32_s4_update, a, b, sizeof(DdFx32S4));
    DdFx32S4 *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

/* draws the sparks from..to-1 of `sp` and moves them */
void *dd_fx32_s4_spin_draw(void *list, int from, int to, DdFx32Spin *sp, DdFx32S4 *d, ScePspFMatrix4 *bill)
{
    int i;

    for (i = from; i < to; i++) {
        mtx_push();
        mtx_translate(&sp->pos[i]);
        mtx_mult(bill);
        mtx_rotate_z(sp->ang[i]);
        list = draw_array(list, 4, 0x183, 4, 0, d->spin_v[sp->frame[i]]);
        mtx_pop();
        sp->pos[i].z -= sp->speed[i];
        sp->ang[i] += sp->spin[i];
    }
    return list;
}

int dd_fx32_s4_update(int phase, DdFx32S4 *d)
{
    int k5;
    int k4;
    int k3;
    int k2;
    int k1;
    int k0;
    int gx;
    int gy;
    DdFxSlot *s = d->slot;
    int run = s->param;
    int buf = d->frame % 2;
    ScePspFMatrix4 bill;
    DdRandSeed *r;
    DdFx32Spin *sp;
    DdTCVtx *v;
    DdRingVtx *q;
    void *list;
    unsigned int alpha;
    float in;
    float out;
    float ang;
    float fr;
    float z;
    float rad;
    float tu;
    float tv;
    int sign;
    float cw;
    float ch;
    float hw;
    float tvv;
    float sn;
    float cs;
    unsigned int c;
    int i;
    int j;
    int j3;
    int i3;
    int j2;
    int i2;
    DdFx32Spin *sp_0;

    dd_mtx_push_saved();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = dd_ease_sin(d->t, 8, 1);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 16), 16);
    dd_mtx_billboard_cam(&bill);
    dd_cue_se(0x6236, d->grow, d->grow_end, &d->se_done, 1);

    /* the tube */
    if (phase == 1) {
        vec3_set(&d->center, 0.0f, 120.0f, 160.0f);
        for (i = 0; i < 49; i++) {
            fr = i / 48.0f;
            z = fr * -6000.0f;
            tvv = fr * 4.0f;
            alpha = (unsigned int)((1.0f - fr) * 255.0f);
            c = alpha << 24 | 0x1860FF;
            for (j = 0; j < 33; j++) {
                ang = j / 32.0f * 6.2831855f;
                d->tube[i][j].u = j / 32.0f * 2.0f;
                d->tube[i][j].v = tvv;
                d->tube[i][j].color = c;
                d->tube[i][j].x = sinf(ang) * 220.0f;
                d->tube[i][j].y = cosf(ang) * 220.0f;
                d->tube[i][j].z = z;
            }
        }
        for (i = 0; i < 48; i++) {
            for (j = 0; j < 33; j++) {
                d->idx[i][j * 2] = j + i * 33;
                d->idx[i][j * 2 + 1] = j + (i + 1) * 33;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_fx32_depth(list, s->id);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dd_tex_pack(list, 12);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->center);
        list = dl_tex_offset(d->tex_u, d->tex_v, list);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i2 = 0; i2 < 48; i2++)
            list = draw_array(list, 4, 0x119F, 66, d->idx[i2], d->tube);
        d->tex_u = d->tex_u + 0.05f;
        d->tex_v = d->tex_v - 0.2f;
        mtx_pop();
        dl_set_list(list);
    }
    dd_fx32_mask_end(s->id);

    /* spinning sparks: the first four red, the other four white */
    if (phase == 1) {
        cw = 0.25f;
        ch = 0.5f;
        hw = 18.0f;
        for (j2 = 0; j2 < 8; j2++) {
            tu = cw * (j2 % 4);
            tv = ch * (j2 / 4);
            d->spin_v[j2][0].u = tu;
            d->spin_v[j2][0].v = tv;
            d->spin_v[j2][0].x = -hw;
            d->spin_v[j2][0].y = hw;
            d->spin_v[j2][0].z = 0.0f;
            d->spin_v[j2][1].u = tu;
            d->spin_v[j2][1].v = tv + ch;
            d->spin_v[j2][1].x = -hw;
            d->spin_v[j2][1].y = -hw;
            d->spin_v[j2][1].z = 0.0f;
            d->spin_v[j2][2].u = tu + cw;
            d->spin_v[j2][2].v = tv;
            d->spin_v[j2][2].x = hw;
            d->spin_v[j2][2].y = hw;
            d->spin_v[j2][2].z = 0.0f;
            d->spin_v[j2][3].u = tu + cw;
            d->spin_v[j2][3].v = tv + ch;
            d->spin_v[j2][3].x = hw;
            d->spin_v[j2][3].y = -hw;
            d->spin_v[j2][3].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        sp = &d->spin[d->spin_next];
        sp->t = 0;
        sp->flags |= 1;
        for (i3 = 0; i3 < 8; i3++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            rad = DD_RAND(r) / 32767.0f * 80.0f;
            sn = sinf(ang);
            cs = cosf(ang);
            sp->frame[i3] = DD_RAND(r) % 8;
            sp->speed[i3] = DD_RAND(r) / 32767.0f * 10.0f + 25.0f;
            vec3_set(&sp->pos[i3], rad * sn, rad * cs + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
            sp->ang[i3] = 0.0f;
#ifdef __MWERKS__
            sp->spin[i3] = (DD_RAND(r) % 2 * 2 - 1) * (DD_RAND(r) / 32767.0f * 0.34906587f);
#else /* port: sign drawn first, as mwcc does; in C the order is unspecified */
            { int sg = DD_RAND(r) % 2 * 2 - 1; sp->spin[i3] = sg * (DD_RAND(r) / 32767.0f * 0.34906587f); }
#endif
        }
        if (d->spin_next < 23)
            d->spin_next++;
        else
            d->spin_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_tex_pack(list, 5);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (j3 = 0; j3 < 24; j3++) {
        sp_0 = &d->spin[j3];
        if (!(sp_0->flags & 1) == 1)
            continue;
        dd_ease_lin(sp_0->t, 24);
        list = dl_ambient_light(list, (unsigned int)(out * (dd_ease_cos(sp_0->t, 24, 1) * 255.0f)) << 24
                                          | 0xFFFFFF);
        list = dd_blend_enable(list, 1, 0, 6, 10, 0, -1);
        list = dl_color(list, 0xFF1860FF);
        list = dd_fx32_s4_spin_draw(list, 0, 4, sp_0, d, &bill);
        list = dd_blend_enable(list, 1, 2, 6, 10, 0, -1);
        list = dl_color(list, 0xFFFFFFFF);
        list = dd_fx32_s4_spin_draw(list, 4, 8, sp_0, d, &bill);
        if (sp_0->t < 23)
            sp_0->t++;
        else
            sp_0->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);

    /* the screen redrawn over a 9 x 9 grid whose inner points wobble by 2 pixels */
    if (phase == 1) {
        for (k2 = 0; k2 < 9; k2++) {
            gy = k2 / 8.0f * 272.0f;
            for (k1 = 0; k1 < 9; k1++) {
                gx = k1 / 8.0f * 480.0f;
                d->grid[k2][k1].u = gx;
                d->grid[k2][k1].v = gy;
                d->grid[k2][k1].x = gx;
                d->grid[k2][k1].y = gy;
                d->grid[k2][k1].z = 0;
            }
        }
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 9; j++) {
                d->grid_idx[i][j * 2] = j + i * 9;
                d->grid_idx[i][j * 2 + 1] = j + (i + 1) * 9;
            }
        }
        for (i = 0; i < 9; i++) {
            for (j = 0; j < 9; j++) {
                d->phase[i][j] = DD_RAND(r) / 32767.0f * 6.2831855f;
                d->sign[i][j] = DD_RAND(r) % 2 * 2 - 1;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 4, 0, 1, 1);
        list = dd_depth_enable(list, 0, 0);
        list = dd_blend_enable(list, 1, 0, 2, 3, 0, -1);
        list = dd_tex_fb(list);
        for (k3 = 1; k3 < 8; k3++) {
            gy = k3 / 8.0f * 272.0f;
            for (k4 = 1; k4 < 8; k4++) {
                ang = d->sign[k3][k4] * (d->phase[k3][k4] + d->frame * 0.043633234f);
                d->grid[k3][k4].x = (int)(k4 / 8.0f * 480.0f) + roundf_vfpu(sinf(ang) * 2.0f);
                d->grid[k3][k4].y = gy + roundf_vfpu(cosf(ang) * 2.0f);
            }
        }
        memcpy(d->grid_buf[buf], d->grid, sizeof(d->grid));
        list = dl_color(list, (unsigned int)(in * 226.0f * dd_ease_lin_out(d->t, 8, d->end)) << 24 | 0x818);
        for (k5 = 0; k5 < 8; k5++)
            list = draw_array(list, 4, 0x800902, 18, d->grid_idx[k5], d->grid_buf[buf]);
        dl_set_list(list);
    }
    dd_mtx_pop_all();
    d->frame++;
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}

/* ---- style 5: a white tube of light, spinning sparks and a fast wobbling screen ---- */

int dd_fx32_s5_update(int phase, DdFx32S5 *d);

int dd_fx32_s5_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_fx32_s5_update, a, b, sizeof(DdFx32S5));
    DdFx32S5 *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

int dd_fx32_s5_update(int phase, DdFx32S5 *d)
{
    int k5;
    int gx;
    int gy;
    int k4;
    int k3;
    int k2;
    int k1;
    int k0;
    DdFxSlot *s = d->slot;
    int run = s->param;
    int buf = d->frame % 2;
    ScePspFMatrix4 bill;
    DdRandSeed *r;
    DdFx32Spin4 *sp;
    DdTCVtx *v;
    DdRingVtx *q;
    void *list;
    unsigned int alpha;
    float in;
    float out;
    float ang;
    float fr;
    float z;
    float rad;
    float tu;
    float tv;
    float cw;
    float ch;
    float hw;
    float tvv;
    float sn;
    float cs;
    unsigned int c;
    int i;
    int j;
    DdFx32Spin4 *sq;
    int j3;
    int i3;
    int j2;
    int i2;

    dd_mtx_push_saved();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = dd_ease_sin(d->t, 8, 1);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 8), 8);
    dd_mtx_billboard_cam(&bill);
    dd_cue_se(0x6236, d->grow, d->grow_end, &d->se_done, 1);

    /* the tube */
    if (phase == 1) {
        vec3_set(&d->center, 0.0f, 120.0f, 160.0f);
        for (i = 0; i < 49; i++) {
            fr = i / 48.0f;
            z = fr * -6000.0f;
            tvv = fr * 5.0f;
            alpha = (unsigned int)((1.0f - fr) * 255.0f);
            c = alpha << 24 | 0xFFFFFF;
            for (j = 0; j < 33; j++) {
                ang = j / 32.0f * 6.2831855f;
                d->tube[i][j].u = j / 32.0f * 3.0f;
                d->tube[i][j].v = tvv;
                d->tube[i][j].color = c;
                d->tube[i][j].x = sinf(ang) * 220.0f;
                d->tube[i][j].y = cosf(ang) * 220.0f;
                d->tube[i][j].z = z;
            }
        }
        for (i = 0; i < 48; i++) {
            for (j = 0; j < 33; j++) {
                d->idx[i][j * 2] = j + i * 33;
                d->idx[i][j * 2 + 1] = j + (i + 1) * 33;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_fx32_depth(list, s->id);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dd_tex_pack(list, 9);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->center);
        list = dl_tex_offset(d->tex_u, d->tex_v, list);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i2 = 0; i2 < 48; i2++)
            list = draw_array(list, 4, 0x119F, 66, d->idx[i2], d->tube);
        d->tex_u = d->tex_u + 0.01f;
        d->tex_v = d->tex_v - 0.1f;
        mtx_pop();
        dl_set_list(list);
    }
    dd_fx32_mask_end(s->id);

    /* spinning sparks */
    if (phase == 1) {
        cw = 0.25f;
        ch = 0.5f;
        hw = 32.0f;
        for (j2 = 0; j2 < 8; j2++) {
            tu = cw * (j2 % 4);
            tv = ch * (j2 / 4);
            d->spin_v[j2][0].u = tu;
            d->spin_v[j2][0].v = tv;
            d->spin_v[j2][0].x = -hw;
            d->spin_v[j2][0].y = hw;
            d->spin_v[j2][0].z = 0.0f;
            d->spin_v[j2][1].u = tu;
            d->spin_v[j2][1].v = tv + ch;
            d->spin_v[j2][1].x = -hw;
            d->spin_v[j2][1].y = -hw;
            d->spin_v[j2][1].z = 0.0f;
            d->spin_v[j2][2].u = tu + cw;
            d->spin_v[j2][2].v = tv;
            d->spin_v[j2][2].x = hw;
            d->spin_v[j2][2].y = hw;
            d->spin_v[j2][2].z = 0.0f;
            d->spin_v[j2][3].u = tu + cw;
            d->spin_v[j2][3].v = tv + ch;
            d->spin_v[j2][3].x = hw;
            d->spin_v[j2][3].y = -hw;
            d->spin_v[j2][3].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        sp = &d->spin[d->spin_next];
        sp->t = 0;
        sp->flags |= 1;
        for (i3 = 0; i3 < 4; i3++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            rad = DD_RAND(r) / 32767.0f * 80.0f;
            sn = sinf(ang);
            cs = cosf(ang);
            sp->frame[i3] = DD_RAND(r) % 8;
            sp->speed[i3] = DD_RAND(r) / 32767.0f * 10.0f + 20.0f;
            vec3_set(&sp->pos[i3], rad * sn, rad * cs + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
            sp->ang[i3] = 0.0f;
#ifdef __MWERKS__
            sp->spin[i3] = (DD_RAND(r) % 2 * 2 - 1) * (DD_RAND(r) / 32767.0f * 0.34906587f);
#else /* port: sign drawn first, as mwcc does; in C the order is unspecified */
            { int sg = DD_RAND(r) % 2 * 2 - 1; sp->spin[i3] = sg * (DD_RAND(r) / 32767.0f * 0.34906587f); }
#endif
        }
        if (d->spin_next < 23)
            d->spin_next++;
        else
            d->spin_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_tex_pack(list, 20);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (j3 = 0; j3 < 24; j3++) {
        int jd;
        sq = &d->spin[j3];
        if (!(sq->flags & 1) == 1)
            continue;
        dd_ease_lin(sq->t, 24);
        list = dl_ambient_light(list, (unsigned int)(out * (dd_ease_cos(sq->t, 24, 1) * 255.0f)) << 24 | 0xFFFFFF);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dl_color(list, 0x80FFFFFF);
        for (jd = 0; jd < 4; jd++) {
            mtx_push();
            mtx_translate(&sq->pos[jd]);
            mtx_mult(&bill);
            mtx_rotate_z(sq->ang[jd]);
            list = draw_array(list, 4, 0x183, 4, 0, d->spin_v[sq->frame[jd]]);
            mtx_pop();
            sq->pos[jd].z -= sq->speed[jd];
            sq->ang[jd] += sq->spin[jd];
        }
        if (sq->t < 23)
            sq->t++;
        else
            sq->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);

    /* the screen redrawn over a 16 x 16 grid whose inner points wobble by 2 pixels */
    if (phase == 1) {
        for (k2 = 0; k2 < 16; k2++) {
            gy = k2 / 15.0f * 272.0f;
            for (k1 = 0; k1 < 16; k1++) {
                gx = k1 / 15.0f * 480.0f;
                d->grid[k2][k1].u = gx;
                d->grid[k2][k1].v = gy;
                d->grid[k2][k1].x = gx;
                d->grid[k2][k1].y = gy;
                d->grid[k2][k1].z = 0;
            }
        }
        for (i = 0; i < 15; i++) {
            for (j = 0; j < 16; j++) {
                d->grid_idx[i][j * 2] = i * 16 + j;
                d->grid_idx[i][j * 2 + 1] = (i + 1) * 16 + j;
            }
        }
        for (i = 0; i < 16; i++) {
            for (j = 0; j < 16; j++) {
                d->phase[i][j] = DD_RAND(r) / 32767.0f * 6.2831855f;
                d->sign[i][j] = DD_RAND(r) % 2 * 2 - 1;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
        list = dd_depth_enable(list, 0, 0);
        list = dd_blend_enable(list, 1, 0, 2, 3, 0, -1);
        list = dd_tex_fb(list);
        for (k3 = 1; k3 < 15; k3++) {
            gy = k3 / 15.0f * 272.0f;
            for (k4 = 1; k4 < 15; k4++) {
                ang = d->sign[k3][k4] * (d->phase[k3][k4] + d->frame * 0.34906587f);
                d->grid[k3][k4].x = (int)(k4 / 15.0f * 480.0f) + roundf_vfpu(sinf(ang) * 2.0f);
                d->grid[k3][k4].y = gy + roundf_vfpu(cosf(ang) * 2.0f);
            }
        }
        memcpy(d->grid_buf[buf], d->grid, sizeof(d->grid));
        list = dl_color(list, (unsigned int)(in * 200.0f * out) << 24 | 0xFFFFFF);
        for (k5 = 0; k5 < 15; k5++)
            list = draw_array(list, 4, 0x800902, 32, d->grid_idx[k5], d->grid_buf[buf]);
        dl_set_list(list);
    }
    dd_mtx_pop_all();
    d->frame++;
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}

/* ---- style 6: long beams of light radiating around the center ---- */

int dd_fx32_s6_update(int phase, DdFx32S6 *d);

int dd_fx32_s6_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_fx32_s6_update, a, b, sizeof(DdFx32S6));
    DdFx32S6 *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

int dd_fx32_s6_update(int phase, DdFx32S6 *d)
{
    int k0;
    DdFxSlot *s = d->slot;
    int run = s->param;
    DdRandSeed *r;
    DdFx32Beams *b;
    DdTCVtx *v;
    void *list;
    float out;
    float fr;
    float z;
    float tv;
    float ang;
    float e;
    float hw;
    int i;
    int j;
    DdFx32Beams *b_0;

    dd_mtx_push_saved();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    dd_ease_sin(d->t, 8, 1);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 8), 8);
    dd_cue_se(0x6236, d->grow, d->grow_end, &d->se_done, 1);
    if (phase == 1) {
        hw = 80.0f;
        for (k0 = 0; k0 < 49; k0++) {
            v = d->beam_v[k0];
            fr = k0 / 48.0f;
            z = fr * -6000.0f;
            tv = fr * 6.0f;
            v[0].u = 0.0f;
            v[0].v = tv;
            v[0].color = 0xFFFFFF;
            v[0].x = -hw;
            v[0].y = 0.0f;
            v[0].z = z;
            v[1].u = 0.5f;
            v[1].v = tv;
            d->beam_v[k0][1].color = (unsigned int)((1.0f - fr) * 255.0f) << 24 | 0xFFFFFF;
            d->beam_v[k0][1].x = 0.0f;
            d->beam_v[k0][1].y = 0.0f;
            d->beam_v[k0][1].z = z;
            d->beam_v[k0][2].u = 1.0f;
            d->beam_v[k0][2].v = tv;
            d->beam_v[k0][2].color = 0xFFFFFF;
            d->beam_v[k0][2].x = hw;
            d->beam_v[k0][2].y = 0.0f;
            d->beam_v[k0][2].z = z;
            d->idx_a[k0 * 2] = k0 * 3;
            d->idx_a[k0 * 2 + 1] = k0 * 3 + 1;
            d->idx_b[k0 * 2] = k0 * 3 + 1;
            d->idx_b[k0 * 2 + 1] = k0 * 3 + 2;
        }
    }
    if (d->grow == d->grow_end) {
        b = &d->beam[d->beam_next];
        b->t = 0;
        b->flags |= 1;
        for (i = 0; i < 8; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            vec3_set(&b->pos[i], sinf(ang) * 220.0f, cosf(ang) * 220.0f + 120.0f, 160.0f);
            b->ang[i] = -ang;
            b->width[i] = DD_RAND(r) / 32767.0f * 0.5f + 0.5f;
        }
        if (d->beam_next < 7)
            d->beam_next++;
        else
            d->beam_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
    list = dd_fx32_depth(list, s->id);
    list = dd_blend_enable(list, 1, 0, 6, 10, 0, -1);
    list = dd_tex_pack(list, 21);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 8; i++) {
        int j;
        b_0 = &d->beam[i];
        if (!(b_0->flags & 1) == 1)
            continue;
        e = dd_ease_sin(b_0->t, 8, 1);
        list = dl_ambient_light(list, (unsigned int)(out * (dd_ease_cos(b_0->t, 8, 1) * 255.0f)) << 24 | 0xFFFFFF);
        for (j = 0; j < 8; j++) {
            list = dl_tex_offset(0.0f, DD_RAND(r) / 32767.0f, list);
            mtx_push();
            mtx_translate(&b_0->pos[j]);
            mtx_rotate_z(b_0->ang[j]);
            dd_scale3(b_0->width[j] * e, 0.0f, 1.0f);
            list = draw_array(list, 4, 0x99F, 98, d->idx_a, d->beam_v);
            list = draw_array(list, 4, 0x99F, 98, d->idx_b, d->beam_v);
            mtx_pop();
        }
        if (b_0->t < 7)
            b_0->t++;
        else
            b_0->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    dd_fx32_mask_end(s->id);
    dd_mtx_pop_all();
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}

/* ---- style 7: short golden blades rushing at the camera ---- */

int dd_fx32_s7_update(int phase, DdFx32S7 *d);

int dd_fx32_s7_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_fx32_s7_update, a, b, sizeof(DdFx32S7));
    DdFx32S7 *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

int dd_fx32_s7_update(int phase, DdFx32S7 *d)
{
    int k0;
    DdFxSlot *s = d->slot;
    int run = s->param;
    DdRandSeed *r;
    DdFx32Blades *bl;
    DdRingVtx *v;
    void *list;
    float out;
    float ang;
    float z;
    float tv;
    float hl;
    float hw;
    float cw;
    float ch;
    float cu;
    float cv;
    float f;
    int i;
    int j;
    DdFx32Blades *bl_0;

    dd_mtx_push_saved();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    dd_ease_sin(d->t, 8, 1);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 8), 8);
    dd_cue_se(0x6236, d->grow, d->grow_end, &d->se_done, 1);
    if (phase == 1) {
        hl = 500.0f;
        hw = 50.0f;
        cw = 0.25f;
        ch = 0.5f;
        for (k0 = 0; k0 < 8; k0++) {
            cu = k0 % 4 * cw;
            cv = k0 / 4 * ch;
            for (j = 0; j < 7; j++) {
                f = j / 6.0f;
                z = -hl + f * 1000.0f;
                tv = cv + ch * f;
                d->blade_v[k0][j][0].u = cu;
                d->blade_v[k0][j][0].v = tv;
                d->blade_v[k0][j][0].x = -hw;
                d->blade_v[k0][j][0].y = 0.0f;
                d->blade_v[k0][j][0].z = z;
                d->blade_v[k0][j][1].u = cu + cw;
                d->blade_v[k0][j][1].v = tv;
                d->blade_v[k0][j][1].x = hw;
                d->blade_v[k0][j][1].y = 0.0f;
                d->blade_v[k0][j][1].z = z;
            }
        }
        for (i = 0; i < 7; i++) {
            d->idx[i * 2] = i * 2;
            d->idx[i * 2 + 1] = i * 2 + 1;
        }
    }
    if (d->grow == d->grow_end) {
        bl = &d->blade[d->blade_next];
        bl->t = 0;
        bl->flags |= 1;
        for (i = 0; i < 12; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            bl->frame[i] = DD_RAND(r) % 8;
            vec3_set(&bl->pos[i], sinf(ang) * 220.0f, cosf(ang) * 220.0f + 120.0f, 160.0f);
            bl->speed[i] = (DD_RAND(r) / 32767.0f * 60.0f + 180.0f) * -1.0f;
            bl->ang[i] = -ang;
        }
        if (d->blade_next < 31)
            d->blade_next++;
        else
            d->blade_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_fx32_depth(list, s->id);
    list = dd_blend_enable(list, 1, 0, 6, 10, 0, -1);
    list = dd_tex_pack(list, 14);
    list = dl_color(list, 0xFFC864FF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 32; i++) {
        int j;
        bl_0 = &d->blade[i];
        if (!(bl_0->flags & 1) == 1)
            continue;
        list = dl_ambient_light(list, (unsigned int)(out * (dd_ease_cos(bl_0->t, 32, 1) * 255.0f)) << 24
                                          | 0xFFFFFF);
        for (j = 0; j < 12; j++) {
            mtx_push();
            mtx_translate(&bl_0->pos[j]);
            mtx_rotate_z(bl_0->ang[j]);
            list = draw_array(list, 4, 0x983, 14, d->idx, d->blade_v[bl_0->frame[j]]);
            mtx_pop();
            bl_0->pos[j].z += bl_0->speed[j];
        }
        if (bl_0->t < 31)
            bl_0->t++;
        else
            bl_0->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    dd_fx32_mask_end(s->id);
    dd_mtx_pop_all();
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}

/* ---- style 8: a blue tube of light scrolling into the screen ---- */

int dd_fx32_s8_update(int phase, DdFx32S8 *d);

int dd_fx32_s8_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_fx32_s8_update, a, b, sizeof(DdFx32S8));
    DdFx32S8 *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

int dd_fx32_s8_update(int phase, DdFx32S8 *d)
{
    int k0;
    DdFxSlot *s = d->slot;
    int run = s->param;
    DdTCVtx *v;
    void *list;
    unsigned int alpha;
    float in;
    float out;
    float fr;
    float z;
    float ang;
    float tvv;
    unsigned int c;
    int i;
    int j;

    dd_mtx_push_saved();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = dd_ease_sin(d->t, 8, 1);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 8), 8);
    dd_cue_se(0x6236, d->grow, d->grow_end, &d->se_done, 1);
    if (phase == 1) {
        vec3_set(&d->center, 0.0f, 120.0f, 160.0f);
        for (i = 0; i < 49; i++) {
            fr = i / 48.0f;
            z = fr * -6000.0f;
            tvv = fr * 6.0f;
            alpha = (unsigned int)((1.0f - fr) * 255.0f);
            c = alpha << 24 | 0x2660FF;
            for (j = 0; j < 33; j++) {
                ang = j / 32.0f * 6.2831855f;
                d->tube[i][j].u = j / 32.0f * 3.0f;
                d->tube[i][j].v = tvv;
                d->tube[i][j].color = c;
                d->tube[i][j].x = sinf(ang) * 220.0f;
                d->tube[i][j].y = cosf(ang) * 220.0f;
                d->tube[i][j].z = z;
            }
        }
        for (i = 0; i < 48; i++) {
            for (j = 0; j < 33; j++) {
                d->idx[i][j * 2] = j + i * 33;
                d->idx[i][j * 2 + 1] = j + (i + 1) * 33;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_fx32_depth(list, s->id);
        list = dd_blend_enable(list, 1, 0, 6, 10, 0, -1);
        list = dd_tex_pack(list, 12);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->center);
        list = dl_tex_offset(d->tex_u, d->tex_v, list);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (k0 = 0; k0 < 48; k0++)
            list = draw_array(list, 4, 0x119F, 66, d->idx[k0], d->tube);
        d->tex_u = d->tex_u + 0.05f;
        d->tex_v = d->tex_v - 0.2f;
        mtx_pop();
        dl_set_list(list);
    }
    dd_fx32_mask_end(s->id);
    dd_mtx_pop_all();
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}
