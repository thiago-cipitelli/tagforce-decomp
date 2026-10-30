// @range 0x08806434-0x08807064
// @data 0x08874A58-0x08874A90
/* The duel scene itself: the scene flags, the standalone test main (loads two recipe decks and
   libduel_eng.prx and runs a duel without libduel_mgr), the per-frame draw of the layers, the
   view state (tag duel, the two highlighted card locations) and small queries on the game state. */
#include "dd_d1.h"

/* other ranges of libduel_draw */
void func_08805C34(int base, int size);
void func_08805CE4(void);
void func_08805D2C(DdDuelParam *p);
void func_08805EE4(void);
int func_08805F88(void);
int func_08806030(void);
int func_0880618C(void);
void func_088061C0(void);
void func_088061C8(void);
int func_088061FC(void);
void dd_view_draw3d(void);
void dd_view_update(float dt);

void func_08811D48();
int func_0881351C(void);
void func_0881CA40(void);
int func_0881CB84(void);
void *func_0881CCF8(void *list);
int *func_0881DE50(void);
void func_0881E150(void);
void func_0881E214(float dt);
void func_08823234(void);
void func_0884D718(void);
void func_0884D838(void);
void func_0884D844(float dt);
void func_08858814(void);
int func_0886677C(void);
int func_088667B4(void);
int func_08866914(void);
int func_08866940(int player);
int func_08866958(int player);
int func_088669B4(int player);
int func_08866A0C(void);
int func_08866BC0(int player, int zone, int index);

/* BOOT */
void recipe_load(int n, RecipeData *out);
int sceKernelCreateFpl(const char *name, int part, int attr, unsigned int size, unsigned int blocks, void *opt);
int sceKernelAllocateFpl(int fpl, void **data, unsigned int *timeout);
int sceKernelDeleteFpl(int fpl);
void *heap_create(void *mem, unsigned int size);
void duel_player_type_set(int player, int type);
void gfx_overlay_enable(int on);
void scene_pop_to(unsigned short from, unsigned short arg);
void sceKernelDcacheWritebackInvalidateAll(void);
int modmgr_load_and_start(int dir, const char *name);
int sceKernelStopModule(int modid, unsigned int argsize, void *argp, int *status, void *option);
int sceKernelUnloadModule(int modid);
unsigned int *dl_depth_range(unsigned int *list, int near, int far);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_clear_color(void *list, unsigned int rgba);
void *dl_clear_depth(void *list, int z);
void *dl_clear_stencil(void *list, unsigned int s);
void *dl_clear(void *list, int flags);
unsigned int *dl_disable(unsigned int *list, int state);
int fade_apply(void **list);
void *game_state2_get(void);
unsigned int cfg_get_flag(int which);
int sysvar_get_a98(void);
unsigned short region_pick_value(int flag);
int req_flags_any(void);
unsigned int cfg_get_bits_1_3_4(void);


/* sets (on > 0) or clears (on == 0) the scene flags `mask`; returns their previous value */
int dd_scene_flags(int mask, int on)
{
    DdScene *sc = &dd_scene;
    int old = mask & sc->flags;

    if (on == 0)
        sc->flags &= ~mask;
    else if (on > 0)
        sc->flags |= mask;
    return old;
}

void dd_test_update(void);
void dd_test_exit(void);
void dd_eng_load(void);
void dd_test_init(void);

/* scene main of the standalone test: duel between recipe decks 1 and 25 */
void dd_test_main(void)
{
    DdDuelParam p;
    RecipeData deck1;
    RecipeData deck0;
    void *mem_main;
    void *mem_movie;
    int fpl_main;
    int fpl_movie;
    void *heap;

    recipe_load(1, &deck0);
    recipe_load(25, &deck1);
    memset(&p, 0, sizeof(p));
    p.n = 2;
    p.chr[0] = 1;
    p.chr[1] = 5;
    p.deck[0] = &deck0;
    p.deck[1] = &deck1;
    p.first = 0;
    dd_eng_load();
    dd_test_init();
    fpl_main = sceKernelCreateFpl("HeapMain", 2, 0, 0x900000, 1, 0);
    fpl_movie = sceKernelCreateFpl("HeapMovie", 2, 0, 0x200000, 1, 0);
    sceKernelAllocateFpl(fpl_main, &mem_main, 0);
    sceKernelAllocateFpl(fpl_movie, &mem_movie, 0);
    heap = heap_create(mem_main, 0x900000);
    func_08805C34((int)heap, (int)heap_create(mem_movie, 0x200000));
    func_08805D2C(&p);
    func_08805EE4();
    while (!func_08805F88())
        sys_frame_wait();
    duel_player_type_set(0, 0);
    duel_player_type_set(1, 1);
    gfx_overlay_enable(0);
    func_08806030();
    sys_state_set_callbacks(0, (int (*)(void))dd_test_update, 0);
    do {
        sys_frame_wait();
        func_088061FC();
    } while (!(pad_get()->b.trig & 1));
    sys_state_set_callbacks(0, 0, 0);
    func_0880618C();
    func_08805CE4();
    sceKernelDeleteFpl(fpl_movie);
    sceKernelDeleteFpl(fpl_main);
    scene_pop_to(3, 0);
}

void dd_eng_unload(void);

/* scene exit */
void dd_test_exit(void)
{
    func_0881351C();
    dd_res_term();
    efs_term();
    func_0881CA40();
    dd_eng_unload();
}

/* per-frame callback of the standalone test */
void dd_test_update(void)
{
    func_088061C0();
    func_088061C8();
}

/* loads and starts libduel_eng.prx (standalone test) */
void dd_eng_load(void)
{
    sceKernelDcacheWritebackInvalidateAll();
    dd_scene.eng = modmgr_load_and_start(2, "libduel_eng.prx");
}

void dd_test_init(void)
{
}

/* stops and unloads libduel_eng.prx */
void dd_eng_unload(void)
{
    int status;

    sceKernelStopModule(dd_scene.eng, 0, 0, &status, 0);
    sceKernelUnloadModule(dd_scene.eng);
    dd_scene.eng = -1;
}

/* per-frame draw of the duel view, layer by layer */
void dd_view_draw(void)
{
    void *list;

    dl_set_list(dl_depth_range(dl_select_layer(0, 0), 50000, 10000));
    if (!dd_scene_flags(8, -1))
        dd_view_draw3d();
    dd_view_update(1.0f / 30.0f);
    if (dd_scene_flags(0x10, -1))
        dd_scene_flags(0x10, 0);
    if (!dd_scene_flags(0x10, -1) && dd_scene_flags(8, -1)) {
        dl_set_list(dl_scissor(dl_select_layer(7, 0), 100, 0, 280, 272));
        dl_set_list(dl_clear(dl_clear_stencil(dl_clear_depth(dl_clear_color(dl_select_layer(7, 0), 0xFF000000), 0), 0), 0x17));
        func_0884D718();
        dl_set_list(dl_disable(dl_select_layer(7, 0), 2));
        func_0884D838();
    }
    dl_set_list(func_0881CCF8(dl_select_layer(7, 0)));
    func_0881E150();
    func_0881E214(1.0f / 30.0f);
    if (!dd_scene_flags(0x10, -1) && dd_scene_flags(8, -1)) {
        func_08858814();
        func_0884D844(1.0f / 30.0f);
    }
    if (dd_scene_flags(4, -1))
        func_08823234();
    if (dd_scene_flags(0x20, -1)) {
        list = dl_select_layer(9, 0);
        list = dl_disable(list, 1);
        fade_apply(&list);
        dl_set_list(list);
    }
}

PadState *dd_pad_get(void)
{
    return pad_get();
}

/* clears the view state */
void dd_view_clear(void)
{
    memset(&dd_view, 0, sizeof(DdView));
}

void dd_view_term(void)
{
}

int dd_view_tag_get(void)
{
    return dd_view.tag;
}

void dd_view_tag_set(int tag)
{
    dd_view.tag = tag;
}

/* 1 unless byte 0x119 of the game state is 2 */
int dd_game_flag119(void)
{
    return ((unsigned char *)game_state_get())[0x119] != 2;
}

/* result of duel `n` of a series as won (1) or lost (0) for `player`, -1 unknown */
int dd_series_result(int player, int n)
{
    if (n >= ((DuelResults *)game_state2_get())->n_duels)
        return -1;
    switch (((DuelResults *)game_state2_get())->duels[n]) {
    case 1:
        return player != 0;
    case 2:
        return player == 0;
    case 3:
        return 2;
    }
    return -1;
}

/* player whose input drives the duel view, or -1 */
int dd_input_player(void)
{
    DdView *v = &dd_view;

    if (func_088669B4(1)) {
        v->idle[0] = 0;
        if (++v->idle[1] > 0x28)
            return 1;
    } else if (func_088669B4(0)) {
        v->idle[1] = 0;
        if (++v->idle[0] > 0x28)
            return 0;
    } else {
        v->idle[0] = 0;
        v->idle[1] = 0;
    }
    if (func_08866A0C())
        return 0;
    if (func_0881CB84() == 2)
        return 0;
    if (func_0881CB84() == 4)
        return 0;
    return func_0886677C();
}

void dd_08806B5C(void)
{
    func_08811D48();
}

/* character slot `i` of the game state */
void *dd_game_slot(int i)
{
    return (char *)game_state_get() + 0x130 + i * 0x188;
}

int dd_08806BA4(void)
{
    return func_088667B4() + 1;
}

/* 2 when flag 8 is set, else 1 or (no card selected) */
int dd_08806BC4(void)
{
    DdView *v = &dd_view;

    if (dd_scene_flags(8, -1))
        return 2;
    if (func_08866914() == 2 || !func_088669B4(func_08866914()) || req_flags_any() ||
        !func_08866958(func_08866914()))
        return 1;
    return !v->x10;
}

int dd_view_x14_get(void)
{
    return dd_view.x14;
}

void dd_view_x14_set(int v)
{
    dd_view.x14 = v;
}

int dd_08806C84(int player)
{
    int *p = func_0881DE50();

    if (cfg_get_flag(2) == 0 || func_08866958(player) || sysvar_get_a98() || (*p & region_pick_value(1)))
        return 0;
    return 1;
}

/* 1 in a tag duel (byte 0x118 of the game state is 2) */
int dd_is_tag_duel(void)
{
    return ((unsigned char *)game_state_get())[0x118] == 2;
}

/* the two highlighted card locations */
void dd_view_locs_get(DdLoc *a, DdLoc *b)
{
    DdView *v = &dd_view;

    if (a)
        *a = v->a;
    if (b)
        *b = v->b;
}

/* sets the highlighted locations; hand indexes 1..4 of the field rows fold into the zone */
void dd_view_locs_set(DdLoc a, DdLoc b)
{
    DdView *v = &dd_view;

    if (a.idx > 0 && a.idx < 5) {
        a.zone += a.idx;
        a.idx = 0;
    }
    if (b.idx > 0 && b.idx < 5) {
        b.zone += b.idx;
        b.idx = 0;
    }
    v->a = a;
    v->b = b;
}

/* 1 when the card at `loc` can be shown face up to the viewer */
int dd_loc_visible(DdLoc loc)
{
    int zone = loc.zone;
    int player;
    unsigned int index;
    DdCobj *c;

    if (zone == 11)
        goto own;
    if (zone == 10 || (zone >= 5 && zone < 10) || (zone >= 0 && zone < 5))
        goto field;
    switch (zone) {
    default:
        return 0;
    case 13:
    case 12:
    case 14:
    case 15:
        return func_08866BC0(loc.player, zone, loc.idx) != 0;
    }
field:
    c = dd_cobj_at(loc.player, zone, loc.idx);
    if (c && c->loc.f14)
        return 1;
own:
    player = loc.player;
    if (func_08866940(player) || func_08866BC0(player, zone, index = loc.idx))
        return 1;
    c = dd_cobj_at(player, zone, index);
    if (c) {
        if (c->a7 || c->a6 || c->b0)
            return 1;
    }
    return 0;
}

void dd_view_x10_set(int v)
{
    dd_view.x10 = v;
}

int dd_08807024(void)
{
    return -1;
}

int dd_0880702C(void)
{
    return 0;
}

float dd_speed_tab[4] = { 0.5f, 1.0f, 2.0f, 5.0f };

/* animation speed of the duel option (0..3) */
float dd_speed_get(void)
{
    return dd_speed_tab[cfg_get_bits_1_3_4()];
}
