// @cc gcc -O3 -fno-math-errno -Iinclude/newlib-1.12.0/libc/include
// @rodata 0x088EB120-0x088EB1B0
/* The functions of vfpu_math.c that the SDK's GCC compiles to the original bytes (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"):
   the `@gcc` ranges of that file. The C is the one in vfpu_math.c, which the PC port compiles directly; here it is
   included as the PSP sees it. The functions that do not match yet stay in the mwcc file (tools/psp_gcc.sh makes
   every definition of a GCC object weak, so theirs win). */

#ifdef __mips__
#define NON_MATCHING

#include "vfpu_math.c"
#endif
