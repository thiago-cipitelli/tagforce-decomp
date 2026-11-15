/* What the newlib libc sources of BOOT (libc_*.c with `// @cc gcc`; docs/build.md, "Trilha GCC") take from newlib
   1.13.0's _ansi.h, stddef.h, limits.h and string.h, plus the names those functions have in symbols/BOOT.txt.
   Private to src/BOOT. */
#ifndef TF_NEWLIB_H
#define TF_NEWLIB_H

/* _ansi.h (ANSI branch) */
#define _PTR void *
#define _AND ,
#define _NOARGS void
#define _CONST const
#define _VOLATILE volatile
#define _SIGNED signed
#define _DOTS , ...
#define _VOID void
#define _EXFUN(name, proto) name proto
#define _EXPARM(name, proto) (* name) proto
#define _DEFUN(name, arglist, args) name(args)
#define _DEFUN_VOID(name) name(_NOARGS)
#define _PARAMS(paramlist) paramlist

typedef __SIZE_TYPE__ size_t;
#ifndef NULL
#define NULL ((void *)0)
#endif
#define LONG_MAX 2147483647L
#define LONG_MIN (-LONG_MAX - 1L)
#define ULONG_MAX 0xFFFFFFFFUL

/* names in BOOT (historical: memcmp is newlib's strncmp there) */
#define strncmp memcmp
#define memcmp memcmp2
#define strchr libc_strchr
#define strrchr strrchr_str

_PTR memchr(_CONST _PTR, int, size_t);
int memcmp(_CONST _PTR, _CONST _PTR, size_t);
char *strcat(char *, _CONST char *);
char *strchr(_CONST char *, int);
char *strcpy(char *, _CONST char *);
size_t strlen(_CONST char *);
char *strrchr(_CONST char *, int);
char *strstr(_CONST char *, _CONST char *);

#endif
