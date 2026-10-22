// @range 0x0881C4E4-0x0881CD50
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
asm Vec4 *mat4_transform_point(Vec4 *out, Mat4 *m, Vec4 *v)
{
    move v0, a0
    lv.q C100, 0(a1)
    lv.q C110, 16(a1)
    lv.q C120, 32(a1)
    lv.q C130, 48(a1)
    lv.q C200, 0(a2)
    vhtfm4.q C000, E100, C200
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
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
#endif

/* out = a * b */
#ifdef __MWERKS__
asm Mat4 *mat4_mul(Mat4 *out, Mat4 *a, Mat4 *b)
{
    move v0, a0
    lv.q C100, 0(a1)
    lv.q C110, 16(a1)
    lv.q C120, 32(a1)
    lv.q C130, 48(a1)
    lv.q C200, 0(a2)
    lv.q C210, 16(a2)
    lv.q C220, 32(a2)
    lv.q C230, 48(a2)
    vmmul.q E000, E200, E100
    sv.q C000, 0(a0)
    sv.q C010, 16(a0)
    sv.q C020, 32(a0)
    sv.q C030, 48(a0)
    jr ra
    nop
}
#else
Mat4 *mat4_mul(Mat4 *out, Mat4 *a, Mat4 *b)
{
    mul4(out, a, b);
    return out;
}
#endif

/* out = m * s (every element) */
#ifdef __MWERKS__
asm Mat4 *mat4_scale(Mat4 *out, Mat4 *m, float s)
{
    move v0, a0
    mfc1 t0, $f12
    mtv t0, S200
    lv.q C100, 0(a1)
    lv.q C110, 16(a1)
    lv.q C120, 32(a1)
    lv.q C130, 48(a1)
    vmscl.q E000, E100, S200
    sv.q C000, 0(a0)
    sv.q C010, 16(a0)
    sv.q C020, 32(a0)
    sv.q C030, 48(a0)
    jr ra
    nop
}
#else
Mat4 *mat4_scale(Mat4 *out, Mat4 *m, float s)
{
    int i;

    for (i = 0; i < 16; i++) {
        out->m[i] = m->m[i] * s;
    }
    return out;
}
#endif

#ifdef __MWERKS__
asm Mat4 *mat4_transpose(Mat4 *out, Mat4 *m)
{
    move v0, a0
    lv.q C000, 0(a1)
    lv.q C010, 16(a1)
    lv.q C020, 32(a1)
    lv.q C030, 48(a1)
    sv.q R000, 0(a0)
    sv.q R001, 16(a0)
    sv.q R002, 32(a0)
    sv.q R003, 48(a0)
    jr ra
    nop
}
#else
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
#endif

/* Inverse of a rigid transform (orthonormal rotation R, translation t): R^T and -R^T * t; the
   rotation columns get w = 0 and the last column keeps m's w. */
#ifdef __MWERKS__
asm Mat4 *mat4_inverse_ortho(Mat4 *out, Mat4 *m)
{
    move v0, a0
    lv.q C100, 0(a1)
    lv.q C110, 16(a1)
    lv.q C120, 32(a1)
    lv.q C000, 48(a1)
    vzero.t C130
    vtfm3.t C010, M100, C000
    sv.q R100, 0(a0)
    sv.q R101, 16(a0)
    vneg.t C000, C010
    sv.q R102, 32(a0)
    sv.q C000, 48(a0)
    jr ra
    nop
}
#else
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
#endif

/* out = m with v.xyz added to its translation (translate(v) * m for an affine m) */
#ifdef __MWERKS__
asm Mat4 *mat4_translate(Mat4 *out, Mat4 *m, Vec4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    lv.q C010, 16(a1)
    lv.q C020, 32(a1)
    lv.q C030, 48(a1)
    lv.q C100, 0(a2)
    vadd.t C100, C100, C030
    sv.q C000, 0(a0)
    sv.q C010, 16(a0)
    sv.q C020, 32(a0)
    sv.q C100, 48(a0)
    jr ra
    nop
}
#else
Mat4 *mat4_translate(Mat4 *out, Mat4 *m, Vec4 *v)
{
    *out = *m;
    out->m[12] += v->x;
    out->m[13] += v->y;
    out->m[14] += v->z;
    return out;
}
#endif

/* out = rotation_z(a) * m; m == 0 gives the rotation alone */
#ifdef __MWERKS__
asm Mat4 *mat4_mul_rot_z(Mat4 *out, Mat4 *m, float a)
{
    addiu sp, sp, -16
    move v0, a0
    swc1 $f12, 0(sp)
    lv.s S100, 0(sp)
    vcst.s S101, VFPU_2_PI
    vmul.s S100, S100, S101
    vrot.q C000, S100, [c, s, 0, 0]
    vrot.q C010, S100, [-s, c, 0, 0]
    vidt.q C020
    vidt.q C030
    beqzl a1, L_0881C660
    vmmov.q E200, E000
    lv.q C100, 0(a1)
    lv.q C110, 16(a1)
    lv.q C120, 32(a1)
    lv.q C130, 48(a1)
    vmmul.q E200, E100, E000
L_0881C660:
    sv.q C200, 0(a0)
    sv.q C210, 16(a0)
    sv.q C220, 32(a0)
    sv.q C230, 48(a0)
    jr ra
    addiu sp, sp, 16
}
#else
Mat4 *mat4_mul_rot_z(Mat4 *out, Mat4 *m, float a)
{
    return rotate_axis(out, m, a, 2);
}
#endif

/* out = rotation_y(a) * m; m == 0 gives the rotation alone */
#ifdef __MWERKS__
asm Mat4 *mat4_mul_rot_y(Mat4 *out, Mat4 *m, float a)
{
    addiu sp, sp, -16
    move v0, a0
    swc1 $f12, 0(sp)
    lv.s S100, 0(sp)
    vcst.s S101, VFPU_2_PI
    vmul.s S100, S100, S101
    vrot.q C000, S100, [c, 0, -s, 0]
    vidt.q C010
    vrot.q C020, S100, [s, 0, c, 0]
    vidt.q C030
    beqzl a1, L_0881C6BC
    vmmov.q E200, E000
    lv.q C100, 0(a1)
    lv.q C110, 16(a1)
    lv.q C120, 32(a1)
    lv.q C130, 48(a1)
    vmmul.q E200, E100, E000
L_0881C6BC:
    sv.q C200, 0(a0)
    sv.q C210, 16(a0)
    sv.q C220, 32(a0)
    sv.q C230, 48(a0)
    jr ra
    addiu sp, sp, 16
}
#else
Mat4 *mat4_mul_rot_y(Mat4 *out, Mat4 *m, float a)
{
    return rotate_axis(out, m, a, 1);
}
#endif

/* out = rotation_x(a) * m; m == 0 gives the rotation alone */
#ifdef __MWERKS__
asm Mat4 *mat4_mul_rot_x(Mat4 *out, Mat4 *m, float a)
{
    addiu sp, sp, -16
    move v0, a0
    swc1 $f12, 0(sp)
    lv.s S100, 0(sp)
    vcst.s S101, VFPU_2_PI
    vmul.s S100, S100, S101
    vidt.q C000
    vrot.q C010, S100, [0, c, s, 0]
    vrot.q C020, S100, [0, -s, c, 0]
    vidt.q C030
    beqzl a1, L_0881C718
    vmmov.q E200, E000
    lv.q C100, 0(a1)
    lv.q C110, 16(a1)
    lv.q C120, 32(a1)
    lv.q C130, 48(a1)
    vmmul.q E200, E100, E000
L_0881C718:
    sv.q C200, 0(a0)
    sv.q C210, 16(a0)
    sv.q C220, 32(a0)
    sv.q C230, 48(a0)
    jr ra
    addiu sp, sp, 16
}
#else
Mat4 *mat4_mul_rot_x(Mat4 *out, Mat4 *m, float a)
{
    return rotate_axis(out, m, a, 0);
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
asm int mat4_is_identity(Mat4 *m)
{
    lw v1, 0(a0)
    lui a1, 0x3f80
    move t0, a0
    beq v1, a1, L_0881C84C
    move a0, zero
L_0881C844:
    jr ra
    move v0, a0
L_0881C84C:
    lw v0, 20(t0)
    bne v0, v1, L_0881C844
    nop
    lw v1, 40(t0)
    bne v1, v0, L_0881C844
    nop
    lw a1, 60(t0)
    bne a1, v1, L_0881C844
    nop
    lw a0, 4(t0)
    lw v0, 8(t0)
    lw t4, 12(t0)
    lw t2, 16(t0)
    or t3, a0, v0
    lw v1, 24(t0)
    or a2, t3, t4
    lw a3, 28(t0)
    or t1, a2, t2
    lw t9, 32(t0)
    or a1, t1, v1
    lw t7, 36(t0)
    or t8, a1, a3
    lw t5, 44(t0)
    or t6, t8, t9
    lw t3, 48(t0)
    or t4, t6, t7
    lw t2, 52(t0)
    or a0, t4, t5
    lw t1, 56(t0)
    or v1, a0, t3
    or a3, v1, t2
    or a2, a3, t1
    j L_0881C844
    sltiu a0, a2, 1
}
#else
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
#endif

#ifdef __MWERKS__
asm float mat4_trace(Mat4 *m)
{
    lwc1 $f0, 20(a0)
    lwc1 $f2, 0(a0)
    lwc1 $f5, 40(a0)
    lwc1 $f4, 60(a0)
    add.s $f3, $f2, $f0
    add.s $f1, $f3, $f5
    jr ra
    add.s $f0, $f1, $f4
}
#else
float mat4_trace(Mat4 *m)
{
    return m->m[0] + m->m[5] + m->m[10] + m->m[15];
}
#endif

#ifdef __MWERKS__
asm float mat4_determinant(Mat4 *m)
{
    addiu sp, sp, -16
    lv.q C000, 0(a0)
    lv.q C010, 16(a0)
    lv.q C020, 32(a0)
    lv.q C030, 48(a0)
    vpfxs X, Z, Y, W
    vpfxt Y, X, Z, W
    vmul.t C100, C011, C021
    vpfxs X, Z, Y, W
    vpfxt Y, X, Z, W
    vmul.t C110, C001, C021
    vpfxs X, Z, Y, W
    vpfxt Y, X, Z, W
    vmul.t C120, C001, C011
    vpfxs X, Z, Y, W
    vpfxt Y, X, Z, W
    vmul.t C130, C001, C011
    vpfxs Z, Y, X, W
    vpfxt Y, X, Z, W
    vmul.t C200, C011, C021
    vpfxs Z, Y, X, W
    vpfxt Y, X, Z, W
    vmul.t C210, C001, C021
    vpfxs Z, Y, X, W
    vpfxt Y, X, Z, W
    vmul.t C220, C001, C011
    vpfxs Z, Y, X, W
    vpfxt Y, X, Z, W
    vmul.t C230, C001, C011
    vpfxt Z, Y, X, W
    vdot.t S100, C100, C031
    vpfxt Z, Y, X, W
    vdot.t S110, C110, C031
    vpfxt Z, Y, X, W
    vdot.t S120, C120, C031
    vpfxt Z, Y, X, W
    vdot.t S130, C130, C021
    vpfxt X, Z, Y, W
    vdot.t S200, C200, C031
    vpfxt X, Z, Y, W
    vdot.t S210, C210, C031
    vpfxt X, Z, Y, W
    vdot.t S220, C220, C031
    vpfxt X, Z, Y, W
    vdot.t S230, C230, C021
    vsub.q R100, R100, R200
    vmul.q R101, R000, R100
    vpfxs X, -Y, Z, -W
    vfad.q S000, R101
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float mat4_determinant(Mat4 *m)
{
    float o[16];

    return adjugate(o, m->m);
}
#endif

/* out = adjugate of m (m * out = det(m) * I) */
#ifdef __MWERKS__
asm Mat4 *mat4_adjugate(Mat4 *out, Mat4 *m)
{
    move v0, a0
    lv.q C000, 0(a1)
    lv.q C010, 16(a1)
    lv.q C020, 32(a1)
    lv.q C030, 48(a1)
    vpfxs 0, Z, Y, Y
    vpfxt 0, W, W, Z
    vmul.q C100, C020, C030
    vpfxs X, 0, W, Z
    vpfxt Y, 0, X, X
    vmul.q C110, C020, C030
    vpfxs 0, Z, Y, Y
    vpfxt 0, W, W, Z
    vmul.q C120, C030, C020
    vpfxs X, 0, W, Z
    vpfxt Y, 0, X, X
    vmul.q C130, C030, C020
    vsub.q C100, C100, C120
    vsub.q C110, C110, C130
    vmov.p R122, R100
    vmov.p R123, C110
    vpfxs X, -Y, Z, W
    vmov.t R110, C101
    vpfxs X, -Y, Z, W
    vmov.p R121, C112
    vtfm4.q R200, E100, C010
    vtfm4.q R201, E100, C000
    vpfxs 0, Z, Y, Y
    vpfxt 0, W, W, Z
    vmul.q C100, C000, C010
    vpfxs X, 0, W, Z
    vpfxt Y, 0, X, X
    vmul.q C110, C000, C010
    vpfxs 0, Z, Y, Y
    vpfxt 0, W, W, Z
    vmul.q C120, C010, C000
    vpfxs X, 0, W, Z
    vpfxt Y, 0, X, X
    vmul.q C130, C010, C000
    vsub.q C100, C100, C120
    vsub.q C110, C110, C130
    vmov.p R122, R100
    vmov.p R123, C110
    vpfxs X, -Y, Z, W
    vmov.t R110, C101
    vpfxs X, -Y, Z, W
    vmov.p R121, C112
    vtfm4.q R202, E100, C030
    vtfm4.q R203, E100, C020
    .word 0xDE000500  /* vpfxd m,,m, */
    vneg.q C200, C200
    .word 0xDE000A00  /* vpfxd ,m,,m */
    vneg.q C210, C210
    .word 0xDE000500  /* vpfxd m,,m, */
    vneg.q C220, C220
    .word 0xDE000A00  /* vpfxd ,m,,m */
    vneg.q C230, C230
    sv.q C200, 0(a0)
    sv.q C210, 16(a0)
    sv.q C220, 32(a0)
    sv.q C230, 48(a0)
    jr ra
    nop
}
#else
Mat4 *mat4_adjugate(Mat4 *out, Mat4 *m)
{
    float o[16];

    adjugate(o, m->m);
    memcpy(out->m, o, sizeof(o));
    return out;
}
#endif

/* out = inverse of m; *det (if det != 0) receives the determinant. A singular m leaves out as is. */
#ifdef __MWERKS__
asm Mat4 *mat4_invert(Mat4 *out, float *det, Mat4 *m)
{
    move v0, a0
    lv.q C000, 0(a2)
    lv.q C010, 16(a2)
    lv.q C020, 32(a2)
    lv.q C030, 48(a2)
    vpfxs X, Z, Y, W
    vpfxt Y, X, Z, W
    vmul.t C100, C011, C021
    vpfxs X, Z, Y, W
    vpfxt Y, X, Z, W
    vmul.t C110, C001, C021
    vpfxs X, Z, Y, W
    vpfxt Y, X, Z, W
    vmul.t C120, C001, C011
    vpfxs X, Z, Y, W
    vpfxt Y, X, Z, W
    vmul.t C130, C001, C011
    vpfxs Z, Y, X, W
    vpfxt Y, X, Z, W
    vmul.t C200, C011, C021
    vpfxs Z, Y, X, W
    vpfxt Y, X, Z, W
    vmul.t C210, C001, C021
    vpfxs Z, Y, X, W
    vpfxt Y, X, Z, W
    vmul.t C220, C001, C011
    vpfxs Z, Y, X, W
    vpfxt Y, X, Z, W
    vmul.t C230, C001, C011
    vpfxt Z, Y, X, W
    vdot.t S100, C100, C031
    vpfxt Z, Y, X, W
    vdot.t S110, C110, C031
    vpfxt Z, Y, X, W
    vdot.t S120, C120, C031
    vpfxt Z, Y, X, W
    vdot.t S130, C130, C021
    vpfxt X, Z, Y, W
    vdot.t S200, C200, C031
    vpfxt X, Z, Y, W
    vdot.t S210, C210, C031
    vpfxt X, Z, Y, W
    vdot.t S220, C220, C031
    vpfxt X, Z, Y, W
    vdot.t S230, C230, C021
    vsub.q R100, R100, R200
    vmul.q R101, R000, R100
    vpfxs X, -Y, Z, -W
    vfad.q S200, R101
    vcmp.s EZ, S200, S200
    mfv t0, S200
    bvt 0, L_0881CC84
    nop
    vpfxs 0, Z, Y, Y
    vpfxt 0, W, W, Z
    vmul.q C100, C020, C030
    vpfxs X, 0, W, Z
    vpfxt Y, 0, X, X
    vmul.q C110, C020, C030
    vpfxs 0, Z, Y, Y
    vpfxt 0, W, W, Z
    vmul.q C120, C030, C020
    vpfxs X, 0, W, Z
    vpfxt Y, 0, X, X
    vmul.q C130, C030, C020
    vsub.q C100, C100, C120
    vsub.q C110, C110, C130
    vmov.p R122, R100
    vmov.p R123, C110
    vpfxs X, -Y, Z, W
    vmov.t R110, C101
    vpfxs X, -Y, Z, W
    vmov.p R121, C112
    vtfm4.q R200, E100, C010
    vtfm4.q R201, E100, C000
    vpfxs 0, Z, Y, Y
    vpfxt 0, W, W, Z
    vmul.q C100, C000, C010
    vpfxs X, 0, W, Z
    vpfxt Y, 0, X, X
    vmul.q C110, C000, C010
    vpfxs 0, Z, Y, Y
    vpfxt 0, W, W, Z
    vmul.q C120, C010, C000
    vpfxs X, 0, W, Z
    vpfxt Y, 0, X, X
    vmul.q C130, C010, C000
    vsub.q C100, C100, C120
    vsub.q C110, C110, C130
    vmov.p R122, R100
    vmov.p R123, C110
    vpfxs X, -Y, Z, W
    vmov.t R110, C101
    vpfxs X, -Y, Z, W
    vmov.p R121, C112
    vtfm4.q R202, E100, C030
    vtfm4.q R203, E100, C020
    .word 0xDE000500  /* vpfxd m,,m, */
    vneg.q C200, C200
    .word 0xDE000A00  /* vpfxd ,m,,m */
    vneg.q C210, C210
    .word 0xDE000500  /* vpfxd m,,m, */
    vneg.q C220, C220
    .word 0xDE000A00  /* vpfxd ,m,,m */
    vneg.q C230, C230
    mtv t0, S000
    vrcp.s S000, S000
    vmscl.q E200, E200, S000
    sv.q C200, 0(a0)
    sv.q C210, 16(a0)
    sv.q C220, 32(a0)
    sv.q C230, 48(a0)
L_0881CC84:
    bnezl a1, L_0881CC8C
    sw t0, 0(a1)
L_0881CC8C:
    jr ra
    nop
}
#else
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
#endif

/* Planar shadow projection: with the plane normalized by the length of its xyz (n = plane / |plane.xyz|,
   clamped to [-1, 1]; a zero normal gives n = 0), out = dot(light, n) * I - light * n^T. */
#ifdef __MWERKS__
asm Mat4 *mat4_shadow(Mat4 *out, Vec4 *light, Vec4 *plane)
{
    move v0, a0
    lv.q C100, 0(a1)
    lv.q C200, 0(a2)
    vdot.t S210, C200, C200
    vzero.s S211
    vcmp.s EZ, S210, S210
    vrsq.s S210, S210
    vcmovt.s S210, S211, 0
    .word 0xDE0000FF  /* vpfxd [-1:1],[-1:1],[-1:1],[-1:1] */
    vscl.q C200, C200, S210
    vdot.q S110, C100, C200
    vneg.q C210, C200
    vscl.q C000, C100, S210
    vscl.q C010, C100, S211
    vscl.q C020, C100, S212
    vscl.q C030, C100, S213
    vadd.s S000, S000, S110
    vadd.s S011, S011, S110
    vadd.s S022, S022, S110
    vadd.s S033, S033, S110
    sv.q C000, 0(a0)
    sv.q C010, 16(a0)
    sv.q C020, 32(a0)
    sv.q C030, 48(a0)
    jr ra
    nop
}
#else
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
#endif

/* Re-orthonormalizes the rotation part of m around its y axis: z = normalize(x cross y),
   x = normalize(y cross z), y = normalize(y); w of the three columns = 0, translation kept. */
#ifdef __MWERKS__
asm Mat4 *mat4_orthonormalize(Mat4 *out, Mat4 *m)
{
    move v0, a0
    lv.q C100, 0(a1)
    lv.q C010, 16(a1)
    lv.q C120, 32(a1)
    lv.q C130, 48(a1)
    vzero.t R003
    vcrsp.t C020, C100, C010
    vcrsp.t C000, C010, C020
    vdot.t S100, C000, C000
    vdot.t S101, C010, C010
    vdot.t S102, C020, C020
    vrsq.t C110, C100
    vscl.t C000, C000, S110
    vscl.t C010, C010, S111
    vscl.t C020, C020, S112
    sv.q C000, 0(a0)
    sv.q C010, 16(a0)
    sv.q C020, 32(a0)
    sv.q C130, 48(a0)
    jr ra
    nop
}
#else
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
#endif
