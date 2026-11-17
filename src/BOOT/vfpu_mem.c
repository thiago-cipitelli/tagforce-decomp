// @range 0x0881EB0C-0x0881F6E4
// @gcc 0x0881F2DC-0x0881F63C
/* VFPU block memory routines (copy/move/fill through quad stores to the uncached mirror, 64-byte
   cache lines created without a fetch) and the VFPU register context: the whole register file, the
   prefix/condition control registers and the random generator state saved, restored and reset.
   Hand-written code kept as whole `asm` functions. Outside the PSP build the copies are the libc
   ones, the register file is emulated by a C VfpuContext and the generator state is vfpu_rcx
   (src/BOOT/vfpu_color.c). */

typedef struct VfpuContext {
    unsigned int regs[128];            /* S000..S733, in sv.q order (C000, C010, ...) */
    unsigned int ctrl[5];              /* +0x200 $128-$132: PFXS, PFXT, PFXD, CC, INF4 */
    unsigned int pad[3];
    unsigned int rcx[8];               /* +0x220 $136-$143: random generator state */
} VfpuContext;

void *vfpu_memcpy(void *dst, const void *src, unsigned int n);

#ifndef __MWERKS__
#include <string.h>

extern unsigned int vfpu_rcx[8];
static VfpuContext vfpu_cur;           /* emulated register file */
#endif

/* memcpy; returns dst */
#ifdef __MWERKS__
asm void *vfpu_memcpy(void *dst, const void *src, unsigned int n)
{
    beqz a2, L_0881EC0C
    move a3, a0
    andi v1, a0, 0x3f
    beqz v1, L_0881EB44
    nop
    lbu v1, 0(a1)
L_0881EB24:
    addiu a2, a2, -1
    addiu a1, a1, 1
    sb v1, 0(a3)
    addiu a3, a3, 1
    beqz a2, L_0881EC0C
    andi v0, a3, 0x3f
    bnezl v0, L_0881EB24
    lbu v1, 0(a1)
L_0881EB44:
    beqz a2, L_0881EC0C
    nop
    sltiu t0, a2, 64
    bnez t0, L_0881EBF4
    lui t1, 0x4000
    andi t0, a1, 0xf
    addiu v0, zero, 4
    move t2, t1
    ins t2, a3, 0x0, 0x1d
    beq t0, v0, L_0881EDC0
    sltiu t3, t0, 5
    beqz t3, L_0881ECD8
    addiu t4, zero, 8
    beqz t0, L_0881EC14
    nop
L_0881EB80:
    cache 0x1b, 0(a3)
    addiu a3, a3, 64
    addiu a2, a2, -64
    sync
L_0881EB90:
    lwr t0, 0(a1)
    lwl t0, 3(a1)
    mtv t0, S000
    lwr t0, 4(a1)
    lwl t0, 7(a1)
    mtv t0, S001
    lwr t0, 8(a1)
    lwl t0, 11(a1)
    mtv t0, S002
    lwr t0, 12(a1)
    lwl t0, 15(a1)
    mtv t0, S003
    addiu a1, a1, 16
    sv.q C000, 0(t2), wb
    addiu t2, t2, 16
    andi t0, t2, 0x3f
    bnez t0, L_0881EB90
    vnop
    srl t0, a2, 0x6
    bnez t0, L_0881EB80
    nop
    vflush
    vnop
L_0881EBEC:
    beqz a2, L_0881EC0C
    nop
L_0881EBF4:
    lbu t2, 0(a1)
    addiu a2, a2, -1
    addiu a1, a1, 1
    sb t2, 0(a3)
    bnez a2, L_0881EBF4
    addiu a3, a3, 1
L_0881EC0C:
    jr ra
    move v0, a0
L_0881EC14:
    srl t0, a2, 0x7
    beqz t0, L_0881EC94
    nop
L_0881EC20:
    cache 0x1b, 0(a3)
    lv.q C000, 0(a1)
    lv.q C010, 16(a1)
    lv.q C020, 32(a1)
    lv.q C030, 48(a1)
    lv.q C100, 64(a1)
    lv.q C110, 80(a1)
    lv.q C120, 96(a1)
    lv.q C130, 112(a1)
    cache 0x1b, 64(a3)
    addiu a3, a3, 128
    addiu a1, a1, 128
    addiu a2, a2, -128
    srl t0, a2, 0x7
    sync
    sv.q C000, 0(t2), wb
    sv.q C010, 16(t2), wb
    sv.q C020, 32(t2), wb
    sv.q C030, 48(t2), wb
    sv.q C100, 64(t2), wb
    sv.q C110, 80(t2), wb
    sv.q C120, 96(t2), wb
    sv.q C130, 112(t2), wb
    addiu t2, t2, 128
    bnez t0, L_0881EC20
    vnop
    srl t0, a2, 0x6
    beqz t0, L_0881ECC8
    nop
L_0881EC94:
    cache 0x1b, 0(a3)
    lv.q C000, 0(a1)
    lv.q C010, 16(a1)
    lv.q C020, 32(a1)
    lv.q C030, 48(a1)
    addiu a3, a3, 64
    addiu a1, a1, 64
    addiu a2, a2, -64
    sync
    sv.q C000, 0(t2), wb
    sv.q C010, 16(t2), wb
    sv.q C020, 32(t2), wb
    sv.q C030, 48(t2), wb
L_0881ECC8:
    vflush
    vnop
    j L_0881EBEC
    nop
L_0881ECD8:
    beq t0, t4, L_0881ED4C
    addiu t5, zero, 12
    bne t0, t5, L_0881EB80
    nop
L_0881ECE8:
    cache 0x1b, 0(a3)
    lv.s S000, 0(a1)
    lv.q C100, 4(a1)
    lv.q C110, 20(a1)
    lv.q C120, 36(a1)
    lv.q C130, 52(a1)
    vmmov.t E001, E100
    vmov.t R010, R103
    vmov.t C031, C130
    addiu a3, a3, 64
    addiu a1, a1, 64
    addiu a2, a2, -64
    srl t0, a2, 0x6
    sync
    sv.q C000, 0(t2), wb
    sv.q C010, 16(t2), wb
    sv.q C020, 32(t2), wb
    sv.q C030, 48(t2), wb
    addiu t2, t2, 64
    bnez t0, L_0881ECE8
    vnop
    vflush
    vnop
    j L_0881EBEC
    nop
L_0881ED4C:
    cache 0x1b, 0(a3)
    lv.s S000, 0(a1)
    lv.s S001, 4(a1)
    lv.q C100, 8(a1)
    lv.q C110, 24(a1)
    lv.q C120, 40(a1)
    lv.q C130, 56(a1)
    vmmov.p E002, E100
    vmov.p C010, C102
    vmov.p C020, C112
    vmov.p C022, C120
    vmov.p C030, C122
    vmov.p C032, C130
    addiu a3, a3, 64
    addiu a1, a1, 64
    addiu a2, a2, -64
    srl t0, a2, 0x6
    sync
    sv.q C000, 0(t2), wb
    sv.q C010, 16(t2), wb
    sv.q C020, 32(t2), wb
    sv.q C030, 48(t2), wb
    addiu t2, t2, 64
    bnez t0, L_0881ED4C
    vnop
    vflush
    vnop
    j L_0881EBEC
    nop
L_0881EDC0:
    addiu a2, a2, -64
    cache 0x1b, 0(a3)
    lv.q C100, -4(a1)
    lv.q C110, 12(a1)
    lv.q C120, 28(a1)
    lv.q C130, 44(a1)
    lv.s S033, 60(a1)
    vmmov.t E000, E101
    vmov.t R003, R110
    vmov.t C030, C131
    addiu a3, a3, 64
    addiu a1, a1, 64
    srl t0, a2, 0x6
    sync
    sv.q C000, 0(t2), wb
    sv.q C010, 16(t2), wb
    sv.q C020, 32(t2), wb
    sv.q C030, 48(t2), wb
    addiu t2, t2, 64
    bnez t0, L_0881EDC0
    vnop
    vflush
    vnop
    j L_0881EBEC
    nop
}
#elif !defined(__mips__)
void *vfpu_memcpy(void *dst, const void *src, unsigned int n)
{
    return memcpy(dst, src, n);
}
#else
void *vfpu_memcpy(void *dst, const void *src, unsigned int n)
{
    /* the SDK's GCC: C around one asm per alignment of src; 36 words left, all register names: the original
       keeps 0x40000000 in t1 and the uncached pointer (and the tail byte) in t2, regrename gives the reverse here
       (every order of the cases tested) */
    unsigned char *d = dst;
    const unsigned char *s = src;
    unsigned int u;

    while (n != 0 && ((unsigned int)d & 0x3F) != 0) {
        *d++ = *s++;
        n--;
    }
    if (n == 0)
        return dst;
    if (n >= 64) {
        u = __builtin_allegrex_ins(0x40000000, (unsigned int)d, 0, 29);
        switch ((unsigned int)s & 0xF) {
        case 0:
            __asm__ volatile (
                ".set push\n"
                ".set noreorder\n"
                "1:\n"
                "srl $8, %2, 0x7\n"
                "beqz $8, 3f\n"
                "nop\n"
                "2:\n"
                "cache 0x1b, 0(%0)\n"
                "lv.q C000, 0(%1)\n"
                "lv.q C010, 16(%1)\n"
                "lv.q C020, 32(%1)\n"
                "lv.q C030, 48(%1)\n"
                "lv.q C100, 64(%1)\n"
                "lv.q C110, 80(%1)\n"
                "lv.q C120, 96(%1)\n"
                "lv.q C130, 112(%1)\n"
                "cache 0x1b, 64(%0)\n"
                "addiu %0, %0, 128\n"
                "addiu %1, %1, 128\n"
                "addiu %2, %2, -128\n"
                "srl $8, %2, 0x7\n"
                "sync\n"
                "sv.q C000, 0(%3), wb\n"
                "sv.q C010, 16(%3), wb\n"
                "sv.q C020, 32(%3), wb\n"
                "sv.q C030, 48(%3), wb\n"
                "sv.q C100, 64(%3), wb\n"
                "sv.q C110, 80(%3), wb\n"
                "sv.q C120, 96(%3), wb\n"
                "sv.q C130, 112(%3), wb\n"
                "addiu %3, %3, 128\n"
                "bnez $8, 2b\n"
                "vnop\n"
                "srl $8, %2, 0x6\n"
                "beqz $8, 4f\n"
                "nop\n"
                "3:\n"
                "cache 0x1b, 0(%0)\n"
                "lv.q C000, 0(%1)\n"
                "lv.q C010, 16(%1)\n"
                "lv.q C020, 32(%1)\n"
                "lv.q C030, 48(%1)\n"
                "addiu %0, %0, 64\n"
                "addiu %1, %1, 64\n"
                "addiu %2, %2, -64\n"
                "sync\n"
                "sv.q C000, 0(%3), wb\n"
                "sv.q C010, 16(%3), wb\n"
                "sv.q C020, 32(%3), wb\n"
                "sv.q C030, 48(%3), wb\n"
                "4:\n"
                "vflush\n"
                "vnop\n"
                ".set pop\n"
                : "+r" (d), "+r" (s), "+r" (n), "+r" (u)
                :
                : "$8", "memory");
            break;
        case 12:
            __asm__ volatile (
                ".set push\n"
                ".set noreorder\n"
                "1:\n"
                "cache 0x1b, 0(%0)\n"
                "lv.s S000, 0(%1)\n"
                "lv.q C100, 4(%1)\n"
                "lv.q C110, 20(%1)\n"
                "lv.q C120, 36(%1)\n"
                "lv.q C130, 52(%1)\n"
                "vmmov.t E001, E100\n"
                "vmov.t R010, R103\n"
                "vmov.t C031, C130\n"
                "addiu %0, %0, 64\n"
                "addiu %1, %1, 64\n"
                "addiu %2, %2, -64\n"
                "srl $8, %2, 0x6\n"
                "sync\n"
                "sv.q C000, 0(%3), wb\n"
                "sv.q C010, 16(%3), wb\n"
                "sv.q C020, 32(%3), wb\n"
                "sv.q C030, 48(%3), wb\n"
                "addiu %3, %3, 64\n"
                "bnez $8, 1b\n"
                "vnop\n"
                "vflush\n"
                "vnop\n"
                ".set pop\n"
                : "+r" (d), "+r" (s), "+r" (n), "+r" (u)
                :
                : "$8", "memory");
            break;
        case 8:
            __asm__ volatile (
                ".set push\n"
                ".set noreorder\n"
                "1:\n"
                "cache 0x1b, 0(%0)\n"
                "lv.s S000, 0(%1)\n"
                "lv.s S001, 4(%1)\n"
                "lv.q C100, 8(%1)\n"
                "lv.q C110, 24(%1)\n"
                "lv.q C120, 40(%1)\n"
                "lv.q C130, 56(%1)\n"
                "vmmov.p E002, E100\n"
                "vmov.p C010, C102\n"
                "vmov.p C020, C112\n"
                "vmov.p C022, C120\n"
                "vmov.p C030, C122\n"
                "vmov.p C032, C130\n"
                "addiu %0, %0, 64\n"
                "addiu %1, %1, 64\n"
                "addiu %2, %2, -64\n"
                "srl $8, %2, 0x6\n"
                "sync\n"
                "sv.q C000, 0(%3), wb\n"
                "sv.q C010, 16(%3), wb\n"
                "sv.q C020, 32(%3), wb\n"
                "sv.q C030, 48(%3), wb\n"
                "addiu %3, %3, 64\n"
                "bnez $8, 1b\n"
                "vnop\n"
                "vflush\n"
                "vnop\n"
                ".set pop\n"
                : "+r" (d), "+r" (s), "+r" (n), "+r" (u)
                :
                : "$8", "memory");
            break;
        case 4:
            __asm__ volatile (
                ".set push\n"
                ".set noreorder\n"
                "1:\n"
                "addiu %2, %2, -64\n"
                "cache 0x1b, 0(%0)\n"
                "lv.q C100, -4(%1)\n"
                "lv.q C110, 12(%1)\n"
                "lv.q C120, 28(%1)\n"
                "lv.q C130, 44(%1)\n"
                "lv.s S033, 60(%1)\n"
                "vmmov.t E000, E101\n"
                "vmov.t R003, R110\n"
                "vmov.t C030, C131\n"
                "addiu %0, %0, 64\n"
                "addiu %1, %1, 64\n"
                "srl $8, %2, 0x6\n"
                "sync\n"
                "sv.q C000, 0(%3), wb\n"
                "sv.q C010, 16(%3), wb\n"
                "sv.q C020, 32(%3), wb\n"
                "sv.q C030, 48(%3), wb\n"
                "addiu %3, %3, 64\n"
                "bnez $8, 1b\n"
                "vnop\n"
                "vflush\n"
                "vnop\n"
                ".set pop\n"
                : "+r" (d), "+r" (s), "+r" (n), "+r" (u)
                :
                : "$8", "memory");
            break;
        default:
            __asm__ volatile (
                ".set push\n"
                ".set noreorder\n"
                "1:\n"
                "cache 0x1b, 0(%0)\n"
                "addiu %0, %0, 64\n"
                "addiu %2, %2, -64\n"
                "sync\n"
                "2:\n"
                "lwr $8,0(%1)\n"
                "lwl $8,3(%1)\n"
                "mtv $8, S000\n"
                "lwr $8,4(%1)\n"
                "lwl $8,7(%1)\n"
                "mtv $8, S001\n"
                "lwr $8,8(%1)\n"
                "lwl $8,11(%1)\n"
                "mtv $8, S002\n"
                "lwr $8,12(%1)\n"
                "lwl $8,15(%1)\n"
                "mtv $8, S003\n"
                "addiu %1, %1, 16\n"
                "sv.q C000, 0(%3), wb\n"
                "addiu %3, %3, 16\n"
                "andi $8, %3, 0x3f\n"
                "bnez $8, 2b\n"
                "vnop\n"
                "srl $8, %2, 0x6\n"
                "bnez $8, 1b\n"
                "nop\n"
                "vflush\n"
                "vnop\n"
                ".set pop\n"
                : "+r" (d), "+r" (s), "+r" (n), "+r" (u)
                :
                : "$8", "memory");
            break;
        }
    }
    while (n != 0) {
        *d++ = *s++;
        n--;
    }
    return dst;
}
#endif

/* memmove (overlapping copies backwards, the rest through vfpu_memcpy); returns dst */
#ifdef __MWERKS__
asm void *vfpu_memmove(void *dst, const void *src, unsigned int n)
{
    addiu sp, sp, -16
    sltu v1, a0, a1
    sw ra, 0(sp)
    move t3, a0
    move a3, a1
    bnez v1, L_0881F13C
    move t2, a2
    subu v1, a0, a1
    slt t0, v1, a2
    beqz t0, L_0881F13C
    nop
    slti a0, v1, 64
    bnezl a0, L_0881F114
    addiu a2, a2, -1
    addu a1, a1, a2
    beqz a2, L_0881EEF4
    addu a0, t3, a2
    andi v1, a0, 0x3f
    beqz v1, L_0881EE98
    nop
    addiu a1, a1, -1
L_0881EE78:
    lbu a2, 0(a1)
    addiu a0, a0, -1
    addiu t2, t2, -1
    andi v1, a0, 0x3f
    beqz t2, L_0881EEF4
    sb a2, 0(a0)
    bnezl v1, L_0881EE78
    addiu a1, a1, -1
L_0881EE98:
    beqz t2, L_0881EEF8
    move v0, t3
    sltiu a3, t2, 64
    bnez a3, L_0881EED4
    lui t1, 0x4000
    andi a3, a1, 0xf
    addiu v0, zero, 4
    move a2, t1
    ins a2, a0, 0x0, 0x1d
    beq a3, v0, L_0881F0B0
    sltiu t4, a3, 5
    beqz t4, L_0881EFC8
    addiu t5, zero, 8
    beqz a3, L_0881EF04
    nop
L_0881EED4:
    beqz t2, L_0881EEF8
    move v0, t3
L_0881EEDC:
    addiu a1, a1, -1
    lbu t7, 0(a1)
    addiu a0, a0, -1
    addiu t2, t2, -1
    bnez t2, L_0881EEDC
    sb t7, 0(a0)
L_0881EEF4:
    move v0, t3
L_0881EEF8:
    lw ra, 0(sp)
L_0881EEFC:
    jr ra
    addiu sp, sp, 16
L_0881EF04:
    srl t0, t2, 0x7
    beqz t0, L_0881EF84
    nop
L_0881EF10:
    lv.q C000, -128(a1)
    lv.q C010, -112(a1)
    lv.q C020, -96(a1)
    lv.q C030, -80(a1)
    lv.q C100, -64(a1)
    lv.q C110, -48(a1)
    lv.q C120, -32(a1)
    lv.q C130, -16(a1)
    cache 0x1b, -128(a0)
    addiu a0, a0, -128
    addiu a1, a1, -128
    addiu t2, t2, -128
    srl t0, t2, 0x7
    cache 0x1b, -64(a0)
    sync
    sv.q C000, -128(a2), wb
    sv.q C010, -112(a2), wb
    sv.q C020, -96(a2), wb
    sv.q C030, -80(a2), wb
    sv.q C100, -64(a2), wb
    sv.q C110, -48(a2), wb
    sv.q C120, -32(a2), wb
    sv.q C130, -16(a2), wb
    addiu a2, a2, -128
    bnez t0, L_0881EF10
    vnop
    srl t0, t2, 0x6
    beqz t0, L_0881EFB8
    nop
L_0881EF84:
    lv.q C000, -64(a1)
    lv.q C010, -48(a1)
    lv.q C020, -32(a1)
    lv.q C030, -16(a1)
    cache 0x1b, -64(a0)
    addiu a0, a0, -64
    addiu a1, a1, -64
    addiu t2, t2, -64
    sync
    sv.q C000, -64(a2), wb
    sv.q C010, -48(a2), wb
    sv.q C020, -32(a2), wb
    sv.q C030, -16(a2), wb
L_0881EFB8:
    vflush
    vnop
    j L_0881EED4
    nop
L_0881EFC8:
    beq a3, t5, L_0881F03C
    addiu t6, zero, 12
    bne a3, t6, L_0881EED4
    nop
L_0881EFD8:
    lv.s S000, -64(a1)
    lv.q C100, -60(a1)
    lv.q C110, -44(a1)
    lv.q C120, -28(a1)
    lv.q C130, -12(a1)
    cache 0x1b, -64(a0)
    vmmov.t E001, E100
    vmov.t R010, R103
    vmov.t C031, C130
    addiu a0, a0, -64
    addiu a1, a1, -64
    addiu t2, t2, -64
    srl t0, t2, 0x6
    sync
    sv.q C000, -64(a2), wb
    sv.q C010, -48(a2), wb
    sv.q C020, -32(a2), wb
    sv.q C030, -16(a2), wb
    addiu a2, a2, -64
    bnez t0, L_0881EFD8
    vnop
    vflush
    vnop
    j L_0881EED4
    nop
L_0881F03C:
    lv.s S000, -64(a1)
    lv.s S001, -60(a1)
    lv.q C100, -56(a1)
    lv.q C110, -40(a1)
    lv.q C120, -24(a1)
    lv.q C130, -8(a1)
    cache 0x1b, -64(a0)
    vmmov.p E002, E100
    vmov.p C010, C102
    vmov.p C020, C112
    vmov.p C022, C120
    vmov.p C030, C122
    vmov.p C032, C130
    addiu a0, a0, -64
    addiu a1, a1, -64
    addiu t2, t2, -64
    srl t0, t2, 0x6
    sync
    sv.q C000, -64(a2), wb
    sv.q C010, -48(a2), wb
    sv.q C020, -32(a2), wb
    sv.q C030, -16(a2), wb
    addiu a2, a2, -64
    bnez t0, L_0881F03C
    vnop
    vflush
    vnop
    j L_0881EED4
    nop
L_0881F0B0:
    lv.q C100, -68(a1)
    lv.q C110, -52(a1)
    lv.q C120, -36(a1)
    lv.q C130, -20(a1)
    lv.s S033, -4(a1)
    cache 0x1b, -64(a0)
    addiu t2, t2, -64
    vmmov.t E000, E101
    vmov.t R003, R110
    vmov.t C030, C131
    addiu a0, a0, -64
    addiu a1, a1, -64
    srl t0, t2, 0x6
    sync
    sv.q C000, -64(a2), wb
    sv.q C010, -48(a2), wb
    sv.q C020, -32(a2), wb
    sv.q C030, -16(a2), wb
    addiu a2, a2, -64
    bnez t0, L_0881F0B0
    vnop
    vflush
    vnop
    j L_0881EED4
    nop
L_0881F114:
    bltz a2, L_0881EEF8
    move v0, t3
L_0881F11C:
    addu t8, a3, a2
    lbu t2, 0(t8)
    addu a1, t3, a2
    addiu a2, a2, -1
    bgez a2, L_0881F11C
    sb t2, 0(a1)
    j L_0881EEF8
    move v0, t3
L_0881F13C:
    jal vfpu_memcpy
    nop
    j L_0881EEFC
    lw ra, 0(sp)
}
#else
void *vfpu_memmove(void *dst, const void *src, unsigned int n)
{
    return memmove(dst, src, n);
}
#endif

/* Resets the VFPU to the vfpu_ctx_init state; returns 0. */
#ifdef __MWERKS__
asm int vfpu_reset(void)
{
    addiu sp, sp, -576
    lui v0, 0x7f80
    ori a0, v0, 0x1
    move t1, sp
    move v0, sp
    addiu v1, zero, 31
L_0881F164:
    addiu v1, v1, -1
    sw a0, 0(v0)
    sw a0, 4(v0)
    sw a0, 8(v0)
    sw a0, 12(v0)
    bgez v1, L_0881F164
    addiu v0, v0, 16
    lui a1, 0x3f80
    lui a2, 0x3f80
    lui a3, 0x3f80
    lui t8, 0x3f80
    addiu t2, t1, 544
    lui t3, 0x3f80
    addiu t6, zero, 228
    ori t7, a1, 0x1
    ori t5, a2, 0x2
    ori t4, a3, 0x4
    ori a0, t8, 0x8
    addiu t0, zero, 63
    sw t7, 544(t1)
    sw t6, 516(t1)
    sw t0, 524(t1)
    sw t6, 512(t1)
    sw zero, 520(t1)
    sw zero, 528(t1)
    sw t3, 28(t2)
    sw t5, 4(t2)
    sw t4, 8(t2)
    sw a0, 12(t2)
    sw t3, 16(t2)
    sw t3, 20(t2)
    sw t3, 24(t2)
    lv.q C000, 0(sp)
    lv.q C010, 16(sp)
    lv.q C020, 32(sp)
    lv.q C030, 48(sp)
    lv.q C100, 64(sp)
    lv.q C110, 80(sp)
    lv.q C120, 96(sp)
    lv.q C130, 112(sp)
    lv.q C200, 128(sp)
    lv.q C210, 144(sp)
    lv.q C220, 160(sp)
    lv.q C230, 176(sp)
    lv.q C300, 192(sp)
    lv.q C310, 208(sp)
    lv.q C320, 224(sp)
    lv.q C330, 240(sp)
    lv.q C400, 256(sp)
    lv.q C410, 272(sp)
    lv.q C420, 288(sp)
    lv.q C430, 304(sp)
    lv.q C500, 320(sp)
    lv.q C510, 336(sp)
    lv.q C520, 352(sp)
    lv.q C530, 368(sp)
    lv.q C600, 384(sp)
    lv.q C610, 400(sp)
    lv.q C620, 416(sp)
    lv.q C630, 432(sp)
    lv.q C700, 448(sp)
    lv.q C710, 464(sp)
    lv.q C720, 480(sp)
    lv.q C730, 496(sp)
    addiu v1, sp, 512
    lw t0, 0(v1)
    lw t1, 4(v1)
    lw t2, 8(v1)
    lw t3, 12(v1)
    mtvc t0, $128
    mtvc t1, $129
    mtvc t2, $130
    mtvc t3, $131
    lw t0, 16(v1)
    mtvc t0, $132
    lw t0, 544(sp)
    mtvc t0, $136
    lw t0, 548(sp)
    mtvc t0, $137
    lw t0, 552(sp)
    mtvc t0, $138
    lw t0, 556(sp)
    mtvc t0, $139
    lw t0, 560(sp)
    mtvc t0, $140
    lw t0, 564(sp)
    mtvc t0, $141
    lw t0, 568(sp)
    mtvc t0, $142
    lw t0, 572(sp)
    mtvc t0, $143
    move v0, zero
    jr ra
    addiu sp, sp, 576
}
#endif /* other builds: after vfpu_ctx_load, which the SDK's GCC inlines into it */

/* Default context: every register NaN (0x7F800001), identity prefixes, all condition bits set and
   the generator seed; returns 0. */
#ifdef __MWERKS__
int vfpu_ctx_init(VfpuContext *ctx);
#else
int vfpu_ctx_init(VfpuContext *ctx)
{
    unsigned int *r;
    int i;

    /* the SDK's GCC: four words per pass, the counter reversed (unused) */
    for (i = 0; i < 32; i++) {
        ctx->regs[4 * i] = 0x7F800001;
        ctx->regs[4 * i + 1] = 0x7F800001;
        ctx->regs[4 * i + 2] = 0x7F800001;
        ctx->regs[4 * i + 3] = 0x7F800001;
    }
    ctx->ctrl[0] = 0xE4;
    ctx->ctrl[1] = 0xE4;
    ctx->ctrl[2] = 0;
    ctx->ctrl[3] = 0x3F;
    ctx->ctrl[4] = 0;
    r = ctx->rcx;
    r[0] = 0x3F800001;
    r[1] = 0x3F800002;
    r[2] = 0x3F800004;
    r[3] = 0x3F800008;
    r[4] = 0x3F800000;
    r[5] = 0x3F800000;
    r[6] = 0x3F800000;
    r[7] = 0x3F800000;
    return 0;
}
#endif

/* returns 0 */
#ifdef __MWERKS__
int vfpu_ctx_save(VfpuContext *ctx);
#elif !defined(__mips__)
int vfpu_ctx_save(VfpuContext *ctx)
{
    memcpy(ctx->regs, vfpu_cur.regs, sizeof(ctx->regs));
    memcpy(ctx->ctrl, vfpu_cur.ctrl, sizeof(ctx->ctrl));
    memcpy(ctx->rcx, vfpu_rcx, sizeof(ctx->rcx));
    return 0;
}
#else
int vfpu_ctx_save(VfpuContext *ctx)
{
    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "sv.q C000, 0(%0)\n"
        "sv.q C010, 16(%0)\n"
        "sv.q C020, 32(%0)\n"
        "sv.q C030, 48(%0)\n"
        "sv.q C100, 64(%0)\n"
        "sv.q C110, 80(%0)\n"
        "sv.q C120, 96(%0)\n"
        "sv.q C130, 112(%0)\n"
        "sv.q C200, 128(%0)\n"
        "sv.q C210, 144(%0)\n"
        "sv.q C220, 160(%0)\n"
        "sv.q C230, 176(%0)\n"
        "sv.q C300, 192(%0)\n"
        "sv.q C310, 208(%0)\n"
        "sv.q C320, 224(%0)\n"
        "sv.q C330, 240(%0)\n"
        "sv.q C400, 256(%0)\n"
        "sv.q C410, 272(%0)\n"
        "sv.q C420, 288(%0)\n"
        "sv.q C430, 304(%0)\n"
        "sv.q C500, 320(%0)\n"
        "sv.q C510, 336(%0)\n"
        "sv.q C520, 352(%0)\n"
        "sv.q C530, 368(%0)\n"
        "sv.q C600, 384(%0)\n"
        "sv.q C610, 400(%0)\n"
        "sv.q C620, 416(%0)\n"
        "sv.q C630, 432(%0)\n"
        "sv.q C700, 448(%0)\n"
        "sv.q C710, 464(%0)\n"
        "sv.q C720, 480(%0)\n"
        "sv.q C730, 496(%0)\n"
        "addiu $3, %0, 512\n"
        "mfvc $8, $128\n"
        "mfvc $9, $129\n"
        "mfvc $10, $130\n"
        "mfvc $11, $131\n"
        "sw $8, 0($3)\n"
        "sw $9, 4($3)\n"
        "sw $10, 8($3)\n"
        "sw $11, 12($3)\n"
        "mfvc $8, $132\n"
        "sw $8, 16($3)\n"
        "mfvc $8, $136\n"
        "sw $8, 544(%0)\n"
        "mfvc $8, $137\n"
        "sw $8, 548(%0)\n"
        "mfvc $8, $138\n"
        "sw $8, 552(%0)\n"
        "mfvc $8, $139\n"
        "sw $8, 556(%0)\n"
        "mfvc $8, $140\n"
        "sw $8, 560(%0)\n"
        "mfvc $8, $141\n"
        "sw $8, 564(%0)\n"
        "mfvc $8, $142\n"
        "sw $8, 568(%0)\n"
        "mfvc $8, $143\n"
        "sw $8, 572(%0)\n"
        ".set pop\n"
        : 
        : "r" (ctx)
        : "memory", "$8", "$9", "$10", "$11", "$3");
    return 0;
}
#endif

/* returns 0 */
#ifdef __MWERKS__
int vfpu_ctx_load(VfpuContext *ctx);
#elif !defined(__mips__)
int vfpu_ctx_load(VfpuContext *ctx)
{
    memcpy(vfpu_cur.regs, ctx->regs, sizeof(ctx->regs));
    memcpy(vfpu_cur.ctrl, ctx->ctrl, sizeof(ctx->ctrl));
    memcpy(vfpu_rcx, ctx->rcx, sizeof(ctx->rcx));
    return 0;
}
#else
int vfpu_ctx_load(VfpuContext *ctx)
{
    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%0)\n"
        "lv.q C010, 16(%0)\n"
        "lv.q C020, 32(%0)\n"
        "lv.q C030, 48(%0)\n"
        "lv.q C100, 64(%0)\n"
        "lv.q C110, 80(%0)\n"
        "lv.q C120, 96(%0)\n"
        "lv.q C130, 112(%0)\n"
        "lv.q C200, 128(%0)\n"
        "lv.q C210, 144(%0)\n"
        "lv.q C220, 160(%0)\n"
        "lv.q C230, 176(%0)\n"
        "lv.q C300, 192(%0)\n"
        "lv.q C310, 208(%0)\n"
        "lv.q C320, 224(%0)\n"
        "lv.q C330, 240(%0)\n"
        "lv.q C400, 256(%0)\n"
        "lv.q C410, 272(%0)\n"
        "lv.q C420, 288(%0)\n"
        "lv.q C430, 304(%0)\n"
        "lv.q C500, 320(%0)\n"
        "lv.q C510, 336(%0)\n"
        "lv.q C520, 352(%0)\n"
        "lv.q C530, 368(%0)\n"
        "lv.q C600, 384(%0)\n"
        "lv.q C610, 400(%0)\n"
        "lv.q C620, 416(%0)\n"
        "lv.q C630, 432(%0)\n"
        "lv.q C700, 448(%0)\n"
        "lv.q C710, 464(%0)\n"
        "lv.q C720, 480(%0)\n"
        "lv.q C730, 496(%0)\n"
        "addiu $3, %0, 512\n"
        "lw $8, 0($3)\n"
        "lw $9, 4($3)\n"
        "lw $10, 8($3)\n"
        "lw $11, 12($3)\n"
        "mtvc $8, $128\n"
        "mtvc $9, $129\n"
        "mtvc $10, $130\n"
        "mtvc $11, $131\n"
        "lw $8, 16($3)\n"
        "mtvc $8, $132\n"
        "lw $8, 544(%0)\n"
        "mtvc $8, $136\n"
        "lw $8, 548(%0)\n"
        "mtvc $8, $137\n"
        "lw $8, 552(%0)\n"
        "mtvc $8, $138\n"
        "lw $8, 556(%0)\n"
        "mtvc $8, $139\n"
        "lw $8, 560(%0)\n"
        "mtvc $8, $140\n"
        "lw $8, 564(%0)\n"
        "mtvc $8, $141\n"
        "lw $8, 568(%0)\n"
        "mtvc $8, $142\n"
        "lw $8, 572(%0)\n"
        "mtvc $8, $143\n"
        ".set pop\n"
        : 
        : "r" (ctx)
        : "memory", "$8", "$9", "$10", "$11", "$3");
    return 0;
}
#endif

#ifndef __MWERKS__
int vfpu_reset(void)
{
    VfpuContext ctx;

    vfpu_ctx_init(&ctx);
    return vfpu_ctx_load(&ctx);
}
#endif

/* Generator seed into st[8] (the vfpu_ctx_init values); returns 0. */
#ifndef __MWERKS__ /* PSP: vfpu_mem_gcc.c */
int vfpu_rng_init(unsigned int *st)
{
    st[0] = 0x3F800001;
    st[1] = 0x3F800002;
    st[2] = 0x3F800004;
    st[3] = 0x3F800008;
    st[4] = 0x3F800000;
    st[5] = 0x3F800000;
    st[6] = 0x3F800000;
    st[7] = 0x3F800000;
    return 0;
}
#else
int vfpu_rng_init(unsigned int *st);
#endif

/* Generator state ($136-$143) into st[8]; returns 0. */
#ifdef __MWERKS__
int vfpu_rng_save(unsigned int *st);
#elif !defined(__mips__)
int vfpu_rng_save(unsigned int *st)
{
    memcpy(st, vfpu_rcx, sizeof(vfpu_rcx));
    return 0;
}
#else
int vfpu_rng_save(unsigned int *st)
{
    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "mfvc $8, $136\n"
        "sw $8, 0(%0)\n"
        "mfvc $8, $137\n"
        "sw $8, 4(%0)\n"
        "mfvc $8, $138\n"
        "sw $8, 8(%0)\n"
        "mfvc $8, $139\n"
        "sw $8, 12(%0)\n"
        "mfvc $8, $140\n"
        "sw $8, 16(%0)\n"
        "mfvc $8, $141\n"
        "sw $8, 20(%0)\n"
        "mfvc $8, $142\n"
        "sw $8, 24(%0)\n"
        "mfvc $8, $143\n"
        "sw $8, 28(%0)\n"
        ".set pop\n"
        : 
        : "r" (st)
        : "memory", "$8");
    return 0;
}
#endif

/* returns 0 */
#ifdef __MWERKS__
int vfpu_rng_load(unsigned int *st);
#elif !defined(__mips__)
int vfpu_rng_load(unsigned int *st)
{
    memcpy(vfpu_rcx, st, sizeof(vfpu_rcx));
    return 0;
}
#else
int vfpu_rng_load(unsigned int *st)
{
    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "lw $8, 0(%0)\n"
        "mtvc $8, $136\n"
        "lw $8, 4(%0)\n"
        "mtvc $8, $137\n"
        "lw $8, 8(%0)\n"
        "mtvc $8, $138\n"
        "lw $8, 12(%0)\n"
        "mtvc $8, $139\n"
        "lw $8, 16(%0)\n"
        "mtvc $8, $140\n"
        "lw $8, 20(%0)\n"
        "mtvc $8, $141\n"
        "lw $8, 24(%0)\n"
        "mtvc $8, $142\n"
        "lw $8, 28(%0)\n"
        "mtvc $8, $143\n"
        ".set pop\n"
        : 
        : "r" (st)
        : "memory", "$8");
    return 0;
}
#endif

/* memset; returns dst */
#ifdef __MWERKS__
asm void *vfpu_memset(void *dst, int c, unsigned int n)
{
    lui t0, 0x4000
    move v0, a0
    move v1, t0
    ins v1, a0, 0x0, 0x1d
    ins a1, a1, 0x8, 0x8
    ins a1, a1, 0x10, 0x10
    mtv a1, S000
    vpfxs X, X, X, X
    vmov.q C000, C000
L_0881F660:
    andi t0, a0, 0x3f
    beqz t0, L_0881F684
    nop
    beqz a2, L_0881F6D4
    addiu a2, a2, -1
    sb a1, 0(a0)
    addiu a0, a0, 1
    j L_0881F660
    addiu v1, v1, 1
L_0881F684:
    sltiu t0, a2, 64
    bnez t0, L_0881F6C0
    nop
L_0881F690:
    cache 0x1b, 0(a0)
    addiu a0, a0, 64
    addiu a2, a2, -64
    sltiu t0, a2, 64
    sync
    sv.q C000, 0(v1), wb
    sv.q C000, 16(v1), wb
    sv.q C000, 32(v1), wb
    sv.q C000, 48(v1), wb
    addiu v1, v1, 64
    beqz t0, L_0881F690
    vnop
L_0881F6C0:
    beqz a2, L_0881F6D4
L_0881F6C4:
    addiu a2, a2, -1
    sb a1, 0(a0)
    bnez a2, L_0881F6C4
    addiu a0, a0, 1
L_0881F6D4:
    vflush
    vnop
    jr ra
    nop
}
#elif !defined(__mips__)
void *vfpu_memset(void *dst, int c, unsigned int n)
{
    return memset(dst, c, n);
}
#else
void *vfpu_memset(void *dst, int c, unsigned int n)
{
    /* the SDK's GCC: 3 words left (the original copies dst to v0 before `move v1,t0; ins`, the sched2 of cc1
       1.7.1 after them: the copy has priority 1 against 2) */
    void *r = dst;
    unsigned int u = __builtin_allegrex_ins(0x40000000, (unsigned int)dst, 0, 29);

    __asm__ volatile (
        ".set push\n"
        ".set noreorder\n"
        "ins %1, %1, 8, 8\n"
        "ins %1, %1, 16, 16\n"
        "mtv %1, S000\n"
        "vpfxs X, X, X, X\n"
        "vmov.q C000, C000\n"
        "1:\n"
        "andi $8, %0, 0x3f\n"
        "beqz $8, 2f\n"
        "nop\n"
        "beqz %2, 5f\n"
        "addiu %2, %2, -1\n"
        "sb %1, 0(%0)\n"
        "addiu %0, %0, 1\n"
        "j 1b\n"
        "addiu %3, %3, 1\n"
        "2:\n"
        "sltiu $8, %2, 64\n"
        "bnez $8, 4f\n"
        "nop\n"
        "3:\n"
        "cache 0x1b, 0(%0)\n"
        "addiu %0, %0, 64\n"
        "addiu %2, %2, -64\n"
        "sltiu $8, %2, 64\n"
        "sync\n"
        "sv.q C000, 0(%3), wb\n"
        "sv.q C000, 16(%3), wb\n"
        "sv.q C000, 32(%3), wb\n"
        "sv.q C000, 48(%3), wb\n"
        "addiu %3, %3, 64\n"
        "beqz $8, 3b\n"
        "vnop\n"
        "4:\n"
        "beqz %2, 5f\n"
        "6:\n"
        "addiu %2, %2, -1\n"
        "sb %1, 0(%0)\n"
        "bnez %2, 6b\n"
        "addiu %0, %0, 1\n"
        "5:\n"
        "vflush\n"
        "vnop\n"
        ".set pop\n"
        : "+r" (dst), "+r" (c), "+r" (n), "+r" (u)
        :
        : "$8", "memory");
    return r;
}
#endif
