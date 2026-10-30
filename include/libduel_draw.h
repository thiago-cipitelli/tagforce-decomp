/* libduel_draw: the duel scene (field, cards, effects, cut-ins, LP panels, duel menus).
   Each block below belongs to the address range in its marker; keep every block compilable. */
#ifndef LIBDUEL_DRAW_H
#define LIBDUEL_DRAW_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"
#include "card_db.h"
#include "libc.h"

/* ---- 0x08857100-0x0886B25C ---- */
typedef struct DdVec4 {
    float x, y, z, w;
} DdVec4;

typedef struct DdVec4A {               /* stack vector, 16-aligned (VFPU) */
    float x, y, z, w;
} __attribute__((aligned(16))) DdVec4A;

typedef struct DdMat4 {
    float m[16];
} __attribute__((aligned(16))) DdMat4;

typedef struct ChrBone {               /* rest pose of one bone of a character model, 0x20 */
    int id;
    float px, py, pz;
    DdVec4 rot;                        /* quaternion */
} ChrBone;

typedef struct ChrVt {                 /* callbacks of a character model (0x0884DC84 picks the setup) */
    char pad[20];
    int (*init)(void *self, void *a, void *b);                      /* 0x14 */
    int (*event)(void *self, int ev, void *a, void *b);              /* 0x18 */
    int (*update)(void *self, void *a, void *b, void *c, void *d);   /* 0x1C */
    void *f20;
    int (*bone_id)(void *self, int i);                              /* 0x24 slot -> bone id */
    int (*bone_rest)(void *self, DdVec4 *pos, DdVec4 *rot, int id); /* 0x28 */
    int (*motion)(void *self, void *a, void *b, void *c, int d);     /* 0x2C */
    int *pairs;                                                     /* 0x30 (bone, x) pairs, 0-terminated */
} ChrVt;

typedef struct ChrModel {              /* character model instance (partial) */
    int chr_id;                        /* 0x00 */
    int f4;                            /* 0x04 costume */
    char pad8[0x2C];
    int snd;                           /* 0x34 sound channel */
    char pad38[0x28];
    int voices[4][4];                  /* 0x60 voice clips per step, -1 = none */
    int voice_step;                    /* 0xA0 */
    int voice_a4;                      /* 0xA4 */
    int voice_playing;                 /* 0xA8 */
    char padAC[0xA4];
    int card_id;                       /* 0x150 card the event is about */
    char pad154[0x20];
    unsigned int flags;                /* 0x174 */
    unsigned int f178;                 /* 0x178 */
} ChrModel;

typedef struct DdCharPos {             /* BOOT charmap_lookup2 output */
    unsigned char kind;
    unsigned char sub;
    unsigned char c;
    unsigned char d;
} DdCharPos;

int charmap_lookup(int ch, int *kind_out);
int charmap_lookup2(int code, DdCharPos *out);

typedef struct DdLoc {                 /* card location on the field (u16) */
    unsigned short side : 1;
    unsigned short zone : 5;
    unsigned short index : 10;
} DdLoc;

typedef struct DdPopupArg {
    int card;
    void (*get_loc)(DdLoc *loc);
} DdPopupArg;

typedef struct DdPopup {               /* 0x08C5B988, 0x24: the action menu of a field card */
    DdPopupArg arg;                    /* +0x00 */
    MenuWidget *menu;                  /* +0x08 */
    int items[4];                      /* +0x0C */
    int count;                         /* +0x1C */
    int text_w;                        /* +0x20 */
} DdPopup;

extern DdPopup popup;

/* BOOT display list / matrices */
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *gim_set_texture(void *list, void *gim);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
void mtx_set_mode(int mode);
void mtx_get34(DdMat4 *m);
void mat4_mul(DdMat4 *out, DdMat4 *a, DdMat4 *b);
void mat4_transform_vec4(DdVec4A *out, DdMat4 *m, DdVec4A *v);
void se_play(int id);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_color(void *list, unsigned int color);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
float ceilf_vfpu(float x);
unsigned short *table_get_entry(void *t, unsigned int i);
unsigned int *gfx_get_state(void);
int region_get(void);
unsigned short region_pick_value(int flag);
PadState *pad_get(void);

typedef struct DdList DdList;
typedef void *(*DdListRectFn)(void *dl, int x, int y, int w, int h);
typedef void *(*DdListItemFn)(void *dl, int i, int x, int y, int w, int h);

struct DdList {                        /* scrolling list widget, 0x34 */
    unsigned short count;              /* 0x00 */
    unsigned short cursor;             /* 0x02 */
    unsigned char active : 1;          /* 0x04 takes input */
    unsigned char no_cancel : 1;
    unsigned char no_accept : 1;
    unsigned char no_move : 1;
    unsigned char pad4 : 4;
    char pad5[3];
    int result;                        /* 0x08 1 cancel, 2 accept */
    short x, y;                        /* 0x0C */
    unsigned short w, h;               /* 0x10 */
    short pad_t, pad_b, pad_l, pad_r;  /* 0x14 */
    DdListRectFn draw_bg;              /* 0x1C */
    DdListItemFn draw_item;            /* 0x20 */
    DdListItemFn draw_item_bg;         /* 0x24 */
    DdListRectFn draw_cursor;          /* 0x28 */
    int (*item_h)(int i, int w);       /* 0x2C */
    short scroll;                      /* 0x30 */
    unsigned short total;              /* 0x32 */
};

void list_reset(DdList *l);
void list_layout(DdList *l);
void *list_draw(void *dl, DdList *l);
void list_term(DdList *l);
void list_set_active(DdList *l, int on);
int list_cursor(DdList *l);
void list_set_cursor(DdList *l, int c);
void listw_input_reset(DdList *l);
void listw_input(DdList *l);
void listw_term(DdList *l);
void listw_set_active(DdList *l, int on);
int listw_cursor(DdList *l);
void listw_set_cursor(DdList *l, int c);

/* base model (other ranges of this module) */
int func_0882837C(void *self, void *a, void *b);
int func_088283D4(void *self, int ev, void *a, void *b);
int func_08828470(void *self, void *a, void *b, void *c, void *d);
int func_08828528(void *self, void *a, void *b, void *c, int d);
int func_0881DF4C(const void *a, const void *b);     /* int compare (first word) */
int func_08839240(void *self, void *a, void *b);
int func_0883936C(void *self, int ev, void *a, void *b);
int func_088393B0(void *self, void *a, void *b, void *c, void *d);

#endif
