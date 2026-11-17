// @range 0x08817D70-0x08818E64
// @rodata 0x088EB210-0x088EB218
// @cc gcc
/* libgcc software floating point, double precision (GCC 3.3.3's fp-bit.c built as dp-bit.c: add/sub, multiply,
   divide, comparisons, int <-> double, negate, make, double -> float) for the soft-float GCC code in BOOT. On the
   PSP this is the GCC source itself (include/gcc-3.3.3/fp-bit.c), compiled by the SDK's GCC (tools/psp_gcc.sh;
   docs/build.md, "Trilha GCC"), and every function matches byte for byte. libgcc builds one object per function
   (FINE_GRAINED_LIBRARIES + L_<name>); here they share a translation unit, and __fpcmp_parts_d has to come after
   the comparison functions: GCC lays their call out differently once it has seen the callee's body. pack_d and
   unpack_d are fpbit_pkd.c. Outside the PSP build the internal routines are the fp-bit C below and the arithmetic
   is the host's (same IEEE results). */

#ifdef __mips__
#define FINE_GRAINED_LIBRARIES
#define FLOAT_BIT_ORDER_MISMATCH       /* t-mips: little-endian bitfield order */
#define __thenan_df D_08874590         /* the shared NaN of _thenan_df.o, in BOOT's .data */
#define L_addsub_df
#define L_mul_df
#define L_div_df
#define L_eq_df
#define L_ne_df
#define L_gt_df
#define L_ge_df
#define L_lt_df
#define L_le_df
#define L_si_to_df
#define L_df_to_si
#define L_negate_df
#define L_make_df
#define L_df_to_sf
#include "gcc-3.3.3/fp-bit.c"
#undef L_addsub_df
#undef L_mul_df
#undef L_div_df
#undef L_eq_df
#undef L_ne_df
#undef L_gt_df
#undef L_ge_df
#undef L_lt_df
#undef L_le_df
#undef L_si_to_df
#undef L_df_to_si
#undef L_negate_df
#undef L_make_df
#undef L_df_to_sf
/* second pass: only __fpcmp_parts_d; the static inline helpers of fp-bit.c get a second name */
#define L_fpcmp_parts_df
#define nan nan_2
#define isnan isnan_2
#define isinf isinf_2
#define iszero iszero_2
#define flip_sign flip_sign_2
#include "gcc-3.3.3/fp-bit.c"
#else
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

double __pack_d(FpD *src);
void __unpack_d(const double *src, FpD *dst);
FpD *_fpadd_parts(FpD *a, FpD *b, FpD *tmp);
int __fpcmp_parts_d(FpD *a, FpD *b);
double __make_dp(int class, unsigned int sign, int exp, UDItype frac);

static FpD thenan_df = { CLASS_SNAN, 0, 0, 0, 0 };
#define THENAN (&thenan_df)
#define D_IMPLICIT_1 ((UDItype)1 << 60)
#define D_IMPLICIT_2 ((UDItype)1 << 61)

/* (x >> s) with the shifted-out bits ORed into the lowest bit */
#define LSHIFT(x, s) ((x) = ((x) >> (s)) | (((x) & (((UDItype)1 << (s)) - 1)) != 0))

FpD *_fpadd_parts(FpD *a, FpD *b, FpD *tmp)
{
    long long tfraction;
    int a_normal_exp;
    int b_normal_exp;
    UDItype a_fraction;
    UDItype b_fraction;

    if (a->class < CLASS_ZERO) {
        return a;
    }
    if (b->class < CLASS_ZERO) {
        return b;
    }
    if (a->class == CLASS_INFINITY) {
        if (b->class == CLASS_INFINITY && a->sign != b->sign) {
            return THENAN;
        }
        return a;
    }
    if (b->class == CLASS_INFINITY) {
        return b;
    }
    if (b->class == CLASS_ZERO) {
        if (a->class == CLASS_ZERO) {
            *tmp = *a;
            tmp->sign = a->sign & b->sign;
            return tmp;
        }
        return a;
    }
    if (a->class == CLASS_ZERO) {
        return b;
    }
    {
        int diff;
        int sdiff;

        a_normal_exp = a->normal_exp;
        b_normal_exp = b->normal_exp;
        a_fraction = a->fraction;
        b_fraction = b->fraction;
        diff = a_normal_exp - b_normal_exp;
        sdiff = diff;
        if (diff < 0) {
            diff = -diff;
        }
        if (diff < 64) {
            if (sdiff > 0) {
                b_normal_exp += diff;
                LSHIFT(b_fraction, diff);
            } else if (sdiff < 0) {
                a_normal_exp += diff;
                LSHIFT(a_fraction, diff);
            }
        } else if (a_normal_exp > b_normal_exp) {
            b_normal_exp = a_normal_exp;
            b_fraction = 0;
        } else {
            a_normal_exp = b_normal_exp;
            a_fraction = 0;
        }
    }
    if (a->sign != b->sign) {
        if (a->sign) {
            tfraction = -(long long)a_fraction + (long long)b_fraction;
        } else {
            tfraction = (long long)a_fraction - (long long)b_fraction;
        }
        if (tfraction >= 0) {
            tmp->sign = 0;
            tmp->normal_exp = a_normal_exp;
            tmp->fraction = tfraction;
        } else {
            tmp->sign = 1;
            tmp->normal_exp = a_normal_exp;
            tmp->fraction = -tfraction;
        }
        while (tmp->fraction < D_IMPLICIT_1 && tmp->fraction) {
            tmp->fraction <<= 1;
            tmp->normal_exp--;
        }
    } else {
        tmp->sign = a->sign;
        tmp->normal_exp = a_normal_exp;
        tmp->fraction = a_fraction + b_fraction;
    }
    tmp->class = CLASS_NUMBER;
    if (tmp->fraction >= D_IMPLICIT_2) {
        LSHIFT(tmp->fraction, 1);
        tmp->normal_exp++;
    }
    return tmp;
}

double __adddf3(double a, double b)
{
    return a + b;
}

double __subdf3(double a, double b)
{
    return a - b;
}

double __muldf3(double a, double b)
{
    return a * b;
}

double __divdf3(double a, double b)
{
    return a / b;
}

/* -1, 0 or 1 as a <, =, > b (1 when either is a NaN, which the callers filter out first) */
int __fpcmp_parts_d(FpD *a, FpD *b)
{
    if (a->class < CLASS_ZERO || b->class < CLASS_ZERO) {
        return 1;
    }
    if (a->class == CLASS_INFINITY && b->class == CLASS_INFINITY) {
        return (int)b->sign - (int)a->sign;
    }
    if (a->class == CLASS_INFINITY) {
        return a->sign ? -1 : 1;
    }
    if (b->class == CLASS_INFINITY) {
        return b->sign ? 1 : -1;
    }
    if (a->class == CLASS_ZERO && b->class == CLASS_ZERO) {
        return 0;
    }
    if (a->class == CLASS_ZERO) {
        return b->sign ? 1 : -1;
    }
    if (b->class == CLASS_ZERO) {
        return a->sign ? -1 : 1;
    }
    if (a->sign != b->sign) {
        return a->sign ? -1 : 1;
    }
    if (a->normal_exp > b->normal_exp) {
        return a->sign ? -1 : 1;
    }
    if (a->normal_exp < b->normal_exp) {
        return a->sign ? 1 : -1;
    }
    if (a->fraction > b->fraction) {
        return a->sign ? -1 : 1;
    }
    if (a->fraction < b->fraction) {
        return a->sign ? 1 : -1;
    }
    return 0;
}

/* the comparison wrappers: `unordered` is what they return when an operand is a NaN */
static int fpcmp(double a, double b, int unordered)
{
    FpD pa;
    FpD pb;

    __unpack_d(&a, &pa);
    __unpack_d(&b, &pb);
    if (pa.class < CLASS_ZERO || pb.class < CLASS_ZERO) {
        return unordered;
    }
    return __fpcmp_parts_d(&pa, &pb);
}

/* 0 when equal */
int __eqdf2(double a, double b)
{
    return fpcmp(a, b, 1);
}

/* nonzero when different */
int __nedf2(double a, double b)
{
    return fpcmp(a, b, 1);
}

/* > 0 when a > b */
int __gtdf2(double a, double b)
{
    return fpcmp(a, b, -1);
}

/* >= 0 when a >= b */
int __gedf2(double a, double b)
{
    return fpcmp(a, b, -1);
}

/* < 0 when a < b */
int __ltdf2(double a, double b)
{
    return fpcmp(a, b, 1);
}

/* <= 0 when a <= b */
int __ledf2(double a, double b)
{
    return fpcmp(a, b, 1);
}

double __floatsidf(int i)
{
    return (double)i;
}

/* truncates toward zero; NaN gives 0, out of range saturates */
int __fixdfsi(double a)
{
    FpD p;
    unsigned int tmp;

    __unpack_d(&a, &p);
    if (p.class == CLASS_ZERO || p.class < CLASS_ZERO) {
        return 0;
    }
    if (p.class != CLASS_INFINITY) {
        if (p.normal_exp < 0) {
            return 0;
        }
        if (p.normal_exp < 31) {
            tmp = (unsigned int)(p.fraction >> (60 - p.normal_exp));
            return p.sign ? -(int)tmp : (int)tmp;
        }
    }
    return p.sign ? (int)0x80000000 : 0x7FFFFFFF;
}

double __negdf2(double a)
{
    return -a;
}

double __make_dp(int class, unsigned int sign, int exp, UDItype frac)
{
    FpD in;

    in.class = class;
    in.sign = sign;
    in.normal_exp = exp;
    in.fraction = frac;
    return __pack_d(&in);
}

float __truncdfsf2(double a)
{
    return (float)a;
}
#endif
