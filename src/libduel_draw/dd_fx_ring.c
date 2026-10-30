// @range 0x08838068-0x08838CDC
// @asm 0x0883813C-0x08838CDC
// @data 0x08876B9C-0x08876BB4
/* The ring of flames: up to 20 flames placed on a tilted circle in 3D, each shedding sprite sparks;
   the full ring (count 20) lights one flame every 16 frames, plays a sound and ends in a white flash
   (task of 0x4944 bytes). */
#include "dd_fx.h"

int dd_ring_update(int phase, DdRingFx *d);

/* camera eye and up vector: local templates of dd_ring_update (kept in asm) */
ScePspFVector3 dd_ring_eye = { 0.0f, 0.0f, 500.0f };
ScePspFVector3 dd_ring_up = { 0.0f, 1.0f, 0.0f };

/* the full ring is done when its flash has held for 30 frames, the others at their end time */
int dd_ring_done(int task)
{
    DdRingFx *d = func_08826440(task);

    if (d == 0)
        return 1;
    if (d->count < 20)
        return d->t >= d->end;
    return d->flash_end >= 30;
}

int dd_ring_start(int count, int a, int b)
{
    int task = func_0882623C(dd_ring_update, a, b, sizeof(DdRingFx));
    DdRingFx *d;

    if (task != -1) {
        d = func_08826440(task);
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

#ifdef NON_MATCHING /* função de 2,9 KB: quadro 16 bytes menor, testes de bit materializados e registradores diferem */
int dd_ring_update(int phase, DdRingFx *d)
{
    int step = func_08828290();
    ScePspFVector3 eye = dd_ring_eye;
    ScePspFVector3 at = { 0 };
    ScePspFVector3 up = dd_ring_up;
    ScePspFMatrix4 bill;
    ScePspFMatrix3 rot;
    ScePspFVector3 scale;
    ScePspFVector3 pos;
    DdRandSeed *r;
    DdRingFlame *f;
    DdRingSpark *p;
    void *list;
    float in;
    float out;
    float a;
    float sc;
    float ang;
    float dist;
    float rad;
    float yc;
    int i;
    int j;
    int k;

    func_08826EE4(0.69813174f, 1.7647059f, 1.0f, 10000.0f, 6, 314, 132, &eye, &at, &up);
    if (d->t < 420)
        in = func_088279A4(d->t, 16);
    else
        in = 0.0f;
    out = func_088279D8(d->out, 16);
    func_08827214(&bill);
    if (d->count == 20) {
        if (!(d->flags & 1) && d->t >= 360) {
            func_0880773C(0x169C);
            d->flags |= 1;
        }
        if (!(d->flags & 2) && d->t >= 510)
            d->flags |= 2;
    }

    /* dim the field */
    list = dl_select_layer(2, 0);
    list = func_088266D8(list, 0, 1, 1, 0, 0, 1, 1);
    list = func_0882698C(list, 0, 1);
    list = func_088269D8(list, 1, 0, 2, 3, 0, -1);
    list = dl_color(list, (unsigned int)(128.0f * in * out) << 24);
    list = draw_set_offsets(list, 0, 0, 480, 272);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    dl_set_list(list);

    if (phase == 1) {
        /* the 8 frames of the flame sheet (4 x 2) */
        for (i = 0; i < 8; i++) {
            d->quad[i][0].u = i % 4 * 0.25f;
            d->quad[i][0].v = i / 4 * 0.5f;
            d->quad[i][0].x = -7.0f;
            d->quad[i][0].y = 10.0f;
            d->quad[i][0].z = 0.0f;
            d->quad[i][1].u = i % 4 * 0.25f + 0.25f;
            d->quad[i][1].v = i / 4 * 0.5f + 0.5f;
            d->quad[i][1].x = 10.0f;
            d->quad[i][1].y = -7.0f;
            d->quad[i][1].z = 0.0f;
        }
        /* 20 places on a circle of radius 150 tilted by 60 degrees */
        mat3_identity(&rot);
        mat3_rot_x(&rot, &rot, 1.0471976f);
        for (i = 0; i < 20; i++) {
            ang = 6.2831855f * i / 20.0f;
            vec3_set(&d->flame[i].pos, sinf(ang) * 150.0f, cosf(ang) * 150.0f, 0.0f);
            mat3_transform_vec3(&d->flame[i].pos, &rot, &d->flame[i].pos);
        }
        if (d->count < 20) {
            for (i = 0; i < d->count - 1; i++)
                d->flame[i].flags |= 1;
        }
    }
    /* light the next flame */
    if (d->count < 20) {
        if (d->shown < 1 && d->t >= 40) {
            d->flame[d->count - 1].flags |= 1;
            d->shown++;
        }
    } else if (d->shown < 20 && d->t >= 40) {
        d->tick += step;
        if (d->tick >= 16) {
            d->flame[d->shown].flags |= 1;
            d->tick = 0;
            d->shown++;
        }
    }

    list = dl_select_layer(6, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
    list = func_0882698C(list, 0, 1);
    list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
    list = func_08826A64(list, 5);
    list = dl_color(list, 0xFF3050FF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 20; i++) {
        f = &d->flame[i];
        if (!(f->flags & 1))
            continue;
        a = out * func_088279A4(f->t, 16);
        sc = 1.0f;
        if (d->flags & 2)
            sc = 1.5f;
        vec3_set(&scale, sc, sc, 0.0f);
        /* `step` new sparks */
        for (j = 0; j < step; j++) {
            p = &f->p[f->next];
            p->flags |= 1;
            p->t = 0;
            ang = 6.2831855f * (DD_RAND(r) / 32767.0f);
            rad = DD_RAND(r) / 32767.0f;
            dist = rad * 5.0f + 5.0f;
            rad *= 1.5707964f;
            yc = cosf(rad);
            p->frame = DD_RAND(r) % 8;
            p->vy = cosf(rad) * 0.95f + 0.35f;
            vec3_set(&p->pos, dist * sinf(ang), yc * -7.5f, dist * cosf(ang));
            if (f->next < 31)
                f->next++;
            else
                f->next = 0;
        }
        /* the sparks rise and fade */
        for (k = 0, p = f->p; k < 32; k++, p++) {
            if (!(p->flags & 1))
                continue;
            list = dl_ambient_light(list, (unsigned int)(a * 255.0f * func_088279A4(p->t, 4) *
                                                         func_08827AD4(p->t - 16, 16, 1)) << 24 | 0xFFFFFF);
            vec3_set(&pos, p->pos.x, p->pos.y + p->vy * p->t, p->pos.z);
            vec3_scale(&pos, &pos, sc);
            vec3_add(&pos, &pos, &f->pos);
            mtx_push();
            mtx_translate(&pos);
            mtx_mult(&bill);
            mtx_scale3(&scale);
            list = draw_array(list, 6, 0x183, 2, 0, d->quad[p->frame]);
            mtx_pop();
            p->t += step;
            if (p->t >= 32)
                p->flags &= ~1;
        }
        f->t += step;
    }
    mtx_pop();
    dl_set_list(list);

    /* white flash of the full ring */
    if (d->flags & 2) {
        list = dl_select_layer(6, 0);
        a = func_08827AD4(d->flash, 30, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088266D8(list, 0, 1, 1, 0, 0, 1, 1);
        list = func_088269D8(list, 1, 0, 2, 3, 0, -1);
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
    func_08826FEC(6);
    if (d->end < 0)
        d->t += step;
    else if (d->t < d->end)
        d->t += step;
    else
        d->out += step;
    return d->out < 16 ? 2 : 4;
}
#endif
