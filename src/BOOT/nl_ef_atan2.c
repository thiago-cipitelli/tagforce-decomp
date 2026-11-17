// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libm/math -fno-builtin -Iinclude/newlib-1.12.0/libm/common
// @rodata 0x0887336C-0x088733B0
/* newlib 1.12.0 libm/math/ef_atan2.c (__ieee754_atan2f), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libm/math/ef_atan2.c, unchanged; the functions listed as `@gcc` ranges of libm.c match byte
   for byte. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define atanf libm_atanf
#define copysignf libm_copysignf
#define fabsf libm_fabsf
#define scalbnf libm_scalbnf
#include "newlib-1.12.0/libm/math/ef_atan2.c"
#endif
