// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio
// @bss 0x088F96B0-0x088F96B4
/* newlib 1.12.0 libc/stdio/findfp.c (std, __sfmoreglue, __sfp, _cleanup_r, _cleanup, __sinit), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdio/findfp.c, unchanged; the functions listed as `@gcc` ranges of libc_stdio.c match byte
   for byte. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __sclose libc_sclose
#define __sfmoreglue libc_sfmoreglue
#define __sfp libc_sfp
#define __sinit libc_sinit
#define __sread libc_sread
#define __sseek libc_sseek
#define __swrite libc_swrite
#define _cleanup libc_cleanup
#define _cleanup_r libc_cleanup_r
#define _fwalk libc_fwalk
#define _impure_ptr libc_global_reent
#define std libc_std

#include "newlib-1.12.0/libc/stdio/findfp.c"
#endif
