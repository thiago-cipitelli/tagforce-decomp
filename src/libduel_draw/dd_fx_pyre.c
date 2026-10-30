// @range 0x08846348-0x0884774C
// @asm 0x0884653C-0x0884774C
// @data 0x08876C40-0x08876C80
/* Effects 83-90 and 118-128: a ring of 16 petals turning under a node of the card model, sparks
   rising from it and, in the red variant (dd_fx_table.arg4 != 0), flame flares bursting outward
   (task of 0x11E0 bytes). */
#include "libduel_draw.h"

int dd_pyre_update(int phase, DdPyreFx *d);

/* local templates of dd_pyre_update (kept in asm): the two node offsets, the petal scale (y = fade
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
    int task = func_0882623C(dd_pyre_update, a, b, sizeof(DdPyreFx));
    DdPyreFx *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
        d->red = red;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 4,6 KB com quadro grande: alocação de registradores e da pilha diferem */
int dd_pyre_update(int phase, DdPyreFx *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    ScePspFMatrix4 bill;
    ScePspFMatrix4 tmp;
    ScePspFMatrix4 rot;
    ScePspFMatrix4 tilt;
    ScePspFVector4 node;
    ScePspFVector4 up;
    ScePspFVector4 axis;
    ScePspFVector4 dir;
    ScePspFVector4 side;
    ScePspFVector4 back;
    ScePspFVector4 off;
    ScePspFVector4 w;
    ScePspFVector3 scale;
    DdRandSeed *r;
    DdHaloSpark *sp;
    DdFlare *f;
    DdRingVtx *q;
    DdTCVtx *v;
    DdTCVtx *m;
    void *list;
    unsigned int c0;
    unsigned int c1;
    float in;
    float out;
    float grow;
    float rad;
    float ang;
    float e;
    float tu;
    float tv;
    int flip;
    int i;
    int j;

    func_08827738();
    func_0882745C(0.0f, 0.0f, 1.0f, 0.0f);
    if (phase == 1)
        dd_pyre_kind(s->id, &d->grow_end, &d->end);
    in = func_088279A4(d->t, 16);
    out = 1.0f - func_088279A4(d->t - (d->end - 16), 16);
    grow = d->t * 0.003f + 1.0f;
    func_08827258(&bill);

    /* first frame: under the node, turned over (above it for effects 84 and 90) */
    if (d->grow == 0 && d->placed == 0) {
        flip = !(s->id == 90 || s->id == 84);
        d->placed = 1;
        func_0884DAF8(&d->m, 0x27);
        if (flip) {
            node = dd_pyre_down;
            mat4_copy(&tmp, &d->m);
            func_08827598(&tmp, &tmp);
            mat4_transform_vec4(&node, &tmp, &node);
            mat4_identity(&tmp);
            mat4_mul_rot_x(&tmp, &tmp, 3.1415927f);
            mat4_mul(&d->m, &d->m, &tmp);
            mat4_translate(&d->m, &d->m, &node);
        } else {
            node = dd_pyre_up;
            mat4_copy(&rot, &d->m);
            func_08827598(&rot, &rot);
            mat4_transform_vec4(&node, &rot, &node);
            mat4_identity(&rot);
            mat4_mul_rot_x(&rot, &rot, -1.5707964f);
            mat4_mul(&d->m, &d->m, &rot);
            mat4_translate(&d->m, &d->m, &node);
        }
    }
    func_08827F68(0x622A, d->grow, d->grow_end, &d->se_done, 1);

    /* the petals */
    if (phase == 1) {
        if (d->red == 0) {
            c0 = 0x00AAFF;
            c1 = 0x8000AAFF;
        } else {
            c0 = 0x6464FF;
            c1 = 0xB26464FF;
        }
        d->petal[0].u = 0.0f; d->petal[0].v = 0.0f; d->petal[0].color = c0;
        d->petal[0].x = -30.0f; d->petal[0].y = 200.0f; d->petal[0].z = 0.0f;
        d->petal[1].u = 0.0f; d->petal[1].v = 0.5f; d->petal[1].color = c1;
        d->petal[1].x = -30.0f; d->petal[1].y = 0.0f; d->petal[1].z = 0.0f;
        d->petal[2].u = 1.0f; d->petal[2].v = 0.0f; d->petal[2].color = c0;
        d->petal[2].x = 30.0f; d->petal[2].y = 200.0f; d->petal[2].z = 0.0f;
        d->petal[3].u = 1.0f; d->petal[3].v = 0.5f; d->petal[3].color = c1;
        d->petal[3].x = 30.0f; d->petal[3].y = 0.0f; d->petal[3].z = 0.0f;
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = func_08826A64(list, 3);
        mtx_set_mode(2);
        mtx_push();
        mtx_load(&d->m);
        scale.x = dd_pyre_scale.x;
        scale.y = in;
        scale.z = dd_pyre_scale.z;
        rad = grow * 35.0f;
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i = 0; i < 16; i++) {
            ang = 6.2831855f * i / 16.0f + d->frame * 0.0034906587f;
            mtx_push();
            func_08827300(rad * sinf(ang), 0.0f, grow * 5.0f * cosf(ang));
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
        for (i = 0; i < 8; i++) {
            q = d->spark_v[i];
            q[0].u = i % 4 * 0.25f;
            q[0].v = i / 4 * 0.5f;
            q[0].x = -4.0f;
            q[0].y = 4.0f;
            q[0].z = 0.0f;
            q[1].u = i % 4 * 0.25f + 0.25f;
            q[1].v = i / 4 * 0.5f + 0.5f;
            q[1].x = 4.0f;
            q[1].y = -4.0f;
            q[1].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        sp = &d->spark[d->spark_next];
        sp->flags |= 1;
        sp->t = 0;
        for (i = 0; i < 3; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            sp->frame[i] = DD_RAND(r) % 8;
            vec4_set(&sp->pos[i], grow * 35.0f * sinf(ang), 0.0f, grow * 20.0f * cosf(ang), 1.0f);
        }
        if (d->spark_next < 31)
            d->spark_next++;
        else
            d->spark_next = 0;
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = func_08826A64(list, d->red == 0 ? 16 : 13);
        list = dl_color(list, d->red == 0 ? 0x808CD2FF : 0xB28282FF);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        for (i = 0; i < 32; i++) {
            sp = &d->spark[i];
            if (!(sp->flags & 1))
                continue;
            list = dl_ambient_light(list, (unsigned int)(out * in * func_08827AD4(sp->t, 32, 1) * 255.0f) << 24
                                              | 0xFFFFFF);
            for (j = 0; j < 3; j++) {
                mtx_push();
                mat4_transform_vec4(&w, &d->m, &sp->pos[j]);
                func_08827300(w.x, w.y, w.z);
                mtx_mult(&bill);
                list = draw_array(list, 6, 0x183, 2, 0, d->spark_v[sp->frame[j]]);
                mtx_pop();
                sp->pos[j].y += 5.0f;
            }
            if (sp->t < 31)
                sp->t++;
            else
                sp->flags &= ~1;
        }
        mtx_pop();
        dl_set_list(list);
    }

    if (d->red != 0) {
        /* the flares: 6 frames of the lower half of the sheet and their mirrors */
        if (phase == 1) {
            for (i = 0; i < 6; i++) {
                v = d->flare_v[i];
                tu = i % 4 * 0.25f;
                tv = i / 4 * 0.25f + 0.5f;
                switch (i / 2) {
                case 0:
                    v[0].u = tu;         v[0].v = tv + 0.25f;
                    v[1].u = tu;         v[1].v = tv + 0.001953125f;
                    v[2].u = tu + 0.25f; v[2].v = tv + 0.25f;
                    v[3].u = tu + 0.25f; v[3].v = tv + 0.001953125f;
                    break;
                case 1:
                    v[0].u = tu + 0.25f; v[0].v = tv;
                    v[1].u = tu;         v[1].v = tv;
                    v[2].u = tu + 0.25f; v[2].v = tv + 0.25f;
                    v[3].u = tu;         v[3].v = tv + 0.25f;
                    break;
                case 2:
                    v[0].u = tu;         v[0].v = tv;
                    v[1].u = tu;         v[1].v = tv + 0.25f;
                    v[2].u = tu + 0.25f; v[2].v = tv;
                    v[3].u = tu + 0.25f; v[3].v = tv + 0.25f;
                    break;
                }
                v[0].color = 0xFF6496;   v[0].x = -30.0f; v[0].y = 0.0f; v[0].z = -120.0f;
                v[1].color = 0xFFFF6496; v[1].x = -30.0f; v[1].y = 0.0f; v[1].z = 0.0f;
                v[2].color = 0xFF6496;   v[2].x = 30.0f;  v[2].y = 0.0f; v[2].z = -120.0f;
                v[3].color = 0xFFFF6496; v[3].x = 30.0f;  v[3].y = 0.0f; v[3].z = 0.0f;
                /* the mirror: texture swapped left and right */
                m = d->flare_v[i + 6];
                m[0] = v[0];
                m[1] = v[1];
                m[2] = v[2];
                m[3] = v[3];
                m[0].u = v[2].u;
                m[0].v = v[2].v;
                m[1].u = v[3].u;
                m[1].v = v[3].v;
                m[2].u = v[0].u;
                m[2].v = v[0].v;
                m[3].u = v[1].u;
                m[3].v = v[1].v;
            }
        }
        if (d->grow == d->grow_end) {
            f = &d->flare[d->flare_next];
            f->flags |= 1;
            f->t = 0;
            for (i = 0; i < 4; i++) {
                f->frame[i] = DD_RAND(r) % 12;
                f->ang[i] = DD_RAND(r) / 32767.0f * 6.2831855f;
            }
            if (d->flare_next < 7)
                d->flare_next++;
            else
                d->flare_next = 0;
        }
        if (d->grow == d->grow_end) {
            list = dl_select_layer(7, 0);
            list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
            list = func_0882698C(list, 0, 1);
            list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
            list = func_08826A64(list, 11);
            mtx_set_mode(2);
            mtx_push();
            mtx_identity();
            axis = dd_pyre_axis;
            vec4_copy(&up, func_0884DB2C());
            mat4_identity(&tilt);
            mat4_mul_rot_x(&tilt, &tilt, 1.3962634f);
            for (i = 0; i < 8; i++) {
                f = &d->flare[i];
                if (!(f->flags & 1))
                    continue;
                e = func_08827A2C(f->t, 8, 1);
                list = dl_ambient_light(list, (unsigned int)(out * in * func_08827AD4(f->t, 8, 1) * 255.0f) << 24
                                                  | 0xFFFFFF);
                for (j = 0; j < 4; j++) {
                    /* a frame whose z follows the flare and whose x faces the camera */
                    vec4_set(&off, sinf(f->ang[j]) * -20.0f, 0.0f, cosf(f->ang[j]) * -5.0f, 1.0f);
                    mat4_mul_rot_y(&tmp, &tilt, f->ang[j]);
                    mat4_translate(&tmp, &tmp, &off);
                    mat4_mul(&tmp, &d->m, &tmp);
                    func_08827598(&rot, &tmp);
                    mat4_transform_vec4(&dir, &rot, &axis);
                    vec3_normalize2(&back, &dir);
                    vec3_cross(&side, &back, &up);
                    vec3_normalize2(&side, &side);
                    vec3_cross(&dir, &side, &back);
                    vec3_normalize2(&dir, &dir);
                    vec4_set(&tmp.x, side.x * e, side.y * e, side.z * e, 0.0f);
                    vec4_set(&tmp.y, dir.x, dir.y, dir.z, 0.0f);
                    vec4_set(&tmp.z, back.x, back.y, back.z, 0.0f);
                    mtx_push();
                    mtx_load(&tmp);
                    list = draw_array(list, 4, 0x19F, 4, 0, d->flare_v[f->frame[j]]);
                    mtx_pop();
                }
                if (f->t < 7)
                    f->t++;
                else
                    f->flags &= ~1;
            }
            mtx_pop();
            dl_set_list(list);
        }
    }
    func_08827548();
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
