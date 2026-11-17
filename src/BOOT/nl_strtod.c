// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdlib -fno-builtin
// @rodata 0x08873D50-0x08873E48
/* newlib 1.12.0 libc/stdlib/strtod.c (_strtod_r, strtod, strtof), compiled by the SDK's GCC (tools/psp_gcc.sh;
   docs/build.md, "Trilha GCC"). The source is include/newlib-1.12.0/libc/stdlib/strtod.c, unchanged; _strtod_r
   matches byte for byte since rodada 39 (the later compiler revision hoists more loop invariants: n_non_fixed_regs in
   tools/psp_gcc_insand.c). Its `@gcc` range in libc_string.c runs to 0x088126F8 (the last block of the function,
   once taken for a separate cold block). strtod and strtof stay in strtol.c. The object's .rodata.cst8 (0x08873D50)
   and .rodata (the jump table, 0x08873D90) are linked in place. The PC port uses the C kept in libc_string.c. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define __mcmp libc_bigint_cmp
#define _d2b __d2b
#define _i2b __i2b
#define _lshift __lshift
#define _multiply __multiply
#define _pow5mult __pow5mult
#define _ratio __ratio
#define _s2b __s2b
#define _ulp __ulp

#include "newlib-1.12.0/libc/stdlib/strtod.c"
#endif
