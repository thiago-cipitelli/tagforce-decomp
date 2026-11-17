// @range 0x08813108-0x08813390
// @gcc 0x08813108-0x08813390 nl_ungetc
/* newlib ungetc (stdio/ungetc.c) with its buffer growth helper, compiled with GCC and kept as whole
   `asm` functions; the C below is the newlib code, used outside the PSP build. */
#include "libc.h"
#include "nl_stdio.h"

/* Grows the ungetc buffer: the 3-byte ubuf moves to a malloc'd BUFSIZ block (pushed bytes at its
   end), a malloc'd one doubles with its contents moved to the upper half. 0, or -1 without memory. */
int nl_submore(NlFile *fp);
#ifndef __MWERKS__ /* PSP: nl_ungetc.c */
int nl_submore(NlFile *fp)
{
    unsigned char *p;
    int i;

    if (fp->ub.base == fp->ubuf) {
        p = nl_malloc_r(nl_getreent(), 1024);
        if (p == 0) {
            return -1;
        }
        fp->ub.base = p;
        fp->ub.size = 1024;
        p += 1024 - sizeof(fp->ubuf);
        for (i = sizeof(fp->ubuf); --i >= 0;) {
            p[i] = fp->ubuf[i];
        }
        fp->p = p;
        return 0;
    }
    i = fp->ub.size;
    p = nl_realloc_r(nl_getreent(), fp->ub.base, i << 1);
    if (p == 0) {
        return -1;
    }
    memcpy(p + i, p, i);
    fp->p = p + i;
    fp->ub.base = p;
    fp->ub.size = i << 1;
    return 0;
}
#endif

#ifndef __MWERKS__ /* PSP: nl_ungetc.c */
int ungetc(int c, NlFile *fp)
{
    if (c == -1) {
        return -1;
    }
    if (nl_getreent()->sdidinit == 0) {
        nl_sinit(nl_getreent());
    }
    fp->flags &= ~NL_SEOF;
    if ((fp->flags & NL_SRD) == 0) {
        if ((fp->flags & NL_SRW) == 0) {
            return -1;
        }
        if (fp->flags & NL_SWR) {
            if (nl_fflush(fp)) {
                return -1;
            }
            fp->flags &= ~NL_SWR;
            fp->w = 0;
            fp->lbfsize = 0;
        }
        fp->flags |= NL_SRD;
    }
    c = (unsigned char)c;
    if (fp->ub.base != 0) {
        if (fp->r >= fp->ub.size && nl_submore(fp)) {
            return -1;
        }
        *--fp->p = (unsigned char)c;
        fp->r++;
        return c;
    }
    if (fp->bf.base != 0 && fp->p > fp->bf.base && fp->p[-1] == c) {
        fp->p--;
        fp->r++;
        return c;
    }
    fp->ur = fp->r;
    fp->up = fp->p;
    fp->ub.base = fp->ubuf;
    fp->ub.size = sizeof(fp->ubuf);
    fp->ubuf[sizeof(fp->ubuf) - 1] = (unsigned char)c;
    fp->p = &fp->ubuf[sizeof(fp->ubuf) - 1];
    fp->r = 1;
    return c;
}
#endif
