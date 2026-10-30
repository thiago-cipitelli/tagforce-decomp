/* libduel_draw: the duel scene (rendering of the field, cards, effects, cut-ins, LP panels, duel
   menus). Module state and the BOOT (libehsys) calls no shared header declares. Offsets are the
   ground truth; field names are partial.
   Each block below belongs to the address range in its marker; keep every block compilable. */
#ifndef LIBDUEL_DRAW_H
#define LIBDUEL_DRAW_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"
#include "game.h"
#include "libc.h"
#include "psp.h"
#include "allegrex.h"

/* ---- 0x088143A4-0x08825DDC ---- */
unsigned int *gfx_set_draw_target(unsigned int *list, int base, int fbp, int w, int h, int psm);
unsigned int *gfx_set_default_target(unsigned int *list);
unsigned int *card_draw(unsigned int *list, int x, int y, int id);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_tex_flush(unsigned int *list);
unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle);
unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr);
unsigned int *dl_tex_offset(float u, float v, unsigned int *list);
unsigned int *dl_tex_scale(float u, float v, unsigned int *list);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
unsigned int *dl_tex_env_color(unsigned int *list, unsigned int color);
unsigned int *gim_set_texture_frame(unsigned int *list, void *gim, int frame);
int strcmp(const char *a, const char *b);
int charmap_lookup(int id, int a1);
int snd_channel_get_u32(int ch);
int bgm_query(int h, int what);
unsigned int *gfx_get_state(void);
unsigned int *dl_select_layer(int layer, int *size);
int dl_get_layer(void);
int dl_set_list(unsigned int *list);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(float *v);
void mtx_mult(float *m);
void mtx_get34(float *m);
float *quat_to_mat4(float *out, float *q);
void mat4_mul(float *r, float *a, float *b);
void model_draw(void *mdl, void *draw);
void *model_hook(void *mdl, int which, void *fn);
float vec4_distance_xyz(float *a, float *b);
unsigned int *dl_alpha_test(unsigned int *list, int func, int ref, int mask);
unsigned int *dl_depth_mask(unsigned int *list, int mask);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
unsigned int *gim_set_texture(unsigned int *list, void *gim);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
void se_play(int id);
unsigned int *gfx_begin_2d(unsigned int *list);
unsigned int *gfx_end_2d(unsigned int *list);
int ehf_get_frame_count(void *e);
int ehf_get_cur_frame(void *e);
unsigned int *ehf_draw(unsigned int *list, void *e, void *opts);
int ehf_get_data_ptr(void *e);
void ehf_get_size(void *e, int *target, int *limit);
int ehf_set_frame_alias(void *e, int frame);
int ehf_set_anim(void *e, int target, int limit);
void req_flag_set(int i);
int cfg_get_bits_1_3_4(void);
int cfg_get_language(void);
int sysvar_get_a98(void);


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

/* ---- end 0x088143A4-0x08825DDC ---- */

#endif
