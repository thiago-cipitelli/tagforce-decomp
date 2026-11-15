// @range 0x08816B1C-0x08816C3C
// @gcc 0x08816B1C-0x08816C3C nl_wsetup
/* newlib __swsetup (stdio/wsetup.c): makes a FILE ready for writing. Compiled with GCC, kept as a
   whole `asm` function; the C below is the newlib code, used outside the PSP build. */
#include "nl_stdio.h"

void _free_r(NlReent *r, void *p); /* _free_r */
void libc_smakebuf(NlFile *fp);          /* __smakebuf */

int nl_swsetup(NlFile *fp);
#ifndef __MWERKS__ /* PSP: nl_wsetup.c */
int nl_swsetup(NlFile *fp)
{
    if (nl_getreent()->sdidinit == 0) {
        nl_sinit(nl_getreent());
    }
    if ((fp->flags & NL_SWR) == 0) {
        if ((fp->flags & NL_SRW) == 0) {
            return -1;
        }
        if (fp->flags & NL_SRD) {
            if (fp->ub.base != 0) {
                if (fp->ub.base != fp->ubuf) {
                    _free_r(nl_getreent(), fp->ub.base);
                }
                fp->ub.base = 0;
            }
            fp->flags &= ~(NL_SRD | NL_SEOF);
            fp->r = 0;
            fp->p = fp->bf.base;
        }
        fp->flags |= NL_SWR;
    }
    if (fp->bf.base == 0) {
        libc_smakebuf(fp);
    }
    if (fp->flags & NL_SLBF) {
        fp->w = 0;
        fp->lbfsize = -fp->bf.size;
    } else {
        fp->w = (fp->flags & NL_SNBF) ? 0 : fp->bf.size;
    }
    return 0;
}
#endif
