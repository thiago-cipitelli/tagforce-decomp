// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio
/* newlib 1.12.0 libc/stdio/vfscanf.c (vfscanf, __svfscanf, _vfscanf_r, __sccl), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdio/vfscanf.c, unchanged; the functions listed as `@gcc` ranges of vfscanf.c match byte
   for byte. The others (__svfscanf_r) need a later revision of that compiler or differ from the SDK's source: they stay in the mwcc
   file and are weak here. The PC port uses the C kept in that file. */

/* This object's .rodata (jump tables and strings of the function that stays in asm) is not linked: no @rodata. */
#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define __sccl nl_sccl
#define __sinit libc_sinit

#pragma weak __svfscanf_r

#include "newlib-1.12.0/libc/stdio/vfscanf.c"
#endif
