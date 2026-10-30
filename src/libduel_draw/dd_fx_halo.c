// @range 0x08843248-0x08844024
// @asm 0x088433F0-0x08844024
// @data 0x08876BF0-0x08876C40
/* Effects 42-50, 63-64, 71-77 and 82: a ring of 16 glowing petals turning under a node of the card
   model, with sparks that rise and stretch along the view (task of 0x710 bytes). */
#include "libduel_draw.h"

int dd_halo_update(int phase, DdHaloFx *d);

/* local templates of dd_halo_update (kept in asm): placement offset, the two node offsets, the petal
   scale (x = fade in, padded to 16 bytes) and the sparks' axis */
ScePspFVector4 dd_halo_off = { 0.0f, 0.0f, 0.0f, 1.0f };
ScePspFVector4 dd_halo_down = { 0.0f, 0.0f, 44.0f, 1.0f };
ScePspFVector4 dd_halo_up = { 0.0f, 0.0f, -44.0f, 1.0f };
ScePspFVector4 dd_halo_scale = { 1.0f, 0.0f, 0.0f, 0.0f };
ScePspFVector4 dd_halo_axis = { 0.0f, 1.0f, 0.0f, 1.0f };

/* grow time and length by effect id */
void dd_halo_kind(int id, int *grow, int *end)
{
    switch (id) {
    case 42:
        *grow = 50;
        break;
    case 43: case 44: case 45:
        *grow = 41;
        break;
    case 46: case 47: case 48:
        *grow = 45;
        break;
    case 49: case 50: case 63: case 64:
        *grow = 50;
        break;
    case 71: case 72: case 73: case 74:
        *grow = 100;
        break;
    case 75: case 76:
        *grow = 85;
        break;
    case 77:
        *grow = 124;
        break;
    case 82:
        *grow = 79;
        break;
    default:
        *grow = 0;
        *end = 0;
        return;
    }
    *end = 48;
}

int dd_fx42_start(DdFxSlot *s, int a, int b)
{
    int task = func_0882623C(dd_halo_update, a, b, sizeof(DdHaloFx));
    DdHaloFx *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 3,1 KB com quadro grande: alocação de registradores e da pilha diferem */
int dd_halo_update(int phase, DdHaloFx *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    ScePspFMatrix4 bill;
    ScePspFMatrix4 tmp;
    ScePspFMatrix4 rot;
    ScePspFVector4 off;
    ScePspFVector4 node;
    ScePspFVector4 up;
    ScePspFVector4 axis;
    ScePspFVector4 dir;
    ScePspFVector4 side;
    ScePspFVector4 back;
    ScePspFVector3 scale;
    DdRandSeed *r;
    DdHaloSpark *sp;
    DdRingVtx *q;
    void *list;
    float in;
    float out;
    float grow;
    float tilt;
    float rad;
    float ang;
    float sx;
    float sy;
    float tu;
    float tv;
    int flip;
    int i;
    int j;

    func_08827738();
    func_0882745C(0.0f, 0.0f, 1.0f, 0.0f);
    if (phase == 1)
        dd_halo_kind(s->id, &d->grow_end, &d->end);
    in = func_088279A4(d->t, 16);
    out = 1.0f - func_088279A4(d->t - (d->end - 16), 16);
    grow = d->t * 0.01f + 1.0f;
    func_08827258(&bill);

    /* first frame: the halo frame, under the node (or above it, turned, for 43 and 46) */
    if (d->grow == 0 && d->placed == 0) {
        tilt = 0.0f;
        off = dd_halo_off;
        flip = 1;
        if (s->id == 76 || s->id == 47) {
            tilt = 0.17453294f;
        } else if (s->id == 46) {
            off.x = 10.0f;
            off.y = 52.0f;
            flip = 0;
        } else if (s->id == 43) {
            off.y = -16.0f;
            flip = 0;
        }
        d->placed = 1;
        func_0884DAF8(&d->m, 0x27);
        if (flip) {
            node = dd_halo_down;
            mat4_copy(&tmp, &d->m);
            func_08827598(&tmp, &tmp);
            mat4_transform_vec4(&node, &tmp, &node);
            mat4_identity(&tmp);
            mat4_mul_rot_x(&tmp, &tmp, tilt + 3.1415927f);
            mat4_mul(&d->m, &d->m, &tmp);
            mat4_translate(&d->m, &d->m, &node);
            mat4_translate(&d->m, &d->m, &off);
        } else {
            node = dd_halo_up;
            mat4_copy(&rot, &d->m);
            func_08827598(&rot, &rot);
            mat4_transform_vec4(&node, &rot, &node);
            mat4_identity(&rot);
            mat4_mul_rot_x(&rot, &rot, tilt + -1.5707964f);
            mat4_mul(&d->m, &d->m, &rot);
            mat4_translate(&d->m, &d->m, &node);
            mat4_translate(&d->m, &d->m, &off);
        }
    }
    func_08827F68(0x6229, d->grow, d->grow_end, &d->se_done, 1);

    /* the petals */
    if (phase == 1) {
        d->petal[0].u = 0.0f;
        d->petal[0].v = 0.0f;
        d->petal[0].color = 0xFFAA64;
        d->petal[0].x = -30.0f;
        d->petal[0].y = 100.0f;
        d->petal[0].z = 0.0f;
        d->petal[1].u = 0.0f;
        d->petal[1].v = 0.5f;
        d->petal[1].color = 0x80FFAA64;
        d->petal[1].x = -30.0f;
        d->petal[1].y = 0.0f;
        d->petal[1].z = 0.0f;
        d->petal[2].u = 1.0f;
        d->petal[2].v = 0.0f;
        d->petal[2].color = 0xFFAA64;
        d->petal[2].x = 30.0f;
        d->petal[2].y = 100.0f;
        d->petal[2].z = 0.0f;
        d->petal[3].u = 1.0f;
        d->petal[3].v = 0.5f;
        d->petal[3].color = 0x80FFAA64;
        d->petal[3].x = 30.0f;
        d->petal[3].y = 0.0f;
        d->petal[3].z = 0.0f;
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
        scale.x = dd_halo_scale.x;
        scale.y = in;
        scale.z = dd_halo_scale.z;
        rad = grow * 28.0f;
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

    /* the sparks: 8 frames of a 4 x 2 sheet, anchored at the bottom */
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            q = d->spark_v[i];
            tu = i % 4 * 0.25f;
            tv = i / 4 * 0.5f;
            q[0].u = tu;
            q[0].v = tv;
            q[0].x = -8.0f;
            q[0].y = 48.0f;
            q[0].z = 0.0f;
            q[1].u = tu;
            q[1].v = tv + 0.5f;
            q[1].x = -8.0f;
            q[1].y = -48.0f;
            q[1].z = 0.0f;
            q[2].u = tu + 0.25f;
            q[2].v = tv;
            q[2].x = 8.0f;
            q[2].y = 48.0f;
            q[2].z = 0.0f;
            q[3].u = tu + 0.25f;
            q[3].v = tv + 0.5f;
            q[3].x = 8.0f;
            q[3].y = -48.0f;
            q[3].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        sp = &d->spark[d->spark_next];
        rad = grow * 28.0f;
        sp->flags |= 1;
        sp->t = 0;
        for (i = 0; i < 3; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            sp->frame[i] = DD_RAND(r) % 8;
            vec4_set(&sp->pos[i], rad * sinf(ang), 0.0f, cosf(ang) * 20.0f, 1.0f);
        }
        if (d->spark_next < 11)
            d->spark_next++;
        else
            d->spark_next = 0;
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = func_08826A64(list, 14);
        list = dl_color(list, 0x80FFAA64);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        axis = dd_halo_axis;
        vec4_copy(&up, func_0884DB2C());
        for (i = 0; i < 12; i++) {
            sp = &d->spark[i];
            if (!(sp->flags & 1))
                continue;
            sx = 1.0f - func_08827A2C(sp->t - 4, 8, 2) * 0.7f;
            sy = func_08827A2C(sp->t - 4, 8, 2) * 0.2f + 1.0f;
            list = dl_ambient_light(list, (unsigned int)(out * in * func_08827AD4(sp->t, 12, 1) * 255.0f) << 24
                                              | 0xFFFFFF);
            for (j = 0; j < 3; j++) {
                /* a frame whose y follows the halo axis and whose x faces the camera */
                mat4_identity(&tmp);
                mat4_translate(&tmp, &tmp, &sp->pos[j]);
                mat4_mul(&tmp, &d->m, &tmp);
                func_08827598(&rot, &tmp);
                mat4_transform_vec4(&dir, &rot, &axis);
                vec3_normalize2(&dir, &dir);
                vec3_cross(&side, &dir, &up);
                vec3_normalize2(&side, &side);
                vec3_cross(&back, &side, &dir);
                vec3_normalize2(&back, &back);
                vec4_set(&tmp.x, side.x * sx, side.y * sx, side.z * sx, 0.0f);
                vec4_set(&tmp.y, dir.x * sy, dir.y * sy, dir.z * sy, 0.0f);
                vec4_set(&tmp.z, back.x, back.y, back.z, 0.0f);
                mtx_push();
                mtx_load(&tmp);
                list = draw_array(list, 4, 0x183, 4, 0, d->spark_v[sp->frame[j]]);
                mtx_pop();
                sp->pos[j].y += 8.0f;
            }
            if (sp->t < 11)
                sp->t++;
            else
                sp->flags &= ~1;
        }
        mtx_pop();
        dl_set_list(list);
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
