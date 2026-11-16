// @cc gcc -O2 -Iinclude/newlib-1.12.0/libc/include -falign-loops=64 -fno-function-sections
// @rodata 0x08873BC0-0x08873C18
/* The Sony SDK libc's per-module reentrancy object (0x0880E280-0x0880E700 of BOOT): _getmodreent, the table of one
   reent struct per module (a hash of 32 slots by module id, chained in 0x104-byte blocks) and _REENT_INIT, compiled by
   the SDK's GCC (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"). The object was built with loop alignment (-falign-loops=64:
   the nops before the printf of _getmodreent and before the two loops of _REENT_INIT_PTR), so it is compiled as one
   section (-fno-function-sections) and linked whole. PSP build only; the PC port uses the C in libc_mem.c. */

#ifdef __mips__
#include <string.h>
#include <sys/reent.h>
#include "sdk_reent.h"
int sceKernelCpuSuspendIntr(void);
void sceKernelCpuResumeIntr(int flags);
int sceKernelPrintf(const char *fmt, ...) __attribute__((weak)); /* tested before each call */
void *_malloc_r(void *r, size_t n);
typedef struct LibcReentTable {        /* 0x088F96BC */
    struct LibcReentTable *next;
    void *reent[32];                   /* +0x04 */
    int module[32];                    /* +0x84 */
} LibcReentTable;
extern LibcReentTable libc_reent_table __attribute__((weak)); /* the original tests its address */
extern char *libc_global_reent;

/* _getmodreent: the reent struct of the calling thread's module (its id is kept at +0x37C of the thread's reent, which
   the thread keeps at 4(k0)); the global one, with a message, when there is none */
void *libc_get_thread_module(void)
{
    register char **k0 __asm__("$26");
    char *t;

    if (k0 == 0 || (t = k0[1]) == 0) {
        if (sceKernelPrintf != 0) {
            sceKernelPrintf("libc:%s: no reent structure\n", "_getmodreent");
        }
    } else {
        int flags = sceKernelCpuSuspendIntr();
        int mod = *(int *)(t + 0x37C);
        int i = mod % 32;
        LibcReentTable *e;

        for (e = &libc_reent_table; e != 0; e = e->next) {
            if (e->module[i] == mod) {
                void *r = e->reent[i];

                sceKernelCpuResumeIntr(flags);
                return r;
            }
        }
        sceKernelCpuResumeIntr(flags);
        if (sceKernelPrintf != 0) {
            sceKernelPrintf("libc:%s: no reent structure found\n", "_getmodreent");
        }
    }
    return libc_global_reent;
}

/* records r as the reent of module mod; -1 when the module already has one */
int libc_set_module_reent(int mod, char *r)
{
    int flags = sceKernelCpuSuspendIntr();
    LibcReentTable *e;
    LibcReentTable *last;
    int i;

    *(int *)(r + 0x37C) = mod;
    i = mod % 32;
    for (e = &libc_reent_table; e != 0; last = e, e = e->next) {
        if (e->module[i] == 0) {
            e->module[i] = mod;
            e->reent[i] = r;
            break;
        }
        if (e->module[i] == mod) {
            sceKernelCpuResumeIntr(flags);
            return -1;
        }
    }
    sceKernelCpuResumeIntr(flags);
    if (e == 0) {
        e = _malloc_r(libc_global_reent, sizeof(LibcReentTable));
        memset(e, 0, sizeof(LibcReentTable));
        e->module[i] = mod;
        e->reent[i] = r;
        flags = sceKernelCpuSuspendIntr();
        last->next = e;
        sceKernelCpuResumeIntr(flags);
    }
    return 0;
}

/* removes the module of reent r from the table; returns its id or -1 */
int libc_module_of(char *r)
{
    int mod = *(int *)(r + 0x37C);
    int flags = sceKernelCpuSuspendIntr();
    int i = mod % 32;
    LibcReentTable *e;

    for (e = &libc_reent_table; e != 0; e = e->next) {
        if (e->module[i] == mod) {
            e->reent[i] = 0;
            e->module[i] = 0;
            sceKernelCpuResumeIntr(flags);
            return mod;
        }
    }
    sceKernelCpuResumeIntr(flags);
    return -1;
}

/* newlib 1.12.0 _REENT_INIT_PTR as the SDK has it (sdk_reent.h) */
void libc_init_reent(struct _reent *r)
{
    SDK_REENT_INIT_PTR(r, "C")
}
#endif
