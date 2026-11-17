// @cc gcc
/* memset of the Sony SDK libc: newlib 1.12.0's libc/machine/mips/memset.c (word-aligns the start, fills two words per
   iteration with the byte replicated, then one word and the remaining bytes), written from the asm in newlib's style;
   compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC") it is the original byte for byte. Its `@gcc`
   range is in libc_mem.c, whose plain C the PC port uses. */
#include "libc.h"

#ifdef __mips__
#define wordtype long
#define LBLOCKSIZE (sizeof(wordtype))
#define UNALIGNED(X)   ((long)(X) & (LBLOCKSIZE - 1))
#define TOO_SMALL(LEN) ((LEN) < 4*LBLOCKSIZE)

void *memset(void *m, int c, size_t n)
{
  char *s = (char *) m;
  unsigned wordtype buffer;
  unsigned wordtype *aligned_addr;
  size_t iter;

  if (!TOO_SMALL (n))
    {
      int unaligned = UNALIGNED (s);

      if (unaligned)
	{
	  while (unaligned++ < LBLOCKSIZE)
	    *s++ = (char)c, n--;
	}

      aligned_addr = (unsigned wordtype *)s;

      c &= 0xff;
      buffer = c;
      if (buffer != 0)
	{
	  buffer |= (buffer << 8);
	  buffer |= (buffer << 16);
	}

      iter = n / (2*LBLOCKSIZE);
      n = n % (2*LBLOCKSIZE);
      while (iter > 0)
	{
	  aligned_addr[0] = buffer;
	  aligned_addr[1] = buffer;
	  aligned_addr += 2;
	  iter--;
	}

      if (n >= LBLOCKSIZE)
	{
	  *aligned_addr++ = buffer;
	  n -= LBLOCKSIZE;
	}

      s = (char*)aligned_addr;
    }

  while (n > 0)
    {
      *s++ = (char)c;
      n--;
    }

  return m;
}
#endif
