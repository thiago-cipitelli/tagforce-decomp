// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio
/* newlib 1.12.0 libc/stdio/refill.c (lflush), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdio/refill.c, unchanged; the functions listed as `@gcc` ranges of libc_misc.c match byte
   for byte. The others (__srefill) need a later revision of that compiler or differ from the SDK's source: they stay in the mwcc
   file. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define lflush libc_lflush

#include "newlib-1.12.0/libc/stdio/refill.c"
#endif
