/* librecipe_viewer: deck recipe viewer sub-screen (imported by libtitle/libstory/libdeck).
   Module state and the BOOT functions it calls that no shared header declares. */
#ifndef LIBRECIPE_VIEWER_H
#define LIBRECIPE_VIEWER_H

#include "font.h"
#include "ehsys.h"

typedef struct RvScroll {      /* scroll state of the card text */
    int card;                  /* +0x00 card shown, -1 none */
    int unk04;
    unsigned int pos;          /* +0x08 */
    unsigned int max;          /* +0x0C */
} RvScroll;

typedef struct RvEntry {       /* one card of the recipe, 6 bytes */
    unsigned short id;         /* card id */
    unsigned short file;       /* index in fileTbl.dat */
    unsigned char lines : 7;   /* +0x04 copies */
    unsigned char flag7 : 1;   /* new card */
    unsigned char b5_0 : 1;    /* +0x05 */
    unsigned char owned : 1;
    unsigned char b5_hi : 6;
} RvEntry;

typedef struct RvGrid {        /* the shown recipe (+0x18 of RecipeViewer) */
    unsigned char col;         /* +0x00 cursor in the recipe grid */
    unsigned char row;         /* +0x01 0..3 main, 4..5 side, 6 extra */
    unsigned short main_cols;  /* +0x02 cards per row of the main deck */
    unsigned short side_cols;  /* +0x04 cards per row of the side deck */
    unsigned short n_main : 7; /* +0x06 */
    unsigned short n_extra : 4;
    unsigned short n_side : 5;
    RvEntry main[80];          /* +0x08 */
    RvEntry extra[15];         /* +0x1E8 */
    RvEntry side[30];          /* +0x242 */
} RvGrid;

typedef struct RvPos {         /* list position of a tab */
    unsigned short top;
    unsigned short cursor;
} RvPos;

typedef struct RvRecipe {      /* one recipe (or saved deck) of the lists, 8 bytes */
    unsigned short index;      /* +0x00 recipe number (1..350) or saved deck slot */
    unsigned short number;     /* +0x02 running number among the existing recipes */
    unsigned char kind;        /* +0x04 DeckName.val (recipe author) */
    unsigned char have;        /* +0x05 different cards owned */
    unsigned char need;        /* +0x06 different cards in the recipe */
    unsigned char pad7;
} RvRecipe;

typedef struct RvExtList {     /* a list read from the extra data, 0x208 bytes */
    unsigned short name[0x104];
} RvExtList;

typedef struct RvParams {      /* recipe_viewer_init argument */
    void *heap;
    void *cip;
    void *deck_pack;
    int unk0c;
    int unk10;
} RvParams;

typedef struct RecipeViewer {  /* rv 0x0880EBE0 (.bss), 0x17E8 bytes cleared by recipe_viewer_init */
    unsigned short mode : 4;   /* +0x00 argument of recipe_viewer_init */
    unsigned short state : 8;  /* 0 load, 1 build, 2 fade in, 3 list, 4 recipe, 5 menu, 6 fade out, 7 release */
    unsigned short bits12 : 4;
    unsigned char release;     /* +0x02 release step */
    unsigned char pad03;
    unsigned short cancelled : 1; /* +0x04 */
    unsigned short busy : 1;
    unsigned short osk : 8;    /* name entry (on-screen keyboard) step */
    unsigned short info : 1;   /* card info open */
    unsigned short info_open : 1; /* BOOT card info screen drawn */
    unsigned short no_frame : 1; /* +0x05 bit 4: no cursor frame */
    unsigned short loaded : 1; /* resources ready */
    unsigned short bits14 : 2;
    unsigned char pad06[4];
    unsigned char tab : 2;     /* +0x0A 0 saved decks, 1 recipes, 2 extra data lists */
    unsigned char bits0a : 6;
    unsigned char pad0b;
    RvPos pos[3];              /* +0x0C */
    RvGrid grid;               /* +0x18 */
    unsigned char shown;       /* +0x30E cards revealed so far (animation) */
    unsigned char pad30f;
    RvScroll scroll;           /* +0x310 */
    unsigned char pad320[0xC];
    unsigned short n_recipes;  /* +0x32C */
    RvRecipe recipes[351];     /* +0x32E */
    RvRecipe decks[50];        /* +0xE26 saved decks */
    unsigned short padfb6;
    struct RvExtList *ext;     /* +0xFB8 lists from the extra data */
    int n_ext;                 /* +0xFBC */
    RvRecipe ext_info[0xF];    /* +0xFC0 */
    unsigned char pad1038[0x17C0 - 0x1038];
    unsigned short n_type[6];  /* +0x17C0 cards per type in the shown recipe */
    void *heap;                /* +0x17CC */
    void *cip;                 /* +0x17D0 card picture reader */
    void *deck_pack;           /* +0x17D4 deck/all pack already in memory (from the caller), or 0 */
    int unk17d8;
    int unk17dc;
    int fpl;                   /* +0x17E0 own 4 MB block when the caller passes no heap */
    void *fpl_mem;             /* +0x17E4 */
} RecipeViewer;

typedef struct RvResFlags {    /* rvres_flags 0x0880E900 */
    unsigned char ready : 1;   /* no read in flight */
    unsigned char step : 7;    /* load progress */
} RvResFlags;

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

typedef struct GfxState {      /* gfx_get_state() */
    unsigned int frame;        /* +0x00 frame counter */
    int fps;                   /* +0x04 30 or 60 */
} GfxState;

extern RecipeViewer rv;
extern RvResFlags rvres_flags;
extern void *rvres_cipbuf;     /* 0x0880E8F0 CIP reader buffer */

/* BOOT (libehsys) */
void *memset(void *, int, unsigned int);
void *memcpy(void *, const void *, unsigned int);
void qsort(void *, unsigned int, unsigned int, int (*)(const void *, const void *));
int sprintf(char *, const char *, ...);
int sceKernelCreateFpl(const char *name, int part, int attr, unsigned int size, unsigned int blocks, void *opt);
int sceKernelAllocateFpl(int fpl, void **data, unsigned int *timeout);
int sceKernelFreeFpl(int fpl, void *data);
int sceKernelDeleteFpl(int fpl);
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
void snd_cmd_fefb(void);
void bgm_fade(int x);
void bgm_play(int id);
void bgm_cmd(int arg);
void se_play(int id);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_is_done(void);
int fade_apply(void **list);
GfxState *gfx_get_state(void);
PadState *pad_get(void);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *gim_set_texture(void *list, void *gim);

/* this module */
void rvres_init(void);
int rvres_poll(void);
void rvres_free(void);
void *rvres_get(unsigned short id);

#endif
