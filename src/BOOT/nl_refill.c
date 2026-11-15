// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio
/* newlib 1.12.0 libc/stdio/refill.c (lflush, __srefill), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdio/refill.c, unchanged; the functions listed as `@gcc` ranges of libc_misc.c match byte
   for byte. __srefill matches since the peephole of round 35
   (tools/psp_gcc_peep.py). The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define __sinit libc_sinit
#define __smakebuf libc_smakebuf
#define __srefill libc_srefill
#define _fwalk libc_fwalk
#define _impure_ptr libc_global_reent
#define lflush libc_lflush

#include "newlib-1.12.0/libc/stdio/refill.c"
#endif
