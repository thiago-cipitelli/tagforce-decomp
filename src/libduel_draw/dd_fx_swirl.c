// @range 0x0884F3D0-0x0884FD60
// @asm 0x0884F468-0x0884FB88
/* Screen transitions of the duel: a swirl that twists the captured screen (a 31x21 grid turned
   more towards the centre, sound 0x6248) or, for id 5788, a plain fade to/from black. */
#include "libduel_draw.h"

void *memcpy(void *, const void *, unsigned int);
void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_color(void *list, unsigned int color);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
float roundf_vfpu(float x);
void *mat3_identity(float *out);
void *mat3_rot_z(float *out, float *m, float angle);
FxVec3 *mat3_transform_vec3(FxVec3 *out, float *m, FxVec3 *v);
FxVec3 *vec3_set(FxVec3 *out, float x, float y, float z);

float func_088279A4(int t, int len);
float func_088279D8(int t, int len);
float func_08827A2C(int t, int len, int n);
float func_08827B6C(int t, int len, int n);
int func_08828290(void);
int func_08806434(int a, int b);
int func_08827F68(int se, int t, int at, unsigned int *flags, unsigned int bit);
int func_0882623C(void *update, void *a, void *b, int size);
void *func_08826440(int task);
void *func_0882698C(void *list, int on, int mask);
void *func_088269D8(void *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *func_088266D8(void *list, int on, int min, int mag, int func, int comp, int wrapu, int wrapv);
void *func_08826C80(void *list);

typedef struct FxVtxS {                /* GU_TEXTURE_16BIT | GU_VERTEX_16BIT, 2D */
    short u, v;
    short x, y, z;
} FxVtxS;

typedef struct DdFxSwirl {             /* 0x56C4 */
    int t;                             /* +0x0000 */
    int frame;                         /* +0x0004 */
    int out;                           /* +0x0008 1: untwist */
    unsigned int se;                   /* +0x000C */
    FxVtxS grid[21][31];               /* +0x0010 */
    FxVtxS buf[2][21][31];             /* +0x197E drawn copy, one per frame parity */
    unsigned char idx[62];             /* +0x4C5A strip of two rows */
    float twist[21][31];               /* +0x4C98 turn of each vertex */
} DdFxSwirl;

typedef struct DdFxBlack {             /* 0x0C */
    int t;
    int frame;
    int out;                           /* +0x08 0: to black */
} DdFxBlack;

int dd_fx_swirl_start(int out, void *a, void *b);
int dd_fx_swirl_update(int phase, DdFxSwirl *w);
int dd_fx_black_start(int out, void *a, void *b);
int dd_fx_black_update(int phase, DdFxBlack *w);

void dd_fx_transition(int out, int id, void *a, void *b)
{
    switch (id) {
    case 5788:
        dd_fx_black_start(out, a, b);
        break;
    default:
        dd_fx_swirl_start(out, a, b);
        break;
    }
}

int dd_fx_swirl_start(int out, void *a, void *b)
{
    int task = func_0882623C(dd_fx_swirl_update, a, b, sizeof(DdFxSwirl));

    if (task != -1)
        ((DdFxSwirl *)func_08826440(task))->out = out;
    return task;
}

#ifdef NON_MATCHING /* função de 1,8 KB: o original usa um registrador de float a menos e outra pilha; C equivalente */
int dd_fx_swirl_update(int phase, DdFxSwirl *w)
{
    int buf = w->frame % 2;
    int step = func_08828290();
    void *list;
    float k;
    float s;
    float y;
    float y2;
    float x;
    float d;
    float u;
    float a;
    unsigned int alpha;
    FxVec3 p;
    float m[9];
    int i;
    int j;
    int t;

    if (w->out == 0)
        k = func_08827A2C(w->t, 16, 1);
    else
        k = func_08827B6C(w->t, 16, 1);
    if (func_08806434(8, -1) == 0)
        func_08827F68(0x6248, w->t, 0, &w->se, 1);
    if (phase == 1) {
        for (i = 0; i < 21; i++) {
            y = ((float)i / 20.0f * 2.0f + -1.0f) * 136.0f;
            u = roundf_vfpu((float)i / 20.0f * 270.0f);
            y2 = y * y;
            for (j = 0; j < 31; j++) {
                x = ((float)j / 30.0f * 2.0f + -1.0f) * 240.0f;
                w->grid[i][j].u = roundf_vfpu((float)j / 30.0f * 478.0f) + 1.0f;
                w->grid[i][j].v = u + 1.0f;
                d = x * x + y2;
                w->grid[i][j].x = roundf_vfpu(x + 240.0f);
                w->grid[i][j].y = roundf_vfpu(y + 136.0f);
                w->grid[i][j].z = 0;
                if (d > 76096.0f)
                    d = 76096.0f;
                w->twist[i][j] = (76096.0f - d) * 0.27925268f / 76096.0f + 0.017453292f;
            }
        }
        for (i = 0; i < 31; i++) {
            w->idx[i * 2] = i;
            w->idx[i * 2 + 1] = i + 31;
        }
    }
    list = dl_select_layer(2, 0);
    list = func_0882698C(list, 0, 0);
    list = func_088269D8(list, 1, 0, 2, 3, 0, 0);
    list = func_08826C80(list);
    s = k;
    if (w->out != 0)
        s = 1.0f - k;
    s = s * 0.6f + 0.4f;
    for (i = 0; i < 21; i++) {
        y = ((float)i / 20.0f * 2.0f + -1.0f) * 136.0f * s;
        for (j = 0; j < 31; j++) {
            if (w->out == 0)
                a = w->twist[i][j] * 24.0f + (6.2831855f - w->twist[i][j] * 24.0f) * k;
            else
                a = w->twist[i][j] * 24.0f * k;
            vec3_set(&p, ((float)j / 30.0f * 2.0f + -1.0f) * 240.0f * s, y, 0.0f);
            mat3_identity(m);
            mat3_rot_z(m, m, a);
            mat3_transform_vec3(&p, m, &p);
            w->grid[i][j].x = roundf_vfpu(p.x + 240.0f);
            w->grid[i][j].y = roundf_vfpu(p.y + 136.0f);
        }
    }
    memcpy(w->buf[buf], w->grid, sizeof(w->grid));
    list = func_088266D8(list, 0, 1, 1, 0, 0, 1, 1);
    list = dl_color(list, 0xFF000000);
    list = draw_set_offsets(list, 0, 0, 480, 272);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    if (w->out == 0)
        alpha = func_088279A4(w->t, 8) * 255.0f;
    else
        alpha = (1.0f - func_088279A4(w->t - 8, 8)) * 255.0f;
    list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
    list = dl_color(list, alpha << 24 | 0xFFFFFF);
    for (i = 0; i < 20; i++)
        list = draw_array(list, 4, 0x800902, 62, w->idx, w->buf[buf][i]);
    dl_set_list(list);
    w->frame++;
    t = w->t;
    w->t = t + step;
    if (t + step < 16)
        return 2;
    return 4;
}
#endif

int dd_fx_black_start(int out, void *a, void *b)
{
    int task = func_0882623C(dd_fx_black_update, a, b, sizeof(DdFxBlack));

    if (task != -1)
        ((DdFxBlack *)func_08826440(task))->out = out;
    return task;
}

int dd_fx_black_update(int phase, DdFxBlack *w)
{
    int step = func_08828290();
    float a;
    void *list;
    int t;

    if (w->out == 0)
        a = func_088279D8(w->t, 60);
    else
        a = func_088279A4(w->t, 60);
    list = dl_select_layer(1, 0);
    list = func_0882698C(list, 0, 1);
    list = func_088266D8(list, 0, 1, 1, 0, 0, 1, 1);
    list = func_088269D8(list, 1, 0, 2, 3, 0, 0xFFFFFFFF);
    list = dl_color(list, (unsigned int)(a * 255.0f) << 24);
    list = draw_set_offsets(list, 0, 0, 480, 272);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    dl_set_list(list);
    w->frame++;
    t = w->t;
    w->t = t + step;
    if (t + step < 60)
        return 2;
    return 4;
}
