// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libm/math -fno-builtin -Iinclude/newlib-1.12.0/libm/common
/* newlib 1.12.0 libm/math/s_isnan.c (isnan), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libm/math/s_isnan.c, unchanged; the functions listed as `@gcc` ranges of libm.c match byte
   for byte. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define isnan __isnand

#include "newlib-1.12.0/libm/math/s_isnan.c"
#endif
