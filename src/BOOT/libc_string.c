// @range 0x08810BA8-0x088126E8
// @asm 0x08810D9C-0x08810E24
// @asm 0x08810FD0-0x0881106C
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

#ifdef __MWERKS__
asm int snprintf(char *buf, size_t size, const char *fmt, ...)
{
    addiu v1, a1, -1
    addiu sp, sp, -144
    movz v1, zero, a1
    addiu v0, zero, 520
    sw s1, 100(sp)
    move s1, a2
    sw s0, 96(sp)
    move s0, a1
    sw a3, 124(sp)
    sw a0, 16(sp)
    sw a0, 0(sp)
    sw ra, 104(sp)
    sw t0, 128(sp)
    sw t1, 132(sp)
    sw t2, 136(sp)
    sw t3, 140(sp)
    sh v0, 12(sp)
    sw v1, 20(sp)
    jal heap_get_current
    sw v1, 8(sp)
    move a0, v0
    move a1, sp
    move a2, s1
    jal _vfprintf_r
    addiu a3, sp, 124
    beqz s0, @74
    lw ra, 104(sp)
    lw v1, 0(sp)
    sb zero, 0(v1)
@74:
    lw s1, 100(sp)
    lw s0, 96(sp)
    jr ra
    addiu sp, sp, 144
}
#else
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

#ifdef __MWERKS__
asm int _sprintf_r(void *r, char *buf, const char *fmt, ...)
{
    addiu sp, sp, -144
    addiu v0, zero, 520
    lui v1, 0x7fff
    move t4, a1
    ori v1, v1, 0xffff
    sw a3, 124(sp)
    move a1, sp
    addiu a3, sp, 124
    sh v0, 12(sp)
    addiu v0, zero, -1
    sw ra, 96(sp)
    sw v1, 20(sp)
    sw v1, 8(sp)
    sw t0, 128(sp)
    sw t1, 132(sp)
    sw t2, 136(sp)
    sw t3, 140(sp)
    sw t4, 16(sp)
    sh v0, 14(sp)
    jal _vfprintf_r
    sw t4, 0(sp)
    lw v1, 0(sp)
    lw ra, 96(sp)
    sb zero, 0(v1)
    jr ra
    addiu sp, sp, 144
}
#else
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

#ifdef __MWERKS__
asm int sprintf(char *buf, const char *fmt, ...)
{
    addiu sp, sp, -144
    addiu v0, zero, 520
    lui v1, 0x7fff
    ori v1, v1, 0xffff
    sh v0, 12(sp)
    addiu v0, zero, -1
    sw ra, 100(sp)
    sw v1, 20(sp)
    sw v1, 8(sp)
    sw s0, 96(sp)
    move s0, a1
    sw t0, 128(sp)
    sw t1, 132(sp)
    sw t2, 136(sp)
    sw t3, 140(sp)
    sw a2, 120(sp)
    sw a3, 124(sp)
    sw a0, 16(sp)
    sw a0, 0(sp)
    jal heap_get_current
    sh v0, 14(sp)
    move a0, v0
    move a2, s0
    move a1, sp
    jal _vfprintf_r
    addiu a3, sp, 120
    lw v1, 0(sp)
    lw ra, 100(sp)
    lw s0, 96(sp)
    sb zero, 0(v1)
    jr ra
    addiu sp, sp, 144
}
#else
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
#ifdef __MWERKS__
asm int libc_sread_eof(void *cookie, char *buf, int n)
{
    jr ra
    move v0, zero
}
#else
int libc_sread_eof(void *cookie, char *buf, int n)
{
    return 0;
}
#endif

#ifdef __MWERKS__
asm int sscanf(const char *str, const char *fmt, ...)
{
    addiu sp, sp, -144
    addiu v0, zero, 4
    sw ra, 100(sp)
    sw a2, 120(sp)
    sw a3, 124(sp)
    sw t0, 128(sp)
    sw t1, 132(sp)
    sw t2, 136(sp)
    sw t3, 140(sp)
    sw a0, 16(sp)
    sw a0, 0(sp)
    sw s0, 96(sp)
    move s0, a1
    jal strlen
    sh v0, 12(sp)
    la v1, libc_sread_eof
    sw v1, 32(sp)
    sw v0, 20(sp)
    sw v0, 4(sp)
    sw zero, 48(sp)
    jal heap_get_current
    sw zero, 68(sp)
    move a0, v0
    move a2, s0
    move a1, sp
    jal __svfscanf_r
    addiu a3, sp, 120
    lw ra, 100(sp)
    lw s0, 96(sp)
    jr ra
    addiu sp, sp, 144
}
#else
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

/* stdio.c __sread: keeps the file offset (__SOFF) up to date */
#ifdef __MWERKS__
asm int libc_sread(LibcFile *fp, char *buf, int n)
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
    jal _read_r
    move a3, s1
    bltz v0, @68
    move v1, v0
    lw v0, 80(s2)
    addu v0, v0, v1
    sw v0, 80(s2)
@4c:
    lw ra, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 16
@68:
    lhu v0, 12(s2)
    ins v0, zero, 0xc, 0x1
    j @4c
    sh v0, 12(s2)
}
#else
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
#ifdef __MWERKS__
asm int libc_swrite(LibcFile *fp, const char *buf, int n)
{
    addiu sp, sp, -16
    sw s2, 8(sp)
    move s2, a2
    sw s1, 4(sp)
    move s1, a1
    sw s0, 0(sp)
    sw ra, 12(sp)
    lhu v1, 12(a0)
    andi v0, v1, 0x100
    bnez v0, @68
    move s0, a0
@2c:
    move v0, v1
    ins v0, zero, 0xc, 0x1
    jal heap_get_current
    sh v0, 12(s0)
    lh a1, 14(s0)
    move a2, s1
    move a3, s2
    jal _write_r
    move a0, v0
    lw ra, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
@68:
    jal heap_get_current
    nop
    lh a1, 14(s0)
    move a0, v0
    move a2, zero
    jal _lseek_r
    addiu a3, zero, 2
    j @2c
    lhu v1, 12(s0)
}
#else
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

#ifdef __MWERKS__
asm int libc_sclose(LibcFile *fp)
{
    addiu sp, sp, -16
    sw s0, 0(sp)
    sw ra, 4(sp)
    jal heap_get_current
    move s0, a0
    move a0, v0
    jal _close_r
    lh a1, 14(s0)
    lw ra, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
int libc_sclose(LibcFile *fp)
{
    return _close_r(heap_get_current(), fp->file);
}
#endif

#ifdef NON_MATCHING /* GCC: the ctype table through %hi/%lo */
/* strcasecmp through the ctype table (bit 1 = upper case), with signed chars */
int stricmp(const char *a, const char *b)
{
    for (;;) {
        int c = (signed char)*a++;
        int d = (signed char)*b++;

        if (c >= 'A' && c <= 'Z') {
            c += 32;
        }
        if (d >= 'A' && d <= 'Z') {
            d += 32;
        }
        if (c != d) {
            return c - d;
        }
        if (c == 0) {
            return 0;
        }
    }
}
#endif

#ifdef __MWERKS__
asm char *strcat(char *d, const char *s)
{
    addiu sp, sp, -16
    andi v0, a0, 0x3
    sw s0, 0(sp)
    move s0, a0
    bnez v0, @74
    sw ra, 4(sp)
    lw v0, 0(a0)
    lui v1, 0xfefe
    ori v1, v1, 0xfeff
    addu v1, v0, v1
    lui a0, 0x8080
    nor v0, zero, v0
    ori a0, a0, 0x8080
    and v1, v1, v0
    and v1, v1, a0
    bnez v1, @74
    move a0, s0
    lui a3, 0xfefe
    lui a2, 0x8080
    ori a3, a3, 0xfeff
    ori a2, a2, 0x8080
    addiu a0, a0, 4
@58:
    lw v0, 0(a0)
    nor v1, zero, v0
    addu v0, v0, a3
    and v0, v0, v1
    and v0, v0, a2
    beqzl v0, @58
    addiu a0, a0, 4
@74:
    lb v0, 0(a0)
    beqz v0, @90
    nop
    addiu a0, a0, 1
@84:
    lb v0, 0(a0)
    bnezl v0, @84
    addiu a0, a0, 1
@90:
    jal strcpy
    nop
    move v0, s0
    lw ra, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
char *strcat(char *d, const char *s)
{
    strcpy(d + strlen(d), s);
    return d;
}
#endif

#ifdef __MWERKS__
asm char *libc_strchr(const char *s, int c)
{
    andi v0, a0, 0x3
    move a2, a0
    bnez v0, @b8
    andi a3, a1, 0xff
    move t2, zero
    move a1, zero
@18:
    addiu a1, a1, 1
    sll v0, t2, 0x8
    sltiu v1, a1, 4
    bnez v1, @18
    or t2, v0, a3
    lw a0, 0(a2)
    lui t0, 0xfefe
    ori t0, t0, 0xfeff
    addu v0, a0, t0
    nor v1, zero, a0
    lui a1, 0x8080
    and v0, v0, v1
    ori a1, a1, 0x8080
    and v0, v0, a1
    bnez v0, @b4
    xor v0, a0, t2
    addu v1, v0, t0
    nor v0, zero, v0
    and v1, v1, v0
    and v1, v1, a1
    bnez v1, @b8
    move a0, a2
    lui t1, 0xfefe
    ori t1, t1, 0xfeff
    move t0, a1
    addiu a2, a2, 4
@80:
    lw a0, 0(a2)
    xor v1, a0, t2
    addu v0, a0, t1
    nor a0, zero, a0
    nor a1, zero, v1
    and v0, v0, a0
    addu v1, v1, t1
    and v1, v1, a1
    and v0, v0, t0
    bnez v0, @b4
    and v1, v1, t0
    beqzl v1, @80
    addiu a2, a2, 4
@b4:
    move a0, a2
@b8:
    lbu v1, 0(a0)
    andi v0, v1, 0xff
    beqz v0, @f0
    andi v1, v1, 0xff
    beq v0, a3, @f4
    xor v1, v1, a3
    addiu a0, a0, 1
@d4:
    lbu v1, 0(a0)
    andi v0, v1, 0xff
    beqzl v0, @f0
    andi v1, v1, 0xff
    bnel v0, a3, @d4
    addiu a0, a0, 1
    andi v1, v1, 0xff
@f0:
    xor v1, v1, a3
@f4:
    move v0, zero
    jr ra
    movz v0, a0, v1
}
#else
char *libc_strchr(const char *s, int c)
{
    c &= 0xFF;
    while (*(const unsigned char *)s != 0 && *(const unsigned char *)s != c) {
        s++;
    }
    return *(const unsigned char *)s == c ? (char *)s : 0;
}
#endif

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
asm char *strcpy(char *d, const char *s)
{
    or v0, a1, a0
    andi v0, v0, 0x3
    move t2, a0
    bnez v0, @7c
    move v1, a0
    lw a3, 0(a1)
    lui v0, 0xfefe
    ori v0, v0, 0xfeff
    addu v0, a3, v0
    nor a0, zero, a3
    lui v1, 0x8080
    and v0, v0, a0
    ori v1, v1, 0x8080
    and v0, v0, v1
    bnez v0, @78
    move a2, t2
    lui t1, 0xfefe
    move a0, a3
    ori t1, t1, 0xfeff
    move t0, v1
@50:
    sw a0, 0(a2)
    addiu a1, a1, 4
    lw v0, 0(a1)
    addu v1, v0, t1
    move a0, v0
    nor v0, zero, v0
    and v1, v1, v0
    and v1, v1, t0
    beqz v1, @50
    addiu a2, a2, 4
@78:
    move v1, a2
@7c:
    lbu v0, 0(a1)
    addiu a1, a1, 1
    sb v0, 0(v1)
    bnez v0, @7c
    addiu v1, v1, 1
    jr ra
    move v0, t2
}
#else
char *strcpy(char *d, const char *s)
{
    char *p = d;

    while ((*p++ = *s++) != 0) {
    }
    return d;
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
#ifdef __MWERKS__
asm int memcmp(const char *a, const char *b, size_t n)
{
    beqz a2, @70
    move v0, zero
    or v0, a0, a1
    andi v0, v0, 0x3
    bnez v0, @44
    move v0, a2
    sltiu v0, a2, 4
    move a3, a0
    bnez v0, @38
    move t0, a1
    lw v1, 0(a0)
    lw v0, 0(a1)
    beq v1, v0, @c4
    lui t1, 0xfefe
@38:
    move a0, a3
@3c:
    move a1, t0
    move v0, a2
@44:
    beqz v0, @b8
    addiu a2, a2, -1
    lbu a3, 0(a0)
    lbu t0, 0(a1)
    seb v1, a3
    seb v0, t0
    beq v1, v0, @78
    move t1, a3
@64:
    andi v1, a3, 0xff
@68:
    andi v0, t0, 0xff
    subu v0, v1, v0
@70:
    jr ra
    nop
@78:
    addiu a0, a0, 1
    addiu a1, a1, 1
    beqz a2, @b0
    seb v0, t1
    beqz v0, @b0
    addiu a2, a2, -1
    lbu a3, 0(a0)
    lbu t0, 0(a1)
    seb v1, a3
    seb v0, t0
    beq v1, v0, @78
    move t1, a3
    j @68
    andi v1, a3, 0xff
@b0:
    jr ra
    move v0, zero
@b8:
    lbu a3, 0(a0)
    j @64
    lbu t0, 0(a1)
@c4:
    lui a1, 0x8080
    ori t1, t1, 0xfeff
    ori a1, a1, 0x8080
    addiu a2, a2, -4
@d4:
    beqz a2, @b0
    sltiu a0, a2, 4
    lw v0, 0(a3)
    addiu t0, t0, 4
    nor v1, zero, v0
    addu v0, v0, t1
    and v0, v0, v1
    and v0, v0, a1
    bnez v0, @b0
    addiu a3, a3, 4
    bnezl a0, @3c
    move a0, a3
    lw v1, 0(a3)
    lw v0, 0(t0)
    beql v1, v0, @d4
    addiu a2, a2, -4
    j @3c
    move a0, a3
}
#else
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

#ifdef __MWERKS__
asm char *strncpy(char *d, const char *s, size_t n)
{
    sltiu v0, a2, 4
    bnez v0, @58
    move v1, a0
    andi t2, a2, 0x3
    subu a2, a2, t2
@14:
    lbu v0, 0(a1)
    lbu a3, 1(a1)
    lbu t0, 2(a1)
    lbu t1, 3(a1)
    addiu a2, a2, -4
    sb v0, 0(v1)
    beqz v0, @17c
    addiu a1, a1, 4
    beqz a3, @170
    sb a3, 1(v1)
    beqz t0, @164
    sb t0, 2(v1)
    beqz t1, @a8
    sb t1, 3(v1)
    bnez a2, @14
    addiu v1, v1, 4
    move a2, t2
@58:
    addu a2, v1, a2
    beq v1, a2, @80
    nop
    lbu v0, 0(a1)
@68:
    addiu a1, a1, 1
    sb v0, 0(v1)
    beqz v0, @88
    addiu v1, v1, 1
    bnel v1, a2, @68
    lbu v0, 0(a1)
@80:
    jr ra
    move v0, a0
@88:
    beq v1, a2, @80
    nop
    sb zero, 0(v1)
@94:
    addiu v1, v1, 1
    bnel v1, a2, @94
    sb zero, 0(v1)
    j @80
    nop
@a8:
    addu a2, a2, t2
    beqz a2, @dc
    addiu v1, v1, 4
    andi v0, v1, 0x3
    beqz v0, @e0
    sltiu v0, a2, 16
    sb zero, 0(v1)
@c4:
    addiu a2, a2, -1
    addiu v1, v1, 1
    beqz a2, @dc
    andi v0, v1, 0x3
    bnezl v0, @c4
    sb zero, 0(v1)
@dc:
    sltiu v0, a2, 16
@e0:
    bnez v0, @10c
    sltiu v0, a2, 8
@e8:
    addiu a2, a2, -16
    addiu v1, v1, 16
    sltiu v0, a2, 16
    sw zero, -16(v1)
    sw zero, -12(v1)
    sw zero, -8(v1)
    beqz v0, @e8
    sw zero, -4(v1)
    sltiu v0, a2, 8
@10c:
    bnez v0, @128
    sltiu v0, a2, 4
    sw zero, 0(v1)
    addiu a2, a2, -8
    sw zero, 4(v1)
    addiu v1, v1, 8
    sltiu v0, a2, 4
@128:
    bnez v0, @140
    sltiu v0, a2, 2
    sw zero, 0(v1)
    addiu a2, a2, -4
    addiu v1, v1, 4
    sltiu v0, a2, 2
@140:
    bnez v0, @154
    nop
    sh zero, 0(v1)
    addiu a2, a2, -2
    addiu v1, v1, 2
@154:
    bnezl a2, @80
    sb zero, 0(v1)
    j @80
    nop
@164:
    addiu a2, a2, 1
    j @a8
    addiu v1, v1, -1
@170:
    addiu a2, a2, 1
    j @164
    addiu v1, v1, -1
@17c:
    addiu a2, a2, 1
    j @170
    addiu v1, v1, -1
}
#else
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
#endif

/* strrchr (historical name strrchr_str) */
#ifdef __MWERKS__
asm char *strrchr_str(const char *s, int c)
{
    addiu sp, sp, -16
    sw s1, 4(sp)
    move v0, a0
    move s1, a1
    sw s0, 0(sp)
    move s0, zero
    beqz a1, @58
    sw ra, 8(sp)
@20:
    move a0, v0
    jal libc_strchr
    move a1, s1
    beqzl v0, @44
    move v0, s0
    move s0, v0
    j @20
    addiu v0, v0, 1
@40:
    move v0, s0
@44:
    lw ra, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
@58:
    jal libc_strchr
    nop
    j @40
    move s0, v0
}
#else
char *strrchr_str(const char *s, int c)
{
    const char *last = 0;

    if (c == 0) {
        return libc_strchr(s, 0);
    }
    while ((s = libc_strchr(s, c)) != 0) {
        last = s;
        s++;
    }
    return (char *)last;
}
#endif

#ifdef __MWERKS__
asm char *strstr(const char *s, const char *find)
{
    lb v0, 0(a0)
    bnez v0, @1c
    move a2, zero
    lb v0, 0(a1)
    movn a0, zero, v0
    jr ra
    move v0, a0
@1c:
    addu v0, a1, a2
@20:
    lb v1, 0(v0)
    addu a3, a0, a2
    move v0, a0
    beqz v1, @54
    addiu a2, a2, 1
    lb v0, 0(a3)
    beq v1, v0, @20
    addu v0, a1, a2
    addiu a0, a0, 1
    lb v0, 0(a0)
    bnezl v0, @1c
    move a2, zero
    move v0, zero
@54:
    jr ra
    nop
}
#else
char *strstr(const char *s, const char *find)
{
    if (*s == 0) {
        return *find != 0 ? 0 : (char *)s;
    }
    do {
        int i;

        for (i = 0;; i++) {
            if (find[i] == 0) {
                return (char *)s;
            }
            if (find[i] != s[i]) {
                break;
            }
        }
    } while (*++s != 0);
    return 0;
}
#endif

#ifdef NON_MATCHING /* GCC: jump table and tables through %hi/%lo */
/* the string-stream variant of _vfprintf_r [?]: same formatting */
int _svfprintf_r(void *r, LibcFile *fp, const char *fmt, va_list ap)
{
    return _vfprintf_r(r, fp, fmt, ap);
}
#endif
