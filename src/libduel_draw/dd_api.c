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
void func_0880BEB0(void);
void func_0880CDCC();
void func_08811D18(int side, int lp, int a);
void func_08811D98(int side, int a);
void func_08811DC4(int phase, int a);
void func_08811E14(int side, int phase);
void func_088153E4(int side, int a, int b, int c);
void func_0881CA2C(int a, int b);
void func_0881CA40(void);
void *func_0881CA74(void);
void func_0881CAA0(void);
void func_0881CAB4(void);
void func_0881CAC8(int on);
int func_0881CB84(void);
void func_0881CDD4(void);
void func_0881DE9C(int on);
void func_0881DF40();
void func_0881E00C(void);
void func_0881E0D0(void);
void func_088212B8();
void func_0882BF00(void);
void func_0882BF24(void);
void func_0882BF50(void);
void func_0882BF58(void);
void func_0884D65C(void);
void func_0884D69C(void);
void func_0884D868(int i, int chr, int sub);
void func_0884D914(int i);
void func_08859DF8(void);
void func_08859E50(void);
void func_088666D8(void);
int func_08866744(int player);
int func_0886675C(int side);
int func_0886677C(void);
int func_088667AC(void);
void func_0886AE68(int side, int a, int b, int c);

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
void func_08805C34(int base, int size)
{
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    func_0881CA2C(base, size);
    efs_set_heap(func_0881CA74());
    dl_set_list(view_init(dl_select_layer(0, 0)));
    model_set_allocator((void *(*)(unsigned int))func_0881CA54, (void (*)(void *))func_0881CA64);
    dd_res_sort();
    dd_param_load(-1, "host0:../src/duel_draw/DuelViewParam.txt");
    dd_scene.result = 0;
    dd_scene.active = 0;
    func_0882BF00();
}

/* export: tears down what func_08805C34 set up */
void func_08805CE4(void)
{
    func_0882BF24();
    func_0884D69C();
    dd_res_term();
    model_set_allocator(0, 0);
    efs_term();
    func_0881CA40();
}

/* export: takes the duel parameters (characters, variants, decks copied into the scene) */
void func_08805D2C(DdDuelParam *p)
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
    func_0882BF50();
    func_0884D65C();
    func_0881CAA0();
    sc->disk[0] = sc->p.disk[0];
    sc->disk[1] = sc->p.disk[1];
    sc->first = sc->p.first;
}

/* export: ends the duel view */
void func_08805E90(void)
{
    func_08859E50();
    func_0881CAB4();
    func_0881E0D0();
    func_0884D69C();
    func_0882BF58();
    dd_view_term();
    dd_view_free();
    dd_res_release();
}

/* export: starts loading the packs, the cut-ins and the card images of the decks */
void func_08805EE4(void)
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

/* export: 1 when everything started by func_08805EE4 is loaded */
int func_08805F88(void)
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
void func_0880600C(int *disk, int first)
{
    dd_scene.disk[0] = disk[0];
    dd_scene.disk[1] = disk[1];
    dd_scene.first = first;
}

/* export: starts the duel view (field, LP, phase, characters) */
int func_08806030(void)
{
    DdScene *sc = &dd_scene;
    int i;

    font_set_flag2(1);
    func_088666D8();
    dd_view_init();
    func_0881E00C();
    func_08859DF8();
    dd_cobjs_setup();
    for (i = 0; i < sc->p.n; i++)
        dd_cobjs_add_deck(i, sc->p.deck[i]);
    dd_cobjs_map(0, sc->disk[func_0886675C(0)]);
    dd_cobjs_map(1, sc->disk[func_0886675C(1)]);
    func_08811D18(0, 8000, 1);
    func_08811D18(1, 8000, 1);
    func_08811D98(func_08866744(sc->first), 1);
    func_08811DC4(7, 1);
    func_0881CAC8(1);
    for (i = 0; i < sc->p.n; i++) {
        func_0884D868(i, sc->p.chr[i], sc->p.sub[i]);
        func_0884D914(i);
    }
    sc->active = 1;
    return 1;
}

/* export: stops the duel view */
int func_0880618C(void)
{
    dd_scene.active = 0;
    return 1;
}

/* export */
void func_0880619C(void)
{
    func_0884D69C();
    dd_cutin_free();
}

/* export: nothing */
void func_088061C0(void)
{
}

/* export: per-frame update of the duel view */
void func_088061C8(void)
{
    if (dd_scene.active)
        dd_view_draw();
}

/* export: nothing */
void func_088061F4(void)
{
}

/* export: draws the duel view; returns the scene result */
int func_088061FC(void)
{
    dd_pad_get();
    func_0881CDD4();
    return dd_scene.result;
}

/* export: forward with the screen side of `player` */
void func_08806228(int player, int a, int b, int c)
{
    func_0886AE68(func_08866744(player), a, b, c);
}

/* export */
void func_08806278(int off)
{
    func_0881DE9C(!off);
}

/* export */
void func_08806284(void)
{
    func_0881DF40();
}

/* export: refreshes the turn and phase panels */
void func_0880628C(void)
{
    int side = func_0886677C();
    int phase = func_088667AC();

    func_0880BEB0();
    func_08811E14(side, phase);
    func_08811E14(side, phase);
}

/* export */
void func_088062DC(void)
{
    func_088212B8();
}

/* export: 1 in the states 5 and 8 of the duel menu */
int func_088062E4(void)
{
    int st;

    if ((st = func_0881CB84()) == 5 || st == 8)
        return 1;
    return 0;
}

/* export: forward with the screen side of `player` */
void func_0880631C(int player, int a, int b)
{
    func_088153E4(func_08866744(player), a, b, 0);
}

/* export: the two highlighted card locations, with the player bit converted from screen side */
void func_08806360(DdLoc *a, DdLoc *b)
{
    dd_view_locs_get(a, b);
    if (a)
        a->player = func_0886675C(a->player);
    if (b)
        b->player = func_0886675C(b->player);
}

/* export */
void func_088063E8(void)
{
    dd_info_select_uid();
}

/* export: shows the turn player and the phase */
void func_088063F0(void)
{
    func_08811D98(func_0886677C(), 1);
    func_08811DC4(func_088667AC(), 1);
}

/* export */
void func_0880642C(void)
{
    func_0880CDCC();
}
