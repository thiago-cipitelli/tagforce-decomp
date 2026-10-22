// @range 0x08816C3C-0x08817784
// @asm 0x08816C64-0x08817784
/* libgcc 64-bit integer helpers (libgcc2.c) called by GCC-compiled code in BOOT: multiply, and
   unsigned divide and modulo (the udivmoddi4 normalisation algorithm, inlined twice). __muldi3 is
   a whole `asm` function; __udivdi3 and __umoddi3 count leading zeros through __clz_tab
   (D_08874490) loaded by a split lui/addiu, so they stay in asm with their C under NON_MATCHING.
   Outside the PSP build the C versions compute the same values with 32-bit operations
   only: a 32-bit host compiler would turn a 64-bit `/` back into a call to these very functions. */

typedef unsigned long long UDItype;
typedef long long DItype;

#if !defined(__MWERKS__) || defined(NON_MATCHING)
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

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: __clz_tab through a split lui/addiu */
UDItype __udivdi3(UDItype n, UDItype d)
{
    return udivmod(n, d, 0);
}

UDItype __umoddi3(UDItype n, UDItype d)
{
    UDItype r;

    udivmod(n, d, &r);
    return r;
}
#endif
