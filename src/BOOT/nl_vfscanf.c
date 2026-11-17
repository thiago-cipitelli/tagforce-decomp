// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio -fno-builtin -DWANT_PRINTF_LONG_LONG
/* newlib 1.12.0 libc/stdio/vfscanf.c (vfscanf, __svfscanf, _vfscanf_r, __sccl), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdio/vfscanf.c, unchanged; the functions listed as `@gcc` ranges of vfscanf.c match byte
   for byte. __svfscanf_r stays in the mwcc file (weak here): with
   the SDK's configuration (%ll: WANT_PRINTF_LONG_LONG, memset calls: -fno-builtin; the same as nl_vfprintf.c) it has the
   original's calls and is 4 words longer, the rest is register allocation (the -1 compared with _mbrtowc_r's result in v0
   instead of a0, the family of _strtod_r) and scheduling; rodada 38 in docs/build.md. The PC port uses the C kept in that file. */

/* This object's .rodata (jump tables and strings of the function that stays in asm) is not linked: no @rodata. */
#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define __sccl nl_sccl
#define __sinit libc_sinit

#pragma weak __svfscanf_r

#include "newlib-1.12.0/libc/stdio/vfscanf.c"
#endif
