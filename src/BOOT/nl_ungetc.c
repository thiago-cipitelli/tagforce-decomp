// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio
/* newlib 1.12.0 libc/stdio/ungetc.c (__submore), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdio/ungetc.c, unchanged; the functions listed as `@gcc` ranges of ungetc.c match byte
   for byte. The others (ungetc) need a later revision of that compiler or differ from the SDK's source: they stay in the mwcc
   file and are weak here. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define __submore nl_submore

#pragma weak ungetc

#include "newlib-1.12.0/libc/stdio/ungetc.c"
#endif
