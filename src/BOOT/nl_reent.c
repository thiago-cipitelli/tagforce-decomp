// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/reent -fno-common
/* newlib 1.12.0 libc/reent/reent.c as the Sony SDK modified it (cleanup_glue, a stream-buffer helper, _reclaim_reent,
   _wrapup_reent), compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"). cleanup_glue and _wrapup_reent
   are include/newlib-1.12.0/libc/reent/reent.c, unchanged. Sony's object also has libc_free_stream_bufs (frees the __SMBF
   buffers of the reent's streams, the _fwalk loop) and a _reclaim_reent that first gives the dtoa/mprec Bigints back
   (_result, the _p5s chain, through _Bfree) and calls the helper after __cleanup; both are written below in upstream's
   style (rodada 38; upstream's _reclaim_reent is compiled under another name, weak and never linked). All four match byte
   for byte (the `@gcc` ranges of libc_misc.c). The PC port uses the C kept in that file. */

#ifdef __mips__
/* names in symbols/BOOT.txt */
#define __getreent heap_get_current
#define cleanup_glue libc_cleanup_glue
#define _impure_ptr libc_global_reent
#define _reclaim_reent nl_reclaim_reent_upstream

#include "newlib-1.12.0/libc/reent/reent.c"

#undef _reclaim_reent
#include <stdio.h>
#include "newlib-1.12.0/libc/stdio/local.h"

void _Bfree _PARAMS ((struct _reent *, struct _Bigint *));

/* Sony: the malloc'ed buffers (__SMBF) of ptr's streams */
static void
_DEFUN (libc_free_stream_bufs, (ptr),
     struct _reent *ptr)
{
  register FILE *fp;
  register int n;
  register struct _glue *g;

  for (g = &ptr->__sglue; g != NULL; g = g->_next)
    for (fp = g->_iobs, n = g->_niobs; --n >= 0; fp++)
      if (fp->_flags & __SMBF)
	if (fp->_bf._base)
	  {
	    _free_r (ptr, fp->_bf._base);
	    fp->_bf._base = NULL;
	  }
}

void
_DEFUN (_reclaim_reent, (ptr),
     struct _reent *ptr)
{
  if (ptr != _impure_ptr)
    {
      if (ptr->_result)
	{
	  ptr->_result->_k = ptr->_result_k;
	  ptr->_result->_maxwds = 1 << ptr->_result_k;
	  _Bfree (ptr, ptr->_result);
	}
      {
	struct _Bigint *p5, *next;
	for (p5 = ptr->_p5s; p5; p5 = next)
	  {
	    next = p5->_next;
	    _Bfree (ptr, p5);
	  }
      }
      /* used by mprec routines. */
      if (_REENT_MP_FREELIST(ptr))
	{
	  int i;
	  for (i = 0; i < 15 /* _Kmax */; i++) 
	    {
	      struct _Bigint *thisone, *nextone;
	
	      nextone = _REENT_MP_FREELIST(ptr)[i];
	      while (nextone)
		{
		  thisone = nextone;
		  nextone = nextone->_next;
		  _free_r (ptr, thisone);
		}
	    }    

	  _free_r (ptr, _REENT_MP_FREELIST(ptr));
	}
      /* atexit stuff */
      if ((ptr->_atexit) && (ptr->_atexit != &ptr->_atexit0))
	{
	  struct _atexit *p, *q;
	  for (p = ptr->_atexit; p != &ptr->_atexit0;)
	    {
	      q = p;
	      p = p->_next;
	      _free_r (ptr, q);
	    }
	}

      if (ptr->_cvtbuf)
	_free_r (ptr, ptr->_cvtbuf);

      if (ptr->__sdidinit)
	{
	  /* cleanup won't reclaim memory 'coz usually it's run
	     before the program exits, and who wants to wait for that? */
	  ptr->__cleanup (ptr);
	  libc_free_stream_bufs (ptr);

	  if (ptr->__sglue._next)
	    cleanup_glue (ptr, ptr->__sglue._next);
	}
    }
}
#endif
