/* libduel_draw: the duel scene (field, cards, effects, cut-ins, LP panels, duel menus). Shared types,
   one block per address range (the blocks were written in parallel and merged by concatenation).
   Offsets are the ground truth; field names are partial. */
#ifndef LIBDUEL_DRAW_H
#define LIBDUEL_DRAW_H


/* ---- 0x0884774C-0x08857100 ---- */

typedef struct FxVec3 {
    float x, y, z;
} FxVec3;

typedef struct FxVec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) FxVec4;

typedef struct FxMat4 {
    float m[16];
} __attribute__((aligned(16))) FxMat4;

typedef struct FxVtx {                 /* GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF */
    float u, v;
    unsigned int color;
    float x, y, z;
} FxVtx;

typedef struct DdObj DdObj;
typedef void (*DdObjFn)(DdObj *o, void *data);
struct DdObj {                         /* scene object (func_0881E2A8), also its creation descriptor, 0x1C */
    int pad00[2];
    DdObjFn draw;                      /* +0x08 */
    DdObjFn update;                    /* +0x0C */
    DdObjFn free;                      /* +0x10 */
    float time;                        /* +0x14 seconds since the update was set */
    void *data;                        /* +0x18 */
};

typedef struct DdCharSlot {            /* duel character (partner/rival model + voice), 0x180 */
    unsigned int chr;                  /* +0x000 character id */
    int variant;                       /* +0x004 */
    int pad008[11];
    int index;                         /* +0x034 slot index (sound channel) */
    int pad038[34];
    float view[16];                    /* +0x0C0 */
    float proj[16];                    /* +0x100 */
    float pos[4];                      /* +0x140 */
    int voice;                         /* +0x150 */
    int pad154[2];
    int anim_from;                     /* +0x15C */
    int anim_to;                       /* +0x160 */
    int pad164[7];
} DdCharSlot;

typedef struct DdCharMgr {             /* 0x08C5AC00, 0x620 */
    int pad00[2];
    DdCharSlot *cur;                   /* +0x08 */
    int cur_idx;                       /* +0x0C */
    int busy;                          /* +0x10 */
    int pad14[3];
    DdCharSlot slot[4];                /* +0x20 */
} DdCharMgr;

typedef struct DdCharInfo {            /* BOOT charmap_lookup2 */
    unsigned char a;
    unsigned char kind;
    unsigned char c : 4;
    unsigned char hi : 4;
    unsigned char pad;
} DdCharInfo;

typedef struct DdRyusen {              /* 0x08C5B220: ehf of speed lines behind the character */
    unsigned int chr;                  /* +0x00 pack of the character drawn */
    int variant;
    int layer;                         /* +0x08 */
    int alpha;                         /* +0x0C */
    int alpha_from;                    /* +0x10 */
    int alpha_to;                      /* +0x14 */
    int t;                             /* +0x18 fade step, 10 steps */
} DdRyusen;

typedef struct DdEhfOpts {             /* BOOT ehf_draw options, 0x60 */
    int time;                          /* +0x00 */
    void *tex_cb;
    void *tex_arg;
    unsigned int flags;                /* +0x0C */
    int angle;
    float x, y;                        /* +0x14 */
    int pad1c;
    float sx, sy;                      /* +0x20 */
    void *eval_cb;
    void *eval_arg;
    unsigned int mask[2];              /* +0x30 */
    int pad38[2];
    int color[8];                      /* +0x40 */
} DdEhfOpts;

typedef struct DdLoc {                 /* card location packed in a short */
    unsigned short player : 1;
    unsigned short zone : 5;
    unsigned short idx : 10;
} DdLoc;

typedef struct DdPos {                 /* field position query (func_0880A8E0) */
    unsigned short player : 1;
    unsigned short zone : 5;
    unsigned short idx : 8;
    unsigned short f14 : 1;
    unsigned short f15 : 1;
} DdPos;

typedef struct DdMarker {              /* cursor marker over a card zone, 0x60 */
    FxVec4 pos;                        /* +0x00 */
    FxVec4 rot;                        /* +0x10 quaternion */
    int moving;                        /* +0x20 */
    DdPos at;                          /* +0x24 zone it stands on */
    DdPos to;                          /* +0x26 zone it moves to */
    int pad28[2];
    FxVec4 from;                       /* +0x30 */
    FxVec4 dest;                       /* +0x40 */
    float dur;                         /* +0x50 */
    int pad54[3];
} DdMarker;

typedef struct DdArrow {               /* attack arrow (particles along a curve + head), 0x70 */
    unsigned char alpha[64];           /* +0x00 per particle */
    void *tex_ptcl;                    /* +0x40 */
    void *tex_head;                    /* +0x44 */
    int head;                          /* +0x48 particle lit by the running wave */
    int alive;                         /* +0x4C */
    int done;                          /* +0x50 */
    FxVec3 from;                       /* +0x54 */
    FxVec3 to;                         /* +0x60 */
    unsigned int color;                /* +0x6C */
} DdArrow;

typedef struct DdMarkMgr {             /* 0x08C5B23C */
    DdObj *marker;
    DdObj *arrow;
    short arrow_from;                  /* +0x08 locations the arrow joins */
    short arrow_to;                    /* +0x0A */
} DdMarkMgr;

typedef struct FxVtxT {                /* GU_TEXTURE_32BITF | GU_VERTEX_32BITF */
    float u, v;
    float x, y, z;
} FxVtxT;

typedef struct FxVtxC {                /* GU_COLOR_8888 | GU_VERTEX_32BITF */
    unsigned int color;
    float x, y, z;
} FxVtxC;

typedef struct DdFxArg {               /* request of a card effect (libduel_mgr) */
    int pad00;
    int run;                           /* +0x04 1 while the effect advances */
    int pad08[2];
    int id;                            /* +0x10 effect id */
} DdFxArg;

typedef struct DdFxHead {              /* common start of the work of an effect task */
    int wait;                          /* +0x00 frames waited before starting */
    int delay;                         /* +0x04 */
    int t;                             /* +0x08 frame of the effect */
    int len;                           /* +0x0C */
    int frame;                         /* +0x10 frames drawn */
    DdFxArg *arg;                      /* +0x14 */
} DdFxHead;

typedef struct DdFxHalo {              /* halo around bone 38 (dd_fx_aura_halo), 0x2A0 */
    int t;                             /* +0x000 */
    int frame;                         /* +0x004 */
    int pad08[2];
    FxMat4 mtx;                        /* +0x010 */
    float alpha;                       /* +0x050 */
    int visible;                       /* +0x054 */
    FxVtxC yellow[18];                 /* +0x058 fan */
    FxVtxC blue[18];                   /* +0x178 */
} DdFxHalo;

typedef struct DdFxCircle {            /* spell circle (dd_fx_aura_circle), 0x180 */
    int t;                             /* +0x000 */
    int frame;                         /* +0x004 */
    int bone;                          /* +0x008 -1: none */
    int pad0c;
    FxMat4 mtx;                        /* +0x010 */
    int started;                       /* +0x050 */
    unsigned int se;                   /* +0x054 */
    FxVtx ring[6];                     /* +0x058 */
    FxVtx blade[6];                    /* +0x0E8 */
} DdFxCircle;

#endif
