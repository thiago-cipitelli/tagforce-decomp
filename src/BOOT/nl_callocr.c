// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdlib -DINTERNAL_NEWLIB -DDEFINE_CALLOC -DMALLOC_ALIGNMENT=16
/* newlib 1.12.0 libc/stdlib/mallocr.c built as newlib's calloc object (-DINTERNAL_NEWLIB -DDEFINE_CALLOC: _calloc_r),
   compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"). Same scheme as nl_mallocr.c:
   the source is include/newlib-1.12.0/libc/stdlib/mallocr.c, unchanged, with 16-byte chunks; matches only with the later
   revision's `ins` (psp_gcc_insand). The allocator state stays in the asm data (weak here, the names in symbols/BOOT.txt
   win). The PC port uses the C kept in libc_misc.c. */

#ifdef __mips__
#include "newlib-1.12.0/libc/stdlib/mallocr.c"
#endif
