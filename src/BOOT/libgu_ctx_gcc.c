// @cc gcc -O3
// @rodata 0x08872FD0-0x08872FEC
/* The functions of libgu_ctx.c that the SDK's GCC compiles to the original bytes (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"):
   the `@gcc` ranges of that file. The C is the one in libgu_ctx.c, which the PC port compiles directly; here it is
   included as the PSP sees it. The functions that do not match yet stay in the mwcc file (tools/psp_gcc.sh makes
   every definition of a GCC object weak, so theirs win). The object's .rodata is the jump table of gu_ctx_light_color;
   the one of gu_ctx_state before it stays in the asm data (gu_ctx_state is not compiled here, see libgu_ctx.c). */

#ifdef __mips__
#define NON_MATCHING

#include "libgu_ctx.c"
#endif
