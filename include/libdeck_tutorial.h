/* libdeck_tutorial: the deck-building lesson (imported by libtitle, libstory, liblabo and libdeck).
   A copy of the deck editor (two card panes, sort/filter menus, word search, card actions) driven by
   a lesson script, with highlighted screen areas, button hints and the talk text box of libtutorial.
   Module state and the BOOT (libehsys) functions it calls that no shared header declares. */
#ifndef LIBDECK_TUTORIAL_H
#define LIBDECK_TUTORIAL_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"
#include "libselect_card.h"

typedef struct DtCfg {           /* dt_init cfg */
    void *heap;                  /* parent heap (0: own 5 MB block) */
    void *cip;
} DtCfg;

typedef struct DtState {         /* dt 0x088228D0 (.bss), cleared by dt_init (0x107C bytes) */
    unsigned short lesson;       /* +0x000 */
    unsigned short state;        /* +0x002 0 load, 1 fade in, 2 run, 3 fade out, 4 unload */
    unsigned char pad004[4];
    void *parent_heap;           /* +0x008 */
    unsigned char text_on;       /* +0x00C bit 0: overlays drawn */
    unsigned char step;          /* +0x00D lesson step kind (9: buttons, 16: one button) */
    unsigned short show;         /* +0x00E bit 0 window, 1 highlight, 2 buttons, 4 button, 6 menu */
    unsigned char pad010[8];
    unsigned char script[0x30];  /* +0x018 lesson script runner (dt_script_*) */
    unsigned short btn_mode;     /* +0x048 */
    unsigned short pad04a;
    unsigned int btn_mask;       /* +0x04C buttons shown with step 9 */
    unsigned short hl;           /* +0x050 highlighted area */
    unsigned short pad052;
    unsigned int btn_mask2;      /* +0x054 */
    unsigned int btn_one;        /* +0x058 */
    unsigned char pad05c[6];
    unsigned char text_box : 1;  /* +0x062 talk box shown */
    unsigned char board : 1;     /* full-screen board shown */
    unsigned char pad062 : 6;
    unsigned char pad063;
    MenuWidget win;              /* +0x064 */
    MenuWidget board_win;        /* +0x184 */
    MenuWidget menu;             /* +0x2A4 */
    unsigned char quit_open : 1; /* +0x3C4 */
    unsigned char pad3c4 : 7;
    unsigned char pad3c5[3];
    MenuWidget quit;             /* +0x3C8 */
    unsigned char saved_flags[0xB87]; /* +0x4E8 inventory flag 7 of cards 0xFA7..0x1B2D */
    unsigned char pad106f;
    unsigned short saved_cfg;    /* +0x1070 */
    unsigned short pad1072;
    int fpl;                     /* +0x1074 */
    void *fpl_mem;               /* +0x1078 */
} DtState;

extern DtState dt;
extern ScView dtv;               /* 0x08823950 */

typedef struct DtSt {            /* dts 0x088239F0, 0x3D4 bytes */
    unsigned char pad000[0x102];
    unsigned short unload_step;  /* +0x102 */
    unsigned char pad104[0x3BC - 0x104];
    void *heap;                  /* +0x3BC */
    void *cip;                   /* +0x3C0 */
    void *cfg_cip;               /* +0x3C4 */
    unsigned char pad3c8[8];
    void *mark;                  /* +0x3D0 heap level of the lists */
} DtSt;

extern DtSt dts;

typedef struct DtRes {           /* dt_res 0x08823DD0 */
    unsigned char ready : 1;
    unsigned char step : 7;
} DtRes;

extern DtRes dt_res;
extern void *dt_pack;            /* 0x08823DC4 deck/tutorial.ehp */

/* BOOT (libehsys) */
void *memcpy(void *, const void *, unsigned int);
int sprintf(char *, const char *, ...);
int cfg_get_language(void);
int cfg_get_byte0_hi2(void);
void cfg_set_byte0_hi2(int v);
int heap_free_block(void *heap);
void font_set_line_extra(int v);
void *dl_stencil_func(void *list, int func, int ref, int mask);
void *dl_stencil_op(void *list, int fail, int zfail, int zpass);
int sceKernelCreateFpl(const char *name, int part, int attr, unsigned int size, unsigned int blocks, void *opt);
int sceKernelAllocateFpl(int fpl, void **data, unsigned int *timeout);
int sceKernelChangeCurrentThreadAttr(unsigned int clear, unsigned int set);

/* dt_main.c */
int dt_unload(void);
void *dt_draw_highlight(void *list, unsigned short kind);
void *dt_draw_button(void *list, int x, int y, unsigned short kind, int lit, int arrow);
void *dt_draw_dpad(void *list, int x, int y, unsigned short lit, unsigned short arrow);
void *dt_draw_buttons(void *list);
void *dt_draw_one_button(void *list);
void *dt_draw_hint_buttons(void *list);
void *dt_draw_board(void *list);
void dt_res_init(void);
int dt_load_step(void);

/* dt_res.c */
void dt_pack_free(void);
void dt_pack_release(void);
void *dt_pack_load(void);
void *dt_res_get(unsigned short id);

#endif
