// @range 0x0880E700-0x0880F60C
// @gcc 0x0880E700-0x0880EA14
// @gcc 0x0880EA9C-0x0880F368
// @gcc 0x0880F4CC-0x0880F60C
/* newlib mprec.c (GCC): the multi-precision integers of dtoa/strtod. Bigints come from a per-reent
   freelist of power-of-two sizes (+0x4C of the reent) and the powers 5^(4 * 2^k) are cached at
   +0x48. On the PSP the functions are newlib's own source compiled by the SDK's GCC (libc_mprec_gcc.c, the
   `@gcc` ranges), except __hi0bits and __d2b, kept here as the original instructions: they need a later
   revision of that compiler. The C here is the PC port's and follows newlib's mprec.c (16-bit halves, as built
   without long long). */
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

#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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

#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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
#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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
LibcBigint *__s2b(char *r, const char *s, int nd0, int nd, unsigned int y9);
#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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

#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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

#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
LibcBigint *__i2b(char *r, int i)
{
    LibcBigint *b = _Balloc(r, 1);

    b->x[0] = i;
    b->wds = 1;
    return b;
}
#endif

#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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
LibcBigint *__pow5mult(char *r, LibcBigint *b, int k);
#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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
LibcBigint *__lshift(char *r, LibcBigint *b, int k);
#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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
#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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
LibcBigint *__mdiff(char *r, LibcBigint *a, LibcBigint *b);
#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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
double __ulp(double x);
#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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
#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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
double __ratio(LibcBigint *a, LibcBigint *b);
#ifndef __MWERKS__ /* PSP: libc_mprec_gcc.c */
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

#if !defined(__MWERKS__) && defined(NON_MATCHING) /* PSP: libc_mprec_gcc.c */
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
