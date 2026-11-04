// @range 0x088420B4-0x088433F0
// @asm 0x08842724-0x08843114
/* UCS-2 text formatting: wsprintf (%s %c %d %x %X %f with flags, width, precision and `*`),
   number-to-text helpers, Latin-1/Latin Extended-A case mapping, and the indexed string table. */
#include "psp.h"
#include "stdarg.h"
#include "libc.h"
#include "ehsys.h"
#include "game.h"

typedef struct TableHdr {              /* "CARD_Indx_"-style string tables */
    unsigned int count;
    unsigned int idx_off;              /* offset of unsigned int[count] entry offsets */
    unsigned int data_off;             /* base added to each entry offset */
} TableHdr;

int wcslen(const unsigned short *s);
void wstr_reverse(unsigned short *s);
int wfmt_parse_int(const unsigned short *s, int *out);
int wfmt_int_digits(unsigned short *buf, int v, int width);
int wfmt_hex_digits(unsigned short *buf, int v, int width, unsigned short alpha);
int wfmt_float_digits(float f, unsigned short *buf, int prec);

void wstr_reverse(unsigned short *s)
{
    int n = wcslen(s);
    int i;
    unsigned short t;

    for (i = 0; i < n / 2; i++) {
        t = s[i];
        s[i] = s[n - i - 1];
        s[n - i - 1] = t;
    }
}

/* Parses an optional '-' and decimal digits into *out; returns the number of characters used. */
int wfmt_parse_int(const unsigned short *s, int *out)
{
    int neg = 0;
    int n = 0;
    const unsigned short *p;

    *out = 0;
    if (*s == '-') {
        neg = 1;
        n = neg;
    }
    p = s + n;
    while (*p >= '0' && *p <= '9') {
        n++;
        *out = *out * 10;
        *out = *out + (*p - '0');
        p++;
    }
    if (neg)
        *out = -*out;
    return n;
}

int wfmt_int_digits(unsigned short *buf, int v, int width)
{
    int i = 0;

    if (v < 0)
        v = -v;
    do {
        buf[i] = v % 10 + '0';
        i++;
        v /= 10;
    } while (v > 0);
    while (i < width)
        buf[i++] = '0';
    buf[i] = 0;
    wstr_reverse(buf);
    return i;
}

/* Hexadecimal digits of v (sic: the original divides by 15 and 10). */
int wfmt_hex_digits(unsigned short *buf, int v, int width, unsigned short alpha)
{
    int i = 0;
    int d;

    do {
        d = v % 15;
        if (d < 10) {
            buf[i] = d + '0';
            i++;
        } else {
            buf[i] = d + alpha;
            i++;
        }
        v /= 10;
    } while (v > 0);
    while (i < width)
        buf[i++] = '0';
    buf[i] = 0;
    wstr_reverse(buf);
    return i;
}

int wfmt_float_digits(float f, unsigned short *buf, int prec)
{
    int i = 0;
    int n = (int)f;
    int k;

    if (n < 0)
        n = -n;
    do {
        buf[i] = n % 10 + '0';
        i++;
        n /= 10;
    } while (n > 0);
    buf[i] = 0;
    wstr_reverse(buf);
    buf[i] = '.';
    i++;
    for (k = 0; k < prec; k++) {
        f = f * 10.0f;
        buf[i] = (int)f % 10 + '0';
        i++;
    }
    buf[i] = 0;
    return i;
}

#ifdef NON_MATCHING
/* Forma igual ao original (buf[64], switch nas flags e na conversão, fmt copiado num local, `if (sign)`):
   os $s e o quadro (0x100) batem; faltam ~276 palavras só de temporários: o original nunca usa a2/a3
   como temporários (o contador dos laços de preenchimento fica em t0, pad em v0, sign em t1), o mwcc
   usa a2/a3 primeiro. Ordem de declarações não muda isso (hill-climb). */
int wsprintf(unsigned short *out, const unsigned short *format, ...)
{
    unsigned short buf[64];
    int width;
    int prec;
    va_list ap;
    int v;
    const unsigned short *fmt;
    int kind;
    unsigned short *s;
    unsigned short sign;
    unsigned short c;
    int n;
    int pad;
    int zero_pad;
    int len;
    unsigned short flag;
    int j;
    int k;

    width = 0;
    prec = 0;
    n = 0;
    len = 0;
    flag = 0;
    zero_pad = 0;
    va_start(ap, format);
    fmt = format;
    while ((c = *fmt) != 0) {
        if (c != '%') {
            out[n++] = c;
        } else {
            kind = 0;
            fmt++;
            switch (*fmt) {
            case '-':
            case '+':
            case ' ':
            case '#':
                flag = *fmt;
                fmt++;
                break;
            }
            if (*fmt == '0') {
                fmt++;
                zero_pad = 1;
            }
            if (*fmt == '*') {
                width = va_arg(ap, int);
                fmt++;
            } else {
                fmt += wfmt_parse_int(fmt, &width);
            }
            if (*fmt == '.')
                fmt++;
            if (*fmt == '*') {
                prec = va_arg(ap, int);
                fmt++;
            } else {
                fmt += wfmt_parse_int(fmt, &prec);
            }
            switch (*fmt) {
            case 'd':
                v = va_arg(ap, int);
                kind = 1;
                if (v < 0)
                    kind |= 2;
                len = wfmt_int_digits(buf, v, prec);
                break;
            case 'x':
            case 'X':
                kind = 1;
                len = wfmt_hex_digits(buf, va_arg(ap, int), prec, *fmt - 0x17);
                if (flag == '#') {
                    out[n] = '0';
                    out[n + 1] = *fmt;
                    n += 2;
                }
                break;
            case 'f':
                {
                    float f;
                    ap += (((unsigned int)ap & 7) ? 4 : 0) + 8;
                    f = (float)*(double *)(ap - 8);
                    kind = 1;
                    if (f < 0.0f)
                        kind |= 2;
                    len = wfmt_float_digits(f, buf, prec == 0 ? 6 : prec);
                }
                break;
            case 'c':
                out[n++] = *(unsigned short *)((ap += 4) - 4);
                break;
            case 's':
                s = va_arg(ap, unsigned short *);
                len = wcslen(s);
                if (prec > 0 && prec < len)
                    len = prec;
                if (flag != '-') {
                    for (j = 0; j < width - len; j++)
                        out[n++] = ' ';
                }
                for (k = 0; k < len; k++)
                    out[n++] = *s++;
                if (flag == '-') {
                    while (k < width) {
                        out[n++] = ' ';
                        k++;
                    }
                }
                break;
            case '%':
                out[n++] = *fmt;
                break;
            }
            if (kind != 0) {
                sign = 0;
                pad = width - len;
                if ((kind >> 1) & 1)
                    sign = '-';
                else if (flag == '+')
                    sign = '+';
                else if (flag == ' ')
                    sign = ' ';
                if (sign)
                    pad--;
                if (flag != '-' && !zero_pad) {
                    for (j = 0; j < pad; j++)
                        out[n++] = ' ';
                }
                if (sign != 0)
                    out[n++] = sign;
                if (flag != '-' && zero_pad) {
                    for (j = 0; j < pad; j++)
                        out[n++] = '0';
                }
                for (k = 0; k < len; k++)
                    out[n++] = buf[k];
                if (flag == '-') {
                    for (j = 0; j < pad; j++)
                        out[n++] = ' ';
                }
            }
        }
        fmt++;
    }
    out[n] = 0;
    return n;
}
#endif

unsigned short *wtext_from_int(int v, unsigned short *out)
{
    int i = 0;
    int neg = 0;

    if (v < 0) {
        v = -v;
        neg = 1;
    }
    do {
        out[i] = v % 10 + '0';
        i++;
        v /= 10;
    } while (v > 0);
    if (neg) {
        out[i] = '-';
        i++;
    }
    out[i] = 0;
    wstr_reverse(out);
    return out;
}

int wc_is_upper(unsigned short c)
{
    if ((c >= 0x41 && c <= 0x5A) || (c >= 0xC0 && c <= 0xDE))
        return 1;
    if (c >= 0x100 && c < 0x180 && !(c & 1))
        return 1;
    return 0;
}

unsigned short towupper(unsigned short c)
{
    if ((c >= 0x61 && c <= 0x7A) || (c >= 0xE0 && c <= 0xFE))
        return c - 0x20;
    if (c >= 0x100 && c < 0x180 && (c & 1))
        return c & 0xFFFE;
    return c;
}

int wc_is_lower(unsigned short c)
{
    if ((c >= 0x61 && c <= 0x7A) || (c >= 0xE0 && c <= 0xFE))
        return 1;
    if (c >= 0x100 && c < 0x180 && (c & 1))
        return 1;
    return 0;
}

unsigned short towlower(unsigned short c)
{
    if ((c >= 0x41 && c <= 0x5A) || (c >= 0xC0 && c <= 0xDE))
        return c + 0x20;
    if (c >= 0x100 && c < 0x180 && !(c & 1))
        return c + 1;
    return c;
}

unsigned short *table_get_entry(TableHdr *t, unsigned int i)
{
    unsigned int *idx;
    char *base;

    if (t != 0) {
        idx = (unsigned int *)((char *)t + t->idx_off);
        base = (char *)t + t->data_off;
        if (i < t->count)
            return (unsigned short *)(base + idx[i]);
    }
    return 0;
}

int text_ret_zero(void)
{
    return 0;
}
