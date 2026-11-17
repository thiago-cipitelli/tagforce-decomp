// @range 0x08819F0C-0x0881AB1C
// @asm 0x08819F30-0x0881A440
// @gcc 0x0881A440-0x0881A520
// @gcc 0x0881A590-0x0881A864
// @gcc 0x0881A8AC-0x0881AB1C
/* FPU trig routines (sceFpuSin/Cos/Atan/Acos) and the 2D part of Sony's libvfpu (sceVfpuVector2*).
   All of it was compiled with GCC or written in VFPU assembly, so the PSP build carries the original
   instructions as `asm` functions; under `#elif !defined(__mips__)` is the portable C for the PC port.
   The GCC functions come from C (vfpu_math_gcc.c, `-fno-math-errno`: sqrt.s without the errno call);
   the port C of the VFPU ones is left out of that object, so its literal pool is the original's
   (`@rodata`). wrap_angle, sinf, cosf and atanf do not match yet (split asm, C under NON_MATCHING),
   but their pools already do, so their constants are written in the order of the original pool. */
#include "vfpu.h"

#if !defined(__MWERKS__) && !defined(__mips__)
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
#elif !defined(__MWERKS__)
#include <math.h>
#endif

/* +-1 (by the sign bit) when x is an infinity or a NaN, 0 when it is finite. */
#ifdef __MWERKS__
asm int isinf_sign(float x)
{
    mfc1 v0, $f12
    sll t0, v0, 0x1
    sra v0, v0, 0x1e
    srl t0, t0, 0x18
    ori v0, v0, 0x1
    sltiu t0, t0, 255
    movn v0, zero, t0
    jr ra
    nop
}
#elif !defined(__mips__)
int isinf_sign(float x)
{
    union {
        float f;
        int i;
    } u;

    u.f = x;
    if (((u.i >> 23) & 0xFF) != 0xFF) {
        return 0;
    }
    return (u.i >> 30) | 1;
}
#else
int isinf_sign(float x);
#endif

#ifdef NON_MATCHING /* GCC: literal pool loaded through $v1 */
#if defined(__mips__) && !defined(__MWERKS__)
/* the SDK's GCC: round.w.s (nearest) for rintf */
static inline float fpu_rint(float x)
{
    float r;

    __asm__ ("round.w.s %0, %0\n\tcvt.s.w %0, %0" : "=f"(r) : "0"(x));
    return r;
}
#define rintf fpu_rint
/* separate objects in the SDK: not inlined into one another (and wrap_angle not deferred to the end) */
float wrap_angle(float x) __attribute__((noinline));
#endif
/* x - round(x / 2pi) * 2pi, in [-pi, pi] (round.w.s: nearest, ties to even). */
float wrap_angle(float x)
{
    return x - rintf(x / 6.28318548f) * 6.28318548f;
}

/* sceFpuSin: fold into [0, pi/2] and evaluate a degree-7 sine or degree-6 cosine polynomial. */
float sinf(float x)
{
    int s = 1;
    float pi;
    float half_pi;
    float t;

    if (x != x) {
        return x;
    }
    pi = 6.28318548f;
    if (x < 0.0f) {
        x = -x;
        s = -1;
    }
    if (x > pi) {
        x -= (float)(int)(x / pi) * pi;
    }
    pi *= 0.5f;
    if (x > pi) {
        x -= pi;
        s = -s;
    }
    half_pi = pi * 0.5f;
    if (x > half_pi) {
        x = pi - x;
    }
    if (x > 0.917345047f) {
        t = half_pi - x;
        t = t * t;
        return (((t * -0.00137198542f + 0.0416631512f) * t + -0.499999791f) * t + 1.0f) * (float)s;
    }
    t = x * x;
    return (((t * -0.000194188775f + 0.00833132491f) * t + -0.166666389f) * t * x + x) * (float)s;
}

/* sceFpuCos: same folding as sinf, with the sign of the cosine. */
float cosf(float x)
{
    int s = 1;
    float pi;
    float half_pi;
    float t;
    float one;

    if (x != x) {
        return x;
    }
    pi = 6.28318548f;
    if (x < 0.0f) {
        x = -x;
    }
    if (x > pi) {
        x -= (float)(int)(x / pi) * pi;
    }
    pi *= 0.5f;
    if (x > pi) {
        x -= pi;
        s = -1;
    }
    half_pi = pi * 0.5f;
    if (x > half_pi) {
        x = pi - x;
        s = -s;
    }
    if (x > 0.653451264f) {
        x = half_pi - x;
        t = x * x;
        return (((t * -0.000194188775f + 0.00833132491f) * t + -0.166666389f) * t * x + x) * (float)s;
    }
    one = 1.0f;
    t = x * x;
    return (((t * -0.00137198542f + 0.0416631512f) * t + -0.499999791f) * t + one) * (float)s;
}

/* atan on [0, 1]: polynomial below tan(pi/16)-ish, otherwise shifted by pi/8 or pi/4. */
static float atan01(float x)
{
    float t;

    if (x < 0.198899999f) {
        t = x * x;
        return x * (((t * -0.140000001f + 0.200000003f) * t + -0.333333313f) * t + 1.0f);
    }
    if (x < 0.668106019f) {
        x = (x - 0.414213568f) / (x * 0.414213568f + 1.0f);
        t = x * x;
        return x * (((t * -0.140000001f + 0.200000003f) * t + -0.333333313f) * t + 1.0f) + 0.392699093f;
    }
    x = (x - 1.0f) / (x + 1.0f);
    t = x * x;
    return x * (((t * -0.140000001f + 0.200000003f) * t + -0.333333313f) * t + 1.0f) + 0.785398185f;
}

/* sceFpuAtan: |x| >= 1 goes through pi/2 - atan(1/|x|). */
float atanf(float x)
{
    int s = 1;

    if (x != x) {
        return x;
    }
    if (x < 0.0f) {
        x = -x;
        s = -1;
    }
    if (x >= 1.0f) {
        x = 1.57079637f - atan01(1.0f / x);
    } else {
        x = atan01(x);
    }
    return x * (float)s;
}

/* asin(|x|) as atan(sqrt(x^2 / (1 - x^2))). */
float asinf(float x)
{
    float x2 = x * x;

    return atanf(sqrtf(x2 / (1.0f - x2)));
}

/* sceFpuAcos: acos(|x|) as atan(sqrt((1 - x^2) / x^2)); negative inputs give acos(-x). */
float acosf(float x)
{
    float x2 = x * x;

    return atanf(sqrtf((1.0f - x2) / x2));
}

/* sceVfpuVector2SignFloat: -1, 0 or 1 per component (0 for a NaN); one store per branch, as in the original. */
ScePspFVector2 *vec2_sign_float(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    if (v->x < 0.0f)
        out->x = -1.0f;
    else if (v->x > 0.0f)
        out->x = 1.0f;
    else
        out->x = 0.0f;
    if (v->y < 0.0f)
        out->y = -1.0f;
    else if (v->y > 0.0f)
        out->y = 1.0f;
    else
        out->y = 0.0f;
    return out;
}
#endif

/* sceVfpuVector2SignInt */
#ifdef __MWERKS__
asm ScePspIVector2 *vec2_sign_int(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    lwc1 $f0, 0(a1)
    mtc1 zero, $f1
    c.lt.s $f0, $f1
    nop
    bc1f @58
    addiu v0, zero, -1
@18:
    sw v0, 0(a0)
@1c:
    lwc1 $f0, 4(a1)
    mtc1 zero, $f1
    c.lt.s $f0, $f1
    nop
    bc1f @40
    addiu v0, zero, -1
@34:
    sw v0, 4(a0)
@38:
    jr ra
    move v0, a0
@40:
    c.lt.s $f1, $f0
    nop
    bc1fl @38
    sw zero, 4(a0)
    j @34
    addiu v0, zero, 1
@58:
    c.lt.s $f1, $f0
    nop
    bc1fl @1c
    sw zero, 0(a0)
    j @18
    addiu v0, zero, 1
}
#elif !defined(__mips__)
ScePspIVector2 *vec2_sign_int(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    out->x = v->x < 0.0f ? -1 : v->x > 0.0f ? 1 : 0;
    out->y = v->y < 0.0f ? -1 : v->y > 0.0f ? 1 : 0;
    return out;
}
#else
ScePspIVector2 *vec2_sign_int(ScePspIVector2 *out, const ScePspFVector2 *v);
#endif

/* sceVfpuVector2Set */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_set(ScePspFVector2 *out, float x, float y)
{
    out->x = x;
    out->y = y;
    return out;
}
#else
ScePspFVector2 *vec2_set(ScePspFVector2 *out, float x, float y);
#endif

/* sceVfpuVector2Copy */
#ifdef __MWERKS__
ScePspFVector2 *vec2_copy(ScePspFVector2 *out, const ScePspFVector2 *v);
#elif !defined(__mips__)
ScePspFVector2 *vec2_copy(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    *out = *v;
    return out;
}
#else
ScePspFVector2 *vec2_copy(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    out->x = v->x;
    out->y = v->y;
    return out;
}
#endif

/* sceVfpuVector2PositiveZero */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_zero(ScePspFVector2 *out)
{
    out->x = 0.0f;
    out->y = 0.0f;
    return out;
}
#else
ScePspFVector2 *vec2_zero(ScePspFVector2 *out);
#endif

/* sceVfpuVector2NegativeZero */
#ifdef __MWERKS__
ScePspFVector2 *vec2_neg_zero(ScePspFVector2 *out);
#elif !defined(__mips__)
ScePspFVector2 *vec2_neg_zero(ScePspFVector2 *out)
{
    out->x = -0.0f;
    out->y = -0.0f;
    return out;
}
#else
ScePspFVector2 *vec2_neg_zero(ScePspFVector2 *out)
{
    unsigned int *p = (unsigned int *)out;

    p[0] = 0x80000000;
    p[1] = 0x80000000;
    return out;
}
#endif

/* sceVfpuVector2Ceil: the SDK uses vf2id here, which rounds down [?] (and vf2iu in Floor). */
#ifdef __MWERKS__
ScePspIVector2 *vec2_ceil(ScePspIVector2 *out, const ScePspFVector2 *v);
#elif !defined(__mips__)
ScePspIVector2 *vec2_ceil(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    out->x = (int)floorf(v->x);
    out->y = (int)floorf(v->y);
    return out;
}
#else
ScePspIVector2 *vec2_ceil(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "vf2id.p C000, C000, 0\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector2Trunc (vf2iz) */
#ifdef __MWERKS__
ScePspIVector2 *vec2_trunc(ScePspIVector2 *out, const ScePspFVector2 *v);
#elif !defined(__mips__)
ScePspIVector2 *vec2_trunc(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    out->x = (int)v->x;
    out->y = (int)v->y;
    return out;
}
#else
ScePspIVector2 *vec2_trunc(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "vf2iz.p C000, C000, 0\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector2Round (vf2in: nearest, ties to even) */
#ifdef __MWERKS__
ScePspIVector2 *vec2_round(ScePspIVector2 *out, const ScePspFVector2 *v);
#elif !defined(__mips__)
ScePspIVector2 *vec2_round(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    out->x = (int)rintf(v->x);
    out->y = (int)rintf(v->y);
    return out;
}
#else
ScePspIVector2 *vec2_round(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "vf2in.p C000, C000, 0\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector2Floor: vf2iu, rounds up [?] (see vec2_ceil). */
#ifdef __MWERKS__
ScePspIVector2 *vec2_floor(ScePspIVector2 *out, const ScePspFVector2 *v);
#elif !defined(__mips__)
ScePspIVector2 *vec2_floor(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    out->x = (int)ceilf(v->x);
    out->y = (int)ceilf(v->y);
    return out;
}
#else
ScePspIVector2 *vec2_floor(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "vf2iu.p C000, C000, 0\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector2FromIVector */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_from_ivec(ScePspFVector2 *out, const ScePspIVector2 *v)
{
    out->x = (float)v->x;
    out->y = (float)v->y;
    return out;
}
#else
ScePspFVector2 *vec2_from_ivec(ScePspFVector2 *out, const ScePspIVector2 *v);
#endif

/* sceVfpuVector2Add */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_add(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    return out;
}
#else
ScePspFVector2 *vec2_add(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
#endif

/* sceVfpuVector2Sub */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_sub(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    return out;
}
#else
ScePspFVector2 *vec2_sub(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
#endif

/* sceVfpuVector2Mul */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_mul(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    return out;
}
#else
ScePspFVector2 *vec2_mul(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
#endif

/* sceVfpuVector2Div */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_div(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = a->x / b->x;
    out->y = a->y / b->y;
    return out;
}
#else
ScePspFVector2 *vec2_div(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
#endif

/* sceVfpuVector2Neg */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_neg(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    out->x = -v->x;
    out->y = -v->y;
    return out;
}
#else
ScePspFVector2 *vec2_neg(ScePspFVector2 *out, const ScePspFVector2 *v);
#endif

/* sceVfpuVector2Abs */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_abs(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    out->x = fabsf(v->x);
    out->y = fabsf(v->y);
    return out;
}
#else
ScePspFVector2 *vec2_abs(ScePspFVector2 *out, const ScePspFVector2 *v);
#endif

/* sceVfpuVector2Lerp: a + t (b - a) */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_lerp(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b, float t)
{
    out->x = a->x + t * (b->x - a->x);
    out->y = a->y + t * (b->y - a->y);
    return out;
}
#else
ScePspFVector2 *vec2_lerp(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b, float t);
#endif

/* sceVfpuVector2Scale */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_scale(ScePspFVector2 *out, const ScePspFVector2 *v, float s)
{
    out->x = v->x * s;
    out->y = v->y * s;
    return out;
}
#else
ScePspFVector2 *vec2_scale(ScePspFVector2 *out, const ScePspFVector2 *v, float s);
#endif

/* Cubic Hermite: the basis (2,-3,0,1) (1,-2,1,0) (1,-1,0,0) (-2,3,0,0) comes from constant prefixes and
   multiplies (t^3, t^2, t, 1) with vtfm4. */
#ifdef __MWERKS__
ScePspFVector2 *vec2_hermite(ScePspFVector2 *out, const ScePspFVector2 *p0, const ScePspFVector2 *v0,
                             const ScePspFVector2 *p1, const ScePspFVector2 *v1, float t);
#elif !defined(__mips__)
ScePspFVector2 *vec2_hermite(ScePspFVector2 *out, const ScePspFVector2 *p0, const ScePspFVector2 *v0,
                             const ScePspFVector2 *p1, const ScePspFVector2 *v1, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
    float h10 = t3 - 2.0f * t2 + t;
    float h11 = t3 - t2;
    float h01 = -2.0f * t3 + 3.0f * t2;

    out->x = p0->x * h00 + v0->x * h10 + v1->x * h11 + p1->x * h01;
    out->y = p0->y * h00 + v0->y * h10 + v1->y * h11 + p1->y * h01;
    return out;
}
#else
ScePspFVector2 *vec2_hermite(ScePspFVector2 *out, const ScePspFVector2 *p0, const ScePspFVector2 *v0,
                             const ScePspFVector2 *p1, const ScePspFVector2 *v1, float t)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "lv.s S010, 0(%3)\n"
        "lv.s S011, 4(%3)\n"
        "lv.s S020, 0(%5)\n"
        "lv.s S021, 4(%5)\n"
        "lv.s S030, 0(%4)\n"
        "lv.s S031, 4(%4)\n"
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
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (p0), "r" (v0), "r" (p1), "r" (v1), "m" (t)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector2Clamp: min(max(v, lo), hi) */
#ifdef __MWERKS__
ScePspFVector2 *vec2_clamp(ScePspFVector2 *out, const ScePspFVector2 *v, float lo, float hi);
#elif !defined(__mips__)
ScePspFVector2 *vec2_clamp(ScePspFVector2 *out, const ScePspFVector2 *v, float lo, float hi)
{
    float x = v->x > lo ? v->x : lo;
    float y = v->y > lo ? v->y : lo;

    out->x = x < hi ? x : hi;
    out->y = y < hi ? y : hi;
    return out;
}
#else
ScePspFVector2 *vec2_clamp(ScePspFVector2 *out, const ScePspFVector2 *v, float lo, float hi)
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
        "vpfxt x, x, z, w\n"
        "vmax.p C000, C000, C010\n"
        "vpfxt y, y, z, w\n"
        "vmin.p C000, C000, C010\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "f" (lo), "f" (hi)
        : "memory", "$8", "$9");
    return out;
}
#endif

/* sceVfpuVector2Max */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_max(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    lwc1 $f3, 0(a1)
    lwc1 $f1, 0(a2)
    move v0, a0
    c.lt.s $f1, $f3
    nop
    bc1t @20
    mov.s $f2, $f3
    mov.s $f2, $f1
@20:
    lwc1 $f3, 4(a1)
    lwc1 $f1, 4(a2)
    swc1 $f2, 0(v0)
    c.lt.s $f1, $f3
    nop
    bc1t @40
    mov.s $f2, $f3
    mov.s $f2, $f1
@40:
    jr ra
    swc1 $f2, 4(v0)
}
#elif !defined(__mips__)
ScePspFVector2 *vec2_max(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = b->x < a->x ? a->x : b->x;
    out->y = b->y < a->y ? a->y : b->y;
    return out;
}
#else
ScePspFVector2 *vec2_max(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
#endif

/* sceVfpuVector2Min */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
ScePspFVector2 *vec2_min(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = a->x < b->x ? a->x : b->x;
    out->y = a->y < b->y ? a->y : b->y;
    return out;
}
#else
ScePspFVector2 *vec2_min(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
#endif

/* sceVfpuVector2InnerProduct */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
float vec2_dot(const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    return a->x * b->x + a->y * b->y;
}
#else
float vec2_dot(const ScePspFVector2 *a, const ScePspFVector2 *b);
#endif

/* sceVfpuVector2Funnel: sum of the components */
#ifndef __MWERKS__ /* PSP: vfpu_math_gcc.c */
float vec2_funnel(const ScePspFVector2 *v)
{
    return v->x + v->y;
}
#else
float vec2_funnel(const ScePspFVector2 *v);
#endif

#ifdef NON_MATCHING /* GCC: literal pool loaded through $v1 */
/* sceVfpuVector2Average */
float vec2_average(const ScePspFVector2 *v)
{
    return (v->x + v->y) * 0.5f;
}
#endif

/* sceVfpuVector2IsEqual */
#ifdef __MWERKS__
int vec2_is_equal(const ScePspFVector2 *a, const ScePspFVector2 *b);
#elif !defined(__mips__)
int vec2_is_equal(const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    return a->x == b->x && a->y == b->y;
}
#else
int vec2_is_equal(const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    int r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%1)\n"
        "lv.s S001, 4(%1)\n"
        "lv.s S010, 0(%2)\n"
        "lv.s S011, 4(%2)\n"
        "move %0, $0\n"
        "vcmp.p eq, C000, C010\n"
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

/* sceVfpuVector2IsZero: both components +0 or -0 */
#ifdef __MWERKS__
int vec2_is_zero(const ScePspFVector2 *v);
#elif !defined(__mips__)
int vec2_is_zero(const ScePspFVector2 *v)
{
    return v->x == 0.0f && v->y == 0.0f;
}
#else
int vec2_is_zero(const ScePspFVector2 *v)
{
    const unsigned int *p = (const unsigned int *)v;

    return ((p[0] | p[1]) & 0x7FFFFFFF) == 0;
}
#endif

/* sceVfpuVector2Normalize: 0 for a zero vector, components saturated to [-1, 1] */
#ifdef __MWERKS__
ScePspFVector2 *vec2_normalize(ScePspFVector2 *out, const ScePspFVector2 *v);
#elif !defined(__mips__)
ScePspFVector2 *vec2_normalize(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    float s = rsq0(v->x * v->x + v->y * v->y);

    out->x = sat1(v->x * s);
    out->y = sat1(v->y * s);
    return out;
}
#else
ScePspFVector2 *vec2_normalize(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "vdot.p S010, C000, C000\n"
        "vzero.s S011\n"
        "vcmp.s ez, S010, S010\n"
        "vrsq.s S010, S010\n"
        "vcmovt.s S010, S011, 0\n"
        "vpfxd [-1:1], [-1:1], m, m\n"
        "vscl.p C000, C000, S010\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v)
        : "memory");
    return out;
}
#endif

/* sceVfpuVector2Length */
#ifdef __MWERKS__
float vec2_length(const ScePspFVector2 *v);
#elif !defined(__mips__)
float vec2_length(const ScePspFVector2 *v)
{
    return sqrtf(v->x * v->x + v->y * v->y);
}
#else
float vec2_length(const ScePspFVector2 *v)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%1)\n"
        "lv.s S001, 4(%1)\n"
        "vdot.p S000, C000, C000\n"
        "vsqrt.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "r" (v)
        : "memory");
    return r;
}
#endif

/* sceVfpuVector2Distance */
#ifdef __MWERKS__
float vec2_distance(const ScePspFVector2 *a, const ScePspFVector2 *b);
#elif !defined(__mips__)
float vec2_distance(const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    float x = a->x - b->x;
    float y = a->y - b->y;

    return sqrtf(x * x + y * y);
}
#else
float vec2_distance(const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, 0(%1)\n"
        "lv.s S001, 4(%1)\n"
        "lv.s S010, 0(%2)\n"
        "lv.s S011, 4(%2)\n"
        "vsub.p C000, C000, C010\n"
        "vdot.p S000, C000, C000\n"
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
ScePspFVector2 *vec2_faceforward(ScePspFVector2 *out, const ScePspFVector2 *n, const ScePspFVector2 *i,
                                 const ScePspFVector2 *nref);
#elif !defined(__mips__)
ScePspFVector2 *vec2_faceforward(ScePspFVector2 *out, const ScePspFVector2 *n, const ScePspFVector2 *i,
                                 const ScePspFVector2 *nref)
{
    float s = i->x * nref->x + i->y * nref->y < 0.0f ? 1.0f : -1.0f;

    out->x = n->x * s;
    out->y = n->y * s;
    return out;
}
#else
ScePspFVector2 *vec2_faceforward(ScePspFVector2 *out, const ScePspFVector2 *n, const ScePspFVector2 *i,
                                 const ScePspFVector2 *nref)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S010, 0(%3)\n"
        "lv.s S011, 4(%3)\n"
        "lv.s S020, 0(%4)\n"
        "lv.s S021, 4(%4)\n"
        "vdot.p S030, C010, C020\n"
        "lv.s S000, 0(%2)\n"
        "lv.s S001, 4(%2)\n"
        "vpfxt 0, y, z, w\n"
        "vcmp.s lt, S030, S030\n"
        "vpfxs -x, -y, z, w\n"
        "vcmovf.p C000, C000, 0\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (n), "r" (i), "r" (nref)
        : "memory");
    return out;
}
#endif

/* normalize(v - 2 dot(v, n) n) */
#ifdef __MWERKS__
ScePspFVector2 *vec2_reflect(ScePspFVector2 *out, const ScePspFVector2 *v, const ScePspFVector2 *n);
#elif !defined(__mips__)
ScePspFVector2 *vec2_reflect(ScePspFVector2 *out, const ScePspFVector2 *v, const ScePspFVector2 *n)
{
    float d = -2.0f * (v->x * n->x + v->y * n->y);
    float x = v->x + n->x * d;
    float y = v->y + n->y * d;
    float s = rsq0(x * x + y * y);

    out->x = sat1(x * s);
    out->y = sat1(y * s);
    return out;
}
#else
ScePspFVector2 *vec2_reflect(ScePspFVector2 *out, const ScePspFVector2 *v, const ScePspFVector2 *n)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S010, 0(%2)\n"
        "lv.s S011, 4(%2)\n"
        "lv.s S020, 0(%3)\n"
        "lv.s S021, 4(%3)\n"
        "vdot.p S031, C010, C020\n"
        "vfim.s S030, -2.0\n"
        "vmul.s S032, S030, S031\n"
        "vscl.p C020, C020, S032\n"
        "vadd.p C000, C010, C020\n"
        "vdot.p S033, C000, C000\n"
        "vcmp.s ez, S033, S033\n"
        "vrsq.s S033, S033\n"
        "vpfxs 0, y, z, w\n"
        "vcmovt.s S033, S033, 0\n"
        "vpfxd [-1:1], [-1:1], m, m\n"
        "vscl.p C000, C000, S033\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "r" (n)
        : "memory");
    return out;
}
#endif

/* normalize(eta v + (eta dot(v, n) - eta^2) n), zero when that is not finite. The original computes
   sqrt(1 - eta^2 (1 - dot^2)) but never uses it (it subtracts eta^2 where the root should go). */
#ifdef __MWERKS__
ScePspFVector2 *vec2_refract(ScePspFVector2 *out, const ScePspFVector2 *v, const ScePspFVector2 *n, float eta);
#elif !defined(__mips__)
ScePspFVector2 *vec2_refract(ScePspFVector2 *out, const ScePspFVector2 *v, const ScePspFVector2 *n, float eta)
{
    float d = v->x * n->x + v->y * n->y;
    float k = eta * d - eta * eta;
    float x = v->x * eta + n->x * k;
    float y = v->y * eta + n->y * k;
    float s = 1.0f / sqrtf(x * x + y * y);

    if (!isfinite(x) || !isfinite(y)) {
        out->x = 0.0f;
        out->y = 0.0f;
        return out;
    }
    out->x = sat1(x * s);
    out->y = sat1(y * s);
    return out;
}
#else
ScePspFVector2 *vec2_refract(ScePspFVector2 *out, const ScePspFVector2 *v, const ScePspFVector2 *n, float eta)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S010, 0(%2)\n"
        "lv.s S011, 4(%2)\n"
        "lv.s S020, 0(%3)\n"
        "lv.s S021, 4(%3)\n"
        "lv.s S030, %4\n"
        "vdot.p S031, C010, C020\n"
        "vscl.p C010, C010, S030\n"
        "vmul.s S032, S030, S030\n"
        "vmul.s S033, S031, S031\n"
        "vmul.s S031, S031, S030\n"
        "vocp.s S033, S033\n"
        "vmul.s S033, S032, S033\n"
        "vocp.s S033, S033\n"
        "vsqrt.s S033, S033\n"
        "vsub.s S031, S031, S032\n"
        "vscl.p C020, C020, S031\n"
        "vadd.p C000, C010, C020\n"
        "vdot.p S033, C000, C000\n"
        "vcmp.p es, C000, C000\n"
        "vrsq.s S033, S033\n"
        "vpfxd [-1:1], [-1:1], m, m\n"
        "vscl.p C000, C000, S033\n"
        "vpfxs 0, 0, z, w\n"
        "vcmovt.p C000, C000, 4\n"
        "sv.s S000, 0(%1)\n"
        "sv.s S001, 4(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (v), "r" (n), "m" (eta)
        : "memory");
    return out;
}
#endif
