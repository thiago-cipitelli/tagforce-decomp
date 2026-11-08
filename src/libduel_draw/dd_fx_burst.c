// @range 0x08844024-0x08846348
// @asm 0x08845168-0x08846348
/* Effects 78-81: a burst of light on the card model, fixed where the model's node was when it starts:
   a glow, petals, a ring and motes thrown outward; kinds 13-18 and 40 get the purple variant with
   flame tongues swirling around it. */
#include "dd_fx.h"

int dd_burst_start(DdFxSlot *s, int a, int b);
int dd_burst2_start(DdFxSlot *s, int a, int b);

/* variant passed to dd_fx78_start: 1 for kinds 13-18 and 40 */
int dd_fx78_arg(DdFxOwner *o)
{
    int type;
    int sub;
    int r;

    dd_pair_get(o, &type, &sub);
    switch (type) {
    case 40: case 13: case 14: case 15: case 16: case 17: case 18:
        r = 1;
        break;
    default:
        r = 0;
        break;
    }
    return r;
}

int dd_fx78_start(DdFxSlot *s, int variant, int a, int b)
{
    switch (variant) {
    case 0:
        return dd_burst_start(s, a, b);
    case 1:
        return dd_burst2_start(s, a, b);
    default:
        return -1;
    }
}

/* grow time and length by effect id (effect 78 depends on the owner kind) */
void dd_burst_kind(int type, int sub, int id, int *grow, int *end)
{
    switch (id) {
    case 78:
        switch (type) {
        case 14:
            *grow = 112;
            break;
        case 16:
            *grow = 100;
            break;
        default:
            *grow = 111;
            break;
        }
        break;
    case 79:
        *grow = 93;
        break;
    case 80:
        *grow = 97;
        break;
    case 81:
        *grow = 125;
        break;
    default:
        *grow = 0;
        *end = 0;
        return;
    }
    *end = 24;
}

/* ---- variant 0 (blue) ---- */

int dd_burst_update(int phase, DdBurstFx *d);

int dd_burst_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_burst_update, a, b, sizeof(DdBurstFx));
    DdBurstFx *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

int dd_burst_update(int phase, DdBurstFx *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    DdRandSeed *r;
    DdBurstPetal *pt;
    DdBurstMote *mt;
    void *list;
    float out;
    float ang;
    float sa;
    float ca;
    float sc;
    float tu;
    float tv;
    float g;
    float w;
    float du;
    float dv;
    float y;
    float a;
    float b;
    int i;
    int i1;
    int i2;
    int i3;
    int i4;
    int i5;
    int i6;
    int i7;
    int i8;
    ScePspFMatrix4 bill __attribute__((aligned(16)));

    dd_mtx_push_saved();
    if (phase == 1)
        dd_burst_kind(s->type, s->sub, s->id, &d->grow_end, &d->end);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 16), 16);
    dd_mtx_billboard_cam(&bill);
    if (d->grow == d->grow_end && d->placed == 0) {
        d->placed = 1;
        dd_slot_pos2(0x25, &d->pos);
    }
    dd_cue_se(0x6235, d->grow, d->grow_end, &d->se_done, 1);

    /* the glow */
    if (phase == 1) {
        g = 80.0f;
        d->glow[0].u = 0.5f;  d->glow[0].v = 0.5f;  d->glow[0].color = 0xFFA0DCFF;
        d->glow[0].x = 0.0f;  d->glow[0].y = 0.0f;  d->glow[0].z = 0.0f;
        d->glow[1].u = 0.0f;  d->glow[1].v = 0.0f;  d->glow[1].color = 0xA0DCFF;
        d->glow[1].x = -g;    d->glow[1].y = g;     d->glow[1].z = 0.0f;
        d->glow[2].u = 1.0f;  d->glow[2].v = 0.0f;  d->glow[2].color = 0xA0DCFF;
        d->glow[2].x = g;     d->glow[2].y = g;     d->glow[2].z = 0.0f;
        d->glow[3].u = 1.0f;  d->glow[3].v = 1.0f;  d->glow[3].color = 0xA0DCFF;
        d->glow[3].x = g;     d->glow[3].y = -g;    d->glow[3].z = 0.0f;
        d->glow[4].u = 0.0f;  d->glow[4].v = 1.0f;  d->glow[4].color = 0xA0DCFF;
        d->glow[4].x = -g;    d->glow[4].y = -g;    d->glow[4].z = 0.0f;
        d->glow[5] = d->glow[1];
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 6, 10, 0, -1);
        list = dd_tex_pack(list, 3);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_mult(&bill);
        list = dl_ambient_light(list, (unsigned int)(out * 255.0f) << 24 | 0xFFFFFF);
        list = draw_array(list, 5, 0x19F, 6, 0, d->glow);
        mtx_pop();
        dl_set_list(list);
    }

    /* 16 petals around the glow, tilted at random */
    if (phase == 1) {
        du = 0.25f;
        w = 8.0f;
        dv = 0.5f;
        for (i1 = 0; i1 < 8; i1++) {
            tu = i1 % 4 * du;
            tv = i1 / 4 * dv;
            d->petal_v[i1][0].u = tu;      d->petal_v[i1][0].v = tv;      d->petal_v[i1][0].x = -w;
            d->petal_v[i1][0].y = 250.0f;  d->petal_v[i1][0].z = 0.0f;
            d->petal_v[i1][1].u = tu;      d->petal_v[i1][1].v = tv + du; d->petal_v[i1][1].x = -w;
            d->petal_v[i1][1].y = 0.0f;    d->petal_v[i1][1].z = 0.0f;
            d->petal_v[i1][2].u = tu + du; d->petal_v[i1][2].v = tv;      d->petal_v[i1][2].x = w;
            d->petal_v[i1][2].y = 250.0f;  d->petal_v[i1][2].z = 0.0f;
            d->petal_v[i1][3].u = tu + du; d->petal_v[i1][3].v = tv + du; d->petal_v[i1][3].x = w;
            d->petal_v[i1][3].y = 0.0f;    d->petal_v[i1][3].z = 0.0f;
        }
        for (i2 = 0; i2 < 16; i2++) {
            pt = &d->petal[i2];
            pt->frame = DD_RAND(r) % 8;
            pt->tilt = DD_RAND(r) / 32767.0f * 0.5235988f * (DD_RAND(r) % 2 * 2 - 1);
            pt->ang = 6.2831855f * i2 / 16.0f + DD_RAND(r) / 32767.0f * 6.2831855f / 16.0f / 2.0f * (DD_RAND(r) % 2 * 2 - 1);
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dd_tex_pack(list, 14);
        list = dl_color(list, 0xC8A0DCFF);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_mult(&bill);
        list = dl_ambient_light(list, (unsigned int)(out * 255.0f) << 24 | 0xFFFFFF);
        for (i3 = 0; i3 < 16; i3++) {
            pt = &d->petal[i3];
            mtx_push();
            mtx_rotate_z(pt->ang);
            mtx_rotate_x(pt->tilt);
            list = draw_array(list, 4, 0x183, 4, 0, d->petal_v[pt->frame]);
            mtx_pop();
        }
        mtx_pop();
        dl_set_list(list);
    }

    /* the ring: three circles (60, 58, 45), opaque on the middle one */
    if (phase == 1) {
        for (i4 = 0; i4 < 65; i4++) {
            ang = 6.2831855f * i4 / 64.0f;
            sa = sinf(ang);
            ca = cosf(ang);
            d->ring[i4][0].color = 0xA0DCFF;   d->ring[i4][0].x = sa * 60.0f; d->ring[i4][0].y = ca * 60.0f; d->ring[i4][0].z = 0.0f;
            d->ring[i4][1].color = 0xFFA0DCFF; d->ring[i4][1].x = sa * 58.0f; d->ring[i4][1].y = ca * 58.0f; d->ring[i4][1].z = 0.0f;
            d->ring[i4][2].color = 0xA0DCFF;   d->ring[i4][2].x = sa * 45.0f; d->ring[i4][2].y = ca * 45.0f; d->ring[i4][2].z = 0.0f;
        }
        for (i5 = 0; i5 < 65; i5++) {
            d->idx_a[i5 * 2] = i5 * 3;
            d->idx_a[i5 * 2 + 1] = i5 * 3 + 1;
            d->idx_b[i5 * 2] = i5 * 3 + 1;
            d->idx_b[i5 * 2 + 1] = i5 * 3 + 2;
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 0, 1, 1, 0, 1, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_mult(&bill);
        sc = dd_ease_sin(d->t, d->end, 1) * 0.3f + 0.7f;
        dd_scale3(sc, sc, 0.0f);
        list = dl_ambient_light(list, (unsigned int)(out * 255.0f) << 24 | 0xFFFFFF);
        for (i6 = 0; i6 < 2; i6++)  /* idx_a, then idx_b right after it */
            list = draw_array(list, 4, 0x99C, 130, d->idx_a + i6 * 130, d->ring);
        mtx_pop();
        dl_set_list(list);
    }

    /* 64 motes on a sphere of radius 30-40 */
    if (phase == 1) {
        du = 0.25f;
        w = 3.0f;
        dv = 0.5f;
        for (i7 = 0; i7 < 8; i7++) {
            tu = i7 % 4 * du;
            tv = i7 / 4 * dv;
            d->mote_v[i7][0].u = tu;      d->mote_v[i7][0].v = tv;      d->mote_v[i7][0].x = -w;
            d->mote_v[i7][0].y = w;       d->mote_v[i7][0].z = 0.0f;
            d->mote_v[i7][1].u = tu + du; d->mote_v[i7][1].v = tv + dv; d->mote_v[i7][1].x = w;
            d->mote_v[i7][1].y = -w;      d->mote_v[i7][1].z = 0.0f;
        }
        for (i = 0; i < 64; i++) {
            mt = &d->mote[i];
            mt->frame = DD_RAND(r) % 8;
            y = DD_RAND(r) / 32767.0f;
            a = DD_RAND(r) / 32767.0f;
            b = DD_RAND(r) / 32767.0f;
            {
                ScePspFVector3 v = { 0 };  /* zeroed by a byte loop */
                ScePspFMatrix3 rot;

                v.y = y * 10.0f + 30.0f;
                mat3_identity(&rot);
                mat3_rot_x(&rot, &rot, 6.2831855f * a);
                mat3_rot_z(&rot, &rot, 6.2831855f * b);
                mat3_transform_vec3(&mt->pos, &rot, &v);
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
        list = dd_depth_enable(list, 1, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dd_tex_pack(list, 14);
        list = dl_color(list, 0xFFA0DCFF);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        sc = dd_ease_sin(d->t, d->end, 1) * 0.1f + 0.9f;
        list = dl_ambient_light(list, (unsigned int)(out * 255.0f) << 24 | 0xFFFFFF);
        for (i8 = 0; i8 < 64; i8++) {
            ScePspFVector3 pos;

            mt = &d->mote[i8];
            vec3_scale(&pos, &mt->pos, sc);
            mtx_push();
            mtx_translate(&pos);
            mtx_mult(&bill);
            list = draw_array(list, 6, 0x183, 2, 0, d->mote_v[mt->frame]);
            mtx_pop();
        }
        mtx_pop();
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

/* ---- variant 1 (purple, flame swirl) ---- */

int dd_burst2_update(int phase, DdBurst2Fx *d);

int dd_burst2_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_burst2_update, a, b, sizeof(DdBurst2Fx));
    DdBurst2Fx *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* 23 palavras (2026-11-08): só a ordem do bloco das bordas da língua de fogo (-ca + x e -sa + y saem
                      antes da cor no original, com -sa num $f próprio); o resto bate */
int dd_burst2_update(int phase, DdBurst2Fx *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    DdRandSeed *r;
    DdSwirl *sw;
    DdBurstMote *mt;
    void *list;
    float w;
    float ca;
    float in;
    float out;
    float x;
    float y;
    float fr;
    float ang;
    float sa;
    float sc;
    float tu;
    float tv;
    float g;
    float du;
    float dv;
    float w1;
    float w2;
    float ry;
    float a;
    float b;
    int i;
    int i1;
    int i2;
    int i3;
    int i4;
    int i5;
    int i6;
    int i7;
    int i8;
    ScePspFMatrix4 bill __attribute__((aligned(16)));

    dd_mtx_push_saved();
    if (phase == 1) {
        dd_burst_kind(s->type, s->sub, s->id, &d->grow_end, &d->end);
        d->end += 16;
        switch (s->id) {
        case 80:
            sc = 0.2f;
            break;
        default:
            sc = 1.0f;
            break;
        }
        vec3_set(&d->scale, sc, sc, sc);
        d->se = 0x6235;
        switch (s->type) {
        case 18:
            if (s->sub == 1)
                d->se = 0x6229;
            break;
        }
    }
    in = dd_ease_lin(d->t, 8);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 16), 16);
    dd_mtx_billboard_cam(&bill);
    if (d->grow == d->grow_end && d->placed == 0) {
        d->placed = 1;
        dd_slot_pos2(0x25, &d->pos);
    }
    dd_cue_se(d->se, d->grow, d->grow_end, &d->se_done, 1);

    /* flame tongues: a strip curving over 60 degrees, swirled 16 times */
    if (phase == 1) {
        x = 0.0f;
        y = 0.0f;
        for (i1 = 0; i1 < 17; i1++) {
            fr = i1 / 16.0f;
            w = (fr * 0.9f + 0.1f) * 15.0f;
            ang = fr * 1.0471976f;
            ca = w * cosf(ang);
            sa = w * sinf(ang);
            d->flame[i1][0].u = 0.0f;
            d->flame[i1][0].v = fr;
            d->flame[i1][0].color = 0x7A2EFF;
            d->flame[i1][0].x = ca + x;
            d->flame[i1][0].y = sa + y;
            d->flame[i1][0].z = 0.0f;
            ca = -ca + x;
            sa = -sa + y;
            d->flame[i1][1].u = 0.2f;
            d->flame[i1][1].v = fr;
            d->flame[i1][1].color = (unsigned int)((1.0f - fr) * 255.0f) << 24 | 0x7A2EFF;
            d->flame[i1][1].x = x;
            d->flame[i1][1].y = y;
            d->flame[i1][1].z = 0.0f;
            d->flame[i1][2].u = 0.4f;
            d->flame[i1][2].v = fr;
            d->flame[i1][2].color = 0x7A2EFF;
            d->flame[i1][2].x = ca;
            d->flame[i1][2].y = sa;
            d->flame[i1][2].z = 0.0f;
            d->flame_idx[i1 * 2] = i1 * 3;
            d->flame_idx[i1 * 2 + 1] = i1 * 3 + 1;
            x += -sinf(ang) * 7.5f;
            y += cosf(ang) * 7.5f;
        }
        for (i2 = 0; i2 < 16; i2++) {
            sw = &d->swirl[i2];
            d->swirl[i2].u = DD_RAND(r) / 32767.0f;
            d->swirl[i2].v = DD_RAND(r) / 32767.0f;
            sw->tilt = DD_RAND(r) / 32767.0f * 1.0471976f * (DD_RAND(r) % 2 * 2 - 1);
            sw->ang = 6.2831855f * i2 / 16.0f + DD_RAND(r) / 32767.0f * 6.2831855f / 16.0f / 2.0f * (DD_RAND(r) % 2 * 2 - 1);
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 3, 0, -1);
        list = dd_tex_pack(list, 12);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_scale3(&d->scale);
        mtx_mult(&bill);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i3 = 0; i3 < 16; i3++) {
            sw = &d->swirl[i3];
            list = dl_tex_offset(d->swirl[i3].u, d->frame * -0.05f + d->swirl[i3].v, list);
            mtx_push();
            mtx_rotate_z(sw->ang);
            mtx_rotate_x(sw->tilt);
            list = draw_array(list, 4, 0x99F, 34, d->flame_idx, &d->flame[0][0]);
            list = draw_array(list, 4, 0x99F, 34, d->flame_idx, &d->flame[0][1]);
            mtx_pop();
        }
        mtx_pop();
        dl_set_list(list);
    }

    /* 32 motes on a sphere of radius 40-50, drawn twice (dark, then additive purple) */
    if (phase == 1) {
        du = 0.25f;
        w1 = 20.0f;
        dv = 0.5f;
        w2 = 10.0f;
        for (i4 = 0; i4 < 8; i4++) {
            tu = i4 % 4 * du;
            tv = i4 / 4 * dv;
            d->mote_v[i4][0].u = tu;       d->mote_v[i4][0].v = tv;       d->mote_v[i4][0].color = 0xFF000000;
            d->mote_v[i4][0].x = -w1;      d->mote_v[i4][0].y = w1;       d->mote_v[i4][0].z = 0.0f;
            d->mote_v[i4][1].u = tu + du;  d->mote_v[i4][1].v = tv + dv;  d->mote_v[i4][1].color = 0xFF000000;
            d->mote_v[i4][1].x = w1;       d->mote_v[i4][1].y = -w1;      d->mote_v[i4][1].z = 0.0f;
            d->mote_v2[i4][0].u = tu;      d->mote_v2[i4][0].v = tv;      d->mote_v2[i4][0].color = 0xC87A2EFF;
            d->mote_v2[i4][0].x = -w2;     d->mote_v2[i4][0].y = w2;      d->mote_v2[i4][0].z = 0.0f;
            d->mote_v2[i4][1].u = tu + du; d->mote_v2[i4][1].v = tv + dv; d->mote_v2[i4][1].color = 0xC87A2EFF;
            d->mote_v2[i4][1].x = w2;      d->mote_v2[i4][1].y = -w2;     d->mote_v2[i4][1].z = 0.0f;
        }
        for (i = 0; i < 32; i++) {
            mt = &d->mote[i];
            mt->frame = DD_RAND(r) % 8;
            ry = DD_RAND(r) / 32767.0f;
            a = DD_RAND(r) / 32767.0f;
            b = DD_RAND(r) / 32767.0f;
            {
                ScePspFVector3 v = { 0 };  /* zeroed by a byte loop */
                ScePspFMatrix3 rot;

                v.y = ry * 10.0f + 40.0f;
                mat3_identity(&rot);
                mat3_rot_x(&rot, &rot, 6.2831855f * a);
                mat3_rot_z(&rot, &rot, 6.2831855f * b);
                mat3_transform_vec3(&mt->pos, &rot, &v);
            }
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
        list = dd_depth_enable(list, 1, 1);
        list = dd_tex_pack(list, 5);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_scale3(&d->scale);
        sc = dd_ease_lin(d->t, d->end) * 0.8f + 0.2f;
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i5 = 0; i5 < 32; i5++) {
            ScePspFVector3 pos;

            mt = &d->mote[i5];
            vec3_scale(&pos, &mt->pos, sc);
            mtx_push();
            mtx_translate(&pos);
            mtx_mult(&bill);
            list = dd_blend_enable(list, 1, 0, 2, 3, 0, -1);
            list = draw_array(list, 6, 0x19F, 2, 0, d->mote_v[mt->frame]);
            list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
            list = draw_array(list, 6, 0x19F, 2, 0, d->mote_v2[mt->frame]);
            mtx_pop();
        }
        mtx_pop();
        dl_set_list(list);
    }

    /* the ring: three circles (70, 68, 55), opaque on the middle one */
    if (phase == 1) {
        for (i6 = 0; i6 < 65; i6++) {
            ang = 6.2831855f * i6 / 64.0f;
            sa = sinf(ang);
            ca = cosf(ang);
            d->ring[i6][0].color = 0x7A2EFF;   d->ring[i6][0].x = sa * 70.0f; d->ring[i6][0].y = ca * 70.0f; d->ring[i6][0].z = 0.0f;
            d->ring[i6][1].color = 0x647A2EFF; d->ring[i6][1].x = sa * 68.0f; d->ring[i6][1].y = ca * 68.0f; d->ring[i6][1].z = 0.0f;
            d->ring[i6][2].color = 0x7A2EFF;   d->ring[i6][2].x = sa * 55.0f; d->ring[i6][2].y = ca * 55.0f; d->ring[i6][2].z = 0.0f;
        }
        for (i7 = 0; i7 < 65; i7++) {
            d->idx_a[i7 * 2] = i7 * 3;
            d->idx_a[i7 * 2 + 1] = i7 * 3 + 1;
            d->idx_b[i7 * 2] = i7 * 3 + 1;
            d->idx_b[i7 * 2 + 1] = i7 * 3 + 2;
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 0, 1, 1, 0, 1, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_scale3(&d->scale);
        mtx_mult(&bill);
        sc = dd_ease_lin(d->t, d->end) * 0.7f + 0.3f;
        dd_scale3(sc, sc, 0.0f);
        list = dl_ambient_light(list, (unsigned int)(out * 255.0f) << 24 | 0xFFFFFF);
        for (i8 = 0; i8 < 2; i8++)  /* idx_a, then idx_b right after it */
            list = draw_array(list, 4, 0x99C, 130, d->idx_a + i8 * 130, d->ring);
        mtx_pop();
        dl_set_list(list);
    }

    /* the glow */
    if (phase == 1) {
        g = 70.0f;
        d->glow[0].u = 0.5f;  d->glow[0].v = 0.5f;  d->glow[0].color = 0xFF7A2EFF;
        d->glow[0].x = 0.0f;  d->glow[0].y = 0.0f;  d->glow[0].z = 0.0f;
        d->glow[1].u = 0.0f;  d->glow[1].v = 0.0f;  d->glow[1].color = 0x7A2EFF;
        d->glow[1].x = -g;    d->glow[1].y = g;     d->glow[1].z = 0.0f;
        d->glow[2].u = 1.0f;  d->glow[2].v = 0.0f;  d->glow[2].color = 0x7A2EFF;
        d->glow[2].x = g;     d->glow[2].y = g;     d->glow[2].z = 0.0f;
        d->glow[3].u = 1.0f;  d->glow[3].v = 1.0f;  d->glow[3].color = 0x7A2EFF;
        d->glow[3].x = g;     d->glow[3].y = -g;    d->glow[3].z = 0.0f;
        d->glow[4].u = 0.0f;  d->glow[4].v = 1.0f;  d->glow[4].color = 0x7A2EFF;
        d->glow[4].x = -g;    d->glow[4].y = -g;    d->glow[4].z = 0.0f;
        d->glow[5] = d->glow[1];
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 6, 10, 0, -1);
        list = dd_tex_pack(list, 3);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_scale3(&d->scale);
        mtx_mult(&bill);
        list = dl_ambient_light(list, (unsigned int)(out * 255.0f) << 24 | 0xFFFFFF);
        list = draw_array(list, 5, 0x19F, 6, 0, d->glow);
        mtx_pop();
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
#endif
