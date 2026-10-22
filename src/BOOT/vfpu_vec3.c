// @range 0x0881AB1C-0x0881B244
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
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_set(ScePspFVector3 *out, float x, float y, float z)
{
    move v0, a0
    swc1 $f12, 0(a0)
    swc1 $f13, 4(a0)
    jr ra
    swc1 $f14, 8(a0)
}
#else
ScePspFVector3 *vec3_set(ScePspFVector3 *out, float x, float y, float z)
{
    out->x = x;
    out->y = y;
    out->z = z;
    return out;
}
#endif

/* sceVfpuVector3Copy */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_copy(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    lwc1 $f3, 8(a1)
    lwc1 $f1, 0(a1)
    lwc1 $f2, 4(a1)
    move v0, a0
    swc1 $f1, 0(a0)
    swc1 $f2, 4(a0)
    jr ra
    swc1 $f3, 8(a0)
}
#else
ScePspFVector3 *vec3_copy(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    *out = *v;
    return out;
}
#endif

/* sceVfpuVector3PositiveZero */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_zero(ScePspFVector3 *out)
{
    move v0, a0
    sw zero, 0(a0)
    sw zero, 4(a0)
    jr ra
    sw zero, 8(a0)
}
#else
ScePspFVector3 *vec3_zero(ScePspFVector3 *out)
{
    out->x = 0.0f;
    out->y = 0.0f;
    out->z = 0.0f;
    return out;
}
#endif

/* sceVfpuVector3NegativeZero */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_neg_zero(ScePspFVector3 *out)
{
    lui a1, 0x8000
    move v0, a0
    sw a1, 8(a0)
    sw a1, 0(a0)
    jr ra
    sw a1, 4(a0)
}
#else
ScePspFVector3 *vec3_neg_zero(ScePspFVector3 *out)
{
    out->x = -0.0f;
    out->y = -0.0f;
    out->z = -0.0f;
    return out;
}
#endif

/* sceVfpuVector3Ceil: vf2id, rounds down [?] (see vec2_ceil). */
#ifdef __MWERKS__
asm ScePspIVector3 *vec3_ceil(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    vf2id.t C000, C000, 0
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspIVector3 *vec3_ceil(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    out->x = (int)floorf(v->x);
    out->y = (int)floorf(v->y);
    out->z = (int)floorf(v->z);
    return out;
}
#endif

/* sceVfpuVector3Trunc (vf2iz) */
#ifdef __MWERKS__
asm ScePspIVector3 *vec3_trunc(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    vf2iz.t C000, C000, 0
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspIVector3 *vec3_trunc(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    out->x = (int)v->x;
    out->y = (int)v->y;
    out->z = (int)v->z;
    return out;
}
#endif

/* sceVfpuVector3Round (vf2in) */
#ifdef __MWERKS__
asm ScePspIVector3 *vec3_round(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    vf2in.t C000, C000, 0
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspIVector3 *vec3_round(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    out->x = (int)rintf(v->x);
    out->y = (int)rintf(v->y);
    out->z = (int)rintf(v->z);
    return out;
}
#endif

/* sceVfpuVector3Floor: vf2iu, rounds up [?] (see vec2_ceil). */
#ifdef __MWERKS__
asm ScePspIVector3 *vec3_floor(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    vf2iu.t C000, C000, 0
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspIVector3 *vec3_floor(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    out->x = (int)ceilf(v->x);
    out->y = (int)ceilf(v->y);
    out->z = (int)ceilf(v->z);
    return out;
}
#endif

/* sceVfpuVector3FromIVector */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_from_ivec(ScePspFVector3 *out, const ScePspIVector3 *v)
{
    lwc1 $f1, 8(a1)
    lwc1 $f2, 0(a1)
    lwc1 $f3, 4(a1)
    cvt.s.w $f4, $f1
    cvt.s.w $f0, $f2
    cvt.s.w $f1, $f3
    move v0, a0
    swc1 $f4, 8(a0)
    swc1 $f0, 0(a0)
    jr ra
    swc1 $f1, 4(a0)
}
#else
ScePspFVector3 *vec3_from_ivec(ScePspFVector3 *out, const ScePspIVector3 *v)
{
    out->x = (float)v->x;
    out->y = (float)v->y;
    out->z = (float)v->z;
    return out;
}
#endif

/* sceVfpuVector3Add */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_add(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    lv.s S010, 0(a2)
    lv.s S011, 4(a2)
    lv.s S012, 8(a2)
    vadd.t C000, C000, C010
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspFVector3 *vec3_add(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    return out;
}
#endif

/* sceVfpuVector3Sub */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_sub(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    lv.s S010, 0(a2)
    lv.s S011, 4(a2)
    lv.s S012, 8(a2)
    vsub.t C000, C000, C010
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspFVector3 *vec3_sub(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    return out;
}
#endif

/* sceVfpuVector3Mul */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_mul(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    lv.s S010, 0(a2)
    lv.s S011, 4(a2)
    lv.s S012, 8(a2)
    vmul.t C000, C000, C010
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspFVector3 *vec3_mul(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    out->z = a->z * b->z;
    return out;
}
#endif

/* sceVfpuVector3Div */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_div(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    move v0, a0
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    lv.s S012, 8(a1)
    lv.s S020, 0(a2)
    lv.s S021, 4(a2)
    lv.s S022, 8(a2)
    vdiv.t C000, C010, C020
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspFVector3 *vec3_div(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x / b->x;
    out->y = a->y / b->y;
    out->z = a->z / b->z;
    return out;
}
#endif

/* sceVfpuVector3Neg (flips the sign bits) */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_neg(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    lw t2, 0(a1)
    lui a2, 0x8000
    move v0, a0
    xor v1, t2, a2
    sw v1, 0(a0)
    lw t1, 4(a1)
    xor a3, t1, a2
    sw a3, 4(a0)
    lw t0, 8(a1)
    xor v1, t0, a2
    jr ra
    sw v1, 8(a0)
}
#else
ScePspFVector3 *vec3_neg(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    out->x = -v->x;
    out->y = -v->y;
    out->z = -v->z;
    return out;
}
#endif

/* sceVfpuVector3Abs (clears the sign bits) */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_abs(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    lw v1, 0(a1)
    move v0, a0
    ins v1, zero, 0x1f, 0x1
    sw v1, 0(a0)
    lw a2, 4(a1)
    ins a2, zero, 0x1f, 0x1
    sw a2, 4(a0)
    lw v1, 8(a1)
    ins v1, zero, 0x1f, 0x1
    jr ra
    sw v1, 8(a0)
}
#else
ScePspFVector3 *vec3_abs(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    out->x = fabsf(v->x);
    out->y = fabsf(v->y);
    out->z = fabsf(v->z);
    return out;
}
#endif

/* sceVfpuVector3Lerp: a + t (b - a) */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_lerp(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b, float t)
{
    move v0, a0
    mfc1 t0, $f12
    mtv t0, S030
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    lv.s S012, 8(a1)
    lv.s S020, 0(a2)
    lv.s S021, 4(a2)
    lv.s S022, 8(a2)
    vsub.t C000, C020, C010
    vscl.t C000, C000, S030
    vadd.t C000, C010, C000
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspFVector3 *vec3_lerp(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b, float t)
{
    out->x = a->x + t * (b->x - a->x);
    out->y = a->y + t * (b->y - a->y);
    out->z = a->z + t * (b->z - a->z);
    return out;
}
#endif

/* Cubic Hermite, as vec2_hermite. */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_hermite(ScePspFVector3 *out, const ScePspFVector3 *p0, const ScePspFVector3 *v0,
                             const ScePspFVector3 *p1, const ScePspFVector3 *v1, float t)
{
    addiu sp, sp, -16
    move v0, a0
    swc1 $f12, 0(sp)
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    lv.s S010, 0(a2)
    lv.s S011, 4(a2)
    lv.s S012, 8(a2)
    lv.s S020, 0(t0)
    lv.s S021, 4(t0)
    lv.s S022, 8(t0)
    lv.s S030, 0(a3)
    lv.s S031, 4(a3)
    lv.s S032, 8(a3)
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
    sv.s S222, 8(a0)
    jr ra
    addiu sp, sp, 16
}
#else
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
#endif

/* sceVfpuVector3Scale */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_scale(ScePspFVector3 *out, const ScePspFVector3 *v, float s)
{
    move v0, a0
    mfc1 t0, $f12
    mtv t0, S010
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    vscl.t C000, C000, S010
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspFVector3 *vec3_scale(ScePspFVector3 *out, const ScePspFVector3 *v, float s)
{
    out->x = v->x * s;
    out->y = v->y * s;
    out->z = v->z * s;
    return out;
}
#endif

/* sceVfpuVector3Clamp: min(max(v, lo), hi) */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_clamp(ScePspFVector3 *out, const ScePspFVector3 *v, float lo, float hi)
{
    move v0, a0
    mfc1 t0, $f12
    mfc1 t1, $f13
    mtv t0, S010
    mtv t1, S011
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    vpfxt x, x, x, w
    vmax.t C000, C000, C010
    vpfxt y, y, y, w
    vmin.t C000, C000, C010
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
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
#endif

/* sceVfpuVector3Max */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_max(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    lv.s S010, 0(a2)
    lv.s S011, 4(a2)
    lv.s S012, 8(a2)
    vmax.t C000, C000, C010
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspFVector3 *vec3_max(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x > b->x ? a->x : b->x;
    out->y = a->y > b->y ? a->y : b->y;
    out->z = a->z > b->z ? a->z : b->z;
    return out;
}
#endif

/* sceVfpuVector3Min */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_min(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    lv.s S010, 0(a2)
    lv.s S011, 4(a2)
    lv.s S012, 8(a2)
    vmin.t C000, C000, C010
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspFVector3 *vec3_min(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    out->x = a->x < b->x ? a->x : b->x;
    out->y = a->y < b->y ? a->y : b->y;
    out->z = a->z < b->z ? a->z : b->z;
    return out;
}
#endif

/* sceVfpuVector3InnerProduct */
#ifdef __MWERKS__
asm float vec3_dot(const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    addiu sp, sp, -16
    lv.s S000, 0(a0)
    lv.s S001, 4(a0)
    lv.s S002, 8(a0)
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    lv.s S012, 8(a1)
    vdot.t S000, C000, C010
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec3_dot(const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}
#endif

/* sceVfpuVector3OuterProduct: a x b */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_cross_nw(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    move v0, a0
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    lv.s S012, 8(a1)
    lv.s S020, 0(a2)
    lv.s S021, 4(a2)
    lv.s S022, 8(a2)
    vcrsp.t C000, C010, C020
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
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
#endif

/* sceVfpuVector3Funnel: sum of the components */
#ifdef __MWERKS__
asm float vec3_funnel(const ScePspFVector3 *v)
{
    addiu sp, sp, -16
    lv.s S000, 0(a0)
    lv.s S001, 4(a0)
    lv.s S002, 8(a0)
    vfad.t S000, C000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec3_funnel(const ScePspFVector3 *v)
{
    return v->x + v->y + v->z;
}
#endif

/* sceVfpuVector3Average */
#ifdef __MWERKS__
asm float vec3_average(const ScePspFVector3 *v)
{
    addiu sp, sp, -16
    lv.s S000, 0(a0)
    lv.s S001, 4(a0)
    lv.s S002, 8(a0)
    vavg.t S000, C000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec3_average(const ScePspFVector3 *v)
{
    return (v->x + v->y + v->z) / 3.0f;
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
asm int vec3_is_zero(const ScePspFVector3 *v)
{
    lw v0, 4(a0)
    lw a1, 0(a0)
    lw a2, 8(a0)
    or a0, a1, v0
    or v1, a0, a2
    ins v1, zero, 0x1f, 0x1
    jr ra
    sltiu v0, v1, 1
}
#else
int vec3_is_zero(const ScePspFVector3 *v)
{
    return v->x == 0.0f && v->y == 0.0f && v->z == 0.0f;
}
#endif

/* sceVfpuVector3SignFloat */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_sign_float(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    vsgn.t C000, C000
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspFVector3 *vec3_sign_float(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    out->x = sgn(v->x);
    out->y = sgn(v->y);
    out->z = sgn(v->z);
    return out;
}
#endif

/* sceVfpuVector3SignInt */
#ifdef __MWERKS__
asm ScePspIVector3 *vec3_sign_int(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    vsgn.t C000, C000
    vf2iz.t C000, C000, 0
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspIVector3 *vec3_sign_int(ScePspIVector3 *out, const ScePspFVector3 *v)
{
    out->x = (int)sgn(v->x);
    out->y = (int)sgn(v->y);
    out->z = (int)sgn(v->z);
    return out;
}
#endif

/* sceVfpuVector3Normalize: 0 for a zero vector, components saturated to [-1, 1] */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_normalize(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    move v0, a0
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    vdot.t S010, C000, C000
    vzero.s S011
    vcmp.s ez, S010, S010
    vrsq.s S010, S010
    vcmovt.s S010, S011, 0
    vpfxd [-1:1], [-1:1], [-1:1], m
    vscl.t C000, C000, S010
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspFVector3 *vec3_normalize(ScePspFVector3 *out, const ScePspFVector3 *v)
{
    float s = rsq0(v->x * v->x + v->y * v->y + v->z * v->z);

    out->x = sat1(v->x * s);
    out->y = sat1(v->y * s);
    out->z = sat1(v->z * s);
    return out;
}
#endif

/* sceVfpuVector3Length */
#ifdef __MWERKS__
asm float vec3_length(const ScePspFVector3 *v)
{
    addiu sp, sp, -16
    lv.s S000, 0(a0)
    lv.s S001, 4(a0)
    lv.s S002, 8(a0)
    vdot.t S000, C000, C000
    vsqrt.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec3_length(const ScePspFVector3 *v)
{
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}
#endif

/* sceVfpuVector3Distance */
#ifdef __MWERKS__
asm float vec3_distance(const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    addiu sp, sp, -16
    lv.s S000, 0(a0)
    lv.s S001, 4(a0)
    lv.s S002, 8(a0)
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    lv.s S012, 8(a1)
    vsub.t C000, C000, C010
    vdot.t S000, C000, C000
    vsqrt.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec3_distance(const ScePspFVector3 *a, const ScePspFVector3 *b)
{
    float x = a->x - b->x;
    float y = a->y - b->y;
    float z = a->z - b->z;

    return sqrtf(x * x + y * y + z * z);
}
#endif

/* n if dot(i, nref) < 0, else -n */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_faceforward(ScePspFVector3 *out, const ScePspFVector3 *n, const ScePspFVector3 *i,
                                 const ScePspFVector3 *nref)
{
    move v0, a0
    lv.s S010, 0(a2)
    lv.s S011, 4(a2)
    lv.s S012, 8(a2)
    lv.s S020, 0(a3)
    lv.s S021, 4(a3)
    lv.s S022, 8(a3)
    vdot.t S030, C010, C020
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.s S002, 8(a1)
    vpfxt 0, y, z, w
    vcmp.s lt, S030, S030
    vpfxs -x, -y, -z, w
    vcmovf.t C000, C000, 0
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
ScePspFVector3 *vec3_faceforward(ScePspFVector3 *out, const ScePspFVector3 *n, const ScePspFVector3 *i,
                                 const ScePspFVector3 *nref)
{
    float s = i->x * nref->x + i->y * nref->y + i->z * nref->z < 0.0f ? 1.0f : -1.0f;

    out->x = n->x * s;
    out->y = n->y * s;
    out->z = n->z * s;
    return out;
}
#endif

/* normalize(v - 2 dot(v, n) n) */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_reflect(ScePspFVector3 *out, const ScePspFVector3 *v, const ScePspFVector3 *n)
{
    move v0, a0
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    lv.s S012, 8(a1)
    lv.s S020, 0(a2)
    lv.s S021, 4(a2)
    lv.s S022, 8(a2)
    vdot.t S031, C010, C020
    vfim.s S030, -2.0
    vmul.s S032, S030, S031
    vscl.t C020, C020, S032
    vadd.t C000, C010, C020
    vdot.t S033, C000, C000
    vcmp.s ez, S033, S033
    vrsq.s S033, S033
    vpfxs 0, y, z, w
    vcmovt.s S033, S033, 0
    vpfxd [-1:1], [-1:1], [-1:1], m
    vscl.t C000, C000, S033
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    nop
}
#else
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
#endif

/* normalize(eta v + (eta dot(v, n) - eta^2) n), zero when that is not finite (see vec2_refract). */
#ifdef __MWERKS__
asm ScePspFVector3 *vec3_refract(ScePspFVector3 *out, const ScePspFVector3 *v, const ScePspFVector3 *n, float eta)
{
    addiu sp, sp, -16
    move v0, a0
    swc1 $f12, 0(sp)
    lv.s S010, 0(a1)
    lv.s S011, 4(a1)
    lv.s S012, 8(a1)
    lv.s S020, 0(a2)
    lv.s S021, 4(a2)
    lv.s S022, 8(a2)
    lv.s S030, 0(sp)
    vdot.t S031, C010, C020
    vscl.t C010, C010, S030
    vmul.s S032, S030, S030
    vmul.s S033, S031, S031
    vmul.s S031, S031, S030
    vocp.s S033, S033
    vmul.s S033, S032, S033
    vocp.s S033, S033
    vsqrt.s S033, S033
    vsub.s S031, S031, S032
    vscl.t C020, C020, S031
    vadd.t C000, C010, C020
    vdot.t S033, C000, C000
    vcmp.t es, C000, C000
    vrsq.s S033, S033
    vpfxd [-1:1], [-1:1], [-1:1], m
    vscl.t C000, C000, S033
    vpfxs 0, 0, 0, w
    vcmovt.t C000, C000, 4
    sv.s S000, 0(a0)
    sv.s S001, 4(a0)
    sv.s S002, 8(a0)
    jr ra
    addiu sp, sp, 16
}
#else
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
#endif
