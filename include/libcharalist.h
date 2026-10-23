/* libcharalist: "List of Duelists" sub-screen (character profiles, portraits, voice test), imported by
   libtitle/libstory/liblabo; libstory also uses it to pick a random duelist. */
#ifndef LIBCHARALIST_H
#define LIBCHARALIST_H

#include "ehsys.h"
#include "font.h"

#define CL_CHARAS  41          /* entries of the character table */
#define CL_STRINGS 0x1020      /* strTbl_%c.bin */

typedef struct GfxState {
    unsigned int frame;        /* frame counter */
    int fps;                   /* +0x04 30 or 60 */
} GfxState;

typedef struct ClInfo {        /* 0x28 heap block: what the list shows of a duelist */
    unsigned short n : 2;      /* +0x00 costumes (bust-ups) */
    unsigned short cur : 2;    /*        costume shown */
    unsigned short sel : 2;    /*        default costume */
    unsigned short voice : 1;  /*        has a voice test */
    unsigned short face : 8;   /*        face icon (main_face01.gim cell) */
    unsigned short pad0 : 1;
    unsigned short pad2;
    const char *bustup[3];     /* +0x04 field/bustup/bu_<name>.ehp.gz per costume */
    unsigned char rank[3];     /* +0x10 per costume */
    unsigned char pad13;
    unsigned short *text[3];   /* +0x14 profile per costume */
    unsigned char bgm[3];      /* +0x20 index in cl_bgm per costume */
    unsigned char pad23[5];
} ClInfo;

typedef struct ClChara {       /* 0x18, character table (cl_charas) */
    union {
        unsigned short raw;
        struct {
            unsigned short id : 10;
            unsigned short rank : 3;
            unsigned short n : 2;      /* costumes */
            unsigned short voice : 1;  /* has a voice test */
        } f;
    } u;
    unsigned short pad2;
    const char *bustup[3];     /* +0x04 */
    union {
        int raw;
        unsigned char face;    /* +0x10 face icon of the first costume */
    } face;
    int pad14;
} ClChara;

typedef struct ClBgm {         /* voice set: theme and base of its voice-test se */
    unsigned short bgm;
    unsigned short se;
    unsigned short se_end;
} ClBgm;

typedef struct ClEntry {       /* 0x10, one line of a tab */
    unsigned short id : 10;    /* +0x00 character */
    unsigned short pad0 : 6;
    unsigned short num : 10;   /* +0x02 number shown */
    unsigned short simple : 1; /*        tab 2 (no profile): rank and card id only */
    unsigned short pad2 : 5;
    int pad4;
    union {
        ClInfo *info;          /* +0x08 */
        struct {
            unsigned char rank : 3;
            unsigned char pad : 5;
            unsigned char pad9;
            unsigned short card;   /* +0x0A */
        } s;
    } u;
    int padc;
} ClEntry;

typedef struct CharaList {     /* cl 0x0880EE90, 0x180 bytes cleared by charalist_init */
    unsigned char mode : 4;    /* +0x00 0/1 title or story, 2 random pick, 3 labo */
    unsigned char sub : 2;
    unsigned char pad00 : 2;
    unsigned char pad01;
    unsigned short state;      /* +0x02 */
    unsigned short step;       /* +0x04 */
    unsigned char tab : 2;     /* +0x06 */
    unsigned char pad06 : 6;
    unsigned char pad07;
    unsigned short top[3];     /* +0x08 first visible line per tab */
    unsigned short cursor[3];  /* +0x0E cursor line relative to top */
    unsigned short voice;      /* +0x14 voice test cursor */
    unsigned short count[3];   /* +0x16 */
    ClEntry *list[3];          /* +0x1C */
    int scroll;                /* +0x28 profile text scroll */
    int scroll_max;            /* +0x2C */
    unsigned char cancelled : 1; /* +0x30 */
    unsigned char pad30 : 7;
    int chosen;                /* +0x34 character picked, -1 none */
    int costume;               /* +0x38 */
    void *heap;                /* +0x3C */
    int fpl;                   /* +0x40 */
    void *fpl_mem;             /* +0x44 */
    unsigned short timer;      /* +0x48 */
    unsigned short anim;       /* +0x4A detail slide in (0..12) */
    unsigned short slide;      /* +0x4C bust-up position */
    unsigned char fade : 2;    /* +0x4E costume cross-fade: 1 out, 2 in */
    unsigned char pad4e : 6;
    unsigned char alpha;       /* +0x4F */
    int bgm;                   /* +0x50 theme playing, -1 none */
    unsigned char bgm_wait : 1; /* +0x54 waiting for the theme to load */
    unsigned char se_wait : 1; /*        second half of a voice pending */
    unsigned char pad54 : 6;
    unsigned char pad55;
    unsigned short se_next;    /* +0x56 */
    unsigned short volume;     /* +0x58 bgm volume to restore */
    unsigned char vol_fade : 2; /* +0x5A 1 down, 2 up */
    unsigned char pad5a : 6;
    unsigned char pad5b;
    unsigned short n_voices;   /* +0x5C */
    unsigned short voices[145]; /* +0x5E indexes in cl_voice_se */
} CharaList;

typedef struct ClBust {        /* cl_bust 0x0880EA94: the bust-up packs (gzipped) and the one unpacked */
    unsigned char ready : 1;   /* no read in flight */
    unsigned char pad0 : 7;
    void *data[CL_CHARAS][3];  /* +0x004 */
    int size[CL_CHARAS][3];    /* +0x1F0 */
    unsigned short id : 12;    /* +0x3DC unpacked one */
    unsigned short var : 3;
    unsigned short pad3dc : 1;
    void *mem;                 /* +0x3E0 */
    void *ehf[2];              /* +0x3E4 body, mouth */
} ClBust;

typedef struct ClResFlags {    /* cl_res 0x0880EE80 */
    unsigned char ready : 1;
    unsigned char step : 7;
} ClResFlags;

extern CharaList cl;
extern ClBust cl_bust;
extern ClResFlags cl_res;
extern void *cl_pack;          /* database/charalist.ehp */

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
int ehfile_busy(void);
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
int ehf_set_anim(void *e, int target, int limit);
unsigned int *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(unsigned int *list);

/* this module */
void cl_res_init(void);
int cl_load_poll(void);
int cl_res_release(void);
void charalist_res_free(void);
void *cl_pack_load(void);
void *cl_res_get(unsigned short id);
void *cl_bustup_get(ClEntry *e, unsigned short which);
void *cl_voicetbl_get(unsigned short id, unsigned short var);
int cl_term(void);
void cl_list_build(void);
void cl_input(void);
void cl_detail_update(void);
ClChara *cl_chara_find(unsigned short id);

/* line i of tab t, 0 past the end */
inline ClEntry *cl_entry(int t, int i)
{
    if (i >= cl.count[t])
        return 0;
    return &cl.list[t][i];
}

#endif
