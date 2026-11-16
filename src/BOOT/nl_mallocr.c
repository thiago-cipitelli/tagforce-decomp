// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdlib -DINTERNAL_NEWLIB -DDEFINE_MALLOC -DMALLOC_ALIGNMENT=16
/* newlib 1.12.0 libc/stdlib/mallocr.c built as newlib's malloc object (-DINTERNAL_NEWLIB -DDEFINE_MALLOC: malloc_extend_top,
   _malloc_r and the allocator state), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"). The source is
   include/newlib-1.12.0/libc/stdlib/mallocr.c, unchanged; the SDK built it with 16-byte chunks (MALLOC_ALIGNMENT=16). Matches
   only with the later revision's `ins` (psp_gcc_insand). The state (__malloc_av_ ...) stays in the asm data: the object's
   definitions are weak (tools/psp_gcc.sh) and lose to the names in symbols/BOOT.txt. The PC port uses the C kept in libc_malloc.c. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define malloc_extend_top libc_malloc_extend_top

#include "newlib-1.12.0/libc/stdlib/mallocr.c"
#endif
