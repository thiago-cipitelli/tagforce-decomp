// @range 0x08816C3C-0x08817784
// @cc gcc -fexceptions -fnon-call-exceptions
/* libgcc 64-bit integer helpers called by GCC-compiled code in BOOT: multiply, and unsigned divide and modulo (the
   udivmoddi4 normalisation algorithm, inlined twice). On the PSP they are GCC 3.3.3's libgcc2.c itself
   (include/gcc-3.3.3/), compiled by the SDK's GCC with the flags libgcc's build gives the division functions
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"), and match byte for byte. Outside the PSP build the C versions below
   compute the same values with 32-bit operations only: a 32-bit host compiler would turn a 64-bit `/` back into a
   call to these very functions. */

#ifdef __mips__
#define IN_LIBGCC2
#define __clz_tab D_08874490          /* the table of _clz.o, in BOOT's .data */
#define L_muldi3
#define L_udivdi3
#define L_umoddi3
#include "gcc-3.3.3/libgcc2.c"
#else
typedef unsigned long long UDItype;
typedef long long DItype;

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
