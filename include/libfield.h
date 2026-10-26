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
    int pad0c[2];
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
    int pad2c[5];
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

typedef struct FieldMap {              /* fld_map_get() */
    int pad00[2];
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

typedef struct FieldLight {            /* 0x08838044[bg][3] */
    int light;
    int pad;
} FieldLight;

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
extern FieldRes field_res;                 /* 0x08FD10A0 */
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

/* libfield, other files */
void fld_chr_reset(FieldChr *c);
void fld_chr_draw(FieldChr *c);
float fld_vec3_len(Vec3 *v);
int fld_dir_from_vec(Vec3 *v, int diag);
void fld_dir_vec(Vec3 *out, int i);
int fld_in_range(int x);
void fld_pos_set(Vec3 *p, int cx, int cz);
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
