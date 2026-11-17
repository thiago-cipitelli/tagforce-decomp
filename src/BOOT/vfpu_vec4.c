// @range 0x0881B244-0x0881B974
// @gcc 0x0881B244-0x0881B974
/* sceVfpuVector4* of Sony's libvfpu: 16-byte aligned vectors loaded whole (lv.q/sv.q). The *XYZ
   variants (vec3_*_w, vec4_*_xyz) work on x, y, z and keep the w of their first vector argument.
   GCC/VFPU code kept as `asm` functions for the PSP build; under `#else` is the portable C. */
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

static float max1(float a, float b)
{
    return a > b ? a : b;
}

static float min1(float a, float b)
{
    return a < b ? a : b;
}
#endif

/* sceVfpuVector4Set */
#ifndef __MWERKS__ /* PSP: vfpu_vec4_gcc.c */
ScePspFVector4 *vec4_set(ScePspFVector4 *out, float x, float y, float z, float w)
{
    out->x = x;
    out->y = y;
    out->z = z;
    out->w = w;
    return out;
}
#else
ScePspFVector4 *vec4_set(ScePspFVector4 *out, float x, float y, float z, float w);
#endif

/* sceVfpuVector4SetXYZ: w untouched */
#ifndef __MWERKS__ /* PSP: vfpu_vec4_gcc.c */
ScePspFVector4 *vec4_set_xyz(ScePspFVector4 *out, float x, float y, float z)
{
    out->x = x;
    out->y = y;
    out->z = z;
    return out;
}
#else
ScePspFVector4 *vec4_set_xyz(ScePspFVector4 *out, float x, float y, float z);
#endif

/* sceVfpuVector4Copy */
#ifdef __MWERKS__
ScePspFVector4 *vec4_copy(ScePspFVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspFVector4 *vec4_copy(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    *out = *v;
    return out;
}
#else
ScePspFVector4 *vec4_copy(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    out->x = v->x;
    out->y = v->y;
    out->z = v->z;
    out->w = v->w;
    return out;
}
#endif

/* sceVfpuVector4PositiveZero */
#ifndef __MWERKS__ /* PSP: vfpu_vec4_gcc.c */
ScePspFVector4 *vec4_zero(ScePspFVector4 *out)
{
    out->x = 0.0f;
    out->y = 0.0f;
    out->z = 0.0f;
    out->w = 0.0f;
    return out;
}
#else
ScePspFVector4 *vec4_zero(ScePspFVector4 *out);
#endif

/* sceVfpuVector4NegativeZero */
#ifdef __MWERKS__
ScePspFVector4 *vec4_neg_zero(ScePspFVector4 *out);
#elif !defined(__mips__)
ScePspFVector4 *vec4_neg_zero(ScePspFVector4 *out)
{
    out->x = -0.0f;
    out->y = -0.0f;
    out->z = -0.0f;
    out->w = -0.0f;
    return out;
}
#else
ScePspFVector4 *vec4_neg_zero(ScePspFVector4 *out)
{
    unsigned int *p = (unsigned int *)out;

    p[0] = 0x80000000;
    p[1] = 0x80000000;
    p[2] = 0x80000000;
    p[3] = 0x80000000;
    return out;
}
#endif

/* sceVfpuVector4Ceil: vf2id, rounds down [?] (see vec2_ceil). */
#ifdef __MWERKS__
ScePspIVector4 *vec4_ceil(ScePspIVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspIVector4 *vec4_ceil(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    out->x = (int)floorf(v->x);
    out->y = (int)floorf(v->y);
    out->z = (int)floorf(v->z);
    out->w = (int)floorf(v->w);
    return out;
}
#else
ScePspIVector4 *vec4_ceil(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vf2id.q C000, C000, 0\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Trunc (vf2iz) */
#ifdef __MWERKS__
ScePspIVector4 *vec4_trunc(ScePspIVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspIVector4 *vec4_trunc(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    out->x = (int)v->x;
    out->y = (int)v->y;
    out->z = (int)v->z;
    out->w = (int)v->w;
    return out;
}
#else
ScePspIVector4 *vec4_trunc(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vf2iz.q C000, C000, 0\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Round (vf2in) */
#ifdef __MWERKS__
ScePspIVector4 *vec4_round(ScePspIVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspIVector4 *vec4_round(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    out->x = (int)rintf(v->x);
    out->y = (int)rintf(v->y);
    out->z = (int)rintf(v->z);
    out->w = (int)rintf(v->w);
    return out;
}
#else
ScePspIVector4 *vec4_round(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vf2in.q C000, C000, 0\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Floor: vf2iu, rounds up [?] (see vec2_ceil). */
#ifdef __MWERKS__
ScePspIVector4 *vec4_floor(ScePspIVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspIVector4 *vec4_floor(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    out->x = (int)ceilf(v->x);
    out->y = (int)ceilf(v->y);
    out->z = (int)ceilf(v->z);
    out->w = (int)ceilf(v->w);
    return out;
}
#else
ScePspIVector4 *vec4_floor(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vf2iu.q C000, C000, 0\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4FromIVector */
#ifdef __MWERKS__
ScePspFVector4 *vec4_from_ivec(ScePspFVector4 *out, const ScePspIVector4 *v);
#elif !defined(__mips__)
ScePspFVector4 *vec4_from_ivec(ScePspFVector4 *out, const ScePspIVector4 *v)
{
    out->x = (float)v->x;
    out->y = (float)v->y;
    out->z = (float)v->z;
    out->w = (float)v->w;
    return out;
}
#else
ScePspFVector4 *vec4_from_ivec(ScePspFVector4 *out, const ScePspIVector4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vi2f.q C000, C000, 0\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Add */
#ifdef __MWERKS__
ScePspFVector4 *vec4_add(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
ScePspFVector4 *vec4_add(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    out->w = a->w + b->w;
    return out;
}
#else
ScePspFVector4 *vec4_add(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vadd.q C000, C010, C020\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4AddXYZ */
#ifdef __MWERKS__
ScePspFVector4 *vec3_add_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
ScePspFVector4 *vec3_add_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    out->w = a->w;
    return out;
}
#else
ScePspFVector4 *vec3_add_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 0(%3)\n"
        "vadd.t C000, C000, C010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Sub */
#ifdef __MWERKS__
ScePspFVector4 *vec4_sub(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
ScePspFVector4 *vec4_sub(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    out->w = a->w - b->w;
    return out;
}
#else
ScePspFVector4 *vec4_sub(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vsub.q C000, C010, C020\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4SubXYZ */
#ifdef __MWERKS__
ScePspFVector4 *vec3_sub_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
ScePspFVector4 *vec3_sub_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    out->w = a->w;
    return out;
}
#else
ScePspFVector4 *vec3_sub_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 0(%3)\n"
        "vsub.t C000, C000, C010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Mul */
#ifdef __MWERKS__
ScePspFVector4 *vec4_mul(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
ScePspFVector4 *vec4_mul(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    out->z = a->z * b->z;
    out->w = a->w * b->w;
    return out;
}
#else
ScePspFVector4 *vec4_mul(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vmul.q C000, C010, C020\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4MulXYZ */
#ifdef __MWERKS__
ScePspFVector4 *vec3_mul_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
ScePspFVector4 *vec3_mul_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    out->z = a->z * b->z;
    out->w = a->w;
    return out;
}
#else
ScePspFVector4 *vec3_mul_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 0(%3)\n"
        "vmul.t C000, C000, C010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Div */
#ifdef __MWERKS__
ScePspFVector4 *vec4_div(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
ScePspFVector4 *vec4_div(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x / b->x;
    out->y = a->y / b->y;
    out->z = a->z / b->z;
    out->w = a->w / b->w;
    return out;
}
#else
ScePspFVector4 *vec4_div(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vdiv.q C000, C010, C020\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4DivXYZ */
#ifdef __MWERKS__
ScePspFVector4 *vec3_div_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
ScePspFVector4 *vec3_div_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x / b->x;
    out->y = a->y / b->y;
    out->z = a->z / b->z;
    out->w = a->w;
    return out;
}
#else
ScePspFVector4 *vec3_div_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vdiv.t C000, C010, C020\n"
        "vmov.s S003, S013\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Neg (flips the sign bits) */
#ifdef __MWERKS__
ScePspFVector4 *vec4_neg(ScePspFVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspFVector4 *vec4_neg(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    out->x = -v->x;
    out->y = -v->y;
    out->z = -v->z;
    out->w = -v->w;
    return out;
}
#else
ScePspFVector4 *vec4_neg(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    unsigned int *p = (unsigned int *)out;
    const unsigned int *q = (const unsigned int *)v;

    unsigned int x = q[0], y = q[1], z = q[2], w = q[3];

    p[0] = x ^ 0x80000000;
    p[1] = y ^ 0x80000000;
    p[2] = z ^ 0x80000000;
    p[3] = w ^ 0x80000000;
    return out;
}
#endif

/* sceVfpuVector4Abs (clears the sign bits) */
#ifdef __MWERKS__
ScePspFVector4 *vec4_abs(ScePspFVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspFVector4 *vec4_abs(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    out->x = fabsf(v->x);
    out->y = fabsf(v->y);
    out->z = fabsf(v->z);
    out->w = fabsf(v->w);
    return out;
}
#else
ScePspFVector4 *vec4_abs(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    unsigned int *p = (unsigned int *)out;
    const unsigned int *q = (const unsigned int *)v;

    unsigned int x = q[0], y = q[1], z = q[2], w = q[3];

    p[0] = x & 0x7FFFFFFF;
    p[1] = y & 0x7FFFFFFF;
    p[2] = z & 0x7FFFFFFF;
    p[3] = w & 0x7FFFFFFF;
    return out;
}
#endif

/* sceVfpuVector4Lerp: a + t (b - a) */
#ifdef __MWERKS__
ScePspFVector4 *vec4_lerp(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t);
#elif !defined(__mips__)
ScePspFVector4 *vec4_lerp(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t)
{
    out->x = a->x + t * (b->x - a->x);
    out->y = a->y + t * (b->y - a->y);
    out->z = a->z + t * (b->z - a->z);
    out->w = a->w + t * (b->w - a->w);
    return out;
}
#else
ScePspFVector4 *vec4_lerp(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %4\n"
        "mtv $8, S030\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vsub.q C000, C020, C010\n"
        "vscl.q C000, C000, S030\n"
        "vadd.q C010, C010, C000\n"
        "sv.q C010, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b), "f" (t)
        : "memory", "$8");
    return out;
}
#endif

/* sceVfpuVector4LerpXYZ */
#ifdef __MWERKS__
ScePspFVector4 *vec3_lerp_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t);
#elif !defined(__mips__)
ScePspFVector4 *vec3_lerp_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t)
{
    out->x = a->x + t * (b->x - a->x);
    out->y = a->y + t * (b->y - a->y);
    out->z = a->z + t * (b->z - a->z);
    out->w = a->w;
    return out;
}
#else
ScePspFVector4 *vec3_lerp_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %4\n"
        "mtv $8, S030\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vsub.t C000, C020, C010\n"
        "vscl.t C000, C000, S030\n"
        "vadd.t C010, C010, C000\n"
        "sv.q C010, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b), "f" (t)
        : "memory", "$8");
    return out;
}
#endif

/* Cubic Hermite, as vec2_hermite. */
#ifdef __MWERKS__
ScePspFVector4 *vec4_hermite(ScePspFVector4 *out, const ScePspFVector4 *p0, const ScePspFVector4 *v0,
                             const ScePspFVector4 *p1, const ScePspFVector4 *v1, float t);
#elif !defined(__mips__)
ScePspFVector4 *vec4_hermite(ScePspFVector4 *out, const ScePspFVector4 *p0, const ScePspFVector4 *v0,
                             const ScePspFVector4 *p1, const ScePspFVector4 *v1, float t)
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
    out->w = p0->w * h00 + v0->w * h10 + v1->w * h11 + p1->w * h01;
    return out;
}
#else
ScePspFVector4 *vec4_hermite(ScePspFVector4 *out, const ScePspFVector4 *p0, const ScePspFVector4 *v0,
                             const ScePspFVector4 *p1, const ScePspFVector4 *v1, float t)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 0(%3)\n"
        "lv.q C020, 0(%5)\n"
        "lv.q C030, 0(%4)\n"
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
        "sv.q C220, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (p0), "r" (v0), "r" (p1), "r" (v1), "m" (t)
        : "memory");
    return out;
}
#endif

/* Cubic Hermite on x, y, z; w = p0->w. */
#ifdef __MWERKS__
ScePspFVector4 *vec4_hermite_xyz(ScePspFVector4 *out, const ScePspFVector4 *p0, const ScePspFVector4 *v0,
                                 const ScePspFVector4 *p1, const ScePspFVector4 *v1, float t);
#elif !defined(__mips__)
ScePspFVector4 *vec4_hermite_xyz(ScePspFVector4 *out, const ScePspFVector4 *p0, const ScePspFVector4 *v0,
                                 const ScePspFVector4 *p1, const ScePspFVector4 *v1, float t)
{
    float w = p0->w;

    vec4_hermite(out, p0, v0, p1, v1, t);
    out->w = w;
    return out;
}
#else
ScePspFVector4 *vec4_hermite_xyz(ScePspFVector4 *out, const ScePspFVector4 *p0, const ScePspFVector4 *v0,
                                 const ScePspFVector4 *p1, const ScePspFVector4 *v1, float t)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 0(%3)\n"
        "lv.q C020, 0(%5)\n"
        "lv.q C030, 0(%4)\n"
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
        "vmov.s S223, S003\n"
        "sv.q C220, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (p0), "r" (v0), "r" (p1), "r" (v1), "m" (t)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Scale */
#ifdef __MWERKS__
ScePspFVector4 *vec4_scale(ScePspFVector4 *out, const ScePspFVector4 *v, float s);
#elif !defined(__mips__)
ScePspFVector4 *vec4_scale(ScePspFVector4 *out, const ScePspFVector4 *v, float s)
{
    out->x = v->x * s;
    out->y = v->y * s;
    out->z = v->z * s;
    out->w = v->w * s;
    return out;
}
#else
ScePspFVector4 *vec4_scale(ScePspFVector4 *out, const ScePspFVector4 *v, float s)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %3\n"
        "mtv $8, S010\n"
        "lv.q C000, 0(%2)\n"
        "vscl.q C000, C000, S010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "f" (s)
        : "memory", "$8");
    return out;
}
#endif

/* sceVfpuVector4ScaleXYZ */
#ifdef __MWERKS__
ScePspFVector4 *vec3_scale_w(ScePspFVector4 *out, const ScePspFVector4 *v, float s);
#elif !defined(__mips__)
ScePspFVector4 *vec3_scale_w(ScePspFVector4 *out, const ScePspFVector4 *v, float s)
{
    out->x = v->x * s;
    out->y = v->y * s;
    out->z = v->z * s;
    out->w = v->w;
    return out;
}
#else
ScePspFVector4 *vec3_scale_w(ScePspFVector4 *out, const ScePspFVector4 *v, float s)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %3\n"
        "mtv $8, S010\n"
        "lv.q C000, 0(%2)\n"
        "vscl.t C000, C000, S010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "f" (s)
        : "memory", "$8");
    return out;
}
#endif

/* sceVfpuVector4Clamp: min(max(v, lo), hi) */
#ifdef __MWERKS__
ScePspFVector4 *vec4_clamp(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi);
#elif !defined(__mips__)
ScePspFVector4 *vec4_clamp(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi)
{
    out->x = min1(max1(v->x, lo), hi);
    out->y = min1(max1(v->y, lo), hi);
    out->z = min1(max1(v->z, lo), hi);
    out->w = min1(max1(v->w, lo), hi);
    return out;
}
#else
ScePspFVector4 *vec4_clamp(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %3\n"
        "mfc1 $9, %4\n"
        "mtv $8, S010\n"
        "mtv $9, S011\n"
        "lv.q C000, 0(%2)\n"
        "vpfxt x, x, x, x\n"
        "vmax.q C000, C000, C010\n"
        "vpfxt y, y, y, y\n"
        "vmin.q C000, C000, C010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "f" (lo), "f" (hi)
        : "memory", "$8", "$9");
    return out;
}
#endif

/* sceVfpuVector4ClampXYZ */
#ifdef __MWERKS__
ScePspFVector4 *vec4_clamp_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi);
#elif !defined(__mips__)
ScePspFVector4 *vec4_clamp_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi)
{
    out->x = min1(max1(v->x, lo), hi);
    out->y = min1(max1(v->y, lo), hi);
    out->z = min1(max1(v->z, lo), hi);
    out->w = v->w;
    return out;
}
#else
ScePspFVector4 *vec4_clamp_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %3\n"
        "mfc1 $9, %4\n"
        "mtv $8, S010\n"
        "mtv $9, S011\n"
        "lv.q C000, 0(%2)\n"
        "vpfxt x, x, x, w\n"
        "vmax.t C000, C000, C010\n"
        "vpfxt y, y, y, w\n"
        "vmin.t C000, C000, C010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "f" (lo), "f" (hi)
        : "memory", "$8", "$9");
    return out;
}
#endif

/* sceVfpuVector4Max */
#ifdef __MWERKS__
ScePspFVector4 *vec4_max(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
ScePspFVector4 *vec4_max(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = max1(a->x, b->x);
    out->y = max1(a->y, b->y);
    out->z = max1(a->z, b->z);
    out->w = max1(a->w, b->w);
    return out;
}
#else
ScePspFVector4 *vec4_max(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 0(%3)\n"
        "vmax.q C000, C000, C010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Min */
#ifdef __MWERKS__
ScePspFVector4 *vec4_min(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
ScePspFVector4 *vec4_min(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = min1(a->x, b->x);
    out->y = min1(a->y, b->y);
    out->z = min1(a->z, b->z);
    out->w = min1(a->w, b->w);
    return out;
}
#else
ScePspFVector4 *vec4_min(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vmin.q C000, C010, C020\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4InnerProduct */
#ifdef __MWERKS__
float vec4_dot(const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
float vec4_dot(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}
#else
float vec4_dot(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%1)\n"
        "lv.q C020, 0(%2)\n"
        "vdot.q S000, C010, C020\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (a), "r" (b)
        : "memory");
    return r;
}
#endif

/* sceVfpuVector4InnerProductXYZ */
#ifdef __MWERKS__
float vec3_dot2(const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
float vec3_dot2(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}
#else
float vec3_dot2(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%1)\n"
        "lv.q C020, 0(%2)\n"
        "vdot.t S000, C010, C020\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (a), "r" (b)
        : "memory");
    return r;
}
#endif

/* sceVfpuVector4OuterProductXYZ: a x b with w = 0 */
#ifdef __MWERKS__
ScePspFVector4 *vec3_cross(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
ScePspFVector4 *vec3_cross(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    float x = a->y * b->z - a->z * b->y;
    float y = a->z * b->x - a->x * b->z;
    float z = a->x * b->y - a->y * b->x;

    out->x = x;
    out->y = y;
    out->z = z;
    out->w = 0.0f;
    return out;
}
#else
ScePspFVector4 *vec3_cross(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vzero.s S003\n"
        "vcrsp.t C000, C010, C020\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Funnel: sum of the components */
#ifdef __MWERKS__
float vec4_funnel(const ScePspFVector4 *v);
#elif !defined(__mips__)
float vec4_funnel(const ScePspFVector4 *v)
{
    return v->x + v->y + v->z + v->w;
}
#else
float vec4_funnel(const ScePspFVector4 *v)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%1)\n"
        "vfad.q S000, C000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (v)
        : "memory");
    return r;
}
#endif

/* sceVfpuVector4Average */
#ifdef __MWERKS__
float vec4_average(const ScePspFVector4 *v);
#elif !defined(__mips__)
float vec4_average(const ScePspFVector4 *v)
{
    return (v->x + v->y + v->z + v->w) * 0.25f;
}
#else
float vec4_average(const ScePspFVector4 *v)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%1)\n"
        "vavg.q S000, C000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (v)
        : "memory");
    return r;
}
#endif

/* sceVfpuVector4IsEqual */
#ifdef __MWERKS__
int vec4_is_equal(const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
int vec4_is_equal(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    return a->x == b->x && a->y == b->y && a->z == b->z && a->w == b->w;
}
#else
int vec4_is_equal(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    int r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%1)\n"
        "lv.q C010, 0(%2)\n"
        "move %0, $0\n"
        "vcmp.q eq, C000, C010\n"
        "bvtl 5, 1f\n"
        "addiu %0, $0, 1\n"
        "1:\n"
        ".set pop\n"
        : "=&r" (r)
        : "r" (a), "r" (b)
        : "memory");
    return r;
}
#endif

/* sceVfpuVector4IsZero: every component +0 or -0 */
#ifdef __MWERKS__
int vec4_is_zero(const ScePspFVector4 *v);
#elif !defined(__mips__)
int vec4_is_zero(const ScePspFVector4 *v)
{
    return v->x == 0.0f && v->y == 0.0f && v->z == 0.0f && v->w == 0.0f;
}
#else
int vec4_is_zero(const ScePspFVector4 *v)
{
    const unsigned int *p = (const unsigned int *)v;

    return ((p[0] | p[1] | p[2] | p[3]) & 0x7FFFFFFF) == 0;
}
#endif

/* sceVfpuVector4SignFloat */
#ifdef __MWERKS__
ScePspFVector4 *vec4_sign_float(ScePspFVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspFVector4 *vec4_sign_float(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    out->x = sgn(v->x);
    out->y = sgn(v->y);
    out->z = sgn(v->z);
    out->w = sgn(v->w);
    return out;
}
#else
ScePspFVector4 *vec4_sign_float(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vsgn.q C000, C000\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4SignInt */
#ifdef __MWERKS__
ScePspIVector4 *vec4_sign_int(ScePspIVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspIVector4 *vec4_sign_int(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    out->x = (int)sgn(v->x);
    out->y = (int)sgn(v->y);
    out->z = (int)sgn(v->z);
    out->w = (int)sgn(v->w);
    return out;
}
#else
ScePspIVector4 *vec4_sign_int(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vsgn.q C000, C000\n"
        "vf2iz.q C000, C000, 0\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4Normalize: 0 for a zero vector, components saturated to [-1, 1] */
#ifdef __MWERKS__
ScePspFVector4 *vec4_normalize(ScePspFVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspFVector4 *vec4_normalize(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    float s = rsq0(v->x * v->x + v->y * v->y + v->z * v->z + v->w * v->w);

    out->x = sat1(v->x * s);
    out->y = sat1(v->y * s);
    out->z = sat1(v->z * s);
    out->w = sat1(v->w * s);
    return out;
}
#else
ScePspFVector4 *vec4_normalize(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vdot.q S010, C000, C000\n"
        "vzero.s S011\n"
        "vcmp.s ez, S010, S010\n"
        "vrsq.s S010, S010\n"
        "vcmovt.s S010, S011, 0\n"
        "vpfxd [-1:1], [-1:1], [-1:1], [-1:1]\n"
        "vscl.q C000, C000, S010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4NormalizeXYZ */
#ifdef __MWERKS__
ScePspFVector4 *vec3_normalize2(ScePspFVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspFVector4 *vec3_normalize2(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    float s = rsq0(v->x * v->x + v->y * v->y + v->z * v->z);

    out->x = sat1(v->x * s);
    out->y = sat1(v->y * s);
    out->z = sat1(v->z * s);
    out->w = v->w;
    return out;
}
#else
ScePspFVector4 *vec3_normalize2(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vdot.t S010, C000, C000\n"
        "vzero.s S011\n"
        "vcmp.s ez, S010, S010\n"
        "vrsq.s S010, S010\n"
        "vcmovt.s S010, S011, 0\n"
        "vpfxd [-1:1], [-1:1], [-1:1], m\n"
        "vscl.t C000, C000, S010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4NormalizePhase: v - floor((v + pi) * k) * 2pi with k = VFPU_2_PI (2/pi), as in the
   original; a wrap into [-pi, pi) would need 1/(2pi). */
#ifdef __MWERKS__
ScePspFVector4 *vec4_normalize_phase(ScePspFVector4 *out, const ScePspFVector4 *v);
#elif !defined(__mips__)
ScePspFVector4 *vec4_normalize_phase(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    const float pi = 3.14159265f;
    const float k = 2.0f / pi;

    out->x = v->x - floorf((v->x + pi) * k) * (2.0f * pi);
    out->y = v->y - floorf((v->y + pi) * k) * (2.0f * pi);
    out->z = v->z - floorf((v->z + pi) * k) * (2.0f * pi);
    out->w = v->w - floorf((v->w + pi) * k) * (2.0f * pi);
    return out;
}
#else
ScePspFVector4 *vec4_normalize_phase(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vcst.q C010, VFPU_PI\n"
        "vadd.q C020, C000, C010\n"
        "vcst.s S010, VFPU_2_PI\n"
        "vscl.q C020, C020, S010\n"
        "vf2id.q C020, C020, 0\n"
        "vi2f.q C020, C020, 0\n"
        "vcst.s S011, VFPU_2PI\n"
        "vscl.q C020, C020, S011\n"
        "vsub.q C000, C000, C020\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector4LengthXYZ */
#ifdef __MWERKS__
float vec4_length_xyz(const ScePspFVector4 *v);
#elif !defined(__mips__)
float vec4_length_xyz(const ScePspFVector4 *v)
{
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}
#else
float vec4_length_xyz(const ScePspFVector4 *v)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%1)\n"
        "vdot.t S010, C000, C000\n"
        "vsqrt.s S010, S010\n"
        "sv.s S010, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (v)
        : "memory");
    return r;
}
#endif

/* sceVfpuVector4DistanceXYZ */
#ifdef __MWERKS__
float vec4_distance_xyz(const ScePspFVector4 *a, const ScePspFVector4 *b);
#elif !defined(__mips__)
float vec4_distance_xyz(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    float x = a->x - b->x;
    float y = a->y - b->y;
    float z = a->z - b->z;

    return sqrtf(x * x + y * y + z * z);
}
#else
float vec4_distance_xyz(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%1)\n"
        "lv.q C010, 0(%2)\n"
        "vsub.t C000, C000, C010\n"
        "vdot.t S010, C000, C000\n"
        "vsqrt.s S010, S010\n"
        "sv.s S010, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (a), "r" (b)
        : "memory");
    return r;
}
#endif

/* n (x, y, z) if dot(i, nref) < 0, else -n; w = n->w */
#ifdef __MWERKS__
ScePspFVector4 *vec4_faceforward_xyz(ScePspFVector4 *out, const ScePspFVector4 *n, const ScePspFVector4 *i,
                                     const ScePspFVector4 *nref);
#elif !defined(__mips__)
ScePspFVector4 *vec4_faceforward_xyz(ScePspFVector4 *out, const ScePspFVector4 *n, const ScePspFVector4 *i,
                                     const ScePspFVector4 *nref)
{
    float s = i->x * nref->x + i->y * nref->y + i->z * nref->z < 0.0f ? 1.0f : -1.0f;

    out->x = n->x * s;
    out->y = n->y * s;
    out->z = n->z * s;
    out->w = n->w;
    return out;
}
#else
ScePspFVector4 *vec4_faceforward_xyz(ScePspFVector4 *out, const ScePspFVector4 *n, const ScePspFVector4 *i,
                                     const ScePspFVector4 *nref)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%3)\n"
        "lv.q C020, 0(%4)\n"
        "vdot.t S030, C010, C020\n"
        "lv.q C000, 0(%2)\n"
        "vpfxt 0, y, z, w\n"
        "vcmp.s lt, S030, S030\n"
        "vpfxs -x, -y, -z, w\n"
        "vcmovf.t C000, C000, 0\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (n), "r" (i), "r" (nref)
        : "memory");
    return out;
}
#endif

/* normalize(v - 2 dot(v, n) n) on x, y, z; w = v->w */
#ifdef __MWERKS__
ScePspFVector4 *vec4_reflect_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, const ScePspFVector4 *n);
#elif !defined(__mips__)
ScePspFVector4 *vec4_reflect_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, const ScePspFVector4 *n)
{
    float d = -2.0f * (v->x * n->x + v->y * n->y + v->z * n->z);
    float x = v->x + n->x * d;
    float y = v->y + n->y * d;
    float z = v->z + n->z * d;
    float s = rsq0(x * x + y * y + z * z);

    out->x = sat1(x * s);
    out->y = sat1(y * s);
    out->z = sat1(z * s);
    out->w = v->w;
    return out;
}
#else
ScePspFVector4 *vec4_reflect_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, const ScePspFVector4 *n)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vdot.t S031, C000, C020\n"
        "vfim.s S030, -2.0\n"
        "vmul.s S032, S030, S031\n"
        "vscl.t C020, C020, S032\n"
        "vadd.t C000, C000, C020\n"
        "vdot.t S033, C000, C000\n"
        "vcmp.s ez, S033, S033\n"
        "vrsq.s S033, S033\n"
        "vpfxs 0, y, z, w\n"
        "vcmovt.s S033, S033, 0\n"
        "vpfxd [-1:1], [-1:1], [-1:1], m\n"
        "vscl.t C000, C000, S033\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "r" (n)
        : "memory");
    return out;
}
#endif

/* vec3_refract on x, y, z; w = v->w */
#ifdef __MWERKS__
ScePspFVector4 *vec4_refract_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, const ScePspFVector4 *n, float eta);
#elif !defined(__mips__)
ScePspFVector4 *vec4_refract_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, const ScePspFVector4 *n, float eta)
{
    float d = v->x * n->x + v->y * n->y + v->z * n->z;
    float k = eta * d - eta * eta;
    float x = v->x * eta + n->x * k;
    float y = v->y * eta + n->y * k;
    float z = v->z * eta + n->z * k;
    float s = 1.0f / sqrtf(x * x + y * y + z * z);

    out->w = v->w;
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
ScePspFVector4 *vec4_refract_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, const ScePspFVector4 *n, float eta)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "lv.s S030, %4\n"
        "vdot.t S031, C000, C020\n"
        "vscl.t C000, C000, S030\n"
        "vmul.s S032, S030, S030\n"
        "vmul.s S033, S031, S031\n"
        "vmul.s S031, S031, S030\n"
        "vocp.s S033, S033\n"
        "vmul.s S033, S032, S033\n"
        "vocp.s S033, S033\n"
        "vsqrt.s S033, S033\n"
        "vsub.s S031, S031, S032\n"
        "vscl.t C020, C020, S031\n"
        "vadd.t C000, C000, C020\n"
        "vdot.t S033, C000, C000\n"
        "vcmp.t es, C000, C000\n"
        "vrsq.s S033, S033\n"
        "vpfxd [-1:1], [-1:1], [-1:1], m\n"
        "vscl.t C000, C000, S033\n"
        "vpfxs 0, 0, 0, w\n"
        "vcmovt.t C000, C000, 4\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "r" (n), "m" (eta)
        : "memory");
    return out;
}
#endif
