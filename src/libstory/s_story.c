// @range 0x0880C9E4-0x0880DE58
// @asm 0x0880D9A0-0x0880DE58
// @rodata 0x0881C29C-0x0881C3F0
/* The story mode (story_ctx): start (save areas handed to every subsystem, new game per story mode
   or resume after a sub-scene: duel, lesson, shop...), the per-frame dispatch of the story states
   (field, sub-screen modules, staff roll, ending save), the scene left at the end, and the first
   field setup commands (date/time advance, room info, room objects, player position). */
#include "libstory.h"
#include "duel_setup.h"

typedef struct StFldInfo {             /* argument of field command 0: the story data the PDA shows */
    int *date[6];                      /* +0x00 scal_get_ptrs */
    int *items;                        /* +0x18 */
    int *cards;                        /* +0x1C */
    int *x2c;                          /* +0x20 */
    int night;                         /* +0x24 */
    void (*item_take)();               /* +0x28 */
    void (*card_add)();                /* +0x2C */
    int (*period_of)(int cal_month, int day); /* +0x30 */
    void *(*mail_list)();              /* +0x34 */
    void (*mail_open)();               /* +0x38 */
    int (*points)();                   /* +0x3C */
} StFldInfo;

typedef struct StFldObjs {             /* argument of field command 3: objects of the room */
    int bg;                            /* +0x00 */
    unsigned short *list;              /* +0x04 pairs */
    int n;                             /* +0x08 */
} StFldObjs;

typedef struct StFldPos {              /* argument of field command 10: player position */
    float x;
    float z;
    int dir;
} StFldPos;

void *sysstate_get_ptr_97f0(void);
void *game_state2_get(void);
void deck_cur_get_u16_0(short *out);
void deck_cur_set_bit0(void);
void sysstate_max_byte_97d0(int i, int v);
void sysflag_set(unsigned int bit);
void scene_push(int from, int scene, int arg);
void scene_pop_to(int from, int arg);

void st_chr_attach(void *save, void *flags);
void st_mail_set_list(void *p);
void st_req_set_save(void *p);
void vm_set_slot(void *p);
void st_pl_attach(void *p, void *bits);
void st_sv_set_ptr(void *p);
void sev_set_done(void *p);
void st_pickup_set_flags(void *p);
void st_duel_set_params(void *p);
void st_free_set_state(void *p);
void st_ev2_attach(void *p);
void st_bgm_attach(void *p);
int story_mode_set(int mode);
int story_mode_get(void);
int story_mode_is1(void);
int st_msg_open(int mode);
int st_msg_load_all(void);
void st_msg_load_iso(int chapter);
void st_msg_load_chr(int deck);
void st_msg_unload(int kind);
int st_req_exit(void);
int st_req_c(void);
void st_req_clear_save(void);
void st_req_set_x10(int v);
void st_req_set_night(int v);
void st_req_set_exit(int v);
void st_req_set_c(int v);
void st_req_set_8(int v);
int st_req_get_8(void);
int st_req_is_night(void);
void st_snd_stop_all(void);
void st_bgm_fade(void);
void st_bgm_clear(void);
void st_ev2_clear(void);
int st_ev2_today(void);
void st_ev2_set_4(int v);
int st_sv_alt(void);
int st_sv_chapter(void);
int st_sv_get_0c(void);
void st_sv_clear(void);
void sev_clear_done(void);
void st_field_open(void);
void st_field_open_empty(void);
void st_field_room_map(void);
void st_field_fade_out(void);
void st_field_walk(void);
void st_field_walk_poll(void);
void st_field_reopen(void);
void st_vm_menu(void);
void st_mod_switch(int id, int load);
void st_staffroll_start(void);
void st_staffroll_play(void);
void st_ending_save(void);
void st_ending_save_poll(void);
void st_ending_save_done(void);
void st_draw_hook(void);
void st_fade(int out, int wait, int frames);
int st_duel_param0(void);
int st_duel_param1(void);
int st_duel_param2(void);
int st_duel_param3(void);
int st_duel_param4(void);
void st_duel_clear(void);
void st_duel_clear_params(void);
void st_free_clear_state(void);
int st_free_state0(void);
int st_free_state1(void);
void st_free_state1_set(int v);
void st_event_start(int kind, int a, int b);
void st_chr_reset(int i);
void st_chr_affinity_set(int i, int v);
void st_chr_affinity_clear_all(void);
void st_chr_named_set(int i);
void st_chr_away_set(int i, int on);
void st_chr_flag_set(int i, unsigned int bit, int on);
void st_chr_update_all(int same_day);
void st_mail_clear(int i);
void vm_clear_slot(void);
void st_pl_clear(void);
void st_pl_room_set(int room);
int st_pl_room_get(void);
int st_pl_room_prev_get(void);
void st_pl_x14_set(int v);
int st_pl_x14_get(void);
void st_pl_x18_set(int v);
void st_pl_x150_set(int v);
void st_pl_x154_set(int v);
void st_pl_x156_set(int v);
void st_pl_x158_set(int v);
int *st_pl_items_get(void);
int *st_pl_cards_get(void);
int *st_pl_x2c_ptr(void);
float st_pl_x_get(void);
float st_pl_z_get(void);
int st_pl_dir_get(void);
void st_pl_pos_set(float x, float y, int z);
void st_pl_card_add();
int st_pl_points();
void *st_mail_view_list();
void st_mail_open();
void st_fld_item_take();
int st_id_to_deck(int id);
int st_room_to_bg(int room);

void scal_start(int mode);
void scal_clear(void);
void scal_bind(ScalDate *date, int *state, unsigned int *flags);
void scal_flag_set(unsigned int id, int on);
void scal_flag2_set(unsigned int id, int on);
int scal_flag2_get(unsigned int id);
void scal_tick_next(void);
void scal_tick_add(int n);
void scal_set_slot(int slot);
void scal_set_day(int day);
void scal_set_month(int month);
void scal_next_day(void);
void scal_next_month(void);
int scal_day_changed(void);
int scal_after_m1_10(void);
void scal_get_ptrs(int **out);
int scal_period_of(int cal_month, int day);
int stt_branch_x(void);

void story_quit(int fade);
void st_fld_setup(int kind, int arg);

void st_story_init(void)
{
    memset(&story_ctx, 0, sizeof(story_ctx));
}

void st_story_term(void)
{
    memset(&story_ctx, 0, sizeof(story_ctx));
}

/* hands the save areas to the subsystems, then a new game or the return from a sub-scene */
int story_start(void)
{
    unsigned char *sv = sysstate_get_ptr_97f0();
    StoryCtx *c;
    DeckCur f;
    int i;
    DuelResults *r;
    int arg;
    int on;

    st_chr_attach(sv, sv + 0x5D80);
    st_mail_set_list(sv + 0x5910);
    st_req_set_save(sv + 0x5B68);
    vm_set_slot(sv + 0x5B78);
    st_pl_attach(sv + 0x5B7C, sv + 0x5D78);
    scal_bind((ScalDate *)(sv + 0x5CD8), (int *)(sv + 0x5CFC), (unsigned int *)(sv + 0x64A8));
    st_sv_set_ptr(sv + 0x5D08);
    sev_set_done(sv + 0x5D20);
    st_pickup_set_flags(sv + 0x5D70);
    st_duel_set_params(sv + 0x5D30);
    st_free_set_state(sv + 0x5D50);
    st_ev2_attach(sv + 0x5D5C);
    st_bgm_attach(sv + 0x5D64);
    c = &story_ctx;
    deck_cur_get_u16_0((short *)&f);
    if (story_mode_set(f.which) == 0)
        deck_cur_get_u16_0((short *)&f);
    if (st_msg_open(story_mode_get()) == 0)
        return 0;
    if (st_msg_load_all() == 0)
        return 0;
    if (st_req_exit() != 3) {
        st_snd_stop_all();
        st_bgm_fade();
    }
    c->prev = 0xC;
    st_req_set_x10(0);
    if (f.bit0) {
        /* back from a sub-scene */
        st_req_set_night(1);
        st_fld_setup(1, 0);
        if (st_sv_alt())
            st_msg_load_iso(st_sv_chapter());
        switch (st_req_exit()) {
        case 0:
        case 1:
        default:
            st_field_open();
            sys_frame_wait();
            c->state = 5;
            break;
        case 2:
            st_field_open();
            sys_frame_wait();
            c->state = 5;
            break;
        case 3:
            c->state = 0x22;
            break;
        case 4:
        case 5:
            st_field_open();
            sys_frame_wait();
            r = game_state2_get();
            switch (r->result) {
            case 1:
                arg = st_duel_param2();
                break;
            case 2:
                arg = st_duel_param3();
                break;
            case 0:
            case 3:
            default:
                arg = st_duel_param4();
                break;
            }
            st_pl_x158_set(r->mode);
            st_pl_x154_set(r->lp[0]);
            st_pl_x156_set(r->lp[1]);
            st_msg_load_chr(st_duel_param0());
            st_event_start(6, -1, arg);
            st_duel_clear();
            c->state = 0xB;
            break;
        case 6:
            st_field_open();
            sys_frame_wait();
            st_event_start(0xD, -1, -1);
            st_duel_clear();
            c->state = 0xB;
            break;
        case 7:
            st_duel_clear();
            c->state = 0x25;
            break;
        case 8:
            st_field_open();
            sys_frame_wait();
            st_duel_param1();
            r = game_state2_get();
            switch (r->result) {
            case 1:
                arg = st_duel_param2();
                break;
            case 0:
            case 3:
            case 2:
            default:
                arg = st_duel_param3();
                break;
            }
            st_pl_x158_set(r->mode);
            st_msg_load_chr(st_duel_param0());
            st_event_start(7, -1, arg);
            st_duel_clear();
            c->state = 0xB;
            break;
        case 9:
            c->state = 1;
            break;
        }
        st_req_set_exit(0);
        return 1;
    }
    /* new game */
    for (i = 0; i < 150; i++) {
        st_chr_reset(i);
        st_chr_affinity_set(i, 0);
    }
    for (i = 0; i < 100; i++)
        st_mail_clear(i);
    st_req_clear_save();
    vm_clear_slot();
    st_pl_clear();
    scal_clear();
    st_sv_clear();
    sev_clear_done();
    st_duel_clear_params();
    st_free_clear_state();
    st_ev2_clear();
    st_bgm_clear();
    st_chr_affinity_clear_all();
    st_pl_x18_set(-1);
    switch (story_mode_get()) {
    case 1:
    default:
        st_pl_room_set(5);
        scal_flag2_set(2, 1);
        scal_start(1);
        st_pl_x14_set(-1);
        break;
    case 2:
        st_pl_room_set(0xE);
        scal_start(2);
        i = st_id_to_deck(f.val);
        st_pl_x14_set(i);
        st_chr_away_set(i, 1);
        st_chr_flag_set(i, 1, 1);
        break;
    case 3:
        st_pl_room_set(5);
        scal_flag2_set(2, 1);
        scal_start(3);
        i = st_id_to_deck(f.val);
        st_pl_x14_set(i);
        st_chr_away_set(i, 1);
        scal_flag_set(0x34, 1);
        st_chr_flag_set(i, 1, 1);
        break;
    case 4:
        scal_start(4);
        st_pl_x14_set(-1);
        st_pl_room_set(6);
        for (i = 0; i < 150; i++)
            st_chr_named_set(i);
        break;
    }
    c->state = 4;
    scal_flag_set(0x35, 1);
    on = sysstate_get()->play_counter & 1;
    switch (st_pl_x14_get()) {
    case 0x1C:
    case 0x1D:
        on = 1;
        break;
    }
    scal_flag_set(0x15, on);
    deck_cur_set_bit0();
    st_req_set_c(0);
    st_fld_setup(1, 0);
    return 1;
}

/* scene entered when the story ends */
void story_leave(void)
{
    switch (st_req_exit()) {
    case 0:
    case 1:
    case 9:
    default:
        scene_pop_to(2, st_req_c());
        break;
    case 2:
        scene_push(2, 4, 1);
        break;
    case 3:
        scene_push(2, 4, 2);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        scene_push(2, 6, 0);
        break;
    }
}

/* one frame of the story: 8..0x2D load (odd) and unload (even) the sub-screen modules */
void story_update(void)
{
    StoryCtx *c = &story_ctx;

    switch (story_ctx.state) {
    case 1:
        st_field_open_empty();
        return;
    case 2:
        st_field_room_map();
        break;
    case 3:
        st_field_fade_out();
        break;
    case 4:
        st_field_open();
        break;
    case 5:
        st_field_walk();
        break;
    case 6:
        st_field_walk_poll();
        break;
    case 7:
        st_field_reopen();
        break;
    case 8:
        st_mod_switch(0, 1);
        break;
    case 9:
        break;
    case 10:
        st_mod_switch(0, 0);
        break;
    case 0xB:
        st_vm_menu();
        break;
    case 0xC:
        story_quit(1);
        break;
    case 0xD:
        st_mod_switch(1, 1);
        c->state = 0xE;
        break;
    case 0xE:
        break;
    case 0xF:
        st_mod_switch(1, 0);
        c->state = 4;
        break;
    case 0x10:
        st_mod_switch(2, 1);
        c->state = 0x11;
        break;
    case 0x11:
        break;
    case 0x12:
        st_mod_switch(2, 0);
        c->state = 4;
        break;
    case 0x13:
        st_mod_switch(3, 1);
        c->state = 0x14;
        break;
    case 0x14:
        break;
    case 0x15:
        st_mod_switch(3, 0);
        c->state = 4;
        break;
    case 0x16:
        st_mod_switch(4, 1);
        c->state = 0x17;
        break;
    case 0x17:
        break;
    case 0x18:
        st_mod_switch(4, 0);
        c->state = 4;
        break;
    case 0x19:
        st_mod_switch(5, 1);
        c->state = 0x1A;
        break;
    case 0x1A:
        break;
    case 0x1B:
        st_mod_switch(5, 0);
        c->state = 4;
        break;
    case 0x1C:
        st_mod_switch(6, 1);
        c->state = 0x1D;
        break;
    case 0x1D:
        break;
    case 0x1E:
        st_mod_switch(6, 0);
        c->state = 4;
        break;
    case 0x1F:
        st_mod_switch(7, 1);
        c->state = 0x20;
        break;
    case 0x20:
        break;
    case 0x21:
        st_mod_switch(7, 0);
        c->state = 4;
        break;
    case 0x22:
        st_mod_switch(8, 1);
        c->state = 0x23;
        break;
    case 0x23:
        break;
    case 0x24:
        st_mod_switch(8, 0);
        c->state = 4;
        break;
    case 0x25:
        st_mod_switch(9, 1);
        c->state = 0x26;
        break;
    case 0x26:
        break;
    case 0x27:
        st_mod_switch(9, 0);
        c->state = 4;
        break;
    case 0x28:
        st_mod_switch(10, 1);
        c->state = 0x29;
        break;
    case 0x29:
        break;
    case 0x2A:
        st_mod_switch(10, 0);
        c->state = 4;
        break;
    case 0x2B:
        st_mod_switch(11, 1);
        c->state = 0x2C;
        break;
    case 0x2C:
        break;
    case 0x2D:
        st_mod_switch(11, 0);
        c->state = 4;
        break;
    case 0x2E:
        st_staffroll_start();
        break;
    case 0x2F:
        break;
    case 0x30:
        st_staffroll_play();
        break;
    case 0x31:
        break;
    case 0x32:
        st_ending_save();
        c->state = 0x33;
        break;
    case 0x33:
        st_ending_save_poll();
        break;
    case 0x34:
        st_ending_save_done();
        c->state = 0xC;
        break;
    default:
        c->state = 4;
        break;
    }
}

/* ends the story: unloads every sub-screen module and the message packs */
void story_quit(int fade)
{
    StoryCtx *c = &story_ctx;
    int i;

    if (fade) {
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
    }
    if (st_req_exit() != 3)
        st_snd_stop_all();
    for (i = 0; i < 12; i++)
        st_mod_switch(i, 0);
    st_msg_unload(1);
    st_msg_unload(2);
    c->x00 = 1;
}

int story_done(void)
{
    return story_ctx.x00;
}

void story_draw(void)
{
    StoryCtx *c = &story_ctx;

    st_draw_hook();
    if (story_ctx.step)
        c->step();
}

/* advances the date/time (kind 2..11), then the new-day updates */
void st_fld_setup(int kind, int arg)
{
    int same_day;
    int chr;

    switch (kind) {
    case 2:
        scal_tick_next();
        break;
    case 3:
        scal_tick_add(arg);
        break;
    case 4:
        scal_set_slot(arg);
        break;
    case 5:
        scal_next_day();
        break;
    case 6:
        scal_set_day(arg);
        break;
    case 7:
        st_pl_room_set(0xE);
        scal_next_day();
        scal_set_slot(1);
        break;
    case 8:
        scal_next_month();
        break;
    case 9:
        scal_set_month(arg);
        break;
    case 10:
        scal_set_slot(10);
        break;
    case 11:
        scal_set_month(3);
        scal_set_day(0x1F);
        scal_set_slot(1);
        break;
    }
    if (scal_day_changed()) {
        if (scal_after_m1_10())
            sysstate_max_byte_97d0(2, 1);
        same_day = 0;
        st_req_set_8(1);
        if (st_ev2_today())
            st_ev2_set_4(1);
        else
            st_ev2_set_4(0);
        st_pl_x150_set(0);
        if (st_room_variant())
            sysflag_set(4);
        if (story_mode_is1()) {
            chr = st_pl_x14_get();
            if (chr >= 0) {
                st_pl_x14_set(-1);
                st_chr_away_set(chr, 0);
            }
        }
    } else {
        same_day = 1;
    }
    st_chr_update_all(same_day);
}

/* field command 0: the story data shown by the PDA */
void st_fld_setup_room(void)
{
    StFldInfo buf;
    int res;
    StFldInfo *info = (StFldInfo *)(char *)&buf;
    StoryCtx *c = &story_ctx;

    if (st_sv_alt())
        st_pl_room_set(st_sv_get_0c());
    if (st_req_exit() == 2 || c->prev == 2)
        info->night = 1;
    else
        info->night = 0;
    scal_get_ptrs(info->date);
    info->items = st_pl_items_get();
    info->cards = st_pl_cards_get();
    info->x2c = st_pl_x2c_ptr();
    info->item_take = st_fld_item_take;
    info->card_add = st_pl_card_add;
    info->period_of = scal_period_of;
    info->mail_list = st_mail_view_list;
    info->mail_open = st_mail_open;
    info->points = st_pl_points;
    fld_cmd_start(0, info);
    while (!fld_cmd_poll(0, &res))
        sys_frame_wait();
}

/* field command 3: the objects of the room, from the branch table entry of the room */
void st_fld_setup_objs(void)
{
    StFldObjs objs;
    int res;
    unsigned short list[0x100];
    int n = 0;
    int room;
    unsigned short *tab;
    unsigned short *e;
    int i;
    unsigned short *d;
    int id, z, y;

    memset(list, 0, sizeof(list));
    room = st_pl_room_get();
    objs.bg = st_room_to_bg(room);
    if (st_sv_alt()) {
        objs.n = 0;
        objs.list = list;
    } else {
        room = stt_branch_find(room);
        tab = (unsigned short *)stt_branch_x();
        for (i = 0, e = tab;; e += 3, i++) {
            id = e[0];
            y = e[1];
            z = e[2];
            if (id == 0 && y == 0 && z == 0)
                break;
            if (id == room) {
                e = tab + (i + 1) * 3;
                d = list;
                for (;;) {
                    if (e[0] != 0)
                        break;
                    if (n >= 0x80)
                        break;
                    n++;
                    d[0] = e[1];
                    d[1] = e[2];
                    d += 2;
                    e += 3;
                }
                break;
            }
        }
        objs.n = n;
        objs.list = list;
    }
    fld_cmd_start(3, &objs);
    while (!fld_cmd_poll(3, &res))
        sys_frame_wait();
}

#ifdef NON_MATCHING /* o original mantém blocos mortos e corpos iguais duplicados (cases 2, 14, 26 e 27) */
/* field command 10: where the player appears in the room (by the room it came from) */
void st_fld_setup_items(void)
{
    StFldPos pos;
    int res;
    int x;
    int z;
    int dir;

    if (st_req_is_night() != 0 || scal_flag2_get(3) != 0) {
        scal_flag2_set(3, 0);
        pos.x = st_pl_x_get();
        pos.z = st_pl_z_get();
        pos.dir = st_pl_dir_get();
    } else {
        switch (st_pl_room_get()) {
        case 0:
        case 1:
        default:
            x = 0x2D; z = 0x1F; dir = 3;
            break;
        case 2:
            if (st_free_state0()) {
                x = 8; z = 0x19; dir = 1;
            } else {
                switch (st_free_state1()) {
                case 1:
                default:
                    x = 0xB; z = 2; dir = 1;
                    break;
                case 2:
                    x = 8; z = 0x19; dir = 1;
                    st_free_state1_set(0);
                    break;
                }
            }
            break;
        case 3:
            x = 0x1F; z = 0x11; dir = 3;
            break;
        case 4:
            x = 0x2E; z = 0x18; dir = 3;
            break;
        case 5:
            if (scal_flag2_get(2)) {
                x = 8; z = 10; dir = 3;
                scal_flag2_set(2, 0);
            } else {
                x = 0x12; z = 9; dir = 3;
            }
            break;
        case 6:
            switch (st_pl_room_prev_get()) {
            case 7: x = 0x11; z = 0x18; dir = 7; break;
            case 8: x = 0x10; z = 0x14; dir = 7; break;
            case 9: x = 0x10; z = 0x10; dir = 7; break;
            case 10: x = 0x10; z = 0xC; dir = 7; break;
            case 11: x = 0xE; z = 0x18; dir = 7; break;
            case 12: x = 0xE; z = 0x14; dir = 7; break;
            case 13: x = 0xE; z = 0x10; dir = 7; break;
            case 14: x = 0xE; z = 0xC; dir = 7; break;
            case 15: x = 0x10; z = 6; dir = 7; break;
            default: x = 0x1F; z = 0x1D; dir = 3; break;
            }
            break;
        case 7: x = 0xB; z = 6; dir = 3; break;
        case 8: x = 0xB; z = 6; dir = 3; break;
        case 9: x = 0xB; z = 6; dir = 3; break;
        case 10: x = 0xB; z = 6; dir = 3; break;
        case 11: x = 0xB; z = 6; dir = 3; break;
        case 12: x = 0xB; z = 6; dir = 3; break;
        case 13: x = 9; z = 6; dir = 3; break;
        case 14:
            dir = st_pl_room_prev_get();
            if (st_req_get_8() && dir == 6)
                dir = 0xE;
            z = 6;
            if (dir != z) {
                x = 4; z = x; dir = 0;
            } else {
                x = 0xB; dir = 3;
            }
            break;
        case 15: x = 0x12; z = 2; dir = 3; break;
        case 16: z = 1; x = 0x12; dir = z; break;
        case 17: x = 0x18; z = 0x1C; dir = 3; break;
        case 18: x = 0x26; z = 0x28; dir = 3; break;
        case 19: x = 0x3E; z = 10; dir = 3; break;
        case 20: z = 1; x = 0xC; dir = z; break;
        case 21: x = 0x3D; z = 0x1F; dir = 3; break;
        case 22: x = 0x21; z = 0x3D; dir = 5; break;
        case 23: x = 0x26; z = 0x2C; dir = 3; break;
        case 24: x = 0x23; z = 0x2F; dir = 5; break;
        case 25: x = 0x10; z = 2; dir = 3; break;
        case 26:
            if (st_pl_room_prev_get() == 0x1B) {
                x = 10; z = 2; dir = 1;
            } else {
                x = 0x3E; z = 0x36; dir = 3;
            }
            break;
        case 27:
            if (st_pl_room_prev_get() == 0x1A) {
                x = 0x37; z = 0x20; dir = 3;
            } else {
                x = 0x37; z = 0x20; dir = 3;
            }
            break;
        }
        pos.x = x * 0.5f + 0.25f;
        pos.z = z * 0.5f + 0.25f;
        pos.dir = dir;
    }
    st_pl_pos_set(pos.x, pos.z, pos.dir);
    fld_cmd_start(10, &pos);
    while (!fld_cmd_poll(10, &res))
        sys_frame_wait();
}
#endif
