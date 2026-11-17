// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdlib
/* newlib 1.12.0 libc/stdlib/rand.c (srand, rand = rand_next), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC").
   The source is include/newlib-1.12.0/libc/stdlib/rand.c, unchanged; the functions listed as `@gcc` ranges of libc_misc.c match byte
   for byte (rand with rule G of tools/psp_gcc_peep.py: the `nop` after the 64-bit store, rodada 38). The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define rand rand_next

#include "newlib-1.12.0/libc/stdlib/rand.c"
#endif
