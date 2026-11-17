// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio -fno-builtin
/* newlib 1.12.0 libc/stdio/fread.c, compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdio/fread.c, unchanged. The PC port uses the C kept in libc_stdio.c. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define __srefill libc_srefill

#include "newlib-1.12.0/libc/stdio/fread.c"
#endif
