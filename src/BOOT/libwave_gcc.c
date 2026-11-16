// @cc gcc -O3
// @rodata 0x088EAFC4-0x088EAFD0
/* The functions of libwave.c that the SDK's GCC compiles to the original bytes (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"):
   the `@gcc` ranges of that file. The C is the one in libwave.c, which the PC port compiles directly; here it is
   included as the PSP sees it. The functions that do not match yet stay in the mwcc file (tools/psp_gcc.sh makes
   every definition of a GCC object weak, so theirs win). The object's .rodata is the thread name of wave_init
   ("SceWaveMain"); the VAG coefficient table before it (0x088EAFB8) stays in the asm data, read through its symbol. */

#ifdef __mips__
#define NON_MATCHING

#include "libwave.c"
#endif
