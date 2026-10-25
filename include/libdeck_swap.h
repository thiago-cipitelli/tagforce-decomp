/* libdeck_swap: deck swap sub-screen between the duels of a match (built from the same source as
   libselect_card), imported by libduel_mgr. Shared state and the
   BOOT (libehsys) calls it uses; prototypes copy the BOOT definitions where the parameter types decide
   the caller's masking. */
#ifndef LIBDECK_SWAP_H
#define LIBDECK_SWAP_H

#include "ehsys.h"
#include "font.h"
#include "psp.h"
#include "libc.h"
#include "cip.h"
#include "card_db.h"
#include "game.h"

/* ds.kind: what the screen is picking for */
#define DS_KIND_RENTAL 0   /* labo/rental.ehp, the rental list comes from sysstate */
#define DS_KIND_CONV   1   /* labo/conv_machine.ehp */
#define DS_KIND_TRADE  2   /* trade.ehp */

typedef struct DsEnt {                 /* one card line, 6 bytes */
    unsigned short id;                 /* card id */
    unsigned short key;                /* sort key (row of the sort table, ds_sort_key) */
    unsigned char count : 7;           /* +0x04 copies */
    unsigned char fixed : 1;           /*       entry from the caller's list, cannot be removed */
    unsigned char marked : 1;          /* +0x05 */
    unsigned char b5_1 : 1;
    unsigned char rank : 3;            /*       rental rank (4 = none) */
    unsigned char b5_5 : 3;
} DsEnt;

typedef struct DsList {                /* 0x20: the cards of one pane */
    unsigned short tab;                /* +0x00 list shown (0 the cards owned, 1 the list being built) */
    unsigned short pad02;
    DsEnt *ents;                       /* +0x04 */
    unsigned short n;                  /* +0x08 entries in ents */
    unsigned short pad0a;
    int total;                         /* +0x0C sum of the counts */
    unsigned short *order;             /* +0x10 visible entries (indices into ents), sorted/filtered */
    unsigned short count;              /* +0x14 entries in order */
    unsigned short sort : 4;           /* +0x16 sort key */
    unsigned short dir : 2;            /*       sort direction */
    unsigned short filter : 4;         /*       filter */
    unsigned short pad16 : 6;
    unsigned short u18;                /* +0x18 */
    unsigned short u1a;                /* +0x1A */
    unsigned char u1c_lo : 4;          /* +0x1C */
    unsigned char u1c_hi : 4;
    unsigned char u1d_lo : 4;          /* +0x1D */
    unsigned char u1d_hi : 4;
    unsigned short pad1e;
} DsList;

typedef struct DsLayout {              /* 0x1E, copied from ds_layouts */
    short x, y, w, h;                  /* +0x00 frame of the pane */
    short row_x, row_y;                /* +0x08 first row (list mode) */
    unsigned short rows;               /* +0x0C visible rows (list mode) */
    short row_w, row_h;                /* +0x0E */
    short grid_x, grid_y;              /* +0x12 first cell (grid mode) */
    unsigned short cols;               /* +0x16 columns (grid mode) */
    short cell_w;                      /* +0x18 */
    unsigned short grid_rows;          /* +0x1A visible rows (grid mode) */
    short cell_h;                      /* +0x1C */
} DsLayout;

typedef struct DsPane {                /* 0x2C */
    DsList *list;                      /* +0x00 */
    unsigned char list_mode : 1;       /* +0x04 one card per row (else grid) */
    unsigned char pad04 : 7;
    unsigned char pad05;
    unsigned short top;                /* +0x06 first visible entry */
    unsigned short cx;                 /* +0x08 cursor column */
    unsigned short cy;                 /* +0x0A cursor row */
    DsLayout lay;                      /* +0x0C */
    unsigned short pad2a;
} DsPane;

typedef struct DsScroll {              /* analog-scrolled value */
    int id;                            /* +0x00 card shown (-1 none) */
    int pad04;
    unsigned int pos;                  /* +0x08 */
    unsigned int max;                  /* +0x0C */
} DsScroll;

typedef struct DsCursor {              /* 0x28 */
    unsigned short id;                 /* +0x00 card under the cursor */
    unsigned short key;                /* +0x02 */
    unsigned short timer;              /* +0x04 frames on the same card */
    unsigned short pad06;
    DsEnt *ent;                        /* +0x08 */
    DsScroll scroll;                   /* +0x0C analog-scrolled info text */
    unsigned char pad1c[0x0C];
} DsCursor;

typedef struct DsView {                /* 0x0881B540, 0x94 bytes cleared by libdeck_swap_08804008 */
    union {
        struct {
            unsigned char mode : 4;    /* +0x00 pane layout (ds_modes) */
            unsigned char side : 2;    /*       pane with the cursor */
            unsigned char busy : 1;    /*       bit 6 */
            unsigned char b07 : 1;
        };
        struct {
            unsigned short f00 : 7;
            unsigned short popup : 8;  /*       popup being shown */
            unsigned short f15 : 1;
        };
    };
    unsigned short pad02;
    int scroll_max;                    /* +0x04 wide view text scroll */
    int scroll_pos;                    /* +0x08 */
    unsigned char lock : 1;            /* +0x0C cursor info frozen */
    unsigned char pad0c : 7;
    unsigned char pad0d[3];
    DsCursor cursor;                   /* +0x10 cursor box animation (ds_draw_cursor) */
    unsigned char info : 1;            /* +0x38 card info (cardinfo_*) shown */
    unsigned char pad38 : 7;
    unsigned char pad39[3];
    DsPane pane[2];                    /* +0x3C */
} DsView;

typedef struct DsConfig {              /* libdeck_swap_08804008 cfg */
    void *heap;
    void *cip;
    void *pack_a;                      /* pack 1 already loaded by the caller */
    void *pack_b;                      /* pack 2..4 already loaded by the caller */
} DsConfig;

typedef struct DsMove {                /* a card moved between the panes, 10 bytes */
    unsigned char b[10];
} DsMove;

typedef struct DsRect {
    short x, y, w, h;
} DsRect;

typedef struct DsState {               /* ds 0x0881A680, 0x2EC bytes cleared by libdeck_swap_088157E8 */
    int kind;                          /* +0x000 */
    int state;                         /* +0x004 libdeck_swap_088158E0 state */
    unsigned short info_step;          /* +0x008 card info screen */
    unsigned short unload_step;        /* +0x00A */
    DsList list[3];                    /* +0x00C */
    unsigned short words[8][0x20];     /* +0x06C last search words (sysstate blob) */
    unsigned short attr_count[6];      /* +0x26C cards of the deck per attribute icon */
    unsigned short nhist;              /* +0x278 undo history of moves */
    DsMove hist[8];                    /* +0x27A */
    DsRect focus;                      /* +0x2CA frame around the focused popup */
    unsigned char info_open : 1;       /* +0x2D2 card info screen running */
    unsigned char pad2d2 : 7;
    unsigned char pad2d3;
    void *gs;                          /* +0x2D4 init arguments (libduel_mgr) */
    int n_partners;                    /* +0x2D8 */
    void *partners;                    /* +0x2DC */
    int time_limit;                    /* +0x2E0 */
    void *heap;                        /* +0x2E4 */
    void *cip;                         /* +0x2E8 */
} DsState;

typedef char ds_state_size_check[sizeof(DsState) == 0x2EC ? 1 : -1];

typedef struct DsLoad {                /* ds_load 0x0881B980 */
    unsigned char ready : 1;           /* no read in flight */
    unsigned char step : 7;
} DsLoad;

typedef struct DsPack {                /* ds_packs: the EHP packs, 12 bytes */
    unsigned short id;
    void *data;                        /* +0x04 */
    const char *name;                  /* +0x08 printf format, %c = language letter */
} DsPack;

typedef struct DsRes {                 /* resource tables, 12 bytes */
    unsigned short id;
    unsigned short pack;               /* +0x02 DsPack id */
    void *data;                        /* +0x04 */
    const char *name;                  /* +0x08 */
} DsRes;

typedef struct DsIdList {              /* count, then sorted (id | sub << 16) words */
    int n;
    unsigned int e[1];
} DsIdList;

typedef int DsPt;                      /* screen position packed as x | y << 16 */
/* entry at row i of the visible order, 0 past the end */
#define DS_ENT(l, i) ((i) >= (l)->count ? 0 : &(l)->ents[(l)->order[i]])

#define PT(x, y) ((unsigned short)(x) | (unsigned short)(y) << 16)

typedef struct DsSprite {              /* ds_draw_sprite */
    unsigned short x, y;
    unsigned short w, h;
    unsigned short u, v;
} DsSprite;

typedef struct DsStat {                /* ds_draw_stat, passed by value */
    unsigned short kind;               /* 0 "ATK/", 1 "DEF/", 2/3 icons */
    unsigned short value;              /* 0xFFFF = "?" */
} DsStat;

typedef struct DsCounter {             /* ds_draw_counters, 12 bytes x 3 */
    unsigned char icon;                /* ds_count_icons row, 0 = none */
    unsigned char pad01[3];
    unsigned int color;                /* +0x04 */
    unsigned char value;               /* +0x08 */
    unsigned char pad09[3];
} DsCounter;

typedef struct DsIconText {            /* ds_draw_icon_text */
    void *gim;
    unsigned char u, v, w, h;          /* +0x04 */
    unsigned short *text;              /* +0x08 */
    unsigned int color;                /* +0x0C 0 = black */
} DsIconText;


typedef struct DsAction {              /* ds_action 0x0881B984, heap allocated */
    unsigned char no_ent : 1;          /* +0x00 opened on an empty slot */
    unsigned char pad00 : 7;
    unsigned char pad01;
    unsigned short step;               /* +0x02 */
    DsEnt *ent;                        /* +0x04 */
    unsigned short mode;               /* +0x08 ds_modes row being picked */
    unsigned short side;               /* +0x0A pane of the other list */
    unsigned short tab;                /* +0x0C its tab */
    unsigned short mark;               /* +0x0E "new" mark picked */
    struct MenuWidget *menu;                  /* +0x10 */
    struct MenuWidget *sub;                   /* +0x14 */
} DsAction;

extern DsAction *ds_action;

extern DsView dsv;
extern DsState ds;
extern void *ds_cip_mem;
extern DsLoad ds_load;

/* this module */
void res_free(void);
void *res_get(unsigned short id);
unsigned short ds_sort_key(unsigned short id, unsigned short *tab);
int ds_id_find(unsigned short id, unsigned short sub, DsIdList *l);
int ds_kind_icon(unsigned short kind);
void *ds_draw_digits(void *list, DsPt pos, int pad, unsigned int value, unsigned short fmt, unsigned int color, void *gim);
void *ds_draw_num_text(void *list, int x, int y, int value, int step, unsigned short n, int pad);
void *ds_draw_frame(void *list, int x, int y, int w, int h, void *gim, int style);
void *ds_draw_sprite(void *list, DsSprite *s, void *gim);
void *ds_draw_icon_a(void *list, DsPt pos, unsigned short id, void *gim);
void *ds_draw_icon_b(void *list, DsPt pos, unsigned short id, void *gim);
void *ds_draw_icon_c(void *list, DsPt pos, unsigned short id, void *gim);
void *ds_draw_icon_d(void *list, DsPt pos, unsigned short id, void *gim);
void *ds_draw_icon_e(void *list, DsPt pos, unsigned short id, void *gim);
void *ds_draw_level(void *list, DsPt pos, unsigned short level, unsigned int color, void *gim);
void *ds_draw_stat(void *list, unsigned int st, DsPt pos, unsigned short size, unsigned int color, void *gim);
void *ds_draw_card_pict(void *list, int x, int y, int shadow, unsigned int color, DsEnt *e, void *gim, void *gim_alt, DsIdList *alt);
void *ds_draw_card_marks(void *list, int x, int y, DsEnt *e, int show_count, int blink, int show_fixed, unsigned short rank, void *gim_digits, void *gim);
void *ds_draw_row_frame(void *list, DsPt pos, unsigned short w, unsigned short kind, void *gim);
void *ds_draw_flag_icon(void *list, DsPt pos, unsigned short kind, void *gim);
void *ds_draw_counters(void *list, DsPt pos, DsCounter *c, void *gim_strip, void *gim);
void *ds_draw_icon_text(void *list, DsIconText *t, int x, int y);
void *ds_draw_arrow(void *list, int x, int y, unsigned char dir, unsigned int period, int blink, int base, unsigned int t, void *gim);
void *ds_draw_scrollbar(void *list, int x, int y, int h, int pos, int first, int total, int frame, void *gim);
void ds_scroll_analog(DsScroll *s);
void *ds_draw_box(void *list, int x, int y, int w, int h, void *gim);
void *ds_draw_help(void *list, int x, int y, int alt, unsigned int flags, void *gim, void *strtbl);

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
void snd_cmd_fefa(void);
void bgm_play(int id);
void se_play(int id);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_is_done(void);
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
void *dl_clear_color(void *list, unsigned int color);
void *dl_clear_depth(void *list, unsigned int z);
void *dl_clear_stencil(void *list, unsigned int s);
void *dl_clear(void *list, unsigned int flags);
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
int inv_get_bit0(unsigned short id);
int inv_get_flag7(unsigned short id);
void inv_set_flag7(unsigned short id, int on);
int inv_get_count(unsigned short id);

#endif
