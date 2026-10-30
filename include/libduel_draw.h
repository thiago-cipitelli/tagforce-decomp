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
struct DdObj {                         /* scene object (dd_task_add), also its creation descriptor, 0x1C */
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

/* ---- 0x08825DDC-0x08837098 ---- */
#define DD_TASK_MAX 254

typedef struct DdFxVert {              /* GE vertex: float texture, 8888 colour, float position (0x19F) */
    float u, v;
    unsigned int color;
    float x, y, z;
} DdFxVert;

typedef struct DdMon {                 /* monster model instance played by the duel scene */
    int chara;                         /* +0x000 character of the voice set (< 41: story) */
    int x04;                           /* +0x004 variant of the voice set */
    int x08;
    int x0c;                           /* +0x00C partner table of the voice lines */
    int voice;                         /* +0x010 */
    unsigned char pad14[0x20];
    int chan;                          /* +0x034 sound channel */
    void *model;                       /* +0x038 */
    int *anim_ok;                      /* +0x03C nonzero for each animation id the model has */
    unsigned char pad40[0x110];
    int card;                          /* +0x150 card id shown in the ATK/DEF panel */
    int pad154[2];
    int lp_from;                       /* +0x15C LP panel values */
    int lp_to;                         /* +0x160 */
    int pad164;
    int hold;                          /* +0x168 keep the last frame */
    int faded;                         /* +0x16C fade of the destruction started */
} DdMon;

typedef struct DdMonCb {               /* callbacks of a monster model (0x34) */
    int a;                             /* +0x00 */
    int b;                             /* +0x04 */
    int pad08[3];
    int (*on_start)(int a, int b, int c);              /* +0x14 */
    int (*on_event)(int a, int anim, int c, int d);    /* +0x18 */
    int pad1c;
    int (*pick_anim)(DdMon *m, int anim);              /* +0x20 */
    int (*cb24)(void);                                 /* +0x24 */
    int (*cb28)(void);                                 /* +0x28 */
    void *bone;                                        /* +0x2C bone transform (dd_mon_bone) */
    int pad30;
} DdMonCb;


typedef int (*DdTaskUpdateFn)(int state, void *mem);   /* returns the next state */
typedef void (*DdTaskTermFn)(void *arg);

typedef struct DdFxTask {                /* 0x1C */
    int state;                         /* +0x00 0 free, 1/2 running, 3 killed, 4 freed next frame */
    unsigned int flags;                /* +0x04 bit 0: paused (dd_task_pause) */
    int handle;                        /* +0x08 slot << 24 | serial, -1 when free */
    DdTaskUpdateFn update;             /* +0x0C */
    DdTaskTermFn term;                 /* +0x10 called with arg when the slot is freed */
    void *arg;                         /* +0x14 */
    void *mem;                         /* +0x18 zeroed work buffer from the duel heap */
} DdFxTask;

typedef struct DdFxTaskMgr {             /* 0x08C58B90 */
    unsigned int serial;               /* +0x0000 low 24 bits of the next handle */
    DdFxTask task[DD_TASK_MAX];          /* +0x0004 */
    unsigned char pad1bcc[0x124];      /* cleared with the table (0x1CF0 bytes) */
} DdFxTaskMgr;

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

int dd_int_cmp(const void *a, const void *b);     /* int compare (first word) */

#endif
