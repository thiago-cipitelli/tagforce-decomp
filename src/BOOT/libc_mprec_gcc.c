// @cc gcc -Iinclude/newlib-1.13.0/shim
// @rodata 0x08873C18-0x08873D50
/* newlib 1.13.0 libc/stdlib/mprec.c (the multi-precision integers of dtoa/strtod) compiled by the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"): the source is include/newlib-1.13.0/mprec.c, unchanged, and every
   function matches byte for byte except __d2b (one delay slot the later revision fills: `sll v1,s0,2` under the
   `beqz s3`), which stays in libc_mprec.c (__hi0bits matches with the `ins` hook since round 37). It is compiled here as
   well, weak and never linked (its section is not in the linker script), because GCC lays out a call differently once it has seen the callee's body. The `@gcc` lines
   of libc_mprec.c give the ranges; the tables (p05, __mprec_tens, __mprec_bigtens, __mprec_tinytens) are this
   object's .rodata. The PC port uses the C of libc_mprec.c. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define _multadd __multadd
#define _s2b __s2b
#define _hi0bits __hi0bits
#define _lo0bits __lo0bits
#define _i2b __i2b
#define _multiply __multiply
#define _pow5mult __pow5mult
#define _lshift __lshift
#define __mcmp libc_bigint_cmp
#define _ulp __ulp
#define _b2d __b2d
#define _d2b __d2b
#define _ratio __ratio

#pragma weak __hi0bits
#pragma weak __d2b

#include "newlib-1.13.0/mprec.c"
#endif
