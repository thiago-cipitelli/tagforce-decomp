// @range 0x088370A4-0x08837B28
// @asm 0x08837674-0x08837AE0
/* Drawing entry points: each primitive is available inside a display list (flushing the dirty
   matrices first) and as an immediate call that closes the current sub-list, draws and reopens
   it; plus the view-frustum test of a bounding sphere and the matrix dirty flags. */
#include "psp.h"
#include "libc.h"

typedef struct DrawSphere {
    float pos[4];
    float radius;                      /* +0x10 */
    float fovy;                        /* +0x14 */
    float near;                        /* +0x18 */
    float far;                         /* +0x1C */
} DrawSphere;

extern int mtx_dirty[4];               /* 0x089050C0 */
extern int mtx_cur_mode;               /* 0x08905104 */

void *dl_get_layer(void);
void dl_set_list(void *p);
void *dl_select_layer(void *layer, unsigned int *size);
void dl_set_cur_list_base(void *p);
void *dl_get_cur_list_base(void);
void sceGuStart(int cid, void *list, int size);
int sceGuFinish(void);
void *mtx_flush(void *list);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void *dl_draw_array_n(void *list, int a, int b, int c, int d, int e, int f);
void *dl_draw_bezier(void *list, int a, int b, int c, int d, int e);
void *dl_draw_spline(void *list, int a, int b, int c, int d, int e, int f, int g);
void mtx_set_mode(int mode);
void mtx_get34(float *m);
void mat4_mul(float *out, float *a, float *b);
void mat4_transform_vec4(float *out, float *m, float *v);
float tanf_vfpu(float x);

void draw_array_now(int prim, int vtype, int count, void *indices, void *verts)
{
    void *layer;
    void *base;
    int n;
    void *layer2;
    void *list;
    void *buf;
    unsigned int size;

    layer = dl_get_layer();
    base = dl_get_cur_list_base();
    n = sceGuFinish();
    dl_set_list((char *)base + n - 8);
    list = dl_draw_array(mtx_flush(dl_select_layer(layer, 0)), prim, vtype, count, indices, verts);
    layer2 = dl_get_layer();
    dl_set_list(list);
    dl_set_cur_list_base(buf = dl_select_layer(layer2, &size));
    sceGuStart(2, buf, size);
}

void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts)
{
    return dl_draw_array(mtx_flush(list), prim, vtype, count, indices, verts);
}

void draw_0883A988_now(int a, int b, int c, int d, int e, int f)
{
    void *layer;
    void *base;
    int n;
    void *layer2;
    void *list;
    void *buf;
    unsigned int size;

    layer = dl_get_layer();
    base = dl_get_cur_list_base();
    n = sceGuFinish();
    dl_set_list((char *)base + n - 8);
    list = dl_draw_array_n(mtx_flush(dl_select_layer(layer, 0)), a, b, c, d, e, f);
    layer2 = dl_get_layer();
    dl_set_list(list);
    dl_set_cur_list_base(buf = dl_select_layer(layer2, &size));
    sceGuStart(2, buf, size);
}

void *draw_0883A988(void *list, int a, int b, int c, int d, int e, int f)
{
    return dl_draw_array_n(mtx_flush(list), a, b, c, d, e, f);
}

void draw_0883AAC4_now(int a, int b, int c, int d, int e)
{
    void *layer;
    void *base;
    int n;
    void *layer2;
    void *list;
    void *buf;
    unsigned int size;

    layer = dl_get_layer();
    base = dl_get_cur_list_base();
    n = sceGuFinish();
    dl_set_list((char *)base + n - 8);
    list = dl_draw_bezier(mtx_flush(dl_select_layer(layer, 0)), a, b, c, d, e);
    layer2 = dl_get_layer();
    dl_set_list(list);
    dl_set_cur_list_base(buf = dl_select_layer(layer2, &size));
    sceGuStart(2, buf, size);
}

void *draw_0883AAC4(void *list, int a, int b, int c, int d, int e)
{
    return dl_draw_bezier(mtx_flush(list), a, b, c, d, e);
}

void draw_0883ABC0_now(int a, int b, int c, int d, int e, int f, int g)
{
    void *layer;
    void *base;
    int n;
    void *layer2;
    void *list;
    void *buf;
    unsigned int size;

    layer = dl_get_layer();
    base = dl_get_cur_list_base();
    n = sceGuFinish();
    dl_set_list((char *)base + n - 8);
    list = dl_draw_spline(mtx_flush(dl_select_layer(layer, 0)), a, b, c, d, e, f, g);
    layer2 = dl_get_layer();
    dl_set_list(list);
    dl_set_cur_list_base(buf = dl_select_layer(layer2, &size));
    sceGuStart(2, buf, size);
}

void *draw_0883ABC0(void *list, int a, int b, int c, int d, int e, int f, int g)
{
    return dl_draw_spline(mtx_flush(list), a, b, c, d, e, f, g);
}

#ifdef NON_MATCHING /* reescrito do asm com os flags do original (bit 8 = passou em todos os planos;
   o resto é calculado e descartado): mesmo tamanho, 125 palavras de agendamento de float (o original
   calcula c[2]*c[2] uma vez e r2 antes de z; kx só depois dos dois primeiros planos) */
/* 1 when the sphere is inside the view frustum of the current projection and view matrices. */
int draw_sphere_visible(DrawSphere *s)
{
    float c[10];
    float pv[18];
    float v[4];
    float view[16];
    float proj[16];
    unsigned int flags;
    char *p;
    int n;
    float cot;
    float r2;
    float z;
    float ky, kx;
    float d, q;

    flags = 0;
    p = (char *)c;
    n = sizeof(c);
    if (p)
        do {
            *p++ = 0;
        } while (--n);
    c[0] = 1.0f;
    c[1] = 1.0f;
    c[3] = 480.0f;
    c[4] = 272.0f;
    cot = tanf_vfpu(s->fovy / 2.0f);
    c[5] = s->near;
    c[2] = 0.5f * (c[4] / cot);
    c[6] = s->far;
    c[7] = 4186116.0f + c[2] * c[2];
    c[8] = c[3] * c[3] / 4.0f + c[2] * c[2];
    c[9] = c[4] * c[4] / 4.0f + c[2] * c[2];
    mtx_set_mode(1);
    mtx_get34(proj);
    mtx_set_mode(2);
    mtx_get34(view);
    mat4_mul(pv, proj, view);
    mat4_transform_vec4(v, pv, s->pos);
    v[2] *= -1.0f;
    do {
        if (c[5] > v[2] + s->radius) {
            flags = 1;
            break;
        }
        if (c[5] > v[2] - s->radius)
            flags |= 3;
        if (c[6] < v[2] - s->radius) {
            flags = 1;
            break;
        }
        if (c[6] < v[2] + s->radius)
            flags |= 1;
        r2 = s->radius * s->radius;
        z = 2046.0f * v[2];
        ky = c[1] * (c[2] * v[1]);
        kx = c[2] * v[0];
        d = z - ky;
        q = d * d / c[7];
        if (d < 0.0f) {
            if (q >= r2) {
                flags = 1;
                break;
            }
            flags |= 5;
        } else if (q <= r2) {
            flags |= 5;
        }
        d = z + ky;
        q = d * d / c[7];
        if (d < 0.0f) {
            if (q >= r2) {
                flags = 1;
                break;
            }
            flags |= 5;
        } else if (q <= r2) {
            flags |= 5;
        }
        d = z - kx;
        q = d * d / c[7];
        if (d < 0.0f) {
            if (q >= r2) {
                flags = 1;
                break;
            }
            flags |= 5;
        } else if (q <= r2) {
            flags |= 5;
        }
        d = z + kx;
        q = d * d / c[7];
        if (d < 0.0f) {
            if (q >= r2) {
                flags = 1;
                break;
            }
            flags |= 5;
        } else if (q <= r2) {
            flags |= 5;
        }
        z = 0.5f * (v[2] * c[4]);
        d = z - ky;
        q = d * d / c[9];
        if (d < 0.0f) {
            if (q >= r2) {
                flags |= 4;
                break;
            }
            flags |= 4;
        } else if (q <= r2) {
            flags |= 4;
        }
        d = z + ky;
        q = d * d / c[9];
        if (d < 0.0f) {
            if (q >= r2) {
                flags |= 4;
                break;
            }
            flags |= 4;
        } else if (q <= r2) {
            flags |= 4;
        }
        z = 0.5f * (v[2] * c[3]);
        d = z - kx;
        q = d * d / c[8];
        if (d < 0.0f) {
            if (q >= r2) {
                flags |= 4;
                break;
            }
            flags |= 4;
        } else if (q <= r2) {
            flags |= 4;
        }
        d = z + kx;
        q = d * d / c[8];
        if (d < 0.0f) {
            if (q >= r2) {
                flags |= 4;
                break;
            }
            flags |= 4;
        } else if (q <= r2) {
            flags |= 4;
        }
        flags |= 8;
    } while (0);
    return (flags & 8) != 0;
}

#endif

void mtx_mark_cur_dirty(void)
{
    mtx_dirty[mtx_cur_mode] = 1;
}

void mtx_clear_dirty(void)
{
    mtx_dirty[3] = 0;
    mtx_dirty[2] = 0;
    mtx_dirty[1] = 0;
    mtx_dirty[0] = 0;
}
