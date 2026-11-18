// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdlib -fno-builtin -strsec
// @rodata 0x08873AF0-0x08873B70 .rodata.str1.4 .rodata.cst8 .rodata
/* newlib 1.12.0 libc/stdlib/dtoa.c (_dtoa_r), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha
   GCC"). The source is include/newlib-1.12.0/libc/stdlib/dtoa.c, unchanged; _dtoa_r matches byte for byte since
   rodada 43 (the later compiler revision emits the increment queue before the assignment of `++*s++`:
   tools/psp_gcc_insand.c). quorem stays the asm of libc_stdlib.c (libc_quorem: the registers of the 1.7.1 copy are
   one `t` off), so the static is made global under that name; psp_gcc.sh makes it weak and the asm wins. The
   strings go to .rodata.str1.4 (`-strsec`), linked with the .rodata.cst8 and the jump table in the original order.
   The PC port uses the C kept in libc_stdlib.c. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define static
#define quorem libc_quorem
#define __mcmp libc_bigint_cmp
#define _d2b __d2b
#define _hi0bits __hi0bits
#define _i2b __i2b
#define _lshift __lshift
#define _multadd __multadd
#define _multiply __multiply
#define _pow5mult __pow5mult

#include "newlib-1.12.0/libc/stdlib/dtoa.c"
#endif
