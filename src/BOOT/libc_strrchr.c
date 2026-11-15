// @cc gcc
/* newlib 1.13.0 libc/string/strrchr.c (strrchr_str in symbols/BOOT.txt).
   0x088115E0-0x08811648 of BOOT (the range is listed as `@gcc` in the file that covers it). Compiled by the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); the source below is upstream's, unchanged but for the headers. */

/*
FUNCTION
	<<strrchr>>---reverse search for character in string

INDEX
	strrchr

ANSI_SYNOPSIS
	#include <string.h>
	char * strrchr(const char *<[string]>, int <[c]>);

TRAD_SYNOPSIS
	#include <string.h>
	char * strrchr(<[string]>, <[c]>);
	char *<[string]>;
	int *<[c]>;

DESCRIPTION
	This function finds the last occurence of <[c]> (converted to
	a char) in the string pointed to by <[string]> (including the
	terminating null character).

RETURNS
	Returns a pointer to the located character, or a null pointer
	if <[c]> does not occur in <[string]>.

PORTABILITY
<<strrchr>> is ANSI C.

<<strrchr>> requires no supporting OS subroutines.

QUICKREF
	strrchr ansi pure
*/

#include "newlib.h"

char *
_DEFUN (strrchr, (s, i),
	_CONST char *s _AND
	int i)
{
  _CONST char *last = NULL;

  if (i)
    {
      while (s=strchr(s, i))
	{
	  last = s;
	  s++;
	}
    }
  else
    {
      last = strchr(s, i);
    }
		  
  return (char *) last;
}
