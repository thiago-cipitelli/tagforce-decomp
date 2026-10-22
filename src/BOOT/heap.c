// @range 0x08837B28-0x088390C4
// @asm 0x08837B28-0x08837D10
// @asm 0x08837FA4-0x08838468
// @asm 0x0883864C-0x08838AF0
// @asm 0x08838EC8-0x088390C4
/* "HOGE" heap: a sorted table of block boundaries, each word holding an address and a free flag
   (block i spans ent[i].addr to ent[i + 1].addr). The table lives in a block of the heap itself
   and grows by doubling, moving to the lowest free block when possible. Allocation is best fit
   with blocks split from the top; freeing merges with free neighbours. */
#include "psp.h"
#include "libc.h"
#include "allegrex.h"

#define HEAP_MAGIC 0x45474F48          /* "HOGE" */

typedef struct HeapEnt {
    unsigned int addr : 31;
    unsigned int free : 1;
} HeapEnt;

typedef struct Heap {
    unsigned int magic;
    unsigned int level : 2;            /* +0x04 table moves since the last compaction (max 3) */
    unsigned int pad : 6;              /*       offset from the allocation, 0x3F external memory */
    unsigned int size : 24;
    HeapEnt *ent;                      /* +0x08 */
    unsigned short tabsize;            /* +0x0C bytes */
    unsigned short n;                  /* +0x0E blocks (n + 1 boundaries) */
} Heap;

void *heap_malloc_cur(int align, unsigned int size);
void free(void *p);
void mem_move(void *dst, void *src, unsigned int n);

int heap_grow_table(Heap *h);
HeapEnt *heap_best_fit(Heap *h, unsigned int size);
int heap_compact(Heap *h);
HeapEnt *heap_find_entry(Heap *h, void *p);
HeapEnt *heap_split(Heap *h, HeapEnt *e, int size, int top);
int heap_merge(Heap *h, HeapEnt *e);
int heap_relocate_table(Heap *h);

/* Creates a heap of `size` bytes in `mem` (allocated when 0). */
#ifdef NON_MATCHING /* o original calcula h->pad com `subu v0,a0,a0` (h e mem no mesmo registrador); aqui o resultado do malloc fica em v0 e os temporários dos bitfields mudam. Todas as formas de cópia (h = mem = f(), local raw, cópia depois do teste) dobram para zero ou deixam mem em v0 */
Heap *heap_create(Heap *mem, unsigned int size)
{
    Heap *h = mem;
    HeapEnt *e;

    if (h == 0) {
        if ((mem = h = heap_malloc_cur(0x10, size)) == 0) {
            return 0;
        }
        h->size = size;
        h->pad = (char *)h - (char *)mem;
    } else {
        h->pad = 0x3F;
        h->size = size & ~0xF;
    }
    h->magic = HEAP_MAGIC;
    e = (HeapEnt *)(h + 1);
    h->ent = e;
    h->tabsize = 0x80;
    h->tabsize = h->tabsize + 0xF & ~0xF;
    h->n = 3;
    h->level = 0;
    e[0].addr = (unsigned int)h;
    e[0].free = 0;
    e[1].addr = (unsigned int)e;
    e[1].free = 0;
    e[2].addr = e[1].addr + h->tabsize;
    e[2].free = 1;
    e[3].addr = (unsigned int)h + h->size;
    e[3].free = 0;
    return h;
}
#endif

int heap_free_block(Heap *h)
{
    int pad;

    h->magic = 0;
    pad = h->pad;
    if (pad < 0x11) {
        free((char *)h - pad);
    }
    return 1;
}

/* Best-fit allocation of `size` bytes (rounded to 16); 0 when no block fits. */
void *heap_alloc(Heap *h, int n)
{
    HeapEnt *e;
    int size = n;
    HeapEnt *s;

    if (n <= 0) {
        return 0;
    }
    if (h->tabsize < (h->n + 2) * sizeof(HeapEnt) && heap_grow_table(h) == 0) {
        return 0;
    }
    size = size + 0xF & ~0xF;
    if ((e = heap_best_fit(h, size)) == 0) {
        heap_compact(h);
        if ((e = heap_best_fit(h, size)) == 0) {
            return 0;
        }
    }
    if (size + 0x40 < (int)(e[1].addr - e[0].addr) && (s = heap_split(h, e, size, 1)) != 0) {
        e = s;
    }
    e->free = 0;
    size = e->addr;
    heap_relocate_table(h);
    return (void *)size;
}

int heap_set_level(Heap *h, void *p)
{
    HeapEnt *e = heap_find_entry(h, p);

    e->free = 1;
    h->level = ALLEGREX_MIN(3, h->level + 1);
    heap_merge(h, e);
    heap_relocate_table(h);
    return 1;
}

void heap_stats(Heap *h, int *used, int *freed)
{
    HeapEnt *e = h->ent;
    HeapEnt *end = e + h->n;
    int u;
    int f;

    f = 0;
    u = 0;
    for (; e < end; e++) {
        if (e->free) {
            f += e[1].addr - e[0].addr;
        } else {
            u += e[1].addr - e[0].addr;
        }
    }
    if (used != 0) {
        *used = u;
    }
    if (freed != 0) {
        *freed = f;
    }
}

#ifdef NON_MATCHING /* unrolled copies and register allocation differ */
/* Doubles the table: in place when the block after it is free and large enough, otherwise in a
   new block (the old one is freed). Returns 1 when the table could stay in place. */
int heap_grow_table(Heap *h)
{
    HeapEnt *t = heap_find_entry(h, h->ent);
    HeapEnt *d;
    HeapEnt *s;
    HeapEnt *end;
    HeapEnt *ne;
    unsigned int want;
    unsigned int old;
    int delta;
    int len;

    if (t[1].free) {
        old = t[1].addr;
        if (h->tabsize < t[2].addr - old) {
            t[1].addr = old + h->tabsize;
            h->tabsize <<= 1;
            return 1;
        }
        end = h->ent + h->n;
        d = t + 1;
        for (s = t + 2; s <= end; s++) {
            *d++ = *s;
        }
        h->n--;
        h->tabsize = t[1].addr - t[0].addr;
        return 1;
    }
    want = h->tabsize << 1;
    if (want <= h->tabsize + 0x40) {
        return 0;
    }
    while ((ne = heap_best_fit(h, want)) == 0) {
        want = h->tabsize + (int)(want - h->tabsize) / 2 & ~0xF;
        if ((int)want <= (int)(h->tabsize + 0x40)) {
            return 0;
        }
    }
    len = h->tabsize;
    memcpy((void *)ne->addr, h->ent, len);
    delta = ne->addr - (unsigned int)h->ent;
    ne = (HeapEnt *)((char *)ne + (ne->addr - (unsigned int)h->ent));
    ((HeapEnt *)((char *)t + delta))->free = 1;
    len *= 2;
    ne->free = 0;
    h->ent = (HeapEnt *)ne->addr;
    if ((int)(ne[1].addr - ne[0].addr) <= len) {
        h->tabsize = ne[1].addr - ne[0].addr;
    } else {
        end = h->ent + h->n;
        for (s = end, d = end + 1; s >= ne + 1; s--) {
            *d-- = *s;
        }
        h->n++;
        ne[1].addr = ne[0].addr + len;
        ne[1].free = 1;
        h->tabsize = ne[1].addr - ne[0].addr;
    }
    h->level = ALLEGREX_MIN(3, h->level + 1);
    return 0;
}
#endif

HeapEnt *heap_best_fit(Heap *h, unsigned int size)
{
    HeapEnt *e = h->ent;
    HeapEnt *end = e + h->n;
    HeapEnt *best = 0;
    unsigned int bsize = 0;
    unsigned int s;

    for (; e < end; e++) {
        if (e->free) {
            s = e[1].addr - e[0].addr;
            if (s >= size) {
                if (best != 0) {
                    if (bsize >= s) {
                        bsize = s;
                        best = e;
                    }
                } else {
                    bsize = s;
                    best = e;
                }
            }
        }
    }
    return best;
}

/* Merges runs of free blocks. */
int heap_compact(Heap *h)
{
    HeapEnt *d;
    HeapEnt *s = h->ent;
    HeapEnt *end = s + h->n;

    d = s;

    while (s < end) {
        if (d != s) {
            *d = *s;
        }
        s++;
        d++;
        if (s[-1].free) {
            while (s->free && s < end) {
                s++;
            }
        }
    }
    *d = *s;
    h->n = (d - h->ent);
    h->level = 0;
    return 1;
}

/* Boundary entry whose address is `p` (binary search), or 0. */
HeapEnt *heap_find_entry(Heap *h, void *p)
{
    int lo = 0;
    int hi = h->n;
    int mid = hi / 2;
    HeapEnt *e;

    if (mid > 0) {
        do {
            e = &h->ent[mid];
            if (e->addr < (unsigned int)p) {
                lo = mid;
            } else if ((unsigned int)p < e->addr) {
                hi = mid;
            } else {
                return e;
            }
            mid = (lo + hi) / 2;
        } while (lo < mid);
    }
    return 0;
}

#ifdef NON_MATCHING /* unrolled copies and register allocation differ */
/* Splits `size` bytes off free block `e`: from its top (`top`) or its bottom, joining a free
   neighbour when there is one. Returns the entry of the new used block, 0 if the table is full. */
HeapEnt *heap_split(Heap *h, HeapEnt *e, int size, int top)
{
    unsigned int sz = size + 0xF & ~0xF;
    HeapEnt *s;
    HeapEnt *d;

    if (top) {
        if (e[1].free) {
            e[1].addr = e[2].addr - sz;
            e[1].free = 1;
            return e + 1;
        }
        if (e[-1].free) {
            e[0].addr = e[1].addr - sz;
            e[0].free = 1;
            return e;
        }
        if (h->tabsize < (h->n + 2) * sizeof(HeapEnt)) {
            return 0;
        }
        s = h->ent + h->n;
        for (d = s + 1; s >= e + 1; s--) {
            *d-- = *s;
        }
        h->n++;
        e[1].addr = e[2].addr - sz;
        e[1].free = 1;
        return e + 1;
    }
    if (e[1].free) {
        e[1].addr = e[0].addr + sz;
        e[1].free = 1;
        return e;
    }
    if (e[-1].free) {
        e[0].addr = e[-1].addr + sz;
        e[0].free = 1;
        return e - 1;
    }
    if (h->tabsize < (h->n + 2) * sizeof(HeapEnt)) {
        return 0;
    }
    s = h->ent + h->n;
    for (d = s + 1; s >= e + 1; s--) {
        *d-- = *s;
    }
    h->n++;
    e[1].addr = e[0].addr + sz;
    e[1].free = 1;
    return e;
}
#endif

/* Merges free block `e` with its free neighbours by removing boundaries. */
int heap_merge(Heap *h, HeapEnt *e)
{
    HeapEnt *s;
    HeapEnt *end;
    HeapEnt *d;

    if (!e->free) {
        return 1;
    }
    if (e[-1].free) {
        if (e[1].free) {
            end = h->ent + h->n;
            for (s = e + 2; s <= end; s++) {
                *e++ = *s;
            }
            h->n -= 2;
        } else {
            end = h->ent + h->n;
            for (d = e + 1; d <= end; d++) {
                *e++ = *d;
            }
            h->n--;
        }
    } else if (e[1].free) {
        HeapEnt *src, *dst; /* locais do ramo: com s/d os temporários do laço desenrolado mudam */

        end = h->ent + h->n;
        dst = e + 1;
        for (src = e + 2; src <= end; src++) {
            *dst++ = *src;
        }
        h->n--;
    }
    return 1;
}

#ifdef NON_MATCHING /* register allocation differs */
/* Moves the table down into the free block in front of it when there is one. */
int heap_relocate_table(Heap *h)
{
    HeapEnt *t = h->ent;
    HeapEnt *b = &t[1];
    unsigned int next;
    unsigned int room;
    HeapEnt *e;
    unsigned int sz;

    if (!t[1].free) {
        return 1;
    }
    next = t[2].addr;
    if ((unsigned int)t == next) {
        mem_move((void *)b->addr, (void *)next, (h->n + 1) * 4);
        t[2].addr = b->addr + h->tabsize;
        t[2].free = 1;
        t[1].free = 0;
        h->ent = (HeapEnt *)b->addr;
        return 1;
    }
    sz = h->tabsize;
    room = next - b->addr;
    if (sz > room) {
        return 1;
    }
    if (sz < room) {
        if (sz < (h->n + 2) * sizeof(HeapEnt)) {
            return 0;
        }
        if (heap_split(h, b, sz, 0) == 0) {
            return 0;
        }
    }
    e = heap_find_entry(h, h->ent);
    t[1].free = 0;
    e->free = 1;
    memcpy((void *)b->addr, h->ent, h->tabsize);
    h->ent = (HeapEnt *)((HeapEnt *)((char *)b + (b->addr - (unsigned int)h->ent)))->addr;
    h->level = ALLEGREX_MIN(3, h->level + 1);
    return 1;
}
#endif
