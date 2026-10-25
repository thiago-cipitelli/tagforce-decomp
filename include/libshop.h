/* libshop: the card shop scene (scene 7, also driven by libstory through its exports): booster
   packs bought with the duel points, the 3D pack opening and the list of the cards that came out. */
#ifndef LIBSHOP_H
#define LIBSHOP_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"

typedef struct Vec3 {
    float x, y, z;
} Vec3;

typedef struct Vec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) Vec4;

typedef struct Mat4 {
    float m[16];
} Mat4;

typedef struct GfxState {       /* gfx_get_state() */
    unsigned int frame;
    int fps;
} GfxState;

typedef struct Light {          /* BOOT light, 0x3C */
    int type;
    unsigned int ambient;
    unsigned int diffuse;
    unsigned int specular;
    Vec3 pos;                   /* +0x10 */
    Vec3 dir;
    float spot_exp;
    float spot_cutoff;
    float att[3];
} Light;

typedef struct ModelTex {       /* texture slot of a TMS: file name and the texture bound to it, 0x28 */
    const char *name;
    void *tex;
    int rest[8];
} ModelTex;

typedef struct Tms {            /* TMS model data (header) */
    int pad00[4];
    ModelTex *texs;             /* +0x10 */
    int pad14[8];
    int ntexs;                  /* +0x34 */
} Tms;

typedef struct Model {          /* BOOT model, 0x68 */
    Tms *tms;
    void **anims;
    int nanims;
    void *bones;
    void *skin;
    void *cur;
    void *prev;
    int index;
    float frame;                /* +0x20 */
    int prev_index;
    float prev_frame;
    void *mats;
    void *texs;
    void *tgma;
    float tgma_time;
    unsigned int flags;         /* +0x3C */
    int rest[10];
} Model;

typedef struct ModelDraw {      /* BOOT model_draw options, 0x34 */
    int layer[2];
    int pad[10];
    void *user;                 /* +0x30 */
} ModelDraw;

typedef struct ShopItem {       /* a line of the bought list, 10 bytes */
    unsigned short id;          /* card */
    unsigned short key;         /* +0x02 sort key */
    unsigned short pack;        /* +0x04 pack kind it came from */
    unsigned short count;       /* +0x06 copies that came out */
    unsigned short partners : 4; /* +0x08 partner decks holding the card */
    unsigned short owned : 8;   /*        copies owned */
    unsigned short pad8 : 4;
} ShopItem;

typedef struct ShopBox {        /* a pack on the shelf: picture, position, name */
    unsigned short gim;
    unsigned char x, y;
    unsigned short name;
} ShopBox;

typedef struct ShopClerk {      /* shop_clerk 0x08F8EC10: the shop girl over the counter, 0x14 */
    unsigned char alt : 1;      /* the other clerk (pc_sei) and her music */
    unsigned char pad00 : 7;
    unsigned char pad01;
    unsigned short state;       /* +0x02 0 idle, 1 sliding in, 2 blinking, 3 talking, 4 reacting, 5 sliding out */
    short timeout;              /* +0x04 */
    unsigned short count;       /* +0x06 */
    short slide;                /* +0x08 x offset while she slides */
    short pad0a;
    void *eyes;                 /* +0x0C pc_*_e.ehf */
    void *mouth;                /* +0x10 pc_*_m.ehf */
} ShopClerk;

typedef struct EhfOpts {        /* BOOT ehf_draw options */
    int time;                   /* +0x00 */
    void *tex_cb;
    void *tex_arg;
    unsigned int flags;         /* +0x0C 0x10000 position */
    int angle;
    float x, y;                 /* +0x14 */
    int pad1c;
    float sx, sy;               /* +0x20 */
    void *eval_cb;
    void *eval_arg;
    unsigned int mask[2];       /* +0x30 */
    int pad38[2];
    int color[8];               /* +0x40 */
} EhfOpts;

/* shop.cur: pack on the shelf under the cursor, column in bits 0-6, row in bit 7 (0 front, 1 back) */
#define SHELF_IDX(c) ((c) & 0x7F)
#define SHELF_SIDE(c) ((c) >> 7 & 1)

typedef struct ShopMsgLine {    /* one line of the message window, 0x18 */
    int x, y;
    int w;                      /* +0x08 width typed so far */
    int h;                      /* +0x0C */
    int start;                  /* +0x10 first character */
    int len;                    /* +0x14 characters that fit */
} ShopMsgLine;

typedef struct ShopMsgWin {     /* shop_msg 0x08F8EC24: the typewriter message window, 0x180 */
    int state;                  /* +0x000 0 hidden, 1 opening, 2 open, 3 closing */
    float t;                    /* +0x004 open/close progress */
    Menu menu;                  /* +0x008 */
    MenuItemDef def;            /* +0x054 */
    void *heap;                 /* +0x0CC */
    int x;                      /* +0x0D0 */
    int w;                      /* +0x0D4 */
    int font_size;              /* +0x0D8 */
    int n_lines;                /* +0x0DC */
    int line_w;                 /* +0x0E0 */
    unsigned short *text;       /* +0x0E4 */
    int speed;                  /* +0x0E8 message speed option (0..2) */
    int input;                  /* +0x0EC the player can advance the text */
    int style;                  /* +0x0F0 */
    ShopMsgLine lines[3];       /* +0x0F4 */
    int line;                   /* +0x13C line being typed */
    int start;                  /* +0x140 first character of the page */
    int page;                   /* +0x144 */
    int done;                   /* +0x148 the whole text is shown */
    int closed;                 /* +0x14C */
    int full;                   /* +0x150 the page is full */
    int shown;                  /* +0x154 characters shown */
    int narrow;                 /* +0x158 narrow font (Japanese) */
    void *icon;                 /* +0x15C "next" arrow */
    int blink;                  /* +0x160 */
    int hold;                   /* +0x164 */
    float pos[2];               /* +0x168 */
    float from[2];              /* +0x170 */
    float to[2];                /* +0x178 */
} ShopMsgWin;

typedef struct ShopCardList {   /* cards of one rarity of a pack */
    unsigned short *ids;
    int n;
} ShopCardList;

typedef struct ShopPackDef {    /* shop_pack_defs 0x0881FE88: one of the 48 pack kinds, 12 bytes */
    unsigned char state : 2;    /* 0 hidden, 1 sold out, 2 on sale */
    unsigned char n_cards : 6;  /*   cards per pack */
    unsigned char complete;     /* +0x01 percent of its cards owned */
    unsigned short pad2;
    unsigned short price;       /* +0x04 */
    unsigned short rank;        /* +0x06 */
    ShopCardList *lists;        /* +0x08 4 rarities: common, rare, super, ultra */
} ShopPackDef;

typedef int UiPt;               /* screen position packed as x | y << 16 */
#define PT(x, y) ((unsigned short)(x) | (unsigned short)(y) << 16)

typedef struct UiEnt {          /* one card line as the shared drawing pieces see it, 6 bytes */
    unsigned short id;
    unsigned short key;         /* sort key */
    unsigned char count : 7;    /* +0x04 copies */
    unsigned char fixed : 1;
    unsigned char marked : 1;   /* +0x05 */
    unsigned char b5_1 : 1;
    unsigned char rank : 3;
    unsigned char b5_5 : 3;
} UiEnt;

typedef struct UiIdList {       /* count, then sorted (id | sub << 16) words */
    int n;
    unsigned int e[1];
} UiIdList;

typedef struct UiSprite {
    unsigned short x, y;
    unsigned short w, h;
    unsigned short u, v;
} UiSprite;

typedef struct UiCounter {      /* 12 bytes x 3 */
    unsigned char icon;
    unsigned char pad01[3];
    unsigned int color;
    unsigned char value;
    unsigned char pad09[3];
} UiCounter;

typedef struct UiIconText {
    void *gim;
    unsigned char u, v, w, h;
    unsigned short *text;
    unsigned int color;
} UiIconText;

typedef struct UiScroll {       /* analog-scrolled value */
    int pad0[2];
    unsigned int pos;           /* +0x08 */
    unsigned int max;           /* +0x0C */
} UiScroll;

typedef struct ShopOpenCard {   /* a card coming out of the pack, 0x60 */
    unsigned short state;       /* +0x00 0 hidden, 2 flying, 3 shown, 4 leaving, 5 gone */
    unsigned short timer;       /* +0x02 */
    unsigned short id;          /* +0x04 */
    unsigned short key;         /* +0x06 */
    unsigned short pack;        /* +0x08 */
    unsigned short n;           /* +0x0A */
    int pad0c;
    Vec4 pos;                   /* +0x10 */
    Mat4 rot;                   /* +0x20 */
} ShopOpenCard;

typedef struct ShopOpenPack {   /* the pack model */
    unsigned short kind;        /* +0x00 */
    unsigned short pad[7];
    Vec4 pos;                   /* +0x10 */
    float rot;                  /* +0x20 */
    float speed;                /* +0x24 */
    int pad28[2];
} ShopOpenPack;

typedef struct ShopOpen {       /* shop_open, 0x520 allocated while packs are opened */
    unsigned short step;        /* +0x000 */
    unsigned short opened;      /* +0x002 packs of this kind opened */
    unsigned short n_cards;     /* +0x004 */
    unsigned short pad006[5];
    ShopOpenPack pack;          /* +0x010 */
    ShopOpenCard cards[10];     /* +0x040 */
    unsigned short speed;       /* +0x400 1, 3 while the button is held */
    unsigned short timer;       /* +0x402 */
    unsigned char pad404[12];
    Vec4 card_from;             /* +0x410 cards: in the pack */
    Vec4 card_up;               /* +0x420 risen out of the pack */
    Vec4 card_ctrl1;            /* +0x430 */
    Vec4 card_mid;              /* +0x440 */
    Vec4 card_ctrl2;            /* +0x450 */
    Vec4 card_show;             /* +0x460 shown in front of the camera */
    Vec4 card_ctrl3;            /* +0x470 */
    Vec4 card_out;              /* +0x480 gone to the side */
    Mat4 card_rot;              /* +0x490 face down */
    Mat4 card_face;             /* +0x4D0 facing the camera */
    unsigned short *cur;        /* +0x510 card whose picture is drawn */
    unsigned char pad514[12];
} ShopOpen;

typedef struct ShopState {      /* shop 0x08E20530, 0x450 bytes cleared at start */
    unsigned short state;       /* +0x000 */
    unsigned char term : 4;     /* +0x002 release step of shop_term */
    unsigned char pad002 : 4;
    unsigned char pad003;
    unsigned short step;        /* +0x004 step inside the state */
    unsigned short pad006;
    int spent;                  /* +0x008 points spent this visit */
    unsigned short bought[48];  /* +0x00C packs bought per kind */
    unsigned char hud : 4;      /* +0x06C window shown over the shelf (0 none) */
    unsigned char info : 1;     /*        card info window */
    unsigned char info_open : 1;
    unsigned char msg_dirty : 1; /*       the pack message has to be rebuilt */
    unsigned char back : 1;     /*        leaving the pack */
    unsigned char pack;         /* +0x06D pack kind under the cursor (0..47) */
    unsigned char pad06e[2];
    float slide[2];             /* +0x070 */
    float rot;                  /* +0x078 shelf rotation, -26 back row, 26 front row */
    unsigned char cur;          /* +0x07C SHELF_IDX / SHELF_SIDE */
    unsigned char pad07d[3];
    float lift[2][10];          /* +0x080 packs lifted on the shelf */
    unsigned char stock[48];    /* +0x0D0 packs left per kind: low nibble front row, high back */
    unsigned char taken : 1;    /* +0x100 a pack is flying to the counter */
    unsigned char pad100 : 7;
    unsigned char pad101[15];
    Vec4 from;                  /* +0x110 flight start */
    Vec4 ctrl;                  /* +0x120 flight control point */
    Vec4 pos;                   /* +0x130 pack position */
    Mat4 rot0;                  /* +0x140 */
    Mat4 rotm;                  /* +0x180 */
    unsigned char mode : 4;     /* +0x1C0 */
    unsigned char arg : 4;
    unsigned char pad1c1[3];
    ShopItem *items;            /* +0x1C4 */
    unsigned short *order;      /* +0x1C8 */
    unsigned short n_items;     /* +0x1CC */
    unsigned short top;         /* +0x1CE */
    unsigned short cursor;      /* +0x1D0 */
    unsigned short sort;        /* +0x1D2 sort key of the list */
    unsigned short order_dir;   /* +0x1D4 */
    unsigned short n_owned;     /* +0x1D6 cards in the collection */
    unsigned short timer;       /* +0x1D8 */
    unsigned short pad1da;
    MenuWidget menu;            /* +0x1DC */
    unsigned char pad2fc[0x120];
    int sel_id;                 /* +0x41C card shown in the info window */
    int pad420;
    int sel_timer;              /* +0x424 */
    unsigned char pad428[0x10];
    void *heap;                 /* +0x438 */
    void *cip;                  /* +0x43C */
    unsigned short code_pos;    /* +0x440 progress of the button code */
    unsigned char code_done : 1; /* +0x442 */
    unsigned char pad442 : 7;
    unsigned char pad443[13];
} ShopState;

typedef char shop_state_size_check[sizeof(ShopState) == 0x450 ? 1 : -1];



typedef struct ShopResFlags {   /* shop_res 0x08E20BC0 */
    unsigned char ready : 1;
    unsigned char step : 7;
    unsigned char pad[3];
} ShopResFlags;

extern ShopState shop;
extern ShopResFlags shop_res;
extern ShopClerk shop_clerk;
extern ShopMsgWin shop_msg;
extern ShopPackDef shop_pack_defs[48];
extern ShopOpen *shop_open;
extern unsigned char shop_heap_mem[0x600000];
extern unsigned char shop_cip_mem[0x16E000];
extern void *shop_box_tex[48][3];

/* BOOT (libehsys) */
void *memset(void *, int, unsigned int);
void *memcpy(void *, const void *, unsigned int);
void qsort(void *, unsigned int, unsigned int, int (*)(const void *, const void *));
int sprintf(char *, const char *, ...);
char *strcpy(char *, const char *);
int strcmp(const char *a, const char *b);
char *strrchr_str(char *s, int c);
int sceKernelChangeCurrentThreadAttr(unsigned int clear, unsigned int set);
void *heap_create(void *mem, unsigned int size);
void *heap_alloc(void *h, int n);
int heap_set_level(void *h, void *p);
void heap_free_block(void *h);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehp_mount(void (*cb)(void *), void *pack);
void *ehp_find(void *pack, const char *name, int *size);
int cfg_get_language(void);
int region_get(void);
void *cip_open_h(void *handle, int bufsize, int prefetch);
void cip_close(void *c);
int cip_is_idle(void *c);
void cardlist_clear(void);
void rand_seed_from_rtc(void);
void scene_pop_to(unsigned short from, unsigned short arg);
void bgm_cmd3(int x);
void bgm_play(int id);
int snd_count_se();
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void gfx_overlay_enable(int on);
void view_term(void);
void model_set_allocator(void *(*alloc)(unsigned int size), void (*freefn)(void *p));
Model *model_load(void *data, void **anims);
void model_free(Model *m);
void model_anim_set(Model *m, void *data);
void model_set_tgma(Model *m, void *a);
int model_anim_play(Model *m, int n, unsigned int flags);
unsigned short *table_get_entry(void *t, unsigned int i);
void inv_adjust_count(unsigned short id, int dir);
unsigned int *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(unsigned int *list);
unsigned int *dl_offset(unsigned int *list, int x, int y);
unsigned int *dl_viewport(unsigned int *list, int cx, int cy, int w, int h);
unsigned int *dl_depth_range(unsigned int *list, int near, int far);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_scissor(unsigned int *list, int x, int y, int w, int h);
unsigned int *dl_alpha_test(unsigned int *list, int func, int ref, int mask);
unsigned int *dl_depth_func(unsigned int *list, int func);
unsigned int *dl_front_face(unsigned int *list, int cw);
unsigned int *dl_shade_model(unsigned int *list, int mode);
unsigned int *dl_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear(unsigned int *list, unsigned int flags);
unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
unsigned int *dl_fog(float near, float far, unsigned int *list, unsigned int color);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
unsigned int *lighting_enable(unsigned int *list);
unsigned int *lighting_disable(unsigned int *list);
unsigned int *light_enable(unsigned int *list, int n);
unsigned int *light_set_ambient(unsigned int *list, unsigned int color);
void mtx_set_mode(int mode);
void mtx_identity(void);
void mtx_perspective(float fovy, float aspect, float near, float far);
void camera_set(Vec4 *pos, Vec4 *target, Vec4 *up);
void camera_load_view(void);

void *gim_set_texture(void *list, void *gim);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
GfxState *gfx_get_state(void);
void se_play(int id);

/* this module */
unsigned int *shop_dl_2d(unsigned int *list);
unsigned int *shop_clerk_draw(unsigned int *list);
unsigned int *shop_pts_draw(unsigned int *list);
void shop_msgwin_draw(unsigned int **list);
void shop_main_update(void);
int shop_main_input(void);
void shop_draw_all(void);
void shop_msgwin_free(void);
void shop_msg_set(unsigned short *text, int title);
int shop_msgwin_update(void);
void shop_msgwin_open(void);
void shop_msgwin_close(void);
int shop_msg_typing(void);
int shop_msgwin_moving(void);
void shop_menu_free(void);
void shop_setup(int mode);
void shop_res_init(void);
int shop_load_poll(void);
void shop_res_free(void);
void *shop_pack_get(unsigned short id);
void *shop_res_get(unsigned short id);
void *shop_ehf_get(unsigned short id);
void *shop_str_get(unsigned short id);
Model *shop_model_get(unsigned short id);
int shop_bsearch(unsigned short key, void *base, int n, int size);
void shop_clerk_load(void);
void shop_menu_init(void *gim);
void shop_tabs_init(void);
unsigned int *shop_draw_box(void *list, ShopBox *b, int x, int y);
void *shop_ui_draw_icon_text(void *list, UiIconText *t, int x, int y);
void shop_packs_free(void);
void shop_models_free(void);
void shop_model_set_box(unsigned short id, int pack);
unsigned int *shop_ge_state(unsigned int *list);
void shop_sel_update(void);
int shop_load_deck_pack(void);

/* card of line `i` of the bought list in display order, 0 past the end */
static inline int shop_item_id_at(int i)
{
    if (i >= shop.n_items)
        return 0;
    return shop.items[shop.order[i]].id;
}

/* line `i` of the bought list in display order, 0 past the end */
static inline ShopItem *shop_item_at(int i)
{
    if (i >= shop.n_items)
        return 0;
    return &shop.items[shop.order[i]];
}

/* the line under the cursor of the bought list */
static inline ShopItem *shop_cur_item(void)
{
    int i = shop.top + shop.cursor;

    if (i >= shop.n_items)
        return 0;
    return &shop.items[shop.order[i]];
}

#endif
