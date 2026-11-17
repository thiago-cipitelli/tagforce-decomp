// @cc gcc
/* strcmp of the Sony SDK libc: newlib 1.12.0's libc/machine/mips/strcmp.c (two bytes per iteration when both strings
   are 16-bit aligned), written from the asm in newlib's text; compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md,
   "Trilha GCC") it is the original byte for byte. Its `@gcc` range is in libc_string.c, whose plain C the PC port uses. */

#ifdef __mips__
int
strcmp (const char *s1, const char *s2)
{
  unsigned const char *us1 = (unsigned const char *)s1;
  unsigned const char *us2 = (unsigned const char *)s2;
  int c1a, c1b;
  int c2a, c2b;

  if (((((long)us1) | ((long)us2)) & 1) == 0)
    {
      c1a = *us1;
      for (;;)
	{
	  c1b = *us2;
	  us1 += 2;
	  if (c1a == '\0')
	    goto ret1;

	  c2a = us1[-1];
	  if (c1a != c1b)
	    goto ret1;

	  c2b = us2[1];
	  us2 += 2;
	  if (c2a == '\0')
	    break;

	  c1a = *us1;
	  if (c2a != c2b)
	    break;
	}

      return c2a - c2b;
    }
  else
    {
      do
	{
	  c1a = *us1++;
	  c1b = *us2++;
	}
      while (c1a != '\0' && c1a == c1b);
    }

 ret1:
  return c1a - c1b;
}
#endif
