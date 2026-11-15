// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/string
/* newlib 1.12.0 libc/string/strncmp.c (strncmp), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/string/strncmp.c, unchanged; the functions listed as `@gcc` ranges of libc_string.c match byte
   for byte. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define strncmp memcmp

#include "newlib-1.12.0/libc/string/strncmp.c"
#endif
