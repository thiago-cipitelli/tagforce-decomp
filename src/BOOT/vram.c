// @range 0x08835F48-0x088362E8
// @asm 0x088361B8-0x088362E8
// @rodata 0x0887536C-0x08875390
/* VRAM texture area after the framebuffers (0x158000 bytes of EDRAM): a bump allocator, upload
   of a GIM image frame into it (recorded in the GIM in place of its signature), and a walk over
   the GIM blocks whose per-type handling was compiled out. */
#include "psp.h"
#include "libc.h"

typedef struct GimBlock {
    unsigned short id;
    unsigned short unk2;
    unsigned int size;
    unsigned int child;
    unsigned int data;
} GimBlock;

typedef struct GimImage {
    unsigned short hsize;
    unsigned short unk2;
    unsigned short format;             /* +0x04 */
    unsigned short order;
    unsigned short width;              /* +0x08 */
    unsigned short height;             /* +0x0A */
    unsigned char pad0c[0xC];
    int offsets;                       /* +0x18 */
    unsigned char pad1c[0x10];
    unsigned short type;               /* +0x2C */
    unsigned short frames;             /* +0x2E */
} GimImage;

typedef struct Gim {
    char magic[8];
    void *vram;                        /* +0x08 */
    int vram_frame;                    /* +0x0C */
    GimBlock root;                     /* +0x10 */
} Gim;

typedef struct VramArea {              /* 0x08905098 */
    char *base;
    char *next;
    int size;
} VramArea;

extern VramArea vram_area;

void *sceGeEdramGetAddr(void);
int sceGeEdramGetSize(void);
GimBlock *gim_block_find(GimBlock *b, GimBlock *end, unsigned int id, int nth);
int gim_walk(GimBlock *b, Gim *g, GimBlock *end);

void vram_init(void)
{
    char *edram = sceGeEdramGetAddr();
    int size = sceGeEdramGetSize();

    vram_area.base = edram + 0x158000;
    vram_area.next = vram_area.base;
    vram_area.size = size - (vram_area.base - edram);
}

void vram_set_size(int size)
{
    sceGeEdramGetSize();
    vram_area.size = size;
}

void vram_reset(void)
{
    vram_area.next = vram_area.base;
}

void *vram_alloc(int size)
{
    VramArea *va = &vram_area;
    char *end = va->next + size;

    if ((unsigned int)(vram_area.base + vram_area.size) < (unsigned int)end) {
        return 0;
    }
    end = va->next;
    va->next = end + size;
    return end;
}

/* Copies image frame `frame` of `g` into VRAM; -1 when the area is full. */
int gim_upload(Gim *g, int frame)
{
    GimBlock *root = &g->root;
    unsigned int *sz = &root->size;
    GimBlock *blk;
    GimImage *img;
    int off;
    int size;
    void *p;

    gim_block_find(root, (GimBlock *)((char *)root + *sz), 5, 0);
    blk = gim_block_find(root, (GimBlock *)((char *)root + *sz), 4, 0);
    img = (GimImage *)((char *)blk + blk->data);
    if (img->type == 3) {
        off = ((int *)((char *)img + img->offsets))[frame % img->frames];
    } else {
        off = *(int *)((char *)img + img->offsets);
    }
    size = img->width * img->height;
    switch (img->format) {
    case 4:
    case 8:
        size /= 2;
        break;
    case 0:
    case 1:
    case 2:
    case 6:
        size *= 2;
        break;
    case 3:
    case 7:
        size *= 4;
        break;
    case 5:
        break;
    }
    p = vram_alloc(size);
    if (p == 0) {
        return -1;
    }
    g->vram = p;
    g->vram_frame = frame;
    memcpy(g->vram, (char *)img + off, size);
    return 0;
}

int vram_get_size(void)
{
    return vram_area.size;
}

int vram_get_used(void)
{
    return vram_area.next - vram_area.base;
}

void gim_check(Gim *g)
{
    GimBlock *root = &g->root;

    gim_walk(root, g, (GimBlock *)((char *)root + g->root.size));
}

#ifdef NON_MATCHING /* the original keeps a dead copy of the first load of the 0xFF case and reloads the child/size fields for the call */
/* Visits every block; the file info block (0xFF) holds three aligned strings. */
int gim_walk(GimBlock *start, Gim *g, GimBlock *end)
{
    GimBlock *b = start;
    char *s;
    int n;

    do {
        switch (b->id) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            break;
        case 6:
            break;
        case 0xFF:
            s = (char *)b + b->data;
            n = strlen(s) + 1;
            s = (char *)((unsigned int)(s + n) + 0xF & ~0xF);
            n = strlen(s) + 1;
            s = (char *)((unsigned int)(s + n) + 0xF & ~0xF);
            strlen(s);
            break;
        }
        if (b->child < b->size) {
            gim_walk((GimBlock *)((char *)b + b->child), g, (GimBlock *)((char *)b + b->size));
        }
        b = (GimBlock *)((char *)b + b->size);
    } while (!(b >= end));
    return 1;
}
#endif
