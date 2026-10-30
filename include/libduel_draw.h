/* libduel_draw: the duel scene (rendering of the field, cards, effects, cut-ins, LP panels, duel
   menus). Module state and the BOOT (libehsys) calls no shared header declares. Offsets are the
   ground truth; field names are partial.
   Each block below belongs to the address range in its marker; keep every block compilable. */
#ifndef LIBDUEL_DRAW_H
#define LIBDUEL_DRAW_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"
#include "game.h"
#include "libc.h"
#include "psp.h"

/* ---- 0x088143A4-0x08825DDC ---- */
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
typedef struct GfxState GfxState;
int *gfx_get_state(void);


typedef struct DdLoc {                 /* a field location (cursor, targets) */
    unsigned short player : 1;
    unsigned short zone : 5;           /* 0-4 monsters, 5-9 spells/traps, 10 field, 11 hand, 12-15 decks/grave, 16 none */
    unsigned short idx : 10;           /* card index inside zones with several cards */
} DdLoc;

typedef struct DdCursor {              /* 0x90 at 0x08C54640 */
    int unk0[8];
    DdLoc cur;                         /* +0x20 */
    int col;                           /* +0x24 grid column */
    int row;                           /* +0x28 grid row */
    int saved;                         /* +0x2C saved_loc is valid */
    DdLoc saved_loc;                   /* +0x30 */
    int dirty;                         /* +0x34 */
    int unk38[22];
} DdCursor;

/* ---- end 0x088143A4-0x08825DDC ---- */

#endif
