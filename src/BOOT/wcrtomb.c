// @range 0x0881635C-0x08816B1C
// @asm 0x08816694-0x08816AB4
// @asm 0x08816AB4-0x08816B1C
/* newlib vsprintf, the wide to multibyte conversions (wcrtomb, wcsrtombs, wctomb with the C-UTF-8,
   C-SJIS, C-EUCJP and C-JIS locales) and _write_r, compiled with GCC. The ones without data
   references are whole `asm` functions; _wctomb_r (reads __lc_ctype, D_088EB5A8) and _write_r (the
   global errno, D_08E35A34) load through split lui/lo pairs and stay in asm with their newlib C
   under NON_MATCHING. Outside the PSP build the C is used. */
#include "libc.h"
#include "nl_stdio.h"
#include "stdarg.h"

int _vfprintf_r(NlReent *data, NlFile *fp, const char *fmt, va_list ap);
int _wctomb_r(NlReent *r, char *s, int wc, NlMbState *state);
int _wcrtomb_r(NlReent *r, char *s, int wc, NlMbState *ps);
unsigned int _wcsrtombs_r(NlReent *r, char *dst, const int **src, unsigned int len, NlMbState *ps);

#if !defined(__MWERKS__) || defined(NON_MATCHING)
extern char D_088EB5A8[];              /* __lc_ctype: "C", "C-UTF-8", "C-SJIS", "C-EUCJP" or "C-JIS" */
extern int D_08E35A34;                 /* errno, set by the system call wrappers */
int strcmp(const char *a, const char *b);
int func_088047B8(int fd, const void *buf, unsigned int n); /* _write */
#endif

#ifdef __MWERKS__
asm int vsprintf(char *str, const char *fmt, va_list ap)
{
    addiu sp, sp, -112
    addiu v0, zero, 520
    lui v1, 0x7fff
    ori v1, v1, 0xffff
    sh v0, 12(sp)
    addiu v0, zero, -1
    sw ra, 104(sp)
    sw v1, 20(sp)
    sw v1, 8(sp)
    sw s1, 100(sp)
    move s1, a2
    sw s0, 96(sp)
    move s0, a1
    sw a0, 16(sp)
    sw a0, 0(sp)
    jal heap_get_current
    sh v0, 14(sp)
    move a0, v0
    move a2, s0
    move a3, s1
    jal _vfprintf_r
    move a1, sp
    lw v1, 0(sp)
    lw ra, 104(sp)
    lw s1, 100(sp)
    lw s0, 96(sp)
    sb zero, 0(v1)
    jr ra
    addiu sp, sp, 112
}
#else
int vsprintf(char *str, const char *fmt, va_list ap)
{
    NlFile f;
    int ret;

    f.flags = NL_SWR | NL_SSTR;
    f.bf.base = f.p = (unsigned char *)str;
    f.bf.size = f.w = 0x7FFFFFFF;
    f.file = -1;
    ret = _vfprintf_r(nl_getreent(), &f, fmt, ap);
    *f.p = 0;
    return ret;
}
#endif

#ifdef __MWERKS__
asm int _vsprintf_r(NlReent *ptr, char *str, const char *fmt, va_list ap)
{
    addiu sp, sp, -112
    addiu v0, zero, 520
    lui v1, 0x7fff
    move t0, a1
    ori v1, v1, 0xffff
    sh v0, 12(sp)
    move a1, sp
    addiu v0, zero, -1
    sw ra, 96(sp)
    sw v1, 20(sp)
    sw v1, 8(sp)
    sw t0, 16(sp)
    sh v0, 14(sp)
    jal _vfprintf_r
    sw t0, 0(sp)
    lw v1, 0(sp)
    lw ra, 96(sp)
    sb zero, 0(v1)
    jr ra
    addiu sp, sp, 112
}
#else
int _vsprintf_r(NlReent *ptr, char *str, const char *fmt, va_list ap)
{
    NlFile f;
    int ret;

    f.flags = NL_SWR | NL_SSTR;
    f.bf.base = f.p = (unsigned char *)str;
    f.bf.size = f.w = 0x7FFFFFFF;
    f.file = -1;
    ret = _vfprintf_r(ptr, &f, fmt, ap);
    *f.p = 0;
    return ret;
}
#endif

/* Multibyte form of wc (s == 0: of L'\0' into a scratch buffer, i.e. the reset sequence); -1 with
   errno EILSEQ for an invalid character. */
#ifdef __MWERKS__
asm int _wcrtomb_r(NlReent *r, char *s, int wc, NlMbState *ps)
{
    addiu sp, sp, -32
    sw s1, 20(sp)
    move s1, a0
    sw s0, 16(sp)
    move s0, a3
    beqz a1, L_08816480
    sw ra, 24(sp)
L_0881643C:
    jal _wctomb_r
    nop
    move v1, v0
    move a0, v0
    addiu v0, zero, -1
    beq v1, v0, L_08816470
    addiu v0, zero, 138
L_08816458:
    lw ra, 24(sp)
    lw s1, 20(sp)
    lw s0, 16(sp)
    move v0, a0
    jr ra
    addiu sp, sp, 32
L_08816470:
    sw zero, 0(s0)
    addiu a0, zero, -1
    j L_08816458
    sw v0, 0(s1)
L_08816480:
    move a1, sp
    j L_0881643C
    move a2, zero
}
#else
int _wcrtomb_r(NlReent *r, char *s, int wc, NlMbState *ps)
{
    char buf[10];
    int retval;

    if (s == 0) {
        s = buf;
        wc = 0;
    }
    retval = _wctomb_r(r, s, wc, ps);
    if (retval == -1) {
        ps->count = 0;
        r->err = EILSEQ;
    }
    return retval;
}
#endif

#ifdef __MWERKS__
asm int wcrtomb(char *s, int wc, NlMbState *ps)
{
    addiu sp, sp, -16
    sw ra, 12(sp)
    sw s2, 8(sp)
    move s2, a2
    sw s1, 4(sp)
    move s1, a1
    sw s0, 0(sp)
    jal heap_get_current
    move s0, a0
    move a0, v0
    move a1, s0
    move a2, s1
    jal _wcrtomb_r
    move a3, s2
    lw ra, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
int wcrtomb(char *s, int wc, NlMbState *ps)
{
    return _wcrtomb_r(nl_getreent(), s, wc, ps);
}
#endif

/* Converts the wide string *src into at most len bytes at dst (dst == 0: only counts); *src advances
   per converted character and becomes 0 after the terminator. Returns the byte count without the
   terminator, or -1 (EILSEQ). As in the original a character that exactly fills len is not taken. */
#ifdef __MWERKS__
asm unsigned int _wcsrtombs_r(NlReent *r, char *dst, const int **src, unsigned int len, NlMbState *ps)
{
    addiu sp, sp, -80
    sw s8, 64(sp)
    addiu v0, zero, -1
    move s8, a1
    sw s7, 60(sp)
    move s7, a2
    sw s6, 56(sp)
    move s6, zero
    sw s2, 40(sp)
    move s2, t0
    sw s1, 36(sp)
    move s1, a3
    movz s1, v0, a1
    sw s0, 32(sp)
    move s0, a1
    sw ra, 68(sp)
    sw s5, 52(sp)
    sw s4, 48(sp)
    sw s3, 44(sp)
    sw a0, 16(sp)
    beqz s1, L_088165C8
    lw s3, 0(a2)
    lw a2, 0(s3)
L_08816538:
    lw a0, 16(sp)
    move a1, sp
    move a3, s2
    lw s4, 0(s2)
    jal _wcrtomb_r
    lw s5, 4(s2)
    move a1, v0
    addiu v0, zero, -1
    beq a1, v0, L_08816620
    subu v0, s1, a1
    sltu v0, v0, s6
    xori v0, v0, 0x1
    sltu v1, a1, s1
    and v0, v0, v1
    beqzl v0, L_08816614
    sw s4, 0(s2)
    beqz s8, L_088165B0
    addu s6, s6, a1
    blez a1, L_088165A4
    move a0, zero
L_08816588:
    addu v0, sp, a0
    lbu v1, 0(v0)
    addiu a0, a0, 1
    slt v0, a0, a1
    sb v1, 0(s0)
    bnez v0, L_08816588
    addiu s0, s0, 1
L_088165A4:
    lw v0, 0(s7)
    addiu v0, v0, 4
    sw v0, 0(s7)
L_088165B0:
    lw v0, 0(s3)
    beqz v0, L_08816600
    addiu s3, s3, 4
L_088165BC:
    sltu v0, s6, s1
    bnezl v0, L_08816538
    lw a2, 0(s3)
L_088165C8:
    move v1, s6
L_088165CC:
    lw ra, 68(sp)
    lw s8, 64(sp)
    lw s7, 60(sp)
    lw s6, 56(sp)
    lw s5, 52(sp)
    lw s4, 48(sp)
    lw s3, 44(sp)
    lw s2, 40(sp)
    lw s1, 36(sp)
    lw s0, 32(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 80
L_08816600:
    bnezl s8, L_08816608
    sw zero, 0(s7)
L_08816608:
    addiu v1, s6, -1
L_0881660C:
    j L_088165CC
    sw zero, 0(s2)
L_08816614:
    move s1, zero
    j L_088165BC
    sw s5, 4(s2)
L_08816620:
    lw a0, 16(sp)
    addiu v0, zero, 138
    addiu v1, zero, -1
    j L_0881660C
    sw v0, 0(a0)
}
#else
unsigned int _wcsrtombs_r(NlReent *r, char *dst, const int **src, unsigned int len, NlMbState *ps)
{
    char *ptr = dst;
    char buff[16];
    const int *pwcs;
    unsigned int n = 0;
    int i;

    if (dst == 0) {
        len = (unsigned int)-1;
    }
    pwcs = *src;
    while (n < len) {
        int count = ps->count;
        unsigned int wch = ps->value.wch;
        int bytes = _wcrtomb_r(r, buff, *pwcs, ps);

        if (bytes == -1) {
            r->err = EILSEQ;
            ps->count = 0;
            return (unsigned int)-1;
        }
        if ((unsigned int)bytes < len && n <= len - bytes) {
            n += bytes;
            if (dst) {
                for (i = 0; i < bytes; ++i) {
                    *ptr++ = buff[i];
                }
                ++*src;
            }
            if (*pwcs++ == 0) {
                if (dst) {
                    *src = 0;
                }
                ps->count = 0;
                return n - 1;
            }
        } else {
            ps->count = count;
            ps->value.wch = wch;
            len = 0;
        }
    }
    return n;
}
#endif

#ifdef __MWERKS__
asm unsigned int wcsrtombs(char *dst, const int **src, unsigned int len, NlMbState *ps)
{
    addiu sp, sp, -32
    sw ra, 16(sp)
    sw s3, 12(sp)
    move s3, a3
    sw s2, 8(sp)
    move s2, a2
    sw s1, 4(sp)
    move s1, a1
    sw s0, 0(sp)
    jal heap_get_current
    move s0, a0
    move a0, v0
    move a1, s0
    move a2, s1
    move a3, s2
    jal _wcsrtombs_r
    move t0, s3
    lw ra, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 32
}
#else
unsigned int wcsrtombs(char *dst, const int **src, unsigned int len, NlMbState *ps)
{
    return _wcsrtombs_r(nl_getreent(), dst, src, len, ps);
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: __lc_ctype and the locale names by lui/lo */
int _wctomb_r(NlReent *r, char *s, int wchar, NlMbState *state)
{
    if (strlen(D_088EB5A8) <= 1) {
    } else if (!strcmp(D_088EB5A8, "C-UTF-8")) {
        if (s == 0) {
            return 0;
        }
        if (wchar <= 0x7F) {
            *s = (char)wchar;
            return 1;
        } else if ((unsigned int)(wchar - 0x80) < 0x780) {
            *s++ = (char)(0xC0 | ((wchar & 0x7C0) >> 6));
            *s = (char)(0x80 | (wchar & 0x3F));
            return 2;
        } else if ((unsigned int)(wchar - 0x800) < 0xF800) {
            if ((unsigned int)(wchar - 0xD800) < 0x800) {
                return -1;
            }
            *s++ = (char)(0xE0 | ((wchar & 0xF000) >> 12));
            *s++ = (char)(0x80 | ((wchar & 0xFC0) >> 6));
            *s = (char)(0x80 | (wchar & 0x3F));
            return 3;
        } else if ((unsigned int)(wchar - 0x10000) < 0x1F0000) {
            *s++ = (char)(0xF0 | ((wchar & 0x1C0000) >> 18));
            *s++ = (char)(0x80 | ((wchar & 0x3F000) >> 12));
            *s++ = (char)(0x80 | ((wchar & 0xFC0) >> 6));
            *s = (char)(0x80 | (wchar & 0x3F));
            return 4;
        } else if ((unsigned int)(wchar - 0x200000) < 0x3E00000) {
            *s++ = (char)(0xF8 | ((wchar & 0x3000000) >> 24));
            *s++ = (char)(0x80 | ((wchar & 0xFC0000) >> 18));
            *s++ = (char)(0x80 | ((wchar & 0x3F000) >> 12));
            *s++ = (char)(0x80 | ((wchar & 0xFC0) >> 6));
            *s = (char)(0x80 | (wchar & 0x3F));
            return 5;
        } else if (wchar >= 0x4000000) {
            *s++ = (char)(0xFC | ((wchar & 0x40000000) >> 30));
            *s++ = (char)(0x80 | ((wchar & 0x3F000000) >> 24));
            *s++ = (char)(0x80 | ((wchar & 0xFC0000) >> 18));
            *s++ = (char)(0x80 | ((wchar & 0x3F000) >> 12));
            *s++ = (char)(0x80 | ((wchar & 0xFC0) >> 6));
            *s = (char)(0x80 | (wchar & 0x3F));
            return 6;
        } else {
            return -1;
        }
    } else if (!strcmp(D_088EB5A8, "C-SJIS")) {
        unsigned char char2 = (unsigned char)wchar;
        unsigned char char1 = (unsigned char)(wchar >> 8);

        if (s == 0) {
            return 0;
        }
        if (char1 != 0) {
            if (((char1 >= 0x81 && char1 <= 0x9F) || (char1 >= 0xE0 && char1 <= 0xEF))
                && ((char2 >= 0x40 && char2 <= 0x7E) || (char2 >= 0x80 && char2 <= 0xFC))) {
                *s++ = (char)char1;
                *s = (char)char2;
                return 2;
            }
            return -1;
        }
    } else if (!strcmp(D_088EB5A8, "C-EUCJP")) {
        unsigned char char2 = (unsigned char)wchar;
        unsigned char char1 = (unsigned char)(wchar >> 8);

        if (s == 0) {
            return 0;
        }
        if (char1 != 0) {
            if (char1 >= 0xA1 && char1 <= 0xFE && char2 >= 0xA1 && char2 <= 0xFE) {
                *s++ = (char)char1;
                *s = (char)char2;
                return 2;
            }
            return -1;
        }
    } else if (!strcmp(D_088EB5A8, "C-JIS")) {
        int cnt = 0;
        unsigned char char2 = (unsigned char)wchar;
        unsigned char char1 = (unsigned char)(wchar >> 8);

        if (s == 0) {
            return 1;
        }
        if (char1 != 0) {
            if (char1 >= 0x21 && char1 <= 0x7E && char2 >= 0x21 && char2 <= 0x7E) {
                if (state->count == 0) {
                    state->count = 1;
                    *s++ = 0x1B;
                    *s++ = '$';
                    *s++ = 'B';
                    cnt = 3;
                }
                *s++ = (char)char1;
                *s = (char)char2;
                return cnt + 2;
            }
            return -1;
        }
        if (state->count != 0) {
            state->count = 0;
            *s++ = 0x1B;
            *s++ = '(';
            *s++ = 'B';
            cnt = 3;
        }
        *s = (char)char2;
        return cnt + 1;
    }
    if (s == 0) {
        return 0;
    }
    *s = (char)wchar;
    return 1;
}

int _write_r(NlReent *ptr, int fd, const void *buf, unsigned int cnt)
{
    int ret;

    D_08E35A34 = 0;
    if ((ret = func_088047B8(fd, buf, cnt)) == -1 && D_08E35A34 != 0) {
        ptr->err = D_08E35A34;
    }
    return ret;
}
#endif
