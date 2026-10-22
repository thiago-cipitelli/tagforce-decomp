// @range 0x088151B8-0x0881635C
// @asm 0x088152A0-0x08816284
/* newlib vfscanf (stdio/vfscanf.c, 1.1x: %lc/%ls through _mbrtowc_r, exact float scanning with
   leading-zero compensation), compiled with GCC. The entry points and __sccl are whole `asm`
   functions; __svfscanf_r (jump tables, _ctype_ and constants through split lui/addiu pairs) stays in
   asm with the newlib C under NON_MATCHING. Outside the PSP build the C is used. */
#include "libc.h"
#include "nl_stdio.h"
#include "stdarg.h"

int __svfscanf_r(NlReent *rptr, NlFile *fp, const char *fmt, va_list ap);
const char *nl_sccl(char *tab, const char *fmt);

#if !defined(__MWERKS__) || defined(NON_MATCHING)
int func_08810940(NlFile *fp);         /* __srefill */
unsigned int func_0880C3B4(void *buf, unsigned int size, unsigned int n, NlFile *fp); /* fread */
int func_0880DA9C(NlReent *r, int *pwc, const char *s, unsigned int n, NlMbState *ps); /* _mbrtowc_r */
int func_0880CDFC(int wc);             /* iswspace */
extern int D_088EB5A0;                 /* __mb_cur_max */
double _svfprintf_r(NlReent *r, const char *s, char **end); /* _strtod_r */
long _strtol_r(NlReent *r, const char *s, char **end, int base);
unsigned long _strtoul_r(NlReent *r, const char *s, char **end, int base);
long long _strtoll_r(NlReent *r, const char *s, char **end, int base);
unsigned long long _strtoull_r(NlReent *r, const char *s, char **end, int base);
int ungetc(int c, NlFile *fp);
#endif

#ifdef __MWERKS__
asm int vfscanf(NlFile *fp, const char *fmt, va_list ap)
{
    addiu sp, sp, -16
    sw s2, 8(sp)
    move s2, a2
    sw s1, 4(sp)
    move s1, a1
    sw s0, 0(sp)
    sw ra, 12(sp)
    jal heap_get_current
    move s0, a0
    lw v1, 56(v0)
    beqz v1, L_0881521C
    nop
L_088151E8:
    jal heap_get_current
    nop
    move a0, v0
    move a1, s0
    move a2, s1
    jal __svfscanf_r
    move a3, s2
    lw ra, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
L_0881521C:
    jal heap_get_current
    nop
    jal libc_sinit
    move a0, v0
    j L_088151E8
    nop
}
#else
int vfscanf(NlFile *fp, const char *fmt, va_list ap)
{
    if (nl_getreent()->sdidinit == 0) {
        nl_sinit(nl_getreent());
    }
    return __svfscanf_r(nl_getreent(), fp, fmt, ap);
}
#endif

#ifdef __MWERKS__
asm int __svfscanf(NlFile *fp, const char *fmt, va_list ap)
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
    jal __svfscanf_r
    move a3, s2
    lw ra, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
int __svfscanf(NlFile *fp, const char *fmt, va_list ap)
{
    return __svfscanf_r(nl_getreent(), fp, fmt, ap);
}
#endif

#ifdef __MWERKS__
asm int _vfscanf_r(NlReent *data, NlFile *fp, const char *fmt, va_list ap)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    jal __svfscanf_r
    nop
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
int _vfscanf_r(NlReent *data, NlFile *fp, const char *fmt, va_list ap)
{
    return __svfscanf_r(data, fp, fmt, ap);
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: jump tables, _ctype_, constants by lui/addiu */
#define S_LONG 0x01
#define S_LONGDBL 0x02
#define S_SHORT 0x04
#define S_CHAR 0x08
#define S_SUPPRESS 0x10
#define S_POINTER 0x20
#define S_NOSKIP 0x40
#define S_SIGNOK 0x80
#define S_NDIGITS 0x100
#define S_DPTOK 0x200
#define S_EXPOK 0x400
#define S_PFXOK 0x200
#define S_NZDIGITS 0x400

#define CT_CHAR 0
#define CT_CCL 1
#define CT_STRING 2
#define CT_INT 3
#define CT_FLOAT 4

#define SC_SPACE(c) ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define SC_UPPER(c) ((c) >= 'A' && (c) <= 'Z')
#define BUF_EMPTY (fp->r <= 0 && func_08810940(fp))
#define MAX_LONG_LEN 11

typedef long (*NlStrtol)(NlReent *, const char *, char **, int);

int __svfscanf_r(NlReent *rptr, NlFile *fp, const char *fmt, va_list ap)
{
    static const short basefix[17] = { 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    int c;
    unsigned int width;
    char *p;
    int n;
    int flags;
    char *p0;
    int nassigned = 0;
    int nread = 0;
    int base = 0;
    NlStrtol ccfn = 0;
    char ccltab[256];
    char buf[350];
    int wc;
    int *wcp;
    int mbslen;
    NlMbState state;

    for (;;) {
        c = *fmt++;
        if (c == 0) {
            return nassigned;
        }
        if (SC_SPACE(c)) {
            for (;;) {
                if (BUF_EMPTY || !SC_SPACE(*fp->p)) {
                    break;
                }
                nread++, fp->r--, fp->p++;
            }
            continue;
        }
        if (c != '%') {
            goto literal;
        }
        width = 0;
        flags = 0;
again:
        c = *fmt++;
        switch (c) {
        case '%':
literal:
            if (BUF_EMPTY) {
                goto input_failure;
            }
            if (*fp->p != (unsigned char)fmt[-1]) {
                goto match_failure;
            }
            fp->r--, fp->p++;
            nread++;
            continue;
        case '*':
            flags |= S_SUPPRESS;
            goto again;
        case 'l':
            if (*fmt == 'l') {
                fmt++;
                flags |= S_LONGDBL;
            } else {
                flags |= S_LONG;
            }
            goto again;
        case 'L':
            flags |= S_LONGDBL;
            goto again;
        case 'h':
            if (*fmt == 'h') {
                fmt++;
                flags |= S_CHAR;
            } else {
                flags |= S_SHORT;
            }
            goto again;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            width = width * 10 + c - '0';
            goto again;
        case 'D':
            flags |= S_LONG;
            /* fall through */
        case 'd':
            c = CT_INT;
            ccfn = _strtol_r;
            base = 10;
            break;
        case 'i':
            c = CT_INT;
            ccfn = _strtol_r;
            base = 0;
            break;
        case 'O':
            flags |= S_LONG;
            /* fall through */
        case 'o':
            c = CT_INT;
            ccfn = (NlStrtol)_strtoul_r;
            base = 8;
            break;
        case 'u':
            c = CT_INT;
            ccfn = (NlStrtol)_strtoul_r;
            base = 10;
            break;
        case 'X':
        case 'x':
            flags |= S_PFXOK;
            c = CT_INT;
            ccfn = (NlStrtol)_strtoul_r;
            base = 16;
            break;
        case 'E':
        case 'G':
        case 'e':
        case 'f':
        case 'g':
            c = CT_FLOAT;
            break;
        case 'S':
            flags |= S_LONG;
            /* fall through */
        case 's':
            c = CT_STRING;
            break;
        case '[':
            fmt = nl_sccl(ccltab, fmt);
            flags |= S_NOSKIP;
            c = CT_CCL;
            break;
        case 'C':
            flags |= S_LONG;
            /* fall through */
        case 'c':
            flags |= S_NOSKIP;
            c = CT_CHAR;
            break;
        case 'p':
            flags |= S_POINTER | S_PFXOK;
            c = CT_INT;
            ccfn = (NlStrtol)_strtoul_r;
            base = 16;
            break;
        case 'n':
            if (flags & S_SUPPRESS) {
                continue;
            }
            if (flags & S_CHAR) {
                *va_arg(ap, char *) = (char)nread;
            } else if (flags & S_SHORT) {
                *va_arg(ap, short *) = (short)nread;
            } else if (flags & S_LONG) {
                *va_arg(ap, long *) = nread;
            } else if (flags & S_LONGDBL) {
                *va_arg(ap, long long *) = nread;
            } else {
                *va_arg(ap, int *) = nread;
            }
            continue;
        case '\0':
            return -1;
        default:
            if (SC_UPPER(c)) {
                flags |= S_LONG;
            }
            c = CT_INT;
            ccfn = _strtol_r;
            base = 10;
            break;
        }

        if (BUF_EMPTY) {
            goto input_failure;
        }
        if ((flags & S_NOSKIP) == 0) {
            while (SC_SPACE(*fp->p)) {
                nread++;
                if (--fp->r > 0) {
                    fp->p++;
                } else if (func_08810940(fp)) {
                    goto input_failure;
                }
            }
        }

        switch (c) {
        case CT_CHAR:
            if (width == 0) {
                width = 1;
            }
            if (flags & S_LONG) {
                wcp = (flags & S_SUPPRESS) ? 0 : va_arg(ap, int *);
                n = 0;
                while (width != 0) {
                    if (n == D_088EB5A0) {
                        goto input_failure;
                    }
                    buf[n++] = *fp->p;
                    fp->r -= 1;
                    fp->p += 1;
                    memset(&state, 0, sizeof(state));
                    if ((mbslen = func_0880DA9C(rptr, wcp, buf, n, &state)) == -1) {
                        goto input_failure;
                    }
                    if (mbslen == 0 && !(flags & S_SUPPRESS)) {
                        *wcp = 0;
                    }
                    if (mbslen != -2) {
                        nread += n;
                        width -= 1;
                        if (!(flags & S_SUPPRESS)) {
                            wcp += 1;
                        }
                        n = 0;
                    }
                    if (BUF_EMPTY) {
                        if (n != 0) {
                            goto input_failure;
                        }
                        break;
                    }
                }
                if (!(flags & S_SUPPRESS)) {
                    nassigned++;
                }
            } else if (flags & S_SUPPRESS) {
                unsigned int sum = 0;

                for (;;) {
                    if ((n = fp->r) < (int)width) {
                        sum += n;
                        width -= n;
                        fp->p += n;
                        if (func_08810940(fp)) {
                            if (sum == 0) {
                                goto input_failure;
                            }
                            break;
                        }
                    } else {
                        sum += width;
                        fp->r -= width;
                        fp->p += width;
                        break;
                    }
                }
                nread += sum;
            } else {
                unsigned int r = func_0880C3B4(va_arg(ap, char *), 1, width, fp);

                if (r == 0) {
                    goto input_failure;
                }
                nread += r;
                nassigned++;
            }
            break;

        case CT_CCL:
            if (width == 0) {
                width = ~0u;
            }
            if (flags & S_SUPPRESS) {
                n = 0;
                while (ccltab[*fp->p]) {
                    n++, fp->r--, fp->p++;
                    if (--width == 0) {
                        break;
                    }
                    if (BUF_EMPTY) {
                        if (n == 0) {
                            goto input_failure;
                        }
                        break;
                    }
                }
                if (n == 0) {
                    goto match_failure;
                }
            } else {
                p0 = p = va_arg(ap, char *);
                while (ccltab[*fp->p]) {
                    fp->r--;
                    *p++ = *fp->p++;
                    if (--width == 0) {
                        break;
                    }
                    if (BUF_EMPTY) {
                        if (p == p0) {
                            goto input_failure;
                        }
                        break;
                    }
                }
                n = p - p0;
                if (n == 0) {
                    goto match_failure;
                }
                *p = 0;
                nassigned++;
            }
            nread += n;
            break;

        case CT_STRING:
            if (width == 0) {
                width = ~0u;
            }
            if (flags & S_LONG) {
                wcp = (flags & S_SUPPRESS) ? &wc : va_arg(ap, int *);
                n = 0;
                while (!SC_SPACE(*fp->p) && width != 0) {
                    if (n == D_088EB5A0) {
                        goto input_failure;
                    }
                    buf[n++] = *fp->p;
                    fp->r -= 1;
                    fp->p += 1;
                    memset(&state, 0, sizeof(state));
                    if ((mbslen = func_0880DA9C(rptr, wcp, buf, n, &state)) == -1) {
                        goto input_failure;
                    }
                    if (mbslen == 0) {
                        *wcp = 0;
                    }
                    if (mbslen != -2) {
                        if (func_0880CDFC(*wcp)) {
                            while (n != 0) {
                                ungetc(buf[--n], fp);
                            }
                            break;
                        }
                        nread += n;
                        width -= 1;
                        if ((flags & S_SUPPRESS) == 0) {
                            wcp += 1;
                        }
                        n = 0;
                    }
                    if (BUF_EMPTY) {
                        if (n != 0) {
                            goto input_failure;
                        }
                        break;
                    }
                }
                if (!(flags & S_SUPPRESS)) {
                    *wcp = 0;
                    nassigned++;
                }
            } else if (flags & S_SUPPRESS) {
                n = 0;
                while (!SC_SPACE(*fp->p)) {
                    n++, fp->r--, fp->p++;
                    if (--width == 0) {
                        break;
                    }
                    if (BUF_EMPTY) {
                        break;
                    }
                }
                nread += n;
            } else {
                p0 = p = va_arg(ap, char *);
                while (!SC_SPACE(*fp->p)) {
                    fp->r--;
                    *p++ = *fp->p++;
                    if (--width == 0) {
                        break;
                    }
                    if (BUF_EMPTY) {
                        break;
                    }
                }
                *p = 0;
                nread += p - p0;
                nassigned++;
            }
            continue;

        case CT_INT:
            if (--width > sizeof(buf) - 2) {
                width = sizeof(buf) - 2;
            }
            width++;
            flags |= S_SIGNOK | S_NDIGITS | S_NZDIGITS;
            for (p = buf; width; width--) {
                c = *fp->p;
                switch (c) {
                case '0':
                    if (base == 0) {
                        base = 8;
                        flags |= S_PFXOK;
                    }
                    if (flags & S_NZDIGITS) {
                        flags &= ~(S_SIGNOK | S_NZDIGITS | S_NDIGITS);
                    } else {
                        flags &= ~(S_SIGNOK | S_PFXOK | S_NDIGITS);
                    }
                    goto ok;
                case '1': case '2': case '3': case '4': case '5': case '6': case '7':
                    base = basefix[base];
                    flags &= ~(S_SIGNOK | S_PFXOK | S_NDIGITS);
                    goto ok;
                case '8':
                case '9':
                    base = basefix[base];
                    if (base <= 8) {
                        break;
                    }
                    flags &= ~(S_SIGNOK | S_PFXOK | S_NDIGITS);
                    goto ok;
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                    if (base <= 10) {
                        break;
                    }
                    flags &= ~(S_SIGNOK | S_PFXOK | S_NDIGITS);
                    goto ok;
                case '+':
                case '-':
                    if (flags & S_SIGNOK) {
                        flags &= ~S_SIGNOK;
                        goto ok;
                    }
                    break;
                case 'x':
                case 'X':
                    if ((flags & S_PFXOK) && p == buf + 1) {
                        base = 16;
                        flags &= ~S_PFXOK;
                        goto ok;
                    }
                    break;
                }
                break;
ok:
                *p++ = (char)c;
                if (--fp->r > 0) {
                    fp->p++;
                } else if (func_08810940(fp)) {
                    break;
                }
            }
            if (flags & S_NDIGITS) {
                if (p > buf) {
                    ungetc(*--p, fp);
                }
                goto match_failure;
            }
            c = p[-1];
            if (c == 'x' || c == 'X') {
                --p;
                ungetc(c, fp);
            }
            if ((flags & S_SUPPRESS) == 0) {
                unsigned long res;

                *p = 0;
                res = ccfn(rptr, buf, 0, base);
                if (flags & S_POINTER) {
                    *va_arg(ap, void **) = (void *)res;
                } else if (flags & S_CHAR) {
                    *va_arg(ap, char *) = (char)res;
                } else if (flags & S_SHORT) {
                    *va_arg(ap, short *) = (short)res;
                } else if (flags & S_LONG) {
                    *va_arg(ap, long *) = res;
                } else if (flags & S_LONGDBL) {
                    unsigned long long resll;

                    if (ccfn == (NlStrtol)_strtoul_r) {
                        resll = _strtoull_r(rptr, buf, 0, base);
                    } else {
                        resll = _strtoll_r(rptr, buf, 0, base);
                    }
                    *va_arg(ap, long long *) = resll;
                } else {
                    *va_arg(ap, int *) = res;
                }
                nassigned++;
            }
            nread += p - buf;
            break;

        case CT_FLOAT: {
            long leading_zeroes = 0;
            long zeroes;
            long exp_adjust;
            char *exp_start = 0;

            if (--width > sizeof(buf) - 2) {
                width = sizeof(buf) - 2;
            }
            width++;
            flags |= S_SIGNOK | S_NDIGITS | S_DPTOK | S_EXPOK;
            zeroes = 0;
            exp_adjust = 0;
            for (p = buf; width;) {
                c = *fp->p;
                switch (c) {
                case '0':
                    if (flags & S_NDIGITS) {
                        flags &= ~S_SIGNOK;
                        zeroes++;
                        goto fskip;
                    }
                    /* fall through */
                case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                    flags &= ~(S_SIGNOK | S_NDIGITS);
                    goto fok;
                case '+':
                case '-':
                    if (flags & S_SIGNOK) {
                        flags &= ~S_SIGNOK;
                        goto fok;
                    }
                    break;
                case '.':
                    if (flags & S_DPTOK) {
                        flags &= ~(S_SIGNOK | S_DPTOK);
                        leading_zeroes = zeroes;
                        goto fok;
                    }
                    break;
                case 'e':
                case 'E':
                    if ((flags & (S_NDIGITS | S_EXPOK)) == S_EXPOK || ((flags & S_EXPOK) && zeroes)) {
                        if (!(flags & S_DPTOK)) {
                            exp_adjust = zeroes - leading_zeroes;
                            exp_start = p;
                        }
                        flags = (flags & ~(S_EXPOK | S_DPTOK)) | S_SIGNOK | S_NDIGITS;
                        zeroes = 0;
                        goto fok;
                    }
                    break;
                }
                break;
fok:
                *p++ = (char)c;
fskip:
                width--;
                ++nread;
                if (--fp->r > 0) {
                    fp->p++;
                } else if (func_08810940(fp)) {
                    break;
                }
            }
            if (zeroes) {
                flags &= ~S_NDIGITS;
            }
            if (flags & S_NDIGITS) {
                if (flags & S_EXPOK) {
                    while (p > buf) {
                        ungetc(*--p, fp);
                    }
                    goto match_failure;
                }
                c = *--p;
                --nread;
                if (c != 'e' && c != 'E') {
                    ungetc(c, fp);
                    c = *--p;
                    --nread;
                }
                ungetc(c, fp);
            }
            if ((flags & S_SUPPRESS) == 0) {
                double res;
                long new_exp = 0;

                *p = 0;
                if ((flags & (S_DPTOK | S_EXPOK)) == S_EXPOK) {
                    exp_adjust = zeroes - leading_zeroes;
                    new_exp = -exp_adjust;
                    exp_start = p;
                } else if (exp_adjust) {
                    new_exp = _strtol_r(rptr, exp_start + 1, 0, 10) - exp_adjust;
                }
                if (exp_adjust) {
                    if (exp_start >= buf + sizeof(buf) - MAX_LONG_LEN) {
                        exp_start = buf + sizeof(buf) - MAX_LONG_LEN - 1;
                    }
                    sprintf(exp_start, "e%ld", new_exp);
                }
                res = _svfprintf_r(rptr, buf, 0);
                if (flags & (S_LONG | S_LONGDBL)) {
                    *va_arg(ap, double *) = res;
                } else {
                    *va_arg(ap, float *) = (float)res;
                }
                nassigned++;
            }
            break;
        }
        }
    }
input_failure:
    return nassigned ? nassigned : -1;
match_failure:
    return nassigned;
}
#endif

/* Builds the %[ scanset table (1 = member) from the class after '[' ('^' negates, a-b ranges, a
   leading ']' is a member); returns the format position after the closing ']'. */
#ifdef __MWERKS__
asm const char *nl_sccl(char *tab, const char *fmt)
{
    lb a2, 0(a1)
    addiu v0, zero, 94
    addiu a1, a1, 1
    beq a2, v0, L_0881634C
    move t0, zero
L_08816298:
    move a3, zero
L_0881629C:
    addu v1, a0, a3
    addiu a3, a3, 1
    slti v0, a3, 256
    bnez v0, L_0881629C
    sb t0, 0(v1)
    beqz a2, L_08816334
    addiu v0, a1, -1
    addiu v0, zero, 1
    subu t0, v0, t0
    addiu t1, zero, 45
    addiu t2, zero, 93
L_088162C8:
    addu v0, a0, a2
    sb t0, 0(v0)
    lb a3, 0(a1)
L_088162D4:
    bne a3, t1, L_08816318
    addiu a1, a1, 1
    lb a3, 0(a1)
    xori v0, a3, 0x5d
    sltiu v0, v0, 1
    slt v1, a3, a2
    or v0, v0, v1
    bnezl v0, L_088162C8
    addiu a2, zero, 45
    addiu a1, a1, 1
L_088162FC:
    addiu a2, a2, 1
    addu v0, a0, a2
    slt v1, a2, a3
    bnez v1, L_088162FC
    sb t0, 0(v0)
    j L_088162D4
    lb a3, 0(a1)
L_08816318:
    slti v0, a3, 46
    beqz v0, L_0881633C
    nop
    beqz a3, L_08816334
    addiu v0, a1, -1
L_0881632C:
    j L_088162C8
    move a2, a3
L_08816334:
    jr ra
    nop
L_0881633C:
    bne a3, t2, L_0881632C
    move v0, a1
    j L_08816334
    nop
L_0881634C:
    lb a2, 0(a1)
    addiu t0, zero, 1
    j L_08816298
    addiu a1, a1, 1
}
#else
const char *nl_sccl(char *tab, const char *fmt)
{
    int c;
    int n;
    int v;

    c = *fmt++;
    if (c == '^') {
        v = 1;
        c = *fmt++;
    } else {
        v = 0;
    }
    for (n = 0; n < 256; n++) {
        tab[n] = (char)v;
    }
    if (c == 0) {
        return fmt - 1;
    }
    v = 1 - v;
    for (;;) {
        tab[c] = (char)v;
doswitch:
        n = *fmt++;
        switch (n) {
        case 0:
            return fmt - 1;
        case '-':
            n = *fmt;
            if (n == ']' || n < c) {
                c = '-';
                break;
            }
            fmt++;
            do {
                tab[++c] = (char)v;
            } while (c < n);
            goto doswitch;
        case ']':
            return fmt;
        default:
            c = n;
            break;
        }
    }
}
#endif
