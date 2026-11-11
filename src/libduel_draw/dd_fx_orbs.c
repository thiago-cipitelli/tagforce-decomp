// @range 0x08837098-0x08838068
// @asm 0x08837108-0x08838068
// @data 0x08876B54-0x08876B9C
/* The spirit orbs: up to five flames that appear in a row over a dimmed field, bob, and shed sparks;
   the last one joins when the others have faded in (task of 0x219C bytes). */
#include "dd_fx.h"
#undef DD_RAND
#define DD_RAND(r) ((r) = &rand3_seeds[rand3_slot], (r)->lcg *= 0x41C64E6D, (int)(((r)->lcg += 0x3039) >> 16 & 0x7FFF))


int dd_orbs_update(int phase, DdOrbFx *d);

typedef struct DdXy {                  /* x, y of an orb or a spark, copied as a pair */
    float x, y;
} DdXy;

/* flame frames of the orb sheet (u from, u to) and the two rows (v from, v to): local templates of
   dd_orb_draw (data of the asm while it is kept as asm) */
int dd_orb_frames[7][2] = {
    { 0, 22 }, { 0, 26 }, { 26, 60 }, { 60, 94 }, { 94, 148 }, { 148, 202 }, { 202, 256 },
};
int dd_orb_bands[2][2] = { { 0, 64 }, { 64, 128 } };

int dd_orbs_start(int count, int flag, int a, int b)
{
    int task = dd_task_create(dd_orbs_update, a, b, sizeof(DdOrbFx));
    DdOrbFx *d;

    if (task != -1) {
        d = dd_task_mem(task);
        d->count = count;
        d->flag = flag;
    }
    return task;
}

#ifdef NON_MATCHING /* 2 palavras (rodada 9): falta só o registrador do 31.0f do halo (lui/mtc1 em v1 no original, v0 aqui); `list` dividida em l2 a partir do primeiro sprite e o 31 do halo em `v` resolveram o resto. Testados: ordem de w/v/hu/u, w = 31 fora do if, float local, ternário em full */
/* one orb: the flame animation and, once faded in, the halo above it */
void dd_orb_draw(DdOrb *o, int flag, int x, int y, float alpha, int step)
{
    void *l2;
    void *list = dl_select_layer(6, 0);
    int t = o->t;
    int full = t >= 16;
    int anim = o->anim;
    int frames[7][2] = {
        { 0, 22 }, { 0, 26 }, { 26, 60 }, { 60, 94 }, { 94, 148 }, { 148, 202 }, { 202, 256 },
    };
    int bands[2][2] = { { 0, 64 }, { 64, 128 } };
    int u = frames[anim][0];
    int v = bands[anim != 0][0];
    int w = frames[anim][1] - u;
    int h = bands[anim != 0][1] - v;
    int sw = (float)w;
    int sh = (float)h;
    int hu;

    list = dl_color(list, (unsigned int)(alpha * (255.0f * dd_ease_lin(t, 16))) << 24 | 0xFFFFFF);
    list = draw_set_offsets(list, w, h, sw, sh);
    l2 = draw_sprite(list, x + -sw / 2, y + -sh / 2, 0, u, v, 0, 0);
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
        w = 31;
        v = (float)w;
        hu = o->idx * 32 + 23;
        u = 0;
        h = -v / 2;
        sh = -(int)v / 2 + 4;
        sw = (unsigned int)(alpha * (255.0f * dd_ease_lin(o->t2, 16)));
        if (flag == 0)
            u += 32;
        anim = sh + 5.0f * sinf(0.08726647f * o->t2 + o->phase2);
        l2 = dl_color(l2, sw << 24 | 0xFFFFFF);
        l2 = draw_set_offsets(l2, 31, 31, v, v);
        l2 = draw_sprite(l2, x + h, y + anim, 0, hu, u, 0, 0);
        o->t2 += step;
    }
    dl_set_list(l2);
}

#endif

#ifdef NON_MATCHING /* 16 palavras (rodada 22; eram 65): faltam $f20/$f22 trocados entre `one` e `amp` (o alfa das faíscas divide o registrador com amp) e as posições de pilha do contador e do ponteiro de orb derramados no laço que solta as faíscas (+92/+84 trocados). Resolveram o resto: set = &d->set[xo], amp separado de k, xo = io * 45 (indução criada depois do ponteiro), cópia de x/y como par (DdXy) e q próprio no laço das faíscas. Testados sem efeito: ordem das declarações (busca gulosa), blocos, nomes dos contadores, o = d->orb + i */
/* phase 1 = first frame; returns 2 while running, 4 when done */
int dd_orbs_update(int phase, DdOrbFx *d)
{
    int io;
    DdOrb *ob;
    int ns;
    int js;
    int step = dd_anim_step();
    DdRandSeed *r;
    DdOrb *o;
    DdSparkSet *set;
    DdSpark *p;
    DdSparkSpr *sp;
    void *list;
    float fade;
    float out;
    float k;
    float one;
    float a;
    float amp;
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
    fade = dd_ease_lin(d->fade, 16);
    out = dd_ease_lin_inv(d->t - (d->end - 16), 16);

    /* dim the field */
    list = dl_select_layer(2, 0);
    list = dd_tex_enable(list, 0, 1, 1, 0, 0, 1, 1);
    list = dd_depth_enable(list, 0, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, -1);
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
        xo = d->count - 1;
        o = &d->orb[xo];
        o->flags |= 1;
        set = &d->set[xo];
        set->flags |= 1;
        o->idx = d->count - 1;
        o->phase = 12.566371f * o->idx / 5.0f + 0.017453292f * (DD_RAND(r) / 32767.0f);
        o->phase2 = 6.2831855f * (DD_RAND(r) / 32767.0f);
        d->last = 1;
    }

    /* the orbs */
    list = dl_select_layer(6, 0);
    list = dd_depth_enable(list, 0, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dd_tex_pack(list, 30);
    dl_set_list(list);
    if (d->count == 5) {
        k = dd_ease_cos_inv(d->t - 32, 32, 1);
        one = 1.0f;
        k = one + k * 3.0f;
    } else {
        one = 1.0f;
        k = one;
    }
    amp = 5.0f * k;
    a = fade * out;
    for (io = 4; io >= 0; io--) {
        ob = &d->orb[io];
        if (!(ob->flags & 1) != 1) {
            s = sinf(0.08726647f * ob->t + ob->phase);
            y = (int)(amp * s);
            e = dd_ease_sin(ob->t, 32, 1);
            xo = io * 45;
            x = (xo - 90) * e + 314.0f;
            y = e * (y - 40) + 157.0f;
            ob->x = x;
            ob->y = y;
            dd_orb_draw(ob, d->flag, x, y, a, step);
        }
    }

    /* spawn `step` sparks per orb */
    for (i = 0; i < 5; i++) {
        set = &d->set[i];
        if (!(set->flags & 1) != 1) {
            o = &d->orb[i];
            a = dd_ease_lin(set->t, 32);
            set->t += step;
            if (set->t >= 32)
                set->t = 32;
            for (ns = 0; ns < step; ns++) {
                p = &set->p[set->next];
                p->flags |= 1;
                p->t = 0;
                p->alpha = a;
                for (js = 0; js < 2; js++) {
                    sp = &p->spr[js];
                    x = DD_RAND(r) % 8;
                    sp->u = x % 4 * 32;
                    sp->v = x / 4 * 32;
                    *(DdXy *)&sp->x = *(DdXy *)&o->x;
                    dist = DD_RAND(r) / 32767.0f;
                    ang = 6.2831855f * (DD_RAND(r) / 32767.0f);
                    sp->x = sp->x + dist * 30.0f * sinf(ang);
                    sp->y = sp->y + (dist * 10.0f * cosf(ang) + 35.0f);
                    sp->vy = (one - dist) * -0.5f + -2.0f;
                }
                if (set->next < 31)
                    set->next++;
                else
                    set->next = 0;
            }
        }
    }

    /* the sparks */
    {
    DdSpark *q;                        /* own variable: p gives other $s */

    list = dl_select_layer(6, 0);
    list = dd_depth_enable(list, 0, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dd_tex_pack(list, 5);
    list = draw_set_offsets(list, 32, 32, 25, 25);
    for (i = 0; i < 5; i++) {
        set = &d->set[i];
        if (!(set->flags & 1) != 1) {
            for (n = 0; n < 32; n++) {
                q = &set->p[n];
                if (!(q->flags & 1) != 1) {
                    s = sinf(3.1415927f * q->t / 32.0f);
                    list = dl_color(list, (unsigned int)(out * (fade * (q->alpha * (s * 200.0f)))) << 24 | 0xFFFF80);
                    for (j = 0; j < 2; j++) {
                        sp = &q->spr[j];
                        list = draw_sprite(list, sp->x - 12.0f, sp->y - 12.0f + sp->vy * q->t, 0, sp->u, sp->v, 0, 0);
                    }
                    q->t += step;
                    if (q->t >= 32)
                        q->flags &= ~1;
                }
            }
        }
    }
    dl_set_list(list);
    }

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
