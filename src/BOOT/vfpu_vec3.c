// @range 0x0881AB1C-0x0881B244
// @gcc 0x0881AB1C-0x0881AFC8
// @gcc 0x0881AFF8-0x0881B244
/* sceVfpuVector3* of Sony's libvfpu: 12-byte vectors, loaded and stored a float at a time (lv.s/sv.s)
   because they are not 16-byte aligned. GCC/VFPU code kept as `asm` functions for the PSP build;
   under `#else` is the portable C. */
#include "vfpu.h"

#ifndef __MWERKS__
#include <math.h>

/* vpfxd [-1:1]: saturation of the result */
static float sat1(float x)
{
    return x < -1.0f ? -1.0f : x > 1.0f ? 1.0f : x;
}

/* vrsq.s guarded by vcmp ez + vcmovt: 0 for a zero length */
static float rsq0(float d)
{
    return d == 0.0f ? 0.0f : 1.0f / sqrtf(d);
}

/* vsgn */
static float sgn(float x)
{
    return x > 0.0f ? 1.0f : x < 0.0f ? -1.0f : 0.0f;
}
#endif

/* sceVfpuVector3Set */
#ifndef __MWERKS__ /* PSP: vfpu_vec3_gcc.c */
ScePspFVector3 *vec3_set(ScePspFVector3 *out, float x, float y, float z)
{
    out->x = x;
    out->y = y;
    out->z = z;
    return out;
}
#else
ScePspFVector3 *vec3_set(ScePspFVector3 *out, float x, float y, float z);
#endif

/* sceVfpuVector3Copy */
#ifdef __MWERKS__
ScePspFVector3 *vec3_copy(ScePspFVector3 *out, const ScePspFVector3 *v);
#elif !defined(__mips__)
ScePspFVector3 *vec3_copy(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    *out = *v;
    return out;
}
#else
ScePspFVector3 *vec3_copy(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    out->x = v->x;
    out->y = v->y;
    out->z = v->z;
    return out;
}
#endif

/* sceVfpuVector3PositiveZero */
#ifndef __MWERKS__ /* PSP: vfpu_vec3_gcc.c */
ScePspFVector3 *vec3_zero(ScePspFVector3 *out)
{
    out->x = 0.0f;
    out->y = 0.0f;
    out->z = 0.0f;
    return out;
}
#else
ScePspFVector3 *vec3_zero(ScePspFVector3 *out);
#endif

/* sceVfpuVector3NegativeZero */
#ifdef __MWERKS__
ScePspFVector3 *vec3_neg_zero(ScePspFVector3 *out);
#elif !defined(__mips__)
ScePspFVector3 *vec3_neg_zero(ScePspFVector3 *out)
{
    out->x = -0.0f;
    out->y = -0.0f;
    out->z = -0.0f;
    return out;
}
#else
ScePspFVector3 *vec3_neg_zero(ScePspFVector3 *out)
{
    unsigned int *p = (unsigned int *)out;

    p[0] = 0x80000000;
    p[1] = 0x80000000;
    p[2] = 0x80000000;
    return out;
}
#endif

/* sceVfpuVector3Ceil: vf2id, rounds down [?] (see vec2_ceil). */
#ifdef __MWERKS__
ScePspIVector3 *vec3_ceil(ScePspIVector3 *out, const ScePspFVector3 *v);
#elif !defined(__mips__)
ScePspIVector3 *vec3_ceil(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    out->x = (int)floorf(v->x);
    out->y = (int)floorf(v->y);
    out->z = (int)floorf(v->z);
    return out;
}
#else
ScePspIVector3 *vec3_ceil(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "vf2id.t C000, C000, 0\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Trunc (vf2iz) */
#ifdef __MWERKS__
ScePspIVector3 *vec3_trunc(ScePspIVector3 *out, const ScePspFVector3 *v);
#elif !defined(__mips__)
ScePspIVector3 *vec3_trunc(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    out->x = (int)v->x;
    out->y = (int)v->y;
    out->z = (int)v->z;
    return out;
}
#else
ScePspIVector3 *vec3_trunc(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "vf2iz.t C000, C000, 0\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Round (vf2in) */
#ifdef __MWERKS__
ScePspIVector3 *vec3_round(ScePspIVector3 *out, const ScePspFVector3 *v);
#elif !defined(__mips__)
ScePspIVector3 *vec3_round(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    out->x = (int)rintf(v->x);
    out->y = (int)rintf(v->y);
    out->z = (int)rintf(v->z);
    return out;
}
#else
ScePspIVector3 *vec3_round(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "vf2in.t C000, C000, 0\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Floor: vf2iu, rounds up [?] (see vec2_ceil). */
#ifdef __MWERKS__
ScePspIVector3 *vec3_floor(ScePspIVector3 *out, const ScePspFVector3 *v);
#elif !defined(__mips__)
ScePspIVector3 *vec3_floor(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    out->x = (int)ceilf(v->x);
    out->y = (int)ceilf(v->y);
    out->z = (int)ceilf(v->z);
    return out;
}
#else
ScePspIVector3 *vec3_floor(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "vf2iu.t C000, C000, 0\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3FromIVector */
#ifndef __MWERKS__ /* PSP: vfpu_vec3_gcc.c */
ScePspFVector3 *vec3_from_ivec(ScePspFVector3 *out, const ScePspIVector3 *v)
{
    out->x = (float)v->x;
    out->y = (float)v->y;
    out->z = (float)v->z;
    return out;
}
#else
ScePspFVector3 *vec3_from_ivec(ScePspFVector3 *out, const ScePspIVector3 *v);
#endif

/* sceVfpuVector3Add */
#ifdef __MWERKS__
ScePspFVector3 *vec3_add(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
#elif !defined(__mips__)
ScePspFVector3 *vec3_add(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    return out;
}
#else
ScePspFVector3 *vec3_add(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "lv.s S010, 0(%3)\n"
        "lv.s S011, 4(%3)\n"
        "lv.s S012, 8(%3)\n"
        "vadd.t C000, C000, C010\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Sub */
#ifdef __MWERKS__
ScePspFVector3 *vec3_sub(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
#elif !defined(__mips__)
ScePspFVector3 *vec3_sub(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    return out;
}
#else
ScePspFVector3 *vec3_sub(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "lv.s S010, 0(%3)\n"
        "lv.s S011, 4(%3)\n"
        "lv.s S012, 8(%3)\n"
        "vsub.t C000, C000, C010\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Mul */
#ifdef __MWERKS__
ScePspFVector3 *vec3_mul(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
#elif !defined(__mips__)
ScePspFVector3 *vec3_mul(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    out->z = a->z * b->z;
    return out;
}
#else
ScePspFVector3 *vec3_mul(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "lv.s S010, 0(%3)\n"
        "lv.s S011, 4(%3)\n"
        "lv.s S012, 8(%3)\n"
        "vmul.t C000, C000, C010\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Div */
#ifdef __MWERKS__
ScePspFVector3 *vec3_div(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
#elif !defined(__mips__)
ScePspFVector3 *vec3_div(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x / b->x;
    out->y = a->y / b->y;
    out->z = a->z / b->z;
    return out;
}
#else
ScePspFVector3 *vec3_div(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S010, 0(%2)\n"
        "lv.s S011, 4(%2)\n"
        "lv.s S012, 8(%2)\n"
        "lv.s S020, 0(%3)\n"
        "lv.s S021, 4(%3)\n"
        "lv.s S022, 8(%3)\n"
        "vdiv.t C000, C010, C020\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Neg (flips the sign bits) */
#ifdef __MWERKS__
ScePspFVector3 *vec3_neg(ScePspFVector3 *out, const ScePspFVector3 *v);
#elif !defined(__mips__)
ScePspFVector3 *vec3_neg(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    out->x = -v->x;
    out->y = -v->y;
    out->z = -v->z;
    return out;
}
#else
ScePspFVector3 *vec3_neg(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    unsigned int *p = (unsigned int *)out;
    const unsigned int *q = (const unsigned int *)v;

    p[0] = q[0] ^ 0x80000000;
    p[1] = q[1] ^ 0x80000000;
    p[2] = q[2] ^ 0x80000000;
    return out;
}
#endif

/* sceVfpuVector3Abs (clears the sign bits) */
#ifdef __MWERKS__
ScePspFVector3 *vec3_abs(ScePspFVector3 *out, const ScePspFVector3 *v);
#elif !defined(__mips__)
ScePspFVector3 *vec3_abs(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    out->x = fabsf(v->x);
    out->y = fabsf(v->y);
    out->z = fabsf(v->z);
    return out;
}
#else
ScePspFVector3 *vec3_abs(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    unsigned int *p = (unsigned int *)out;
    const unsigned int *q = (const unsigned int *)v;

    p[0] = q[0] & 0x7FFFFFFF;
    p[1] = q[1] & 0x7FFFFFFF;
    p[2] = q[2] & 0x7FFFFFFF;
    return out;
}
#endif

/* sceVfpuVector3Lerp: a + t (b - a) */
#ifdef __MWERKS__
ScePspFVector3 *vec3_lerp(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b, float t);
#elif !defined(__mips__)
ScePspFVector3 *vec3_lerp(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b, float t)
{
    out->x = a->x + t * (b->x - a->x);
    out->y = a->y + t * (b->y - a->y);
    out->z = a->z + t * (b->z - a->z);
    return out;
}
#else
ScePspFVector3 *vec3_lerp(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b, float t)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %4\n"
        "mtv $8, S030\n"
        "lv.s S010, 0(%2)\n"
        "lv.s S011, 4(%2)\n"
        "lv.s S012, 8(%2)\n"
        "lv.s S020, 0(%3)\n"
        "lv.s S021, 4(%3)\n"
        "lv.s S022, 8(%3)\n"
        "vsub.t C000, C020, C010\n"
        "vscl.t C000, C000, S030\n"
        "vadd.t C000, C010, C000\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b), "f" (t)
        : "memory", "$8");
    return out;
}
#endif

/* Cubic Hermite, as vec2_hermite. */
#ifdef __MWERKS__
ScePspFVector3 *vec3_hermite(ScePspFVector3 *out, const ScePspFVector3 *p0, const ScePspFVector3 *v0,
                             const ScePspFVector3 *p1, const ScePspFVector3 *v1, float t);
#elif !defined(__mips__)
ScePspFVector3 *vec3_hermite(ScePspFVector3 *out, const ScePspFVector3 *p0, const ScePspFVector3 *v0,
                             const ScePspFVector3 *p1, const ScePspFVector3 *v1, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
    float h10 = t3 - 2.0f * t2 + t;
    float h11 = t3 - t2;
    float h01 = -2.0f * t3 + 3.0f * t2;

    out->x = p0->x * h00 + v0->x * h10 + v1->x * h11 + p1->x * h01;
    out->y = p0->y * h00 + v0->y * h10 + v1->y * h11 + p1->y * h01;
    out->z = p0->z * h00 + v0->z * h10 + v1->z * h11 + p1->z * h01;
    return out;
}
#else
ScePspFVector3 *vec3_hermite(ScePspFVector3 *out, const ScePspFVector3 *p0, const ScePspFVector3 *v0,
                             const ScePspFVector3 *p1, const ScePspFVector3 *v1, float t)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "lv.s S010, 0(%3)\n"
        "lv.s S011, 4(%3)\n"
        "lv.s S012, 8(%3)\n"
        "lv.s S020, 0(%5)\n"
        "lv.s S021, 4(%5)\n"
        "lv.s S022, 8(%5)\n"
        "lv.s S030, 0(%4)\n"
        "lv.s S031, 4(%4)\n"
        "lv.s S032, 8(%4)\n"
        "lv.s S202, %6\n"
        "vone.s S203\n"
        "vmul.s S201, S202, S202\n"
        "vpfxs 2, 1, 1, -2\n"
        "vmov.q C100, C100\n"
        "vpfxs -3, -2, -1, 3\n"
        "vmov.q C110, C110\n"
        "vmul.s S200, S201, S202\n"
        "vpfxs 0, 1, 0, 0\n"
        "vmov.q C120, C120\n"
        "vpfxs 1, 0, 0, 0\n"
        "vmov.q C130, C130\n"
        "vtfm4.q C210, E100, C200\n"
        "vtfm4.q C220, E000, C210\n"
        "sv.s S220, 0(%1)\n"
        "sv.s S221, 4(%1)\n"
        "sv.s S222, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (p0), "r" (v0), "r" (p1), "r" (v1), "m" (t)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Scale */
#ifdef __MWERKS__
ScePspFVector3 *vec3_scale(ScePspFVector3 *out, const ScePspFVector3 *v, float s);
#elif !defined(__mips__)
ScePspFVector3 *vec3_scale(ScePspFVector3 *out, const ScePspFVector3 *v, float s)
{
    out->x = v->x * s;
    out->y = v->y * s;
    out->z = v->z * s;
    return out;
}
#else
ScePspFVector3 *vec3_scale(ScePspFVector3 *out, const ScePspFVector3 *v, float s)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %3\n"
        "mtv $8, S010\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "vscl.t C000, C000, S010\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "f" (s)
        : "memory", "$8");
    return out;
}
#endif

/* sceVfpuVector3Clamp: min(max(v, lo), hi) */
#ifdef __MWERKS__
ScePspFVector3 *vec3_clamp(ScePspFVector3 *out, const ScePspFVector3 *v, float lo, float hi);
#elif !defined(__mips__)
ScePspFVector3 *vec3_clamp(ScePspFVector3 *out, const ScePspFVector3 *v, float lo, float hi)
{
    float x = v->x > lo ? v->x : lo;
    float y = v->y > lo ? v->y : lo;
    float z = v->z > lo ? v->z : lo;

    out->x = x < hi ? x : hi;
    out->y = y < hi ? y : hi;
    out->z = z < hi ? z : hi;
    return out;
}
#else
ScePspFVector3 *vec3_clamp(ScePspFVector3 *out, const ScePspFVector3 *v, float lo, float hi)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %3\n"
        "mfc1 $9, %4\n"
        "mtv $8, S010\n"
        "mtv $9, S011\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "vpfxt x, x, x, w\n"
        "vmax.t C000, C000, C010\n"
        "vpfxt y, y, y, w\n"
        "vmin.t C000, C000, C010\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "f" (lo), "f" (hi)
        : "memory", "$8", "$9");
    return out;
}
#endif

/* sceVfpuVector3Max */
#ifdef __MWERKS__
ScePspFVector3 *vec3_max(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
#elif !defined(__mips__)
ScePspFVector3 *vec3_max(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x > b->x ? a->x : b->x;
    out->y = a->y > b->y ? a->y : b->y;
    out->z = a->z > b->z ? a->z : b->z;
    return out;
}
#else
ScePspFVector3 *vec3_max(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "lv.s S010, 0(%3)\n"
        "lv.s S011, 4(%3)\n"
        "lv.s S012, 8(%3)\n"
        "vmax.t C000, C000, C010\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Min */
#ifdef __MWERKS__
ScePspFVector3 *vec3_min(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
#elif !defined(__mips__)
ScePspFVector3 *vec3_min(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x < b->x ? a->x : b->x;
    out->y = a->y < b->y ? a->y : b->y;
    out->z = a->z < b->z ? a->z : b->z;
    return out;
}
#else
ScePspFVector3 *vec3_min(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "lv.s S010, 0(%3)\n"
        "lv.s S011, 4(%3)\n"
        "lv.s S012, 8(%3)\n"
        "vmin.t C000, C000, C010\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3InnerProduct */
#ifdef __MWERKS__
float vec3_dot(const ScePspFVector3 *a, const ScePspFVector3 *b);
#elif !defined(__mips__)
float vec3_dot(const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}
#else
float vec3_dot(const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%1)\n"
        "lv.s S001, 4(%1)\n"
        "lv.s S002, 8(%1)\n"
        "lv.s S010, 0(%2)\n"
        "lv.s S011, 4(%2)\n"
        "lv.s S012, 8(%2)\n"
        "vdot.t S000, C000, C010\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (a), "r" (b)
        : "memory");
    return r;
}
#endif

/* sceVfpuVector3OuterProduct: a x b */
#ifdef __MWERKS__
ScePspFVector3 *vec3_cross_nw(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
#elif !defined(__mips__)
ScePspFVector3 *vec3_cross_nw(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    float x = a->y * b->z - a->z * b->y;
    float y = a->z * b->x - a->x * b->z;
    float z = a->x * b->y - a->y * b->x;

    out->x = x;
    out->y = y;
    out->z = z;
    return out;
}
#else
ScePspFVector3 *vec3_cross_nw(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S010, 0(%2)\n"
        "lv.s S011, 4(%2)\n"
        "lv.s S012, 8(%2)\n"
        "lv.s S020, 0(%3)\n"
        "lv.s S021, 4(%3)\n"
        "lv.s S022, 8(%3)\n"
        "vcrsp.t C000, C010, C020\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Funnel: sum of the components */
#ifdef __MWERKS__
float vec3_funnel(const ScePspFVector3 *v);
#elif !defined(__mips__)
float vec3_funnel(const ScePspFVector3 *v)
{
    return v->x + v->y + v->z;
}
#else
float vec3_funnel(const ScePspFVector3 *v)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%1)\n"
        "lv.s S001, 4(%1)\n"
        "lv.s S002, 8(%1)\n"
        "vfad.t S000, C000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (v)
        : "memory");
    return r;
}
#endif

/* sceVfpuVector3Average */
#ifdef __MWERKS__
float vec3_average(const ScePspFVector3 *v);
#elif !defined(__mips__)
float vec3_average(const ScePspFVector3 *v)
{
    return (v->x + v->y + v->z) / 3.0f;
}
#else
float vec3_average(const ScePspFVector3 *v)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%1)\n"
        "lv.s S001, 4(%1)\n"
        "lv.s S002, 8(%1)\n"
        "vavg.t S000, C000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (v)
        : "memory");
    return r;
}
#endif

/* sceVfpuVector3IsEqual */
#ifdef __MWERKS__
asm int vec3_is_equal(const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    lv.s S000, 0(a0)
    lv.s S001, 4(a0)
    lv.s S002, 8(a0)
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    lv.s S012, 8(a1)
    move v0, zero
    vcmp.t eq, C000, C010
    bvtl 5, @28
    addiu v0, zero, 1
@28:
    jr ra
    nop
}
#else
int vec3_is_equal(const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    return a->x == b->x && a->y == b->y && a->z == b->z;
}
#endif

/* sceVfpuVector3IsZero: every component +0 or -0 */
#ifdef __MWERKS__
int vec3_is_zero(const ScePspFVector3 *v);
#elif !defined(__mips__)
int vec3_is_zero(const ScePspFVector3 *v)
{
    return v->x == 0.0f && v->y == 0.0f && v->z == 0.0f;
}
#else
int vec3_is_zero(const ScePspFVector3 *v)
{
    const unsigned int *p = (const unsigned int *)v;

    return ((p[0] | p[1] | p[2]) & 0x7FFFFFFF) == 0;
}
#endif

/* sceVfpuVector3SignFloat */
#ifdef __MWERKS__
ScePspFVector3 *vec3_sign_float(ScePspFVector3 *out, const ScePspFVector3 *v);
#elif !defined(__mips__)
ScePspFVector3 *vec3_sign_float(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    out->x = sgn(v->x);
    out->y = sgn(v->y);
    out->z = sgn(v->z);
    return out;
}
#else
ScePspFVector3 *vec3_sign_float(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "vsgn.t C000, C000\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3SignInt */
#ifdef __MWERKS__
ScePspIVector3 *vec3_sign_int(ScePspIVector3 *out, const ScePspFVector3 *v);
#elif !defined(__mips__)
ScePspIVector3 *vec3_sign_int(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    out->x = (int)sgn(v->x);
    out->y = (int)sgn(v->y);
    out->z = (int)sgn(v->z);
    return out;
}
#else
ScePspIVector3 *vec3_sign_int(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "vsgn.t C000, C000\n"
        "vf2iz.t C000, C000, 0\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Normalize: 0 for a zero vector, components saturated to [-1, 1] */
#ifdef __MWERKS__
ScePspFVector3 *vec3_normalize(ScePspFVector3 *out, const ScePspFVector3 *v);
#elif !defined(__mips__)
ScePspFVector3 *vec3_normalize(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    float s = rsq0(v->x * v->x + v->y * v->y + v->z * v->z);

    out->x = sat1(v->x * s);
    out->y = sat1(v->y * s);
    out->z = sat1(v->z * s);
    return out;
}
#else
ScePspFVector3 *vec3_normalize(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "vdot.t S010, C000, C000\n"
        "vzero.s S011\n"
        "vcmp.s ez, S010, S010\n"
        "vrsq.s S010, S010\n"
        "vcmovt.s S010, S011, 0\n"
        "vpfxd [-1:1], [-1:1], [-1:1], m\n"
        "vscl.t C000, C000, S010\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector3Length */
#ifdef __MWERKS__
float vec3_length(const ScePspFVector3 *v);
#elif !defined(__mips__)
float vec3_length(const ScePspFVector3 *v)
{
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}
#else
float vec3_length(const ScePspFVector3 *v)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%1)\n"
        "lv.s S001, 4(%1)\n"
        "lv.s S002, 8(%1)\n"
        "vdot.t S000, C000, C000\n"
        "vsqrt.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (v)
        : "memory");
    return r;
}
#endif

/* sceVfpuVector3Distance */
#ifdef __MWERKS__
float vec3_distance(const ScePspFVector3 *a, const ScePspFVector3 *b);
#elif !defined(__mips__)
float vec3_distance(const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    float x = a->x - b->x;
    float y = a->y - b->y;
    float z = a->z - b->z;

    return sqrtf(x * x + y * y + z * z);
}
#else
float vec3_distance(const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%1)\n"
        "lv.s S001, 4(%1)\n"
        "lv.s S002, 8(%1)\n"
        "lv.s S010, 0(%2)\n"
        "lv.s S011, 4(%2)\n"
        "lv.s S012, 8(%2)\n"
        "vsub.t C000, C000, C010\n"
        "vdot.t S000, C000, C000\n"
        "vsqrt.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (a), "r" (b)
        : "memory");
    return r;
}
#endif

/* n if dot(i, nref) < 0, else -n */
#ifdef __MWERKS__
ScePspFVector3 *vec3_faceforward(ScePspFVector3 *out, const ScePspFVector3 *n, const ScePspFVector3 *i,
                                 const ScePspFVector3 *nref);
#elif !defined(__mips__)
ScePspFVector3 *vec3_faceforward(ScePspFVector3 *out, const ScePspFVector3 *n, const ScePspFVector3 *i,
                                 const ScePspFVector3 *nref)
{
    float s = i->x * nref->x + i->y * nref->y + i->z * nref->z < 0.0f ? 1.0f : -1.0f;

    out->x = n->x * s;
    out->y = n->y * s;
    out->z = n->z * s;
    return out;
}
#else
ScePspFVector3 *vec3_faceforward(ScePspFVector3 *out, const ScePspFVector3 *n, const ScePspFVector3 *i,
                                 const ScePspFVector3 *nref)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S010, 0(%3)\n"
        "lv.s S011, 4(%3)\n"
        "lv.s S012, 8(%3)\n"
        "lv.s S020, 0(%4)\n"
        "lv.s S021, 4(%4)\n"
        "lv.s S022, 8(%4)\n"
        "vdot.t S030, C010, C020\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S002, 8(%2)\n"
        "vpfxt 0, y, z, w\n"
        "vcmp.s lt, S030, S030\n"
        "vpfxs -x, -y, -z, w\n"
        "vcmovf.t C000, C000, 0\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (n), "r" (i), "r" (nref)
        : "memory");
    return out;
}
#endif

/* normalize(v - 2 dot(v, n) n) */
#ifdef __MWERKS__
ScePspFVector3 *vec3_reflect(ScePspFVector3 *out, const ScePspFVector3 *v, const ScePspFVector3 *n);
#elif !defined(__mips__)
ScePspFVector3 *vec3_reflect(ScePspFVector3 *out, const ScePspFVector3 *v, const ScePspFVector3 *n)
{
    float d = -2.0f * (v->x * n->x + v->y * n->y + v->z * n->z);
    float x = v->x + n->x * d;
    float y = v->y + n->y * d;
    float z = v->z + n->z * d;
    float s = rsq0(x * x + y * y + z * z);

    out->x = sat1(x * s);
    out->y = sat1(y * s);
    out->z = sat1(z * s);
    return out;
}
#else
ScePspFVector3 *vec3_reflect(ScePspFVector3 *out, const ScePspFVector3 *v, const ScePspFVector3 *n)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S010, 0(%2)\n"
        "lv.s S011, 4(%2)\n"
        "lv.s S012, 8(%2)\n"
        "lv.s S020, 0(%3)\n"
        "lv.s S021, 4(%3)\n"
        "lv.s S022, 8(%3)\n"
        "vdot.t S031, C010, C020\n"
        "vfim.s S030, -2.0\n"
        "vmul.s S032, S030, S031\n"
        "vscl.t C020, C020, S032\n"
        "vadd.t C000, C010, C020\n"
        "vdot.t S033, C000, C000\n"
        "vcmp.s ez, S033, S033\n"
        "vrsq.s S033, S033\n"
        "vpfxs 0, y, z, w\n"
        "vcmovt.s S033, S033, 0\n"
        "vpfxd [-1:1], [-1:1], [-1:1], m\n"
        "vscl.t C000, C000, S033\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "r" (n)
        : "memory");
    return out;
}
#endif

/* normalize(eta v + (eta dot(v, n) - eta^2) n), zero when that is not finite (see vec2_refract). */
#ifdef __MWERKS__
ScePspFVector3 *vec3_refract(ScePspFVector3 *out, const ScePspFVector3 *v, const ScePspFVector3 *n, float eta);
#elif !defined(__mips__)
ScePspFVector3 *vec3_refract(ScePspFVector3 *out, const ScePspFVector3 *v, const ScePspFVector3 *n, float eta)
{
    float d = v->x * n->x + v->y * n->y + v->z * n->z;
    float k = eta * d - eta * eta;
    float x = v->x * eta + n->x * k;
    float y = v->y * eta + n->y * k;
    float z = v->z * eta + n->z * k;
    float s = 1.0f / sqrtf(x * x + y * y + z * z);

    if (!isfinite(x) || !isfinite(y) || !isfinite(z)) {
        out->x = 0.0f;
        out->y = 0.0f;
        out->z = 0.0f;
        return out;
    }
    out->x = sat1(x * s);
    out->y = sat1(y * s);
    out->z = sat1(z * s);
    return out;
}
#else
ScePspFVector3 *vec3_refract(ScePspFVector3 *out, const ScePspFVector3 *v, const ScePspFVector3 *n, float eta)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S010, 0(%2)\n"
        "lv.s S011, 4(%2)\n"
        "lv.s S012, 8(%2)\n"
        "lv.s S020, 0(%3)\n"
        "lv.s S021, 4(%3)\n"
        "lv.s S022, 8(%3)\n"
        "lv.s S030, %4\n"
        "vdot.t S031, C010, C020\n"
        "vscl.t C010, C010, S030\n"
        "vmul.s S032, S030, S030\n"
        "vmul.s S033, S031, S031\n"
        "vmul.s S031, S031, S030\n"
        "vocp.s S033, S033\n"
        "vmul.s S033, S032, S033\n"
        "vocp.s S033, S033\n"
        "vsqrt.s S033, S033\n"
        "vsub.s S031, S031, S032\n"
        "vscl.t C020, C020, S031\n"
        "vadd.t C000, C010, C020\n"
        "vdot.t S033, C000, C000\n"
        "vcmp.t es, C000, C000\n"
        "vrsq.s S033, S033\n"
        "vpfxd [-1:1], [-1:1], [-1:1], m\n"
        "vscl.t C000, C000, S033\n"
        "vpfxs 0, 0, 0, w\n"
        "vcmovt.t C000, C000, 4\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        "sv.s S002, 8(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "r" (n), "m" (eta)
        : "memory");
    return out;
}
#endif
