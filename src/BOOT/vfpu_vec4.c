// @range 0x0881B244-0x0881B974
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
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_set(ScePspFVector4 *out, float x, float y, float z, float w)
{
    move v0, a0
    swc1 $f12, 0(a0)
    swc1 $f13, 4(a0)
    swc1 $f14, 8(a0)
    jr ra
    swc1 $f15, 12(a0)
}
#else
ScePspFVector4 *vec4_set(ScePspFVector4 *out, float x, float y, float z, float w)
{
    out->x = x;
    out->y = y;
    out->z = z;
    out->w = w;
    return out;
}
#endif

/* sceVfpuVector4SetXYZ: w untouched */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_set_xyz(ScePspFVector4 *out, float x, float y, float z)
{
    move v0, a0
    swc1 $f12, 0(a0)
    swc1 $f13, 4(a0)
    jr ra
    swc1 $f14, 8(a0)
}
#else
ScePspFVector4 *vec4_set_xyz(ScePspFVector4 *out, float x, float y, float z)
{
    out->x = x;
    out->y = y;
    out->z = z;
    return out;
}
#endif

/* sceVfpuVector4Copy */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_copy(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    lwc1 $f4, 12(a1)
    lwc1 $f1, 0(a1)
    lwc1 $f2, 4(a1)
    lwc1 $f3, 8(a1)
    move v0, a0
    swc1 $f1, 0(a0)
    swc1 $f2, 4(a0)
    swc1 $f3, 8(a0)
    jr ra
    swc1 $f4, 12(a0)
}
#else
ScePspFVector4 *vec4_copy(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    *out = *v;
    return out;
}
#endif

/* sceVfpuVector4PositiveZero */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_zero(ScePspFVector4 *out)
{
    move v0, a0
    sw zero, 0(a0)
    sw zero, 4(a0)
    sw zero, 8(a0)
    jr ra
    sw zero, 12(a0)
}
#else
ScePspFVector4 *vec4_zero(ScePspFVector4 *out)
{
    out->x = 0.0f;
    out->y = 0.0f;
    out->z = 0.0f;
    out->w = 0.0f;
    return out;
}
#endif

/* sceVfpuVector4NegativeZero */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_neg_zero(ScePspFVector4 *out)
{
    lui a1, 0x8000
    move v0, a0
    sw a1, 12(a0)
    sw a1, 0(a0)
    sw a1, 4(a0)
    jr ra
    sw a1, 8(a0)
}
#else
ScePspFVector4 *vec4_neg_zero(ScePspFVector4 *out)
{
    out->x = -0.0f;
    out->y = -0.0f;
    out->z = -0.0f;
    out->w = -0.0f;
    return out;
}
#endif

/* sceVfpuVector4Ceil: vf2id, rounds down [?] (see vec2_ceil). */
#ifdef __MWERKS__
asm ScePspIVector4 *vec4_ceil(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vf2id.q C000, C000, 0
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspIVector4 *vec4_ceil(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    out->x = (int)floorf(v->x);
    out->y = (int)floorf(v->y);
    out->z = (int)floorf(v->z);
    out->w = (int)floorf(v->w);
    return out;
}
#endif

/* sceVfpuVector4Trunc (vf2iz) */
#ifdef __MWERKS__
asm ScePspIVector4 *vec4_trunc(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vf2iz.q C000, C000, 0
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspIVector4 *vec4_trunc(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    out->x = (int)v->x;
    out->y = (int)v->y;
    out->z = (int)v->z;
    out->w = (int)v->w;
    return out;
}
#endif

/* sceVfpuVector4Round (vf2in) */
#ifdef __MWERKS__
asm ScePspIVector4 *vec4_round(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vf2in.q C000, C000, 0
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspIVector4 *vec4_round(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    out->x = (int)rintf(v->x);
    out->y = (int)rintf(v->y);
    out->z = (int)rintf(v->z);
    out->w = (int)rintf(v->w);
    return out;
}
#endif

/* sceVfpuVector4Floor: vf2iu, rounds up [?] (see vec2_ceil). */
#ifdef __MWERKS__
asm ScePspIVector4 *vec4_floor(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vf2iu.q C000, C000, 0
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspIVector4 *vec4_floor(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    out->x = (int)ceilf(v->x);
    out->y = (int)ceilf(v->y);
    out->z = (int)ceilf(v->z);
    out->w = (int)ceilf(v->w);
    return out;
}
#endif

/* sceVfpuVector4FromIVector */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_from_ivec(ScePspFVector4 *out, const ScePspIVector4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vi2f.q C000, C000, 0
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_from_ivec(ScePspFVector4 *out, const ScePspIVector4 *v)
{
    out->x = (float)v->x;
    out->y = (float)v->y;
    out->z = (float)v->z;
    out->w = (float)v->w;
    return out;
}
#endif

/* sceVfpuVector4Add */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_add(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    move v0, a0
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vadd.q C000, C010, C020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_add(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    out->w = a->w + b->w;
    return out;
}
#endif

/* sceVfpuVector4AddXYZ */
#ifdef __MWERKS__
asm ScePspFVector4 *vec3_add_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    move v0, a0
    lv.q C000, 0(a1)
    lv.q C010, 0(a2)
    vadd.t C000, C000, C010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec3_add_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    out->w = a->w;
    return out;
}
#endif

/* sceVfpuVector4Sub */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_sub(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    move v0, a0
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vsub.q C000, C010, C020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_sub(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    out->w = a->w - b->w;
    return out;
}
#endif

/* sceVfpuVector4SubXYZ */
#ifdef __MWERKS__
asm ScePspFVector4 *vec3_sub_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    move v0, a0
    lv.q C000, 0(a1)
    lv.q C010, 0(a2)
    vsub.t C000, C000, C010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec3_sub_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    out->w = a->w;
    return out;
}
#endif

/* sceVfpuVector4Mul */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_mul(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    move v0, a0
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vmul.q C000, C010, C020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_mul(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    out->z = a->z * b->z;
    out->w = a->w * b->w;
    return out;
}
#endif

/* sceVfpuVector4MulXYZ */
#ifdef __MWERKS__
asm ScePspFVector4 *vec3_mul_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    move v0, a0
    lv.q C000, 0(a1)
    lv.q C010, 0(a2)
    vmul.t C000, C000, C010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec3_mul_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    out->z = a->z * b->z;
    out->w = a->w;
    return out;
}
#endif

/* sceVfpuVector4Div */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_div(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    move v0, a0
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vdiv.q C000, C010, C020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_div(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x / b->x;
    out->y = a->y / b->y;
    out->z = a->z / b->z;
    out->w = a->w / b->w;
    return out;
}
#endif

/* sceVfpuVector4DivXYZ */
#ifdef __MWERKS__
asm ScePspFVector4 *vec3_div_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    move v0, a0
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vdiv.t C000, C010, C020
    vmov.s S003, S013
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec3_div_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = a->x / b->x;
    out->y = a->y / b->y;
    out->z = a->z / b->z;
    out->w = a->w;
    return out;
}
#endif

/* sceVfpuVector4Neg (flips the sign bits) */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_neg(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    lw v0, 4(a1)
    lw a2, 0(a1)
    lw t4, 8(a1)
    lw v1, 12(a1)
    lui a3, 0x8000
    xor t2, v0, a3
    xor a1, v1, a3
    xor t3, a2, a3
    xor t1, t4, a3
    move v0, a0
    sw t3, 0(a0)
    sw t2, 4(a0)
    sw t1, 8(a0)
    jr ra
    sw a1, 12(a0)
}
#else
ScePspFVector4 *vec4_neg(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    out->x = -v->x;
    out->y = -v->y;
    out->z = -v->z;
    out->w = -v->w;
    return out;
}
#endif

/* sceVfpuVector4Abs (clears the sign bits) */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_abs(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    lw t0, 12(a1)
    lw v1, 0(a1)
    lw a2, 4(a1)
    lw a3, 8(a1)
    ins v1, zero, 0x1f, 0x1
    ins a2, zero, 0x1f, 0x1
    ins a3, zero, 0x1f, 0x1
    ins t0, zero, 0x1f, 0x1
    move v0, a0
    sw v1, 0(a0)
    sw a2, 4(a0)
    sw a3, 8(a0)
    jr ra
    sw t0, 12(a0)
}
#else
ScePspFVector4 *vec4_abs(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    out->x = fabsf(v->x);
    out->y = fabsf(v->y);
    out->z = fabsf(v->z);
    out->w = fabsf(v->w);
    return out;
}
#endif

/* sceVfpuVector4Lerp: a + t (b - a) */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_lerp(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t)
{
    move v0, a0
    mfc1 t0, $f12
    mtv t0, S030
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vsub.q C000, C020, C010
    vscl.q C000, C000, S030
    vadd.q C010, C010, C000
    sv.q C010, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_lerp(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t)
{
    out->x = a->x + t * (b->x - a->x);
    out->y = a->y + t * (b->y - a->y);
    out->z = a->z + t * (b->z - a->z);
    out->w = a->w + t * (b->w - a->w);
    return out;
}
#endif

/* sceVfpuVector4LerpXYZ */
#ifdef __MWERKS__
asm ScePspFVector4 *vec3_lerp_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t)
{
    move v0, a0
    mfc1 t0, $f12
    mtv t0, S030
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vsub.t C000, C020, C010
    vscl.t C000, C000, S030
    vadd.t C010, C010, C000
    sv.q C010, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec3_lerp_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t)
{
    out->x = a->x + t * (b->x - a->x);
    out->y = a->y + t * (b->y - a->y);
    out->z = a->z + t * (b->z - a->z);
    out->w = a->w;
    return out;
}
#endif

/* Cubic Hermite, as vec2_hermite. */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_hermite(ScePspFVector4 *out, const ScePspFVector4 *p0, const ScePspFVector4 *v0,
                             const ScePspFVector4 *p1, const ScePspFVector4 *v1, float t)
{
    addiu sp, sp, -16
    move v0, a0
    swc1 $f12, 0(sp)
    lv.q C000, 0(a1)
    lv.q C010, 0(a2)
    lv.q C020, 0(t0)
    lv.q C030, 0(a3)
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
    sv.q C220, 0(a0)
    jr ra
    addiu sp, sp, 16
}
#else
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
#endif

/* Cubic Hermite on x, y, z; w = p0->w. */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_hermite_xyz(ScePspFVector4 *out, const ScePspFVector4 *p0, const ScePspFVector4 *v0,
                                 const ScePspFVector4 *p1, const ScePspFVector4 *v1, float t)
{
    addiu sp, sp, -16
    move v0, a0
    swc1 $f12, 0(sp)
    lv.q C000, 0(a1)
    lv.q C010, 0(a2)
    lv.q C020, 0(t0)
    lv.q C030, 0(a3)
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
    vmov.s S223, S003
    sv.q C220, 0(a0)
    jr ra
    addiu sp, sp, 16
}
#else
ScePspFVector4 *vec4_hermite_xyz(ScePspFVector4 *out, const ScePspFVector4 *p0, const ScePspFVector4 *v0,
                                 const ScePspFVector4 *p1, const ScePspFVector4 *v1, float t)
{
    float w = p0->w;

    vec4_hermite(out, p0, v0, p1, v1, t);
    out->w = w;
    return out;
}
#endif

/* sceVfpuVector4Scale */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_scale(ScePspFVector4 *out, const ScePspFVector4 *v, float s)
{
    move v0, a0
    mfc1 t0, $f12
    mtv t0, S010
    lv.q C000, 0(a1)
    vscl.q C000, C000, S010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_scale(ScePspFVector4 *out, const ScePspFVector4 *v, float s)
{
    out->x = v->x * s;
    out->y = v->y * s;
    out->z = v->z * s;
    out->w = v->w * s;
    return out;
}
#endif

/* sceVfpuVector4ScaleXYZ */
#ifdef __MWERKS__
asm ScePspFVector4 *vec3_scale_w(ScePspFVector4 *out, const ScePspFVector4 *v, float s)
{
    move v0, a0
    mfc1 t0, $f12
    mtv t0, S010
    lv.q C000, 0(a1)
    vscl.t C000, C000, S010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec3_scale_w(ScePspFVector4 *out, const ScePspFVector4 *v, float s)
{
    out->x = v->x * s;
    out->y = v->y * s;
    out->z = v->z * s;
    out->w = v->w;
    return out;
}
#endif

/* sceVfpuVector4Clamp: min(max(v, lo), hi) */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_clamp(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi)
{
    move v0, a0
    mfc1 t0, $f12
    mfc1 t1, $f13
    mtv t0, S010
    mtv t1, S011
    lv.q C000, 0(a1)
    vpfxt x, x, x, x
    vmax.q C000, C000, C010
    vpfxt y, y, y, y
    vmin.q C000, C000, C010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_clamp(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi)
{
    out->x = min1(max1(v->x, lo), hi);
    out->y = min1(max1(v->y, lo), hi);
    out->z = min1(max1(v->z, lo), hi);
    out->w = min1(max1(v->w, lo), hi);
    return out;
}
#endif

/* sceVfpuVector4ClampXYZ */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_clamp_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi)
{
    move v0, a0
    mfc1 t0, $f12
    mfc1 t1, $f13
    mtv t0, S010
    mtv t1, S011
    lv.q C000, 0(a1)
    vpfxt x, x, x, w
    vmax.t C000, C000, C010
    vpfxt y, y, y, w
    vmin.t C000, C000, C010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_clamp_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi)
{
    out->x = min1(max1(v->x, lo), hi);
    out->y = min1(max1(v->y, lo), hi);
    out->z = min1(max1(v->z, lo), hi);
    out->w = v->w;
    return out;
}
#endif

/* sceVfpuVector4Max */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_max(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    move v0, a0
    lv.q C000, 0(a1)
    lv.q C010, 0(a2)
    vmax.q C000, C000, C010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_max(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = max1(a->x, b->x);
    out->y = max1(a->y, b->y);
    out->z = max1(a->z, b->z);
    out->w = max1(a->w, b->w);
    return out;
}
#endif

/* sceVfpuVector4Min */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_min(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    move v0, a0
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vmin.q C000, C010, C020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_min(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    out->x = min1(a->x, b->x);
    out->y = min1(a->y, b->y);
    out->z = min1(a->z, b->z);
    out->w = min1(a->w, b->w);
    return out;
}
#endif

/* sceVfpuVector4InnerProduct */
#ifdef __MWERKS__
asm float vec4_dot(const ScePspFVector4 *a, const ScePspFVector4 *b)
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
#else
float vec4_dot(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}
#endif

/* sceVfpuVector4InnerProductXYZ */
#ifdef __MWERKS__
asm float vec3_dot2(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    addiu sp, sp, -16
    lv.q C010, 0(a0)
    lv.q C020, 0(a1)
    vdot.t S000, C010, C020
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec3_dot2(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}
#endif

/* sceVfpuVector4OuterProductXYZ: a x b with w = 0 */
#ifdef __MWERKS__
asm ScePspFVector4 *vec3_cross(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    move v0, a0
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vzero.s S003
    vcrsp.t C000, C010, C020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
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
#endif

/* sceVfpuVector4Funnel: sum of the components */
#ifdef __MWERKS__
asm float vec4_funnel(const ScePspFVector4 *v)
{
    addiu sp, sp, -16
    lv.q C000, 0(a0)
    vfad.q S000, C000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec4_funnel(const ScePspFVector4 *v)
{
    return v->x + v->y + v->z + v->w;
}
#endif

/* sceVfpuVector4Average */
#ifdef __MWERKS__
asm float vec4_average(const ScePspFVector4 *v)
{
    addiu sp, sp, -16
    lv.q C000, 0(a0)
    vavg.q S000, C000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec4_average(const ScePspFVector4 *v)
{
    return (v->x + v->y + v->z + v->w) * 0.25f;
}
#endif

/* sceVfpuVector4IsEqual */
#ifdef __MWERKS__
asm int vec4_is_equal(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    lv.q C000, 0(a0)
    lv.q C010, 0(a1)
    move v0, zero
    vcmp.q eq, C000, C010
    bvtl 5, @18
    addiu v0, zero, 1
@18:
    jr ra
    nop
}
#else
int vec4_is_equal(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    return a->x == b->x && a->y == b->y && a->z == b->z && a->w == b->w;
}
#endif

/* sceVfpuVector4IsZero: every component +0 or -0 */
#ifdef __MWERKS__
asm int vec4_is_zero(const ScePspFVector4 *v)
{
    lw v0, 4(a0)
    lw a1, 0(a0)
    lw t0, 8(a0)
    lw a3, 12(a0)
    or a2, a1, v0
    or a0, a2, t0
    or v1, a0, a3
    ins v1, zero, 0x1f, 0x1
    jr ra
    sltiu v0, v1, 1
}
#else
int vec4_is_zero(const ScePspFVector4 *v)
{
    return v->x == 0.0f && v->y == 0.0f && v->z == 0.0f && v->w == 0.0f;
}
#endif

/* sceVfpuVector4SignFloat */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_sign_float(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vsgn.q C000, C000
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_sign_float(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    out->x = sgn(v->x);
    out->y = sgn(v->y);
    out->z = sgn(v->z);
    out->w = sgn(v->w);
    return out;
}
#endif

/* sceVfpuVector4SignInt */
#ifdef __MWERKS__
asm ScePspIVector4 *vec4_sign_int(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vsgn.q C000, C000
    vf2iz.q C000, C000, 0
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspIVector4 *vec4_sign_int(ScePspIVector4 *out, const ScePspFVector4 *v)
{
    out->x = (int)sgn(v->x);
    out->y = (int)sgn(v->y);
    out->z = (int)sgn(v->z);
    out->w = (int)sgn(v->w);
    return out;
}
#endif

/* sceVfpuVector4Normalize: 0 for a zero vector, components saturated to [-1, 1] */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_normalize(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vdot.q S010, C000, C000
    vzero.s S011
    vcmp.s ez, S010, S010
    vrsq.s S010, S010
    vcmovt.s S010, S011, 0
    vpfxd [-1:1], [-1:1], [-1:1], [-1:1]
    vscl.q C000, C000, S010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec4_normalize(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    float s = rsq0(v->x * v->x + v->y * v->y + v->z * v->z + v->w * v->w);

    out->x = sat1(v->x * s);
    out->y = sat1(v->y * s);
    out->z = sat1(v->z * s);
    out->w = sat1(v->w * s);
    return out;
}
#endif

/* sceVfpuVector4NormalizeXYZ */
#ifdef __MWERKS__
asm ScePspFVector4 *vec3_normalize2(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vdot.t S010, C000, C000
    vzero.s S011
    vcmp.s ez, S010, S010
    vrsq.s S010, S010
    vcmovt.s S010, S011, 0
    vpfxd [-1:1], [-1:1], [-1:1], m
    vscl.t C000, C000, S010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
ScePspFVector4 *vec3_normalize2(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    float s = rsq0(v->x * v->x + v->y * v->y + v->z * v->z);

    out->x = sat1(v->x * s);
    out->y = sat1(v->y * s);
    out->z = sat1(v->z * s);
    out->w = v->w;
    return out;
}
#endif

/* sceVfpuVector4NormalizePhase: v - floor((v + pi) * k) * 2pi with k = VFPU_2_PI (2/pi), as in the
   original; a wrap into [-pi, pi) would need 1/(2pi). */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_normalize_phase(ScePspFVector4 *out, const ScePspFVector4 *v)
{
    move v0, a0
    lv.q C000, 0(a1)
    vcst.q C010, VFPU_PI
    vadd.q C020, C000, C010
    vcst.s S010, VFPU_2_PI
    vscl.q C020, C020, S010
    vf2id.q C020, C020, 0
    vi2f.q C020, C020, 0
    vcst.s S011, VFPU_2PI
    vscl.q C020, C020, S011
    vsub.q C000, C000, C020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
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
#endif

/* sceVfpuVector4LengthXYZ */
#ifdef __MWERKS__
asm float vec4_length_xyz(const ScePspFVector4 *v)
{
    addiu sp, sp, -16
    lv.q C000, 0(a0)
    vdot.t S010, C000, C000
    vsqrt.s S010, S010
    sv.s S010, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec4_length_xyz(const ScePspFVector4 *v)
{
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}
#endif

/* sceVfpuVector4DistanceXYZ */
#ifdef __MWERKS__
asm float vec4_distance_xyz(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    addiu sp, sp, -16
    lv.q C000, 0(a0)
    lv.q C010, 0(a1)
    vsub.t C000, C000, C010
    vdot.t S010, C000, C000
    vsqrt.s S010, S010
    sv.s S010, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float vec4_distance_xyz(const ScePspFVector4 *a, const ScePspFVector4 *b)
{
    float x = a->x - b->x;
    float y = a->y - b->y;
    float z = a->z - b->z;

    return sqrtf(x * x + y * y + z * z);
}
#endif

/* n (x, y, z) if dot(i, nref) < 0, else -n; w = n->w */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_faceforward_xyz(ScePspFVector4 *out, const ScePspFVector4 *n, const ScePspFVector4 *i,
                                     const ScePspFVector4 *nref)
{
    move v0, a0
    lv.q C010, 0(a2)
    lv.q C020, 0(a3)
    vdot.t S030, C010, C020
    lv.q C000, 0(a1)
    vpfxt 0, y, z, w
    vcmp.s lt, S030, S030
    vpfxs -x, -y, -z, w
    vcmovf.t C000, C000, 0
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
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
#endif

/* normalize(v - 2 dot(v, n) n) on x, y, z; w = v->w */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_reflect_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, const ScePspFVector4 *n)
{
    move v0, a0
    lv.q C000, 0(a1)
    lv.q C020, 0(a2)
    vdot.t S031, C000, C020
    vfim.s S030, -2.0
    vmul.s S032, S030, S031
    vscl.t C020, C020, S032
    vadd.t C000, C000, C020
    vdot.t S033, C000, C000
    vcmp.s ez, S033, S033
    vrsq.s S033, S033
    vpfxs 0, y, z, w
    vcmovt.s S033, S033, 0
    vpfxd [-1:1], [-1:1], [-1:1], m
    vscl.t C000, C000, S033
    sv.q C000, 0(a0)
    jr ra
    nop
}
#else
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
#endif

/* vec3_refract on x, y, z; w = v->w */
#ifdef __MWERKS__
asm ScePspFVector4 *vec4_refract_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, const ScePspFVector4 *n, float eta)
{
    addiu sp, sp, -16
    move v0, a0
    swc1 $f12, 0(sp)
    lv.q C000, 0(a1)
    lv.q C020, 0(a2)
    lv.s S030, 0(sp)
    vdot.t S031, C000, C020
    vscl.t C000, C000, S030
    vmul.s S032, S030, S030
    vmul.s S033, S031, S031
    vmul.s S031, S031, S030
    vocp.s S033, S033
    vmul.s S033, S032, S033
    vocp.s S033, S033
    vsqrt.s S033, S033
    vsub.s S031, S031, S032
    vscl.t C020, C020, S031
    vadd.t C000, C000, C020
    vdot.t S033, C000, C000
    vcmp.t es, C000, C000
    vrsq.s S033, S033
    vpfxd [-1:1], [-1:1], [-1:1], m
    vscl.t C000, C000, S033
    vpfxs 0, 0, 0, w
    vcmovt.t C000, C000, 4
    sv.q C000, 0(a0)
    jr ra
    addiu sp, sp, 16
}
#else
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
#endif
