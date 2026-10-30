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

/* the charge trail: three ribbons following a point of the card model, a glow at its head and, for
   the second variant, sparks and a ring that refracts the screen (task data, 0x7F70 bytes) */
typedef struct DdCVtx {                /* COLOR8888 | VERTEX_32BITF */
    unsigned int color;
    float x, y, z;
} DdCVtx;

typedef struct DdTCVtx {               /* TEXTURE_32BITF | COLOR8888 | VERTEX_32BITF */
    float u, v;
    unsigned int color;
    float x, y, z;
} DdTCVtx;

typedef struct DdTrailSpark {          /* 0xE8 */
    int flags;                         /* bit 0: alive */
    int t;
    struct {
        int frame;                     /* 0-3: d->sq[frame] */
        ScePspFVector3 pos;
        ScePspFVector3 vel;
    } p[8];
} DdTrailSpark;

typedef struct DdTrailFx {
    int grow;                          /* +0x0000 frames until the trail is drawn */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    int frame;                         /* +0x0010 parity picks the vertex buffer */
    DdFxSlot *slot;                    /* +0x0014 */
    int variant;                       /* +0x0018 1: with sparks and ring (dd_fx13_start_b) */
    unsigned int se_done;              /* +0x001C bits of func_08827F68 */
    int se_a;                          /* +0x0020 */
    int se_b;                          /* +0x0024 */
    int se_at;                         /* +0x0028 */
    int r, g, b, a;                    /* +0x002C ribbon color */
    int npts;                          /* +0x003C points in the history (<= 32) */
    int nsteps;                        /* +0x0040 ribbon steps built (<= 48) */
    DdCVtx strip[3][48][3];            /* +0x0044 edge, center, edge */
    DdCVtx strip_buf[2][3][48][3];     /* +0x1B44 */
    unsigned char idx[96];             /* +0x5144 strip indices: 3i, 3i+1 */
    int pad51a4[3];
    ScePspFVector4 blade[3][2];        /* +0x51B0 the edges of the three ribbons */
    ScePspFVector4 pts[32];            /* +0x5210 history of the head position */
    ScePspFVector4 coef[32];           /* +0x5410 spline coefficients */
    ScePspFVector4 curve[32][8];       /* +0x5610 spline points, 8 per segment */
    int r2, g2, b2, a2;                /* +0x6610 glow color */
    DdTCVtx glow[6];                   /* +0x6620 fan */
    int spark_next;                    /* +0x66B0 */
    DdRingVtx sq[4][2];                /* +0x66B4 the 4 spark frames */
    DdTrailSpark spark[16];            /* +0x6754 */
    DdTCVtx fan[34];                   /* +0x75D4 the ring (screen-textured fan) */
    DdTCVtx fan_buf[2][34];            /* +0x7904 */
    int pad7f64[3];
} DdTrailFx;

void func_08827738(void);                                   /* 3D begin (duel camera matrices) */
void func_08827668(void);                                   /* 3D end */
void func_08827F14(int node, ScePspFVector3 *out);          /* position of a node of the scene */
void func_08827ED4(int node, ScePspFVector3 *out);
void func_08827258(ScePspFMatrix4 *m);                      /* billboard matrix of the duel camera */
void func_08827408(ScePspFMatrix4 *m);                      /* view x projection */
int func_08827F68(int se, int t, int at, unsigned int *done, unsigned int bit); /* sound once at `at` */
void func_08827BA4(float len, ScePspFVector3 *out);         /* random vector of length `len` */
void func_088272BC(float x, float y, float z);              /* mtx_scale3 of (x, y, z) */
void *func_08826C80(void *list);                            /* binds a copy of the frame buffer */

/* BOOT */
ScePspFVector4 *vec4_set(ScePspFVector4 *out, float x, float y, float z, float w);
ScePspFVector4 *vec4_scale(ScePspFVector4 *out, const ScePspFVector4 *v, float s);
ScePspFVector4 *vec4_add(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *mat4_transform_vec4(ScePspFVector4 *out, const ScePspFMatrix4 *m, const ScePspFVector4 *v);
int spline_init(ScePspFVector4 *coef, ScePspFVector4 *p, int n);
void spline_eval(float t, ScePspFVector4 *out, ScePspFVector4 *p, ScePspFVector4 *coef, int n);

/* the tunnel of effects 27-31, 38-41 and 54: a textured disc of 33 rings scrolling toward the center
   and light streaks bursting around it (task data, 0x67CC bytes) */
typedef struct DdStreak {              /* 0x18 */
    int t;
    int flags;                         /* bit 0: alive */
    float ang[4];
} DdStreak;

typedef struct DdTunnelFx {
    int grow;                          /* +0x0000 */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    int frame;                         /* +0x0010 texture scroll */
    DdFxSlot *slot;                    /* +0x0014 */
    unsigned int se_done;              /* +0x0018 */
    DdTCVtx grid[33][33];              /* +0x001C ring i, spoke j */
    unsigned char idx[66];             /* +0x6634 strip of two rings: j, j + 33 */
    int streak_next;                   /* +0x6678 */
    DdTCVtx streak_quad[6];            /* +0x667C */
    DdStreak streak[8];                /* +0x670C */
} DdTunnelFx;

void func_08826E30(int layer, int cx, int cy);              /* 3D begin with the screen center */

/* BOOT */
void *dl_tex_offset(float u, float v, void *list);
void mtx_rotate_z(float a);

#endif
