// @cc gcc -D__DYNAMIC_REENT__ -Iinclude/newlib-1.12.0/libc/include -Iinclude/newlib-1.12.0/libc/stdlib -falign-loops=64
/* newlib 1.12.0 libc/stdlib/exit.c as the Sony SDK modified it, compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md,
   "Trilha GCC"): the atexit list is the one of the calling thread's reent (libc_get_thread_module, the libc_reent.c object)
   instead of _GLOBAL_REENT, and there is no __cleanup call; the rest is upstream's text. Built with aligned loops like the
   libc_reent.c object (-falign-loops=64: the `nop`s before the inner loop). Upstream's exit is compiled under another name,
   weak and never linked. exit (0x0880BF40-0x0880C040, with the padding the aligned section brings) is the `@gcc` range of libc_stdlib.c. The PC port uses the C kept
   in that file. */

#ifdef __mips__
#define exit nl_exit_upstream
#include "newlib-1.12.0/libc/stdlib/exit.c"
#undef exit
struct _reent *libc_get_thread_module _PARAMS ((void));

void
_DEFUN (exit, (code),
	int code)
{
  register struct _atexit *p;
  register struct _on_exit_args * args;
  register int n;
  int i;

  p = libc_get_thread_module ()->_atexit;
  while (p)
    {
      args = & p->_on_exit_args;
  
      for (n = p->_ind - 1, i = (n >= 0) ? (1 << n) : 0; n >= 0; --n, i >>= 1)
        if (args->_fntypes & i)
          (*((void (*)(int, void *)) p->_fns[n]))(code, args->_fnargs[n]);
        else
          p->_fns[n] ();

      p = p->_next;
    }

  _exit (code);
}
#endif
