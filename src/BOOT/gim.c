// @range 0x08835650-0x08835F48
// @asm 0x088357C0-0x08835CAC
// @asm 0x08835DF4-0x08835F48
/* GIM textures: block search in the GIM tree, texture setup (format, image of a frame, CLUT of
   the palette block), sprite-sheet cells, and binding a texture from outside a display list. A
   GIM loaded in VRAM records the address and the frame there in place of the "PSP" signature. */
#include "psp.h"
#include "libc.h"

typedef struct GimBlock {
    unsigned short id;                 /* 4 image, 5 palette */
    unsigned short unk2;
    unsigned int size;                 /* +0x04 */
    unsigned int child;                /* +0x08 offset of the first child */
    unsigned int data;                 /* +0x0C offset of the data */
} GimBlock;

typedef struct GimImage {              /* image or palette data */
    unsigned short hsize;
    unsigned short unk2;
    unsigned short format;             /* +0x04 */
    unsigned short order;              /* +0x06 swizzle */
    unsigned short width;              /* +0x08 */
    unsigned short height;             /* +0x0A */
    unsigned short bpp;                /* +0x0C */
    unsigned short align;              /* +0x0E pitch alignment in bytes */
    unsigned char pad10[8];
    int offsets;                       /* +0x18 offset of the per-frame offsets */
    unsigned char pad1c[0xE];
    unsigned short cells;              /* +0x2A cells per frame */
    unsigned short type;               /* +0x2C 3 sequence */
    unsigned short frames;             /* +0x2E */
} GimImage;

typedef struct Gim {
    char magic[8];                     /* "MIG.00.1" */
    unsigned int vram;                 /* +0x08 "PSP", or the VRAM address of the image */
    int vram_frame;                    /* +0x0C frame resident in VRAM */
    GimBlock root;                     /* +0x10 */
} Gim;

void *dl_get_layer(void);
void dl_set_list(void *p);
void *dl_select_layer(void *layer, unsigned int *size);
void dl_set_cur_list_base(void *p);
void *dl_get_cur_list_base(void);
void sceGuStart(int cid, void *list, int size);
int sceGuFinish(void);
void *dl_tex_mode(void *list, int psm, int maxmips, int a2, int swizzle);
void *dl_tex_image(void *list, int mip, int w, int h, int tbw, void *addr);
void *ehsys_0883C57C(void *list, int psm, int shift, int mask, int start);
void *ehsys_0883C50C(void *list, int n, void *clut);

GimBlock *gim_block_find(GimBlock *b, GimBlock *end, unsigned int id, int nth);
void *gim_set_texture_frame(void *list, Gim *g, int frame);

void gim_reset(Gim *g)
{
    g->vram = 0;
    g->vram_frame = 0;
}

/* Sets the texture of `g` in the current GE list outside of the drawing code: closes the
   sub-list, emits the texture, reopens it. */
void gim_bind(Gim *g)
{
    void *layer;
    void *base;
    int n;
    void *list;
    void *layer2;
    void *buf;
    unsigned int size;

    layer = dl_get_layer();
    base = dl_get_cur_list_base();
    n = sceGuFinish();
    dl_set_list((char *)base + n - 8);
    list = gim_set_texture_frame(dl_select_layer(layer, 0), g, 0);
    layer2 = dl_get_layer();
    dl_set_list(list);
    dl_set_cur_list_base(buf = dl_select_layer(layer2, &size));
    sceGuStart(2, buf, size);
}

void *gim_set_texture(void *list, Gim *g)
{
    return gim_set_texture_frame(list, g, 0);
}

void gim_bind_frame(Gim *g, int frame)
{
    void *layer;
    void *base;
    int n;
    void *list;
    void *layer2;
    void *buf;
    unsigned int size;

    layer = dl_get_layer();
    base = dl_get_cur_list_base();
    n = sceGuFinish();
    dl_set_list((char *)base + n - 8);
    list = gim_set_texture_frame(dl_select_layer(layer, 0), g, frame);
    layer2 = dl_get_layer();
    dl_set_list(list);
    dl_set_cur_list_base(buf = dl_select_layer(layer2, &size));
    sceGuStart(2, buf, size);
}

#ifdef NON_MATCHING /* the original reloads root->size through a pointer to the field (addiu + lw) for the second search */
/* Texture mode and image of `frame` of the GIM's image, then its CLUT when it has a palette. */
void *gim_set_texture_frame(void *list, Gim *g, int frame)
{
    GimBlock *root = &g->root;
    GimBlock *pal = gim_block_find(root, (GimBlock *)((char *)root + g->root.size), 5, 0);
    GimBlock *blk = gim_block_find(root, (GimBlock *)((char *)root + g->root.size), 4, 0);
    GimImage *img = (GimImage *)((char *)blk + blk->data);
    GimImage *clut;
    int off;
    int mask;
    int n;

    if (img->type == 3) {
        off = ((int *)((char *)img + img->offsets))[frame % img->frames];
    } else {
        off = *(int *)((char *)img + img->offsets);
    }
    list = dl_tex_mode(list, img->format, 0, 0, img->order);
    mask = (img->align << 3) / img->bpp - 1;
    if (g->vram != 0 && g->vram != 0x505350 && g->vram_frame == frame) {
        list = dl_tex_image(list, 0, img->width, img->height, ~mask & img->width + mask, (void *)g->vram);
    } else {
        list = dl_tex_image(list, 0, img->width, img->height, ~mask & img->width + mask, (char *)img + off);
    }
    if (pal == 0) {
        return list;
    }
    clut = (GimImage *)((char *)pal + pal->data);
    if (clut->format == 3) {
        if (img->format == 5) {
            n = 0x20;
        } else if (img->format == 4) {
            n = 2;
        }
    } else if (img->format == 5) {
        n = 0x10;
    } else if (img->format == 4) {
        n = 1;
    }
    if (clut->type == 3) {
        off = ((int *)((char *)clut + clut->offsets))[frame % clut->frames];
    } else {
        off = *(int *)((char *)clut + clut->offsets);
    }
    list = ehsys_0883C57C(list, clut->format, 0, 0xFF, 0);
    return ehsys_0883C50C(list, n, (char *)clut + off);
}
#endif

#ifdef NON_MATCHING /* same as gim_set_texture_frame */
/* Same with a cell of a sprite sheet: image cell `cell` of `frame`, palette cell `pcell` of
   palette frame `pframe` (cells clamped to the sheet). */
void *gim_set_texture_sub(void *list, Gim *g, int frame, int cell, int pframe, int pcell)
{
    GimBlock *root = &g->root;
    GimBlock *pal = gim_block_find(root, (GimBlock *)((char *)root + g->root.size), 5, 0);
    GimBlock *blk = gim_block_find(root, (GimBlock *)((char *)root + g->root.size), 4, 0);
    GimImage *img = (GimImage *)((char *)blk + blk->data);
    GimImage *clut;
    int off;
    int mask;
    int n;

    if (img->type == 3) {
        if (cell < 0) {
            cell = 0;
        } else if (cell >= img->cells) {
            cell = img->cells - 1;
        }
        off = ((int *)((char *)img + img->offsets))[cell + img->cells * (frame % img->frames)];
    } else {
        off = *(int *)((char *)img + img->offsets);
    }
    list = dl_tex_mode(list, img->format, 0, 0, img->order);
    mask = (img->align << 3) / img->bpp - 1;
    if (g->vram != 0 && g->vram != 0x505350 && g->vram_frame == frame) {
        list = dl_tex_image(list, 0, img->width, img->height, ~mask & img->width + mask, (void *)g->vram);
    } else {
        list = dl_tex_image(list, 0, img->width, img->height, ~mask & img->width + mask, (char *)img + off);
    }
    if (pal == 0) {
        return list;
    }
    clut = (GimImage *)((char *)pal + pal->data);
    if (clut->format == 3) {
        if (img->format == 5) {
            n = 0x20;
        } else if (img->format == 4) {
            n = 2;
        }
    } else if (img->format == 5) {
        n = 0x10;
    } else if (img->format == 4) {
        n = 1;
    }
    if (clut->type == 3) {
        if (pcell < 0) {
            pcell = 0;
        } else if (pcell >= clut->cells) {
            pcell = clut->cells - 1;
        }
        off = ((int *)((char *)clut + clut->offsets))[pcell + clut->cells * (pframe % clut->frames)];
    } else {
        off = *(int *)((char *)clut + clut->offsets);
    }
    list = ehsys_0883C57C(list, clut->format, 0, 0xFF, 0);
    return ehsys_0883C50C(list, n, (char *)clut + off);
}
#endif

/* Image only, no CLUT. */
void *gim_set_image(void *list, Gim *g, int frame)
{
    GimBlock *root = &g->root;
    GimBlock *blk = gim_block_find(root, (GimBlock *)((char *)root + root->size), 4, 0);
    GimImage *img = (GimImage *)((char *)blk + blk->data);
    int off;
    int mask;

    if (img->type == 3) {
        off = ((int *)((char *)img + img->offsets))[frame % img->frames];
    } else {
        off = *(int *)((char *)img + img->offsets);
    }
    list = dl_tex_mode(list, img->format, 0, 0, img->order);
    mask = (img->align << 3) / img->bpp - 1;
    if (g->vram != 0 && g->vram != 0x505350 && g->vram_frame == frame) {
        return dl_tex_image(list, 0, img->width, img->height, ~mask & img->width + mask, (void *)g->vram);
    }
    return dl_tex_image(list, 0, img->width, img->height, ~mask & img->width + mask, (char *)img + off);
}

#ifdef NON_MATCHING /* the parameters land in the $s registers in reverse order in the original */
/* `nth` block with `id` among the siblings from `b` (depth first). */
GimBlock *gim_block_find(GimBlock *b, GimBlock *end, unsigned int id, int nth)
{
    GimBlock *r;

    for (;;) {
        if (b->id == id) {
            if (nth == 0) {
                return b;
            }
            nth--;
        }
        if (b->child < b->size) {
            r = gim_block_find((GimBlock *)((char *)b + *(int *)((char *)b + 8)), end, id, nth);
            if (r != 0) {
                return r;
            }
        }
        b = (GimBlock *)((char *)b + b->size);
        if (b >= end) {
            break;
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* same reload of root->size as gim_set_texture_frame */
/* Data of the image (4) or palette (5) block. */
void *gim_find_block(Gim *g, int id)
{
    GimBlock *root = &g->root;
    GimBlock *pal = gim_block_find(root, (GimBlock *)((char *)root + g->root.size), 5, 0);
    GimBlock *blk = gim_block_find(root, (GimBlock *)((char *)root + g->root.size), 4, 0);

    if (id == 4) {
        return (char *)blk + blk->data;
    }
    if (id == 5) {
        return (char *)pal + pal->data;
    }
    return 0;
}
#endif
