// @range 0x08813390-0x088151B8
// @asm 0x08813500-0x08814F18
/* newlib vfprintf (stdio/vfprintf.c, 1.1x: no positional arguments, %lc/%ls through the wide
   character conversions), compiled with GCC. Output goes through an iovec list of at most 8 entries
   flushed by __sfvwrite; unbuffered files are printed through a 1 KB stack buffer (__sbprintf).
   Floating point uses _dtoa_r through cvt(). The helpers are whole `asm` functions; _vfprintf_r
   (jump table and string constants through split lui/addiu pairs) stays in asm with its newlib C
   under NON_MATCHING. Outside the PSP build the C is used. */
#include "libc.h"
#include "nl_stdio.h"
#include "stdarg.h"
#ifdef __MWERKS__
void __eqdf2(void);
void __nedf2(void);
void __negdf2(void);
#endif


int vfprintf(NlFile *fp, const char *fmt, va_list ap);
int _vfprintf_r(NlReent *data, NlFile *fp, const char *fmt, va_list ap);
char *nl_cvt(NlReent *data, double value, int ndigits, int flags, char *sign, int *decpt, int ch, int *length);
int nl_exponent(char *p0, int exp, int fmtch);
int nl_swsetup(NlFile *fp);
int _wcrtomb_r(NlReent *r, char *s, int wc, NlMbState *ps);
int _wcsrtombs_r(NlReent *r, char *dst, const int **src, unsigned int len, NlMbState *ps);
char *_dtoa_r(NlReent *r, double d, int mode, int ndigits, int *decpt, int *sign, char **rve); /* _dtoa_r */

#if !defined(__MWERKS__) || defined(NON_MATCHING)
char **func_0880CF10(NlReent *r);      /* _localeconv_r: first field decimal_point */
int func_08809B54(double d);           /* isinf */
int func_08809B88(double d);           /* isnan */
void *func_0880DBB0(const void *s, int c, unsigned int n); /* memchr */
#endif

#define F_ALT 0x001
#define F_HEXPREFIX 0x002
#define F_LADJUST 0x004
#define F_LONGDBL 0x008
#define F_LONGINT 0x010
#define F_QUADINT 0x020
#define F_SHORTINT 0x040
#define F_ZEROPAD 0x080
#define F_FPT 0x100

/* Writes and empties the pending iovecs; 0 or the __sfvwrite error. */
#ifdef __MWERKS__
asm int nl_sprint(NlFile *fp, NlUio *uio)
{
    addiu sp, sp, -16
    sw s0, 0(sp)
    move v1, zero
    sw ra, 4(sp)
    lw v0, 8(a1)
    bnez v0, L_088133C4
    move s0, a1
    sw zero, 4(a1)
L_088133B0:
    lw ra, 4(sp)
    lw s0, 0(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 16
L_088133C4:
    jal libc_sfvwrite
    nop
    sw zero, 4(s0)
    move v1, v0
    j L_088133B0
    sw zero, 8(s0)
}
#else
int nl_sprint(NlFile *fp, NlUio *uio)
{
    int err;

    if (uio->resid == 0) {
        uio->iovcnt = 0;
        return 0;
    }
    err = nl_sfvwrite(fp, uio);
    uio->resid = 0;
    uio->iovcnt = 0;
    return err;
}
#endif

/* vfprintf on an unbuffered file through a temporary 1 KB buffered copy of it */
#ifdef __MWERKS__
asm int nl_sbprintf(NlFile *fp, const char *fmt, va_list ap)
{
    addiu sp, sp, -1136
    sw s1, 1124(sp)
    addiu v1, sp, 96
    addiu a3, zero, 1024
    sw s0, 1120(sp)
    move s1, a0
    sw ra, 1128(sp)
    lhu v0, 12(a0)
    lhu t0, 14(a0)
    lw t1, 28(a0)
    lw t2, 36(a0)
    ins v0, zero, 0x1, 0x1
    move a0, sp
    sh v0, 12(sp)
    sh t0, 14(sp)
    sw t1, 28(sp)
    sw t2, 36(sp)
    sw v1, 16(sp)
    sw a3, 20(sp)
    sw v1, 0(sp)
    sw a3, 8(sp)
    jal vfprintf
    sw zero, 24(sp)
    move s0, v0
    bltz v0, L_08813454
    move a0, sp
    jal fflush
    nop
    addiu v1, zero, -1
    movn s0, v1, v0
L_08813454:
    lhu v1, 12(sp)
    andi v1, v1, 0x40
    beqz v1, L_08813470
    move v0, s0
    lhu v1, 12(s1)
    ori v1, v1, 0x40
    sh v1, 12(s1)
L_08813470:
    lw ra, 1128(sp)
    lw s1, 1124(sp)
    lw s0, 1120(sp)
    jr ra
    addiu sp, sp, 1136
}
#else
int nl_sbprintf(NlFile *fp, const char *fmt, va_list ap)
{
    NlFile fake;
    unsigned char buf[1024];
    int ret;

    fake.flags = fp->flags & ~NL_SNBF;
    fake.file = fp->file;
    fake.cookie = fp->cookie;
    fake.write = fp->write;
    fake.bf.base = fake.p = buf;
    fake.bf.size = fake.w = sizeof(buf);
    fake.lbfsize = 0;
    ret = vfprintf(&fake, fmt, ap);
    if (ret >= 0 && nl_fflush(&fake)) {
        ret = -1;
    }
    if (fake.flags & NL_SERR) {
        fp->flags |= NL_SERR;
    }
    return ret;
}
#endif

#ifdef __MWERKS__
asm int vfprintf(NlFile *fp, const char *fmt, va_list ap)
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
    beqz v1, L_088134E8
    nop
L_088134B4:
    jal heap_get_current
    nop
    move a0, v0
    move a1, s0
    move a2, s1
    jal _vfprintf_r
    move a3, s2
    lw ra, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
L_088134E8:
    jal heap_get_current
    nop
    jal libc_sinit
    move a0, v0
    j L_088134B4
    nop
}
#else
int vfprintf(NlFile *fp, const char *fmt, va_list ap)
{
    if (nl_getreent()->sdidinit == 0) {
        nl_sinit(nl_getreent());
    }
    return _vfprintf_r(nl_getreent(), fp, fmt, ap);
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: switch table and strings via split lui/addiu */
#define NIOV 8
#define PADSIZE 16
static const char nl_blanks[PADSIZE] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
static const char nl_zeroes[PADSIZE] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };


#define PRINT(ptr_, len_) do { \
        iovp->base = (ptr_); \
        iovp->len = (len_); \
        uio.resid += (len_); \
        iovp++; \
        if (++uio.iovcnt >= NIOV) { \
            if (nl_sprint(fp, &uio)) { \
                goto error; \
            } \
            iovp = iov; \
        } \
    } while (0)
#define PAD(howmany, with) do { \
        if ((n = (howmany)) > 0) { \
            while (n > PADSIZE) { \
                PRINT(with, PADSIZE); \
                n -= PADSIZE; \
            } \
            PRINT(with, n); \
        } \
    } while (0)
#define FLUSH() do { \
        if (uio.resid && nl_sprint(fp, &uio)) { \
            goto error; \
        } \
        uio.iovcnt = 0; \
        iovp = iov; \
    } while (0)

int _vfprintf_r(NlReent *data, NlFile *fp, const char *fmt, va_list ap)
{
    char *cp;
    int ch;
    int n;
    int m;
    NlIov *iovp;
    int flags;
    int ret = 0;
    int width;
    int prec;
    char sign;
    char *decimal_point = *func_0880CF10(data);
    char softsign;
    double fpvalue = 0;
    int expt;
    int expsize = 0;
    int ndig;
    char expstr[7];
    unsigned long long uquad;
    enum { OCT, DEC, HEX } base;
    int dprec;
    int realsz;
    int size;
    const char *xdigs = 0;
    NlUio uio;
    NlIov iov[NIOV];
    char buf[348];
    char ox[2];
    NlMbState ps;
    char *malloc_buf = 0;

    if (((fp->flags & NL_SWR) == 0 || fp->bf.base == 0) && nl_swsetup(fp)) {
        return -1;
    }
    if ((fp->flags & (NL_SNBF | NL_SWR | NL_SRW)) == (NL_SNBF | NL_SWR) && fp->file >= 0) {
        return nl_sbprintf(fp, fmt, ap);
    }
    uio.iov = iovp = iov;
    uio.resid = 0;
    uio.iovcnt = 0;
    for (;;) {
        for (cp = (char *)fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++) {
        }
        if ((m = fmt - cp) != 0) {
            PRINT(cp, m);
            ret += m;
        }
        if (ch == '\0') {
            goto done;
        }
        fmt++;
        flags = 0;
        dprec = 0;
        width = 0;
        prec = -1;
        sign = '\0';
rflag:
        ch = *fmt++;
reswitch:
        switch (ch) {
        case ' ':
            if (!sign) {
                sign = ' ';
            }
            goto rflag;
        case '#':
            flags |= F_ALT;
            goto rflag;
        case '*':
            width = va_arg(ap, int);
            if (width >= 0) {
                goto rflag;
            }
            width = -width;
            /* fall through */
        case '-':
            flags |= F_LADJUST;
            goto rflag;
        case '+':
            sign = '+';
            goto rflag;
        case '.':
            if ((ch = *fmt++) == '*') {
                n = va_arg(ap, int);
                prec = n < 0 ? -1 : n;
                goto rflag;
            }
            n = 0;
            while ((unsigned int)(ch - '0') < 10) {
                n = 10 * n + (ch - '0');
                ch = *fmt++;
            }
            prec = n < 0 ? -1 : n;
            goto reswitch;
        case '0':
            flags |= F_ZEROPAD;
            goto rflag;
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            n = 0;
            do {
                n = 10 * n + (ch - '0');
                ch = *fmt++;
            } while ((unsigned int)(ch - '0') < 10);
            width = n;
            goto reswitch;
        case 'h':
            flags |= F_SHORTINT;
            goto rflag;
        case 'l':
            if (*fmt == 'l') {
                fmt++;
                flags |= F_QUADINT;
            } else {
                flags |= F_LONGINT;
            }
            goto rflag;
        case 'q':
            flags |= F_QUADINT;
            goto rflag;
        case 'L':
            flags |= F_LONGDBL;
            goto rflag;
        case 'C':
        case 'c':
            cp = buf;
            if (ch == 'C' || (flags & F_LONGINT)) {
                memset(&ps, 0, sizeof(ps));
                size = _wcrtomb_r(data, cp, va_arg(ap, int), &ps);
                if (size == -1) {
                    goto error;
                }
            } else {
                *cp = (char)va_arg(ap, int);
                size = 1;
            }
            sign = '\0';
            break;
        case 'D':
            flags |= F_LONGINT;
            /* fall through */
        case 'd':
        case 'i':
            if (flags & F_QUADINT) {
                uquad = (unsigned long long)va_arg(ap, long long);
            } else if (flags & F_LONGINT) {
                uquad = (unsigned long long)(long long)va_arg(ap, long);
            } else if (flags & F_SHORTINT) {
                uquad = (unsigned long long)(long long)(short)va_arg(ap, int);
            } else {
                uquad = (unsigned long long)(long long)va_arg(ap, int);
            }
            if ((long long)uquad < 0) {
                uquad = -uquad;
                sign = '-';
            }
            base = DEC;
            goto number;
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'G':
            if (prec == -1) {
                prec = 6;
            } else if ((ch == 'g' || ch == 'G') && prec == 0) {
                prec = 1;
            }
            fpvalue = va_arg(ap, double);
            if (func_08809B54(fpvalue)) {
                if (fpvalue < 0) {
                    sign = '-';
                }
                cp = "Inf";
                size = 3;
                break;
            }
            if (func_08809B88(fpvalue)) {
                cp = "NaN";
                size = 3;
                break;
            }
            flags |= F_FPT;
            cp = nl_cvt(data, fpvalue, prec, flags, &softsign, &expt, ch, &ndig);
            if (ch == 'g' || ch == 'G') {
                if (expt <= -4 || expt > prec) {
                    ch = (ch == 'g') ? 'e' : 'E';
                } else {
                    ch = 'g';
                }
            }
            if (ch <= 'e') {
                --expt;
                expsize = nl_exponent(expstr, expt, ch);
                size = expsize + ndig;
                if (ndig > 1 || (flags & F_ALT)) {
                    ++size;
                }
            } else if (ch == 'f') {
                if (expt > 0) {
                    size = expt;
                    if (prec || (flags & F_ALT)) {
                        size += prec + 1;
                    }
                } else {
                    size = (prec || (flags & F_ALT)) ? prec + 2 : 1;
                }
            } else if (expt >= ndig) {
                size = expt;
                if (flags & F_ALT) {
                    ++size;
                }
            } else {
                size = ndig + (expt > 0 ? 1 : 2 - expt);
            }
            if (softsign) {
                sign = '-';
            }
            break;
        case 'n':
            if (flags & F_QUADINT) {
                *va_arg(ap, long long *) = ret;
            } else if (flags & F_LONGINT) {
                *va_arg(ap, long *) = ret;
            } else if (flags & F_SHORTINT) {
                *va_arg(ap, short *) = (short)ret;
            } else {
                *va_arg(ap, int *) = ret;
            }
            continue;
        case 'O':
            flags |= F_LONGINT;
            /* fall through */
        case 'o':
            if (flags & F_QUADINT) {
                uquad = va_arg(ap, unsigned long long);
            } else if (flags & F_LONGINT) {
                uquad = va_arg(ap, unsigned long);
            } else if (flags & F_SHORTINT) {
                uquad = (unsigned short)va_arg(ap, int);
            } else {
                uquad = va_arg(ap, unsigned int);
            }
            base = OCT;
            goto nosign;
        case 'p':
            uquad = (unsigned int)va_arg(ap, void *);
            base = HEX;
            xdigs = "0123456789abcdef";
            flags |= F_HEXPREFIX;
            ch = 'x';
            goto nosign;
        case 's':
        case 'S':
            cp = va_arg(ap, char *);
            sign = '\0';
            if (cp == 0) {
                cp = "(null)";
                size = 6;
            } else if (ch == 'S' || (flags & F_LONGINT)) {
                const int *wcp = (const int *)cp;

                size = m = 0;
                memset(&ps, 0, sizeof(ps));
                if (prec >= 0) {
                    for (;;) {
                        if (wcp[m] == 0) {
                            break;
                        }
                        if ((n = _wcrtomb_r(data, buf, wcp[m], &ps)) == -1) {
                            goto error;
                        }
                        if (n + size > prec) {
                            break;
                        }
                        m += 1;
                        size += n;
                        if (size == prec) {
                            break;
                        }
                    }
                } else {
                    if ((size = _wcsrtombs_r(data, 0, &wcp, 0, &ps)) == -1) {
                        goto error;
                    }
                    wcp = (const int *)cp;
                }
                if (size == 0) {
                    break;
                }
                if ((malloc_buf = nl_malloc_r(data, size + 1)) == 0) {
                    goto error;
                }
                memset(&ps, 0, sizeof(ps));
                if (_wcsrtombs_r(data, malloc_buf, &wcp, size, &ps) != size) {
                    goto error;
                }
                cp = malloc_buf;
                cp[size] = '\0';
            } else if (prec >= 0) {
                char *p = func_0880DBB0(cp, 0, prec);

                if (p != 0) {
                    size = p - cp;
                    if (size > prec) {
                        size = prec;
                    }
                } else {
                    size = prec;
                }
            } else {
                size = strlen(cp);
            }
            break;
        case 'U':
            flags |= F_LONGINT;
            /* fall through */
        case 'u':
            if (flags & F_QUADINT) {
                uquad = va_arg(ap, unsigned long long);
            } else if (flags & F_LONGINT) {
                uquad = va_arg(ap, unsigned long);
            } else if (flags & F_SHORTINT) {
                uquad = (unsigned short)va_arg(ap, int);
            } else {
                uquad = va_arg(ap, unsigned int);
            }
            base = DEC;
            goto nosign;
        case 'X':
            xdigs = "0123456789ABCDEF";
            goto hex;
        case 'x':
            xdigs = "0123456789abcdef";
hex:
            if (flags & F_QUADINT) {
                uquad = va_arg(ap, unsigned long long);
            } else if (flags & F_LONGINT) {
                uquad = va_arg(ap, unsigned long);
            } else if (flags & F_SHORTINT) {
                uquad = (unsigned short)va_arg(ap, int);
            } else {
                uquad = va_arg(ap, unsigned int);
            }
            base = HEX;
            if ((flags & F_ALT) && uquad != 0) {
                flags |= F_HEXPREFIX;
            }
nosign:
            sign = '\0';
number:
            if ((dprec = prec) >= 0) {
                flags &= ~F_ZEROPAD;
            }
            cp = buf + sizeof(buf);
            if (uquad != 0 || prec != 0) {
                switch (base) {
                case OCT:
                    do {
                        *--cp = (char)('0' + (uquad & 7));
                        uquad >>= 3;
                    } while (uquad);
                    if ((flags & F_ALT) && *cp != '0') {
                        *--cp = '0';
                    }
                    break;
                case DEC:
                    while (uquad >= 10) {
                        *--cp = (char)('0' + uquad % 10);
                        uquad /= 10;
                    }
                    *--cp = (char)('0' + uquad);
                    break;
                case HEX:
                    do {
                        *--cp = xdigs[uquad & 15];
                        uquad >>= 4;
                    } while (uquad);
                    break;
                default:
                    cp = "bug in vfprintf: bad base";
                    size = strlen(cp);
                    goto skipsize;
                }
            } else if (base == OCT && (flags & F_ALT)) {
                *--cp = '0';
            }
            size = buf + sizeof(buf) - cp;
skipsize:
            break;
        default:
            if (ch == '\0') {
                goto done;
            }
            cp = buf;
            *cp = (char)ch;
            size = 1;
            sign = '\0';
            break;
        }

        realsz = dprec > size ? dprec : size;
        if (sign) {
            realsz++;
        } else if (flags & F_HEXPREFIX) {
            realsz += 2;
        }
        if ((flags & (F_LADJUST | F_ZEROPAD)) == 0) {
            PAD(width - realsz, nl_blanks);
        }
        if (sign) {
            PRINT(&sign, 1);
        } else if (flags & F_HEXPREFIX) {
            ox[0] = '0';
            ox[1] = (char)ch;
            PRINT(ox, 2);
        }
        if ((flags & (F_LADJUST | F_ZEROPAD)) == F_ZEROPAD) {
            PAD(width - realsz, nl_zeroes);
        }
        PAD(dprec - size, nl_zeroes);
        if ((flags & F_FPT) == 0) {
            PRINT(cp, size);
        } else if (ch >= 'f') {
            if (fpvalue == 0) {
                PRINT("0", 1);
                if (expt < ndig || (flags & F_ALT) != 0) {
                    PRINT(decimal_point, 1);
                    PAD(ndig - 1, nl_zeroes);
                }
            } else if (expt <= 0) {
                PRINT("0", 1);
                if (expt || ndig) {
                    PRINT(decimal_point, 1);
                    PAD(-expt, nl_zeroes);
                    PRINT(cp, ndig);
                }
            } else if (expt >= ndig) {
                PRINT(cp, ndig);
                PAD(expt - ndig, nl_zeroes);
                if (flags & F_ALT) {
                    PRINT(".", 1);
                }
            } else {
                PRINT(cp, expt);
                cp += expt;
                PRINT(".", 1);
                PRINT(cp, ndig - expt);
            }
        } else {
            if (ndig > 1 || (flags & F_ALT)) {
                ox[0] = *cp++;
                ox[1] = '.';
                PRINT(ox, 2);
                if (fpvalue != 0) {
                    PRINT(cp, ndig - 1);
                } else {
                    PAD(ndig - 1, nl_zeroes);
                }
            } else {
                PRINT(cp, 1);
            }
            PRINT(expstr, expsize);
        }
        if (flags & F_LADJUST) {
            PAD(width - realsz, nl_blanks);
        }
        ret += width > realsz ? width : realsz;
        FLUSH();
        if (malloc_buf != 0) {
            free(malloc_buf);
            malloc_buf = 0;
        }
    }
done:
    FLUSH();
error:
    if (malloc_buf != 0) {
        free(malloc_buf);
    }
    return (fp->flags & NL_SERR) ? -1 : ret;
}
#endif

/* Digits of value for %e (ndigits + 1 significant), %f (ndigits after the point) or %g, through
   _dtoa_r; zero-padded to the requested precision except for %g without '#'. */
#ifdef __MWERKS__
asm char *nl_cvt(NlReent *data, double value, int ndigits, int flags, char *sign, int *decpt, int ch, int *length)
{
    addiu sp, sp, -80
    sw s5, 52(sp)
    addiu v0, zero, 102
    lw s5, 80(sp)
    sw s8, 64(sp)
    move s8, t1
    sw s7, 60(sp)
    move s7, a0
    sw s6, 56(sp)
    move s6, t3
    sw s4, 48(sp)
    move s4, t0
    sw s3, 44(sp)
    move s3, a3
    sw s2, 40(sp)
    move s2, a2
    sw s1, 36(sp)
    addiu s1, zero, 3
    sw s0, 32(sp)
    move s0, t2
    beq s5, v0, L_08814F8C
    sw ra, 68(sp)
    xori v1, s5, 0x65
    xori v0, s5, 0x45
    sltiu v1, v1, 1
    sltiu v0, v0, 1
    or v1, v1, v0
    addu s4, t0, v1
    addiu s1, zero, 2
L_08814F8C:
    bltz s3, L_088150C4
    move a0, s2
    sb zero, 0(s0)
L_08814F98:
    move t0, s1
    addiu v0, sp, 16
    move a0, s7
    move a2, s2
    move a3, s3
    move t1, s4
    move t2, s6
    addiu t3, sp, 20
    jal _dtoa_r
    sw v0, 0(sp)
    move s1, v0
    xori v1, s5, 0x67
    xori v0, s5, 0x47
    sltu v1, zero, v1
    sltu v0, zero, v0
    and v1, v1, v0
    bnez v1, L_08814FF0
    addiu v0, zero, 102
    andi v0, s8, 0x1
    beqz v0, L_08815040
    lw v1, 16(sp)
    addiu v0, zero, 102
L_08814FF0:
    beq s5, v0, L_08815080
    addu s0, s1, s4
L_08814FF8:
    move a0, s2
    move a2, zero
    move a3, zero
    jal __eqdf2
    move a1, s3
    lw v1, 16(sp)
    movz v1, s0, v0
    sltu v0, v1, s0
    sw v1, 16(sp)
    beqz v0, L_08815040
    move a0, v1
    addiu a1, zero, 48
L_08815028:
    addiu v1, a0, 1
    sltu v0, v1, s0
    sb a1, 0(a0)
    move a0, v1
    bnez v0, L_08815028
    sw v1, 16(sp)
L_08815040:
    lw v0, 84(sp)
    subu v1, v1, s1
    lw ra, 68(sp)
    sw v1, 0(v0)
    move v0, s1
    lw s8, 64(sp)
    lw s7, 60(sp)
    lw s6, 56(sp)
    lw s5, 52(sp)
    lw s4, 48(sp)
    lw s3, 44(sp)
    lw s2, 40(sp)
    lw s1, 36(sp)
    lw s0, 32(sp)
    jr ra
    addiu sp, sp, 80
L_08815080:
    lb v1, 0(s1)
    addiu v0, zero, 48
    beql v1, v0, L_0881509C
    move a0, s2
L_08815090:
    lw v0, 0(s6)
L_08815094:
    j L_08814FF8
    addu s0, s0, v0
L_0881509C:
    move a2, zero
    move a3, zero
    jal __nedf2
    move a1, s3
    beqzl v0, L_08815094
    lw v0, 0(s6)
    addiu v0, zero, 1
    subu v0, v0, s4
    j L_08815090
    sw v0, 0(s6)
L_088150C4:
    jal __negdf2
    move a1, s3
    move s2, v0
    move s3, v1
    addiu v0, zero, 45
    j L_08814F98
    sb v0, 0(s0)
}
#else
char *nl_cvt(NlReent *data, double value, int ndigits, int flags, char *sign, int *decpt, int ch, int *length)
{
    int mode;
    int dsgn;
    char *digits;
    char *bp;
    char *rve;

    if (ch == 'f') {
        mode = 3;
    } else {
        if (ch == 'e' || ch == 'E') {
            ndigits++;
        }
        mode = 2;
    }
    if (value < 0) {
        value = -value;
        *sign = '-';
    } else {
        *sign = '\0';
    }
    digits = _dtoa_r(data, value, mode, ndigits, decpt, &dsgn, &rve);
    if ((ch != 'g' && ch != 'G') || (flags & F_ALT)) {
        bp = digits + ndigits;
        if (ch == 'f') {
            if (*digits == '0' && value != 0) {
                *decpt = -ndigits + 1;
            }
            bp += *decpt;
        }
        if (value == 0) {
            rve = bp;
        }
        while (rve < bp) {
            *rve++ = '0';
        }
    }
    *length = rve - digits;
    return digits;
}
#endif

/* "e+NN": the exponent letter, sign and at least two digits; returns the length */
#ifdef __MWERKS__
asm int nl_exponent(char *p0, int exp, int fmtch)
{
    addiu sp, sp, -48
    move t1, a0
    bltz a1, L_088151A4
    sb a2, 0(a0)
    addiu v0, zero, 43
    sb v0, 1(a0)
    addiu a3, a0, 2
L_088150FC:
    addiu a2, sp, 40
    slti v0, a1, 10
    bnez v0, L_08815188
    move t2, a2
    lui t0, 0x6666
    ori t0, t0, 0x6667
L_08815114:
    mult a1, t0
    sra a0, a1, 0x1f
    addiu a2, a2, -1
    mfhi v1
    sra v1, v1, 0x2
    subu v1, v1, a0
    sll v0, v1, 0x2
    addu v0, v0, v1
    sll v0, v0, 0x1
    subu v0, a1, v0
    addiu v0, v0, 48
    move a1, v1
    slti v1, v1, 10
    beqz v1, L_08815114
    sb v0, 0(a2)
    addiu a2, a2, -1
    addiu v0, a1, 48
    sltu v1, a2, t2
    beqz v1, L_0881517C
    sb v0, 0(a2)
L_08815164:
    lbu v0, 0(a2)
    addiu a2, a2, 1
    sltu v1, a2, t2
    sb v0, 0(a3)
    bnez v1, L_08815164
    addiu a3, a3, 1
L_0881517C:
    subu v0, a3, t1
    jr ra
    addiu sp, sp, 48
L_08815188:
    addiu v0, zero, 48
    sb v0, 0(a3)
    addiu v1, a1, 48
    addiu a3, a3, 1
    sb v1, 0(a3)
    j L_0881517C
    addiu a3, a3, 1
L_088151A4:
    addiu v0, zero, 45
    negu a1, a1
    addiu a3, a0, 2
    j L_088150FC
    sb v0, 1(a0)
}
#else
int nl_exponent(char *p0, int exp, int fmtch)
{
    char *p = p0;
    char *t;
    char expbuf[8];

    *p++ = (char)fmtch;
    if (exp < 0) {
        exp = -exp;
        *p++ = '-';
    } else {
        *p++ = '+';
    }
    t = expbuf + sizeof(expbuf);
    if (exp > 9) {
        do {
            *--t = (char)('0' + exp % 10);
        } while ((exp /= 10) > 9);
        *--t = (char)('0' + exp);
        for (; t < expbuf + sizeof(expbuf); *p++ = *t++) {
        }
    } else {
        *p++ = '0';
        *p++ = (char)('0' + exp);
    }
    return p - p0;
}
#endif
