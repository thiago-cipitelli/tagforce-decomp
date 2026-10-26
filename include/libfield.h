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

#endif
