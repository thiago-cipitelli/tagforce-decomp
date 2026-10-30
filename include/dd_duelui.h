/* Private declarations of the duel UI files of libduel_draw (0x08857100-0x0886B25C): BOOT calls
   and functions of other ranges of the module. Types live in libduel_draw.h. */
#ifndef DD_DUELUI_H
#define DD_DUELUI_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"
#include "card_db.h"
#include "libc.h"
#include "libduel_draw.h"

int charmap_lookup(int ch, int *kind_out);
int charmap_lookup2(int code, DdCharPos *out);
extern DdPopup popup;
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *gim_set_texture(void *list, void *gim);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
void mtx_set_mode(int mode);
void mtx_get34(DdMat4 *m);
void mat4_mul(DdMat4 *out, DdMat4 *a, DdMat4 *b);
void mat4_transform_vec4(DdVec4A *out, DdMat4 *m, DdVec4A *v);
void se_play(int id);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_color(void *list, unsigned int color);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
float ceilf_vfpu(float x);
unsigned short *table_get_entry(void *t, unsigned int i);
unsigned int *gfx_get_state(void);
int region_get(void);
unsigned short region_pick_value(int flag);
PadState *pad_get(void);
void list_reset(DdList *l);
void list_layout(DdList *l);
void *list_draw(void *dl, DdList *l);
void list_term(DdList *l);
void list_set_active(DdList *l, int on);
int list_cursor(DdList *l);
void list_set_cursor(DdList *l, int c);
void listw_input_reset(DdList *l);
void listw_input(DdList *l);
void listw_term(DdList *l);
void listw_set_active(DdList *l, int on);
int listw_cursor(DdList *l);
void listw_set_cursor(DdList *l, int c);
int func_0882837C(void *self, void *a, void *b);
int func_088283D4(void *self, int ev, void *a, void *b);
int func_08828470(void *self, void *a, void *b, void *c, void *d);
int func_08828528(void *self, void *a, void *b, void *c, int d);
int func_08839240(void *self, void *a, void *b);
int func_0883936C(void *self, int ev, void *a, void *b);
int func_088393B0(void *self, void *a, void *b, void *c, void *d);

#endif
