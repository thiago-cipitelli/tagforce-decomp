// @range 0x08837098-0x08838068
// @asm 0x08837108-0x08838068
// @data 0x08876B54-0x08876B9C
/* The spirit orbs: up to five flames that appear in a row over a dimmed field, bob, and shed sparks;
   the last one joins when the others have faded in (task of 0x219C bytes). */
#include "dd_fx.h"

int dd_orbs_update(int phase, DdOrbFx *d);

/* flame frames of the orb sheet (u from, u to) and the two rows (v from, v to); local templates of
   dd_orb_draw (kept in asm) */
int dd_orb_frames[7][2] = {
    { 0, 22 }, { 0, 26 }, { 26, 60 }, { 60, 94 }, { 94, 148 }, { 148, 202 }, { 202, 256 },
};
int dd_orb_bands[2][2] = { { 0, 64 }, { 64, 128 } };

int dd_orbs_start(int count, int flag, int a, int b)
{
    int task = func_0882623C(dd_orbs_update, a, b, sizeof(DdOrbFx));
    DdOrbFx *d;

    if (task != -1) {
        d = func_08826440(task);
        d->count = count;
        d->flag = flag;
    }
    return task;
}

#ifdef NON_MATCHING /* mesma forma; o original guarda y na pilha e passo/x em $s3/$s4 (alocação de registradores) */
/* one orb: the flame animation and, once faded in, the halo above it */
void dd_orb_draw(float alpha, DdOrb *o, int flag, int x, int y, int step)
{
    void *list = dl_select_layer(6, 0);
    int t = o->t;
    int anim = o->anim;
    int full = t >= 16;
    int u = dd_orb_frames[anim][0];
    int w = dd_orb_frames[anim][1] - u;
    int v = dd_orb_bands[anim != 0][0];
    int h = dd_orb_bands[anim != 0][1] - v;
    int sw = (float)w;
    int sh = (float)h;
    int hu;

    list = dl_color(list, (unsigned int)(alpha * (255.0f * func_088279A4(t, 16))) << 24 | 0xFFFFFF);
    list = draw_set_offsets(list, w, h, sw, sh);
    list = draw_sprite(list, x + -sw / 2, y + -sh / 2, 0, u, v, 0, 0);
    o->tick += step;
    if (o->tick >= 3) {
        o->tick = 0;
        if (o->anim < 6)
            o->anim++;
        else
            o->anim = 4;
    }
    o->t += step;
    if (full == 1) {
        /* the halo: 31x31, column idx of the sheet, row 0 or 1 by `flag` */
        hu = o->idx * 32 + 23;
        u = 0;
        v = 31.0f;
        h = -v / 2;
        sh = -v / 2 + 4;
        sw = (unsigned int)(alpha * (255.0f * func_088279A4(o->t2, 16)));
        if (flag == 0)
            u += 32;
        sh = sh + 5.0f * sinf(0.08726647f * o->t2 + o->phase2);
        list = dl_color(list, sw << 24 | 0xFFFFFF);
        list = draw_set_offsets(list, 31, 31, v, v);
        list = draw_sprite(list, x + h, y + sh, 0, hu, u, 0, 0);
        o->t2 += step;
    }
    dl_set_list(list);
}

#endif

#ifdef NON_MATCHING /* função de 2,9 KB: ordem dos cálculos de float e registradores diferem */
/* phase 1 = first frame; returns 2 while running, 4 when done */
int dd_orbs_update(int phase, DdOrbFx *d)
{
    int step = func_08828290();
    DdRandSeed *r;
    DdOrb *o;
    DdSparkSet *set;
    DdSpark *p;
    DdSparkSpr *sp;
    void *list;
    float fade;
    float out;
    float k;
    float a;
    float s;
    float e;
    float dist;
    float ang;
    int i;
    int n;
    int j;
    int x;
    int y;
    int xo;

    if (phase == 1) {
        if (d->count < 5)
            d->end = 120;
        else
            d->end = 180;
        if (d->count == 1)
            d->fade = 16;
    }
    fade = func_088279A4(d->fade, 16);
    out = func_088279D8(d->t - (d->end - 16), 16);

    /* dim the field */
    list = dl_select_layer(2, 0);
    list = func_088266D8(list, 0, 1, 1, 0, 0, 1, 1);
    list = func_0882698C(list, 0, 1);
    list = func_088269D8(list, 1, 0, 2, 3, 0, -1);
    list = dl_color(list, (unsigned int)(128.0f * fade * out) << 24);
    list = draw_set_offsets(list, 0, 0, 480, 272);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    dl_set_list(list);

    /* first frame: show all orbs but the last */
    if (d->fade == 0) {
        for (i = 0; i < d->count - 1; i++) {
            o = &d->orb[i];
            o->flags |= 1;
            d->set[i].flags |= 1;
            o->idx = i;
            o->phase = 12.566371f * o->idx / 5.0f + 0.017453292f * (DD_RAND(r) / 32767.0f);
            o->phase2 = 6.2831855f * (DD_RAND(r) / 32767.0f);
            if (i != d->count - 1)
                o->t = 32;
        }
    }
    /* faded in: the last orb appears */
    if (d->last == 0 && d->fade >= 16) {
        i = d->count - 1;
        o = &d->orb[i];
        o->flags |= 1;
        d->set[i].flags |= 1;
        o->idx = d->count - 1;
        o->phase = 12.566371f * o->idx / 5.0f + 0.017453292f * (DD_RAND(r) / 32767.0f);
        o->phase2 = 6.2831855f * (DD_RAND(r) / 32767.0f);
        d->last = 1;
    }

    /* the orbs */
    list = dl_select_layer(6, 0);
    list = func_0882698C(list, 0, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
    list = func_088269D8(list, 1, 0, 2, 3, 0, 0);
    list = func_08826A64(list, 30);
    dl_set_list(list);
    if (d->count == 5)
        k = func_08827B6C(d->t - 32, 32, 1) * 3.0f + 1.0f;
    else
        k = 1.0f;
    a = fade * out;
    for (i = 4, xo = 180; i >= 0; i--, xo -= 45) {
        o = &d->orb[i];
        if (o->flags & 1) {
            s = sinf(0.08726647f * o->t + o->phase);
            e = func_08827A2C(o->t, 32, 1);
            x = (xo - 90) * e + 314.0f;
            y = e * ((int)(k * 5.0f * s) - 40) + 157.0f;
            o->x = x;
            o->y = y;
            dd_orb_draw(a, o, d->flag, x, y, step);
        }
    }

    /* spawn `step` sparks per orb */
    for (i = 0; i < 5; i++) {
        set = &d->set[i];
        if (set->flags & 1) {
            a = func_088279A4(set->t, 32);
            set->t += step;
            if (set->t >= 32)
                set->t = 32;
            for (n = 0; n < step; n++) {
                p = &set->p[set->next];
                p->flags |= 1;
                p->t = 0;
                p->alpha = a;
                for (j = 0; j < 2; j++) {
                    sp = &p->spr[j];
                    x = DD_RAND(r);
                    sp->u = (x & 3) << 5;
                    sp->v = ((x & 7) >> 2) << 5;
                    sp->x = d->orb[i].x;
                    sp->y = d->orb[i].y;
                    dist = DD_RAND(r) / 32767.0f;
                    ang = 6.2831855f * (DD_RAND(r) / 32767.0f);
                    sp->x = sp->x + dist * 30.0f * sinf(ang);
                    sp->y = sp->y + dist * 10.0f * cosf(ang) + 35.0f;
                    sp->vy = (1.0f - dist) * -0.5f - 2.0f;
                }
                if (set->next < 31)
                    set->next++;
                else
                    set->next = 0;
            }
        }
    }

    /* the sparks */
    list = dl_select_layer(6, 0);
    list = func_0882698C(list, 0, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
    list = func_088269D8(list, 1, 0, 2, 3, 0, 0);
    list = func_08826A64(list, 5);
    list = draw_set_offsets(list, 32, 32, 25, 25);
    for (i = 0; i < 5; i++) {
        set = &d->set[i];
        if (set->flags & 1) {
            for (n = 0; n < 32; n++) {
                p = &set->p[n];
                if (p->flags & 1) {
                    s = sinf(3.1415927f * p->t / 32.0f);
                    list = dl_color(list, (unsigned int)(out * fade * p->alpha * s * 200.0f) << 24 | 0xFFFF80);
                    for (j = 0; j < 2; j++) {
                        sp = &p->spr[j];
                        list = draw_sprite(list, sp->x - 12.0f, sp->y - 12.0f + sp->vy * p->t, 0, sp->u, sp->v, 0, 0);
                    }
                    p->t += step;
                    if (p->t >= 32)
                        p->flags &= ~1;
                }
            }
        }
    }
    dl_set_list(list);

    if (d->fade < 16) {
        d->fade += step;
        if (d->fade > 16)
            d->fade = 16;
    } else {
        d->t += step;
    }
    return d->t < d->end ? 2 : 4;
}
#endif
