/* libduel_draw: the duel scene (scene thread, field, cards, cut-ins, LP panels, duel menus).
   Module state and the BOOT (libehsys) calls no shared header declares. Offsets are the ground
   truth; field names are partial. Each block below belongs to the address range in its marker;
   keep every block compilable. */
#ifndef LIBDUEL_DRAW_H
#define LIBDUEL_DRAW_H

/* ---- 0x08804000-0x088143A4 ---- */
/* DdLoc and DdPos as in the 0x0884774C block (identical text; keep one at merge) */
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

typedef struct DdLp {                  /* LP and phase panel, 0x08C54460, 0x3C */
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
} DdLp;

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
