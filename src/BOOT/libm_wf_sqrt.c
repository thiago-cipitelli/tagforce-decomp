// @cc gcc -fno-builtin
// @rodata 0x08873960-0x08873970
/* newlib 1.13.0 libm/math/wf_sqrt.c: sqrtf (libm_sqrtf in BOOT), the SVID error wrapper of __ieee754_sqrtf.
   0x0880A484-0x0880A5B4 of BOOT (the range is listed as `@gcc` in the file that covers it). Compiled by the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); the source below is upstream's, unchanged but for the headers. */

/* wf_sqrt.c -- float version of w_sqrt.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/* 
 * wrapper sqrtf(x)
 */

#include "fdlibm.h"

#ifdef __STDC__
	float sqrtf(float x)		/* wrapper sqrtf */
#else
	float sqrtf(x)			/* wrapper sqrtf */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_sqrtf(x);
#else
	float z;
	struct exception exc;
	z = __ieee754_sqrtf(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(x)) return z;
	if(x<(float)0.0) {
            /* sqrtf(negative) */
            exc.type = DOMAIN;
            exc.name = "sqrtf";
	    exc.err = 0;
	    exc.arg1 = exc.arg2 = (double)x;
            if (_LIB_VERSION == _SVID_)
              exc.retval = 0.0;
            else
              exc.retval = 0.0/0.0;
            if (_LIB_VERSION == _POSIX_) 
              errno = EDOM;
            else if (!matherr(&exc)) {
              errno = EDOM;
            }
            if (exc.err != 0)
	      errno = exc.err;
	    return (float)exc.retval; 
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double sqrt(double x)
#else
	double sqrt(x)
	double x;
#endif
{
	return (double) sqrtf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
