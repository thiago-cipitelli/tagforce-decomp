// @range 0x08854C44-0x0885538C
// @data 0x08876E20-0x08876E2C
/* Character effect "petals": 128 petals (PT88_R_a.gim) drifting from above a fixed point in front
   of the camera, spinning slowly and fading over 512 frames. */
#include "libduel_draw.h"

void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_color(void *list, unsigned int color);
void *dl_ambient_light(void *list, unsigned int color);
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
FxVec3 *vec3_scale(FxVec3 *out, FxVec3 *v, float s);
FxVec3 *vec3_add(FxVec3 *out, FxVec3 *a, FxVec3 *b);
float sinf(float x);
float cosf(float x);

float dd_ease_lin(int t, int len);
float dd_ease_lin_inv(int t, int len);
float dd_ease_sin(int t, int len, int n);
void dd_mtx_push_field(void);
void dd_mtx_pop_all(void);
void dd_mtx_billboard_field(FxMat4 *out);
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

#define RAND15() (rand3_seeds[rand3_slot].lcg *= 0x41C64E6D, \
                  (int)((rand3_seeds[rand3_slot].lcg += 0x3039) >> 16 & 0x7FFF))
#define RANDF() (RAND15() / 32767.0f)

typedef struct DdPetalPath {           /* +0x08 of DdPetal */
    int cell;
    float spin;                        /* radians per frame */
    FxVec3 out;                        /* +0x10 burst direction */
    FxVec3 drift;                      /* +0x1C wind over the lifetime */
} DdPetalPath;

typedef struct DdPetal {               /* 0x28 */
    unsigned int flags;
    int t;
    DdPetalPath m;
} DdPetal;

typedef struct DdFxPetal {             /* 0x169C */
    int arg;                           /* +0x000 */
    int frame;                         /* +0x004 */
    int parity;                        /* +0x008 */
    FxVec3 pos;                        /* +0x00C */
    int cur;                           /* +0x018 */
    FxVtxT cell[8][4];                 /* +0x01C */
    DdPetal petal[128];                /* +0x29C */
} DdFxPetal;

int dd_fx_petal_update(int phase, DdFxPetal *w);

int dd_fx_petal_start(int arg, void *a, void *b)
{
    int task = dd_task_create(dd_fx_petal_update, a, b, sizeof(DdFxPetal));
    DdFxPetal *w;

    if (task != -1) {
        FxVec3 pos = { 0.0f, 7.46299982f, -44.6949997f };

        w = dd_task_mem(task);
        w->arg = arg;
        w->pos = pos;
    }
    return task;
}

int dd_fx_petal_update(int phase, DdFxPetal *w)
{
    FxMat4 cam;
    void *list;
    DdPetal *p;
    DdPetalPath *m;
    FxVec3 a;
    FxVec3 b;
    FxVec3 q;
    float r;
    float ang;
    int k;
    float f;
    float v;
    float cw = 0.25f;
    float ch = 0.5f;
    float h = 2.0f;
    int n;
    int i;

    dd_mtx_push_field();
    dd_mtx_billboard_field(&cam);
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            f = (float)(i % 4) * cw;
            v = (float)(i / 4) * ch;
            w->cell[i][0].u = f; w->cell[i][0].v = v;
            w->cell[i][0].x = -h; w->cell[i][0].y = h; w->cell[i][0].z = 0.0f;
            w->cell[i][1].u = f; w->cell[i][1].v = v + ch;
            w->cell[i][1].x = -h; w->cell[i][1].y = -h; w->cell[i][1].z = 0.0f;
            w->cell[i][2].u = f + cw; w->cell[i][2].v = v;
            w->cell[i][2].x = h; w->cell[i][2].y = h; w->cell[i][2].z = 0.0f;
            w->cell[i][3].u = f + cw; w->cell[i][3].v = v + ch;
            w->cell[i][3].x = h; w->cell[i][3].y = -h; w->cell[i][3].z = 0.0f;
        }
    }
    if (w->parity % 4 == 0) {
        n = phase == 1 ? 128 : 1;
        for (i = 0; i < n; i++) {
            p = &w->petal[w->cur];
            p->flags |= 1;
            p->t = (n - 1 - i) * 4;
            m = &p->m;
            r = RANDF() * 0.5f;
            ang = RANDF() * 6.2831855f;
            m->cell = RAND15() % 8;
            k = RAND15();
            f = 0.017453292f * RANDF();
            m->spin = f * (float)(k % 2 * 2 - 1);
            vec3_set(&m->out, r * sinf(ang), 0.5f, r * cosf(ang));
            f = RANDF();
            vec3_set(&m->drift, RANDF() * 15.0f + 1.0f, f * 5.0f + 5.0f, 0.0f);
            if (w->cur < 127)
                w->cur++;
            else
                w->cur = 0;
        }
    }
    list = dl_select_layer(1, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 5);
    list = dl_color(list, 0xC8FFEBEB);
    list = dl_depth_range_i(list, -5);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(&w->pos);
    for (k = 0; k < 128; k++) {
        p = &w->petal[k];
        if (!(p->flags & 1) != 1) {
            f = dd_ease_lin(p->t, 512);
            r = dd_ease_sin(p->t, 16, 1) * 2.0f + 1.0f;
            list = dl_ambient_light(list, (unsigned int)(dd_ease_lin_inv(p->t - 256, 256) * 255.0f) << 24 | 0xFFFFFF);
            m = &p->m;
            vec3_scale(&a, &m->out, r);
            vec3_scale(&b, &m->drift, f);
            vec3_add(&q, &a, &b);
            mtx_push();
            mtx_translate(&q);
            mtx_mult(&cam);
            mtx_rotate_z(m->spin * (float)p->t);
            list = draw_array(list, 4, 0x183, 4, 0, w->cell[m->cell]);
            mtx_pop();
            if (p->t < 511)
                p->t++;
            else
                p->flags &= ~1;
        }
    }
    mtx_pop();
    list = dl_depth_range_i(list, 0);
    dl_set_list(list);
    dd_mtx_pop_all();
    w->parity++;
    w->frame++;
    return 2;
}
