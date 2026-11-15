// @cc gcc
/* Functions of libc_malloc.c (newlib of the Sony SDK libc) that come out of C byte for byte with the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); their ranges are the `@gcc` lines of libc_malloc.c. */

extern int libc_charset;               /* 0x088EB5A4 */
extern char libc_lconv[];              /* 0x08873B7C */
int _lseek(int fd, int offset, int whence);
extern int libc_errno;                 /* 0x08E35A34 */

int libc_locale_charset(void)
{
    return libc_charset;
}

void *_localeconv_r(void *r)
{
    return libc_lconv;
}

int _lseek_r(int *r, int fd, int offset, int whence)
{
    int ret;

    libc_errno = 0;
    ret = _lseek(fd, offset, whence);
    if (ret == -1 && libc_errno != 0) {
        *r = libc_errno;
    }
    return ret;
}
