// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdlib
// @rodata 0x08873BBC-0x08873BC0
/* newlib 1.12.0 libc/stdlib/mbrtowc.c (_mbrtowc_r, mbrtowc), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdlib/mbrtowc.c, unchanged; the functions listed as `@gcc` ranges of libc_malloc.c match byte
   for byte. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current

#include "newlib-1.12.0/libc/stdlib/mbrtowc.c"
#endif
