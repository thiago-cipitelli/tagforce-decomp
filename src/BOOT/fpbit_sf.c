// @cc gcc
/* libgcc software floating point, the two single-precision objects that come out of GCC 3.3.3's fp-bit.c (built
   with FLOAT, as libgcc's fp-bit.c) byte for byte with the SDK's GCC: __make_fp (_make_sf.o) and __extendsfdf2
   (_sf_to_df.o); a `@gcc` range of fpbit.c (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"). __pack_f and
   __unpack_f are fpbit_pkf.c; fpbit.c keeps the C of the PC port. */

#ifdef __mips__
#define FINE_GRAINED_LIBRARIES
#define FLOAT
#define FLOAT_BIT_ORDER_MISMATCH       /* t-mips: little-endian bitfield order */
#define L_make_sf
#define L_sf_to_df
#include "gcc-3.3.3/fp-bit.c"
#endif
