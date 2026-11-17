// @cc gcc
/* libgcc software floating point, __pack_d (_pack_df.o) and __unpack_d (_unpack_df.o): GCC 3.3.3's fp-bit.c built
   as dp-bit.c, compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); a `@gcc` range of
   fpbit.c. Its own translation unit, like fpbit_pkf.c. */

#ifdef __mips__
#define FINE_GRAINED_LIBRARIES
#define FLOAT_BIT_ORDER_MISMATCH       /* t-mips: little-endian bitfield order */
#define L_pack_df
#define L_unpack_df
#include "gcc-3.3.3/fp-bit.c"
#endif
