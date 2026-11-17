// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio
/* newlib 1.12.0 libc/stdio/stdio.c (__sread, __swrite, __sclose), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdio/stdio.c, unchanged; the functions listed as `@gcc` ranges of libc_string.c match byte
   for byte. __sread and __swrite match since the peephole of round 35 (tools/psp_gcc_peep.py). __sseek matches with the `ins` hook of tools/psp_gcc.sh (round 37). The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __sread libc_sread
#define __swrite libc_swrite
#define __getreent heap_get_current
#define __sclose libc_sclose
#define __sseek libc_sseek

#include "newlib-1.12.0/libc/stdio/stdio.c"
#endif
