// @cc gcc
/* newlib 1.13.0 libc/string/strstr.c.
   0x08811648-0x088116A4 of BOOT (the range is listed as `@gcc` in the file that covers it). Compiled by the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); the source below is upstream's, unchanged but for the headers. */

/*
FUNCTION
	<<strstr>>---find string segment

INDEX
	strstr

ANSI_SYNOPSIS
	#include <string.h>
	char *strstr(const char *<[s1]>, const char *<[s2]>);

TRAD_SYNOPSIS
	#include <string.h>
	char *strstr(<[s1]>, <[s2]>)
	char *<[s1]>;
	char *<[s2]>;

DESCRIPTION
	Locates the first occurence in the string pointed to by <[s1]> of
	the sequence of characters in the string pointed to by <[s2]>
	(excluding the terminating null  character).

RETURNS
	Returns a pointer to the located string segment, or a null
	pointer if the string <[s2]> is not found. If <[s2]> points to
	a string with zero length, the <[s1]> is returned.

PORTABILITY
<<strstr>> is ANSI C.

<<strstr>> requires no supporting OS subroutines.

QUICKREF
	strstr ansi pure
*/

#include "newlib.h"

char *
_DEFUN (strstr, (searchee, lookfor),
	_CONST char *searchee _AND
	_CONST char *lookfor)
{
  if (*searchee == 0)
    {
      if (*lookfor)
	return (char *) NULL;
      return (char *) searchee;
    }

  while (*searchee)
    {
      size_t i;
      i = 0;

      while (1)
	{
	  if (lookfor[i] == 0)
	    {
	      return (char *) searchee;
	    }

	  if (lookfor[i] != searchee[i])
	    {
	      break;
	    }
	  i++;
	}
      searchee++;
    }

  return (char *) NULL;
}
