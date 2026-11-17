// @range 0x0880CDD4-0x0880DC98
// @gcc 0x0880CDD4-0x0880CDFC
// @gcc 0x0880CDFC-0x0880CE24 nl_iswspace
// @gcc 0x0880CE24-0x0880CEB8
// @gcc 0x0880CEB8-0x0880CF9C
// @gcc 0x0880CF9C-0x0880D104 nl_makebuf
// @gcc 0x0880D104-0x0880D144 nl_malign
// @gcc 0x0880D144-0x0880D2CC nl_memalignr
// @gcc 0x0880D2CC-0x0880D2FC nl_malloc
// @gcc 0x0880D2FC-0x0880D32C nl_malloc
// @gcc 0x0880D32C-0x0880D554 nl_mallocr
// @gcc 0x0880D554-0x0880DA9C nl_mallocr
// @gcc 0x0880DA9C-0x0880DB10 nl_mbrtowc
// @gcc 0x0880DB10-0x0880DB70 nl_mbrtowc
// @gcc 0x0880DB70-0x0880DBB0 nl_mbtowc_r
// @gcc 0x0880DBB0-0x0880DC98 libc_memchr
/* newlib (GCC) of the Sony SDK libc, in link order: __getreent, isspace, the "C" locale
   (setlocale/localeconv), _lseek_r, __smakebuf_r, the dlmalloc entry points (memalign, malloc,
   free, malloc_extend_top, _malloc_r), the ASCII mbrtowc/mbtowc and memchr. The port's allocator
   is platform_malloc/platform_free (libc.h). Functions that reach globals through %hi/%lo stay in
   the split asm with their C under NON_MATCHING. */
#include "libc.h"

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
    void *read;
    void *write;
    void *seek;                        /* +0x28 */
    void *close;
    char pad30[0x13];
    unsigned char nbuf[1];             /* +0x43 */
    char pad44[0x0C];
    int blksize;                       /* +0x4C */
    char pad50[0x0C];
} LibcFile;

void *heap_get_current(void);
void *_malloc_r(void *r, size_t n);
void _free_r(void *r, void *p);
void *_memalign_r(void *r, size_t align, size_t n);
int _fstat_r(void *r, int fd, void *st);
int _isatty(int fd);
int libc_cleanup_r(void *r);
int libc_sseek(void *cookie, int off, int whence);
int _mbtowc_r(void *r, int *pwc, const char *s, size_t n, void *state);
int _mbrtowc_r(int *r, int *pwc, const char *s, size_t n, int *ps);
int strcmp(const char *a, const char *b);
char *_setlocale_r(char *r, int category, const char *locale);
void __malloc_lock(void *r);
void __malloc_unlock(void *r);
extern const unsigned char libc_ctype[]; /* 0x088739F0 _ctype_ + 1 */
extern const char libc_empty_str[];    /* 0x08873BBC "" */
void *_localeconv_r(void *r);

#ifdef NON_MATCHING /* GCC: _global_impure_ptr through %hi/%lo */
extern char *libc_global_reent;        /* 0x088EB598 */

/* the current thread's reent struct (the original reads it at k0 + 4, the thread-local word of
   the PSP kernel); the single-threaded port has only the global one */
void *heap_get_current(void)
{
    return libc_global_reent;
}
#endif

/* ctype isspace over the newlib _ctype_ table (bit 8 = _S) */
int libc_isspace(unsigned int c);
#ifndef __MWERKS__ /* PSP: nl_iswspace.c */
int libc_isspace(unsigned int c)
{
    return c < 256 ? libc_ctype[c] & 8 : 0;
}
#endif

#ifdef NON_MATCHING /* GCC: locale data through %hi/%lo */
extern int libc_charset;               /* 0x088EB5A4 */
extern char libc_locale_c[];           /* 0x08873BB8 "C" */
extern char libc_lconv[];              /* 0x08873B7C */

/* _setlocale_r: only "C" and "" are accepted; returns "C" */
char *_setlocale_r(char *r, int category, const char *locale)
{
    if (locale != 0) {
        if (strcmp(locale, "C") != 0 && strcmp(locale, "") != 0) {
            return 0;
        }
        *(const char **)(r + 0x34) = locale;
        *(int *)(r + 0x30) = category;
    }
    return libc_locale_c;
}

#endif

#ifndef __MWERKS__ /* PSP: libc_malloc_gcc.c */
char *setlocale(int category, const char *locale)
{
    return _setlocale_r(heap_get_current(), category, locale);
}
#endif

#ifndef __MWERKS__ /* PSP: libc_malloc_gcc.c */
void *localeconv(void)
{
    return _localeconv_r(heap_get_current());
}
#endif

#ifdef NON_MATCHING /* GCC: errno global through %hi/%lo */
int _lseek(int fd, int offset, int whence);
extern int libc_errno;                 /* 0x08E35A34 */

#endif

/* makebuf.c __smakebuf_r: a 1 KB buffer (or the 1-byte one when unbuffered / out of memory);
   regular files using __sseek get __SOPT, ttys get line buffering */
void libc_smakebuf(LibcFile *fp);
#ifndef __MWERKS__ /* PSP: nl_makebuf.c */
void libc_smakebuf(LibcFile *fp)
{
    unsigned int st[0x58 / 4];
    int couldbetty = 0;
    unsigned char *p;

    if (fp->flags & 2) {
        fp->size = 1;
        fp->base = fp->nbuf;
        fp->p = fp->nbuf;
        return;
    }
    if (fp->file >= 0 && _fstat_r(heap_get_current(), fp->file, st) >= 0) {
        unsigned int mode = st[1] & 0xF000;

        couldbetty = mode == 0x2000;
        if (mode == 0x8000 && fp->seek == (void *)libc_sseek) {
            fp->blksize = 1024;
            fp->flags |= 0x400;
        } else {
            fp->flags |= 0x800;
        }
    } else {
        fp->flags |= 0x800;
    }
    p = _malloc_r(heap_get_current(), 1024);
    if (p == 0) {
        fp->size = 1;
        fp->flags |= 2;
        fp->base = fp->nbuf;
        fp->p = fp->nbuf;
        return;
    }
    *(int (**)(void *))((char *)heap_get_current() + 0x3C) = libc_cleanup_r;
    fp->flags |= 0x80;
    fp->base = p;
    fp->size = 1024;
    fp->p = p;
    if (couldbetty && _isatty(fp->file)) {
        fp->flags |= 1;
    }
}
#endif

/* memalign (exported as heap_malloc_cur) */
void *heap_malloc_cur(size_t align, size_t n);
#ifndef __MWERKS__ /* PSP: nl_malign.c */
void *heap_malloc_cur(size_t align, size_t n)
{
    return _memalign_r(heap_get_current(), align, n);
}
#endif

/* dlmalloc _memalign_r */
#ifndef __MWERKS__ /* PSP: nl_memalignr.c */
void *_memalign_r(void *r, size_t align, size_t n)
{
    if (align <= 16) {
        return _malloc_r(r, n);
    }
    return platform_memalign(align, n);
}
#endif

void *malloc(size_t n);
#ifndef __MWERKS__ /* PSP: nl_malloc.c */
void *malloc(size_t n)
{
    return _malloc_r(heap_get_current(), n);
}
#endif

void free(void *p);
#ifndef __MWERKS__ /* PSP: nl_malloc.c */
void free(void *p)
{
    _free_r(heap_get_current(), p);
}
#endif

#ifndef __MWERKS__ /* PSP: nl_mallocr.c */
/* dlmalloc's sbrk growth of the top chunk is the platform allocator's business in the port */
void libc_malloc_extend_top(void *r, size_t n)
{
}

void *_malloc_r(void *r, size_t n)
{
    return platform_malloc(n);
}
#endif

/* _mbrtowc_r: ASCII only; EILSEQ (138) and a reset state when the conversion fails */
#ifndef __MWERKS__ /* PSP: nl_mbrtowc.c */
int _mbrtowc_r(int *r, int *pwc, const char *s, size_t n, int *ps)
{
    int ret;

    if (s == 0) {
        pwc = 0;
        s = libc_empty_str;
        n = 1;
    }
    ret = _mbtowc_r(r, pwc, s, n, ps);
    if (ret == -1) {
        ps[0] = 0;
        *r = 138;
    }
    return ret;
}
#endif

int mbrtowc(int *pwc, const char *s, size_t n, int *ps);
#ifndef __MWERKS__ /* PSP: nl_mbrtowc.c */
int mbrtowc(int *pwc, const char *s, size_t n, int *ps)
{
    return _mbrtowc_r(heap_get_current(), pwc, s, n, ps);
}
#endif

/* __ascii_mbtowc: one byte per character; -2 for an empty input */
#ifndef __MWERKS__ /* PSP: nl_mbtowc_r.c */
int _mbtowc_r(void *r, int *pwc, const char *s, size_t n, void *state)
{
    int dummy;

    if (pwc == 0) {
        pwc = &dummy;
    }
    if (s == 0) {
        return 0;
    }
    if (n == 0) {
        return -2;
    }
    *pwc = (unsigned char)*s;
    return *s != 0;
}
#endif

