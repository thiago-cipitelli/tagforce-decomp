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

typedef void *(*TMenuDrawFn)(void *list, unsigned short index, int x, int y, int w, int h);

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

typedef struct TSprite {               /* title_sprites, 0x98 */
    union {
        struct {
            unsigned char kind;
            unsigned char gim;         /* +0x01 index into title_gim_names */
        };
        int hdr;
    };
    char name[0x80];                   /* +0x04 */
    short x, y, w, h;                  /* +0x84 */
    short u, v;                        /* +0x8C */
    short tw, th;                      /* +0x90 */
    short a, b;                        /* +0x94 */
} TSprite;

typedef struct TSpr16 {                /* title_sprites2, 0x16 */
    unsigned char kind;
    unsigned char gim;                 /* +0x01 index into title_gim_names */
    short x, y, w, h;                  /* +0x02 */
    short u, v;                        /* +0x0A */
    short tw, th;                      /* +0x0E */
    short a, b;                        /* +0x12 */
} TSpr16;

typedef struct TRes {                  /* title_res: a file inside the title pack, 0x84 */
    int pack;
    char name[0x80];
} TRes;

typedef struct TColorFade {            /* color interpolation written to *dst */
    unsigned int *dst;
    unsigned int from;                 /* +0x04 */
    unsigned int to;                   /* +0x08 */
    unsigned short len;                /* +0x0C frames */
    unsigned short t;                  /* +0x0E */
    unsigned short active;             /* +0x10 */
} TColorFade;

typedef struct TDeckRec {              /* a deck edited in the free duel setup, 0x148 */
    RecipeData recipe;
    unsigned short name[0x20];         /* +0x108 */
} TDeckRec;

typedef struct TDuelOpts {             /* free duel options */
    unsigned char b0 : 5;
    unsigned char tag : 1;             /* bit 5: tag duel */
    unsigned char swap : 1;            /* bit 6: deck swap before the duel */
    unsigned char b7 : 1;
    unsigned char lp : 4;              /* +0x01 life points (title_lp_level) */
    unsigned char time : 4;            /*       answer time */
} TDuelOpts;

typedef struct TDuelCfg {              /* title +0x974 */
    unsigned char pad000[0xEB4];
    TDuelOpts opts;                    /* +0xEB4 */
    unsigned char pad0eb6[0x117C - 0xEB6];
    unsigned short card;               /* +0x117C */
    unsigned char pad117e[0x2170 - 0x117E];
    TDeckRec decks[4];                 /* +0x2170 */
} TDuelCfg;

typedef struct DeckFlags {             /* deck_cur_get_u16_0 */
    unsigned short b0 : 1;             /* a story character was chosen */
    unsigned short kind : 3;
    unsigned short chara : 8;
    unsigned short rest : 4;
} DeckFlags;

typedef struct TGameHdr {             /* view of GameState used by the duel setups */
    unsigned char deck[0x110];
    int mode;                          /* +0x110 duel_setup_mode (random) */
    int side;                          /* +0x114 first player */
    unsigned char kind;                /* +0x118 1 free duel, 2 tutorial, 3 lesson, 5 random duel */
    unsigned char kind2;               /* +0x119 2: deck swap before the duel */
    unsigned char partner_mode;        /* +0x11A 1 single, 2 tag duel */
    unsigned char tutorial;            /* +0x11B tutorial id */
    unsigned char bgm_set;             /* +0x11C */
    unsigned char rematch;             /* +0x11D */
    unsigned char lp_level;            /* +0x11E starting life points */
    unsigned char time_level;          /* +0x11F answer time limit */
    PlayerSlot slot[4];                /* +0x120 */
} TGameHdr;

typedef struct TSlotInfo {             /* a player of the free duel setup (func_0881F6E0) */
    short type;
    short type2;
    short deck_kind;                   /* 0 saved deck, 1 recipe, 2 the deck of the setup, 3 none */
    short deck;                        /* +0x06 */
} TSlotInfo;

typedef void *(*TPanelDrawFn)(void *list, int i, int x, int y, int w);

typedef struct TTab {                  /* a tab of TPanel, 0x24 */
    unsigned char state;
    unsigned char pad01;
    short x, y, w, h;                  /* +0x02 */
    unsigned char pos : 2;             /* +0x0A 0 first, 1 middle, 2 last */
    unsigned char cap : 2;
    unsigned char b0a : 4;
    unsigned char pad0b;
    unsigned int color;                /* +0x0C */
    unsigned int color_off;            /* +0x10 */
    unsigned int text_color;           /* +0x14 */
    unsigned int text_color_off;       /* +0x18 */
    void *u1c;                         /* +0x1C */
    TPanelDrawFn draw;                 /* +0x20 */
} TTab;

typedef struct TBox {                  /* frame of TPanel, 0x2C */
    unsigned char state;
    unsigned char pad01;
    short x, y, w, h;                  /* +0x02 */
    short pad0a;
    unsigned int color;                /* +0x0C */
    unsigned int color2;               /* +0x10 */
    unsigned char b14;
    unsigned char b15;
    unsigned char f0 : 1;              /* +0x16 */
    unsigned char f1 : 1;
    unsigned char arrows : 1;
    unsigned char f3 : 1;
    unsigned char f4 : 1;
    unsigned char f5 : 1;
    unsigned char f6 : 2;
    unsigned char b17;
    unsigned char b18;
    unsigned char rows;                /* +0x19 */
    unsigned char rows2;               /* +0x1A */
    unsigned char b1b;
    short ix, iy, iw, ih;              /* +0x1C */
    int u24;                           /* +0x24 */
    int u28;                           /* +0x28 */
} TBox;

typedef struct TPanel {                /* title +0x508 */
    TTab tab[4];                       /* +0x000 */
    TBox box;                          /* +0x090 */
    MenuWidget mw;                     /* +0x0BC the list */
    unsigned char pad1dc[0x2FC - 0x1DC];
    unsigned char f0 : 2;              /* +0x2FC */
    unsigned char f2 : 1;
    unsigned char f3 : 5;
    unsigned char pad2fd;
    short x, y, w, h;                  /* +0x2FE */
    unsigned char cur;                 /* +0x306 current tab */
    unsigned char n;                   /* +0x307 tabs */
} TPanel;

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
    unsigned short fade_state;         /* +0x020 0 none, 1 fading, 2 fade done */
    short pad022;
    void *heap;                        /* +0x024 */
    SceneTrans trans;                  /* +0x028 */
    PadState *pad;                     /* +0x030 */
    unsigned char pad034[0x03C - 0x034];
    int skip;                          /* +0x03C START pressed during the logos */
    unsigned int color0;               /* +0x040 title screen colors (faded by cfade) */
    unsigned int color1;               /* +0x044 */
    unsigned char pad048[0x050 - 0x048];
    TColorFade cfade;                  /* +0x050 */
    unsigned char pad064[0x0A0 - 0x064];
    void *ehf[17];                     /* +0x0A0 EHF layers, by title_ehf_names index */
    Menu menu;                         /* +0x0E4 */
    unsigned char pad130[0x1A8 - 0x130];
    MenuWidget win[3];                 /* +0x1A8 */
    TPanel panel;                      /* +0x508 tabbed list window */
    unsigned char pad810[0x870 - 0x810];
    int arg;                           /* +0x870 value passed between the states */
    unsigned char pad874[0x87C - 0x874];
    TMenu tm[2];                       /* +0x87C main menu, sub menu */
    TDuelCfg duel;                     /* +0x974 */
    int lang;                          /* +0x3004 language of the loaded title pack */
    unsigned char cfg_lang;            /* +0x3008 */
    unsigned char sub_shown;           /* +0x3009 the sub-screen module draws */
    unsigned char pad300a[0x300C - 0x300A];
    int voice;                         /* +0x300C title call voice still to fade out */
    void *packs[9];                    /* +0x3010 title_menu_X.ehp, by title_pack_names index */
    unsigned char pad3034[0x3038 - 0x3034];
    unsigned char b3038_0 : 1;         /* +0x3038 */
    unsigned char b3038_1 : 1;
    unsigned char busy : 1;            /*         a fade/transition is running */
    unsigned char no_movie : 1;        /*         the opening movie was shown or skipped */
    unsigned char mod_loaded : 1;      /*         a sub-screen module is started */
    unsigned char b3038_5 : 1;
    unsigned char b3038_6 : 2;
    unsigned char pad3039[3];
    int sub_mod;                       /* +0x303C id of the sub-screen module */
    unsigned char skippable;           /* +0x3040 the logos can be skipped */
    unsigned char pad3041[0x304C - 0x3041];
    void *movie_mem;                   /* +0x304C */
    unsigned short msg_buf[0x202];     /* +0x3050 */
} Title;

typedef struct TMsgLine {              /* a line of TMsgWin, 0x18 */
    int x, y;                          /* +0x00 */
    int u08;
    int h;                             /* +0x0C */
    int start;                         /* +0x10 first character */
    int count;                         /* +0x14 characters on the line */
} TMsgLine;

typedef struct TMsgWin {               /* tmsg 0x09433B44: message window, 0x180 */
    int done;                          /* +0x000 */
    int pad004;
    Menu menu;                         /* +0x008 */
    MenuItemDef def;                   /* +0x054 */
    void *heap;                        /* +0x0CC */
    int x;                             /* +0x0D0 */
    int w0;                            /* +0x0D4 */
    int size;                          /* +0x0D8 font size */
    int nlines;                        /* +0x0DC */
    int w;                             /* +0x0E0 */
    unsigned short *text;              /* +0x0E4 */
    int speed;                         /* +0x0E8 text speed option */
    int u0ec;                          /* +0x0EC */
    int style;                         /* +0x0F0 */
    TMsgLine line[3];                  /* +0x0F4 */
    int u13c;                          /* +0x13C */
    int pos;                           /* +0x140 */
    int u144;                          /* +0x144 */
    int u148, u14c, u150, u154;        /* +0x148 */
    int wide;                          /* +0x158 */
    int u15c;                          /* +0x15C */
    unsigned char pad160[0x168 - 0x160];
    float fx;                          /* +0x168 */
    float fy;                          /* +0x16C */
    float fx2;                         /* +0x170 */
    float fy2;                         /* +0x174 */
    float fx3;                         /* +0x178 */
    float fy3;                         /* +0x17C */
} TMsgWin;

extern Title title;
extern TMsgWin tmsg;

typedef struct TCfgVals {              /* the options being edited, 4 bytes */
    unsigned char mode1 : 1;           /* cfg_is_mode1 */
    unsigned char speed : 2;           /* message speed (cfg byte 0 bits 4-5) */
    unsigned char b134 : 2;            /* cfg_get_bits_1_3_4 */
    unsigned char flag0 : 1;
    unsigned char flag3 : 1;
    unsigned char flag2 : 1;
    unsigned char vol[3];              /* +0x01 volumes of channels 2, 0, 1 */
} TCfgVals;

typedef struct TCfg {                  /* options screen, heap allocated (tcfg), 0x30 */
    int mode;                          /* +0x00 */
    int step;                          /* +0x04 */
    TCfgVals cur;                      /* +0x08 */
    TCfgVals orig;                     /* +0x0C */
    int u10;                           /* +0x10 */
    void *heap;                        /* +0x14 */
    void *pack;                        /* +0x18 */
    unsigned char pad1c[0x28 - 0x1C];
    unsigned int color;                /* +0x28 */
    unsigned int color2;               /* +0x2C */
} TCfg;

extern TCfg *tcfg;
/* title.ehf[i]; the original adds the index to &title as an integer (the field offset is not folded) */
#define TITLE_EHF(i) (((Title *)((char *)&title + (i) * 4))->ehf[0])
extern char title_pack_names[9][64];
extern char title_ehf_names[17][64];
extern char title_gim_names[22][64];
extern TRes title_res[18];
extern TSprite title_sprites[39];
extern TSpr16 title_sprites2[];
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
unsigned int cfg_get_byte0_bits4_5(void);
int region_get(void);
int cfg_is_mode1(void);
unsigned int cfg_get_bits_1_3_4(void);
unsigned int cfg_get_flag(int which);
void cfg_set_byte0_bit3(int on);
void cfg_set_byte0_bits4_5(int v);
void cfg_set_bits_1_3_4(int v);
void cfg_set_flag(int which, int on);
int cfg_get_byte3(int i);
void snd_set_volume(int channel, int vol);
int font_draw_wtext_ex(int x, int y, int w, int h, unsigned short *text, int max_glyphs);
int wcslen(const unsigned short *s);
void scene_push(int from, int scene, int arg);
void gamestate_load(const void *src);
void gamestate_fill_partner_list(RecipeData *deck, int with_side);
unsigned int rand_next(void);
unsigned short *table_get_entry(void *t, unsigned int i);
unsigned short *card_get_name(int id);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
void cfg_set_byte0_bits0_2(int v);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehp_mount(void (*cb)(void *), void *pack);
void fade_start_in(int type, unsigned int color, int frames);
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
int ehf_set_anim(void *e, int target, int limit);
int ehf_get_frame_count(void *e);
unsigned int *gfx_get_state(void);
void snd_cmd_fefb_0(void);
int snd_count_bgm(void);
int snd_count_se();
void snd_cmd_fefb(void);
void bgm_fade(int x);
DeckSlot *deck_slot_get(int i);
int sysflag_test(unsigned int bit);
int deck_slot_get_level(int i);
void deck_slot_set_level(int i, int n);
void sysflag_apply(unsigned int which, int v);
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
void fade_start(int type, unsigned int color, int frames);
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
unsigned short title_scene_push(int scene, unsigned short arg);
void func_0881F6E0(TSlotInfo *out, int i);
void func_0881F0A0(void);
void *tmenu_gfx_setup(void *list, int mode);
void *title_logo_draw(void *list);
void title_sound_reset(int a);
void title_screen_update(void);
void func_08813134(void);
void func_0880F950(void);
void func_08821FB0(void);
void topt_update(void);
void func_08813FC8(void);
void tcfg_screen_update(void);
void func_0880EC68(void);
void *title_screen_draw(void *list);
void *func_08813460(void *list);
void *func_088102A0(void *list);
void *func_08823090(void *list);
void *topt_draw(void *list);
void *func_0881DC1C(void *list);
void *tcfg_screen_draw(void *list);
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
void title_story_start(int mode, int id);
void func_0880FBB8(int text, int a, int b);
void func_088119D4(int a);
void title_state_init(int state, int sub);
void title_logo_enter(int state, int sub);
void title_movie_enter(int state, int sub);

void func_08813088(void);
void func_0880F900(void);
void func_08821D3C(void);

void func_08813DF4(void);
void func_0880EB20(void);
void title_random_duel_start(void);
void title_lesson_start(void);
void title_free_duel_start(void);
void func_08815ADC(void);
void title_dtut_duel_start(void);
void title_recipe_open(void);
void title_chara_open(void);
void title_album_open(void);
void title_record_open(void);
void tmsg_init(void *heap, int x, int w, int size, int a, int style);
void tpanel_init(TPanel *p, void *heap);
void tmsg_free(void);
void func_088225A0(int a);
void title_module_stop(void);
void title_ehf_load(int id, int a, int b, int c);
void title_menu_save_update(void);
void title_save_check_update(MenuWidget *w);
void *title_ehf_draw(void *list, int id);
void *title_screen_label(void *list);
void *title_save_check_draw(void *list);
int title_ehf_frame(int id);
void title_ehf_anim(int id, int a, int b, int c);
int title_any_deck(void);
int title_flag8(void);
void *title_msg_draw(void *list, int x, int y, int w, int a, int text, int b);

/* this module */
void *title_msg_text(void *list, int x, int y, int w, int file, int id);
void title_msg_center(int x, int w, int file, int id, unsigned short **text, int *tx, int *tw);
void *title_msg_font(void *list, int x, int y, int w, int file, int id, int align, int fw, int fh,
                     unsigned int color);
void *title_text_draw(void *list, int x, int y, int w, int a, unsigned short *text, int align);
void *title_text_left(void *list, int x, int y, int w, unsigned short *text);
void *title_text_wide(void *list, int x, int y, int w, unsigned short *text);
void *title_text_box(void *list, int x, int y, int w, int a, unsigned short *text);
void *title_pack(void);
char *title_res_name(int i);
char *title_sprite_name(int i);
char *title_sprite_gim(TSpr16 *tab, int i);
char *title_gim_name(int i);
char *title_ehf_name(int i);
unsigned short *title_msg(int id);
unsigned short *title_text(int id, int file);
void title_lang_load(int mode);
void *title_sprite(void *list, int kind, int idx, int x, int y);
void *title_sprite_flip(void *list, int kind, int idx, int x, int y, short a, short b);
void *title_sprite_uv(void *list, int kind, int idx, int x, int y, int du, int dv, short a, short b);
void *title_sprite_ex(void *list, int kind, int idx, int x, int y, int w, int h, int du, int dv, short a,
                      short b, int color);
void tcfade_init(TColorFade *f, unsigned int *dst, unsigned int to, unsigned short len);
void tcfade_start(TColorFade *f);
int tcfade_step(TColorFade *f);
void title_fade_out(int kind);
void title_fade(int kind, int frames);
void title_fade_ex(int kind, int type, unsigned int color, int frames);
void title_goto_step(int kind, short state, short sub, short step);
void title_goto_fade(int kind, short state, short sub);
void title_goto_fade_n(int kind, int frames, short state, short sub);
int title_fade_state(void);
void twin_open(MenuWidget *w);
void twin_init(MenuWidget *w);
void *twin_draw(void *list, MenuWidget *w);
void *twin_draw_center(void *list, MenuWidget *w, int center);
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
unsigned int tmenu_text_color(TMenu *m, int i0);

#endif
