// @range 0x088362E8-0x088370A4
// @data 0x088EDFC4-0x088EDFD4
/* Matrix stacks, one per GE matrix (0 projection, 1 world, 2 view, 3 texture), in a pool given at
   init. The current matrix of the current stack lives in the VFPU (M300) between calls: the
   transforms multiply it in place and mark the stack dirty, and mtx_flush writes the dirty ones
   into the display list. Outside the PSP build the VFPU state is the C global `vfpu_m300`. */
#include "psp.h"
#include "libc.h"

typedef struct Mat4 {
    float m[16];                       /* column-major, as in the VFPU */
} Mat4;

typedef struct Vec4 {
    float x, y, z, w;
} Vec4;

typedef struct MtxSlot {               /* 0x089050D0 + mode * 12 */
    float *base;
    float *end;
    float *cur;                        /* saved top while another stack is current */
} MtxSlot;

typedef struct MtxState {              /* mtx_state_save / mtx_state_load */
    int mode;
    float *top;
    MtxSlot slots[4];                  /* +0x08 */
    int dirty[4];                      /* +0x38 */
} MtxState;

typedef struct MtxProj {
    float fovy;
    float aspect;
    float near;
    float far;
} MtxProj;

extern int mtx_dirty[4];               /* 0x089050C0 */
extern MtxSlot mtx_slots[4];           /* 0x089050D0 */
extern float *mtx_stack_top;           /* 0x08905100 */
extern int mtx_cur_mode;               /* 0x08905104 */
extern Vec4 mtx_view_dir;              /* 0x089050B0 */

MtxProj mtx_proj = { 0.265115499f, 1.7647059f, 1.0f, 1000.0f };

unsigned int *dl_set_matrix(unsigned int *list, int type, float *m);
int dl_get_layer(void);
void *dl_get_cur_list_base(void);
void dl_set_list(void *p);
void *dl_select_layer(int layer, unsigned int *size);
void dl_set_cur_list_base(void *p);
void sceGuStart(int cid, void *list, int size);
int sceGuFinish(void);
float *vec3_sub(float *out, float *a, float *b);

void mtx_identity(void);
void mtx_set_view_dir(Vec4 *v);

#ifdef __MWERKS__
#define MTX_SAVE(p) asm { sv.q C300, 0(p); sv.q C310, 0x10(p); sv.q C320, 0x20(p); sv.q C330, 0x30(p) }
#define MTX_LOAD(p) asm { lv.q C300, 0(p); lv.q C310, 0x10(p); lv.q C320, 0x20(p); lv.q C330, 0x30(p) }
#else
float cosf(float x);
float sinf(float x);
float sqrtf(float x);

Mat4 vfpu_m300;
#define MTX_SAVE(p) (*(Mat4 *)(p) = vfpu_m300)
#define MTX_LOAD(p) (vfpu_m300 = *(Mat4 *)(p))

/* r = a * b (column-major) */
static void mat4_mul(Mat4 *r, Mat4 *a, Mat4 *b)
{
    Mat4 t;
    int i;
    int j;
    int k;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            float s = 0.0f;

            for (k = 0; k < 4; k++) {
                s += a->m[k * 4 + j] * b->m[i * 4 + k];
            }
            t.m[i * 4 + j] = s;
        }
    }
    *r = t;
}

static void mtx_mul_cur(Mat4 *m)
{
    mat4_mul(&vfpu_m300, &vfpu_m300, m);
}

static void mat4_set_identity(Mat4 *m)
{
    int i;

    for (i = 0; i < 16; i++) {
        m->m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
}

/* Rotation by `a` radians about axis 0 (x), 1 (y) or 2 (z). */
static void mtx_rotate_axis(int axis, float a)
{
    Mat4 r;
    float c = cosf(a);
    float s = sinf(a);
    int u = (axis + 1) % 3;
    int v = (axis + 2) % 3;

    mat4_set_identity(&r);
    r.m[u * 4 + u] = c;
    r.m[u * 4 + v] = s;
    r.m[v * 4 + u] = -s;
    r.m[v * 4 + v] = c;
    mtx_mul_cur(&r);
}
#endif

/* Saves the current matrix and writes the matrix of stack `mode` (if it exists) into the list. */
unsigned int *mtx_emit(unsigned int *list, int mode)
{
    register float *p = mtx_stack_top;

    MTX_SAVE(p);
    mtx_slots[mtx_cur_mode].cur = mtx_stack_top;
    if (mtx_slots[mode].cur != 0) {
        list = dl_set_matrix(list, mode, mtx_slots[mode].cur);
    }
    return list;
}

unsigned int *mtx_flush(unsigned int *list)
{
    int i = 0;
    int *d = mtx_dirty;

    do {
        if (*d != 0) {
            list = mtx_emit(list, i);
            *d = 0;
        }
        i++;
        d++;
    } while (i < 4);
    return list;
}

/* Carves the stacks out of `pool` (n0..n3 matrices for projection, world, view and texture), each
   starting with an identity; the projection is always marked dirty. */
void mtx_init(Mat4 *pool, int n0, int n1, int n2, int n3)
{
    register float *p;

    mtx_stack_top = 0;
    if (n3 != 0) {
        mtx_slots[3].base = (float *)(pool + (n2 + (n0 + n1)));
        mtx_slots[3].cur = (float *)(pool + (n2 + (n0 + n1)));
        mtx_slots[3].end = (float *)(pool + (n3 + (n2 + (n0 + n1))));
        mtx_stack_top = (float *)(pool + (n2 + (n0 + n1)));
        mtx_cur_mode = 3;
        mtx_identity();
        p = mtx_stack_top;
        MTX_SAVE(p);
        mtx_dirty[3] = 1;
    } else {
        mtx_slots[3].base = 0;
        mtx_slots[3].cur = 0;
        mtx_slots[3].end = 0;
        mtx_dirty[3] = 0;
    }
    if (n2 != 0) {
        mtx_slots[2].base = (float *)(pool + (n0 + n1));
        mtx_slots[2].cur = (float *)(pool + (n0 + n1));
        mtx_slots[2].end = (float *)(pool + (n2 + (n0 + n1)));
        mtx_stack_top = (float *)(pool + (n0 + n1));
        mtx_cur_mode = 2;
        mtx_identity();
        p = mtx_stack_top;
        MTX_SAVE(p);
        mtx_dirty[2] = 1;
    } else {
        mtx_slots[2].base = 0;
        mtx_slots[2].cur = 0;
        mtx_slots[2].end = 0;
        mtx_dirty[2] = 0;
    }
    if (n1 != 0) {
        mtx_slots[1].base = (float *)(pool + n0);
        mtx_slots[1].cur = (float *)(pool + n0);
        mtx_slots[1].end = (float *)(pool + (n0 + n1));
        mtx_stack_top = (float *)(pool + n0);
        mtx_cur_mode = 1;
        mtx_identity();
        p = mtx_stack_top;
        MTX_SAVE(p);
        mtx_dirty[1] = 1;
    } else {
        mtx_slots[1].base = 0;
        mtx_slots[1].cur = 0;
        mtx_slots[1].end = 0;
        mtx_dirty[1] = 0;
    }
    if (n0 != 0) {
        mtx_slots[0].base = (float *)pool;
        mtx_slots[0].cur = (float *)pool;
        mtx_slots[0].end = (float *)(pool + n0);
        mtx_stack_top = (float *)pool;
        mtx_cur_mode = 0;
        mtx_identity();
        p = mtx_stack_top;
        MTX_SAVE(p);
        mtx_dirty[0] = 1;
    } else {
        mtx_slots[0].base = 0;
        mtx_slots[0].cur = 0;
        mtx_slots[0].end = 0;
        mtx_dirty[0] = 1;
    }
    mtx_cur_mode = 0;
}

/* Writes the current matrix into the GE list right away (outside the drawing code). */
void mtx_emit_now(void)
{
    int layer;
    void *base;
    int n;
    unsigned int *list;
    int layer2;
    void *buf;
    unsigned int size;

    layer = dl_get_layer();
    base = dl_get_cur_list_base();
    n = sceGuFinish();
    dl_set_list((char *)base + n - 8);
    list = mtx_emit(dl_select_layer(layer, 0), mtx_cur_mode);
    layer2 = dl_get_layer();
    dl_set_list(list);
    dl_set_cur_list_base(buf = dl_select_layer(layer2, &size));
    sceGuStart(2, buf, size);
}

unsigned int *mtx_commit(unsigned int *list)
{
    return mtx_emit(list, mtx_cur_mode);
}

/* Same with every dirty matrix. */
void mtx_flush_now(void)
{
    int layer;
    void *base;
    int n;
    unsigned int *list;
    int layer2;
    void *buf;
    unsigned int size;

    layer = dl_get_layer();
    base = dl_get_cur_list_base();
    n = sceGuFinish();
    dl_set_list((char *)base + n - 8);
    list = mtx_flush(dl_select_layer(layer, 0));
    layer2 = dl_get_layer();
    dl_set_list(list);
    dl_set_cur_list_base(buf = dl_select_layer(layer2, &size));
    sceGuStart(2, buf, size);
}

unsigned int *mtx_flush_alias(unsigned int *list)
{
    return mtx_flush(list);
}

void mtx_state_save(MtxState *s)
{
    register float *p = mtx_stack_top;

    MTX_SAVE(p);
    s->mode = mtx_cur_mode;
    s->top = mtx_stack_top;
    s->slots[0].base = mtx_slots[0].base;
    s->slots[0].end = mtx_slots[0].end;
    s->slots[0].cur = mtx_slots[0].cur;
    s->dirty[0] = mtx_dirty[0];
    s->slots[1].base = mtx_slots[1].base;
    s->slots[1].end = mtx_slots[1].end;
    s->slots[1].cur = mtx_slots[1].cur;
    s->dirty[1] = mtx_dirty[1];
    s->slots[2].base = mtx_slots[2].base;
    s->slots[2].end = mtx_slots[2].end;
    s->slots[2].cur = mtx_slots[2].cur;
    s->dirty[2] = mtx_dirty[2];
    s->slots[3].base = mtx_slots[3].base;
    s->slots[3].end = mtx_slots[3].end;
    s->slots[3].cur = mtx_slots[3].cur;
    s->dirty[3] = mtx_dirty[3];
}

void mtx_state_load(MtxState *s)
{
    register float *p;

    mtx_cur_mode = s->mode;
    mtx_stack_top = p = s->top;
    mtx_slots[0].base = s->slots[0].base;
    mtx_slots[0].end = s->slots[0].end;
    mtx_slots[0].cur = s->slots[0].cur;
    mtx_dirty[0] = s->dirty[0];
    mtx_slots[1].base = s->slots[1].base;
    mtx_slots[1].end = s->slots[1].end;
    mtx_slots[1].cur = s->slots[1].cur;
    mtx_dirty[1] = s->dirty[1];
    mtx_slots[2].base = s->slots[2].base;
    mtx_slots[2].end = s->slots[2].end;
    mtx_slots[2].cur = s->slots[2].cur;
    mtx_dirty[2] = s->dirty[2];
    mtx_slots[3].base = s->slots[3].base;
    mtx_slots[3].end = s->slots[3].end;
    mtx_slots[3].cur = s->slots[3].cur;
    mtx_dirty[3] = s->dirty[3];
    MTX_LOAD(p);
}

void mtx_identity(void)
{
#ifdef __MWERKS__
    asm { vmidt.q E300 }
#else
    mat4_set_identity(&vfpu_m300);
#endif
    mtx_dirty[mtx_cur_mode] = 1;
}

void mtx_rotate_x(float a)
{
#ifdef __MWERKS__
    asm {
        mfc1 t0, $f12
        mtv t0, S100
        vcst.s S101, VFPU_2_PI
        vmul.s S100, S100, S101
        vidt.q C000
        vrot.q C010, S100, [0, c, s, 0]
        vrot.q C020, S100, [0, -s, c, 0]
        vidt.q C030
        vmmul.q E200, E000, E300
        nop
        vmmov.q E300, E200
    }
#else
    mtx_rotate_axis(0, a);
#endif
    mtx_dirty[mtx_cur_mode] = 1;
}

void mtx_rotate_y(float a)
{
#ifdef __MWERKS__
    asm {
        mfc1 t0, $f12
        mtv t0, S100
        vcst.s S101, VFPU_2_PI
        vmul.s S100, S100, S101
        vrot.q C000, S100, [c, 0, -s, 0]
        vidt.q C010
        vrot.q C020, S100, [s, 0, c, 0]
        vidt.q C030
        vmmul.q E200, E000, E300
        nop
        vmmov.q E300, E200
    }
#else
    mtx_rotate_axis(1, a);
#endif
    mtx_dirty[mtx_cur_mode] = 1;
}

void mtx_rotate_z(float a)
{
#ifdef __MWERKS__
    asm {
        mfc1 t0, $f12
        mtv t0, S100
        vcst.s S101, VFPU_2_PI
        vmul.s S100, S100, S101
        vrot.q C000, S100, [c, s, 0, 0]
        vrot.q C010, S100, [-s, c, 0, 0]
        vidt.q C020
        vidt.q C030
        vmmul.q E200, E000, E300
        nop
        vmmov.q E300, E200
    }
#else
    mtx_rotate_axis(2, a);
#endif
    mtx_dirty[mtx_cur_mode] = 1;
}

/* Rotations about x, then y, then z by the angles in v. */
void mtx_rotate_xyz(float *v)
{
#ifdef __MWERKS__
    asm {
        lv.s S100, 0(a0)
        lv.s S101, 4(a0)
        lv.s S102, 8(a0)
        vcst.s S110, VFPU_2_PI
        vscl.t C100, C100, S110
        vidt.q C000
        vrot.q C010, S100, [0, c, s, 0]
        vrot.q C020, S100, [0, -s, c, 0]
        vidt.q C030
        vmmul.q E200, E000, E300
        nop
        vrot.q C000, S101, [c, 0, -s, 0]
        vidt.q C010
        vrot.q C020, S101, [s, 0, c, 0]
        vidt.q C030
        vmmul.q E300, E000, E200
        nop
        vrot.q C000, S102, [c, s, 0, 0]
        vrot.q C010, S102, [-s, c, 0, 0]
        vidt.q C020
        vidt.q C030
        vmmul.q E200, E000, E300
        nop
        vmmov.q E300, E200
    }
#else
    mtx_rotate_axis(0, v[0]);
    mtx_rotate_axis(1, v[1]);
    mtx_rotate_axis(2, v[2]);
#endif
    mtx_dirty[mtx_cur_mode] = 1;
}

/* Rotations about z, then y, then x. */
void mtx_rotate_zyx(float *v)
{
#ifdef __MWERKS__
    asm {
        lv.s S100, 0(a0)
        lv.s S101, 4(a0)
        lv.s S102, 8(a0)
        vcst.s S110, VFPU_2_PI
        vscl.t C100, C100, S110
        vrot.q C000, S102, [c, s, 0, 0]
        vrot.q C010, S102, [-s, c, 0, 0]
        vidt.q C020
        vidt.q C030
        vmmul.q E200, E000, E300
        nop
        vrot.q C000, S101, [c, 0, -s, 0]
        vidt.q C010
        vrot.q C020, S101, [s, 0, c, 0]
        vidt.q C030
        vmmul.q E300, E000, E200
        vidt.q C000
        vrot.q C010, S100, [0, c, s, 0]
        vrot.q C020, S100, [0, -s, c, 0]
        vidt.q C030
        vmmul.q E200, E000, E300
        nop
        vmmov.q E300, E200
    }
#else
    mtx_rotate_axis(2, v[2]);
    mtx_rotate_axis(1, v[1]);
    mtx_rotate_axis(0, v[0]);
#endif
    mtx_dirty[mtx_cur_mode] = 1;
}

void mtx_translate(float *v)
{
#ifdef __MWERKS__
    asm {
        lv.s S000, 0(a0)
        lv.s S001, 4(a0)
        lv.s S002, 8(a0)
        vhtfm4.q C010, E300, C000
        vmov.q C330, C010
    }
#else
    int i;

    for (i = 0; i < 4; i++) {
        vfpu_m300.m[12 + i] += vfpu_m300.m[i] * v[0] + vfpu_m300.m[4 + i] * v[1] + vfpu_m300.m[8 + i] * v[2];
    }
#endif
    mtx_dirty[mtx_cur_mode] = 1;
}

void mtx_scale3(float *v)
{
#ifdef __MWERKS__
    asm {
        lv.s S000, 0(a0)
        lv.s S001, 4(a0)
        lv.s S002, 8(a0)
        vscl.t C300, C300, S000
        vscl.t C310, C310, S001
        vscl.t C320, C320, S002
    }
#else
    int i;

    for (i = 0; i < 3; i++) {
        vfpu_m300.m[i] *= v[0];
        vfpu_m300.m[4 + i] *= v[1];
        vfpu_m300.m[8 + i] *= v[2];
    }
#endif
    mtx_dirty[mtx_cur_mode] = 1;
}

/* Multiplies the current matrix by m. */
void mtx_mult(Mat4 *m)
{
#ifdef __MWERKS__
    asm {
        lv.q C000, 0(a0)
        lv.q C010, 0x10(a0)
        lv.q C020, 0x20(a0)
        lv.q C030, 0x30(a0)
        vmmul.q E200, E000, E300
        nop
        vmmov.q E300, E200
    }
#else
    mtx_mul_cur(m);
#endif
    mtx_dirty[mtx_cur_mode] = 1;
}

/* Parameters of the last mtx_perspective. */
void mtx_get_proj(float *fovy, float *aspect, float *near, float *far)
{
    if (fovy != 0) {
        *fovy = mtx_proj.fovy;
    }
    if (aspect != 0) {
        *aspect = mtx_proj.aspect;
    }
    if (near != 0) {
        *near = mtx_proj.near;
    }
    if (far != 0) {
        *far = mtx_proj.far;
    }
}

/* Multiplies by a perspective projection (fovy in radians) and remembers the parameters. */
void mtx_perspective(float fovy, float aspect, float near, float far)
{
#ifdef __MWERKS__
    asm {
        vmzero.q E100
        vcst.s S010, VFPU_1_PI
        mfc1 t0, $f12
        mfc1 t1, $f13
        mfc1 t2, $f14
        mfc1 t3, $f15
        mtv t0, S000
        mtv t1, S001
        mtv t2, S002
        mtv t3, S003
        vmul.s S000, S000, S010
        vrcp.s S001, S001
        vsub.s S010, S002, S003
        vadd.s S011, S002, S003
        vmul.s S012, S002, S003
        vrot.p C020, S000, [c, s]
        vrcp.s S021, S021
        vrcp.s S010, S010
        vmul.s S122, S010, S011
        vmul.s S013, S012, S010
        vadd.s S132, S013, S013
        vmul.s S111, S020, S021
        vmul.s S100, S111, S001
        vfim.s S123, -1.0
        vzero.s S133
        vmmul.q E200, E100, E300
        nop
        vmmov.q E300, E200
    }
#else
    Mat4 p;
    float ct = cosf(fovy * 0.5f) / sinf(fovy * 0.5f);

    memset(&p, 0, sizeof(p));
    p.m[0] = ct / aspect;
    p.m[5] = ct;
    p.m[10] = (far + near) / (near - far);
    p.m[11] = -1.0f;
    p.m[14] = 2.0f * far * near / (near - far);
    mtx_mul_cur(&p);
#endif
    mtx_proj.fovy = fovy;
    mtx_proj.aspect = aspect;
    mtx_proj.near = near;
    mtx_proj.far = far;
}

#ifdef __MWERKS__
asm void mtx_ortho(float left, float right, float bottom, float top, float near, float far)
{
    vmidt.q E100
    mfc1 t0, $f12
    mfc1 t1, $f13
    mfc1 t2, $f14
    mfc1 t3, $f15
    mfc1 t4, $f16
    mfc1 t5, $f17
    mtv t0, S000
    mtv t1, S001
    mtv t2, S002
    mtv t3, S003
    mtv t4, S010
    mtv t5, S011
    vfim.s S012, 2.0
    vbfy1.q C000, C000
    vbfy1.p C010, C010
    vrcp.s S001, S001
    vrcp.s S003, S003
    vrcp.s S011, S011
    vpfxs -X, X, X, X
    vmul.s S100, S012, S001
    vpfxs -X, X, X, X
    vmul.s S111, S012, S003
    vmul.s S122, S012, S011
    vmul.s S130, S000, S001
    vmul.s S131, S002, S003
    vmul.s S132, S010, S011
    vmmul.q E200, E100, E300
    jr ra
    vmmov.q E300, E200
}
#else
/* Note: no dirty mark, as in the original. */
void mtx_ortho(float left, float right, float bottom, float top, float near, float far)
{
    Mat4 o;

    mat4_set_identity(&o);
    o.m[0] = 2.0f / (right - left);
    o.m[5] = 2.0f / (top - bottom);
    o.m[10] = -2.0f / (far - near);
    o.m[12] = -(right + left) / (right - left);
    o.m[13] = -(top + bottom) / (top - bottom);
    o.m[14] = -(far + near) / (far - near);
    mtx_mul_cur(&o);
}
#endif

void mtx_set_view_dir(Vec4 *v)
{
    mtx_view_dir = *v;
}

void mtx_get_view_dir(Vec4 *out)
{
    *out = mtx_view_dir;
}

/* Multiplies by a look-at view matrix and records the viewing direction (target - eye). */
void mtx_look_at(float *eye, float *target, float *up)
{
    register float *t = target;
    Vec4 dir;

#ifdef __MWERKS__
    asm {
        lv.s S000, 0(a0)
        lv.s S001, 4(a0)
        lv.s S002, 8(a0)
        lv.s S010, 0(t)
        lv.s S011, 4(t)
        lv.s S012, 8(t)
        lv.s S020, 0(a2)
        lv.s S021, 4(a2)
        lv.s S022, 8(a2)
        vsub.q R102, C000, C010
        vdot.t S200, R102, R102
        vzero.s S203
        vcmp.s ez, S200, S200
        nop
        vrsq.s S200, S200
        vcmovt.s S200, S203, 0
        .word 0xDE00003F
        vscl.t R102, R102, S200
        vcrsp.t R100, C020, R102
        vcrsp.t R101, R102, R100
        vdot.t S200, R100, R100
        vdot.t S201, R101, R101
        vcmp.p ez, C200, C200
        nop
        vrsq.p C200, C200
        vcmovt.s S200, S203, 0
        vcmovt.s S201, S203, 1
        .word 0xDE00003F
        vscl.t R100, R100, S200
        .word 0xDE00003F
        vscl.t R101, R101, S201
        vidt.q R103
        vzero.t C130
        vmmul.q E200, E100, E300
        nop
        vmmov.q E300, E200
        vneg.t C100, C000
        vhtfm4.q C110, E300, C100
        vmov.q C330, C110
    }
#else
    /* ponytail: plain gluLookAt math, check against the VFPU version in phase 5 */
    float f[3];
    float s[3];
    float u[3];
    float n;
    Mat4 m;
    int i;

    for (i = 0; i < 3; i++) {
        f[i] = eye[i] - t[i];
    }
    n = sqrtf(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    if (n != 0.0f) {
        for (i = 0; i < 3; i++) {
            f[i] /= n;
        }
    }
    s[0] = up[1] * f[2] - up[2] * f[1];
    s[1] = up[2] * f[0] - up[0] * f[2];
    s[2] = up[0] * f[1] - up[1] * f[0];
    u[0] = f[1] * s[2] - f[2] * s[1];
    u[1] = f[2] * s[0] - f[0] * s[2];
    u[2] = f[0] * s[1] - f[1] * s[0];
    n = sqrtf(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
    if (n != 0.0f) {
        for (i = 0; i < 3; i++) {
            s[i] /= n;
        }
    }
    n = sqrtf(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
    if (n != 0.0f) {
        for (i = 0; i < 3; i++) {
            u[i] /= n;
        }
    }
    mat4_set_identity(&m);
    for (i = 0; i < 3; i++) {
        m.m[i * 4 + 0] = s[i];
        m.m[i * 4 + 1] = u[i];
        m.m[i * 4 + 2] = f[i];
    }
    mtx_mul_cur(&m);
    for (i = 0; i < 4; i++) {
        vfpu_m300.m[12 + i] += -(vfpu_m300.m[i] * eye[0] + vfpu_m300.m[4 + i] * eye[1] + vfpu_m300.m[8 + i] * eye[2]);
    }
#endif
    vec3_sub(&dir.x, eye, t);
    dir.w = 0.0f;
    mtx_set_view_dir(&dir);
}

/* Makes stack `mode` current (its top goes into the VFPU); the current one keeps its top. */
void mtx_set_mode(int mode)
{
    register float *p = mtx_stack_top;

    MTX_SAVE(p);
    mtx_slots[mtx_cur_mode].cur = mtx_stack_top;
    if (mtx_slots[mode].cur != 0) {
        mtx_cur_mode = mode;
        mtx_stack_top = p = mtx_slots[mode].cur;
        MTX_LOAD(p);
    }
}

void mtx_push(void)
{
    register float *p = mtx_stack_top;

    if (p + 16 < mtx_slots[mtx_cur_mode].end) {
        MTX_SAVE(p);
        mtx_stack_top += 16;
    }
}

void mtx_pop(void)
{
    int mode = mtx_cur_mode;
    register float *p = mtx_stack_top;

    if (p != mtx_slots[mode].base) {
        mtx_stack_top = p = p - 16;
        MTX_LOAD(p);
        mtx_dirty[mode] = 1;
    }
}

void mtx_load(Mat4 *m)
{
    register Mat4 *p = m;

    MTX_LOAD(p);
    mtx_dirty[mtx_cur_mode] = 1;
}

#ifdef __MWERKS__
asm void mtx_get34(Mat4 *m)
{
    sv.q C300, 0(a0)
    sv.q C310, 0x10(a0)
    sv.q C320, 0x20(a0)
    jr ra
    sv.q C330, 0x30(a0)
}
#else
void mtx_get34(Mat4 *m)
{
    *m = vfpu_m300;
}
#endif

int mtx_get_mode(void)
{
    return mtx_cur_mode;
}
