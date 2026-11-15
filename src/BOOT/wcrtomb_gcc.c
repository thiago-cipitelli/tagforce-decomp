// @cc gcc
/* Functions of wcrtomb.c (newlib of the Sony SDK libc) that come out of C byte for byte with the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); their ranges are the `@gcc` lines of wcrtomb.c. */

#include "nl_stdio.h"
extern int libc_errno;                 /* 0x08E35A34: errno, set by the system call wrappers */
int _write(int fd, const void *buf, int n);

int _write_r(NlReent *ptr, int fd, const void *buf, unsigned int cnt)
{
    int ret;

    libc_errno = 0;
    if ((ret = _write(fd, buf, cnt)) == -1 && libc_errno != 0) {
        ptr->err = libc_errno;
    }
    return ret;
}
