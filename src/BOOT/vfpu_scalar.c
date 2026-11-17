// @range 0x0881CD50-0x0881D214
// @gcc 0x0881CD50-0x0881D214
/* VFPU scalar math: the float functions of libm computed by the VFPU (angles in radians, scaled by
   2/pi for vsin/vcos/vasin, which work in quarter turns). Hand-written code kept as whole `asm`
   functions; the _vfpu suffix keeps them apart from the libm names (sqrtf is also an mwcc intrinsic).
   Outside the PSP build they map to libm. */

#ifndef __MWERKS__
#include <math.h>
#endif


#ifdef __MWERKS__
float fabsf_vfpu(float x);
#elif !defined(__mips__)
float fabsf_vfpu(float x)
{
    return fabsf(x);
}
#else
float fabsf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vabs.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

/* vasin on x below sqrt(1/2), on sqrt(1 - x^2) above */
#ifdef __MWERKS__
float acosf_vfpu(float x);
#elif !defined(__mips__)
float acosf_vfpu(float x)
{
    return acosf(x);
}
#else
float acosf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vcst.s S001, VFPU_SQRT1_2\n"
        "vcst.s S002, VFPU_PI_2\n"
        "vpfxs |X|, Y, Z, W\n"
        "vcmp.s LT, S000, S001\n"
        "vmul.s S003, S000, S000\n"
        "vsgn.s S001, S000\n"
        "bvtl 0, 1f\n"
        "vasin.s S000, S000\n"
        "vocp.s S003, S003\n"
        "vsqrt.s S003, S003\n"
        "vasin.s S003, S003\n"
        "vocp.s S000, S003\n"
        "vmul.s S000, S000, S001\n"
        "1:\n"
        "vocp.s S000, S000\n"
        "vmul.s S000, S000, S002\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

/* vasin on x below sqrt(1/2), on sqrt(1 - x^2) above */
#ifdef __MWERKS__
float asinf_vfpu(float x);
#elif !defined(__mips__)
float asinf_vfpu(float x)
{
    return asinf(x);
}
#else
float asinf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vcst.s S001, VFPU_SQRT1_2\n"
        "vcst.s S002, VFPU_PI_2\n"
        "vpfxs |X|, Y, Z, W\n"
        "vcmp.s LT, S000, S001\n"
        "vmul.s S003, S000, S000\n"
        "vsgn.s S001, S000\n"
        "bvtl 0, 1f\n"
        "vasin.s S000, S000\n"
        "vocp.s S003, S003\n"
        "vsqrt.s S003, S003\n"
        "vmul.s S002, S002, S001\n"
        "vasin.s S003, S003\n"
        "vocp.s S000, S003\n"
        "1:\n"
        "vmul.s S000, S000, S002\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

/* asin(x / sqrt(1 + x^2)); NaN is returned as is */
#ifdef __MWERKS__
float atanf_vfpu(float x);
#elif !defined(__mips__)
float atanf_vfpu(float x)
{
    return atanf(x);
}
#else
float atanf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vcmp.s EN, S000, S000\n"
        "vmul.s S001, S000, S000\n"
        "bvt 0, 3f\n"
        "vone.s S002\n"
        "vpfxs |X|, Y, Z, W\n"
        "vcmp.s LT, S000, S002\n"
        "vadd.s S003, S001, S002\n"
        "vcst.s S002, VFPU_PI_2\n"
        "bvfl 0, 1f\n"
        "vrcp.s S003, S003\n"
        "vrsq.s S003, S003\n"
        "vmul.s S000, S000, S003\n"
        "b 2f\n"
        "vasin.s S000, S000\n"
        "1:\n"
        "vmul.s S003, S001, S003\n"
        "vsgn.s S001, S000\n"
        "vcmp.s ES, S003, S003\n"
        "vsocp.s C012, S003\n"
        "vsqrt.s S003, S012\n"
        "vmul.s S002, S002, S001\n"
        "bvtl 0, 2f\n"
        "vone.s S000\n"
        "vasin.s S003, S003\n"
        "vocp.s S000, S003\n"
        "2:\n"
        "vmul.s S000, S000, S002\n"
        "3:\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

/* quadrant fix-up of atan(y / x), with the zero/infinite cases of C99 */
#ifdef __MWERKS__
float atan2f_vfpu(float y, float x);
#elif !defined(__mips__)
float atan2f_vfpu(float y, float x)
{
    return atan2f(y, x);
}
#else
float atan2f_vfpu(float y, float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "lv.s S001, %2\n"
        "vcmp.p ES, C000, C000\n"
        "vcst.s S002, VFPU_PI_2\n"
        "bvt 4, 1f\n"
        "vrcp.s S003, S001\n"
        "vmul.s S003, S000, S003\n"
        "vcmp.s EN, S003, S003\n"
        "bvt 0, 5f\n"
        "vmov.s S010, S003\n"
        "vcmp.s ES, S003, S003\n"
        "bvtl 0, 3f\n"
        "vsgn.s S010, S000\n"
        "vmul.s S011, S003, S003\n"
        "vone.s S003\n"
        "vpfxs |X|, Y, Z, W\n"
        "vcmp.s LT, S010, S003\n"
        "vadd.s S003, S011, S003\n"
        "bvfl 0, 2f\n"
        "vrcp.s S003, S003\n"
        "vrsq.s S003, S003\n"
        "vmul.s S010, S010, S003\n"
        "b 3f\n"
        "vasin.s S010, S010\n"
        "1:\n"
        "vfim.s S010, -1.0\n"
        "bvt 5, 5f\n"
        "vrsq.s S010, S010\n"
        "vcmp.p EN, C000, C000\n"
        "bvt 4, 5f\n"
        "vcmp.p EI, C000, C000\n"
        "vsgn.s S000, S000\n"
        "vcst.s S002, VFPU_PI_4\n"
        "bvt 0, 4f\n"
        "vone.s S010\n"
        "vsgn.s S011, S001\n"
        "b 4f\n"
        "vsub.s S010, S010, S011\n"
        "2:\n"
        "vmul.s S003, S011, S003\n"
        "vocp.s S003, S003\n"
        "vsqrt.s S003, S003\n"
        "vasin.s S003, S003\n"
        "vocp.s S010, S003\n"
        "3:\n"
        ".word 0xDE000C04  /* vpfxd ,[0:1],m,m */\n"
        "vpfxs X, -Y, Z, W\n"
        "vsgn.p C000, C000\n"
        "vcmp.s EZ, S001, S001\n"
        "bvt 0, 4f\n"
        "nop\n"
        "vpfxs 2, Y, Z, W\n"
        "vpfxt |X|, Y, Z, W\n"
        "vsub.s S010, S010, S010\n"
        "4:\n"
        "vpfxs |X|, Y, Z, W\n"
        "vmul.s S010, S010, S000\n"
        "vmul.s S010, S010, S002\n"
        "5:\n"
        "sv.s S010, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (y), "m" (x)
        : "memory");
    return r;
}
#endif

/* vf2iu: rounds toward +infinity */
#ifdef __MWERKS__
float ceilf_vfpu(float x);
#elif !defined(__mips__)
float ceilf_vfpu(float x)
{
    return ceilf(x);
}
#else
float ceilf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vf2iu.s S000, S000, 0\n"
        "vi2f.s S000, S000, 0\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

#ifdef __MWERKS__
float cosf_vfpu(float x);
#elif !defined(__mips__)
float cosf_vfpu(float x)
{
    return cosf(x);
}
#else
float cosf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vcst.s S001, VFPU_2_PI\n"
        "vmul.s S000, S000, S001\n"
        "vcos.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

/* exp2(x * log2(e)) */
#ifdef __MWERKS__
float expf_vfpu(float x);
#elif !defined(__mips__)
float expf_vfpu(float x)
{
    return exp2f(x * 1.44269504f);
}
#else
float expf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vcst.s S001, VFPU_LOG2E\n"
        "vmul.s S000, S000, S001\n"
        "vexp2.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

/* vf2id: rounds toward -infinity */
#ifdef __MWERKS__
float floorf_vfpu(float x);
#elif !defined(__mips__)
float floorf_vfpu(float x)
{
    return floorf(x);
}
#else
float floorf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vf2id.s S000, S000, 0\n"
        "vi2f.s S000, S000, 0\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

/* log2(x) / log2(e) */
#ifdef __MWERKS__
float logf_vfpu(float x);
#elif !defined(__mips__)
float logf_vfpu(float x)
{
    return log2f(x) / 1.44269504f;
}
#else
float logf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vlog2.s S000, S000\n"
        "vcst.s S001, VFPU_LOG2E\n"
        "vdiv.s S000, S000, S001\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

#ifdef __MWERKS__
float log2f_vfpu(float x);
#elif !defined(__mips__)
float log2f_vfpu(float x)
{
    return log2f(x);
}
#else
float log2f_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vlog2.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

/* log2(x) / log2(10) */
#ifdef __MWERKS__
float log10f_vfpu(float x);
#elif !defined(__mips__)
float log10f_vfpu(float x)
{
    return log2f(x) / 3.32192809f;
}
#else
float log10f_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vlog2.s S000, S000\n"
        "vcst.s S001, VFPU_LOG2TEN\n"
        "vdiv.s S000, S000, S001\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

#ifdef __MWERKS__
float fmaxf_vfpu(float a, float b);
#elif !defined(__mips__)
float fmaxf_vfpu(float a, float b)
{
    return a > b ? a : b;
}
#else
float fmaxf_vfpu(float a, float b)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "lv.s S001, %2\n"
        "vmax.s S000, S000, S001\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (a), "m" (b)
        : "memory");
    return r;
}
#endif

#ifdef __MWERKS__
float fminf_vfpu(float a, float b);
#elif !defined(__mips__)
float fminf_vfpu(float a, float b)
{
    return a < b ? a : b;
}
#else
float fminf_vfpu(float a, float b)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "lv.s S001, %2\n"
        "vmin.s S000, S000, S001\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (a), "m" (b)
        : "memory");
    return r;
}
#endif

/* exp2(log2(x) * y): x must be positive */
#ifdef __MWERKS__
float powf_vfpu(float x, float y);
#elif !defined(__mips__)
float powf_vfpu(float x, float y)
{
    return exp2f(log2f(x) * y);
}
#else
float powf_vfpu(float x, float y)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "lv.s S001, %2\n"
        "vlog2.s S000, S000\n"
        "vmul.s S000, S000, S001\n"
        "vexp2.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x), "m" (y)
        : "memory");
    return r;
}
#endif

#ifdef __MWERKS__
float exp2f_vfpu(float x);
#elif !defined(__mips__)
float exp2f_vfpu(float x)
{
    return exp2f(x);
}
#else
float exp2f_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vexp2.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

/* vf2in: to nearest, ties to even (rintf, not roundf) */
#ifdef __MWERKS__
float roundf_vfpu(float x);
#elif !defined(__mips__)
float roundf_vfpu(float x)
{
    return rintf(x);
}
#else
float roundf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vf2in.s S000, S000, 0\n"
        "vi2f.s S000, S000, 0\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

#ifdef __MWERKS__
float rsqrtf(float x);
#elif !defined(__mips__)
float rsqrtf(float x)
{
    return 1.0f / sqrtf(x);
}
#else
float rsqrtf(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vrsq.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

#ifdef __MWERKS__
float sinf_vfpu(float x);
#elif !defined(__mips__)
float sinf_vfpu(float x)
{
    return sinf(x);
}
#else
float sinf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vcst.s S001, VFPU_2_PI\n"
        "vmul.s S000, S000, S001\n"
        "vsin.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

#ifdef __MWERKS__
float sqrtf_vfpu(float x);
#elif !defined(__mips__)
float sqrtf_vfpu(float x)
{
    return sqrtf(x);
}
#else
float sqrtf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vsqrt.s S000, S000\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

/* vrot gives cos and sin, then sin / cos */
#ifdef __MWERKS__
float tanf_vfpu(float x);
#elif !defined(__mips__)
float tanf_vfpu(float x)
{
    return sinf(x) / cosf(x);
}
#else
float tanf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vcst.s S001, VFPU_2_PI\n"
        "vmul.s S000, S000, S001\n"
        "vrot.p C002, S000, [c, s]\n"
        "vdiv.s S000, S003, S002\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif

/* vf2iz: rounds toward zero */
#ifdef __MWERKS__
float truncf_vfpu(float x);
#elif !defined(__mips__)
float truncf_vfpu(float x)
{
    return truncf(x);
}
#else
float truncf_vfpu(float x)
{
    float r;

    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.s S000, %1\n"
        "vf2iz.s S000, S000, 0\n"
        "vi2f.s S000, S000, 0\n"
        "sv.s S000, %0\n"
        ".set pop\n"
        : "=m" (r)
        : "m" (x)
        : "memory");
    return r;
}
#endif
