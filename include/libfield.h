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

typedef struct FieldCtx {              /* fld_ctx_get() */
    unsigned int frame;                /* +0x00 frame counter */
    int pad04;
    int speaker;                       /* +0x08 slot talking, -1 none */
} FieldCtx;

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

extern FieldChrGlb field_chr_glb;          /* 0x08844960 */
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

/* libfield, other files */
void *fld_alloc(unsigned int size);            /* 0x0880BEF0 */
void fld_free(void *p);                        /* 0x0880BF00 */
void fld_unload(int kind, int name);           /* 0x0880C390 */
int fld_load_size(int kind, int chara);        /* 0x0880C450 */
void fld_load(int name, void (*cb)(int *file, void *arg), void *arg, int flag); /* 0x0880C588 */
void fld_load_cancel_all(void);                /* 0x0880C744 */
FieldCtx *fld_ctx_get(void);                   /* 0x0880CF0C */
int fld_mode_get(void);                        /* 0x0880ED50 */

#endif
