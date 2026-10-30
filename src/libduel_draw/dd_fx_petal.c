// @range 0x08854C44-0x0885538C
// @asm 0x08854CD8-0x0885538C
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

float func_088279A4(int t, int len);
float func_088279D8(int t, int len);
float func_08827A2C(int t, int len, int n);
void func_08827840(void);
void func_08827668(void);
void func_08827288(FxMat4 *out);
int func_0882623C(void *update, void *a, void *b, int size);
void *func_08826440(int task);
void *func_0882698C(void *list, int on, int mask);
void *func_088269D8(void *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *func_08826A64(void *list, int tex);
void *func_088266D8(void *list, int on, int min, int mag, int func, int comp, int wrapu, int wrapv);

typedef struct RandSeed {              /* BOOT rand3_seeds entry, 12 bytes */
    unsigned int lcg;
    unsigned int b, c;
} RandSeed;
extern int rand3_slot;                 /* BOOT variable imports, 0 in the relocated ELF */
extern RandSeed rand3_seeds[];

#define RAND15() (rand3_seeds[rand3_slot].lcg *= 0x41C64E6D, rand3_seeds[rand3_slot].lcg += 0x3039, \
                  (int)(rand3_seeds[rand3_slot].lcg >> 16 & 0x7FFF))
#define RANDF() (RAND15() / 32767.0f)

typedef struct DdPetal {               /* 0x28 */
    unsigned int flags;
    int t;
    int cell;
    float spin;                        /* radians per frame */
    FxVec3 out;                        /* +0x10 burst direction */
    FxVec3 drift;                      /* +0x1C wind over the lifetime */
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
    int task = func_0882623C(dd_fx_petal_update, a, b, sizeof(DdFxPetal));
    DdFxPetal *w;

    if (task != -1) {
        FxVec3 pos = { 0.0f, 7.46299982f, -44.6949997f };

        w = func_08826440(task);
        w->arg = arg;
        w->pos = pos;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 1,7 KB; C equivalente */
int dd_fx_petal_update(int phase, DdFxPetal *w)
{
    FxMat4 cam;
    void *list;
    DdPetal *p;
    FxVec3 a;
    FxVec3 b;
    FxVec3 q;
    float r;
    float ang;
    float sign;
    float f;
    int n;
    int i;

    func_08827840();
    func_08827288(&cam);
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            f = (float)(i % 4) * 0.25f;
            w->cell[i][0].u = f; w->cell[i][0].v = (float)(i / 4) * 0.5f;
            w->cell[i][0].x = -2.0f; w->cell[i][0].y = 2.0f; w->cell[i][0].z = 0.0f;
            w->cell[i][1].u = f; w->cell[i][1].v = (float)(i / 4) * 0.5f + 0.5f;
            w->cell[i][1].x = -2.0f; w->cell[i][1].y = -2.0f; w->cell[i][1].z = 0.0f;
            w->cell[i][2].u = f + 0.25f; w->cell[i][2].v = (float)(i / 4) * 0.5f;
            w->cell[i][2].x = 2.0f; w->cell[i][2].y = 2.0f; w->cell[i][2].z = 0.0f;
            w->cell[i][3].u = f + 0.25f; w->cell[i][3].v = (float)(i / 4) * 0.5f + 0.5f;
            w->cell[i][3].x = 2.0f; w->cell[i][3].y = -2.0f; w->cell[i][3].z = 0.0f;
        }
    }
    if (w->parity % 4 == 0) {
        n = phase == 1 ? 128 : 1;
        for (i = 0; i < n; i++) {
            p = &w->petal[w->cur];
            p->flags |= 1;
            p->t = (n - 1 - i) * 4;
            r = RANDF() * 0.5f;
            ang = RANDF() * 6.2831855f;
            p->cell = RAND15() % 8;
            sign = (float)((RAND15() & 1) * 2 - 1);
            p->spin = RANDF() * 0.017453292f * sign;
            vec3_set(&p->out, r * sinf(ang), 0.5f, r * cosf(ang));
            f = RANDF();
            vec3_set(&p->drift, RANDF() * 15.0f + 1.0f, f * 5.0f + 5.0f, 0.0f);
            if (w->cur < 127)
                w->cur++;
            else
                w->cur = 0;
        }
    }
    list = dl_select_layer(1, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
    list = func_0882698C(list, 1, 1);
    list = func_088269D8(list, 1, 0, 2, 3, 0, 0xFFFFFFFF);
    list = func_08826A64(list, 5);
    list = dl_color(list, 0xC8FFEBEB);
    list = dl_depth_range_i(list, -5);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(&w->pos);
    for (i = 0; i < 128; i++) {
        p = &w->petal[i];
        if (p->flags & 1) {
            f = func_088279A4(p->t, 512);
            r = func_08827A2C(p->t, 16, 1);
            list = dl_ambient_light(list, (unsigned int)(func_088279D8(p->t - 256, 256) * 255.0f) << 24 | 0xFFFFFF);
            vec3_scale(&a, &p->out, r * 2.0f + 1.0f);
            vec3_scale(&b, &p->drift, f);
            vec3_add(&q, &a, &b);
            mtx_push();
            mtx_translate(&q);
            mtx_mult(&cam);
            mtx_rotate_z(p->spin * (float)p->t);
            list = draw_array(list, 4, 0x183, 4, 0, w->cell[p->cell]);
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
    func_08827668();
    w->parity++;
    w->frame++;
    return 2;
}
#endif
