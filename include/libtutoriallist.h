/* libtutoriallist: tutorial (lesson) list sub-screen, imported by libtitle/libstory/liblabo. */
#ifndef LIBTUTORIALLIST_H
#define LIBTUTORIALLIST_H

#include "font.h"
#include "ehsys.h"

#define TL_LESSONS 24          /* lesson ids 0..23; 0..7 = tab 1 ("basic"), 8..23 = tab 0 */
#define TL_NONE    24          /* id used past the end of a list */
#define TL_ROWS    8           /* visible rows of the list window */

typedef struct TutorialList {  /* tl 0x0880A190 (.bss), cleared by tlist_init (0xB8 bytes) */
    unsigned short state;      /* 0 load, 1 fade in, 2 input, 3 fade out, 4 done */
    unsigned short pad02;
    unsigned char tab : 4;     /* +0x04 current tab (0 or 1) */
    unsigned char cancelled : 1; /* +0x04 bit 4: left with cancel */
    unsigned char pad04 : 3;
    unsigned char pad05;
    unsigned short selected;   /* +0x06 lesson chosen (also the one to preselect on entry) */
    unsigned short top[2];     /* +0x08 first visible row per tab */
    unsigned short cursor[2];  /* +0x0C cursor row relative to top, per tab */
    unsigned short count[2];   /* +0x10 lessons in each tab */
    unsigned short list[2][TL_LESSONS]; /* +0x14 lesson ids per tab */
    unsigned short lines[TL_LESSONS + 1]; /* +0x74 rows each lesson's title takes; [TL_NONE] = 1 */
    unsigned short pad_a6;
    void *heap;                /* +0xA8 */
    int fpl;                   /* +0xAC own 2 MB block when the caller passes no heap */
    void *fpl_mem;             /* +0xB0 */
    int pad_b4;
} TutorialList;

typedef struct TlResFlags {    /* tlres_flags 0x0880A250 */
    unsigned char ready : 1;   /* no read in flight */
    unsigned char step : 7;    /* load progress */
} TlResFlags;

extern TutorialList tl;
extern TlResFlags tlres_flags;
extern void *tlres_pack;

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
int ehp_mount(void (*cb)(void *), void *pack);
void *ehp_find(void *pack, const char *name, int *size);
int cfg_get_language(void);
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
unsigned char sysstate_get_byte_97d0(int i);
unsigned short region_pick_value(int flag);
unsigned short *table_get_entry(void *t, unsigned int i);

/* this module */
int tlist_release(void);
unsigned int *tlist_draw_bg(unsigned int *list);
unsigned int *tlist_draw_header(unsigned int *list);
unsigned int *tlist_draw_tabs(unsigned int *list, int x, int y, int w, int h);
unsigned int *tlist_draw_entry(unsigned int *list, int x, int y, int id);
unsigned int *tlist_draw_frame(unsigned int *list, int x, int y, int w, int h);
unsigned int *tlist_draw_cursor(unsigned int *list, int x, int y, int w, int h);
unsigned int *tlist_draw_scrollbar(unsigned int *list, int x, int y, int h, int pos, int page, int total);
void tlist_draw_number(int x, int y, int n);
void tlres_init(void);
int tlres_poll(void);
void *tlres_load(void);
void tlres_free(void);
void *tlres_get(unsigned short id);
void tlist_build(void);
void tlist_input(void);

#endif
