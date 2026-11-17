// @range 0x08810BA8-0x088126F8
// @gcc 0x08810BA8-0x08810C2C nl_snprintf
// @gcc 0x08810C2C-0x08810C94 nl_sprintf
// @gcc 0x08810C94-0x08810D14 nl_sprintf
// @gcc 0x08810D14-0x08810D1C nl_sscanf
// @gcc 0x08810D1C-0x08810D9C nl_sscanf
// @gcc 0x08810D9C-0x08810E24 nl_sscanf
// @gcc 0x08810E24-0x08810E9C nl_stdio
// @gcc 0x08810E9C-0x08810F28 nl_stdio
// @gcc 0x08810F28-0x08810FA0 nl_stdio
// @gcc 0x08810FA0-0x08810FD0 nl_stdio
// @gcc 0x08810FD0-0x0881106C libc_strcasecmp
// @gcc 0x0881106C-0x08811118 libc_strcat
// @gcc 0x08811118-0x08811218 libc_strchr
// @gcc 0x08811218-0x0881128C libc_strcmp_gcc
// @gcc 0x0881128C-0x08811324 libc_strcpy
// @gcc 0x08811324-0x0881133C libc_strlen_gcc
// @gcc 0x0881133C-0x08811458 nl_strncmp
// @gcc 0x08811458-0x088115E0 nl_strncpy
// @gcc 0x088115E0-0x08811648 libc_strrchr
// @gcc 0x08811648-0x088116A4 libc_strstr
// @gcc 0x088116A4-0x088126F8 nl_strtod
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

#ifndef __MWERKS__ /* PSP: nl_stdio.c */
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

/* PSP: libc_strcmp_gcc.c (newlib machine/mips) */
#ifndef __MWERKS__
int strcmp(const char *a, const char *b)
{
    while (*a != 0 && *a == *b) {
        a++;
        b++;
    }
    return *(const unsigned char *)a - *(const unsigned char *)b;
}
#endif

/* PSP: libc_strlen_gcc.c (newlib machine/mips) */
#ifndef __MWERKS__
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
#ifndef __MWERKS__ /* PSP: nl_strtod.c */
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
#else
double _strtod_r(void *r, const char *s, char **end);
#endif
