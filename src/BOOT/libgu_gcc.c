// @cc gcc -O3
// @rodata 0x088EB110-0x088EB114 .rodata.cst4
/* The functions of libgu.c that the SDK's GCC compiles to the original bytes (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"):
   the `@gcc` ranges of that file. The C is the one in libgu.c, which the PC port compiles directly; here it is
   included as the PSP sees it. The functions that do not match yet stay in the mwcc file (tools/psp_gcc.sh makes
   every definition of a GCC object weak, so theirs win). The object's .rodata.cst4 is the 1.0f of sceGuStart; its
   .rodata (0x08872EF0) stays in the asm data, read through its symbols (GU_RODATA in libgu.c), because the jump table
   of sceGuFinishId, which does not match, is in it. */

#ifdef __mips__
#define NON_MATCHING

#include "libgu.c"
#endif
