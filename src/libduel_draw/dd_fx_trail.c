// @range 0x088395D4-0x0883AB6C
// @asm 0x0883970C-0x0883AB6C
// @rodata 0x088707A4-0x088707D0
// @data 0x08876BB4-0x08876BD0
/* The charge trail of effects 13-23: three ribbons that follow a point of the scene along a spline
   of its last 32 positions, a glow at the head and, in the second variant (owners of kind 19),
   sparks and a ring that refracts the screen behind it (task of 0x7F70 bytes). */
#include "dd_fx.h"

#define DD_BIT(f, m) ((f) & (m) ? 1 : 0)

int dd_trail_update(int phase, DdTrailFx *d);

typedef struct DdTrailPart {           /* DdTrailSpark.p[] */
    int frame;
    ScePspFVector3 pos;
    ScePspFVector3 vel;
} DdTrailPart;

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
    int task = dd_task_create(dd_trail_update, a, b, sizeof(DdTrailFx));
    DdTrailFx *d;

    if (task != -1) {
        d = dd_task_mem(task);
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
        d = dd_task_mem(task);
        d->variant = 1;
    }
    return task;
}

#ifdef NON_MATCHING /* 16 palavras (2026-11-08): forma e quadro iguais ao original; faltam os $s do laço de spline_eval
                      (contador s0/ponteiro s1 trocados) e as posições de derrame de list e do contador externo das fitas
                      (0x60/0x64); os contadores nN têm um nome por laço para os $s (busca automática) */
int dd_trail_update(int phase, DdTrailFx *d)
{
    int n39;
    int n33;
    int n29;
    int n27;
    int n19;
    int n9;
    int n5;
    DdFxSlot *s = d->slot;
    int q;
    int q2;
    int run = s->param;
    int buf = d->frame % 2;
    ScePspFVector3 head;
    ScePspFVector3 ring;
    ScePspFVector3 pos;
    ScePspFMatrix4 bill __attribute__((aligned(16)));
    ScePspFVector4 bl[3][2] __attribute__((aligned(16)));
    ScePspFVector4 edge[2] __attribute__((aligned(16)));
    ScePspFMatrix4 vp __attribute__((aligned(16)));
    ScePspFVector4 w __attribute__((aligned(16)));
    DdRandSeed *r;
    DdTrailSpark *sp;
    DdTrailPart *pp;
    DdCVtx *v0;
    DdCVtx *v1;
    DdCVtx *v2;
    ScePspFVector4 *cv;
    void *list;
    unsigned int rgb;
    unsigned int alpha;
    unsigned int c;
    float in;
    float out;
    float k;
    float ang;
    float sc;
    float e;
    float g;
    float fv;
    float du;
    float dv;
    float w3;
    float tu;
    float tv;
    int i;
    int j;
    int n;
    int m;
    int n26;

    dd_mtx_push_saved();
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
    in = dd_ease_sin(d->t, 4, 1);
    out = 1.0f - dd_ease_lin(d->t - (d->end - 8), 8);
    dd_slot_pos2(0x25, &head);
    dd_slot_pos(0x17, &ring);
    dd_mtx_billboard_cam(&bill);
    dd_cue_se(d->se_a, d->t, d->se_at, &d->se_done, 1);
    dd_cue_se(d->se_b, d->t, d->se_at, &d->se_done, 2);

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
        list = dd_tex_enable(list, 0, 1, 1, 0, 0, 0, 0);
        list = dd_depth_enable(list, 1, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        /* push the head into the history */
        if (d->npts < 32)
            d->npts++;
        for (n5 = d->npts - 1; n5 > 0; n5--) {
            d->pts[n5] = d->pts[n5 - 1];
            for (j = 0; j < 8; j++)
                d->curve[n5][j] = d->curve[n5 - 1][j];
        }
        vec4_set(&d->pts[0], head.x, head.y, head.z, 1.0f);
        spline_init(d->coef, d->pts, d->npts);
        for (n9 = 0; n9 < 8; n9++)
            spline_eval(n9 / 8.0f, &d->curve[0][n9], d->pts, d->coef, d->npts);
        rgb = d->r | (d->b << 16 | d->g << 8);
        for (i = 0; i < 3; i++)
            for (j = 0; j < 2; j++)
                mat4_transform_vec4(&bl[i][j], &bill, &d->blade[i][j]);
        /* build the ribbons, thinner and fainter toward the tail */
        d->nsteps = 0;
        for (i = 0; i < d->npts; i++) {
            for (j = 0; j < 8; j++) {
                cv = &d->curve[i][j];
                k = 1.0f - d->nsteps / 47.0f;
                alpha = (unsigned int)d->a * (k * 0.5f + 0.5f);
                c = d->r | (d->g << 8 | (alpha << 24 | d->b << 16));
                for (n19 = 0; n19 < 3; n19++) {
                    v0 = &d->strip[n19][d->nsteps][0];
                    v1 = &d->strip[n19][d->nsteps][1];
                    v2 = &d->strip[n19][d->nsteps][2];
                    for (m = 0; m < 2; m++) {
                        vec4_scale(&edge[m], &bl[n19][m], k);
                        vec4_add(&edge[m], &edge[m], cv);
                    }
                    v0->color = rgb;
                    v0->x = edge[0].x;
                    v0->y = edge[0].y;
                    v0->z = edge[0].z;
                    v1->color = c;
                    v1->x = cv->x;
                    v1->y = cv->y;
                    v1->z = cv->z;
                    v2->color = rgb;
                    v2->x = edge[1].x;
                    v2->y = edge[1].y;
                    v2->z = edge[1].z;
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
                for (n26 = 0; n26 < 2; n26++)
                    list = draw_array(list, 4, 0x99C, d->nsteps * 2, d->idx, &d->strip_buf[buf][n][0][n26]);
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
        c = d->r2 | (d->g2 << 8 | (d->a2 << 24 | d->b2 << 16));
        rgb = d->r2 | (d->b2 << 16 | d->g2 << 8);
        g = 20.0f;
        d->glow[0].u = 0.5f;
        d->glow[0].v = 0.5f;
        d->glow[0].color = c;
        d->glow[0].x = 0.0f;
        d->glow[0].y = 0.0f;
        d->glow[0].z = 0.0f;
        d->glow[1].u = 0.0f;
        d->glow[1].v = 0.0f;
        d->glow[1].color = rgb;
        d->glow[1].x = -g;
        d->glow[1].y = g;
        d->glow[1].z = 0.0f;
        d->glow[2].u = 1.0f;
        d->glow[2].v = 0.0f;
        d->glow[2].color = rgb;
        d->glow[2].x = g;
        d->glow[2].y = g;
        d->glow[2].z = 0.0f;
        d->glow[3].u = 1.0f;
        d->glow[3].v = 1.0f;
        d->glow[3].color = rgb;
        d->glow[3].x = g;
        d->glow[3].y = -g;
        d->glow[3].z = 0.0f;
        d->glow[4].u = 0.0f;
        d->glow[4].v = 1.0f;
        d->glow[4].color = rgb;
        d->glow[4].x = -g;
        d->glow[4].y = -g;
        d->glow[4].z = 0.0f;
        d->glow[5] = d->glow[1];
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
        list = dd_depth_enable(list, 1, 1);
        list = dd_blend_enable(list, 1, 0, 6, 10, 0, -1);
        list = dd_tex_pack(list, 3);
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
            du = 0.25f;
            w3 = 3.0f;
            dv = 0.5f;
            for (n27 = 0; n27 < 4; n27++) {
                tu = n27 % 4 * du;
                tv = n27 / 4 * dv;
                d->sq[n27][0].u = tu;      d->sq[n27][0].v = tv;      d->sq[n27][0].x = -w3;
                d->sq[n27][0].y = w3;      d->sq[n27][0].z = 0.0f;
                d->sq[n27][1].u = tu + du; d->sq[n27][1].v = tv + dv; d->sq[n27][1].x = w3;
                d->sq[n27][1].y = -w3;     d->sq[n27][1].z = 0.0f;
            }
        }
        if (d->grow == d->grow_end && d->npts > 0) {
            sp = &d->spark[d->spark_next];
            sp->flags |= 1;
            sp->t = 0;
            for (n29 = 0; n29 < 8; n29++) {
                pp = (DdTrailPart *)&sp->p[n29];
                pp->frame = DD_RAND(r) % 4;
                vec3_set(&pp->pos, d->curve[0][n29].x, d->curve[0][n29].y, d->curve[0][n29].z);
                dd_rand_dir(5.0f, &pp->vel);
            }
            if (d->spark_next < 15)
                d->spark_next++;
            else
                d->spark_next = 0;
        }
        if (d->grow == d->grow_end) {
            list = dl_select_layer(7, 0);
            list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
            list = dd_depth_enable(list, 1, 1);
            list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
            list = dd_tex_pack(list, 14);
            list = dl_color(list, 0xFFE6FF64);
            mtx_set_mode(2);
            mtx_push();
            mtx_identity();
            for (i = 0; i < 16; i++) {
                DdTrailSpark *sp2 = &d->spark[i];

                if (!DD_BIT(d->spark[i].flags, 1) != 1) {
                int jb;
                e = dd_ease_sin(sp2->t, 16, 1) * 0.3f + 0.7f;
                list = dl_ambient_light(list, (unsigned int)(out * (in * (dd_ease_cos(sp2->t, 16, 1) * 255.0f))) << 24
                                                  | 0xFFFFFF);
                for (n33 = 0; n33 < 8; n33++) {
                    pp = (DdTrailPart *)&sp2->p[n33];
                    vec3_scale(&pos, &pp->vel, e);
                    vec3_add(&pos, &pos, &pp->pos);
                    mtx_push();
                    mtx_translate(&pos);
                    mtx_mult(&bill);
                    list = draw_array(list, 6, 0x183, 2, 0, d->sq[pp->frame]);
                    mtx_pop();
                }
                if (sp2->t < 15)
                    sp2->t++;
                else
                    sp2->flags &= ~1;
            }
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
            for (q = 0; q < 33; q++) {
                ang = 6.2831855f * q / 32.0f;
                d->fan[q + 1].u = 0.0f;
                d->fan[q + 1].v = 0.0f;
                d->fan[q + 1].color = 0x00E6FF64;
                d->fan[q + 1].x = sinf(ang) * 30.0f;
                d->fan[q + 1].y = cosf(ang) * 30.0f;
                d->fan[q + 1].z = 0.0f;
            }
        }
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 1, 1);
        list = dd_depth_enable(list, 0, 1);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, -1);
        list = dd_tex_fb_copy(list);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_translate(&ring);
        mtx_mult(&bill);
        dd_mtx_mvp_cam(&vp);
        for (q2 = 0; q2 < 34; q2++) {
            w = dd_trail_w;
            w.x = d->fan[q2].x;
            w.y = d->fan[q2].y;
            w.z = d->fan[q2].z;
            mat4_transform_vec4(&w, &vp, &w);
            fv = (w.y * -136.0f / w.w + 2048.0f - 1912.0f) / 512.0f;
            d->fan[q2].u = (w.x * 240.0f / w.w + 2048.0f - 1808.0f) / 512.0f;
            d->fan[q2].v = fv;
        }
        memcpy(d->fan_buf[buf], d->fan, sizeof(d->fan));
        for (n39 = 0; n39 < 3; n39++) {
            k = n39 / 3.0f;
            sc = k * 0.3f + 1.0f;
            list = dl_ambient_light(list, (unsigned int)(out * ((1.0f - k) * 255.0f *
                                                         dd_ease_sin(d->grow, d->grow_end, 1))) << 24 | 0xFFFFFF);
            mtx_push();
            dd_scale3(sc, sc, 0.0f);
            list = draw_array(list, 5, 0x19F, 34, 0, d->fan_buf[buf]);
            mtx_pop();
        }
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
