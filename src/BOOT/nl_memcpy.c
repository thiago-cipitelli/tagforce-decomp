// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include
/* newlib 1.12.0 libc/machine/mips/memcpy.c (memcpy, word copies when aligned, `ulw`/`usw` otherwise), compiled by
   the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"). The source is
   include/newlib-1.12.0/libc/machine/mips/memcpy.c, unchanged; tools/psp_gcc_peep.py (rule F) writes its `ulw`/`usw`
   as the lwl/lwr (swl/swr) pair of Sony's assembler. Its `@gcc` range in libc_mem.c matches byte for byte. The PC
   port uses the C kept in that file. */

#ifdef __mips__
#include "newlib-1.12.0/libc/machine/mips/memcpy.c"
#endif
