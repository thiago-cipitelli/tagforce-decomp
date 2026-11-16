// @range 0x0880F60C-0x08810BA8
// @gcc 0x0880F60C-0x0880F64C nl_printf
// @gcc 0x0880F64C-0x0880F69C nl_printf
// @gcc 0x0880F69C-0x0880FF70 nl_qsort
// @gcc 0x0880FF70-0x0880FFA8 nl_rand
// @gcc 0x08810038-0x088100A0
// @gcc 0x088100A0-0x088105F8 nl_reallocr
// @gcc 0x088105F8-0x0881064C nl_reent
// @asm 0x088106E0-0x08810854
// @gcc 0x08810854-0x08810904 nl_reent
// @gcc 0x08810904-0x08810940 nl_refill
// @gcc 0x08810940-0x08810ADC nl_refill
// @gcc 0x08810ADC-0x08810B38
// @gcc 0x08810B38-0x08810BA8 nl_snprintf
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
int vfprintf(LibcFile *fp, const char *fmt, va_list ap);
void _free_r(void *r, void *p);
void libc_cleanup_glue(void *r, void *glue);
int fflush(LibcFile *fp);

/* _printf_r: printf on r's stdout */
int _printf_r(char *r, const char *fmt, ...);
#ifndef __MWERKS__ /* PSP: nl_printf.c */
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
int dbg_printf(const char *fmt, ...);
#ifndef __MWERKS__ /* PSP: nl_printf.c */
int dbg_printf(const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = vfprintf(*(LibcFile **)((char *)heap_get_current() + 8), fmt, ap);
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
#ifndef __MWERKS__ /* PSP: nl_qsort.c */
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

void srand(unsigned int seed);
#ifndef __MWERKS__ /* PSP: nl_rand.c */
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

void *_realloc_r(void *r, void *p, size_t n)
{
    return platform_realloc(p, n);
}
#endif

/* reent.c cleanup_glue: frees the chain of stream blocks, last first */
#ifndef __MWERKS__ /* PSP: nl_reent.c */
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
#ifndef __MWERKS__ /* PSP: nl_reent.c */
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
int libc_lflush(LibcFile *fp);
#ifndef __MWERKS__ /* PSP: nl_refill.c */
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
#ifndef __MWERKS__ /* PSP: nl_refill.c */
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
#endif

unsigned int _sbrk(int incr);

#endif

/* _snprintf_r: a string stream (__SWR | __SSTR) over buf, NUL-terminated when size > 0 */
int _snprintf_r(void *r, char *buf, size_t size, const char *fmt, ...);
#ifndef __MWERKS__ /* PSP: nl_snprintf.c */
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
