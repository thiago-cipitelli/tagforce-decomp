// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio
/* newlib 1.12.0 libc/stdio/fvwrite.c (__sfvwrite), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdio/fvwrite.c, unchanged; the functions listed as `@gcc` ranges of libc_stdio.c match byte
   for byte. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define __sfvwrite libc_sfvwrite
#define __swsetup nl_swsetup
#define memmove mem_move

#include "newlib-1.12.0/libc/stdio/fvwrite.c"
#endif
