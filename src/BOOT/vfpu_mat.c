// @range 0x0881B974-0x0881C4E4
// @gcc 0x0881B974-0x0881BF7C
// @gcc 0x0881C0E8-0x0881C260
// @gcc 0x0881C348-0x0881C4E4
/* sceVfpuMatrix2*, sceVfpuMatrix3* and the start of sceVfpuMatrix4* of Sony's libvfpu. Matrices are
   column-major and products are the usual ones: mul(out, a, b) = a b, transform(out, m, v) = m v, and
   the rotations give R m (R alone when m is NULL), with angles in radians. GCC/VFPU code kept as `asm`
   functions for the PSP build; under `#else` is the portable C. */
#include "vfpu.h"

#ifndef __MWERKS__
#include <math.h>

static int float_bits(float f)
{
    union {
        float f;
        int i;
    } u;

    u.f = f;
    return u.i;
}

static void cross3(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    ScePspFVector3 r;

    r.x = a->y * b->z - a->z * b->y;
    r.y = a->z * b->x - a->x * b->z;
    r.z = a->x * b->y - a->y * b->x;
    *out = r;
}

/* m v for a column-major 3x3 */
static ScePspFVector3 mat3_apply(const ScePspFMatrix3 *m, const ScePspFVector3 *v)
{
    ScePspFVector3 r;

    r.x = m->x.x * v->x + m->y.x * v->y + m->z.x * v->z;
    r.y = m->x.y * v->x + m->y.y * v->y + m->z.y * v->z;
    r.z = m->x.z * v->x + m->y.z * v->y + m->z.z * v->z;
    return r;
}

/* out = r m, or r when m is NULL */
static ScePspFMatrix3 *mat3_rot_apply(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, const ScePspFMatrix3 *r)
{
    if (m == 0) {
        *out = *r;
        return out;
    }
    return mat3_mul(out, r, m);
}
#endif

/* sceVfpuMatrix2Unit */
#ifdef __MWERKS__
ScePspFMatrix2 *mat2_identity(ScePspFMatrix2 *out);
#elif !defined(__mips__)
ScePspFMatrix2 *mat2_identity(ScePspFMatrix2 *out)
{
    out->x.x = 1.0f;
    out->x.y = 0.0f;
    out->y.x = 0.0f;
    out->y.y = 1.0f;
    return out;
}
#else
ScePspFMatrix2 *mat2_identity(ScePspFMatrix2 *out)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "vmidt.p E000\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S010, 8(%1)\n"
        "sv.s S011, 12(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix2Zero */
#ifndef __MWERKS__ /* PSP: vfpu_mat_gcc.c */
ScePspFMatrix2 *mat2_zero(ScePspFMatrix2 *out)
{
    vec2_zero(&out->x);
    vec2_zero(&out->y);
    return out;
}
#else
ScePspFMatrix2 *mat2_zero(ScePspFMatrix2 *out);
#endif

/* sceVfpuMatrix2Copy */
#ifndef __MWERKS__ /* PSP: vfpu_mat_gcc.c */
ScePspFMatrix2 *mat2_copy(ScePspFMatrix2 *out, const ScePspFMatrix2 *m)
{
    *out = *m;
    return out;
}
#else
ScePspFMatrix2 *mat2_copy(ScePspFMatrix2 *out, const ScePspFMatrix2 *m);
#endif

/* sceVfpuMatrix2Transform: m v */
#ifdef __MWERKS__
ScePspFVector2 *mat2_transform(ScePspFVector2 *out, const ScePspFMatrix2 *m, const ScePspFVector2 *v);
#elif !defined(__mips__)
ScePspFVector2 *mat2_transform(ScePspFVector2 *out, const ScePspFMatrix2 *m, const ScePspFVector2 *v)
{
    float x = m->x.x * v->x + m->y.x * v->y;
    float y = m->x.y * v->x + m->y.y * v->y;

    out->x = x;
    out->y = y;
    return out;
}
#else
ScePspFVector2 *mat2_transform(ScePspFVector2 *out, const ScePspFMatrix2 *m, const ScePspFVector2 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, 0(%2)\n"
        "lv.s S101, 4(%2)\n"
        "lv.s S110, 8(%2)\n"
        "lv.s S111, 12(%2)\n"
        "lv.s S200, 0(%3)\n"
        "lv.s S201, 4(%3)\n"
        "vtfm2.p C000, E100, C200\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix2Mul: a b */
#ifdef __MWERKS__
ScePspFMatrix2 *mat2_mul(ScePspFMatrix2 *out, const ScePspFMatrix2 *a, const ScePspFMatrix2 *b);
#elif !defined(__mips__)
ScePspFMatrix2 *mat2_mul(ScePspFMatrix2 *out, const ScePspFMatrix2 *a, const ScePspFMatrix2 *b)
{
    ScePspFMatrix2 r;

    mat2_transform(&r.x, a, &b->x);
    mat2_transform(&r.y, a, &b->y);
    *out = r;
    return out;
}
#else
ScePspFMatrix2 *mat2_mul(ScePspFMatrix2 *out, const ScePspFMatrix2 *a, const ScePspFMatrix2 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, 0(%2)\n"
        "lv.s S101, 4(%2)\n"
        "lv.s S110, 8(%2)\n"
        "lv.s S111, 12(%2)\n"
        "lv.s S200, 0(%3)\n"
        "lv.s S201, 4(%3)\n"
        "lv.s S210, 8(%3)\n"
        "lv.s S211, 12(%3)\n"
        "vmmul.p E000, E200, E100\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S010, 8(%1)\n"
        "sv.s S011, 12(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix2Scale */
#ifndef __MWERKS__ /* PSP: vfpu_mat_gcc.c */
ScePspFMatrix2 *mat2_scale(ScePspFMatrix2 *out, const ScePspFMatrix2 *m, float s)
{
    vec2_scale(&out->x, &m->x, s);
    vec2_scale(&out->y, &m->y, s);
    return out;
}
#else
ScePspFMatrix2 *mat2_scale(ScePspFMatrix2 *out, const ScePspFMatrix2 *m, float s);
#endif

/* sceVfpuMatrix2Transpose */
#ifndef __MWERKS__ /* PSP: vfpu_mat_gcc.c */
ScePspFMatrix2 *mat2_transpose(ScePspFMatrix2 *out, const ScePspFMatrix2 *m)
{
    float xy = m->x.y;
    float yx = m->y.x;

    out->x.x = m->x.x;
    out->x.y = yx;
    out->y.x = xy;
    out->y.y = m->y.y;
    return out;
}
#else
ScePspFMatrix2 *mat2_transpose(ScePspFMatrix2 *out, const ScePspFMatrix2 *m);
#endif

/* sceVfpuMatrix2RotZ */
#ifdef __MWERKS__
ScePspFMatrix2 *mat2_rot_z(ScePspFMatrix2 *out, const ScePspFMatrix2 *m, float angle);
#elif !defined(__mips__)
ScePspFMatrix2 *mat2_rot_z(ScePspFMatrix2 *out, const ScePspFMatrix2 *m, float angle)
{
    ScePspFMatrix2 r;
    float c = cosf(angle);
    float s = sinf(angle);

    r.x.x = c;
    r.x.y = s;
    r.y.x = -s;
    r.y.y = c;
    if (m == 0) {
        *out = r;
        return out;
    }
    return mat2_mul(out, &r, m);
}
#else
ScePspFMatrix2 *mat2_rot_z(ScePspFMatrix2 *out, const ScePspFMatrix2 *m, float angle)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, %3\n"
        "vcst.s S101, VFPU_2_PI\n"
        "vmul.s S100, S100, S101\n"
        "vrot.p C000, S100, [c, s]\n"
        "vrot.p C010, S100, [-s, c]\n"
        "beqzl %2, 1f\n"
        "vmmov.p E200, E000\n"
        "lv.s S020, 0(%2)\n"
        "lv.s S021, 4(%2)\n"
        "lv.s S030, 8(%2)\n"
        "lv.s S031, 12(%2)\n"
        "vmmul.p E200, E100, E000\n"
        "1:\n"
        "sv.s S200, 0(%1)\n"
        "sv.s S201, 4(%1)\n"
        "sv.s S210, 8(%1)\n"
        "sv.s S211, 12(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "m" (angle)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix2IsUnit: compares the bits (so -0 is not zero) */
#ifdef __MWERKS__
int mat2_is_identity(const ScePspFMatrix2 *m);
#elif !defined(__mips__)
int mat2_is_identity(const ScePspFMatrix2 *m)
{
    int d = float_bits(m->x.x);

    if (d != float_bits(1.0f) || float_bits(m->y.y) != d) {
        return 0;
    }
    return (float_bits(m->x.y) | float_bits(m->y.x)) == 0;
}
#else
int mat2_is_identity(const ScePspFMatrix2 *m)
{
    const unsigned int *p = (const unsigned int *)m;
    int r = 0;

    if (p[0] == 0x3F800000 && p[3] == p[0]) {
        r = (p[1] | p[2]) == 0;
    }
    return r;
}
#endif

/* sceVfpuMatrix2Trace */
#ifndef __MWERKS__ /* PSP: vfpu_mat_gcc.c */
float mat2_trace(const ScePspFMatrix2 *m)
{
    return m->x.x + m->y.y;
}
#else
float mat2_trace(const ScePspFMatrix2 *m);
#endif

/* sceVfpuMatrix2Determinant */
#ifdef __MWERKS__
float mat2_determinant(const ScePspFMatrix2 *m);
#elif !defined(__mips__)
float mat2_determinant(const ScePspFMatrix2 *m)
{
    return m->x.x * m->y.y - m->x.y * m->y.x;
}
#else
float mat2_determinant(const ScePspFMatrix2 *m)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%1)\n"
        "lv.s S001, 4(%1)\n"
        "lv.s S010, 8(%1)\n"
        "lv.s S011, 12(%1)\n"
        "vdet.p S000, C000, C010\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (m)
        : "memory");
    return r;
}
#endif

/* sceVfpuMatrix2Adjoint */
#ifdef __MWERKS__
ScePspFMatrix2 *mat2_adjoint(ScePspFMatrix2 *out, const ScePspFMatrix2 *m);
#elif !defined(__mips__)
ScePspFMatrix2 *mat2_adjoint(ScePspFMatrix2 *out, const ScePspFMatrix2 *m)
{
    float xx = m->x.x;
    float xy = m->x.y;
    float yx = m->y.x;

    out->x.x = m->y.y;
    out->x.y = -yx;
    out->y.x = -xy;
    out->y.y = xx;
    return out;
}
#else
ScePspFMatrix2 *mat2_adjoint(ScePspFMatrix2 *out, const ScePspFMatrix2 *m)
{
    float xx = m->x.x;
    float xy = m->x.y;
    float yx = m->y.x;

    out->x.x = m->y.y;
    out->x.y = -yx;
    out->y.x = -xy;
    out->y.y = xx;
    return out;
}
#endif

/* sceVfpuMatrix3Unit */
#ifdef __MWERKS__
ScePspFMatrix3 *mat3_identity(ScePspFMatrix3 *out);
#elif !defined(__mips__)
ScePspFMatrix3 *mat3_identity(ScePspFMatrix3 *out)
{
    mat3_zero(out);
    out->x.x = 1.0f;
    out->y.y = 1.0f;
    out->z.z = 1.0f;
    return out;
}
#else
ScePspFMatrix3 *mat3_identity(ScePspFMatrix3 *out)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "vmidt.t E000\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        "sv.s S010, 12(%1)\n"
        "sv.s S011, 16(%1)\n"
        "sv.s S012, 20(%1)\n"
        "sv.s S020, 24(%1)\n"
        "sv.s S021, 28(%1)\n"
        "sv.s S022, 32(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix3Zero */
#ifdef __MWERKS__
ScePspFMatrix3 *mat3_zero(ScePspFMatrix3 *out);
#elif !defined(__mips__)
ScePspFMatrix3 *mat3_zero(ScePspFMatrix3 *out)
{
    vec3_zero(&out->x);
    vec3_zero(&out->y);
    vec3_zero(&out->z);
    return out;
}
#else
ScePspFMatrix3 *mat3_zero(ScePspFMatrix3 *out)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "vmzero.t E000\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        "sv.s S010, 12(%1)\n"
        "sv.s S011, 16(%1)\n"
        "sv.s S012, 20(%1)\n"
        "sv.s S020, 24(%1)\n"
        "sv.s S021, 28(%1)\n"
        "sv.s S022, 32(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix3Copy */
#ifndef __MWERKS__ /* PSP: vfpu_mat_gcc.c */
ScePspFMatrix3 *mat3_copy(ScePspFMatrix3 *out, const ScePspFMatrix3 *m)
{
    *out = *m;
    return out;
}
#else
ScePspFMatrix3 *mat3_copy(ScePspFMatrix3 *out, const ScePspFMatrix3 *m);
#endif

/* sceVfpuMatrix3Transform: m v */
#ifdef __MWERKS__
ScePspFVector3 *mat3_transform_vec3(ScePspFVector3 *out, const ScePspFMatrix3 *m, const ScePspFVector3 *v);
#elif !defined(__mips__)
ScePspFVector3 *mat3_transform_vec3(ScePspFVector3 *out, const ScePspFMatrix3 *m, const ScePspFVector3 *v)
{
    *out = mat3_apply(m, v);
    return out;
}
#else
ScePspFVector3 *mat3_transform_vec3(ScePspFVector3 *out, const ScePspFMatrix3 *m, const ScePspFVector3 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, 0(%2)\n"
        "lv.s S101, 4(%2)\n"
        "lv.s S102, 8(%2)\n"
        "lv.s S110, 12(%2)\n"
        "lv.s S111, 16(%2)\n"
        "lv.s S112, 20(%2)\n"
        "lv.s S120, 24(%2)\n"
        "lv.s S121, 28(%2)\n"
        "lv.s S122, 32(%2)\n"
        "lv.s S200, 0(%3)\n"
        "lv.s S201, 4(%3)\n"
        "lv.s S202, 8(%3)\n"
        "vtfm3.t C000, E100, C200\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix3Mul: a b */
#ifdef __MWERKS__
ScePspFMatrix3 *mat3_mul(ScePspFMatrix3 *out, const ScePspFMatrix3 *a, const ScePspFMatrix3 *b);
#elif !defined(__mips__)
ScePspFMatrix3 *mat3_mul(ScePspFMatrix3 *out, const ScePspFMatrix3 *a, const ScePspFMatrix3 *b)
{
    ScePspFMatrix3 r;

    r.x = mat3_apply(a, &b->x);
    r.y = mat3_apply(a, &b->y);
    r.z = mat3_apply(a, &b->z);
    *out = r;
    return out;
}
#else
ScePspFMatrix3 *mat3_mul(ScePspFMatrix3 *out, const ScePspFMatrix3 *a, const ScePspFMatrix3 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, 0(%2)\n"
        "lv.s S101, 4(%2)\n"
        "lv.s S102, 8(%2)\n"
        "lv.s S110, 12(%2)\n"
        "lv.s S111, 16(%2)\n"
        "lv.s S112, 20(%2)\n"
        "lv.s S120, 24(%2)\n"
        "lv.s S121, 28(%2)\n"
        "lv.s S122, 32(%2)\n"
        "lv.s S200, 0(%3)\n"
        "lv.s S201, 4(%3)\n"
        "lv.s S202, 8(%3)\n"
        "lv.s S210, 12(%3)\n"
        "lv.s S211, 16(%3)\n"
        "lv.s S212, 20(%3)\n"
        "lv.s S220, 24(%3)\n"
        "lv.s S221, 28(%3)\n"
        "lv.s S222, 32(%3)\n"
        "vmmul.t E000, E200, E100\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        "sv.s S010, 12(%1)\n"
        "sv.s S011, 16(%1)\n"
        "sv.s S012, 20(%1)\n"
        "sv.s S020, 24(%1)\n"
        "sv.s S021, 28(%1)\n"
        "sv.s S022, 32(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix3Scale */
#ifdef __MWERKS__
ScePspFMatrix3 *mat3_scale(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float s);
#elif !defined(__mips__)
ScePspFMatrix3 *mat3_scale(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float s)
{
    vec3_scale(&out->x, &m->x, s);
    vec3_scale(&out->y, &m->y, s);
    vec3_scale(&out->z, &m->z, s);
    return out;
}
#else
ScePspFMatrix3 *mat3_scale(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float s)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %3\n"
        "mtv $8, S200\n"
        "lv.s S100, 0(%2)\n"
        "lv.s S101, 4(%2)\n"
        "lv.s S102, 8(%2)\n"
        "lv.s S110, 12(%2)\n"
        "lv.s S111, 16(%2)\n"
        "lv.s S112, 20(%2)\n"
        "lv.s S120, 24(%2)\n"
        "lv.s S121, 28(%2)\n"
        "lv.s S122, 32(%2)\n"
        "vmscl.t E000, E100, S200\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        "sv.s S010, 12(%1)\n"
        "sv.s S011, 16(%1)\n"
        "sv.s S012, 20(%1)\n"
        "sv.s S020, 24(%1)\n"
        "sv.s S021, 28(%1)\n"
        "sv.s S022, 32(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "f" (s)
        : "memory", "$8");
    return out;
}
#endif

/* sceVfpuMatrix3Transpose */
#ifdef __MWERKS__
ScePspFMatrix3 *mat3_transpose(ScePspFMatrix3 *out, const ScePspFMatrix3 *m);
#elif !defined(__mips__)
ScePspFMatrix3 *mat3_transpose(ScePspFMatrix3 *out, const ScePspFMatrix3 *m)
{
    ScePspFMatrix3 r;

    r.x.x = m->x.x;
    r.x.y = m->y.x;
    r.x.z = m->z.x;
    r.y.x = m->x.y;
    r.y.y = m->y.y;
    r.y.z = m->z.y;
    r.z.x = m->x.z;
    r.z.y = m->y.z;
    r.z.z = m->z.z;
    *out = r;
    return out;
}
#else
ScePspFMatrix3 *mat3_transpose(ScePspFMatrix3 *out, const ScePspFMatrix3 *m)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lw $8, 0(%2)\n"
        "lw $9, 4(%2)\n"
        "lw $10, 8(%2)\n"
        "lw $11, 12(%2)\n"
        "lw $12, 16(%2)\n"
        "lw $13, 20(%2)\n"
        "lw $14, 24(%2)\n"
        "lw $15, 28(%2)\n"
        "lw $24, 32(%2)\n"
        "sw $8, 0(%1)\n"
        "sw $11, 4(%1)\n"
        "sw $14, 8(%1)\n"
        "sw $9, 12(%1)\n"
        "sw $12, 16(%1)\n"
        "sw $15, 20(%1)\n"
        "sw $10, 24(%1)\n"
        "sw $13, 28(%1)\n"
        "sw $24, 32(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m)
        : "memory", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24");
    return out;
}
#endif

/* sceVfpuMatrix3RotZ */
#ifdef __MWERKS__
ScePspFMatrix3 *mat3_rot_z(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle);
#elif !defined(__mips__)
ScePspFMatrix3 *mat3_rot_z(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle)
{
    ScePspFMatrix3 r;
    float c = cosf(angle);
    float s = sinf(angle);

    mat3_identity(&r);
    r.x.x = c;
    r.x.y = s;
    r.y.x = -s;
    r.y.y = c;
    return mat3_rot_apply(out, m, &r);
}
#else
ScePspFMatrix3 *mat3_rot_z(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, %3\n"
        "vcst.s S101, VFPU_2_PI\n"
        "vmul.s S100, S100, S101\n"
        "vrot.t C000, S100, [c, s, 0]\n"
        "vrot.t C010, S100, [-s, c, 0]\n"
        "vidt.q C020\n"
        "beqzl %2, 1f\n"
        "vmmov.t E200, E000\n"
        "lv.s S100, 0(%2)\n"
        "lv.s S101, 4(%2)\n"
        "lv.s S102, 8(%2)\n"
        "lv.s S110, 12(%2)\n"
        "lv.s S111, 16(%2)\n"
        "lv.s S112, 20(%2)\n"
        "lv.s S120, 24(%2)\n"
        "lv.s S121, 28(%2)\n"
        "lv.s S122, 32(%2)\n"
        "vmmul.t E200, E100, E000\n"
        "1:\n"
        "sv.s S200, 0(%1)\n"
        "sv.s S201, 4(%1)\n"
        "sv.s S202, 8(%1)\n"
        "sv.s S210, 12(%1)\n"
        "sv.s S211, 16(%1)\n"
        "sv.s S212, 20(%1)\n"
        "sv.s S220, 24(%1)\n"
        "sv.s S221, 28(%1)\n"
        "sv.s S222, 32(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "m" (angle)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix3RotY */
#ifdef __MWERKS__
ScePspFMatrix3 *mat3_rot_y(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle);
#elif !defined(__mips__)
ScePspFMatrix3 *mat3_rot_y(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle)
{
    ScePspFMatrix3 r;
    float c = cosf(angle);
    float s = sinf(angle);

    mat3_identity(&r);
    r.x.x = c;
    r.x.z = -s;
    r.z.x = s;
    r.z.z = c;
    return mat3_rot_apply(out, m, &r);
}
#else
ScePspFMatrix3 *mat3_rot_y(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, %3\n"
        "vcst.s S101, VFPU_2_PI\n"
        "vmul.s S100, S100, S101\n"
        "vrot.t C000, S100, [c, 0, -s]\n"
        "vidt.q C010\n"
        "vrot.t C020, S100, [s, 0, c]\n"
        "beqzl %2, 1f\n"
        "vmmov.t E200, E000\n"
        "lv.s S100, 0(%2)\n"
        "lv.s S101, 4(%2)\n"
        "lv.s S102, 8(%2)\n"
        "lv.s S110, 12(%2)\n"
        "lv.s S111, 16(%2)\n"
        "lv.s S112, 20(%2)\n"
        "lv.s S120, 24(%2)\n"
        "lv.s S121, 28(%2)\n"
        "lv.s S122, 32(%2)\n"
        "vmmul.t E200, E100, E000\n"
        "1:\n"
        "sv.s S200, 0(%1)\n"
        "sv.s S201, 4(%1)\n"
        "sv.s S202, 8(%1)\n"
        "sv.s S210, 12(%1)\n"
        "sv.s S211, 16(%1)\n"
        "sv.s S212, 20(%1)\n"
        "sv.s S220, 24(%1)\n"
        "sv.s S221, 28(%1)\n"
        "sv.s S222, 32(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "m" (angle)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix3RotX */
#ifdef __MWERKS__
ScePspFMatrix3 *mat3_rot_x(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle);
#elif !defined(__mips__)
ScePspFMatrix3 *mat3_rot_x(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle)
{
    ScePspFMatrix3 r;
    float c = cosf(angle);
    float s = sinf(angle);

    mat3_identity(&r);
    r.y.y = c;
    r.y.z = s;
    r.z.y = -s;
    r.z.z = c;
    return mat3_rot_apply(out, m, &r);
}
#else
ScePspFMatrix3 *mat3_rot_x(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, %3\n"
        "vcst.s S101, VFPU_2_PI\n"
        "vmul.s S100, S100, S101\n"
        "vidt.q C000\n"
        "vrot.t C010, S100, [0, c, s]\n"
        "vrot.t C020, S100, [0, -s, c]\n"
        "beqzl %2, 1f\n"
        "vmmov.t E200, E000\n"
        "lv.s S100, 0(%2)\n"
        "lv.s S101, 4(%2)\n"
        "lv.s S102, 8(%2)\n"
        "lv.s S110, 12(%2)\n"
        "lv.s S111, 16(%2)\n"
        "lv.s S112, 20(%2)\n"
        "lv.s S120, 24(%2)\n"
        "lv.s S121, 28(%2)\n"
        "lv.s S122, 32(%2)\n"
        "vmmul.t E200, E100, E000\n"
        "1:\n"
        "sv.s S200, 0(%1)\n"
        "sv.s S201, 4(%1)\n"
        "sv.s S202, 8(%1)\n"
        "sv.s S210, 12(%1)\n"
        "sv.s S211, 16(%1)\n"
        "sv.s S212, 20(%1)\n"
        "sv.s S220, 24(%1)\n"
        "sv.s S221, 28(%1)\n"
        "sv.s S222, 32(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "m" (angle)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix3Rot: Rx(angles->x) Ry(angles->y) Rz(angles->z) m (m may be NULL) */
#ifdef __MWERKS__
asm ScePspFMatrix3 *mat3_from_euler(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, const ScePspFVector3 *angles)
{
    lwc1 $f2, 8(a2)
    addiu sp, sp, -16
    move v0, a0
    swc1 $f2, 0(sp)
    lv.s S100, 0(sp)
    vcst.s S101, VFPU_2_PI
    vmul.s S100, S100, S101
    vrot.t C000, S100, [c, s, 0]
    vrot.t C010, S100, [-s, c, 0]
    vidt.q C020
    beqzl a1, @58
    vmmov.t E200, E000
    lv.s S100, 0(a1)
    lv.s S101, 4(a1)
    lv.s S102, 8(a1)
    lv.s S110, 12(a1)
    lv.s S111, 16(a1)
    lv.s S112, 20(a1)
    lv.s S120, 24(a1)
    lv.s S121, 28(a1)
    lv.s S122, 32(a1)
    vmmul.t E200, E100, E000
@58:
    sv.s S200, 0(a0)
    sv.s S201, 4(a0)
    sv.s S202, 8(a0)
    sv.s S210, 12(a0)
    sv.s S211, 16(a0)
    sv.s S212, 20(a0)
    sv.s S220, 24(a0)
    sv.s S221, 28(a0)
    sv.s S222, 32(a0)
    lwc1 $f1, 4(a2)
    swc1 $f1, 4(sp)
    lv.s S100, 4(sp)
    vcst.s S101, VFPU_2_PI
    vmul.s S100, S100, S101
    vrot.t C000, S100, [c, 0, -s]
    vidt.q C010
    vrot.t C020, S100, [s, 0, c]
    beqzl a0, @cc
    vmmov.t E200, E000
    lv.s S100, 0(a0)
    lv.s S101, 4(a0)
    lv.s S102, 8(a0)
    lv.s S110, 12(a0)
    lv.s S111, 16(a0)
    lv.s S112, 20(a0)
    lv.s S120, 24(a0)
    lv.s S121, 28(a0)
    lv.s S122, 32(a0)
    vmmul.t E200, E100, E000
@cc:
    sv.s S200, 0(a0)
    sv.s S201, 4(a0)
    sv.s S202, 8(a0)
    sv.s S210, 12(a0)
    sv.s S211, 16(a0)
    sv.s S212, 20(a0)
    sv.s S220, 24(a0)
    sv.s S221, 28(a0)
    sv.s S222, 32(a0)
    lwc1 $f1, 0(a2)
    swc1 $f1, 8(sp)
    lv.s S100, 8(sp)
    vcst.s S101, VFPU_2_PI
    vmul.s S100, S100, S101
    vidt.q C000
    vrot.t C010, S100, [0, c, s]
    vrot.t C020, S100, [0, -s, c]
    beqzl a0, @140
    vmmov.t E200, E000
    lv.s S100, 0(a0)
    lv.s S101, 4(a0)
    lv.s S102, 8(a0)
    lv.s S110, 12(a0)
    lv.s S111, 16(a0)
    lv.s S112, 20(a0)
    lv.s S120, 24(a0)
    lv.s S121, 28(a0)
    lv.s S122, 32(a0)
    vmmul.t E200, E100, E000
@140:
    sv.s S200, 0(a0)
    sv.s S201, 4(a0)
    sv.s S202, 8(a0)
    sv.s S210, 12(a0)
    sv.s S211, 16(a0)
    sv.s S212, 20(a0)
    sv.s S220, 24(a0)
    sv.s S221, 28(a0)
    sv.s S222, 32(a0)
    jr ra
    addiu sp, sp, 16
}
#else
ScePspFMatrix3 *mat3_from_euler(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, const ScePspFVector3 *angles)
{
    float x = angles->x;
    float y = angles->y;

    mat3_rot_z(out, m, angles->z);
    mat3_rot_y(out, out, y);
    return mat3_rot_x(out, out, x);
}
#endif

/* sceVfpuMatrix3IsUnit: compares the bits (so -0 is not zero) */
#ifdef __MWERKS__
int mat3_is_identity(const ScePspFMatrix3 *m);
#elif !defined(__mips__)
int mat3_is_identity(const ScePspFMatrix3 *m)
{
    int d = float_bits(m->x.x);

    if (d != float_bits(1.0f) || float_bits(m->y.y) != d || float_bits(m->z.z) != d) {
        return 0;
    }
    return (float_bits(m->x.y) | float_bits(m->x.z) | float_bits(m->y.x) | float_bits(m->y.z) |
            float_bits(m->z.x) | float_bits(m->z.y)) == 0;
}
#else
int mat3_is_identity(const ScePspFMatrix3 *m)
{
    const unsigned int *p = (const unsigned int *)m;
    int r = 0;

    if (p[0] == 0x3F800000 && p[4] == p[0] && p[8] == p[4]) {
        r = (p[1] | p[2] | p[3] | p[5] | p[6] | p[7]) == 0;
    }
    return r;
}
#endif

/* sceVfpuMatrix3Trace */
#ifndef __MWERKS__ /* PSP: vfpu_mat_gcc.c */
float mat3_trace(const ScePspFMatrix3 *m)
{
    return m->x.x + m->y.y + m->z.z;
}
#else
float mat3_trace(const ScePspFMatrix3 *m);
#endif

/* sceVfpuMatrix3Determinant: x . (y x z) */
#ifdef __MWERKS__
float mat3_determinant(const ScePspFMatrix3 *m);
#elif !defined(__mips__)
float mat3_determinant(const ScePspFMatrix3 *m)
{
    ScePspFVector3 c;

    cross3(&c, &m->y, &m->z);
    return m->x.x * c.x + m->x.y * c.y + m->x.z * c.z;
}
#else
float mat3_determinant(const ScePspFMatrix3 *m)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, 0(%1)\n"
        "lv.s S101, 4(%1)\n"
        "lv.s S102, 8(%1)\n"
        "lv.s S110, 12(%1)\n"
        "lv.s S111, 16(%1)\n"
        "lv.s S112, 20(%1)\n"
        "lv.s S120, 24(%1)\n"
        "lv.s S121, 28(%1)\n"
        "lv.s S122, 32(%1)\n"
        "vpfxs y, z, x, w\n"
        "vpfxt z, x, y, w\n"
        "vmul.t C000, C110, C120\n"
        "vpfxs z, x, y, w\n"
        "vpfxt y, z, x, w\n"
        "vmul.t C010, C100, C110\n"
        "vdot.t S000, C100, C000\n"
        "vdot.t S010, C010, C120\n"
        "vsub.s S000, S000, S010\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (m)
        : "memory");
    return r;
}
#endif

/* sceVfpuMatrix3Adjoint: the adjugate, whose rows are y x z, z x x and x x y */
#ifdef __MWERKS__
ScePspFMatrix3 *mat3_adjoint(ScePspFMatrix3 *out, const ScePspFMatrix3 *m);
#elif !defined(__mips__)
ScePspFMatrix3 *mat3_adjoint(ScePspFMatrix3 *out, const ScePspFMatrix3 *m)
{
    ScePspFVector3 a;
    ScePspFVector3 b;
    ScePspFVector3 c;

    cross3(&a, &m->y, &m->z);
    cross3(&b, &m->z, &m->x);
    cross3(&c, &m->x, &m->y);
    out->x.x = a.x;
    out->x.y = b.x;
    out->x.z = c.x;
    out->y.x = a.y;
    out->y.y = b.y;
    out->y.z = c.y;
    out->z.x = a.z;
    out->z.y = b.z;
    out->z.z = c.z;
    return out;
}
#else
ScePspFMatrix3 *mat3_adjoint(ScePspFMatrix3 *out, const ScePspFMatrix3 *m)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, 0(%2)\n"
        "lv.s S101, 4(%2)\n"
        "lv.s S102, 8(%2)\n"
        "lv.s S110, 12(%2)\n"
        "lv.s S111, 16(%2)\n"
        "lv.s S112, 20(%2)\n"
        "lv.s S120, 24(%2)\n"
        "lv.s S121, 28(%2)\n"
        "lv.s S122, 32(%2)\n"
        "vpfxt 0, z, -y, w\n"
        "vdot.t S000, R101, R102\n"
        "vpfxt -z, 0, x, w\n"
        "vdot.t S001, R101, R102\n"
        "vpfxt y, -x, 0, w\n"
        "vdot.t S002, R101, R102\n"
        "vpfxt 0, -z, y, w\n"
        "vdot.t S010, R100, R102\n"
        "vpfxt z, 0, -x, w\n"
        "vdot.t S011, R100, R102\n"
        "vpfxt -y, x, 0, w\n"
        "vdot.t S012, R100, R102\n"
        "vpfxt 0, z, -y, w\n"
        "vdot.t S020, R100, R101\n"
        "vpfxt -z, 0, x, w\n"
        "vdot.t S021, R100, R101\n"
        "vpfxt y, -x, 0, w\n"
        "vdot.t S022, R100, R101\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        "sv.s S010, 12(%1)\n"
        "sv.s S011, 16(%1)\n"
        "sv.s S012, 20(%1)\n"
        "sv.s S020, 24(%1)\n"
        "sv.s S021, 28(%1)\n"
        "sv.s S022, 32(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m)
        : "memory");
    return out;
}
#endif

/* adjoint / det; returns NULL (out untouched) for a singular m. The determinant goes to *det if det
   is not NULL. */
#ifdef __MWERKS__
asm ScePspFMatrix3 *mat3_inverse(ScePspFMatrix3 *out, float *det, const ScePspFMatrix3 *m)
{
    addiu sp, sp, -16
    move v0, a0
    lv.s S100, 0(a2)
    lv.s S101, 4(a2)
    lv.s S102, 8(a2)
    lv.s S110, 12(a2)
    lv.s S111, 16(a2)
    lv.s S112, 20(a2)
    lv.s S120, 24(a2)
    lv.s S121, 28(a2)
    lv.s S122, 32(a2)
    vpfxs y, z, x, w
    vpfxt z, x, y, w
    vmul.t C000, C110, C120
    vpfxs z, x, y, w
    vpfxt y, z, x, w
    vmul.t C010, C100, C110
    vdot.t S000, C100, C000
    vdot.t S010, C010, C120
    vsub.s S000, S000, S010
    vcmp.s ez, S000, S000
    sv.s S000, 0(sp)
    bvtl 0, @d4
    move v0, zero
    vrcp.s S030, S000
    vpfxt 0, z, -y, w
    vdot.t S000, R101, R102
    vpfxt -z, 0, x, w
    vdot.t S001, R101, R102
    vpfxt y, -x, 0, w
    vdot.t S002, R101, R102
    vpfxt 0, -z, y, w
    vdot.t S010, R100, R102
    vpfxt z, 0, -x, w
    vdot.t S011, R100, R102
    vpfxt -y, x, 0, w
    vdot.t S012, R100, R102
    vpfxt 0, z, -y, w
    vdot.t S020, R100, R101
    vpfxt -z, 0, x, w
    vdot.t S021, R100, R101
    vpfxt y, -x, 0, w
    vdot.t S022, R100, R101
    vmscl.t E000, E000, S030
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    sv.s S010, 12(a0)
    sv.s S011, 16(a0)
    sv.s S012, 20(a0)
    sv.s S020, 24(a0)
    sv.s S021, 28(a0)
    sv.s S022, 32(a0)
@d4:
    beqz a1, @e0
    lwc1 $f1, 0(sp)
    swc1 $f1, 0(a1)
@e0:
    jr ra
    addiu sp, sp, 16
}
#else
ScePspFMatrix3 *mat3_inverse(ScePspFMatrix3 *out, float *det, const ScePspFMatrix3 *m)
{
    float d = mat3_determinant(m);
    ScePspFMatrix3 *r = 0;

    if (d != 0.0f) {
        ScePspFMatrix3 a;

        mat3_adjoint(&a, m);
        r = mat3_scale(out, &a, 1.0f / d);
    }
    if (det != 0) {
        *det = d;
    }
    return r;
}
#endif

/* sceVfpuMatrix3Normalize: x' = y x (x x y), z' = x x y, each normalized (no zero guard). The
   original never loads its y' register (it stores whatever C010 holds, normalized); normalize(y) is
   the evident intent. */
#ifdef __MWERKS__
ScePspFMatrix3 *mat3_normalize(ScePspFMatrix3 *out, const ScePspFMatrix3 *m);
#elif !defined(__mips__)
ScePspFMatrix3 *mat3_normalize(ScePspFMatrix3 *out, const ScePspFMatrix3 *m)
{
    ScePspFVector3 x;
    ScePspFVector3 y = m->y;
    ScePspFVector3 z;

    cross3(&z, &m->x, &m->y);
    cross3(&x, &m->y, &z);
    vec3_scale(&out->x, &x, 1.0f / sqrtf(vec3_dot(&x, &x)));
    vec3_scale(&out->y, &y, 1.0f / sqrtf(vec3_dot(&y, &y)));
    vec3_scale(&out->z, &z, 1.0f / sqrtf(vec3_dot(&z, &z)));
    return out;
}
#else
ScePspFMatrix3 *mat3_normalize(ScePspFMatrix3 *out, const ScePspFMatrix3 *m)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S100, 0(%2)\n"
        "lv.s S101, 4(%2)\n"
        "lv.s S102, 8(%2)\n"
        "lv.s S110, 12(%2)\n"
        "lv.s S111, 16(%2)\n"
        "lv.s S112, 20(%2)\n"
        "lv.s S120, 24(%2)\n"
        "lv.s S121, 28(%2)\n"
        "lv.s S122, 32(%2)\n"
        "vcrsp.t C020, C100, C110\n"
        "vcrsp.t C000, C110, C020\n"
        "vdot.t S100, C000, C000\n"
        "vdot.t S101, C010, C010\n"
        "vdot.t S102, C020, C020\n"
        "vrsq.t C110, C100\n"
        "vscl.t C000, C000, S110\n"
        "vscl.t C010, C010, S111\n"
        "vscl.t C020, C020, S112\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        "sv.s S010, 12(%1)\n"
        "sv.s S011, 16(%1)\n"
        "sv.s S012, 20(%1)\n"
        "sv.s S020, 24(%1)\n"
        "sv.s S021, 28(%1)\n"
        "sv.s S022, 32(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix4Unit */
#ifdef __MWERKS__
ScePspFMatrix4 *mat4_identity(ScePspFMatrix4 *out);
#elif !defined(__mips__)
ScePspFMatrix4 *mat4_identity(ScePspFMatrix4 *out)
{
    mat4_zero(out);
    out->x.x = 1.0f;
    out->y.y = 1.0f;
    out->z.z = 1.0f;
    out->w.w = 1.0f;
    return out;
}
#else
ScePspFMatrix4 *mat4_identity(ScePspFMatrix4 *out)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "vmidt.q E000\n"
        "sv.q C000, 0(%1)\n"
        "sv.q C010, 16(%1)\n"
        "sv.q C020, 32(%1)\n"
        "sv.q C030, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix4Zero */
#ifdef __MWERKS__
ScePspFMatrix4 *mat4_zero(ScePspFMatrix4 *out);
#elif !defined(__mips__)
ScePspFMatrix4 *mat4_zero(ScePspFMatrix4 *out)
{
    vec4_zero(&out->x);
    vec4_zero(&out->y);
    vec4_zero(&out->z);
    vec4_zero(&out->w);
    return out;
}
#else
ScePspFMatrix4 *mat4_zero(ScePspFMatrix4 *out)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "vmzero.q E000\n"
        "sv.q C000, 0(%1)\n"
        "sv.q C010, 16(%1)\n"
        "sv.q C020, 32(%1)\n"
        "sv.q C030, 48(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix4Copy */
#ifndef __MWERKS__ /* PSP: vfpu_mat_gcc.c */
ScePspFMatrix4 *mat4_copy(ScePspFMatrix4 *out, const ScePspFMatrix4 *m)
{
    *out = *m;
    return out;
}
#else
ScePspFMatrix4 *mat4_copy(ScePspFMatrix4 *out, const ScePspFMatrix4 *m);
#endif

/* sceVfpuMatrix4SetTransfer: the w column (translation) = v */
#ifdef __MWERKS__
ScePspFMatrix4 *mat4_set_transfer(ScePspFMatrix4 *m, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspFMatrix4 *mat4_set_transfer(ScePspFMatrix4 *m, const ScePspFVector4 *v)
{
    m->w = *v;
    return m;
}
#else
ScePspFMatrix4 *mat4_set_transfer(ScePspFMatrix4 *m, const ScePspFVector4 *v)
{
    m->w.x = v->x;
    m->w.y = v->y;
    m->w.z = v->z;
    m->w.w = v->w;
    return m;
}
#endif

/* sceVfpuMatrix4GetTransfer */
#ifdef __MWERKS__
ScePspFVector4 *mat4_get_transfer(ScePspFVector4 *out, const ScePspFMatrix4 *m);
#elif !defined(__mips__)
ScePspFVector4 *mat4_get_transfer(ScePspFVector4 *out, const ScePspFMatrix4 *m)
{
    *out = m->w;
    return out;
}
#else
ScePspFVector4 *mat4_get_transfer(ScePspFVector4 *out, const ScePspFMatrix4 *m)
{
    out->x = m->w.x;
    out->y = m->w.y;
    out->z = m->w.z;
    out->w = m->w.w;
    return out;
}
#endif

/* sceVfpuMatrix4Transform: m v */
#ifdef __MWERKS__
ScePspFVector4 *mat4_transform_vec4(ScePspFVector4 *out, const ScePspFMatrix4 *m, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspFVector4 *mat4_transform_vec4(ScePspFVector4 *out, const ScePspFMatrix4 *m, const ScePspFVector4 *v)
{
    ScePspFVector4 r;

    r.x = m->x.x * v->x + m->y.x * v->y + m->z.x * v->z + m->w.x * v->w;
    r.y = m->x.y * v->x + m->y.y * v->y + m->z.y * v->z + m->w.y * v->w;
    r.z = m->x.z * v->x + m->y.z * v->y + m->z.z * v->z + m->w.z * v->w;
    r.w = m->x.w * v->x + m->y.w * v->y + m->z.w * v->z + m->w.w * v->w;
    *out = r;
    return out;
}
#else
ScePspFVector4 *mat4_transform_vec4(ScePspFVector4 *out, const ScePspFMatrix4 *m, const ScePspFVector4 *v)
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
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuMatrix4TransformXYZ: the 3x3 part of m times v.xyz; w = v->w */
#ifdef __MWERKS__
ScePspFVector4 *mat4_transform_xyz(ScePspFVector4 *out, const ScePspFMatrix4 *m, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspFVector4 *mat4_transform_xyz(ScePspFVector4 *out, const ScePspFMatrix4 *m, const ScePspFVector4 *v)
{
    ScePspFVector4 r;

    r.x = m->x.x * v->x + m->y.x * v->y + m->z.x * v->z;
    r.y = m->x.y * v->x + m->y.y * v->y + m->z.y * v->z;
    r.z = m->x.z * v->x + m->y.z * v->y + m->z.z * v->z;
    r.w = v->w;
    *out = r;
    return out;
}
#else
ScePspFVector4 *mat4_transform_xyz(ScePspFVector4 *out, const ScePspFMatrix4 *m, const ScePspFVector4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C100, 0(%2)\n"
        "lv.q C110, 16(%2)\n"
        "lv.q C120, 32(%2)\n"
        "lv.q C200, 0(%3)\n"
        "vmov.s S003, S203\n"
        "vtfm3.t C000, E100, C200\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (m), "r" (v)
        : "memory");
    return out;
}
#endif
