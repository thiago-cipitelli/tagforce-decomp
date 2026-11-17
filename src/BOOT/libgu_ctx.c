// @range 0x08807070-0x088087F8
// @gcc 0x08807070-0x0880712C
// @asm 0x0880712C-0x08807338
// @gcc 0x08807C6C-0x08807D40
// @gcc 0x08807DD0-0x08807FD8
// @gcc 0x08808058-0x08808130
// @asm 0x08808130-0x0880823C
// @gcc 0x0880823C-0x08808294
// @asm 0x08808294-0x088086F0
// @gcc 0x088086F0-0x088087F8
/* libgu of the Sony SDK (GCC), context level: the builders that write into a given GuList (state
   enables, matrices, sprites, clear, depth range, lights, ...), the GE signal/finish handlers, the
   context reset, the stall update and the sceGuDebugPrint/Flush text overlay. The builders do not
   touch globals and are kept as the original instructions (`asm` functions, portable C under
   `#else`); the rest reads the libgu globals or uses jump tables and stays in the split asm with
   its C under NON_MATCHING. */
#include "libgu.h"

int sceKernelSetEventFlag(int evf, unsigned int bits);

#ifndef __MWERKS__
static void ctx_push(GuList *l, unsigned int w)
{
    *l->cur++ = w;
}

/* BASE + 24-bit address command */
static void ctx_addr(GuList *l, unsigned int cmd, unsigned int a)
{
    ctx_push(l, (((a & 0xFFFFFFF) >> 24) << 16) | 0x10000000);
    ctx_push(l, (a & 0xFFFFFF) | cmd);
}

static unsigned int ctx_f24(float f)
{
    union {
        float f;
        unsigned int i;
    } u;

    u.f = f;
    return u.i >> 8;
}
#endif

/* SIGNAL (1), END/FINISH variants: 1 = signal + END, 2 = signal (wait) + END, 3 = signal, JUMP-like
   pair and END */
#ifndef __MWERKS__ /* PSP: libgu_ctx_gcc.c */
void gu_ctx_finish(GuList *l, int mode, unsigned int arg)
{
    arg &= 0xFFFF;
    switch (mode) {
    case 1:
        ctx_push(l, arg | 0x0E010000);
        ctx_push(l, 0x0C000000);
        break;
    case 2:
        ctx_push(l, arg | 0x0E020000);
        ctx_push(l, 0x0C000000);
        break;
    case 3:
        ctx_push(l, arg | 0x0E030000);
        ctx_push(l, 0x0C000000);
        ctx_push(l, 0x0F000000);
        ctx_push(l, 0x0C000000);
        break;
    }
}
#else
void gu_ctx_finish(GuList *l, int mode, unsigned int arg);
#endif

/* Not in the SDK's GCC object (libgu_ctx_gcc.c): its jump table would come first in the object's .rodata, and the
   code still differs in register allocation (the context pointer in a3 instead of t0: global allocation, not
   regrename; case order, parameter types and a local copy do not move it), so the table stays in the asm data and
   the object's .rodata is only the table of gu_ctx_light_color. */
#if defined(NON_MATCHING) && (!defined(__mips__) || defined(__MWERKS__))
/* the enable command of each GU_* state (0..20, a jump table); 2 = scissor test (SCISSOR1/2 from the context or
   the whole display), 21 = color doubling (TFUNC bit 16) */
void gu_ctx_state(GuList *l, int state, int on)
{
    switch (state) {
    case 0:
        ctx_push(l, on | 0x22000000);
        break;
    case 1:
        ctx_push(l, on | 0x23000000);
        break;
    case 2: {
        unsigned int *p;
        unsigned int a;
        unsigned int b;

        if (on) {
            p = l->cur;
            a = l->scissor[1] << 10 | l->scissor[0];
            b = l->scissor[3] << 10 | l->scissor[2];
            l->scissor_on = 1;
            *p++ = a | 0xD4000000;
        } else {
            p = l->cur;
            b = (l->disp_h - 1) << 10 | (l->disp_w - 1);
            l->scissor_on = 0;
            *p++ = 0xD4000000;
        }
        p[0] = b | 0xD5000000;
        p[1] = 0x15000000;
        p[2] = b | 0x16000000;
        l->cur = p + 3;
        break;
    }
    case 3:
        ctx_push(l, on | 0x24000000);
        break;
    case 4:
        ctx_push(l, on | 0x21000000);
        break;
    case 5:
        ctx_push(l, on | 0x1D000000);
        break;
    case 6:
        ctx_push(l, on | 0x20000000);
        break;
    case 7:
        ctx_push(l, on | 0x1F000000);
        break;
    case 8:
        ctx_push(l, on | 0x1C000000);
        break;
    case 9:
        ctx_push(l, on | 0x1E000000);
        break;
    case 10:
        ctx_push(l, on | 0x17000000);
        break;
    case 11:
        ctx_push(l, on | 0x18000000);
        break;
    case 12:
        ctx_push(l, on | 0x19000000);
        break;
    case 13:
        ctx_push(l, on | 0x1A000000);
        break;
    case 14:
        ctx_push(l, on | 0x1B000000);
        break;
    case 15:
        ctx_push(l, on | 0x25000000);
        break;
    case 16:
        ctx_push(l, on | 0x26000000);
        break;
    case 17:
        ctx_push(l, on | 0x27000000);
        break;
    case 18:
        ctx_push(l, on | 0x28000000);
        break;
    case 19:
        ctx_push(l, on | 0x51000000);
        break;
    case 20:
        ctx_push(l, on | 0x38000000);
        break;
    case 21:
        if (on) {
            l->pad38[10] = 1;
        } else {
            l->pad38[10] = 0;
        }
        ctx_push(l, l->pad38[10] << 16 | l->pad38[12] << 8 | l->pad38[11] | 0xC9000000);
        break;
    }
}
#endif

/* GU_PROJECTION (0, 4x4) / GU_VIEW (2) / GU_MODEL (1) / GU_TEXTURE (3): index command then the
   float24 elements (4x3 for all but the projection) */
#ifdef __MWERKS__
asm void gu_ctx_matrix(GuList *l, int type, const float *m)
{
    addiu v1, zero, 1
    beq a1, v1, @25c
    slti v1, a1, 2
    bnez v1, @178
    addiu a3, zero, 2
    beq a1, a3, @d0
    addiu t0, zero, 3
    beql a1, t0, @2c
    lw v0, 8(a0)
    jr ra
    nop
@2c:
    lui t0, 0x4000
    sw t0, 0(v0)
    lui t0, 0x4100
    lui t1, 0x4100
    lui t2, 0x4100
    lwr t0, 1(a2)
    lwr t1, 5(a2)
    lwr t2, 9(a2)
    sw t0, 4(v0)
    sw t1, 8(v0)
    sw t2, 12(v0)
    lui t0, 0x4100
    lui t1, 0x4100
    lui t2, 0x4100
    lwr t0, 17(a2)
    lwr t1, 21(a2)
    lwr t2, 25(a2)
    sw t0, 16(v0)
    sw t1, 20(v0)
    sw t2, 24(v0)
    lui t0, 0x4100
    lui t1, 0x4100
    lui t2, 0x4100
    lwr t0, 33(a2)
    lwr t1, 37(a2)
    lwr t2, 41(a2)
    sw t0, 28(v0)
    sw t1, 32(v0)
    sw t2, 36(v0)
    lui t0, 0x4100
    lui t1, 0x4100
    lui t2, 0x4100
    lwr t0, 49(a2)
    lwr t1, 53(a2)
    lwr t2, 57(a2)
    sw t0, 40(v0)
    sw t1, 44(v0)
    sw t2, 48(v0)
    addiu v0, v0, 52
@c8:
    jr ra
    sw v0, 8(a0)
@d0:
    lw v0, 8(a0)
    lui t0, 0x3a00
    sw t0, 0(v0)
    lui t0, 0x3b00
    lui t1, 0x3b00
    lui t2, 0x3b00
    lwr t0, 1(a2)
    lwr t1, 5(a2)
    lwr t2, 9(a2)
    sw t0, 4(v0)
    sw t1, 8(v0)
    sw t2, 12(v0)
    lui t0, 0x3b00
    lui t1, 0x3b00
    lui t2, 0x3b00
    lwr t0, 17(a2)
    lwr t1, 21(a2)
    lwr t2, 25(a2)
    sw t0, 16(v0)
    sw t1, 20(v0)
    sw t2, 24(v0)
    lui t0, 0x3b00
    lui t1, 0x3b00
    lui t2, 0x3b00
    lwr t0, 33(a2)
    lwr t1, 37(a2)
    lwr t2, 41(a2)
    sw t0, 28(v0)
    sw t1, 32(v0)
    sw t2, 36(v0)
    lui t0, 0x3b00
    lui t1, 0x3b00
    lui t2, 0x3b00
    lwr t0, 49(a2)
    lwr t1, 53(a2)
    lwr t2, 57(a2)
    sw t0, 40(v0)
    sw t1, 44(v0)
    sw t2, 48(v0)
    addiu v0, v0, 52
    j @c8
    nop
@178:
    beqzl a1, @188
    lw v0, 8(a0)
    jr ra
    nop
@188:
    lui t0, 0x3e00
    sw t0, 0(v0)
    lui t0, 0x3f00
    lui t1, 0x3f00
    lui t2, 0x3f00
    lui t3, 0x3f00
    lwr t0, 1(a2)
    lwr t1, 5(a2)
    lwr t2, 9(a2)
    lwr t3, 13(a2)
    sw t0, 4(v0)
    sw t1, 8(v0)
    sw t2, 12(v0)
    sw t3, 16(v0)
    lui t0, 0x3f00
    lui t1, 0x3f00
    lui t2, 0x3f00
    lui t3, 0x3f00
    lwr t0, 17(a2)
    lwr t1, 21(a2)
    lwr t2, 25(a2)
    lwr t3, 29(a2)
    sw t0, 20(v0)
    sw t1, 24(v0)
    sw t2, 28(v0)
    sw t3, 32(v0)
    lui t0, 0x3f00
    lui t1, 0x3f00
    lui t2, 0x3f00
    lui t3, 0x3f00
    lwr t0, 33(a2)
    lwr t1, 37(a2)
    lwr t2, 41(a2)
    lwr t3, 45(a2)
    sw t0, 36(v0)
    sw t1, 40(v0)
    sw t2, 44(v0)
    sw t3, 48(v0)
    lui t0, 0x3f00
    lui t1, 0x3f00
    lui t2, 0x3f00
    lui t3, 0x3f00
    lwr t0, 49(a2)
    lwr t1, 53(a2)
    lwr t2, 57(a2)
    lwr t3, 61(a2)
    sw t0, 52(v0)
    sw t1, 56(v0)
    sw t2, 60(v0)
    sw t3, 64(v0)
    addiu v0, v0, 68
    j @c8
    nop
@25c:
    lw v0, 8(a0)
    lui t0, 0x3c00
    sw t0, 0(v0)
    lui t0, 0x3d00
    lui t1, 0x3d00
    lui t2, 0x3d00
    lwr t0, 1(a2)
    lwr t1, 5(a2)
    lwr t2, 9(a2)
    sw t0, 4(v0)
    sw t1, 8(v0)
    sw t2, 12(v0)
    lui t0, 0x3d00
    lui t1, 0x3d00
    lui t2, 0x3d00
    lwr t0, 17(a2)
    lwr t1, 21(a2)
    lwr t2, 25(a2)
    sw t0, 16(v0)
    sw t1, 20(v0)
    sw t2, 24(v0)
    lui t0, 0x3d00
    lui t1, 0x3d00
    lui t2, 0x3d00
    lwr t0, 33(a2)
    lwr t1, 37(a2)
    lwr t2, 41(a2)
    sw t0, 28(v0)
    sw t1, 32(v0)
    sw t2, 36(v0)
    lui t0, 0x3d00
    lui t1, 0x3d00
    lui t2, 0x3d00
    lwr t0, 49(a2)
    lwr t1, 53(a2)
    lwr t2, 57(a2)
    sw t0, 40(v0)
    sw t1, 44(v0)
    sw t2, 48(v0)
    addiu v0, v0, 52
    j @c8
    nop
}
#elif !defined(__mips__) /* the port (the SDK's GCC sees only the prototype) */
void gu_ctx_matrix(GuList *l, int type, const float *m)
{
    static const unsigned int idx[4] = { 0x3E000000, 0x3C000000, 0x3A000000, 0x40000000 };
    int i;
    int j;

    if (type < 0 || type > 3) {
        return;
    }
    ctx_push(l, idx[type]);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < (type == 0 ? 4 : 3); j++) {
            ctx_push(l, ctx_f24(m[i * 4 + j]) | (idx[type] + 0x01000000));
        }
    }
}
#endif

/* one sprite: two 16-bit vertices (u, v, x, y, z) behind a JUMP, size from the context (+0x38..),
   flip bit 0 swaps u, bit 1 swaps v; rot 1 swaps y, 2 swaps x and y, 3 swaps x */
#ifdef __MWERKS__
asm void gu_ctx_sprite(GuList *l, int x, int y, int z, int u, int v, int flip, int rot)
{
    lw t8, 8(a0)
    lw t7, 56(a0)
    move t9, a0
    lw t5, 60(a0)
    addiu a0, t8, 28
    ext t4, a0, 0x18, 0x4
    lw t6, 68(t9)
    sll v1, t4, 0x10
    lui v0, 0x1000
    lw t4, 64(t9)
    or v1, v1, v0
    sw a0, 8(t9)
    lui v0, 0x800
    ins a0, zero, 0x18, 0x8
    sw v1, 0(t8)
    or v1, a0, v0
    addiu t8, t8, 4
    sw v1, 0(t8)
    addu a0, t0, t7
    addiu t8, t8, 4
    addu t5, t1, t5
    addu v1, a1, t4
    addu v0, a2, t6
    andi t7, t2, 0x1
    sh t5, 12(t8)
    ext t2, t2, 0x1, 0x1
    sh v1, 14(t8)
    sh v0, 16(t8)
    sh a3, 18(t8)
    sh t0, 0(t8)
    sh t1, 2(t8)
    sh a1, 4(t8)
    sh a2, 6(t8)
    sh a3, 8(t8)
    beqz t7, @98
    sh a0, 10(t8)
    sh a0, 0(t8)
    sh t0, 10(t8)
@98:
    beqz t2, @b0
    addiu a2, zero, 1
    lh a1, 2(t8)
    lhu a0, 12(t8)
    sh a1, 12(t8)
    sh a0, 2(t8)
@b0:
    beq t3, a2, @16c
    slti a3, t3, 2
    bnezl a3, @d8
    lw a0, 8(t9)
    addiu t0, zero, 2
    beq t3, t0, @148
    addiu t1, zero, 3
    beql t3, t1, @138
    lh t4, 4(t8)
@d4:
    lw a0, 8(t9)
@d8:
    lui a2, 0x1280
    ori a1, a2, 0x102
    sw a1, 0(a0)
    addiu a1, a0, 4
    beqz t8, @11c
    sw a1, 8(t9)
    ext t5, t8, 0x18, 0x4
    sll t2, t5, 0x10
    ins t8, zero, 0x18, 0x8
    lui t3, 0x1000
    lui t1, 0x100
    or a3, t8, t1
    or t0, t2, t3
    addiu t8, a1, 8
    sw t0, 0(a1)
    sw a3, 4(a1)
    sw t8, 8(t9)
@11c:
    lw t7, 8(t9)
    lui t4, 0x406
    ori v1, t4, 0x2
    addiu t6, t7, 4
    sw v1, 0(t7)
    jr ra
    sw t6, 8(t9)
@138:
    lhu v1, 14(t8)
    sh t4, 14(t8)
    j @d4
    sh v1, 4(t8)
@148:
    lh v0, 4(t8)
    lh t5, 6(t8)
    lhu t7, 14(t8)
    lhu t6, 16(t8)
    sh v0, 14(t8)
    sh t7, 4(t8)
    sh t6, 6(t8)
    j @d4
    sh t5, 16(t8)
@16c:
    lh t3, 6(t8)
    lhu t2, 16(t8)
    sh t3, 16(t8)
    j @d4
    sh t2, 6(t8)
}
#elif !defined(__mips__) /* the port (the SDK's GCC sees only the prototype) */
void gu_ctx_sprite(GuList *l, int x, int y, int z, int u, int v, int flip, int rot)
{
    unsigned int *p = l->cur;
    short *s = (short *)(p + 2);
    short t;

    l->cur = p + 7;
    p[0] = ((((unsigned int)(p + 7)) & 0xFFFFFFF) >> 24) << 16 | 0x10000000;
    p[1] = ((unsigned int)(p + 7) & 0xFFFFFF) | 0x08000000;
    s[0] = u;
    s[1] = v;
    s[2] = x;
    s[3] = y;
    s[4] = z;
    s[5] = u + l->pad38[0];
    s[6] = v + l->pad38[1];
    s[7] = x + l->pad38[2];
    s[8] = y + l->pad38[3];
    s[9] = z;
    if (flip & 1) {
        t = s[0];
        s[0] = s[5];
        s[5] = t;
    }
    if (flip & 2) {
        t = s[1];
        s[1] = s[6];
        s[6] = t;
    }
    if (rot == 1 || rot == 2) {
        t = s[3];
        s[3] = s[8];
        s[8] = t;
    }
    if (rot == 2 || rot == 3) {
        t = s[2];
        s[2] = s[7];
        s[7] = t;
    }
    ctx_push(l, 0x12800102);
    ctx_addr(l, 0x01000000, (unsigned int)(p + 2));
    ctx_push(l, 0x04060002);
}
#endif

/* sceGuClear: CLEAR mode on, then sprites covering the display in the clear color/stencil/depth
   (+0x48..+0x50): one sprite, or with flag 0x10 a strip of 32 (psm 3) or 16 sprites; flags bits
   0-2 = color/stencil/depth */
#ifdef __MWERKS__
asm void gu_ctx_clear(GuList *l, int flags)
{
    addiu sp, sp, -16
    andi v1, a1, 0x10
    sw s0, 0(sp)
    move t2, a0
    andi s0, a1, 0x1
    ext a3, a1, 0x1, 0x1
    beqz v1, @22c
    ext t9, a1, 0x2, 0x1
    lw v1, 24(a0)
    lw a0, 76(a0)
    addiu a1, zero, 3
    lw t3, 80(t2)
    lw t0, 72(t2)
    beq v1, a1, @224
    sll v0, a0, 0x18
    addiu a2, zero, 1
    beq v1, a2, @224
    sll v0, a0, 0x1f
    addiu t1, zero, 2
    beq v1, t1, @224
    sll v0, a0, 0x1c
@54:
    addiu a0, zero, 3
    beq v1, a0, @190
    lui t8, 0x1000
    lw t1, 8(t2)
    lui v1, 0x800
    addiu t5, t1, 200
    ext v0, t5, 0x18, 0x4
    sll t6, v0, 0x10
    addiu t4, t1, 4
    or a1, t6, t8
    sw t5, 8(t2)
    ins t5, zero, 0x18, 0x8
    or a2, t5, v1
    sw a1, 0(t1)
    addiu t1, t4, 4
    move a1, t1
    sw a2, 0(t4)
    addiu t4, zero, 16
    move a2, zero
@a0:
    srl t8, a2, 0x1f
    addu v0, a2, t8
    sra t5, v0, 0x1
    sll a0, t5, 0x1
    subu t6, a2, a0
    sll t7, t6, 0x4
    addu t8, t7, t6
    addiu a2, a2, 1
    addu t7, t5, t6
    sll t6, t7, 0x6
    sll t5, t8, 0x4
    slti t7, a2, 16
    sh t6, 4(a1)
    sh t5, 6(a1)
    sw t0, 0(a1)
    sh t3, 8(a1)
    bnez t7, @a0
    addiu a1, a1, 12
    sll t7, t9, 0xa
@ec:
    sll a0, a3, 0x9
    sll v1, s0, 0x8
    lw s0, 8(t2)
    or t6, t7, a0
    lui t5, 0xd300
    or a2, t6, v1
    ori a1, t5, 0x1
    or t9, a2, a1
    lui t3, 0x1280
    addiu a2, s0, 4
    ori t0, t3, 0x11c
    addiu a3, a2, 4
    sw t9, 0(s0)
    sw a3, 8(t2)
    beqz t1, @158
    sw t0, 0(a2)
    ext s0, t1, 0x18, 0x4
    sll t0, s0, 0x10
    ins t1, zero, 0x18, 0x8
    lui t3, 0x1000
    lui a3, 0x100
    or v0, t1, a3
    or t8, t0, t3
    addiu t1, a2, 12
    sw t8, 4(a2)
    sw v0, 8(a2)
    sw t1, 8(t2)
@158:
    lui a2, 0x6
    lw a0, 8(t2)
    or t9, t4, a2
    lui t4, 0x400
    or v0, t9, t4
@16c:
    addiu t0, a0, 4
    lw s0, 0(sp)
    addiu t7, t0, 4
    sw t7, 8(t2)
    lui t2, 0xd300
    sw v0, 0(a0)
    sw t2, 0(t0)
    jr ra
    addiu sp, sp, 16
@190:
    lw t5, 8(t2)
    lui a1, 0x1000
    lui v1, 0x800
    addiu t7, t5, 392
    ext a2, t7, 0x18, 0x4
    sll t8, a2, 0x10
    addiu v0, t5, 4
    sw t7, 8(t2)
    ins t7, zero, 0x18, 0x8
    or t6, t8, a1
    or t4, t7, v1
    addiu t1, v0, 4
    sw t6, 0(t5)
    move a2, zero
    move a1, t1
    sw t4, 0(v0)
    addiu t4, zero, 32
@1d4:
    srl t7, a2, 0x1f
    addu v0, a2, t7
    sra t5, v0, 0x1
    sll a0, t5, 0x1
    subu t6, a2, a0
    sll t8, t6, 0x4
    addu t7, t8, t6
    addiu a2, a2, 1
    addu t8, t5, t6
    sll t6, t8, 0x5
    sll v0, t7, 0x4
    slti t5, a2, 32
    sh t6, 4(a1)
    sh v0, 6(a1)
    sw t0, 0(a1)
    sh t3, 8(a1)
    bnez t5, @1d4
    addiu a1, a1, 12
    j @ec
    sll t7, t9, 0xa
@224:
    j @54
    or t0, t0, v0
@22c:
    lw v1, 48(a0)
    lw t4, 40(a0)
    lw t7, 52(a0)
    lw t5, 44(a0)
    subu t1, v1, t4
    lw a0, 24(a0)
    lw v1, 76(t2)
    subu t6, t7, t5
    addiu a1, zero, 3
    addiu t8, t6, 1
    addiu t7, t1, 1
    lw t6, 80(t2)
    lw t3, 72(t2)
    beq a0, a1, @354
    sll v0, v1, 0x18
    addiu t0, zero, 1
    beq a0, t0, @354
    sll v0, v1, 0x1f
    addiu a2, zero, 2
    beq a0, a2, @354
    sll v0, v1, 0x1c
@280:
    lw t0, 8(t2)
    sll a0, t9, 0xa
    sll v0, a3, 0x9
    addiu t1, t0, 32
    ext t9, t1, 0x18, 0x4
    sll a2, t9, 0x10
    or a1, a0, v0
    lui t9, 0x1000
    move a0, t1
    or v0, a2, t9
    sll a3, s0, 0x8
    ins a0, zero, 0x18, 0x8
    lui v1, 0x800
    lui a2, 0xd300
    sw v0, 0(t0)
    or s0, a0, v1
    or a1, a1, a3
    addiu v0, t0, 4
    ori a2, a2, 0x1
    lui a3, 0x1280
    addiu t0, v0, 4
    sw s0, 0(v0)
    addu a0, t4, t7
    addu v1, t5, t8
    or s0, a1, a2
    ori t8, a3, 0x11c
    addiu t7, t1, 8
    sw t3, 12(t0)
    sw t3, 0(t0)
    sh a0, 16(t0)
    sh v1, 18(t0)
    sh t6, 20(t0)
    sw t7, 8(t2)
    sw s0, 0(t1)
    sw t8, 4(t1)
    sh t4, 4(t0)
    sh t5, 6(t0)
    beqz t0, @344
    sh t6, 8(t0)
    ext t8, t0, 0x18, 0x4
    sll t6, t8, 0x10
    ins t0, zero, 0x18, 0x8
    lui a1, 0x100
    or t4, t6, t9
    or t3, t0, a1
    addiu t5, t1, 16
    sw t4, 8(t1)
    sw t3, 12(t1)
    sw t5, 8(t2)
@344:
    lw a0, 8(t2)
    lui t1, 0x406
    j @16c
    ori v0, t1, 0x2
@354:
    j @280
    or t3, t3, v0
}
#elif !defined(__mips__) /* the port (the SDK's GCC sees only the prototype) */
void gu_ctx_clear(GuList *l, int flags)
{
    unsigned int color = l->pad38[4];
    unsigned int mode = ((flags & 7) >> 2) << 10 | ((flags & 3) >> 1) << 9 | (flags & 1) << 8 | 0xD3000001;
    unsigned int *verts;
    unsigned int prim;
    int n;
    int i;

    if (l->fbp_psm == 3) {
        color |= l->pad38[5] << 24;
    } else if (l->fbp_psm == 1) {
        color |= l->pad38[5] << 31;
    } else if (l->fbp_psm == 2) {
        color |= l->pad38[5] << 28;
    }
    if (!(flags & 0x10)) {
        unsigned int *p = l->cur;
        short *s;

        verts = p + 2;
        p[0] = ((((unsigned int)(p + 8)) & 0xFFFFFFF) >> 24) << 16 | 0x10000000;
        p[1] = ((unsigned int)(p + 8) & 0xFFFFFF) | 0x08000000;
        p[2] = color;
        s = (short *)(p + 3);
        s[0] = l->scissor[0];
        s[1] = l->scissor[1];
        s[2] = l->pad38[6];
        p[5] = color;
        s = (short *)(p + 6);
        s[0] = l->scissor[2] + 1;
        s[1] = l->scissor[3] + 1;
        s[2] = l->pad38[6];
        l->cur = p + 8;
        prim = 0x04060002;
    } else {
        int step = l->fbp_psm == 3 ? 32 : 64;
        unsigned int *p = l->cur;

        n = l->fbp_psm == 3 ? 32 : 16;
        verts = p + 2;
        p[0] = ((((unsigned int)(p + 2 + n * 3)) & 0xFFFFFFF) >> 24) << 16 | 0x10000000;
        p[1] = ((unsigned int)(p + 2 + n * 3) & 0xFFFFFF) | 0x08000000;
        for (i = 0; i < n; i++) {
            unsigned int *v = verts + i * 3;
            short *s = (short *)(v + 1);

            v[0] = color;
            s[0] = (i / 2 + i % 2) * step;
            s[1] = (i % 2) * 0x110;
            s[2] = l->pad38[6];
        }
        l->cur = p + 2 + n * 3;
        prim = n | 0x04060000;
    }
    ctx_push(l, mode);
    ctx_push(l, 0x1280011C);
    ctx_addr(l, 0x01000000, (unsigned int)verts);
    ctx_push(l, prim);
    ctx_push(l, 0xD3000000);
}
#endif

/* sceGuDepthRange: viewport Z scale/center with the depth offset (+0x5C), MINZ/MAXZ */
#ifdef __MWERKS__
asm void gu_ctx_depth_range(GuList *l, int near, int far)
{
    addu t1, a1, a2
    srl v0, t1, 0x1f
    addu t0, t1, v0
    sra a3, t0, 0x1
    mtc1 a3, $f1
    mtc1 a1, $f2
    move a3, a0
    sw a1, 84(a0)
    cvt.s.w $f4, $f2
    cvt.s.w $f2, $f1
    sw a2, 88(a0)
    slt t3, a1, a2
    lui a0, 0x4400
    sub.s $f0, $f2, $f4
    move t1, a1
    move t0, a2
    lwc1 $f1, 92(a3)
    mfc1 v1, $f0
    srl t4, v1, 0x8
    bnez t3, @5c
    or t2, t4, a0
    move t1, a2
    move t0, a1
@5c:
    cvt.s.w $f6, $f1
    lw t8, 8(a3)
    lui a1, 0xd600
    lui v1, 0xd700
    add.s $f5, $f2, $f6
    sw t2, 0(t8)
    lui a0, 0x4700
    addiu t2, t8, 4
    mfc1 t4, $f5
    or t5, t1, a1
    or t3, t0, v1
    srl t9, t4, 0x8
    or t6, t9, a0
    addiu t7, t2, 12
    sw t7, 8(a3)
    sw t6, 0(t2)
    sw t5, 4(t2)
    jr ra
    sw t3, 8(t2)
}
#elif !defined(__mips__) /* the port (the SDK's GCC sees only the prototype) */
void gu_ctx_depth_range(GuList *l, int near, int far)
{
    float z = (float)((near + far) / 2);
    int lo = near < far ? near : far;
    int hi = near < far ? far : near;

    l->pad38[7] = near;
    l->pad38[8] = far;
    ctx_push(l, ctx_f24(z - (float)near) | 0x44000000);
    ctx_push(l, ctx_f24(z + (float)l->pad38[9]) | 0x47000000);
    ctx_push(l, lo | 0xD6000000);
    ctx_push(l, hi | 0xD7000000);
}
#endif

/* sceGuDepthOffset: re-emits the depth range with the new offset */
#ifdef __MWERKS__
asm void gu_ctx_depth_offset(GuList *l, int offset)
{
    lw a3, 84(a0)
    lw t0, 88(a0)
    mtc1 a1, $f2
    mtc1 a3, $f8
    addu t5, a3, t0
    srl v0, t5, 0x1f
    addu t3, t5, v0
    sra t2, t3, 0x1
    mtc1 t2, $f7
    cvt.s.w $f6, $f8
    cvt.s.w $f4, $f2
    cvt.s.w $f3, $f7
    sw a1, 92(a0)
    move t1, a0
    lui t6, 0x4400
    sub.s $f0, $f3, $f6
    add.s $f5, $f3, $f4
    lui a1, 0x4700
    slt t5, a3, t0
    mfc1 v1, $f0
    mfc1 a0, $f5
    move t3, a3
    srl t4, v1, 0x8
    srl a2, a0, 0x8
    or v1, t4, t6
    move t2, t0
    bnez t5, @78
    or t4, a2, a1
    move t3, t0
    move t2, a3
@78:
    lw t9, 8(t1)
    lui a0, 0xd600
    lui a2, 0xd700
    addiu a3, t9, 4
    or t7, t3, a0
    or t0, t2, a2
    addiu t8, a3, 12
    sw v1, 0(t9)
    sw t8, 8(t1)
    sw t4, 0(a3)
    sw t7, 4(a3)
    jr ra
    sw t0, 8(a3)
}
#elif !defined(__mips__) /* the port (the SDK's GCC sees only the prototype) */
void gu_ctx_depth_offset(GuList *l, int offset)
{
    int near = l->pad38[7];
    int far = l->pad38[8];
    float z = (float)((near + far) / 2);
    int lo = near < far ? near : far;
    int hi = near < far ? far : near;

    l->pad38[9] = offset;
    ctx_push(l, ctx_f24(z - (float)near) | 0x44000000);
    ctx_push(l, ctx_f24(z + (float)offset) | 0x47000000);
    ctx_push(l, lo | 0xD6000000);
    ctx_push(l, hi | 0xD7000000);
}
#endif

/* PATCHPRIM: points (2) -> 1... GU_POINTS 0, lines 2, triangles 4 */
#ifndef __MWERKS__ /* PSP: libgu_ctx_gcc.c */
void gu_ctx_patch_prim(GuList *l, int prim)
{
    switch (prim) {
    case 4:
        ctx_push(l, 0x37000000);
        break;
    case 2:
        ctx_push(l, 0x37000001);
        break;
    case 0:
        ctx_push(l, 0x37000002);
        break;
    }
}
#else
void gu_ctx_patch_prim(GuList *l, int prim);
#endif

/* EMISSIVE, DIFFUSE, AMBIENT + ALPHA, SPECULAR */
#ifdef __MWERKS__
void gu_ctx_model_color(GuList *l, unsigned int emissive, unsigned int ambient, unsigned int diffuse,
                        unsigned int specular);
#else /* PSP: libgu_ctx_gcc.c */
void gu_ctx_model_color(GuList *l, unsigned int emissive, unsigned int ambient, unsigned int diffuse,
                        unsigned int specular)
{
    ctx_push(l, (emissive & 0xFFFFFF) | 0x54000000);
    ctx_push(l, (diffuse & 0xFFFFFF) | 0x56000000);
    ctx_push(l, (ambient & 0xFFFFFF) | 0x55000000);
    ctx_push(l, ambient >> 24 | 0x58000000);
    ctx_push(l, (specular & 0xFFFFFF) | 0x57000000);
}
#endif

/* material colors selected by mode bits 0 (ambient + alpha), 1 (diffuse), 2 (specular) */
#ifdef __MWERKS__
asm void gu_ctx_material(GuList *l, int mode, unsigned int color)
{
    move t5, a2
    ins t5, zero, 0x18, 0x8
    srl a3, a2, 0x18
    andi t4, a1, 0x1
    move t0, a0
    lui t7, 0x5600
    lui a0, 0x5500
    lui v1, 0x5700
    lui v0, 0x5800
    andi t3, a1, 0x4
    andi t2, a1, 0x2
    or t1, t5, a0
    or a2, t5, v1
    or a0, t5, t7
    beqz t4, @58
    or a1, a3, v0
    lw t8, 8(t0)
    sw t1, 0(t8)
    addiu t1, t8, 4
    addiu t4, t1, 4
    sw t4, 8(t0)
    sw a1, 0(t1)
@58:
    beqz t2, @70
    nop
    lw t9, 8(t0)
    addiu t2, t9, 4
    sw a0, 0(t9)
    sw t2, 8(t0)
@70:
    beqz t3, @88
    nop
    lw t3, 8(t0)
    sw a2, 0(t3)
    addiu a2, t3, 4
    sw a2, 8(t0)
@88:
    jr ra
    nop
}
#elif !defined(__mips__) /* the port (the SDK's GCC sees only the prototype) */
void gu_ctx_material(GuList *l, int mode, unsigned int color)
{
    unsigned int c = color & 0xFFFFFF;

    if (mode & 1) {
        ctx_push(l, c | 0x55000000);
        ctx_push(l, color >> 24 | 0x58000000);
    }
    if (mode & 2) {
        ctx_push(l, c | 0x56000000);
    }
    if (mode & 4) {
        ctx_push(l, c | 0x57000000);
    }
}
#endif

/* FFACE: 0 = clockwise -> 1 */
#ifndef __MWERKS__ /* PSP: libgu_ctx_gcc.c */
void gu_ctx_front_face(GuList *l, int order)
{
    if (order == 0) {
        ctx_push(l, 0x9B000001);
    } else {
        ctx_push(l, 0x9B000000);
    }
}
#else
void gu_ctx_front_face(GuList *l, int order);
#endif

/* LXP/LYP/LZP of the light position and LTYPE (components, type: 8 -> 2, 6 -> 1, else 0) */
#ifndef __MWERKS__ /* PSP: libgu_ctx_gcc.c */
void gu_ctx_light(GuList *l, int light, int type, int components, const float *pos)
{
    unsigned int w0, w1, w2, w3;
    int k;
    unsigned int t;

    t = 2;
    if (components != 8) {
        t = components == 6;
    }
    k = light * 3;
    w0 = (unsigned int)(k + 0x63) << 24 | ctx_f24(pos[0]);
    w1 = (unsigned int)(k + 0x64) << 24 | ctx_f24(pos[1]);
    w2 = (unsigned int)(k + 0x65) << 24 | ctx_f24(pos[2]);
    w3 = (unsigned int)(light + 0x5F) << 24 | type << 8 | t;
    ctx_push(l, w0);
    ctx_push(l, w1);
    ctx_push(l, w2);
    ctx_push(l, w3);
}
#else
void gu_ctx_light(GuList *l, int light, int type, int components, const float *pos);
#endif

#ifndef __MWERKS__ /* PSP: libgu_ctx_gcc.c (its jump table is the object's .rodata); the case order is the one that
   gives the original's registers (regrename) */
/* ALC/DLC/SLC of light n by component (1 ambient, 2 diffuse, 4 specular, 3 and 6 two at once) */
void gu_ctx_light_color(GuList *l, int light, int component, unsigned int color)
{
    switch (component) {
    case 1:
        ctx_push(l, (light * 3 + 0x8F) << 24 | (color & 0xFFFFFF));
        break;
    case 2:
        ctx_push(l, (light * 3 + 0x90) << 24 | (color & 0xFFFFFF));
        break;
    case 4:
        ctx_push(l, (light * 3 + 0x91) << 24 | (color & 0xFFFFFF));
        break;
    case 3:
        ctx_push(l, (light * 3 + 0x8F) << 24 | (color & 0xFFFFFF));
        ctx_push(l, (light * 3 + 0x90) << 24 | (color & 0xFFFFFF));
        break;
    case 6:
        ctx_push(l, (light * 3 + 0x90) << 24 | (color & 0xFFFFFF));
        ctx_push(l, (light * 3 + 0x91) << 24 | (color & 0xFFFFFF));
        break;
    }
}
#else
void gu_ctx_light_color(GuList *l, int light, int component, unsigned int color);
#endif

/* TMODE (maxmips, a2, swizzle), TPSM, TFLUSH */
#ifndef __MWERKS__ /* PSP: libgu_ctx_gcc.c */
void gu_ctx_tex_mode(GuList *l, int tpsm, int maxmips, int a2, int swizzle)
{
    ctx_push(l, maxmips << 16 | a2 << 8 | swizzle | 0xC2000000);
    ctx_push(l, tpsm | 0xC3000000);
    ctx_push(l, 0xCB000000);
}
#else
void gu_ctx_tex_mode(GuList *l, int tpsm, int maxmips, int a2, int swizzle);
#endif

/* sceGuScissor(x, y, w, h): kept in the context, emitted only while the scissor test is on */
#ifdef __MWERKS__
asm void gu_ctx_scissor(GuList *l, int x, int y, int w, int h)
{
    sll t4, a2, 0xa
    lui t5, 0xd400
    or t9, t4, a1
    addu v0, a2, t0
    addiu t1, v0, -1
    or t4, t9, t5
    addu t8, a1, a3
    lw t5, 36(a0)
    addiu t6, t8, -1
    sll t3, t1, 0xa
    move t2, a0
    or t7, t3, t6
    lui a0, 0x1600
    lui v1, 0xd500
    or t3, t7, a0
    sw a1, 40(t2)
    or a0, t7, v1
    sw a2, 44(t2)
    sw t6, 48(t2)
    beqz t5, @78
    sw t1, 52(t2)
    lw t0, 8(t2)
    lui a2, 0x1500
    addiu a1, t0, 4
    addiu a3, a1, 12
    sw t4, 0(t0)
    sw a3, 8(t2)
    sw a0, 0(a1)
    sw a2, 4(a1)
    sw t3, 8(a1)
@78:
    jr ra
    nop
}
#elif !defined(__mips__) /* the port (the SDK's GCC sees only the prototype) */
void gu_ctx_scissor(GuList *l, int x, int y, int w, int h)
{
    int x1 = x + w - 1;
    int y1 = y + h - 1;
    unsigned int b = y1 << 10 | x1;

    l->scissor[0] = x;
    l->scissor[1] = y;
    l->scissor[2] = x1;
    l->scissor[3] = y1;
    if (l->scissor_on) {
        ctx_push(l, y << 10 | x | 0xD4000000);
        ctx_push(l, b | 0xD5000000);
        ctx_push(l, 0x15000000);
        ctx_push(l, b | 0x16000000);
    }
}
#endif

/* SHADE: smooth (non-zero) -> 1 */
#ifndef __MWERKS__ /* PSP: libgu_ctx_gcc.c */
void gu_ctx_shade_model(GuList *l, int mode)
{
    if (mode == 0) {
        ctx_push(l, 0x50000000);
    } else {
        ctx_push(l, 0x50000001);
    }
}
#else
void gu_ctx_shade_model(GuList *l, int mode);
#endif

/* GE signal handler: records the signal id (ring of 16), calls the user callback, sets the event */
#ifndef __MWERKS__ /* PSP: libgu_ctx_gcc.c */
void gu_signal_handler(int id, void *arg)
{
    GuCallbacks *c = arg;
    int sid = id & 0xFFFF;
    int n = c->signal_count;

    c->signals[n % 16] = sid;
    c->signal_count = n + 1;
    if (c->signal != 0) {
        c->signal(sid);
    }
    sceKernelSetEventFlag(c->event_flag, 1);
}
#else
void gu_signal_handler(int id, void *arg);
#endif

/* GE finish handler: the user finish callback */
#ifndef __MWERKS__ /* PSP: libgu_ctx_gcc.c */
void gu_finish_handler(int id, void *arg)
{
    GuCallbacks *c = arg;

    if (c->finish != 0) {
        c->finish(id & 0xFFFF);
    }
}
#else
void gu_finish_handler(int id, void *arg);
#endif

#ifdef NON_MATCHING /* GCC: libgu globals through %hi/%lo */
int sceGeListUpdateStallAddr(int qid, void *stall);
void *sceGeEdramGetAddr(void);
void gu_set_display_hooks(void (*pre)(void), void (*post)(void), void (*disp)(int, int, int),
                          void (*draw)(int, unsigned int, int));

/* default settings and the five contexts (none open) */
void gu_init_contexts(void)
{
    int i;
    GuList *l;

    gu.psm = 1;
    gu.started = 0;
    gu.fbw = 480;
    gu.width = 480;
    gu.draw_fbp = 0;
    gu.disp_fbp = 0;
    gu.depth_fbp = 0;
    gu.depth_w = 0;
    gu.display_on = 0;
    gu.call_mode = 0;
    gu.list = 0;
    gu.states = 0;
    gu.height = 272;
    gu.cid = -1;
    l = gu.lists;
    for (i = 0; i < 5; i++, l++) {
        l->pad0 = 4;
        l->obj_depth = 0;
        l->pad10[0] = 100;
        l->pad10[1] = 0;
        l->fbp_psm = 1;
        l->disp_w = 480;
        l->disp_h = 272;
        l->scissor_on = 0;
        l->scissor[0] = 0;
        l->scissor[1] = 0;
        l->scissor[2] = 0;
        l->scissor[3] = 0;
        l->pad38[7] = 0;
        l->pad38[8] = 1;
        l->pad38[9] = 0;
        l->pad38[0] = 0;
        l->pad38[1] = 0;
        l->pad38[2] = 0;
        l->pad38[3] = 0;
        l->pad38[4] = 0;
        l->pad38[5] = 0;
        l->pad38[6] = 0xFFFF;
        l->pad38[10] = 0;
        l->pad38[12] = 0;
        l->pad38[11] = 0;
        l->pad38[13] = 0;
        l->pad38[14] = 0;
        gu.lists[i].next = -1;
    }
    gu_cb.signal = 0;
    gu_cb.finish = 0;
}

/* moves the GE stall address to the write position of the main list */
int gu_update_stall(void)
{
    if (gu.cid == 0 && gu.list->obj_depth == 0) {
        int r = sceGeListUpdateStallAddr(gu.ge_list, gu.list->cur);

        if (r < 0) {
            return r;
        }
    }
    return 0;
}

/* sceGuDebugPrint/Flush: up to 1024 characters per buffer, drawn with the 8x8 font at 0x08872FF0
   straight into VRAM at flush time, in the frame buffer format of the draw hook */
typedef struct GuDebugChar {
    int x;
    int y;
    unsigned int color;
    signed char ch;
} GuDebugChar;

extern int gu_debug_buf;               /* 0x088F1658 */
extern int gu_debug_init;              /* 0x088F165C */
extern GuDebugChar gu_debug_chars[2][1024]; /* 0x088F1660 */
extern int gu_debug_count[2];          /* 0x088F9660 */
extern unsigned int gu_debug_edram;    /* 0x088F9668 */
extern int gu_debug_psm;               /* 0x088F966C */
extern unsigned int gu_debug_fbp;      /* 0x088F9670 */
extern int gu_debug_fbw;               /* 0x088F9674 */
extern int gu_debug_w;                 /* 0x088F9678 */
extern int gu_debug_h;                 /* 0x088F967C */
extern const unsigned char gu_debug_font[]; /* 0x08872FF0, 8 bytes per glyph from ' ' */

void sceGuDebugFlush(void);
void gu_debug_swap(void);
void gu_debug_set_draw(int psm, unsigned int fbp, int fbw);
void gu_debug_set_disp(int psm, int w, int h);

static void gu_debug_start(void)
{
    if (!gu_debug_init) {
        gu_debug_edram = (unsigned int)sceGeEdramGetAddr();
        gu_debug_buf = 0;
        gu_set_display_hooks(sceGuDebugFlush, gu_debug_swap, (void (*)(int, int, int))gu_debug_set_disp,
                             gu_debug_set_draw);
        gu_debug_init = 1;
    }
}

void sceGuDebugPrint(int x, int y, unsigned int color, const char *str)
{
    int n;
    GuDebugChar *c;
    int cx;
    int cy;
    int r, g, b, a;
    int ch;

    gu_debug_start();
    n = gu_debug_count[gu_debug_buf];
    c = gu_debug_chars[gu_debug_buf];
    if (n >= 1024) {
        return;
    }
    c += n;
    cx = x;
    cy = y;
    r = color & 0xFF;
    g = color << 16 >> 24;
    b = (color >> 16) & 0xFF;
    a = color >> 24;
    /* still 53 words off in GCC: the original shares the tail `| (g >> k) << 5 | r >> 3` of PSM 0 and 1 (cross-jumping
       with equal registers) */
    if (gu_debug_psm == 0) {
        color = (b >> 3) << 11 | (g >> 2) << 5 | r >> 3;
    } else if (gu_debug_psm == 1) {
        color = (a >> 7) << 15 | (b >> 3) << 10 | (g >> 3) << 5 | r >> 3;
    } else if (gu_debug_psm == 2) {
        color = (a >> 4) << 12 | (b >> 4) << 8 | (g & ~0xF) | r >> 4;
    }
    while (*str != 0) {
        ch = *str;
        if (ch == '\n') {
            cx = x;
            cy += 8;
            str++;
        } else {
            n++;
            c->x = cx;
            str++;
            cx += 8;
            c->ch = ch - 0x20;
            c->y = cy;
            c->color = color;
            c++;
            if (n >= 1024) {
                break;
            }
        }
    }
    gu_debug_count[gu_debug_buf] = n;
}

void sceGuDebugFlush(void)
{
    int n;
    GuDebugChar *c;

    gu_debug_start();
    n = gu_debug_count[gu_debug_buf];
    c = gu_debug_chars[gu_debug_buf];
    for (; n != 0; n--, c++) {
        int x = c->x;
        int y = c->y;
        const unsigned int *glyph;
        unsigned int color;
        unsigned int bits;
        unsigned int mask;
        int row;

        if (y + 7 >= gu_debug_h || y < 0 || x + 7 >= gu_debug_w || x < 0) {
            continue;
        }
        /* the 8 rows of a glyph are two little-endian words, one bit per pixel */
        glyph = (const unsigned int *)&gu_debug_font[c->ch * 8];
        color = c->color;
        for (row = 0; row < 8; row++) {
            int pos = x + (y + row) * gu_debug_fbw;
            unsigned int p16 = gu_debug_edram + pos * 2 + gu_debug_fbp;
            unsigned int p32 = gu_debug_edram + pos * 4 + gu_debug_fbp;
            int col;

            if (row == 0) {
                bits = glyph[0];
                mask = 1;
            } else if (row == 4) {
                bits = glyph[1];
                mask = 1;
            }
            for (col = 0; col < 8; col++) {
                if (gu_debug_psm == 3) {
                    if (bits & mask) {
                        unsigned short *q = (unsigned short *)PSP_UNCACHED_PHYS(p32);

                        q[1] = (int)color >> 16;
                        q[0] = color;
                    }
                } else if (bits & mask) {
                    *(unsigned short *)PSP_UNCACHED_PHYS(p16) = color;
                }
                mask <<= 1;
                p16 += 2;
                p32 += 4;
            }
        }
    }
    gu_debug_count[gu_debug_buf] = 0;
}

void gu_debug_swap(void)
{
    gu_debug_buf = !gu_debug_buf;
}

void gu_debug_set_disp(int psm, int w, int h)
{
    gu_debug_psm = psm;
    gu_debug_w = w;
    gu_debug_h = h;
}

void gu_debug_set_draw(int psm, unsigned int fbp, int fbw)
{
    gu_debug_fbp = fbp;
    gu_debug_fbw = fbw;
}

/* starts the overlay without printing */
void gu_debug_open(void)
{
    gu_debug_edram = (unsigned int)sceGeEdramGetAddr();
    gu_debug_buf = 0;
    gu_set_display_hooks(sceGuDebugFlush, gu_debug_swap, (void (*)(int, int, int))gu_debug_set_disp,
                         gu_debug_set_draw);
    gu_debug_init = 1;
}
#endif

int sceKernelRegisterSubIntrHandler(int intno, int no, void *handler, void *arg);
int sceKernelReleaseSubIntrHandler(int intno, int no);
int sceKernelEnableSubIntr(int intno, int no);

/* installs handler as VBLANK sub-interrupt handler number no (removes it when handler is NULL) */
#ifndef __MWERKS__ /* PSP: libgu_ctx_gcc.c */
int vblank_handler_set(int no, void *handler, void *arg)
{
    int ret;

    if (handler != 0) {
        ret = sceKernelRegisterSubIntrHandler(30, no, handler, arg);
        if (ret >= 0) {
            ret = sceKernelEnableSubIntr(30, no);
        }
    } else {
        ret = sceKernelReleaseSubIntrHandler(30, no);
    }
    return ret;
}
#else
int vblank_handler_set(int no, void *handler, void *arg);
#endif
