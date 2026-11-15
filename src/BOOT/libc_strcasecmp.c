// @cc gcc
/* newlib 1.13.0 libc/string/strcasecmp.c: strcasecmp (stricmp in symbols/BOOT.txt), 0x08810FD0-0x0881106C of BOOT
   (a `@gcc` range of libc_string.c). Compiled by the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); the
   function is upstream's, with the two <ctype.h> macros it uses written out. */

#ifdef __mips__
/* newlib <ctype.h>: (_ctype_ + 1)[c] & _U; libc_ctype is _ctype_ + 1 */
extern const char libc_ctype[];
#define isupper(c) (libc_ctype[(unsigned)(c)] & 01)
#else
/* PC port: the same class without the PSP table */
#define isupper(c) ((c) >= 'A' && (c) <= 'Z')
#endif
#define tolower(c) \
	__extension__ ({ int __x = (c); isupper(__x) ? (__x - 'A' + 'a') : __x;})

int
stricmp (const char *s1, const char *s2)
{
  while (*s1 != '\0' && tolower(*s1) == tolower(*s2))
    {
      s1++;
      s2++;
    }

  return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}
