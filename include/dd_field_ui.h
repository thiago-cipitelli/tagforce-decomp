/* libduel_draw, range 0x088143A4-0x08825DDC (field cursor, cut-ins, duel events, card list window,
   scene modes, presentation tasks, card effects, message windows): the BOOT (libehsys) calls and the
   cross-file calls of these files. Private to them; the shared types are in libduel_draw.h. */
#ifndef DD_FIELD_UI_H
#define DD_FIELD_UI_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"
#include "game.h"
#include "libc.h"
#include "psp.h"
#include "allegrex.h"
#include "libduel_draw.h"

unsigned int *gfx_set_draw_target(unsigned int *list, int base, int fbp, int w, int h, int psm);
unsigned int *gfx_set_default_target(unsigned int *list);
unsigned int *card_draw(unsigned int *list, int x, int y, int id);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_tex_flush(unsigned int *list);
unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle);
unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr);
unsigned int *dl_tex_offset(float u, float v, unsigned int *list);
unsigned int *dl_tex_scale(float u, float v, unsigned int *list);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
unsigned int *dl_tex_env_color(unsigned int *list, unsigned int color);
unsigned int *gim_set_texture_frame(unsigned int *list, void *gim, int frame);
int strcmp(const char *a, const char *b);
int charmap_lookup(int id, int a1);
int snd_channel_get_u32(int ch);
int bgm_query(int h, int what);
unsigned int *gfx_get_state(void);
unsigned int *dl_select_layer(int layer, int *size);
int dl_get_layer(void);
int dl_set_list(unsigned int *list);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(float *v);
void mtx_mult(float *m);
void mtx_get34(float *m);
float *quat_to_mat4(float *out, float *q);
void mat4_mul(float *r, float *a, float *b);
void model_draw(void *mdl, void *draw);
void *model_hook(void *mdl, int which, void *fn);
float vec4_distance_xyz(float *a, float *b);
unsigned int *dl_alpha_test(unsigned int *list, int func, int ref, int mask);
unsigned int *dl_depth_mask(unsigned int *list, int mask);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
unsigned int *gim_set_texture(unsigned int *list, void *gim);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
void se_play(int id);
unsigned int *gfx_begin_2d(unsigned int *list);
unsigned int *gfx_end_2d(unsigned int *list);
int ehf_get_frame_count(void *e);
int ehf_get_cur_frame(void *e);
unsigned int *ehf_draw(unsigned int *list, void *e, void *opts);
int ehf_get_data_ptr(void *e);
void ehf_get_size(void *e, int *target, int *limit);
int ehf_set_frame_alias(void *e, int frame);
int ehf_set_anim(void *e, int target, int limit);
void req_flag_set(int i);
int cfg_get_bits_1_3_4(void);
int cfg_get_language(void);
int sysvar_get_a98(void);

/* cross-file calls of this range */
void dd_cursor_set(int player, int zone, int idx, int save);
void dd_cursor_restore(void);
void dd_cursor_get(DdLoc *out);
void dd_cursor_first_target(void);
void dd_cutin_play(DdCutinDesc *d);
void dd_ev_reset(int player);
void dd_ev_slot_set(int i, int a, int b, int c);
void dd_ev_8817670(int v);
unsigned short *dlg_get_text(int id);
unsigned short region_pick_value(int flag);
unsigned int cfg_get_flag(int which);
int card_get_atk(int id);
int card_get_def(int id);
int card_is_attribute9(int id);

/* message window (dd_msg_win.c): one MenuWidget shared by the duel's questions and choices */
typedef struct DdMsgWin {              /* 0x08C58A40, 0x144 */
    MenuWidget w;                      /* +0x000 */
    void (*on_close)(void);            /* +0x120 called once by func_08825778 */
    void *(*draw_hook)(void *dl);      /* +0x124 drawing before the window */
    void (*on_move)(MenuWidget *w, int old, int cur); /* +0x128 the cursor moved */
    int (*update)(void);               /* +0x12C replaces the menu update */
    int result;                        /* +0x130 menu_update's last answer */
    unsigned short *title;             /* +0x134 heap copies of the texts */
    unsigned short *text;              /* +0x138 */
    int x13c;                          /* +0x13C */
    int hidden;                        /* +0x140 */
} DdMsgWin;

typedef struct DdNumPrompt {           /* 0x08C58A2C: number choice (dd_prompt_win.c, drawn by dd_msg_win.c) */
    int min;
    int max;
    int step;
    int fmt;                           /* dialog text id of the number format */
    MenuWidget *w;                     /* +0x10 */
} DdNumPrompt;

extern DdMsgWin dd_msg_win;
extern DdNumPrompt dd_num_prompt;
extern unsigned short dd_num_text[256]; /* 0x08C5882C */

#endif
