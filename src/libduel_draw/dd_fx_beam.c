// @range 0x08853674-0x088546C4
/* Character effect "light beam": every 620-640 frames a beam of 12 scrolling layers
   (k02_256y.gim) comes down for 600 frames with sparkles (lens_aP8A.gim) at its foot, and glows
   circle around the source. */
#include "libduel_draw.h"

void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_ambient_light(void *list, unsigned int color);
void *dl_tex_offset(float u, float v, void *list);
void *dl_tex_scale(float u, float v, void *list);
void *dl_depth_range_i(void *list, int bias);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(FxVec3 *v);
void mtx_mult(FxMat4 *m);
void mtx_rotate_z(float a);
FxVec3 *vec3_set(FxVec3 *out, float x, float y, float z);
float sinf(float x);
float cosf(float x);

float dd_ease_lin(int t, int len);
float dd_ease_sin(int t, int len, int n);
float dd_ease_cos(int t, int len, int n);
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
#define RANDSIGN() ((float)(RAND15() % 2 * 2 - 1))

typedef struct DdBeamLayer {
    float u;                           /* texture offset */
    float su, sv;                      /* texture scale, +-1 */
} DdBeamLayer;

typedef struct DdBeam {                /* 0xA28 */
    unsigned int flags;
    int t;
    FxVtx v[2][17][3];                 /* +0x008 */
    DdBeamLayer layer[12];             /* +0x998 */
} DdBeam;

typedef struct DdBeamSpr {
    float u;                           /* texture offset */
    float angle;
    FxVec3 pos;
} DdBeamSpr;

typedef struct DdBeamGlow {            /* 0x1C */
    unsigned int flags;
    int t;
    DdBeamSpr p;                       /* +0x08 */
} DdBeamGlow;

typedef struct DdFxBeam {              /* 0xBF8 */
    int arg;                           /* +0x000 */
    int frame;                         /* +0x004 */
    int parity;                        /* +0x008 */
    int wait;                          /* +0x00C */
    int next;                          /* +0x010 */
    int cur;                           /* +0x014 */
    unsigned char idx[34];             /* +0x018 */
    short pad3a;
    DdBeam beam[1];                    /* +0x03C */
    FxVtx flash[6];                    /* +0xA64 */
    int glow_cur;                      /* +0xAF4 */
    FxVtx glow_v[6];                   /* +0xAF8 */
    DdBeamGlow glow[4];                /* +0xB88 */
} DdFxBeam;

int dd_fx_beam_update(int phase, DdFxBeam *w);

int dd_fx_beam_start(int arg, void *a, void *b)
{
    int task = dd_task_create(dd_fx_beam_update, a, b, sizeof(DdFxBeam));

    if (task != -1)
        ((DdFxBeam *)dd_task_mem(task))->arg = arg;
    return task;
}

int dd_fx_beam_update(int phase, DdFxBeam *w)
{
    int buf = w->parity % 2;
    FxMat4 cam;
    void *list;
    DdBeam *b;
    DdBeamGlow *g;
    DdBeamLayer *l;
    DdBeamSpr *q;
    unsigned int c;
    float ph;
    float freq;
    float f;
    float x;
    float y;
    float s;
    float ang;
    float r;
    float hh;
    float hw;
    int n;
    int i;
    int k;
    int j;
    int k2;
    int i2;

    dd_mtx_push_field();
    dd_mtx_billboard_field(&cam);
    if (phase == 1) {
        for (i = 0; i < 17; i++) {
            w->idx[i * 2] = i * 3;
            w->idx[i * 2 + 1] = i * 3 + 1;
        }
        w->next = 20;
        r = 1.0f;
        w->flash[0].u = 0.5f; w->flash[0].v = 0.5f; w->flash[0].color = 0xFF32C8FF;
        w->flash[0].x = 0.0f; w->flash[0].y = 0.0f; w->flash[0].z = 0.0f;
        w->flash[1].u = 0.0f; w->flash[1].v = 0.0f; w->flash[1].color = 0x32C8FF;
        w->flash[1].x = -r; w->flash[1].y = r; w->flash[1].z = 0.0f;
        w->flash[2].u = 1.0f; w->flash[2].v = 0.0f; w->flash[2].color = 0x32C8FF;
        w->flash[2].x = r; w->flash[2].y = r; w->flash[2].z = 0.0f;
        w->flash[3].u = 1.0f; w->flash[3].v = 1.0f; w->flash[3].color = 0x32C8FF;
        w->flash[3].x = r; w->flash[3].y = -r; w->flash[3].z = 0.0f;
        w->flash[4].u = 0.0f; w->flash[4].v = 1.0f; w->flash[4].color = 0x32C8FF;
        w->flash[4].x = -r; w->flash[4].y = -r; w->flash[4].z = 0.0f;
        w->flash[5] = w->flash[1];
    }
    if (w->wait < w->next) {
        w->wait++;
    } else {
        b = &w->beam[w->cur];
        b->flags |= 1;
        b->t = 0;
        for (k = 0; k < 12; k++) {
            l = &b->layer[k];
            l->u = RANDF();
            l->su = RANDSIGN();
            l->sv = RANDSIGN();
        }
        ph = RANDF() * 6.2831855f;
        freq = RANDF() * 6.2831855f + 1.5707964f;
        hh = 2.5f;
        hw = 0.3f;
        for (i = 0; i < 17; i++) {
            f = (float)i / 16.0f;
            y = -hh * f;
            x = sinf(ph + freq * f) * 0.0f * cosf(f * 1.5707964f);
            b->v[0][i][0].u = f; b->v[0][i][0].v = 0.0f; b->v[0][i][0].color = 0xFFFFFF;
            b->v[0][i][0].x = -hw + x; b->v[0][i][0].y = y; b->v[0][i][0].z = 0.0f;
            b->v[0][i][1].u = f; b->v[0][i][1].v = 0.5f; b->v[0][i][1].color = 0x40FFFFFF;
            b->v[0][i][1].x = x; b->v[0][i][1].y = y; b->v[0][i][1].z = 0.0f;
            b->v[0][i][2].u = f; b->v[0][i][2].v = 1.0f; b->v[0][i][2].color = 0xFFFFFF;
            b->v[0][i][2].x = hw + x; b->v[0][i][2].y = y; b->v[0][i][2].z = 0.0f;
            b->v[1][i][0] = b->v[0][i][0];
            b->v[1][i][1] = b->v[0][i][1];
            b->v[1][i][2] = b->v[0][i][2];
        }
        w->wait = 0;
        w->next = RAND15() % 20 + 620;
        if (w->cur < 0)
            w->cur++;
        else
            w->cur = 0;
    }
    list = dl_select_layer(1, 0);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (j = 0; j < 1; j++) {
        b = &w->beam[j];
        if (!(b->flags & 1) != 1) {
            c = (unsigned int)(dd_ease_cos(b->t - 576, 24, 1) * 255.0f);
            n = b->t * 2;
            if (n < 2)
                n = 2;
            else if (n > 17)
                n = 17;
            list = dd_depth_enable(list, 1, 1);
            list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
            list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
            list = dd_tex_pack(list, 29);
            c = c << 24 | 0xFFFFFF;
            list = dl_ambient_light(list, c);
            for (i2 = 0; i2 < n; i2++) {
                if (i2 == n - 1)
                    b->v[buf][i2][1].color = 0xFFFFFF;
                else
                    b->v[buf][i2][1].color = 0x40FFFFFF;
            }
            mtx_push();
            dd_translate3(0.0f, 4.25f, 0.0f);
            for (k = 0; k < 12; k++) {
                l = &b->layer[k];
                list = dl_tex_offset(l->u + RANDF(), 0.0f, list);
                list = dl_tex_scale(l->su, l->sv, list);
                for (k2 = 0; k2 < 2; k2++)
                    list = draw_array(list, 4, 0x99F, n * 2, w->idx, &b->v[buf][0][k2]);
            }
            mtx_pop();
            if (n == 17) {
                list = dd_depth_enable(list, 1, 1);
                list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
                list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
                list = dd_tex_pack(list, 3);
                list = dl_ambient_light(list, c);
                list = dl_depth_range_i(list, 2000);
                mtx_push();
                dd_translate3(0.0f, 1.75f, 0.0f);
                mtx_mult(&cam);
                for (i2 = 0; i2 < 8; i2++) {
                    s = RANDF() * 0.5f + 0.5f;
                    mtx_push();
                    mtx_rotate_z(RANDF() * 6.2831855f);
                    dd_scale3(s, s, 0.0f);
                    list = draw_array(list, 5, 0x19F, 6, 0, w->flash);
                    mtx_pop();
                }
                mtx_pop();
                list = dl_depth_range_i(list, 0);
            }
            if (b->t < 599)
                b->t++;
            else
                b->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    if (phase == 1) {
        r = 0.7f;
        w->glow_v[0].u = 0.5f; w->glow_v[0].v = 0.5f; w->glow_v[0].color = 0xC80064FF;
        w->glow_v[0].x = 2.5f; w->glow_v[0].y = 0.0f; w->glow_v[0].z = 0.0f;
        w->glow_v[1].u = 0.0f; w->glow_v[1].v = 0.0f; w->glow_v[1].color = 0x64FF;
        w->glow_v[1].x = 0.0f; w->glow_v[1].y = r; w->glow_v[1].z = 0.0f;
        w->glow_v[2].u = 1.0f; w->glow_v[2].v = 0.0f; w->glow_v[2].color = 0x64FF;
        w->glow_v[2].x = 5.0f; w->glow_v[2].y = r; w->glow_v[2].z = 0.0f;
        w->glow_v[3].u = 1.0f; w->glow_v[3].v = 1.0f; w->glow_v[3].color = 0x64FF;
        w->glow_v[3].x = 5.0f; w->glow_v[3].y = -r; w->glow_v[3].z = 0.0f;
        w->glow_v[4].u = 0.0f; w->glow_v[4].v = 1.0f; w->glow_v[4].color = 0x64FF;
        w->glow_v[4].x = 0.0f; w->glow_v[4].y = -r; w->glow_v[4].z = 0.0f;
        w->glow_v[5] = w->glow_v[1];
    }
    if (w->frame % 3 == 0) {
        n = phase == 1 ? 4 : 1;
        for (i = 0; i < n; i++) {
            g = &w->glow[w->glow_cur];
            g->flags |= 1;
            g->t = (n - 1 - i) * 3;
            ang = RANDF() * 6.2831855f;
            g->p.u = RANDF();
            g->p.angle = ang;
            q = &g->p;
            vec3_set(&q->pos, cosf(ang) * 2.0f, sinf(ang), 0.0f);
            if (w->glow_cur < 3)
                w->glow_cur++;
            else
                w->glow_cur = 0;
        }
    }
    list = dl_select_layer(1, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 29);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_translate3(0.6f, 2.6f, -5.0f);
    for (j = 0; j < 4; j++) {
        g = &w->glow[j];
        if (!(g->flags & 1) != 1) {
            s = dd_ease_sin(g->t, 12, 1) * 0.1f + 0.9f;
            list = dl_ambient_light(list, (unsigned int)(dd_ease_lin(g->t, 4) * 255.0f *
                                                         dd_ease_cos(g->t - 4, 8, 1)) << 24 | 0xFFFFFF);
            q = &g->p;
            list = dl_tex_offset(g->p.u + RANDF(), 0.0f, list);
            mtx_push();
            mtx_translate(&q->pos);
            mtx_rotate_z(q->angle);
            dd_scale3(s, s, 0.0f);
            list = draw_array(list, 5, 0x19F, 6, 0, w->glow_v);
            mtx_pop();
            if (g->t < 11)
                g->t++;
            else
                g->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    dd_mtx_pop_all();
    w->parity++;
    w->frame++;
    return 2;
}
