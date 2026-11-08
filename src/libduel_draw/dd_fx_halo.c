// @range 0x08843248-0x08844024
// @data 0x08876BF0-0x08876C40
/* Effects 42-50, 63-64, 71-77 and 82: a ring of 16 glowing petals turning under a node of the card
   model, with sparks that rise and stretch along the view (task of 0x710 bytes). */
#include "dd_fx.h"

int dd_halo_update(int phase, DdHaloFx *d);

/* local templates of dd_halo_update: placement offset, the two node offsets, the petal
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
    int task = dd_task_create(dd_halo_update, a, b, sizeof(DdHaloFx));
    DdHaloFx *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

int dd_halo_update(int phase, DdHaloFx *d)
{
    DdHaloSpark *ns;
    DdFxSlot *s = d->slot;
    int run = s->param;
    ScePspFVector3 scale;
    ScePspFMatrix4 bill __attribute__((aligned(16)));
    ScePspFVector4 off __attribute__((aligned(16)));
    ScePspFVector4 down __attribute__((aligned(16)));
    ScePspFMatrix4 tmp __attribute__((aligned(16)));
    ScePspFVector4 node __attribute__((aligned(16)));
    ScePspFMatrix4 rot __attribute__((aligned(16)));
    ScePspFVector4 axis __attribute__((aligned(16)));
    ScePspFVector4 up __attribute__((aligned(16)));
    ScePspFVector4 dir __attribute__((aligned(16)));
    ScePspFVector4 side __attribute__((aligned(16)));
    ScePspFVector4 ndir __attribute__((aligned(16)));
    ScePspFVector4 back __attribute__((aligned(16)));
    ScePspFMatrix4 m __attribute__((aligned(16)));
    ScePspFMatrix4 mrot __attribute__((aligned(16)));
    DdRandSeed *r;
    DdHaloSpark *sp;
    DdRingVtx *q;
    void *list;
    float in;
    float out;
    float grow;
    float tilt;
    float rad;
    float radz;
    float ang;
    float sx;
    float sy;
    float tu;
    float tv;
    int flip;
    int i;
    int j;
    int k;
    int n;

    dd_mtx_push_saved();
    dd_proj_set(0.0f, 0.0f, 1.0f, 0.0f);
    if (phase == 1)
        dd_halo_kind(s->id, &d->grow_end, &d->end);
    in = dd_ease_lin(d->t, 16);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 16), 16);
    grow = d->t * 0.01f + 1.0f;
    dd_mtx_billboard_cam(&bill);

    /* first frame: the halo frame, under the node (or above it, turned, for 43 and 46) */
    if (d->grow == 0 && d->placed == 0) {
        off = dd_halo_off;
        tilt = 0.0f;
        flip = 1;
        switch (s->id) {
        case 43:
            flip = 0;
            off.y = -16.0f;
            break;
        case 46:
            off.x = 10.0f;
            flip = 0;
            off.y = 52.0f;
            break;
        case 47: case 76:
            tilt = 0.17453294f;
            break;
        }
        d->placed = 1;
        dd_chara_bone_mtx(&d->m, 0x27);
        if (flip == 1) {
            down = dd_halo_down;
            mat4_copy(&tmp, &d->m);
            dd_mtx_strip(&tmp, &tmp);
            mat4_transform_vec4(&down, &tmp, &down);
            mat4_identity(&tmp);
            mat4_mul_rot_x(&tmp, &tmp, tilt + 3.1415927f);
            mat4_mul(&d->m, &d->m, &tmp);
            mat4_translate(&d->m, &d->m, &down);
            mat4_translate(&d->m, &d->m, &off);
        } else {
            node = dd_halo_up;
            mat4_copy(&rot, &d->m);
            dd_mtx_strip(&rot, &rot);
            mat4_transform_vec4(&node, &rot, &node);
            mat4_identity(&rot);
            mat4_mul_rot_x(&rot, &rot, tilt + -1.5707964f);
            mat4_mul(&d->m, &d->m, &rot);
            mat4_translate(&d->m, &d->m, &node);
            mat4_translate(&d->m, &d->m, &off);
        }
    }
    dd_cue_se(0x6229, d->grow, d->grow_end, &d->se_done, 1);

    /* the petals */
    if (phase == 1) {
        float pw = 30.0f;

        d->petal[0].u = 0.0f;
        d->petal[0].v = 0.0f;
        d->petal[0].color = 0xFFAA64;
        d->petal[0].x = -pw;
        d->petal[0].y = 100.0f;
        d->petal[0].z = 0.0f;
        d->petal[1].u = 0.0f;
        d->petal[1].v = 0.5f;
        d->petal[1].color = 0x80FFAA64;
        d->petal[1].x = -pw;
        d->petal[1].y = 0.0f;
        d->petal[1].z = 0.0f;
        d->petal[2].u = 1.0f;
        d->petal[2].v = 0.0f;
        d->petal[2].color = 0xFFAA64;
        d->petal[2].x = pw;
        d->petal[2].y = 100.0f;
        d->petal[2].z = 0.0f;
        d->petal[3].u = 1.0f;
        d->petal[3].v = 0.5f;
        d->petal[3].color = 0x80FFAA64;
        d->petal[3].x = pw;
        d->petal[3].y = 0.0f;
        d->petal[3].z = 0.0f;
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
        scale.x = dd_halo_scale.x;
        scale.y = dd_halo_scale.y;
        scale.z = dd_halo_scale.z;
        scale.y = in;
        rad = grow * 28.0f;
        radz = grow * 5.0f;
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (k = 0; k < 16; k++) {
            ang = 6.2831855f * k / 16.0f + d->frame * 0.0034906587f;
            mtx_push();
            dd_translate3(rad * sinf(ang), 0.0f, radz * cosf(ang));
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
        float w = 8.0f;
        float h = 48.0f;
        float du = 0.25f;
        float dv = 0.5f;

        for (n = 0; n < 8; n++) {
            tu = n % 4 * du;
            tv = n / 4 * dv;
            d->spark_v[n][0].u = tu;
            d->spark_v[n][0].v = tv;
            d->spark_v[n][0].x = -w;
            d->spark_v[n][0].y = h;
            d->spark_v[n][0].z = 0.0f;
            d->spark_v[n][1].u = tu;
            d->spark_v[n][1].v = tv + dv;
            d->spark_v[n][1].x = -w;
            d->spark_v[n][1].y = -h;
            d->spark_v[n][1].z = 0.0f;
            d->spark_v[n][2].u = tu + du;
            d->spark_v[n][2].v = tv;
            d->spark_v[n][2].x = w;
            d->spark_v[n][2].y = h;
            d->spark_v[n][2].z = 0.0f;
            d->spark_v[n][3].u = tu + du;
            d->spark_v[n][3].v = tv + dv;
            d->spark_v[n][3].x = w;
            d->spark_v[n][3].y = -h;
            d->spark_v[n][3].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        ns = &d->spark[d->spark_next];
        rad = grow * 28.0f;
        ns->flags |= 1;
        ns->t = 0;
        for (i = 0; i < 3; i++) {
            ang = DD_RAND(r) / 32767.0f * 6.2831855f;
            ns->frame[i] = DD_RAND(r) % 8;
            vec4_set(&ns->pos[i], rad * sinf(ang), 0.0f, cosf(ang) * 20.0f, 1.0f);
        }
        if (d->spark_next < 11)
            d->spark_next++;
        else
            d->spark_next = 0;
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dd_tex_pack(list, 14);
        list = dl_color(list, 0x80FFAA64);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        axis = dd_halo_axis;
        vec4_copy(&up, dd_chara_pos());
        for (i = 0; i < 12; i++) {
            sp = &d->spark[i];
            if (!(sp->flags & 1) == 1)
                continue;
            sx = 1.0f - dd_ease_sin(sp->t - 4, 8, 2) * 0.7f;
            sy = dd_ease_sin(sp->t - 4, 8, 2) * 0.2f + 1.0f;
            list = dl_ambient_light(list, (unsigned int)(out * (in * (dd_ease_cos(sp->t, 12, 1) * 255.0f))) << 24
                                              | 0xFFFFFF);
            for (j = 0; j < 3; j++) {
                /* a frame whose y follows the halo axis and whose x faces the camera */
                mat4_identity(&m);
                mat4_translate(&m, &m, &sp->pos[j]);
                mat4_mul(&m, &d->m, &m);
                dd_mtx_strip(&mrot, &m);
                mat4_transform_vec4(&dir, &mrot, &axis);
                vec3_normalize2(&ndir, &dir);
                vec3_cross(&side, &ndir, &up);
                vec3_normalize2(&side, &side);
                vec3_cross(&back, &side, &ndir);
                vec3_normalize2(&back, &back);
                vec4_set(&m.x, side.x * sx, side.y * sx, side.z * sx, 0.0f);
                vec4_set(&m.y, ndir.x * sy, ndir.y * sy, ndir.z * sy, 0.0f);
                vec4_set(&m.z, back.x, back.y, back.z, 0.0f);
                mtx_push();
                mtx_load(&m);
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
