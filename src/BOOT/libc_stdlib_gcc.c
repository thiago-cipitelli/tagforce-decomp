// @cc gcc
/* Functions of libc_stdlib.c (newlib of the Sony SDK libc) that come out of C byte for byte with the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); their ranges are the `@gcc` lines of libc_stdlib.c. */

typedef struct LibcAtexit {            /* newlib struct _atexit */
    struct LibcAtexit *next;
    int ind;
    void (*fns[32])(void);
    void *fnargs[32];                  /* +0x88 */
    unsigned int fntypes;              /* +0x108 bit i: fns[i] takes (status, arg) */
} LibcAtexit;

void *libc_get_thread_module(void);    /* the calling thread's struct _reent */
void *_malloc_r(void *r, unsigned int n);
int _close(int fd);
int sceKernelCpuSuspendIntr(void);
void sceKernelCpuResumeIntr(int flags);
extern int libc_errno;                 /* 0x08E35A34 */

int atexit(void (*fn)(void))
{
    int flags = sceKernelCpuSuspendIntr();
    char *r = libc_get_thread_module();
    LibcAtexit *p = *(LibcAtexit **)(r + 0x148);

    if (p == 0) {
        p = (LibcAtexit *)(r + 0x14C);
        *(LibcAtexit **)(r + 0x148) = p;
    }
    if (p->ind >= 32) {
        sceKernelCpuResumeIntr(flags);
        p = _malloc_r(r, sizeof(LibcAtexit));
        if (p == 0) {
            return -1;
        }
        flags = sceKernelCpuSuspendIntr();
        p->ind = 0;
        p->next = *(LibcAtexit **)(r + 0x148);
        p->fntypes = 0;
        *(LibcAtexit **)(r + 0x148) = p;
    }
    p->fns[p->ind++] = fn;
    sceKernelCpuResumeIntr(flags);
    return 0;
}

int _close_r(int *r, int fd)
{
    int ret;

    libc_errno = 0;
    ret = _close(fd);
    if (ret == -1 && libc_errno != 0) {
        *r = libc_errno;
    }
    return ret;
}
