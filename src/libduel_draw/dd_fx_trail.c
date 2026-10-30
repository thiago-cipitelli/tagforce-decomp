// @range 0x088395D4-0x0883AB6C
// @asm 0x0883970C-0x0883AB6C
// @rodata 0x088707A4-0x088707D0
// @data 0x08876BB4-0x08876BD0
/* The charge trail of effects 13-23: three ribbons that follow a point of the scene along a spline
   of its last 32 positions, a glow at the head and, in the second variant (owners of kind 19),
   sparks and a ring that refracts the screen behind it (task of 0x7F70 bytes). */
#include "dd_fx.h"

int dd_trail_update(int phase, DdTrailFx *d);

/* zero vector and w = 1 point: local templates of dd_trail_update (kept in asm) */
#pragma explicit_zero_data on
ScePspFVector3 dd_trail_zero = { 0.0f, 0.0f, 0.0f };
#pragma explicit_zero_data off
ScePspFVector4 dd_trail_w = { 0.0f, 0.0f, 0.0f, 1.0f };

/* grow time and length of the trail by effect id */
void dd_fxc_kind(int id, int *grow, int *end)
{
    switch (id) {
    case 13:
    case 14:
        *grow = 13;
        break;
    case 15:
        *grow = 10;
        *end = 22;
        return;
    case 16:
        *grow = 12;
        break;
    case 17:
        *grow = 10;
        break;
    case 18:
    case 19:
    case 20:
        *grow = 20;
        break;
    case 21:
    case 22:
    case 23:
        *grow = 24;
        break;
    default:
        *grow = 0;
        *end = 0;
        return;
    }
    *end = 20;
}

int dd_fx13_start(DdFxSlot *s, int a, int b)
{
    int task = func_0882623C(dd_trail_update, a, b, sizeof(DdTrailFx));
    DdTrailFx *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
        d->variant = 0;
    }
    return task;
}

int dd_fx13_start_b(DdFxSlot *s, int a, int b)
{
    int task = dd_fx13_start(s, a, b);
    DdTrailFx *d;

    if (task != -1) {
        d = func_08826440(task);
        d->variant = 1;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 5,2 KB com quadro de 448 bytes: alocação de registradores e da pilha diferem */
int dd_trail_update(int phase, DdTrailFx *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    int buf = d->frame % 2;
    ScePspFMatrix4 bill;
    ScePspFMatrix4 vp;
    ScePspFVector4 bl[3][2];
    ScePspFVector4 edge[2];
    ScePspFVector4 w;
    ScePspFVector3 head;
    ScePspFVector3 ring;
    ScePspFVector3 pos;
    DdRandSeed *r;
    DdTrailSpark *sp;
    DdCVtx *v;
    void *list;
    unsigned int rgb;
    unsigned int alpha;
    float in;
    float out;
    float k;
    float ang;
    float sc;
    float e;
    int i;
    int j;
    int n;

    func_08827738();
    if (phase == 1) {
        dd_fxc_kind(s->id, &d->grow_end, &d->end);
        switch (s->id) {
        case 13:
        case 16:
            d->se_a = 0x6235;
            d->se_b = 0x6232;
            d->se_at = 4;
            break;
        case 14:
            d->se_a = 0x6235;
            d->se_b = 0x6232;
            d->se_at = 6;
            break;
        case 15:
            d->se_a = 0x6235;
            d->se_b = 0x6232;
            d->se_at = 5;
            break;
        case 17:
            d->se_a = 0x6235;
            d->se_b = 0x6232;
            d->se_at = 8;
            break;
        case 18:
        case 19:
        case 20:
            d->se_a = 0x623D;
            d->se_b = -1;
            d->se_at = 3;
            break;
        case 21:
        case 22:
        case 23:
            d->se_a = 0x6235;
            d->se_b = -1;
            d->se_at = 7;
            break;
        default:
            d->se_a = -1;
            d->se_b = -1;
            break;
        }
    }
    in = func_08827A2C(d->t, 4, 1);
    out = 1.0f - func_088279A4(d->t - (d->end - 8), 8);
    func_08827F14(0x25, &head);
    func_08827ED4(0x17, &ring);
    func_08827258(&bill);
    func_08827F68(d->se_a, d->t, d->se_at, &d->se_done, 1);
    func_08827F68(d->se_b, d->t, d->se_at, &d->se_done, 2);

    /* the ribbons */
    if (phase == 1) {
        if (d->variant == 0) {
            d->r = 255;
            d->g = 230;
            d->b = 100;
            d->a = 128;
        } else {
            d->r = 100;
            d->g = 255;
            d->b = 230;
            d->a = 128;
        }
        for (i = 0; i < 3; i++) {
            ang = 3.1415927f * i / 3.0f;
            vec4_set(&d->blade[i][0], sinf(ang) * 10.0f, cosf(ang) * 10.0f, 0.0f, 1.0f);
            ang += 3.1415927f;
            vec4_set(&d->blade[i][1], sinf(ang) * 10.0f, cosf(ang) * 10.0f, 0.0f, 1.0f);
        }
        for (i = 0; i < 48; i++) {
            d->idx[i * 2] = i * 3;
            d->idx[i * 2 + 1] = i * 3 + 1;
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 0, 1, 1, 0, 0, 0, 0);
        list = func_0882698C(list, 1, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        /* push the head into the history */
        if (d->npts < 32)
            d->npts++;
        for (i = d->npts - 1; i > 0; i--) {
            d->pts[i] = d->pts[i - 1];
            for (j = 0; j < 8; j++)
                d->curve[i][j] = d->curve[i - 1][j];
        }
        vec4_set(&d->pts[0], head.x, head.y, head.z, 1.0f);
        spline_init(d->coef, d->pts, d->npts);
        for (j = 0; j < 8; j++)
            spline_eval(j / 8.0f, &d->curve[0][j], d->pts, d->coef, d->npts);
        rgb = d->r | d->b << 16 | d->g << 8;
        for (i = 0; i < 3; i++)
            for (j = 0; j < 2; j++)
                mat4_transform_vec4(&bl[i][j], &bill, &d->blade[i][j]);
        /* build the ribbons, thinner and fainter toward the tail */
        d->nsteps = 0;
        for (i = 0; i < d->npts; i++) {
            for (j = 0; j < 8; j++) {
                k = 1.0f - d->nsteps / 47.0f;
                alpha = (unsigned int)d->a * (k * 0.5f + 0.5f);
                for (n = 0; n < 3; n++) {
                    vec4_scale(&edge[0], &bl[n][0], k);
                    vec4_add(&edge[0], &edge[0], &d->curve[i][j]);
                    vec4_scale(&edge[1], &bl[n][1], k);
                    vec4_add(&edge[1], &edge[1], &d->curve[i][j]);
                    v = d->strip[n][d->nsteps];
                    v[0].color = rgb;
                    v[0].x = edge[0].x;
                    v[0].y = edge[0].y;
                    v[0].z = edge[0].z;
                    v[1].color = d->r | d->g << 8 | alpha << 24 | d->b << 16;
                    v[1].x = d->curve[i][j].x;
                    v[1].y = d->curve[i][j].y;
                    v[1].z = d->curve[i][j].z;
                    v[2].color = rgb;
                    v[2].x = edge[1].x;
                    v[2].y = edge[1].y;
                    v[2].z = edge[1].z;
                }
                if (++d->nsteps >= 48)
                    goto built;
            }
        }
    built:
        memcpy(d->strip_buf[buf], d->strip, sizeof(d->strip));
        if (d->nsteps > 1) {
            list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
            for (n = 0; n < 3; n++)
                for (j = 0; j < 2; j++)
                    list = draw_array(list, 4, 0x99C, d->nsteps * 2, d->idx, &d->strip_buf[buf][n][0][j]);
        }
        mtx_pop();
        dl_set_list(list);
    }

    /* the glow at the head */
    if (phase == 1) {
        if (d->variant == 0) {
            d->r2 = 255;
            d->g2 = 230;
            d->b2 = 100;
            d->a2 = 255;
        } else {
            d->r2 = 100;
            d->g2 = 255;
            d->b2 = 230;
            d->a2 = 255;
        }
        rgb = d->r2 | d->b2 << 16 | d->g2 << 8;
        d->glow[0].u = 0.5f;
        d->glow[0].v = 0.5f;
        d->glow[0].color = d->r2 | d->g2 << 8 | d->a2 << 24 | d->b2 << 16;
        d->glow[0].x = 0.0f;
        d->glow[0].y = 0.0f;
        d->glow[0].z = 0.0f;
        d->glow[1].u = 0.0f;
        d->glow[1].v = 0.0f;
        d->glow[1].color = rgb;
        d->glow[1].x = -20.0f;
        d->glow[1].y = 20.0f;
        d->glow[1].z = 0.0f;
        d->glow[2].u = 1.0f;
        d->glow[2].v = 0.0f;
        d->glow[2].color = rgb;
        d->glow[2].x = 20.0f;
        d->glow[2].y = 20.0f;
        d->glow[2].z = 0.0f;
        d->glow[3].u = 1.0f;
        d->glow[3].v = 1.0f;
        d->glow[3].color = rgb;
        d->glow[3].x = 20.0f;
        d->glow[3].y = -20.0f;
        d->glow[3].z = 0.0f;
        d->glow[4].u = 0.0f;
        d->glow[4].v = 1.0f;
        d->glow[4].color = rgb;
        d->glow[4].x = -20.0f;
        d->glow[4].y = -20.0f;
        d->glow[4].z = 0.0f;
        d->glow[5] = d->glow[1];
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
        list = func_0882698C(list, 1, 1);
        list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
        list = func_08826A64(list, 3);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&head);
        mtx_mult(&bill);
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        list = draw_array(list, 5, 0x19F, 6, 0, d->glow);
        mtx_pop();
        dl_set_list(list);
    }

    if (d->variant != 0) {
        /* sparks shed along the newest segment */
        if (phase == 1) {
            for (i = 0; i < 4; i++) {
                d->sq[i][0].u = i % 4 * 0.25f;
                d->sq[i][0].v = i / 4 * 0.5f;
                d->sq[i][0].x = -3.0f;
                d->sq[i][0].y = 3.0f;
                d->sq[i][0].z = 0.0f;
                d->sq[i][1].u = i % 4 * 0.25f + 0.25f;
                d->sq[i][1].v = i / 4 * 0.5f + 0.5f;
                d->sq[i][1].x = 3.0f;
                d->sq[i][1].y = -3.0f;
                d->sq[i][1].z = 0.0f;
            }
        }
        if (d->grow == d->grow_end && d->npts > 0) {
            sp = &d->spark[d->spark_next];
            sp->flags |= 1;
            sp->t = 0;
            for (j = 0; j < 8; j++) {
                sp->p[j].frame = DD_RAND(r) % 4;
                vec3_set(&sp->p[j].pos, d->curve[0][j].x, d->curve[0][j].y, d->curve[0][j].z);
                func_08827BA4(5.0f, &sp->p[j].vel);
            }
            if (d->spark_next < 15)
                d->spark_next++;
            else
                d->spark_next = 0;
        }
        if (d->grow == d->grow_end) {
            list = dl_select_layer(7, 0);
            list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
            list = func_0882698C(list, 1, 1);
            list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
            list = func_08826A64(list, 14);
            list = dl_color(list, 0xFFE6FF64);
            mtx_set_mode(2);
            mtx_push();
            mtx_identity();
            for (i = 0; i < 16; i++) {
                sp = &d->spark[i];
                if (!(sp->flags & 1))
                    continue;
                e = func_08827A2C(sp->t, 16, 1);
                list = dl_ambient_light(list, (unsigned int)(out * in * func_08827AD4(sp->t, 16, 1) * 255.0f) << 24
                                                  | 0xFFFFFF);
                for (j = 0; j < 8; j++) {
                    vec3_scale(&pos, &sp->p[j].vel, e * 0.3f + 0.7f);
                    vec3_add(&pos, &pos, &sp->p[j].pos);
                    mtx_push();
                    mtx_translate(&pos);
                    mtx_mult(&bill);
                    list = draw_array(list, 6, 0x183, 2, 0, d->sq[sp->p[j].frame]);
                    mtx_pop();
                }
                if (sp->t < 15)
                    sp->t++;
                else
                    sp->flags &= ~1;
            }
            mtx_pop();
            dl_set_list(list);
        }

        /* the ring: a fan textured with the screen behind it, in three growing copies */
        if (phase == 1) {
            d->fan[0].u = 0.0f;
            d->fan[0].v = 0.0f;
            d->fan[0].color = 0x64E6FF64;
            d->fan[0].x = 0.0f;
            d->fan[0].y = 0.0f;
            d->fan[0].z = 0.0f;
            for (i = 0; i < 33; i++) {
                d->fan[i + 1].u = 0.0f;
                d->fan[i + 1].v = 0.0f;
                d->fan[i + 1].color = 0x00E6FF64;
                ang = 6.2831855f * i / 32.0f;
                d->fan[i + 1].x = sinf(ang) * 30.0f;
                d->fan[i + 1].y = cosf(ang) * 30.0f;
                d->fan[i + 1].z = 0.0f;
            }
        }
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, -1);
        list = func_08826C80(list);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&ring);
        mtx_mult(&bill);
        func_08827408(&vp);
        for (i = 0; i < 34; i++) {
            w = dd_trail_w;
            w.x = d->fan[i].x;
            w.y = d->fan[i].y;
            w.z = d->fan[i].z;
            mat4_transform_vec4(&w, &vp, &w);
            d->fan[i].u = (w.x * 240.0f / w.w + 2048.0f - 1808.0f) / 512.0f;
            d->fan[i].v = (w.y * -136.0f / w.w + 2048.0f - 1912.0f) / 512.0f;
        }
        memcpy(d->fan_buf[buf], d->fan, sizeof(d->fan));
        for (i = 0; i < 3; i++) {
            sc = i / 3.0f * 0.3f + 1.0f;
            list = dl_ambient_light(list, (unsigned int)(out * (1.0f - i / 3.0f) * 255.0f *
                                                         func_08827A2C(d->grow, d->grow_end, 1)) << 24 | 0xFFFFFF);
            mtx_push();
            func_088272BC(sc, sc, 0.0f);
            list = draw_array(list, 5, 0x19F, 34, 0, d->fan_buf[buf]);
            mtx_pop();
        }
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
