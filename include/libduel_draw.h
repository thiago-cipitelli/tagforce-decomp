/* libduel_draw: the duel scene (field, cards, effects, cut-ins, LP panels, duel menus).
   Module state and the BOOT (libehsys) calls no shared header declares. Offsets are the ground
   truth; field names are partial. Each block below belongs to the address range in its marker;
   keep every block compilable. */
#ifndef LIBDUEL_DRAW_H
#define LIBDUEL_DRAW_H

#include "ehsys.h"
#include "menu.h"
#include "libc.h"
#include "psp.h"

/* ---- 0x08825DDC-0x08837098 ---- */
/* BOOT display-list calls (signatures of src/BOOT) */
unsigned int *dl_select_layer(int layer, int *size);
int dl_set_list(unsigned int *list);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_color(unsigned int *list, unsigned int color);
unsigned int *dl_light_mode(unsigned int *list, int mode);
unsigned int *dl_ambient_light(unsigned int *list, unsigned int color);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
unsigned int *dl_material_colors(unsigned int *list, unsigned int emissive, unsigned int ambient, unsigned int diffuse, unsigned int specular);
unsigned int *dl_color_material(unsigned int *list, int comps);
unsigned int *dl_shade_model(unsigned int *list, int mode);
unsigned int *dl_front_face(unsigned int *list, int cw);
unsigned int *dl_tex_map_mode(unsigned int *list, int mode, int a, int b);
unsigned int *dl_tex_offset(unsigned int *list, float u, float v);
unsigned int *dl_tex_scale(unsigned int *list, float u, float v);
unsigned int *dl_alpha_test(unsigned int *list, int func, int ref, int mask);
unsigned int *dl_depth_func(unsigned int *list, int func);
unsigned int *dl_depth_mask(unsigned int *list, int mask);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);

#define DD_TASK_MAX 254

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

void *dd_fb_addr(int back);            /* VRAM address of the front (0) or back (1) frame buffer */
void *dd_vram_addr(int offset);

typedef int (*DdTaskUpdateFn)(int state, void *mem);   /* returns the next state */
typedef void (*DdTaskTermFn)(void *arg);

typedef struct DdTask {                /* 0x1C */
    int state;                         /* +0x00 0 free, 1/2 running, 3 killed, 4 freed next frame */
    unsigned int flags;                /* +0x04 bit 0: paused (dd_task_pause) */
    int handle;                        /* +0x08 slot << 24 | serial, -1 when free */
    DdTaskUpdateFn update;             /* +0x0C */
    DdTaskTermFn term;                 /* +0x10 called with arg when the slot is freed */
    void *arg;                         /* +0x14 */
    void *mem;                         /* +0x18 zeroed work buffer from the duel heap */
} DdTask;

typedef struct DdTaskMgr {             /* 0x08C58B90 */
    unsigned int serial;               /* +0x0000 low 24 bits of the next handle */
    DdTask task[DD_TASK_MAX];          /* +0x0004 */
    unsigned char pad1bcc[0x124];      /* cleared with the table (0x1CF0 bytes) */
} DdTaskMgr;

int dd_task_create(DdTaskUpdateFn update, DdTaskTermFn term, void *arg, int size);
int dd_task_kill(int id);
int dd_task_alive(int id);
void *dd_task_mem(int id);
int dd_task_flag_set(int id, unsigned int flags);
int dd_task_flag_test(int id, unsigned int flags);
int dd_task_pause(int id);

#endif
