// @range 0x0880B874-0x0880BD30
/* Scene 4 (deck editor): module entry, the scene thread (init, per-frame loop until the editor asks
   to leave, fade out, save the deck), its exit and draw callbacks, and the setup of the lists, fonts
   and menu after the resources are loaded. */
#include "libdeck.h"
#include "menu.h"

int sceKernelChangeCurrentThreadAttr(unsigned int clear, unsigned int set);
void scene_get_transition_info(SceneTrans *out);
void scene_pop_to(unsigned short from, unsigned short arg);
int sys_state_set_callbacks(int id, int (*update)(void), int (*exit)(void));
void sys_frame_wait(void);
int bgm_get_status(int id);
void snd_cmd_fefb(void);
void bgm_fade(int x);
void bgm_cmd(int arg);
int fade_is_done(void);
void cardlist_clear(void);
void heap_free_block(void *h);
void recipe_viewer_draw(void);
void libdeck_tutorial_08804554(void);
void sysstate_blob_get(void *dst);
int cfg_get_bits_1_0_2(void);
void font_set_line_extra(int v);
void deck_name_copy_out(void *dst);

/* other parts of this module */
void dk_lists_init(void);
void dk_lists_free(void);
void dk_deck_store(void);
int func_0880931C(void *pad);
void func_0880AF34(unsigned short mode);
void dk_cardlist_setup(void);
void dk_check_rewards(void);
void dk_update(void);
void dk_res_init(void);
int dk_res_load(void);
void dk_res_free_alias(void);
void dk_draw_scene(void);
void *dk_help_draw(void *list);

extern unsigned char dk_scene_data[];  /* 0x0882427C */
extern unsigned char dk_heap_mem[];    /* 0x08824290, 0xDAE000 bytes */

void dk_main(void);
void dk_exit(void);
void dk_draw_exit(void);

int module_start(void)
{
    ehscript_register((int (*)(void))dk_main, (int (*)(void))dk_exit, dk_scene_data);
    return 0;
}

void dk_init(void)
{
    SceneTrans t;

    memset(dk_lists, 0, 0x3D4);
    memset(&dkv, 0, sizeof(dkv));
    scene_get_transition_info(&t);
    dk_scene_arg = t.arg;
    dk_heap = heap_create(dk_heap_mem, 0xDAE000);
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    if (!(bgm_get_status(6) & 1)) {
        snd_cmd_fefb();
        bgm_fade(330);
    }
    dk_res_init();
    while (!dk_res_load())
        sys_frame_wait();
    dk_setup();
}

void dk_main(void)
{
    dk_init();
    gfx_overlay_enable(0);
    sys_state_set_callbacks(4, (int (*)(void))dk_draw, (int (*)(void))dk_draw_exit);
    fade_start(1, 0xFF000000, 16);
    bgm_play(6);
    do {
        sys_frame_wait();
        dk_update();
    } while (!func_0880931C(pad_get()));
    fade_start_in(1, 0xFF000000, 16);
    if (dk_scene_arg != 2)
        bgm_cmd(600);
    while (!fade_is_done())
        sys_frame_wait();
    sys_state_set_callbacks(0, 0, 0);
    gfx_overlay_enable(1);
    sys_frame_wait();
    dk_deck_store();
    sys_frame_wait();
    scene_pop_to(4, dk_scene_arg);
}

void dk_exit(void)
{
    dk_lists_free();
    menu_free_items(&dk_menu);
    cardlist_clear();
    if (dk_cip != 0) {
        cip_close(dk_cip);
        while (!cip_is_idle(dk_cip))
            sys_frame_wait();
        dk_cip = 0;
    }
    if (dk_module > 0) {
        sceKernelStopModule(dk_module, 0, 0, 0, 0);
        sceKernelUnloadModule(dk_module);
        dk_module = 0;
    }
    heap_free_block(dk_heap);
}

void dk_draw(void)
{
    if (dkv.info) {
        cardinfo_draw();
    } else if (dkv.help) {
        dl_set_list(dk_help_draw(dl_select_layer(4, 0)));
    } else if (dkv.recipe) {
        recipe_viewer_draw();
    } else if (dkv.tutorial) {
        libdeck_tutorial_08804554();
    } else {
        dk_draw_scene();
    }
}

void dk_draw_exit(void)
{
    dk_res_free_alias();
}

void dk_setup(void)
{
    dk_lists_init();
    sysstate_blob_get(dk_profile);
    dkv.side = 1;
    dkv.pane[0].list = &dk_lists[0];
    dkv.pane[1].list = &dk_lists[3];
    func_0880AF34(cfg_get_bits_1_0_2());
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
    font_set_line_extra(0);
    menu_init(&dk_menu, dk_heap);
    dk_cardlist_setup();
    deck_name_copy_out(dk_deck_name);
    dk_check_rewards();
}
