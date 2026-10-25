/* libdeck: deck editor scene (scene 4 of the BOOT scene table). Shared state and the BOOT (libehsys)
   calls it uses; prototypes copy the BOOT definitions where the parameter types decide the caller's
   masking. Several files are written in parallel: append new declarations at the end of the section of
   your file, never change a line another file may rely on. */
#ifndef LIBDECK_H
#define LIBDECK_H

#include "ehsys.h"
#include "font.h"
#include "psp.h"
#include "libc.h"
#include "cip.h"
#include "card_db.h"
#include "game.h"

typedef struct DkEnt {                 /* one card line, 6 bytes (same layout as libselect_card ScEnt) */
    unsigned short id;                 /* card id */
    unsigned short key;                /* sort key */
    unsigned char count : 7;           /* +0x04 copies */
    unsigned char fixed : 1;
    unsigned char marked : 1;          /* +0x05 */
    unsigned char b5_1 : 7;
} DkEnt;

typedef struct DkList {                /* 0x20: one card list (owned cards, deck, side...) */
    unsigned short tab;                /* +0x00 index in dk_lists */
    unsigned short pad02;
    DkEnt *ents;                       /* +0x04 */
    unsigned short n;                  /* +0x08 entries in ents */
    unsigned short pad0a;
    int total;                         /* +0x0C */
    unsigned short *order;             /* +0x10 visible entries (indices into ents), sorted/filtered */
    unsigned short count;              /* +0x14 entries in order */
    unsigned short u16;                /* +0x16 */
    unsigned short u18;                /* +0x18 */
    unsigned short u1a;                /* +0x1A */
    unsigned short u1c;                /* +0x1C */
    unsigned short pad1e;
} DkList;

extern DkList dk_lists[6];             /* 0x095D2330 */
extern void *dk_heap;                  /* 0x095D26EC */

/* BOOT (libehsys) */
int sceKernelFreeFpl(int fpl, void *data);
int sceKernelDeleteFpl(int fpl);
void *heap_create(void *mem, unsigned int size);
void *heap_alloc(void *h, int n);
int heap_set_level(void *h, void *p);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehp_mount(void (*cb)(void *), void *pack);
void *ehp_find(void *pack, const char *name, int *size);
void bgm_play(int id);
void se_play(int id);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_apply(void **list);
void gfx_overlay_enable(int on);
unsigned short region_pick_value(int flag);
unsigned short *table_get_entry(void *t, unsigned int i);
unsigned int *gfx_get_state(void);
void cardinfo_draw(void);
void *gfx_begin_2d(void *list);
void *gfx_end_2d(void *list);
void *ehf_draw(void *list, void *ehf, void *opts);
int ehf_set_anim(void *e, int target, int limit);
void *gim_set_texture(void *list, void *gim);
void *gim_set_texture_frame(void *list, void *gim, int frame);
void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_tex_func(void *list, int func, int comp);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_alloc_inline(void **list, int bytes);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
int inv_get_bit3(unsigned short id);
int inv_get_hi(unsigned short id);
int inv_get_flag7(unsigned short id);
void inv_set_flag7(unsigned short id, int on);
int inv_get_count(unsigned short id);

/* ---- per-file additions below (append only; keep each file's block together) ---- */

#endif
