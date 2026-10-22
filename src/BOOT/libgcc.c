// @range 0x08816C3C-0x08817784
/* libgcc 64-bit integer helpers (libgcc2.c) called by GCC-compiled code in BOOT: multiply, and
   unsigned divide and modulo (the udivmoddi4 normalisation algorithm, inlined twice). Whole `asm`
   functions. Outside the PSP build the C versions compute the same values with 32-bit operations
   only: a 32-bit host compiler would turn a 64-bit `/` back into a call to these very functions. */

typedef unsigned long long UDItype;
typedef long long DItype;

#ifndef __MWERKS__
/* restoring shift-subtract division; division by zero gives quotient ~0 and remainder n */
static UDItype udivmod(UDItype n, UDItype d, UDItype *rem)
{
    UDItype q = 0;
    UDItype r = 0;
    int i;

    if (d == 0) {
        if (rem != 0) {
            *rem = n;
        }
        return ~(UDItype)0;
    }
    for (i = 63; i >= 0; i--) {
        r = (r << 1) | ((n >> i) & 1);
        if (r >= d) {
            r -= d;
            q |= (UDItype)1 << i;
        }
    }
    if (rem != 0) {
        *rem = r;
    }
    return q;
}
#endif

#ifdef __MWERKS__
asm DItype __muldi3(DItype a, DItype b)
{
    mult a1, a2
    mflo t2
    multu a0, a2
    mfhi t1
    mflo v0
    mult a0, a3
    mflo a0
    addu a0, a0, t2
    jr ra
    addu v1, a0, t1
}
#else
DItype __muldi3(DItype a, DItype b)
{
    unsigned int al = (unsigned int)a;
    unsigned int ah = (unsigned int)((UDItype)a >> 32);
    unsigned int bl = (unsigned int)b;
    unsigned int bh = (unsigned int)((UDItype)b >> 32);
    unsigned int l0 = (al & 0xFFFF) * (bl & 0xFFFF);
    unsigned int l1 = (al >> 16) * (bl & 0xFFFF);
    unsigned int l2 = (al & 0xFFFF) * (bl >> 16);
    unsigned int l3 = (al >> 16) * (bl >> 16);
    unsigned int mid = (l0 >> 16) + (l1 & 0xFFFF) + (l2 & 0xFFFF);
    unsigned int lo = (l0 & 0xFFFF) | (mid << 16);
    unsigned int hi = l3 + (l1 >> 16) + (l2 >> 16) + (mid >> 16);

    hi += ah * bl + al * bh;
    return (DItype)(((UDItype)hi << 32) | lo);
}
#endif

#ifdef __MWERKS__
asm UDItype __udivdi3(UDItype n, UDItype d)
{
    addiu sp, sp, -16
    sw s1, 4(sp)
    move t0, a3
    move t2, a1
    sw s0, 0(sp)
    move t1, a2
    bnez a3, L_08817024
    move t6, a0
    sltu v0, a1, a2
    beqz v0, L_08816DE4
    ori v0, zero, 0xffff
    addiu v1, zero, 8
    sltiu a0, a2, 256
    move a1, v1
    sltu v0, v0, a2
    beqz v0, L_08816CC4
    movn a1, zero, a0
    lui v0, 0xff
    ori v0, v0, 0xffff
    addiu v1, zero, 16
    sltu v0, v0, a2
    addiu a0, zero, 24
    move a1, v1
    movn a1, a0, v0
L_08816CC4:
    lui v1, 0x887
    srlv v0, a2, a1
    addiu v1, v1, 17552
    addu v0, v0, v1
    lbu a0, 0(v0)
    addiu v0, zero, 32
    addu a0, a0, a1
    subu a3, v0, a0
    beqzl a3, L_08816D08
    srl a3, t1, 0x10
    subu v0, v0, a3
    srlv v0, t6, v0
    sllv v1, t2, a3
    or t2, v1, v0
    sllv t6, t6, a3
    sllv t1, t1, a3
    srl a3, t1, 0x10
L_08816D08:
    divu t2, a3
    andi t4, t1, 0xffff
    srl a0, t6, 0x10
    beqzl a3, L_08816D1C
    .word 0x000001CD  /* break 0x0,0x7 */
L_08816D1C:
    mflo v1
    mfhi v0
    mflo t3
    mult v1, t4
    sll v0, v0, 0x10
    or a2, v0, a0
    mflo a1
    sltu v1, a2, a1
    beqz v1, L_08816D68
    divu t2, a3
    addu a2, a2, t1
    sltu v0, a2, t1
    bnez v0, L_08816D68
    addiu t3, t3, -1
    sltu v0, a2, a1
    beqzl v0, L_08816D6C
    subu a2, a2, a1
    addiu t3, t3, -1
    addu a2, a2, t1
L_08816D68:
    subu a2, a2, a1
L_08816D6C:
    divu a2, a3
    andi a0, t6, 0xffff
    beqzl a3, L_08816D7C
    .word 0x000001CD  /* break 0x0,0x7 */
L_08816D7C:
    mflo v1
    mfhi v0
    mflo t0
    mult v1, t4
    sll v0, v0, 0x10
    or a0, v0, a0
    mflo a1
    sltu v1, a0, a1
    beqz v1, L_08816DC0
    divu a2, a3
    addu a0, a0, t1
    sltu v0, a0, t1
    bnez v0, L_08816DC0
    addiu t0, t0, -1
    sltu v0, a0, a1
    sltu v0, zero, v0
    subu t0, t0, v0
L_08816DC0:
    sll v0, t3, 0x10
    or a0, v0, t0
L_08816DC8:
    move a2, zero
L_08816DCC:
    lw s1, 4(sp)
    lw s0, 0(sp)
    move v0, a0
    move v1, a2
    jr ra
    addiu sp, sp, 16
L_08816DE4:
    bnez a2, L_08816E08
    addiu v1, zero, 8
    addiu v0, zero, 1
    divu v0, a3
    beqzl a2, L_08816DFC
    .word 0x000001CD  /* break 0x0,0x7 */
L_08816DFC:
    mflo t1
    ori v0, zero, 0xffff
    addiu v1, zero, 8
L_08816E08:
    sltiu a0, t1, 256
    move a1, v1
    sltu v0, v0, t1
    beqz v0, L_08816E38
    movn a1, zero, a0
    lui v0, 0xff
    ori v0, v0, 0xffff
    addiu v1, zero, 16
    sltu v0, v0, t1
    addiu a0, zero, 24
    move a1, v1
    movn a1, a0, v0
L_08816E38:
    lui v1, 0x887
    srlv v0, t1, a1
    addiu v1, v1, 17552
    addu v0, v0, v1
    lbu a0, 0(v0)
    addiu v0, zero, 32
    addu a0, a0, a1
    subu a3, v0, a0
    bnez a3, L_08816F30
    subu a1, v0, a3
    subu t2, t2, t1
    addiu a2, zero, 1
    srl t0, t1, 0x10
    andi s0, t1, 0xffff
L_08816E70:
    divu t2, t0
    srl a0, t6, 0x10
    beqzl t0, L_08816E80
    .word 0x000001CD  /* break 0x0,0x7 */
L_08816E80:
    mflo v1
    mfhi v0
    mflo t3
    mult v1, s0
    sll v0, v0, 0x10
    or a3, v0, a0
    mflo a1
    sltu v1, a3, a1
    beqz v1, L_08816ECC
    divu t2, t0
    addu a3, a3, t1
    sltu v0, a3, t1
    bnez v0, L_08816ECC
    addiu t3, t3, -1
    sltu v0, a3, a1
    beqzl v0, L_08816ED0
    subu a3, a3, a1
    addiu t3, t3, -1
    addu a3, a3, t1
L_08816ECC:
    subu a3, a3, a1
L_08816ED0:
    divu a3, t0
    andi a0, t6, 0xffff
    beqzl t0, L_08816EE0
    .word 0x000001CD  /* break 0x0,0x7 */
L_08816EE0:
    mflo v1
    mfhi v0
    mflo t2
    mult v1, s0
    sll v0, v0, 0x10
    or a0, v0, a0
    mflo a1
    sltu v1, a0, a1
    beqz v1, L_08816F24
    divu a3, t0
    addu a0, a0, t1
    sltu v0, a0, t1
    bnez v0, L_08816F24
    addiu t2, t2, -1
    sltu v0, a0, a1
    sltu v0, zero, v0
    subu t2, t2, v0
L_08816F24:
    sll v0, t3, 0x10
    j L_08816DCC
    or a0, v0, t2
L_08816F30:
    sllv t1, t1, a3
    srl t0, t1, 0x10
    srlv t4, t2, a1
    divu t4, t0
    andi s0, t1, 0xffff
    srlv v1, t6, a1
    sllv v0, t2, a3
    or t2, v0, v1
    srl a1, t2, 0x10
    sllv t6, t6, a3
    beqzl t0, L_08816F60
    .word 0x000001CD  /* break 0x0,0x7 */
L_08816F60:
    move t3, t0
    move s1, s0
    mflo a2
    mfhi a0
    mflo t7
    mult a2, s0
    sll a0, a0, 0x10
    or a3, a0, a1
    mflo t5
    sltu v0, a3, t5
    beqz v0, L_08816FB4
    divu t4, t0
    addu a3, a3, t1
    sltu v0, a3, t1
    bnez v0, L_08816FB4
    addiu t7, a2, -1
    sltu v0, a3, t5
    beqzl v0, L_08816FB8
    subu a3, a3, t5
    addiu t7, t7, -1
    addu a3, a3, t1
L_08816FB4:
    subu a3, a3, t5
L_08816FB8:
    divu a3, t3
    andi a0, t2, 0xffff
    beqzl t3, L_08816FC8
    .word 0x000001CD  /* break 0x0,0x7 */
L_08816FC8:
    mflo v1
    mfhi v0
    mflo a2
    mult v1, s1
    sll v0, v0, 0x10
    or a0, v0, a0
    mflo t5
    sltu v1, a0, t5
    beqz v1, L_08817014
    divu a3, t3
    addu a0, a0, t1
    sltu v0, a0, t1
    bnez v0, L_08817014
    addiu a2, a2, -1
    sltu v0, a0, t5
    beqz v0, L_08817018
    sll v0, t7, 0x10
    addiu a2, a2, -1
    addu a0, a0, t1
L_08817014:
    sll v0, t7, 0x10
L_08817018:
    or a2, v0, a2
    j L_08816E70
    subu t2, a0, t5
L_08817024:
    sltu v0, a1, a3
    move a0, zero
    bnez v0, L_08816DCC
    move a2, zero
    ori v0, zero, 0xffff
    addiu v1, zero, 8
    sltiu a0, a3, 256
    move a1, v1
    sltu v0, v0, a3
    beqz v0, L_0881706C
    movn a1, zero, a0
    lui v0, 0xff
    ori v0, v0, 0xffff
    addiu v1, zero, 16
    sltu v0, v0, a3
    addiu a0, zero, 24
    move a1, v1
    movn a1, a0, v0
L_0881706C:
    lui v1, 0x887
    srlv v0, t0, a1
    addiu v1, v1, 17552
    addu v0, v0, v1
    lbu a0, 0(v0)
    addiu v0, zero, 32
    addu a0, a0, a1
    subu a3, v0, a0
    bnez a3, L_088170AC
    subu a1, v0, a3
    sltu v0, t6, t1
    sltu v1, t0, t2
    xori v0, v0, 0x1
    or v1, v1, v0
    j L_08816DC8
    sltu a0, zero, v1
L_088170AC:
    srlv v0, t1, a1
    sllv v1, t0, a3
    or t0, v1, v0
    srl t3, t0, 0x10
    srlv t4, t2, a1
    divu t4, t3
    andi s0, t0, 0xffff
    srlv v1, t6, a1
    sllv v0, t2, a3
    or t2, v0, v1
    srl a1, t2, 0x10
    sllv t6, t6, a3
    sllv t1, t1, a3
    beqzl t3, L_088170E8
    .word 0x000001CD  /* break 0x0,0x7 */
L_088170E8:
    mflo a2
    mfhi a0
    mflo t7
    mult a2, s0
    sll a0, a0, 0x10
    or a3, a0, a1
    mflo t5
    sltu v0, a3, t5
    beqz v0, L_08817134
    divu t4, t3
    addu a3, a3, t0
    sltu v0, a3, t0
    bnez v0, L_08817134
    addiu t7, a2, -1
    sltu v0, a3, t5
    beqzl v0, L_08817138
    subu a3, a3, t5
    addiu t7, t7, -1
    addu a3, a3, t0
L_08817134:
    subu a3, a3, t5
L_08817138:
    divu a3, t3
    andi a0, t2, 0xffff
    beqzl t3, L_08817148
    .word 0x000001CD  /* break 0x0,0x7 */
L_08817148:
    mflo v1
    mfhi v0
    mflo a2
    mult v1, s0
    sll v0, v0, 0x10
    or a1, v0, a0
    mflo t5
    sltu v1, a1, t5
    beqz v1, L_08817194
    divu a3, t3
    addu a1, a1, t0
    sltu v0, a1, t0
    bnez v0, L_08817194
    addiu a2, a2, -1
    sltu v0, a1, t5
    beqz v0, L_08817198
    sll v0, t7, 0x10
    addu a1, a1, t0
    addiu a2, a2, -1
L_08817194:
    sll v0, t7, 0x10
L_08817198:
    or a0, v0, a2
    subu a1, a1, t5
    multu a0, t1
    mfhi v1
    sltu v0, a1, v1
    bnezl v0, L_08816DC8
    addiu a0, a0, -1
    xor v0, v1, a1
    mflo a1
    sltu v1, t6, a1
    sltiu v0, v0, 1
    and v0, v0, v1
    beqz v0, L_08816DCC
    move a2, zero
    j L_08816DCC
    addiu a0, a0, -1
}
#else
UDItype __udivdi3(UDItype n, UDItype d)
{
    return udivmod(n, d, 0);
}
#endif

#ifdef __MWERKS__
asm UDItype __umoddi3(UDItype n, UDItype d)
{
    addiu sp, sp, -32
    sw s3, 28(sp)
    move t0, a3
    move s3, sp
    sw s2, 24(sp)
    move t3, a2
    move t7, a0
    sw s1, 20(sp)
    move t1, a1
    bnez a3, L_08817574
    sw s0, 16(sp)
    sltu v0, a1, a2
    beqz v0, L_0881736C
    ori v0, zero, 0xffff
    addiu v1, zero, 8
    sltiu a0, a2, 256
    move a1, v1
    sltu v0, v0, a2
    beqz v0, L_08817244
    movn a1, zero, a0
    lui v0, 0xff
    ori v0, v0, 0xffff
    addiu v1, zero, 16
    sltu v0, v0, a2
    addiu a0, zero, 24
    move a1, v1
    movn a1, a0, v0
L_08817244:
    lui v1, 0x887
    srlv v0, a2, a1
    addiu v1, v1, 17552
    addu v0, v0, v1
    lbu a0, 0(v0)
    addiu v0, zero, 32
    addu a0, a0, a1
    subu a3, v0, a0
    beqz a3, L_08817288
    srl a2, t3, 0x10
    subu v0, v0, a3
    srlv v0, t7, v0
    sllv v1, t1, a3
    or t1, v1, v0
    sllv t3, t3, a3
    sllv t7, t7, a3
    srl a2, t3, 0x10
L_08817288:
    divu t1, a2
    andi t2, t3, 0xffff
    srl a0, t7, 0x10
    beqzl a2, L_0881729C
    .word 0x000001CD  /* break 0x0,0x7 */
L_0881729C:
    mflo v1
    mfhi v0
    mult v1, t2
    sll v0, v0, 0x10
    or a1, v0, a0
    mflo t0
    sltu v1, a1, t0
    beqz v1, L_088172DC
    divu t1, a2
    addu a1, a1, t3
    sltu v0, a1, t3
    bnezl v0, L_088172E0
    subu a1, a1, t0
    sltu v0, a1, t0
    addu v1, a1, t3
    movn a1, v1, v0
L_088172DC:
    subu a1, a1, t0
L_088172E0:
    divu a1, a2
    andi a0, t7, 0xffff
    beqzl a2, L_088172F0
    .word 0x000001CD  /* break 0x0,0x7 */
L_088172F0:
    mflo v1
    mfhi v0
    mult v1, t2
    sll v0, v0, 0x10
    or a0, v0, a0
    mflo t0
    sltu v1, a0, t0
    beqz v1, L_08817330
    divu a1, a2
    addu a0, a0, t3
    sltu v0, a0, t3
    bnez v0, L_08817334
    subu t7, a0, t0
    sltu v0, a0, t0
    addu v1, a0, t3
    movn a0, v1, v0
L_08817330:
    subu t7, a0, t0
L_08817334:
    srlv t8, t7, a3
    move t9, zero
L_0881733C:
    beqz s3, L_0881734C
    nop
L_08817344:
    sw t8, 0(s3)
    sw t9, 4(s3)
L_0881734C:
    lw v0, 0(sp)
    lw v1, 4(sp)
    lw s3, 28(sp)
    lw s2, 24(sp)
    lw s1, 20(sp)
    lw s0, 16(sp)
    jr ra
    addiu sp, sp, 32
L_0881736C:
    bnez a2, L_08817390
    addiu v1, zero, 8
    addiu v0, zero, 1
    divu v0, a3
    beqzl a2, L_08817384
    .word 0x000001CD  /* break 0x0,0x7 */
L_08817384:
    mflo t3
    ori v0, zero, 0xffff
    addiu v1, zero, 8
L_08817390:
    sltiu a0, t3, 256
    move a1, v1
    sltu v0, v0, t3
    beqz v0, L_088173C0
    movn a1, zero, a0
    lui v0, 0xff
    ori v0, v0, 0xffff
    addiu v1, zero, 16
    sltu v0, v0, t3
    addiu a0, zero, 24
    move a1, v1
    movn a1, a0, v0
L_088173C0:
    lui v1, 0x887
    srlv v0, t3, a1
    addiu v1, v1, 17552
    addu v0, v0, v1
    lbu a0, 0(v0)
    addiu v0, zero, 32
    addu a0, a0, a1
    subu a3, v0, a0
    bnez a3, L_088174A0
    subu s0, v0, a3
    subu t1, t1, t3
    srl t0, t3, 0x10
    andi t6, t3, 0xffff
L_088173F4:
    divu t1, t0
    srl a0, t7, 0x10
    beqzl t0, L_08817404
    .word 0x000001CD  /* break 0x0,0x7 */
L_08817404:
    mflo v1
    mfhi v0
    mult v1, t6
    sll v0, v0, 0x10
    or a1, v0, a0
    mflo a2
    sltu v1, a1, a2
    beqz v1, L_08817444
    divu t1, t0
    addu a1, a1, t3
    sltu v0, a1, t3
    bnezl v0, L_08817448
    subu a1, a1, a2
    sltu v0, a1, a2
    addu v1, a1, t3
    movn a1, v1, v0
L_08817444:
    subu a1, a1, a2
L_08817448:
    divu a1, t0
    andi a0, t7, 0xffff
    beqzl t0, L_08817458
    .word 0x000001CD  /* break 0x0,0x7 */
L_08817458:
    mflo v1
    mfhi v0
    mult v1, t6
    sll v0, v0, 0x10
    or a0, v0, a0
    mflo a2
    sltu v1, a0, a2
    beqz v1, L_08817498
    divu a1, t0
    addu a0, a0, t3
    sltu v0, a0, t3
    bnezl v0, L_08817334
    subu t7, a0, a2
    sltu v0, a0, a2
    addu v1, a0, t3
    movn a0, v1, v0
L_08817498:
    j L_08817334
    subu t7, a0, a2
L_088174A0:
    sllv t3, t3, a3
    srl t0, t3, 0x10
    srlv t5, t1, s0
    divu t5, t0
    andi t6, t3, 0xffff
    srlv v1, t7, s0
    sllv v0, t1, a3
    or t1, v0, v1
    srl a1, t1, 0x10
    beqzl t0, L_088174CC
    .word 0x000001CD  /* break 0x0,0x7 */
L_088174CC:
    move t4, t0
    sllv t7, t7, a3
    move s0, t6
    mflo a2
    mfhi a0
    mult a2, t6
    sll a0, a0, 0x10
    or t2, a0, a1
    mflo a2
    sltu v0, t2, a2
    beqz v0, L_08817518
    divu t5, t0
    addu t2, t2, t3
    sltu v0, t2, t3
    bnezl v0, L_0881751C
    subu t2, t2, a2
    sltu v0, t2, a2
    addu v1, t2, t3
    movn t2, v1, v0
L_08817518:
    subu t2, t2, a2
L_0881751C:
    divu t2, t4
    andi a0, t1, 0xffff
    beqzl t4, L_0881752C
    .word 0x000001CD  /* break 0x0,0x7 */
L_0881752C:
    mflo v1
    mfhi v0
    mult v1, s0
    sll v0, v0, 0x10
    or a0, v0, a0
    mflo a2
    sltu v1, a0, a2
    beqz v1, L_0881756C
    divu t2, t4
    addu a0, a0, t3
    sltu v0, a0, t3
    bnezl v0, L_088173F4
    subu t1, a0, a2
    sltu v0, a0, a2
    addu v1, a0, t3
    movn a0, v1, v0
L_0881756C:
    j L_088173F4
    subu t1, a0, a2
L_08817574:
    sltu v0, a1, a3
    beqz v0, L_08817594
    ori v0, zero, 0xffff
    move t8, a0
    move t9, a1
    sw t8, 0(sp)
    j L_0881734C
    sw t9, 4(sp)
L_08817594:
    addiu v1, zero, 8
    sltiu a0, a3, 256
    move a1, v1
    sltu v0, v0, a3
    beqz v0, L_088175C8
    movn a1, zero, a0
    lui v0, 0xff
    ori v0, v0, 0xffff
    addiu v1, zero, 16
    sltu v0, v0, a3
    addiu a0, zero, 24
    move a1, v1
    movn a1, a0, v0
L_088175C8:
    lui v1, 0x887
    srlv v0, t0, a1
    addiu v1, v1, 17552
    addu v0, v0, v1
    lbu a0, 0(v0)
    addiu v0, zero, 32
    addu a0, a0, a1
    subu a3, v0, a0
    bnez a3, L_08817628
    subu s0, v0, a3
    sltu v1, t7, t3
    xori v1, v1, 0x1
    sltu v0, t0, t1
    or v0, v0, v1
    beqzl v0, L_08817620
    move t8, t7
    subu a0, t7, t3
    sltu v0, t7, a0
    subu v1, t1, t0
    subu t1, v1, v0
    move t7, a0
    move t8, t7
L_08817620:
    j L_0881733C
    move t9, t1
L_08817628:
    srlv v0, t3, s0
    sllv v1, t0, a3
    or t0, v1, v0
    srl t4, t0, 0x10
    srlv t5, t1, s0
    divu t5, t4
    andi s2, t0, 0xffff
    srlv v0, t7, s0
    sllv v1, t1, a3
    or t1, v1, v0
    srl a1, t1, 0x10
    beqzl t4, L_0881765C
    .word 0x000001CD  /* break 0x0,0x7 */
L_0881765C:
    sllv t3, t3, a3
    sllv t7, t7, a3
    mflo a2
    mfhi a0
    mflo s1
    mult a2, s2
    sll a0, a0, 0x10
    or t2, a0, a1
    mflo t6
    sltu v0, t2, t6
    beqz v0, L_088176B0
    divu t5, t4
    addu t2, t2, t0
    sltu v0, t2, t0
    bnez v0, L_088176B0
    addiu s1, a2, -1
    sltu v0, t2, t6
    beqzl v0, L_088176B4
    subu t2, t2, t6
    addiu s1, s1, -1
    addu t2, t2, t0
L_088176B0:
    subu t2, t2, t6
L_088176B4:
    divu t2, t4
    andi a0, t1, 0xffff
    beqzl t4, L_088176C4
    .word 0x000001CD  /* break 0x0,0x7 */
L_088176C4:
    mflo v1
    mfhi v0
    mflo a2
    mult v1, s2
    sll v0, v0, 0x10
    or a1, v0, a0
    mflo t6
    sltu v1, a1, t6
    beqz v1, L_08817710
    divu t2, t4
    addu a1, a1, t0
    sltu v0, a1, t0
    bnez v0, L_08817710
    addiu a2, a2, -1
    sltu v0, a1, t6
    beqz v0, L_08817714
    sll v0, s1, 0x10
    addiu a2, a2, -1
    addu a1, a1, t0
L_08817710:
    sll v0, s1, 0x10
L_08817714:
    or v0, v0, a2
    subu a1, a1, t6
    multu v0, t3
    mfhi t2
    sltu v1, a1, t2
    mflo a2
    bnez v1, L_0881774C
    subu a0, a2, t3
    xor v0, t2, a1
    sltiu v0, v0, 1
    sltu v1, t7, a2
    and v0, v0, v1
    beqz v0, L_0881775C
    nop
L_0881774C:
    sltu v0, a2, a0
    subu v1, t2, t0
    subu t2, v1, v0
    move a2, a0
L_0881775C:
    beqz s3, L_0881734C
    subu v1, t7, a2
    subu a0, a1, t2
    sltu v0, t7, v1
    subu t1, a0, v0
    sllv a1, t1, s0
    srlv v1, v1, a3
    or t8, a1, v1
    j L_08817344
    srlv t9, t1, a3
}
#else
UDItype __umoddi3(UDItype n, UDItype d)
{
    UDItype r;

    udivmod(n, d, &r);
    return r;
}
#endif
