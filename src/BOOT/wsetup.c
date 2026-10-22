// @range 0x08816B1C-0x08816C3C
/* newlib __swsetup (stdio/wsetup.c): makes a FILE ready for writing. Compiled with GCC, kept as a
   whole `asm` function; the C below is the newlib code, used outside the PSP build. */
#include "nl_stdio.h"

void func_0880C4D0(NlReent *r, void *p); /* _free_r */
void func_0880CF9C(NlFile *fp);          /* __smakebuf */

#ifdef __MWERKS__
asm int nl_swsetup(NlFile *fp)
{
    addiu sp, sp, -16
    sw s0, 0(sp)
    sw ra, 4(sp)
    jal heap_get_current
    move s0, a0
    lw v1, 56(v0)
    beqz v1, L_08816C24
    nop
    lhu a1, 12(s0)
L_08816B40:
    andi v1, a1, 0xffff
    andi v0, v1, 0x8
    bnezl v0, L_08816BC0
    lw v1, 16(s0)
    andi v0, v1, 0x10
    beqz v0, L_08816BEC
    addiu a0, zero, -1
    andi v0, v1, 0x4
    beqzl v0, L_08816BB8
    lw v1, 16(s0)
    lw v1, 48(s0)
    beqzl v1, L_08816BA0
    lw v1, 16(s0)
    addiu v0, s0, 64
    beql v1, v0, L_08816B9C
    sw zero, 48(s0)
    jal heap_get_current
    nop
    lw a1, 48(s0)
    jal func_0880C4D0
    move a0, v0
    lhu a1, 12(s0)
    sw zero, 48(s0)
L_08816B9C:
    lw v1, 16(s0)
L_08816BA0:
    addiu v0, zero, -37
    and v0, a1, v0
    move a1, v0
    sh v0, 12(s0)
    sw zero, 4(s0)
    sw v1, 0(s0)
L_08816BB8:
    ori v0, a1, 0x8
    sh v0, 12(s0)
L_08816BC0:
    beqz v1, L_08816C14
    nop
    lhu v1, 12(s0)
L_08816BCC:
    andi v0, v1, 0x1
    beqz v0, L_08816C00
    andi v0, v1, 0x2
    lw v0, 20(s0)
    sw zero, 8(s0)
    negu v0, v0
    sw v0, 24(s0)
L_08816BE8:
    move a0, zero
L_08816BEC:
    lw ra, 4(sp)
    lw s0, 0(sp)
    move v0, a0
    jr ra
    addiu sp, sp, 16
L_08816C00:
    bnez v0, L_08816C0C
    move v1, zero
    lw v1, 20(s0)
L_08816C0C:
    j L_08816BE8
    sw v1, 8(s0)
L_08816C14:
    jal func_0880CF9C
    move a0, s0
    j L_08816BCC
    lhu v1, 12(s0)
L_08816C24:
    jal heap_get_current
    nop
    jal func_0880C330
    move a0, v0
    j L_08816B40
    lhu a1, 12(s0)
}
#else
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
                    func_0880C4D0(nl_getreent(), fp->ub.base);
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
        func_0880CF9C(fp);
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
