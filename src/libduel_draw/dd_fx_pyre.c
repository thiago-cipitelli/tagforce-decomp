// @range 0x08846348-0x0884774C
// @data 0x08876C40-0x08876C80
/* Effects 83-90 and 118-128: a ring of 16 petals turning under a node of the card model, sparks
   rising from it and, in the red variant (dd_fx_table.arg4 != 0), flame flares bursting outward
   (task of 0x11E0 bytes). */
#include "dd_fx.h"

#define DD_BIT(f, m) ((f) & (m) ? 1 : 0)

int dd_pyre_update(int phase, DdPyreFx *d);

/* templates read by dd_pyre_update: the two node offsets, the petal scale (y = fade
   in, padded to 16 bytes) and the flares' axis */
ScePspFVector4 dd_pyre_down = { 0.0f, 0.0f, 44.0f, 1.0f };
ScePspFVector4 dd_pyre_up = { 0.0f, 0.0f, -44.0f, 1.0f };
ScePspFVector4 dd_pyre_scale = { 1.0f, 0.0f, 0.0f, 0.0f };
ScePspFVector4 dd_pyre_axis = { 0.0f, 0.0f, 1.0f, 1.0f };

/* grow time and length by effect id */
void dd_pyre_kind(int id, int *grow, int *end)
{
    switch (id) {
    case 83:
        *grow = 110;
        break;
    case 84:
        *grow = 120;
        break;
    case 85:
        *grow = 100;
        break;
    case 86: case 87:
        *grow = 125;
        break;
    case 88:
        *grow = 96;
        break;
    case 89:
        *grow = 107;
        break;
    case 90:
        *grow = 128;
        break;
    case 118:
        *grow = 65;
        break;
    case 119: case 120:
        *grow = 55;
        break;
    case 121:
        *grow = 75;
        break;
    case 122: case 123: case 127:
        *grow = 70;
        break;
    case 124:
        *grow = 65;
        break;
    case 125:
        *grow = 80;
        break;
    case 126:
        *grow = 62;
        break;
    case 128:
        *grow = 60;
        break;
    default:
        *grow = 0;
        *end = 0;
        return;
    }
    *end = 48;
}

/* sets dd_fx_table.arg4 (the variant of the next effects 83-90/118-128) */
void dd_fx_set_arg4(int v)
{
    dd_fx_table.arg4 = v;
}

int dd_fx83_start(DdFxSlot *s, int red, int a, int b)
{
    int task = dd_task_create(dd_pyre_update, a, b, sizeof(DdPyreFx));
    DdPyreFx *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
        d->red = red;
    }
    return task;
}

int dd_pyre_update(int phase, DdPyreFx *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    ScePspFVector3 scale;
    ScePspFMatrix4 bill __attribute__((aligned(16)));
    DdRandSeed *r;
    DdHaloSpark *sp;
    DdFlare *f;
    void *list;
    unsigned int c0;
    unsigned int c1;
    float in;
    float out;
    float grow;
    float rad;
    float zr;
    float ang;
    float e;
    float g;
    float w4;
    float du;
    float dv;
    float tu;
    float tv;
    float tu2;
    float tv2;
    float fw;
    float fz;
    float eps;
    int flip;
    int i;
    int i1;
    int i2;
    int i3;
    int i4;
    int i5;
    int i6;
    int i7;
    int j;
    int j1;

    dd_mtx_push_saved();
    dd_proj_set(0.0f, 0.0f, 1.0f, 0.0f);
    if (phase == 1)
        dd_pyre_kind(s->id, &d->grow_end, &d->end);
    in = dd_ease_lin(d->t, 16);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 16), 16);
    grow = d->t * 0.003f + 1.0f;
    dd_mtx_billboard_cam(&bill);

    /* first frame: under the node, turned over (above it for effects 84 and 90) */
    if (d->grow == 0 && d->placed == 0) {
        if (s->id != 90 && s->id != 84)
            flip = 1;
        else
            flip = 0;
        d->placed = 1;
        dd_chara_bone_mtx(&d->m, 0x27);
        if (flip == 1) {
            ScePspFVector4 node __attribute__((aligned(16)));
            ScePspFMatrix4 tmp __attribute__((aligned(16)));

            node = dd_pyre_down;
            mat4_copy(&tmp, &d->m);
            dd_mtx_strip(&tmp, &tmp);
            mat4_transform_vec4(&node, &tmp, &node);
            mat4_identity(&tmp);
            mat4_mul_rot_x(&tmp, &tmp, 3.1415927f);
            mat4_mul(&d->m, &d->m, &tmp);
            mat4_translate(&d->m, &d->m, &node);
        } else {
            ScePspFVector4 node __attribute__((aligned(16)));
            ScePspFMatrix4 tmp __attribute__((aligned(16)));

            node = dd_pyre_up;
            mat4_copy(&tmp, &d->m);
            dd_mtx_strip(&tmp, &tmp);
            mat4_transform_vec4(&node, &tmp, &node);
            mat4_identity(&tmp);
            mat4_mul_rot_x(&tmp, &tmp, -1.5707964f);
            mat4_mul(&d->m, &d->m, &tmp);
            mat4_translate(&d->m, &d->m, &node);
        }
    }
    dd_cue_se(0x622A, d->grow, d->grow_end, &d->se_done, 1);

    /* the petals */
    if (phase == 1) {
        if (d->red == 0) {
            c0 = 0x00AAFF;
            c1 = 0x8000AAFF;
        } else {
            c0 = 0x6464FF;
            c1 = 0xB26464FF;
        }
        g = 30.0f;
        d->petal[0].u = 0.0f; d->petal[0].v = 0.0f; d->petal[0].color = c0;
        d->petal[0].x = -g;   d->petal[0].y = 200.0f; d->petal[0].z = 0.0f;
        d->petal[1].u = 0.0f; d->petal[1].v = 0.5f; d->petal[1].color = c1;
        d->petal[1].x = -g;   d->petal[1].y = 0.0f; d->petal[1].z = 0.0f;
        d->petal[2].u = 1.0f; d->petal[2].v = 0.0f; d->petal[2].color = c0;
        d->petal[2].x = g;    d->petal[2].y = 200.0f; d->petal[2].z = 0.0f;
        d->petal[3].u = 1.0f; d->petal[3].v = 0.5f; d->petal[3].color = c1;
        d->petal[3].x = g;    d->petal[3].y = 0.0f; d->petal[3].z = 0.0f;
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dd_tex_pack(list, 3);
        mtx_set_mode(2);
        mtx_push();
        mtx_load(&d->m);
        scale.x = dd_pyre_scale.x;
        scale.y = dd_pyre_scale.y;
        scale.z = dd_pyre_scale.z;
        rad = 35.0f * grow;
        zr = 5.0f * grow;
        scale.y = in;
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i1 = 0; i1 < 16; i1++) {
            ang = 6.2831855f * i1 / 16.0f + d->frame * 0.0034906587f;
            mtx_push();
            dd_translate3(rad * sinf(ang), 0.0f, zr * cosf(ang));
            mtx_rotate_y(ang);
            mtx_rotate_x(0.17453294f);
            mtx_scale3(&scale);
            list = draw_array(list, 4, 0x19F, 4, 0, d->petal);
            mtx_pop();
        }
        mtx_pop();
        dl_set_list(list);
    }

    /* sparks rising from the ring */
    if (phase == 1) {
        du = 0.25f;
        w4 = 4.0f;
        dv = 0.5f;
        for (i2 = 0; i2 < 8; i2++) {
            tu = i2 % 4 * du;
            tv = i2 / 4 * dv;
            d->spark_v[i2][0].u = tu;      d->spark_v[i2][0].v = tv;      d->spark_v[i2][0].x = -w4;
            d->spark_v[i2][0].y = w4;      d->spark_v[i2][0].z = 0.0f;
            d->spark_v[i2][1].u = tu + du; d->spark_v[i2][1].v = tv + dv; d->spark_v[i2][1].x = w4;
            d->spark_v[i2][1].y = -w4;     d->spark_v[i2][1].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        sp = &d->spark[d->spark_next];
        rad = 35.0f * grow;
        zr = 20.0f * grow;
        sp->flags |= 1;
        sp->t = 0;
        for (i3 = 0; i3 < 3; i3++) {
            ang = 6.2831855f * (DD_RAND(r) / 32767.0f);
            sp->frame[i3] = DD_RAND(r) % 8;
            vec4_set(&sp->pos[i3], rad * sinf(ang), 0.0f, zr * cosf(ang), 1.0f);
        }
        if (d->spark_next < 31)
            d->spark_next++;
        else
            d->spark_next = 0;
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        if (d->red == 0)
            list = dd_tex_pack(list, 16);
        else
            list = dd_tex_pack(list, 13);
        if (d->red == 0)
            list = dl_color(list, 0x808CD2FF);
        else
            list = dl_color(list, 0xB28282FF);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        for (i4 = 0; i4 < 32; i4++) {
            DdHaloSpark *sp2 = &d->spark[i4];

            if (!DD_BIT(d->spark[i4].flags, 1) != 1) {
                int jb;
                list = dl_ambient_light(list, (unsigned int)(out * (in * (dd_ease_cos(sp2->t, 32, 1) * 255.0f))) << 24
                                                  | 0xFFFFFF);
                for (jb = 0; jb < 3; jb++) {
                    ScePspFVector4 w __attribute__((aligned(16)));

                    mtx_push();
                    mat4_transform_vec4(&w, &d->m, &sp2->pos[jb]);
                    dd_translate3(w.x, w.y, w.z);
                    mtx_mult(&bill);
                    list = draw_array(list, 6, 0x183, 2, 0, d->spark_v[sp2->frame[jb]]);
                    mtx_pop();
                    sp2->pos[jb].y += 5.0f;
                }
                if (sp2->t < 31)
                    sp2->t++;
                else
                    sp2->flags &= ~1;
            }
        }
        mtx_pop();
        dl_set_list(list);
    }

    if (d->red != 0) {
        /* the flares: 6 frames of the lower half of the sheet and their mirrors */
        if (phase == 1) {
            fw = 30.0f;
            fz = 120.0f;
            du = 0.25f;
            dv = 0.5f;
            eps = 0.001953125f;
            for (i5 = 0; i5 < 6; i5++) {
                tu = du * (i5 % 4);
                tv = dv + du * (i5 / 4);
                switch (i5 / 2) {
                case 0:
                    d->flare_v[i5][0].u = tu;         d->flare_v[i5][0].v = tv + du;
                    d->flare_v[i5][0].color = 0xFF6496;
                    d->flare_v[i5][0].x = -fw; d->flare_v[i5][0].y = 0.0f; d->flare_v[i5][0].z = -fz;
                    d->flare_v[i5][1].u = tu;         d->flare_v[i5][1].v = tv + eps;
                    d->flare_v[i5][1].color = 0xFFFF6496;
                    d->flare_v[i5][1].x = -fw; d->flare_v[i5][1].y = 0.0f; d->flare_v[i5][1].z = 0.0f;
                    d->flare_v[i5][2].u = tu + du; d->flare_v[i5][2].v = tv + du;
                    d->flare_v[i5][2].color = 0xFF6496;
                    d->flare_v[i5][2].x = fw;  d->flare_v[i5][2].y = 0.0f; d->flare_v[i5][2].z = -fz;
                    d->flare_v[i5][3].u = tu + du; d->flare_v[i5][3].v = tv + eps;
                    d->flare_v[i5][3].color = 0xFFFF6496;
                    d->flare_v[i5][3].x = fw;  d->flare_v[i5][3].y = 0.0f; d->flare_v[i5][3].z = 0.0f;
                    break;
                case 1:
                    d->flare_v[i5][0].u = tu + du; d->flare_v[i5][0].v = tv;
                    d->flare_v[i5][0].color = 0xFF6496;
                    d->flare_v[i5][0].x = -fw; d->flare_v[i5][0].y = 0.0f; d->flare_v[i5][0].z = -fz;
                    d->flare_v[i5][1].u = tu;         d->flare_v[i5][1].v = tv;
                    d->flare_v[i5][1].color = 0xFFFF6496;
                    d->flare_v[i5][1].x = -fw; d->flare_v[i5][1].y = 0.0f; d->flare_v[i5][1].z = 0.0f;
                    d->flare_v[i5][2].u = tu + du; d->flare_v[i5][2].v = tv + du;
                    d->flare_v[i5][2].color = 0xFF6496;
                    d->flare_v[i5][2].x = fw;  d->flare_v[i5][2].y = 0.0f; d->flare_v[i5][2].z = -fz;
                    d->flare_v[i5][3].u = tu;         d->flare_v[i5][3].v = tv + du;
                    d->flare_v[i5][3].color = 0xFFFF6496;
                    d->flare_v[i5][3].x = fw;  d->flare_v[i5][3].y = 0.0f; d->flare_v[i5][3].z = 0.0f;
                    break;
                case 2:
                    d->flare_v[i5][0].u = tu;         d->flare_v[i5][0].v = tv;
                    d->flare_v[i5][0].color = 0xFF6496;
                    d->flare_v[i5][0].x = -fw; d->flare_v[i5][0].y = 0.0f; d->flare_v[i5][0].z = -fz;
                    d->flare_v[i5][1].u = tu;         d->flare_v[i5][1].v = tv + du;
                    d->flare_v[i5][1].color = 0xFFFF6496;
                    d->flare_v[i5][1].x = -fw; d->flare_v[i5][1].y = 0.0f; d->flare_v[i5][1].z = 0.0f;
                    d->flare_v[i5][2].u = tu + du; d->flare_v[i5][2].v = tv;
                    d->flare_v[i5][2].color = 0xFF6496;
                    d->flare_v[i5][2].x = fw;  d->flare_v[i5][2].y = 0.0f; d->flare_v[i5][2].z = -fz;
                    d->flare_v[i5][3].u = tu + du; d->flare_v[i5][3].v = tv + du;
                    d->flare_v[i5][3].color = 0xFFFF6496;
                    d->flare_v[i5][3].x = fw;  d->flare_v[i5][3].y = 0.0f; d->flare_v[i5][3].z = 0.0f;
                    break;
                }
                /* the mirror: texture swapped left and right */
                d->flare_v[(unsigned int)(i5 + 6)][0] = d->flare_v[i5][0];
                d->flare_v[(unsigned int)(i5 + 6)][1] = d->flare_v[i5][1];
                d->flare_v[(unsigned int)(i5 + 6)][2] = d->flare_v[i5][2];
                d->flare_v[(unsigned int)(i5 + 6)][3] = d->flare_v[i5][3];
                d->flare_v[(unsigned int)(i5 + 6)][0].u = d->flare_v[i5][2].u;
                d->flare_v[(unsigned int)(i5 + 6)][0].v = d->flare_v[i5][2].v;
                d->flare_v[(unsigned int)(i5 + 6)][1].u = d->flare_v[i5][3].u;
                d->flare_v[(unsigned int)(i5 + 6)][1].v = d->flare_v[i5][3].v;
                d->flare_v[(unsigned int)(i5 + 6)][2].u = d->flare_v[i5][0].u;
                d->flare_v[(unsigned int)(i5 + 6)][2].v = d->flare_v[i5][0].v;
                d->flare_v[(unsigned int)(i5 + 6)][3].u = d->flare_v[i5][1].u;
                d->flare_v[(unsigned int)(i5 + 6)][3].v = d->flare_v[i5][1].v;
            }
        }
        if (d->grow == d->grow_end) {
            f = &d->flare[d->flare_next];
            f->flags |= 1;
            f->t = 0;
            i6 = 0;
            do {
                f->frame[i6] = DD_RAND(r) % 12;
                f->ang[i6] = 6.2831855f * (DD_RAND(r) / 32767.0f);
            } while (++i6 < 4);
            if (d->flare_next < 7)
                d->flare_next++;
            else
                d->flare_next = 0;
        }
        if (d->grow == d->grow_end) {
            ScePspFVector4 axis __attribute__((aligned(16)));
            ScePspFMatrix4 tilt __attribute__((aligned(16)));
            ScePspFVector4 up __attribute__((aligned(16)));

            list = dl_select_layer(7, 0);
            list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
            list = dd_depth_enable(list, 0, 1);
            list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
            list = dd_tex_pack(list, 11);
            mtx_set_mode(2);
            mtx_push();
            mtx_identity();
            axis = dd_pyre_axis;
            vec4_copy(&up, dd_chara_pos());
            mat4_identity(&tilt);
            mat4_mul_rot_x(&tilt, &tilt, 1.3962635f);
            for (i7 = 0; i7 < 8; i7++) {
                DdFlare *f2 = &d->flare[i7];

                if (!DD_BIT(d->flare[i7].flags, 1) != 1) {
                    int jb;

                    e = dd_ease_sin(f2->t, 8, 1);
                    list = dl_ambient_light(list, (unsigned int)(out * (in * (dd_ease_cos(f2->t, 8, 1) * 255.0f))) << 24
                                                      | 0xFFFFFF);
                    for (jb = 0; jb < 4; jb++) {
                        /* a frame whose z follows the flare and whose x faces the camera */
                        ScePspFVector4 off __attribute__((aligned(16)));
                        ScePspFVector4 dir __attribute__((aligned(16)));
                        ScePspFVector4 side __attribute__((aligned(16)));
                        ScePspFVector4 upv __attribute__((aligned(16)));
                        ScePspFVector4 back __attribute__((aligned(16)));
                        ScePspFMatrix4 tmp __attribute__((aligned(16)));
                        ScePspFMatrix4 rot __attribute__((aligned(16)));

                        vec4_set(&off, -20.0f * sinf(f2->ang[jb]), 0.0f, -5.0f * cosf(f2->ang[jb]), 1.0f);
                        mat4_mul_rot_y(&tmp, &tilt, f2->ang[jb]);
                        mat4_translate(&tmp, &tmp, &off);
                        mat4_mul(&tmp, &d->m, &tmp);
                        dd_mtx_strip(&rot, &tmp);
                        mat4_transform_vec4(&dir, &rot, &axis);
                        vec3_normalize2(&back, &dir);
                        vec3_cross(&side, &back, &up);
                        vec3_normalize2(&side, &side);
                        vec3_cross(&upv, &side, &back);
                        vec3_normalize2(&upv, &upv);
                        vec4_set(&tmp.x, side.x * e, side.y * e, side.z * e, 0.0f);
                        vec4_set(&tmp.y, upv.x, upv.y, upv.z, 0.0f);
                        vec4_set(&tmp.z, back.x, back.y, back.z, 0.0f);
                        mtx_push();
                        mtx_load(&tmp);
                        list = draw_array(list, 4, 0x19F, 4, 0, d->flare_v[f2->frame[jb]]);
                        mtx_pop();
                    }
                    if (f2->t < 7)
                        f2->t++;
                    else
                        f2->flags &= ~1;
                }
            }
            mtx_pop();
            dl_set_list(list);
        }
    }
    dd_proj_restore();
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
