// @range 0x0881D8A8-0x0881DE88
// @asm 0x0881D8A8-0x0881DA34
// @gcc 0x0881DA34-0x0881DAC0
// @gcc 0x0881DC60-0x0881DE30
/* VFPU camera and projection matrices: planar shadow onto n . p = 1, view (look-at), perspective,
   ortho, frustum, and the point projections. Column-major (m[4 * col + row]); functions with an
   output matrix return it. Hand-written VFPU code kept as whole `asm` functions; the functions the SDK
   compiled with GCC come from C (vfpu_proj_gcc.c). The port C of the VFPU functions is left out of that GCC
   object (`#elif !defined(__mips__)`), so its literal pool is exactly the original's (`@rodata`).
   Outside the PSP build each one has a C equivalent. */

typedef struct Vec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) Vec4;

typedef struct Mat4 {
    float m[16];
} __attribute__((aligned(16))) Mat4;

Mat4 *mat4_identity(Mat4 *m);
Mat4 *mat4_mul(Mat4 *out, Mat4 *a, Mat4 *b);
Mat4 *mat4_translate(Mat4 *out, Mat4 *m, Vec4 *v);
Mat4 *mat4_inverse_ortho(Mat4 *out, Mat4 *m);
Vec4 *vec3_cross(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec3_normalize2(Vec4 *out, Vec4 *v);

#ifndef __MWERKS__
#include <math.h>
#include <string.h>

/* v.xyz scaled to unit length (0 for a zero vector), clamped to [-1, 1] */
static void unit3(float *v)
{
    float l2 = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    float k = l2 == 0.0f ? 0.0f : 1.0f / sqrtf(l2);
    int i;

    for (i = 0; i < 3; i++) {
        v[i] *= k;
        v[i] = v[i] < -1.0f ? -1.0f : v[i] > 1.0f ? 1.0f : v[i];
    }
}

static void cross3(float *r, const float *a, const float *b)
{
    float x = a[1] * b[2] - a[2] * b[1];
    float y = a[2] * b[0] - a[0] * b[2];
    float z = a[0] * b[1] - a[1] * b[0];

    r[0] = x;
    r[1] = y;
    r[2] = z;
}
#endif

#ifdef NON_MATCHING /* GCC (vfpu_proj_gcc.c, does not match yet); 1.0f and -1.0f open the literal pool */
/* Projection onto the plane a*x + b*y + c*z = 1 from the light `l`: a point light when `point` is
   set, else a direction. */
Mat4 *mat4_shadow_plane(Mat4 *out, Vec4 *l, int point, float a, float b, float c)
{
    float *m = out->m;
    float lx, ly, lz;
    float d;
    float k;

    if (point) {
        lx = l->x;
        ly = l->y;
        lz = l->z;
        d = a * lx + b * ly + c * lz;
        k = 1.0f - d;
        m[0] = a * lx + k;
        m[4] = b * lx;
        m[8] = c * lx;
        m[12] = -lx;
        m[1] = a * ly;
        m[5] = b * ly + k;
        m[9] = c * ly;
        m[13] = -ly;
        m[2] = a * lz;
        m[6] = b * lz;
        m[10] = c * lz + k;
        m[14] = -lz;
        m[3] = a;
        m[7] = b;
        m[11] = c;
        m[15] = k - 1.0f;
    } else {
        lx = l->x;
        ly = l->y;
        lz = l->z;
        d = a * lx + b * ly + c * lz;
        k = -1.0f / d;
        m[3] = 0.0f;
        m[7] = 0.0f;
        m[11] = 0.0f;
        m[0] = k * (a * lx - d);
        m[4] = k * (b * lx);
        m[8] = k * (c * lx);
        m[12] = k * -lx;
        m[1] = k * (a * ly);
        m[5] = k * (b * ly - d);
        m[9] = k * (c * ly);
        m[13] = k * -ly;
        m[2] = k * (a * lz);
        m[6] = k * (b * lz);
        m[10] = k * (c * lz - d);
        m[14] = k * -lz;
        m[15] = k * -d;
    }
    return out;
}
#endif

/* View matrix of a camera at `eye` looking at `target`: rows x = unit(up cross z), y = z cross x,
   z = unit(eye - target), translation -R * eye. */
#ifdef __MWERKS__
Mat4 *mat4_view(Mat4 *out, Vec4 *eye, Vec4 *target, Vec4 *up);
#elif !defined(__mips__)
Mat4 *mat4_view(Mat4 *out, Vec4 *eye, Vec4 *target, Vec4 *up)
{
    float x[3];
    float y[3];
    float z[3];
    float *m = out->m;
    int i;

    z[0] = eye->x - target->x;
    z[1] = eye->y - target->y;
    z[2] = eye->z - target->z;
    unit3(z);
    cross3(x, &up->x, z);
    cross3(y, z, x);
    unit3(x);
    unit3(y);
    for (i = 0; i < 3; i++) {
        m[i * 4] = x[i];
        m[i * 4 + 1] = y[i];
        m[i * 4 + 2] = z[i];
        m[i * 4 + 3] = 0.0f;
    }
    m[12] = -(x[0] * eye->x + x[1] * eye->y + x[2] * eye->z);
    m[13] = -(y[0] * eye->x + y[1] * eye->y + y[2] * eye->z);
    m[14] = -(z[0] * eye->x + z[1] * eye->y + z[2] * eye->z);
    m[15] = 1.0f;
    return out;
}
#else
Mat4 *mat4_view(Mat4 *out, Vec4 *eye, Vec4 *target, Vec4 *up)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 0(%3)\n"
        "lv.q C020, 0(%4)\n"
        "vsub.q R102, C000, C010\n"
        "vdot.t S200, R102, R102\n"
        "vzero.s S203\n"
        "vcmp.s EZ, S200, S200\n"
        "vrsq.s S200, S200\n"
        "vcmovt.s S200, S203, 0\n"
        ".word 0xDE00083F  /* vpfxd [-1:1],[-1:1],[-1:1],m */\n"
        "vscl.t R102, R102, S200\n"
        "vcrsp.t R100, C020, R102\n"
        "vcrsp.t R101, R102, R100\n"
        "vdot.t S200, R100, R100\n"
        "vdot.t S201, R101, R101\n"
        "vcmp.p EZ, C200, C200\n"
        "vrsq.p C200, C200\n"
        "vcmovt.s S200, S203, 0\n"
        "vcmovt.s S201, S203, 1\n"
        ".word 0xDE00083F  /* vpfxd [-1:1],[-1:1],[-1:1],m */\n"
        "vscl.t R100, R100, S200\n"
        ".word 0xDE00083F  /* vpfxd [-1:1],[-1:1],[-1:1],m */\n"
        "vscl.t R101, R101, S201\n"
        "vidt.q R103\n"
        "vzero.t C130\n"
        "vneg.t C000, C000\n"
        "vhtfm4.q C010, E100, C000\n"
        "vmov.t C130, C010\n"
        "sv.q C100, 0(%1)\n"
        "sv.q C110, 16(%1)\n"
        "sv.q C120, 32(%1)\n"
        "sv.q C130, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (eye), "r" (target), "r" (up)
        : "memory");
    return out;
}
#endif

/* Perspective projection (fovy in radians), as gluPerspective. */
#ifdef __MWERKS__
asm Mat4 *mat4_perspective(Mat4 *out, float fovy, float aspect, float near, float far)
{
    addiu sp, sp, -16
    swc1 $f12, 0(sp)
    move v0, a0
    swc1 $f13, 4(sp)
    swc1 $f14, 8(sp)
    swc1 $f15, 12(sp)
    vmzero.q E100
    vcst.s S010, VFPU_1_PI
    lv.s S000, 0(sp)
    lv.s S001, 4(sp)
    lv.s S002, 8(sp)
    lv.s S003, 12(sp)
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
    sv.q C100, 0(a0)
    sv.q C110, 16(a0)
    sv.q C120, 32(a0)
    sv.q C130, 48(a0)
    jr ra
    addiu sp, sp, 16
}
#elif !defined(__mips__)
Mat4 *mat4_perspective(Mat4 *out, float fovy, float aspect, float near, float far)
{
    float ct = cosf(fovy * 0.5f) / sinf(fovy * 0.5f);

    memset(out, 0, sizeof(*out));
    out->m[0] = ct / aspect;
    out->m[5] = ct;
    out->m[10] = (far + near) / (near - far);
    out->m[11] = -1.0f;
    out->m[14] = 2.0f * far * near / (near - far);
    return out;
}
#else
Mat4 *mat4_perspective(Mat4 *out, float fovy, float aspect, float near, float far);
#endif

/* Orthographic projection, as glOrtho. */
#ifdef __MWERKS__
asm Mat4 *mat4_ortho(Mat4 *out, float left, float right, float bottom, float top, float near, float far)
{
    addiu sp, sp, -32
    swc1 $f12, 0(sp)
    move v0, a0
    swc1 $f13, 4(sp)
    swc1 $f14, 8(sp)
    swc1 $f15, 12(sp)
    swc1 $f16, 16(sp)
    swc1 $f17, 20(sp)
    vmidt.q E100
    lv.s S000, 0(sp)
    lv.s S001, 4(sp)
    lv.s S002, 8(sp)
    lv.s S003, 12(sp)
    lv.s S010, 16(sp)
    lv.s S011, 20(sp)
    vfim.s S012, 2.0
    vbfy1.q C000, C000
    vbfy1.p C010, C010
    vrcp.s S001, S001
    vrcp.s S003, S003
    vrcp.s S011, S011
    vpfxs -X, Y, Z, W
    vmul.s S100, S012, S001
    vpfxs -X, Y, Z, W
    vmul.s S111, S012, S003
    vmul.s S122, S012, S011
    vmul.s S130, S000, S001
    vmul.s S131, S002, S003
    vmul.s S132, S010, S011
    sv.q C100, 0(a0)
    sv.q C110, 16(a0)
    sv.q C120, 32(a0)
    sv.q C130, 48(a0)
    jr ra
    addiu sp, sp, 32
}
#elif !defined(__mips__)
Mat4 *mat4_ortho(Mat4 *out, float left, float right, float bottom, float top, float near, float far)
{
    memset(out, 0, sizeof(*out));
    out->m[0] = 2.0f / (right - left);
    out->m[5] = 2.0f / (top - bottom);
    out->m[10] = 2.0f / (near - far);
    out->m[12] = (left + right) / (left - right);
    out->m[13] = (bottom + top) / (bottom - top);
    out->m[14] = (near + far) / (near - far);
    out->m[15] = 1.0f;
    return out;
}
#else
Mat4 *mat4_ortho(Mat4 *out, float left, float right, float bottom, float top, float near, float far);
#endif

/* Perspective frustum, as glFrustum. */
#ifdef __MWERKS__
asm Mat4 *mat4_frustum(Mat4 *out, float left, float right, float bottom, float top, float near, float far)
{
    addiu sp, sp, -32
    swc1 $f12, 0(sp)
    move v0, a0
    swc1 $f13, 4(sp)
    swc1 $f14, 8(sp)
    swc1 $f15, 12(sp)
    swc1 $f16, 16(sp)
    swc1 $f17, 20(sp)
    vmzero.q E100
    lv.s S000, 4(sp)
    lv.s S001, 0(sp)
    lv.s S002, 12(sp)
    lv.s S003, 8(sp)
    lv.s S010, 16(sp)
    lv.s S011, 20(sp)
    vadd.s S012, S010, S010
    vmul.s S013, S010, S011
    vbfy1.q C000, C000
    vbfy1.p C010, C010
    vadd.s S013, S013, S013
    vrcp.s S001, S001
    vrcp.s S003, S003
    vrcp.s S011, S011
    vmul.s S100, S012, S001
    vmul.s S111, S012, S003
    vmul.s S120, S000, S001
    vmul.s S121, S002, S003
    vmul.s S122, S010, S011
    vfim.s S123, -1.0
    vmul.s S132, S013, S011
    sv.q C100, 0(a0)
    sv.q C110, 16(a0)
    sv.q C120, 32(a0)
    sv.q C130, 48(a0)
    jr ra
    addiu sp, sp, 32
}
#elif !defined(__mips__)
Mat4 *mat4_frustum(Mat4 *out, float left, float right, float bottom, float top, float near, float far)
{
    memset(out, 0, sizeof(*out));
    out->m[0] = 2.0f * near / (right - left);
    out->m[5] = 2.0f * near / (top - bottom);
    out->m[8] = (right + left) / (right - left);
    out->m[9] = (top + bottom) / (top - bottom);
    out->m[10] = (near + far) / (near - far);
    out->m[11] = -1.0f;
    out->m[14] = 2.0f * near * far / (near - far);
    return out;
}
#else
Mat4 *mat4_frustum(Mat4 *out, float left, float right, float bottom, float top, float near, float far);
#endif

/* View matrix from a position, a view direction and an up vector: the inverse of the camera matrix
   [unit(up cross dir), dir' cross x, dir' = unit(dir), pos]. */
#ifndef __MWERKS__ /* PSP: vfpu_proj_gcc.c */
Mat4 *mat4_look_at(Mat4 *out, Vec4 *pos, Vec4 *dir, Vec4 *up)
{
    Mat4 cam;
    Vec4 x;

    mat4_identity(&cam);
    vec3_cross(&x, up, dir);
    vec3_normalize2((Vec4 *)&cam.m[0], &x);
    vec3_normalize2((Vec4 *)&cam.m[8], dir);
    vec3_cross((Vec4 *)&cam.m[4], (Vec4 *)&cam.m[8], (Vec4 *)&cam.m[0]);
    mat4_translate(&cam, &cam, pos);
    mat4_inverse_ortho(out, &cam);
    return out;
}
#else
Mat4 *mat4_look_at(Mat4 *out, Vec4 *pos, Vec4 *dir, Vec4 *up);
#endif

#ifdef NON_MATCHING /* PSP: vfpu_proj_gcc.c */
/* Perspective with explicit depth mapping: out = T * P with P (x, y, z, 1) = (s x, s y, 1, z) and
   T = scale (sx, sy, A) + translation (tx, ty, B), A = far * near * (zmax - zmin) / (far - near),
   B = (zmin * far - zmax * near) / (far - near). */
Mat4 *mat4_perspective_ex(Mat4 *out, float s, float sx, float sy, float tx, float ty, float zmin, float zmax, float near, float far)
{
    Mat4 t;
    float b = (zmin * far - zmax * near) / (far - near);
    float a = far * near * (zmax - zmin) / (far - near);

    mat4_identity(out);
    out->m[14] = 1.0f;
    out->m[11] = 1.0f;
    out->m[0] = s;
    out->m[5] = s;
    out->m[10] = 0.0f;
    out->m[15] = 0.0f;
    mat4_identity(&t);
    t.m[0] = sx;
    t.m[5] = sy;
    t.m[10] = a;
    t.m[12] = tx;
    t.m[13] = ty;
    t.m[14] = b;
    mat4_mul(out, &t, out);
    return out;
}
#endif

/* out = (p.xyz / p.w, 1 / p.w) with p = m * v */
#ifdef __MWERKS__
Vec4 *mat4_project(Vec4 *out, Mat4 *m, Vec4 *v);
#elif !defined(__mips__)
Vec4 *mat4_project(Vec4 *out, Mat4 *m, Vec4 *v)
{
    float p[4];
    int i;

    for (i = 0; i < 4; i++) {
        p[i] = m->m[i] * v->x + m->m[4 + i] * v->y + m->m[8 + i] * v->z + m->m[12 + i] * v->w;
    }
    p[3] = 1.0f / p[3];
    out->x = p[0] * p[3];
    out->y = p[1] * p[3];
    out->z = p[2] * p[3];
    out->w = p[3];
    return out;
}
#else
Vec4 *mat4_project(Vec4 *out, Mat4 *m, Vec4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C100, 0(%2)\n"
        "lv.q C110, 16(%2)\n"
        "lv.q C120, 32(%2)\n"
        "lv.q C130, 48(%2)\n"
        "lv.q C200, 0(%3)\n"
        "vtfm4.q C000, E100, C200\n"
        "vrcp.s S003, S003\n"
        "vscl.t C000, C000, S003\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "r" (v)
        : "memory");
    return out;
}
#endif

/* Projects n points (n >= 1) and stores x, y, z / w as 12.4 fixed point (rounded to nearest) in
   three shorts, `stride` bytes apart. */
#ifdef __MWERKS__
asm int mat4_project_s16(short *out, int stride, Mat4 *m, Vec4 *v, int n)
{
    move v1, t0
    lv.q C000, 0(a2)
    lv.q C010, 16(a2)
    lv.q C020, 32(a2)
    lv.q C030, 48(a2)
L_0881DE44:
    lv.q C100, 0(a3)
    addiu a3, a3, 16
    vtfm4.q C200, E000, C100
    vrcp.s S203, S203
    vscl.t C200, C200, S203
    vf2in.t C200, C200, 4
    mfv t0, S200
    mfv t1, S201
    mfv t2, S202
    addiu v1, v1, -1
    sh t0, 0(a0)
    sh t1, 2(a0)
    sh t2, 4(a0)
    bne zero, v1, L_0881DE44
    addu a0, a0, a1
    jr ra
    move v0, zero
}
#elif !defined(__mips__)
int mat4_project_s16(short *out, int stride, Mat4 *m, Vec4 *v, int n)
{
    do {
        float p[4];
        int i;

        for (i = 0; i < 4; i++) {
            p[i] = m->m[i] * v->x + m->m[4 + i] * v->y + m->m[8 + i] * v->z + m->m[12 + i] * v->w;
        }
        for (i = 0; i < 3; i++) {
            out[i] = (short)(int)rintf(p[i] / p[3] * 16.0f);
        }
        v++;
        out = (short *)((char *)out + stride);
    } while (--n != 0);
    return 0;
}
#else
int mat4_project_s16(short *out, int stride, Mat4 *m, Vec4 *v, int n);
#endif
