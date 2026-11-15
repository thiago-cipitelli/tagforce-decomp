// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/reent -fno-common
/* newlib 1.12.0 libc/reent/reent.c (cleanup_glue, _wrapup_reent), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/reent/reent.c, unchanged; the functions listed as `@gcc` ranges of libc_misc.c match byte
   for byte. The others (_reclaim_reent) need a later revision of that compiler or differ from the SDK's source: they stay in the mwcc
   file and are weak here. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define cleanup_glue libc_cleanup_glue

#pragma weak _reclaim_reent

#include "newlib-1.12.0/libc/reent/reent.c"
#endif
