// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio
/* newlib 1.12.0 libc/stdio/makebuf.c (__smakebuf), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdio/makebuf.c, unchanged; the functions listed as `@gcc` ranges of libc_malloc.c match byte
   for byte. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define __smakebuf libc_smakebuf
#define __sseek libc_sseek
#define _cleanup_r libc_cleanup_r
#define isatty _isatty

#include "newlib-1.12.0/libc/stdio/makebuf.c"
#endif
