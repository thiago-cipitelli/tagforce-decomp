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
    unsigned short side;               /* +0x00 0 or 1 */
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
    short v8, va;                      /* +0x08 */
    unsigned short rows;               /* +0x0C visible rows (list mode) */
    short ve, v10, v12, v14;           /* +0x0E */
    unsigned short cols;               /* +0x16 columns (grid mode) */
    short v18;                         /* +0x18 */
    unsigned short grid_rows;          /* +0x1A visible rows (grid mode) */
    short v1c;                         /* +0x1C */
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

typedef struct ScView {                /* 0x0881B540, 0x94 bytes cleared by libselect_card_08804008 */
    unsigned short mode : 4;           /* +0x00 pane layout (sc_modes) */
    unsigned short side : 2;           /*       pane with the cursor */
    unsigned short busy : 1;           /*       bit 6 */
    unsigned short popup : 8;          /*       popup being shown */
    unsigned short pad00 : 1;
    unsigned char pad02[0x36];
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

typedef struct ScState {               /* 0x0881B5E0, 0x398 bytes cleared by libselect_card_08804008 */
    int kind;                          /* +0x000 SC_KIND_* */
    int state;                         /* +0x004 libselect_card_088041C4 state */
    unsigned short u08;                /* +0x008 */
    unsigned short unload_step;        /* +0x00A */
    ScList list[2];                    /* +0x00C */
    unsigned char blob[0x200];         /* +0x04C sysstate_blob_set */
    unsigned short u24c;               /* +0x24C */
    unsigned char pad24e[0x50];
    short fx, fy, fw, fh;              /* +0x29E focus frame */
    unsigned short flags2a6;           /* +0x2A6 */
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

extern ScView scv;
extern ScState sc;
extern void *sc_cip_mem;
extern ScLoad sc_load;

/* this module */
void res_free(void);
void *res_get(unsigned short id);
unsigned short sc_sort_key(unsigned short id, unsigned short *tab);

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

#endif
