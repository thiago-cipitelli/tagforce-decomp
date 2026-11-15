// @range 0x0881635C-0x08816B1C
// @gcc 0x0881635C-0x088163D0 nl_vsprintf
// @gcc 0x088163D0-0x08816420 nl_vsprintf
// @gcc 0x08816420-0x0881648C nl_wcrtomb
// @gcc 0x0881648C-0x088164DC nl_wcrtomb
// @gcc 0x088164DC-0x08816634 nl_wcsrtombs
// @gcc 0x08816634-0x08816694 nl_wcsrtombs
// @gcc 0x08816694-0x08816AB4 nl_wctomb_r
// @gcc 0x08816AB4-0x08816B1C
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

int vsprintf(char *str, const char *fmt, va_list ap);
#ifndef __MWERKS__ /* PSP: nl_vsprintf.c */
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

int _vsprintf_r(NlReent *ptr, char *str, const char *fmt, va_list ap);
#ifndef __MWERKS__ /* PSP: nl_vsprintf.c */
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
#ifndef __MWERKS__ /* PSP: nl_wcrtomb.c */
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

int wcrtomb(char *s, int wc, NlMbState *ps);
#ifndef __MWERKS__ /* PSP: nl_wcrtomb.c */
int wcrtomb(char *s, int wc, NlMbState *ps)
{
    return _wcrtomb_r(nl_getreent(), s, wc, ps);
}
#endif

/* Converts the wide string *src into at most len bytes at dst (dst == 0: only counts); *src advances
   per converted character and becomes 0 after the terminator. Returns the byte count without the
   terminator, or -1 (EILSEQ). As in the original a character that exactly fills len is not taken. */
#ifndef __MWERKS__ /* PSP: nl_wcsrtombs.c */
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

unsigned int wcsrtombs(char *dst, const int **src, unsigned int len, NlMbState *ps);
#ifndef __MWERKS__ /* PSP: nl_wcsrtombs.c */
unsigned int wcsrtombs(char *dst, const int **src, unsigned int len, NlMbState *ps)
{
    return _wcsrtombs_r(nl_getreent(), dst, src, len, ps);
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: __lc_ctype and the locale names by lui/lo */
#ifndef __MWERKS__ /* PSP: nl_wctomb_r.c */
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
#endif

#endif
