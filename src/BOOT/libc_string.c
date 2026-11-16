// @range 0x08810BA8-0x088126E8
// @gcc 0x08810BA8-0x08810C2C nl_snprintf
// @gcc 0x08810C2C-0x08810C94 nl_sprintf
// @gcc 0x08810C94-0x08810D14 nl_sprintf
// @gcc 0x08810D14-0x08810D1C nl_sscanf
// @gcc 0x08810D1C-0x08810D9C nl_sscanf
// @gcc 0x08810D9C-0x08810E24 nl_sscanf
// @gcc 0x08810E24-0x08810E9C nl_stdio
// @gcc 0x08810E9C-0x08810F28 nl_stdio
// @gcc 0x08810FA0-0x08810FD0 nl_stdio
// @gcc 0x08810FD0-0x0881106C libc_strcasecmp
// @gcc 0x0881106C-0x08811118 libc_strcat
// @gcc 0x08811118-0x08811218 libc_strchr
// @gcc 0x0881128C-0x08811324 libc_strcpy
// @gcc 0x0881133C-0x08811458 nl_strncmp
// @gcc 0x08811458-0x088115E0 nl_strncpy
// @gcc 0x088115E0-0x08811648 libc_strrchr
// @gcc 0x08811648-0x088116A4 libc_strstr
// @asm 0x088116A4-0x088126E8
/* newlib (GCC) of the Sony SDK libc, in link order: snprintf/_sprintf_r/sprintf and
   sscanf/_sscanf_r over string streams, stdio.c (__sread/__swrite/__sseek/__sclose), strcasecmp,
   strcat, strchr, strcmp, strcpy, strlen, strncmp, strncpy, strrchr, strstr and the string-stream
   vfprintf. Several keep their historical names in symbols/BOOT.txt (stricmp = strcasecmp,
   memcmp = strncmp, strrchr_str = strrchr). The word-at-a-time loops of the originals are written
   bytewise in the port C. Functions that reach globals through %hi/%lo or use jump tables stay in
   the split asm with their C under NON_MATCHING. */
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
    void *write;
    void *seek;
    void *close;
    char pad30[0x10];
    int (*ub_read)(void *, char *, int); /* +0x40 string streams: no more data */
    char pad44[0x0C];
    int offset;                        /* +0x50 */
    char pad54[0x08];
} LibcFile;

void *heap_get_current(void);
int _vfprintf_r(void *r, LibcFile *fp, const char *fmt, va_list ap);
int __svfscanf_r(void *r, LibcFile *fp, const char *fmt, va_list ap);
int _read_r(void *r, int fd, void *buf, int n);
int _write_r(void *r, int fd, const void *buf, int n);
int _lseek_r(void *r, int fd, int off, int whence);
int _close_r(void *r, int fd);
char *libc_strchr(const char *s, int c);
int libc_sread_eof(void *cookie, char *buf, int n);

/* a write-only string stream (__SWR | __SSTR) on buf of size bytes */
#ifndef __MWERKS__
static void libc_strstream(LibcFile *f, char *buf, int size)
{
    f->flags = 0x208;
    f->file = -1;
    f->base = (unsigned char *)buf;
    f->p = (unsigned char *)buf;
    f->size = size;
    f->w = size;
}
#endif

int snprintf(char *buf, size_t size, const char *fmt, ...);
#ifndef __MWERKS__ /* PSP: nl_snprintf.c */
int snprintf(char *buf, size_t size, const char *fmt, ...)
{
    LibcFile f;
    va_list ap;
    int n;

    libc_strstream(&f, buf, size != 0 ? (int)size - 1 : 0);
    f.file = 0;
    va_start(ap, fmt);
    n = _vfprintf_r(heap_get_current(), &f, fmt, ap);
    va_end(ap);
    if (size != 0) {
        *f.p = 0;
    }
    return n;
}
#endif

int _sprintf_r(void *r, char *buf, const char *fmt, ...);
#ifndef __MWERKS__ /* PSP: nl_sprintf.c */
int _sprintf_r(void *r, char *buf, const char *fmt, ...)
{
    LibcFile f;
    va_list ap;
    int n;

    libc_strstream(&f, buf, 0x7FFFFFFF);
    va_start(ap, fmt);
    n = _vfprintf_r(r, &f, fmt, ap);
    va_end(ap);
    *f.p = 0;
    return n;
}
#endif

int sprintf(char *buf, const char *fmt, ...);
#ifndef __MWERKS__ /* PSP: nl_sprintf.c */
int sprintf(char *buf, const char *fmt, ...)
{
    LibcFile f;
    va_list ap;
    int n;

    libc_strstream(&f, buf, 0x7FFFFFFF);
    va_start(ap, fmt);
    n = _vfprintf_r(heap_get_current(), &f, fmt, ap);
    va_end(ap);
    *f.p = 0;
    return n;
}
#endif

/* the read hook of sscanf's stream: nothing beyond the string */
#ifndef __MWERKS__ /* PSP: nl_sscanf.c */
int libc_sread_eof(void *cookie, char *buf, int n)
{
    return 0;
}
#endif

int sscanf(const char *str, const char *fmt, ...);
#ifndef __MWERKS__ /* PSP: nl_sscanf.c */
int sscanf(const char *str, const char *fmt, ...)
{
    LibcFile f;
    va_list ap;
    int n;

    f.flags = 4;
    f.base = (unsigned char *)str;
    f.p = (unsigned char *)str;
    f.size = strlen(str);
    f.r = f.size;
    f.read = libc_sread_eof;
    *(int *)(f.pad30 + 0) = 0;
    *(int *)(f.pad44 + 0) = 0;
    va_start(ap, fmt);
    n = __svfscanf_r(heap_get_current(), &f, fmt, ap);
    va_end(ap);
    return n;
}
#endif

#ifdef NON_MATCHING /* GCC: the read hook through %hi/%lo */
int _sscanf_r(void *r, const char *str, const char *fmt, ...);
#ifndef __MWERKS__ /* PSP: nl_sscanf.c */
int _sscanf_r(void *r, const char *str, const char *fmt, ...)
{
    LibcFile f;
    va_list ap;
    int n;

    f.flags = 4;
    f.base = (unsigned char *)str;
    f.p = (unsigned char *)str;
    f.size = strlen(str);
    f.r = f.size;
    f.read = libc_sread_eof;
    *(int *)(f.pad30 + 0) = 0;
    *(int *)(f.pad44 + 0) = 0;
    va_start(ap, fmt);
    n = __svfscanf_r(r, &f, fmt, ap);
    va_end(ap);
    return n;
}
#endif
#endif

/* stdio.c __sread: keeps the file offset (__SOFF) up to date */
#ifndef __MWERKS__ /* PSP: nl_stdio.c */
int libc_sread(LibcFile *fp, char *buf, int n)
{
    int ret = _read_r(heap_get_current(), fp->file, buf, n);

    if (ret >= 0) {
        fp->offset += ret;
    } else {
        fp->flags &= ~0x1000;
    }
    return ret;
}
#endif

/* stdio.c __swrite: append streams seek to the end first */
#ifndef __MWERKS__ /* PSP: nl_stdio.c */
int libc_swrite(LibcFile *fp, const char *buf, int n)
{
    if (fp->flags & 0x100) {
        _lseek_r(heap_get_current(), fp->file, 0, 2);
    }
    fp->flags &= ~0x1000;
    return _write_r(heap_get_current(), fp->file, buf, n);
}
#endif

#ifdef __MWERKS__
asm int libc_sseek(LibcFile *fp, int off, int whence)
{
    addiu sp, sp, -16
    sw s2, 8(sp)
    move s2, a0
    sw ra, 12(sp)
    sw s1, 4(sp)
    move s1, a2
    sw s0, 0(sp)
    jal heap_get_current
    move s0, a1
    lh a1, 14(s2)
    move a0, v0
    move a2, s0
    jal _lseek_r
    move a3, s1
    move v1, v0
    addiu v0, zero, -1
    beq v1, v0, @70
    lhu v0, 12(s2)
    sw v1, 80(s2)
    ori v0, v0, 0x1000
@50:
    sh v0, 12(s2)
    move v0, v1
    lw ra, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
@70:
    j @50
    ins v0, zero, 0xc, 0x1
}
#else
int libc_sseek(LibcFile *fp, int off, int whence)
{
    int ret = _lseek_r(heap_get_current(), fp->file, off, whence);

    if (ret == -1) {
        fp->flags &= ~0x1000;
    } else {
        fp->flags |= 0x1000;
        fp->offset = ret;
    }
    return ret;
}
#endif

int libc_sclose(LibcFile *fp);
#ifndef __MWERKS__ /* PSP: nl_stdio.c */
int libc_sclose(LibcFile *fp)
{
    return _close_r(heap_get_current(), fp->file);
}
#endif

/* strcasecmp (stricmp) is libc_strcasecmp.c: newlib's source, compiled by the SDK's GCC (`@gcc` range above) */

#ifdef __MWERKS__
asm int strcmp(const char *a, const char *b)
{
    or v0, a0, a1
    andi v0, v0, 0x1
    bnez v0, @4c
    lbu a2, 0(a0)
@10:
    lbu a3, 0(a1)
    beqz a2, @40
    addiu a0, a0, 2
    bne a2, a3, @40
    lbu v0, -1(a0)
    lbu v1, 1(a1)
    beqz v0, @38
    addiu a1, a1, 2
    beq v0, v1, @10
    lbu a2, 0(a0)
@38:
    jr ra
    subu v0, v0, v1
@40:
    jr ra
    subu v0, a2, a3
@48:
    lbu a2, 0(a0)
@4c:
    lbu a3, 0(a1)
    addiu a0, a0, 1
    sltu v1, zero, a2
    xor v0, a2, a3
    sltiu v0, v0, 1
    and v1, v1, v0
    bnez v1, @48
    addiu a1, a1, 1
    j @40
    nop
}
#else
int strcmp(const char *a, const char *b)
{
    while (*a != 0 && *a == *b) {
        a++;
        b++;
    }
    return *(const unsigned char *)a - *(const unsigned char *)b;
}
#endif

#ifdef __MWERKS__
asm size_t strlen(const char *s)
{
    addiu v0, a0, 1
@4:
    lbu v1, 0(a0)
    bnez v1, @4
    addiu a0, a0, 1
    jr ra
    subu v0, a0, v0
}
#else
size_t strlen(const char *s)
{
    const char *p = s;

    while (*p != 0) {
        p++;
    }
    return p - s;
}
#endif

/* strncmp (historical name memcmp) */
int memcmp(const char *a, const char *b, size_t n);
#ifndef __MWERKS__ /* PSP: nl_strncmp.c */
int memcmp(const char *a, const char *b, size_t n)
{
    if (n == 0) {
        return 0;
    }
    while (--n != 0 && *a != 0 && *a == *b) {
        a++;
        b++;
    }
    return *(const unsigned char *)a - *(const unsigned char *)b;
}
#endif

#ifndef __MWERKS__ /* PSP: nl_strncpy.c */
char *strncpy(char *d, const char *s, size_t n)
{
    char *p = d;

    while (n != 0 && (*p = *s) != 0) {
        p++;
        s++;
        n--;
    }
    while (n != 0) {
        *p++ = 0;
        n--;
    }
    return d;
}
#else
char *strncpy(char *d, const char *s, size_t n);
#endif

/* strrchr (historical name strrchr_str) */
#ifdef NON_MATCHING /* GCC: jump table and tables through %hi/%lo */
/* newlib _strtod_r (gdtoa: _Bfree, __lshift, __ulp, __d2b and the soft-double helpers); was taken for a
   string-stream vfprintf before the port's prototype check (strtod and vfscanf call it with (reent, s, end)) */
double _strtod_r(void *r, const char *s, char **end)
{
    /* port: the gdtoa body is not reconstructed yet; nothing is converted */
    if (end != 0) {
        *end = (char *)s;
    }
    return 0.0;
}
#endif
