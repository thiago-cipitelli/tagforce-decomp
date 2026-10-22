// @range 0x088087F8-0x0880A5C0
// @asm 0x088087F8-0x08809B54
// @asm 0x08809BB4-0x08809E50
// @asm 0x08809F4C-0x0880A028
// @asm 0x0880A03C-0x0880A154
// @asm 0x0880A220-0x0880A5B4
/* newlib libm (fdlibm float code, GCC) of the Sony SDK libc: the __ieee754_ and __kernel_ cores,
   sinf/cosf/atanf/asinf/atan2f/sqrtf with their SVID error wrappers and a few bit helpers. The
   functions that load their polynomial constants through %hi/%lo stay in the split asm. The
   port's C calls the host libm, whose float functions compute the same results up to the last
   bit (the kernel helpers are only used inside this file); the names carry a libm_ prefix where
   the plain one belongs to the VFPU versions (see vfpu_math.c). */

#ifndef __MWERKS__
#include <math.h>
#endif

float __ieee754_asinf(float x);
float __ieee754_atan2f(float y, float x);
int __ieee754_rem_pio2f(float x, float *y);
float __ieee754_sqrtf(float x);
float __kernel_cosf(float x, float y);
float __kernel_sinf(float x, float y, int iy);
float libm_atanf(float x);

typedef union {
    float f;
    unsigned int i;
} libm_bits;

#ifdef NON_MATCHING /* GCC: constants loaded through %hi/%lo */
float __ieee754_asinf(float x)
{
    return asinf(x);
}

float __ieee754_atan2f(float y, float x)
{
    return atan2f(y, x);
}

/* x - n * pi/2 as y[0] + y[1]; returns n */
int __ieee754_rem_pio2f(float x, float *y)
{
    double n = nearbyint((double)x / 1.5707963267948966);
    double r = (double)x - n * 1.5707963267948966;

    y[0] = (float)r;
    y[1] = (float)(r - (double)y[0]);
    return (int)n;
}

float __ieee754_sqrtf(float x)
{
    return sqrtf(x);
}

/* cos(x + y) for |x| <= pi/4 */
float __kernel_cosf(float x, float y)
{
    return (float)cos((double)x + (double)y);
}

/* ponytail: only __ieee754_rem_pio2f calls it, and the port's version does the reduction in
   double precision instead; this keeps the interface for the large-argument path. */
int __kernel_rem_pio2f(float *x, float *y, int e0, int nx, int prec, const int *ipio2)
{
    double v = ldexp((double)x[0], e0);
    double n = nearbyint(v / 1.5707963267948966);

    y[0] = (float)(v - n * 1.5707963267948966);
    return (int)n & 7;
}

/* sin(x + y) for |x| <= pi/4 (iy = 0: y is 0) */
float __kernel_sinf(float x, float y, int iy)
{
    return (float)sin((double)x + (iy != 0 ? (double)y : 0.0));
}
#endif

/* 1 when the double (lo, hi words) is an infinity */
#ifdef __MWERKS__
asm int __isinfd(unsigned int lo, unsigned int hi)
{
    negu v0, a0
    or v0, v0, a0
    srl v0, v0, 0x1f
    ext v1, a1, 0x0, 0x1f
    or v1, v1, v0
    lui a0, 0x7ff0
    subu v1, a0, v1
    negu v0, v1
    or v1, v1, v0
    srl v1, v1, 0x1f
    addiu v0, zero, 1
    jr ra
    subu v0, v0, v1
}
#else
int __isinfd(unsigned int lo, unsigned int hi)
{
    unsigned int v = (hi & 0x7FFFFFFF) | ((lo | -lo) >> 31);

    return v == 0x7FF00000;
}
#endif

/* 1 when the double (lo, hi words) is a NaN */
#ifdef __MWERKS__
asm int __isnand(unsigned int lo, unsigned int hi)
{
    negu v1, a0
    or v1, v1, a0
    srl v1, v1, 0x1f
    ext v0, a1, 0x0, 0x1f
    or v0, v0, v1
    lui a0, 0x7ff0
    subu v0, a0, v0
    jr ra
    srl v0, v0, 0x1f
}
#else
int __isnand(unsigned int lo, unsigned int hi)
{
    unsigned int v = (hi & 0x7FFFFFFF) | ((lo | -lo) >> 31);

    return v > 0x7FF00000;
}
#endif

/* SVID matherr: no handler */
#ifdef __MWERKS__
asm int matherr(void *exc)
{
    jr ra
    move v0, zero
}
#else
int matherr(void *exc)
{
    return 0;
}
#endif

#ifdef NON_MATCHING /* GCC: constants loaded through %hi/%lo */
float libm_atanf(float x)
{
    return atanf(x);
}
#endif

#ifdef __MWERKS__
asm float libm_copysignf(float x, float y)
{
    mfc1 v1, $f12
    mfc1 v0, $f13
    ins v1, zero, 0x1f, 0x1
    ins v0, zero, 0x0, 0x1f
    or v1, v1, v0
    mtc1 v1, $f0
    jr ra
    nop
}
#else
float libm_copysignf(float x, float y)
{
    libm_bits a;
    libm_bits b;

    a.f = x;
    b.f = y;
    a.i = (a.i & 0x7FFFFFFF) | (b.i & 0x80000000);
    return a.f;
}
#endif

/* newlib cosf */
#ifdef __MWERKS__
asm float libm_cosf(float x)
{
    mfc1 a0, $f12
    lui v0, 0x3f49
    ori v0, v0, 0xfd8
    ext v1, a0, 0x0, 0x1f
    addiu sp, sp, -32
    slt v0, v0, v1
    mtc1 zero, $f13
    beqz v0, @a0
    sw ra, 16(sp)
    lui v0, 0x7f7f
    ori v0, v0, 0xffff
    slt v0, v0, v1
    beqz v0, @48
    nop
    sub.s $f0, $f12, $f12
@3c:
    lw ra, 16(sp)
@40:
    jr ra
    addiu sp, sp, 32
@48:
    jal __ieee754_rem_pio2f
    move a0, sp
    andi v1, v0, 0x3
    addiu v0, zero, 1
    beq v1, v0, @b0
    slti v0, v1, 2
    bnez v0, @94
    addiu v0, zero, 2
    beq v1, v0, @84
    lwc1 $f12, 0(sp)
@70:
    lwc1 $f13, 4(sp)
    jal __kernel_sinf
    addiu a0, zero, 1
    j @40
    lw ra, 16(sp)
@84:
    jal __kernel_cosf
    lwc1 $f13, 4(sp)
    j @3c
    neg.s $f0, $f0
@94:
    bnez v1, @70
    lwc1 $f12, 0(sp)
    lwc1 $f13, 4(sp)
@a0:
    jal __kernel_cosf
    nop
    j @40
    lw ra, 16(sp)
@b0:
    lwc1 $f12, 0(sp)
    lwc1 $f13, 4(sp)
    jal __kernel_sinf
    addiu a0, zero, 1
    j @3c
    neg.s $f0, $f0
}
#else
float libm_cosf(float x)
{
    return cosf(x);
}
#endif

#ifdef __MWERKS__
asm float libm_fabsf(float x)
{
    mfc1 v0, $f12
    ext v0, v0, 0x0, 0x1f
    mtc1 v0, $f0
    jr ra
    nop
}
#else
float libm_fabsf(float x)
{
    return fabsf(x);
}
#endif

#ifdef NON_MATCHING /* GCC: constants loaded through %hi/%lo */
float libm_floorf(float x)
{
    return floorf(x);
}
#endif

#ifdef __MWERKS__
asm int libm_isnanf(float x)
{
    mfc1 v1, $f12
    lui v0, 0x7f80
    ins v1, zero, 0x1f, 0x1
    jr ra
    slt v0, v0, v1
}
#else
int libm_isnanf(float x)
{
    libm_bits b;

    b.f = x;
    return (b.i & 0x7FFFFFFF) > 0x7F800000;
}
#endif

#ifdef NON_MATCHING /* GCC: constants loaded through %hi/%lo */
float libm_scalbnf(float x, int n)
{
    return scalbnf(x, n);
}
#endif

/* newlib sinf */
#ifdef __MWERKS__
asm float msl_sinf(float x)
{
    mfc1 a0, $f12
    lui v0, 0x3f49
    ori v0, v0, 0xfd8
    ext v1, a0, 0x0, 0x1f
    addiu sp, sp, -32
    slt v0, v0, v1
    sw ra, 16(sp)
    mtc1 zero, $f13
    beqz v0, @a8
    move a0, zero
    lui v0, 0x7f7f
    ori v0, v0, 0xffff
    slt v0, v0, v1
    beqz v0, @4c
    nop
    sub.s $f0, $f12, $f12
@40:
    lw ra, 16(sp)
@44:
    jr ra
    addiu sp, sp, 32
@4c:
    jal __ieee754_rem_pio2f
    move a0, sp
    andi v1, v0, 0x3
    addiu v0, zero, 1
    beq v1, v0, @b8
    slti v0, v1, 2
    bnez v0, @98
    addiu v0, zero, 2
    beq v1, v0, @84
    lwc1 $f12, 0(sp)
@74:
    jal __kernel_cosf
    lwc1 $f13, 4(sp)
    j @40
    neg.s $f0, $f0
@84:
    lwc1 $f13, 4(sp)
    jal __kernel_sinf
    addiu a0, zero, 1
    j @40
    neg.s $f0, $f0
@98:
    bnez v1, @74
    lwc1 $f12, 0(sp)
    lwc1 $f13, 4(sp)
    addiu a0, zero, 1
@a8:
    jal __kernel_sinf
    nop
    j @44
    lw ra, 16(sp)
@b8:
    lwc1 $f12, 0(sp)
    jal __kernel_cosf
    lwc1 $f13, 4(sp)
    j @44
    lw ra, 16(sp)
}
#else
float msl_sinf(float x)
{
    return sinf(x);
}
#endif

#ifdef NON_MATCHING /* GCC: constants loaded through %hi/%lo */
/* SVID wrappers: EDOM (33) through errno for the domain errors, as newlib's w_*f.c */
int *__errno(void);

float libm_asinf(float x)
{
    if (fabsf(x) > 1.0f) {
        *__errno() = 33;
    }
    return asinf(x);
}

float atan2f(float y, float x)
{
    if (x == 0.0f && y == 0.0f) {
        *__errno() = 33;
    }
    return (float)atan2((double)y, (double)x);
}

float libm_sqrtf(float x)
{
    if (x < 0.0f) {
        *__errno() = 33;
    }
    return sqrtf(x);
}
#endif

#ifdef __MWERKS__
asm int int_abs(int x)
{
    negu v1, a0
    jr ra
    max v0, a0, v1
}
#else
int int_abs(int x)
{
    return x < 0 ? -x : x;
}
#endif
