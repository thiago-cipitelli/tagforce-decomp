// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio -fno-builtin -DWANT_PRINTF_LONG_LONG -fshort-enums
// @rodata 0x08873E48-0x08874028
/* newlib 1.12.0 libc/stdio/vfprintf.c (__sprint, __sbprintf, vfprintf, _vfprintf_r, cvt, exponent), compiled by the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"). The source is include/newlib-1.12.0/libc/stdio/vfprintf.c, unchanged; the
   functions listed as `@gcc` ranges of vfprintf.c match byte for byte, and the object's .rodata (the jump table of the
   conversion switch and the strings) is linked in place. Flags: the SDK's newlib has %ll/%q (WANT_PRINTF_LONG_LONG: calls
   to __udivdi3/__umoddi3), calls memset for the mbstate_t (-fno-builtin), and switches on the `base` enum through a signed
   int index (-fshort-enums gives that here; rodada 38 in docs/build.md). The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __sbprintf nl_sbprintf
#define __getreent heap_get_current
#define __sfvwrite libc_sfvwrite
#define __sinit libc_sinit
#define __sprint nl_sprint
#define __swsetup nl_swsetup
#define isinf __isinfd
#define isnan __isnand
#define cvt nl_cvt
#define exponent nl_exponent


#include "newlib-1.12.0/libc/stdio/vfprintf.c"
#endif
