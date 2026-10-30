/* libduel_draw: the duel scene (rendering of the field, cards, effects, cut-ins, LP panels, duel
   menus): shared types only (BOOT prototypes go in each area's private header). Offsets are the
   ground truth; field names are partial. Each block below belongs to the address range in its
   marker; keep every block compilable. */
#ifndef LIBDUEL_DRAW_H
#define LIBDUEL_DRAW_H

/* ---- 0x088143A4-0x08825DDC ---- */
typedef struct DdLoc {                 /* a field location (cursor, targets) */
    unsigned short player : 1;
    unsigned short zone : 5;           /* 0-4 monsters, 5-9 spells/traps, 10 field, 11 hand, 12-15 decks/grave, 16 none */
    unsigned short idx : 10;           /* card index inside zones with several cards */
} DdLoc;

typedef struct DdLoc2 {                /* location as libduel_draw's card placement takes it */
    unsigned short player : 1;
    unsigned short zone : 5;
    unsigned short idx : 8;
    unsigned short b14 : 1;
    unsigned short b15 : 1;
} DdLoc2;

typedef struct DdVec3 {
    float v[3];
} DdVec3;

typedef struct DdVec4 {
    float v[4];
} DdVec4;

typedef struct DdTexMat {              /* material of a model texture (texture hooks) */
    const char *name;                  /* +0x00 */
    int unk4;
    short unk8;
    signed char func, comp;            /* +0x0A */
    unsigned int env;                  /* +0x0C */
    signed char wrap_u, wrap_v;        /* +0x10 */
    float u, v, su, sv;                /* +0x14 */
} DdTexMat;

typedef void (*DdCursorCb)(DdLoc loc, int card);

typedef struct DdCursor {              /* 0x90 at 0x08C54640 */
    float pos[4];                      /* +0x00 3D position */
    float rot[4];                      /* +0x10 quaternion */
    DdLoc cur;                         /* +0x20 */
    int col;                           /* +0x24 grid column */
    int row;                           /* +0x28 grid row */
    int saved;                         /* +0x2C saved_loc is valid */
    DdLoc saved_loc;                   /* +0x30 */
    int dirty;                         /* +0x34 */
    int locked;                        /* +0x38 */
    DdCursorCb listeners[16];          /* +0x3C */
    DdLoc hand_back;                   /* +0x7C where the cursor was before jumping to the hand */
    int hand_idx;                      /* +0x80 */
    int lit;                           /* +0x84 the location can act */
    int unk88[2];
} DdCursor;

typedef struct DdCutinDesc {           /* 0x44: a cut-in request */
    void *ehf;                         /* +0x00 animation */
    int start;                         /* +0x04 first frame */
    int end;                           /* +0x08 last frame, -1: the animation's */
    int loop_from;                     /* +0x0C */
    int loop_to;                       /* +0x10 */
    void (*done)(int arg);             /* +0x14 */
    int done_arg;                      /* +0x18 */
    int flag;                          /* +0x1C request flag raised while it plays */
    int always;                        /* +0x20 raise the flag whatever the speed setting */
    int se;                            /* +0x24 */
    float x, y;                        /* +0x28 */
    int layer;                         /* +0x30 > 0: drawn in 2D on that layer */
    int text_frame;                    /* +0x34 caption shown after this frame */
    int text;                          /* +0x38 caption id */
    int text_x, text_y;                /* +0x3C */
} DdCutinDesc;

typedef struct DdCutin {               /* 0x9C at 0x08C546D0 */
    DdCutinDesc cur;
    int active;                        /* +0x44 */
    int queued;                        /* +0x48 */
    int ending;                        /* +0x4C */
    int looping;                       /* +0x50 */
    int loops;                         /* +0x54 */
    DdCutinDesc next;                  /* +0x58 */
} DdCutin;

typedef union DdEvArg {                /* a packed argument of a duel event */
    int i;
    short s;
    struct {
        unsigned char lo, hi;
    } b;
    struct {
        unsigned short lo, hi;
    } h;
} DdEvArg;

typedef struct DdEvSlot {
    int a, b, c;
} DdEvSlot;

typedef struct DdEvState {             /* 0x2C at 0x08C5476C */
    DdEvSlot slot[2];
    int player;                        /* +0x18 */
    int rival;                         /* +0x1C */
    int unk20[2];
    int unk28;                         /* +0x28 */
} DdEvState;

typedef struct DdCard {                /* a card object of the field (other ranges own it) */
    char unk0[0x80];
    unsigned short id;                 /* +0x80 card id */
    DdLoc2 loc;                        /* +0x82 */
    DdLoc2 loc2;                       /* +0x84 bit 14: face up */
    short unk86;
    int uid;                           /* +0x88 instance */
    signed char unk8c_0 : 6;           /* +0x8C */
    signed char moving2 : 1;
    signed char moving : 1;
    signed char b0 : 4;                /* +0x8D */
    signed char b4 : 1;
    signed char b5 : 1;
    signed char b6 : 1;
    signed char flip_pending : 1;
    char unk8e[2];
    int glow;                          /* +0x90 glow effect, -1 none */
    int mark;                          /* +0x94 target mark, -1 none */
    int fx;                            /* +0x98 */
} DdCard;

typedef struct DdFlip {                /* argument of the flip cut-in's end callback */
    DdLoc loc;
    int value;
} DdFlip;

/* ---- end 0x088143A4-0x08825DDC ---- */

#endif
