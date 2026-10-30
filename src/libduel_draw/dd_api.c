// @range 0x08805BF4-0x08806434
// @data 0x08874A2C-0x08874A58
/* Module entry and the API the duel manager calls (libduel_mgr imports them by NID): heap and
   loader setup, the duel parameters (characters, decks), the loading of the duel resources, the
   start and end of the duel view, and small forwards to the field and menu code. */
#include "dd_d1.h"

/* other ranges of libduel_draw */
void dd_param_load(int a, const char *path);
void dd_view_init(void);
void dd_view_free(void);
void dd_info_select_uid();
void dd_cobjs_setup(void);
void dd_cobjs_add_deck(int i, RecipeData *deck);
void dd_cobjs_map(int side, int v);
void dd_cobjs_rebuild(void);
void dd_cobjs_sync_set();
void dd_lpp_set(int side, int lp, int a);
void dd_lpp_turn_set(int side, int a);
void dd_lpp_phase_set(int phase, int a);
void dd_lpp_phase_state(int side, int phase);
void dd_cursor_set(int side, int a, int b, int c);
void dd_heap_set(int a, int b);
void dd_heap_clear(void);
void *dd_heap_get(void);
void dd_mode_init(void);
void dd_mode_term(void);
void dd_mode_set(int on);
int dd_mode_get(void);
void dd_mode_update(void);
void dd_input_set(int on);
void dd_act_set_filter();
void dd_tasks_init(void);
void dd_tasks_term(void);
void func_088212B8();
void dd_menu_hook_start(void);
void dd_menu_hook_frame(void);
void dd_menu_hook_nop(void);
void dd_menu_hook_end(void);
void dd_chara_init(void);
void dd_chara_term(void);
void dd_chara_load(int i, int chr, int sub);
void dd_chara_select(int i);
void zone_views_init(void);
void zone_views_term(void);
void dv_init(void);
int dv_player_to_side(int player);
int dv_side_to_player(int side);
int dv_turn_side(void);
int dv_phase(void);
void sel_start(int side, int a, int b, int c);

extern int dd_script_data[];           /* 0x08C3CDB0 data block of the scene script */

/* PRX entry: registers the scene with the script runner */
int module_start(int argc, void *argp)
{
    if (argc > 0)
        return 0;
    ehscript_register((int (*)(void))dd_test_main, (int (*)(void))dd_test_exit, dd_script_data);
    return 0;
}

/* export: creates the scene heap (base, size), the EFS and display list setup, the model
   allocator; sorts the resource tables and reads the view parameters */
void dd_api_init(int base, int size)
{
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    dd_heap_set(base, size);
    efs_set_heap(dd_heap_get());
    dl_set_list(view_init(dl_select_layer(0, 0)));
    model_set_allocator((void *(*)(unsigned int))dd_alloc, (void (*)(void *))dd_free);
    dd_res_sort();
    dd_param_load(-1, "host0:../src/duel_draw/DuelViewParam.txt");
    dd_scene.result = 0;
    dd_scene.active = 0;
    dd_menu_hook_start();
}

/* export: tears down what dd_api_init set up */
void dd_api_term(void)
{
    dd_menu_hook_frame();
    dd_chara_term();
    dd_res_term();
    model_set_allocator(0, 0);
    efs_term();
    dd_heap_clear();
}

/* export: takes the duel parameters (characters, variants, decks copied into the scene) */
void dd_api_set_param(DdDuelParam *p)
{
    DdScene *sc = &dd_scene;
    int i;

    sc->p = *p;
    for (i = 0; i < 4; i++) {
        if (p->deck[i] == 0)
            memset(&sc->decks[i], 0, sizeof(RecipeData));
        else
            memcpy(&sc->decks[i], p->deck[i], sizeof(RecipeData));
        sc->p.deck[i] = &sc->decks[i];
    }
    sc->result = 0;
    sc->active = 0;
    dd_scene_flags(-1, 0);
    dd_scene_flags(0x80, 1);
    if (sc->p.disk[0] == 0 && sc->p.disk[1] == 0)
        sc->p.disk[1] = 1;
    dd_view_clear();
    dd_view_tag_set(p->n >= 3);
    dd_menu_hook_nop();
    dd_chara_init();
    dd_mode_init();
    sc->disk[0] = sc->p.disk[0];
    sc->disk[1] = sc->p.disk[1];
    sc->first = sc->p.first;
}

/* export: ends the duel view */
void dd_api_view_end(void)
{
    zone_views_term();
    dd_mode_term();
    dd_tasks_term();
    dd_chara_term();
    dd_menu_hook_end();
    dd_view_term();
    dd_view_free();
    dd_res_release();
}

/* export: starts loading the packs, the cut-ins and the card images of the decks */
void dd_api_load_start(void)
{
    DdDuelParam *p = &dd_scene.p;
    int i;

    dd_pack_get(0x80001);
    dd_pack_get(0x80002);
    dd_cutin_setup(p->chr, p->sub, p->n);
    dd_cutin_load();
    dd_cards_reset();
    for (i = 0; i < p->n; i++) {
        if (p->deck[i])
            dd_cards_add_deck(p->deck[i]);
    }
}

/* export: 1 when everything started by dd_api_load_start is loaded */
int dd_api_load_ready(void)
{
    if (dd_pack_get(0x80001) == 0)
        return 0;
    if (dd_pack_get(0x80002) == 0)
        return 0;
    if (dd_cutin_ready() == 0)
        return 0;
    if (dd_cards_ready_m() == 0)
        return 0;
    dd_cards_ready_h();
    return 1;
}

/* export: duel disks of both players and the player who starts */
void dd_api_set_disks(int *disk, int first)
{
    dd_scene.disk[0] = disk[0];
    dd_scene.disk[1] = disk[1];
    dd_scene.first = first;
}

/* export: starts the duel view (field, LP, phase, characters) */
int dd_api_view_start(void)
{
    DdScene *sc = &dd_scene;
    int i;

    font_set_flag2(1);
    dv_init();
    dd_view_init();
    dd_tasks_init();
    zone_views_init();
    dd_cobjs_setup();
    for (i = 0; i < sc->p.n; i++)
        dd_cobjs_add_deck(i, sc->p.deck[i]);
    dd_cobjs_map(0, sc->disk[dv_side_to_player(0)]);
    dd_cobjs_map(1, sc->disk[dv_side_to_player(1)]);
    dd_lpp_set(0, 8000, 1);
    dd_lpp_set(1, 8000, 1);
    dd_lpp_turn_set(dv_player_to_side(sc->first), 1);
    dd_lpp_phase_set(7, 1);
    dd_mode_set(1);
    for (i = 0; i < sc->p.n; i++) {
        dd_chara_load(i, sc->p.chr[i], sc->p.sub[i]);
        dd_chara_select(i);
    }
    sc->active = 1;
    return 1;
}

/* export: stops the duel view */
int dd_api_view_stop(void)
{
    dd_scene.active = 0;
    return 1;
}

/* export */
void dd_api_free_cutins(void)
{
    dd_chara_term();
    dd_cutin_free();
}

/* export: nothing */
void dd_api_nop1(void)
{
}

/* export: per-frame update of the duel view */
void dd_api_update(void)
{
    if (dd_scene.active)
        dd_view_draw();
}

/* export: nothing */
void dd_api_nop2(void)
{
}

/* export: draws the duel view; returns the scene result */
int dd_api_draw(void)
{
    dd_pad_get();
    dd_mode_update();
    return dd_scene.result;
}

/* export: forward with the screen side of `player` */
void dd_api_fwd_ae68(int player, int a, int b, int c)
{
    sel_start(dv_player_to_side(player), a, b, c);
}

/* export */
void dd_api_set_de9c(int off)
{
    dd_input_set(!off);
}

/* export */
void dd_api_df40(void)
{
    dd_act_set_filter();
}

/* export: refreshes the turn and phase panels */
void dd_api_refresh_turn(void)
{
    int side = dv_turn_side();
    int phase = dv_phase();

    dd_cobjs_rebuild();
    dd_lpp_phase_state(side, phase);
    dd_lpp_phase_state(side, phase);
}

/* export */
void dd_api_212b8(void)
{
    func_088212B8();
}

/* export: 1 in the states 5 and 8 of the duel menu */
int dd_api_menu_state58(void)
{
    int st;

    if ((st = dd_mode_get()) == 5 || st == 8)
        return 1;
    return 0;
}

/* export: forward with the screen side of `player` */
void dd_api_fwd_153e4(int player, int a, int b)
{
    dd_cursor_set(dv_player_to_side(player), a, b, 0);
}

/* export: the two highlighted card locations, with the player bit converted from screen side */
void dd_api_view_locs(DdLoc *a, DdLoc *b)
{
    dd_view_locs_get(a, b);
    if (a)
        a->player = dv_side_to_player(a->player);
    if (b)
        b->player = dv_side_to_player(b->player);
}

/* export */
void dd_api_info_select_uid(void)
{
    dd_info_select_uid();
}

/* export: shows the turn player and the phase */
void dd_api_show_turn_phase(void)
{
    dd_lpp_turn_set(dv_turn_side(), 1);
    dd_lpp_phase_set(dv_phase(), 1);
}

/* export */
void dd_api_cobjs_sync_set(void)
{
    dd_cobjs_sync_set();
}
