/* libtutorial: scripted tutorial duels (imported by libduel_mgr). Runs a lesson script on top of
   libduel_eng/libduel_draw: message windows with bust-up faces, screen highlights, arrows and the
   checks that wait for the player's action. Module state and the BOOT functions no shared header
   declares. */
#ifndef LIBTUTORIAL_H
#define LIBTUTORIAL_H

#include "font.h"
#include "ehsys.h"
#include "menu.h"

typedef struct Tut {             /* tut 0x0880FFC0 (.bss), cleared by tut_init (0x144 bytes) */
    int lesson;                  /* +0x00 lesson number (file prefix %02d) */
    int state;                   /* +0x04 0 run script, 1 window, 2 wait action, 3/4 duel busy,
                                    5 wait event, 6 fade out, 7 done */
    int prev_state;              /* +0x08 */
    int wait;                    /* +0x0C frames to idle before the next step */
    int frame;                   /* +0x10 frame counter (arrow bobbing) */
    int window;                  /* +0x14 window slot waited on in state 1 */
    int lock;                    /* +0x18 duel input locked (read by libduel_mgr) */
    unsigned char show_windows : 1; /* +0x1C draw the message windows this frame */
    unsigned char pad1c : 7;
    unsigned char pad1d[3];
    MenuWidget quit;             /* +0x20 "quit the tutorial?" box */
    unsigned char quit_open : 1; /* +0x140 */
    unsigned char saved_lock : 1;
    unsigned char saved_cfg134 : 3; /* game options overridden during the lesson */
    unsigned char saved_cfg0 : 1;
    unsigned char saved_cfg2 : 1;
    unsigned char saved_cfg3 : 1;
} Tut;

extern Tut tut;
extern void *tut_pack;           /* 0x08810214 tutorial_<l>.ehp */

typedef struct TutFace TutFace; /* a loaded bust-up face (tut_face.c) */

typedef struct TutFaceRef {
    TutFace *face;
    int chara;
} TutFaceRef;

typedef struct TutBox {          /* tutbox 0x08810224 */
    MenuWidget box;              /* +0x000 window of slots 6..8 */
    TutFaceRef faces[8];         /* +0x120 faces preloaded by the script */
    int cur_face;                /* +0x160 face of the talk window being opened */
} TutBox;

extern TutBox tutbox;

/* BOOT (libehsys) */
void *memset(void *, int, unsigned int);
int sprintf(char *, const char *, ...);
unsigned short region_pick_value(int flag);
void req_flag_set(int i);
int req_flag_get(int i);
unsigned int cfg_get_bits_1_3_4(void);
unsigned int cfg_get_flag(int which);
void cfg_set_flag(int which, int on);
void cfg_set_bits_1_3_4(int v);
int cfg_get_language(void);
void bgm_play(int id);
void bgm_cmd(int arg);
void se_play(int id);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_is_done(void);
int fade_apply(void **list);
unsigned int *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(unsigned int *list);
void sys_frame_wait(void);
void *heap_alloc(void *h, int n);
int heap_set_level(void *h, void *p);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehp_mount(void (*cb)(void *), void *pack);
void *ehp_find(void *pack, const char *name, int *size);
unsigned short *table_get_entry(void *t, unsigned int i);

/* libduel_draw / libduel_eng */
int dd_api_menu_state58(void);
void dd_api_view_end(void);

/* tut_main.c */
void tut_update(void);
void tut_set_state(int s);
int tut_get_state(void);
void tut_wait_window(int slot);
void tut_set_lock(int on);
int tut_get_lock(void);
int tut_is_busy(void);
void tut_command(int player, int cmd, int a, int b);
void tut_cfg_save(void);
void tut_cfg_restore(void);

/* tut_cmd.c */
void tut_cmd_dispatch(int player, int cmd, unsigned short a, unsigned short b);
void tut_arrows_clear(void);

/* tut_res.c */
void tut_heap_init_thunk(void);
void tut_heap_term_thunk(void);
void tut_res_load(int lesson);
void tut_res_free(void);
void *tut_find(const char *name, int *size);
void tut_release(void *p);
unsigned short *tut_msg_get(int id);
void *tut_duel_commands(int lesson);
char *tut_field_set(int lesson);
char *tut_field_reset_file(int n);
void *tut_heap_get(void);

/* tut_window.c */
void tut_windows_init(void);
void tut_windows_term(void);
void tut_windows_draw(void);
int tut_window_busy(int slot);
void tut_pause_open(void);

/* tut_sprite.c */
void tut_sprites_init(void);
void tut_sprites_term_thunk(void);
void tut_sprites_draw(void);

/* tut_script.c */
void tut_script_init(int lesson, void (*start)(void), void (*host)(void));
void tut_script_term_thunk(void);
void tut_script_update(void);
int tut_script_busy(void);
int tut_script_done(void);
int tut_action_done(void);
int tut_event_done(void);
void tut_duel_resume(void);
void tut_duel_draw_term(void);

#endif
