// @range 0x088522F0-0x08853674
/* Character effect "lightning": bolts (k02_256b.gim) striking at random places every 5-34
   frames, growing down from the sky with a flash (lens_aP8A.gim) at both ends, and glowing rings
   left on the ground. */
#include "libduel_draw.h"

void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_ambient_light(void *list, unsigned int color);
void *dl_tex_offset(float u, float v, void *list);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(FxVec3 *v);
void mtx_mult(FxMat4 *m);
void mtx_rotate_x(float a);
void mtx_rotate_y(float a);
FxVec3 *vec3_set(FxVec3 *out, float x, float y, float z);
float sinf(float x);
float cosf(float x);

float dd_ease_lin(int t, int len);
float dd_ease_sin(int t, int len, int n);
float dd_ease_cos(int t, int len, int n);
int dd_rand_percent(int percent);
void dd_mtx_push_field(void);
void dd_mtx_pop_all(void);
void dd_mtx_billboard_field(FxMat4 *out);
void dd_translate3(float x, float y, float z);
void dd_scale3(float x, float y, float z);
int dd_task_create(void *update, void *a, void *b, int size);
void *dd_task_mem(int task);
void *dd_depth_enable(void *list, int on, int mask);
void *dd_blend_enable(void *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dd_tex_pack(void *list, int tex);
void *dd_tex_enable(void *list, int on, int min, int mag, int func, int comp, int wrapu, int wrapv);

typedef struct RandSeed {              /* BOOT rand3_seeds entry, 12 bytes */
    unsigned int lcg;
    unsigned int b, c;
} RandSeed;
extern int rand3_slot;                 /* BOOT variable imports, 0 in the relocated ELF */
extern RandSeed rand3_seeds[];

#define RAND15() (rand3_seeds[rand3_slot].lcg *= 0x41C64E6D, rand3_seeds[rand3_slot].lcg += 0x3039, \
                  (int)(rand3_seeds[rand3_slot].lcg >> 16 & 0x7FFF))
#define RANDF() (RAND15() / 32767.0f)

typedef struct DdBolt {                /* 0x9C8 */
    unsigned int flags;                /* +0x000 bit 0 alive */
    int t;
    int big;                           /* +0x008 on the character (10%) */
    FxVec3 pos;                        /* +0x00C */
    FxVec3 end;                        /* +0x018 flash at the lower end */
    float scale;                       /* +0x024 */
    FxVtx v[2][17][3];                 /* +0x028 one copy per frame parity */
    float u[4];                        /* +0x9B8 texture offsets of the 4 layers */
} DdBolt;

typedef struct DdRingTex {             /* reached through a pointer to it */
    float u;
    float angle;
    FxVec3 pos;
} DdRingTex;

typedef struct DdBoltRing {            /* 0x1C */
    unsigned int flags;
    int t;
    DdRingTex p;                       /* +0x08 */
} DdBoltRing;

typedef struct DdFxBolt {              /* 0x4A28 */
    int arg;                           /* +0x0000 */
    int frame;                         /* +0x0004 */
    int parity;                        /* +0x0008 */
    int wait;                          /* +0x000C */
    int next;                          /* +0x0010 */
    int cur;                           /* +0x0014 */
    unsigned char idx[34];             /* +0x0018 */
    short pad3a;
    DdBolt bolt[7];                    /* +0x003C */
    FxVtx flash_a[6];                  /* +0x44B4 */
    FxVtx flash_b[6];                  /* +0x4544 */
    FxVtx end_a[6];                    /* +0x45D4 */
    FxVtx end_b[6];                    /* +0x4664 */
    int ring_cur;                      /* +0x46F4 */
    FxVtx ring_v[6];                   /* +0x46F8 */
    DdBoltRing ring[24];               /* +0x4788 */
} DdFxBolt;

int dd_fx_bolt_update(int phase, DdFxBolt *w);

int dd_fx_bolt_start(int arg, void *a, void *b)
{
    int task = dd_task_create(dd_fx_bolt_update, a, b, sizeof(DdFxBolt));

    if (task != -1)
        ((DdFxBolt *)dd_task_mem(task))->arg = arg;
    return task;
}

inline static void dd_fan(FxVtx *q, unsigned int c0, unsigned int c, float r, float b)
{
    q[0].u = 0.5f; q[0].v = 0.5f; q[0].color = c0; q[0].x = 0.0f; q[0].y = 0.0f; q[0].z = 0.0f;
    q[1].u = 0.0f; q[1].v = 0.0f; q[1].color = c; q[1].x = -r; q[1].y = r; q[1].z = 0.0f;
    q[2].u = 1.0f; q[2].v = 0.0f; q[2].color = c; q[2].x = r; q[2].y = r; q[2].z = 0.0f;
    q[3].u = 1.0f; q[3].v = 1.0f; q[3].color = c; q[3].x = r; q[3].y = b; q[3].z = 0.0f;
    q[4].u = 0.0f; q[4].v = 1.0f; q[4].color = c; q[4].x = -r; q[4].y = b; q[4].z = 0.0f;
    q[5] = q[1];
}

int dd_fx_bolt_update(int phase, DdFxBolt *w)
{
    int buf = w->parity % 2;
    FxMat4 cam;
    void *list;
    DdBolt *b;
    DdBoltRing *r;
    DdRingTex *q;
    unsigned int c;
    float ph;
    float freq;
    float f;
    float x;
    float y;
    float s;
    float d;
    float ang;
    float h = 5.0f, wd = 0.8f;
    int n;
    int i;
    int j;
    int iv, k, id, ir, ic;

    dd_mtx_push_field();
    dd_mtx_billboard_field(&cam);
    if (phase == 1) {
        for (i = 0; i < 17; i++) {
            w->idx[i * 2] = i * 3;
            w->idx[i * 2 + 1] = i * 3 + 1;
        }
        w->next = RAND15() % 30 + 5;
        {
            float r2 = 2.0f, r5 = 5.0f, r08 = 0.8f, hf = 0.5f;

            dd_fan(w->flash_a, 0xFFFF6400, 0xFF6400, r2, -r2);
            dd_fan(w->end_a, 0xFFFF6400, 0xFF6400, r5, -r5 * hf);
            dd_fan(w->flash_b, 0xFFFFFFC8, 0xFFFFC8, r08, -r08);
            dd_fan(w->end_b, 0xFFFFFFC8, 0xFFFFC8, r2, -r2 * hf);
        }
    }
    if (w->wait < w->next) {
        w->wait++;
    } else {
        b = &w->bolt[w->cur];
        b->flags |= 1;
        b->t = 0;
        if (dd_rand_percent(10) == 1) {
            b->big = 1;
            b->scale = 1.0f;
            vec3_set(&b->pos, 0.0f, 0.0f, 0.0f);
        } else {
            b->big = 0;
            b->scale = 0.8f;
            f = RANDF();
            vec3_set(&b->pos, (RANDF() * 2.0f + -1.0f) * 8.0f, 0.0f, f * -6.0f + -6.0f);
        }
        for (k = 0; k < 4; k++)
            b->u[k] = RANDF();
        ph = 6.2831855f * RANDF();
        freq = 1.5707964f + 6.2831855f * RANDF();
        for (iv = 0; iv < 17; iv++) {
            f = (float)iv / 16.0f;
            y = -h * f;
            x = 0.5f * sinf(ph + freq * f) * sinf(1.5707964f * f);
            b->v[0][iv][0].u = f; b->v[0][iv][0].v = 0.0f; b->v[0][iv][0].color = 0xFFFFFF;
            b->v[0][iv][0].x = -wd + x; b->v[0][iv][0].y = y; b->v[0][iv][0].z = 0.0f;
            b->v[0][iv][1].u = f; b->v[0][iv][1].v = 0.5f; b->v[0][iv][1].color = 0xFFFFFFFF;
            b->v[0][iv][1].x = x; b->v[0][iv][1].y = y; b->v[0][iv][1].z = 0.0f;
            b->v[0][iv][2].u = f; b->v[0][iv][2].v = 1.0f; b->v[0][iv][2].color = 0xFFFFFF;
            b->v[0][iv][2].x = 0.8f + x; b->v[0][iv][2].y = y; b->v[0][iv][2].z = 0.0f;
            b->v[1][iv][0] = b->v[0][iv][0];
            b->v[1][iv][1] = b->v[0][iv][1];
            b->v[1][iv][2] = b->v[0][iv][2];
        }
        vec3_set(&b->end, sinf(ph + freq) * 0.5f, -1.2f, 0.0f);
        w->wait = 0;
        w->next = RAND15() % 30 + 5;
        if (w->cur < 6)
            w->cur++;
        else
            w->cur = 0;
    }
    list = dl_select_layer(1, 0);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (j = 0; j < 7; j++) {
        b = &w->bolt[j];
        if (!(b->flags & 1) == 1)
            continue;
        s = b->scale * (dd_ease_sin(b->t, 32, 1) * 0.2f + 0.8f);
        c = (unsigned int)(dd_ease_cos(b->t - 8, 24, 1) * 255.0f);
        n = b->t * 5;
        if (b->big == 0)
            c >>= 1;
        if (n < 2)
            n = 2;
        else if (n > 17)
            n = 17;
        mtx_push();
        mtx_translate(&b->pos);
        list = dd_depth_enable(list, 0, 1);
        list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
        list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
        list = dd_tex_pack(list, 27);
        c = c << 24 | 0xFFFFFF;
        list = dl_ambient_light(list, c);
        for (ic = 0; ic < n; ic++) {
            if (ic == 0 || ic == n - 1)
                b->v[buf][ic][1].color = 0xFFFFFF;
            else
                b->v[buf][ic][1].color = 0xFFFFFFFF;
        }
        mtx_push();
        dd_translate3(0.0f, 3.7f, 0.0f);
        dd_scale3(s, 1.0f, 0.0f);
        for (ic = 0; ic < 4; ic++) {
            list = dl_tex_offset(b->u[ic], 0.0f, list);
            for (id = 0; id < 2; id++)
                list = draw_array(list, 4, 0x99F, n * 2, w->idx, &b->v[buf][0][id]);
        }
        mtx_pop();
        list = dd_depth_enable(list, 0, 1);
        list = dd_tex_enable(list, 1, 1, 1, 0, 1, 0, 0);
        list = dd_blend_enable(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
        list = dd_tex_pack(list, 3);
        list = dl_ambient_light(list, c);
        mtx_push();
        dd_translate3(0.0f, 3.7f, 0.0f);
        mtx_mult(&cam);
        list = draw_array(list, 5, 0x19F, 6, 0, w->flash_a);
        list = draw_array(list, 5, 0x19F, 6, 0, w->flash_b);
        mtx_pop();
        if (n == 17) {
            mtx_push();
            mtx_translate(&b->end);
            dd_scale3(s, s, 0.0f);
            mtx_mult(&cam);
            list = draw_array(list, 5, 0x19F, 6, 0, w->end_a);
            list = draw_array(list, 5, 0x19F, 6, 0, w->end_b);
            mtx_pop();
        }
        mtx_pop();
        if (b->t < 31)
            b->t++;
        else
            b->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    if (phase == 1) {
        float r2 = 2.0f;

        w->ring_v[0].u = 0.5f; w->ring_v[0].v = 0.5f; w->ring_v[0].color = 0x82FFFFFF;
        w->ring_v[0].x = 0.0f; w->ring_v[0].y = 0.0f; w->ring_v[0].z = 0.0f;
        w->ring_v[1].u = 0.0f; w->ring_v[1].v = 0.0f; w->ring_v[1].color = 0xFFFFFF;
        w->ring_v[1].x = -r2; w->ring_v[1].y = 0.0f; w->ring_v[1].z = r2;
        w->ring_v[2].u = 1.0f; w->ring_v[2].v = 0.0f; w->ring_v[2].color = 0xFFFFFF;
        w->ring_v[2].x = r2; w->ring_v[2].y = 0.0f; w->ring_v[2].z = r2;
        w->ring_v[3].u = 1.0f; w->ring_v[3].v = 1.0f; w->ring_v[3].color = 0xFFFFFF;
        w->ring_v[3].x = r2; w->ring_v[3].y = 0.0f; w->ring_v[3].z = -r2;
        w->ring_v[4].u = 0.0f; w->ring_v[4].v = 1.0f; w->ring_v[4].color = 0xFFFFFF;
        w->ring_v[4].x = -r2; w->ring_v[4].y = 0.0f; w->ring_v[4].z = -r2;
        w->ring_v[5] = w->ring_v[1];
    }
    if (w->frame % 3 == 0) {
        r = &w->ring[w->ring_cur];
        r->flags |= 1;
        r->t = 0;
        q = &r->p;
        q->u = RANDF();
        q->angle = RANDF() * 6.2831855f;
        d = RANDF() * 8.0f + 3.0f;
        ang = RANDF() * 6.2831855f;
        vec3_set(&q->pos, d * sinf(ang), 0.0f, d * cosf(ang));
        if (w->ring_cur < 23)
            w->ring_cur++;
        else
            w->ring_cur = 0;
    }
    list = dl_select_layer(1, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
    list = dd_depth_enable(list, 0, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 27);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_translate3(0.0f, 3.7f, 0.0f);
    mtx_rotate_x(-0.13962634f);
    for (ir = 0; ir < 24; ir++) {
        r = &w->ring[ir];
        if (!(r->flags & 1) != 1) {
            s = dd_ease_sin(r->t, 24, 1) * 0.1f + 0.9f;
            list = dl_ambient_light(list, (unsigned int)(dd_ease_lin(r->t, 4) * 255.0f *
                                                         dd_ease_cos(r->t - 4, 20, 1)) << 24 | 0xFFFFFF);
            q = &r->p;
            list = dl_tex_offset(q->u, 0.0f, list);
            mtx_push();
            mtx_translate(&q->pos);
            mtx_rotate_y(q->angle);
            dd_scale3(s, 0.0f, s);
            list = draw_array(list, 5, 0x19F, 6, 0, w->ring_v);
            mtx_pop();
            if (r->t < 23)
                r->t++;
            else
                r->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    dd_mtx_pop_all();
    w->parity++;
    w->frame++;
    return 2;
}
