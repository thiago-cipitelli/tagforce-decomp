// @range 0x08817784-0x08818E64
// @asm 0x08817D70-0x088180B8
// @asm 0x08818194-0x08818730
// @asm 0x08818BC4-0x08818CA0
/* libgcc software floating point (fp-bit.c, the GCC 3/4 generation: QUIET_NAN 0x100000 for single,
   denormals supported) for the soft-float GCC code in BOOT. Numbers are unpacked into
   class/sign/exponent/fraction with guard bits (8 for double, 7 for float), operated on and packed
   back with round-to-nearest-even. Whole `asm` functions; _fpadd_parts, __muldf3, __divdf3 (the
   shared NaN __thenan_df, D_08874590) and __floatsidf (the constant -2^31, D_088EB210) load data
   through split lui/lo pairs and stay in asm with their C under NON_MATCHING. Outside the PSP build
   the internal routines are the fp-bit C and the arithmetic is the host's (same IEEE results). */

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

#if !defined(__MWERKS__) || defined(NON_MATCHING)
#ifdef __MWERKS__
extern FpD D_08874590;                 /* __thenan_df */
#define THENAN (&D_08874590)
#else
static FpD thenan_df = { CLASS_SNAN, 0, 0, 0, 0 };
#define THENAN (&thenan_df)
#endif
#define D_IMPLICIT_1 ((UDItype)1 << 60)
#define D_IMPLICIT_2 ((UDItype)1 << 61)
#endif

#ifdef __MWERKS__
asm float __pack_f(FpF *src)
{
    lw v1, 0(a0)
    move a2, zero
    lw a1, 12(a0)
    sltiu v0, v1, 2
    lw t0, 4(a0)
    beqz v0, L_088177C4
    move a3, zero
    lui v0, 0x10
    or a1, a1, v0
    addiu a3, zero, 255
L_088177AC:
    ins a2, a1, 0x0, 0x17
L_088177B0:
    ins a2, a3, 0x17, 0x8
    ins a2, t0, 0x1f, 0x1
    mtc1 a2, $f0
    jr ra
    nop
L_088177C4:
    xori v0, v1, 0x4
    beqzl v0, L_08817874
    addiu a3, zero, 255
    xori v0, v1, 0x2
    beqzl v0, L_088177AC
    move a1, zero
    beqzl a1, L_088177B0
    ins a2, a1, 0x0, 0x17
    lw a0, 8(a0)
    slti v0, a0, -126
    beqz v0, L_08817868
    slti v0, a0, 128
    addiu v0, zero, -126
    subu a0, v0, a0
    slti v1, a0, 26
    bnez v1, L_0881784C
    addiu v0, zero, 1
    move a1, zero
L_0881780C:
    andi v1, a1, 0x7f
    addiu v0, zero, 64
    beql v1, v0, L_0881783C
    andi v0, a1, 0x80
    addiu a1, a1, 63
L_08817820:
    lui v0, 0x3fff
L_08817824:
    ori v0, v0, 0xffff
    sltu v0, v0, a1
    addiu v1, zero, 1
    movn a3, v1, v0
L_08817834:
    j L_088177AC
    srl a1, a1, 0x7
L_0881783C:
    bnezl v0, L_08817820
    addiu a1, a1, 64
    j L_08817824
    lui v0, 0x3fff
L_0881784C:
    sllv v0, v0, a0
    addiu v0, v0, -1
    and v0, a1, v0
    srlv v1, a1, a0
    sltu v0, zero, v0
    j L_0881780C
    or a1, v1, v0
L_08817868:
    bnezl v0, L_0881787C
    andi v1, a1, 0x7f
    addiu a3, zero, 255
L_08817874:
    j L_088177AC
    move a1, zero
L_0881787C:
    addiu v0, zero, 64
    beq v1, v0, L_088178A0
    addiu a3, a0, 127
    addiu a1, a1, 63
L_0881788C:
    bgezl a1, L_088177AC
    srl a1, a1, 0x7
    srl a1, a1, 0x1
    j L_08817834
    addiu a3, a3, 1
L_088178A0:
    andi v0, a1, 0x80
    bnezl v0, L_0881788C
    addiu a1, a1, 64
    j L_0881788C
    nop
}
#else
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

#ifdef __MWERKS__
asm void __unpack_f(const float *src, FpF *dst)
{
    lw v0, 0(a0)
    srl v1, v0, 0x1f
    move a2, v0
    ext a0, v0, 0x17, 0x8
    sw v1, 4(a1)
    bnez a0, L_08817928
    ins a2, zero, 0x17, 0x9
    beqz a2, L_08817920
    addiu v0, zero, 2
    lui v0, 0x3fff
    sll a2, a2, 0x7
    ori v0, v0, 0xffff
    addiu v1, zero, -126
    addiu a0, zero, 3
    sltu v0, v0, a2
    sw v1, 8(a1)
    bnez v0, L_08817918
    sw a0, 0(a1)
    lui a0, 0x3fff
    ori a0, a0, 0xffff
L_08817904:
    sll a2, a2, 0x1
    sltu v0, a0, a2
    beqz v0, L_08817904
    addiu v1, v1, -1
    sw v1, 8(a1)
L_08817918:
    jr ra
    sw a2, 12(a1)
L_08817920:
    jr ra
    sw v0, 0(a1)
L_08817928:
    addiu v0, zero, 255
    beq a0, v0, L_08817950
    sll v0, a2, 0x7
    lui v1, 0x4000
    or v0, v0, v1
    addiu a0, a0, -127
    sw v0, 12(a1)
    addiu v0, zero, 3
    j L_08817920
    sw a0, 8(a1)
L_08817950:
    bnezl a2, L_08817960
    lui v0, 0x10
    j L_08817920
    addiu v0, zero, 4
L_08817960:
    and v0, a2, v0
    beqzl v0, L_08817918
    sw zero, 0(a1)
    addiu v0, zero, 1
    j L_08817918
    sw v0, 0(a1)
}
#else
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

#ifdef __MWERKS__
asm float __make_fp(int class, unsigned int sign, int exp, unsigned int frac)
{
    addiu sp, sp, -32
    move v0, a0
    move a0, sp
    sw ra, 16(sp)
    sw v0, 0(sp)
    sw a1, 4(sp)
    sw a2, 8(sp)
    jal __pack_f
    sw a3, 12(sp)
    lw ra, 16(sp)
    jr ra
    addiu sp, sp, 32
}
#else
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

#ifdef __MWERKS__
asm double __extendsfdf2(float a)
{
    addiu sp, sp, -48
    addiu a0, sp, 16
    move a1, sp
    sw ra, 32(sp)
    jal __unpack_f
    swc1 $f12, 16(sp)
    move t1, zero
    lw t0, 12(sp)
    lw a0, 0(sp)
    lw a1, 4(sp)
    lw a2, 8(sp)
    srl v0, t0, 0x2
    sll t1, t1, 0x1e
    or t1, t1, v0
    jal __make_dp
    sll t0, t0, 0x1e
    lw ra, 32(sp)
    jr ra
    addiu sp, sp, 48
}
#else
double __extendsfdf2(float a)
{
    return (double)a;
}
#endif

#ifdef __MWERKS__
asm double __pack_d(FpD *src)
{
    lw v1, 0(a0)
    addiu sp, sp, -16
    lw t2, 16(a0)
    lw t3, 20(a0)
    sltiu v0, v1, 2
    lw t4, 4(a0)
    beqz v0, L_08817A50
    move t1, zero
    addiu v0, zero, 0
    lui v1, 0x8
    or t2, t2, v0
    or t3, t3, v1
    addiu t1, zero, 2047
L_08817A28:
    lw v0, 4(sp)
L_08817A2C:
    ins v0, t3, 0x0, 0x14
    ins v0, t1, 0x14, 0xb
    ins v0, t4, 0x1f, 0x1
    sw v0, 4(sp)
    sw t2, 0(sp)
    lw v0, 0(sp)
    lw v1, 4(sp)
    jr ra
    addiu sp, sp, 16
L_08817A50:
    xori v0, v1, 0x4
    beqzl v0, L_08817BBC
    addiu t1, zero, 2047
    xori v0, v1, 0x2
    beqz v0, L_08817BBC
    nop
    or v0, t2, t3
    beqz v0, L_08817A2C
    lw v0, 4(sp)
    lw a0, 8(a0)
    slti v0, a0, -1022
    beqz v0, L_08817BB4
    slti v0, a0, 1024
    addiu v0, zero, -1022
    subu t5, v0, a0
    slti v1, t5, 57
    bnez v1, L_08817B1C
    nop
    move t2, zero
    move t3, zero
L_08817AA0:
    addiu v1, zero, 0
    addiu v0, zero, 255
    and v0, t2, v0
    addiu a0, zero, 128
    beq v0, a0, L_08817AEC
    and v1, t3, v1
    addiu t2, t2, 127
L_08817ABC:
    sltiu v0, t2, 127
L_08817AC0:
    addu t3, t3, v0
    lui v0, 0xfff
L_08817AC8:
    ori v0, v0, 0xffff
    sltu v0, v0, t3
    addiu v1, zero, 1
    srl t2, t2, 0x8
    sll a0, t3, 0x18
    movn t1, v1, v0
    or t2, t2, a0
L_08817AE4:
    j L_08817A28
    srl t3, t3, 0x8
L_08817AEC:
    bnezl v1, L_08817ABC
    addiu t2, t2, 127
    addiu v1, zero, 0
    addiu v0, zero, 256
    and v0, t2, v0
    and v1, t3, v1
    or v0, v0, v1
    beqzl v0, L_08817AC8
    lui v0, 0xfff
    addiu t2, t2, 128
    j L_08817AC0
    sltiu v0, t2, 128
L_08817B1C:
    addiu a1, zero, 0
    addiu a0, zero, 1
    sll a2, t5, 0x1a
    bgez a2, L_08817B3C
    nop
    sllv v1, a0, t5
    b L_08817B54
    move v0, zero
L_08817B3C:
    beqz a2, L_08817B50
    sllv v1, a1, t5
    negu a2, t5
    srlv a2, a0, a2
    or v1, v1, a2
L_08817B50:
    sllv v0, a0, t5
L_08817B54:
    addiu v0, v0, -1
    sltiu a3, v0, -1
    addiu v1, v1, -1
    addu v1, v1, a3
    and v0, t2, v0
    and v1, t3, v1
    or v0, v0, v1
    sll t0, t5, 0x1a
    bgez t0, L_08817B88
    nop
    srlv a2, t3, t5
    b L_08817BA0
    move a3, zero
L_08817B88:
    beqz t0, L_08817B9C
    srlv a2, t2, t5
    negu t0, t5
    sllv t0, t3, t0
    or a2, a2, t0
L_08817B9C:
    srlv a3, t3, t5
L_08817BA0:
    sltu v0, zero, v0
    move a1, zero
    or t2, a2, v0
    j L_08817AA0
    or t3, a3, a1
L_08817BB4:
    bnez v0, L_08817BCC
    addiu t1, zero, 2047
L_08817BBC:
    move t2, zero
    move t3, zero
    j L_08817A2C
    lw v0, 4(sp)
L_08817BCC:
    addiu v1, zero, 0
    addiu v0, zero, 255
    and a2, t2, v0
    addiu v0, zero, 128
    and a3, t3, v1
    beq a2, v0, L_08817C2C
    addiu t1, a0, 1023
    addiu t2, t2, 127
L_08817BEC:
    sltiu v0, t2, 127
L_08817BF0:
    addu t3, t3, v0
    lui v0, 0x1fff
L_08817BF8:
    ori v0, v0, 0xffff
    sltu v0, v0, t3
    beqzl v0, L_08817C20
    srl t2, t2, 0x8
    sll v0, t3, 0x1f
    srl t2, t2, 0x1
    or t2, t2, v0
    srl t3, t3, 0x1
    addiu t1, t1, 1
    srl t2, t2, 0x8
L_08817C20:
    sll v0, t3, 0x18
    j L_08817AE4
    or t2, t2, v0
L_08817C2C:
    bnezl a3, L_08817BEC
    addiu t2, t2, 127
    addiu v1, zero, 0
    addiu v0, zero, 256
    and v0, t2, v0
    and v1, t3, v1
    or v0, v0, v1
    beqzl v0, L_08817BF8
    lui v0, 0x1fff
    addiu t2, t2, 128
    j L_08817BF0
    sltiu v0, t2, 128
}
#else
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

#ifdef __MWERKS__
asm void __unpack_d(const double *src, FpD *dst)
{
    lw v0, 4(a0)
    lw t0, 0(a0)
    move t2, a1
    srl v1, v0, 0x1f
    move t1, v0
    ext a3, v0, 0x14, 0xb
    sw v1, 4(a1)
    bnez a3, L_08817CFC
    ins t1, zero, 0x14, 0xc
    or v0, t0, t1
    bnez v0, L_08817C98
    srl v0, t0, 0x18
    addiu v0, zero, 2
L_08817C90:
    jr ra
    sw v0, 0(t2)
L_08817C98:
    lui a0, 0xfff
    sll t1, t1, 0x8
    or t1, t1, v0
    ori a0, a0, 0xffff
    addiu v0, zero, -1022
    addiu v1, zero, 3
    sltu a0, a0, t1
    sll t0, t0, 0x8
    sw v0, 8(a1)
    bnez a0, L_08817CF0
    sw v1, 0(a1)
    lui a1, 0xfff
    ori a1, a1, 0xffff
    addiu a0, zero, -1022
L_08817CD0:
    srl v1, t0, 0x1f
    sll t1, t1, 0x1
    or t1, t1, v1
    sltu v0, a1, t1
    sll t0, t0, 0x1
    beqz v0, L_08817CD0
    addiu a0, a0, -1
    sw a0, 8(t2)
L_08817CF0:
    sw t0, 16(t2)
    jr ra
    sw t1, 20(t2)
L_08817CFC:
    addiu v0, zero, 2047
    beq a3, v0, L_08817D3C
    srl a2, t0, 0x18
    sll v1, t1, 0x8
    or v1, v1, a2
    addiu a0, zero, 0
    lui a1, 0x1000
    sll v0, t0, 0x8
    or v0, v0, a0
    or v1, v1, a1
    addiu a2, a3, -1023
    sw v0, 16(t2)
    sw v1, 20(t2)
    addiu v0, zero, 3
    j L_08817C90
    sw a2, 8(t2)
L_08817D3C:
    or v0, t0, t1
    beqz v0, L_08817C90
    addiu v0, zero, 4
    addiu v0, zero, 0
    lui v1, 0x8
    and v0, t0, v0
    and v1, t1, v1
    or v0, v0, v1
    beqzl v0, L_08817CF0
    sw zero, 0(a1)
    addiu v0, zero, 1
    j L_08817CF0
    sw v0, 0(a1)
}
#else
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

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: __thenan_df through a split lui/addiu */
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
#endif

#ifdef __MWERKS__
asm double __adddf3(double a, double b)
{
    addiu sp, sp, -144
    move v1, a1
    move v0, a0
    sw s0, 128(sp)
    addiu a0, sp, 96
    addiu s0, sp, 32
    sw ra, 132(sp)
    sw a2, 112(sp)
    sw a3, 116(sp)
    sw v0, 96(sp)
    sw v1, 100(sp)
    jal __unpack_d
    move a1, sp
    addiu a0, sp, 112
    jal __unpack_d
    move a1, s0
    move a1, s0
    addiu a2, sp, 64
    jal _fpadd_parts
    move a0, sp
    jal __pack_d
    move a0, v0
    lw ra, 132(sp)
    lw s0, 128(sp)
    jr ra
    addiu sp, sp, 144
}
#else
double __adddf3(double a, double b)
{
    return a + b;
}
#endif

#ifdef __MWERKS__
asm double __subdf3(double a, double b)
{
    addiu sp, sp, -144
    move v1, a1
    move v0, a0
    sw s0, 128(sp)
    addiu a0, sp, 96
    addiu s0, sp, 32
    sw ra, 132(sp)
    sw v0, 96(sp)
    sw v1, 100(sp)
    sw a2, 112(sp)
    sw a3, 116(sp)
    jal __unpack_d
    move a1, sp
    addiu a0, sp, 112
    jal __unpack_d
    move a1, s0
    lw v0, 36(sp)
    move a1, s0
    addiu a2, sp, 64
    move a0, sp
    xori v0, v0, 0x1
    jal _fpadd_parts
    sw v0, 36(sp)
    jal __pack_d
    move a0, v0
    lw ra, 132(sp)
    lw s0, 128(sp)
    jr ra
    addiu sp, sp, 144
}
#else
double __subdf3(double a, double b)
{
    return a - b;
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: __thenan_df through a split lui/addiu */
double __muldf3(double a, double b)
{
    return a * b;
}

double __divdf3(double a, double b)
{
    return a / b;
}
#endif

/* -1, 0 or 1 as a <, =, > b (1 when either is a NaN, which the callers filter out first) */
#ifdef __MWERKS__
asm int __fpcmp_parts_d(FpD *a, FpD *b)
{
    lw v1, 0(a0)
    sltiu v0, v1, 2
    bnez v0, L_08818750
    move a3, a0
    lw a2, 0(a1)
    sltiu v0, a2, 2
    beqzl v0, L_0881875C
    xori v0, v1, 0x4
L_08818750:
    addiu a0, zero, 1
L_08818754:
    jr ra
    move v0, a0
L_0881875C:
    bnez v0, L_08818780
    xori v0, v1, 0x4
    xori v0, a2, 0x4
    bnezl v0, L_08818780
    xori v0, v1, 0x4
    lw v1, 4(a1)
    lw v0, 4(a0)
    j L_08818754
    subu a0, v1, v0
L_08818780:
    beqzl v0, L_088187D4
    lw v1, 4(a3)
    xori v0, a2, 0x4
    beqzl v0, L_088187B8
    lw v1, 4(a1)
    xori v0, v1, 0x2
    bnez v0, L_088187C8
    xori v0, a2, 0x2
    beqz v0, L_08818754
    move a0, zero
    xori v0, v1, 0x2
    bnezl v0, L_088187C8
    xori v0, a2, 0x2
    lw v1, 4(a1)
L_088187B8:
    addiu a0, zero, 1
    addiu v0, zero, -1
L_088187C0:
    j L_08818754
    movz a0, v0, v1
L_088187C8:
    bnezl v0, L_088187E0
    lw a2, 4(a3)
    lw v1, 4(a3)
L_088187D4:
    addiu a0, zero, -1
    j L_088187C0
    addiu v0, zero, 1
L_088187E0:
    lw v1, 4(a1)
    addiu a0, zero, -1
    addiu v0, zero, 1
    bne a2, v1, L_08818754
    movz a0, v0, a2
    lw t1, 8(a3)
    lw t0, 8(a1)
    addiu a0, zero, -1
    slt v1, t0, t1
    bnez v1, L_08818754
    movz a0, v0, a2
    addiu a0, zero, 1
    addiu v0, zero, -1
    slt v1, t1, t0
    bnez v1, L_08818754
    movz a0, v0, a2
    lw t0, 20(a3)
    lw a0, 20(a1)
    sltu v0, a0, t0
    bnezl v0, L_0881888C
    addiu a0, zero, -1
    beql t0, a0, L_08818878
    lw v1, 16(a3)
    sltu v0, t0, a0
L_08818840:
    bnezl v0, L_0881886C
    addiu a0, zero, 1
    beql a0, t0, L_08818858
    lw v1, 16(a1)
    j L_08818754
    move a0, zero
L_08818858:
    lw v0, 16(a3)
    sltu v0, v0, v1
    beqzl v0, L_08818754
    move a0, zero
    addiu a0, zero, 1
L_0881886C:
    addiu v0, zero, -1
L_08818870:
    j L_08818754
    movz a0, v0, a2
L_08818878:
    lw v0, 16(a1)
    sltu v0, v0, v1
    beqzl v0, L_08818840
    sltu v0, t0, a0
    addiu a0, zero, -1
L_0881888C:
    j L_08818870
    addiu v0, zero, 1
}
#else
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
#endif

#ifndef __MWERKS__
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
#endif

/* 0 when equal */
#ifdef __MWERKS__
asm int __eqdf2(double a, double b)
{
    addiu sp, sp, -112
    move v0, a0
    move v1, a1
    sw s0, 96(sp)
    addiu a0, sp, 64
    move a1, sp
    sw v0, 64(sp)
    sw v1, 68(sp)
    sw ra, 100(sp)
    sw a2, 80(sp)
    sw a3, 84(sp)
    jal __unpack_d
    addiu s0, sp, 32
    addiu a0, sp, 80
    jal __unpack_d
    move a1, s0
    lw v0, 0(sp)
    sltiu v0, v0, 2
    bnez v0, L_088188F8
    addiu v1, zero, 1
    lw v0, 32(sp)
    move a1, s0
    sltiu v0, v0, 2
    beqz v0, L_0881890C
    move a0, sp
L_088188F8:
    lw ra, 100(sp)
    lw s0, 96(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 112
L_0881890C:
    jal __fpcmp_parts_d
    nop
    j L_088188F8
    move v1, v0
}
#else
int __eqdf2(double a, double b)
{
    return fpcmp(a, b, 1);
}
#endif

/* nonzero when different */
#ifdef __MWERKS__
asm int __nedf2(double a, double b)
{
    addiu sp, sp, -112
    move v0, a0
    move v1, a1
    sw s0, 96(sp)
    addiu a0, sp, 64
    move a1, sp
    sw v0, 64(sp)
    sw v1, 68(sp)
    sw ra, 100(sp)
    sw a2, 80(sp)
    sw a3, 84(sp)
    jal __unpack_d
    addiu s0, sp, 32
    addiu a0, sp, 80
    jal __unpack_d
    move a1, s0
    lw v0, 0(sp)
    sltiu v0, v0, 2
    bnez v0, L_08818980
    addiu v1, zero, 1
    lw v0, 32(sp)
    move a1, s0
    sltiu v0, v0, 2
    beqz v0, L_08818994
    move a0, sp
L_08818980:
    lw ra, 100(sp)
    lw s0, 96(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 112
L_08818994:
    jal __fpcmp_parts_d
    nop
    j L_08818980
    move v1, v0
}
#else
int __nedf2(double a, double b)
{
    return fpcmp(a, b, 1);
}
#endif

/* > 0 when a > b */
#ifdef __MWERKS__
asm int __gtdf2(double a, double b)
{
    addiu sp, sp, -112
    move v0, a0
    move v1, a1
    sw s0, 96(sp)
    addiu a0, sp, 64
    move a1, sp
    sw v0, 64(sp)
    sw v1, 68(sp)
    sw ra, 100(sp)
    sw a2, 80(sp)
    sw a3, 84(sp)
    jal __unpack_d
    addiu s0, sp, 32
    addiu a0, sp, 80
    jal __unpack_d
    move a1, s0
    lw v0, 0(sp)
    sltiu v0, v0, 2
    bnez v0, L_08818A08
    addiu v1, zero, -1
    lw v0, 32(sp)
    move a1, s0
    sltiu v0, v0, 2
    beqz v0, L_08818A1C
    move a0, sp
L_08818A08:
    lw ra, 100(sp)
    lw s0, 96(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 112
L_08818A1C:
    jal __fpcmp_parts_d
    nop
    j L_08818A08
    move v1, v0
}
#else
int __gtdf2(double a, double b)
{
    return fpcmp(a, b, -1);
}
#endif

/* >= 0 when a >= b */
#ifdef __MWERKS__
asm int __gedf2(double a, double b)
{
    addiu sp, sp, -112
    move v0, a0
    move v1, a1
    sw s0, 96(sp)
    addiu a0, sp, 64
    move a1, sp
    sw v0, 64(sp)
    sw v1, 68(sp)
    sw ra, 100(sp)
    sw a2, 80(sp)
    sw a3, 84(sp)
    jal __unpack_d
    addiu s0, sp, 32
    addiu a0, sp, 80
    jal __unpack_d
    move a1, s0
    lw v0, 0(sp)
    sltiu v0, v0, 2
    bnez v0, L_08818A90
    addiu v1, zero, -1
    lw v0, 32(sp)
    move a1, s0
    sltiu v0, v0, 2
    beqz v0, L_08818AA4
    move a0, sp
L_08818A90:
    lw ra, 100(sp)
    lw s0, 96(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 112
L_08818AA4:
    jal __fpcmp_parts_d
    nop
    j L_08818A90
    move v1, v0
}
#else
int __gedf2(double a, double b)
{
    return fpcmp(a, b, -1);
}
#endif

/* < 0 when a < b */
#ifdef __MWERKS__
asm int __ltdf2(double a, double b)
{
    addiu sp, sp, -112
    move v0, a0
    move v1, a1
    sw s0, 96(sp)
    addiu a0, sp, 64
    move a1, sp
    sw v0, 64(sp)
    sw v1, 68(sp)
    sw ra, 100(sp)
    sw a2, 80(sp)
    sw a3, 84(sp)
    jal __unpack_d
    addiu s0, sp, 32
    addiu a0, sp, 80
    jal __unpack_d
    move a1, s0
    lw v0, 0(sp)
    sltiu v0, v0, 2
    bnez v0, L_08818B18
    addiu v1, zero, 1
    lw v0, 32(sp)
    move a1, s0
    sltiu v0, v0, 2
    beqz v0, L_08818B2C
    move a0, sp
L_08818B18:
    lw ra, 100(sp)
    lw s0, 96(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 112
L_08818B2C:
    jal __fpcmp_parts_d
    nop
    j L_08818B18
    move v1, v0
}
#else
int __ltdf2(double a, double b)
{
    return fpcmp(a, b, 1);
}
#endif

/* <= 0 when a <= b */
#ifdef __MWERKS__
asm int __ledf2(double a, double b)
{
    addiu sp, sp, -112
    move v0, a0
    move v1, a1
    sw s0, 96(sp)
    addiu a0, sp, 64
    move a1, sp
    sw v0, 64(sp)
    sw v1, 68(sp)
    sw ra, 100(sp)
    sw a2, 80(sp)
    sw a3, 84(sp)
    jal __unpack_d
    addiu s0, sp, 32
    addiu a0, sp, 80
    jal __unpack_d
    move a1, s0
    lw v0, 0(sp)
    sltiu v0, v0, 2
    bnez v0, L_08818BA0
    addiu v1, zero, 1
    lw v0, 32(sp)
    move a1, s0
    sltiu v0, v0, 2
    beqz v0, L_08818BB4
    move a0, sp
L_08818BA0:
    lw ra, 100(sp)
    lw s0, 96(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 112
L_08818BB4:
    jal __fpcmp_parts_d
    nop
    j L_08818BA0
    move v1, v0
}
#else
int __ledf2(double a, double b)
{
    return fpcmp(a, b, 1);
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: -2^31 from the .rodata pool (D_088EB210) */
double __floatsidf(int i)
{
    return (double)i;
}
#endif

/* truncates toward zero; NaN gives 0, out of range saturates */
#ifdef __MWERKS__
asm int __fixdfsi(double a)
{
    addiu sp, sp, -64
    move v0, a0
    move v1, a1
    addiu a0, sp, 32
    sw v0, 32(sp)
    sw v1, 36(sp)
    sw ra, 48(sp)
    jal __unpack_d
    move a1, sp
    move a1, zero
    lw v1, 0(sp)
    xori v0, v1, 0x2
    beqz v0, L_08818D14
    sltiu a0, v1, 2
    bnez a0, L_08818D14
    xori v0, v1, 0x4
    lui v1, 0x7fff
    beqz v0, L_08818D08
    lui a1, 0x8000
    lw v0, 8(sp)
    move a1, zero
    bltz v0, L_08818D14
    slti a0, v0, 31
    lui v1, 0x7fff
    bnez a0, L_08818D24
    lui a1, 0x8000
L_08818D08:
    lw v0, 4(sp)
    ori v1, v1, 0xffff
    movz a1, v1, v0
L_08818D14:
    lw ra, 48(sp)
    move v0, a1
    jr ra
    addiu sp, sp, 64
L_08818D24:
    addiu a0, zero, 60
    subu a0, a0, v0
    lw v0, 16(sp)
    lw v1, 20(sp)
    sll t0, a0, 0x1a
    bgez t0, L_08818D4C
    nop
    srlv a2, v1, a0
    b L_08818D64
    move a3, zero
L_08818D4C:
    beqz t0, L_08818D60
    srlv a2, v0, a0
    negu t0, a0
    sllv t0, v1, t0
    or a2, a2, t0
L_08818D60:
    srlv a3, v1, a0
L_08818D64:
    lw v0, 4(sp)
    negu a1, a2
    j L_08818D14
    movz a1, a2, v0
}
#else
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
#endif

#ifdef __MWERKS__
asm double __negdf2(double a)
{
    addiu sp, sp, -64
    move v0, a0
    move v1, a1
    addiu a0, sp, 32
    sw ra, 48(sp)
    sw v0, 32(sp)
    sw v1, 36(sp)
    jal __unpack_d
    move a1, sp
    move a0, sp
    lw v0, 4(sp)
    sltiu v0, v0, 1
    jal __pack_d
    sw v0, 4(sp)
    lw ra, 48(sp)
    jr ra
    addiu sp, sp, 64
}
#else
double __negdf2(double a)
{
    return -a;
}
#endif

#ifdef __MWERKS__
asm double __make_dp(int class, unsigned int sign, int exp, UDItype frac)
{
    addiu sp, sp, -48
    move v0, a0
    sw ra, 32(sp)
    sw v0, 0(sp)
    sw a1, 4(sp)
    sw a2, 8(sp)
    sw t0, 16(sp)
    sw t1, 20(sp)
    jal __pack_d
    move a0, sp
    lw ra, 32(sp)
    jr ra
    addiu sp, sp, 48
}
#else
double __make_dp(int class, unsigned int sign, int exp, UDItype frac)
{
    FpD in;

    in.class = class;
    in.sign = sign;
    in.normal_exp = exp;
    in.fraction = frac;
    return __pack_d(&in);
}
#endif

#ifdef __MWERKS__
asm float __truncdfsf2(double a)
{
    addiu sp, sp, -64
    move v0, a0
    move v1, a1
    addiu a0, sp, 32
    sw ra, 48(sp)
    sw v0, 32(sp)
    sw v1, 36(sp)
    jal __unpack_d
    move a1, sp
    addiu v1, zero, 0
    lui v0, 0x3fff
    ori v0, v0, 0xffff
    lw t0, 16(sp)
    lw t1, 20(sp)
    lw a0, 0(sp)
    lw a1, 4(sp)
    and v1, t1, v1
    and v0, t0, v0
    sll a2, t1, 0x2
    or v0, v0, v1
    srl t0, t0, 0x1e
    or t0, t0, a2
    sltu v0, zero, v0
    lw a2, 8(sp)
    or v0, v0, t0
    jal __make_fp
    move a3, v0
    lw ra, 48(sp)
    jr ra
    addiu sp, sp, 64
}
#else
float __truncdfsf2(double a)
{
    return (float)a;
}
#endif
