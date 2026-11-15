// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdlib
/* newlib 1.12.0 libc/stdlib/wcsrtombs.c (_wcsrtombs_r, wcsrtombs), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdlib/wcsrtombs.c, unchanged; the functions listed as `@gcc` ranges of wcrtomb.c match byte
   for byte. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current

#include "newlib-1.12.0/libc/stdlib/wcsrtombs.c"
#endif
