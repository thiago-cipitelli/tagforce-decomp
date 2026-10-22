// @range 0x0880E700-0x0880F60C
// @asm 0x0880F56C-0x0880F60C
/* newlib mprec.c (GCC): the multi-precision integers of dtoa/strtod. Bigints come from a per-reent
   freelist of power-of-two sizes (+0x4C of the reent) and the powers 5^(4 * 2^k) are cached at
   +0x48. Kept as the original instructions; the C under `#else` follows newlib's mprec.c (16-bit
   halves, as built without long long). _mprec_log10 loads its table through %hi/%lo and stays in
   the split asm. */
#include "libc.h"

typedef struct LibcBigint {            /* newlib _Bigint */
    struct LibcBigint *next;
    int k;
    int maxwds;
    int sign;
    int wds;                           /* +0x10 */
    unsigned int x[1];                 /* +0x14 */
} LibcBigint;

typedef union {
    double d;
    unsigned int w[2];                 /* w[1]: sign, exponent and the high mantissa bits */
} LibcDouble;

void *_calloc_r(void *r, size_t n, size_t size);
LibcBigint *_Balloc(char *r, int k);
void _Bfree(char *r, LibcBigint *v);
LibcBigint *__multadd(char *r, LibcBigint *b, int m, int a);
int __hi0bits(unsigned int x);
int __lo0bits(unsigned int *y);
LibcBigint *__i2b(char *r, int i);
LibcBigint *__multiply(char *r, LibcBigint *a, LibcBigint *b);
int libc_bigint_cmp(LibcBigint *a, LibcBigint *b);
double __b2d(LibcBigint *a, int *e);
double __divdf3(double a, double b);
extern const int libc_p05[3];          /* 0x08873C18 */

#ifdef __MWERKS__
asm LibcBigint *_Balloc(char *r, int k)
{
    addiu sp, sp, -16
    sw s1, 4(sp)
    addiu a2, zero, 16
    move s1, a1
    sw s0, 0(sp)
    addiu a1, zero, 4
    sw ra, 8(sp)
    lw a3, 76(a0)
    beqz a3, @9c
    move s0, a0
@28:
    sll v1, s1, 0x2
    addu v1, v1, a3
    addiu v0, zero, 1
    lw a3, 0(v1)
    move a0, s0
    sllv s0, v0, s1
    sll a2, s0, 0x2
    addiu a1, zero, 1
    beqz a3, @7c
    addiu a2, a2, 20
    lw v0, 0(a3)
    sw v0, 0(v1)
@58:
    sw zero, 16(a3)
    move v1, a3
    sw zero, 12(a3)
@64:
    lw ra, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 16
@7c:
    jal _calloc_r
    nop
    move a3, v0
    beqz v0, @64
    move v1, zero
    sw s1, 4(v0)
    j @58
    sw s0, 8(v0)
@9c:
    jal _calloc_r
    nop
    sw v0, 76(s0)
    beqz v0, @64
    move v1, zero
    j @28
    move a3, v0
}
#else
LibcBigint *_Balloc(char *r, int k)
{
    LibcBigint **list = *(LibcBigint ***)(r + 0x4C);
    LibcBigint *rv;

    if (list == 0) {
        list = _calloc_r(r, sizeof(LibcBigint *), 16);
        *(LibcBigint ***)(r + 0x4C) = list;
        if (list == 0) {
            return 0;
        }
    }
    rv = list[k];
    if (rv != 0) {
        list[k] = rv->next;
    } else {
        int x = 1 << k;

        rv = _calloc_r(r, 1, sizeof(LibcBigint) + (x - 1) * sizeof(unsigned int));
        if (rv == 0) {
            return 0;
        }
        rv->k = k;
        rv->maxwds = x;
    }
    rv->wds = 0;
    rv->sign = 0;
    return rv;
}
#endif

#ifdef __MWERKS__
asm void _Bfree(char *r, LibcBigint *v)
{
    beqz a1, @24
    nop
    lw v0, 4(a1)
    lw a0, 76(a0)
    sll v0, v0, 0x2
    addu v0, v0, a0
    lw v1, 0(v0)
    sw v1, 0(a1)
    sw a1, 0(v0)
@24:
    jr ra
    nop
}
#else
void _Bfree(char *r, LibcBigint *v)
{
    if (v != 0) {
        LibcBigint **list = *(LibcBigint ***)(r + 0x4C);

        v->next = list[v->k];
        list[v->k] = v;
    }
}
#endif

/* b * m + a */
#ifdef __MWERKS__
asm LibcBigint *__multadd(char *r, LibcBigint *b, int m, int a)
{
    addiu sp, sp, -32
    sw s4, 16(sp)
    move t1, a2
    move s4, a0
    sw s3, 12(sp)
    move t0, zero
    move s3, a1
    sw s1, 4(sp)
    move s1, a3
    addiu a3, a1, 20
    sw ra, 20(sp)
    sw s2, 8(sp)
    sw s0, 0(sp)
    lw s2, 16(a1)
@38:
    lw a0, 0(a3)
    addiu t0, t0, 1
    slt a2, t0, s2
    andi v1, a0, 0xffff
    mult v1, t1
    srl a0, a0, 0x10
    mflo v1
    mult a0, t1
    addu v1, v1, s1
    srl a1, v1, 0x10
    andi v1, v1, 0xffff
    mflo a0
    addu a0, a0, a1
    sll v0, a0, 0x10
    addu v0, v0, v1
    sw v0, 0(a3)
    srl s1, a0, 0x10
    bnez a2, @38
    addiu a3, a3, 4
    beqz s1, @b4
    move v0, s3
    lw v0, 8(s3)
    slt v0, s2, v0
    beqzl v0, @d4
    lw a1, 4(s3)
@9c:
    sll v0, s2, 0x2
    addu v0, v0, s3
    addiu s2, s2, 1
    sw s1, 20(v0)
    sw s2, 16(s3)
    move v0, s3
@b4:
    lw ra, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 32
@d4:
    move a0, s4
    jal _Balloc
    addiu a1, a1, 1
    lw a2, 16(s3)
    addiu a1, s3, 12
    addiu a0, v0, 12
    sll a2, a2, 0x2
    addiu a2, a2, 8
    jal memcpy
    move s0, v0
    move a1, s3
    jal _Bfree
    move a0, s4
    j @9c
    move s3, s0
}
#else
LibcBigint *__multadd(char *r, LibcBigint *b, int m, int a)
{
    int wds = b->wds;
    unsigned int *x = b->x;
    unsigned int carry = a;
    int i = 0;

    do {
        unsigned int xi = *x;
        unsigned int y = (xi & 0xFFFF) * m + carry;
        unsigned int z = (xi >> 16) * m + (y >> 16);

        carry = z >> 16;
        *x++ = (z << 16) + (y & 0xFFFF);
    } while (++i < wds);
    if (carry != 0) {
        if (wds >= b->maxwds) {
            LibcBigint *b1 = _Balloc(r, b->k + 1);

            memcpy(&b1->sign, &b->sign, b->wds * sizeof(unsigned int) + 2 * sizeof(int));
            _Bfree(r, b);
            b = b1;
        }
        b->x[wds++] = carry;
        b->wds = wds;
    }
    return b;
}
#endif

/* the decimal string s (nd digits, the first nd0 before the point, first 9 already in y9) */
#ifdef __MWERKS__
asm LibcBigint *__s2b(char *r, const char *s, int nd0, int nd, unsigned int y9)
{
    lui v0, 0x38e3
    addiu v1, a3, 8
    ori v0, v0, 0x8e39
    mult v1, v0
    addiu sp, sp, -32
    sra v1, v1, 0x1f
    sw s4, 16(sp)
    move s4, a3
    mfhi v0
    sra v0, v0, 0x1
    addiu a3, zero, 1
    subu v1, v0, v1
    sw s2, 8(sp)
    move s2, a0
    slt a0, a3, v1
    sw s3, 12(sp)
    move s3, a2
    sw s1, 4(sp)
    move s1, t0
    sw s0, 0(sp)
    move s0, a1
    move a1, zero
    beqz a0, @70
    sw ra, 20(sp)
@60:
    sll a3, a3, 0x1
    slt v0, a3, v1
    bnez v0, @60
    addiu a1, a1, 1
@70:
    jal _Balloc
    move a0, s2
    move a1, v0
    sw s1, 20(v0)
    slti v1, s3, 10
    addiu v0, zero, 1
    addiu s1, zero, 9
    bnez v1, @118
    sw v0, 16(a1)
    addiu s0, s0, 9
@98:
    lb a3, 0(s0)
    move a0, s2
    addiu a2, zero, 10
    jal __multadd
    addiu a3, a3, -48
    addiu s1, s1, 1
    move a1, v0
    slt v0, s1, s3
    bnez v0, @98
    addiu s0, s0, 1
    addiu s0, s0, 1
@c4:
    slt v0, s1, s4
    beqz v0, @f4
    subu s1, s4, s1
@d0:
    lb a3, 0(s0)
    move a0, s2
    addiu a2, zero, 10
    jal __multadd
    addiu a3, a3, -48
    addiu s1, s1, -1
    move a1, v0
    bnez s1, @d0
    addiu s0, s0, 1
@f4:
    lw ra, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    move v0, a1
    jr ra
    addiu sp, sp, 32
@118:
    j @c4
    addiu s0, s0, 10
}
#else
LibcBigint *__s2b(char *r, const char *s, int nd0, int nd, unsigned int y9)
{
    LibcBigint *b;
    int i;
    int k;
    int x;
    int y;

    x = (nd + 8) / 9;
    for (k = 0, y = 1; x > y; y <<= 1, k++) {
    }
    b = _Balloc(r, k);
    b->x[0] = y9;
    b->wds = 1;
    i = 9;
    if (9 < nd0) {
        s += 9;
        do {
            b = __multadd(r, b, 10, *s++ - '0');
        } while (++i < nd0);
        s++;
    } else {
        s += 10;
    }
    for (; i < nd; i++) {
        b = __multadd(r, b, 10, *s++ - '0');
    }
    return b;
}
#endif

#ifdef __MWERKS__
asm int __hi0bits(unsigned int x)
{
    move v0, a0
    ins v0, zero, 0x0, 0x10
    bnez v0, @18
    move v1, zero
    addiu v1, zero, 16
    sll a0, a0, 0x10
@18:
    move v0, a0
    ins v0, zero, 0x0, 0x18
    bnez v0, @34
    move v0, a0
    addiu v1, v1, 8
    sll a0, a0, 0x8
    move v0, a0
@34:
    ins v0, zero, 0x0, 0x1c
    bnez v0, @4c
    move v0, a0
    addiu v1, v1, 4
    sll a0, a0, 0x4
    move v0, a0
@4c:
    ins v0, zero, 0x0, 0x1e
    bnez v0, @60
    nop
    addiu v1, v1, 2
    sll a0, a0, 0x2
@60:
    bltzl a0, @80
    move a0, v1
    lui v0, 0x4000
    and v0, a0, v0
    addiu v1, v1, 1
    beqz v0, @80
    addiu a0, zero, 32
    move a0, v1
@80:
    jr ra
    move v0, a0
}
#else
int __hi0bits(unsigned int x)
{
    int k = 0;

    if (!(x & 0xFFFF0000)) {
        k = 16;
        x <<= 16;
    }
    if (!(x & 0xFF000000)) {
        k += 8;
        x <<= 8;
    }
    if (!(x & 0xF0000000)) {
        k += 4;
        x <<= 4;
    }
    if (!(x & 0xC0000000)) {
        k += 2;
        x <<= 2;
    }
    if (!(x & 0x80000000)) {
        k++;
        if (!(x & 0x40000000)) {
            return 32;
        }
    }
    return k;
}
#endif

#ifdef __MWERKS__
asm int __lo0bits(unsigned int *y)
{
    lw v1, 0(a0)
    andi v0, v1, 0x7
    beqz v0, @44
    andi v0, v1, 0xffff
    andi v0, v1, 0x1
    bnez v0, @34
    move a1, zero
    andi v0, v1, 0x2
    beqzl v0, @3c
    srl v0, v1, 0x2
    srl v0, v1, 0x1
    addiu a1, zero, 1
@30:
    sw v0, 0(a0)
@34:
    jr ra
    move v0, a1
@3c:
    j @30
    addiu a1, zero, 2
@44:
    bnez v0, @54
    move a2, zero
    addiu a2, zero, 16
    srl v1, v1, 0x10
@54:
    andi v0, v1, 0xff
    bnez v0, @6c
    andi v0, v1, 0xf
    addiu a2, a2, 8
    srl v1, v1, 0x8
    andi v0, v1, 0xf
@6c:
    bnez v0, @80
    andi v0, v1, 0x3
    addiu a2, a2, 4
    srl v1, v1, 0x4
    andi v0, v1, 0x3
@80:
    bnez v0, @94
    andi v0, v1, 0x1
    addiu a2, a2, 2
    srl v1, v1, 0x2
    andi v0, v1, 0x1
@94:
    bnezl v0, @b0
    sw v1, 0(a0)
    srl v1, v1, 0x1
    addiu a2, a2, 1
    beqz v1, @34
    addiu a1, zero, 32
    sw v1, 0(a0)
@b0:
    j @34
    move a1, a2
}
#else
int __lo0bits(unsigned int *y)
{
    int k;
    unsigned int x = *y;

    if (x & 7) {
        if (x & 1) {
            return 0;
        }
        if (x & 2) {
            *y = x >> 1;
            return 1;
        }
        *y = x >> 2;
        return 2;
    }
    k = 0;
    if (!(x & 0xFFFF)) {
        k = 16;
        x >>= 16;
    }
    if (!(x & 0xFF)) {
        k += 8;
        x >>= 8;
    }
    if (!(x & 0xF)) {
        k += 4;
        x >>= 4;
    }
    if (!(x & 3)) {
        k += 2;
        x >>= 2;
    }
    if (!(x & 1)) {
        k++;
        x >>= 1;
        if (x == 0) {
            return 32;
        }
    }
    *y = x;
    return k;
}
#endif

#ifdef __MWERKS__
asm LibcBigint *__i2b(char *r, int i)
{
    addiu sp, sp, -16
    sw s0, 0(sp)
    move s0, a1
    sw ra, 4(sp)
    jal _Balloc
    addiu a1, zero, 1
    addiu v1, zero, 1
    sw s0, 20(v0)
    lw ra, 4(sp)
    lw s0, 0(sp)
    sw v1, 16(v0)
    jr ra
    addiu sp, sp, 16
}
#else
LibcBigint *__i2b(char *r, int i)
{
    LibcBigint *b = _Balloc(r, 1);

    b->x[0] = i;
    b->wds = 1;
    return b;
}
#endif

#ifdef __MWERKS__
asm LibcBigint *__multiply(char *r, LibcBigint *a, LibcBigint *b)
{
    addiu sp, sp, -32
    sw s4, 16(sp)
    move s4, a2
    sw s0, 0(sp)
    sw ra, 24(sp)
    sw s5, 20(sp)
    sw s3, 12(sp)
    sw s2, 8(sp)
    sw s1, 4(sp)
    lw s3, 16(a1)
    lw s2, 16(a2)
    slt v0, s3, s2
    beqz v0, @48
    move s0, a1
    move s3, s2
    lw s2, 16(a1)
    move s0, a2
    move s4, a1
@48:
    lw v0, 8(s0)
    lw a1, 4(s0)
    addu s1, s3, s2
    slt v0, v0, s1
    jal _Balloc
    addu a1, a1, v0
    addiu t8, v0, 20
    sll s5, s1, 0x2
    addu t7, t8, s5
    move t9, v0
    sltu v0, t8, t7
    beqz v0, @90
    move t1, t8
    sw zero, 0(t1)
@80:
    addiu t1, t1, 4
    sltu v0, t1, t7
    bnezl v0, @80
    sw zero, 0(t1)
@90:
    addiu t4, s4, 20
    sll v0, s2, 0x2
    addu s2, t4, v0
    addiu t7, s0, 20
    sll v0, s3, 0x2
    sltu v1, t4, s2
    addu t5, t7, v0
    beqz v1, @1bc
    move t6, t8
@b4:
    lw v0, 0(t4)
    andi t3, v0, 0xffff
    beqzl t3, @134
    srl t3, v0, 0x10
    move t1, t7
    move t0, t6
    move t2, zero
@d0:
    lw v0, 0(t1)
    lw a0, 0(t0)
    addiu t1, t1, 4
    andi v1, v0, 0xffff
    mult v1, t3
    srl v0, v0, 0x10
    andi a1, a0, 0xffff
    srl a0, a0, 0x10
    sltu a2, t1, t5
    mflo v1
    mult v0, t3
    addu v1, v1, a1
    addu a3, v1, t2
    srl t2, a3, 0x10
    sh a3, 0(t0)
    mflo v0
    addu v0, v0, a0
    addu v0, v0, t2
    sh v0, 2(t0)
    srl t2, v0, 0x10
    bnez a2, @d0
    addiu t0, t0, 4
    sw t2, 0(t0)
    lw v0, 0(t4)
    srl t3, v0, 0x10
@134:
    beqzl t3, @1b0
    addiu t4, t4, 4
    lw v0, 0(t6)
    move t0, t6
    move t1, t7
    move t2, zero
    move a0, v0
@150:
    lw v1, 0(t1)
    sh v0, 0(t0)
    srl a0, a0, 0x10
    andi v0, v1, 0xffff
    mult v0, t3
    srl v1, v1, 0x10
    addiu t1, t1, 4
    sltu a1, t1, t5
    mflo v0
    addu v0, v0, a0
    addu a3, v0, t2
    mult v1, t3
    sh a3, 2(t0)
    addiu t0, t0, 4
    srl t2, a3, 0x10
    lw a0, 0(t0)
    mflo v1
    andi v0, a0, 0xffff
    addu v1, v1, v0
    addu v0, v1, t2
    bnez a1, @150
    srl t2, v0, 0x10
    sw v0, 0(t0)
    addiu t4, t4, 4
@1b0:
    sltu v0, t4, s2
    bnez v0, @b4
    addiu t6, t6, 4
@1bc:
    blez s1, @1ec
    addu t0, t8, s5
    addiu t0, t0, -4
    lw v0, 0(t0)
    bnezl v0, @1f0
    sw s1, 16(t9)
    addiu s1, s1, -1
@1d8:
    blez s1, @1ec
    addiu t0, t0, -4
    lw v0, 0(t0)
    beqzl v0, @1d8
    addiu s1, s1, -1
@1ec:
    sw s1, 16(t9)
@1f0:
    move v0, t9
    lw ra, 24(sp)
    lw s5, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 32
}
#else
LibcBigint *__multiply(char *r, LibcBigint *a, LibcBigint *b)
{
    LibcBigint *c;
    int k;
    int wa;
    int wb;
    int wc;
    unsigned int *x;
    unsigned int *xa;
    unsigned int *xae;
    unsigned int *xb;
    unsigned int *xbe;
    unsigned int *xc;
    unsigned int *xc0;
    unsigned int y;
    unsigned int carry;
    unsigned int z;
    unsigned int z2;

    if (a->wds < b->wds) {
        c = a;
        a = b;
        b = c;
    }
    k = a->k;
    wa = a->wds;
    wb = b->wds;
    wc = wa + wb;
    if (wc > a->maxwds) {
        k++;
    }
    c = _Balloc(r, k);
    for (x = c->x, xa = x + wc; x < xa; x++) {
        *x = 0;
    }
    xa = a->x;
    xae = xa + wa;
    xb = b->x;
    xbe = xb + wb;
    xc0 = c->x;
    for (; xb < xbe; xb++, xc0++) {
        if ((y = *xb & 0xFFFF) != 0) {
            x = xa;
            xc = xc0;
            carry = 0;
            do {
                z = (*x & 0xFFFF) * y + (*xc & 0xFFFF) + carry;
                carry = z >> 16;
                z2 = (*x++ >> 16) * y + (*xc >> 16) + carry;
                carry = z2 >> 16;
                *xc++ = (z2 << 16) | (z & 0xFFFF);
            } while (x < xae);
            *xc = carry;
        }
        if ((y = *xb >> 16) != 0) {
            x = xa;
            xc = xc0;
            carry = 0;
            z2 = *xc;
            do {
                z = (*x & 0xFFFF) * y + (*xc >> 16) + carry;
                carry = z >> 16;
                *xc = (z << 16) | (z2 & 0xFFFF);
                xc++;
                z2 = (*x++ >> 16) * y + (*xc & 0xFFFF) + carry;
                carry = z2 >> 16;
            } while (x < xae);
            *xc = z2;
        }
    }
    for (xc0 = c->x, xc = xc0 + wc; wc > 0 && *--xc == 0; --wc) {
    }
    c->wds = wc;
    return c;
}
#endif

/* b * 5^k; the 5^(4 * 2^n) factors are cached in a chain from 625 */
#ifdef __MWERKS__
asm LibcBigint *__pow5mult(char *r, LibcBigint *b, int k)
{
    addiu sp, sp, -32
    andi v1, a2, 0x3
    sw s4, 16(sp)
    move s4, a0
    sw s3, 12(sp)
    move s3, a1
    sw s2, 8(sp)
    move s2, a2
    sw ra, 20(sp)
    sw s1, 4(sp)
    bnez v1, @f4
    sw s0, 0(sp)
@30:
    sra s2, s2, 0x2
    beqz s2, @9c
    move v0, s3
    lw s1, 72(s4)
    beqz s1, @dc
    move a0, s4
@48:
    andi v0, s2, 0x1
    move a2, s1
    move a0, s4
    move a1, s3
    bnez v0, @bc
    sra s2, s2, 0x1
@60:
    move a1, s1
    move a2, s1
    beqz s2, @98
    move a0, s4
    lw v0, 0(s1)
    beqz v0, @84
    nop
@7c:
    j @48
    move s1, v0
@84:
    jal __multiply
    nop
    sw v0, 0(s1)
    j @7c
    sw zero, 0(v0)
@98:
    move v0, s3
@9c:
    lw ra, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 32
@bc:
    jal __multiply
    nop
    move s0, v0
    move a1, s3
    jal _Bfree
    move a0, s4
    j @60
    move s3, s0
@dc:
    jal __i2b
    addiu a1, zero, 625
    sw v0, 72(s4)
    move s1, v0
    j @48
    sw zero, 0(v0)
@f4:
    la v0, libc_p05
    sll v1, v1, 0x2
    addu v1, v1, v0
    lw a2, -4(v1)
    jal __multadd
    move a3, zero
    j @30
    move s3, v0
}
#else
LibcBigint *__pow5mult(char *r, LibcBigint *b, int k)
{
    static const int p05[3] = { 5, 25, 125 };
    LibcBigint *p5;
    LibcBigint *p51;
    LibcBigint *b1;
    int i;

    if ((i = k & 3) != 0) {
        b = __multadd(r, b, p05[i - 1], 0);
    }
    if (!(k >>= 2)) {
        return b;
    }
    p5 = *(LibcBigint **)(r + 0x48);
    if (p5 == 0) {
        p5 = __i2b(r, 625);
        *(LibcBigint **)(r + 0x48) = p5;
        p5->next = 0;
    }
    for (;;) {
        if (k & 1) {
            b1 = __multiply(r, b, p5);
            _Bfree(r, b);
            b = b1;
        }
        if (!(k >>= 1)) {
            break;
        }
        p51 = p5->next;
        if (p51 == 0) {
            p51 = __multiply(r, p5, p5);
            p5->next = p51;
            p51->next = 0;
        }
        p5 = p51;
    }
    return b;
}
#endif

/* b << k */
#ifdef __MWERKS__
asm LibcBigint *__lshift(char *r, LibcBigint *b, int k)
{
    addiu sp, sp, -32
    sw s5, 20(sp)
    move s5, a0
    sw s3, 12(sp)
    move s3, a1
    sw s2, 8(sp)
    sw s1, 4(sp)
    sra s1, a2, 0x5
    sw s0, 0(sp)
    move s0, a2
    sw ra, 24(sp)
    sw s4, 16(sp)
    lw v0, 16(a1)
    lw a2, 8(a1)
    addu v0, s1, v0
    addiu s2, v0, 1
    slt v1, a2, s2
    beqz v1, @5c
    lw a1, 4(a1)
@4c:
    sll a2, a2, 0x1
    slt v0, a2, s2
    bnez v0, @4c
    addiu a1, a1, 1
@5c:
    jal _Balloc
    move a0, s5
    move s4, v0
    blez s1, @84
    addiu a3, v0, 20
    move a2, s1
@74:
    addiu a2, a2, -1
    sw zero, 0(a3)
    bnez a2, @74
    addiu a3, a3, 4
@84:
    lw v0, 16(s3)
    addiu a0, s3, 20
    andi s0, s0, 0x1f
    sll v0, v0, 0x2
    beqz s0, @118
    addu a2, a0, v0
    addiu v0, zero, 32
    subu a1, v0, s0
    move v1, zero
@a8:
    lw v0, 0(a0)
    sllv v0, v0, s0
    or v0, v0, v1
    sw v0, 0(a3)
    addiu a3, a3, 4
    lw v1, 0(a0)
    addiu a0, a0, 4
    sltu v0, a0, a2
    bnez v0, @a8
    srlv v1, v1, a1
    beqz v1, @dc
    sw v1, 0(a3)
    addiu s2, s2, 1
@dc:
    addiu v0, s2, -1
@e0:
    sw v0, 16(s4)
    move a0, s5
    jal _Bfree
    move a1, s3
    move v0, s4
    lw ra, 24(sp)
    lw s5, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 32
@118:
    lw v0, 0(a0)
    addiu a0, a0, 4
    sltu v1, a0, a2
    sw v0, 0(a3)
    bnez v1, @118
    addiu a3, a3, 4
    j @e0
    addiu v0, s2, -1
}
#else
LibcBigint *__lshift(char *r, LibcBigint *b, int k)
{
    int i;
    int k1;
    int n;
    int n1;
    LibcBigint *b1;
    unsigned int *x;
    unsigned int *x1;
    unsigned int *xe;
    unsigned int z;

    n = k >> 5;
    k1 = b->k;
    n1 = n + b->wds + 1;
    for (i = b->maxwds; n1 > i; i <<= 1) {
        k1++;
    }
    b1 = _Balloc(r, k1);
    x1 = b1->x;
    for (i = 0; i < n; i++) {
        *x1++ = 0;
    }
    x = b->x;
    xe = x + b->wds;
    if (k &= 0x1F) {
        k1 = 32 - k;
        z = 0;
        do {
            *x1++ = *x << k | z;
            z = *x++ >> k1;
        } while (x < xe);
        if ((*x1 = z) != 0) {
            ++n1;
        }
    } else {
        do {
            *x1++ = *x++;
        } while (x < xe);
    }
    b1->wds = n1 - 1;
    _Bfree(r, b);
    return b1;
}
#endif

/* __mcmp */
#ifdef __MWERKS__
asm int libc_bigint_cmp(LibcBigint *a, LibcBigint *b)
{
    lw v0, 16(a0)
    lw v1, 16(a1)
    subu v0, v0, v1
    bnez v0, @4c
    move a2, v0
    sll v1, v1, 0x2
    addiu a0, a0, 20
    addiu v0, a1, 20
    addu v0, v0, v1
    addu v1, a0, v1
    addiu v1, v1, -4
@2c:
    addiu v0, v0, -4
    lw a3, 0(v1)
    lw a1, 0(v0)
    bne a3, a1, @54
    sltu a2, a0, v1
    bnezl a2, @2c
    addiu v1, v1, -4
    move a2, zero
@4c:
    jr ra
    move v0, a2
@54:
    sltu v1, a3, a1
    addiu a2, zero, -1
    addiu v0, zero, 1
    j @4c
    movz a2, v0, v1
}
#else
int libc_bigint_cmp(LibcBigint *a, LibcBigint *b)
{
    unsigned int *xa;
    unsigned int *xa0;
    unsigned int *xb;
    int i = a->wds;
    int j = b->wds;

    if ((i -= j) != 0) {
        return i;
    }
    xa0 = a->x;
    xa = xa0 + j;
    xb = b->x + j;
    for (;;) {
        if (*--xa != *--xb) {
            return *xa < *xb ? -1 : 1;
        }
        if (xa <= xa0) {
            break;
        }
    }
    return 0;
}
#endif

/* |a - b| with the sign in ->sign */
#ifdef __MWERKS__
asm LibcBigint *__mdiff(char *r, LibcBigint *a, LibcBigint *b)
{
    addiu sp, sp, -32
    sw s1, 4(sp)
    move s1, a1
    move a1, a2
    sw s3, 12(sp)
    move s3, a0
    move a0, s1
    sw s2, 8(sp)
    move s2, a2
    sw ra, 16(sp)
    jal libc_bigint_cmp
    sw s0, 0(sp)
    beqz v0, @160
    move a0, s3
    bltz v0, @14c
    move s0, zero
@40:
    lw a1, 4(s1)
    jal _Balloc
    move a0, s3
    lw t2, 16(s1)
    lw v1, 16(s2)
    move t4, v0
    sw s0, 12(v0)
    addiu t0, s1, 20
    addiu a2, s2, 20
    sll v1, v1, 0x2
    sll v0, t2, 0x2
    addu t5, t0, v0
    addu t3, a2, v1
    addiu a3, t4, 20
    move t1, zero
@7c:
    lw a0, 0(t0)
    lw a1, 0(a2)
    addiu a2, a2, 4
    andi v0, a0, 0xffff
    andi v1, a1, 0xffff
    subu v0, v0, v1
    addu v1, v0, t1
    srl a1, a1, 0x10
    srl a0, a0, 0x10
    sra t1, v1, 0x10
    subu a0, a0, a1
    addu a1, a0, t1
    sltu v0, a2, t3
    sh a1, 2(a3)
    addiu t0, t0, 4
    sra t1, a1, 0x10
    sh v1, 0(a3)
    bnez v0, @7c
    addiu a3, a3, 4
    sltu v0, t0, t5
    beqzl v0, @10c
    addiu a3, a3, -4
@d4:
    lw v0, 0(t0)
    addiu t0, t0, 4
    sltu a0, t0, t5
    andi v1, v0, 0xffff
    addu v1, v1, t1
    sra t1, v1, 0x10
    srl v0, v0, 0x10
    addu a1, v0, t1
    sh a1, 2(a3)
    sra t1, a1, 0x10
    sh v1, 0(a3)
    bnez a0, @d4
    addiu a3, a3, 4
    addiu a3, a3, -4
@10c:
    lw v0, 0(a3)
    bnezl v0, @12c
    sw t2, 16(t4)
@118:
    addiu a3, a3, -4
    lw v0, 0(a3)
    beqz v0, @118
    addiu t2, t2, -1
    sw t2, 16(t4)
@12c:
    lw ra, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    move v0, t4
    jr ra
    addiu sp, sp, 32
@14c:
    move t4, s1
    addiu s0, zero, 1
    move s1, s2
    j @40
    move s2, t4
@160:
    jal _Balloc
    move a1, zero
    move t4, v0
    addiu v0, zero, 1
    sw v0, 16(t4)
    j @12c
    sw zero, 20(t4)
}
#else
LibcBigint *__mdiff(char *r, LibcBigint *a, LibcBigint *b)
{
    LibcBigint *c;
    int i;
    int wa;
    int wb;
    int borrow;
    unsigned int *xa;
    unsigned int *xae;
    unsigned int *xb;
    unsigned int *xbe;
    unsigned int *xc;

    i = libc_bigint_cmp(a, b);
    if (i == 0) {
        c = _Balloc(r, 0);
        c->wds = 1;
        c->x[0] = 0;
        return c;
    }
    if (i < 0) {
        c = a;
        a = b;
        b = c;
        i = 1;
    } else {
        i = 0;
    }
    c = _Balloc(r, a->k);
    c->sign = i;
    wa = a->wds;
    xa = a->x;
    xae = xa + wa;
    wb = b->wds;
    xb = b->x;
    xbe = xb + wb;
    xc = c->x;
    borrow = 0;
    do {
        int y = (int)(*xa & 0xFFFF) - (int)(*xb & 0xFFFF) + borrow;
        int z;

        borrow = y >> 16;
        z = (int)(*xa++ >> 16) - (int)(*xb++ >> 16) + borrow;
        borrow = z >> 16;
        *xc++ = (unsigned int)(z & 0xFFFF) << 16 | (y & 0xFFFF);
    } while (xb < xbe);
    while (xa < xae) {
        int y = (int)(*xa & 0xFFFF) + borrow;
        int z;

        borrow = y >> 16;
        z = (int)(*xa++ >> 16) + borrow;
        borrow = z >> 16;
        *xc++ = (unsigned int)(z & 0xFFFF) << 16 | (y & 0xFFFF);
    }
    while (*--xc == 0) {
        wa--;
    }
    c->wds = wa;
    return c;
}
#endif

/* one unit in the last place of x */
#ifdef __MWERKS__
asm double __ulp(double x)
{
    lui v0, 0x7ff0
    and v0, a1, v0
    lui v1, 0xfcc0
    addu a0, v0, v1
    negu v0, a0
    blez a0, @2c
    move a3, a0
@1c:
    move a2, zero
@20:
    move v0, a2
    jr ra
    move v1, a3
@2c:
    sra a0, v0, 0x14
    slti v0, a0, 20
    beqz v0, @48
    addiu a1, zero, 1
    lui v0, 0x8
    j @1c
    srav a3, v0, a0
@48:
    addiu a0, a0, -20
    slti v1, a0, 31
    addiu v0, zero, 31
    subu a0, v0, a0
    beqz v1, @68
    move a3, zero
    addiu v0, zero, 1
    sllv a1, v0, a0
@68:
    j @20
    move a2, a1
}
#else
double __ulp(double x)
{
    LibcDouble a;
    LibcDouble u;
    int L;

    u.d = x;
    L = (u.w[1] & 0x7FF00000) - (53 - 1) * 0x100000;
    if (L > 0) {
        a.w[1] = L;
        a.w[0] = 0;
    } else {
        L = -L >> 20;
        if (L < 20) {
            a.w[1] = 0x80000 >> L;
            a.w[0] = 0;
        } else {
            a.w[1] = 0;
            L -= 20;
            a.w[0] = L >= 31 ? 1 : 1u << (31 - L);
        }
    }
    return a.d;
}
#endif

/* the top 53 bits of a as a double in [1, 2); *e = bits above them */
#ifdef __MWERKS__
asm double __b2d(LibcBigint *a, int *e)
{
    addiu sp, sp, -32
    sw s3, 12(sp)
    addiu s3, a0, 20
    sw s1, 4(sp)
    sw s0, 0(sp)
    move s0, a1
    sw ra, 24(sp)
    sw s5, 20(sp)
    sw s4, 16(sp)
    sw s2, 8(sp)
    lw v0, 16(a0)
    sll v0, v0, 0x2
    addu s1, s3, v0
    addiu s1, s1, -4
    lw s2, 0(s1)
    jal __hi0bits
    move a0, s2
    move a1, v0
    addiu v0, zero, 32
    subu v0, v0, a1
    slti v1, a1, 11
    bnez v1, @e8
    sw v0, 0(s0)
    sltu v0, s3, s1
    beqz v0, @70
    move a2, zero
    addiu s1, s1, -4
    lw a2, 0(s1)
@70:
    lui v0, 0x3ff0
    addiu a1, a1, -11
    or s5, s2, v0
    beqz a1, @bc
    move s4, a2
    negu v0, a1
    sllv v1, s2, a1
    srlv v0, a2, v0
    or v1, v1, v0
    sltu a0, s3, s1
    lui v0, 0x3ff0
    or s5, v1, v0
    beqz a0, @ac
    move s2, zero
    lw s2, -4(s1)
@ac:
    negu v0, a1
    srlv v0, s2, v0
    sllv v1, a2, a1
@b8:
    or s4, v1, v0
@bc:
    move v0, s4
    move v1, s5
    lw ra, 24(sp)
    lw s5, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 32
@e8:
    addiu v0, zero, 11
    subu v0, v0, a1
    srlv v0, s2, v0
    lui v1, 0x3ff0
    sltu a0, s3, s1
    or s5, v0, v1
    beqz a0, @10c
    move a2, zero
    lw a2, -4(s1)
@10c:
    addiu v1, zero, 11
    subu v1, v1, a1
    addiu v0, a1, 21
    sllv v0, s2, v0
    j @b8
    srlv v1, a2, v1
}
#else
double __b2d(LibcBigint *a, int *e)
{
    unsigned int *xa0 = a->x;
    unsigned int *xa = xa0 + a->wds;
    unsigned int y = *--xa;
    unsigned int z;
    unsigned int w;
    int k = __hi0bits(y);
    LibcDouble d;

    *e = 32 - k;
    if (k < 11) {
        d.w[1] = 0x3FF00000 | y >> (11 - k);
        w = xa > xa0 ? *--xa : 0;
        d.w[0] = y << (32 - 11 + k) | w >> (11 - k);
        return d.d;
    }
    z = xa > xa0 ? *--xa : 0;
    if (k -= 11) {
        d.w[1] = 0x3FF00000 | y << k | z >> (32 - k);
        y = xa > xa0 ? *--xa : 0;
        d.w[0] = z << k | y >> (32 - k);
    } else {
        d.w[1] = 0x3FF00000 | y;
        d.w[0] = z;
    }
    return d.d;
}
#endif

/* the Bigint of d's mantissa; *e = binary exponent, *bits = significant bits */
#ifdef __MWERKS__
asm LibcBigint *__d2b(char *r, double dd, int *e, int *bits)
{
    addiu sp, sp, -48
    sw s1, 20(sp)
    addiu a1, zero, 1
    move s1, a3
    sw s6, 40(sp)
    move s6, t1
    sw s5, 36(sp)
    move s5, t0
    sw s4, 32(sp)
    sw s3, 28(sp)
    sw s2, 24(sp)
    sw s0, 16(sp)
    sw ra, 44(sp)
    jal _Balloc
    move s0, a2
    move v1, s1
    ext s1, s1, 0x0, 0x1f
    ins v1, zero, 0x14, 0xc
    srl s3, s1, 0x14
    move s2, v0
    sw v1, 4(sp)
    beqz s3, @68
    addiu s4, v0, 20
    lui v0, 0x10
    or v0, v1, v0
    sw v0, 4(sp)
@68:
    beqz s0, @13c
    move a0, sp
    jal __lo0bits
    sw s0, 0(sp)
    beqz v0, @130
    move a1, v0
    lw v1, 4(sp)
    negu a0, v0
    lw v0, 0(sp)
    sllv v1, v1, a0
    or v0, v0, v1
    sw v0, 20(s2)
    lw v1, 4(sp)
    srlv v1, v1, a1
    sw v1, 4(sp)
@a4:
    lw a0, 4(sp)
    addiu v1, zero, 2
    addiu v0, zero, 1
    movz v1, v0, a0
    move s0, v1
    sw a0, 4(s4)
    sw v1, 16(s2)
@c0:
    beqz s3, @10c
    sll v1, s0, 0x2
    addu v0, s3, a1
    addiu v1, zero, 53
    subu v1, v1, a1
    addiu v0, v0, -1075
    sw v0, 0(s5)
    sw v1, 0(s6)
@e0:
    move v0, s2
    lw ra, 44(sp)
    lw s6, 40(sp)
    lw s5, 36(sp)
    lw s4, 32(sp)
    lw s3, 28(sp)
    lw s2, 24(sp)
    lw s1, 20(sp)
    lw s0, 16(sp)
    jr ra
    addiu sp, sp, 48
@10c:
    addu v1, v1, s4
    addiu v0, a1, -1074
    sw v0, 0(s5)
    sll s0, s0, 0x5
    jal __hi0bits
    lw a0, -4(v1)
    subu s0, s0, v0
    j @e0
    sw s0, 0(s6)
@130:
    lw v0, 0(sp)
    j @a4
    sw v0, 20(s2)
@13c:
    jal __lo0bits
    addiu a0, sp, 4
    move a1, v0
    addiu v0, zero, 1
    sw v0, 16(s2)
    addiu s0, zero, 1
    addiu a1, a1, 32
    lw v0, 4(sp)
    j @c0
    sw v0, 20(s2)
}
#else
LibcBigint *__d2b(char *r, double dd, int *e, int *bits)
{
    LibcBigint *b = _Balloc(r, 1);
    LibcDouble d;
    unsigned int *x = b->x;
    unsigned int y;
    unsigned int z;
    int de;
    int i;
    int k;

    d.d = dd;
    z = d.w[1] & 0xFFFFF;
    de = (d.w[1] & 0x7FFFFFFF) >> 20;
    if (de != 0) {
        z |= 0x100000;
    }
    if ((y = d.w[0]) != 0) {
        if ((k = __lo0bits(&y)) != 0) {
            x[0] = y | z << (32 - k);
            z >>= k;
        } else {
            x[0] = y;
        }
        i = b->wds = (x[1] = z) != 0 ? 2 : 1;
    } else {
        k = __lo0bits(&z);
        x[0] = z;
        i = b->wds = 1;
        k += 32;
    }
    if (de != 0) {
        *e = de - 1023 - (53 - 1) + k;
        *bits = 53 - k;
    } else {
        *e = de - 1023 - (53 - 1) + 1 + k;
        *bits = 32 * i - __hi0bits(x[i - 1]);
    }
    return b;
}
#endif

/* a / b as doubles */
#ifdef __MWERKS__
asm double __ratio(LibcBigint *a, LibcBigint *b)
{
    addiu sp, sp, -48
    sw s0, 16(sp)
    move s0, a1
    move a1, sp
    sw ra, 32(sp)
    sw s3, 28(sp)
    sw s2, 24(sp)
    sw s1, 20(sp)
    jal __b2d
    move s1, a0
    move a0, s0
    addiu a1, sp, 4
    move s2, v0
    jal __b2d
    move s3, v1
    lw a0, 16(s0)
    lw a2, 16(s1)
    lw a1, 4(sp)
    subu a2, a2, a0
    lw a0, 0(sp)
    sll a2, a2, 0x5
    subu a0, a0, a1
    addu a0, a0, a2
    blez a0, @98
    sll a1, a0, 0x14
    addu s3, a1, s3
@68:
    move a0, s2
    move a1, s3
    move a2, v0
    jal __divdf3
    move a3, v1
    lw ra, 32(sp)
    lw s3, 28(sp)
    lw s2, 24(sp)
    lw s1, 20(sp)
    lw s0, 16(sp)
    jr ra
    addiu sp, sp, 48
@98:
    j @68
    subu v1, v1, a1
}
#else
double __ratio(LibcBigint *a, LibcBigint *b)
{
    LibcDouble da;
    LibcDouble db;
    int ka;
    int kb;
    int k;

    da.d = __b2d(a, &ka);
    db.d = __b2d(b, &kb);
    k = ka - kb + 32 * (a->wds - b->wds);
    if (k > 0) {
        da.w[1] += k * 0x100000;
    } else {
        db.w[1] -= k * 0x100000;
    }
    return da.d / db.d;
}
#endif

#ifdef NON_MATCHING /* GCC: table and constants through %hi/%lo */
/* 10^n: from the table for n < 24, else by repeated multiplication */
double _mprec_log10(int n)
{
    double v = 1.0;

    if (n < 24) {
        static const double tens[24] = {
            1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11,
            1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23,
        };

        return tens[n];
    }
    while (n-- > 0) {
        v *= 10.0;
    }
    return v;
}
#endif
