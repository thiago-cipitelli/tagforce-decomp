// @range 0x08804000-0x088046B4
// @data 0x0880F6A0-0x0880F6E0
/* Entry points of the tutorial duel (init / per-frame / term, exported to libduel_mgr): the step
   state machine (script, message window, waits for the player's action), the quit box opened with
   START and the game options that the lesson overrides and restores. */
#include "libtutorial.h"

/* 0x08804000: one frame of the lesson; called from tut_frame */
void tut_update(void)
{
    PadState *pad = pad_get();
    Tut *t;
    int r;

    region_pick_value(0);
    t = &tut;
    if (tut.quit_open) {
        r = menu_update(&tut.quit);
        tut.show_windows = 0;
        if (r == -1)
            return;
        if (r == 2) {
            tut.state = 6;
            fade_start_in(1, 0xFF000000, 16);
        } else {
            tut.quit_open = 0;
            tut_set_lock(tut.saved_lock);
        }
        menu_reset(&tut.quit);
        return;
    }
    if (pad->b.down & 0x100)
        tut.show_windows = 0;
    else
        tut.show_windows = 1;
    if ((pad->b.trig & 8) && libduel_draw_088062E4() == 0) {
        tut_pause_open();
        return;
    }
    if (tut.wait > 0) {
        tut.wait--;
        return;
    }
    tut.prev_state = tut.state;
    switch (tut.state) {
    case 0:
        if (tut_script_done()) {
            fade_start_in(1, 0xFF000000, 16);
            tut.state = 6;
        }
        break;
    case 1:
        if (tut_window_busy(tut.window) == 0)
            tut.state = 0;
        break;
    case 2:
        if (tut_action_done()) {
            tut_duel_resume();
            tut.wait = 10;
            tut.state = 3;
        }
        break;
    case 3:
        if (tut_script_busy() == 0)
            tut.state = 0;
        break;
    case 4:
        if (tut_script_busy() == 0)
            tut.state = 0;
        break;
    case 5:
        if (tut_event_done())
            tut.state = 0;
        break;
    case 6:
        if (fade_is_done())
            tut.state = 7;
        break;
    case 7:
        break;
    }
    if (t->frame != -1)
        t->frame++;
    else
        t->frame = 0;
}

int module_start(void)
{
    return 0;
}

/* 0x088042D4: export; start (deck/field setup) and host (duel event sink) come from libduel_mgr */
void tut_init(int lesson, void (*start)(void), void (*host)(void))
{
    memset(&tut, 0, sizeof(Tut));
    tut.lesson = lesson;
    tut.state = 0;
    tut_heap_init_thunk();
    tut_arrows_clear();
    tut_res_load(tut.lesson);
    tut_sprites_init();
    tut_windows_init();
    tut_cfg_save();
    tut_script_init(tut.lesson, start, host);
    bgm_play(0x25);
    fade_start(1, 0xFF000000, 16);
}

/* 0x0880438C: export */
void tut_term(void)
{
    bgm_cmd(600);
    tut_duel_draw_term();
    tut_cfg_restore();
    tut_windows_term();
    tut_sprites_term_thunk();
    tut_res_free();
    tut_heap_term_thunk();
}

/* 0x088043D8: export; per frame: script, step machine and the overlays */
void tut_frame(void)
{
    unsigned int *list;
    unsigned int *fl;

    tut_script_update();
    tut_update();
    if (libduel_draw_088062E4() == 0) {
        tut_sprites_draw();
        if (tut.show_windows)
            tut_windows_draw();
        if (tut.quit_open) {
            list = dl_select_layer(8, 0);
            menu_draw((void **)&list, &tut.quit);
            dl_set_list(list);
        }
    }
    fl = dl_select_layer(9, 0);
    fade_apply((void **)&fl);
    dl_set_list(fl);
    if (tut_is_busy())
        req_flag_set(0x3E);
}

/* 0x088044A4: export */
int tut_is_finished(void)
{
    return tut.state == 7;
}

/* 0x088044B8: export; script command from the duel engine */
void tut_command(int player, int cmd, int a, int b)
{
    tut_cmd_dispatch(player, cmd, a, b);
}

/* 0x088044C8: export; nonzero while a message window is open */
int tut_is_busy(void)
{
    int r;

    switch (tut.state) {
    case 0:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        r = 0;
        break;
    case 1:
    default:
        r = 1;
    }
    return r;
}

/* 0x0880450C: export */
int tut_get_lock(void)
{
    return tut.lock;
}

void tut_set_state(int s)
{
    tut.state = s;
}

int tut_get_state(void)
{
    return tut.state;
}

void tut_wait_window(int slot)
{
    tut.window = slot;
    tut.state = 1;
}

void tut_set_lock(int on)
{
    tut.lock = on;
}

/* 0x08804554: the lesson runs with fixed options; the player's are kept in tut */
void tut_cfg_save(void)
{
    tut.saved_cfg134 = cfg_get_bits_1_3_4();
    tut.saved_cfg0 = cfg_get_flag(0);
    tut.saved_cfg2 = cfg_get_flag(2);
    tut.saved_cfg3 = cfg_get_flag(3);
    cfg_set_bits_1_3_4(1);
    cfg_set_flag(0, 0);
    cfg_set_flag(2, 0);
    cfg_set_flag(3, 0);
}

void tut_cfg_restore(void)
{
    cfg_set_bits_1_3_4(tut.saved_cfg134);
    cfg_set_flag(0, tut.saved_cfg0);
    cfg_set_flag(2, tut.saved_cfg2);
    cfg_set_flag(3, tut.saved_cfg3);
}
