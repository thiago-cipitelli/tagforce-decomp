// @range 0x08817784-0x08817D70
// @gcc 0x08817978-0x088179F4 fpbit_sf
/* libgcc software floating point (fp-bit.c, the GCC 3/4 generation: QUIET_NAN 0x100000 for single,
   denormals supported) for the soft-float GCC code in BOOT: the single-precision objects (pack, unpack, make,
   float to double) and the double pack/unpack. Numbers are unpacked into class/sign/exponent/fraction with guard
   bits (8 for double, 7 for float) and packed back with round-to-nearest-even. Whole `asm` functions: the pack
   and unpack routines come from a later revision of the SDK's GCC than the one obtainable (`ins` for masks, inline
   64-bit shifts; docs/build.md, "Trilha GCC"). The rest of the double routines is fpbit_df.c, real fp-bit.c
   compiled by GCC. Outside the PSP build the routines are the fp-bit C. */

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
