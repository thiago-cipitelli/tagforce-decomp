/* Private declarations of the libduel_draw files of 0x08804000-0x088143A4: BOOT calls, globals and
   functions shared between these files (kept out of the shared libduel_draw.h). */
#ifndef DD_D1_H
#define DD_D1_H

#include "ehsys.h"
#include "game.h"
#include "libc.h"
#include "psp.h"
#include "cip.h"
#include "card_db.h"
#include "duel_setup.h"
#include "libduel_draw.h"

typedef struct Model Model;            /* BOOT model (opaque here) */

typedef struct Model Model;            /* BOOT model (opaque here) */
Model *model_load(void *data, void **anims);
void model_free(Model *m);
void model_anim_set(Model *m, void *data);
void model_set_tgma(Model *m, void *a);
void ehp_mount(int slot, void *pack);
void *ehp_find(void *pack, const char *name, int flags);
int ehf_tex_or_flags(void *ehf, void *flags);
int ehfile_get_size(int dir, const char *name);
void ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehfile_busy(void);
void *gim_set_texture(void *list, void *gim);
int inv_get_hi(unsigned short id);
int region_get(void);
int cfg_get_language(void);

typedef struct CharmapPos {            /* BOOT charmap_lookup2 */
    unsigned char a;
    unsigned char b;
    unsigned char c : 4;
    unsigned char hi : 4;
    unsigned char pad;
} CharmapPos;

typedef struct DdScene {               /* 0x08C3CDBC */
    int active;                        /* duel view running */
    int result;                        /* +0x04 returned to libduel_mgr each frame */
    int flags;                         /* +0x08 dd_scene_flags */
    int eng;                           /* +0x0C libduel_eng.prx module (standalone test) */
    DdDuelParam p;                     /* +0x10 */
    int disk[2];                       /* +0x50 */
    int first;                         /* +0x58 */
    RecipeData decks[4];               /* +0x5C copies of p.deck */
} DdScene;

int charmap_lookup(int ch, int *kind_out);
int charmap_lookup2(int code, CharmapPos *out);
int charmap_lookup3(int ch);
/* heap of the duel scene (0x0881CA54/0x0881CA64) */
void *func_0881CA54(int size);
void func_0881CA64(void *p);
int efs_set_heap(void *heap);
int efs_term(void);
unsigned int *dl_select_layer(int layer, unsigned int *size);
unsigned int *view_init(unsigned int *list);
int dl_set_list(void *list);
void model_set_allocator(void *(*alloc)(unsigned int size), void (*freefn)(void *p));
void font_set_flag2(int on);
extern DdCobjs dd_cobjs;
DdCobj *dd_zone_list(int player, int zone);
DdCobj *dd_cobj_alloc(void);
int dd_list_unlink(DdCobj *o, DdCobj *list);
int dd_list_add_tail(DdCobj *o, DdCobj *list);
int dd_list_add_head(DdCobj *o, DdCobj *list);
void dd_cobj_move(float dur, DdCobj *o, DdXyzw *pos, DdXyzw *rot, int sound);
void dd_cobj_tick(DdCobj *o, float dt);
void dd_slot_xform(DdXyzw *pos, DdXyzw *rot, DdPos at);
float dd_move_time(void);
DdCobj *dd_cobj_at(int player, int zone, int index);
int dd_zone_count(int player, int zone);
DdCobj *dd_cobj_find(int player, int zone, int uid);
extern DdInfo dd_info;
extern DdField dd_field;
extern DdScene dd_scene;
extern DdView dd_view;
extern DdModelRes dd_models[0x26];
extern DdPack dd_packs[4];
extern DdGimRes dd_gims[0x23];
extern DdEhfRes dd_ehfs[0xD];
extern DdGimRes dd_bins[0x23];
extern DdCards dd_cards;
extern DdBg dd_bg;
extern DdCutins dd_cutins;
int dd_res_cmp(const void *a, const void *b);
int dd_u16_cmp(const void *a, const void *b);
int dd_res_find(unsigned int id, void *table, int n, int size);
Model *dd_model_get(int id);
void *dd_ehf_get(int id);
void *dd_gim_get(int id);
void *dd_bin_get(int id);
void *dd_pack_get(int id);
void dd_test_main(void);
void dd_test_exit(void);
int dd_scene_flags(int mask, int on);
void dd_view_draw(void);
PadState *dd_pad_get(void);
void dd_view_clear(void);
void dd_view_term(void);
void dd_view_tag_set(int tag);
int dd_is_tag_duel(void);
void dd_view_locs_get(DdLoc *a, DdLoc *b);
void *dd_bg_get(int id);
void dd_bg_release(int id);
int dd_bg_filelist(int id, int *a, int *b, int *c, int *d);
void *dl_clear_color(void *list, unsigned int rgba);
void *dl_clear_depth(void *list, int z);
void *dl_clear_stencil(void *list, unsigned int s);
void *dl_clear(void *list, int flags);
unsigned int *dl_disable(unsigned int *list, int state);
void *dl_enable(void *list, int state);
void dd_res_sort(void);
void dd_res_term(void);
void dd_res_release(void);
void dd_cards_reset(void);
void dd_cards_add_deck(RecipeData *deck);
int dd_cards_ready_m(void);
int dd_cards_ready_h(void);
void dd_cutin_setup(int *ids, int *subs, int n);
void dd_cutin_free(void);
void dd_cutin_load(void);
int dd_cutin_ready(void);

extern DdLp dd_lp;
void *draw_set_offsets();

#endif
