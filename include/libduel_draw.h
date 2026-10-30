/* libduel_draw: the duel scene (field, cards, effects, cut-ins, LP panels, duel menus). Shared types,
   one block per address range (the blocks were written in parallel and merged by concatenation).
   Offsets are the ground truth; field names are partial. */
#ifndef LIBDUEL_DRAW_H
#define LIBDUEL_DRAW_H

#include "ehsys.h"
#include "game.h"
#include "libc.h"
#include "psp.h"
#include "vfpu.h"

/* ---- 0x08837098-0x0884774C ---- */

/* one running visual effect, the user data of its task (0x08C5AB80[4]) */
typedef struct DdFxSlot {
    int task;                          /* +0x00 task handle (func_0882623C), -1 when free */
    int param;                         /* +0x04 set by dd_fx_set_param */
    int type;                          /* +0x08 kind of the owner */
    int sub;                           /* +0x0C sub-kind of the owner */
    int id;                            /* +0x10 effect id (13-376) */
    void *owner;                       /* +0x14 DdFxOwner */
} DdFxSlot;

typedef struct DdFxTable {             /* 0x08C5AB74, 0x80 bytes */
    int arg0;                          /* +0x00 passed to the effects 32-37/52-53 */
    int arg4;                          /* +0x04 passed to the effects 83-90/118-128 */
    int enable8;                       /* +0x08 effects 102-106 run only when 1 */
    DdFxSlot slot[4];                  /* +0x0C */
    int pad6c[5];
} DdFxTable;

/* the object an effect is attached to (a card model on the field; type/sub read by func_08827F54) */
typedef struct DdFxOwner {
    int type;                          /* +0x00 */
    int sub;                           /* +0x04 */
    int pad08[12];
    void *model;                       /* +0x38 Model (model_hook) */
    int pad3c[28];
    DdFxSlot *fx;                      /* +0xAC effect started by dd_fx_start */
    DdFxSlot *fx2;                     /* +0xB0 effect started by dd_fx2_start */
    DdFxSlot *fx3;                     /* +0xB4 effect started by dd_fx3_start */
} DdFxOwner;

extern DdFxTable dd_fx_table;

/* task list of the module (outside this range) */
int func_0882623C(void *update, int a, int b, int size);   /* task_create: returns a handle or -1 */
void *func_08826440(int task);                              /* task user data (zeroed, `size` bytes) */
int func_0882636C(int task);                                /* task_kill */
void func_08827F54(DdFxOwner *o, int *type, int *sub);      /* owner kind */

/* inline rand3 LCG of the BOOT (variable imports, 0 in the relocated ELF) */
typedef struct DdRandSeed {
    unsigned int lcg;                  /* x * 0x41C64E6D + 0x3039 */
    unsigned int b, c;
} DdRandSeed;
extern int rand3_slot;
extern DdRandSeed rand3_seeds[];
#define DD_RAND(r) ((r) = &rand3_seeds[rand3_slot], (r)->lcg *= 0x41C64E6D, (r)->lcg += 0x3039, \
                    (int)((r)->lcg >> 16 & 0x7FFF))

/* drawing helpers of the module (outside this range) */
int func_08828290(void);                                    /* frames elapsed since the last update */
float func_088279A4(int t, int n);                          /* clamp(t, 0, n) / n */
float func_088279D8(int t, int n);                          /* 1 - func_088279A4 */
float func_08827A2C(int t, int n, int k);                   /* ease out: sin(x*pi/2) applied k times */
float func_08827B6C(int t, int n, int k);                   /* 1 - ease in */
void *func_088266D8(void *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v);
void *func_0882698C(void *list, int on, int mask);          /* depth test */
void *func_088269D8(void *list, int on, int op, int src, int dst, int fix_a, int fix_b); /* blending */
void *func_08826A64(void *list, int tex);                   /* binds texture `tex` of the duel packs */

/* BOOT */
void *dl_select_layer(int layer, unsigned int *size);
void dl_set_list(void *list);
void *dl_color(void *list, unsigned int color);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
float sinf(float x);
float cosf(float x);

/* the five spirit orbs that rise in a row (task data, 0x219C bytes) */
typedef struct DdOrb {                 /* 0x28 */
    int flags;                         /* +0x00 bit 0: shown */
    int idx;                           /* +0x04 place in the row, also the halo sprite column */
    int t;                             /* +0x08 */
    int anim;                          /* +0x0C flame frame 0-6 (loops 4-6) */
    int tick;                          /* +0x10 */
    float phase;                       /* +0x14 bobbing phase */
    int t2;                            /* +0x18 halo timer */
    float phase2;                      /* +0x1C halo bobbing phase */
    float x, y;                        /* +0x20 */
} DdOrb;

typedef struct DdSparkSpr {            /* 0x14 */
    int u, v;
    float x, y;
    float vy;
} DdSparkSpr;

typedef struct DdSpark {               /* 0x34 */
    int flags;                         /* bit 0: alive */
    int t;
    float alpha;
    DdSparkSpr spr[2];
} DdSpark;

typedef struct DdSparkSet {            /* 0x68C sparks rising from one orb */
    int flags;
    int t;
    int next;                          /* ring index of the next spark */
    DdSpark p[32];
} DdSparkSet;

typedef struct DdOrbFx {
    int t;                             /* +0x00 */
    int end;                           /* +0x04 120 or 180 frames */
    int fade;                          /* +0x08 0-16 */
    int count;                         /* +0x0C orbs */
    int flag;                          /* +0x10 */
    int last;                          /* +0x14 the last orb was shown */
    DdOrb orb[5];                      /* +0x18 */
    DdSparkSet set[5];                 /* +0xE0 */
} DdOrbFx;

/* the ring of 20 flames turning in perspective (task data, 0x4944 bytes) */
typedef struct DdRingVtx {             /* GU_SPRITES vertex: TEXTURE_32BITF | VERTEX_32BITF */
    float u, v;
    float x, y, z;
} DdRingVtx;

typedef struct DdRingSpark {           /* 0x1C */
    int flags;                         /* bit 0: alive */
    int t;
    int frame;                         /* 0-7: d->quad[frame] */
    float vy;
    ScePspFVector3 pos;
} DdRingSpark;

typedef struct DdRingFlame {           /* 0x398 */
    int flags;                         /* bit 0: shown */
    int t;
    ScePspFVector3 pos;                /* on the ring */
    int next;                          /* ring index of the next spark */
    DdRingSpark p[32];
} DdRingFlame;

typedef struct DdRingFx {
    int t;                             /* +0x0000 */
    int end;                           /* +0x0004 -1: until stopped */
    int out;                           /* +0x0008 fade out 0-16 */
    int count;                         /* +0x000C flames (20: the full ring with the flash) */
    int shown;                         /* +0x0010 */
    int tick;                          /* +0x0014 frames to the next flame */
    int flags;                         /* +0x0018 bit 0: sound played, bit 1: flash */
    DdRingVtx quad[8][2];              /* +0x001C the 8 frames of the flame sheet */
    DdRingFlame flame[20];             /* +0x015C */
    int flash;                         /* +0x493C 0-30 */
    int flash_end;                     /* +0x4940 0-30 */
} DdRingFx;

void func_08826EE4(float fov, float aspect, float znear, float zfar, int layer, int cx, int cy,
                   ScePspFVector3 *eye, ScePspFVector3 *at, ScePspFVector3 *up); /* 3D camera begin */
void func_08826FEC(int layer);                               /* 3D camera end */
void func_08827214(ScePspFMatrix4 *m);                      /* billboard matrix of the camera */
float func_08827AD4(int t, int n, int k);                   /* ease in */
int func_0880773C(int se);                                  /* plays a sound effect */

/* BOOT */
ScePspFMatrix3 *mat3_identity(ScePspFMatrix3 *out);
ScePspFMatrix3 *mat3_rot_x(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle);
ScePspFVector3 *mat3_transform_vec3(ScePspFVector3 *out, const ScePspFMatrix3 *m, const ScePspFVector3 *v);
ScePspFVector3 *vec3_set(ScePspFVector3 *out, float x, float y, float z);
ScePspFVector3 *vec3_scale(ScePspFVector3 *out, const ScePspFVector3 *v, float s);
ScePspFVector3 *vec3_add(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
void *dl_ambient_light(void *list, unsigned int color);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(ScePspFVector3 *v);
void mtx_mult(ScePspFMatrix4 *m);
void mtx_scale3(ScePspFVector3 *v);

#endif
