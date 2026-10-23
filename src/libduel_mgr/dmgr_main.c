// @range 0x08804000-0x08805FF8
// @asm 0x088056C4-0x08805794
// @data 0x0881A14C-0x0881A1D4
// @rodata 0x08819D20-0x08819D90
/* Scene 6 (duel): module entry, the scene thread (load libduel_draw and the codecs, deck swap,
   tutorial, the duel itself, result screen, net teardown), its exit and draw callbacks, and the
   setup of a duel (players, decks, life points, music). */
#include "libduel_mgr.h"

/* libduel_eng */
int duel_0884D874(void);
int duel_series_result(void);
int duel_finish(void);
void duel_log_init(int mode, void *buf);
void duel_host_cb_set(void (*cb)(void));
void duel_host_args_get(int *out);
void duel_view_player_set(int p);
void duel_setup_mode_set(int v);
void duel_setup_10_set(int v);
void duel_player_type_set(int p, int t);
void duel_deck_set4(int i, void *src);
void duel_start_decks(int a);
void duel_lp_set(unsigned int p, int lp);
int duel_tag_slot(int side);
int duel_must_answer(int p);
void duel_0884CE44(void);
void duel_0884CEBC(unsigned int p);

/* libduel_draw */
void libduel_draw_08805C34(void *heap_big, void *heap);
void libduel_draw_08805CE4(void);
void libduel_draw_08805D2C(void *setup);
void libduel_draw_08805E90(void);
void libduel_draw_08805EE4(void);
int libduel_draw_08805F88(void);
void libduel_draw_0880600C(int *slots, int side);
void libduel_draw_08806030(void);
void libduel_draw_0880619C(void);
void libduel_draw_088061C0(void);
void libduel_draw_088061C8(void);
void libduel_draw_088061F4(float t);
int libduel_draw_088061FC(void);
void libduel_draw_088176A0(int cmd, int a, int b, int c);

/* libdeck_swap, libtutorial */
void libdeck_swap_08806654(void);
void libdeck_swap_088157E8(void *gs, int n, void *partners);
int libdeck_swap_088158E0(void);
void libtutorial_088042D4(int id, void (*start)(void), void (*host)(void));
void libtutorial_0880438C(void);
void libtutorial_088043D8(void);
int libtutorial_088044A4(void);
int libtutorial_0880450C(void);

/* this module */
void dmgr_menu_draw(void);
void dmgr_result_draw(void);
void dmgr_events_draw(void);
void dmgr_host_cb(void);
void dmgr_event_reset(void);
void dmgr_net_draw(void);
int dmgr_partner_sync(void);
void dmgr_menu_init(void *buf, void *heap);
int dmgr_menu_update(void);
void dmgr_menu_close(void);
void dmgr_result_init(void);
int dmgr_result_update(void);
void dmgr_event_dispatch(void);

extern unsigned char dmgr_heap_big[0x938000];
extern unsigned char dmgr_heap_mem[0x1C8000];
extern unsigned char dmgr_scene_data[];

int dmgr_draw(void);
int dmgr_state_exit(void);
int dmgr_start_read_done(void *arg, int r);
void dmgr_start_free(void);
void dmgr_start_load(void);
int dmgr_start_lp(void);
int dmgr_view_player(void);
void dmgr_sound_reset(void);
void dmgr_load_draw(void);
int dmgr_load_modules(void);
void dmgr_deck_swap(void);
void dmgr_tutorial_start_cb(void);
void dmgr_tutorial(void);
void dmgr_answer_time_reset(void);
void dmgr_voices_load(void);
void dmgr_start_duel(void);
void dmgr_fade_update(void);
void dmgr_answer_time_update(void);
int dmgr_net_lost(void);

unsigned int dmgr_mode_tab[4][2] = {
    { 0, 0 }, { 0x44CEED80, 0x44CFA3D1 }, { 0x44CF13CB, 0x44CEF7C6 }, { 0x44CFE1E6, 0x44CEEF52 },
};
DmgrBgm dmgr_bgm_sets[6] = {
    { { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0x30, 0x31, 0x32 }, { 9, 0xA, 9 } },
    { { 0x30, 0x31, 0x32 }, { 9, 0xA, 9 } },
    { { 0x33, 0x34, 0x35 }, { 9, 0xA, 9 } },
    { { 0x36, 0x37, 0x38 }, { 9, 0xB, 9 } },
    { { 0x39, 0x3A, 0x3B }, { 9, 0xB, 9 } },
};
DmgrBgm dmgr_tutorial_bgm = { { 0x25, 0x25, 0x25 }, { 0x25, 0x25, 0x25 } };

int dmgr_main(void);
int dmgr_exit(void);

int module_entry(void)
{
    ehscript_register(dmgr_main, dmgr_exit, dmgr_scene_data);
    return 0;
}

int dmgr_main(void)
{
    unsigned short from;
    PadState *pad;
    DmgrGameHdr *gs;
    DuelResults *res;
    SysSave *save;
    int loaded;
    int r;

    from = scene_stack_peek_arg(6);
    pad = pad_get();
    gs = (DmgrGameHdr *)game_state_get();
    res = game_state2_get();
    save = sysstate_get();
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    rand_seed_from_rtc();
    memset(&dmgr, 0, sizeof dmgr);
    dmgr.heap_big = heap_create(dmgr_heap_big, 0x938000);
    dmgr.heap = heap_create(dmgr_heap_mem, 0x1C8000);
    sys_state_set_callbacks(6, dmgr_draw, dmgr_state_exit);
    dmgr_sound_reset();
    switch (from) {
    case 13:
        switch (gs->kind) {
        case 1:
        case 5:
            dmgr.state = 2;
            dmgr.load_step = 0;
            dmgr.sub = 0;
            break;
        case 2:
            dmgr.state = 9;
            dmgr.sub = 0;
            dmgr.next_state = 19;
            break;
        }
        break;
    case 1:
        gfx_overlay_enable(0);
        dmgr.state = 12;
        break;
    case 2:
        switch (gs->kind) {
        case 5:
            gs->mode = dmgr_mode_tab[gs->mode][rand_next() & 1];
            gs->side = 0;
            dmgr.load_step = 0;
            dmgr.sub = 0;
            dmgr.state = 2;
            break;
        case 1:
            dmgr.load_step = 0;
            dmgr.sub = 0;
            dmgr.state = 2;
            break;
        case 4:
        case 3:
            break;
        case 2:
            dmgr.state = 9;
            dmgr.sub = 0;
            dmgr.next_state = 19;
            break;
        }
        break;
    case 15:
        break;
    }
    if (gs->kind == 5) {
        memcpy(dmgr.cfg_save, &save->cfg, 8);
        cfg_set_bits_1_3_4(2);
        cfg_set_flag(0, 1);
        cfg_set_flag(2, 0);
        sysvar_set_a8c(1);
    }
    memcpy(dmgr.gs_save, gs, 0x740);
    while (!dmgr.quit) {
        switch (dmgr.state) {
        case 2:
            gfx_overlay_enable(1);
            dmgr_voices_load();
            bgm_fade(0x151);
            switch (gs->bgm_set) {
            case 4:
            case 5:
                bgm_fade(13);
                break;
            default:
                bgm_fade(12);
                break;
            }
            if (gs->kind2 == 2 && res->n_duels != 0) {
                dmgr.state = 8;
                dmgr.sub = 0;
                dmgr.next_state = 3;
            } else {
                dmgr.state = 3;
            }
            break;
        case 3:
            dmgr_start_load();
            while (!dmgr.start_loaded) {
                sys_frame_wait();
            }
            dmgr_menu_init(dmgr.start_buf, dmgr.heap);
            dmgr.state = 4;
            break;
        case 4:
            loaded = 0;
            dmgr.load_step = 0;
            for (;;) {
                r = dmgr_menu_update();
                if (loaded && r >= 3) {
                    dmgr.fade = 7;
                    dmgr.state = 5;
                    goto next;
                }
                if (r > 0) {
                    gs->side = r - 1;
                    if (sysvar_get_a98() != 0 && netsync_local_side() == 1) {
                        gs->side ^= 1;
                    }
                }
                dmgr.busy = 0;
                if (!loaded) {
                    loaded = dmgr_load_modules();
                    if (loaded) {
                        dmgr_menu_close();
                    }
                }
                if (!dmgr.busy && dmgr_net_lost()) {
                    dmgr.state = 18;
                    goto next;
                }
                sys_frame_wait();
            }
        case 5:
            dmgr_start_free();
            while (snd_count_bgm() != 0) {
                sys_frame_wait();
            }
            snd_cmd_fefa();
            bgm_cmd3(0x151);
            dmgr_start_duel();
            break;
        case 1:
            if (dmgr.fade != 2) {
                switch (gs->kind) {
                case 1:
                    if (dmgr.answered) {
                        dmgr.fade = 5;
                    }
                    break;
                case 5: {
                    unsigned short mask;

                    switch (scene_stack_peek_arg(6)) {
                    case 13:
                        mask = 0xFFFF;
                        dmgr.timer++;
                        break;
                    case 2:
                    case 1:
                        mask = 8;
                        dmgr.timer = 0;
                        break;
                    }
                    if ((pad->b.dir_rep & mask) || dmgr.timer >= 3600 || dmgr.answered) {
                        dmgr.fade = 5;
                    }
                    break;
                }
                }
            }
            libduel_draw_088061FC();
            break;
        case 6:
            netsync_set_mode(0);
            libduel_draw_08805E90();
            if (duel_finish() != 0) {
                dmgr_result_init();
                dmgr.state = 7;
            } else {
                r = 0;
                do {
                    snd_channel_update(r);
                    r++;
                } while (r < 4);
                bgm_cmd(200);
                dmgr.state = 18;
            }
            break;
        case 7:
            switch (dmgr_result_update()) {
            case 0:
                break;
            case 1:
                if (duel_series_result() == 0) {
                    dmgr.state = 2;
                } else {
                    dmgr.state = 18;
                }
                break;
            case 2:
                res->mode = 0;
                res->result = 0;
                res->lp[1] = 0;
                res->lp[0] = 0;
                res->n_duels = 0;
                res->duels[2] = 0;
                res->duels[1] = 0;
                res->duels[0] = 0;
                memcpy(gs, dmgr.gs_save, 0x740);
                dmgr.state = 2;
                break;
            }
            break;
        case 8:
            dmgr_deck_swap();
            break;
        case 9:
            dmgr_load_draw();
            while (!dmgr.draw_loaded) {
                if (modmgr_none_loading() != 0) {
                    libduel_draw_08805C34(dmgr.heap_big, dmgr.heap);
                    dmgr.draw_loaded = 1;
                }
                sys_frame_wait();
            }
            if (!dmgr.mod8_loaded) {
                modmgr_load_by_id(8);
                while (!dmgr.mod8_loaded) {
                    dmgr.mod8_loaded = modmgr_none_loading();
                    sys_frame_wait();
                }
            }
            dmgr.state = 10;
            break;
        case 10:
            dmgr_tutorial();
            break;
        case 18:
            if (sysvar_get_a98() != 0) {
                if (net_get_state() == 0x1F) {
                    net_set_state(0x21);
                    break;
                }
                if (net_get_state() != -1) {
                    break;
                }
                netsync_set_mode(0);
            }
            dmgr.state = 19;
            break;
        case 19:
            dmgr.quit = 1;
            gfx_overlay_enable(1);
            break;
        }
    next:
        if (dmgr.state < 18 && dmgr_net_lost()) {
            dmgr.state = 18;
        }
        sys_frame_wait();
    }
    sys_state_set_callbacks(0, 0, 0);
    sys_frame_wait();
    if (scene_stack_peek_arg(6) == 2 && gs->rematch == 1 && duel_series_result() == 2) {
        scene_reset_to(6, 13, 1);
    } else {
        scene_pop_to(6, 0);
    }
}

int dmgr_exit(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    SysSave *save = sysstate_get();

    if (dmgr.swap_loaded) {
        modmgr_update(10);
    }
    if (dmgr.codecs_loaded) {
        duel_modules_unload();
    }
    if (dmgr.mod8_loaded) {
        modmgr_update(8);
    }
    if (dmgr.draw_loaded) {
        libduel_draw_08805CE4();
        modmgr_update(9);
    }
    if (sysvar_get_a98() != 0) {
        net_adhoc_stop();
    }
    if (gs->kind == 5) {
        memcpy(&save->cfg, dmgr.cfg_save, 8);
    }
    sysvar_set_a8c(0);
    snd_mute_off();
}

int dmgr_draw(void)
{
    unsigned int size;
    unsigned int *list;
    int n;

    dl_set_cur_list_base(list = dl_select_layer(0, &size));
    sceGuStart(2, list, size);
    gu_set_color_mask(0);
    gu_set_param_50(0);
    gu_set_param_4c(0);
    gu_set_param_494(0x17);
    n = (int)dl_get_cur_list_base();
    dl_set_list((unsigned int *)(n + sceGuFinish() - 8));
    font_set_color(0xFFFFFFFF);
    dmgr.frame++;
    switch (dmgr.state) {
    case 1:
        dmgr.answered = duel_0884D874();
        req_flags_clear();
        libduel_draw_088061C0();
        libduel_draw_088061C8();
        libduel_draw_088061F4(1.0f / 30.0f);
        dmgr_events_draw();
        dmgr_answer_time_update();
        break;
    case 10:
        if (dmgr.sub == 1) {
            libtutorial_088043D8();
            duel_0884D874();
            req_flags_clear();
            libduel_draw_088061C0();
            libduel_draw_088061C8();
            libduel_draw_088061F4(1.0f / 30.0f);
            dmgr_events_draw();
        }
        break;
    case 4:
        dmgr_menu_draw();
        break;
    case 7:
        dmgr_result_draw();
        break;
    case 8:
        if (dmgr.sub == 2) {
            libdeck_swap_08806654();
        }
        break;
    case 0:
    case 2:
    case 3:
    case 5:
    case 6:
    case 9:
    case 11:
    case 18:
    case 19:
        break;
    default:
        dmgr.state = 19;
        break;
    }
    dmgr_fade_update();
}

int dmgr_state_exit(void)
{
}

int dmgr_start_read_done(void *arg, int r)
{
    dmgr.start_loaded = 1;
    return 1;
}

void dmgr_start_free(void)
{
    if (dmgr.start_buf) {
        heap_set_level(dmgr.heap, dmgr.start_buf);
        dmgr.start_buf = 0;
    }
    dmgr.start_loaded = 0;
}

void dmgr_start_load(void)
{
    game_state_get();
    dmgr_start_free();
    dmgr.start_buf = heap_alloc(dmgr.heap, ehfile_get_size(4, "start/start_all.ehp"));
    ehfile_read_async_alias(4, "start/start_all.ehp", dmgr.start_buf, 0x1C8000, dmgr_start_read_done,
                            dmgr.start_buf);
}

int dmgr_start_lp(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    int lp;

    switch (gs->lp_level) {
    case 1:
        lp = 2000;
        break;
    case 2:
        lp = 4000;
        break;
    case 3:
        lp = 12000;
        break;
    case 4:
        lp = 16000;
        break;
    default:
        lp = 8000;
        break;
    }
    return lp;
}

int dmgr_view_player(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    int i;
    unsigned int sides = 0;
    int n = 2;

    if (gs->partner_mode == 2) {
        n = 4;
    }
    for (i = 0; i < n; i++) {
        if (gs->slot[i].type == 1) {
            switch (i) {
            case 0:
            case 2:
                sides |= 1;
                break;
            case 1:
            case 3:
                sides |= 2;
                break;
            }
        }
    }
    return sides == 2;
}

void dmgr_sound_reset(void)
{
    int n;

    memset(game_state2_get(), 0, 0x2A0);
    snd_cmd_fefb_0();
    n = snd_count_bgm();
    while (n + snd_count_se() != 0) {
        sys_frame_wait();
        n = snd_count_bgm();
    }
    snd_cmd_fefb();
    bgm_fade(0x14A);
    bgm_fade(0x14C);
}

void dmgr_load_draw(void)
{
    int argp;
    int status;

    if (dmgr.draw_loaded == 0) {
        dmgr.mod_args.size = 0x14;
        dmgr.mod_args.a = 0;
        dmgr.mod_args.b = 0x4000;
        dmgr.mod_args.c = 0;
        dmgr.mod_args.d = 0;
        argp = 0;
        modmgr_load_ex(9, 4, &argp, &status, &dmgr.mod_args);
    }
}

int dmgr_load_modules(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrDrawSetup setup;
    PlayerSlot *s;
    int i;

    switch (dmgr.load_step) {
    case 0:
        dmgr_load_draw();
        dmgr.load_step++;
        break;
    case 1:
        if (dmgr.draw_loaded) {
            dmgr.load_step++;
        } else if (modmgr_none_loading() != 0) {
            libduel_draw_08805C34(dmgr.heap_big, dmgr.heap);
            dmgr.draw_loaded = 1;
        } else {
            dmgr.busy = 1;
        }
        break;
    case 2:
        if (!dmgr.mod8_loaded) {
            modmgr_load_by_id(8);
        }
        dmgr.load_step++;
        break;
    case 3:
        if (dmgr.mod8_loaded) {
            dmgr.load_step++;
        } else if (modmgr_none_loading() != 0) {
            dmgr.mod8_loaded = 1;
        } else {
            dmgr.busy = 1;
        }
        break;
    case 4:
        if (!dmgr.codecs_loaded) {
            duel_modules_load(0, 1);
        }
        dmgr.load_step++;
        break;
    case 5:
        if (dmgr.codecs_loaded) {
            dmgr.load_step++;
        } else if (modmgr_none_loading_alias() != 0) {
            dmgr.codecs_loaded = 1;
        } else {
            dmgr.busy = 1;
        }
        break;
    case 6:
        if (sysvar_get_a98() != 0) {
            if (dmgr_partner_sync() == 0) {
                break;
            }
        } else {
            for (i = 0; i < 4; i++) {
                s = &gs->slot[i];
                if (s->type == 1) {
                    memcpy(SLOT_RECIPE(s), gs, 0x108);
                }
            }
        }
        memset(&setup, 0, sizeof setup);
        switch (gs->partner_mode) {
        case 1:
            setup.n = 2;
            break;
        case 2:
            setup.n = 4;
            break;
        default:
            dmgr.state = 19;
            return 0;
        }
        for (i = 0; i < setup.n; i++) {
            setup.type[i] = gs->slot[i].type;
            setup.type2[i] = gs->slot[i].type2;
            setup.deck[i] = SLOT_RECIPE(&gs->slot[i]);
        }
        switch (gs->partner_mode) {
        case 1:
            setup.order[1] = 1;
            setup.order[0] = 0;
            break;
        case 2:
            if (gs->side != 0) {
                setup.order[0] = 2;
                setup.order[1] = 1;
            } else {
                setup.order[0] = 0;
                setup.order[1] = 3;
            }
            break;
        }
        libduel_draw_08805D2C(&setup);
        libduel_draw_08805EE4();
        dmgr.load_step++;
        break;
    case 7:
        if (libduel_draw_08805F88() != 0) {
            return 1;
        }
        break;
    }
    return 0;
}

void dmgr_deck_swap(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();

    dmgr.busy = 0;
    switch (dmgr.sub) {
    case 0:
        if (sysvar_get_a98() != 0) {
            local_record_set0(5);
        }
        gfx_overlay_enable(1);
        libduel_draw_0880619C();
        modmgr_load_by_id(10);
        dmgr.sub++;
        break;
    case 1:
        if (dmgr.swap_loaded) {
            libdeck_swap_088157E8(gs, partner_count_get(), gs->extra);
            dmgr.sub++;
        } else if (modmgr_none_loading() != 0) {
            dmgr.swap_loaded = 1;
        } else {
            dmgr.busy = 1;
        }
        break;
    case 2:
        if (libdeck_swap_088158E0() == 1) {
            dmgr.sub++;
        }
        break;
    case 3:
        modmgr_update(10);
        gfx_overlay_enable(1);
        dmgr.swap_loaded = 0;
        dmgr.state = dmgr.next_state;
        break;
    }
}

void dmgr_tutorial_start_cb(void)
{
    dmgr_event_reset();
    dmgr_answer_time_reset();
}

void dmgr_tutorial(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrSeq *q = &dmgr.seq;
    int done;

    switch (dmgr.sub) {
    case 0:
        gfx_overlay_enable(1);
        sys_frame_wait();
        modmgr_load_by_id(11);
        while (!modmgr_none_loading()) {
            sys_frame_wait();
        }
        libtutorial_088042D4(gs->tutorial, dmgr_tutorial_start_cb, dmgr_host_cb);
        for (done = 0; done < 3; done++) {
            q->bgm.a[done] = dmgr_tutorial_bgm.a[done];
            q->bgm.b[done] = dmgr_tutorial_bgm.b[done];
        }
        dmgr.sub++;
        break;
    case 1:
        done = 0;
        if (libtutorial_0880450C() == 0) {
            done = libduel_draw_088061FC();
        }
        if (done | libtutorial_088044A4()) {
            dmgr.sub++;
        }
        break;
    case 2:
        libtutorial_0880438C();
        modmgr_update(11);
        gfx_overlay_enable(1);
        dmgr.sub = 0;
        dmgr.state = dmgr.next_state;
        break;
    }
}

void dmgr_answer_time_reset(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    int *t = game_state2_get();
    int n;

    if (gs->kind == 2) {
        n = 0;
    } else {
        switch (gs->time_level) {
        case 1:
            n = 900;
            break;
        case 2:
            n = 1800;
            break;
        case 3:
            n = 5400;
            break;
        case 4:
            n = 9000;
            break;
        default:
            n = 0;
            break;
        }
    }
    n--;
    t[3] = n;
    t[2] = n;
    t[1] = n;
    t[0] = n;
}

#ifdef NON_MATCHING /* o original testa o modo antes de zerar o contador do segundo laço (bne + nop); o mwcc põe o zero no atraso de um bnel */
void dmgr_voices_load(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    int i;
    int v;
    int j;
    int n;

    snd_cmd_fefa();
    for (i = 0; i < 4; i++) {
        PlayerSlot *s = &gs->slot[i];
        if (s->voice_obj != 0) {
            bgm_cmd3(s->voice_obj);
            s->voice_obj = 0;
        }
    }
    n = 2;
    if (gs->partner_mode == 2) {
        n = 4;
    }
    for (j = 0; j < n; j++) {
        PlayerSlot *s = &gs->slot[j];
        if (s->type < 2) {
            s->voice_obj = 0;
        } else {
            s->voice_obj = v = partner_voice_id(s->type, s->type2);
            if (v != 0) {
                bgm_fade(v);
            }
        }
    }
}
#endif

void dmgr_start_duel(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrSeq *q = &dmgr.seq;
    int order[2];
    int i;

    memset(q, 0, sizeof *q);
    for (i = 0; i < 3; i++) {
        q->bgm.a[i] = dmgr_bgm_sets[gs->bgm_set].a[i];
        q->bgm.b[i] = dmgr_bgm_sets[gs->bgm_set].b[i];
    }
    bgm_fade(q->bgm.a[0]);
    gfx_overlay_enable(0);
    duel_log_init(0, 0);
    duel_host_cb_set(dmgr_host_cb);
    duel_view_player_set(2);
    duel_setup_mode_set(gs->mode);
    duel_setup_10_set(gs->side);
    if (sysvar_get_a98() != 0) {
        netsync_init();
        netsync_set_mode(1);
        i = netsync_local_side();
        if (i >= 2) {
            duel_view_player_set(0);
        } else {
            duel_view_player_set(i);
        }
    } else {
        duel_view_player_set(dmgr_view_player());
    }
    switch (gs->partner_mode) {
    case 1:
        order[1] = 1;
        order[0] = 0;
        break;
    case 2:
        if (gs->side != 0) {
            order[0] = 2;
            order[1] = 1;
        } else {
            order[0] = 0;
            order[1] = 3;
        }
        break;
    }
    for (i = 0; i < 2; i++) {
        switch (gs->slot[order[i]].type) {
        case 0:
            duel_player_type_set(i, 2);
            break;
        case 1:
            duel_player_type_set(i, 0);
            break;
        default:
            if (sysvar_get_a98() != 0 && i != netsync_local_side()) {
                duel_player_type_set(i, 2);
            } else {
                duel_player_type_set(i, 1);
            }
            break;
        }
    }
    libduel_draw_0880600C(order, gs->side);
    libduel_draw_08806030();
    switch (gs->partner_mode) {
    case 1:
        for (i = 0; i < 2; i++) {
            duel_deck_set4(i, SLOT_RECIPE(&gs->slot[i]));
        }
        duel_start_decks(0);
        break;
    case 2:
        if (gs->side != 0) {
            duel_deck_set4(2, SLOT_RECIPE(&gs->slot[0]));
            duel_deck_set4(0, SLOT_RECIPE(&gs->slot[2]));
            duel_deck_set4(1, SLOT_RECIPE(&gs->slot[1]));
            duel_deck_set4(3, SLOT_RECIPE(&gs->slot[3]));
        } else {
            duel_deck_set4(0, SLOT_RECIPE(&gs->slot[0]));
            duel_deck_set4(2, SLOT_RECIPE(&gs->slot[2]));
            duel_deck_set4(3, SLOT_RECIPE(&gs->slot[1]));
            duel_deck_set4(1, SLOT_RECIPE(&gs->slot[3]));
        }
        duel_start_decks(1);
        break;
    default:
        return;
    }
    duel_lp_set(0, dmgr_start_lp());
    duel_lp_set(1, dmgr_start_lp());
    libduel_draw_088176A0(13, 0, dmgr_start_lp(), 0);
    libduel_draw_088176A0(13, 1, dmgr_start_lp(), 0);
    dmgr_event_reset();
    req_flags_clear();
    dmgr.answered = 0;
    dmgr.timer = 0;
    dmgr.fade = 6;
    dmgr.state = 11;
    dmgr_answer_time_reset();
}

void dmgr_host_cb(void)
{
    int args[4];

    duel_host_args_get(args);
    dmgr_event_dispatch();
    if (args[0] != 1) {
        libduel_draw_088176A0(args[0], args[1], args[2], args[3]);
    }
}

void dmgr_fade_update(void)
{
    unsigned int *list;

    game_state2_get();
    if (dmgr.fade != 0) {
        switch (dmgr.fade) {
        case 5:
            fade_start_in(1, 0xFF000000, 0x40);
            dmgr.fade = 2;
            return;
        case 6:
            fade_start(1, 0xFFFFFFFF, 0x10);
            dmgr.fade = 3;
            dmgr.state = 1;
            break;
        case 7:
            fade_start(1, 0xFFFFFFFF, 0x10);
            break;
        }
        list = dl_select_layer(9, 0);
        list = dl_disable(list, 1);
        if (fade_apply(&list) != 0) {
            switch (dmgr.fade) {
            case 1:
                dmgr.fade = 0;
                break;
            case 2:
                dmgr.fade = 0;
                dmgr.state = 6;
                break;
            case 3:
                dmgr.fade = 0;
                break;
            case 4:
                fade_start(1, 0xFFFFFFFF, 0x10);
                dmgr.fade = 3;
                dmgr.state = 1;
                break;
            }
        }
        dl_set_list(list);
    }
}

void dmgr_answer_time_update(void)
{
    int *t = game_state2_get();
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    int i;
    int k;
    int id;
    int *p;

    if (sysvar_get_a98() != 0) {
        for (i = 0; i < 2; i++) {
            k = duel_tag_slot(i);
            id = gs->slot[k].net_id;
            if (id == peer_self_get()) {
                if (duel_must_answer(i) != 0) {
                    p = &t[k];
                    if (*p > 0) {
                        (*p)--;
                    }
                    if (*p == 0) {
                        duel_0884CE44();
                        switch (k) {
                        case 0:
                        case 2:
                            duel_0884CEBC(0);
                            break;
                        case 1:
                        case 3:
                            duel_0884CEBC(1);
                            break;
                        }
                    }
                }
                local_record_set_u32(t[k]);
            } else {
                t[k] = peer_self_u32(id);
            }
        }
    } else {
        for (i = 0; i < 2; i++) {
            k = duel_tag_slot(i);
            if (gs->slot[k].type < 2 && duel_must_answer(i) != 0) {
                if (t[k] > 0) {
                    t[k]--;
                }
            }
        }
        for (i = 0; i < 4; i++, t++) {
            if (*t == 0) {
                duel_0884CE44();
                switch (i) {
                case 0:
                case 2:
                    duel_0884CEBC(0);
                    break;
                case 1:
                case 3:
                    duel_0884CEBC(1);
                    break;
                }
                return;
            }
        }
    }
}

int dmgr_net_lost(void)
{
    if (sysvar_get_a98() == 0) {
        return 0;
    }
    switch (net_get_state()) {
    case -1:
    case 10:
        snd_cmd_fefb_0();
        return !ehfile_busy();
    }
    return 0;
}
