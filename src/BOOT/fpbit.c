// @range 0x08817784-0x08817D70
// @gcc 0x08817784-0x08817978 fpbit_pkf
// @gcc 0x08817978-0x088179F4 fpbit_sf
// @gcc 0x088179F4-0x08817D70 fpbit_pkd
/* libgcc software floating point (fp-bit.c, the GCC 3/4 generation: QUIET_NAN 0x100000 for single,
   denormals supported) for the soft-float GCC code in BOOT: the single-precision objects (pack, unpack, make,
   float to double) and the double pack/unpack. Numbers are unpacked into class/sign/exponent/fraction with guard
   bits (8 for double, 7 for float) and packed back with round-to-nearest-even. On the PSP every function is
   GCC 3.3.3's fp-bit.c compiled by the SDK's GCC: pack/unpack in fpbit_pkf.c and fpbit_pkd.c, make and float to
   double in fpbit_sf.c, the rest of the double routines in fpbit_df.c (docs/build.md, "Trilha GCC"). Outside the
   PSP build the routines are the fp-bit C below. */

typedef unsigned long long UDItype;

enum { CLASS_SNAN, CLASS_QNAN, CLASS_ZERO, CLASS_NUMBER, CLASS_INFINITY };

typedef struct FpD {                   /* fp_number_type for double */
    int class;
    unsigned int sign;
    int normal_exp;
    unsigned int pad;                  /* fraction is 8-aligned on the PSP */
    UDItype fraction;                  /* +0x10, IMPLICIT_1 = 1 << 60 */
} FpD;

typedef struct FpF {                   /* fp_number_type for float */
    int class;
    unsigned int sign;
    int normal_exp;
    unsigned int fraction;             /* +0x0C, IMPLICIT_1 = 1 << 30 */
} FpF;

float __pack_f(FpF *src);
void __unpack_f(const float *src, FpF *dst);
double __pack_d(FpD *src);
void __unpack_d(const double *src, FpD *dst);
FpD *_fpadd_parts(FpD *a, FpD *b, FpD *tmp);
int __fpcmp_parts_d(FpD *a, FpD *b);
double __make_dp(int class, unsigned int sign, int exp, UDItype frac);
float __make_fp(int class, unsigned int sign, int exp, unsigned int frac);


#ifndef __MWERKS__ /* PSP: fpbit_pkf.c */
float __pack_f(FpF *src)
{
    unsigned int fraction = src->fraction;
    int exp = 0;
    union {
        unsigned int u;
        float f;
    } dst;

    if (src->class < CLASS_ZERO) {
        exp = 0xFF;
        fraction |= 0x100000;
    } else if (src->class == CLASS_INFINITY) {
        exp = 0xFF;
        fraction = 0;
    } else if (src->class == CLASS_ZERO) {
        fraction = 0;
    } else if (fraction == 0) {
    } else if (src->normal_exp < -126) {
        int shift = -126 - src->normal_exp;

        if (shift > 25) {
            fraction = 0;
        } else {
            unsigned int lowbit = (fraction & ((1u << shift) - 1)) ? 1 : 0;

            fraction = (fraction >> shift) | lowbit;
        }
        if ((fraction & 0x7F) == 0x40) {
            if (fraction & 0x80) {
                fraction += 0x40;
            }
        } else {
            fraction += 0x3F;
        }
        if (fraction >= (1u << 30)) {
            exp = 1;
        }
        fraction >>= 7;
    } else if (src->normal_exp > 127) {
        exp = 0xFF;
        fraction = 0;
    } else {
        exp = src->normal_exp + 127;
        if ((fraction & 0x7F) == 0x40) {
            if (fraction & 0x80) {
                fraction += 0x40;
            }
        } else {
            fraction += 0x3F;
        }
        if (fraction >= (1u << 31)) {
            fraction >>= 1;
            exp += 1;
        }
        fraction >>= 7;
    }
    dst.u = (fraction & 0x7FFFFF) | (unsigned int)(exp & 0xFF) << 23 | src->sign << 31;
    return dst.f;
}
#endif

#ifndef __MWERKS__ /* PSP: fpbit_pkf.c */
void __unpack_f(const float *src, FpF *dst)
{
    unsigned int bits = *(const unsigned int *)src;
    unsigned int fraction = bits & 0x7FFFFF;
    int exp = (bits & 0x7FFFFFFF) >> 23;

    dst->sign = bits >> 31;
    if (exp == 0) {
        if (fraction == 0) {
            dst->class = CLASS_ZERO;
        } else {
            dst->normal_exp = exp - 127 + 1;
            fraction <<= 7;
            dst->class = CLASS_NUMBER;
            while (fraction < (1u << 30)) {
                fraction <<= 1;
                dst->normal_exp--;
            }
            dst->fraction = fraction;
        }
    } else if (exp == 0xFF) {
        if (fraction == 0) {
            dst->class = CLASS_INFINITY;
        } else {
            dst->class = (fraction & 0x100000) ? CLASS_QNAN : CLASS_SNAN;
            dst->fraction = fraction;
        }
    } else {
        dst->normal_exp = exp - 127;
        dst->class = CLASS_NUMBER;
        dst->fraction = (fraction << 7) | (1u << 30);
    }
}
#endif

#ifndef __MWERKS__ /* PSP: fpbit_sf.c */
float __make_fp(int class, unsigned int sign, int exp, unsigned int frac)
{
    FpF in;

    in.class = class;
    in.sign = sign;
    in.normal_exp = exp;
    in.fraction = frac;
    return __pack_f(&in);
}
#endif

#ifndef __MWERKS__ /* PSP: fpbit_sf.c */
double __extendsfdf2(float a)
{
    return (double)a;
}
#endif

#ifndef __MWERKS__ /* PSP: fpbit_pkd.c */
double __pack_d(FpD *src)
{
    UDItype fraction = src->fraction;
    int exp = 0;
    union {
        UDItype u;
        double d;
    } dst;

    if (src->class < CLASS_ZERO) {
        exp = 0x7FF;
        fraction |= (UDItype)0x80000 << 32;
    } else if (src->class == CLASS_INFINITY) {
        exp = 0x7FF;
        fraction = 0;
    } else if (src->class == CLASS_ZERO) {
        fraction = 0;
    } else if (fraction == 0) {
    } else if (src->normal_exp < -1022) {
        int shift = -1022 - src->normal_exp;

        if (shift > 56) {
            fraction = 0;
        } else {
            unsigned int lowbit = (fraction & (((UDItype)1 << shift) - 1)) ? 1 : 0;

            fraction = (fraction >> shift) | lowbit;
        }
        if ((fraction & 0xFF) == 0x80) {
            if (fraction & 0x100) {
                fraction += 0x80;
            }
        } else {
            fraction += 0x7F;
        }
        if (fraction >= ((UDItype)1 << 60)) {
            exp = 1;
        }
        fraction >>= 8;
    } else if (src->normal_exp > 1023) {
        exp = 0x7FF;
        fraction = 0;
    } else {
        exp = src->normal_exp + 1023;
        if ((fraction & 0xFF) == 0x80) {
            if (fraction & 0x100) {
                fraction += 0x80;
            }
        } else {
            fraction += 0x7F;
        }
        if (fraction >= ((UDItype)1 << 61)) {
            fraction >>= 1;
            exp += 1;
        }
        fraction >>= 8;
    }
    dst.u = (fraction & (((UDItype)1 << 52) - 1)) | (UDItype)(exp & 0x7FF) << 52 | (UDItype)src->sign << 63;
    return dst.d;
}
#endif

#ifndef __MWERKS__ /* PSP: fpbit_pkd.c */
void __unpack_d(const double *src, FpD *dst)
{
    UDItype bits = *(const UDItype *)src;
    UDItype fraction = bits & (((UDItype)1 << 52) - 1);
    int exp = (int)((bits >> 52) & 0x7FF);

    dst->sign = (unsigned int)(bits >> 63);
    if (exp == 0) {
        if (fraction == 0) {
            dst->class = CLASS_ZERO;
        } else {
            dst->normal_exp = exp - 1023 + 1;
            fraction <<= 8;
            dst->class = CLASS_NUMBER;
            while (fraction < ((UDItype)1 << 60)) {
                fraction <<= 1;
                dst->normal_exp--;
            }
            dst->fraction = fraction;
        }
    } else if (exp == 0x7FF) {
        if (fraction == 0) {
            dst->class = CLASS_INFINITY;
        } else {
            dst->class = (fraction & ((UDItype)0x80000 << 32)) ? CLASS_QNAN : CLASS_SNAN;
            dst->fraction = fraction;
        }
    } else {
        dst->normal_exp = exp - 1023;
        dst->class = CLASS_NUMBER;
        dst->fraction = (fraction << 8) | ((UDItype)1 << 60);
    }
}
#endif
