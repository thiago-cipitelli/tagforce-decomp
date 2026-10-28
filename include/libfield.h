/* libfield: the campus map / field scene (scene 5): the rooms of the academy (bg_NN_NN), the SD
   characters walking in them, the bust-up portraits and the dialogs. Module state and the BOOT
   (libehsys) calls no shared header declares. Offsets are the ground truth; field names are partial. */
#ifndef LIBFIELD_H
#define LIBFIELD_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"
#include "game.h"
#include "libc.h"
#include "psp.h"

typedef struct FieldSeqStep {          /* step of the startup sequence, 0x088329D0[15] */
    int kind;
    void *arg;
} FieldSeqStep;

typedef struct FieldSeq {              /* 0x0883DDF8, 0x6820 */
    int state;                         /* +0x0000 */
    int pad04[15];
    unsigned short text[12][0x400];    /* +0x0040 wide copies of field_seq_src */
    Menu menu;                         /* +0x6040 */
    int pad608c[(0x7D0 - sizeof(Menu)) / 4];
    int active;                        /* +0x6810 */
    int index;                         /* +0x6814 step running */
    int started;                       /* +0x6818 */
    int done;                          /* +0x681C */
} FieldSeq;

typedef struct EhfOpts {               /* BOOT ehf_draw options, 0x60 */
    int time;                          /* +0x00 */
    void *tex_cb;
    void *tex_arg;
    unsigned int flags;                /* +0x0C 0x10000 position */
    int angle;
    float x, y;                        /* +0x14 */
    int pad1c;
    float sx, sy;                      /* +0x20 */
    void *eval_cb;
    void *eval_arg;
    unsigned int mask[2];              /* +0x30 */
    int pad38[2];
    int color[8];                      /* +0x40 */
} EhfOpts;

typedef struct FieldCtx {              /* 0x08FD1408, 0x18: fld_ctx_get() */
    unsigned int frame;                /* +0x00 frame counter */
    int pad04;
    int speaker;                       /* +0x08 slot talking, -1 none */
    int se_on;                         /* +0x0C */
    int pad10;
    int x14;                           /* +0x14 */
} FieldCtx;

typedef struct FieldWaiter {           /* callback waiting for a file, 0xC */
    void (*cb)();
    void *arg;
    struct FieldWaiter *next;
} FieldWaiter;

typedef struct FieldWaitQ {
    FieldWaiter *head;
    FieldWaiter *tail;
} FieldWaitQ;

typedef struct FieldList {             /* argument of the field commands: an array and its count */
    void *items;
    int n;
} FieldList;

typedef struct FieldBuOpen {           /* item of the bust-up open command */
    int chara;
    int index;                         /* slot */
    int anim;
} FieldBuOpen;

typedef struct FieldBuAnim {           /* item of the bust-up animation command */
    int index;
    int anim;
} FieldBuAnim;

typedef struct FieldAnimFrame {        /* 0x08832AB8[mode][19] */
    int id;
    int frame;
} FieldAnimFrame;

typedef struct FieldBuSlot {           /* bust-up portrait, 0x100 */
    int *file;                         /* +0x00 loaded file (bu_%s.ehp.gz) */
    void *pack;                        /* +0x04 decompressed EHP */
    void *body;                        /* +0x08 body.ehf */
    int pad0c;
    EhfOpts body_opts;                 /* +0x10 */
    int anim;                          /* +0x70 */
    int blink;                         /* +0x74 */
    int blink_wait;                    /* +0x78 */
    void *mouth;                       /* +0x7C mouth.ehf */
    EhfOpts mouth_opts;                /* +0x80 */
    int talk;                          /* +0xE0 */
    int loaded;                        /* +0xE4 */
    int state;                         /* +0xE8 0 idle, 1 fade in, 2 shown, 3 fade out */
    int chara;                         /* +0xEC */
    int index;                         /* +0xF0 */
    int padf4[3];
} FieldBuSlot;

typedef struct FieldBu {               /* 0x08844620, 0x340 */
    int loading;                       /* +0x000 */
    int unloading;                     /* +0x004 */
    int opening;                       /* +0x008 */
    int animating;                     /* +0x00C */
    int closing;                       /* +0x010 */
    int open[3];                       /* +0x014 */
    int ready;                         /* +0x020 */
    int close[3];                      /* +0x024 */
    FieldBuSlot slot[3];               /* +0x030 */
    int pending;                       /* +0x330 */
    int to_load;                       /* +0x334 */
    int speaker;                       /* +0x338 */
    int voice;                         /* +0x33C */
} FieldBu;

typedef struct Vec3 {
    float x, y, z;
} Vec3;

typedef struct FieldChr {              /* SD character on the map */
    int kind;                          /* +0x00 1 = the player */
    int id;                            /* +0x04 chara id */
    int pad08[2];                      /* +0x08 */
    int type;                          /* +0x10 */
    Vec3 pos;                          /* +0x14 */
    int anim;                          /* +0x20 0 idle, 1 walk */
    int dir;                           /* +0x24 0..7 */
    int frame;                         /* +0x28 */
    Vec3 vel;                          /* +0x2C step of the last move */
    int pad38[2];
    int shadow[16];                    /* +0x40 */
    int visible;                       /* +0x80 */
    int fx;                            /* +0x84 */
    int run;                           /* +0x88 */
    int arg8c;                         /* +0x8C */
} FieldChr;

typedef struct FieldVtx {              /* GU vertex: texture, color, position */
    float u, v;
    unsigned int color;
    float x, y, z;
} FieldVtx;

typedef struct CharmapPos {            /* BOOT charmap_lookup2 */
    unsigned char a;
    unsigned char b;
    unsigned char c : 4;
    unsigned char hi : 4;
} CharmapPos;

typedef struct FieldMob {              /* character with its loaded file, 0xA0 */
    FieldChr c;
    int active;                        /* +0x90 */
    int loaded;                        /* +0x94 */
    int *file;                         /* +0x98 */
    int x9c;                           /* +0x9C */
} FieldMob;

typedef struct FieldNpc {              /* NPC walking a path, 0xF0 */
    FieldMob m;
    int xa0;
    int xa4;
    int xa8;
    int xac;
    int xb0;
    int xb4;
    int xb8;
    int xbc;
    int xc0;                           /* +0xC0 */
    int turned;                        /* +0xC4 turned to the player while talking */
    int saved_dir;                     /* +0xC8 */
    int saved_anim;                    /* +0xCC */
    int xd0;
    int xd4;
    int pad_d8[4];
    int *path;                         /* +0xE8 */
    int n_path;                        /* +0xEC */
} FieldNpc;

typedef struct FieldNpcs {             /* 0x08844DB0, 0x5020 */
    int loading;                       /* +0x0000 */
    int setting;                       /* +0x0004 */
    int pad08[2];
    FieldNpc npc[32];                  /* +0x0010 */
    FieldMob mob[80];                  /* +0x1E10 */
    int pending;                       /* +0x5010 */
    int talking;                       /* +0x5014 */
    int talk_id;                       /* +0x5018 */
    int talk_dir;                      /* +0x501C */
} FieldNpcs;

typedef struct FieldPlayer {           /* 0x08849DD0, 0xC0 */
    int *file_a;                       /* +0x00 */
    int *file_b;                       /* +0x04 */
    int pad08[2];
    FieldChr c;                        /* +0x10 */
    int active;                        /* +0xA0 */
    int lock;                          /* +0xA4 input disabled */
    int busy;                          /* +0xA8 */
    int padac[5];
} FieldPlayer;

typedef struct FieldReq {              /* queued load request, 0x1C */
    int kind;                          /* +0x00 pool kind, 0 = shared GIM */
    int step;                          /* +0x04 */
    void *ent;                         /* +0x08 FieldResEnt or FieldPoolDesc */
    void *ent2;                        /* +0x0C */
    void *arg;                         /* +0x10 */
    struct FieldReq *next;             /* +0x14 */
    void (*cb)();                      /* +0x18 */
} FieldReq;

typedef struct FieldReqQ {
    FieldReq *head;
    FieldReq *tail;
    int busy;                          /* a read is in flight */
} FieldReqQ;

typedef struct FieldRes {              /* 0x08FD10A0, 0x14: file loader */
    void *heap;                        /* +0x00 */
    void *cip;                         /* +0x04 card pictures */
    FieldReqQ q;                       /* +0x08 */
} FieldRes;

typedef struct FieldResEnt {           /* cached file, 0x28 */
    const char *name;
    const char *dir;                   /* +0x04 */
    const char *file;                  /* +0x08 */
    int pack;                          /* +0x0C file is an EHP to mount */
    void *buf;                         /* +0x10 */
    int *data;                         /* +0x14 */
    int loading;                       /* +0x18 */
    FieldWaitQ waiters;                /* +0x1C */
    int discard;                       /* +0x24 */
} FieldResEnt;

typedef struct FieldGim {              /* 0x0883C49C[] */
    int pad;
    const char *name;
    void *gim;
} FieldGim;

typedef struct FieldPoolEnt {          /* loaded file of a pool, 0x20 */
    int name;
    void *buf;                         /* +0x04 */
    int *data;                         /* +0x08 */
    int loading;                       /* +0x0C */
    int waiters[2];                    /* +0x10 */
    int discard;                       /* +0x18 */
    int x1c;
} FieldPoolEnt;

typedef struct FieldPool {
    int count;
    FieldPoolEnt e[1];
} FieldPool;

typedef struct FieldPoolDesc {         /* 0x0883C5B4[5]: chr, chr2, bg, misc, voice */
    FieldPool *pool;
    int n;
    const char *dir;
    const char **names;
    int x10;
} FieldPoolDesc;

typedef struct FieldSys {              /* 0x08FD1A44, 0x30: state of the field commands */
    int x00;                           /* +0x00 */
    int x04;
    int x08;
    int x0c;
    int fading;                        /* +0x10 */
    int x14;
    int x18;
    int active;                        /* +0x1C */
    int drawing;                       /* +0x20 */
    int mode;                          /* +0x24 */
    int fade_out;                      /* +0x28 */
    int x2c;
} FieldSys;

typedef struct FieldCmd {              /* 0x0883C640[25] */
    void (*start)(void *arg);
    int (*poll)(void *arg);
    int busy;
} FieldCmd;

typedef struct FieldFadeCmd {
    int out;
    unsigned short frames;
    int color;                         /* +0x08 index into field_fade_colors */
} FieldFadeCmd;

typedef struct FieldStory {            /* 0x40 */
    int w[16];
} FieldStory;

typedef struct FieldScene {            /* 0x08FD1A74, 0xB8: state of the scene commands */
    int loading;                       /* +0x00 */
    int x04;                           /* +0x04 player placed */
    int x08;
    int saving;                        /* +0x0C */
    int x10;
    int x14;
    int x18;
    int x1c;
    int bg;                            /* +0x20 */
    int save[5];                       /* +0x24 */
    int x38;
    int x3c;
    int x40;
    int *file;                         /* +0x44 */
    FieldStory story;                  /* +0x48 */
    int x88;
    int pad8c[7];
    int xa8;
    int padac[3];
} FieldScene;

typedef struct FieldSaveNpc {
    int id;
    int cx, cz;
    int dir;
} FieldSaveNpc;

typedef struct FieldSave {             /* what the save command fills */
    int bg;
    float x, z;
    int dir;
    FieldSaveNpc npc[32];              /* +0x010 */
    int n_npc;                         /* +0x210 */
    int save[5];                       /* +0x214 */
} FieldSave;

typedef struct Vec2 {
    float x, y;
} Vec2;

typedef struct FieldHudMove {          /* +0x128 of FieldHud */
    int on;                            /* 0 off, 1 waiting, 2 moving */
    Vec2 from;                         /* +0x04 */
    Vec2 pos;                          /* +0x0C */
    int arg;                           /* +0x14 */
    float t;                           /* +0x18 */
    float s;                           /* +0x1C scale */
    int wait;                          /* +0x20 */
} FieldHudMove;

typedef struct FieldHudSlide {         /* 0x10 */
    int state;                         /* 0 off, 1 sliding, 2 done */
    Vec2 pos;
    float t;
} FieldHudSlide;

typedef struct FieldHudName {          /* 0x88 */
    int state;                         /* 0 off, 1 sliding, 2 done */
    Vec2 pos;
    float t;
    MenuItemDef item;                  /* +0x10 */
} FieldHudName;

typedef struct FieldHud {              /* 0x08FD1DAC, 0x200: the HUD of the field */
    int on;                            /* +0x000 */
    void *gim0;                        /* +0x004 */
    void *gim1;                        /* +0x008 */
    void *gim19;                       /* +0x00C */
    int p[5];                          /* +0x010 */
    Menu menu;                         /* +0x024 */
    FieldHudName name;                 /* +0x070 room name window sliding in */
    FieldHudSlide clock;               /* +0x0F8 clock slide-in */
    FieldHudSlide day;                 /* +0x108 */
    FieldHudSlide date;                /* +0x118 */
    FieldHudMove move;                 /* +0x128 */
    FieldHudSlide panel;               /* +0x14C */
    FieldHudName boxw;                 /* +0x15C message box (item at +0x16C) */
    int blink_on;                      /* +0x1E4 */
    int blink;                         /* +0x1E8 */
    int blink_kind;                    /* +0x1EC */
    unsigned int period;               /* +0x1F0 */
    int duty;                          /* +0x1F4 */
    int t;                             /* +0x1F8 */
    int phase;                         /* +0x1FC */
} FieldHud;

typedef struct FieldScTab {            /* 0x08FD1B2C, 0x280 */
    int state;                         /* +0x000 */
    MenuWidget w1;                     /* +0x004 */
    unsigned short *texts[6];          /* +0x124 */
    int idx[6];                        /* +0x13C */
    MenuWidget w2;                     /* +0x154 */
    unsigned short *texts2[3];         /* +0x274 */
} FieldScTab;

typedef struct FieldMap {              /* fld_map_get() */
    int pad00;
    unsigned short pad04;
    unsigned short n;                  /* +0x06 cells (w * h) */
    unsigned short w;                  /* +0x08 cells */
    unsigned short h;                  /* +0x0A */
} FieldMap;

typedef struct FieldChrGlb {           /* 0x08844960, 0x20 */
    int unloading;                     /* +0x00 */
    int pad04[3];
    Vec3 light;                        /* +0x10 */
    int light_w;                       /* +0x1C */
} FieldChrGlb;

typedef struct FieldCell {
    short x, z;
} FieldCell;

typedef struct FieldBgCmd {            /* argument of the room load command */
    int id;                            /* bg id */
    FieldCell *cells;                  /* marked cells */
    int n_cells;
} FieldBgCmd;

typedef struct Mat4 {
    float m[16];
} Mat4;

typedef struct FieldObj {              /* placed object of the room, 0x60 */
    int *info;                         /* +0x00 info[2] = area */
    void *model;                       /* +0x04 */
    int pad08[2];
    Mat4 mtx;                          /* +0x10 */
    int mode;                          /* +0x50 */
    int x54;
    int pad58[2];
} FieldObj;

typedef struct ModelDrawOpts {         /* BOOT model_draw options, 0x34 */
    int a;
    int b;
    int rest[11];
} ModelDrawOpts;

typedef struct FieldBg {               /* 0x08844980, 0x430: the room */
    int loading;                       /* +0x000 */
    int unloading;                     /* +0x004 */
    int *file;                         /* +0x008 */
    int id;                            /* +0x00C */
    void *model[2];                    /* +0x010 areas */
    void *door[2];                     /* +0x018 */
    void *model2[2];                   /* +0x020 shadow/stencil models */
    int pad028[2];
    FieldObj obj[10];                  /* +0x030 */
    int n_models;                      /* +0x3F0 */
    int area;                          /* +0x3F4 */
    int prev_area;                     /* +0x3F8 */
    int n_doors;                       /* +0x3FC */
    int n_objs;                        /* +0x400 */
    Vec3 light;                        /* +0x404 */
    int fade;                          /* +0x410 */
    unsigned int fade_t;               /* +0x414 */
    FieldMap *map;                     /* +0x418 */
    FieldCell *cells;                  /* +0x41C */
    int n_cells;                       /* +0x420 */
    int pad424[3];
} FieldBg;

typedef struct Vec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) Vec4;

typedef struct FieldItem {             /* object lying on a cell, 0x60 */
    int active;
    int id;
    Vec3 pos;                          /* +0x08 */
    int pad14[3];
    Mat4 mtx;                          /* +0x20 */
} FieldItem;

typedef struct FieldItems {            /* 0x08FD1420, 0x610 */
    int loading_a;                     /* +0x000 */
    int loading_b;                     /* +0x004 */
    int *file;                         /* +0x008 */
    void *gim;                         /* +0x00C */
    FieldItem a[8];                    /* +0x010 */
    FieldItem b[8];                    /* +0x310 */
} FieldItems;

typedef struct FieldItemCmd {          /* item of the place command, 0xC */
    int id;
    int x, z;
} FieldItemCmd;

typedef struct FieldLight {            /* 0x08838044[bg][3] */
    int light;
    int pad;
} FieldLight;

typedef struct FieldMsgLine {          /* a line of FieldMsg, 0x18 */
    int x, y;                          /* +0x00 */
    int shown;                         /* +0x08 width typed so far (pixels) */
    int h;                             /* +0x0C */
    int start;                         /* +0x10 first character */
    int count;                         /* +0x14 characters on the line */
} FieldMsgLine;

typedef struct FieldMsg {              /* 0x08FD1FAC, 0x180: message window (libtitle's tmsg is a copy) */
    int state;                         /* +0x000 0 off, 1 opening, 2 open, 3 closing */
    float t;                           /* +0x004 slide 0..1 */
    Menu menu;                         /* +0x008 */
    MenuItemDef def;                   /* +0x054 */
    void *heap;                        /* +0x0CC */
    int x;                             /* +0x0D0 */
    int w0;                            /* +0x0D4 */
    int size;                          /* +0x0D8 font size */
    int nlines;                        /* +0x0DC */
    int w;                             /* +0x0E0 */
    unsigned short *text;              /* +0x0E4 */
    int speed;                         /* +0x0E8 text speed option */
    int input;                         /* +0x0EC the player advances the text */
    int style;                         /* +0x0F0 */
    FieldMsgLine line[3];              /* +0x0F4 */
    int cur;                           /* +0x13C line being typed */
    int pos;                           /* +0x140 first character of the page */
    int page;                          /* +0x144 */
    int all;                           /* +0x148 the whole text is shown */
    int done;                          /* +0x14C */
    int more;                          /* +0x150 the page is full, more text follows */
    int typed;                         /* +0x154 characters typed */
    int wide;                          /* +0x158 */
    void *cursor;                      /* +0x15C "next" cursor gim */
    unsigned int blink;                /* +0x160 */
    int lock;                          /* +0x164 */
    Vec2 pos2;                         /* +0x168 window position */
    Vec2 from;                         /* +0x170 */
    Vec2 to;                           /* +0x178 */
} FieldMsg;

extern FieldMsg field_msg;                 /* 0x08FD1FAC */
/* f_msg.c */
void fmsg_init(void *heap, int x, int w, int size, int input, int style);
void fmsg_reset(int x, int w, int size, int input, int style);
void fmsg_free(void);
int fmsg_update(void);
void fmsg_draw(void **list);
int fmsg_done(void);
void fmsg_set_cursor(void *gim);
MenuItemDef *fmsg_def(void);
Vec2 *fmsg_pos(void);
void fmsg_lock(int on);
void fmsg_set_size(int size);
void fmsg_set_input(int on);
void fmsg_set_text(unsigned short *text, int title);
void fmsg_open(void);
void fmsg_close(void);
int fmsg_active(void);
int fmsg_waiting(void);
int fmsg_moving(void);

typedef struct FieldTalkMsg {          /* 0x10 */
    int close;                         /* close the window when the text is done */
    int closing;                       /* +0x04 */
    int speaker;                       /* +0x08 slot talking (FieldCtx.speaker) */
    float t;                           /* +0x0C window slide (R hides it) */
} FieldTalkMsg;

typedef struct FieldTalkChoice {       /* 0x138 */
    MenuWidget w;                      /* the choice list */
    unsigned short *texts[5];          /* +0x120 */
    int result;                        /* +0x134 choice picked */
} FieldTalkChoice;

typedef struct FieldTalk {             /* 0x08FD212C, 0x154: the talk and choice commands */
    int active;                        /* +0x000 */
    int choosing;                      /* +0x004 */
    int state;                         /* +0x008 0 none, 1 message, 2 choice */
    FieldTalkMsg msg;                  /* +0x00C */
    FieldTalkChoice ch;                /* +0x01C */
} FieldTalk;

typedef struct FieldTalkCmd {          /* argument of the talk command */
    unsigned short *text;
    int title;
    int speaker;
} FieldTalkCmd;

extern FieldTalk field_talk;               /* 0x08FD212C */

typedef struct FieldSpr {              /* a sprite of a texture */
    short u, v, w, h;
} FieldSpr;

typedef struct FieldOff {
    int x, y;
} FieldOff;

typedef struct FieldMoneySlide {       /* 0xC */
    Vec2 pos;
    float t;
} FieldMoneySlide;

typedef struct FieldMoneyPop {         /* the amount added, 0x10 */
    Vec2 pos;
    float t;                           /* +0x08 */
    float scale;                       /* +0x0C */
} FieldMoneyPop;

typedef struct FieldMoney {            /* 0x08FD2280, 0x5C: the money counter of the HUD */
    int wait;                          /* +0x00 */
    int state;                         /* +0x04 0 off, 1 sliding in, 2 shown, 3 amount shown, 4 amount flies, 5 counting */
    int shown;                         /* +0x08 */
    int target;                        /* +0x0C */
    int delta;                         /* +0x10 */
    int step;                          /* +0x14 per frame while counting */
    Vec2 from;                         /* +0x18 */
    Vec2 to;                           /* +0x20 */
    Vec2 dfrom;                        /* +0x28 where the amount appears */
    int blink_on;                      /* +0x30 */
    int period;                        /* +0x34 */
    int duty;                          /* +0x38 */
    void *gim;                         /* +0x3C */
    FieldMoneySlide slide;             /* +0x40 */
    FieldMoneyPop pop;                 /* +0x4C */
} FieldMoney;

extern FieldMoney field_money;             /* 0x08FD2280 */

typedef struct FieldShake {            /* 0x20 */
    int forever;                       /* +0x00 */
    int power;                         /* +0x04 0..3 */
    int count;                         /* +0x08 */
    int frames;                        /* +0x0C */
    Vec4 off;                          /* +0x10 added to the eye and the target */
} FieldShake;

typedef struct FieldCam {              /* 0x08FD22E0, 0x80: camera and light of the field */
    int shaking;                       /* +0x00 */
    int x04;                           /* +0x04 */
    int pad08[2];
    FieldShake shake;                  /* +0x10 */
    Vec4 light_dir;                    /* +0x30 view direction (row 3 of the camera) */
    int light[15];                     /* +0x40 light_set data */
    int mode;                          /* +0x7C 0 day, 1 evening, 2 night */
} FieldCam;

extern FieldCam field_cam;                 /* 0x08FD22E0 */

typedef struct FieldBmCmd {            /* argument of the campus map command, 0x20 */
    int a;
    int b;
    int home;                          /* +0x08 place id the player is at */
    int *people;                       /* +0x0C place ids with someone to meet */
    int n_people;                      /* +0x10 */
    int alt;                           /* +0x14 */
    void (*on_save)(void);             /* +0x18 */
    void (*on_saved)(int ok);          /* +0x1C */
} FieldBmCmd;

typedef struct FieldBmFade {           /* 0x1C */
    int on;
    int out;                           /* +0x04 */
    unsigned int color;                /* +0x08 */
    unsigned int cur;                  /* +0x0C */
    unsigned int t;                    /* +0x10 */
    unsigned int len;                  /* +0x14 */
    int layer;                         /* +0x18 3 under the menus, 4 over them */
} FieldBmFade;

typedef struct FieldBm {               /* 0x08FD2360, 0x314: the campus map */
    int active;                        /* +0x000 */
    int res[2];                        /* +0x004 result: 0 a place / 1 the home place, place id */
    int *pack;                         /* +0x00C */
    void *island;                      /* +0x010 bm_island.gim */
    void *parts;                       /* +0x014 bm_parts.gim */
    void *whole;                       /* +0x018 bm_efct_whole.ehf */
    void *efct_m;                      /* +0x01C */
    void *arrow[4];                    /* +0x020 bm_arrow11..14.ehf */
    FieldBmCmd cmd;                    /* +0x030 */
    int loaded;                        /* +0x050 */
    int state;                         /* +0x054 */
    Menu menu;                         /* +0x058 */
    MenuItemDef item;                  /* +0x0A4 place name window */
    int slide;                         /* +0x11C */
    float slide_t;                     /* +0x120 */
    float move_t;                      /* +0x124 */
    int cur;                           /* +0x128 */
    int prev;                          /* +0x12C */
    int home;                          /* +0x130 */
    int sub;                           /* +0x134 */
    MenuWidget w;                      /* +0x138 the rooms of the academy */
    int dest;                          /* +0x258 */
    unsigned int marks[14];            /* +0x25C people at each place */
    unsigned int sub_marks[5];         /* +0x294 */
    unsigned short people[30];         /* +0x2A8 */
    int n_people;                      /* +0x2E4 */
    unsigned int blocked;              /* +0x2E8 */
    int pad2ec[2];
    int alt;                           /* +0x2F4 */
    FieldBmFade fade;                  /* +0x2F8 */
} FieldBm;

extern FieldBm field_bm;                   /* 0x08FD2360 */

typedef struct FieldPartner {          /* 0x08FD2680, 0xB0: the partner walking behind the player */
    int *file_a;                       /* +0x00 loaded model/sprite files */
    int *file_b;                       /* +0x04 */
    int active;                        /* +0x08 */
    int pad0c;
    FieldChr c;                        /* +0x10 */
    Vec3 last;                         /* +0xA0 player position of the previous frame */
    int padac;
} FieldPartner;

extern FieldPartner field_partner;

typedef struct FieldShVtx {            /* vertex of the shadow volume (texture, normal, position) */
    float u, v;
    short nx, ny, nz;
    short x, y, z;
} FieldShVtx;

typedef struct FieldShadow {           /* 0x08FD2730, 0x50: the light of the stencil shadows */
    Vec4 light[4];                     /* +0x00 matrix rows: side, direction, side, origin */
    int alpha;                         /* +0x40 */
    int pad44[3];
} FieldShadow;

typedef struct FieldDrawEnt {          /* character queued for the frame, 0x50 */
    Mat4 mtx;                          /* +0x00 copy of its shadow matrix (row 3: position) */
    int volume;                        /* +0x40 casts a stencil shadow */
    float scale;                       /* +0x44 width of the shadow */
    void *(*fn)(void *list, FieldChr *c); /* +0x48 */
    FieldChr *arg;                     /* +0x4C drawn in the last pass when arg->arg8c */
} FieldDrawEnt;

typedef struct FieldDraw {             /* 0x08FD2780, 0x28C0: draw list and cell attributes */
    FieldDrawEnt ent[130];
    int n;                             /* +0x28A0 */
    int pad28a4[3];
    int *cells;                        /* +0x28B0 one attribute per cell, 2 = taken by the partner */
    int w;                             /* +0x28B4 */
    int h;                             /* +0x28B8 */
    int pad28bc;
} FieldDraw;

extern FieldShadow field_shadow;
extern FieldDraw field_draw;

extern FieldBg field_bg_state;             /* 0x08844980 */
extern FieldLight field_bg_lights[][3];    /* 0x08838044 */
extern int field_hall_cells[14][2];        /* 0x088382FC */
extern FieldChrGlb field_chr_glb;
extern FieldPlayer field_player;           /* 0x08849DD0 */
extern FieldNpcs field_npcs;
extern FieldPoolDesc field_pools[5];       /* 0x0883C5B4 */
extern struct { int count; FieldPoolEnt e[12]; } field_pool_chr;   /* 0x08FD1284 */
extern struct { int count; FieldPoolEnt e[2]; } field_pool_chr2;   /* 0x08FD1240 */
extern struct { int count; FieldPoolEnt e[1]; } field_pool_bg;     /* 0x08FD121C */
extern struct { int count; FieldPoolEnt e[10]; } field_pool_misc;  /* 0x08FD10D8 */
extern struct { int count; FieldPoolEnt e[1]; } field_pool_voice;  /* 0x08FD10B4 */
extern FieldRes field_res;
extern FieldHud field_hud;                 /* 0x08FD1DAC */
extern FieldScTab field_scene_tab;         /* 0x08FD1B2C */
extern FieldScene field_scene;             /* 0x08FD1A74 */
extern FieldSys field_sys;                 /* 0x08FD1A44 */
extern FieldCmd field_cmds[25];            /* 0x0883C640 */
extern FieldItems field_items;             /* 0x08FD1420 */
extern FieldCtx field_ctx;                 /* 0x08FD1408 */                 /* 0x08FD10A0 */
extern FieldResEnt field_res_gim[9];      /* 0x0883BEB8 */
extern FieldGim field_gims[];              /* 0x0883C49C */               /* 0x08844DB0 */          /* 0x08844960 */
extern Vec3 field_dirs[8];                 /* 0x08834AD4 */

extern FieldBu field_bu;                   /* 0x08844620 */
extern int field_bu_xpos[3];               /* 0x08834AC8 */
extern FieldAnimFrame field_anim_frames[][19]; /* 0x08832AB8 */

extern unsigned char field_scene_data[];   /* 0x0883DCC0, given to ehscript_register */
extern int field_counter_a[];              /* 0x0883DCD0 */
extern int field_counter_b[];              /* 0x0883DD9C */
extern int field_bg;                       /* 0x0883DDF0 */
extern int field_room;                     /* 0x0883DDF4 */
extern FieldSeq field_seq;                 /* 0x0883DDF8 */
extern const char field_room_menu[];       /* 0x08839E74 "bg_01_01 [R_HOSTEL_OUTSIDE]\n..." */
extern const char field_seq_src[12][0x400];/* 0x0882F9D0 */
extern FieldSeqStep field_seq_steps[15];   /* 0x088329D0 */

/* libehsys (BOOT) */
int font_mbs_to_wcs(unsigned short *dst, unsigned int n, const char *src);
int menu_wait_select(const char *text, int idx, int (*cancel_cb)(void));
void scene_pop_to(unsigned short from, unsigned short arg);
void gfx_overlay_enable(int on);
void sys_state_set_default_callbacks(void);
int sceKernelChangeCurrentThreadAttr(unsigned int clear, unsigned int set);

void *ehf_draw(void *list, void *ehf, void *opts);
void ehf_set_anim(void *ehf, int first, int last);
int ehf_set_frame(void *e, unsigned int frame);
unsigned int ehf_get_frame_count(void *e);
void *ehf_resolve(void *f);
void ehp_mount(int slot, void *pack);
void *ehp_find(void *pack, const char *name, int flags);
int ehfile_is_gzip(const void *p);
unsigned int read_u32_before(const unsigned char *p, int off);
int gzip_decompress_alias(void *dst, int dst_len, const void *src);
unsigned int rand(void);
void *gfx_begin_2d(void *list);
void *gfx_end_2d(void *list);
void *dl_select_layer(int layer, unsigned int *size_out);
void dl_set_list(void *list);

Vec3 *vec3_scale(float s, Vec3 *out, Vec3 *a);
Vec3 *vec3_normalize(Vec3 *out, Vec3 *a);
void vec3_add(Vec3 *out, Vec3 *a, Vec3 *b);
Vec3 *vec3_sub(Vec3 *out, Vec3 *a, Vec3 *b);
float *mat3_identity(float *m);
float *mat3_rot_y(float *out, float *m, float angle);
float *mat3_transform_vec3(Vec3 *out, float *m, Vec3 *v);

int charmap_lookup2(int code, CharmapPos *out);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_ortho(float l, float r, float b, float t, float n, float f);

void model_free(void *m);
void model_update(void *model, void *dt);

void model_draw(void *model, void *params);
void model_set_mask(void *m, unsigned int mask);
void *model_hook(void *m, int which, void *v);
void mat4_identity(Mat4 *m);
void mat4_mul(Mat4 *out, Mat4 *a, Mat4 *b);
void mtx_load(Mat4 *m);
unsigned int *dl_depth_mask(unsigned int *list, int mask);
unsigned int *dl_stencil_op(unsigned int *list, int fail, int zfail, int zpass);
unsigned int *dl_stencil_func(unsigned int *list, int func, int ref, int mask);
unsigned int *dl_pixel_mask(unsigned int *list, unsigned int mask);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
char *strstr(const char *s, const char *find);

float vec3_dot(Vec3 *a, Vec3 *b);
float fabsf_vfpu(float x);

void *heap_create(void *mem, int size);
void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
int heap_free_block(void *heap);
void *cip_open_h(void *handle, int bufsize, int prefetch);
void cip_close(void *c);
int cip_is_idle(void *c);

int region_get(void);
int cfg_get_language(void);
int ehfile_get_size(int dir, const char *name);
int sprintf(char *buf, const char *fmt, ...);

void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
void color_unpack(float *out, unsigned int rgba);
void vec4_lerp2(float t, float *out, float *a, float *b);
unsigned int color_pack(float *v);
void deck_cur_get_u16_0(short *out);
void ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
float sqrtf_vfpu(float x);

int se_play(int id);
void snd_cmd_fef9(void);
int snd_count_se(void);
void *gim_set_texture_frame(void *list, void *g, int frame);
void *dl_alloc_inline(void **list, int size);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
Mat4 *mat4_translate(Mat4 *out, Mat4 *m, Vec4 *v);

void bgm_set_volume(int a, int b, int t);

void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_apply(void **list);
unsigned int *view_init(unsigned int *list);
void model_set_allocator(void *(*alloc)(unsigned int size), void (*freefn)(void *p));
int efs_set_heap(void *heap);
int efs_term(void);

void font_text_set(void *list);
void font_set_size(int w, int h);
void font_set_flag4(int on);
unsigned int font_set_color(unsigned int c);
int font_draw_text(int x, int y, int w, unsigned short *text);
void *font_text_clear(void);
unsigned short *table_get_entry(void *table, int i);
unsigned int *gfx_get_state(void);

Vec2 *vec2_lerp(Vec2 *out, Vec2 *a, Vec2 *b, float t);
Vec2 *vec2_add(Vec2 *out, Vec2 *a, Vec2 *b);

/* libfield, other files */
void *fld_heap_get(void);
void fld_chr_reset(FieldChr *c);
void fld_chr_draw(FieldChr *c);
float fld_vec3_len(Vec3 *v);
int fld_dir_from_vec(Vec3 *v, int diag);
void fld_dir_vec(Vec3 *out, int i);
int fld_in_range(int x);
void fld_pos_set(Vec3 *p, int cx, int cz);
void fld_npc_all(FieldNpc **out, int *n);
FieldChr *fld_player_get(void);
void fld_player_setup(float x, float z, int dir);
int fld_story_flag(int id, int mode);
void fld_shadow_set(Vec3 *pos, void *shadow);
void fld_cell_mark(int cx, int cz, int v);
int fld_draw_add(float scale, void *shadow, int volume, int area, void *fn, void *arg);
void fld_load_gim(int name, void (*cb)(int *file, void *arg), void *arg);
void *fld_gim_get(int i);
int fld_chr_setup(FieldChr *c, int kind, int id, int x, int z, int type, int dir, int visible,
                  int arg8c, int a, int b, int fx);
void fld_chr_get_pos(Vec3 *out, FieldChr *c);
void fld_chr_corners(float r, Vec3 *p, Vec3 *a, Vec3 *b, Vec3 *c, Vec3 *d);
void fld_map_height(float x, float z, Vec3 *p);
void fld_pos_to_cell(float x, float z, int *cx, int *cz);
int fld_map_attr(int x, int z);
FieldMap *fld_map_get(void);
void *fld_alloc(unsigned int size);            /* 0x0880BEF0 */
void fld_free(void *p);                        /* 0x0880BF00 */
void fld_unload(int kind, int name);           /* 0x0880C390 */
int fld_load_size(int kind, int chara);        /* 0x0880C450 */
void fld_load(int name, void (*cb)(int *file, void *arg), void *arg, int flag); /* 0x0880C588 */
void fld_load_cancel_all(void);                /* 0x0880C744 */
FieldCtx *fld_ctx_get(void);                   /* 0x0880CF0C */
int fld_bu_index(int chara);                   /* 0x0880ED50 */

#endif
