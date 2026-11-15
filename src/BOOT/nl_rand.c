// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdlib
/* newlib 1.12.0 libc/stdlib/rand.c (srand), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdlib/rand.c, unchanged; the functions listed as `@gcc` ranges of libc_misc.c match byte
   for byte. The others (rand) need a later revision of that compiler or differ from the SDK's source: they stay in the mwcc
   file and are weak here. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current

#pragma weak rand

#include "newlib-1.12.0/libc/stdlib/rand.c"
#endif
