/* libduel_draw: the duel scene (field, cards, effects, cut-ins, LP panels, duel menus).
   Each block below belongs to the address range in its marker; keep every block compilable. */
#ifndef LIBDUEL_DRAW_H
#define LIBDUEL_DRAW_H

/* ---- 0x08857100-0x0886B25C ---- */
typedef struct ChrVec4 {
    float x, y, z, w;
} ChrVec4;

typedef struct DdVec4A {               /* stack vector, 16-aligned (VFPU) */
    float x, y, z, w;
} __attribute__((aligned(16))) DdVec4A;

typedef struct DdMat4 {
    float m[16];
} __attribute__((aligned(16))) DdMat4;

typedef struct ChrBone {               /* rest pose of one bone of a character model, 0x20 */
    int id;
    float px, py, pz;
    ChrVec4 rot;                        /* quaternion */
} ChrBone;

typedef struct ChrVt {                 /* callbacks of a character model (0x0884DC84 picks the setup) */
    char pad[20];
    int (*init)(void *self, void *a, void *b);                      /* 0x14 */
    int (*event)(void *self, int ev, void *a, void *b);              /* 0x18 */
    int (*update)(void *self, void *a, void *b, void *c, void *d);   /* 0x1C */
    void *f20;
    int (*bone_id)(void *self, int i);                              /* 0x24 slot -> bone id */
    int (*bone_rest)(void *self, ChrVec4 *pos, ChrVec4 *rot, int id); /* 0x28 */
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

typedef struct PopupLoc {                 /* card location on the field (u16) */
    unsigned short side : 1;
    unsigned short zone : 5;
    unsigned short index : 10;
} PopupLoc;

typedef struct DdPopupArg {
    int card;
    void (*get_loc)(PopupLoc *loc);
} DdPopupArg;

typedef struct DdPopup {               /* 0x08C5B988, 0x24: the action menu of a field card */
    DdPopupArg arg;                    /* +0x00 */
    struct MenuWidget *menu;           /* +0x08 */
    int items[4];                      /* +0x0C */
    int count;                         /* +0x1C */
    int text_w;                        /* +0x20 */
} DdPopup;

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

int func_0881DF4C(const void *a, const void *b);     /* int compare (first word) */

#endif
