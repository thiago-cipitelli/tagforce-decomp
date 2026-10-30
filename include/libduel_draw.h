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
int func_08806D04(void);

#endif
