// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdio -fno-builtin -DWANT_PRINTF_LONG_LONG
// @rodata 0x08874028-0x08874470
/* newlib 1.12.0 libc/stdio/vfscanf.c (vfscanf, __svfscanf, _vfscanf_r, __sccl, __svfscanf_r), compiled by the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"). The source is include/newlib-1.12.0/libc/stdio/vfscanf.c, unchanged,
   with the SDK's configuration (%ll: WANT_PRINTF_LONG_LONG, memset calls: -fno-builtin; the same as nl_vfprintf.c), and
   every function matches byte for byte: __svfscanf_r since round 41 (`*p++ = *fp->_p++` stores fp->_p before the byte:
   the queued-increment change of tools/psp_gcc_insand.c). The .rodata (jump tables and strings) is this object's. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define __sccl nl_sccl
#define __sinit libc_sinit
#define __srefill libc_srefill
#define iswspace libc_isspace
#define __mb_cur_max D_088EB5A0


#include "newlib-1.12.0/libc/stdio/vfscanf.c"
#endif
