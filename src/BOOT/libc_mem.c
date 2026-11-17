// @range 0x0880DC98-0x0880E700
// @gcc 0x0880DC98-0x0880DD1C libc_memcmp
// @gcc 0x0880DD1C-0x0880DE28 nl_memcpy
// @asm 0x0880DE28-0x0880DE40
// @gcc 0x0880DE40-0x0880E140
// @gcc 0x0880E1F8-0x0880E270
// @asm 0x0880E270-0x0880E280
// @gcc 0x0880E280-0x0880E700 libc_reent
/* newlib (GCC) of the Sony SDK libc: memcmp, memcpy, memmove, memset, the interrupt-disabling
   malloc lock, and the Sony layer that keeps one reentrancy struct per module (a hash of 32 slots
   by module id, chained in 0x104-byte blocks) plus _REENT_INIT. The lock and the module table use
   globals through %hi/%lo and stay in the split asm with their C under NON_MATCHING. */
#include "libc.h"

int sceKernelCpuSuspendIntr(void);
void sceKernelCpuResumeIntr(int flags);
int sceKernelPrintf(const char *fmt, ...);
void *_malloc_r(void *r, size_t n);
extern const char libc_locale_c2[];    /* 0x08873C14 "C" */

/* newlib memcmp (word compare when both are aligned) */
/* newlib memcpy (16/4-byte blocks when aligned) */
#ifdef __MWERKS__ /* nl_memcpy.c (GCC track) */
void *memcpy(void *d, const void *s, size_t n);
#else
void *memcpy(void *d, const void *s, size_t n)
{
    unsigned char *p = d;
    const unsigned char *q = s;

    while (n-- != 0) {
        *p++ = *q++;
    }
    return d;
}
#endif

/* memmove: the SDK's is a variant of newlib's machine/mips memcpy, compiled by GCC (libc_mem_gcc.c); the port
   uses this plain C */
#ifndef __MWERKS__
void *mem_move(void *d, const void *s, size_t n)
{
    unsigned char *p = d;
    const unsigned char *q = s;

    if (q < p && p < q + n) {
        p += n;
        q += n;
        while (n-- != 0) {
            *--p = *--q;
        }
    } else {
        while (n-- != 0) {
            *p++ = *q++;
        }
    }
    return d;
}
#endif

/* newlib memset */
#ifdef __MWERKS__
asm void *memset(void *d, int c, size_t n)
{
    sltiu v0, a2, 16
    bnez v0, @98
    move v1, a0
    andi a3, a0, 0x3
    beqzl a3, @40
    andi a1, a1, 0xff
    sltiu v0, a3, 4
    beqz v0, @3c
    addiu a3, a3, 1
@24:
    sltiu v0, a3, 4
    sb a1, 0(v1)
    addiu a2, a2, -1
    addiu v1, v1, 1
    bnez v0, @24
    addiu a3, a3, 1
@3c:
    andi a1, a1, 0xff
@40:
    move a3, v1
    beqz a1, @5c
    move t0, a1
    sll v0, a1, 0x8
    or t0, a1, v0
    sll v1, t0, 0x10
    or t0, t0, v1
@5c:
    srl v0, a2, 0x3
    beqz v0, @7c
    andi a2, a2, 0x7
@68:
    addiu v0, v0, -1
    sw t0, 0(a3)
    sw t0, 4(a3)
    bnez v0, @68
    addiu a3, a3, 8
@7c:
    sltiu v0, a2, 4
    bnez v0, @98
    move v1, a3
    sw t0, 0(a3)
    addiu a2, a2, -4
    addiu a3, a3, 4
    move v1, a3
@98:
    beqz a2, @b0
    nop
@a0:
    addiu a2, a2, -1
    sb a1, 0(v1)
    bnez a2, @a0
    addiu v1, v1, 1
@b0:
    jr ra
    move v0, a0
}
#else
void *memset(void *d, int c, size_t n)
{
    unsigned char *p = d;

    while (n-- != 0) {
        *p++ = (unsigned char)c;
    }
    return d;
}
#endif

#ifdef NON_MATCHING /* PSP: libc_reent.c (GCC) */
typedef struct LibcReentTable {        /* 0x088F96BC */
    struct LibcReentTable *next;
    void *reent[32];                   /* +0x04 */
    int module[32];                    /* +0x84 */
} LibcReentTable;

extern LibcReentTable libc_reent_table;
extern int libc_malloc_depth;          /* 0x088F96B4 */
extern int libc_malloc_intr;           /* 0x088F96B8 */
extern char *libc_global_reent;        /* 0x088EB598 */

/* the malloc lock disables interrupts (nesting) */
/* _getmodreent: the reent struct of the calling thread's module (its id is kept at +0x37C of the
   thread's reent); the global one, with a message, when there is none */
void *libc_get_thread_module(void)
{
    extern char *libc_thread_reent(void); /* k0 + 4 in the original */
    char *t = libc_thread_reent();
    const char *msg;

    if (t == 0) {
        msg = "libc:%s: no reent structure\n";
    } else {
        int flags = sceKernelCpuSuspendIntr();
        int mod = *(int *)(t + 0x37C);
        LibcReentTable *e;

        for (e = &libc_reent_table; e != 0; e = e->next) {
            if (e->module[mod % 32] == mod) {
                void *r = e->reent[mod % 32];

                sceKernelCpuResumeIntr(flags);
                return r;
            }
        }
        sceKernelCpuResumeIntr(flags);
        msg = "libc:%s: no reent structure found\n";
    }
    sceKernelPrintf(msg, "_getmodreent");
    return libc_global_reent;
}

/* records r as the reent of module mod; -1 when the module already has one */
int libc_set_module_reent(int mod, char *r)
{
    int flags = sceKernelCpuSuspendIntr();
    LibcReentTable *e = &libc_reent_table;
    LibcReentTable *last = 0;
    int i = mod % 32;

    *(int *)(r + 0x37C) = mod;
    for (; e != 0; last = e, e = e->next) {
        if (e->module[i] == 0) {
            e->reent[i] = r;
            e->module[i] = mod;
            sceKernelCpuResumeIntr(flags);
            return 0;
        }
        if (e->module[i] == mod) {
            sceKernelCpuResumeIntr(flags);
            return -1;
        }
    }
    sceKernelCpuResumeIntr(flags);
    e = _malloc_r(libc_global_reent, sizeof(LibcReentTable));
    memset(e, 0, sizeof(LibcReentTable));
    e->reent[i] = r;
    e->module[i] = mod;
    flags = sceKernelCpuSuspendIntr();
    last->next = e;
    sceKernelCpuResumeIntr(flags);
    return 0;
}

/* removes the module of reent r from the table; returns its id or -1 */
int libc_module_of(char *r)
{
    int mod = *(int *)(r + 0x37C);
    int flags = sceKernelCpuSuspendIntr();
    LibcReentTable *e;

    for (e = &libc_reent_table; e != 0; e = e->next) {
        if (e->module[mod % 32] == mod) {
            e->module[mod % 32] = 0;
            e->reent[mod % 32] = 0;
            sceKernelCpuResumeIntr(flags);
            return mod;
        }
    }
    sceKernelCpuResumeIntr(flags);
    return -1;
}
#endif

/* newlib _REENT_INIT_PTR: std streams inside the struct, "C" locale, rand48 seed, and the
   stream area cleared */
#ifndef __MWERKS__ /* PSP: libc_reent.c */
void libc_init_reent(char *r)
{
    int i;

    *(int *)(r + 0x0C) = (int)(r + 800);
    *(int *)(r + 0x00) = 0;
    *(int *)(r + 0x04) = (int)(r + 616);
    *(int *)(r + 0x10) = 0;
    *(int *)(r + 0x08) = (int)(r + 708);
    for (i = 0; i < 25; i++) {
        r[0x14 + i] = 0;
    }
    *(const char **)(r + 0x34) = libc_locale_c2;
    for (i = 0x30; i <= 0x5C; i += 4) {
        if (i != 0x34) {
            *(int *)(r + i) = 0;
        }
    }
    r[0x60] = 0;
    for (i = 0; i < 36; i++) {
        r[0x7C + i] = 0;
    }
    *(int *)(r + 0xA8) = 1;
    *(int *)(r + 0xAC) = 0;
    *(unsigned short *)(r + 0xB0) = 0x330E;
    *(unsigned short *)(r + 0xB2) = 0xABCD;
    *(unsigned short *)(r + 0xB4) = 0x1234;
    *(unsigned short *)(r + 0xB6) = 0xE66D;
    *(unsigned short *)(r + 0xB8) = 0xDEEC;
    *(unsigned short *)(r + 0xBC) = 11;
    *(unsigned short *)(r + 0xBA) = 5;
    *(int *)(r + 0xA0) = 0;
    for (i = 0xC0; i <= 0xD4; i += 4) {
        *(int *)(r + i) = 0;
    }
    for (i = 0xFC; i <= 0x120; i += 4) {
        *(int *)(r + i) = 0;
    }
    r[0xD8] = 0;
    r[0xE0] = 0;
    *(int *)(r + 0xF8) = 0;
    for (i = 0x148; i <= 0x154; i += 4) {
        *(int *)(r + i) = 0;
    }
    *(int *)(r + 0x254) = 0;
    *(int *)(r + 0x1D4) = 0;
    *(int *)(r + 0x258) = 0;
    *(int *)(r + 0x25C) = 0;
    *(int *)(r + 0x260) = 0;
    *(int *)(r + 0x264) = 0;
    memset(r + 616, 0, 276);
    *(int *)(r + 0x37C) = 0;
}
#else
void libc_init_reent(char *r);
#endif
