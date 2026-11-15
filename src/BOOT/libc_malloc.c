// @range 0x0880CDD4-0x0880DC98
// @asm 0x0880CDD4-0x0880CDFC
// @gcc 0x0880CDFC-0x0880CE24 nl_iswspace
// @asm 0x0880CE24-0x0880CEB8
// @gcc 0x0880CEB8-0x0880CED0
// @gcc 0x0880CF34-0x0880CF9C
// @gcc 0x0880CF9C-0x0880D104 nl_makebuf
// @gcc 0x0880D104-0x0880D144 nl_malign
// @gcc 0x0880D2CC-0x0880D2FC nl_malloc
// @gcc 0x0880D2FC-0x0880D32C nl_malloc
// @asm 0x0880D32C-0x0880DA9C
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

#ifdef __MWERKS__
asm char *setlocale(int category, const char *locale)
{
    addiu sp, sp, -16
    sw ra, 8(sp)
    sw s1, 4(sp)
    move s1, a1
    sw s0, 0(sp)
    jal heap_get_current
    move s0, a0
    move a0, v0
    move a1, s0
    jal _setlocale_r
    move a2, s1
    lw ra, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
char *setlocale(int category, const char *locale)
{
    return _setlocale_r(heap_get_current(), category, locale);
}
#endif

#ifdef __MWERKS__
asm void *localeconv(void)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    jal heap_get_current
    nop
    jal _localeconv_r
    move a0, v0
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
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
#ifdef __MWERKS__
asm void *_memalign_r(void *r, size_t align, size_t n)
{
    addiu sp, sp, -32
    sltiu v0, a1, 17
    sw s4, 16(sp)
    move s4, a0
    sw s0, 0(sp)
    move s0, a1
    move a1, a2
    sw ra, 20(sp)
    sw s3, 12(sp)
    sw s2, 8(sp)
    bnez v0, @178
    sw s1, 4(sp)
    addiu v0, a1, 19
    addiu a2, zero, 16
    sltiu v1, v0, 31
    move s3, a2
    ins v0, zero, 0x0, 0x4
    movz s3, v0, v1
    sltu a1, s3, a1
    srl v1, s3, 0x1f
    sltiu v0, s0, 16
    or v1, v1, a1
    movn s0, a2, v0
    bnez v1, @12c
    move v0, zero
    addu a1, s3, s0
    jal _malloc_r
    addiu a1, a1, 16
    move s1, v0
    beqz s1, @12c
    move v0, zero
    jal __malloc_lock
    move a0, s4
    divu s1, s0
    beqzl s0, @90
    .word 0x000001CD /* break 0x0,0x7 */
@90:
    mfhi v0
    beqz v0, @108
    addiu s2, s1, -8
    addu v0, s1, s0
    addiu v0, v0, -1
    negu v1, s0
    and v0, v0, v1
    addiu a3, v0, -8
    subu t0, a3, s2
    slti v0, t0, 16
    beqz v0, @c8
    lw a2, 4(s2)
    addu a3, a3, s0
    subu t0, a3, s2
@c8:
    move a1, s1
    move a0, s4
    ins a2, zero, 0x0, 0x2
    subu a2, a2, t0
    ori v0, a2, 0x1
    sw v0, 4(a3)
    addu a2, a3, a2
    lw v0, 4(a2)
    ori v0, v0, 0x1
    sw v0, 4(a2)
    lw v1, 4(s2)
    andi v1, v1, 0x1
    or v1, v1, t0
    sw v1, 4(s2)
    jal _free_r
    move s2, a3
@108:
    lw v0, 4(s2)
    ins v0, zero, 0x0, 0x2
    subu v0, v0, s3
    slti v1, v0, 16
    beqz v1, @14c
    ori v0, v0, 0x1
@120:
    jal __malloc_unlock
    move a0, s4
    addiu v0, s2, 8
@12c:
    lw ra, 20(sp)
@130:
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 32
@14c:
    addu a1, s2, s3
    sw v0, 4(a1)
    move a0, s4
    addiu a1, a1, 8
    lw v0, 4(s2)
    andi v0, v0, 0x1
    or v0, v0, s3
    jal _free_r
    sw v0, 4(s2)
    j @120
    nop
@178:
    jal _malloc_r
    nop
    j @130
    lw ra, 20(sp)
}
#else
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

#ifdef NON_MATCHING /* GCC: dlmalloc state through %hi/%lo */
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

