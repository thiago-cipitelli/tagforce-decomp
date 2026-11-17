// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libm/math -fno-builtin -Iinclude/newlib-1.12.0/libm/common
// @rodata 0x08873948-0x08873954
/* newlib 1.12.0 libm/math/wf_asin.c (asinf, the SVID wrapper; libm_asinf), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libm/math/wf_asin.c, unchanged; the functions listed as `@gcc` ranges of libm.c match byte
   for byte. The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __fdlib_version D_08873880     /* _LIB_VERSION, in the data of the asm */
#define asinf libm_asinf
#define atanf libm_atanf
#define isnanf libm_isnanf
#define copysignf libm_copysignf
#define fabsf libm_fabsf
#define scalbnf libm_scalbnf
#include "newlib-1.12.0/libm/math/wf_asin.c"
#endif
