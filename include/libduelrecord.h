/* libduelrecord: duel record screen (three tabs of counters read from the save, drawn as a scrolling
   list). Module state and the BOOT functions it calls that no shared header declares. */
#ifndef LIBDUELRECORD_H
#define LIBDUELRECORD_H

typedef struct DrecEntry {             /* one line of a tab, 16 bytes */
    unsigned short index;              /* +0x00 item index in the tab's text table */
    unsigned short number;             /* +0x02 running number of the visible items */
    unsigned char is_new : 1;          /* +0x04 bit 0: first time shown (save flag cleared on build) */
    unsigned char has_value : 1;       /*       bit 1: draws a value column */
    unsigned char flags_pad : 6;
    unsigned char pad5[3];
    unsigned int value;                /* +0x08 */
    unsigned char lines : 6;           /* +0x0C text height in font lines */
    unsigned char lines_pad : 2;
    unsigned char padd[3];
} DrecEntry;

typedef struct DuelRecordState {       /* drec 0x0880A7D0, 0x34 bytes zeroed by duelrecord_init */
    unsigned short state;              /* +0x00 0 load, 1 fade in, 2 input, 3 fade out, 4 term */
    unsigned short pad2;
    unsigned char tab : 4;             /* +0x04 current tab */
    unsigned char n_tabs : 4;          /*       tabs available (2 or 3) */
    unsigned char percent;             /* +0x05 completion */
    unsigned short scroll[3];          /* +0x06 first visible entry per tab */
    unsigned short count[3];           /* +0x0C entries per tab */
    DrecEntry *entries[3];             /* +0x14 */
    unsigned char *save;               /* +0x20 sysstate2_get_ptr_51c() */
    void *heap;                        /* +0x24 */
    int fpl;                           /* +0x28 own 2 MB pool when the caller passes no heap */
    void *fpl_mem;                     /* +0x2C */
    int unk30;
} DuelRecordState;

typedef struct DrecFile {              /* files inside database/duelrecord.ehp, sorted by id */
    unsigned short id;
    void *data;
    const char *name;                  /* sprintf format, %c = language letter */
} DrecFile;

typedef struct DrecPackFlags {         /* 0x0880A810 */
    unsigned char ready : 1;           /* no read in flight */
    unsigned char loads : 7;           /* completed reads */
} DrecPackFlags;

extern DuelRecordState drec;
extern void *drec_pack;                /* 0x0880A804 database/duelrecord.ehp in memory */
extern DrecPackFlags drec_pack_flags;

void *drec_get_file(unsigned short id);
int drec_tab_rows(); /* K&R definition: callers pass an int, the callee truncates */
unsigned short drec_text_index(unsigned short tab, unsigned short i);

/* BOOT */
void *heap_create(void *mem, int size);
void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
int sceKernelFreeFpl(int fpl, void *data);
int sceKernelDeleteFpl(int fpl);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
void ehp_mount(int slot, void *pack);
void *ehp_find(void *pack, const char *name, int flags);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_is_done(void);
int fade_apply(void **list);
void gfx_overlay_enable(int on);
unsigned int *gfx_get_state(void);
void se_play(int id);
int sysflag_test(unsigned int bit);
int deck_slot_get_level(int i);
int day_next(void);
int font_get_height(void);
unsigned short *table_get_entry(void *table, int i);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *gim_set_texture(void *list, void *gim);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_clear_color(void *list, unsigned int rgba);
void *dl_clear_depth(void *list, int z);
void *dl_clear_stencil(void *list, int s);
void *dl_clear(void *list, int flags);
void *dl_tex_func(void *list, int func, int rgba);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);

#endif
