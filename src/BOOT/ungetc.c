// @range 0x08813108-0x08813390
/* newlib ungetc (stdio/ungetc.c) with its buffer growth helper, compiled with GCC and kept as whole
   `asm` functions; the C below is the newlib code, used outside the PSP build. */
#include "libc.h"
#include "nl_stdio.h"

/* Grows the ungetc buffer: the 3-byte ubuf moves to a malloc'd BUFSIZ block (pushed bytes at its
   end), a malloc'd one doubles with its contents moved to the upper half. 0, or -1 without memory. */
#ifdef __MWERKS__
asm int nl_submore(NlFile *fp)
{
    addiu sp, sp, -32
    sw s2, 8(sp)
    addiu v0, a0, 64
    sw ra, 20(sp)
    sw s4, 16(sp)
    sw s3, 12(sp)
    sw s1, 4(sp)
    sw s0, 0(sp)
    lw v1, 48(a0)
    beq v1, v0, L_088131A0
    move s2, a0
    jal heap_get_current
    lw s3, 52(a0)
    lw a1, 48(s2)
    sll s4, s3, 0x1
    move a0, v0
    jal _realloc_r
    move a2, s4
    move s1, v0
    beqz s1, L_08813180
    addiu v0, zero, -1
    addu s0, s1, s3
    move a2, s3
    move a0, s0
    jal memcpy
    move a1, s1
    sw s4, 52(s2)
    move v0, zero
    sw s0, 0(s2)
    sw s1, 48(s2)
L_08813180:
    lw ra, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 32
L_088131A0:
    jal heap_get_current
    nop
    move a0, v0
    jal _malloc_r
    addiu a1, zero, 1024
    move s1, v0
    beqz s1, L_08813180
    addiu v0, zero, -1
    addiu v0, zero, 1024
    sw v0, 52(s2)
    addiu s3, zero, 2
    sw s1, 48(s2)
    addiu s1, s1, 1021
L_088131D4:
    addu v0, s3, s2
    lbu a0, 64(v0)
    addu v1, s1, s3
    addiu s3, s3, -1
    bgez s3, L_088131D4
    sb a0, 0(v1)
    move v0, zero
    j L_08813180
    sw s1, 0(s2)
}
#else
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

#ifdef __MWERKS__
asm int ungetc(int c, NlFile *fp)
{
    addiu sp, sp, -16
    addiu v0, zero, -1
    sw s1, 4(sp)
    addiu a3, zero, -1
    move s1, a0
    sw s0, 0(sp)
    move s0, a1
    beq a0, v0, L_088132AC
    sw ra, 8(sp)
    jal heap_get_current
    nop
    lw v1, 56(v0)
    beqz v1, L_08813378
    nop
    lhu v0, 12(s0)
L_08813234:
    ins v0, zero, 0x5, 0x1
    andi a0, v0, 0xffdf
    andi v1, a0, 0x4
    bnez v1, L_0881326C
    sh v0, 12(s0)
    andi v0, a0, 0x10
    beqz v0, L_088132AC
    addiu a3, zero, -1
    andi v0, a0, 0x8
    bnez v0, L_08813350
    nop
L_08813260:
    lhu v0, 12(s0)
    ori v0, v0, 0x4
    sh v0, 12(s0)
L_0881326C:
    lw v0, 48(s0)
    beqz v0, L_088132DC
    andi s1, s1, 0xff
    lw v0, 4(s0)
    lw v1, 52(s0)
    slt v0, v0, v1
    beqz v0, L_088132C4
    nop
L_0881328C:
    lw v1, 0(s0)
    move a3, s1
    addiu v0, v1, -1
    sw v0, 0(s0)
    sb s1, -1(v1)
    lw v0, 4(s0)
    addiu v0, v0, 1
    sw v0, 4(s0)
L_088132AC:
    lw ra, 8(sp)
L_088132B0:
    lw s1, 4(sp)
    lw s0, 0(sp)
    move v0, a3
    jr ra
    addiu sp, sp, 16
L_088132C4:
    jal nl_submore
    move a0, s0
    beqz v0, L_0881328C
    addiu a3, zero, -1
    j L_088132B0
    lw ra, 8(sp)
L_088132DC:
    lw v0, 16(s0)
    beqz v0, L_08813300
    lw a2, 0(s0)
    sltu v0, v0, a2
    beqzl v0, L_08813304
    lw v1, 4(s0)
    lbu v0, -1(a2)
    beql v0, s1, L_08813338
    lw v0, 4(s0)
L_08813300:
    lw v1, 4(s0)
L_08813304:
    addiu v0, zero, 1
    addiu a0, s0, 64
    addiu a1, s0, 66
    sw v0, 4(s0)
    addiu v0, zero, 3
    move a3, s1
    sw v1, 60(s0)
    sw a2, 56(s0)
    sw a0, 48(s0)
    sw v0, 52(s0)
    sw a1, 0(s0)
    j L_088132AC
    sb s1, 66(s0)
L_08813338:
    addiu v1, a2, -1
    move a3, s1
    addiu v0, v0, 1
    sw v0, 4(s0)
    j L_088132AC
    sw v1, 0(s0)
L_08813350:
    jal fflush
    move a0, s0
    bnez v0, L_088132AC
    addiu a3, zero, -1
    lhu v0, 12(s0)
    sw zero, 8(s0)
    ins v0, zero, 0x3, 0x1
    sh v0, 12(s0)
    j L_08813260
    sw zero, 24(s0)
L_08813378:
    jal heap_get_current
    nop
    jal libc_sinit
    move a0, v0
    j L_08813234
    lhu v0, 12(s0)
}
#else
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
