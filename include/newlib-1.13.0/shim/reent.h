/* shim, see _ansi.h: the part of newlib's sys/reent.h that mprec.c uses. The SDK's struct _reent is the full
   (not _REENT_SMALL) layout: _p5s at +0x48 and _freelist at +0x4C. */
#ifndef TF_REENT_H
#define TF_REENT_H
#include "_ansi.h"
typedef unsigned long __ULong;
typedef long __Long;
struct _Bigint
{
  struct _Bigint *_next;
  int _k, _maxwds, _sign, _wds;
  __ULong _x[1];
};
struct _reent
{
  int _unused_00[18];                  /* +0x00.._errno, stdio, locale, result */
  struct _Bigint *_p5s;                /* +0x48 */
  struct _Bigint **_freelist;          /* +0x4C */
};
#define _REENT_CHECK_MP(ptr)           /* nothing */
#define _REENT_MP_FREELIST(ptr) ((ptr)->_freelist)
#define _REENT_MP_P5S(ptr) ((ptr)->_p5s)
_PTR _calloc_r(struct _reent *, size_t, size_t);
#endif
