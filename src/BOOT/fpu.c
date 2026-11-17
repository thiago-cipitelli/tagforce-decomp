// @range 0x08819BEC-0x08819F0C
// @gcc 0x08819BEC-0x08819F0C
/* Sony's libfpu (sceFpu*): single precision helpers on the Allegrex FPU and on the IEEE bit
   patterns (signs, zero/denormal/infinity tests, NaN constructors) plus the FCR31 accessors.
   Hand-written or inline-asm code kept as whole `asm` functions; sceFpuRsqrt comes from fpu_gcc.c
   (`-fno-math-errno`), whose one-word pool is the original's (`@rodata 0x088EB11C`).
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
unsigned int fpu_get_fcr31(void);
#elif !defined(__mips__)
unsigned int fpu_get_fcr31(void)
{
    return fpu_fcr31;
}
#else
unsigned int fpu_get_fcr31(void)
{
    unsigned int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "cfc1 %0, $31\n"
        ".set pop\n"
        : "=&r" (r)
        : );
    return r;
}
#endif

/* sets FCR31 (cleared first), returns the old value */
#ifdef __MWERKS__
unsigned int fpu_set_fcr31(unsigned int v);
#elif !defined(__mips__)
unsigned int fpu_set_fcr31(unsigned int v)
{
    unsigned int old = fpu_fcr31;

    fpu_fcr31 = v;
    return old;
}
#else
unsigned int fpu_set_fcr31(unsigned int v)
{
    unsigned int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "cfc1 %0, $31\n"
        "ctc1 $0, $31\n"
        "nop\n"
        "ctc1 %1, $31\n"
        ".set pop\n"
        : "=&r" (r)
        : "r" (v));
    return r;
}
#endif

/* sceFpuAbs */
#ifndef __MWERKS__ /* PSP: fpu_gcc.c */
float fpu_abs(float x)
{
    return fabsf(x);
}
#else
float fpu_abs(float x);
#endif

/* sceFpuCeil (ceil.w.s) */
#ifdef __MWERKS__
int ceil_to_int(float x);
#elif !defined(__mips__)
int ceil_to_int(float x)
{
    return (int)ceilf(x);
}
#else
int ceil_to_int(float x)
{
    int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "ceil.w.s $f1, %1\n"
        "mfc1 %0, $f1\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (x)
        : "$f1");
    return r;
}
#endif

/* sceFpuFloor (floor.w.s) */
#ifdef __MWERKS__
int floor_to_int(float x);
#elif !defined(__mips__)
int floor_to_int(float x)
{
    return (int)floorf(x);
}
#else
int floor_to_int(float x)
{
    int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "floor.w.s $f1, %1\n"
        "mfc1 %0, $f1\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (x)
        : "$f1");
    return r;
}
#endif

/* sceFpuMax */
#ifndef __MWERKS__ /* PSP: fpu_gcc.c */
float fpu_max(float a, float b)
{
    return b < a ? a : b;
}
#else
float fpu_max(float a, float b);
#endif

/* sceFpuMin */
#ifndef __MWERKS__ /* PSP: fpu_gcc.c */
float fpu_min(float a, float b)
{
    return a < b ? a : b;
}
#else
float fpu_min(float a, float b);
#endif

/* sceFpuNeg */
#ifndef __MWERKS__ /* PSP: fpu_gcc.c */
float fpu_neg(float x)
{
    return -x;
}
#else
float fpu_neg(float x);
#endif

/* sceFpuRound (round.w.s: nearest, ties to even) */
#ifdef __MWERKS__
int fpu_round(float x);
#elif !defined(__mips__)
int fpu_round(float x)
{
    return (int)rintf(x);
}
#else
int fpu_round(float x)
{
    int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "round.w.s $f1, %1\n"
        "mfc1 %0, $f1\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (x)
        : "$f1");
    return r;
}
#endif

/* sceFpuRsqrt */
#ifndef __MWERKS__ /* PSP: fpu_gcc.c */
float fpu_rsqrt(float x)
{
    return 1.0f / sqrtf(x);
}
#else
float fpu_rsqrt(float x);
#endif

/* sceFpuSqrt */
#ifdef __MWERKS__
float sqrtf_fpu(float x);
#elif !defined(__mips__)
float sqrtf_fpu(float x)
{
    return sqrtf(x);
}
#else
float sqrtf_fpu(float x)
{
    return __builtin_sqrtf(x);
}
#endif

/* sceFpuTrunc */
#ifndef __MWERKS__ /* PSP: fpu_gcc.c */
int fpu_trunc(float x)
{
    return (int)x;
}
#else
int fpu_trunc(float x);
#endif

/* sceFpuFmod: a - trunc(a / b) * b */
#ifdef __MWERKS__
float fpu_fmod(float a, float b);
#elif !defined(__mips__)
float fpu_fmod(float a, float b)
{
    return a - (float)(int)(a / b) * b;
}
#else
float fpu_fmod(float a, float b)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "div.s %0, %1, %2\n"
        "trunc.w.s %0, %0\n"
        "cvt.s.w %0, %0\n"
        "mul.s %0, %0, %2\n"
        "sub.s %0, %1, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : "f" (a), "f" (b));
    return r;
}
#endif

/* sceFpuFrac: x - trunc(x) */
#ifdef __MWERKS__
float fpu_frac(float x);
#elif !defined(__mips__)
float fpu_frac(float x)
{
    return x - (float)(int)x;
}
#else
float fpu_frac(float x)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "trunc.w.s %0, %1\n"
        "cvt.s.w %0, %0\n"
        "sub.s %0, %1, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : "f" (x));
    return r;
}
#endif

/* sceFpuReinterpretFloat */
#ifndef __MWERKS__ /* PSP: fpu_gcc.c */
float fpu_reinterpret_float(unsigned int u)
{
    union { unsigned int u; float f; } v;

    v.u = u;
    return v.f;
}
#else
float fpu_reinterpret_float(unsigned int u);
#endif

/* sceFpuReinterpretUint */
#ifndef __MWERKS__ /* PSP: fpu_gcc.c */
unsigned int fpu_reinterpret_uint(float f)
{
    union { unsigned int u; float f; } v;

    v.f = f;
    return v.u;
}
#else
unsigned int fpu_reinterpret_uint(float f);
#endif

/* sceFpuIsEqual */
#ifdef __MWERKS__
int fpu_is_equal(float a, float b);
#elif !defined(__mips__)
int fpu_is_equal(float a, float b)
{
    return a == b;
}
#else
int fpu_is_equal(float a, float b)
{
    int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "c.eq.s %2, %1\n"
        "move %0, $0\n"
        "bc1tl 1f\n"
        "addiu %0, $0, 1\n"
        "1:\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (a), "f" (b));
    return r;
}
#endif

/* sceFpuSignFloat: +-1.0, +-0.0 for zero and denormals */
#ifdef __MWERKS__
float fpu_sign_float(float x);
#elif !defined(__mips__)
float fpu_sign_float(float x)
{
    unsigned int u = BITS(x);

    return FLOAT((u >> 23 & 0xFF) ? ((u & 0x80000000) | 0x3F800000) : (u & 0x80000000));
}
#else
float fpu_sign_float(float x)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %1\n"
        "lui $10, 0x3f80\n"
        "srl $9, $8, 0x17\n"
        "srl $8, $8, 0x1f\n"
        "andi $9, $9, 0xff\n"
        "sll $8, $8, 0x1f\n"
        "movz $10, $0, $9\n"
        "or $10, $10, $8\n"
        "mtc1 $10, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : "f" (x)
        : "$10", "$8", "$9");
    return r;
}
#endif

/* sceFpuSignInt: +-1, 0 for zero and denormals */
#ifdef __MWERKS__
int fpu_sign_int(float x);
#elif !defined(__mips__)
int fpu_sign_int(float x)
{
    unsigned int u = BITS(x);

    return (u & 0x7F800000) ? ((int)u >> 30 | 1) : 0;
}
#else
int fpu_sign_int(float x)
{
    int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 %0, %1\n"
        "lui $8, 0x7f80\n"
        "and $8, $8, %0\n"
        "sra %0, %0, 0x1e\n"
        "ori %0, %0, 0x1\n"
        "movz %0, $0, $8\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (x)
        : "$8");
    return r;
}
#endif

#ifndef __MWERKS__ /* PSP: fpu_gcc.c */
float fpu_positive_zero(void)
{
    return FLOAT(0);
}
#else
float fpu_positive_zero(void);
#endif

#ifdef __MWERKS__
float fpu_negative_zero(void);
#elif !defined(__mips__)
float fpu_negative_zero(void)
{
    return FLOAT(0x80000000);
}
#else
float fpu_negative_zero(void)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "lui $8, 0x8000\n"
        "mtc1 $8, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : 
        : "$8");
    return r;
}
#endif

/* sceFpuIsZero: +-1 (the sign) for +-0.0, else 0 */
#ifdef __MWERKS__
int fpu_is_zero(float x);
#elif !defined(__mips__)
int fpu_is_zero(float x)
{
    unsigned int u = BITS(x);

    return (u << 1) ? 0 : ((int)u >> 30 | 1);
}
#else
int fpu_is_zero(float x)
{
    int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %1\n"
        "sra %0, $8, 0x1e\n"
        "sll $8, $8, 0x1\n"
        "ori %0, %0, 0x1\n"
        "movn %0, $0, $8\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (x)
        : "$8");
    return r;
}
#endif

#ifdef __MWERKS__
int fpu_is_positive_zero(float x);
#elif !defined(__mips__)
int fpu_is_positive_zero(float x)
{
    return BITS(x) == 0;
}
#else
int fpu_is_positive_zero(float x)
{
    int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 %0, %1\n"
        "sltiu %0, %0, 1\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (x));
    return r;
}
#endif

#ifdef __MWERKS__
int fpu_is_negative_zero(float x);
#elif !defined(__mips__)
int fpu_is_negative_zero(float x)
{
    return BITS(x) == 0x80000000;
}
#else
int fpu_is_negative_zero(float x)
{
    int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 %0, %1\n"
        "lui $8, 0x8000\n"
        "xor %0, %0, $8\n"
        "sltiu %0, %0, 1\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (x)
        : "$8");
    return r;
}
#endif

/* +-1 (the sign) for a denormal, else 0 */
#ifdef __MWERKS__
int fpu_is_denormal(float x);
#elif !defined(__mips__)
int fpu_is_denormal(float x)
{
    unsigned int u = BITS(x);

    return ((u & 0x7F800000) || !(u << 9)) ? 0 : ((int)(u | 0x7F800000) >> 30);
}
#else
int fpu_is_denormal(float x)
{
    int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %1\n"
        "lui $9, 0x7f80\n"
        "or %0, $8, $9\n"
        "and $9, $8, $9\n"
        "sra %0, %0, 0x1e\n"
        "sll $8, $8, 0x9\n"
        "movn %0, $0, $9\n"
        "movz %0, $0, $8\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (x)
        : "$8", "$9");
    return r;
}
#endif

/* +-1 (the sign) when the exponent is 0, else 0 */
#ifdef __MWERKS__
int fpu_is_zero_or_denormal(float x);
#elif !defined(__mips__)
int fpu_is_zero_or_denormal(float x)
{
    unsigned int u = BITS(x);

    return (u & 0x7F800000) ? 0 : ((int)(u | 0x7F800000) >> 30);
}
#else
int fpu_is_zero_or_denormal(float x)
{
    int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %1\n"
        "lui $9, 0x7f80\n"
        "or %0, $8, $9\n"
        "and $9, $8, $9\n"
        "sra %0, %0, 0x1e\n"
        "movn %0, $0, $9\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (x)
        : "$8", "$9");
    return r;
}
#endif

#ifdef __MWERKS__
float fpu_positive_inf(void);
#elif !defined(__mips__)
float fpu_positive_inf(void)
{
    return FLOAT(0x7F800000);
}
#else
float fpu_positive_inf(void)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "lui $8, 0x7f80\n"
        "mtc1 $8, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : 
        : "$8");
    return r;
}
#endif

#ifdef __MWERKS__
float fpu_negative_inf(void);
#elif !defined(__mips__)
float fpu_negative_inf(void)
{
    return FLOAT(0xFF800000);
}
#else
float fpu_negative_inf(void)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "lui $8, 0xff80\n"
        "mtc1 $8, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : 
        : "$8");
    return r;
}
#endif

/* +-1 (the sign) for an infinity, else 0 */
#ifdef __MWERKS__
int fpu_is_inf(float x);
#elif !defined(__mips__)
int fpu_is_inf(float x)
{
    unsigned int u = BITS(x);

    return ((u << 9) || ((u << 1) >> 24) < 255) ? 0 : ((int)u >> 30 | 1);
}
#else
int fpu_is_inf(float x)
{
    int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "mfc1 $8, %1\n"
        "sll $9, $8, 0x1\n"
        "sra %0, $8, 0x1e\n"
        "srl $9, $9, 0x18\n"
        "sll $8, $8, 0x9\n"
        "ori %0, %0, 0x1\n"
        "sltiu $9, $9, 255\n"
        "movn %0, $0, $8\n"
        "movn %0, $0, $9\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (x)
        : "$8", "$9");
    return r;
}
#endif

#ifdef __MWERKS__
float fpu_positive_snan(void);
#elif !defined(__mips__)
float fpu_positive_snan(void)
{
    return FLOAT(0x7F800001);
}
#else
float fpu_positive_snan(void)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "lui $8, 0x7f80\n"
        "ori $8, $8, 0x1\n"
        "mtc1 $8, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : 
        : "$8");
    return r;
}
#endif

#ifdef __MWERKS__
float fpu_negative_snan(void);
#elif !defined(__mips__)
float fpu_negative_snan(void)
{
    return FLOAT(0xFF800001);
}
#else
float fpu_negative_snan(void)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "lui $8, 0xff80\n"
        "ori $8, $8, 0x1\n"
        "mtc1 $8, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : 
        : "$8");
    return r;
}
#endif

/* positive quiet NaN */
#ifdef __MWERKS__
float nanf_value(void);
#elif !defined(__mips__)
float nanf_value(void)
{
    return FLOAT(0x7FC00000);
}
#else
float nanf_value(void)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "lui $8, 0x7fc0\n"
        "mtc1 $8, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : 
        : "$8");
    return r;
}
#endif

#ifdef __MWERKS__
float fpu_negative_qnan(void);
#elif !defined(__mips__)
float fpu_negative_qnan(void)
{
    return FLOAT(0xFFC00000);
}
#else
float fpu_negative_qnan(void)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "lui $8, 0xffc0\n"
        "mtc1 $8, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : 
        : "$8");
    return r;
}
#endif

/* signalling NaN with the low 22 bits of payload (1 if they are 0) */
#ifdef __MWERKS__
float fpu_positive_nan_payload(unsigned int payload);
#elif !defined(__mips__)
float fpu_positive_nan_payload(unsigned int payload)
{
    unsigned int m = payload & 0x3FFFFF;

    return FLOAT(0x7F800000 | (m ? m : 1));
}
#else
float fpu_positive_nan_payload(unsigned int payload)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "addiu $9, $0, 1\n"
        "ext $8, %1, 0x0, 0x16\n"
        "movn $9, $8, $8\n"
        "lui $8, 0x7f80\n"
        "or $9, $9, $8\n"
        "mtc1 $9, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : "r" (payload)
        : "$8", "$9");
    return r;
}
#endif

#ifdef __MWERKS__
float fpu_negative_nan_payload(unsigned int payload);
#elif !defined(__mips__)
float fpu_negative_nan_payload(unsigned int payload)
{
    unsigned int m = payload & 0x3FFFFF;

    return FLOAT(0xFF800000 | (m ? m : 1));
}
#else
float fpu_negative_nan_payload(unsigned int payload)
{
    float r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "addiu $9, $0, 1\n"
        "ext $8, %1, 0x0, 0x16\n"
        "movn $9, $8, $8\n"
        "lui $8, 0xff80\n"
        "or $9, $9, $8\n"
        "mtc1 $9, %0\n"
        ".set pop\n"
        : "=&f" (r)
        : "r" (payload)
        : "$8", "$9");
    return r;
}
#endif

/* sign and fraction bits of a NaN, 0 for anything else */
#ifdef __MWERKS__
unsigned int fpu_nan_payload(float x);
#elif !defined(__mips__)
unsigned int fpu_nan_payload(float x)
{
    unsigned int u = BITS(x);

    return (!(u << 9) || ((u << 1) >> 24) < 255) ? 0 : (u & 0x807FFFFF);
}
#else
unsigned int fpu_nan_payload(float x)
{
    unsigned int r;

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "lui %0, 0x807f\n"
        "mfc1 $8, %1\n"
        "ori %0, %0, 0xffff\n"
        "sll $9, $8, 0x1\n"
        "and %0, %0, $8\n"
        "srl $9, $9, 0x18\n"
        "sll $8, $8, 0x9\n"
        "sltiu $9, $9, 255\n"
        "movz %0, $0, $8\n"
        "movn %0, $0, $9\n"
        ".set pop\n"
        : "=&r" (r)
        : "f" (x)
        : "$8", "$9");
    return r;
}
#endif
