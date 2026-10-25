/* libdeck: deck editor scene (scene 4 of the BOOT scene table). Shared state and the BOOT (libehsys)
   calls it uses; prototypes copy the BOOT definitions where the parameter types decide the caller's
   masking. Several files are written in parallel: append new declarations at the end of the section of
   your file, never change a line another file may rely on. */
#ifndef LIBDECK_H
#define LIBDECK_H

#include "ehsys.h"
#include "font.h"
#include "psp.h"
#include "libc.h"
#include "cip.h"
#include "card_db.h"
#include "game.h"

typedef struct DkEnt {                 /* one card line, 6 bytes (same layout as libselect_card ScEnt) */
    unsigned short id;                 /* card id */
    unsigned short key;                /* sort key */
    unsigned char count : 7;           /* +0x04 copies */
    unsigned char fixed : 1;
    unsigned char marked : 1;          /* +0x05 */
    unsigned char b5_1 : 7;
} DkEnt;

typedef struct DkList {                /* 0x20: one card list (owned cards, deck, side...) */
    unsigned short tab;                /* +0x00 index in dk_lists */
    unsigned short pad02;
    DkEnt *ents;                       /* +0x04 */
    unsigned short n;                  /* +0x08 entries in ents */
    unsigned short pad0a;
    int total;                         /* +0x0C */
    unsigned short *order;             /* +0x10 visible entries (indices into ents), sorted/filtered */
    unsigned short count;              /* +0x14 entries in order */
    union {
        unsigned short u16;            /* +0x16 */
        struct {
            unsigned short sort : 4;   /*       sort key */
            unsigned short dir : 2;    /*       sort direction */
            unsigned short filter : 4; /*       filter */
            unsigned short pad16 : 6;
        };
    };
    unsigned short u18;                /* +0x18 */
    unsigned short u1a;                /* +0x1A */
    union {
        unsigned short u1c;            /* +0x1C */
        struct {
            unsigned char u1c_lo : 4;
            unsigned char u1c_hi : 4;
            unsigned char u1d_lo : 4;  /* +0x1D */
            unsigned char u1d_hi : 4;
        };
    };
    unsigned short pad1e;
} DkList;

extern DkList dk_lists[6];             /* 0x095D2330 */
extern void *dk_heap;                  /* 0x095D26EC */

/* BOOT (libehsys) */
int sceKernelFreeFpl(int fpl, void *data);
int sceKernelDeleteFpl(int fpl);
void *heap_create(void *mem, unsigned int size);
void *heap_alloc(void *h, int n);
int heap_set_level(void *h, void *p);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehp_mount(void (*cb)(void *), void *pack);
void *ehp_find(void *pack, const char *name, int *size);
void bgm_play(int id);
void se_play(int id);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_apply(void **list);
void gfx_overlay_enable(int on);
unsigned short region_pick_value(int flag);
unsigned short *table_get_entry(void *t, unsigned int i);
unsigned int *gfx_get_state(void);
void cardinfo_draw(void);
void *gfx_begin_2d(void *list);
void *gfx_end_2d(void *list);
void *ehf_draw(void *list, void *ehf, void *opts);
int ehf_set_anim(void *e, int target, int limit);
void *gim_set_texture(void *list, void *gim);
void *gim_set_texture_frame(void *list, void *gim, int frame);
void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_tex_func(void *list, int func, int comp);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_alloc_inline(void **list, int bytes);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
int inv_get_bit3(unsigned short id);
int inv_get_hi(unsigned short id);
int inv_get_flag7(unsigned short id);
void inv_set_flag7(unsigned short id, int on);
int inv_get_count(unsigned short id);

/* ---- per-file additions below (append only; keep each file's block together) ---- */

/* dk_main.c (W2): the view state at 0x095D2290, same layout as libselect_card ScView (0x94 bytes) */
typedef struct DkLayout {              /* 0x1E */
    short x, y, w, h;                  /* +0x00 frame of the pane */
    short row_x, row_y;                /* +0x08 first row (list mode) */
    unsigned short rows;               /* +0x0C visible rows (list mode) */
    short row_w, row_h;                /* +0x0E */
    short grid_x, grid_y;              /* +0x12 first cell (grid mode) */
    unsigned short cols;               /* +0x16 columns (grid mode) */
    short cell_w;                      /* +0x18 */
    unsigned short grid_rows;          /* +0x1A visible rows (grid mode) */
    short cell_h;                      /* +0x1C */
} DkLayout;

typedef struct DkPane {                /* 0x2C */
    DkList *list;                      /* +0x00 */
    unsigned char list_mode : 1;       /* +0x04 one card per row (else grid) */
    unsigned char pad04 : 7;
    unsigned char pad05;
    unsigned short top;                /* +0x06 first visible entry */
    unsigned short cx;                 /* +0x08 cursor column */
    unsigned short cy;                 /* +0x0A cursor row */
    DkLayout lay;                      /* +0x0C */
    unsigned short pad2a;
} DkPane;

typedef struct DkScroll {              /* analog-scrolled value */
    int id;                            /* +0x00 card shown (-1 none) */
    int pad04;
    unsigned int pos;                  /* +0x08 */
    unsigned int max;                  /* +0x0C */
} DkScroll;

typedef struct DkCursor {              /* 0x28 */
    unsigned short id;                 /* +0x00 card under the cursor */
    unsigned short key;                /* +0x02 */
    unsigned short timer;              /* +0x04 frames on the same card */
    unsigned short pad06;
    DkEnt *ent;                        /* +0x08 */
    DkScroll scroll;                   /* +0x0C */
    unsigned char pad1c[0x0C];
} DkCursor;

typedef struct DkView {                /* dkv 0x095D2290, 0x94 bytes cleared by dk_init */
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
    int scroll_max;                    /* +0x04 */
    int scroll_pos;                    /* +0x08 */
    unsigned char lock : 1;            /* +0x0C */
    unsigned char pad0c : 7;
    unsigned char pad0d[3];
    DkCursor cursor;                   /* +0x10 */
    unsigned char info : 1;            /* +0x38 card info (cardinfo_*) shown */
    unsigned char recipe : 1;          /*       librecipe_viewer shown */
    unsigned char tutorial : 1;        /*       libdeck_tutorial shown */
    unsigned char help : 1;            /*       help viewer shown */
    unsigned char loading : 1;         /*       a sub-screen module is being loaded */
    unsigned char pad38 : 3;
    unsigned char pad39[3];
    DkPane pane[2];                    /* +0x3C */
} DkView;

extern DkView dkv;                     /* 0x095D2290 */

/* dk_fmenu.c (W3): the filter menu (kinds list + values grid), same layout as libselect_card ScFilterMenu */
typedef void *(*DkValueDraw)(void *list, unsigned short x, unsigned short y, unsigned short v);

typedef struct DkFilterDef {           /* values of a filter kind (dk_filter_defs) */
    unsigned short first;
    unsigned short last;
    DkValueDraw draw;                  /* +0x04 */
} DkFilterDef;

typedef struct DkFilterMenu {          /* dk_fmenu 0x095D2718, 0x18 bytes, heap allocated */
    unsigned short step;               /* 0 kinds, 1 values, 2 closing */
    unsigned char sub;                 /* +0x02 step inside the values window */
    unsigned char alt : 1;             /* +0x03 some card has an alternate picture */
    unsigned char col : 4;             /*       cursor column */
    unsigned char pad03 : 3;
    unsigned char page : 4;            /* +0x04 cursor row, 0xF = the OK button */
    unsigned char pad04 : 4;
    unsigned char pad05[3];
    unsigned int mask[2];              /* +0x08 values picked */
    struct MenuWidget *kinds;          /* +0x10 */
    struct MenuWidget *vals;           /* +0x14 */
} DkFilterMenu;

extern DkFilterMenu *dk_fmenu;         /* 0x095D2718 */
extern DkFilterDef dk_filter_defs[9];  /* 0x08822F94 */

/* dk_main.c (W2): scene globals (inside the 0x3D4 block cleared from dk_lists) */
extern unsigned short dk_scene_arg;    /* 0x095D2434 argument of the scene transition (2: back to the field) */
extern unsigned char dk_profile[];     /* 0x095D2446 sysstate blob */
extern unsigned char dk_deck_name[];   /* 0x095D23F0 0x40 bytes */
extern struct Menu dk_menu;            /* 0x095D26A0 */
extern Cip *dk_cip;                    /* 0x095D26F0 */
extern int dk_module;                  /* 0x095D26F8 loaded sub-screen module (librecipe_viewer, libdeck_tutorial) */
void dk_setup(void);
void dk_draw(void);

/* dk_lists.c (W1): the block at dk_lists seen as one struct (alias dk_deck), for the fields the
   original addresses from the dk_lists base */
typedef struct DkDeck {                /* 0x095D2330 */
    DkList lists[6];                   /* +0x000 0 main, 1 side, 2 extra, 3 trunk, 4 extra (view), 5 rented */
    unsigned short name[32];           /* +0x0C0 dk_deck_name */
    unsigned char pad100[8];
    unsigned short owned_kinds;        /* +0x108 cards owned at least once */
    unsigned short attr_count[6];      /* +0x10A cards of the deck per attribute icon */
} DkDeck;

extern DkDeck dk_deck;                 /* 0x095D2330 alias of dk_lists */
extern unsigned short dk_undo_count;   /* 0x095D2646 entries in the undo history */
int dk_list_find(DkList *l, unsigned short id);
unsigned short dk_list_count_id(DkList *l, unsigned short id, int same_name);
int dk_list_remove(DkList *l, int i);
void dk_list_add(DkList *l, DkEnt *src, int pos);
void dk_list_total(DkList *l);
void dk_attr_count_update(void);

/* dk_action.c (W3): scene state words and the card action menu (same layout as libselect_card ScAction) */
typedef struct DkFlags {               /* dk_flags 0x095D2436 */
    unsigned char lo : 4;
    unsigned char info : 1;            /* bit 4: open the card info screen */
    unsigned char deck_op : 2;         /* bits 5-6: 1 / 2 deck operation picked in the action menu */
    unsigned char b7 : 1;
} DkFlags;

typedef struct DkAction {              /* dk_action 0x095D271C, 0x18 bytes, heap allocated */
    unsigned char no_ent : 1;          /* +0x00 opened without a card (the first two items hidden) */
    unsigned char pad00 : 7;
    unsigned char pad01;
    unsigned short step;               /* +0x02 */
    DkEnt *ent;                        /* +0x04 */
    unsigned short mode;               /* +0x08 layout being picked */
    unsigned short side;               /* +0x0A pane of the other list */
    unsigned short tab;                /* +0x0C its list */
    unsigned short mark;               /* +0x0E "new" mark picked */
    struct MenuWidget *menu;           /* +0x10 */
    struct MenuWidget *sub;            /* +0x14 */
} DkAction;

extern unsigned short dk_state;        /* 0x095D2430 input state of the scene (0 lists, 2 action menu...) */
extern DkFlags dk_flags;               /* 0x095D2436 */
extern unsigned char dk_help_page;     /* 0x095D2437 help page asked for with START */
extern DkAction *dk_action;            /* 0x095D271C */

/* dk_pane.c (W2) */
unsigned short dk_cur_index(unsigned short side);
DkEnt *dk_cur_entry(unsigned short side);
void dk_info_update(void);
void dk_select_tab(unsigned short side, unsigned short tab, unsigned short how);
unsigned short *dk_list_name(unsigned short tab);
void dk_save_panes(unsigned short which);
void *dk_res_get(unsigned short id);

/* dk_scene.c (W2) */
extern unsigned short dk_substep;      /* 0x095D2432 step of the sub-screen being run */
void dk_check_rewards(void);
void dk_name_input_open(void *a, void *b, void *c);
void dk_name_input_update(void);
void dk_update(void);

#endif
