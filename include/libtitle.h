/* libtitle: title scene (scene 13): logos, press start, the main menu and its sub-menus, the options,
   new game / load, the free duel setup and the sub-screen modules it loads (libcharalist,
   libcardalbum, libduelrecord, liblimitlist, libtutoriallist, libdeck_tutorial, librecipe_viewer,
   libselect_card). Module state and the BOOT (libehsys) calls no shared header declares.
   Offsets are the ground truth; field names are partial. */
#ifndef LIBTITLE_H
#define LIBTITLE_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"
#include "game.h"
#include "libc.h"
#include "psp.h"

typedef void *(*TMenuDrawFn)(void *list, int index, int x, int y, int w, int h);

typedef struct TMenu {                 /* list of panels with fading items, 0x7C */
    unsigned char state;               /* +0x00 0 start, 1 opening, 2 open, 3 input, 4 accepted, 5 anim, 6 cancel, 7 done */
    unsigned char next;                /* +0x01 state after the running animation */
    unsigned char anim;                /* +0x02 running animation (0 none, 1 instant, 2 fade, 3 hold) */
    unsigned char done_state;          /* +0x03 state after an accept without animation */
    unsigned char open_anim;           /* +0x04 */
    unsigned char ok_anim;             /* +0x05 */
    unsigned char cancel_anim;         /* +0x06 */
    unsigned char b07;                 /* +0x07 */
    unsigned char n;                   /* +0x08 items */
    unsigned char pad09;
    short fade_len;                    /* +0x0A frames per item */
    short fade_step;                   /* +0x0C delay between items */
    short fade_t;                      /* +0x0E */
    short fade_end;                    /* +0x10 */
    short pad12;
    unsigned int fade_color[10];       /* +0x14 per item, alpha in the top byte */
    unsigned char item[10];            /* +0x3C 0 normal, 1 disabled, 2 marked */
    short x;                           /* +0x46 */
    short y;                           /* +0x48 */
    short w;                           /* +0x4A */
    short h;                           /* +0x4C row height */
    unsigned char style;               /* +0x4E panel sprite */
    unsigned char style_mark;          /* +0x4F */
    unsigned char cursor;              /* +0x50 */
    unsigned char style_sel;           /* +0x51 */
    unsigned char flags;               /* +0x52 bit 0 wrap, bit 1 repeat */
    unsigned char hilite;              /* +0x53 */
    unsigned char hilite_idx;          /* +0x54 */
    unsigned char style_hilite;        /* +0x55 */
    unsigned char b56;                 /* +0x56 */
    unsigned char pad57;
    short u58;                         /* +0x58 */
    short pad5a;
    unsigned int color;                /* +0x5C */
    unsigned int text_color;           /* +0x60 */
    unsigned int sel_color;            /* +0x64 */
    unsigned int off_color;            /* +0x68 */
    unsigned int off_text_color;       /* +0x6C */
    unsigned char blink : 6;           /* +0x70 */
    unsigned char mode : 2;            /*       0 pulse, 1 blink (accepted), 2 off */
    unsigned char pad71[7];
    TMenuDrawFn draw;                  /* +0x78 draws the label of an item */
} TMenu;

typedef struct DeckFlags {             /* deck_cur_get_u16_0 */
    unsigned short b0 : 1;
    unsigned short rest : 15;
} DeckFlags;

typedef struct TGameHdr {             /* view of GameState +0x118.. */
    unsigned char pad000[0x118];
    unsigned char kind;                /* +0x118 1 story, 2 tutorial, 5 free duel ... */
    unsigned char kind2;               /* +0x119 */
    unsigned char partner_mode;        /* +0x11A */
    unsigned char tutorial;            /* +0x11B tutorial id */
} TGameHdr;

typedef struct Title {                 /* title 0x094306F0, 0x3454 bytes cleared by title_init */
    int ticks;                         /* +0x000 frames since the scene started, wraps at 0x10000 */
    int frame;                         /* +0x004 frames in the current step */
    short state;                       /* +0x008 scene state (title_update) */
    short next_state;                  /* +0x00A 0xFFF = none */
    short sub;                         /* +0x00C sub-state of the state */
    short next_sub;                    /* +0x00E 0xFFF none, 0xFFE next */
    short step;                        /* +0x010 */
    short next_step;                   /* +0x012 */
    short step2;                       /* +0x014 */
    short next_step2;                  /* +0x016 */
    short ret_menu;                    /* +0x018 menu to go back to after the save check */
    unsigned char ret_pending : 1;     /* +0x01A */
    unsigned char pad01a : 7;
    unsigned char pad01b;
    short prev_state;                  /* +0x01C */
    short prev_sub;                    /* +0x01E */
    unsigned short u020;               /* +0x020 */
    short pad022;
    void *heap;                        /* +0x024 */
    SceneTrans trans;                  /* +0x028 */
    PadState *pad;                     /* +0x030 */
    unsigned char pad034[0x03C - 0x034];
    int skip;                          /* +0x03C START pressed during the logos */
    int u040;                          /* +0x040 */
    int u044;                          /* +0x044 */
    unsigned char pad048[0x0E4 - 0x048];
    Menu menu;                         /* +0x0E4 */
    unsigned char pad130[0x1A8 - 0x130];
    MenuWidget win[3];                 /* +0x1A8 */
    unsigned char u508[0x870 - 0x508]; /* +0x508 */
    int arg;                           /* +0x870 value passed between the states */
    unsigned char pad874[0x87C - 0x874];
    TMenu tm[2];                       /* +0x87C main menu, sub menu */
    unsigned char pad974[0x3008 - 0x974];
    unsigned char u3008;               /* +0x3008 */
    unsigned char sub_shown;           /* +0x3009 the sub-screen module draws */
    unsigned char pad300a[0x3038 - 0x300A];
    unsigned char b3038_0 : 1;         /* +0x3038 */
    unsigned char b3038_1 : 1;
    unsigned char busy : 1;            /*         a fade/transition is running */
    unsigned char no_movie : 1;        /*         the opening movie was shown or skipped */
    unsigned char b3038_4 : 4;
    unsigned char pad3039[3];
    int sub_mod;                       /* +0x303C id of the sub-screen module */
    unsigned char skippable;           /* +0x3040 the logos can be skipped */
    unsigned char pad3041[0x304C - 0x3041];
    void *movie_mem;                   /* +0x304C */
    unsigned char pad3050[0x3454 - 0x3050];
} Title;

extern Title title;
extern unsigned char title_heap_mem[0xC00000];
extern unsigned char title_scene_data[];

/* BOOT */
void *heap_create(void *mem, unsigned int size);
void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
void gfx_overlay_enable(int on);
void rand_seed_from_rtc(void);
void sysvar_set_a90(int v);
void scene_get_transition_info(SceneTrans *out);
void deck_cur_get_u16_0(short *out);
int sysstate_get_u32_97e8(void);
void bgm_play(int id);
unsigned int *dl_select_layer(int layer, unsigned int *size);
void *dl_color(void *list, unsigned int color);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_tex_func(void *list, int func, int rgba);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_clear_color(void *list, unsigned int rgba);
void *dl_clear_stencil(void *list, int s);
void *dl_clear_depth(void *list, int z);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *gim_set_texture(void *list, void *gim);
void *ehp_find(void *pack, const char *name, int *size);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
void *ehf_draw(void *list, void *e, void *opts);
void *gfx_begin_2d(void *list);
void *gfx_end_2d(void *list);
void *dl_clear(void *list, int flags);
int dl_set_list(void *list);
int fade_apply(void **list);
void msd_update_dialog(void);
unsigned short region_pick_value(int flag);
int duel_modules_load();
int duel_modules_unload(void);
int modmgr_none_loading_alias(void);
void movie_set_memory(unsigned int base, unsigned int size);
void movie_play(const char *path, unsigned int flags, int arg);
int ehfile_make_path_alias(char *out, int dir, const char *name);
void *sceGeEdramGetAddr(void);
void bgm_cmd(int x);
int ehfile_request_module(int dir, const char *name, int (*cb)(void *, int), void *arg);
int sysstate_checksum(void);
int cfg_get_language(void);
int msd_open(int mode, void *heap);
int msd_update(void);
int card_db_load(void);
void card_db_request_load(void);
int sysvar_get_a94(void);
void newgame_init(void);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_finish(void);
void se_play(int id);

/* sub-screen modules (imports) */
typedef struct TRvParams TRvParams;
typedef struct TDtParams TDtParams;
void recipe_viewer_init(int mode, TRvParams *p);
int recipe_viewer_update(void);
void recipe_viewer_draw(void);
void charalist_init(unsigned short mode, void *heap);
int charalist_update(void);
int charalist_result_id(void);
int charalist_result_costume(void);
void charalist_random(void *heap);
void charalist_draw(void);
void charalist_res_free(void);
void cardalbum_init(int mode, void *heap);
int cardalbum_update(void);
void cardalbum_draw(void);
void duelrecord_init(void *heap);
int duelrecord_update(void);
void duelrecord_draw(void);
void limitlist_init(int mode, void *heap);
int limitlist_update(void);
void limitlist_draw(void);
void tlist_init(int mode, void *heap);
int tlist_update(void);
int tlist_result(void);
void tlist_draw(void);
void libdeck_tutorial_08804008(int id, TDtParams *p);
int libdeck_tutorial_0880418C(void);
void libdeck_tutorial_08804554(void);

/* this module, not named yet */
int title_module_start(void *arg, int id);
void *tmenu_gfx_setup(void *list, int mode);
void *title_logo_draw(void *list);
void func_0880A9A8(int a);
void func_08807E44(int a);
void func_08808900(int a, int b, unsigned int color, int frames);
void func_08808AF4(int a, short b, short c, short d);
void func_0880B0B4(void);
void func_08813134(void);
void func_0880F950(void);
void func_08821FB0(void);
void func_0880BC20(void);
void func_08813FC8(void);
void func_0880CA5C(void);
void func_0880EC68(void);
void *func_0880B304(void *list);
void *func_08813460(void *list);
void *func_088102A0(void *list);
void *func_08823090(void *list);
void *func_0880BD50(void *list);
void *func_0881DC1C(void *list);
void *func_0880CB58(void *list);
void *func_0880EF1C(void *list);
void func_088104E8(void);
void func_0881D2F4(void);
void title_recipe_update(void);
void title_chara_update(void);
void title_album_update(void);
void title_record_update(void);
int title_limit_update(void);
int title_tlist_update(int state);
int title_chara_pick_update(int state);
int title_load_update(void);
void title_dtut_update(void);
void title_album_draw(void);
void title_record_draw(void);
void title_limit_draw(void);
void title_tlist_draw(void);
void title_dtut_draw(void);
void *title_load_draw(void *list);
int title_transition(void);
void title_menu_update(void);
void *title_menu_draw(void *list);
void func_0880AEF0(int mode, int id);
void func_0880FBB8(int text, int a, int b);
void *func_08808D48(void *list, MenuWidget *w, int center);
int func_08808B44(void);
void func_088119D4(int a);
void title_state_init(int state, int sub);
void title_logo_enter(int state, int sub);
void title_movie_enter(int state, int sub);
void func_0880AFA0(void);
void func_08813088(void);
void func_0880F900(void);
void func_08821D3C(void);
void func_0880BB98(void);
void func_08813DF4(void);
void func_0880C998(void);
void func_0880EB20(void);
void func_08809DC4(int a, unsigned short b);
void func_08809E24(void);
void func_0880A074(void);
void func_0880A100(void);
void func_08815ADC(void);
void func_0880A390(void);
void title_recipe_open(void);
void title_chara_open(void);
void title_album_open(void);
void title_record_open(void);
void func_0880C0F0(void *heap, int a, int b, int c, int d, int e);
void func_08808B7C(MenuWidget *w);
void func_088108A0(void *p, void *heap);
void func_0880C2F0(void);
void func_088225A0(int a);
void func_0880A934(void);
void func_088088E8(int a);
void func_0880A41C(int id, int a, int b, int c);
void func_08808AA4(int a, short b, short c);
void title_menu_save_update(void);
void title_save_check_update(MenuWidget *w);
void *func_0880A56C(void *list, int id);
void *func_0880A5FC(void *list);
void *title_save_check_draw(void *list);
int func_0880A534(int id);
void func_0880A4A8(int id, int a, int b, int c);
int func_0880AA3C(void);
int func_0880AA88(void);
void *func_0880ACB0(void *list, int x, int y, int w, int a, int text, int b);

/* this module */
int title_update(void);
void title_draw(void);
void title_init(void);
void title_goto(short state, short sub);
int title_set_state(int st, int sub);
void title_menu_enter(int state, int menu, int no_check);
void title_menu_open(int cursor);
void title_sub_open_play(void);
void title_sub_open_extra(void);
void title_sub_open_opt(void);

void tmenu_init(TMenu *m);
int tmenu_update(TMenu *m, int mode);
void tmenu_back(TMenu *m);
void tmenu_set_fade(TMenu *m, short len, short step);
void *tmenu_draw(void *list, TMenu *m);
unsigned int tmenu_text_color(TMenu *m, int i);

#endif
