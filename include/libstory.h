/* libstory: the story scene (scene 2): the story script interpreter, the story state (characters,
   decks, flags), the events that drive libfield, libshop and the sub-screen modules (charalist,
   cardalbum, recipe_viewer, ...). Module state and the BOOT (libehsys) calls no shared header
   declares. Offsets are the ground truth; field names are partial.
   Each block below belongs to the address range in its marker; keep every block compilable. */
#ifndef LIBSTORY_H
#define LIBSTORY_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"
#include "game.h"
#include "libc.h"
#include "psp.h"

/* BOOT calls not declared by the shared headers */
void gfx_overlay_enable(int on);
void rand_seed_from_rtc(void);

/* imports of libfield */
void fld_sys_init(void);
void fld_sys_update(void);
int fld_sys_draw(void);
int fld_sys_term(void);
int fld_cmd_start(int kind, void *arg);
int fld_cmd_poll(int kind, void *arg);

/* --- 0x08804000-0x08806A18 --- */

/* --- 0x08806A18-0x0880B3E4 --- */

/* --- 0x0880B3E4-0x0880DE58 --- */

/* --- 0x0880DE58-0x08810D24 --- */

/* --- 0x08810D24-0x088138AC --- */

/* --- 0x088138AC-0x088156A4 --- */

/* --- 0x088156A4-0x08817E64 --- */

#endif
