// @cc gcc
/* Functions of libc_mem.c (newlib of the Sony SDK libc) that come out of C byte for byte with the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); their ranges are the `@gcc` lines of libc_mem.c. */

int sceKernelCpuSuspendIntr(void);
void sceKernelCpuResumeIntr(int flags);
extern int libc_malloc_depth;          /* 0x088F96B4 */
extern int libc_malloc_intr;           /* 0x088F96B8 */

void __malloc_lock(void *r)
{
    int flags = sceKernelCpuSuspendIntr();

    if (libc_malloc_depth == 0) {
        libc_malloc_intr = flags;
    }
    libc_malloc_depth++;
}

void __malloc_unlock(void *r)
{
    if (--libc_malloc_depth == 0) {
        sceKernelCpuResumeIntr(libc_malloc_intr);
    }
}
