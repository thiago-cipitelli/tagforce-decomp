/* libduel_draw: the duel scene (scene thread, field, cards, cut-ins, LP panels, duel menus).
   Module state and the BOOT (libehsys) calls no shared header declares. Offsets are the ground
   truth; field names are partial. Each block below belongs to the address range in its marker;
   keep every block compilable. */
#ifndef LIBDUEL_DRAW_H
#define LIBDUEL_DRAW_H

#include "ehsys.h"
#include "game.h"
#include "libc.h"
#include "psp.h"
#include "cip.h"
#include "card_db.h"
#include "duel_setup.h"

/* ---- 0x08804000-0x088143A4 ---- */
typedef struct Model Model;            /* BOOT model (opaque here) */
Model *model_load(void *data, void **anims);
void model_free(Model *m);
void model_anim_set(Model *m, void *data);
void model_set_tgma(Model *m, void *a);
void ehp_mount(int slot, void *pack);
void *ehp_find(void *pack, const char *name, int flags);
int ehf_tex_or_flags(void *ehf, void *flags);
int ehfile_get_size(int dir, const char *name);
void ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehfile_busy(void);
void *gim_set_texture(void *list, void *gim);
int inv_get_hi(unsigned short id);
int region_get(void);
int cfg_get_language(void);

typedef struct CharmapPos {            /* BOOT charmap_lookup2 */
    unsigned char a;
    unsigned char b;
    unsigned char c : 4;
    unsigned char hi : 4;
    unsigned char pad;
} CharmapPos;
int charmap_lookup(int ch, int *kind_out);
int charmap_lookup2(int code, CharmapPos *out);
int charmap_lookup3(int ch);

/* heap of the duel scene (0x0881CA54/0x0881CA64) */
void *func_0881CA54(int size);
void func_0881CA64(void *p);

typedef struct DdModelRes {            /* 3D model of a pack, 0x18 */
    int id;                            /* 0x1xxxx, sorted */
    int pack;                          /* DdPack id, -1 = none */
    const char *tms;
    const char *anim;                  /* .tmt (TGMT) or ptrs.txt */
    const char *tma;
    Model *model;
} DdModelRes;

typedef struct DdPack {                /* EHP pack loaded on demand, 0xB4 */
    int id;                            /* 0x8xxxx */
    const char *name[7][6];            /* [region][language], [0][0] default */
    void *buf;                         /* +0xAC */
    int loading;                       /* +0xB0 */
} DdPack;

typedef struct DdGimRes {              /* texture or binary inside a pack, 0x10 */
    int id;
    int pack;
    const char *name;
    void *data;
} DdGimRes;

typedef struct DdEhfRes {              /* EHF animation inside a pack, 0x14 */
    int id;                            /* 0x2xxxx */
    int pack;
    const char *ehf;
    const char *tmf;
    void *data;
} DdEhfRes;

typedef struct DdCardEnt {             /* card image cache entry, 0x10 */
    int id;
    int pad[3];
} DdCardEnt;

typedef struct DdCards {               /* card images (CIP) of the duel, 0x0897D0D0, 0x2BFCE0 */
    DdCardEnt e[0x1CC];
    int n;                             /* +0x1CC0 */
    int pad1cc4;
    void *cip_h;                       /* +0x1CC8 large images */
    void *cip_m;                       /* +0x1CCC medium images */
    unsigned char buf_h[0x1D2000];     /* +0x1CD0 */
    unsigned char buf_m[0xEC000];      /* +0x1D3CD0 */
    int pad2bfcd0[4];
} DdCards;

typedef struct DdBgSlot {              /* duel field background (duel/bg/fmagicNNNN.ehp), 0x18 */
    int id;
    void *buf;
    int used;
    int loaded;
    int busy;
    int release;                       /* free when the read ends */
} DdBgSlot;

typedef struct DdBg {                  /* 0x0887D0A0 */
    DdBgSlot slot[2];
    unsigned char buf[2][0x80000];
} DdBg;

typedef struct DdCutinSlot {           /* character cut-in pack, 0x20 */
    int id;                            /* charmap character */
    int sub;                           /* variant */
    void *buf;
    int req;                           /* read pending */
    int loaded;
    int faces;                         /* face/voice files still to load */
    int faces_req;                     /* +0x18 */
    int voice_req;                     /* +0x1C */
} DdCutinSlot;

typedef struct DdCutins {              /* 0x0887D010 */
    int n;
    DdCutinSlot slot[4];
    void *heap;                        /* +0x84 */
} DdCutins;

typedef struct DdDuelParam {           /* duel parameters from libduel_mgr, 0x40 */
    int n;                             /* players (2, or 4 in a tag duel) */
    int chr[4];                        /* +0x04 charmap character of each player */
    int sub[4];                        /* +0x14 cut-in variant */
    RecipeData *deck[4];               /* +0x24 */
    int disk[2];                       /* +0x34 duel disk of players 0/1 */
    int first;                         /* +0x3C player who starts */
} DdDuelParam;

typedef struct DdScene {               /* 0x08C3CDBC */
    int active;                        /* duel view running */
    int result;                        /* +0x04 returned to libduel_mgr each frame */
    int flags;                         /* +0x08 dd_scene_flags */
    int eng;                           /* +0x0C libduel_eng.prx module (standalone test) */
    DdDuelParam p;                     /* +0x10 */
    int disk[2];                       /* +0x50 */
    int first;                         /* +0x58 */
    RecipeData decks[4];               /* +0x5C copies of p.deck */
} DdScene;

int efs_set_heap(void *heap);
int efs_term(void);
unsigned int *dl_select_layer(int layer, unsigned int *size);
unsigned int *view_init(unsigned int *list);
int dl_set_list(void *list);
void model_set_allocator(void *(*alloc)(unsigned int size), void (*freefn)(void *p));
void font_set_flag2(int on);

typedef struct DdLoc {                 /* card location: player, zone, index in the zone */
    unsigned short player : 1;
    unsigned short zone : 5;
    unsigned short index : 10;
} DdLoc;

typedef struct DdView {                /* 0x08C3D238, 0x20 */
    int tag;                           /* tag duel view */
    int x04;
    unsigned short x08;
    DdLoc a;                           /* +0x0A highlighted locations */
    DdLoc b;                           /* +0x0C */
    unsigned short x0e;
    int x10;
    int x14;
    int idle[2];                       /* +0x18 frames each player held the input */
} DdView;

typedef struct DdCardObj {             /* card drawn on the field (0x0880B7B8) */
    unsigned char pad00[0x85];
    unsigned char f85_lo : 6;
    unsigned char shown : 1;           /* +0x85 bit 6 */
    unsigned char f85_7 : 1;
    unsigned char pad86[0x8C - 0x86];
    signed char f8c_lo : 6;
    signed char f8c_6 : 1;
    signed char f8c_7 : 1;
    signed char f8d_0 : 1;
} DdCardObj;

typedef struct DdVec3 {
    float x, y, z;
} DdVec3;

typedef struct DdVec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) DdVec4;

typedef struct DdMat {
    float m[16];
} __attribute__((aligned(16))) DdMat;

typedef struct DdLight {               /* BOOT Light, 0x3C */
    int type;                          /* 0 directional, 1 point, 2 spot */
    unsigned int ambient;              /* +0x04 */
    unsigned int diffuse;              /* +0x08 */
    unsigned int specular;             /* +0x0C */
    DdVec3 pos;                        /* +0x10 */
    DdVec3 dir;                        /* +0x1C */
    float spot_exp;                    /* +0x28 */
    float spot_cutoff;                 /* +0x2C */
    float atten[3];                    /* +0x30 */
} DdLight;

typedef struct DdCam {                 /* camera, fog and lights of the field, 0x150 */
    DdVec4 eye;                        /* +0x00 */
    DdVec4 up;                         /* +0x10 */
    DdVec4 at;                         /* +0x20 */
    float fov;                         /* +0x30 */
    float near;                        /* +0x34 */
    float far;                         /* +0x38 */
    int fog;                           /* +0x3C */
    float fog_near;                    /* +0x40 */
    float fog_far;                     /* +0x44 */
    unsigned int fog_color;            /* +0x48 */
    unsigned int ambient;              /* +0x4C */
    int light_on[4];                   /* +0x50 */
    DdLight light[4];                  /* +0x60 */
} DdCam;

typedef struct DdFieldSlot {           /* background slot, 0x28 */
    int id;                            /* fmagic number, -1 none */
    void *tms;                         /* +0x04 from filelist-ptrs.txt */
    void *tmt;                         /* +0x08 */
    void *tma;                         /* +0x0C */
    void *cam;                         /* +0x10 camera/light tracks */
    int state;                         /* +0x14 0 free, 1 loading, 2 fading in, 3 shown, 4 leaving */
    Model *model;                      /* +0x18 */
    float time;                        /* +0x1C */
    int fresh;                         /* +0x20 effect not started yet */
    int fx;                            /* +0x24 effect handle, -1 none */
} DdFieldSlot;

typedef struct DdField {               /* 0x08C3D260, 0x220 */
    DdFieldSlot slot[2];
    int cur;                           /* +0x050 slot shown */
    int next;                          /* +0x054 background requested meanwhile, -1 none */
    int mode;                          /* +0x058 1: default camera */
    int pad5c;
    DdCam cam;                         /* +0x060 */
    int loaded;                        /* +0x1B0 set by the load transition callback */
    int cleared;                       /* +0x1B4 set by the show transition callback */
    int pad1b8[2];
    DdMat view;                        /* +0x1C0 */
    float fov;                         /* +0x200 */
    float aspect;
    float near;
    float far;
    DdVec4 dir;                        /* +0x210 */
} DdField;

typedef struct DdCardStats {           /* BOOT CardStats, 0x3C */
    int attr;                          /* card_get_attribute: 0xD/0xE spell/trap */
    int attr_icon;
    int atk;                           /* -1 = "?" */
    int def;
    int type;
    int kind;
    int level;
    int prop5;
    int extra[6];                      /* +0x20 */
    int mask;                          /* +0x38 */
} DdCardStats;

typedef struct DdInfoView {            /* BOOT CardView plus the card shown, 0x124 */
    int id;                            /* +0x000 -1 none */
    int variant;
    int vram;
    DdCardStats cur;                   /* +0x00C */
    DdCardStats cmp;                   /* +0x048 values on the field */
    int show_mask;                     /* +0x084 */
    unsigned int desc_scroll;          /* +0x088 */
    unsigned int desc_max;             /* +0x08C */
    int shown_id;                      /* +0x090 */
    int shown_variant;
    int fade;
    DdCardStats shown;                 /* +0x09C */
    DdCardStats shown_cmp;             /* +0x0D8 */
    int shown_mask;                    /* +0x114 */
    unsigned short uid;                /* +0x118 card instance on the field, 0 none */
    int card;                          /* +0x11C card id */
    int on;                            /* +0x120 */
} DdInfoView;

typedef struct DdInfo {                /* card info panel(s), 0x08C3D480, 0x4B0 */
    DdInfoView view[2];
    DdInfoView save[2];                /* +0x248 */
    int pinned;                        /* +0x490 opened by the player */
    int peek;                          /* +0x494 opened while browsing the field */
    int off;                           /* +0x498 disabled */
    void *frame_buf;                   /* +0x49C */
    int state;                         /* +0x4A0 0 closed .. 5 */
    int target;                        /* +0x4A4 0 single, 3 pair */
    float timer;                       /* +0x4A8 */
    int x;                             /* +0x4AC slide offset */
} DdInfo;

extern DdInfo dd_info;
extern DdField dd_field;
extern DdScene dd_scene;
extern DdView dd_view;
extern DdModelRes dd_models[0x26];
extern DdPack dd_packs[4];
extern DdGimRes dd_gims[0x23];
extern DdEhfRes dd_ehfs[0xD];
extern DdGimRes dd_bins[0x23];
extern DdCards dd_cards;
extern DdBg dd_bg;
extern DdCutins dd_cutins;

int dd_res_cmp(const void *a, const void *b);
int dd_u16_cmp(const void *a, const void *b);
int dd_res_find(unsigned int id, void *table, int n, int size);
Model *dd_model_get(int id);
void *dd_ehf_get(int id);
void *dd_gim_get(int id);
void *dd_bin_get(int id);
void *dd_pack_get(int id);
void dd_test_main(void);
void dd_test_exit(void);
int dd_scene_flags(int mask, int on);
void dd_view_draw(void);
PadState *dd_pad_get(void);
void dd_view_clear(void);
void dd_view_term(void);
void dd_view_tag_set(int tag);
int dd_is_tag_duel(void);
void dd_view_locs_get(DdLoc *a, DdLoc *b);
void *dd_bg_get(int id);
void dd_bg_release(int id);
int dd_bg_filelist(int id, int *a, int *b, int *c, int *d);
void *dl_clear_color(void *list, unsigned int rgba);
void *dl_clear_depth(void *list, int z);
void *dl_clear_stencil(void *list, unsigned int s);
void *dl_clear(void *list, int flags);
unsigned int *dl_disable(unsigned int *list, int state);
void *dl_enable(void *list, int state);
void dd_res_sort(void);
void dd_res_term(void);
void dd_res_release(void);
void dd_cards_reset(void);
void dd_cards_add_deck(RecipeData *deck);
int dd_cards_ready_m(void);
int dd_cards_ready_h(void);
void dd_cutin_setup(int *ids, int *subs, int n);
void dd_cutin_free(void);
void dd_cutin_load(void);
int dd_cutin_ready(void);

#endif
