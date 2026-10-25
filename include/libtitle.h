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
    int u000;                          /* +0x000 */
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
    short u01c;                        /* +0x01C */
    short prev_sub;                    /* +0x01E */
    int u020;                          /* +0x020 */
    void *heap;                        /* +0x024 */
    SceneTrans trans;                  /* +0x028 */
    unsigned char pad030[0x03C - 0x030];
    int u03c;                          /* +0x03C */
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
    unsigned char pad974[0x3454 - 0x974];
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
void bgm_cmd(int x);
int sysstate_checksum(void);
int cfg_get_language(void);

/* this module, not named yet */
void func_0880C0F0(void *heap, int a, int b, int c, int d, int e);
void func_08808B7C(MenuWidget *w);
void func_088108A0(void *p, void *heap);
void func_0880C2F0(void);
void func_088225A0(int a);
void func_0880A934(void);
void func_088088E8(int a);
void func_0880A41C(int id, int a, int b, int c);
void func_08808AA4(int a, short b, short c);
void func_088062B4(void);
void func_088063D0(MenuWidget *w);
void *func_0880A56C(void *list, int id);
void *func_0880A5FC(void *list);
void *func_08806538(void *list);
int func_0880A534(int id);
void func_0880A4A8(int id, int a, int b, int c);
int func_0880AA3C(void);
int func_0880AA88(void);
void *func_0880ACB0(void *list, int x, int y, int w, int a, int text, int b);

/* this module */
int title_update(void);
int title_draw(void);
void title_init(void);
void title_goto(short state, short sub);
int title_set_state(int state, unsigned int sub);
void title_menu_enter(short state, int menu, int no_check);
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
