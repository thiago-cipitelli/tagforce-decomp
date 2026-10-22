// @range 0x0881CD50-0x0881D214
/* VFPU scalar math: the float functions of libm computed by the VFPU (angles in radians, scaled by
   2/pi for vsin/vcos/vasin, which work in quarter turns). Hand-written code kept as whole `asm`
   functions; the _vfpu suffix keeps them apart from the libm names (sqrtf is also an mwcc intrinsic).
   Outside the PSP build they map to libm. */

#ifndef __MWERKS__
#include <math.h>
#endif


#ifdef __MWERKS__
asm float fabsf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vabs.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float fabsf_vfpu(float x)
{
    return fabsf(x);
}
#endif

/* vasin on x below sqrt(1/2), on sqrt(1 - x^2) above */
#ifdef __MWERKS__
asm float acosf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vcst.s S001, VFPU_SQRT1_2
    vcst.s S002, VFPU_PI_2
    vpfxs |X|, Y, Z, W
    vcmp.s LT, S000, S001
    vmul.s S003, S000, S000
    vsgn.s S001, S000
    bvtl 0, L_0881CDB0
    vasin.s S000, S000
    vocp.s S003, S003
    vsqrt.s S003, S003
    vasin.s S003, S003
    vocp.s S000, S003
    vmul.s S000, S000, S001
L_0881CDB0:
    vocp.s S000, S000
    vmul.s S000, S000, S002
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float acosf_vfpu(float x)
{
    return acosf(x);
}
#endif

/* vasin on x below sqrt(1/2), on sqrt(1 - x^2) above */
#ifdef __MWERKS__
asm float asinf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vcst.s S001, VFPU_SQRT1_2
    vcst.s S002, VFPU_PI_2
    vpfxs |X|, Y, Z, W
    vcmp.s LT, S000, S001
    vmul.s S003, S000, S000
    vsgn.s S001, S000
    bvtl 0, L_0881CE08
    vasin.s S000, S000
    vocp.s S003, S003
    vsqrt.s S003, S003
    vmul.s S002, S002, S001
    vasin.s S003, S003
    vocp.s S000, S003
L_0881CE08:
    vmul.s S000, S000, S002
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float asinf_vfpu(float x)
{
    return asinf(x);
}
#endif

/* asin(x / sqrt(1 + x^2)); NaN is returned as is */
#ifdef __MWERKS__
asm float atanf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vcmp.s EN, S000, S000
    vmul.s S001, S000, S000
    bvt 0, L_0881CE8C
    vone.s S002
    vpfxs |X|, Y, Z, W
    vcmp.s LT, S000, S002
    vadd.s S003, S001, S002
    vcst.s S002, VFPU_PI_2
    bvfl 0, L_0881CE60
    vrcp.s S003, S003
    vrsq.s S003, S003
    vmul.s S000, S000, S003
    b L_0881CE88
    vasin.s S000, S000
L_0881CE60:
    vmul.s S003, S001, S003
    vsgn.s S001, S000
    vcmp.s ES, S003, S003
    vsocp.s C012, S003
    vsqrt.s S003, S012
    vmul.s S002, S002, S001
    bvtl 0, L_0881CE88
    vone.s S000
    vasin.s S003, S003
    vocp.s S000, S003
L_0881CE88:
    vmul.s S000, S000, S002
L_0881CE8C:
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float atanf_vfpu(float x)
{
    return atanf(x);
}
#endif

/* quadrant fix-up of atan(y / x), with the zero/infinite cases of C99 */
#ifdef __MWERKS__
asm float atan2f_vfpu(float y, float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    swc1 $f13, 8(sp)
    lv.s S000, 4(sp)
    lv.s S001, 8(sp)
    vcmp.p ES, C000, C000
    vcst.s S002, VFPU_PI_2
    bvt 4, L_0881CF08
    vrcp.s S003, S001
    vmul.s S003, S000, S003
    vcmp.s EN, S003, S003
    bvt 0, L_0881CF80
    vmov.s S010, S003
    vcmp.s ES, S003, S003
    bvtl 0, L_0881CF50
    vsgn.s S010, S000
    vmul.s S011, S003, S003
    vone.s S003
    vpfxs |X|, Y, Z, W
    vcmp.s LT, S010, S003
    vadd.s S003, S011, S003
    bvfl 0, L_0881CF3C
    vrcp.s S003, S003
    vrsq.s S003, S003
    vmul.s S010, S010, S003
    b L_0881CF50
    vasin.s S010, S010
L_0881CF08:
    vfim.s S010, -1.0
    bvt 5, L_0881CF80
    vrsq.s S010, S010
    vcmp.p EN, C000, C000
    bvt 4, L_0881CF80
    vcmp.p EI, C000, C000
    vsgn.s S000, S000
    vcst.s S002, VFPU_PI_4
    bvt 0, L_0881CF74
    vone.s S010
    vsgn.s S011, S001
    b L_0881CF74
    vsub.s S010, S010, S011
L_0881CF3C:
    vmul.s S003, S011, S003
    vocp.s S003, S003
    vsqrt.s S003, S003
    vasin.s S003, S003
    vocp.s S010, S003
L_0881CF50:
    .word 0xDE000C04  /* vpfxd ,[0:1],m,m */
    vpfxs X, -Y, Z, W
    vsgn.p C000, C000
    vcmp.s EZ, S001, S001
    bvt 0, L_0881CF74
    nop
    vpfxs 2, Y, Z, W
    vpfxt |X|, Y, Z, W
    vsub.s S010, S010, S010
L_0881CF74:
    vpfxs |X|, Y, Z, W
    vmul.s S010, S010, S000
    vmul.s S010, S010, S002
L_0881CF80:
    sv.s S010, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float atan2f_vfpu(float y, float x)
{
    return atan2f(y, x);
}
#endif

/* vf2iu: rounds toward +infinity */
#ifdef __MWERKS__
asm float ceilf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vf2iu.s S000, S000, 0
    vi2f.s S000, S000, 0
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float ceilf_vfpu(float x)
{
    return ceilf(x);
}
#endif

#ifdef __MWERKS__
asm float cosf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vcst.s S001, VFPU_2_PI
    vmul.s S000, S000, S001
    vcos.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float cosf_vfpu(float x)
{
    return cosf(x);
}
#endif

/* exp2(x * log2(e)) */
#ifdef __MWERKS__
asm float expf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vcst.s S001, VFPU_LOG2E
    vmul.s S000, S000, S001
    vexp2.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float expf_vfpu(float x)
{
    return exp2f(x * 1.44269504f);
}
#endif

/* vf2id: rounds toward -infinity */
#ifdef __MWERKS__
asm float floorf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vf2id.s S000, S000, 0
    vi2f.s S000, S000, 0
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float floorf_vfpu(float x)
{
    return floorf(x);
}
#endif

/* log2(x) / log2(e) */
#ifdef __MWERKS__
asm float logf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vlog2.s S000, S000
    vcst.s S001, VFPU_LOG2E
    vdiv.s S000, S000, S001
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float logf_vfpu(float x)
{
    return log2f(x) / 1.44269504f;
}
#endif

#ifdef __MWERKS__
asm float log2f_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vlog2.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float log2f_vfpu(float x)
{
    return log2f(x);
}
#endif

/* log2(x) / log2(10) */
#ifdef __MWERKS__
asm float log10f_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vlog2.s S000, S000
    vcst.s S001, VFPU_LOG2TEN
    vdiv.s S000, S000, S001
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float log10f_vfpu(float x)
{
    return log2f(x) / 3.32192809f;
}
#endif

#ifdef __MWERKS__
asm float fmaxf_vfpu(float a, float b)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    swc1 $f13, 8(sp)
    lv.s S000, 4(sp)
    lv.s S001, 8(sp)
    vmax.s S000, S000, S001
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float fmaxf_vfpu(float a, float b)
{
    return a > b ? a : b;
}
#endif

#ifdef __MWERKS__
asm float fminf_vfpu(float a, float b)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    swc1 $f13, 8(sp)
    lv.s S000, 4(sp)
    lv.s S001, 8(sp)
    vmin.s S000, S000, S001
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float fminf_vfpu(float a, float b)
{
    return a < b ? a : b;
}
#endif

/* exp2(log2(x) * y): x must be positive */
#ifdef __MWERKS__
asm float powf_vfpu(float x, float y)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    swc1 $f13, 8(sp)
    lv.s S000, 4(sp)
    lv.s S001, 8(sp)
    vlog2.s S000, S000
    vmul.s S000, S000, S001
    vexp2.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float powf_vfpu(float x, float y)
{
    return exp2f(log2f(x) * y);
}
#endif

#ifdef __MWERKS__
asm float exp2f_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vexp2.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float exp2f_vfpu(float x)
{
    return exp2f(x);
}
#endif

/* vf2in: to nearest, ties to even (rintf, not roundf) */
#ifdef __MWERKS__
asm float roundf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vf2in.s S000, S000, 0
    vi2f.s S000, S000, 0
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float roundf_vfpu(float x)
{
    return rintf(x);
}
#endif

#ifdef __MWERKS__
asm float rsqrtf(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vrsq.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float rsqrtf(float x)
{
    return 1.0f / sqrtf(x);
}
#endif

#ifdef __MWERKS__
asm float sinf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vcst.s S001, VFPU_2_PI
    vmul.s S000, S000, S001
    vsin.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float sinf_vfpu(float x)
{
    return sinf(x);
}
#endif

#ifdef __MWERKS__
asm float sqrtf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vsqrt.s S000, S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float sqrtf_vfpu(float x)
{
    return sqrtf(x);
}
#endif

/* vrot gives cos and sin, then sin / cos */
#ifdef __MWERKS__
asm float tanf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vcst.s S001, VFPU_2_PI
    vmul.s S000, S000, S001
    vrot.p C002, S000, [c, s]
    vdiv.s S000, S003, S002
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float tanf_vfpu(float x)
{
    return sinf(x) / cosf(x);
}
#endif

/* vf2iz: rounds toward zero */
#ifdef __MWERKS__
asm float truncf_vfpu(float x)
{
    addiu sp, sp, -16
    swc1 $f12, 4(sp)
    lv.s S000, 4(sp)
    vf2iz.s S000, S000, 0
    vi2f.s S000, S000, 0
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float truncf_vfpu(float x)
{
    return truncf(x);
}
#endif
