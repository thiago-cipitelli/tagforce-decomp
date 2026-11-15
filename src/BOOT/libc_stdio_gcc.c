// @cc gcc
/* Functions of libc_stdio.c (newlib of the Sony SDK libc) that come out of C byte for byte with the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); their ranges are the `@gcc` lines of libc_stdio.c. */

typedef struct LibcFile {              /* newlib FILE, 0x5C bytes */
    unsigned char *p;
    int r;
    int w;                             /* +0x08 */
    short flags;                       /* +0x0C */
    short file;                        /* +0x0E */
    unsigned char *base;               /* +0x10 _bf._base */
    int size;                          /* +0x14 _bf._size */
    int lbfsize;                       /* +0x18 */
    void *cookie;                      /* +0x1C */
    int (*read)(void *, char *, int);  /* +0x20 */
    int (*write)(void *, const char *, int); /* +0x24 */
    int (*seek)(void *, int, int);     /* +0x28 */
    int (*close)(void *);              /* +0x2C */
    char pad30[0x2C];
} LibcFile;

int fflush(LibcFile *fp);
int libc_fwalk(void *r, int (*fn)(LibcFile *));
void libc_std(LibcFile *fp, int flags, int file, void *r);
int libc_cleanup_r(void *r);
int _fstat(int fd, void *st);
extern int libc_errno;                 /* 0x08E35A34 */

int libc_cleanup_r(void *r)
{
    return libc_fwalk(r, fflush);
}

void libc_sinit(char *r)
{
    *(int (**)(void *))(r + 0x3C) = libc_cleanup_r;
    *(int *)(r + 0x38) = 1;
    *(int *)(r + 0x260) = 3;
    *(char **)(r + 0x264) = r + 0x268;
    *(int *)(r + 0x25C) = 0;
    libc_std(*(LibcFile **)(r + 4), 4, 0, r);
    libc_std(*(LibcFile **)(r + 8), 9, 1, r);
    libc_std(*(LibcFile **)(r + 0xC), 10, 2, r);
}

int _fstat_r(int *r, int fd, void *st)
{
    int ret;

    libc_errno = 0;
    ret = _fstat(fd, st);
    if (ret == -1 && libc_errno != 0) {
        *r = libc_errno;
    }
    return ret;
}
