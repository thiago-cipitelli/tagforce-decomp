// @range 0x088FF0DC-0x088FF3FC
/* Memory helpers and UCS-2 text helpers; a text argument below 0x20000 is a dialog text id. */
#include "libc.h"

#define TEXT_ID(p) (((unsigned int)(p) & 0xFFFE0000) == 0)

unsigned short *dlg_get_text(int i);
unsigned short *wcscpy(unsigned short *dst, const unsigned short *src);
unsigned short *wcscat(unsigned short *dst, const unsigned short *src);

void duel_bzero(void *p, int n)
{
    memset(p, 0, n);
}

void duel_memcpy(void *dst, void *src, int n)
{
    memcpy(dst, src, n);
}

void duel_wcscpy(unsigned short *dst, unsigned short *src)
{
    if (TEXT_ID(src)) {
        src = dlg_get_text((int)src);
    }
    wcscpy(dst, src);
}

void duel_wcscat(unsigned short *dst, unsigned short *src)
{
    if (TEXT_ID(src)) {
        src = dlg_get_text((int)src);
    }
    wcscat(dst, src);
}

/* Append n in decimal. */
void duel_wcscat_int(unsigned short *dst, int n)
{
    char buf[0x20];
    int i = 0x20;

    buf[--i] = 0;
    buf[--i] = 0;
    do {
        buf[--i] = 0;
        buf[--i] = '0' + n % 10;
        n /= 10;
    } while (n != 0);
    duel_wcscat(dst, (unsigned short *)&buf[i]);
}

/* Copy fmt to dst replacing the first "%s" with arg. Parameters copied to locals in reverse order (arg, fmt,
   dst) for the original's $s2/$s1/$s0; the const copy of fmt is a type change, so mwcc keeps the copy and tests
   it instead of the argument register. */
void duel_wfmt_s(char *dst, char *fmt, unsigned short *arg)
{
    unsigned short *a = arg;
    const char *f = fmt;
    char *d = dst;
    char c;

    d[1] = 0;
    d[0] = 0;
    if (TEXT_ID(f)) {
        f = (char *)dlg_get_text((int)f);
    }
    if (TEXT_ID(a)) {
        a = dlg_get_text((int)a);
    }
    if (f == 0) {
        return;
    }
    while ((c = f[0]) != 0 || f[1] != 0) {
        if (c == '%' && f[1] == 0 && f[2] == 's' && f[3] == 0) {
            d[1] = 0;
            d[0] = 0;
            duel_wcscat((unsigned short *)d, a);
            duel_wcscat((unsigned short *)d, (unsigned short *)(f + 4));
            return;
        }
        d[0] = c;
        d[1] = f[1];
        f += 2;
        d += 2;
    }
    d[1] = 0;
    d[0] = 0;
}

/* Copy fmt to dst replacing the first "%d" with n (same shape as duel_wfmt_s). */
void duel_wfmt_d(char *dst, char *fmt, int n)
{
    int v = n;
    const char *f = fmt;
    char *d = dst;
    char c;

    d[1] = 0;
    d[0] = 0;
    if (TEXT_ID(f)) {
        f = (char *)dlg_get_text((int)f);
    }
    while ((c = f[0]) != 0 || f[1] != 0) {
        if (c == '%' && f[1] == 0 && f[2] == 'd' && f[3] == 0) {
            d[1] = 0;
            d[0] = 0;
            duel_wcscat_int((unsigned short *)d, v);
            duel_wcscat((unsigned short *)d, (unsigned short *)(f + 4));
            return;
        }
        d[0] = c;
        d[1] = f[1];
        f += 2;
        d += 2;
    }
    d[1] = 0;
    d[0] = 0;
}
