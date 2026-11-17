// @range 0x0881DE88-0x0881EB0C
// @gcc 0x0881DE88-0x0881E0FC
// @asm 0x0881E0FC-0x0881E510
// @gcc 0x0881E524-0x0881E54C
// @gcc 0x0881E64C-0x0881E8F0
/* VFPU quaternions (x, y, z, w with w the scalar part; vqmul is the Hamilton product). Euler angles
   are radians in a vector (x, y, z); the _zyx/_xyz/_yxz suffix is the product order of the axis
   rotations (zyx: q = qz * qy * qx). Hand-written VFPU code kept as whole `asm` functions; the
   compiled C ones that load float constants from .rodata (GCC) come from vfpu_quat_gcc.c (matrix to
   quaternion; slerp, a C function around one asm statement, and squad, which inlines it three times) or stay in
   asm with their C under NON_MATCHING (quaternion to Euler angles). The
   port C of the VFPU functions is left out of the GCC object (`#elif !defined(__mips__)`), so its
   literal pool is exactly the original's (`@rodata`). Functions with an output return it. Outside the
   PSP build each one has a C equivalent. */

typedef struct Vec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) Vec4;

typedef struct Mat4 {
    float m[16];
} __attribute__((aligned(16))) Mat4;

Vec4 *quat_slerp(Vec4 *out, Vec4 *a, Vec4 *b, float t);

#if !defined(__MWERKS__) || defined(NON_MATCHING)
float sqrtf(float x);
float asinf(float x);
float atan2f(float y, float x);
#endif
#if defined(__mips__) && !defined(__MWERKS__)
/* the libm wrappers have other names in BOOT (fdlibm.h): sqrtf stays the builtin (sqrt.s inline, the call only for
   a NaN result), its call goes to libm_sqrtf */
__asm__("sqrtf = libm_sqrtf\n\tasinf = libm_asinf");
#endif

#if !defined(__MWERKS__) && !defined(__mips__)
#include <math.h>

static void qmul(float *r, const float *a, const float *b)
{
    float x = a[3] * b[0] + b[3] * a[0] + a[1] * b[2] - a[2] * b[1];
    float y = a[3] * b[1] + b[3] * a[1] + a[2] * b[0] - a[0] * b[2];
    float z = a[3] * b[2] + b[3] * a[2] + a[0] * b[1] - a[1] * b[0];
    float w = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];

    r[0] = x;
    r[1] = y;
    r[2] = z;
    r[3] = w;
}

/* q = q0 * q1 * q2, qi the rotation by v[axis[i]] about axis[i] */
static Vec4 *from_euler(Vec4 *out, Vec4 *v, int a0, int a1, int a2)
{
    float q[3][4];
    float r[4];
    int ax[3];
    int i;
    int j;

    ax[0] = a0;
    ax[1] = a1;
    ax[2] = a2;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            q[i][j] = 0.0f;
        }
        q[i][ax[i]] = sinf((&v->x)[ax[i]] * 0.5f);
        q[i][3] = cosf((&v->x)[ax[i]] * 0.5f);
    }
    qmul(r, q[0], q[1]);
    qmul(r, r, q[2]);
    out->x = r[0];
    out->y = r[1];
    out->z = r[2];
    out->w = r[3];
    return out;
}
#endif

/* Quaternion of the rotation part of m (Shoemake: trace or largest diagonal element); the result is built in
   locals and stored at the end (m and out may be the same memory). */
#ifndef __MWERKS__ /* PSP: vfpu_quat_gcc.c */
Vec4 *mat3_to_quat(Vec4 *out, Mat4 *m)
{
    float *e = m->m;
    float t = e[0] + e[5] + e[10] + 1.0f;
    float k;
    float x, y, z, w;

    if (0.01f < t) {
        w = sqrtf(t) * 0.5f;
        k = 0.25f / w;
        x = (e[6] - e[9]) * k;
        y = (e[8] - e[2]) * k;
        z = (e[1] - e[4]) * k;
    } else if (e[5] < e[0]) {
        if (e[10] < e[0]) {
            x = sqrtf(e[0] + 1.0f - e[5] - e[10]) * 0.5f;
            k = 0.25f / x;
            y = (e[4] + e[1]) * k;
            z = (e[8] + e[2]) * k;
            w = (e[6] - e[9]) * k;
        } else {
            z = sqrtf(e[10] + 1.0f - e[0] - e[5]) * 0.5f;
            k = 0.25f / z;
            x = (e[8] + e[2]) * k;
            y = (e[9] + e[6]) * k;
            w = (e[1] - e[4]) * k;
        }
    } else {
        if (e[10] < e[5]) {
            y = sqrtf(e[5] + 1.0f - e[10] - e[0]) * 0.5f;
            k = 0.25f / y;
            x = (e[4] + e[1]) * k;
            z = (e[9] + e[6]) * k;
            w = (e[8] - e[2]) * k;
        } else {
            z = sqrtf(e[10] + 1.0f - e[0] - e[5]) * 0.5f;
            k = 0.25f / z;
            x = (e[8] + e[2]) * k;
            y = (e[9] + e[6]) * k;
            w = (e[1] - e[4]) * k;
        }
    }
    out->x = x;
    out->y = y;
    out->z = z;
    out->w = w;
    return out;
}
#else
Vec4 *mat3_to_quat(Vec4 *out, Mat4 *m);
#endif

#ifdef NON_MATCHING /* GCC: constants from the .rodata pool (D_088EB1E4...) */

/* Euler angles of q for q = qz * qy * qx; near the poles (|sin y| > 0.99995) z is 0. w = 0. */
Vec4 *quat_to_euler_zyx(Vec4 *out, Vec4 *q)
{
    float x = q->x;
    float y = q->y;
    float z = q->z;
    float w = q->w;
    float w2 = w + w;
    float z2 = z + z;
    float sy = -(z2 * x - w2 * y);
    float rx, ry, rz;

    if (sy <= 0.99995f && -0.99995f <= sy) {
        float x2 = x + x;
        float y2 = y + y;
        float ya = y2 * z + w2 * x;
        float yb = 1.0f - x2 * x - y2 * y;
        float za = x2 * y + w2 * z;
        float zb = 1.0f - y2 * y - z2 * z;

        rx = atan2f(ya, yb);
        ry = asinf(sy);
        rz = atan2f(za, zb);
    } else {
        float y2 = y + y;
        float x2 = x + x;
        float ya = -(y2 * z - w2 * x);
        float yb = 1.0f - x2 * x - z2 * z;

        rx = atan2f(ya, yb);
        ry = asinf(sy);
        rz = 0.0f;
    }
    out->x = rx;
    out->y = ry;
    out->z = rz;
    out->w = 0.0f;
    return out;
}

/* Euler angles of q for q = qx * qy * qz; near the poles z is 0. w = 0. */
Vec4 *quat_to_euler_xyz(Vec4 *out, Vec4 *q)
{
    float x = q->x;
    float y = q->y;
    float z = q->z;
    float w = q->w;
    float w2 = w + w;
    float z2 = z + z;
    float sy = z2 * x + w2 * y;
    float rx, ry, rz;

    if (sy <= 0.99995f && -0.99995f <= sy) {
        float y2 = y + y;
        float x2 = x + x;
        float ya = -(y2 * z - w2 * x);
        float yb = 1.0f - x2 * x - y2 * y;
        float za = -(x2 * y - w2 * z);
        float zb = 1.0f - y2 * y - z2 * z;

        rx = atan2f(ya, yb);
        ry = asinf(sy);
        rz = atan2f(za, zb);
    } else {
        float x2 = x + x;
        float y2 = y + y;
        float ya = y2 * z + w2 * x;
        float yb = 1.0f - x2 * x - z2 * z;

        rx = atan2f(ya, yb);
        ry = asinf(sy);
        rz = 0.0f;
    }
    out->x = rx;
    out->y = ry;
    out->z = rz;
    out->w = 0.0f;
    return out;
}

/* Euler angles of q for q = qy * qx * qz; near the poles z is 0. w = 0. */
Vec4 *quat_to_euler_yxz(Vec4 *out, Vec4 *q)
{
    float x = q->x;
    float y = q->y;
    float z = q->z;
    float w = q->w;
    float w2 = w + w;
    float y2 = y + y;
    float sx = -(y2 * z - w2 * x);
    float rx, ry, rz;

    if (sx <= 0.99995f && -0.99995f <= sx) {
        float x2 = x + x;
        float z2 = z + z;
        float ya = z2 * x + w2 * y;
        float yb = 1.0f - x2 * x - y2 * y;
        float za = x2 * y + w2 * z;
        float zb = 1.0f - x2 * x - z2 * z;

        rx = asinf(sx);
        ry = atan2f(ya, yb);
        rz = atan2f(za, zb);
    } else {
        float z2 = z + z;
        float ya = -(z2 * x - w2 * y);
        float yb = 1.0f - y2 * y - z2 * z;

        rx = asinf(sx);
        ry = atan2f(ya, yb);
        rz = 0.0f;
    }
    out->x = rx;
    out->y = ry;
    out->z = rz;
    out->w = 0.0f;
    return out;
}
#endif

#ifdef __MWERKS__
asm Vec4 *quat_identity(Vec4 *out)
{
    move v0, a0
    vidt.q C030
    sv.q C030, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_identity(Vec4 *out)
{
    out->x = out->y = out->z = 0.0f;
    out->w = 1.0f;
    return out;
}
#else
Vec4 *quat_identity(Vec4 *out);
#endif

#ifndef __MWERKS__ /* PSP: vfpu_quat_gcc.c */
Vec4 *quat_copy(Vec4 *out, Vec4 *q)
{
    *out = *q;
    return out;
}
#else
Vec4 *quat_copy(Vec4 *out, Vec4 *q);
#endif

/* Rotation matrix of the unit quaternion q (translation 0). */
#ifdef __MWERKS__
asm Mat4 *quat_to_mat4(Mat4 *out, Vec4 *q)
{
    move v0, a0
    lv.q C130, 0(a1)
    vpfxs W, Z, -Y, -X
    vmov.q C100, C130
    vpfxs -Z, W, X, -Y
    vmov.q C110, C130
    vpfxs Y, -X, W, -Z
    vmov.q C120, C130
    vpfxs W, Z, -Y, X
    vmov.q C200, C130
    vpfxs -Z, W, X, Y
    vmov.q C210, C130
    vpfxs Y, -X, W, Z
    vmov.q C220, C130
    vpfxs -X, -Y, -Z, W
    vmov.q C230, C130
    vmmul.q E000, E200, E100
    vidt.q R003
    vidt.q C030
    sv.q C000, 0(a0)
    sv.q C010, 16(a0)
    sv.q C020, 32(a0)
    sv.q C030, 48(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Mat4 *quat_to_mat4(Mat4 *out, Vec4 *q)
{
    float x = q->x;
    float y = q->y;
    float z = q->z;
    float w = q->w;
    float *m = out->m;

    m[0] = w * w + x * x - y * y - z * z;
    m[1] = 2.0f * (x * y + w * z);
    m[2] = 2.0f * (x * z - w * y);
    m[4] = 2.0f * (x * y - w * z);
    m[5] = w * w - x * x + y * y - z * z;
    m[6] = 2.0f * (y * z + w * x);
    m[8] = 2.0f * (x * z + w * y);
    m[9] = 2.0f * (y * z - w * x);
    m[10] = w * w - x * x - y * y + z * z;
    m[3] = m[7] = m[11] = 0.0f;
    m[12] = m[13] = m[14] = 0.0f;
    m[15] = 1.0f;
    return out;
}
#else
Mat4 *quat_to_mat4(Mat4 *out, Vec4 *q);
#endif

/* out = q * v * conj(q) (v rotated by the unit quaternion q, v as a quaternion) */
#ifdef __MWERKS__
asm Vec4 *quat_rotate(Vec4 *out, Vec4 *q, Vec4 *v)
{
    move v0, a0
    lv.q C100, 0(a1)
    lv.q C200, 0(a2)
    vpfxs -X, -Y, -Z, W
    vmov.q C110, C100
    vqmul.q C120, C100, C200
    vqmul.q C000, C120, C110
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_rotate(Vec4 *out, Vec4 *q, Vec4 *v)
{
    float c[4];
    float r[4];

    c[0] = -q->x;
    c[1] = -q->y;
    c[2] = -q->z;
    c[3] = q->w;
    qmul(r, &q->x, &v->x);
    qmul(r, r, c);
    out->x = r[0];
    out->y = r[1];
    out->z = r[2];
    out->w = r[3];
    return out;
}
#else
Vec4 *quat_rotate(Vec4 *out, Vec4 *q, Vec4 *v);
#endif

#ifdef __MWERKS__
asm Vec4 *quat_add(Vec4 *out, Vec4 *a, Vec4 *b)
{
    move v0, a0
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vadd.q C000, C010, C020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_add(Vec4 *out, Vec4 *a, Vec4 *b)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    out->w = a->w + b->w;
    return out;
}
#else
Vec4 *quat_add(Vec4 *out, Vec4 *a, Vec4 *b);
#endif

#ifdef __MWERKS__
asm Vec4 *quat_sub(Vec4 *out, Vec4 *a, Vec4 *b)
{
    move v0, a0
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vsub.q C000, C010, C020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_sub(Vec4 *out, Vec4 *a, Vec4 *b)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    out->w = a->w - b->w;
    return out;
}
#else
Vec4 *quat_sub(Vec4 *out, Vec4 *a, Vec4 *b);
#endif

/* out = a * b */
#ifdef __MWERKS__
asm Vec4 *quat_mul(Vec4 *out, Vec4 *a, Vec4 *b)
{
    move v0, a0
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vqmul.q C000, C010, C020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_mul(Vec4 *out, Vec4 *a, Vec4 *b)
{
    float r[4];

    qmul(r, &a->x, &b->x);
    out->x = r[0];
    out->y = r[1];
    out->z = r[2];
    out->w = r[3];
    return out;
}
#else
Vec4 *quat_mul(Vec4 *out, Vec4 *a, Vec4 *b);
#endif

#ifdef __MWERKS__
asm float quat_dot(Vec4 *a, Vec4 *b)
{
    addiu sp, sp, -16
    lv.q C010, 0(a0)
    lv.q C020, 0(a1)
    vdot.q S000, C010, C020
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#elif !defined(__mips__)
float quat_dot(Vec4 *a, Vec4 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}
#else
float quat_dot(Vec4 *a, Vec4 *b);
#endif

/* Spherical interpolation along the shorter arc (b negated when dot(a, b) < 0); linear (not
   normalized) when dot(a, b) >= 0.998047, a when sin(angle) < 5e-5. */
#ifdef __MWERKS__
Vec4 *quat_slerp(Vec4 *out, Vec4 *a, Vec4 *b, float t);
#elif !defined(__mips__)
Vec4 *quat_slerp(Vec4 *out, Vec4 *a, Vec4 *b, float t)
{
    float d = a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
    float ka = 1.0f - t;
    float kb = t;

    d = d < -1.0f ? -1.0f : d > 1.0f ? 1.0f : d;
    if (!(d >= 0.998047f)) {
        float th = acosf(fabsf(d));
        float s = sinf(th);

        if (s < 5e-5f) {
            *out = *a;
            return out;
        }
        ka = sinf(ka * th) / s;
        kb = sinf(kb * th) / s;
        if (d < 0.0f) {
            kb = -kb;
        }
    }
    out->x = a->x * ka + b->x * kb;
    out->y = a->y * ka + b->y * kb;
    out->z = a->z * ka + b->z * kb;
    out->w = a->w * ka + b->w * kb;
    return out;
}
#else
/* the SDK's C: one asm statement (operands in memory, so quat_squad's inlined copies address its stack temporaries
   directly) */
Vec4 *quat_slerp(Vec4 *out, Vec4 *a, Vec4 *b, float t)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        ".set noat\n"
        "mfc1 $8, %3\n"
        "mtv $8, S031\n"
        "lv.q C010, %1\n"
        "lv.q C020, %2\n"
        "vcst.s S001, VFPU_SQRT1_2\n"
        ".word 0xDE000E03\n" /* vpfxd [-1:1],m,m,m */
        "vdot.q S000, C010, C020\n"
        "vfim.s S002, 0.998047\n"
        "vcmp.s GE, S000, S002\n"
        "bvt 0, 2f\n"
        "vocp.s S030, S031\n"
        "vpfxs |X|, Y, Z, W\n"
        "vcmp.s LT, S000, S001\n"
        "vpfxs |X|, Y, Z, W\n"
        "vasin.s S032, S000\n"
        "bvtl 0, 1f\n"
        "vocp.s S032, S032\n"
        "vmul.s S001, S000, S000\n"
        "vocp.s S001, S001\n"
        "vsqrt.s S001, S001\n"
        "vasin.s S032, S001\n"
        "1:\n"
        "vzero.s S001\n"
        "lui $1, 0x3851\n"
        "ori $1, $1, 0xb717\n"
        "mtv $1, S002\n"
        "vscl.p C030, C030, S032\n"
        "vcmp.s LT, S000, S001\n"
        "vsin.t C030, C030\n"
        "vpfxs -X, -Y, -Z, -W\n"
        "vcmovt.q C020, C020, 0\n"
        "vcmp.s LT, S032, S002\n"
        "vrcp.s S032, S032\n"
        "vscl.p C030, C030, S032\n"
        "bvtl 0, 3f\n"
        "vmov.q C000, C010\n"
        "2:\n"
        "vscl.q C000, C010, S030\n"
        "vscl.q C030, C020, S031\n"
        "vadd.q C000, C000, C030\n"
        "3:\n"
        "sv.q C000, %0\n"
        ".set pop\n"
        : "=m" (*out) : "m" (*a), "m" (*b), "f" (t) : "$8");
    return out;
}
#endif

#ifdef NON_MATCHING /* GCC (vfpu_quat_gcc.c): quat_slerp inlined three times, 1.0f from the pool */
/* squad: slerp(slerp(p, q, t), slerp(a, b, t), 2t(1 - t)) */
Vec4 *quat_squad(Vec4 *out, Vec4 *p, Vec4 *q, Vec4 *a, Vec4 *b, float t)
{
    Vec4 u;
    Vec4 v;
    quat_slerp(&u, p, q, t);
    quat_slerp(&v, a, b, t);
    return quat_slerp(out, &u, &v, 2.0f * t * (1.0f - t));
}
#endif

/* q / |q| */
#ifdef __MWERKS__
asm Vec4 *quat_normalize(Vec4 *out, Vec4 *q)
{
    move v0, a0
    lv.q C000, 0(a1)
    vdot.q S010, C000, C000
    vrsq.s S010, S010
    vscl.q C000, C000, S010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_normalize(Vec4 *out, Vec4 *q)
{
    float k = 1.0f / sqrtf(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);

    out->x = q->x * k;
    out->y = q->y * k;
    out->z = q->z * k;
    out->w = q->w * k;
    return out;
}
#else
Vec4 *quat_normalize(Vec4 *out, Vec4 *q);
#endif

/* (-x, -y, -z, w) */
#ifdef __MWERKS__
asm Vec4 *quat_conj(Vec4 *out, Vec4 *q)
{
    move v0, a0
    lv.q C000, 0(a1)
    vneg.t C000, C000
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_conj(Vec4 *out, Vec4 *q)
{
    out->x = -q->x;
    out->y = -q->y;
    out->z = -q->z;
    out->w = q->w;
    return out;
}
#else
Vec4 *quat_conj(Vec4 *out, Vec4 *q);
#endif

/* conj(q) / |q|^2 */
#ifdef __MWERKS__
asm Vec4 *quat_inverse(Vec4 *out, Vec4 *q)
{
    move v0, a0
    lv.q C010, 0(a1)
    vdot.q S020, C010, C010
    vrcp.s S020, S020
    vpfxs -X, -Y, -Z, W
    vscl.q C000, C010, S020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_inverse(Vec4 *out, Vec4 *q)
{
    float k = 1.0f / (q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);

    out->x = -q->x * k;
    out->y = -q->y * k;
    out->z = -q->z * k;
    out->w = q->w * k;
    return out;
}
#else
Vec4 *quat_inverse(Vec4 *out, Vec4 *q);
#endif

/* Logarithm: (v / |v|) * angle, w = 0, with v = q.xyz. As in the original the angle is computed as
   acos(w / |v|) (asin of sqrt(1 - c^2) when c = w / |v| >= sqrt(1/2)); a zero v gives (0, 0, 0, 0). */
#ifdef __MWERKS__
asm Vec4 *quat_log(Vec4 *out, Vec4 *q)
{
    move v0, a0
    lv.q C010, 0(a1)
    vdot.t S000, C010, C010
    vsqrt.s S000, S000
    vcmp.s EZ, S000, S000
    bvtl 0, L_0881E9A0
    vone.s S000
    vrcp.s S000, S000
    vmul.s S001, S013, S000
    vcst.s S002, VFPU_SQRT1_2
    vcmp.s LT, S001, S002
    vasin.s S002, S001
    vmul.s S003, S001, S001
    vcst.s S001, VFPU_PI_2
    vmul.s S000, S000, S001
    bvtl 0, L_0881E99C
    vocp.s S003, S002
    vocp.s S003, S003
    vsqrt.s S003, S003
    vasin.s S003, S003
L_0881E99C:
    vmul.s S000, S003, S000
L_0881E9A0:
    vzero.s S003
    vscl.t C000, C010, S000
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_log(Vec4 *out, Vec4 *q)
{
    float l = sqrtf(q->x * q->x + q->y * q->y + q->z * q->z);
    float k = 1.0f;

    if (l != 0.0f) {
        float c = q->w * (1.0f / l);
        float a = c < 0.70710678f ? acosf(c) : asinf(sqrtf(1.0f - c * c));

        k = a * (1.0f / l);
    }
    out->x = q->x * k;
    out->y = q->y * k;
    out->z = q->z * k;
    out->w = 0.0f;
    return out;
}
#else
Vec4 *quat_log(Vec4 *out, Vec4 *q);
#endif

/* Exponential of the pure quaternion v: (v / |v| * sin |v|, cos |v|); v = 0 gives (0, 0, 0, 1). */
#ifdef __MWERKS__
asm Vec4 *quat_exp(Vec4 *out, Vec4 *v)
{
    move v0, a0
    lv.q C010, 0(a1)
    vdot.t S000, C010, C010
    vsqrt.s S000, S000
    vcst.s S001, VFPU_2_PI
    vcmp.s EZ, S000, S000
    vrcp.s S002, S000
    vmul.s S000, S000, S001
    vsin.s S001, S000
    vcos.s S003, S000
    vmul.s S000, S001, S002
    vzero.s S001
    vcmovt.s S000, S001, 0
    vscl.t C000, C010, S000
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_exp(Vec4 *out, Vec4 *v)
{
    float l = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    float k = l == 0.0f ? 0.0f : sinf(l) / l;

    out->x = v->x * k;
    out->y = v->y * k;
    out->z = v->z * k;
    out->w = cosf(l);
    return out;
}
#else
Vec4 *quat_exp(Vec4 *out, Vec4 *v);
#endif

#ifdef __MWERKS__
asm Vec4 *quat_from_euler_zyx(Vec4 *out, Vec4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vcst.s S010, VFPU_1_PI
    vscl.t C000, C000, S010
    vcos.t C010, C000
    vsin.t C000, C000
    vcrs.t C020, C010, C010
    vcrs.t C030, C000, C000
    vmul.s S003, S020, S010
    vmul.s S013, S030, S000
    vmul.t C020, C020, C000
    vmul.t C030, C030, C010
    vadd.s S003, S003, S013
    vpfxt -X, Y, -Z, W
    vadd.t C000, C020, C030
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_from_euler_zyx(Vec4 *out, Vec4 *v)
{
    return from_euler(out, v, 2, 1, 0);
}
#else
Vec4 *quat_from_euler_zyx(Vec4 *out, Vec4 *v);
#endif

#ifdef __MWERKS__
asm Vec4 *quat_from_euler_xyz(Vec4 *out, Vec4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vcst.s S010, VFPU_1_PI
    vscl.t C000, C000, S010
    vcos.t C010, C000
    vsin.t C000, C000
    vcrs.t C020, C010, C010
    vcrs.t C030, C000, C000
    vmul.s S003, S020, S010
    vmul.s S013, S030, S000
    vmul.t C020, C020, C000
    vmul.t C030, C030, C010
    vsub.s S003, S003, S013
    vpfxt X, -Y, Z, W
    vadd.t C000, C020, C030
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_from_euler_xyz(Vec4 *out, Vec4 *v)
{
    return from_euler(out, v, 0, 1, 2);
}
#else
Vec4 *quat_from_euler_xyz(Vec4 *out, Vec4 *v);
#endif

#ifdef __MWERKS__
asm Vec4 *quat_from_euler_yxz(Vec4 *out, Vec4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vcst.s S010, VFPU_1_PI
    vscl.t C000, C000, S010
    vcos.t C010, C000
    vsin.t C000, C000
    vcrs.t C020, C010, C010
    vcrs.t C030, C000, C000
    vmul.s S003, S020, S010
    vmul.s S013, S030, S000
    vmul.t C020, C020, C000
    vmul.t C030, C030, C010
    vadd.s S003, S003, S013
    vpfxt X, -Y, -Z, W
    vadd.t C000, C020, C030
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_from_euler_yxz(Vec4 *out, Vec4 *v)
{
    return from_euler(out, v, 1, 0, 2);
}
#else
Vec4 *quat_from_euler_yxz(Vec4 *out, Vec4 *v);
#endif

/* Rotation by `angle` radians about `axis` (normalized by its 4-component length, w included). */
#ifdef __MWERKS__
asm Vec4 *quat_from_axis_angle(Vec4 *out, Vec4 *axis, float angle)
{
    move v0, a0
    lv.q C000, 0(a1)
    mfc1 t0, $f12
    vdot.q S010, C000, C000
    vcst.s S020, VFPU_1_PI
    mtv t0, S021
    vmul.s S020, S020, S021
    vcos.s S003, S020
    vsin.s S020, S020
    vrsq.s S010, S010
    vmul.s S010, S010, S020
    vscl.t C000, C000, S010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *quat_from_axis_angle(Vec4 *out, Vec4 *axis, float angle)
{
    float k = sinf(angle * 0.5f) / sqrtf(axis->x * axis->x + axis->y * axis->y + axis->z * axis->z + axis->w * axis->w);

    out->x = axis->x * k;
    out->y = axis->y * k;
    out->z = axis->z * k;
    out->w = cosf(angle * 0.5f);
    return out;
}
#else
Vec4 *quat_from_axis_angle(Vec4 *out, Vec4 *axis, float angle);
#endif
