// @range 0x0880C728-0x0880D6C0
// @rodata 0x08822574-0x088225B0
// @data 0x08822BF4-0x08822C24
/* Scene steps: the rewards checked when the editor is entered from the story (first deck, owned
   cards...), the deck name input, the per-frame update that runs the sub-screens (card info, help,
   recipe viewer, deck tutorial, each faded in and out) and the cursor's info text, and the
   callback that keeps the id of a sub-screen module loaded asynchronously. */
#include "libdeck.h"

unsigned char sysstate_get_byte_97d0(int i);
void sysstate_max_byte_97d0(int i, unsigned char v);
int cfg_is_mode1(void);
int deck_card_free_count_a(unsigned short id);
void cardinfo_open(int id, void *gim, void *bg);
int cardinfo_update(void);
void cardinfo_set_lists(void *gim, void *names, void *list);
int cardinfo_has_alt_pict(void);
int fade_is_done(void);
int ehfile_request_module(int dir, const char *name, int (*cb)(void *, int), void *arg);
int sceKernelStartModule(int modid, unsigned int argsize, void *argp, int *status, void *option);
void cip_close(Cip *c);
int cip_is_idle(Cip *c);
void recipe_viewer_init(int op, void *cfg);
int recipe_viewer_update(void);
void libdeck_tutorial_08804008(int lesson, void *cfg);
int libdeck_tutorial_0880418C(void);

void func_0880A8BC(int msg, int arg);
void func_0880B81C(void);
void dk_action_reopen(DkEnt *e);
int dk_osk_open(void *a, void *b, int max, void *c);
int dk_osk_update(int mode);
void func_0881D918(int page, void *heap, void *gim, void *strtbl, void *labels);
void func_0881DB9C(int page, void *heap, void *gim, void *strtbl, void *labels);
int func_0881DC28(void);
int func_0881E024(void);
void dk_deck_save(void);
void dk_deck_load(void);
void *dk_pack_get(void);
int dk_res_load(void);
void dk_cardlist_setup(void);

extern int dk_tut_module;              /* 0x095D26FC libdeck_tutorial module id */
extern void *dk_cip_mem;               /* 0x095D2700 card image reader memory (released while the tutorial runs) */

typedef struct DkRecipeCfg {           /* recipe_viewer_init */
    void *heap;
    Cip *cip;
    void *pack;
    void (*save)(void);
    void (*load)(void);
} DkRecipeCfg;

typedef struct DkTutorialCfg {         /* libdeck_tutorial_08804008 */
    void *heap;
    void *pack;
} DkTutorialCfg;

int dk_module_loaded(int *slot, int id);
void dk_info_run(void);
void dk_help_run(void);
void dk_recipe_run(void);
void dk_tutorial_run(void);

/* entered from the story: grants the deck-building rewards still pending (message 7 shown in
   mode 1): 0 first visit, 1 an owned card of attribute icon 3, 2 the one granted elsewhere,
   3 / 4 at least 150 / 300 cards owned */
void dk_check_rewards(void)
{
    int id;

    if (dk_scene_arg != 1)
        return;
    if (sysstate_get_byte_97d0(0) != 2) {
        sysstate_max_byte_97d0(0, 2);
        dk_flags.lo = 0;
        if (cfg_is_mode1()) {
            func_0880A8BC(7, dk_flags.lo);
            se_play(0x621A);
            return;
        }
    }
    if (sysstate_get_byte_97d0(1) != 2) {
        for (id = 4007; id < 6958; id++) {
            if (card_get_flag31(id) && deck_card_free_count_a(id) > 0 && card_attribute_icon_is3(id)) {
                sysstate_max_byte_97d0(1, 2);
                dk_flags.lo = 1;
                if (cfg_is_mode1()) {
                    func_0880A8BC(7, dk_flags.lo);
                    se_play(0x621A);
                    return;
                }
                break;
            }
        }
    }
    if (sysstate_get_byte_97d0(2) != 2 && sysstate_get_byte_97d0(2) == 1) {
        sysstate_max_byte_97d0(2, 2);
        dk_flags.lo = 2;
        if (cfg_is_mode1()) {
            func_0880A8BC(7, dk_flags.lo);
            se_play(0x621A);
            return;
        }
    }
    if (sysstate_get_byte_97d0(3) != 2) {
        int n = 0;
        int i;

        for (i = 4007; i < 6958; i++) {
            if (card_get_flag31(i))
                n += deck_card_free_count_a(i);
        }
        if (n >= 150) {
            sysstate_max_byte_97d0(3, 2);
            dk_flags.lo = 3;
            if (cfg_is_mode1()) {
                func_0880A8BC(7, dk_flags.lo);
                se_play(0x621A);
                return;
            }
        }
    }
    if (sysstate_get_byte_97d0(4) != 2) {
        int i;
        int n = 0;

        for (i = 4007; i < 6958; i++) {
            if (card_get_flag31(i))
                n += deck_card_free_count_a(i);
        }
        if (n >= 300) {
            sysstate_max_byte_97d0(4, 2);
            dk_flags.lo = 4;
            if (cfg_is_mode1()) {
                func_0880A8BC(7, dk_flags.lo);
                se_play(0x621A);
            }
        }
    }
}

/* deck name input on the system keyboard */
void dk_name_input_open(void *a, void *b, void *c)
{
    if (dk_osk_open(a, b, 16, c) == 0)
        dkv.busy = 1;
}

void dk_name_input_update(void)
{
    if (dk_osk_update(4))
        dkv.busy = 0;
}

/* per frame: the sub-screen being shown, else the cursor timer ("new" mark cleared after 16
   frames) and the info text when the card changes */
void dk_update(void)
{
    DkEnt *e;

    if (dk_flags.b7) {
        dk_tutorial_run();
    } else if (dk_flags.info) {
        dk_info_run();
    } else if (dk_help_page) {
        dk_help_run();
    } else if (dk_flags.deck_op) {
        dk_recipe_run();
    } else {
        if (++dkv.cursor.timer >= 0x10) {
            if (dkv.cursor.ent != 0)
                dkv.cursor.ent->marked = 1;
            if (dkv.cursor.timer > 0x8000)
                dkv.cursor.timer = 0x100;
        }
        if (!dkv.lock && (e = dk_cur_entry(dkv.side)) != 0 && e->id != dkv.cursor.id)
            dk_info_update();
    }
}

/* the card info screen: fade out, open, run until closed, fade back */
void dk_info_run(void)
{
    DkEnt *e;

    switch (dk_substep) {
    case 0:
        fade_start_in(1, 0xFF000000, 16);
        dk_substep++;
    case 1:
        if (fade_is_done() == 0)
            return;
        e = dk_cur_entry(dkv.side);
        cardinfo_open(e->id, dk_res_get(0x800C), dk_res_get(0x8012));
        cardinfo_set_lists(dk_res_get(0x8015), dk_res_get(0x8036), dk_res_get(0x8037));
        cardinfo_has_alt_pict();
        dkv.info = 1;
        dk_substep++;
        return;
    case 2:
        if (cardinfo_update() == 0)
            return;
        dkv.info = 0;
        fade_start(1, 0xFF000000, 16);
        dk_substep++;
    case 3:
        if (fade_is_done()) {
            dk_substep = 0;
            dk_flags.info = 0;
        }
        return;
    default:
        return;
    }
}

/* the help viewer (page 0xFF: the contents) */
void dk_help_run(void)
{
    switch (dk_substep) {
    case 0:
        se_play(0x6217);
        fade_start_in(1, 0xFF000000, 16);
        dk_substep++;
    case 1:
        if (fade_is_done() == 0)
            return;
        if (dk_help_page == 0xFF)
            func_0881D918(13, dk_heap, dk_res_get(0x8014), dk_res_get(0x8034), dk_res_get(0x8035));
        else
            func_0881DB9C(dk_help_page, dk_heap, dk_res_get(0x8014), dk_res_get(0x8034), dk_res_get(0x8035));
        dkv.help = 1;
        dk_substep++;
        return;
    case 2:
        if (func_0881DC28() == 0)
            return;
        dk_substep++;
    case 3:
        if (func_0881E024() == 0)
            return;
        dkv.help = 0;
        fade_start(1, 0xFF000000, 16);
        dk_substep++;
    case 4:
        if (fade_is_done()) {
            dk_substep = 0;
            dk_help_page = 0;
        }
        return;
    default:
        return;
    }
}

/* librecipe_viewer (deck recipes: save / load the deck), loaded on first use */
void dk_recipe_run(void)
{
    int one;
    DkRecipeCfg cfg;
    int r;

    switch (dk_substep) {
    case 0:
        fade_start_in(1, 0xFF000000, 16);
        dk_substep++;
    case 1:
        if (fade_is_done())
            dk_substep++;
        break;
    case 2:
        if (dk_module == 0) {
            gfx_overlay_enable(1);
            dkv.loading = 1;
            if (ehfile_request_module(2, "librecipe_viewer.prx", (int (*)(void *, int))dk_module_loaded, &dk_module) == 0)
                break;
            dk_substep++;
            break;
        }
        dk_substep++;   /* already loaded: skip starting it */
        (*(volatile unsigned short *)&dk_substep)++;
        break;
    case 3:
        if (dk_module == 0)
            break;
        one = 1;
        sceKernelStartModule(dk_module, 4, &one, 0, 0);
        dk_substep++;
        break;
    case 4:
        cfg.heap = dk_heap;
        cfg.cip = dk_cip;
        cfg.pack = dk_pack_get();
        cfg.save = dk_deck_save;
        cfg.load = dk_deck_load;
        recipe_viewer_init(dk_flags.deck_op, &cfg);
        dkv.recipe = 1;
        gfx_overlay_enable(0);
        dkv.loading = 0;
        dk_substep++;
        break;
    case 5:
        r = recipe_viewer_update();
        if (r == -1)
            break;
        if (r >= 0) {
            if (dk_flags.deck_op != 1 && dk_flags.deck_op == 2)
                dk_action_reopen(dk_cur_entry(dkv.side));
        }
        fade_start(1, 0xFF000000, 16);
        dkv.recipe = 0;
        dk_substep++;
    case 6:
        if (fade_is_done()) {
            dk_substep = 0;
            dk_flags.deck_op = 0;
        }
        break;
    }
}

/* libdeck_tutorial (lesson dk_flags.lo): the card images are released while it runs and loaded
   again afterwards */
void dk_tutorial_run(void)
{
    int one;
    DkTutorialCfg cfg;

    switch (dk_substep) {
    case 0:
        fade_start_in(1, 0xFF000000, 16);
        dk_substep++;
        break;
    case 1:
        if (fade_is_done())
            dk_substep++;
        break;
    case 2:
        if (dk_cip != 0)
            cip_close(dk_cip);
        if (dk_tut_module == 0) {
            gfx_overlay_enable(1);
            dkv.loading = 1;
            if (ehfile_request_module(2, "libdeck_tutorial.prx", (int (*)(void *, int))dk_module_loaded, &dk_tut_module) == 0)
                break;
        }
        dk_substep++;
        break;
    case 3:
        if (dk_tut_module == 0)
            break;
        one = 1;
        sceKernelStartModule(dk_tut_module, 4, &one, 0, 0);
        dk_substep++;
        break;
    case 4:
        if (cip_is_idle(dk_cip) == 0)
            break;
        dk_cip = 0;
        if (dk_cip_mem != 0) {
            heap_set_level(dk_heap, dk_cip_mem);
            dk_cip_mem = 0;
        }
        cfg.heap = dk_heap;
        cfg.pack = dk_pack_get();
        libdeck_tutorial_08804008(dk_flags.lo, &cfg);
        dkv.tutorial = 1;
        dkv.loading = 0;
        dk_substep++;
        break;
    case 5:
        if (libdeck_tutorial_0880418C() == 0)
            break;
        dkv.tutorial = 0;
        dkv.loading = 1;
        sceKernelStopModule(dk_tut_module, 0, 0, 0, 0);
        sceKernelUnloadModule(dk_tut_module);
        dk_tut_module = 0;
        dk_substep++;
    case 6:
        if (dk_res_load() == 0)
            break;
        dk_cardlist_setup();
        func_0880B81C();
        dkv.loading = 0;
        fade_start(1, 0xFF000000, 16);
        dk_substep++;
    case 7:
        if (fade_is_done()) {
            dk_substep = 0;
            dk_flags.b7 = 0;
            dk_check_rewards();
        }
        break;
    }
}

/* ehfile_request_module callback: keeps the module id */
int dk_module_loaded(int *slot, int id)
{
    int loaded = id > 0;

    if (loaded)
        *slot = id;
    return loaded;
}
