/* libconv_machine: "Card Convert Machine" sub-screen (labo): up to N cards picked with libselect_card
   are fed to the machine and one random card of the matching group comes out. */
#ifndef LIBCONV_MACHINE_H
#define LIBCONV_MACHINE_H

#include "ehsys.h"
#include "font.h"

#define CM_STRINGS 0x1030       /* strTbl.bin */

typedef struct GfxState {
    unsigned int frame;
    int fps;
} GfxState;

typedef struct Model {          /* BOOT model, 0x68 */
    void *tms;
    void **anims;
    int nanims;
    void *bones;
    void *skin;
    void *cur;
    void *prev;
    int index;
    float frame;                /* +0x20 */
    int prev_index;
    float prev_frame;
    void *mats;
    void *texs;
    void *tgma;
    float tgma_time;
    unsigned int flags;         /* +0x3C */
    int rest[10];
} Model;

typedef struct ModelDraw {      /* BOOT model_draw options, 0x34 */
    int layer[2];
    int pad[10];
    void *user;                 /* +0x30 */
} ModelDraw;

typedef struct CmCard {         /* 0x14, one card flying into the machine */
    float x, y, z;
    int state;                  /* 1 inserting, 2 in, 3 burning, 4 gone */
    int t;
} CmCard;

typedef struct CmOut {          /* 0x18, the card coming out of the machine */
    float x, y, z;
    int shown;
    float rx, rz;               /* degrees */
} CmOut;

typedef struct CmState {        /* cm 0x08E0D330, 0xCE4 bytes cleared by conv_init */
    unsigned short state;       /* +0x000 */
    unsigned short step;        /* +0x002 */
    unsigned short term;        /* +0x004 release step */
    unsigned short pad006;
    int n_cards;                /* +0x008 cards chosen */
    unsigned short cards[100];  /* +0x00C */
    unsigned short result;      /* +0x0D4 card that comes out */
    unsigned char glow;         /* +0x0D6 */
    unsigned char zoom;         /* +0x0D7 */
    unsigned short name_w;      /* +0x0D8 */
    unsigned short name_x;      /* +0x0DA */
    unsigned char name_size;    /* +0x0DC */
    unsigned char pad0dd;
    unsigned short text_len;    /* +0x0DE glyphs of the message */
    unsigned short text_pos;    /* +0x0E0 glyphs shown */
    unsigned short text[0x200]; /* +0x0E2 */
    unsigned char typing : 1;   /* +0x4E2 */
    unsigned char pad4e2 : 7;
    unsigned char pad4e3;
    void *heap;                 /* +0x4E4 */
    void *cip;                  /* +0x4E8 */
    unsigned short timer;       /* +0x4EC */
    unsigned short pad4ee;
    int modid;                  /* +0x4F0 libselect_card */
    unsigned char selecting : 1; /* +0x4F4 libselect_card is drawing */
    unsigned char lid : 2;      /*        lid animation: 1 opening, 2 closing */
    unsigned char lid_done : 1;
    unsigned char detail : 1;
    unsigned char pad4f4 : 3;
    unsigned char flash;        /* +0x4F5 */
    unsigned char flash_fade : 2; /* +0x4F6 1 in, 2 out */
    unsigned char name_shown : 1;
    unsigned char pad4f6 : 5;
    unsigned char pad4f7;
    CmCard flying[100];         /* +0x4F8 */
    CmOut out;                  /* +0xCC8 the card coming out */
    unsigned char dim;          /* +0xCE0 */
    unsigned char pad0ce1[3];
} CmState;

typedef struct CmResFlags {     /* cm_res 0x08E78020 */
    unsigned char ready : 1;
    unsigned char step : 7;
} CmResFlags;

extern CmState cm;
extern CmResFlags cm_res;
extern unsigned char cm_heap_mem[0x600000];
extern unsigned char cm_cip_mem[0x6A000];

/* BOOT (libehsys) */
void *memset(void *, int, unsigned int);
void *memcpy(void *, const void *, unsigned int);
void qsort(void *, unsigned int, unsigned int, int (*)(const void *, const void *));
int sprintf(char *, const char *, ...);
int sceKernelChangeCurrentThreadAttr(unsigned int clear, unsigned int set);
int sceKernelStartModule(int modid, unsigned int argsize, void *argp, int *status, void *option);
int sceKernelStopModule(int modid, unsigned int argsize, void *argp, int *status, void *option);
int sceKernelUnloadModule(int modid);
void *heap_create(void *mem, unsigned int size);
void *heap_alloc(void *h, int n);
void heap_free_block(void *h);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehfile_request_module(int dir, const char *name, int (*cb)(void *, int), void *arg);
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
int cip_is_loaded(void *c);
void cip_close(void *c);
int cip_is_idle(void *c);
void cardlist_clear(void);
Model *model_load(void *data, void **anims);
void model_anim_set(Model *m, void *data);
void model_set_tgma(Model *m, void *a);
int snd_count_se();
void snd_cmd_fefa(void);
void bgm_fade(int x);
void bgm_play(int id);
void bgm_cmd3(int x);
unsigned int *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(unsigned int *list);

/* this module */
int cm_term(void);
void cm_res_init(void);
int cm_load_poll(void);
void *cm_pack_get(unsigned short id);
void *cm_res_get(unsigned short id);
Model *cm_model_get(unsigned short id);
void cm_gfx_init(void);
void cm_lid_close(void);
void cm_state_intro(void);
void cm_state_select(void);
void cm_state_insert(void);
void cm_state_convert(void);
void cm_state_result(void);
void cm_state_ask(void);

#endif
