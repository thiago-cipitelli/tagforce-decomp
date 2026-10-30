// @range 0x0883B62C-0x08842824
// @rodata 0x088707FC-0x08870820
// @asm 0x0883B9D4-0x0883C6E8
// @asm 0x0883C744-0x0883D9D4
// @asm 0x0883DA2C-0x0883E6A0
// @asm 0x0883E6F8-0x0883EF6C
// @asm 0x0883F0D8-0x08840230
// @asm 0x08840288-0x08841470
// @asm 0x088414C8-0x08841B60
// @asm 0x08841BB8-0x08842250
// @asm 0x088422A8-0x08842824
/* Effects 32-37 and 52-53: nine styles of full-screen burst picked by dd_fx_table.arg0 (style 0-8),
   each a task with its own particles, drawn through a stencil mask of the field except for effect 33,
   and the shared helpers (timings, depth state, the mask). */
#include "libduel_draw.h"

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
    return func_0882698C(list, 0, 1);
}

/* except for effect 33, draws only over the pixels the field left empty: marks them in the stencil */
void dd_fx32_mask_begin(int id)
{
    void *list;

    if (id != 33) {
        list = dl_select_layer(7, 0);
        list = dl_clear_depth(list, 0);
        list = dl_clear(list, 0x12);
        list = func_088266D8(list, 0, 1, 1, 0, 0, 0, 0);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = dl_color(list, 0xFF000000);
        list = func_0882698C(list, 1, 1);
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
    int task = func_0882623C(dd_fx32_s0_update, a, b, sizeof(DdFx32S0));
    DdFx32S0 *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 3,3 KB: alocação de registradores e laços desenrolados diferem */
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
    int i;
    int j;
    int x;
    int y;
    int w;
    int h;

    func_08827738();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = func_08827A2C(d->t, 8, 1);
    out = 1.0f - func_088279A4(d->t - (d->end - 8), 8);
    func_08827258(&bill);
    func_08827F68(0x6236, d->grow, d->grow_end, &d->se_done, 1);

    /* the blades: 8 frames of a 4 x 2 sheet stretched along z */
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 9; j++) {
                v = d->blade_v[i][j];
                z = j / 8.0f * 1600.0f + -800.0f;
                tv = i / 4 * 0.5f + j / 8.0f * 0.5f;
                v[0].u = i % 4 * 0.25f;
                v[0].v = tv;
                v[0].x = -20.0f;
                v[0].y = 0.0f;
                v[0].z = z;
                v[1].u = i % 4 * 0.25f + 0.25f;
                v[1].v = tv;
                v[1].x = 20.0f;
                v[1].y = 0.0f;
                v[1].z = z;
            }
        }
        for (i = 0; i < 18; i++)
            d->idx[i] = i;
    }
    if (d->grow == d->grow_end) {
        bl = &d->blade[d->blade_next];
        bl->t = 0;
        bl->flags |= 1;
        for (i = 0; i < 12; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            bl->frame[i] = DD_RAND(r) % 8;
            vec3_set(&bl->pos[i], sinf(ang) * 220.0f, cosf(ang) * 220.0f + 120.0f, 160.0f);
            bl->speed[i] = (DD_RAND(r) / 32767.0f * 60.0f + 240.0f) * -1.0f;
            bl->ang[i] = -ang;
        }
        if (d->blade_next < 23)
            d->blade_next++;
        else
            d->blade_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_fx32_depth(list, s->id);
    list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
    list = func_08826A64(list, 14);
    list = dl_color(list, 0xFFA0DCFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        bl = &d->blade[i];
        if (!(bl->flags & 1))
            continue;
        list = dl_ambient_light(list, (unsigned int)(out * func_08827AD4(bl->t, 24, 1) * 255.0f) << 24 | 0xFFFFFF);
        for (j = 0; j < 12; j++) {
            mtx_push();
            mtx_translate(&bl->pos[j]);
            mtx_rotate_z(bl->ang[j]);
            list = draw_array(list, 4, 0x983, 18, d->idx, d->blade_v[bl->frame[j]]);
            mtx_pop();
            bl->pos[j].z += bl->speed[j];
        }
        if (bl->t < 23)
            bl->t++;
        else
            bl->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);

    /* the motes: 4 frames of a 2 x 2 sheet */
    if (phase == 1) {
        for (i = 0; i < 4; i++) {
            d->mote_v[i][0].u = i % 2 * 0.5f;
            d->mote_v[i][0].v = i / 2 * 0.5f;
            d->mote_v[i][0].x = -80.0f;
            d->mote_v[i][0].y = 80.0f;
            d->mote_v[i][0].z = 0.0f;
            d->mote_v[i][1].u = i % 2 * 0.5f + 0.5f;
            d->mote_v[i][1].v = i / 2 * 0.5f + 0.5f;
            d->mote_v[i][1].x = 80.0f;
            d->mote_v[i][1].y = -80.0f;
            d->mote_v[i][1].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        m = &d->mote[d->mote_next];
        m->t = 0;
        m->flags |= 1;
        for (i = 0; i < 4; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            m->frame[i] = DD_RAND(r) % 4;
            m->speed[i] = (DD_RAND(r) / 32767.0f * 60.0f + 120.0f) * -1.0f;
            vec3_set(&m->pos[i], sinf(ang) * 220.0f, cosf(ang) * 220.0f + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
        }
        if (d->mote_next < 23)
            d->mote_next++;
        else
            d->mote_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_fx32_depth(list, s->id);
    list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
    list = func_08826A64(list, 10);
    list = dl_color(list, 0xC8A0DCFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        m = &d->mote[i];
        if (!(m->flags & 1))
            continue;
        func_088279A4(m->t, 24);
        list = dl_ambient_light(list, (unsigned int)(out * func_08827AD4(m->t, 24, 1) * 255.0f) << 24 | 0xFFFFFF);
        for (j = 0; j < 4; j++) {
            mtx_push();
            mtx_translate(&m->pos[j]);
            mtx_mult(&bill);
            list = draw_array(list, 6, 0x183, 2, 0, d->mote_v[m->frame[j]]);
            mtx_pop();
            m->pos[j].z += m->speed[j];
        }
        if (m->t < 23)
            m->t++;
        else
            m->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    dd_fx32_mask_end(s->id);

    /* flash: the screen copied tile by tile (64 x 32) and added back tinted */
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
        list = func_0882698C(list, 0, 0);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = dl_color(list, (unsigned int)(in * 200.0f * out) << 24 | 0xA0DCFF);
        for (i = 0, y = 0; i < 9; i++, y += 32) {
            h = i < 8 ? 32 : 272 - y;
            for (j = 0, x = 0; j < 8; j++, x += 64) {
                w = j < 7 ? 64 : 480 - x;
                list = func_08826BD8(list, x, y, 64, 32);
                list = draw_set_offsets(list, w, h, w, h);
                list = draw_sprite(list, x, y, 0, 0, 0, 0, 0);
            }
        }
        dl_set_list(list);
    }
    func_08827668();
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}
#endif

/* ---- style 1: a wavy tube of light going into the screen, motes and spinning sparks ---- */

int dd_fx32_s1_update(int phase, DdFx32S1 *d);

int dd_fx32_s1_start(DdFxSlot *s, int a, int b)
{
    int task = func_0882623C(dd_fx32_s1_update, a, b, sizeof(DdFx32S1));
    DdFx32S1 *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 4,6 KB: alocação de registradores e laços desenrolados diferem */
int dd_fx32_s1_update(int phase, DdFx32S1 *d)
{
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
    int i;
    int j;

    func_08827738();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = func_08827A2C(d->t, 8, 1);
    out = 1.0f - func_088279A4(d->t - (d->end - 8), 8);
    func_08827258(&bill);
    buf = d->frame % 2;
    func_08827F68(0x6236, d->grow, d->grow_end, &d->se_done, 1);

    /* motes */
    if (phase == 1) {
        for (i = 0; i < 4; i++) {
            d->mote_v[i][0].u = i % 2 * 0.5f;
            d->mote_v[i][0].v = i / 2 * 0.5f;
            d->mote_v[i][0].x = -120.0f;
            d->mote_v[i][0].y = 120.0f;
            d->mote_v[i][0].z = 0.0f;
            d->mote_v[i][1].u = i % 2 * 0.5f + 0.5f;
            d->mote_v[i][1].v = i / 2 * 0.5f + 0.5f;
            d->mote_v[i][1].x = 120.0f;
            d->mote_v[i][1].y = -120.0f;
            d->mote_v[i][1].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        m = &d->mote[d->mote_next];
        m->t = 0;
        m->flags |= 1;
        for (i = 0; i < 4; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            m->frame[i] = DD_RAND(r) % 4;
            m->speed[i] = (DD_RAND(r) / 32767.0f * 60.0f + 120.0f) * -1.0f;
            vec3_set(&m->pos[i], sinf(ang) * 270.0f, cosf(ang) * 270.0f + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
        }
        if (d->mote_next < 23)
            d->mote_next++;
        else
            d->mote_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_fx32_depth(list, s->id);
    list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
    list = func_08826A64(list, 10);
    list = dl_color(list, 0xFF0000FF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        m = &d->mote[i];
        if (!(m->flags & 1))
            continue;
        func_088279A4(m->t, 24);
        list = dl_ambient_light(list, (unsigned int)(out * func_08827AD4(m->t, 24, 1) * 255.0f) << 24 | 0xFFFFFF);
        for (j = 0; j < 4; j++) {
            mtx_push();
            mtx_translate(&m->pos[j]);
            mtx_mult(&bill);
            list = draw_array(list, 6, 0x183, 2, 0, d->mote_v[m->frame[j]]);
            mtx_pop();
            m->pos[j].z += m->speed[j];
        }
        if (m->t < 23)
            m->t++;
        else
            m->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);

    /* the tube: 49 rings of 33 vertices, radius waving per vertex */
    if (phase == 1) {
        vec3_set(&d->center, 0.0f, 120.0f, 160.0f);
        for (i = 0; i < 49; i++) {
            fr = i / 48.0f;
            z = fr * -6000.0f;
            alpha = (unsigned int)((1.0f - fr) * 220.0f);
            for (j = 0; j < 33; j++) {
                v = &d->tube[i][j];
                ang = j / 32.0f * 6.2831855f;
                v->u = j / 32.0f * 2.0f;
                v->v = fr * 4.0f;
                v->color = alpha << 24 | 0xFF;
                v->x = sinf(ang) * 220.0f;
                v->y = cosf(ang) * 220.0f;
                v->z = z;
            }
        }
        for (i = 0; i < 48; i++) {
            for (j = 0; j < 33; j++) {
                d->idx[i][j * 2] = i * 33 + j;
                d->idx[i][j * 2 + 1] = i * 33 + j + 33;
                d->phase[i][j] = DD_RAND(r) / 32767.0f * 6.2831855f;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_fx32_depth(list, s->id);
        list = func_088269D8(list, 1, 0, 6, 3, 0, 0);
        list = func_08826A64(list, 9);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->center);
        for (i = 0; i < 49; i++) {
            for (j = 0; j < 32; j++) {
                ang = j / 32.0f * 6.2831855f;
                rad = (sinf(d->phase[i][j] + d->frame * 0.17453294f) * 0.12f + 1.0f) * 220.0f;
                d->tube[i][j].x = rad * sinf(ang);
                d->tube[i][j].y = rad * cosf(ang);
            }
            d->tube[i][32].x = d->tube[i][0].x;
            d->tube[i][32].y = d->tube[i][0].y;
        }
        memcpy(d->tube_buf[buf], d->tube, sizeof(d->tube));
        list = dl_tex_offset(d->tex_u, d->tex_v, list);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
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
        for (i = 0; i < 8; i++) {
            q = d->spin_v[i];
            tu = i % 4 * 0.25f;
            tv = i / 4 * 0.5f;
            q[0].u = tu;
            q[0].v = tv;
            q[0].x = -25.0f;
            q[0].y = 25.0f;
            q[0].z = 0.0f;
            q[1].u = tu;
            q[1].v = tv + 0.5f;
            q[1].x = -25.0f;
            q[1].y = -25.0f;
            q[1].z = 0.0f;
            q[2].u = tu + 0.25f;
            q[2].v = tv;
            q[2].x = 25.0f;
            q[2].y = 25.0f;
            q[2].z = 0.0f;
            q[3].u = tu + 0.25f;
            q[3].v = tv + 0.5f;
            q[3].x = 25.0f;
            q[3].y = -25.0f;
            q[3].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end && d->t < d->end) {
        sp = &d->spin[d->spin_next];
        sp->t = 0;
        sp->flags |= 1;
        for (i = 0; i < 8; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            rad = DD_RAND(r) / 32767.0f * 100.0f;
            sp->frame[i] = DD_RAND(r) % 8;
            sp->speed[i] = DD_RAND(r) / 32767.0f * 10.0f + 20.0f;
            vec3_set(&sp->pos[i], rad * sinf(ang), rad * cosf(ang) + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
            sp->ang[i] = 0.0f;
            sign = (DD_RAND(r) & 1) * 2 - 1;
            sp->spin[i] = DD_RAND(r) / 32767.0f * 0.34906587f * sign;
        }
        if (d->spin_next < 23)
            d->spin_next++;
        else
            d->spin_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
    list = func_0882698C(list, 1, 1);
    list = func_088269D8(list, 1, 2, 2, 10, 0, -1);
    list = func_08826A64(list, 5);
    list = dl_color(list, 0x80FFFFFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        sp = &d->spin[i];
        if (!(sp->flags & 1))
            continue;
        func_088279A4(sp->t, 24);
        list = dl_ambient_light(list, (unsigned int)(out * func_08827AD4(sp->t, 24, 1) * 255.0f) << 24 | 0xFFFFFF);
        for (j = 0; j < 8; j++) {
            mtx_push();
            mtx_translate(&sp->pos[j]);
            mtx_mult(&bill);
            mtx_rotate_z(sp->ang[j]);
            list = draw_array(list, 4, 0x183, 4, 0, d->spin_v[sp->frame[j]]);
            mtx_pop();
            sp->pos[j].z -= sp->speed[j];
            sp->ang[j] += sp->spin[j];
        }
        if (sp->t < 23)
            sp->t++;
        else
            sp->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    func_08827668();
    d->frame++;
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}
#endif

/* ---- style 2: a white tube of light scrolling fast into the screen and spinning sparks ---- */

int dd_fx32_s2_update(int phase, DdFx32S2 *d);

int dd_fx32_s2_start(DdFxSlot *s, int a, int b)
{
    int task = func_0882623C(dd_fx32_s2_update, a, b, sizeof(DdFx32S2));
    DdFx32S2 *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 3,1 KB: alocação de registradores e laços desenrolados diferem */
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
    int i;
    int j;

    func_08827738();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = func_08827A2C(d->t, 8, 1);
    out = func_088279A4(d->t - (d->end - 8), 8);
    func_08827258(&bill);
    func_08827F68(0x6236, d->grow, d->grow_end, &d->se_done, 1);

    /* the tube: 49 rings of 33 vertices */
    if (phase == 1) {
        vec3_set(&d->center, 0.0f, 120.0f, 160.0f);
        for (i = 0; i < 49; i++) {
            fr = i / 48.0f;
            z = fr * -6000.0f;
            alpha = (unsigned int)((1.0f - fr) * 255.0f);
            for (j = 0; j < 33; j++) {
                v = &d->tube[i][j];
                ang = j / 32.0f * 6.2831855f;
                v->u = j / 32.0f * 2.0f;
                v->v = fr * 4.0f;
                v->color = alpha << 24 | 0xFFFFFF;
                v->x = sinf(ang) * 220.0f;
                v->y = cosf(ang) * 220.0f;
                v->z = z;
            }
        }
        for (i = 0; i < 48; i++) {
            for (j = 0; j < 33; j++) {
                d->idx[i][j * 2] = i * 33 + j;
                d->idx[i][j * 2 + 1] = i * 33 + j + 33;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_fx32_depth(list, s->id);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = func_08826A64(list, 12);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->center);
        list = dl_tex_offset(d->tex_u, d->tex_v, list);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * (1.0f - out)) << 24 | 0xFFFFFF);
        for (i = 0; i < 48; i++)
            list = draw_array(list, 4, 0x119F, 66, d->idx[i], d->tube);
        d->tex_u = d->tex_u + 0.05f;
        d->tex_v = d->tex_v - 0.2f;
        mtx_pop();
        dl_set_list(list);
    }
    dd_fx32_mask_end(s->id);

    /* spinning sparks: 8 frames of a 4 x 2 sheet */
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            q = d->spin_v[i];
            tu = i % 4 * 0.25f;
            tv = i / 4 * 0.5f;
            q[0].u = tu;
            q[0].v = tv;
            q[0].x = -18.0f;
            q[0].y = 18.0f;
            q[0].z = 0.0f;
            q[1].u = tu;
            q[1].v = tv + 0.5f;
            q[1].x = -18.0f;
            q[1].y = -18.0f;
            q[1].z = 0.0f;
            q[2].u = tu + 0.25f;
            q[2].v = tv;
            q[2].x = 18.0f;
            q[2].y = 18.0f;
            q[2].z = 0.0f;
            q[3].u = tu + 0.25f;
            q[3].v = tv + 0.5f;
            q[3].x = 18.0f;
            q[3].y = -18.0f;
            q[3].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end && d->t < d->end) {
        sp = &d->spin[d->spin_next];
        sp->t = 0;
        sp->flags |= 1;
        for (i = 0; i < 8; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            rad = DD_RAND(r) / 32767.0f * 80.0f;
            sp->frame[i] = DD_RAND(r) % 8;
            sp->speed[i] = DD_RAND(r) / 32767.0f * 10.0f + 20.0f;
            vec3_set(&sp->pos[i], rad * sinf(ang), rad * cosf(ang) + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
            sp->ang[i] = 0.0f;
            sign = (DD_RAND(r) & 1) * 2 - 1;
            sp->spin[i] = DD_RAND(r) / 32767.0f * 0.34906587f * sign;
        }
        if (d->spin_next < 23)
            d->spin_next++;
        else
            d->spin_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
    list = func_0882698C(list, 1, 1);
    list = func_088269D8(list, 1, 0, 2, 3, 0, 0);
    list = func_08826A64(list, 5);
    list = dl_color(list, 0xFFFFFFFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        sp = &d->spin[i];
        if (!(sp->flags & 1))
            continue;
        func_088279A4(sp->t, 24);
        list = dl_ambient_light(list, (unsigned int)((1.0f - out) * func_08827AD4(sp->t, 24, 1) * 255.0f) << 24
                                          | 0xFFFFFF);
        for (j = 0; j < 8; j++) {
            mtx_push();
            mtx_translate(&sp->pos[j]);
            mtx_mult(&bill);
            mtx_rotate_z(sp->ang[j]);
            list = draw_array(list, 4, 0x183, 4, 0, d->spin_v[sp->frame[j]]);
            mtx_pop();
            sp->pos[j].z -= sp->speed[j];
            sp->ang[j] += sp->spin[j];
        }
        if (sp->t < 23)
            sp->t++;
        else
            sp->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    func_08827668();
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}
#endif

/* ---- style 3: comets with long fading tails rushing at the camera ---- */

int dd_fx32_s3_update(int phase, DdFx32S3 *d);

int dd_fx32_s3_start(DdFxSlot *s, int a, int b)
{
    int task = func_0882623C(dd_fx32_s3_update, a, b, sizeof(DdFx32S3));
    DdFx32S3 *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 2,1 KB: alocação de registradores e laços desenrolados diferem */
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
    int i;
    int j;

    func_08827738();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    func_08827A2C(d->t, 8, 1);
    out = 1.0f - func_088279A4(d->t - (d->end - 8), 8);
    func_08827258(&bill);
    func_08827F68(0x6236, d->grow, d->grow_end, &d->se_done, 1);
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            tu = i % 4 * 0.25f;
            tv = i / 4 * 0.5f;
            /* the head of the tail, opaque */
            for (j = 0; j < 2; j++) {
                v = d->tail_v[i][j];
                z = (j - 1.0f) * 700.0f * 0.2f;
                v[0].u = tu;
                v[0].v = tv + j * 0.25f;
                v[0].color = 0xFF9B9BFF;
                v[0].x = -30.0f;
                v[0].y = 0.0f;
                v[0].z = z;
                v[1].u = tu + 0.25f;
                v[1].v = tv + j * 0.25f;
                v[1].color = 0xFF9B9BFF;
                v[1].x = 30.0f;
                v[1].y = 0.0f;
                v[1].z = z;
            }
            /* the rest fades out over 1400 units */
            for (j = 2; j < 9; j++) {
                v = d->tail_v[i][j];
                k = (j - 1) / 7.0f;
                color = (unsigned int)((1.0f - k) * 255.0f) << 24 | 0x9B9BFF;
                v[0].u = tu;
                v[0].v = tv + (k * 0.5f + 0.5f) * 0.5f;
                v[0].color = color;
                v[0].x = -30.0f;
                v[0].y = 0.0f;
                v[0].z = k * 1400.0f;
                v[1].u = tu + 0.25f;
                v[1].v = tv + (k * 0.5f + 0.5f) * 0.5f;
                v[1].color = color;
                v[1].x = 30.0f;
                v[1].y = 0.0f;
                v[1].z = k * 1400.0f;
            }
            v = d->head_v[i];
            v[0].u = tu;
            v[0].v = tv;
            v[0].color = 0xFF9B9BFF;
            v[0].x = -80.0f;
            v[0].y = 80.0f;
            v[0].z = 0.0f;
            v[1].u = tu + 0.25f;
            v[1].v = tv + 0.5f;
            v[1].color = 0xFF9B9BFF;
            v[1].x = 80.0f;
            v[1].y = -80.0f;
            v[1].z = 0.0f;
        }
        for (i = 0; i < 18; i++)
            d->idx[i] = i;
    }
    if (d->grow == d->grow_end) {
        c = &d->comet[d->comet_next];
        c->t = 0;
        c->flags |= 1;
        for (i = 0; i < 6; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            c->frame[i] = DD_RAND(r) % 8;
            vec3_set(&c->pos[i], sinf(ang) * 220.0f, cosf(ang) * 220.0f + 120.0f,
                     DD_RAND(r) / 32767.0f * 160.0f + 20.0f);
            c->speed[i] = (DD_RAND(r) / 32767.0f * 60.0f + 180.0f) * -1.0f;
            c->ang[i] = -ang;
        }
        if (d->comet_next < 31)
            d->comet_next++;
        else
            d->comet_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_fx32_depth(list, s->id);
    list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
    list = func_08826A64(list, 15);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 32; i++) {
        c = &d->comet[i];
        if (!(c->flags & 1))
            continue;
        list = dl_ambient_light(list, (unsigned int)(out * func_08827AD4(c->t, 32, 1) * 255.0f) << 24 | 0xFFFFFF);
        for (j = 0; j < 6; j++) {
            mtx_push();
            mtx_translate(&c->pos[j]);
            mtx_push();
            mtx_rotate_z(c->ang[j]);
            list = draw_array(list, 4, 0x99F, 18, d->idx, d->tail_v[c->frame[j]]);
            mtx_pop();
            mtx_mult(&bill);
            list = draw_array(list, 6, 0x19F, 2, 0, d->head_v[c->frame[j]]);
            mtx_pop();
            c->pos[j].z += c->speed[j];
        }
        if (c->t < 31)
            c->t++;
        else
            c->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    dd_fx32_mask_end(s->id);
    func_08827668();
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
    int task = func_0882623C(dd_fx32_s4_update, a, b, sizeof(DdFx32S4));
    DdFx32S4 *d;

    if (task != -1) {
        d = func_08826440(task);
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

#ifdef NON_MATCHING /* função de 4,4 KB: alocação de registradores e laços desenrolados diferem */
int dd_fx32_s4_update(int phase, DdFx32S4 *d)
{
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
    int i;
    int j;

    func_08827738();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = func_08827A2C(d->t, 8, 1);
    out = func_088279A4(d->t - (d->end - 16), 16);
    func_08827258(&bill);
    func_08827F68(0x6236, d->grow, d->grow_end, &d->se_done, 1);

    /* the tube */
    if (phase == 1) {
        vec3_set(&d->center, 0.0f, 120.0f, 160.0f);
        for (i = 0; i < 49; i++) {
            fr = i / 48.0f;
            z = fr * -6000.0f;
            alpha = (unsigned int)((1.0f - fr) * 255.0f);
            for (j = 0; j < 33; j++) {
                v = &d->tube[i][j];
                ang = j / 32.0f * 6.2831855f;
                v->u = j / 32.0f * 2.0f;
                v->v = fr * 4.0f;
                v->color = alpha << 24 | 0x1860FF;
                v->x = sinf(ang) * 220.0f;
                v->y = cosf(ang) * 220.0f;
                v->z = z;
            }
        }
        for (i = 0; i < 48; i++) {
            for (j = 0; j < 33; j++) {
                d->idx[i][j * 2] = i * 33 + j;
                d->idx[i][j * 2 + 1] = i * 33 + j + 33;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_fx32_depth(list, s->id);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = func_08826A64(list, 12);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->center);
        list = dl_tex_offset(d->tex_u, d->tex_v, list);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * (1.0f - out)) << 24 | 0xFFFFFF);
        for (i = 0; i < 48; i++)
            list = draw_array(list, 4, 0x119F, 66, d->idx[i], d->tube);
        d->tex_u = d->tex_u + 0.05f;
        d->tex_v = d->tex_v - 0.2f;
        mtx_pop();
        dl_set_list(list);
    }
    dd_fx32_mask_end(s->id);

    /* spinning sparks: the first four red, the other four white */
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            q = d->spin_v[i];
            tu = i % 4 * 0.25f;
            tv = i / 4 * 0.5f;
            q[0].u = tu;
            q[0].v = tv;
            q[0].x = -18.0f;
            q[0].y = 18.0f;
            q[0].z = 0.0f;
            q[1].u = tu;
            q[1].v = tv + 0.5f;
            q[1].x = -18.0f;
            q[1].y = -18.0f;
            q[1].z = 0.0f;
            q[2].u = tu + 0.25f;
            q[2].v = tv;
            q[2].x = 18.0f;
            q[2].y = 18.0f;
            q[2].z = 0.0f;
            q[3].u = tu + 0.25f;
            q[3].v = tv + 0.5f;
            q[3].x = 18.0f;
            q[3].y = -18.0f;
            q[3].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        sp = &d->spin[d->spin_next];
        sp->t = 0;
        sp->flags |= 1;
        for (i = 0; i < 8; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            rad = DD_RAND(r) / 32767.0f * 80.0f;
            sp->frame[i] = DD_RAND(r) % 8;
            sp->speed[i] = DD_RAND(r) / 32767.0f * 10.0f + 25.0f;
            vec3_set(&sp->pos[i], rad * sinf(ang), rad * cosf(ang) + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
            sp->ang[i] = 0.0f;
            sign = (DD_RAND(r) & 1) * 2 - 1;
            sp->spin[i] = DD_RAND(r) / 32767.0f * 0.34906587f * sign;
        }
        if (d->spin_next < 23)
            d->spin_next++;
        else
            d->spin_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
    list = func_0882698C(list, 1, 1);
    list = func_08826A64(list, 5);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        sp = &d->spin[i];
        if (!(sp->flags & 1))
            continue;
        func_088279A4(sp->t, 24);
        list = dl_ambient_light(list, (unsigned int)((1.0f - out) * func_08827AD4(sp->t, 24, 1) * 255.0f) << 24
                                          | 0xFFFFFF);
        list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
        list = dl_color(list, 0xFF1860FF);
        list = dd_fx32_s4_spin_draw(list, 0, 4, sp, d, &bill);
        list = func_088269D8(list, 1, 2, 6, 10, 0, -1);
        list = dl_color(list, 0xFFFFFFFF);
        list = dd_fx32_s4_spin_draw(list, 4, 8, sp, d, &bill);
        if (sp->t < 23)
            sp->t++;
        else
            sp->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);

    /* the screen redrawn over a 9 x 9 grid whose inner points wobble by 2 pixels */
    if (phase == 1) {
        for (i = 0; i < 9; i++) {
            for (j = 0; j < 9; j++) {
                d->grid[i][j].u = j / 8.0f * 480.0f;
                d->grid[i][j].v = i / 8.0f * 272.0f;
                d->grid[i][j].x = j / 8.0f * 480.0f;
                d->grid[i][j].y = i / 8.0f * 272.0f;
                d->grid[i][j].z = 0;
            }
        }
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 9; j++) {
                d->grid_idx[i][j * 2] = i * 9 + j;
                d->grid_idx[i][j * 2 + 1] = i * 9 + j + 9;
            }
        }
        for (i = 0; i < 9; i++) {
            for (j = 0; j < 9; j++) {
                d->phase[i][j] = DD_RAND(r) / 32767.0f * 6.2831855f;
                d->sign[i][j] = (DD_RAND(r) & 1) * 2 - 1;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 4, 0, 1, 1);
        list = func_0882698C(list, 0, 0);
        list = func_088269D8(list, 1, 0, 2, 3, 0, -1);
        list = func_08826B6C(list);
        for (i = 1; i < 8; i++) {
            for (j = 1; j < 8; j++) {
                ang = d->sign[i][j] * (d->phase[i][j] + d->frame * 0.043633234f);
                d->grid[i][j].x = (int)(j / 8.0f * 480.0f) + roundf_vfpu(sinf(ang) * 2.0f);
                d->grid[i][j].y = (int)(i / 8.0f * 272.0f) + roundf_vfpu(cosf(ang) * 2.0f);
            }
        }
        memcpy(d->grid_buf[buf], d->grid, sizeof(d->grid));
        list = dl_color(list, (unsigned int)(in * 226.0f * func_08827A00(d->t, 8, d->end)) << 24 | 0x818);
        for (i = 0; i < 8; i++)
            list = draw_array(list, 4, 0x800902, 18, d->grid_idx[i], d->grid_buf[buf]);
        dl_set_list(list);
    }
    func_08827668();
    d->frame++;
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}
#endif

/* ---- style 5: a white tube of light, spinning sparks and a fast wobbling screen ---- */

int dd_fx32_s5_update(int phase, DdFx32S5 *d);

int dd_fx32_s5_start(DdFxSlot *s, int a, int b)
{
    int task = func_0882623C(dd_fx32_s5_update, a, b, sizeof(DdFx32S5));
    DdFx32S5 *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 4,5 KB: alocação de registradores e laços desenrolados diferem */
int dd_fx32_s5_update(int phase, DdFx32S5 *d)
{
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
    int sign;
    int i;
    int j;

    func_08827738();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = func_08827A2C(d->t, 8, 1);
    out = 1.0f - func_088279A4(d->t - (d->end - 8), 8);
    func_08827258(&bill);
    func_08827F68(0x6236, d->grow, d->grow_end, &d->se_done, 1);

    /* the tube */
    if (phase == 1) {
        vec3_set(&d->center, 0.0f, 120.0f, 160.0f);
        for (i = 0; i < 49; i++) {
            fr = i / 48.0f;
            z = fr * -6000.0f;
            alpha = (unsigned int)((1.0f - fr) * 255.0f);
            for (j = 0; j < 33; j++) {
                v = &d->tube[i][j];
                ang = j / 32.0f * 6.2831855f;
                v->u = j / 32.0f * 3.0f;
                v->v = fr * 5.0f;
                v->color = alpha << 24 | 0xFFFFFF;
                v->x = sinf(ang) * 220.0f;
                v->y = cosf(ang) * 220.0f;
                v->z = z;
            }
        }
        for (i = 0; i < 48; i++) {
            for (j = 0; j < 33; j++) {
                d->idx[i][j * 2] = i * 33 + j;
                d->idx[i][j * 2 + 1] = i * 33 + j + 33;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_fx32_depth(list, s->id);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = func_08826A64(list, 9);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->center);
        list = dl_tex_offset(d->tex_u, d->tex_v, list);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i = 0; i < 48; i++)
            list = draw_array(list, 4, 0x119F, 66, d->idx[i], d->tube);
        d->tex_u = d->tex_u + 0.01f;
        d->tex_v = d->tex_v - 0.1f;
        mtx_pop();
        dl_set_list(list);
    }
    dd_fx32_mask_end(s->id);

    /* spinning sparks */
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            q = d->spin_v[i];
            tu = i % 4 * 0.25f;
            tv = i / 4 * 0.5f;
            q[0].u = tu;
            q[0].v = tv;
            q[0].x = -32.0f;
            q[0].y = 32.0f;
            q[0].z = 0.0f;
            q[1].u = tu;
            q[1].v = tv + 0.5f;
            q[1].x = -32.0f;
            q[1].y = -32.0f;
            q[1].z = 0.0f;
            q[2].u = tu + 0.25f;
            q[2].v = tv;
            q[2].x = 32.0f;
            q[2].y = 32.0f;
            q[2].z = 0.0f;
            q[3].u = tu + 0.25f;
            q[3].v = tv + 0.5f;
            q[3].x = 32.0f;
            q[3].y = -32.0f;
            q[3].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        sp = &d->spin[d->spin_next];
        sp->t = 0;
        sp->flags |= 1;
        for (i = 0; i < 4; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            rad = DD_RAND(r) / 32767.0f * 80.0f;
            sp->frame[i] = DD_RAND(r) % 8;
            sp->speed[i] = DD_RAND(r) / 32767.0f * 10.0f + 20.0f;
            vec3_set(&sp->pos[i], rad * sinf(ang), rad * cosf(ang) + 120.0f,
                     DD_RAND(r) / 32767.0f * 20.0f + 160.0f);
            sp->ang[i] = 0.0f;
            sign = (DD_RAND(r) & 1) * 2 - 1;
            sp->spin[i] = DD_RAND(r) / 32767.0f * 0.34906587f * sign;
        }
        if (d->spin_next < 23)
            d->spin_next++;
        else
            d->spin_next = 0;
    }
    list = dl_select_layer(7, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
    list = func_0882698C(list, 1, 1);
    list = func_08826A64(list, 20);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        sp = &d->spin[i];
        if (!(sp->flags & 1))
            continue;
        func_088279A4(sp->t, 24);
        list = dl_ambient_light(list, (unsigned int)(out * func_08827AD4(sp->t, 24, 1) * 255.0f) << 24 | 0xFFFFFF);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = dl_color(list, 0x80FFFFFF);
        for (j = 0; j < 4; j++) {
            mtx_push();
            mtx_translate(&sp->pos[j]);
            mtx_mult(&bill);
            mtx_rotate_z(sp->ang[j]);
            list = draw_array(list, 4, 0x183, 4, 0, d->spin_v[sp->frame[j]]);
            mtx_pop();
            sp->pos[j].z -= sp->speed[j];
            sp->ang[j] += sp->spin[j];
        }
        if (sp->t < 23)
            sp->t++;
        else
            sp->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);

    /* the screen redrawn over a 16 x 16 grid whose inner points wobble by 2 pixels */
    if (phase == 1) {
        for (i = 0; i < 16; i++) {
            for (j = 0; j < 16; j++) {
                d->grid[i][j].u = j / 15.0f * 480.0f;
                d->grid[i][j].v = i / 15.0f * 272.0f;
                d->grid[i][j].x = j / 15.0f * 480.0f;
                d->grid[i][j].y = i / 15.0f * 272.0f;
                d->grid[i][j].z = 0;
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
                d->sign[i][j] = (DD_RAND(r) & 1) * 2 - 1;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
        list = func_0882698C(list, 0, 0);
        list = func_088269D8(list, 1, 0, 2, 3, 0, -1);
        list = func_08826B6C(list);
        for (i = 1; i < 15; i++) {
            for (j = 1; j < 15; j++) {
                ang = d->sign[i][j] * (d->phase[i][j] + d->frame * 0.34906587f);
                d->grid[i][j].x = (int)(j / 15.0f * 480.0f) + roundf_vfpu(sinf(ang) * 2.0f);
                d->grid[i][j].y = (int)(i / 15.0f * 272.0f) + roundf_vfpu(cosf(ang) * 2.0f);
            }
        }
        memcpy(d->grid_buf[buf], d->grid, sizeof(d->grid));
        list = dl_color(list, (unsigned int)(in * 200.0f * out) << 24 | 0xFFFFFF);
        for (i = 0; i < 15; i++)
            list = draw_array(list, 4, 0x800902, 32, d->grid_idx[i], d->grid_buf[buf]);
        dl_set_list(list);
    }
    func_08827668();
    d->frame++;
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}
#endif

/* ---- style 6: long beams of light radiating around the center ---- */

int dd_fx32_s6_update(int phase, DdFx32S6 *d);

int dd_fx32_s6_start(DdFxSlot *s, int a, int b)
{
    int task = func_0882623C(dd_fx32_s6_update, a, b, sizeof(DdFx32S6));
    DdFx32S6 *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 1,7 KB: alocação de registradores difere */
int dd_fx32_s6_update(int phase, DdFx32S6 *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    DdRandSeed *r;
    DdFx32Beams *b;
    DdTCVtx *v;
    void *list;
    float out;
    float fr;
    float tv;
    float z;
    float ang;
    float e;
    int i;
    int j;

    func_08827738();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    func_08827A2C(d->t, 8, 1);
    out = 1.0f - func_088279A4(d->t - (d->end - 8), 8);
    func_08827F68(0x6236, d->grow, d->grow_end, &d->se_done, 1);
    if (phase == 1) {
        for (i = 0; i < 49; i++) {
            v = d->beam_v[i];
            fr = i / 48.0f;
            tv = fr * 6.0f;
            z = fr * -6000.0f;
            v[0].u = 0.0f;
            v[0].v = tv;
            v[0].color = 0xFFFFFF;
            v[0].x = -80.0f;
            v[0].y = 0.0f;
            v[0].z = z;
            v[1].u = 0.5f;
            v[1].v = tv;
            v[1].color = (unsigned int)((1.0f - fr) * 255.0f) << 24 | 0xFFFFFF;
            v[1].x = 0.0f;
            v[1].y = 0.0f;
            v[1].z = z;
            v[2].u = 1.0f;
            v[2].v = tv;
            v[2].color = 0xFFFFFF;
            v[2].x = 80.0f;
            v[2].y = 0.0f;
            v[2].z = z;
            d->idx_a[i * 2] = i * 3;
            d->idx_a[i * 2 + 1] = i * 3 + 1;
            d->idx_b[i * 2] = i * 3 + 1;
            d->idx_b[i * 2 + 1] = i * 3 + 2;
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
    list = func_088266D8(list, 1, 1, 1, 0, 0, 0, 0);
    list = dd_fx32_depth(list, s->id);
    list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
    list = func_08826A64(list, 21);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 8; i++) {
        b = &d->beam[i];
        if (!(b->flags & 1))
            continue;
        e = func_08827A2C(b->t, 8, 1);
        list = dl_ambient_light(list, (unsigned int)(out * func_08827AD4(b->t, 8, 1) * 255.0f) << 24 | 0xFFFFFF);
        for (j = 0; j < 8; j++) {
            list = dl_tex_offset(0.0f, DD_RAND(r) / 32767.0f, list);
            mtx_push();
            mtx_translate(&b->pos[j]);
            mtx_rotate_z(b->ang[j]);
            func_088272BC(b->width[j] * e, 0.0f, 1.0f);
            list = draw_array(list, 4, 0x99F, 98, d->idx_a, d->beam_v);
            list = draw_array(list, 4, 0x99F, 98, d->idx_b, d->beam_v);
            mtx_pop();
        }
        if (b->t < 7)
            b->t++;
        else
            b->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    dd_fx32_mask_end(s->id);
    func_08827668();
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}
#endif

/* ---- style 7: short golden blades rushing at the camera ---- */

int dd_fx32_s7_update(int phase, DdFx32S7 *d);

int dd_fx32_s7_start(DdFxSlot *s, int a, int b)
{
    int task = func_0882623C(dd_fx32_s7_update, a, b, sizeof(DdFx32S7));
    DdFx32S7 *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 1,7 KB: alocação de registradores difere */
int dd_fx32_s7_update(int phase, DdFx32S7 *d)
{
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
    int i;
    int j;

    func_08827738();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    func_08827A2C(d->t, 8, 1);
    out = func_088279A4(d->t - (d->end - 8), 8);
    func_08827F68(0x6236, d->grow, d->grow_end, &d->se_done, 1);
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 7; j++) {
                v = d->blade_v[i][j];
                z = j / 6.0f * 1000.0f + -500.0f;
                tv = i / 4 * 0.5f + j / 6.0f * 0.5f;
                v[0].u = i % 4 * 0.25f;
                v[0].v = tv;
                v[0].x = -50.0f;
                v[0].y = 0.0f;
                v[0].z = z;
                v[1].u = i % 4 * 0.25f + 0.25f;
                v[1].v = tv;
                v[1].x = 50.0f;
                v[1].y = 0.0f;
                v[1].z = z;
            }
        }
        for (i = 0; i < 14; i++)
            d->idx[i] = i;
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
    list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
    list = dd_fx32_depth(list, s->id);
    list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
    list = func_08826A64(list, 14);
    list = dl_color(list, 0xFFC864FF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 32; i++) {
        bl = &d->blade[i];
        if (!(bl->flags & 1))
            continue;
        list = dl_ambient_light(list, (unsigned int)((1.0f - out) * func_08827AD4(bl->t, 32, 1) * 255.0f) << 24
                                          | 0xFFFFFF);
        for (j = 0; j < 12; j++) {
            mtx_push();
            mtx_translate(&bl->pos[j]);
            mtx_rotate_z(bl->ang[j]);
            list = draw_array(list, 4, 0x983, 14, d->idx, d->blade_v[bl->frame[j]]);
            mtx_pop();
            bl->pos[j].z += bl->speed[j];
        }
        if (bl->t < 31)
            bl->t++;
        else
            bl->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    dd_fx32_mask_end(s->id);
    func_08827668();
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}
#endif

/* ---- style 8: a blue tube of light scrolling into the screen ---- */

int dd_fx32_s8_update(int phase, DdFx32S8 *d);

int dd_fx32_s8_start(DdFxSlot *s, int a, int b)
{
    int task = func_0882623C(dd_fx32_s8_update, a, b, sizeof(DdFx32S8));
    DdFx32S8 *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 1,4 KB: alocação de registradores e laços desenrolados diferem */
int dd_fx32_s8_update(int phase, DdFx32S8 *d)
{
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
    int i;
    int j;

    func_08827738();
    dd_fx32_mask_begin(s->id);
    if (phase == 1)
        dd_fx32_kind(s->id, &d->grow_end, &d->end);
    in = func_08827A2C(d->t, 8, 1);
    out = func_088279A4(d->t - (d->end - 8), 8);
    func_08827F68(0x6236, d->grow, d->grow_end, &d->se_done, 1);
    if (phase == 1) {
        vec3_set(&d->center, 0.0f, 120.0f, 160.0f);
        for (i = 0; i < 49; i++) {
            fr = i / 48.0f;
            z = fr * -6000.0f;
            alpha = (unsigned int)((1.0f - fr) * 255.0f);
            for (j = 0; j < 33; j++) {
                v = &d->tube[i][j];
                ang = j / 32.0f * 6.2831855f;
                v->u = j / 32.0f * 3.0f;
                v->v = fr * 6.0f;
                v->color = alpha << 24 | 0x2660FF;
                v->x = sinf(ang) * 220.0f;
                v->y = cosf(ang) * 220.0f;
                v->z = z;
            }
        }
        for (i = 0; i < 48; i++) {
            for (j = 0; j < 33; j++) {
                d->idx[i][j * 2] = i * 33 + j;
                d->idx[i][j * 2 + 1] = i * 33 + j + 33;
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_fx32_depth(list, s->id);
        list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
        list = func_08826A64(list, 12);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->center);
        list = dl_tex_offset(d->tex_u, d->tex_v, list);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * (1.0f - out)) << 24 | 0xFFFFFF);
        for (i = 0; i < 48; i++)
            list = draw_array(list, 4, 0x119F, 66, d->idx[i], d->tube);
        d->tex_u = d->tex_u + 0.05f;
        d->tex_v = d->tex_v - 0.2f;
        mtx_pop();
        dl_set_list(list);
    }
    dd_fx32_mask_end(s->id);
    func_08827668();
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}
#endif
