/* shim, see _ansi.h: sys/config.h and machine/ieeefp.h for the PSP (little-endian MIPS, 32-bit int) */
#ifndef TF_SYS_CONFIG_H
#define TF_SYS_CONFIG_H
#define __IEEE_LITTLE_ENDIAN
typedef int __int32_t;
typedef unsigned int __uint32_t;
#endif
