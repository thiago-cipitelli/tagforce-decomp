// @range 0x0881C4E4-0x0881CD50
// @gcc 0x0881C4E4-0x0881C730
// @gcc 0x0881C830-0x0881CD50
/* VFPU matrix library, second half (the first half, from mat4_identity, lives before 0x0881C4E4):
   products, transforms, rotations, determinant, inverse and the helpers built on them. Matrices are
   column-major as in the VFPU (m[4 * col + row]); each function is hand-written VFPU code (the SDK's
   libvfpu style: arguments spilled to the stack to reach the VFPU), kept as whole `asm` functions.
   Functions that take an output matrix return it. Angles are in radians (scaled by 2/pi for
   vrot/vsin, which work in quarter turns). Outside the PSP build each one has a C equivalent. */

typedef struct Vec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) Vec4;

typedef struct Mat4 {
    float m[16];
} __attribute__((aligned(16))) Mat4;

#ifndef __MWERKS__
#include <math.h>
#include <string.h>

/* r = a * b, safe when r aliases a or b */
static void mul4(Mat4 *r, const Mat4 *a, const Mat4 *b)
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

/* out = R * m (m == 0: out = R), R the rotation by `a` radians about axis 0 (x), 1 (y) or 2 (z) */
static Mat4 *rotate_axis(Mat4 *out, Mat4 *m, float a, int axis)
{
    Mat4 r;
    float c = cosf(a);
    float s = sinf(a);
    int u = (axis + 1) % 3;
    int v = (axis + 2) % 3;
    int i;

    for (i = 0; i < 16; i++) {
        r.m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
    r.m[u * 4 + u] = c;
    r.m[u * 4 + v] = s;
    r.m[v * 4 + u] = -s;
    r.m[v * 4 + v] = c;
    if (m == 0) {
        *out = r;
    } else {
        mul4(out, &r, m);
    }
    return out;
}

/* adjugate (transposed cofactor matrix): m * adj = det(m) * I; returns det(m) */
static float adjugate(float *o, const float *m)
{
    float det;

    o[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    o[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    o[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    o[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    o[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    o[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    o[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    o[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    o[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    o[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    o[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    o[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    o[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    o[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    o[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    o[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
    det = m[0] * o[0] + m[1] * o[4] + m[2] * o[8] + m[3] * o[12];
    return det;
}

static void normalize3(float *v)
{
    float l = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
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

/* out = m * (v.xyz, 1) */
#ifdef __MWERKS__
Vec4 *mat4_transform_point(Vec4 *out, Mat4 *m, Vec4 *v);
#elif !defined(__mips__)
Vec4 *mat4_transform_point(Vec4 *out, Mat4 *m, Vec4 *v)
{
    float x = v->x;
    float y = v->y;
    float z = v->z;
    float r[4];
    int i;

    for (i = 0; i < 4; i++) {
        r[i] = m->m[i] * x + m->m[4 + i] * y + m->m[8 + i] * z + m->m[12 + i];
    }
    out->x = r[0];
    out->y = r[1];
    out->z = r[2];
    out->w = r[3];
    return out;
}
#else
Vec4 *mat4_transform_point(Vec4 *out, Mat4 *m, Vec4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C100, 0(%2)\n"
        "lv.q C110, 16(%2)\n"
        "lv.q C120, 32(%2)\n"
        "lv.q C130, 48(%2)\n"
        "lv.q C200, 0(%3)\n"
        "vhtfm4.q C000, E100, C200\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "r" (v)
        : "memory");
    return out;
}
#endif

/* out = a * b */
#ifdef __MWERKS__
Mat4 *mat4_mul(Mat4 *out, Mat4 *a, Mat4 *b);
#elif !defined(__mips__)
Mat4 *mat4_mul(Mat4 *out, Mat4 *a, Mat4 *b)
{
    mul4(out, a, b);
    return out;
}
#else
Mat4 *mat4_mul(Mat4 *out, Mat4 *a, Mat4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C100, 0(%2)\n"
        "lv.q C110, 16(%2)\n"
        "lv.q C120, 32(%2)\n"
        "lv.q C130, 48(%2)\n"
        "lv.q C200, 0(%3)\n"
        "lv.q C210, 16(%3)\n"
        "lv.q C220, 32(%3)\n"
        "lv.q C230, 48(%3)\n"
        "vmmul.q E000, E200, E100\n"
        "sv.q C000, 0(%1)\n"
        "sv.q C010, 16(%1)\n"
        "sv.q C020, 32(%1)\n"
        "sv.q C030, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* out = m * s (every element) */
#ifdef __MWERKS__
Mat4 *mat4_scale(Mat4 *out, Mat4 *m, float s);
#elif !defined(__mips__)
Mat4 *mat4_scale(Mat4 *out, Mat4 *m, float s)
{
    int i;

    for (i = 0; i < 16; i++) {
        out->m[i] = m->m[i] * s;
    }
    return out;
}
#else
Mat4 *mat4_scale(Mat4 *out, Mat4 *m, float s)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %3\n"
        "mtv $8, S200\n"
        "lv.q C100, 0(%2)\n"
        "lv.q C110, 16(%2)\n"
        "lv.q C120, 32(%2)\n"
        "lv.q C130, 48(%2)\n"
        "vmscl.q E000, E100, S200\n"
        "sv.q C000, 0(%1)\n"
        "sv.q C010, 16(%1)\n"
        "sv.q C020, 32(%1)\n"
        "sv.q C030, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "f" (s)
        : "memory", "$8");
    return out;
}
#endif

#ifdef __MWERKS__
Mat4 *mat4_transpose(Mat4 *out, Mat4 *m);
#elif !defined(__mips__)
Mat4 *mat4_transpose(Mat4 *out, Mat4 *m)
{
    Mat4 t;
    int i;
    int j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            t.m[i * 4 + j] = m->m[j * 4 + i];
        }
    }
    *out = t;
    return out;
}
#else
Mat4 *mat4_transpose(Mat4 *out, Mat4 *m)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 16(%2)\n"
        "lv.q C020, 32(%2)\n"
        "lv.q C030, 48(%2)\n"
        "sv.q R000, 0(%1)\n"
        "sv.q R001, 16(%1)\n"
        "sv.q R002, 32(%1)\n"
        "sv.q R003, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m)
        : "memory");
    return out;
}
#endif

/* Inverse of a rigid transform (orthonormal rotation R, translation t): R^T and -R^T * t; the
   rotation columns get w = 0 and the last column keeps m's w. */
#ifdef __MWERKS__
Mat4 *mat4_inverse_ortho(Mat4 *out, Mat4 *m);
#elif !defined(__mips__)
Mat4 *mat4_inverse_ortho(Mat4 *out, Mat4 *m)
{
    Mat4 t;
    int i;
    int j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            t.m[i * 4 + j] = m->m[j * 4 + i];
        }
        t.m[i * 4 + 3] = 0.0f;
        t.m[12 + i] = -(m->m[i * 4] * m->m[12] + m->m[i * 4 + 1] * m->m[13] + m->m[i * 4 + 2] * m->m[14]);
    }
    t.m[15] = m->m[15];
    *out = t;
    return out;
}
#else
Mat4 *mat4_inverse_ortho(Mat4 *out, Mat4 *m)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C100, 0(%2)\n"
        "lv.q C110, 16(%2)\n"
        "lv.q C120, 32(%2)\n"
        "lv.q C000, 48(%2)\n"
        "vzero.t C130\n"
        "vtfm3.t C010, M100, C000\n"
        "sv.q R100, 0(%1)\n"
        "sv.q R101, 16(%1)\n"
        "vneg.t C000, C010\n"
        "sv.q R102, 32(%1)\n"
        "sv.q C000, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m)
        : "memory");
    return out;
}
#endif

/* out = m with v.xyz added to its translation (translate(v) * m for an affine m) */
#ifdef __MWERKS__
Mat4 *mat4_translate(Mat4 *out, Mat4 *m, Vec4 *v);
#elif !defined(__mips__)
Mat4 *mat4_translate(Mat4 *out, Mat4 *m, Vec4 *v)
{
    *out = *m;
    out->m[12] += v->x;
    out->m[13] += v->y;
    out->m[14] += v->z;
    return out;
}
#else
Mat4 *mat4_translate(Mat4 *out, Mat4 *m, Vec4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 16(%2)\n"
        "lv.q C020, 32(%2)\n"
        "lv.q C030, 48(%2)\n"
        "lv.q C100, 0(%3)\n"
        "vadd.t C100, C100, C030\n"
        "sv.q C000, 0(%1)\n"
        "sv.q C010, 16(%1)\n"
        "sv.q C020, 32(%1)\n"
        "sv.q C100, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "r" (v)
        : "memory");
    return out;
}
#endif

/* out = rotation_z(a) * m; m == 0 gives the rotation alone */
#ifdef __MWERKS__
Mat4 *mat4_mul_rot_z(Mat4 *out, Mat4 *m, float a);
#elif !defined(__mips__)
Mat4 *mat4_mul_rot_z(Mat4 *out, Mat4 *m, float a)
{
    return rotate_axis(out, m, a, 2);
}
#else
Mat4 *mat4_mul_rot_z(Mat4 *out, Mat4 *m, float a)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, %3\n"
        "vcst.s S101, VFPU_2_PI\n"
        "vmul.s S100, S100, S101\n"
        "vrot.q C000, S100, [c, s, 0, 0]\n"
        "vrot.q C010, S100, [-s, c, 0, 0]\n"
        "vidt.q C020\n"
        "vidt.q C030\n"
        "beqzl %2, 1f\n"
        "vmmov.q E200, E000\n"
        "lv.q C100, 0(%2)\n"
        "lv.q C110, 16(%2)\n"
        "lv.q C120, 32(%2)\n"
        "lv.q C130, 48(%2)\n"
        "vmmul.q E200, E100, E000\n"
        "1:\n"
        "sv.q C200, 0(%1)\n"
        "sv.q C210, 16(%1)\n"
        "sv.q C220, 32(%1)\n"
        "sv.q C230, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "m" (a)
        : "memory");
    return out;
}
#endif

/* out = rotation_y(a) * m; m == 0 gives the rotation alone */
#ifdef __MWERKS__
Mat4 *mat4_mul_rot_y(Mat4 *out, Mat4 *m, float a);
#elif !defined(__mips__)
Mat4 *mat4_mul_rot_y(Mat4 *out, Mat4 *m, float a)
{
    return rotate_axis(out, m, a, 1);
}
#else
Mat4 *mat4_mul_rot_y(Mat4 *out, Mat4 *m, float a)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, %3\n"
        "vcst.s S101, VFPU_2_PI\n"
        "vmul.s S100, S100, S101\n"
        "vrot.q C000, S100, [c, 0, -s, 0]\n"
        "vidt.q C010\n"
        "vrot.q C020, S100, [s, 0, c, 0]\n"
        "vidt.q C030\n"
        "beqzl %2, 1f\n"
        "vmmov.q E200, E000\n"
        "lv.q C100, 0(%2)\n"
        "lv.q C110, 16(%2)\n"
        "lv.q C120, 32(%2)\n"
        "lv.q C130, 48(%2)\n"
        "vmmul.q E200, E100, E000\n"
        "1:\n"
        "sv.q C200, 0(%1)\n"
        "sv.q C210, 16(%1)\n"
        "sv.q C220, 32(%1)\n"
        "sv.q C230, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "m" (a)
        : "memory");
    return out;
}
#endif

/* out = rotation_x(a) * m; m == 0 gives the rotation alone */
#ifdef __MWERKS__
Mat4 *mat4_mul_rot_x(Mat4 *out, Mat4 *m, float a);
#elif !defined(__mips__)
Mat4 *mat4_mul_rot_x(Mat4 *out, Mat4 *m, float a)
{
    return rotate_axis(out, m, a, 0);
}
#else
Mat4 *mat4_mul_rot_x(Mat4 *out, Mat4 *m, float a)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, %3\n"
        "vcst.s S101, VFPU_2_PI\n"
        "vmul.s S100, S100, S101\n"
        "vidt.q C000\n"
        "vrot.q C010, S100, [0, c, s, 0]\n"
        "vrot.q C020, S100, [0, -s, c, 0]\n"
        "vidt.q C030\n"
        "beqzl %2, 1f\n"
        "vmmov.q E200, E000\n"
        "lv.q C100, 0(%2)\n"
        "lv.q C110, 16(%2)\n"
        "lv.q C120, 32(%2)\n"
        "lv.q C130, 48(%2)\n"
        "vmmul.q E200, E100, E000\n"
        "1:\n"
        "sv.q C200, 0(%1)\n"
        "sv.q C210, 16(%1)\n"
        "sv.q C220, 32(%1)\n"
        "sv.q C230, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "m" (a)
        : "memory");
    return out;
}
#endif

/* out = rot_x(v.x) * rot_y(v.y) * rot_z(v.z) * m (z applied first); m == 0 starts from identity */
#ifdef __MWERKS__
asm Mat4 *mat4_rotate_xyz(Mat4 *out, Mat4 *m, Vec4 *v)
{
    lwc1 $f2, 8(a2)
    addiu sp, sp, -16
    move v0, a0
    swc1 $f2, 0(sp)
    lv.s S100, 0(sp)
    vcst.s S101, VFPU_2_PI
    vmul.s S100, S100, S101
    vrot.q C000, S100, [c, s, 0, 0]
    vrot.q C010, S100, [-s, c, 0, 0]
    vidt.q C020
    vidt.q C030
    beqzl a1, L_0881C778
    vmmov.q E200, E000
    lv.q C100, 0(a1)
    lv.q C110, 16(a1)
    lv.q C120, 32(a1)
    lv.q C130, 48(a1)
    vmmul.q E200, E100, E000
L_0881C778:
    sv.q C200, 0(a0)
    sv.q C210, 16(a0)
    sv.q C220, 32(a0)
    sv.q C230, 48(a0)
    lwc1 $f1, 4(a2)
    swc1 $f1, 4(sp)
    lv.s S100, 4(sp)
    vcst.s S101, VFPU_2_PI
    vmul.s S100, S100, S101
    vrot.q C000, S100, [c, 0, -s, 0]
    vidt.q C010
    vrot.q C020, S100, [s, 0, c, 0]
    vidt.q C030
    beqzl a0, L_0881C7C8
    vmmov.q E200, E000
    lv.q C100, 0(a0)
    lv.q C110, 16(a0)
    lv.q C120, 32(a0)
    lv.q C130, 48(a0)
    vmmul.q E200, E100, E000
L_0881C7C8:
    sv.q C200, 0(a0)
    sv.q C210, 16(a0)
    sv.q C220, 32(a0)
    sv.q C230, 48(a0)
    lwc1 $f1, 0(a2)
    swc1 $f1, 8(sp)
    lv.s S100, 8(sp)
    vcst.s S101, VFPU_2_PI
    vmul.s S100, S100, S101
    vidt.q C000
    vrot.q C010, S100, [0, c, s, 0]
    vrot.q C020, S100, [0, -s, c, 0]
    vidt.q C030
    beqzl a0, L_0881C818
    vmmov.q E200, E000
    lv.q C100, 0(a0)
    lv.q C110, 16(a0)
    lv.q C120, 32(a0)
    lv.q C130, 48(a0)
    vmmul.q E200, E100, E000
L_0881C818:
    sv.q C200, 0(a0)
    sv.q C210, 16(a0)
    sv.q C220, 32(a0)
    sv.q C230, 48(a0)
    jr ra
    addiu sp, sp, 16
}
#else
Mat4 *mat4_rotate_xyz(Mat4 *out, Mat4 *m, Vec4 *v)
{
    rotate_axis(out, m, v->z, 2);
    rotate_axis(out, out, v->y, 1);
    return rotate_axis(out, out, v->x, 0);
}
#endif

/* 1 when m is exactly the identity (bit compare: -0.0 is not 0) */
#ifdef __MWERKS__
int mat4_is_identity(Mat4 *m);
#elif !defined(__mips__)
int mat4_is_identity(Mat4 *m)
{
    unsigned int *w = (unsigned int *)m->m;
    unsigned int other = 0;
    int i;

    if (w[0] != 0x3F800000 || w[5] != w[0] || w[10] != w[0] || w[15] != w[0]) {
        return 0;
    }
    for (i = 0; i < 16; i++) {
        if (i % 5 != 0) {
            other |= w[i];
        }
    }
    return other == 0;
}
#else
int mat4_is_identity(Mat4 *m)
{
    const unsigned int *p = (const unsigned int *)m;
    int r = 0;

    if (p[0] == 0x3F800000 && p[5] == p[0] && p[10] == p[5] && p[15] == p[10]) {
        r = (p[1] | p[2] | p[3] | p[4] | p[6] | p[7] | p[8] | p[9] | p[11] | p[12] | p[13] | p[14]) == 0;
    }
    return r;
}
#endif

#ifndef __MWERKS__ /* PSP: vfpu_mat4_gcc.c */
float mat4_trace(Mat4 *m)
{
    return m->m[0] + m->m[5] + m->m[10] + m->m[15];
}
#else
float mat4_trace(Mat4 *m);
#endif

#ifdef __MWERKS__
float mat4_determinant(Mat4 *m);
#elif !defined(__mips__)
float mat4_determinant(Mat4 *m)
{
    float o[16];

    return adjugate(o, m->m);
}
#else
float mat4_determinant(Mat4 *m)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%1)\n"
        "lv.q C010, 16(%1)\n"
        "lv.q C020, 32(%1)\n"
        "lv.q C030, 48(%1)\n"
        "vpfxs X, Z, Y, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C100, C011, C021\n"
        "vpfxs X, Z, Y, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C110, C001, C021\n"
        "vpfxs X, Z, Y, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C120, C001, C011\n"
        "vpfxs X, Z, Y, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C130, C001, C011\n"
        "vpfxs Z, Y, X, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C200, C011, C021\n"
        "vpfxs Z, Y, X, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C210, C001, C021\n"
        "vpfxs Z, Y, X, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C220, C001, C011\n"
        "vpfxs Z, Y, X, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C230, C001, C011\n"
        "vpfxt Z, Y, X, W\n"
        "vdot.t S100, C100, C031\n"
        "vpfxt Z, Y, X, W\n"
        "vdot.t S110, C110, C031\n"
        "vpfxt Z, Y, X, W\n"
        "vdot.t S120, C120, C031\n"
        "vpfxt Z, Y, X, W\n"
        "vdot.t S130, C130, C021\n"
        "vpfxt X, Z, Y, W\n"
        "vdot.t S200, C200, C031\n"
        "vpfxt X, Z, Y, W\n"
        "vdot.t S210, C210, C031\n"
        "vpfxt X, Z, Y, W\n"
        "vdot.t S220, C220, C031\n"
        "vpfxt X, Z, Y, W\n"
        "vdot.t S230, C230, C021\n"
        "vsub.q R100, R100, R200\n"
        "vmul.q R101, R000, R100\n"
        "vpfxs X, -Y, Z, -W\n"
        "vfad.q S000, R101\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (m)
        : "memory");
    return r;
}
#endif

/* out = adjugate of m (m * out = det(m) * I) */
#ifdef __MWERKS__
Mat4 *mat4_adjugate(Mat4 *out, Mat4 *m);
#elif !defined(__mips__)
Mat4 *mat4_adjugate(Mat4 *out, Mat4 *m)
{
    float o[16];

    adjugate(o, m->m);
    memcpy(out->m, o, sizeof(o));
    return out;
}
#else
Mat4 *mat4_adjugate(Mat4 *out, Mat4 *m)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 16(%2)\n"
        "lv.q C020, 32(%2)\n"
        "lv.q C030, 48(%2)\n"
        "vpfxs 0, Z, Y, Y\n"
        "vpfxt 0, W, W, Z\n"
        "vmul.q C100, C020, C030\n"
        "vpfxs X, 0, W, Z\n"
        "vpfxt Y, 0, X, X\n"
        "vmul.q C110, C020, C030\n"
        "vpfxs 0, Z, Y, Y\n"
        "vpfxt 0, W, W, Z\n"
        "vmul.q C120, C030, C020\n"
        "vpfxs X, 0, W, Z\n"
        "vpfxt Y, 0, X, X\n"
        "vmul.q C130, C030, C020\n"
        "vsub.q C100, C100, C120\n"
        "vsub.q C110, C110, C130\n"
        "vmov.p R122, R100\n"
        "vmov.p R123, C110\n"
        "vpfxs X, -Y, Z, W\n"
        "vmov.t R110, C101\n"
        "vpfxs X, -Y, Z, W\n"
        "vmov.p R121, C112\n"
        "vtfm4.q R200, E100, C010\n"
        "vtfm4.q R201, E100, C000\n"
        "vpfxs 0, Z, Y, Y\n"
        "vpfxt 0, W, W, Z\n"
        "vmul.q C100, C000, C010\n"
        "vpfxs X, 0, W, Z\n"
        "vpfxt Y, 0, X, X\n"
        "vmul.q C110, C000, C010\n"
        "vpfxs 0, Z, Y, Y\n"
        "vpfxt 0, W, W, Z\n"
        "vmul.q C120, C010, C000\n"
        "vpfxs X, 0, W, Z\n"
        "vpfxt Y, 0, X, X\n"
        "vmul.q C130, C010, C000\n"
        "vsub.q C100, C100, C120\n"
        "vsub.q C110, C110, C130\n"
        "vmov.p R122, R100\n"
        "vmov.p R123, C110\n"
        "vpfxs X, -Y, Z, W\n"
        "vmov.t R110, C101\n"
        "vpfxs X, -Y, Z, W\n"
        "vmov.p R121, C112\n"
        "vtfm4.q R202, E100, C030\n"
        "vtfm4.q R203, E100, C020\n"
        ".word 0xDE000500  /* vpfxd m,,m, */\n"
        "vneg.q C200, C200\n"
        ".word 0xDE000A00  /* vpfxd ,m,,m */\n"
        "vneg.q C210, C210\n"
        ".word 0xDE000500  /* vpfxd m,,m, */\n"
        "vneg.q C220, C220\n"
        ".word 0xDE000A00  /* vpfxd ,m,,m */\n"
        "vneg.q C230, C230\n"
        "sv.q C200, 0(%1)\n"
        "sv.q C210, 16(%1)\n"
        "sv.q C220, 32(%1)\n"
        "sv.q C230, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m)
        : "memory");
    return out;
}
#endif

/* out = inverse of m; *det (if det != 0) receives the determinant. A singular m leaves out as is. */
#ifdef __MWERKS__
Mat4 *mat4_invert(Mat4 *out, float *det, Mat4 *m);
#elif !defined(__mips__)
Mat4 *mat4_invert(Mat4 *out, float *det, Mat4 *m)
{
    float o[16];
    float d = adjugate(o, m->m);
    int i;

    if (d != 0.0f) {
        for (i = 0; i < 16; i++) {
            out->m[i] = o[i] * (1.0f / d);
        }
    }
    if (det != 0) {
        *det = d;
    }
    return out;
}
#else
Mat4 *mat4_invert(Mat4 *out, float *det, Mat4 *m)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%3)\n"
        "lv.q C010, 16(%3)\n"
        "lv.q C020, 32(%3)\n"
        "lv.q C030, 48(%3)\n"
        "vpfxs X, Z, Y, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C100, C011, C021\n"
        "vpfxs X, Z, Y, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C110, C001, C021\n"
        "vpfxs X, Z, Y, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C120, C001, C011\n"
        "vpfxs X, Z, Y, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C130, C001, C011\n"
        "vpfxs Z, Y, X, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C200, C011, C021\n"
        "vpfxs Z, Y, X, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C210, C001, C021\n"
        "vpfxs Z, Y, X, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C220, C001, C011\n"
        "vpfxs Z, Y, X, W\n"
        "vpfxt Y, X, Z, W\n"
        "vmul.t C230, C001, C011\n"
        "vpfxt Z, Y, X, W\n"
        "vdot.t S100, C100, C031\n"
        "vpfxt Z, Y, X, W\n"
        "vdot.t S110, C110, C031\n"
        "vpfxt Z, Y, X, W\n"
        "vdot.t S120, C120, C031\n"
        "vpfxt Z, Y, X, W\n"
        "vdot.t S130, C130, C021\n"
        "vpfxt X, Z, Y, W\n"
        "vdot.t S200, C200, C031\n"
        "vpfxt X, Z, Y, W\n"
        "vdot.t S210, C210, C031\n"
        "vpfxt X, Z, Y, W\n"
        "vdot.t S220, C220, C031\n"
        "vpfxt X, Z, Y, W\n"
        "vdot.t S230, C230, C021\n"
        "vsub.q R100, R100, R200\n"
        "vmul.q R101, R000, R100\n"
        "vpfxs X, -Y, Z, -W\n"
        "vfad.q S200, R101\n"
        "vcmp.s EZ, S200, S200\n"
        "mfv $8, S200\n"
        "bvt 0, L_0881CC84\n"
        "nop\n"
        "vpfxs 0, Z, Y, Y\n"
        "vpfxt 0, W, W, Z\n"
        "vmul.q C100, C020, C030\n"
        "vpfxs X, 0, W, Z\n"
        "vpfxt Y, 0, X, X\n"
        "vmul.q C110, C020, C030\n"
        "vpfxs 0, Z, Y, Y\n"
        "vpfxt 0, W, W, Z\n"
        "vmul.q C120, C030, C020\n"
        "vpfxs X, 0, W, Z\n"
        "vpfxt Y, 0, X, X\n"
        "vmul.q C130, C030, C020\n"
        "vsub.q C100, C100, C120\n"
        "vsub.q C110, C110, C130\n"
        "vmov.p R122, R100\n"
        "vmov.p R123, C110\n"
        "vpfxs X, -Y, Z, W\n"
        "vmov.t R110, C101\n"
        "vpfxs X, -Y, Z, W\n"
        "vmov.p R121, C112\n"
        "vtfm4.q R200, E100, C010\n"
        "vtfm4.q R201, E100, C000\n"
        "vpfxs 0, Z, Y, Y\n"
        "vpfxt 0, W, W, Z\n"
        "vmul.q C100, C000, C010\n"
        "vpfxs X, 0, W, Z\n"
        "vpfxt Y, 0, X, X\n"
        "vmul.q C110, C000, C010\n"
        "vpfxs 0, Z, Y, Y\n"
        "vpfxt 0, W, W, Z\n"
        "vmul.q C120, C010, C000\n"
        "vpfxs X, 0, W, Z\n"
        "vpfxt Y, 0, X, X\n"
        "vmul.q C130, C010, C000\n"
        "vsub.q C100, C100, C120\n"
        "vsub.q C110, C110, C130\n"
        "vmov.p R122, R100\n"
        "vmov.p R123, C110\n"
        "vpfxs X, -Y, Z, W\n"
        "vmov.t R110, C101\n"
        "vpfxs X, -Y, Z, W\n"
        "vmov.p R121, C112\n"
        "vtfm4.q R202, E100, C030\n"
        "vtfm4.q R203, E100, C020\n"
        ".word 0xDE000500  /* vpfxd m,,m, */\n"
        "vneg.q C200, C200\n"
        ".word 0xDE000A00  /* vpfxd ,m,,m */\n"
        "vneg.q C210, C210\n"
        ".word 0xDE000500  /* vpfxd m,,m, */\n"
        "vneg.q C220, C220\n"
        ".word 0xDE000A00  /* vpfxd ,m,,m */\n"
        "vneg.q C230, C230\n"
        "mtv $8, S000\n"
        "vrcp.s S000, S000\n"
        "vmscl.q E200, E200, S000\n"
        "sv.q C200, 0(%1)\n"
        "sv.q C210, 16(%1)\n"
        "sv.q C220, 32(%1)\n"
        "sv.q C230, 48(%1)\n"
        "L_0881CC84:\n"
        "bnezl %2, L_0881CC8C\n"
        "sw $8, 0(%2)\n"
        "L_0881CC8C:\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (det), "r" (m)
        : "memory", "$8");
    return out;
}
#endif

/* Planar shadow projection: with the plane normalized by the length of its xyz (n = plane / |plane.xyz|,
   clamped to [-1, 1]; a zero normal gives n = 0), out = dot(light, n) * I - light * n^T. */
#ifdef __MWERKS__
Mat4 *mat4_shadow(Mat4 *out, Vec4 *light, Vec4 *plane);
#elif !defined(__mips__)
Mat4 *mat4_shadow(Mat4 *out, Vec4 *light, Vec4 *plane)
{
    float *l = &light->x;
    float n[4];
    float len2 = plane->x * plane->x + plane->y * plane->y + plane->z * plane->z;
    float k = (len2 == 0.0f) ? 0.0f : 1.0f / sqrtf(len2);
    float d;
    int i;
    int j;

    for (i = 0; i < 4; i++) {
        n[i] = (&plane->x)[i] * k;
        n[i] = n[i] < -1.0f ? -1.0f : n[i] > 1.0f ? 1.0f : n[i];
    }
    d = l[0] * n[0] + l[1] * n[1] + l[2] * n[2] + l[3] * n[3];
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            out->m[j * 4 + i] = -l[i] * n[j] + (i == j ? d : 0.0f);
        }
    }
    return out;
}
#else
Mat4 *mat4_shadow(Mat4 *out, Vec4 *light, Vec4 *plane)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C100, 0(%2)\n"
        "lv.q C200, 0(%3)\n"
        "vdot.t S210, C200, C200\n"
        "vzero.s S211\n"
        "vcmp.s EZ, S210, S210\n"
        "vrsq.s S210, S210\n"
        "vcmovt.s S210, S211, 0\n"
        ".word 0xDE0000FF  /* vpfxd [-1:1],[-1:1],[-1:1],[-1:1] */\n"
        "vscl.q C200, C200, S210\n"
        "vdot.q S110, C100, C200\n"
        "vneg.q C210, C200\n"
        "vscl.q C000, C100, S210\n"
        "vscl.q C010, C100, S211\n"
        "vscl.q C020, C100, S212\n"
        "vscl.q C030, C100, S213\n"
        "vadd.s S000, S000, S110\n"
        "vadd.s S011, S011, S110\n"
        "vadd.s S022, S022, S110\n"
        "vadd.s S033, S033, S110\n"
        "sv.q C000, 0(%1)\n"
        "sv.q C010, 16(%1)\n"
        "sv.q C020, 32(%1)\n"
        "sv.q C030, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (light), "r" (plane)
        : "memory");
    return out;
}
#endif

/* Re-orthonormalizes the rotation part of m around its y axis: z = normalize(x cross y),
   x = normalize(y cross z), y = normalize(y); w of the three columns = 0, translation kept. */
#ifdef __MWERKS__
Mat4 *mat4_orthonormalize(Mat4 *out, Mat4 *m);
#elif !defined(__mips__)
Mat4 *mat4_orthonormalize(Mat4 *out, Mat4 *m)
{
    Mat4 t;

    t.m[4] = m->m[4];
    t.m[5] = m->m[5];
    t.m[6] = m->m[6];
    cross3(&t.m[8], &m->m[0], &t.m[4]);
    cross3(&t.m[0], &t.m[4], &t.m[8]);
    normalize3(&t.m[0]);
    normalize3(&t.m[4]);
    normalize3(&t.m[8]);
    t.m[3] = t.m[7] = t.m[11] = 0.0f;
    t.m[12] = m->m[12];
    t.m[13] = m->m[13];
    t.m[14] = m->m[14];
    t.m[15] = m->m[15];
    *out = t;
    return out;
}
#else
Mat4 *mat4_orthonormalize(Mat4 *out, Mat4 *m)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C100, 0(%2)\n"
        "lv.q C010, 16(%2)\n"
        "lv.q C120, 32(%2)\n"
        "lv.q C130, 48(%2)\n"
        "vzero.t R003\n"
        "vcrsp.t C020, C100, C010\n"
        "vcrsp.t C000, C010, C020\n"
        "vdot.t S100, C000, C000\n"
        "vdot.t S101, C010, C010\n"
        "vdot.t S102, C020, C020\n"
        "vrsq.t C110, C100\n"
        "vscl.t C000, C000, S110\n"
        "vscl.t C010, C010, S111\n"
        "vscl.t C020, C020, S112\n"
        "sv.q C000, 0(%1)\n"
        "sv.q C010, 16(%1)\n"
        "sv.q C020, 32(%1)\n"
        "sv.q C130, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m)
        : "memory");
    return out;
}
#endif
