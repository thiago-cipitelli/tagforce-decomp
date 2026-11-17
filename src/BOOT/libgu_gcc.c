// @cc gcc -O3
// @rodata 0x08872EF0-0x08872F78 .rodata
// @rodata 0x088EB110-0x088EB114 .rodata.cst4
/* The functions of libgu.c that the SDK's GCC compiles to the original bytes (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"):
   the `@gcc` ranges of that file. The C is the one in libgu.c, which the PC port compiles directly; here it is
   included as the PSP sees it. The functions that do not match yet stay in the mwcc file (tools/psp_gcc.sh makes
   every definition of a GCC object weak, so theirs win). The object's .rodata (0x08872EF0: gu_list_arg, the event
   flag name, the dither matrix, the jump tables of sceGuFinishId and sceGuSync) and its .rodata.cst4 (the 1.0f of
   sceGuStart) are linked here. */

#ifdef __mips__
#define NON_MATCHING

#include "libgu.c"
#endif
