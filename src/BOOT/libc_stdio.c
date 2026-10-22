// @range 0x0880C040-0x0880CDD4
// @asm 0x0880C040-0x0880C194
// @asm 0x0880C208-0x0880C3B4
// @asm 0x0880C4D0-0x0880C8EC
// @asm 0x0880CCCC-0x0880CDD4
/* newlib (GCC) of the Sony SDK libc, stdio and malloc pieces in link order: fflush, findfp.c
   (std, __sfmoreglue, __sfp, _cleanup_r, _cleanup, __sinit), fread, freer.c (_free_r,
   _malloc_trim_r), _fstat_r, __sfvwrite_r and _fwalk. The FILE streams call their
   _read/_write/_seek/_close hooks without the reent argument (this libc's variant). Functions that
   reach globals through %hi/%lo stay in the split asm (C under NON_MATCHING); the port's malloc
   pieces are the host allocator. */
#include "libc.h"

typedef struct LibcFile {              /* newlib FILE, 0x5C bytes */
    unsigned char *p;
    int r;
    int w;                             /* +0x08 */
    short flags;                       /* +0x0C */
    short file;                        /* +0x0E */
    unsigned char *base;               /* +0x10 _bf._base */
    int size;                          /* +0x14 _bf._size */
    int lbfsize;                       /* +0x18 */
    void *cookie;                      /* +0x1C */
    int (*read)(void *, char *, int);  /* +0x20 */
    int (*write)(void *, const char *, int); /* +0x24 */
    int (*seek)(void *, int, int);     /* +0x28 */
    int (*close)(void *);              /* +0x2C */
    char pad30[0x2C];
} LibcFile;

typedef struct LibcGlue {              /* newlib struct _glue */
    struct LibcGlue *next;
    int niobs;
    LibcFile *iobs;
} LibcGlue;

typedef struct LibcIov {
    const char *base;
    int len;
} LibcIov;

typedef struct LibcUio {               /* newlib struct __suio */
    LibcIov *iov;
    int iovcnt;
    int resid;
} LibcUio;

void *heap_get_current(void);
void *_malloc_r(void *r, size_t n);
void *_realloc_r(void *r, void *p, size_t n);
int nl_swsetup(void *r, LibcFile *fp);
void *memchr(const void *s, int c, size_t n);
void *memmove(void *d, const void *s, size_t n);
void *mem_move(void *d, const void *s, size_t n);
int fflush(LibcFile *fp);
int libc_fwalk(void *r, int (*fn)(LibcFile *));
int libc_srefill(LibcFile *fp);

#ifdef NON_MATCHING /* GCC: _global_impure_ptr and the stream hooks through %hi/%lo */
extern char *libc_global_reent;        /* 0x088EB598 _global_impure_ptr */
void libc_sinit(char *r);
int libc_sread(void *cookie, char *buf, int n);
int libc_swrite(void *cookie, const char *buf, int n);
int libc_sseek(void *cookie, int off, int whence);
int libc_sclose(void *cookie);

/* flushes one stream (or all of them for NULL) */
int fflush(LibcFile *fp)
{
    unsigned char *p;
    int n;

    if (fp == 0) {
        return libc_fwalk(libc_global_reent, fflush);
    }
    if (*(int *)((char *)heap_get_current() + 0x38) == 0) {
        libc_sinit(heap_get_current());
    }
    if (!(fp->flags & 8)) {
        return 0;
    }
    p = fp->base;
    if (p == 0) {
        return 0;
    }
    n = fp->p - p;
    fp->p = p;
    fp->w = (fp->flags & 3) ? 0 : fp->size;
    while (n > 0) {
        int t = fp->write(fp->cookie, (const char *)p, n);

        if (t <= 0) {
            fp->flags |= 0x40;
            return -1;
        }
        n -= t;
        p += t;
    }
    return 0;
}

/* findfp.c std(): an unbuffered stream on fd file */
void libc_std(LibcFile *fp, int flags, int file, void *r)
{
    fp->close = libc_sclose;
    fp->read = libc_sread;
    fp->flags = flags;
    fp->file = file;
    fp->write = libc_swrite;
    fp->seek = libc_sseek;
    fp->p = 0;
    fp->r = 0;
    fp->w = 0;
    fp->base = 0;
    fp->size = 0;
    fp->lbfsize = 0;
    fp->cookie = fp;
}
#endif

/* findfp.c __sfmoreglue: a glue block with n cleared streams */
#ifdef __MWERKS__
asm LibcGlue *libc_sfmoreglue(void *r, int n)
{
    addiu sp, sp, -16
    sw s0, 0(sp)
    addiu s0, zero, 92
    mult a1, s0
    sw s2, 8(sp)
    move s2, a1
    sw s1, 4(sp)
    mflo s0
    sw ra, 12(sp)
    jal _malloc_r
    addiu a1, s0, 12
    addiu v1, v0, 12
    move s1, v0
    move a0, v1
    move a2, s0
    move a1, zero
    beqz s1, @5c
    move v0, zero
    sw s2, 4(s1)
    sw zero, 0(s1)
    jal memset
    sw v1, 8(s1)
    move v0, s1
@5c:
    lw ra, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
LibcGlue *libc_sfmoreglue(void *r, int n)
{
    LibcGlue *g = _malloc_r(r, sizeof(LibcGlue) + n * sizeof(LibcFile));

    if (g == 0) {
        return 0;
    }
    g->niobs = n;
    g->next = 0;
    g->iobs = (LibcFile *)(g + 1);
    memset(g->iobs, 0, n * sizeof(LibcFile));
    return g;
}
#endif

#ifdef NON_MATCHING /* GCC: _global_impure_ptr through %hi/%lo */
/* findfp.c __sfp: a free stream, growing the list by 4 when full; ENOMEM (12) on failure */
LibcFile *libc_sfp(int *r)
{
    LibcGlue *g;

    if (*(int *)(libc_global_reent + 0x38) == 0) {
        libc_sinit(libc_global_reent);
    }
    for (g = (LibcGlue *)(libc_global_reent + 0x25C);; g = g->next) {
        int n;
        LibcFile *fp = g->iobs;

        for (n = g->niobs; --n >= 0; fp++) {
            if (fp->flags == 0) {
                fp->flags = 1;
                fp->file = -1;
                fp->p = 0;
                fp->w = 0;
                fp->r = 0;
                fp->base = 0;
                fp->size = 0;
                fp->lbfsize = 0;
                memset(fp->pad30, 0, 8);
                memset(fp->pad30 + 0x14, 0, 8);
                return fp;
            }
        }
        if (g->next == 0 && (g->next = libc_sfmoreglue(r, 4)) == 0) {
            break;
        }
    }
    *r = 12;
    return 0;
}

int libc_cleanup_r(void *r)
{
    return libc_fwalk(r, fflush);
}

void libc_cleanup(void)
{
    libc_cleanup_r(libc_global_reent);
}

/* findfp.c __sinit: stdin (read), stdout (write, line buffered), stderr (unbuffered) */
void libc_sinit(char *r)
{
    *(int (**)(void *))(r + 0x3C) = libc_cleanup_r;
    *(int *)(r + 0x38) = 1;
    *(int *)(r + 0x260) = 3;
    *(char **)(r + 0x264) = r + 0x268;
    *(int *)(r + 0x25C) = 0;
    libc_std(*(LibcFile **)(r + 4), 4, 0, r);
    libc_std(*(LibcFile **)(r + 8), 9, 1, r);
    libc_std(*(LibcFile **)(r + 0xC), 10, 2, r);
}
#endif

/* fread: from the buffer, refilling with __srefill */
#ifdef __MWERKS__
asm size_t fread(void *buf, size_t size, size_t count, LibcFile *fp)
{
    mult a2, a1
    addiu sp, sp, -32
    sw s2, 8(sp)
    mflo s2
    sw s5, 20(sp)
    move s5, a2
    move a2, zero
    sw s4, 16(sp)
    move s4, a1
    sw s1, 4(sp)
    move s1, a3
    sw ra, 28(sp)
    sw s6, 24(sp)
    sw s3, 12(sp)
    beqz s2, @d0
    sw s0, 0(sp)
    lw a2, 4(a3)
    bltzl a2, @114
    sw zero, 4(a3)
@4c:
    sltu v0, a2, s2
    move s0, a2
    move s3, a0
    beqz v0, @a4
    move s6, s2
    lw a1, 0(s1)
@64:
    move a0, s3
    jal memcpy
    move a2, s0
    lw v1, 0(s1)
    move a0, s1
    subu s2, s2, s0
    addu v1, v1, s0
    sw v1, 0(s1)
    jal libc_srefill
    addu s3, s3, s0
    bnez v0, @fc
    subu v0, s6, s2
    lw s0, 4(s1)
    sltu v0, s0, s2
    bnezl v0, @64
    lw a1, 0(s1)
@a4:
    lw a1, 0(s1)
    move a2, s2
    jal memcpy
    move a0, s3
    lw v1, 4(s1)
    lw v0, 0(s1)
    move a2, s5
    subu v1, v1, s2
    addu v0, v0, s2
    sw v0, 0(s1)
    sw v1, 4(s1)
@d0:
    lw ra, 28(sp)
@d4:
    lw s6, 24(sp)
    lw s5, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    move v0, a2
    jr ra
    addiu sp, sp, 32
@fc:
    divu v0, s4
    beqzl s4, @108
    .word 0x000001CD /* break 0x0,0x7 */
@108:
    mflo a2
    j @d4
    lw ra, 28(sp)
@114:
    j @4c
    move a2, zero
}
#else
size_t fread(void *buf, size_t size, size_t count, LibcFile *fp)
{
    size_t resid = size * count;
    size_t total = resid;
    char *p = buf;
    size_t r;

    if (resid == 0) {
        return 0;
    }
    if (fp->r < 0) {
        fp->r = 0;
    }
    while (resid > (size_t)(r = fp->r)) {
        memcpy(p, fp->p, r);
        fp->p += r;
        p += r;
        resid -= r;
        if (libc_srefill(fp) != 0) {
            return (total - resid) / size;
        }
    }
    memcpy(p, fp->p, resid);
    fp->p += resid;
    fp->r -= resid;
    return count;
}
#endif

#ifdef NON_MATCHING /* GCC: dlmalloc state through %hi/%lo */
/* the port's allocator is platform_malloc (see libc_malloc.c) */
void _free_r(void *r, void *p)
{
    platform_free(p);
}

int _malloc_trim_r(void *r, size_t pad)
{
    return 0;
}

int _fstat(int fd, void *st);
extern int libc_errno;                 /* 0x08E35A34 */

int _fstat_r(int *r, int fd, void *st)
{
    int ret;

    libc_errno = 0;
    ret = _fstat(fd, st);
    if (ret == -1 && libc_errno != 0) {
        *r = libc_errno;
    }
    return ret;
}
#endif

/* fvwrite.c __sfvwrite_r: writes uio to fp, through its buffer (string streams grow it when
   __SMBF), or straight to the hook for unbuffered streams; line-buffered streams flush at each
   newline */
#ifdef __MWERKS__
asm int libc_sfvwrite(LibcFile *fp, LibcUio *uio)
{
    addiu sp, sp, -48
    sw s5, 20(sp)
    move v1, zero
    move s5, a1
    sw s2, 8(sp)
    sw ra, 32(sp)
    sw s7, 28(sp)
    sw s6, 24(sp)
    sw s4, 16(sp)
    sw s3, 12(sp)
    sw s1, 4(sp)
    sw s0, 0(sp)
    lw s1, 8(a1)
    beqz s1, @d8
    move s2, a0
    lhu v1, 12(a0)
    andi v0, v1, 0x8
    beqz v0, @58
    nop
    lw v0, 16(a0)
    bnezl v0, @70
    andi a0, v1, 0xffff
@58:
    jal nl_swsetup
    nop
    bnez v0, @d8
    addiu v1, zero, -1
    lhu v1, 12(s2)
    andi a0, v1, 0xffff
@70:
    andi v0, a0, 0x2
    lw s3, 0(s5)
    beqz v0, @11c
    move s1, zero
@80:
    bnezl s1, @9c
    lw v0, 36(s2)
@88:
    lw s1, 4(s3)
    lw s4, 0(s3)
    beqz s1, @88
    addiu s3, s3, 8
    lw v0, 36(s2)
@9c:
    lw a0, 28(s2)
    sltiu v1, s1, 1025
    addiu a2, zero, 1024
    movn a2, s1, v1
    jalr ra, v0
    move a1, s4
    blez v0, @108
    move s0, v0
    lw v0, 8(s5)
    addu s4, s4, s0
    subu s1, s1, s0
    subu v0, v0, s0
    bnez v0, @80
    sw v0, 8(s5)
@d4:
    move v1, zero
@d8:
    lw ra, 32(sp)
    lw s7, 28(sp)
    lw s6, 24(sp)
    lw s5, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 48
@108:
    lhu v0, 12(s2)
@10c:
    addiu v1, zero, -1
    ori v0, v0, 0x40
    j @d8
    sh v0, 12(s2)
@11c:
    andi v0, a0, 0x1
    bnez v0, @298
    move s7, zero
@128:
    lw s1, 4(s3)
@12c:
    lw s4, 0(s3)
    beqz s1, @128
    addiu s3, s3, 8
@138:
    andi v1, v1, 0xffff
    andi v0, v1, 0x200
    beqz v0, @200
    lw s0, 8(s2)
    sltu v0, s0, s1
    beqz v0, @164
    sltu v0, s1, s0
    andi v0, v1, 0x80
    bnezl v0, @1bc
    lw v1, 16(s2)
@160:
    sltu v0, s1, s0
@164:
    lw a0, 0(s2)
    movn s0, s1, v0
    move a2, s0
    jal mem_move
    move a1, s4
    lw v1, 8(s2)
    lw v0, 0(s2)
    subu v1, v1, s0
    addu v0, v0, s0
    move s0, s1
@18c:
    sw v1, 8(s2)
    sw v0, 0(s2)
@194:
    lw v0, 8(s5)
@198:
    addu s4, s4, s0
    subu s1, s1, s0
    subu v0, v0, s0
    beqz v0, @d4
    sw v0, 8(s5)
    bnez s1, @138
    lhu v1, 12(s2)
    j @12c
    lw s1, 4(s3)
@1bc:
    lw v0, 0(s2)
    jal heap_get_current
    subu s0, v0, v1
    lw a1, 16(s2)
    addu s6, s0, s1
    move a0, v0
    jal _realloc_r
    move a2, s6
    beqz v0, @108
    move v1, v0
    addu v0, v0, s0
    sw v0, 0(s2)
    move s0, s1
    sw s6, 20(s2)
    sw v1, 16(s2)
    j @160
    sw s1, 8(s2)
@200:
    lw a0, 0(s2)
    lw v0, 16(s2)
    sltu v1, s0, s1
    sltu v0, v0, a0
    and v0, v0, v1
    bnez v0, @26c
    move a1, s4
    lw s0, 20(s2)
    sltu v0, s1, s0
    bnez v0, @24c
    move a2, s0
    lw v0, 36(s2)
    lw a0, 28(s2)
    jalr ra, v0
    move a1, s4
    bgtz v0, @194
    move s0, v0
    j @10c
    lhu v0, 12(s2)
@24c:
    jal mem_move
    move a2, s1
    lw v1, 8(s2)
    lw v0, 0(s2)
    move s0, s1
    subu v1, v1, s1
    j @18c
    addu v0, v0, s1
@26c:
    jal mem_move
    move a2, s0
    lw v1, 0(s2)
    move a0, s2
    addu v1, v1, s0
    jal fflush
    sw v1, 0(s2)
    beqzl v0, @198
    lw v0, 8(s5)
    j @10c
    lhu v0, 12(s2)
@298:
    bnez s1, @2b4
    nop
    move s7, zero
@2a4:
    lw s1, 4(s3)
    lw s4, 0(s3)
    beqz s1, @2a4
    addiu s3, s3, 8
@2b4:
    beqz s7, @3b8
    move a0, s4
@2bc:
    lw a0, 8(s2)
    lw t0, 20(s2)
    lw a3, 0(s2)
    lw v1, 16(s2)
    sltu v0, s1, s6
    move a1, s1
    movz a1, s6, v0
    addu s0, a0, t0
    sltu v1, v1, a3
    slt v0, s0, a1
    and v1, v1, v0
    bnez v1, @384
    slt v0, a1, t0
    bnez v0, @358
    move s0, a1
    lw v0, 36(s2)
    lw a0, 28(s2)
    move a2, t0
    jalr ra, v0
    move a1, s4
    blez v0, @108
    move s0, v0
@314:
    subu s6, s6, s0
@318:
    beqz s6, @340
    nop
    lw v0, 8(s5)
@324:
    addu s4, s4, s0
    subu s1, s1, s0
    subu v0, v0, s0
    bnez v0, @298
    sw v0, 8(s5)
    j @d8
    move v1, zero
@340:
    jal fflush
    move a0, s2
    bnez v0, @108
    move s7, zero
    j @324
    lw v0, 8(s5)
@358:
    move a0, a3
    move a1, s4
    jal mem_move
    move a2, s0
    lw v1, 8(s2)
    lw v0, 0(s2)
    subu v1, v1, s0
    addu v0, v0, s0
    sw v1, 8(s2)
    j @314
    sw v0, 0(s2)
@384:
    move a0, a3
    move a1, s4
    jal mem_move
    move a2, s0
    lw v1, 0(s2)
    move a0, s2
    addu v1, v1, s0
    jal fflush
    sw v1, 0(s2)
    beqz v0, @318
    subu s6, s6, s0
    j @10c
    lhu v0, 12(s2)
@3b8:
    addiu a1, zero, 10
    jal memchr
    move a2, s1
    subu v1, v0, s4
    beqz v0, @3d8
    addiu s6, v1, 1
@3d0:
    j @2bc
    addiu s7, zero, 1
@3d8:
    j @3d0
    addiu s6, s1, 1
}
#else
int libc_sfvwrite(LibcFile *fp, LibcUio *uio)
{
    LibcIov *iov = uio->iov;
    const char *p = 0;
    int len = 0;
    int w;

    if (uio->resid == 0) {
        return 0;
    }
    if (!(fp->flags & 8) || fp->base == 0) {
        if (nl_swsetup(heap_get_current(), fp) != 0) {
            return -1;
        }
    }
    if (fp->flags & 2) {
        do {
            while (len == 0) {
                p = iov->base;
                len = iov->len;
                iov++;
            }
            w = fp->write(fp->cookie, p, len > 1024 ? 1024 : len);
            if (w <= 0) {
                goto err;
            }
            p += w;
            len -= w;
        } while ((uio->resid -= w) != 0);
    } else if (!(fp->flags & 1)) {
        do {
            while (len == 0) {
                p = iov->base;
                len = iov->len;
                iov++;
            }
            w = fp->w;
            if (fp->flags & 0x200) {
                if (len > w && (fp->flags & 0x80)) {
                    int off = fp->p - fp->base;
                    int nsize = off + len;
                    unsigned char *b = _realloc_r(heap_get_current(), fp->base, nsize);

                    if (b == 0) {
                        goto err;
                    }
                    fp->p = b + off;
                    fp->size = nsize;
                    fp->base = b;
                    fp->w = len;
                    w = len;
                }
                if (len < w) {
                    w = len;
                }
                mem_move(fp->p, p, w);
                fp->w -= w;
                fp->p += w;
                w = len;
            } else if (fp->p > fp->base && len > w) {
                mem_move(fp->p, p, w);
                fp->p += w;
                if (fflush(fp) != 0) {
                    goto err;
                }
            } else if (len >= fp->size) {
                w = fp->write(fp->cookie, p, fp->size);
                if (w <= 0) {
                    goto err;
                }
            } else {
                w = len;
                mem_move(fp->p, p, w);
                fp->w -= w;
                fp->p += w;
            }
            p += w;
            len -= w;
        } while ((uio->resid -= w) != 0);
    } else {
        int nlknown = 0;
        int nldist = 0;

        do {
            int s;

            while (len == 0) {
                nlknown = 0;
                p = iov->base;
                len = iov->len;
                iov++;
            }
            if (!nlknown) {
                const char *nl = memchr(p, '\n', len);

                nldist = nl != 0 ? nl + 1 - p : len + 1;
                nlknown = 1;
            }
            s = len < nldist ? len : nldist;
            w = fp->w + fp->size;
            if (fp->p > fp->base && s > w) {
                mem_move(fp->p, p, w);
                fp->p += w;
                if (fflush(fp) != 0) {
                    goto err;
                }
            } else if (s >= fp->size) {
                w = fp->write(fp->cookie, p, fp->size);
                if (w <= 0) {
                    goto err;
                }
            } else {
                w = s;
                mem_move(fp->p, p, w);
                fp->w -= w;
                fp->p += w;
            }
            if ((nldist -= w) == 0) {
                if (fflush(fp) != 0) {
                    goto err;
                }
                nlknown = 0;
            }
            p += w;
            len -= w;
        } while ((uio->resid -= w) != 0);
    }
    return 0;
err:
    fp->flags |= 0x40;
    return -1;
}
#endif

#ifdef NON_MATCHING /* GCC: _global_impure_ptr through %hi/%lo */
/* fwalk.c: fn on every open stream of r's list, then of the global list */
static int libc_fwalk_list(char *r, int (*fn)(LibcFile *))
{
    LibcGlue *g;
    int ret = 0;

    for (g = (LibcGlue *)(r + 0x25C); g != 0; g = g->next) {
        int n;
        LibcFile *fp = g->iobs;

        for (n = g->niobs; --n >= 0; fp++) {
            if (fp->flags != 0) {
                ret |= fn(fp);
            }
        }
    }
    return ret;
}

int libc_fwalk(void *r, int (*fn)(LibcFile *))
{
    int ret = 0;

    if ((char *)r != libc_global_reent) {
        ret = libc_fwalk_list(r, fn);
    }
    return ret | libc_fwalk_list(libc_global_reent, fn);
}
#endif
