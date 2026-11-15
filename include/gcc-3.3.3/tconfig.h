/* Target configuration seen by libgcc2.c and fp-bit.c (what GCC's build generates as tconfig.h/tm.h), reduced to
   the macros those two files read, with the values of the PSP target (little-endian MIPS EABI, 32-bit words). The
   other files of this directory are GCC 3.3.3's, unchanged; src/BOOT/libgcc.c and fpbit.c include them. */
#ifndef TF_TCONFIG_H
#define TF_TCONFIG_H
#define BITS_PER_UNIT 8
#define UNITS_PER_WORD 4
#define MIN_UNITS_PER_WORD 4
#define WORDS_BIG_ENDIAN 0
#define LONG_TYPE_SIZE 32
#define LONG_LONG_TYPE_SIZE 64
#define LONG_DOUBLE_TYPE_SIZE 64
#define GCOV_TYPE_SIZE 64
#define ROUND_TOWARDS_ZERO 0
#define inhibit_libc
#endif
