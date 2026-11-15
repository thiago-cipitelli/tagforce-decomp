// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio
/* newlib 1.12.0 libc/stdio/stdio.c (__sclose), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdio/stdio.c, unchanged; the functions listed as `@gcc` ranges of libc_string.c match byte
   for byte. The others (__sread, __sseek, __swrite) need a later revision of that compiler or differ from the SDK's source: they stay in the mwcc
   file. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define __sclose libc_sclose

#include "newlib-1.12.0/libc/stdio/stdio.c"
#endif
