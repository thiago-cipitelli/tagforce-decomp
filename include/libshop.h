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
    unsigned short pad2;
    unsigned short pad4;
    unsigned short count;       /* +0x06 */
    unsigned short pad8;
} ShopItem;

typedef struct ShopBox {        /* a pack on the shelf: picture, position, name */
    unsigned short gim;
    unsigned char x, y;
    unsigned short name;
} ShopBox;

typedef struct ShopPanel {      /* argument of shop_panel_draw, 0x10 */
    void *gim;
    unsigned char x, y;
    unsigned char b6, b7;
    unsigned short *text;       /* +0x08 */
    int pad0c;
} ShopPanel;

typedef struct ShopView {       /* shop_view 0x08F8EC10 */
    unsigned char flags;        /* bit 0: the alternative field music is playing */
} ShopView;

typedef struct ShopState {      /* shop 0x08E20530, 0x450 bytes cleared at start */
    unsigned char pad000[2];
    unsigned char term : 4;     /* +0x002 release step of shop_term */
    unsigned char pad002 : 4;
    unsigned char pad003[0x69];
    unsigned char pad06c;
    unsigned char pack;         /* +0x06D pack being opened (box textures) */
    unsigned char pad06e[0x152];
    unsigned char mode : 4;     /* +0x1C0 */
    unsigned char arg : 4;
    unsigned char pad1c1[3];
    ShopItem *items;            /* +0x1C4 */
    unsigned short *order;      /* +0x1C8 */
    unsigned short n_items;     /* +0x1CC */
    unsigned short top;         /* +0x1CE */
    unsigned short cursor;      /* +0x1D0 */
    unsigned char pad1d2[10];
    MenuWidget menu;            /* +0x1DC */
    unsigned char pad2fc[0x120];
    int sel_id;                 /* +0x41C card shown in the info window */
    int pad420;
    int sel_timer;              /* +0x424 */
    unsigned char pad428[0x10];
    void *heap;                 /* +0x438 */
    void *cip;                  /* +0x43C */
    unsigned char pad440[0x10];
} ShopState;

typedef struct ShopResFlags {   /* shop_res 0x08E20BC0 */
    unsigned char ready : 1;
    unsigned char step : 7;
    unsigned char pad[3];
} ShopResFlags;

extern ShopState shop;
extern ShopResFlags shop_res;
extern ShopView shop_view;
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

/* this module */
int shop_main_update(void);
int shop_main_draw(void);
void shop_draw_all(void);
void shop_list_free(void);
void shop_menu_free(void);
void shop_list_init(int mode);
void shop_res_init(void);
int shop_load_poll(void);
void shop_res_free(void);
void *shop_pack_get(unsigned short id);
void *shop_res_get(unsigned short id);
void *shop_ehf_get(unsigned short id);
void *shop_str_get(unsigned short id);
Model *shop_model_get(unsigned short id);
int shop_bsearch(unsigned short key, void *base, int n, int size);
void shop_info_init(void);
void shop_menu_init(void *gim);
void shop_tabs_init(void);
void shop_draw_box(void *list, ShopBox *b, int x, int y);
void shop_panel_draw(void *list, ShopPanel *p, int x, int y);
void shop_packs_free(void);
void shop_models_free(void);
void shop_model_set_box(unsigned short id, int pack);
unsigned int *shop_ge_state(unsigned int *list);
void shop_sel_update(void);
int shop_load_deck_pack(void);

#endif
