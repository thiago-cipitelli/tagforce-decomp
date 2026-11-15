// @cc gcc -O3
/* The functions of libgu_ctx.c that the SDK's GCC compiles to the original bytes (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"):
   the `@gcc` ranges of that file. The C is the one in libgu_ctx.c, which the PC port compiles directly; here it is
   included as the PSP sees it. The functions that do not match yet stay in the mwcc file (tools/psp_gcc.sh makes
   every definition of a GCC object weak, so theirs win). */

#ifdef __mips__
#define NON_MATCHING

#include "libgu_ctx.c"
#endif
