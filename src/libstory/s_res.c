// @range 0x0880C728-0x0880C9E4
// @data 0x0881D32C-0x0881D330
/* Memory and files of the story scene (st_res): three heaps in the module's .bss, the read of a whole
   EHP file into the first one (waiting frame by frame) and the extraction of a gzip member of an EHP
   into heap 1 or 2. */
#include "libstory.h"

void *heap_create(void *mem, unsigned int size);
void *heap_alloc(void *h, int n);
int heap_set_level(void *h, void *p);
void heap_free_block(void *h);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehp_mount(void (*cb)(void *), void *pack);
void *ehp_find(void *pack, const char *name, int *size);
int read_u32_before(void *p, int size);
void gzip_decompress_alias(void *dst, int size, void *src);

int st_file_loaded(void);

extern unsigned char st_heap_mem0[0x240000];   /* 0x08889180 */
extern unsigned char st_heap_mem1[0x40000];    /* 0x08849180 */
extern unsigned char st_heap_mem2[0x20000];    /* 0x08829180 */

void st_res_init(void)
{
    memset(&st_res, 0, sizeof(st_res));
    st_res.heap[0] = heap_create(st_heap_mem0, 0x240000);
    st_res.heap[1] = heap_create(st_heap_mem1, 0x40000);
    st_res.heap[2] = heap_create(st_heap_mem2, 0x20000);
}

void st_res_term(void)
{
    StRes *r = &st_res;
    int i;

    for (i = 0; i < 3; i++) {
        if (r->heap[i]) {
            heap_free_block(r->heap[i]);
            r->heap[i] = 0;
        }
    }
}

void *st_save_buf(void)
{
    return st_res.heap[0];
}

/* reads a whole file of dir 3 into heap 0 and mounts it as an EHP (heap is not used) */
void *st_file_load(const char *path, int heap)
{
    char name[0x200];
    StRes *r;

    st_res.file = 0;
    st_res.file_size = 0;
    st_res.file_done = 0;
    r = &st_res;
    sprintf(name, "%s", path);
    if ((st_res.file_size = ehfile_get_size(3, name)) <= 0)
        return 0;
    if ((r->file = heap_alloc(r->heap[0], st_res.file_size)) == 0)
        return 0;
    ehfile_read_async_alias(3, name, r->file, r->file_size, (int (*)(void *, int))st_file_loaded, r->file);
    for (;;) {
        if (r->file_done != 0)
            break;
        sys_frame_wait();
    }
    return r->file;
}

void st_res_free(void *p, int heap)
{
    StRes *r = &st_res;

    if (p)
        heap_set_level(r->heap[heap], p);
}

/* unpacks the gzip member name of an EHP into heap 1 or 2 */
void *st_ehp_load(void *ehp, const char *name, int heap)
{
    StRes *r = &st_res;
    void *src;
    void *dst;
    int size;
    int n;

    if (heap != 2 && heap != 1)
        return 0;
    src = ehp_find(ehp, name, &size);
    if (src == 0)
        return 0;
    n = read_u32_before(src, size);
    dst = heap_alloc(r->heap[heap], n);
    if (dst == 0)
        return 0;
    gzip_decompress_alias(dst, n, src);
    return dst;
}

int st_file_loaded(void)
{
    ehp_mount(0, st_res.file);
    return st_res.file_done = 1;
}
