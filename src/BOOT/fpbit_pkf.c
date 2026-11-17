// @cc gcc
/* libgcc software floating point, __pack_f (_pack_sf.o) and __unpack_f (_unpack_sf.o): GCC 3.3.3's fp-bit.c built
   with FLOAT, compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); a `@gcc` range of fpbit.c.
   Its own translation unit: __make_fp (fpbit_sf.c) calls __pack_f differently once GCC has seen its body. */

#ifdef __mips__
#define FINE_GRAINED_LIBRARIES
#define FLOAT
#define FLOAT_BIT_ORDER_MISMATCH       /* t-mips: little-endian bitfield order */
#define L_pack_sf
#define L_unpack_sf
#include "gcc-3.3.3/fp-bit.c"
#endif
