// @range 0x0880C040-0x0880CDD4
// @gcc 0x0880C040-0x0880C13C nl_fflush
// @gcc 0x0880C13C-0x0880C194 nl_findfp
// @gcc 0x0880C194-0x0880C208 nl_findfp
// @gcc 0x0880C208-0x0880C2EC nl_findfp
// @gcc 0x0880C2EC-0x0880C30C nl_findfp
// @gcc 0x0880C30C-0x0880C330 nl_findfp
// @gcc 0x0880C330-0x0880C3B4 nl_findfp
// @gcc 0x0880C3B4-0x0880C4D0 nl_fread
// @gcc 0x0880C4D0-0x0880C888 nl_freer
// @gcc 0x0880C888-0x0880C8EC
// @gcc 0x0880C8EC-0x0880CCCC nl_fvwrite
// @gcc 0x0880CCCC-0x0880CDD4 nl_fwalk
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
int nl_swsetup(LibcFile *fp);
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
#ifndef __MWERKS__ /* PSP: nl_fflush.c */
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
#endif

/* findfp.c std(): an unbuffered stream on fd file */
void libc_std(LibcFile *fp, int flags, int file, void *r);
#ifndef __MWERKS__ /* PSP: nl_findfp.c */
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
#endif

/* findfp.c __sfmoreglue: a glue block with n cleared streams */
LibcGlue *libc_sfmoreglue(void *r, int n);
#ifndef __MWERKS__ /* PSP: nl_findfp.c */
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
LibcFile *libc_sfp(int *r);
#ifndef __MWERKS__ /* PSP: nl_findfp.c */
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
#endif

void libc_cleanup(void);
#ifndef __MWERKS__ /* PSP: nl_findfp.c */
void libc_cleanup(void)
{
    libc_cleanup_r(libc_global_reent);
}
#endif

/* findfp.c __sinit: stdin (read), stdout (write, line buffered), stderr (unbuffered) */
#endif

/* fread: from the buffer, refilling with __srefill */
#ifdef __MWERKS__
size_t fread(void *buf, size_t size, size_t count, LibcFile *fp);
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

#ifdef NON_MATCHING /* PSP: nl_freer.c (newlib mallocr.c, DEFINE_FREE) */
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

#endif

/* fvwrite.c __sfvwrite_r: writes uio to fp, through its buffer (string streams grow it when
   __SMBF), or straight to the hook for unbuffered streams; line-buffered streams flush at each
   newline */
int libc_sfvwrite(LibcFile *fp, LibcUio *uio);
#ifndef __MWERKS__ /* PSP: nl_fvwrite.c */
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
        if (nl_swsetup(fp) != 0) { /* the original passes fp alone (a0) */
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

#ifndef __MWERKS__ /* PSP: nl_fwalk.c */
int libc_fwalk(void *r, int (*fn)(LibcFile *))
{
    int ret = 0;

    if ((char *)r != libc_global_reent) {
        ret = libc_fwalk_list(r, fn);
    }
    return ret | libc_fwalk_list(libc_global_reent, fn);
}
#endif
#endif
