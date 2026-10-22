// @range 0x08819F0C-0x0881AB1C
// @asm 0x08819F30-0x0881A520
// @asm 0x0881A924-0x0881A940
/* FPU trig routines (sceFpuSin/Cos/Atan/Acos) and the 2D part of Sony's libvfpu (sceVfpuVector2*).
   All of it was compiled with GCC or written in VFPU assembly, so the PSP build carries the original
   instructions as `asm` functions; under `#else` is the portable C for the PC port. The functions that
   load float literals from .data (lui/lwc1 through $v1, which the mwcc inline assembler cannot
   express) stay in the split asm, with their C under NON_MATCHING. */
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
#else
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
#endif

#ifdef NON_MATCHING /* GCC: literal pool loaded through $v1 */
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
    if (x < 0.0f) {
        x = -x;
        s = -1;
    }
    if (x > 6.28318548f) {
        x -= (float)(int)(x / 6.28318548f) * 6.28318548f;
    }
    pi = 6.28318548f * 0.5f;
    half_pi = pi * 0.5f;
    if (x > pi) {
        x -= pi;
        s = -s;
    }
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

    if (x != x) {
        return x;
    }
    if (x < 0.0f) {
        x = -x;
    }
    if (x > 6.28318548f) {
        x -= (float)(int)(x / 6.28318548f) * 6.28318548f;
    }
    pi = 6.28318548f * 0.5f;
    half_pi = pi * 0.5f;
    if (x > pi) {
        x -= pi;
        s = -1;
    }
    if (x > half_pi) {
        x = pi - x;
        s = -s;
    }
    if (x > 0.653451264f) {
        x = half_pi - x;
        t = x * x;
        return (((t * -0.000194188775f + 0.00833132491f) * t + -0.166666389f) * t * x + x) * (float)s;
    }
    t = x * x;
    return (((t * -0.00137198542f + 0.0416631512f) * t + -0.499999791f) * t + 1.0f) * (float)s;
}

/* atan on [0, 1]: polynomial below tan(pi/16)-ish, otherwise shifted by pi/8 or pi/4. */
static float atan01(float x)
{
    float t;
    float base = 0.0f;

    if (!(x < 0.198899999f)) {
        if (x < 0.668106019f) {
            x = (x - 0.414213568f) / (x * 0.414213568f + 1.0f);
            base = 0.392699093f;
        } else {
            x = (x - 1.0f) / (x + 1.0f);
            base = 0.785398185f;
        }
    }
    t = x * x;
    return x * (((t * -0.140000001f + 0.200000003f) * t + -0.333333313f) * t + 1.0f) + base;
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
        return (1.57079637f - atan01(1.0f / x)) * (float)s;
    }
    return atan01(x) * (float)s;
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

/* sceVfpuVector2SignFloat: -1, 0 or 1 per component (0 for a NaN). */
ScePspFVector2 *vec2_sign_float(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    out->x = v->x < 0.0f ? -1.0f : v->x > 0.0f ? 1.0f : 0.0f;
    out->y = v->y < 0.0f ? -1.0f : v->y > 0.0f ? 1.0f : 0.0f;
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
#else
ScePspIVector2 *vec2_sign_int(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    out->x = v->x < 0.0f ? -1 : v->x > 0.0f ? 1 : 0;
    out->y = v->y < 0.0f ? -1 : v->y > 0.0f ? 1 : 0;
    return out;
}
#endif

/* sceVfpuVector2Set */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_set(ScePspFVector2 *out, float x, float y)
{
    move v0, a0
    swc1 $f12, 0(a0)
    jr ra
    swc1 $f13, 4(a0)
}
#else
ScePspFVector2 *vec2_set(ScePspFVector2 *out, float x, float y)
{
    out->x = x;
    out->y = y;
    return out;
}
#endif

/* sceVfpuVector2Copy */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_copy(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    lwc1 $f1, 4(a1)
    lwc1 $f2, 0(a1)
    move v0, a0
    swc1 $f1, 4(a0)
    jr ra
    swc1 $f2, 0(a0)
}
#else
ScePspFVector2 *vec2_copy(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    *out = *v;
    return out;
}
#endif

/* sceVfpuVector2PositiveZero */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_zero(ScePspFVector2 *out)
{
    move v0, a0
    sw zero, 0(a0)
    jr ra
    sw zero, 4(a0)
}
#else
ScePspFVector2 *vec2_zero(ScePspFVector2 *out)
{
    out->x = 0.0f;
    out->y = 0.0f;
    return out;
}
#endif

/* sceVfpuVector2NegativeZero */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_neg_zero(ScePspFVector2 *out)
{
    lui a1, 0x8000
    move v0, a0
    sw a1, 4(a0)
    jr ra
    sw a1, 0(a0)
}
#else
ScePspFVector2 *vec2_neg_zero(ScePspFVector2 *out)
{
    out->x = -0.0f;
    out->y = -0.0f;
    return out;
}
#endif

/* sceVfpuVector2Ceil: the SDK uses vf2id here, which rounds down [?] (and vf2iu in Floor). */
#ifdef __MWERKS__
asm ScePspIVector2 *vec2_ceil(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    vf2id.p C000, C000, 0
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    jr ra
    nop
}
#else
ScePspIVector2 *vec2_ceil(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    out->x = (int)floorf(v->x);
    out->y = (int)floorf(v->y);
    return out;
}
#endif

/* sceVfpuVector2Trunc (vf2iz) */
#ifdef __MWERKS__
asm ScePspIVector2 *vec2_trunc(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    vf2iz.p C000, C000, 0
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    jr ra
    nop
}
#else
ScePspIVector2 *vec2_trunc(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    out->x = (int)v->x;
    out->y = (int)v->y;
    return out;
}
#endif

/* sceVfpuVector2Round (vf2in: nearest, ties to even) */
#ifdef __MWERKS__
asm ScePspIVector2 *vec2_round(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    vf2in.p C000, C000, 0
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    jr ra
    nop
}
#else
ScePspIVector2 *vec2_round(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    out->x = (int)rintf(v->x);
    out->y = (int)rintf(v->y);
    return out;
}
#endif

/* sceVfpuVector2Floor: vf2iu, rounds up [?] (see vec2_ceil). */
#ifdef __MWERKS__
asm ScePspIVector2 *vec2_floor(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    vf2iu.p C000, C000, 0
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    jr ra
    nop
}
#else
ScePspIVector2 *vec2_floor(ScePspIVector2 *out, const ScePspFVector2 *v)
{
    out->x = (int)ceilf(v->x);
    out->y = (int)ceilf(v->y);
    return out;
}
#endif

/* sceVfpuVector2FromIVector */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_from_ivec(ScePspFVector2 *out, const ScePspIVector2 *v)
{
    lwc1 $f1, 4(a1)
    lwc1 $f2, 0(a1)
    move v0, a0
    cvt.s.w $f3, $f1
    cvt.s.w $f0, $f2
    swc1 $f3, 4(a0)
    jr ra
    swc1 $f0, 0(a0)
}
#else
ScePspFVector2 *vec2_from_ivec(ScePspFVector2 *out, const ScePspIVector2 *v)
{
    out->x = (float)v->x;
    out->y = (float)v->y;
    return out;
}
#endif

/* sceVfpuVector2Add */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_add(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    lwc1 $f3, 4(a1)
    lwc1 $f1, 4(a2)
    lwc1 $f5, 0(a1)
    lwc1 $f0, 0(a2)
    add.s $f4, $f3, $f1
    move v0, a0
    add.s $f2, $f5, $f0
    swc1 $f4, 4(a0)
    jr ra
    swc1 $f2, 0(a0)
}
#else
ScePspFVector2 *vec2_add(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    return out;
}
#endif

/* sceVfpuVector2Sub */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_sub(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    lwc1 $f3, 4(a1)
    lwc1 $f1, 4(a2)
    lwc1 $f5, 0(a1)
    lwc1 $f0, 0(a2)
    sub.s $f4, $f3, $f1
    move v0, a0
    sub.s $f2, $f5, $f0
    swc1 $f4, 4(a0)
    jr ra
    swc1 $f2, 0(a0)
}
#else
ScePspFVector2 *vec2_sub(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    return out;
}
#endif

/* sceVfpuVector2Mul */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_mul(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    lwc1 $f3, 4(a1)
    lwc1 $f1, 4(a2)
    lwc1 $f5, 0(a1)
    lwc1 $f0, 0(a2)
    mul.s $f4, $f3, $f1
    move v0, a0
    mul.s $f2, $f5, $f0
    swc1 $f4, 4(a0)
    jr ra
    swc1 $f2, 0(a0)
}
#else
ScePspFVector2 *vec2_mul(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    return out;
}
#endif

/* sceVfpuVector2Div */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_div(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    lwc1 $f3, 4(a1)
    lwc1 $f1, 4(a2)
    lwc1 $f5, 0(a1)
    lwc1 $f0, 0(a2)
    div.s $f4, $f3, $f1
    move v0, a0
    div.s $f2, $f5, $f0
    swc1 $f4, 4(a0)
    jr ra
    swc1 $f2, 0(a0)
}
#else
ScePspFVector2 *vec2_div(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = a->x / b->x;
    out->y = a->y / b->y;
    return out;
}
#endif

/* sceVfpuVector2Neg */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_neg(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    lwc1 $f1, 4(a1)
    lwc1 $f4, 0(a1)
    move v0, a0
    neg.s $f3, $f1
    neg.s $f2, $f4
    swc1 $f3, 4(a0)
    jr ra
    swc1 $f2, 0(a0)
}
#else
ScePspFVector2 *vec2_neg(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    out->x = -v->x;
    out->y = -v->y;
    return out;
}
#endif

/* sceVfpuVector2Abs */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_abs(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    lwc1 $f1, 4(a1)
    lwc1 $f4, 0(a1)
    move v0, a0
    abs.s $f3, $f1
    abs.s $f2, $f4
    swc1 $f3, 4(a0)
    jr ra
    swc1 $f2, 0(a0)
}
#else
ScePspFVector2 *vec2_abs(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    out->x = fabsf(v->x);
    out->y = fabsf(v->y);
    return out;
}
#endif

/* sceVfpuVector2Lerp: a + t (b - a) */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_lerp(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b, float t)
{
    lwc1 $f5, 4(a1)
    lwc1 $f4, 0(a1)
    lwc1 $f9, 4(a2)
    lwc1 $f8, 0(a2)
    move v0, a0
    sub.s $f7, $f9, $f5
    sub.s $f6, $f8, $f4
    mul.s $f3, $f12, $f7
    mul.s $f2, $f12, $f6
    add.s $f1, $f5, $f3
    add.s $f0, $f4, $f2
    swc1 $f1, 4(a0)
    jr ra
    swc1 $f0, 0(a0)
}
#else
ScePspFVector2 *vec2_lerp(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b, float t)
{
    out->x = a->x + t * (b->x - a->x);
    out->y = a->y + t * (b->y - a->y);
    return out;
}
#endif

/* sceVfpuVector2Scale */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_scale(ScePspFVector2 *out, const ScePspFVector2 *v, float s)
{
    lwc1 $f1, 4(a1)
    lwc1 $f4, 0(a1)
    move v0, a0
    mul.s $f3, $f1, $f12
    mul.s $f2, $f4, $f12
    swc1 $f3, 4(a0)
    jr ra
    swc1 $f2, 0(a0)
}
#else
ScePspFVector2 *vec2_scale(ScePspFVector2 *out, const ScePspFVector2 *v, float s)
{
    out->x = v->x * s;
    out->y = v->y * s;
    return out;
}
#endif

/* Cubic Hermite: the basis (2,-3,0,1) (1,-2,1,0) (1,-1,0,0) (-2,3,0,0) comes from constant prefixes and
   multiplies (t^3, t^2, t, 1) with vtfm4. */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_hermite(ScePspFVector2 *out, const ScePspFVector2 *p0, const ScePspFVector2 *v0,
                             const ScePspFVector2 *p1, const ScePspFVector2 *v1, float t)
{
    addiu sp, sp, -16
    move v0, a0
    swc1 $f12, 0(sp)
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S010, 0(a2)
    lv.s S011, 4(a2)
    lv.s S020, 0(t0)
    lv.s S021, 4(t0)
    lv.s S030, 0(a3)
    lv.s S031, 4(a3)
    lv.s S202, 0(sp)
    vone.s S203
    vmul.s S201, S202, S202
    vpfxs 2, 1, 1, -2
    vmov.q C100, C100
    vpfxs -3, -2, -1, 3
    vmov.q C110, C110
    vmul.s S200, S201, S202
    vpfxs 0, 1, 0, 0
    vmov.q C120, C120
    vpfxs 1, 0, 0, 0
    vmov.q C130, C130
    vtfm4.q C210, E100, C200
    vtfm4.q C220, E000, C210
    sv.s S220, 0(a0)
    sv.s S221, 4(a0)
    jr ra
    addiu sp, sp, 16
}
#else
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
#endif

/* sceVfpuVector2Clamp: min(max(v, lo), hi) */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_clamp(ScePspFVector2 *out, const ScePspFVector2 *v, float lo, float hi)
{
    move v0, a0
    mfc1 t0, $f12
    mfc1 t1, $f13
    mtv t0, S010
    mtv t1, S011
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    vpfxt x, x, z, w
    vmax.p C000, C000, C010
    vpfxt y, y, z, w
    vmin.p C000, C000, C010
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    jr ra
    nop
}
#else
ScePspFVector2 *vec2_clamp(ScePspFVector2 *out, const ScePspFVector2 *v, float lo, float hi)
{
    float x = v->x > lo ? v->x : lo;
    float y = v->y > lo ? v->y : lo;

    out->x = x < hi ? x : hi;
    out->y = y < hi ? y : hi;
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
#else
ScePspFVector2 *vec2_max(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = b->x < a->x ? a->x : b->x;
    out->y = b->y < a->y ? a->y : b->y;
    return out;
}
#endif

/* sceVfpuVector2Min */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_min(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    lwc1 $f3, 0(a1)
    lwc1 $f1, 0(a2)
    move v0, a0
    c.lt.s $f3, $f1
    nop
    bc1t @20
    mov.s $f2, $f3
    mov.s $f2, $f1
@20:
    lwc1 $f3, 4(a1)
    lwc1 $f1, 4(a2)
    swc1 $f2, 0(v0)
    c.lt.s $f3, $f1
    nop
    bc1t @40
    mov.s $f2, $f3
    mov.s $f2, $f1
@40:
    jr ra
    swc1 $f2, 4(v0)
}
#else
ScePspFVector2 *vec2_min(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    out->x = a->x < b->x ? a->x : b->x;
    out->y = a->y < b->y ? a->y : b->y;
    return out;
}
#endif

/* sceVfpuVector2InnerProduct */
#ifdef __MWERKS__
asm float vec2_dot(const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    lwc1 $f0, 0(a1)
    lwc1 $f2, 4(a0)
    lwc1 $f1, 4(a1)
    lwc1 $f5, 0(a0)
    mul.s $f4, $f2, $f1
    mul.s $f3, $f5, $f0
    jr ra
    add.s $f0, $f3, $f4
}
#else
float vec2_dot(const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    return a->x * b->x + a->y * b->y;
}
#endif

/* sceVfpuVector2Funnel: sum of the components */
#ifdef __MWERKS__
asm float vec2_funnel(const ScePspFVector2 *v)
{
    lwc1 $f3, 4(a0)
    lwc1 $f2, 0(a0)
    jr ra
    add.s $f0, $f2, $f3
}
#else
float vec2_funnel(const ScePspFVector2 *v)
{
    return v->x + v->y;
}
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
asm int vec2_is_equal(const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    lv.s S000, 0(a0)
    lv.s S001, 4(a0)
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    move v0, zero
    vcmp.p eq, C000, C010
    bvtl 5, @20
    addiu v0, zero, 1
@20:
    jr ra
    nop
}
#else
int vec2_is_equal(const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    return a->x == b->x && a->y == b->y;
}
#endif

/* sceVfpuVector2IsZero: both components +0 or -0 */
#ifdef __MWERKS__
asm int vec2_is_zero(const ScePspFVector2 *v)
{
    lw v0, 4(a0)
    lw a1, 0(a0)
    or a0, a1, v0
    ins a0, zero, 0x1f, 0x1
    jr ra
    sltiu v0, a0, 1
}
#else
int vec2_is_zero(const ScePspFVector2 *v)
{
    return v->x == 0.0f && v->y == 0.0f;
}
#endif

/* sceVfpuVector2Normalize: 0 for a zero vector, components saturated to [-1, 1] */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_normalize(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    vdot.p S010, C000, C000
    vzero.s S011
    vcmp.s ez, S010, S010
    vrsq.s S010, S010
    vcmovt.s S010, S011, 0
    vpfxd [-1:1], [-1:1], m, m
    vscl.p C000, C000, S010
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    jr ra
    nop
}
#else
ScePspFVector2 *vec2_normalize(ScePspFVector2 *out, const ScePspFVector2 *v)
{
    float s = rsq0(v->x * v->x + v->y * v->y);

    out->x = sat1(v->x * s);
    out->y = sat1(v->y * s);
    return out;
}
#endif

/* sceVfpuVector2Length */
#ifdef __MWERKS__
asm float vec2_length(const ScePspFVector2 *v)
{
    addiu sp, sp, -16
    lv.s S000, 0(a0)
    lv.s S001, 4(a0)
    vdot.p S000, C000, C000
    vsqrt.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec2_length(const ScePspFVector2 *v)
{
    return sqrtf(v->x * v->x + v->y * v->y);
}
#endif

/* sceVfpuVector2Distance */
#ifdef __MWERKS__
asm float vec2_distance(const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    addiu sp, sp, -16
    lv.s S000, 0(a0)
    lv.s S001, 4(a0)
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    vsub.p C000, C000, C010
    vdot.p S000, C000, C000
    vsqrt.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec2_distance(const ScePspFVector2 *a, const ScePspFVector2 *b)
{
    float x = a->x - b->x;
    float y = a->y - b->y;

    return sqrtf(x * x + y * y);
}
#endif

/* n if dot(i, nref) < 0, else -n */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_faceforward(ScePspFVector2 *out, const ScePspFVector2 *n, const ScePspFVector2 *i,
                                 const ScePspFVector2 *nref)
{
    move v0, a0
    lv.s S010, 0(a2)
    lv.s S011, 4(a2)
    lv.s S020, 0(a3)
    lv.s S021, 4(a3)
    vdot.p S030, C010, C020
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    vpfxt 0, y, z, w
    vcmp.s lt, S030, S030
    vpfxs -x, -y, z, w
    vcmovf.p C000, C000, 0
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    jr ra
    nop
}
#else
ScePspFVector2 *vec2_faceforward(ScePspFVector2 *out, const ScePspFVector2 *n, const ScePspFVector2 *i,
                                 const ScePspFVector2 *nref)
{
    float s = i->x * nref->x + i->y * nref->y < 0.0f ? 1.0f : -1.0f;

    out->x = n->x * s;
    out->y = n->y * s;
    return out;
}
#endif

/* normalize(v - 2 dot(v, n) n) */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_reflect(ScePspFVector2 *out, const ScePspFVector2 *v, const ScePspFVector2 *n)
{
    move v0, a0
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    lv.s S020, 0(a2)
    lv.s S021, 4(a2)
    vdot.p S031, C010, C020
    vfim.s S030, -2.0
    vmul.s S032, S030, S031
    vscl.p C020, C020, S032
    vadd.p C000, C010, C020
    vdot.p S033, C000, C000
    vcmp.s ez, S033, S033
    vrsq.s S033, S033
    vpfxs 0, y, z, w
    vcmovt.s S033, S033, 0
    vpfxd [-1:1], [-1:1], m, m
    vscl.p C000, C000, S033
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    jr ra
    nop
}
#else
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
#endif

/* normalize(eta v + (eta dot(v, n) - eta^2) n), zero when that is not finite. The original computes
   sqrt(1 - eta^2 (1 - dot^2)) but never uses it (it subtracts eta^2 where the root should go). */
#ifdef __MWERKS__
asm ScePspFVector2 *vec2_refract(ScePspFVector2 *out, const ScePspFVector2 *v, const ScePspFVector2 *n, float eta)
{
    addiu sp, sp, -16
    move v0, a0
    swc1 $f12, 0(sp)
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    lv.s S020, 0(a2)
    lv.s S021, 4(a2)
    lv.s S030, 0(sp)
    vdot.p S031, C010, C020
    vscl.p C010, C010, S030
    vmul.s S032, S030, S030
    vmul.s S033, S031, S031
    vmul.s S031, S031, S030
    vocp.s S033, S033
    vmul.s S033, S032, S033
    vocp.s S033, S033
    vsqrt.s S033, S033
    vsub.s S031, S031, S032
    vscl.p C020, C020, S031
    vadd.p C000, C010, C020
    vdot.p S033, C000, C000
    vcmp.p es, C000, C000
    vrsq.s S033, S033
    vpfxd [-1:1], [-1:1], m, m
    vscl.p C000, C000, S033
    vpfxs 0, 0, z, w
    vcmovt.p C000, C000, 4
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    jr ra
    addiu sp, sp, 16
}
#else
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
#endif
