// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libm/math -fno-builtin -Iinclude/newlib-1.12.0/libm/common
/* newlib 1.12.0 libm/common/sf_isnan.c (isnanf), written from the asm in newlib's text (the file is not in
   include/newlib-1.12.0), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"): it is the original byte
   for byte (`@gcc` range of libm.c). The PC port uses the C kept in libm.c. */

#ifdef __mips__
#define isnanf libm_isnanf
#include "fdlibm.h"

#ifdef __STDC__
	int isnanf(float x)
#else
	int isnanf(x)
	float x;
#endif
{
	__int32_t ix;
	GET_FLOAT_WORD(ix,x);
	ix &= 0x7fffffff;
	return FLT_UWORD_IS_NAN(ix);
}
#endif
