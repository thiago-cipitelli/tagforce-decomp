// @cc gcc -falign-loops=64
/* Functions of libc_mem.c (newlib of the Sony SDK libc) that come out of C byte for byte with the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); their ranges are the `@gcc` lines of libc_mem.c.
   -falign-loops=64 is the flag of the mem_move object (the `nop`s before its forward loops and the padding of its
   section up to memset); the lock functions have no loops. */
#include "libc.h"

/* mem_move (memmove): Sony's variant of newlib 1.12.0's libc/machine/mips/memcpy.c, with a backward copy for
   overlapping ranges. Each direction is the memcpy loop set (4-word blocks, words, bytes; `ulw`/`usw` when source and
   destination are not equally aligned) in its own inline helper, after copying the head up to a word boundary with
   one lwl/swl (backward) or lwr/swr (forward); tools/psp_gcc_peep.py (rule F) writes the unaligned accesses as the
   lwl/lwr (swl/swr) words of Sony's assembler. The backward helper returns its own destination, so memmove of an
   overlapping range returns dst + len (no caller reads it). The PC port uses the plain C of libc_mem.c. */
#ifdef __mips__
#define wordtype long
#define BIGBLOCKSIZE    (sizeof (wordtype) << 2)
#define LITTLEBLOCKSIZE (sizeof (wordtype))
#define TOO_SMALL(LEN)  ((LEN) < (LITTLEBLOCKSIZE << 1))

static inline void *mem_move_back(void *dst0, const void *src0, size_t len0) __attribute__((always_inline));
static inline void *mem_move_back(void *dst0, const void *src0, size_t len0)
{
    char *dst = dst0;
    const char *src = src0;
    char *d = dst;
    const char *s = src;
    wordtype *aligned_dst;
    const wordtype *aligned_src;
    int len = len0;
    size_t iter;

    if (!TOO_SMALL(len)) {
        if (!(((long)src ^ (long)dst) & (sizeof (wordtype) - 1))) {
            int k = (long)src & (sizeof (wordtype) - 1);

            if (k) {
                int tmp;

                __asm__ ("lwl %0,%1" : "=r" (tmp) : "m" (src[-1]));
                len -= k;
                s -= k;
                d -= k;
                __asm__ ("swl %1,%0" : "=m" (dst[-1]) : "r" (tmp));
            }
            iter = len / BIGBLOCKSIZE;
            len = len % BIGBLOCKSIZE;
            aligned_dst = (wordtype *)d;
            aligned_src = (wordtype *)s;
            while (iter > 0) {
                wordtype tmp0, tmp1, tmp2, tmp3;

                aligned_src -= 4;
                tmp0 = aligned_src[0];
                tmp1 = aligned_src[1];
                tmp2 = aligned_src[2];
                tmp3 = aligned_src[3];
                aligned_dst -= 4;
                aligned_dst[0] = tmp0;
                aligned_dst[1] = tmp1;
                aligned_dst[2] = tmp2;
                aligned_dst[3] = tmp3;
                iter--;
            }
            iter = len / LITTLEBLOCKSIZE;
            len = len % LITTLEBLOCKSIZE;
            while (iter > 0) {
                *--aligned_dst = *--aligned_src;
                iter--;
            }
        } else {
            int tmp;

            aligned_dst = (wordtype *)dst;
            aligned_src = (wordtype *)src;
            iter = len / 4;
            len = len % 4;
            while (iter > 0) {
                aligned_src--;
                aligned_dst--;
                __asm__ ("ulw %0,%1" : "=r" (tmp) : "m" (*aligned_src));
                __asm__ ("usw %1,%0" : "=m" (*aligned_dst) : "r" (tmp));
                iter--;
            }
        }
        d = (char *)aligned_dst;
        s = (const char *)aligned_src;
    }
    while (len > 0) {
        *--d = *--s;
        len--;
    }
    return dst0;
}

static inline void *mem_move_fwd(void *dst0, const void *src0, size_t len0) __attribute__((always_inline));
static inline void *mem_move_fwd(void *dst0, const void *src0, size_t len0)
{
    char *dst = dst0;
    const char *src = src0;
    char *d = dst;
    const char *s = src;
    wordtype *aligned_dst;
    const wordtype *aligned_src;
    int len = len0;
    size_t iter;

    if (!TOO_SMALL(len)) {
        if (!(((long)src ^ (long)dst) & (sizeof (wordtype) - 1))) {
            int k = (long)src & (sizeof (wordtype) - 1);

            if (k) {
                int tmp;
                int n = sizeof (wordtype) - k;

                len -= n;
                __asm__ ("lwr %0,%1" : "=r" (tmp) : "m" (*s));
                d += n;
                s += n;
                __asm__ ("swr %1,%0" : "=m" (*dst) : "r" (tmp));
            }
            iter = len / BIGBLOCKSIZE;
            len = len % BIGBLOCKSIZE;
            aligned_dst = (wordtype *)d;
            aligned_src = (wordtype *)s;
            while (iter > 0) {
                wordtype tmp0 = aligned_src[0];
                wordtype tmp1 = aligned_src[1];
                wordtype tmp2 = aligned_src[2];
                wordtype tmp3 = aligned_src[3];

                aligned_dst[0] = tmp0;
                aligned_dst[1] = tmp1;
                aligned_dst[2] = tmp2;
                aligned_dst[3] = tmp3;
                aligned_src += 4;
                aligned_dst += 4;
                iter--;
            }
            iter = len / LITTLEBLOCKSIZE;
            len = len % LITTLEBLOCKSIZE;
            while (iter > 0) {
                *aligned_dst++ = *aligned_src++;
                iter--;
            }
        } else {
            int tmp;

            aligned_dst = (wordtype *)dst;
            aligned_src = (wordtype *)src;
            iter = len / 4;
            len = len % 4;
            while (iter > 0) {
                __asm__ ("ulw %0,%1" : "=r" (tmp) : "m" (*aligned_src));
                iter--;
                aligned_src++;
                __asm__ ("usw %1,%0" : "=m" (*aligned_dst) : "r" (tmp));
                aligned_dst++;
            }
        }
        d = (char *)aligned_dst;
        s = (const char *)aligned_src;
    }
    while (len > 0) {
        *d++ = *s++;
        len--;
    }
    return dst0;
}

void *mem_move(void *dst0, const void *src0, size_t len0)
{
    if ((const char *)src0 < (char *)dst0 && (char *)dst0 < (const char *)src0 + len0) {
        return mem_move_back((char *)dst0 + len0, (const char *)src0 + len0, len0);
    }
    return mem_move_fwd(dst0, src0, len0);
}
#endif


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
