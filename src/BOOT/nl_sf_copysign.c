// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libm/math -fno-builtin -Iinclude/newlib-1.12.0/libm/common
/* newlib 1.12.0 libm/common/sf_copysign.c (copysignf), written from the asm in newlib's text (the file is not in
   include/newlib-1.12.0), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"): it is the original byte
   for byte (`@gcc` range of libm.c). The PC port uses the C kept in libm.c. */

#ifdef __mips__
#define copysignf libm_copysignf
#include "fdlibm.h"

#ifdef __STDC__
	float copysignf(float x, float y)
#else
	float copysignf(x,y)
	float x,y;
#endif
{
	__uint32_t ix,iy;
	GET_FLOAT_WORD(ix,x);
	GET_FLOAT_WORD(iy,y);
	SET_FLOAT_WORD(x,(ix&0x7fffffff)|(iy&0x80000000));
        return x;
}

#endif
