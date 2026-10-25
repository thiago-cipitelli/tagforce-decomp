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

typedef struct DtEnt {           /* one card line, 6 bytes (libdeck DkEnt) */
    unsigned short id;           /* card id */
    unsigned short key;          /* sort key */
    unsigned char count : 7;     /* +0x04 copies */
    unsigned char fixed : 1;     /*       rented to the partner, cannot be removed */
    unsigned char marked : 1;    /* +0x05 */
    unsigned char b5_1 : 7;
} DtEnt;

typedef struct DtList {          /* 0x20: one card list (libdeck DkList) */
    unsigned short tab;          /* +0x00 index in dt_lists */
    unsigned short pad02;
    DtEnt *ents;                 /* +0x04 */
    unsigned short n;            /* +0x08 entries in ents */
    unsigned short pad0a;
    int total;                   /* +0x0C */
    unsigned short *order;       /* +0x10 visible entries (indices into ents), sorted/filtered */
    unsigned short count;        /* +0x14 entries in order */
    union {
        unsigned short u16;      /* +0x16 */
        struct {
            unsigned short sort : 4;   /* sort key */
            unsigned short dir : 2;    /* sort direction */
            unsigned short filter : 4; /* filter */
            unsigned short pad16 : 6;
        };
    };
    unsigned short u18;          /* +0x18 */
    unsigned short u1a;          /* +0x1A */
    union {
        unsigned short u1c;      /* +0x1C */
        struct {
            unsigned char u1c_lo : 4;
            unsigned char u1c_hi : 4;
            unsigned char u1d_lo : 4;  /* +0x1D */
            unsigned char u1d_hi : 4;
        };
    };
    unsigned short pad1e;
} DtList;

typedef struct DtLayout {        /* 0x1E (libdeck DkLayout) */
    short x, y, w, h;            /* +0x00 frame of the pane */
    short row_x, row_y;          /* +0x08 first row (list mode) */
    unsigned short rows;         /* +0x0C visible rows (list mode) */
    short row_w, row_h;          /* +0x0E */
    short grid_x, grid_y;        /* +0x12 first cell (grid mode) */
    unsigned short cols;         /* +0x16 columns (grid mode) */
    short cell_w;                /* +0x18 */
    unsigned short grid_rows;    /* +0x1A visible rows (grid mode) */
    short cell_h;                /* +0x1C */
} DtLayout;

typedef struct DtPane {          /* 0x2C (libdeck DkPane) */
    DtList *list;                /* +0x00 */
    unsigned char list_mode : 1; /* +0x04 one card per row (else grid) */
    unsigned char pad04 : 7;
    unsigned char pad05;
    unsigned short top;          /* +0x06 first visible entry */
    unsigned short cx;           /* +0x08 cursor column */
    unsigned short cy;           /* +0x0A cursor row */
    DtLayout lay;                /* +0x0C */
    unsigned short pad2a;
} DtPane;

typedef struct DtScroll {        /* analog-scrolled value */
    int id;                      /* +0x00 card shown (-1 none) */
    int pad04;
    unsigned int pos;            /* +0x08 */
    unsigned int max;            /* +0x0C */
} DtScroll;

typedef struct DtCursor {        /* 0x28 */
    unsigned short id;           /* +0x00 card under the cursor */
    unsigned short key;          /* +0x02 */
    unsigned short timer;        /* +0x04 frames on the same card */
    unsigned short pad06;
    DtEnt *ent;                  /* +0x08 */
    DtScroll scroll;             /* +0x0C */
    unsigned char pad1c[0x0C];
} DtCursor;

typedef struct DtRect {
    short x, y, w, h;
} DtRect;

typedef void *(*DtValueDraw)(void *list, unsigned short x, unsigned short y, unsigned short v);

typedef struct DtFilterDef {     /* values of a filter kind (dt_filter_defs) */
    unsigned short first;
    unsigned short last;
    DtValueDraw draw;            /* +0x04 */
} DtFilterDef;

typedef struct DtFilterMenu {    /* dt_fmenu 0x08824700, 0x18 bytes, heap allocated */
    unsigned short step;         /* 0 kinds, 1 values, 2 closing */
    unsigned char sub;           /* +0x02 step inside the values window */
    unsigned char alt : 1;       /* +0x03 some card has an alternate picture */
    unsigned char col : 4;       /*       cursor column */
    unsigned char pad03 : 3;
    unsigned char page : 4;      /* +0x04 cursor row, 0xF = the OK button */
    unsigned char pad04 : 4;
    unsigned char pad05[3];
    unsigned int mask[2];        /* +0x08 values picked */
    struct MenuWidget *kinds;    /* +0x10 */
    struct MenuWidget *vals;     /* +0x14 */
} DtFilterMenu;

extern DtFilterMenu *dt_fmenu;   /* 0x08824700 */
extern DtFilterDef dt_filter_defs[9]; /* 0x08821FAC */

typedef struct DtCfg {           /* dt_init cfg */
    void *heap;                  /* parent heap (0: own 5 MB block) */
    void *cip;
} DtCfg;

typedef struct DtState {         /* dt 0x088228D0 (.bss), cleared by dt_init (0x107C bytes) */
    unsigned short lesson;       /* +0x000 */
    unsigned short state;        /* +0x002 0 load, 1 fade in, 2 run, 3 fade out, 4 unload */
    int done;                    /* +0x004 actions done toward the step goal */
    void *parent_heap;           /* +0x008 */
    unsigned char text_on;       /* +0x00C bit 0: overlays drawn */
    unsigned char step;          /* +0x00D lesson step kind (9: buttons, 16: one button) */
    unsigned short show;         /* +0x00E bit 0 window, 1 highlight, 2 buttons, 4 button, 6 menu */
    unsigned char pad010[8];
    PadBits pad;                 /* +0x018 input given to the editor */
    unsigned short goal;         /* +0x040 action the step waits for (3 a move opened, 4 a card put) */
    unsigned short goal_n;       /* +0x042 times it must be done */
    unsigned char pad044[4];
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

typedef struct DtView {          /* dtv 0x08823950, 0x94 bytes: the ScView of libselect_card */
    union {
        struct {
            unsigned char mode : 4;    /* +0x00 pane layout */
            unsigned char side : 2;    /*       pane with the cursor */
            unsigned char busy : 1;
            unsigned char b07 : 1;
        };
        struct {
            unsigned short f00 : 7;
            unsigned short popup : 8;  /*       popup being shown */
            unsigned short f15 : 1;
        };
    };
    unsigned short pad02;
    int scroll_max;              /* +0x04 */
    int scroll_pos;              /* +0x08 */
    unsigned char lock : 1;      /* +0x0C */
    unsigned char pad0c : 7;
    unsigned char pad0d[3];
    DtCursor cursor;             /* +0x10 */
    unsigned char info : 4;      /* +0x38 */
    unsigned char no_draw : 1;   /*       the editor is not drawn (card info screen) */
    unsigned char pad38 : 3;
    unsigned char pad39[3];
    DtPane pane[2];              /* +0x3C */
} DtView;

extern DtView dtv;               /* 0x08823950 */

typedef struct DtSt {            /* dts 0x088239F0, 0x3D4 bytes: the editor state */
    DtList list[6];              /* +0x000 0 main, 1 side, 2 extra, 3 trunk, 4 extra (view), 5 rented */
    unsigned short name[0x20];   /* +0x0C0 deck name */
    unsigned short state;        /* +0x100 0 idle, 1 sort menu, 2 actions, 3 search, 4 confirm,
                                    5 result, 6/7 help */
    unsigned short unload_step;  /* +0x102 */
    unsigned char pad104[4];
    unsigned short n_owned;      /* +0x108 different cards owned */
    unsigned short n_type[6];    /* +0x10A cards of the deck per attribute icon (side deck in 3) */
    unsigned char pad116[0x368 - 0x116];
    DtRect focus;                /* +0x368 frame around the focused popup */
    unsigned char pad370[0x3BC - 0x370];
    void *heap;                  /* +0x3BC */
    void *cip;                   /* +0x3C0 */
    void *cfg_cip;               /* +0x3C4 */
    unsigned char pad3c8[8];
    void *mark;                  /* +0x3D0 heap level of the lists */
} DtSt;

extern DtSt dts;

typedef struct DtFlags {         /* dt_flags 0x08823AF6 (dts+0x106) */
    unsigned char lo : 4;
    unsigned char info : 1;      /* bit 4: open the card info screen */
    unsigned char deck_op : 2;   /* bits 5-6: deck operation picked in the action menu */
    unsigned char b7 : 1;
} DtFlags;

typedef struct DtAction {        /* dt_action 0x08824714, 0x18 bytes, heap allocated */
    unsigned char no_ent : 1;    /* +0x00 opened without a card (the first two items hidden) */
    unsigned char pad00 : 7;
    unsigned char pad01;
    unsigned short step;         /* +0x02 */
    DtEnt *ent;                  /* +0x04 */
    unsigned short mode;         /* +0x08 layout being picked */
    unsigned short side;         /* +0x0A pane of the other list */
    unsigned short tab;          /* +0x0C its list */
    unsigned short mark;         /* +0x0E "new" mark picked */
    struct MenuWidget *menu;     /* +0x10 */
    struct MenuWidget *sub;      /* +0x14 */
} DtAction;

extern DtAction *dt_action;      /* 0x08824714 */
extern unsigned short dt_undo_count; /* 0x08823D06 (dts+0x316) entries in the undo history */
extern struct Menu dt_menu;      /* 0x08823D60 (dts+0x370) */
extern unsigned short dt_scene_arg; /* 0x08823AF4 (dts+0x104) */
extern DtFlags dt_flags;         /* 0x08823AF6 */
extern unsigned char dt_help_page; /* 0x08823AF7 */

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
