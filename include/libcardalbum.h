/* libcardalbum: card album sub-screen (a 3D binder of 18-card pages), imported by libtitle/libstory/liblabo. */
#ifndef LIBCARDALBUM_H
#define LIBCARDALBUM_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"

#define CA_PER_PAGE 18         /* cards per page: 9 on the left side, 9 on the right */
#define CA_PAGES(n) (((n) + 17) / 18)

/* resource ids (ca_files: textures/strings, ca_models: tms models) */
#define CA_BG        0x1001    /* bg.gim */
#define CA_MISC      0x1002    /* misc.gim */
#define CA_HAND      0x1003    /* hand.gim: the pointing hand */
#define CA_STRINGS   0x1030    /* strTbl_%c.bin */
#define CA_SKIPMENU  0x1010    /* skipmenu.ehf */
#define CA_COVER     0x1020    /* album_cover00 */
#define CA_PAGE0     0x1021
#define CA_PAGE1     0x1022
#define CA_PAGE2     0x1023
#define CA_CURSOR    0x1024

typedef struct Model {         /* BOOT model, 0x68 */
    void *tms;
    void **anims;
    int nanims;
    void *bones;
    void *skin;
    void *cur;
    void *prev;
    int index;
    float frame;               /* +0x20 */
    int prev_index;
    float prev_frame;
    void *mats;
    void *texs;
    void *tgma;
    float tgma_time;
    unsigned int flags;        /* +0x3C 2 = loop */
    int buf_size;
    void *buf[2];
    int len[2];
    void *hooks[5];            /* +0x54 */
} Model;

typedef struct ModelDraw {     /* BOOT model_draw options, 0x34 */
    int layer[2];              /* display list layers of the two passes */
    int pad[10];
    void *user;                /* +0x30 passed to the hooks */
} ModelDraw;

typedef struct GfxState {
    unsigned int frame;        /* frame counter */
    int fps;                   /* +0x04 30 or 60 */
} GfxState;

typedef struct CaPage {        /* 0xC0 heap block: one page of the binder, in a list sorted by z */
    unsigned char kind : 4;    /* +0x00 animation (ca_kinds) */
    unsigned char slot : 4;    /*        position in the stack, 0..7 (ca_slots) */
    unsigned char live : 1;    /* +0x01 */
    unsigned char pad01 : 7;
    unsigned short page : 15;  /* +0x02 page number */
    unsigned short pad02 : 1;
    unsigned short cards[CA_PER_PAGE]; /* +0x04 */
    float pos[3];              /* +0x28 */
    int pad34[3];
    Model model;               /* +0x40 own copy of the page model */
    float speed;               /* +0xA8 animation step, 0 = still */
    struct CaPage *prev;       /* +0xAC */
    struct CaPage *next;       /* +0xB0 */
    int padb4[3];
} CaPage;

typedef struct CaCursor {      /* +0x1360 */
    unsigned char area : 2;    /* 0 cards, 1 page corners, 2 menu */
    unsigned char pos : 6;     /* card cell 0..35 (6 columns x 6 rows over both sides) */
    unsigned char item : 4;    /* +0x1361 menu entry 0..5 */
    unsigned char side : 2;    /* page corner 0 left, 1 right */
    unsigned char pad : 2;
} CaCursor;

typedef struct CaJump {        /* +0x1370 animated jump to a page */
    unsigned char active : 1;
    unsigned char pos : 6;     /* cursor cell to land on */
    unsigned char pad : 1;
    unsigned char pad1;
    unsigned short page;       /* +0x1372 */
    float speed;               /* +0x1374 */
} CaJump;

typedef struct CaPanel {       /* BOOT CardPanel: the card shown on the right panel */
    int id;
    int vram;
    int scroll;
    int desc_max;
    int shown_id;
    int shown_variant;
    int fade;
} CaPanel;

typedef struct CardAlbum {     /* ca 0x0880DD20, 0x13B0 bytes cleared by cardalbum_init */
    unsigned short state;      /* +0x00 0 load, 1 fade in, 2 cover opening, 3 browse, 4 turning, 5 index, 6 detail, 8 exit */
    unsigned short step;       /* +0x02 */
    unsigned char mode : 2;    /* +0x04 init argument */
    unsigned char book : 2;    /*        cover: 0 closed, 1 opening, 2 open, 3 closing */
    unsigned char pad04 : 4;
    CaPage *pages;             /* +0x08 */
    unsigned short n_cards;    /* +0x0C entries of the normal part (owned or not) */
    unsigned short n_total;    /* +0x0E plus the extra pages after it */
    unsigned short cards[0x993]; /* +0x10 card ids, 0 = empty slot */
    unsigned short pad1336[18];
    unsigned short permille;   /* +0x135A owned cards in per mille */
    float book_x;              /* +0x135C camera shift while the cover opens */
    CaCursor cur;              /* +0x1360 */
    unsigned int held;         /* +0x1364 button held for fast turning */
    float speed_fwd;           /* +0x1368 */
    float speed_back;          /* +0x136C */
    CaJump jump;               /* +0x1370 */
    unsigned char detail : 1;  /* +0x1378 card detail screen open */
    unsigned char pad1378 : 7;
    CaPanel panel;             /* +0x137C */
    void *heap;                /* +0x1398 */
    void *cip_h;               /* +0x139C card image readers */
    void *cip_m;               /* +0x13A0 */
    int fpl;                   /* +0x13A4 own 8 MB block when the caller passes no heap */
    void *fpl_mem;             /* +0x13A8 */
    unsigned short timer;      /* +0x13AC */
} CardAlbum;

typedef struct CaRes {         /* ca_res 0x0880F0E0 */
    unsigned char ready : 1;   /* no read in flight */
    unsigned char step : 7;
    unsigned char pad;
    unsigned short queue[18];  /* card pictures to prefetch */
} CaRes;

typedef struct CaIndex {       /* ca_index 0x0880F144: the "jump to letter" window */
    unsigned char pad0[2];
    unsigned char col : 4;     /* +0x02 */
    unsigned char row : 4;
    unsigned char on_extra : 1; /* +0x03 cursor on the extra-pages button */
    unsigned char pad3 : 7;
    unsigned char *has;        /* +0x04 letters present among the owned cards */
    unsigned char extra : 1;   /* +0x08 the album has extra pages */
    unsigned char pad8 : 7;
    unsigned char *layout;     /* +0x0C letter of each cell, 0xFF = none */
    unsigned char cols;        /* +0x10 */
    unsigned char rows;        /* +0x11 */
    MenuWidget *menu;          /* +0x14 */
} CaIndex;

extern CardAlbum ca;
extern void *ca_pack;          /* database/cardalbum.ehp */
extern void *ca_cip_h_mem;
extern void *ca_cip_m_mem;
extern CaRes ca_res;

/* BOOT (libehsys) */
void *memset(void *, int, unsigned int);
void *memcpy(void *, const void *, unsigned int);
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
int region_get(void);
unsigned short region_pick_value(int flag);
void se_play(int id);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_is_done(void);
int fade_apply(void **list);
void gfx_overlay_enable(int on);
unsigned short *table_get_entry(void *t, unsigned int i);
void *cip_open_h(void *handle, int bufsize, int prefetch);
void *cip_open_m(void *handle, int bufsize, int prefetch);
int cip_is_loaded(void *c);
void cip_close(void *c);
int cip_is_idle(void *c);
int cip_query(void *handle, int id, int variant);
unsigned int *cip_get_image(unsigned int *list, void *handle, int id, int variant, int *loaded_out);
int inv_get_hi(unsigned short id);
int inv_get_state(unsigned short id);
void cardlist_init(void *cfg);
void cardlist_clear(void);
void *cardlist_build(void *list, CaPanel *p);
Model *model_load(void *data, void **anims);
void model_free(Model *m);
void model_anim_set(Model *m, void *data);
void model_set_tgma(Model *m, void *a);
void model_set_allocator(void *(*alloc)(unsigned int size), void (*freefn)(void *p));
void model_update(Model *m, float *dt);
void model_draw(Model *m, ModelDraw *c);
void model_set_mask(Model *m, unsigned int mask);
void *model_hook(Model *m, int which, void *v);
unsigned int *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(unsigned int *list);
unsigned int *gim_set_texture(unsigned int *list, void *gim);

/* this module */
int ca_term(void);
void ca_res_init(void);
int ca_load_poll(void);
void ca_res_free(void);
void ca_res_free_all(void);
void *ca_pack_load(void);
void *ca_res_get(unsigned short id);
Model *ca_model_get(unsigned short id);
void ca_queue_clear(void);
void ca_queue_add(unsigned short id);
void ca_queue_prefetch(void);
void ca_gfx_init(void);
void ca_book_open(void);
void ca_build(void);
void ca_input(void);
void ca_turn_update(void);
void ca_index_update(void);
void ca_detail_update(void);
void ca_prefetch_update(void);
void ca_skipmenu_anim(int which);
void ca_pages_free(void);

#endif
