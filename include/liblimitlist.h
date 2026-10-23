/* liblimitlist: forbidden/limited card list sub-screen (imported by libtitle/libstory/liblabo).
   Module state and the BOOT functions it calls that no shared header declares. */
#ifndef LIBLIMITLIST_H
#define LIBLIMITLIST_H

#include "font.h"
#include "ehsys.h"

typedef struct LlEntry {       /* one card of a list, 6 bytes */
    unsigned short id;         /* card id */
    unsigned short file;       /* index in fileTbl.dat (text_start) */
    unsigned char visible : 1;
    unsigned char flags_pad : 7;
    unsigned char pad5;
} LlEntry;

typedef struct LlScroll {      /* scroll state of the card text (+0x34 of LimitList) */
    int card;                  /* +0x00 card shown, -1 none */
    int unk04;
    unsigned int pos;          /* +0x08 */
    unsigned int max;          /* +0x0C */
} LlScroll;

typedef struct LimitList {     /* ll 0x0880D670 (.bss), 0x5C bytes cleared by limitlist_init */
    unsigned short state;      /* +0x00 0 load, 1 fade in, 2 list, 3/5 menu, 4 card info, 6 fade out, 7 release */
    unsigned short step;       /* +0x02 step of the card info state */
    unsigned short mode : 2;   /* +0x04 argument of limitlist_init */
    unsigned short tab : 4;    /* current list: 0 forbidden, 1 limited, 2 semi-limited */
    unsigned short release : 4; /* release step */
    unsigned short grid : 1;   /* +0x05 bit 2: pictures (11 per row) instead of names */
    unsigned short info : 1;   /* +0x05 bit 3: card info open */
    unsigned short cancelled : 1; /* +0x05 bit 4 */
    unsigned short bits13 : 3;
    unsigned short top[3];     /* +0x06 first visible entry per list */
    unsigned short cursor[3];  /* +0x0C cursor relative to top per list */
    short cur_x, cur_y, cur_w, cur_h; /* +0x12 cursor rectangle */
    unsigned short count[3];   /* +0x1A entries per list */
    LlEntry *list[3];          /* +0x20 */
    void *heap;                /* +0x2C */
    void *cip;                 /* +0x30 card picture reader */
    LlScroll scroll;           /* +0x34 */
    unsigned char pad44[0xC];
    int fpl;                   /* +0x50 own 5 MB block when the caller passes no heap */
    void *fpl_mem;             /* +0x54 */
    int pad58;
} LimitList;

typedef struct LlResFlags {    /* llres_flags 0x0880D660 */
    unsigned char ready : 1;   /* no read in flight */
    unsigned char step : 7;    /* load progress */
} LlResFlags;

typedef struct CardListCfg {   /* BOOT cardlist_init argument, 0x24 */
    void *gim;
    void *digits_gim;
    void *frame_ehf;
    void *attr_gim;
    void *kind_gim;
    void *cil_gim;
    void *icon_gim;
    void *cip;
    int frame_view;
} CardListCfg;

extern LimitList ll;
extern LlResFlags llres_flags;
extern void *llres_cipbuf;     /* 0x0880D650 CIP reader buffer */

/* BOOT (libehsys) */
void *memset(void *, int, unsigned int);
void qsort(void *, unsigned int, unsigned int, int (*)(const void *, const void *));
int sprintf(char *, const char *, ...);
int sceKernelCreateFpl(const char *name, int part, int attr, unsigned int size, unsigned int blocks, void *opt);
int sceKernelAllocateFpl(int fpl, void **data, unsigned int *timeout);
int sceKernelFreeFpl(int fpl, void *data);
int sceKernelDeleteFpl(int fpl);
int sceKernelChangeCurrentThreadAttr(unsigned int clear, unsigned int set);
void *heap_create(void *mem, unsigned int size);
void *heap_alloc(void *h, int n);
int heap_set_level(void *h, void *p);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
void ehp_mount(int slot, void *pack);
void *ehp_find(void *pack, const char *name, int flags);
int cfg_get_language(void);
int region_get(void);
void *cip_open_h(void *handle, int bufsize, int prefetch);
int cip_is_loaded(void *c);
int cip_is_idle(void *c);
void cip_close(void *c);
void cardlist_init(CardListCfg *cfg);
void cardlist_clear(void);
unsigned int bgm_get_status(int id);
void snd_cmd_fefb(void);
void bgm_fade(int x);
void bgm_play(int id);
void bgm_cmd(int arg);
void se_play(int id);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_is_done(void);
int fade_apply(void **list);
void gfx_overlay_enable(int on);

/* this module */
void llres_init(void);
int llres_poll(void);
void llres_free(void);
void *llres_get(unsigned short id);
int limitlist_release(void);
void ll_build(void);
void ll_free_lists(void);
void ll_input(void);
void ll_info_update(void);
void ll_menu_update(void);

#endif
