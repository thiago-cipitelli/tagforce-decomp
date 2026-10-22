// @range 0x0880A5C0-0x0880C040
// @asm 0x0880A5C0-0x0880A694
// @asm 0x0880A820-0x0880A87C
// @asm 0x0880AAC4-0x0880BF10
/* newlib (GCC) of the Sony SDK libc: atexit, bsearch, _calloc_r, _close_r, the quorem and _dtoa_r
   of dtoa.c, __errno and exit. The reentrancy struct of the calling thread comes from
   libc_get_thread_module (newlib's _REENT). Functions that reach globals through %hi/%lo stay in
   the split asm (C under NON_MATCHING); the rest are the original instructions as `asm`
   functions with the portable C under `#else`. */
#include "libc.h"

typedef struct LibcAtexit {            /* newlib struct _atexit */
    struct LibcAtexit *next;
    int ind;
    void (*fns[32])(void);
    void *fnargs[32];                  /* +0x88 */
    unsigned int fntypes;              /* +0x108 bit i: fns[i] takes (status, arg) */
} LibcAtexit;

typedef struct LibcBigint {            /* newlib _Bigint */
    struct LibcBigint *next;
    int k;
    int maxwds;
    int sign;
    int wds;                           /* +0x10 */
    unsigned int x[1];                 /* +0x14 */
} LibcBigint;

void *libc_get_thread_module(void);    /* the calling thread's struct _reent */
void *heap_get_current(void);          /* newlib __getreent */
void *_malloc_r(void *r, size_t n);
int _close(int fd);
void _exit(int status);
int libc_bigint_cmp(LibcBigint *a, LibcBigint *b);

#ifdef NON_MATCHING /* GCC: loads through %hi/%lo */
int sceKernelCpuSuspendIntr(void);
void sceKernelCpuResumeIntr(int flags);

/* registers fn in the thread's atexit list (blocks of 32) */
int atexit(void (*fn)(void))
{
    int flags = sceKernelCpuSuspendIntr();
    char *r = libc_get_thread_module();
    LibcAtexit *p = *(LibcAtexit **)(r + 0x148);

    if (p == 0) {
        p = (LibcAtexit *)(r + 0x14C);
        *(LibcAtexit **)(r + 0x148) = p;
    }
    if (p->ind >= 32) {
        sceKernelCpuResumeIntr(flags);
        p = _malloc_r(r, sizeof(LibcAtexit));
        if (p == 0) {
            return -1;
        }
        flags = sceKernelCpuSuspendIntr();
        p->ind = 0;
        p->next = *(LibcAtexit **)(r + 0x148);
        p->fntypes = 0;
        *(LibcAtexit **)(r + 0x148) = p;
    }
    p->fns[p->ind++] = fn;
    sceKernelCpuResumeIntr(flags);
    return 0;
}
#endif

#ifdef __MWERKS__
asm void *bsearch(const void *key, const void *base, size_t n, size_t size, int (*cmp)(const void *, const void *))
{
    sltiu v0, a3, 1
    sltiu v1, a2, 1
    addiu sp, sp, -48
    or v1, v1, v0
    sw s7, 28(sp)
    move v0, zero
    move s7, a3
    sw s6, 24(sp)
    move s6, a0
    sw s5, 20(sp)
    move s5, a1
    sw s4, 16(sp)
    move s4, t0
    sw s3, 12(sp)
    move s3, zero
    sw s2, 8(sp)
    move s2, a2
    sw ra, 32(sp)
    sw s1, 4(sp)
    bnez v1, @9c
    sw s0, 0(sp)
    beqz a2, @a0
    lw ra, 32(sp)
    addu v0, s3, s2
@60:
    srl s0, v0, 0x1
    mult s0, s7
    move a0, s6
    mflo v1
    addu s1, s5, v1
    jalr ra, s4
    move a1, s1
    bltzl v0, @8c
    move s2, s0
    blez v0, @c8
    addiu s3, s0, 1
@8c:
    sltu v0, s3, s2
    bnez v0, @60
    addu v0, s3, s2
    move v0, zero
@9c:
    lw ra, 32(sp)
@a0:
    lw s7, 28(sp)
    lw s6, 24(sp)
    lw s5, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 48
@c8:
    j @9c
    move v0, s1
}
#else
void *bsearch(const void *key, const void *base, size_t n, size_t size, int (*cmp)(const void *, const void *))
{
    size_t lo = 0;
    size_t hi = n;

    if (n == 0 || size == 0) {
        return 0;
    }
    while (lo < hi) {
        size_t mid = (lo + hi) >> 1;
        const char *p = (const char *)base + mid * size;
        int c = cmp(key, p);

        if (c == 0) {
            return (void *)p;
        }
        if (c < 0) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    return 0;
}
#endif

/* newlib _calloc_r: the block is cleared over its usable size */
#ifdef __MWERKS__
asm void *_calloc_r(void *r, size_t n, size_t size)
{
    mult a1, a2
    addiu sp, sp, -16
    sw s0, 0(sp)
    mflo a1
    sw ra, 4(sp)
    jal _malloc_r
    nop
    move s0, v0
    beqz s0, @9c
    move v0, zero
    lw v0, -4(s0)
    move a0, s0
    move a1, zero
    ins v0, zero, 0x0, 0x2
    addiu v1, v0, -4
    sltiu v0, v1, 37
    move a3, s0
    move a2, v1
    beqz v0, @ac
    sltiu t0, v1, 20
    bnez t0, @88
    sltiu a0, v1, 28
    sltiu v0, v1, 36
    sw zero, 0(s0)
    addiu a3, s0, 8
    bnez a0, @88
    sw zero, 4(s0)
    sw zero, 8(s0)
    addiu a3, s0, 16
    bnez v0, @88
    sw zero, 12(s0)
    sw zero, 16(s0)
    addiu a3, s0, 24
    sw zero, 20(s0)
@88:
    sw zero, 0(a3)
    addiu a3, a3, 4
    sw zero, 4(a3)
    sw zero, 0(a3)
    move v0, s0
@9c:
    lw ra, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
@ac:
    jal memset
    nop
    j @9c
    move v0, s0
}
#else
void *_calloc_r(void *r, size_t n, size_t size)
{
    char *p = _malloc_r(r, n * size);

    if (p != 0) {
        memset(p, 0, (((unsigned int *)p)[-1] & ~3) - 4);
    }
    return p;
}
#endif

#ifdef NON_MATCHING /* GCC: errno global through %hi/%lo */
extern int libc_errno;                 /* 0x08E35A34 */

int _close_r(int *r, int fd)
{
    int ret;

    libc_errno = 0;
    ret = _close(fd);
    if (ret == -1 && libc_errno != 0) {
        *r = libc_errno;
    }
    return ret;
}
#endif

/* dtoa.c quorem: b -= q * S for the quotient digit q (< 10), returns q */
#ifdef __MWERKS__
asm int libc_quorem(LibcBigint *b, LibcBigint *S)
{
    addiu sp, sp, -32
    sw s4, 16(sp)
    move t5, a1
    move s4, a0
    sw ra, 28(sp)
    sw s6, 24(sp)
    sw s5, 20(sp)
    sw s3, 12(sp)
    sw s2, 8(sp)
    sw s1, 4(sp)
    sw s0, 0(sp)
    lw s2, 16(a1)
    lw v0, 16(a0)
    slt v0, v0, s2
    bnez v0, @21c
    move v1, zero
    addiu s2, s2, -1
    addiu t2, a1, 20
    sll v0, s2, 0x2
    addiu s3, a0, 20
    addu s1, t2, v0
    addu t4, s3, v0
    lw v1, 0(s1)
    lw t6, 0(t4)
    move s6, t2
    addiu v1, v1, 1
    divu t6, v1
    beqzl v1, @74
    .word 0x000001CD /* break 0x0,0x7 */
@74:
    mflo s0
    mflo s5
    beqz s0, @140
    move t1, s3
    move t3, zero
    move a0, zero
@8c:
    lw v1, 0(t2)
    lw a2, 0(t1)
    addiu t2, t2, 4
    andi v0, v1, 0xffff
    mult v0, s0
    srl v1, v1, 0x10
    andi a1, a2, 0xffff
    srl a2, a2, 0x10
    sltu a3, s1, t2
    mflo v0
    mult v1, s0
    addu v0, v0, a0
    andi a0, v0, 0xffff
    srl v0, v0, 0x10
    subu a1, a1, a0
    mflo v1
    addu v1, v1, v0
    addu t0, a1, t3
    andi v0, v1, 0xffff
    sra t3, t0, 0x10
    subu a2, a2, v0
    addu a1, a2, t3
    sh a1, 2(t1)
    srl a0, v1, 0x10
    sra t3, a1, 0x10
    sh t0, 0(t1)
    beqz a3, @8c
    addiu t1, t1, 4
    bnezl t6, @144
    move a1, t5
    addiu t4, t4, -4
    sltu v0, s3, t4
    beqzl v0, @140
    sw s2, 16(s4)
    lw v0, 0(t4)
    bnezl v0, @140
    sw s2, 16(s4)
    addiu t4, t4, -4
@124:
    sltu v0, s3, t4
    beqz v0, @13c
    addiu s2, s2, -1
    lw v0, 0(t4)
    beqz v0, @124
    addiu t4, t4, -4
@13c:
    sw s2, 16(s4)
@140:
    move a1, t5
@144:
    jal libc_bigint_cmp
    move a0, s4
    bltz v0, @21c
    move v1, s5
    addiu s5, s0, 1
    move t2, s6
    move t3, zero
    move a0, zero
    move t1, s3
@168:
    lw v1, 0(t2)
    lw a1, 0(t1)
    addiu t2, t2, 4
    andi v0, v1, 0xffff
    addu v0, v0, a0
    srl a0, v0, 0x10
    andi a2, v0, 0xffff
    srl v1, v1, 0x10
    andi v0, a1, 0xffff
    addu v1, v1, a0
    subu v0, v0, a2
    addu t0, v0, t3
    srl a1, a1, 0x10
    andi v0, v1, 0xffff
    subu a1, a1, v0
    sra t3, t0, 0x10
    addu a1, a1, t3
    sltu v0, s1, t2
    sh a1, 2(t1)
    srl a0, v1, 0x10
    sra t3, a1, 0x10
    sh t0, 0(t1)
    beqz v0, @168
    addiu t1, t1, 4
    sll v0, s2, 0x2
    addu t4, s3, v0
    lw v1, 0(t4)
    bnezl v1, @21c
    move v1, s5
    addiu t4, t4, -4
    sltu v0, s3, t4
    beqzl v0, @218
    sw s2, 16(s4)
    lw v0, 0(t4)
    bnezl v0, @218
    sw s2, 16(s4)
    addiu t4, t4, -4
@1fc:
    sltu v0, s3, t4
    beqz v0, @214
    addiu s2, s2, -1
    lw v0, 0(t4)
    beqzl v0, @1fc
    addiu t4, t4, -4
@214:
    sw s2, 16(s4)
@218:
    move v1, s5
@21c:
    lw ra, 28(sp)
    lw s6, 24(sp)
    lw s5, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 32
}
#else
int libc_quorem(LibcBigint *b, LibcBigint *S)
{
    int n = S->wds;
    unsigned int *bx;
    unsigned int *bxe;
    unsigned int *sx;
    unsigned int *sxe;
    unsigned int q;
    unsigned int carry;
    int borrow;

    if (b->wds < n) {
        return 0;
    }
    sx = S->x;
    sxe = sx + --n;
    bx = b->x;
    bxe = bx + n;
    q = *bxe / (*sxe + 1);
    if (q != 0) {
        borrow = 0;
        carry = 0;
        do {
            unsigned int si = *sx++;
            unsigned int ys = (si & 0xFFFF) * q + carry;
            unsigned int zs = (si >> 16) * q + (ys >> 16);
            int y = (int)(*bx & 0xFFFF) - (int)(ys & 0xFFFF) + borrow;
            int z;

            carry = zs >> 16;
            borrow = y >> 16;
            z = (int)(*bx >> 16) - (int)(zs & 0xFFFF) + borrow;
            borrow = z >> 16;
            *bx++ = (unsigned int)(z & 0xFFFF) << 16 | (y & 0xFFFF);
        } while (sx <= sxe);
        if (*bxe == 0) {
            bx = b->x;
            while (--bxe > bx && *bxe == 0) {
                --n;
            }
            b->wds = n;
        }
    }
    if (libc_bigint_cmp(b, S) >= 0) {
        q++;
        borrow = 0;
        bx = b->x;
        sx = S->x;
        do {
            unsigned int si = *sx++;
            int y = (int)(*bx & 0xFFFF) - (int)(si & 0xFFFF) + borrow;
            int z;

            borrow = y >> 16;
            z = (int)(*bx >> 16) - (int)(si >> 16) + borrow;
            borrow = z >> 16;
            *bx++ = (unsigned int)(z & 0xFFFF) << 16 | (y & 0xFFFF);
        } while (sx <= sxe);
        bx = b->x;
        bxe = bx + n;
        if (*bxe == 0) {
            while (--bxe > bx && *bxe == 0) {
                --n;
            }
            b->wds = n;
        }
    }
    return q;
}
#endif

#ifdef NON_MATCHING /* GCC: constant tables through %hi/%lo */
int snprintf(char *buf, size_t n, const char *fmt, ...);
char *strchr(const char *s, int c);
int atoi(const char *s);

static int dtoa_neg(double d)
{
    union {
        double d;
        unsigned int w[2];
    } u;

    u.d = d;
    return (u.w[1] >> 31) != 0;
}

/* newlib _dtoa_r: digits of d for printf (mode 0 shortest, 2 = ndigits significant, 3 = ndigits
   after the point), *decpt = position of the point, *sign, *rve = end of the digits.
   ponytail: built on the host snprintf; mode 0 gives 17 significant digits with the trailing
   zeros removed instead of the shortest round-trip string. */
char *_dtoa_r(void *r, double d, int mode, int ndigits, int *decpt, int *sign, char **rve)
{
    static char buf[400];
    char tmp[400];
    char *p;
    char *q;
    int e;

    *sign = dtoa_neg(d);
    if (*sign) {
        d = -d;
    }
    if (d != d || d - d != d - d) {
        *decpt = 9999;
        strcpy(buf, d == d ? "Infinity" : "NaN");
        if (rve != 0) {
            *rve = buf + strlen(buf);
        }
        return buf;
    }
    if (d == 0.0) {
        *decpt = 1;
        strcpy(buf, "0");
        if (rve != 0) {
            *rve = buf + 1;
        }
        return buf;
    }
    if (mode == 3) {
        snprintf(tmp, sizeof(tmp), "%.*e", 0, d);
        e = atoi(strchr(tmp, 'e') + 1);
        ndigits += e + 1;
        if (ndigits <= 0) {
            *decpt = -ndigits;
            buf[0] = 0;
            if (rve != 0) {
                *rve = buf;
            }
            return buf;
        }
    } else if (mode <= 1 || ndigits <= 0) {
        ndigits = mode <= 1 ? 17 : 1;
    }
    snprintf(tmp, sizeof(tmp), "%.*e", ndigits - 1, d);
    e = atoi(strchr(tmp, 'e') + 1);
    for (p = tmp, q = buf; *p != 'e'; p++) {
        if (*p >= '0' && *p <= '9') {
            *q++ = *p;
        }
    }
    while (q > buf + 1 && q[-1] == '0') {
        q--;
    }
    *q = 0;
    *decpt = e + 1;
    if (rve != 0) {
        *rve = q;
    }
    return buf;
}
#endif

/* newlib __errno: errno is the first word of the current reent struct */
#ifdef __MWERKS__
asm int *__errno(void)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    jal heap_get_current
    nop
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 16
    nop
    nop
    nop
    nop
    nop
}
#else
int *__errno(void)
{
    return heap_get_current();
}
#endif

/* runs the atexit handlers in reverse order (with their argument when registered by on_exit),
   then _exit */
#ifdef __MWERKS__
asm void exit(int status)
{
    addiu sp, sp, -48
    sw s6, 24(sp)
    move s6, a0
    sw s5, 20(sp)
    sw ra, 32(sp)
    sw s7, 28(sp)
    sw s4, 16(sp)
    sw s3, 12(sp)
    sw s2, 8(sp)
    sw s1, 4(sp)
    jal libc_get_thread_module
    sw s0, 0(sp)
    lw s5, 328(v0)
    beqz s5, @dc
    addiu s7, zero, 1
    lw v0, 4(s5)
@40:
    move s3, zero
    addiu s4, s5, 136
    addiu s0, v0, -1
    sllv v1, s7, s0
    slti v0, s0, 0
    bltz s0, @b4
    movz s3, v1, v0
    sll v1, s0, 0x2
    addu v0, v1, s5
    addiu s1, v0, 8
    move s2, v1
    nop
    nop
    nop
    nop
    nop
@80:
    lw v0, 128(s4)
    addu a1, s2, s4
    and v0, v0, s3
    beqz v0, @c8
    move a0, s6
    lw v0, 0(s1)
    jalr ra, v0
    lw a1, 0(a1)
    addiu s0, s0, -1
    addiu s2, s2, -4
@a8:
    addiu s1, s1, -4
    bgez s0, @80
    sra s3, s3, 0x1
@b4:
    lw s5, 0(s5)
    bnezl s5, @40
    lw v0, 4(s5)
    j @dc
    nop
@c8:
    lw v0, 0(s1)
    jalr ra, v0
    addiu s0, s0, -1
    j @a8
    addiu s2, s2, -4
@dc:
    jal _exit
    move a0, s6
    nop
    nop
    nop
    nop
    nop
    nop
    nop
}
#else
void exit(int status)
{
    char *r = libc_get_thread_module();
    LibcAtexit *p;

    for (p = *(LibcAtexit **)(r + 0x148); p != 0; p = p->next) {
        int i;

        for (i = p->ind - 1; i >= 0; i--) {
            if (p->fntypes & (1u << i)) {
                ((void (*)(int, void *))p->fns[i])(status, p->fnargs[i]);
            } else {
                ((void (*)(int))p->fns[i])(status);
            }
        }
    }
    _exit(status);
}
#endif
