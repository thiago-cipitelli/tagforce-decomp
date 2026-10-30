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

typedef struct DdPos {                 /* field position query (dd_slot_xform) */
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

/* ---- 0x08804000-0x088143A4 ---- */
/* DdLoc and DdPos as in the 0x0884774C block (identical text; keep one at merge) */
typedef struct DdModelRes {            /* 3D model of a pack, 0x18 */
    int id;                            /* 0x1xxxx, sorted */
    int pack;                          /* DdPack id, -1 = none */
    const char *tms;
    const char *anim;                  /* .tmt (TGMT) or ptrs.txt */
    const char *tma;
    struct Model *model;
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
    struct RecipeData *deck[4];               /* +0x24 */
    int disk[2];                       /* +0x34 duel disk of players 0/1 */
    int first;                         /* +0x3C player who starts */
} DdDuelParam;

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

typedef struct DdXyz {
    float x, y, z;
} DdXyz;

typedef struct DdXyzw {
    float x, y, z, w;
} __attribute__((aligned(16))) DdXyzw;

typedef struct DdMtx {
    float m[16];
} __attribute__((aligned(16))) DdMtx;

typedef struct DdLight {               /* BOOT Light, 0x3C */
    int type;                          /* 0 directional, 1 point, 2 spot */
    unsigned int ambient;              /* +0x04 */
    unsigned int diffuse;              /* +0x08 */
    unsigned int specular;             /* +0x0C */
    DdXyz pos;                        /* +0x10 */
    DdXyz dir;                        /* +0x1C */
    float spot_exp;                    /* +0x28 */
    float spot_cutoff;                 /* +0x2C */
    float atten[3];                    /* +0x30 */
} DdLight;

typedef struct DdCam {                 /* camera, fog and lights of the field, 0x150 */
    DdXyzw eye;                        /* +0x00 */
    DdXyzw up;                         /* +0x10 */
    DdXyzw at;                         /* +0x20 */
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
    struct Model *model;                      /* +0x18 */
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
    DdMtx view;                        /* +0x1C0 */
    float fov;                         /* +0x200 */
    float aspect;
    float near;
    float far;
    DdXyzw dir;                        /* +0x210 */
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

typedef struct DdCobj {                 /* 3D card on the field, 0xB0 (list node) */
    struct DdCobj *prev;
    struct DdCobj *next;                /* +0x04 */
    int x08;
    int x0c;
    DdXyzw pos;                        /* +0x10 */
    DdXyzw pos0;                       /* +0x20 move start */
    DdXyzw pos1;                       /* +0x30 move end */
    DdXyzw rot;                        /* +0x40 */
    DdXyzw rot0;                       /* +0x50 */
    DdXyzw rot1;                       /* +0x60 */
    float t;                           /* +0x70 */
    float dur;                         /* +0x74 */
    DdPos dest;                        /* +0x78 pending move */
    short pad7a;
    int dest_kind;                     /* +0x7C */
    unsigned short card;               /* +0x80 card id */
    DdLoc home;                        /* +0x82 */
    DdPos loc;                         /* +0x84 */
    short pad86;
    int uid;                           /* +0x88 */
    signed char a0 : 1;                /* +0x8C move pending */
    signed char a1 : 1;
    signed char a2 : 1;
    signed char a3 : 1;
    signed char a4 : 1;
    signed char a5 : 1;
    signed char a6 : 1;
    signed char a7 : 1;
    signed char b0 : 1;                /* +0x8D */
    signed char b1 : 1;
    signed char b2 : 1;
    signed char b3 : 1;
    signed char b4 : 1;
    signed char b5 : 1;                /* face up */
    signed char b6 : 1;
    signed char b7 : 1;
    signed char c0 : 1;                /* +0x8E */
    signed char c1 : 1;
    signed char c2 : 1;
    signed char c3 : 1;
    signed char c4 : 1;
    signed char c5 : 1;
    signed char c6 : 1;
    signed char c7 : 1;
    char pad8f;
    int x90;
    int x94;
    int busy;                          /* +0x98 */
    int link[4];                       /* +0x9C attached effects */
    int padac;
} DdCobj;

typedef struct DdZoneAnim {            /* counter animation of a monster zone, 0x20 */
    int a;
    int b;
    int a0;
    int a1;
    int b0;
    int b1;
    float t;
    float dur;
} DdZoneAnim;

typedef struct DdCobjs {                /* card objects, 0x08C3D940, 0x16B20 */
    DdCobj obj[460];
    DdCobj free;                        /* +0x13C40 */
    DdCobj zone[4][16];                 /* +0x13CF0 list heads by player and zone */
    int x168f0;
    int map[2];                        /* +0x168F4 screen side -> player of the decks */
    int sync;                          /* +0x168FC lists follow the engine */
    int en[2];                         /* +0x16900 */
    DdZoneAnim za[2][5];               /* +0x16908 */
    int sel[2][17];                    /* +0x16A48 */
    DdLoc stack[32];                   /* +0x16AD0 */
    int nstack;                        /* +0x16B10 */
    int pad16b14[3];
} DdCobjs;

typedef struct DdLpp {                  /* LP and phase panel, 0x08C54460, 0x3C */
    unsigned int frame;                /* +0x00 30 per second */
    int lp[2];                         /* +0x04 shown */
    int lp_to[2];                      /* +0x0C */
    int lp_from[2];                    /* +0x14 */
    int lp_dest[2];                    /* +0x1C */
    unsigned int lp_t0[2];             /* +0x24 frame the count started */
    int turn_shown;                    /* +0x2C -1 while switching */
    int turn;                          /* +0x30 */
    int phase_state;                   /* +0x34 animation state of the phase panel */
    int phase;                         /* +0x38 */
} DdLpp;

typedef struct DdUv {                  /* 2D sprite of a sheet */
    unsigned short u, v, w, h;
} DdUv;

typedef struct DdChr DdChr;            /* duel character model (same object as DdCharSlot of 0x0884774C) */
struct DdChr {                         /* 0x180, the first 13 words come from the caller */
    int chr;                           /* charmap character */
    int sub;                           /* +0x04 variant */
    void *disc;                        /* +0x08 duel_disc.gim */
    void *voice_tbl;                   /* +0x0C chr_voiceidchgtbl.bin */
    void *words;                       /* +0x10 chr_wordstbl_<l>.bin */
    int (*on_start)(DdChr *c, int index, int arg);              /* +0x14 */
    int (*on_stop)(DdChr *c, int index, int a, int b);          /* +0x18 */
    int (*on_tick)(DdChr *c, int index, int frame, int anim_frame, int arg); /* +0x1C */
    int (*map_anim)(DdChr *c, int anim);                         /* +0x20 */
    int (*map_bone)(DdChr *c, int bone);                         /* +0x24 */
    void (*get_pos)(DdChr *c, DdXyzw *pos, DdXyzw *rot);         /* +0x28 */
    int x2c;
    struct DdChrBone *bones;           /* +0x30 attach points, 0-terminated */
    int x34;
    struct Model *model;               /* +0x38 */
    void **anims;                      /* +0x3C TGMT per animation (alletc-ptrs.txt) */
    int anim;                          /* +0x40 */
    void *tmt;                         /* +0x44 */
    int index;                         /* +0x48 */
    float time;                        /* +0x4C */
    void *buf;                         /* +0x50 decompressed animation */
    void *voice;                       /* +0x54 voice track */
    void *vbuf;                        /* +0x58 */
    float vtime;                       /* +0x5C */
    int pad60[24];
    DdMtx proj;                        /* +0xC0 */
    DdMtx view;                        /* +0x100 */
    DdXyzw dir;                        /* +0x140 */
    int x150;
    int disc_on;                       /* +0x154 */
    int x158;
    int x15c;
    int x160;
    int done;                          /* +0x164 animation looped/ended this frame */
    int x168;
    int x16c;
    unsigned int mask;                 /* +0x170 parts shown */
    unsigned int mask_sel;             /* +0x174 parts forced */
    unsigned int mask_val;             /* +0x178 */
    int pad17c;
};

typedef struct DdChrMgr {              /* 0x08C544A0, 400 */
    int hdr[4];
    DdChr chr;
} DdChrMgr;

typedef struct DdModelUpd {            /* BOOT model_update argument with a bone callback */
    float dt;
    void *cb;
    int chr;
    void *c;
} DdModelUpd;

#endif
