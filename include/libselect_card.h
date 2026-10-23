/* libselect_card: card picker sub-screen (two card panes: the owned cards and the deck/trade/rental
   list being built), imported by libtitle, libstory, liblabo and libconv_machine. Shared state and the
   BOOT (libehsys) calls it uses; prototypes copy the BOOT definitions where the parameter types decide
   the caller's masking. */
#ifndef LIBSELECT_CARD_H
#define LIBSELECT_CARD_H

#include "ehsys.h"
#include "font.h"
#include "psp.h"
#include "libc.h"
#include "cip.h"
#include "card_db.h"
#include "game.h"

/* sc.kind: what the screen is picking for */
#define SC_KIND_RENTAL 0   /* labo/rental.ehp, the rental list comes from sysstate */
#define SC_KIND_CONV   1   /* labo/conv_machine.ehp */
#define SC_KIND_TRADE  2   /* trade.ehp */

typedef struct ScEnt {                 /* one card line, 6 bytes */
    unsigned short id;                 /* card id */
    unsigned short key;                /* sort key (row of the sort table, sc_sort_key) */
    unsigned char count : 7;           /* +0x04 copies */
    unsigned char fixed : 1;           /*       entry from the caller's list, cannot be removed */
    unsigned char marked : 1;          /* +0x05 */
    unsigned char b5_1 : 1;
    unsigned char rank : 3;            /*       rental rank (4 = none) */
    unsigned char b5_5 : 3;
} ScEnt;

typedef struct ScList {                /* 0x20: the cards of one pane */
    unsigned short tab;                /* +0x00 list shown (0 the cards owned, 1 the list being built) */
    unsigned short pad02;
    ScEnt *ents;                       /* +0x04 */
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
} ScList;

typedef struct ScLayout {              /* 0x1E, copied from sc_layouts */
    short x, y, w, h;                  /* +0x00 frame of the pane */
    short row_x, row_y;                /* +0x08 first row (list mode) */
    unsigned short rows;               /* +0x0C visible rows (list mode) */
    short row_w, row_h;                /* +0x0E */
    short grid_x, grid_y;              /* +0x12 first cell (grid mode) */
    unsigned short cols;               /* +0x16 columns (grid mode) */
    short cell_w;                      /* +0x18 */
    unsigned short grid_rows;          /* +0x1A visible rows (grid mode) */
    short cell_h;                      /* +0x1C */
} ScLayout;

typedef struct ScPane {                /* 0x2C */
    ScList *list;                      /* +0x00 */
    unsigned char list_mode : 1;       /* +0x04 one card per row (else grid) */
    unsigned char pad04 : 7;
    unsigned char pad05;
    unsigned short top;                /* +0x06 first visible entry */
    unsigned short cx;                 /* +0x08 cursor column */
    unsigned short cy;                 /* +0x0A cursor row */
    ScLayout lay;                      /* +0x0C */
    unsigned short pad2a;
} ScPane;

typedef struct ScScroll {              /* analog-scrolled value */
    int id;                            /* +0x00 card shown (-1 none) */
    int pad04;
    unsigned int pos;                  /* +0x08 */
    unsigned int max;                  /* +0x0C */
} ScScroll;

typedef struct ScCursor {              /* 0x28 */
    unsigned short id;                 /* +0x00 card under the cursor */
    unsigned short key;                /* +0x02 */
    int timer;                         /* +0x04 frames on the same card */
    ScEnt *ent;                        /* +0x08 */
    ScScroll scroll;                   /* +0x0C analog-scrolled info text */
    unsigned char pad1c[0x0C];
} ScCursor;

typedef struct ScView {                /* 0x0881B540, 0x94 bytes cleared by libselect_card_08804008 */
    union {
        struct {
            unsigned char mode : 4;    /* +0x00 pane layout (sc_modes) */
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
    ScCursor cursor;                   /* +0x10 cursor box animation (sc_draw_cursor) */
    unsigned char info : 1;            /* +0x38 card info (cardinfo_*) shown */
    unsigned char pad38 : 7;
    unsigned char pad39[3];
    ScPane pane[2];                    /* +0x3C */
} ScView;

typedef struct ScConfig {              /* libselect_card_08804008 cfg */
    void *heap;
    void *cip;
    void *pack_a;                      /* pack 1 already loaded by the caller */
    void *pack_b;                      /* pack 2..4 already loaded by the caller */
} ScConfig;

typedef struct ScMove {                /* a card moved between the panes, 10 bytes */
    unsigned char b[10];
} ScMove;

typedef struct ScRect {
    short x, y, w, h;
} ScRect;

typedef struct ScState {               /* 0x0881B5E0, 0x398 bytes cleared by libselect_card_08804008 */
    int kind;                          /* +0x000 SC_KIND_* */
    int state;                         /* +0x004 libselect_card_088041C4 state */
    unsigned short info_step;          /* +0x008 card info screen (sc_info_anim) */
    unsigned short unload_step;        /* +0x00A */
    ScList list[2];                    /* +0x00C */
    unsigned char blob[0x200];         /* +0x04C sysstate_blob_set */
    unsigned short nhist;              /* +0x24C undo history of moves */
    ScMove hist[8];                    /* +0x24E */
    ScRect focus;                      /* +0x29E frame around the focused popup */
    unsigned char flags2a6;            /* +0x2A6 */
    unsigned char pad2a7;
    int *out_count;                    /* +0x2A8 */
    unsigned short *out_ids;           /* +0x2AC */
    int n_in;                          /* +0x2B0 */
    unsigned short ids_in[100];        /* +0x2B4 */
    void *pack_a;                      /* +0x37C */
    void *pack_b;                      /* +0x380 */
    void *heap;                        /* +0x384 */
    void *cip;                         /* +0x388 */
    unsigned char own_cip : 1;         /* +0x38C cip opened here */
    unsigned char pad38c : 7;
    unsigned char pad38d[3];
    int fpl;                           /* +0x390 own 6 MB block when the caller passes no heap */
    void *fpl_mem;                     /* +0x394 */
} ScState;

typedef struct ScLoad {                /* sc_load 0x0881B980 */
    unsigned char ready : 1;           /* no read in flight */
    unsigned char step : 7;
} ScLoad;

typedef struct ScPack {                /* sc_packs: the EHP packs, 12 bytes */
    unsigned short id;
    void *data;                        /* +0x04 */
    const char *name;                  /* +0x08 printf format, %c = language letter */
} ScPack;

typedef struct ScRes {                 /* resource tables, 12 bytes */
    unsigned short id;
    unsigned short pack;               /* +0x02 ScPack id */
    void *data;                        /* +0x04 */
    const char *name;                  /* +0x08 */
} ScRes;

typedef struct ScIdList {              /* count, then sorted (id | sub << 16) words */
    int n;
    unsigned int e[1];
} ScIdList;

typedef int ScPt;                      /* screen position packed as x | y << 16 */
/* entry at row i of the visible order, 0 past the end */
#define SC_ENT(l, i) ((i) >= (l)->count ? 0 : &(l)->ents[(l)->order[i]])

#define PT(x, y) ((unsigned short)(x) | (unsigned short)(y) << 16)

typedef struct ScSprite {              /* sc_draw_sprite */
    unsigned short x, y;
    unsigned short w, h;
    unsigned short u, v;
} ScSprite;

typedef struct ScStat {                /* sc_draw_stat, passed by value */
    unsigned short kind;               /* 0 "ATK/", 1 "DEF/", 2/3 icons */
    unsigned short value;              /* 0xFFFF = "?" */
} ScStat;

typedef struct ScCounter {             /* sc_draw_counters, 12 bytes x 3 */
    unsigned char icon;                /* sc_count_icons row, 0 = none */
    unsigned char pad01[3];
    unsigned int color;                /* +0x04 */
    unsigned char value;               /* +0x08 */
    unsigned char pad09[3];
} ScCounter;

typedef struct ScIconText {            /* sc_draw_icon_text */
    void *gim;
    unsigned char u, v, w, h;          /* +0x04 */
    unsigned short *text;              /* +0x08 */
    unsigned int color;                /* +0x0C 0 = black */
} ScIconText;


extern ScView scv;
extern ScState sc;
extern void *sc_cip_mem;
extern ScLoad sc_load;

/* this module */
void res_free(void);
void *res_get(unsigned short id);
unsigned short sc_sort_key(unsigned short id, unsigned short *tab);
int sc_id_find(unsigned short id, unsigned short sub, ScIdList *l);
int sc_kind_icon(unsigned short kind);
void *sc_draw_digits(void *list, ScPt pos, int pad, unsigned int value, unsigned short fmt, unsigned int color, void *gim);
void *sc_draw_num_text(void *list, int x, int y, int value, int step, unsigned short n, int pad);
void *sc_draw_frame(void *list, int x, int y, int w, int h, void *gim, int style);
void *sc_draw_sprite(void *list, ScSprite *s, void *gim);
void *sc_draw_icon_a(void *list, ScPt pos, unsigned short id, void *gim);
void *sc_draw_icon_b(void *list, ScPt pos, unsigned short id, void *gim);
void *sc_draw_icon_c(void *list, ScPt pos, unsigned short id, void *gim);
void *sc_draw_icon_d(void *list, ScPt pos, unsigned short id, void *gim);
void *sc_draw_icon_e(void *list, ScPt pos, unsigned short id, void *gim);
void *sc_draw_level(void *list, ScPt pos, unsigned short level, unsigned int color, void *gim);
void *sc_draw_stat(void *list, unsigned int st, ScPt pos, unsigned short size, unsigned int color, void *gim);
void *sc_draw_card_pict(void *list, int x, int y, int shadow, unsigned int color, ScEnt *e, void *gim, void *gim_alt, ScIdList *alt);
void *sc_draw_card_marks(void *list, int x, int y, ScEnt *e, int show_count, int blink, int show_fixed, unsigned short rank, void *gim_digits, void *gim);
void *sc_draw_row_frame(void *list, ScPt pos, unsigned short w, unsigned short kind, void *gim);
void *sc_draw_flag_icon(void *list, ScPt pos, unsigned short kind, void *gim);
void *sc_draw_counters(void *list, ScPt pos, ScCounter *c, void *gim_strip, void *gim);
void *sc_draw_icon_text(void *list, ScIconText *t, int x, int y);
void *sc_draw_arrow(void *list, int x, int y, unsigned char dir, unsigned int period, int blink, int base, unsigned int t, void *gim);
void *sc_draw_scrollbar(void *list, int x, int y, int h, int pos, int first, int total, int frame, void *gim);
void sc_scroll_analog(ScScroll *s);
void *sc_draw_box(void *list, int x, int y, int w, int h, void *gim);
void *sc_draw_help(void *list, int x, int y, int alt, unsigned int flags, void *gim, void *strtbl);

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
