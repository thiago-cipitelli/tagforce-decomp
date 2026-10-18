// @range 0x088FF0DC-0x088FF3FC
// @asm 0x088FF1F4-0x088FF3FC
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

#ifdef NON_MATCHING /* same code; the original keeps dst in $s0 and the argument in $s2 */
/* Copy fmt to dst replacing the first "%s" with arg. */
void duel_wfmt_s(char *dst, char *fmt, unsigned short *arg)
{
    char c;

    dst[1] = 0;
    dst[0] = 0;
    if (TEXT_ID(fmt)) {
        fmt = (char *)dlg_get_text((int)fmt);
    }
    if (TEXT_ID(arg)) {
        arg = dlg_get_text((int)arg);
    }
    if (fmt == 0) {
        return;
    }
    while ((c = fmt[0]) != 0 || fmt[1] != 0) {
        if (c == '%' && fmt[1] == 0 && fmt[2] == 's' && fmt[3] == 0) {
            dst[1] = 0;
            dst[0] = 0;
            duel_wcscat((unsigned short *)dst, arg);
            duel_wcscat((unsigned short *)dst, (unsigned short *)(fmt + 4));
            return;
        }
        dst[0] = c;
        dst[1] = fmt[1];
        fmt += 2;
        dst += 2;
    }
    dst[1] = 0;
    dst[0] = 0;
}
#endif

#ifdef NON_MATCHING /* same code; the original keeps dst in $s0 and the argument in $s2 */
/* Copy fmt to dst replacing the first "%d" with n. */
void duel_wfmt_d(char *dst, char *fmt, int n)
{
    char c;

    dst[1] = 0;
    dst[0] = 0;
    if (TEXT_ID(fmt)) {
        fmt = (char *)dlg_get_text((int)fmt);
    }
    while ((c = fmt[0]) != 0 || fmt[1] != 0) {
        if (c == '%' && fmt[1] == 0 && fmt[2] == 'd' && fmt[3] == 0) {
            dst[1] = 0;
            dst[0] = 0;
            duel_wcscat_int((unsigned short *)dst, n);
            duel_wcscat((unsigned short *)dst, (unsigned short *)(fmt + 4));
            return;
        }
        dst[0] = c;
        dst[1] = fmt[1];
        fmt += 2;
        dst += 2;
    }
    dst[1] = 0;
    dst[0] = 0;
}
#endif
