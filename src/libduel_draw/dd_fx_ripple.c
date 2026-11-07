// @range 0x08842824-0x08843248
// @rodata 0x08870820-0x08870850
// @data 0x08876BD0-0x08876BF0
/* Effects 51 and 55-62: at a node of the card model, a ring that refracts the screen behind it (16
   growing copies) and a glowing flat diamond (task of 0xAB4 bytes). */
#include "dd_fx.h"

int dd_ripple_update(int phase, DdRippleFx *d);

/* offset of the node for the big models and the w = 1 point, copied by dd_ripple_update */
ScePspFVector4 dd_ripple_off = { 0.0f, 0.0f, -0.19111691f, 1.0f };
ScePspFVector4 dd_ripple_w = { 0.0f, 0.0f, 0.0f, 1.0f };

/* grow time and length by effect id (effect 57 is longer for owners of kind 21) */
void dd_ripple_kind(int type, int sub, int id, int *grow, int *end)
{
    switch (id) {
    case 51:
        *grow = 24;
        break;
    case 55: case 56: case 58:
        *grow = 23;
        break;
    case 57:
        if (type == 21)
            *grow = 45;
        else
            *grow = 23;
        break;
    case 59:
        *grow = 43;
        break;
    case 60:
        *grow = 45;
        break;
    case 61:
        *grow = 50;
        break;
    case 62:
        *grow = 45;
        break;
    case 52: case 53: case 54:
    default:
        *grow = 0;
        *end = 0;
        return;
    }
    *end = 24;
}

int dd_fx51_start(DdFxSlot *s, int a, int b)
{
    int task = dd_task_create(dd_ripple_update, a, b, sizeof(DdRippleFx));
    DdRippleFx *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->slot = s;
    }
    return task;
}

int dd_ripple_update(int phase, DdRippleFx *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    int buf = d->frame % 2;
    unsigned int color[3];
    ScePspFMatrix4 bill __attribute__((aligned(16)));
    ScePspFMatrix4 node __attribute__((aligned(16)));
    ScePspFMatrix4 rot __attribute__((aligned(16)));
    ScePspFVector4 off __attribute__((aligned(16)));
    ScePspFMatrix4 vp __attribute__((aligned(16)));
    ScePspFVector4 w __attribute__((aligned(16)));
    void *list;
    float in;
    float out;
    float rad;
    float ang;
    float sc;
    float x;
    float z;
    int type;
    int i;
    int j;
    int k;
    int m;
    int l;

    dd_mtx_push_saved();
    if (phase == 1) {
        dd_ripple_kind(s->type, s->sub, s->id, &d->grow_end, &d->end);
        if (s->type == 8 && s->id == 62)
            d->node = 0x2A;
        else
            d->node = 0x25;
        d->dia_scale = 1.0f;
        d->ring_scale = 1.0f;
        switch (s->type) {
        case 3: case 4: case 24: case 31:
            d->dia_scale = 1.24f;
            break;
        case 8: case 10: case 11: case 12: case 13: case 15: case 17: case 40:
            d->dia_scale = 0.9f;
            break;
        case 21:
            d->ring_scale = 0.8f;
            break;
        default:
            d->dia_scale = 1.0f;
            break;
        }
    }
    in = dd_ease_lin(d->t, 8);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 16), 16);
    dd_mtx_billboard_cam(&bill);
    dd_chara_bone_mtx(&node, d->node);
    /* the large models: move the node a little toward the camera */
    type = s->type;
    if ((type >= 47 && type <= 101) || (type >= 137 && type < 152)) {
        off = dd_ripple_off;
        dd_mtx_strip(&rot, &node);
        mat4_transform_xyz(&off, &rot, &off);
        mat4_translate(&node, &node, &off);
    }
    dd_cue_se(0x6229, d->grow, d->grow_end, &d->se_done, 1);

    /* the ring */
    if (phase == 1) {
        rad = d->ring_scale * 5.0f;
        d->fan[0].u = 0.0f;
        d->fan[0].v = 0.0f;
        d->fan[0].color = 0x805064C8;
        d->fan[0].x = 0.0f;
        d->fan[0].y = 0.0f;
        d->fan[0].z = 0.0f;
        for (j = 0; j < 33; j++) {
            ang = 6.2831855f * j / 32.0f;
            d->fan[j + 1].u = 0.0f;
            d->fan[j + 1].v = 0.0f;
            d->fan[j + 1].color = 0x5064C8;
            d->fan[j + 1].x = rad * sinf(ang);
            d->fan[j + 1].y = rad * cosf(ang);
            d->fan[j + 1].z = 0.0f;
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dd_tex_fb_copy(list);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        dd_translate3(node.w.x, node.w.y, node.w.z);
        mtx_mult(&bill);
        dd_mtx_mvp_cam(&vp);
        for (k = 0; k < 34; k++) {
            w = dd_ripple_w;
            w.x = d->fan[k].x;
            w.y = d->fan[k].y;
            w.z = d->fan[k].z;
            mat4_transform_vec4(&w, &vp, &w);
            z = (w.y * -136.0f / w.w + 2048.0f - 1912.0f) / 512.0f;
            x = (w.x * 240.0f / w.w + 2048.0f - 1808.0f) / 512.0f;
            d->fan[k].u = x;
            d->fan[k].v = z;
        }
        memcpy(d->fan_buf[buf], d->fan, sizeof(d->fan));
        for (m = 0; m < 16; m++) {
            sc = m / 16.0f * 1.5f + 1.0f;
            list = dl_ambient_light(list, (unsigned int)(out * ((1.0f - m / 16.0f) * 255.0f * in)) << 24 | 0xFFFFFF);
            mtx_push();
            dd_scale3(sc, sc, 0.0f);
            list = draw_array(list, 5, 0x19F, 34, 0, d->fan_buf[buf]);
            mtx_pop();
        }
        mtx_pop();
        dl_set_list(list);
    }

    /* the diamond: three nested outlines, opaque in the middle */
    if (phase == 1) {
        {
            unsigned char *c = (unsigned char *)color;
            int n = 12;

            if (c)
                do {
                    *c++ = 0;
                } while (--n);
        }
        color[0] = 0x5064C8;
        color[2] = 0x5064C8;
        color[1] = 0xFF5064C8;
        for (l = 0; l < 3; l++) {
            sc = ((l - 1.0f) * 0.1f + 1.0f) * d->dia_scale;
            x = sc * 2.3889616f;
            z = sc * 3.406471f;
            d->dia[0][l].color = color[l];
            d->dia[0][l].x = -x;
            d->dia[0][l].y = 0.0f;
            d->dia[0][l].z = z;
            d->dia[1][l].color = color[l];
            d->dia[1][l].x = x;
            d->dia[1][l].y = 0.0f;
            d->dia[1][l].z = z;
            d->dia[2][l].color = color[l];
            d->dia[2][l].x = x;
            d->dia[2][l].y = 0.0f;
            d->dia[2][l].z = -z;
            d->dia[3][l].color = color[l];
            d->dia[3][l].x = -x;
            d->dia[3][l].y = 0.0f;
            d->dia[3][l].z = -z;
            d->dia[4][l] = d->dia[0][l];
        }
        d->dia_idx[0] = 0;
        d->dia_idx[1] = 1;
        d->dia_idx[2] = 3;
        d->dia_idx[3] = 4;
        d->dia_idx[4] = 6;
        d->dia_idx[5] = 7;
        d->dia_idx[6] = 9;
        d->dia_idx[7] = 10;
        d->dia_idx[8] = 12;
        d->dia_idx[9] = 13;
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 0, 1, 1, 0, 0, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        mtx_set_mode(2);
        mtx_push();
        mtx_load(&node);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i = 0; i < 2; i++)
            list = draw_array(list, 4, 0x99C, 10, d->dia_idx, &d->dia[0][i]);
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
