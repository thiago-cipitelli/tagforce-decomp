// @cc gcc
/* Functions of libc_misc.c (newlib of the Sony SDK libc) that come out of C byte for byte with the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); their ranges are the `@gcc` lines of libc_misc.c. */

int _read(int fd, void *buf, int n);
unsigned int _sbrk(int incr);
extern int libc_errno;                 /* 0x08E35A34 */

int _read_r(int *r, int fd, void *buf, int n)
{
    int ret;

    libc_errno = 0;
    ret = _read(fd, buf, n);
    if (ret == -1 && libc_errno != 0) {
        *r = libc_errno;
    }
    return ret;
}

void *_sbrk_r(int *r, int incr)
{
    int ret;

    libc_errno = 0;
    ret = (int)_sbrk(incr);
    if (ret == -1 && libc_errno != 0) {
        *r = libc_errno;
    }
    return (void *)ret;
}
