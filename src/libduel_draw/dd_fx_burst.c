// @range 0x08844024-0x08846348
// @asm 0x08844024-0x088440A8
// @asm 0x088441E4-0x08845110
// @asm 0x08845168-0x08846348
/* Effects 78-81: a burst of light on the card model, fixed where the model's node was when it starts:
   a glow, petals, a ring and motes thrown outward; kinds 13-18 and 40 get the purple variant with
   flame tongues swirling around it. */
#include "dd_fx.h"

int dd_burst_start(DdFxSlot *s, int a, int b);
int dd_burst2_start(DdFxSlot *s, int a, int b);

/* variant passed to dd_fx78_start: 1 for kinds 13-18 and 40 */
#ifdef NON_MATCHING /* o teste do 17 sai com beql em vez de beq (cópia do corpo de 18) */
int dd_fx78_arg(DdFxOwner *o)
{
    int type;
    int sub;
    int r;

    func_08827F54(o, &type, &sub);
    switch (type) {
    case 40: case 13: case 14: case 15: case 16: case 17:
        r = 1;
        break;
    case 18:
        r = 1;
        break;
    default:
        r = 0;
        break;
    }
    return r;
}
#endif

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
    int task = func_0882623C(dd_burst_update, a, b, sizeof(DdBurstFx));
    DdBurstFx *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 3,9 KB: alocação de registradores e laços desenrolados diferem */
int dd_burst_update(int phase, DdBurstFx *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    ScePspFMatrix4 bill;
    ScePspFMatrix3 rot;
    ScePspFVector3 v = { 0 };
    ScePspFVector3 pos;
    DdRandSeed *r;
    DdRingVtx *q;
    DdCVtx *c;
    void *list;
    float out;
    float ang;
    float sa;
    float ca;
    float sc;
    float tu;
    float tv;
    int sign;
    int i;

    func_08827738();
    if (phase == 1)
        dd_burst_kind(s->type, s->sub, s->id, &d->grow_end, &d->end);
    out = 1.0f - func_088279A4(d->t - (d->end - 16), 16);
    func_08827258(&bill);
    if (d->grow == d->grow_end && d->placed == 0) {
        d->placed = 1;
        func_08827F14(0x25, &d->pos);
    }
    func_08827F68(0x6235, d->grow, d->grow_end, &d->se_done, 1);

    /* the glow */
    if (phase == 1) {
        d->glow[0].u = 0.5f;  d->glow[0].v = 0.5f;  d->glow[0].color = 0xFFA0DCFF;
        d->glow[0].x = 0.0f;  d->glow[0].y = 0.0f;  d->glow[0].z = 0.0f;
        d->glow[1].u = 0.0f;  d->glow[1].v = 0.0f;  d->glow[1].color = 0xA0DCFF;
        d->glow[1].x = -80.0f; d->glow[1].y = 80.0f; d->glow[1].z = 0.0f;
        d->glow[2].u = 1.0f;  d->glow[2].v = 0.0f;  d->glow[2].color = 0xA0DCFF;
        d->glow[2].x = 80.0f; d->glow[2].y = 80.0f; d->glow[2].z = 0.0f;
        d->glow[3].u = 1.0f;  d->glow[3].v = 1.0f;  d->glow[3].color = 0xA0DCFF;
        d->glow[3].x = 80.0f; d->glow[3].y = -80.0f; d->glow[3].z = 0.0f;
        d->glow[4].u = 0.0f;  d->glow[4].v = 1.0f;  d->glow[4].color = 0xA0DCFF;
        d->glow[4].x = -80.0f; d->glow[4].y = -80.0f; d->glow[4].z = 0.0f;
        d->glow[5] = d->glow[1];
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
        list = func_08826A64(list, 3);
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
        for (i = 0; i < 8; i++) {
            q = d->petal_v[i];
            tu = i % 4 * 0.25f;
            tv = i / 4 * 0.5f;
            q[0].u = tu;         q[0].v = tv;         q[0].x = -8.0f; q[0].y = 250.0f; q[0].z = 0.0f;
            q[1].u = tu;         q[1].v = tv + 0.25f; q[1].x = -8.0f; q[1].y = 0.0f;   q[1].z = 0.0f;
            q[2].u = tu + 0.25f; q[2].v = tv;         q[2].x = 8.0f;  q[2].y = 250.0f; q[2].z = 0.0f;
            q[3].u = tu + 0.25f; q[3].v = tv + 0.25f; q[3].x = 8.0f;  q[3].y = 0.0f;   q[3].z = 0.0f;
        }
        for (i = 0; i < 16; i++) {
            d->petal[i].frame = DD_RAND(r) % 8;
            sign = (DD_RAND(r) & 1) * 2 - 1;
            d->petal[i].tilt = DD_RAND(r) / 32767.0f * 0.5235988f * sign;
            sign = (DD_RAND(r) & 1) * 2 - 1;
            d->petal[i].ang = 6.2831855f * i / 16.0f + DD_RAND(r) / 32767.0f * 6.2831855f / 16.0f / 2.0f * sign;
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = func_08826A64(list, 14);
        list = dl_color(list, 0xC8A0DCFF);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_mult(&bill);
        list = dl_ambient_light(list, (unsigned int)(out * 255.0f) << 24 | 0xFFFFFF);
        for (i = 0; i < 16; i++) {
            mtx_push();
            mtx_rotate_z(d->petal[i].ang);
            mtx_rotate_x(d->petal[i].tilt);
            list = draw_array(list, 4, 0x183, 4, 0, d->petal_v[d->petal[i].frame]);
            mtx_pop();
        }
        mtx_pop();
        dl_set_list(list);
    }

    /* the ring: three circles (60, 58, 45), opaque on the middle one */
    if (phase == 1) {
        for (i = 0; i < 65; i++) {
            ang = 6.2831855f * i / 64.0f;
            sa = sinf(ang);
            ca = cosf(ang);
            c = d->ring[i];
            c[0].color = 0xA0DCFF;   c[0].x = sa * 60.0f; c[0].y = ca * 60.0f; c[0].z = 0.0f;
            c[1].color = 0xFFA0DCFF; c[1].x = sa * 58.0f; c[1].y = ca * 58.0f; c[1].z = 0.0f;
            c[2].color = 0xA0DCFF;   c[2].x = sa * 45.0f; c[2].y = ca * 45.0f; c[2].z = 0.0f;
            d->idx_a[i * 2] = i * 3;
            d->idx_a[i * 2 + 1] = i * 3 + 1;
            d->idx_b[i * 2] = i * 3 + 1;
            d->idx_b[i * 2 + 1] = i * 3 + 2;
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 0, 1, 1, 0, 1, 1, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_mult(&bill);
        sc = func_08827A2C(d->t, d->end, 1) * 0.3f + 0.7f;
        func_088272BC(sc, sc, 0.0f);
        list = dl_ambient_light(list, (unsigned int)(out * 255.0f) << 24 | 0xFFFFFF);
        list = draw_array(list, 4, 0x99C, 130, d->idx_a, d->ring);
        list = draw_array(list, 4, 0x99C, 130, d->idx_b, d->ring);
        mtx_pop();
        dl_set_list(list);
    }

    /* 64 motes on a sphere of radius 30-40 */
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            q = d->mote_v[i];
            q[0].u = i % 4 * 0.25f;
            q[0].v = i / 4 * 0.5f;
            q[0].x = -3.0f;
            q[0].y = 3.0f;
            q[0].z = 0.0f;
            q[1].u = i % 4 * 0.25f + 0.25f;
            q[1].v = i / 4 * 0.5f + 0.5f;
            q[1].x = 3.0f;
            q[1].y = -3.0f;
            q[1].z = 0.0f;
        }
        for (i = 0; i < 64; i++) {
            d->mote[i].frame = DD_RAND(r) % 8;
            v.x = 0.0f;
            v.y = DD_RAND(r) / 32767.0f * 10.0f + 30.0f;
            v.z = 0.0f;
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            sa = DD_RAND(r) / 32767.0f * 6.2831855f;
            mat3_identity(&rot);
            mat3_rot_x(&rot, &rot, ang);
            mat3_rot_z(&rot, &rot, sa);
            mat3_transform_vec3(&d->mote[i].pos, &rot, &v);
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
        list = func_0882698C(list, 1, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = func_08826A64(list, 14);
        list = dl_color(list, 0xFFA0DCFF);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        sc = func_08827A2C(d->t, d->end, 1) * 0.1f + 0.9f;
        list = dl_ambient_light(list, (unsigned int)(out * 255.0f) << 24 | 0xFFFFFF);
        for (i = 0; i < 64; i++) {
            vec3_scale(&pos, &d->mote[i].pos, sc);
            mtx_push();
            mtx_translate(&pos);
            mtx_mult(&bill);
            list = draw_array(list, 6, 0x183, 2, 0, d->mote_v[d->mote[i].frame]);
            mtx_pop();
        }
        mtx_pop();
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

/* ---- variant 1 (purple, flame swirl) ---- */

int dd_burst2_update(int phase, DdBurst2Fx *d);

int dd_burst2_start(DdFxSlot *s, int a, int b)
{
    int task = func_0882623C(dd_burst2_update, a, b, sizeof(DdBurst2Fx));
    DdBurst2Fx *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 4,6 KB: alocação de registradores e laços desenrolados diferem */
int dd_burst2_update(int phase, DdBurst2Fx *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    ScePspFMatrix4 bill;
    ScePspFMatrix3 rot;
    ScePspFVector3 v = { 0 };
    ScePspFVector3 pos;
    DdRandSeed *r;
    DdTCVtx *q;
    DdCVtx *c;
    void *list;
    float in;
    float out;
    float ang;
    float sa;
    float ca;
    float sc;
    float fr;
    float w;
    float x;
    float y;
    float tu;
    float tv;
    int sign;
    int i;

    func_08827738();
    if (phase == 1) {
        dd_burst_kind(s->type, s->sub, s->id, &d->grow_end, &d->end);
        d->end += 16;
        sc = s->id == 80 ? 0.2f : 1.0f;
        vec3_set(&d->scale, sc, sc, sc);
        d->se = 0x6235;
        if (s->type == 18 && s->sub == 1)
            d->se = 0x6229;
    }
    in = func_088279A4(d->t, 8);
    out = 1.0f - func_088279A4(d->t - (d->end - 16), 16);
    func_08827258(&bill);
    if (d->grow == d->grow_end && d->placed == 0) {
        d->placed = 1;
        func_08827F14(0x25, &d->pos);
    }
    func_08827F68(d->se, d->grow, d->grow_end, &d->se_done, 1);

    /* flame tongues: a strip curving over 60 degrees, swirled 16 times */
    if (phase == 1) {
        x = 0.0f;
        y = 0.0f;
        for (i = 0; i < 17; i++) {
            fr = i / 16.0f;
            w = (fr * 0.9f + 0.1f) * 15.0f;
            ang = fr * 1.0471976f;
            ca = w * cosf(ang);
            sa = w * sinf(ang);
            q = d->flame[i];
            q[0].u = 0.0f;
            q[0].v = fr;
            q[0].color = 0x7A2EFF;
            q[0].x = ca + x;
            q[0].y = sa + y;
            q[0].z = 0.0f;
            q[1].u = 0.2f;
            q[1].v = fr;
            q[1].color = (unsigned int)((1.0f - fr) * 255.0f) << 24 | 0x7A2EFF;
            q[1].x = x;
            q[1].y = y;
            q[1].z = 0.0f;
            q[2].u = 0.4f;
            q[2].v = fr;
            q[2].color = 0x7A2EFF;
            q[2].x = -ca + x;
            q[2].y = -sa + y;
            q[2].z = 0.0f;
            d->flame_idx[i * 2] = i * 3;
            d->flame_idx[i * 2 + 1] = i * 3 + 1;
            x += -sinf(ang) * 7.5f;
            y += cosf(ang) * 7.5f;
        }
        for (i = 0; i < 16; i++) {
            d->swirl[i].u = DD_RAND(r) / 32767.0f;
            d->swirl[i].v = DD_RAND(r) / 32767.0f;
            sign = (DD_RAND(r) & 1) * 2 - 1;
            d->swirl[i].tilt = DD_RAND(r) / 32767.0f * 1.0471976f * sign;
            sign = (DD_RAND(r) & 1) * 2 - 1;
            d->swirl[i].ang = 6.2831855f * i / 16.0f + DD_RAND(r) / 32767.0f * 6.2831855f / 16.0f / 2.0f * sign;
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 0, 0);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 2, 3, 0, -1);
        list = func_08826A64(list, 12);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_scale3(&d->scale);
        mtx_mult(&bill);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i = 0; i < 16; i++) {
            list = dl_tex_offset(d->swirl[i].u, d->frame * -0.05f + d->swirl[i].v, list);
            mtx_push();
            mtx_rotate_z(d->swirl[i].ang);
            mtx_rotate_x(d->swirl[i].tilt);
            list = draw_array(list, 4, 0x99F, 34, d->flame_idx, &d->flame[0][0]);
            list = draw_array(list, 4, 0x99F, 34, d->flame_idx, &d->flame[0][1]);
            mtx_pop();
        }
        mtx_pop();
        dl_set_list(list);
    }

    /* 32 motes on a sphere of radius 40-50, drawn twice (dark, then additive purple) */
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            tu = i % 4 * 0.25f;
            tv = i / 4 * 0.5f;
            q = d->mote_v[i];
            q[0].u = tu;         q[0].v = tv;        q[0].color = 0xFF000000;
            q[0].x = -14.0f;     q[0].y = 20.0f;     q[0].z = 0.0f;
            q[1].u = tu + 0.25f; q[1].v = tv + 0.5f; q[1].color = 0xFF000000;
            q[1].x = 20.0f;      q[1].y = -14.0f;    q[1].z = 0.0f;
            q = d->mote_v2[i];
            q[0].u = tu;         q[0].v = tv;        q[0].color = 0xC87A2EFF;
            q[0].x = -10.0f;     q[0].y = 10.0f;     q[0].z = 0.0f;
            q[1].u = tu + 0.25f; q[1].v = tv + 0.5f; q[1].color = 0xC87A2EFF;
            q[1].x = 10.0f;      q[1].y = -10.0f;    q[1].z = 0.0f;
        }
        for (i = 0; i < 32; i++) {
            d->mote[i].frame = DD_RAND(r) % 8;
            v.x = 0.0f;
            v.y = DD_RAND(r) / 32767.0f * 10.0f + 40.0f;
            v.z = 0.0f;
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            sa = DD_RAND(r) / 32767.0f * 6.2831855f;
            mat3_identity(&rot);
            mat3_rot_x(&rot, &rot, ang);
            mat3_rot_z(&rot, &rot, sa);
            mat3_transform_vec3(&d->mote[i].pos, &rot, &v);
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
        list = func_0882698C(list, 1, 1);
        list = func_08826A64(list, 5);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_scale3(&d->scale);
        sc = func_088279A4(d->t, d->end) * 0.8f + 0.2f;
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i = 0; i < 32; i++) {
            vec3_scale(&pos, &d->mote[i].pos, sc);
            mtx_push();
            mtx_translate(&pos);
            mtx_mult(&bill);
            list = func_088269D8(list, 1, 0, 2, 3, 0, -1);
            list = draw_array(list, 6, 0x19F, 2, 0, d->mote_v[d->mote[i].frame]);
            list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
            list = draw_array(list, 6, 0x19F, 2, 0, d->mote_v2[d->mote[i].frame]);
            mtx_pop();
        }
        mtx_pop();
        dl_set_list(list);
    }

    /* the ring: three circles (70, 68, 55), opaque on the middle one */
    if (phase == 1) {
        for (i = 0; i < 65; i++) {
            ang = 6.2831855f * i / 64.0f;
            sa = sinf(ang);
            ca = cosf(ang);
            c = d->ring[i];
            c[0].color = 0x7A2EFF;   c[0].x = sa * 70.0f; c[0].y = ca * 70.0f; c[0].z = 0.0f;
            c[1].color = 0x647A2EFF; c[1].x = sa * 68.0f; c[1].y = ca * 68.0f; c[1].z = 0.0f;
            c[2].color = 0x7A2EFF;   c[2].x = sa * 55.0f; c[2].y = ca * 55.0f; c[2].z = 0.0f;
            d->idx_a[i * 2] = i * 3;
            d->idx_a[i * 2 + 1] = i * 3 + 1;
            d->idx_b[i * 2] = i * 3 + 1;
            d->idx_b[i * 2 + 1] = i * 3 + 2;
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 0, 1, 1, 0, 1, 1, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&d->pos);
        mtx_scale3(&d->scale);
        mtx_mult(&bill);
        sc = func_088279A4(d->t, d->end) * 0.7f + 0.3f;
        func_088272BC(sc, sc, 0.0f);
        list = dl_ambient_light(list, (unsigned int)(out * 255.0f) << 24 | 0xFFFFFF);
        list = draw_array(list, 4, 0x99C, 130, d->idx_a, d->ring);
        list = draw_array(list, 4, 0x99C, 130, d->idx_b, d->ring);
        mtx_pop();
        dl_set_list(list);
    }

    /* the glow */
    if (phase == 1) {
        d->glow[0].u = 0.5f;  d->glow[0].v = 0.5f;  d->glow[0].color = 0xFF7A2EFF;
        d->glow[0].x = 0.0f;  d->glow[0].y = 0.0f;  d->glow[0].z = 0.0f;
        d->glow[1].u = 0.0f;  d->glow[1].v = 0.0f;  d->glow[1].color = 0x7A2EFF;
        d->glow[1].x = -70.0f; d->glow[1].y = 70.0f; d->glow[1].z = 0.0f;
        d->glow[2].u = 1.0f;  d->glow[2].v = 0.0f;  d->glow[2].color = 0x7A2EFF;
        d->glow[2].x = 70.0f; d->glow[2].y = 70.0f; d->glow[2].z = 0.0f;
        d->glow[3].u = 1.0f;  d->glow[3].v = 1.0f;  d->glow[3].color = 0x7A2EFF;
        d->glow[3].x = 70.0f; d->glow[3].y = -70.0f; d->glow[3].z = 0.0f;
        d->glow[4].u = 0.0f;  d->glow[4].v = 1.0f;  d->glow[4].color = 0x7A2EFF;
        d->glow[4].x = -70.0f; d->glow[4].y = -70.0f; d->glow[4].z = 0.0f;
        d->glow[5] = d->glow[1];
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
        list = func_08826A64(list, 3);
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
