// @range 0x08819BEC-0x08819F0C
// @asm 0x08819C88-0x08819C9C
/* Sony's libfpu (sceFpu*): single precision helpers on the Allegrex FPU and on the IEEE bit
   patterns (signs, zero/denormal/infinity tests, NaN constructors) plus the FCR31 accessors.
   Hand-written or inline-asm code kept as whole `asm` functions; sceFpuRsqrt loads 1.0f from the
   .rodata pool (D_088EB11C) through a split lui/lwc1 and stays in asm with its C under NON_MATCHING.
   Outside the PSP build the C versions work on the same bit patterns; FCR31 is emulated by a
   variable (only the rounding mode bits matter, and the C code always rounds to nearest). */

#if !defined(__MWERKS__) || defined(NON_MATCHING)
#ifndef __MWERKS__
#include <math.h>
#endif
static unsigned int fpu_bits(float f)
{
    union {
        unsigned int u;
        float f;
    } v;

    v.f = f;
    return v.u;
}

static float fpu_float(unsigned int u)
{
    union {
        unsigned int u;
        float f;
    } v;

    v.u = u;
    return v.f;
}
#define BITS(x) fpu_bits(x)
#define FLOAT(u) fpu_float(u)
#endif

#ifndef __MWERKS__
static unsigned int fpu_fcr31;
#endif


/* FPU control/status register 31 */
#ifdef __MWERKS__
asm unsigned int fpu_get_fcr31(void)
{
    cfc1 v0, $31
    jr ra
    nop
}
#else
unsigned int fpu_get_fcr31(void)
{
    return fpu_fcr31;
}
#endif

/* sets FCR31 (cleared first), returns the old value */
#ifdef __MWERKS__
asm unsigned int fpu_set_fcr31(unsigned int v)
{
    cfc1 v0, $31
    ctc1 zero, $31
    nop
    ctc1 a0, $31
    jr ra
    nop
}
#else
unsigned int fpu_set_fcr31(unsigned int v)
{
    unsigned int old = fpu_fcr31;

    fpu_fcr31 = v;
    return old;
}
#endif

/* sceFpuAbs */
#ifdef __MWERKS__
asm float fpu_abs(float x)
{
    jr ra
    abs.s $f0, $f12
}
#else
float fpu_abs(float x)
{
    return fabsf(x);
}
#endif

/* sceFpuCeil (ceil.w.s) */
#ifdef __MWERKS__
asm int ceil_to_int(float x)
{
    ceil.w.s $f1, $f12
    mfc1 v0, $f1
    jr ra
    nop
}
#else
int ceil_to_int(float x)
{
    return (int)ceilf(x);
}
#endif

/* sceFpuFloor (floor.w.s) */
#ifdef __MWERKS__
asm int floor_to_int(float x)
{
    floor.w.s $f1, $f12
    mfc1 v0, $f1
    jr ra
    nop
}
#else
int floor_to_int(float x)
{
    return (int)floorf(x);
}
#endif

/* sceFpuMax */
#ifdef __MWERKS__
asm float fpu_max(float a, float b)
{
    c.lt.s $f13, $f12
    nop
    bc1t L_08819C4C
    mov.s $f0, $f12
    mov.s $f0, $f13
L_08819C4C:
    jr ra
    nop
}
#else
float fpu_max(float a, float b)
{
    return b < a ? a : b;
}
#endif

/* sceFpuMin */
#ifdef __MWERKS__
asm float fpu_min(float a, float b)
{
    c.lt.s $f12, $f13
    nop
    bc1t L_08819C68
    mov.s $f0, $f12
    mov.s $f0, $f13
L_08819C68:
    jr ra
    nop
}
#else
float fpu_min(float a, float b)
{
    return a < b ? a : b;
}
#endif

/* sceFpuNeg */
#ifdef __MWERKS__
asm float fpu_neg(float x)
{
    jr ra
    neg.s $f0, $f12
}
#else
float fpu_neg(float x)
{
    return -x;
}
#endif

/* sceFpuRound (round.w.s: nearest, ties to even) */
#ifdef __MWERKS__
asm int fpu_round(float x)
{
    round.w.s $f1, $f12
    mfc1 v0, $f1
    jr ra
    nop
}
#else
int fpu_round(float x)
{
    return (int)rintf(x);
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: 1.0f from the .rodata pool (D_088EB11C) */
/* sceFpuRsqrt */
float fpu_rsqrt(float x)
{
    return 1.0f / sqrtf(x);
}
#endif

/* sceFpuSqrt */
#ifdef __MWERKS__
asm float sqrtf_fpu(float x)
{
    jr ra
    sqrt.s $f0, $f12
}
#else
float sqrtf_fpu(float x)
{
    return sqrtf(x);
}
#endif

/* sceFpuTrunc */
#ifdef __MWERKS__
asm int fpu_trunc(float x)
{
    trunc.w.s $f1, $f12
    mfc1 v0, $f1
    jr ra
    nop
}
#else
int fpu_trunc(float x)
{
    return (int)x;
}
#endif

/* sceFpuFmod: a - trunc(a / b) * b */
#ifdef __MWERKS__
asm float fpu_fmod(float a, float b)
{
    div.s $f0, $f12, $f13
    trunc.w.s $f0, $f0
    cvt.s.w $f0, $f0
    mul.s $f0, $f0, $f13
    sub.s $f0, $f12, $f0
    jr ra
    nop
}
#else
float fpu_fmod(float a, float b)
{
    return a - (float)(int)(a / b) * b;
}
#endif

/* sceFpuFrac: x - trunc(x) */
#ifdef __MWERKS__
asm float fpu_frac(float x)
{
    trunc.w.s $f0, $f12
    cvt.s.w $f0, $f0
    sub.s $f0, $f12, $f0
    jr ra
    nop
}
#else
float fpu_frac(float x)
{
    return x - (float)(int)x;
}
#endif

/* sceFpuReinterpretFloat */
#ifdef __MWERKS__
asm float fpu_reinterpret_float(unsigned int u)
{
    mtc1 a0, $f0
    jr ra
    nop
}
#else
float fpu_reinterpret_float(unsigned int u)
{
    union { unsigned int u; float f; } v;

    v.u = u;
    return v.f;
}
#endif

/* sceFpuReinterpretUint */
#ifdef __MWERKS__
asm unsigned int fpu_reinterpret_uint(float f)
{
    mfc1 v0, $f12
    jr ra
    nop
}
#else
unsigned int fpu_reinterpret_uint(float f)
{
    union { unsigned int u; float f; } v;

    v.f = f;
    return v.u;
}
#endif

/* sceFpuIsEqual */
#ifdef __MWERKS__
asm int fpu_is_equal(float a, float b)
{
    c.eq.s $f13, $f12
    move v0, zero
    bc1tl L_08819D0C
    addiu v0, zero, 1
L_08819D0C:
    jr ra
    nop
}
#else
int fpu_is_equal(float a, float b)
{
    return a == b;
}
#endif

/* sceFpuSignFloat: +-1.0, +-0.0 for zero and denormals */
#ifdef __MWERKS__
asm float fpu_sign_float(float x)
{
    mfc1 t0, $f12
    lui t2, 0x3f80
    srl t1, t0, 0x17
    srl t0, t0, 0x1f
    andi t1, t1, 0xff
    sll t0, t0, 0x1f
    movz t2, zero, t1
    or t2, t2, t0
    mtc1 t2, $f0
    jr ra
    nop
}
#else
float fpu_sign_float(float x)
{
    unsigned int u = BITS(x);

    return FLOAT((u >> 23 & 0xFF) ? ((u & 0x80000000) | 0x3F800000) : (u & 0x80000000));
}
#endif

/* sceFpuSignInt: +-1, 0 for zero and denormals */
#ifdef __MWERKS__
asm int fpu_sign_int(float x)
{
    mfc1 v0, $f12
    lui t0, 0x7f80
    and t0, t0, v0
    sra v0, v0, 0x1e
    ori v0, v0, 0x1
    movz v0, zero, t0
    jr ra
    nop
}
#else
int fpu_sign_int(float x)
{
    unsigned int u = BITS(x);

    return (u & 0x7F800000) ? ((int)u >> 30 | 1) : 0;
}
#endif

#ifdef __MWERKS__
asm float fpu_positive_zero(void)
{
    mtc1 zero, $f0
    jr ra
    nop
}
#else
float fpu_positive_zero(void)
{
    return FLOAT(0);
}
#endif

#ifdef __MWERKS__
asm float fpu_negative_zero(void)
{
    lui t0, 0x8000
    mtc1 t0, $f0
    jr ra
    nop
}
#else
float fpu_negative_zero(void)
{
    return FLOAT(0x80000000);
}
#endif

/* sceFpuIsZero: +-1 (the sign) for +-0.0, else 0 */
#ifdef __MWERKS__
asm int fpu_is_zero(float x)
{
    mfc1 t0, $f12
    sra v0, t0, 0x1e
    sll t0, t0, 0x1
    ori v0, v0, 0x1
    movn v0, zero, t0
    jr ra
    nop
}
#else
int fpu_is_zero(float x)
{
    unsigned int u = BITS(x);

    return (u << 1) ? 0 : ((int)u >> 30 | 1);
}
#endif

#ifdef __MWERKS__
asm int fpu_is_positive_zero(float x)
{
    mfc1 v0, $f12
    sltiu v0, v0, 1
    jr ra
    nop
}
#else
int fpu_is_positive_zero(float x)
{
    return BITS(x) == 0;
}
#endif

#ifdef __MWERKS__
asm int fpu_is_negative_zero(float x)
{
    mfc1 v0, $f12
    lui t0, 0x8000
    xor v0, v0, t0
    sltiu v0, v0, 1
    jr ra
    nop
}
#else
int fpu_is_negative_zero(float x)
{
    return BITS(x) == 0x80000000;
}
#endif

/* +-1 (the sign) for a denormal, else 0 */
#ifdef __MWERKS__
asm int fpu_is_denormal(float x)
{
    mfc1 t0, $f12
    lui t1, 0x7f80
    or v0, t0, t1
    and t1, t0, t1
    sra v0, v0, 0x1e
    sll t0, t0, 0x9
    movn v0, zero, t1
    movz v0, zero, t0
    jr ra
    nop
}
#else
int fpu_is_denormal(float x)
{
    unsigned int u = BITS(x);

    return ((u & 0x7F800000) || !(u << 9)) ? 0 : ((int)(u | 0x7F800000) >> 30);
}
#endif

/* +-1 (the sign) when the exponent is 0, else 0 */
#ifdef __MWERKS__
asm int fpu_is_zero_or_denormal(float x)
{
    mfc1 t0, $f12
    lui t1, 0x7f80
    or v0, t0, t1
    and t1, t0, t1
    sra v0, v0, 0x1e
    movn v0, zero, t1
    jr ra
    nop
}
#else
int fpu_is_zero_or_denormal(float x)
{
    unsigned int u = BITS(x);

    return (u & 0x7F800000) ? 0 : ((int)(u | 0x7F800000) >> 30);
}
#endif

#ifdef __MWERKS__
asm float fpu_positive_inf(void)
{
    lui t0, 0x7f80
    mtc1 t0, $f0
    jr ra
    nop
}
#else
float fpu_positive_inf(void)
{
    return FLOAT(0x7F800000);
}
#endif

#ifdef __MWERKS__
asm float fpu_negative_inf(void)
{
    lui t0, 0xff80
    mtc1 t0, $f0
    jr ra
    nop
}
#else
float fpu_negative_inf(void)
{
    return FLOAT(0xFF800000);
}
#endif

/* +-1 (the sign) for an infinity, else 0 */
#ifdef __MWERKS__
asm int fpu_is_inf(float x)
{
    mfc1 t0, $f12
    sll t1, t0, 0x1
    sra v0, t0, 0x1e
    srl t1, t1, 0x18
    sll t0, t0, 0x9
    ori v0, v0, 0x1
    sltiu t1, t1, 255
    movn v0, zero, t0
    movn v0, zero, t1
    jr ra
    nop
}
#else
int fpu_is_inf(float x)
{
    unsigned int u = BITS(x);

    return ((u << 9) || ((u << 1) >> 24) < 255) ? 0 : ((int)u >> 30 | 1);
}
#endif

#ifdef __MWERKS__
asm float fpu_positive_snan(void)
{
    lui t0, 0x7f80
    ori t0, t0, 0x1
    mtc1 t0, $f0
    jr ra
    nop
}
#else
float fpu_positive_snan(void)
{
    return FLOAT(0x7F800001);
}
#endif

#ifdef __MWERKS__
asm float fpu_negative_snan(void)
{
    lui t0, 0xff80
    ori t0, t0, 0x1
    mtc1 t0, $f0
    jr ra
    nop
}
#else
float fpu_negative_snan(void)
{
    return FLOAT(0xFF800001);
}
#endif

/* positive quiet NaN */
#ifdef __MWERKS__
asm float nanf_value(void)
{
    lui t0, 0x7fc0
    mtc1 t0, $f0
    jr ra
    nop
}
#else
float nanf_value(void)
{
    return FLOAT(0x7FC00000);
}
#endif

#ifdef __MWERKS__
asm float fpu_negative_qnan(void)
{
    lui t0, 0xffc0
    mtc1 t0, $f0
    jr ra
    nop
}
#else
float fpu_negative_qnan(void)
{
    return FLOAT(0xFFC00000);
}
#endif

/* signalling NaN with the low 22 bits of payload (1 if they are 0) */
#ifdef __MWERKS__
asm float fpu_positive_nan_payload(unsigned int payload)
{
    addiu t1, zero, 1
    ext t0, a0, 0x0, 0x16
    movn t1, t0, t0
    lui t0, 0x7f80
    or t1, t1, t0
    mtc1 t1, $f0
    jr ra
    nop
}
#else
float fpu_positive_nan_payload(unsigned int payload)
{
    unsigned int m = payload & 0x3FFFFF;

    return FLOAT(0x7F800000 | (m ? m : 1));
}
#endif

#ifdef __MWERKS__
asm float fpu_negative_nan_payload(unsigned int payload)
{
    addiu t1, zero, 1
    ext t0, a0, 0x0, 0x16
    movn t1, t0, t0
    lui t0, 0xff80
    or t1, t1, t0
    mtc1 t1, $f0
    jr ra
    nop
}
#else
float fpu_negative_nan_payload(unsigned int payload)
{
    unsigned int m = payload & 0x3FFFFF;

    return FLOAT(0xFF800000 | (m ? m : 1));
}
#endif

/* sign and fraction bits of a NaN, 0 for anything else */
#ifdef __MWERKS__
asm unsigned int fpu_nan_payload(float x)
{
    lui v0, 0x807f
    mfc1 t0, $f12
    ori v0, v0, 0xffff
    sll t1, t0, 0x1
    and v0, v0, t0
    srl t1, t1, 0x18
    sll t0, t0, 0x9
    sltiu t1, t1, 255
    movz v0, zero, t0
    movn v0, zero, t1
    jr ra
    nop
}
#else
unsigned int fpu_nan_payload(float x)
{
    unsigned int u = BITS(x);

    return (!(u << 9) || ((u << 1) >> 24) < 255) ? 0 : (u & 0x807FFFFF);
}
#endif
