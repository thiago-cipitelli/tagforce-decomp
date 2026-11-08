// @range 0x08838068-0x08838CDC
// @data 0x08876B9C-0x08876BB4
/* The ring of flames: up to 20 flames placed on a tilted circle in 3D, each shedding sprite sparks;
   the full ring (count 20) lights one flame every 16 frames, plays a sound and ends in a white flash
   (task of 0x4944 bytes). */
#include "dd_fx.h"

int dd_ring_update(int phase, DdRingFx *d);

/* camera eye and up vector: local templates of dd_ring_update */
ScePspFVector3 dd_ring_eye = { 0.0f, 0.0f, 500.0f };
ScePspFVector3 dd_ring_up = { 0.0f, 1.0f, 0.0f };

/* the moving part of a spark (from DdRingSpark.frame) */
typedef struct DdRingSparkMove {
    int frame;
    float vy;
    ScePspFVector3 pos;
} DdRingSparkMove;

#define DD_BIT(f, m) ((f) & (m) ? 1 : 0)

/* the full ring is done when its flash has held for 30 frames, the others at their end time */
int dd_ring_done(int task)
{
    DdRingFx *d = dd_task_mem(task);

    if (d == 0)
        return 1;
    if (d->count < 20)
        return d->t >= d->end;
    return d->flash_end >= 30;
}

int dd_ring_start(int count, int a, int b)
{
    int task = dd_task_create(dd_ring_update, a, b, sizeof(DdRingFx));
    DdRingFx *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->count = count;
        switch (count) {
        case 20:
            d->end = -1;
            d->tick = 16;
            break;
        default:
            d->end = 100;
            break;
        }
    }
    return task;
}

int dd_ring_update(int phase, DdRingFx *d)
{
    float place;
    DdRingFlame *fl;
    int step = dd_anim_step();
    ScePspFVector3 eye = dd_ring_eye;
    ScePspFVector3 at = { 0 };
    ScePspFVector3 up = dd_ring_up;
    ScePspFMatrix3 rot;
    ScePspFVector3 scale;
    ScePspFVector3 pos;
    ScePspFMatrix4 bill __attribute__((aligned(16)));
    DdRandSeed *r;
    DdRingFlame *f;
    DdRingSpark *p;
    DdRingSparkMove *q;
    void *list;
    float in;
    float out;
    float a;
    float sc;
    float ang;
    float dist;
    float yc;
    float ang2;
    float rad;
    int i;
    int j;
    int n;

    dd_cam_begin(0.69813174f, 1.7647059f, 1.0f, 10000.0f, 6, 314, 132, &eye, &at, &up);
    if (d->t < 420)
        in = dd_ease_lin(d->t, 16);
    else
        in = 0.0f;
    out = dd_ease_lin_inv(d->out, 16);
    dd_mtx_billboard_view(&bill);
    if (d->count == 20) {
        if (DD_BIT(d->flags, 1) == 0 && d->t >= 360) {
            dd_field_set(0x169C);
            d->flags |= 1;
        }
        if (DD_BIT(d->flags, 2) == 0 && d->t >= 510)
            d->flags |= 2;
    }

    /* dim the field */
    list = dl_select_layer(2, 0);
    list = dd_tex_enable(list, 0, 1, 1, 0, 0, 1, 1);
    list = dd_depth_enable(list, 0, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, -1);
    list = dl_color(list, (unsigned int)(128.0f * in * out) << 24);
    list = draw_set_offsets(list, 0, 0, 480, 272);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    dl_set_list(list);

    if (phase == 1) {
        /* the 8 frames of the flame sheet (4 x 2), 20 units square */
        float r10 = 10.0f;
        float du = 0.25f;
        float dv = 0.5f;
        float u;
        float v;

        for (i = 0; i < 8; i++) {
            u = i % 4 * du;
            v = i / 4 * dv;
            d->quad[i][0].u = u;
            d->quad[i][0].v = v;
            d->quad[i][0].x = -r10;
            d->quad[i][0].y = r10;
            d->quad[i][0].z = 0.0f;
            d->quad[i][1].u = u + du;
            d->quad[i][1].v = v + dv;
            d->quad[i][1].x = r10;
            d->quad[i][1].y = -r10;
            d->quad[i][1].z = 0.0f;
        }
        /* 20 places on a circle of radius 150 tilted by 60 degrees */
        mat3_identity(&rot);
        mat3_rot_x(&rot, &rot, 1.0471976f);
        for (i = 0; i < 20; i++) {
            place = 6.2831855f * i / 20.0f;
            f = &d->flame[i];
            vec3_set(&f->pos, sinf(place) * 150.0f, cosf(place) * 150.0f, 0.0f);
            mat3_transform_vec3(&f->pos, &rot, &f->pos);
        }
        if (d->count < 20) {
            for (n = 0; n < d->count - 1; n++)
                d->flame[n].flags |= 1;
        }
    }
    /* light the next flame */
    if (d->count < 20) {
        if (d->shown < 1 && d->t >= 40) {
            f = &d->flame[d->count - 1];
            f->flags |= 1;
            d->shown++;
        }
    } else if (d->shown < 20 && d->t >= 40) {
        d->tick += step;
        if (d->tick >= 16) {
            f = &d->flame[d->shown];
            f->flags |= 1;
            d->tick = 0;
            d->shown++;
        }
    }

    list = dl_select_layer(6, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 0, 1);
    list = dd_blend_enable(list, 1, 0, 6, 10, 0, -1);
    list = dd_tex_pack(list, 5);
    list = dl_color(list, 0xFF3050FF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 20; i++) {
        fl = &d->flame[i];
        if (!DD_BIT(fl->flags, 1) == 1)
            continue;
        a = out * dd_ease_lin(fl->t, 16);
        sc = 1.0f;
        if (DD_BIT(d->flags, 2) == 1)
            sc = 1.5f;
        vec3_set(&scale, sc, sc, 0.0f);
        /* `step` new sparks */
        for (j = 0; j < step; j++) {
            p = &fl->p[fl->next];
            p->flags |= 1;
            p->t = 0;
            ang = 6.2831855f * (DD_RAND(r) / 32767.0f);
            rad = DD_RAND(r) / 32767.0f;
            dist = rad * 5.0f + 5.0f;
            ang2 = 1.5707964f * rad;
            q = (DdRingSparkMove *)&p->frame;
            yc = cosf(ang2) * -7.5f;
            q->frame = DD_RAND(r) % 8;
            q->vy = cosf(ang2) * 0.95f + 0.35f;
            vec3_set(&q->pos, dist * sinf(ang), yc, dist * cosf(ang));
            if (fl->next < 31)
                fl->next++;
            else
                fl->next = 0;
        }
        /* the sparks rise and fade */
        for (j = 0; j < 32; j++) {
            p = &fl->p[j];
            if (!DD_BIT(p->flags, 1) == 1)
                continue;
            list = dl_ambient_light(list, (unsigned int)(a * 255.0f * dd_ease_lin(p->t, 4) *
                                                         dd_ease_cos(p->t - 16, 16, 1)) << 24 | 0xFFFFFF);
            q = (DdRingSparkMove *)&p->frame;
            vec3_set(&pos, p->pos.x, p->pos.y + p->vy * p->t, p->pos.z);
            vec3_scale(&pos, &pos, sc);
            vec3_add(&pos, &pos, &fl->pos);
            mtx_push();
            mtx_translate(&pos);
            mtx_mult(&bill);
            mtx_scale3(&scale);
            list = draw_array(list, 6, 0x183, 2, 0, d->quad[q->frame]);
            mtx_pop();
            p->t += step;
            if (p->t >= 32)
                p->flags &= ~1;
        }
        fl->t += step;
    }
    mtx_pop();
    dl_set_list(list);

    /* white flash of the full ring */
    if (DD_BIT(d->flags, 2) == 1) {
        list = dl_select_layer(6, 0);
        a = dd_ease_cos(d->flash, 30, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_tex_enable(list, 0, 1, 1, 0, 0, 1, 1);
        list = dd_blend_enable(list, 1, 0, 2, 3, 0, -1);
        list = dl_color(list, (unsigned int)(a * 200.0f) << 24 | 0xFFFFFF);
        list = draw_set_offsets(list, 0, 0, 480, 272);
        list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
        dl_set_list(list);
        d->flash += step;
        if (d->flash > 30)
            d->flash = 30;
        if (d->flash >= 30) {
            d->flash_end += step;
            if (d->flash_end > 30)
                d->flash_end = 30;
        }
    }
    dd_cam_end(6);
    if (d->end < 0)
        d->t += step;
    else if (d->t < d->end)
        d->t += step;
    else
        d->out += step;
    return d->out < 16 ? 2 : 4;
}
