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
    int f4;                            /* 0x04 */
    char pad8[0x16C];
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
