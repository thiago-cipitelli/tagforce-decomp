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

/* effects 32-37 / 52-53, nine styles (tasks of dd_fx32.c) */
void *func_08826BD8(void *list, int x, int y, int w, int h); /* texture = a w x h tile of the frame buffer */
void *func_08826B6C(void *list);                            /* texture = the frame buffer */
float func_08827A00(int t, int n, int end);                 /* fade out over the last n frames of end */
float roundf_vfpu(float x);                                 /* BOOT */

typedef struct DdFx32Blades {          /* 0x128: 12 blades flying toward the camera */
    int t;
    int flags;                         /* bit 0: alive */
    int frame[12];
    ScePspFVector3 pos[12];
    float speed[12];
    float ang[12];
} DdFx32Blades;

typedef struct DdFx32Motes {           /* 0x58: 4 billboards flying toward the camera */
    int t;
    int flags;
    int frame[4];
    float speed[4];
    ScePspFVector3 pos[4];
} DdFx32Motes;

typedef struct DdFx32Spin {            /* 0xE8: 8 spinning billboards */
    int t;
    int flags;
    int frame[8];
    float speed[8];
    ScePspFVector3 pos[8];
    float ang[8];
    float spin[8];
} DdFx32Spin;

typedef struct DdFx32S0 {              /* style 0, 0x3014 bytes */
    int grow;                          /* +0x0000 */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    DdFxSlot *slot;                    /* +0x0010 */
    unsigned int se_done;              /* +0x0014 */
    int blade_next;                    /* +0x0018 */
    DdRingVtx blade_v[8][9][2];        /* +0x001C 8 frames, strips of 9 segments */
    unsigned char idx[18];             /* +0x0B5C */
    DdFx32Blades blade[24];            /* +0x0B70 */
    int mote_next;                     /* +0x2730 */
    DdRingVtx mote_v[4][2];            /* +0x2734 */
    DdFx32Motes mote[24];              /* +0x27D4 */
} DdFx32S0;

typedef struct DdFx32S1 {              /* style 1, 0x21A24 bytes */
    int grow;                          /* +0x00000 */
    int grow_end;                      /* +0x00004 */
    int t;                             /* +0x00008 */
    int end;                           /* +0x0000C */
    int frame;                         /* +0x00010 */
    DdFxSlot *slot;                    /* +0x00014 */
    unsigned int se_done;              /* +0x00018 */
    int mote_next;                     /* +0x0001C */
    DdRingVtx mote_v[4][2];            /* +0x00020 */
    DdFx32Motes mote[24];              /* +0x000C0 */
    ScePspFVector3 center;             /* +0x00900 */
    DdTCVtx tube[49][33];              /* +0x0090C rings of a wavy tube going into the screen */
    DdTCVtx tube_buf[2][49][33];       /* +0x0A0A4 */
    unsigned short idx[48][66];        /* +0x1CFD4 */
    float tex_u, tex_v;                /* +0x1E894 */
    float phase[49][33];               /* +0x1E89C */
    int spin_next;                     /* +0x201E0 */
    DdRingVtx spin_v[8][4];            /* +0x201E4 */
    DdFx32Spin spin[24];               /* +0x20464 */
} DdFx32S1;

typedef struct DdFx32S2 {              /* style 2, 0xC8C8 bytes */
    int grow;                          /* +0x0000 */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    DdFxSlot *slot;                    /* +0x0010 */
    unsigned int se_done;              /* +0x0014 */
    ScePspFVector3 center;             /* +0x0018 */
    DdTCVtx tube[49][33];              /* +0x0024 */
    unsigned short idx[48][66];        /* +0x97BC */
    float tex_u, tex_v;                /* +0xB07C */
    int spin_next;                     /* +0xB084 */
    DdRingVtx spin_v[8][4];            /* +0xB088 */
    DdFx32Spin spin[24];               /* +0xB308 */
} DdFx32S2;

typedef struct DdFx32Comets {          /* 0x98: 6 comets flying toward the camera */
    int t;
    int flags;
    int frame[6];
    ScePspFVector3 pos[6];
    float speed[6];
    float ang[6];
} DdFx32Comets;

typedef struct DdFx32S3 {              /* style 3, 0x2230 bytes */
    int grow;                          /* +0x0000 */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    DdFxSlot *slot;                    /* +0x0010 */
    unsigned int se_done;              /* +0x0014 */
    int comet_next;                    /* +0x0018 */
    DdTCVtx tail_v[8][9][2];           /* +0x001C 8 frames, strips of 9 segments */
    DdTCVtx head_v[8][2];              /* +0x0D9C */
    unsigned char idx[18];             /* +0x0F1C */
    DdFx32Comets comet[32];            /* +0x0F30 */
} DdFx32S3;

typedef struct DdScrVtx {              /* TRANSFORM_2D | TEXTURE_16BIT | VERTEX_16BIT */
    short u, v;
    short x, y, z;
} DdScrVtx;

typedef struct DdFx32S4 {              /* style 4, 0xD564 bytes */
    int grow;                          /* +0x0000 */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    int frame;                         /* +0x0010 */
    DdFxSlot *slot;                    /* +0x0014 */
    unsigned int se_done;              /* +0x0018 */
    ScePspFVector3 center;             /* +0x001C */
    DdTCVtx tube[49][33];              /* +0x0028 */
    unsigned short idx[48][66];        /* +0x97C0 */
    float tex_u, tex_v;                /* +0xB080 */
    int spin_next;                     /* +0xB088 */
    DdRingVtx spin_v[8][4];            /* +0xB08C */
    DdFx32Spin spin[24];               /* +0xB30C */
    DdScrVtx grid[9][9];               /* +0xC8CC the screen as a 9 x 9 grid that wobbles */
    DdScrVtx grid_buf[2][9][9];        /* +0xCBF6 */
    unsigned char grid_idx[8][18];     /* +0xD24A */
    float phase[9][9];                 /* +0xD2DC */
    float sign[9][9];                  /* +0xD420 */
} DdFx32S4;

typedef struct DdFx32Spin4 {           /* 0x78: 4 spinning billboards */
    int t;
    int flags;
    int frame[4];
    float speed[4];
    ScePspFVector3 pos[4];
    float ang[4];
    float spin[4];
} DdFx32Spin4;

typedef struct DdFx32S5 {              /* style 5, 0xE62C bytes */
    int grow;                          /* +0x0000 */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    int frame;                         /* +0x0010 */
    DdFxSlot *slot;                    /* +0x0014 */
    unsigned int se_done;              /* +0x0018 */
    ScePspFVector3 center;             /* +0x001C */
    DdTCVtx tube[49][33];              /* +0x0028 */
    unsigned short idx[48][66];        /* +0x97C0 */
    float tex_u, tex_v;                /* +0xB080 */
    int spin_next;                     /* +0xB088 */
    DdRingVtx spin_v[8][4];            /* +0xB08C */
    DdFx32Spin4 spin[24];              /* +0xB30C */
    DdScrVtx grid[16][16];             /* +0xBE4C */
    DdScrVtx grid_buf[2][16][16];      /* +0xC84C */
    unsigned char grid_idx[15][32];    /* +0xDC4C */
    float phase[16][16];               /* +0xDE2C */
    float sign[16][16];                /* +0xE22C */
} DdFx32S5;

typedef struct DdFx32Beams {           /* 0xA8: 8 beams around the center */
    int t;
    int flags;
    ScePspFVector3 pos[8];
    float ang[8];
    float width[8];
} DdFx32Beams;

typedef struct DdFx32S6 {              /* style 6, 0x13E8 bytes */
    int grow;                          /* +0x0000 */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    DdFxSlot *slot;                    /* +0x0010 */
    unsigned int se_done;              /* +0x0014 */
    int beam_next;                     /* +0x0018 */
    DdTCVtx beam_v[49][3];             /* +0x001C edge, center, edge along z */
    unsigned char idx_a[98];           /* +0x0DE4 left half: 3i, 3i + 1 */
    unsigned char idx_b[98];           /* +0x0E46 right half: 3i + 1, 3i + 2 */
    DdFx32Beams beam[8];               /* +0x0EA8 */
} DdFx32S6;

typedef struct DdFx32S7 {              /* style 7, 0x2DEC bytes */
    int grow;                          /* +0x0000 */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    DdFxSlot *slot;                    /* +0x0010 */
    unsigned int se_done;              /* +0x0014 */
    int blade_next;                    /* +0x0018 */
    DdRingVtx blade_v[8][7][2];        /* +0x001C 8 frames, strips of 7 segments */
    unsigned char idx[14];             /* +0x08DC */
    DdFx32Blades blade[32];            /* +0x08EC */
} DdFx32S7;

typedef struct DdFx32S8 {              /* style 8, 0xB084 bytes */
    int grow;                          /* +0x0000 */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    DdFxSlot *slot;                    /* +0x0010 */
    unsigned int se_done;              /* +0x0014 */
    ScePspFVector3 center;             /* +0x0018 */
    DdTCVtx tube[49][33];              /* +0x0024 */
    unsigned short idx[48][66];        /* +0x97BC */
    float tex_u, tex_v;                /* +0xB07C */
} DdFx32S8;

/* effects 51 and 55-62: a ring that refracts the screen and a glowing diamond at a node of the card
   model (task data, 0xAB4 bytes) */
typedef struct DdRippleFx {
    int grow;                          /* +0x000 */
    int grow_end;                      /* +0x004 */
    int t;                             /* +0x008 */
    int end;                           /* +0x00C */
    int frame;                         /* +0x010 */
    DdFxSlot *slot;                    /* +0x014 */
    int node;                          /* +0x018 0x25, 0x2A for effect 62 of kind 8 */
    unsigned int se_done;              /* +0x01C */
    float ring_scale;                  /* +0x020 */
    DdTCVtx fan[34];                   /* +0x024 */
    DdTCVtx fan_buf[2][34];            /* +0x354 */
    float dia_scale;                   /* +0x9B4 */
    DdCVtx dia[5][3];                  /* +0x9B8 outer, middle, inner rings of the diamond */
    unsigned char dia_idx[10];         /* +0xAA8 */
} DdRippleFx;

void func_0884DAF8(ScePspFMatrix4 *out, int node);            /* matrix of a node of the scene */
void func_08827598(ScePspFMatrix4 *out, ScePspFMatrix4 *m);   /* rotation part of m */
void func_08827300(float x, float y, float z);              /* mtx_translate of (x, y, z) */

/* BOOT */
ScePspFVector4 *mat4_transform_xyz(ScePspFVector4 *out, const ScePspFMatrix4 *m, const ScePspFVector4 *v);
void mat4_translate(ScePspFMatrix4 *out, ScePspFMatrix4 *m, ScePspFVector4 *v);
void mtx_load(ScePspFMatrix4 *m);

/* effects 42-50, 63-64, 71-77 and 82: a ring of 16 glowing petals turning under a node of the card
   model and sparks rising from it (task data, 0x710 bytes) */
typedef struct DdHaloSpark {           /* 0x50 */
    int flags;                         /* bit 0: alive */
    int t;
    int frame[3];
    int pad14[3];
    ScePspFVector4 pos[3];
} DdHaloSpark;

typedef struct DdHaloFx {
    int grow;                          /* +0x000 */
    int grow_end;                      /* +0x004 */
    int t;                             /* +0x008 */
    int end;                           /* +0x00C */
    int frame;                         /* +0x010 */
    DdFxSlot *slot;                    /* +0x014 */
    int placed;                        /* +0x018 */
    int pad1c;
    ScePspFMatrix4 m;                  /* +0x020 frame of the halo, fixed on the first frame */
    unsigned int se_done;              /* +0x060 */
    DdTCVtx petal[4];                  /* +0x064 */
    int spark_next;                    /* +0x0C4 */
    DdRingVtx spark_v[8][4];           /* +0x0C8 */
    int pad348[2];
    DdHaloSpark spark[12];             /* +0x350 */
} DdHaloFx;

void func_0882745C(float fov, float aspect, float znear, float zfar); /* replaces the projection */
void func_08827548(void);                                   /* restores the projection */
ScePspFVector4 *func_0884DB2C(void);                        /* camera up vector */

/* BOOT */
ScePspFMatrix4 *mat4_copy(ScePspFMatrix4 *out, const ScePspFMatrix4 *m);
ScePspFMatrix4 *mat4_identity(ScePspFMatrix4 *out);
ScePspFMatrix4 *mat4_mul_rot_x(ScePspFMatrix4 *out, ScePspFMatrix4 *m, float a);
ScePspFMatrix4 *mat4_mul(ScePspFMatrix4 *out, ScePspFMatrix4 *a, ScePspFMatrix4 *b);
ScePspFVector4 *vec4_copy(ScePspFVector4 *out, const ScePspFVector4 *v);
ScePspFVector4 *vec3_cross(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec3_normalize2(ScePspFVector4 *out, const ScePspFVector4 *v);
void mtx_rotate_x(float a);
void mtx_rotate_y(float a);

/* effects 78-81: a glowing burst on the card model; variant 0 (blue) for the other kinds, variant 1
   (purple, with a flame swirl) for kinds 13-18 and 40 */
typedef struct DdBurstMote {           /* 0x10 */
    int frame;
    ScePspFVector3 pos;
} DdBurstMote;

typedef struct DdBurstPetal {          /* 0x0C */
    int frame;
    float tilt;
    float ang;
} DdBurstPetal;

typedef struct DdBurstFx {             /* variant 0, 0x166C bytes */
    int grow;                          /* +0x0000 */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    DdFxSlot *slot;                    /* +0x0010 */
    int placed;                        /* +0x0014 */
    ScePspFVector3 pos;                /* +0x0018 */
    unsigned int se_done;              /* +0x0024 */
    DdTCVtx glow[6];                   /* +0x0028 */
    DdBurstPetal petal[16];            /* +0x00B8 */
    DdRingVtx petal_v[8][4];           /* +0x0178 */
    DdCVtx ring[65][3];                /* +0x03F8 outer, middle, inner */
    unsigned char idx_a[130];          /* +0x1028 */
    unsigned char idx_b[130];          /* +0x10AA */
    DdBurstMote mote[64];              /* +0x112C */
    DdRingVtx mote_v[8][2];            /* +0x152C */
} DdBurstFx;

typedef struct DdSwirl {               /* 0x10 */
    float u, v;
    float tilt;
    float ang;
} DdSwirl;

typedef struct DdBurst2Fx {            /* variant 1, 0x18EC bytes */
    int grow;                          /* +0x0000 */
    int grow_end;                      /* +0x0004 */
    int t;                             /* +0x0008 */
    int end;                           /* +0x000C */
    int frame;                         /* +0x0010 */
    DdFxSlot *slot;                    /* +0x0014 */
    int placed;                        /* +0x0018 */
    ScePspFVector3 pos;                /* +0x001C */
    ScePspFVector3 scale;              /* +0x0028 */
    unsigned int se_done;              /* +0x0034 */
    int se;                            /* +0x0038 */
    DdTCVtx glow[6];                   /* +0x003C */
    DdSwirl swirl[16];                 /* +0x00CC */
    DdTCVtx flame[17][3];              /* +0x01CC a curved flame tongue: edge, center, edge */
    unsigned char flame_idx[34];       /* +0x0694 */
    short pad6b6;
    DdCVtx ring[65][3];                /* +0x06B8 */
    unsigned char idx_a[130];          /* +0x12E8 */
    unsigned char idx_b[130];          /* +0x136A */
    DdBurstMote mote[32];              /* +0x13EC */
    DdTCVtx mote_v[8][2];              /* +0x15EC */
    DdTCVtx mote_v2[8][2];             /* +0x176C */
} DdBurst2Fx;

/* BOOT */
ScePspFMatrix3 *mat3_rot_z(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle);

/* effects 83-90 and 118-128: a ring of petals and rising sparks under a node of the card model, and
   for the red variant also flame flares bursting around it (task data, 0x11E0 bytes) */
typedef struct DdFlare {               /* 0x28 */
    int flags;                         /* bit 0: alive */
    int t;
    int frame[4];                      /* 0-11: d->flare_v[frame] */
    float ang[4];
} DdFlare;

typedef struct DdPyreFx {
    int red;                           /* +0x000 dd_fx_table.arg4: 0 blue, else red with flares */
    int grow;                          /* +0x004 */
    int grow_end;                      /* +0x008 */
    int t;                             /* +0x00C */
    int end;                           /* +0x010 */
    int frame;                         /* +0x014 */
    DdFxSlot *slot;                    /* +0x018 */
    int placed;                        /* +0x01C */
    ScePspFMatrix4 m;                  /* +0x020 */
    unsigned int se_done;              /* +0x060 */
    DdTCVtx petal[4];                  /* +0x064 */
    int spark_next;                    /* +0x0C4 */
    DdRingVtx spark_v[8][2];           /* +0x0C8 */
    int pad208[2];
    DdHaloSpark spark[32];             /* +0x210 */
    int flare_next;                    /* +0xC10 */
    DdTCVtx flare_v[12][4];            /* +0xC14 6 frames and their mirrors */
    DdFlare flare[8];                  /* +0x1094 */
    int pad11d4[3];
} DdPyreFx;

/* BOOT */
ScePspFMatrix4 *mat4_mul_rot_y(ScePspFMatrix4 *out, ScePspFMatrix4 *m, float a);

#endif
