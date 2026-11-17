// @cc gcc -O3 -Iinclude/newlib-1.12.0/libc/include
/* The functions of vfpu_scalar.c that the SDK's GCC compiles to the original bytes (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"):
   the `@gcc` ranges of that file: C around one VFPU asm statement, the result through a stack float. The PC port
   compiles the portable C of vfpu_scalar.c. */

#ifdef __mips__
#define NON_MATCHING

#include "vfpu_scalar.c"
#endif
