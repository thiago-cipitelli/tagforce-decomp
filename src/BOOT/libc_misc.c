// @range 0x0880F60C-0x08810BA8
// @asm 0x08810038-0x088105F8
// @asm 0x088106E0-0x08810854
// @asm 0x08810940-0x08810B38
/* newlib (GCC) of the Sony SDK libc, in link order: _printf_r, printf, qsort (Bentley-McIlroy),
   srand/rand (the 64-bit LCG of newlib), _read_r, _realloc_r, reent.c (cleanup_glue,
   _reclaim_reent, _wrapup_reent), refill.c (lflush, __srefill), _sbrk_r and _snprintf_r. Functions
   that reach globals through %hi/%lo stay in the split asm (C under NON_MATCHING); the rest are the
   original instructions with the portable C under `#else`. */
#include "libc.h"
#include "stdarg.h"

typedef struct LibcFile {              /* newlib FILE (see libc_stdio.c) */
    unsigned char *p;
    int r;
    int w;
    short flags;
    short file;
    unsigned char *base;
    int size;
    int lbfsize;
    void *cookie;
    int (*read)(void *, char *, int);  /* +0x20 */
    int (*write)(void *, const char *, int);
    void *seek;
    void *close;
    unsigned char *ub_base;            /* +0x30 ungetc buffer */
    int ub_size;
    unsigned char *up;                 /* +0x38 */
    int ur;                            /* +0x3C */
    unsigned char ubuf[3];             /* +0x40 */
    char pad43[0x19];
} LibcFile;

void *heap_get_current(void);
int _vfprintf_r(void *r, LibcFile *fp, const char *fmt, va_list ap);
int libc_vfprintf(LibcFile *fp, const char *fmt, va_list ap);
void _free_r(void *r, void *p);
void libc_cleanup_glue(void *r, void *glue);
int fflush(LibcFile *fp);

/* _printf_r: printf on r's stdout */
#ifdef __MWERKS__
asm int _printf_r(char *r, const char *fmt, ...)
{
    addiu sp, sp, -48
    sw ra, 0(sp)
    move v1, a1
    lw a1, 8(a0)
    sw a2, 24(sp)
    move a2, v1
    sw a3, 28(sp)
    addiu a3, sp, 24
    sw t0, 32(sp)
    sw t1, 36(sp)
    sw t2, 40(sp)
    jal _vfprintf_r
    sw t3, 44(sp)
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 48
}
#else
int _printf_r(char *r, const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = _vfprintf_r(r, *(LibcFile **)(r + 8), fmt, ap);
    va_end(ap);
    return n;
}
#endif

/* printf (exported as dbg_printf) */
#ifdef __MWERKS__
asm int dbg_printf(const char *fmt, ...)
{
    addiu sp, sp, -48
    sw ra, 4(sp)
    sw s0, 0(sp)
    move s0, a0
    sw a1, 20(sp)
    sw a2, 24(sp)
    sw a3, 28(sp)
    sw t0, 32(sp)
    sw t1, 36(sp)
    sw t2, 40(sp)
    jal heap_get_current
    sw t3, 44(sp)
    move a1, s0
    lw a0, 8(v0)
    jal libc_vfprintf
    addiu a2, sp, 20
    lw ra, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 48
}
#else
int dbg_printf(const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = libc_vfprintf(*(LibcFile **)((char *)heap_get_current() + 8), fmt, ap);
    va_end(ap);
    return n;
}
#endif

#ifndef __MWERKS__
/* swaps of the BSD qsort: by int when everything is int-aligned, else by byte */
static void qsort_swapfunc(char *a, char *b, int n, int swaptype)
{
    if (swaptype <= 1) {
        int i = n / (int)sizeof(int);
        int *pi = (int *)a;
        int *pj = (int *)b;

        do {
            int t = *pi;

            *pi++ = *pj;
            *pj++ = t;
        } while (--i > 0);
    } else {
        int i = n;
        char *pi = a;
        char *pj = b;

        do {
            char t = *pi;

            *pi++ = *pj;
            *pj++ = t;
        } while (--i > 0);
    }
}

#define QSORT_SWAP(a, b)                                                                     \
    do {                                                                                     \
        if (swaptype == 0) {                                                                 \
            int t_ = *(int *)(a);                                                            \
            *(int *)(a) = *(int *)(b);                                                       \
            *(int *)(b) = t_;                                                                \
        } else {                                                                             \
            qsort_swapfunc(a, b, (int)es, swaptype);                                         \
        }                                                                                    \
    } while (0)

static char *qsort_med3(char *a, char *b, char *c, int (*cmp)(const void *, const void *))
{
    return cmp(a, b) < 0 ? (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
                         : (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
}
#endif

/* newlib qsort (BSD, Bentley-McIlroy): median of three (of nine above 40 elements), three-way
   partition, insertion sort below 7 elements or when a pass swapped nothing */
#ifdef __MWERKS__
asm void qsort(void *base, size_t n, size_t es, int (*cmp)(const void *, const void *))
{
    addiu sp, sp, -96
    sw ra, 84(sp)
    sw s8, 80(sp)
    sw s7, 76(sp)
    sw s3, 60(sp)
    sw s2, 56(sp)
    sw s1, 52(sp)
    sw s0, 48(sp)
    sw a1, 0(sp)
    sw s6, 72(sp)
    move s6, a3
    sw s5, 68(sp)
    move s5, a0
    sw s4, 64(sp)
    move s4, a2
@3c:
    andi v0, s5, 0x3
    bnez v0, @5c
    addiu s8, zero, 2
    xori v1, s4, 0x4
    andi v0, s4, 0x3
    beqz v0, @5c
    sltu s8, zero, v1
    addiu s8, zero, 2
@5c:
    lw v1, 0(sp)
    sltiu v0, v1, 7
    bnez v0, @7f0
    sw zero, 8(sp)
    lw v1, 0(sp)
    lw a0, 0(sp)
    srl v0, v1, 0x1
    mult v0, s4
    sltiu v1, v1, 8
    addiu s3, a0, -1
    mflo v0
    bnez v1, @d0
    addu s2, s5, v0
    mult s3, s4
    sltiu v1, a0, 41
    move s0, s5
    mflo v0
    beqz v1, @64c
    addu s7, s5, v0
@a8:
    move a0, s0
    jalr ra, s6
    move a1, s2
    bltz v0, @620
    move a0, s2
    jalr ra, s6
    move a1, s7
    blez v0, @604
    move a0, s2
@cc:
    move s2, a0
@d0:
    bnez s8, @59c
    slti v0, s8, 2
    lw v1, 0(s5)
    lw v0, 0(s2)
    sw v0, 0(s5)
    sw v1, 0(s2)
    mult s3, s4
@ec:
    addu s3, s5, s4
    slti v1, s8, 2
    move a3, s3
    sw s3, 28(sp)
    mflo v0
    addu s2, s5, v0
    move s1, s2
    sw v1, 24(sp)
@10c:
    sltu s0, s1, s3
    beqzl s0, @4ec
    move a1, s5
@118:
    slti s7, s8, 2
@11c:
    bnez s0, @264
    move a1, s5
    move a0, s1
    jalr ra, s6
    sw a3, 32(sp)
    move a1, v0
    bltz v0, @1d4
    lw a3, 32(sp)
    bnezl a1, @168
    subu s1, s1, s4
    addiu v1, zero, 1
    bnez s8, @170
    sw v1, 8(sp)
    lw v1, 0(s1)
    lw v0, 0(s2)
    sw v0, 0(s1)
    sw v1, 0(s2)
    subu s2, s2, s4
@164:
    subu s1, s1, s4
@168:
    j @11c
    sltu s0, s1, s3
@170:
    move a2, s4
    move a1, s1
    beqz s7, @1ac
    move a0, s2
    srl a2, s4, 0x2
@184:
    lw v1, 0(a1)
    lw v0, 0(a0)
    addiu a2, a2, -1
    sw v0, 0(a1)
    addiu a1, a1, 4
    sw v1, 0(a0)
    bgtz a2, @184
    addiu a0, a0, 4
    j @164
    subu s2, s2, s4
@1ac:
    lb v1, 0(a1)
    lbu v0, 0(a0)
    addiu a2, a2, -1
    sb v0, 0(a1)
    addiu a1, a1, 1
    sb v1, 0(a0)
    bgtz a2, @1ac
    addiu a0, a0, 1
    j @164
    subu s2, s2, s4
@1d4:
    bnez s8, @200
    lw v0, 24(sp)
    lw v1, 0(s3)
    lw v0, 0(s1)
    sw v0, 0(s3)
    sw v1, 0(s1)
    addiu v1, zero, 1
@1f0:
    addu s3, s3, s4
    subu s1, s1, s4
    j @10c
    sw v1, 8(sp)
@200:
    move a2, s4
    move a1, s3
    beqz v0, @23c
    move a0, s1
    srl a2, s4, 0x2
@214:
    lw v1, 0(a1)
    lw v0, 0(a0)
    addiu a2, a2, -1
    sw v0, 0(a1)
    addiu a1, a1, 4
    sw v1, 0(a0)
    bgtz a2, @214
    addiu a0, a0, 4
    j @1f0
    addiu v1, zero, 1
@23c:
    lb v1, 0(a1)
    lbu v0, 0(a0)
    addiu a2, a2, -1
    sb v0, 0(a1)
    addiu a1, a1, 1
    sb v1, 0(a0)
    bgtz a2, @23c
    addiu a0, a0, 1
    j @1f0
    addiu v1, zero, 1
@264:
    lw a0, 8(sp)
    beqz a0, @408
    lw v0, 0(sp)
    subu a1, s3, a3
    move t0, a1
    mult v0, s4
    subu v0, a3, s5
    min a1, a1, v0
    mflo v1
    blez a1, @2c8
    addu s7, s5, v1
    subu v1, s3, a1
    slti v0, s8, 2
    move a3, a1
    move a2, s5
    bnez v0, @3d4
    move a0, v1
@2a8:
    lb v1, 0(a2)
    lbu v0, 0(a0)
    addiu a3, a3, -1
    sb v0, 0(a2)
    addiu a2, a2, 1
    sb v1, 0(a0)
    bgtz a3, @2a8
    addiu a0, a0, 1
@2c8:
    subu v1, s7, s2
@2cc:
    subu s0, s2, s1
    subu a1, v1, s4
    sltu v0, s0, a1
    movn a1, s0, v0
    blez a1, @318
    subu v1, s7, a1
    slti v0, s8, 2
    move a3, a1
    move a2, s3
    bnez v0, @3a0
    move a0, v1
@2f8:
    lb v1, 0(a2)
    lbu v0, 0(a0)
    addiu a3, a3, -1
    sb v0, 0(a2)
    addiu a2, a2, 1
    sb v1, 0(a0)
    bgtz a3, @2f8
    addiu a0, a0, 1
@318:
    sltu v0, s4, t0
@31c:
    bnezl v0, @378
    divu t0, s4
    sltu v0, s4, s0
@328:
    beqz v0, @348
    subu s5, s7, s0
    divu s0, s4
    beqzl s4, @33c
    .word 0x000001CD /* break 0x0,0x7 */
@33c:
    mflo v0
    j @3c
    sw v0, 0(sp)
@348:
    lw ra, 84(sp)
@34c:
    lw s8, 80(sp)
    lw s7, 76(sp)
    lw s6, 72(sp)
    lw s5, 68(sp)
    lw s4, 64(sp)
    lw s3, 60(sp)
    lw s2, 56(sp)
    lw s1, 52(sp)
    lw s0, 48(sp)
    jr ra
    addiu sp, sp, 96
@378:
    move a0, s5
    move a2, s4
    move a3, s6
    beqzl s4, @38c
    .word 0x000001CD /* break 0x0,0x7 */
@38c:
    mflo a1
    jal qsort
    nop
    j @328
    sltu v0, s4, s0
@3a0:
    srl a1, a1, 0x2
    move a0, s3
    move a2, v1
@3ac:
    lw v1, 0(a0)
    lw v0, 0(a2)
    addiu a1, a1, -1
    sw v0, 0(a0)
    addiu a0, a0, 4
    sw v1, 0(a2)
    bgtz a1, @3ac
    addiu a2, a2, 4
    j @31c
    sltu v0, s4, t0
@3d4:
    srl a1, a1, 0x2
    move a2, v1
    move a0, s5
@3e0:
    lw v1, 0(a0)
    lw v0, 0(a2)
    addiu a1, a1, -1
    sw v0, 0(a0)
    addiu a0, a0, 4
    sw v1, 0(a2)
    bgtz a1, @3e0
    addiu a2, a2, 4
    j @2cc
    subu v1, s7, s2
@408:
    lw s2, 28(sp)
    mult v0, s4
    mflo v1
    addu v1, s5, v1
    sltu v0, s2, v1
    beqz v0, @348
    move s7, v1
    slti s3, s8, 2
    move s0, s2
@42c:
    sltu v0, s5, s0
    beqz v0, @4d4
    subu s1, s0, s4
    move a0, s1
    jalr ra, s6
    move a1, s0
    blezl v0, @4d8
    addu s2, s2, s4
    bnez s8, @46c
    move a2, s4
    lw v1, 0(s0)
    lw v0, 0(s1)
    sw v0, 0(s0)
    sw v1, 0(s1)
    j @42c
    move s0, s1
@46c:
    move a1, s0
    beqz s3, @4ac
    move a0, s1
    move a0, s0
    srl a1, s4, 0x2
    move a2, s1
@484:
    lw v1, 0(a0)
    lw v0, 0(a2)
    addiu a1, a1, -1
    sw v0, 0(a0)
    addiu a0, a0, 4
    sw v1, 0(a2)
    bgtz a1, @484
    addiu a2, a2, 4
    j @42c
    move s0, s1
@4ac:
    lb v1, 0(a1)
    lbu v0, 0(a0)
    addiu a2, a2, -1
    sb v0, 0(a1)
    addiu a1, a1, 1
    sb v1, 0(a0)
    bgtz a2, @4ac
    addiu a0, a0, 1
    j @42c
    move s0, s1
@4d4:
    addu s2, s2, s4
@4d8:
    sltu v0, s2, s7
    bnez v0, @42c
    move s0, s2
    j @34c
    lw ra, 84(sp)
@4ec:
    move a0, s3
    jalr ra, s6
    sw a3, 32(sp)
    move a1, v0
    bgtz v0, @118
    lw a3, 32(sp)
    bnezl a1, @10c
    addu s3, s3, s4
    addiu a0, zero, 1
    bnez s8, @534
    sw a0, 8(sp)
    lw v1, 0(a3)
    lw v0, 0(s3)
    sw v0, 0(a3)
    sw v1, 0(s3)
    addu a3, a3, s4
@52c:
    j @10c
    addu s3, s3, s4
@534:
    lw v0, 24(sp)
    move a2, s4
    move a1, a3
    beqz v0, @574
    move a0, s3
    srl a2, s4, 0x2
@54c:
    lw v1, 0(a1)
    lw v0, 0(a0)
    addiu a2, a2, -1
    sw v0, 0(a1)
    addiu a1, a1, 4
    sw v1, 0(a0)
    bgtz a2, @54c
    addiu a0, a0, 4
    j @52c
    addu a3, a3, s4
@574:
    lb v1, 0(a1)
    lbu v0, 0(a0)
    addiu a2, a2, -1
    sb v0, 0(a1)
    addiu a1, a1, 1
    sb v1, 0(a0)
    bgtz a2, @574
    addiu a0, a0, 1
    j @52c
    addu a3, a3, s4
@59c:
    move a2, s4
    move a1, s5
    bnez v0, @5d4
    move a0, s2
@5ac:
    lb v1, 0(a1)
    lbu v0, 0(a0)
    addiu a2, a2, -1
    sb v0, 0(a1)
    addiu a1, a1, 1
    sb v1, 0(a0)
    bgtz a2, @5ac
    addiu a0, a0, 1
    j @ec
    mult s3, s4
@5d4:
    srl a1, s4, 0x2
    move a2, s5
@5dc:
    lw v1, 0(a2)
    lw v0, 0(a0)
    addiu a1, a1, -1
    sw v0, 0(a2)
    addiu a2, a2, 4
    sw v1, 0(a0)
    bgtz a1, @5dc
    addiu a0, a0, 4
    j @ec
    mult s3, s4
@604:
    move a0, s0
    jalr ra, s6
    move a1, s7
    slti v0, v0, 0
    move a0, s0
    j @cc
    movz a0, s7, v0
@620:
    jalr ra, s6
    move a1, s7
    bltz v0, @cc
    move a0, s2
    move a0, s0
    jalr ra, s6
    move a1, s7
    slti v0, v0, 0
    move a0, s7
    j @cc
    movz a0, s0, v0
@64c:
    srl v0, a0, 0x3
    mult v0, s4
    move a0, s5
    mflo v1
    addu s0, s5, v1
    move a1, s0
    jalr ra, s6
    sw v1, 4(sp)
    lw a0, 4(sp)
    sll a0, a0, 0x1
    sw a0, 12(sp)
    bltz v0, @7c0
    addu s1, s5, a0
    move a0, s0
    jalr ra, s6
    move a1, s1
    blez v0, @7a4
    move v1, s0
@694:
    lw v0, 4(sp)
    move a1, s2
    move s0, v1
    subu v0, s2, v0
    move a0, v0
    jalr ra, s6
    sw v0, 16(sp)
    lw v1, 4(sp)
    bltz v0, @774
    addu s1, s2, v1
    move a0, s2
    jalr ra, s6
    move a1, s1
    blez v0, @758
    move v1, s2
@6d0:
    lw v0, 4(sp)
    lw a0, 12(sp)
    move s2, v1
    subu s1, s7, v0
    subu a0, s7, a0
    move a1, s1
    jalr ra, s6
    sw a0, 20(sp)
    bltz v0, @72c
    move a0, s1
    jalr ra, s6
    move a1, s7
    blez v0, @710
    move a0, s1
@708:
    j @a8
    move s7, a0
@710:
    lw a0, 20(sp)
    jalr ra, s6
    move a1, s7
    lw a0, 20(sp)
    slti v0, v0, 0
    j @708
    movz a0, s7, v0
@72c:
    jalr ra, s6
    move a1, s7
    bltz v0, @708
    move a0, s1
    lw a0, 20(sp)
    jalr ra, s6
    move a1, s7
    lw a0, 20(sp)
    slti v0, v0, 0
    j @708
    movn a0, s7, v0
@758:
    lw a0, 16(sp)
    jalr ra, s6
    move a1, s1
    lw v1, 16(sp)
    slti v0, v0, 0
    j @6d0
    movz v1, s1, v0
@774:
    move a0, s2
    jalr ra, s6
    move a1, s1
    bltz v0, @6d0
    move v1, s2
    lw a0, 16(sp)
    jalr ra, s6
    move a1, s1
    lw v1, 16(sp)
    slti v0, v0, 0
    j @6d0
    movn v1, s1, v0
@7a4:
    move a0, s5
    jalr ra, s6
    move a1, s1
    slti v0, v0, 0
    move v1, s5
    j @694
    movz v1, s1, v0
@7c0:
    move a0, s0
    jalr ra, s6
    move a1, s1
    bltz v0, @694
    move v1, s0
    move a0, s5
    jalr ra, s6
    move a1, s1
    slti v0, v0, 0
    move v1, s1
    j @694
    movz v1, s5, v0
@7f0:
    mult v1, s4
    addu s2, s5, s4
    mflo v1
    addu v1, s5, v1
    sltu v0, s2, v1
    beqz v0, @348
    move s7, v1
    slti s3, s8, 2
    move s0, s2
@814:
    sltu v0, s5, s0
    beqz v0, @8bc
    subu s1, s0, s4
    move a0, s1
    jalr ra, s6
    move a1, s0
    blezl v0, @8c0
    addu s2, s2, s4
    bnez s8, @854
    move a2, s4
    lw v1, 0(s0)
    lw v0, 0(s1)
    sw v0, 0(s0)
    sw v1, 0(s1)
    j @814
    move s0, s1
@854:
    move a1, s0
    beqz s3, @894
    move a0, s1
    move a0, s0
    srl a1, s4, 0x2
    move a2, s1
@86c:
    lw v1, 0(a0)
    lw v0, 0(a2)
    addiu a1, a1, -1
    sw v0, 0(a0)
    addiu a0, a0, 4
    sw v1, 0(a2)
    bgtz a1, @86c
    addiu a2, a2, 4
    j @814
    move s0, s1
@894:
    lb v1, 0(a1)
    lbu v0, 0(a0)
    addiu a2, a2, -1
    sb v0, 0(a1)
    addiu a1, a1, 1
    sb v1, 0(a0)
    bgtz a2, @894
    addiu a0, a0, 1
    j @814
    move s0, s1
@8bc:
    addu s2, s2, s4
@8c0:
    sltu v0, s2, s7
    bnez v0, @814
    move s0, s2
    j @34c
    lw ra, 84(sp)
}
#else
void qsort(void *base, size_t n, size_t es, int (*cmp)(const void *, const void *))
{
    char *a = base;
    char *pa;
    char *pb;
    char *pc;
    char *pd;
    char *pl;
    char *pm;
    char *pn;
    int d;
    int r;
    int swaptype;
    int swap_cnt;

loop:
    swaptype = ((unsigned int)(a - (char *)0) % sizeof(int)) || es % sizeof(int) ? 2 : es == sizeof(int) ? 0 : 1;
    swap_cnt = 0;
    if (n < 7) {
        for (pm = a + es; pm < a + n * es; pm += es) {
            for (pl = pm; pl > a && cmp(pl - es, pl) > 0; pl -= es) {
                QSORT_SWAP(pl, pl - es);
            }
        }
        return;
    }
    pm = a + (n / 2) * es;
    if (n > 7) {
        pl = a;
        pn = a + (n - 1) * es;
        if (n > 40) {
            d = (n / 8) * es;
            pl = qsort_med3(pl, pl + d, pl + 2 * d, cmp);
            pm = qsort_med3(pm - d, pm, pm + d, cmp);
            pn = qsort_med3(pn - 2 * d, pn - d, pn, cmp);
        }
        pm = qsort_med3(pl, pm, pn, cmp);
    }
    QSORT_SWAP(a, pm);
    pa = pb = a + es;
    pc = pd = a + (n - 1) * es;
    for (;;) {
        while (pb <= pc && (r = cmp(pb, a)) <= 0) {
            if (r == 0) {
                swap_cnt = 1;
                QSORT_SWAP(pa, pb);
                pa += es;
            }
            pb += es;
        }
        while (pb <= pc && (r = cmp(pc, a)) >= 0) {
            if (r == 0) {
                swap_cnt = 1;
                QSORT_SWAP(pc, pd);
                pd -= es;
            }
            pc -= es;
        }
        if (pb > pc) {
            break;
        }
        QSORT_SWAP(pb, pc);
        swap_cnt = 1;
        pb += es;
        pc -= es;
    }
    if (swap_cnt == 0) {
        for (pm = a + es; pm < a + n * es; pm += es) {
            for (pl = pm; pl > a && cmp(pl - es, pl) > 0; pl -= es) {
                QSORT_SWAP(pl, pl - es);
            }
        }
        return;
    }
    pn = a + n * es;
    r = pa - a < pb - pa ? pa - a : pb - pa;
    if (r > 0) {
        qsort_swapfunc(a, pb - r, r, swaptype);
    }
    r = pd - pc < pn - pd - (int)es ? pd - pc : pn - pd - (int)es;
    if (r > 0) {
        qsort_swapfunc(pb, pn - r, r, swaptype);
    }
    if ((r = pb - pa) > (int)es) {
        qsort(a, r / es, es, cmp);
    }
    if ((r = pd - pc) > (int)es) {
        a = pn - r;
        n = r / es;
        goto loop;
    }
}
#endif

#ifdef __MWERKS__
asm void srand(unsigned int seed)
{
    addiu sp, sp, -16
    sw ra, 8(sp)
    sw s1, 4(sp)
    move s1, zero
    sw s0, 0(sp)
    jal heap_get_current
    move s0, a0
    sw s0, 168(v0)
    sw s1, 172(v0)
    lw ra, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
void srand(unsigned int seed)
{
    char *r = heap_get_current();

    *(unsigned int *)(r + 0xA8) = seed;
    *(unsigned int *)(r + 0xAC) = 0;
}
#endif

/* newlib rand: next = next * 6364136223846793005 + 1, returns bits 32-62 */
#ifdef __MWERKS__
asm int rand_next(void)
{
    addiu sp, sp, -16
    sw ra, 4(sp)
    jal heap_get_current
    sw s0, 0(sp)
    jal heap_get_current
    move s0, v0
    lw a2, 168(v0)
    lw a3, 172(v0)
    lui a0, 0x4c95
    ori a0, a0, 0x7f2d
    lui a1, 0x5851
    ori a1, a1, 0xf42d
    mult a2, a1
    mflo t0
    multu a2, a0
    mfhi v1
    mflo v0
    mult a3, a0
    addu t0, t0, v1
    addiu v0, v0, 1
    sltiu a2, v0, 1
    mflo a0
    addu v1, t0, a0
    addu v1, v1, a2
    sw v0, 168(s0)
    sw v1, 172(s0)
    jal heap_get_current
    nop
    lw v1, 172(v0)
    lw v0, 168(v0)
    lw ra, 4(sp)
    lw s0, 0(sp)
    srl v0, v1, 0x0
    ins v0, zero, 0x1f, 0x1
    jr ra
    addiu sp, sp, 16
}
#else
int rand_next(void)
{
    char *r = heap_get_current();
    unsigned long long next = (unsigned long long)*(unsigned int *)(r + 0xAC) << 32 | *(unsigned int *)(r + 0xA8);

    next = next * 6364136223846793005ULL + 1;
    *(unsigned int *)(r + 0xA8) = (unsigned int)next;
    *(unsigned int *)(r + 0xAC) = (unsigned int)(next >> 32);
    return (int)((next >> 32) & 0x7FFFFFFF);
}
#endif

#ifdef NON_MATCHING /* GCC: errno and dlmalloc state through %hi/%lo */
int _read(int fd, void *buf, int n);
extern int libc_errno;                 /* 0x08E35A34 */

int _read_r(int *r, int fd, void *buf, int n)
{
    int ret;

    libc_errno = 0;
    ret = _read(fd, buf, n);
    if (ret == -1 && libc_errno != 0) {
        *r = libc_errno;
    }
    return ret;
}

void *_realloc_r(void *r, void *p, size_t n)
{
    return platform_realloc(p, n);
}
#endif

/* reent.c cleanup_glue: frees the chain of stream blocks, last first */
#ifdef __MWERKS__
asm void libc_cleanup_glue(void *r, void *glue)
{
    addiu sp, sp, -16
    sw s1, 4(sp)
    move s1, a1
    sw s0, 0(sp)
    move s0, a0
    sw ra, 8(sp)
    lw v0, 0(a1)
    bnez v0, @44
    move a1, v0
    move a0, s0
@28:
    jal _free_r
    move a1, s1
    lw ra, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
@44:
    jal libc_cleanup_glue
    nop
    j @28
    move a0, s0
}
#else
void libc_cleanup_glue(void *r, void *glue)
{
    void *next = *(void **)glue;

    if (next != 0) {
        libc_cleanup_glue(r, next);
    }
    _free_r(r, glue);
}
#endif

/* frees the malloc'ed buffers (__SMBF) of r's streams */
#ifdef __MWERKS__
asm void libc_free_stream_bufs(char *r)
{
    addiu sp, sp, -32
    sw s2, 8(sp)
    addiu s2, a0, 604
    sw s3, 12(sp)
    move s3, a0
    sw ra, 16(sp)
    sw s1, 4(sp)
    beqz s2, @68
    sw s0, 0(sp)
    lw s1, 4(s2)
@28:
    addiu s1, s1, -1
    bltz s1, @5c
    lw s0, 8(s2)
@34:
    lhu v0, 12(s0)
    andi v0, v0, 0x80
    beqz v0, @54
    addiu s1, s1, -1
    lw v0, 16(s0)
    move a0, s3
    bnez v0, @84
    move a1, v0
@54:
    bgez s1, @34
    addiu s0, s0, 92
@5c:
    lw s2, 0(s2)
    bnezl s2, @28
    lw s1, 4(s2)
@68:
    lw ra, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 32
@84:
    jal _free_r
    nop
    j @54
    sw zero, 16(s0)
}
#else
void libc_free_stream_bufs(char *r)
{
    char *g;

    for (g = r + 0x25C; g != 0; g = *(char **)g) {
        int n = *(int *)(g + 4);
        LibcFile *fp = *(LibcFile **)(g + 8);

        for (; --n >= 0; fp++) {
            if ((fp->flags & 0x80) && fp->base != 0) {
                _free_r(r, fp->base);
                fp->base = 0;
            }
        }
        if (*(char **)g == 0) {
            break;
        }
    }
}
#endif

#ifdef NON_MATCHING /* GCC: _global_impure_ptr through %hi/%lo */
extern char *libc_global_reent;        /* 0x088EB598 */
void _Bfree(char *r, void *v);

/* reent.c _reclaim_reent: frees everything a module's reent struct owns */
void _reclaim_reent(char *r)
{
    void **p;
    int i;

    if (r == libc_global_reent) {
        return;
    }
    if (*(void **)(r + 0x40) != 0) {
        int *res = *(int **)(r + 0x40);

        res[1] = *(int *)(r + 0x44);
        res[2] = 1 << *(int *)(r + 0x44);
        _Bfree(r, res);
    }
    for (p = *(void ***)(r + 0x48); p != 0;) {
        void **next = *p;

        _Bfree(r, p);
        p = next;
    }
    if (*(void ***)(r + 0x4C) != 0) {
        for (i = 0; i < 15; i++) {
            void **q = (*(void ****)(r + 0x4C))[i];

            while (q != 0) {
                void **next = *q;

                _free_r(r, q);
                q = next;
            }
        }
        _free_r(r, *(void **)(r + 0x4C));
    }
    if (*(void ***)(r + 0x148) != 0 && *(char **)(r + 0x148) != r + 0x14C) {
        void **q = *(void ***)(r + 0x148);

        do {
            void **next = *q;

            _free_r(r, q);
            q = next;
        } while ((char *)q != r + 0x14C);
    }
    if (*(void **)(r + 0x54) != 0) {
        _free_r(r, *(void **)(r + 0x54));
    }
    if (*(int *)(r + 0x38) != 0) {
        (*(void (**)(void *))(r + 0x3C))(r);
        libc_free_stream_bufs(r);
        if (*(void **)(r + 0x25C) != 0) {
            libc_cleanup_glue(r, *(void **)(r + 0x25C));
        }
    }
}
#endif

/* reent.c _wrapup_reent: the atexit handlers (without arguments) then __cleanup */
#ifdef __MWERKS__
asm void _wrapup_reent(char *r)
{
    addiu sp, sp, -32
    sw s3, 12(sp)
    move s3, a0
    sw ra, 16(sp)
    sw s2, 8(sp)
    sw s1, 4(sp)
    beqz a0, @a0
    sw s0, 0(sp)
@20:
    lw s2, 328(s3)
    beqzl s2, @6c
    lw v0, 60(s3)
    lw s1, 4(s2)
@30:
    addiu s1, s1, -1
    bltz s1, @5c
    sll v0, s1, 0x2
    addu v0, v0, s2
    addiu s0, v0, 8
    lw v0, 0(s0)
@48:
    addiu s1, s1, -1
    jalr ra, v0
    addiu s0, s0, -4
    bgezl s1, @48
    lw v0, 0(s0)
@5c:
    lw s2, 0(s2)
    bnezl s2, @30
    lw s1, 4(s2)
    lw v0, 60(s3)
@6c:
    bnez v0, @90
    nop
    lw ra, 16(sp)
@78:
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 32
@90:
    jalr ra, v0
    move a0, s3
    j @78
    lw ra, 16(sp)
@a0:
    jal heap_get_current
    nop
    j @20
    move s3, v0
}
#else
void _wrapup_reent(char *r)
{
    void **p;

    if (r == 0) {
        r = heap_get_current();
    }
    for (p = *(void ***)(r + 0x148); p != 0; p = *p) {
        int i;

        for (i = ((int *)p)[1] - 1; i >= 0; i--) {
            ((void (*)(void))p[2 + i])();
        }
    }
    if (*(void (**)(void *))(r + 0x3C) != 0) {
        (*(void (**)(void *))(r + 0x3C))(r);
    }
}
#endif

/* refill.c lflush: flushes line-buffered output streams (__SLBF | __SWR) */
#ifdef __MWERKS__
asm int libc_lflush(LibcFile *fp)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    addiu v0, zero, 9
    lhu v1, 12(a0)
    andi v1, v1, 0x9
    beq v1, v0, @2c
    move a1, zero
@1c:
    lw ra, 0(sp)
    move v0, a1
    jr ra
    addiu sp, sp, 16
@2c:
    jal fflush
    nop
    j @1c
    move a1, v0
}
#else
int libc_lflush(LibcFile *fp)
{
    if ((fp->flags & 9) == 9) {
        return fflush(fp);
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* GCC: _global_impure_ptr through %hi/%lo */
void libc_sinit(void *r);
void libc_smakebuf(LibcFile *fp);
int libc_fwalk(void *r, int (*fn)(LibcFile *));

/* refill.c __srefill: EOF (-1) at end of file or error */
int libc_srefill(LibcFile *fp)
{
    if (*(int *)((char *)heap_get_current() + 0x38) == 0) {
        libc_sinit(heap_get_current());
    }
    fp->r = 0;
    if (fp->flags & 0x20) {
        return -1;
    }
    if (!(fp->flags & 4)) {
        if (!(fp->flags & 0x10)) {
            return -1;
        }
        if (fp->flags & 8) {
            if (fflush(fp) != 0) {
                return -1;
            }
            fp->flags &= ~8;
            fp->w = 0;
            fp->lbfsize = 0;
        }
        fp->flags |= 4;
    } else if (fp->ub_base != 0) {
        if (fp->ub_base != fp->ubuf) {
            _free_r(heap_get_current(), fp->ub_base);
        }
        fp->ub_base = 0;
        if ((fp->r = fp->ur) != 0) {
            fp->p = fp->up;
            return 0;
        }
    }
    if (fp->base == 0) {
        libc_smakebuf(fp);
    }
    if (fp->flags & 3) {
        libc_fwalk(libc_global_reent, libc_lflush);
    }
    fp->p = fp->base;
    fp->r = fp->read(fp->cookie, (char *)fp->p, fp->size);
    fp->flags &= ~0x2000;
    if (fp->r <= 0) {
        if (fp->r == 0) {
            fp->flags |= 0x20;
        } else {
            fp->r = 0;
            fp->flags |= 0x40;
        }
        return -1;
    }
    return 0;
}

unsigned int _sbrk(int incr);

void *_sbrk_r(int *r, int incr)
{
    int ret;

    libc_errno = 0;
    ret = (int)_sbrk(incr);
    if (ret == -1 && libc_errno != 0) {
        *r = libc_errno;
    }
    return (void *)ret;
}
#endif

/* _snprintf_r: a string stream (__SWR | __SSTR) over buf, NUL-terminated when size > 0 */
#ifdef __MWERKS__
asm int _snprintf_r(void *r, char *buf, size_t size, const char *fmt, ...)
{
    addiu sp, sp, -128
    sw s0, 96(sp)
    move s0, a2
    addiu v1, s0, -1
    move t4, a1
    movz v1, zero, s0
    move a2, a3
    addiu v0, zero, 520
    addiu a3, sp, 112
    move a1, sp
    sw ra, 100(sp)
    sw t0, 112(sp)
    sw t1, 116(sp)
    sw t2, 120(sp)
    sw t3, 124(sp)
    sh v0, 12(sp)
    sw t4, 16(sp)
    sw v1, 20(sp)
    sw t4, 0(sp)
    jal _vfprintf_r
    sw v1, 8(sp)
    beqz s0, @64
    lw ra, 100(sp)
    lw v1, 0(sp)
    sb zero, 0(v1)
@64:
    lw s0, 96(sp)
    jr ra
    addiu sp, sp, 128
}
#else
int _snprintf_r(void *r, char *buf, size_t size, const char *fmt, ...)
{
    LibcFile f;
    va_list ap;
    int n;

    f.flags = 0x208;
    f.base = (unsigned char *)buf;
    f.p = (unsigned char *)buf;
    f.size = size != 0 ? (int)size - 1 : 0;
    f.w = f.size;
    va_start(ap, fmt);
    n = _vfprintf_r(r, &f, fmt, ap);
    va_end(ap);
    if (size != 0) {
        *f.p = 0;
    }
    return n;
}
#endif
