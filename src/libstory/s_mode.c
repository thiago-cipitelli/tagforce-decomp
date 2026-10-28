// @range 0x0880EC38-0x0880FBE4
// @rodata 0x0881C484-0x0881C510
/* Story mode states around the field: opening the field (empty or with the characters, partner,
   events and pickups of the room), the room map, the free walk and what its result leads to (talk,
   duel, shop, sub-screens, sleep...), the menu results of the story scripts and the ending checks. */
#include "libstory.h"

int st_pl_room_get(void);
void st_pl_room_set(int place);
int st_pl_x14_get(void);
void st_pl_pos_set(float x, float y, int z);
void st_pl_x18_set(int id);
void st_pl_x1c_set(int v);
void st_pl_x20_set(int v);
int st_sv_alt(void);
int st_id_to_deck(int id);
void st_chr_set_pos(int chr, int x, int y, int dir);
void st_chr_set_pos_b(int chr, int x, int y);
void st_chr_set_dir_b(int chr, int dir);
int st_bg_to_room(int bg);
void st_fld_setup(int kind, int arg);
void st_fld_setup_room(void);
void st_fld_setup_objs(void);
void st_fld_setup_items(void);
int st_fld_place_chrs(void);
int st_fld_partner(int chr);
int st_fld_events(void);
int st_fld_pickups(void);
void st_fld_night(void);
void st_fld_links(void);
void st_fld_fade_in(int on);
int st_fld_fade_in_done(void);
void st_fld_fade_out_wait(void);
void st_mod_switch(int id, int load);
void st_snd_stop_all(void);
void st_bgm_fade(void);
void st_bgm_play_24(void);
void st_bgm_play_room(void);
void st_fade(int out, int wait, int frames);
void st_req_set_night(int v);
void st_req_set_exit(int v);
void st_req_set_c(int v);
void st_req_set_lesson(int v);
int st_ev2_get_4(void);
void st_ev2_set_4(int v);
void st_ev2_set_0(int v);
void st_event_start(int kind, int a, int b);
void st_fld_wait_idle(void);
int st_after_free(void);
void st_ev_clear(void);
int st_ev_run(int id);
void st_msg_load_chr(int deck);
void st_flag666c_set(int v);
void st_lesson_duel_start(void);
void st_lesson_duel_start2(void);
int st_mod_loaded(int id);
int st_duel_pending(void);
void st_duel_prepare(void);
void vm_step(void);
int vm_update(void);
int vm_var(int i);
int story_mode_is1(void);
int sdate_check(int a);
int scal_get_cal_month(void);
int st_check_ending(void);

typedef struct StFreeChr {             /* character moved during the free walk */
    int id;
    int x;
    int y;
    int dir;
} StFreeChr;

typedef struct StFreeResult {          /* result of fld command 0x11 (free walk), 0x228 */
    int kind;                          /* +0x000 what ended the walk */
    float x;                           /* +0x004 player position */
    float y;                           /* +0x008 */
    int z;                             /* +0x00C */
    StFreeChr chr[32];                 /* +0x010 */
    int nchr;                          /* +0x210 */
    int id;                            /* +0x214 character talked to (deck id) */
    int arg;                           /* +0x218 */
    int hour;                          /* +0x21C */
    int minute;                        /* +0x220 */
    int bg;                            /* +0x224 */
} StFreeResult;

/* stops the sound when coming back from the sub-screens that play their own */
void st_snd_after_sub(void)
{
    switch (story_ctx.prev) {
    case 2:
    case 8:
    case 9:
    case 7:
        st_snd_stop_all();
        st_bgm_fade();
    }
}

/* opens the field without characters (dorm room at night) */
void st_field_open_empty(void)
{
    StoryCtx *c = &story_ctx;

    st_snd_after_sub();
    st_mod_switch(0, 1);
    st_fld_setup(1, 0);
    st_fld_setup_room();
    st_fld_links();
    st_bgm_play_24();
    st_fld_fade_in(0);
    gfx_overlay_enable(0);
    st_fade(1, 1, 0);
    while (!st_fld_fade_in_done())
        sys_frame_wait();
    st_req_set_night(0);
    c->prev = 12;
    c->state = 2;
}

/* the room map: picks the room to go to (fld command 0x1E) */
void st_field_room_map(void)
{
    StoryCtx *c = &story_ctx;
    int arg;
    int res[2];
    int room;

    fld_cmd_start(0x1E, &arg);
    while (!fld_cmd_poll(0x1E, res))
        sys_frame_wait();
    room = st_bg_to_room(res[1]);
    if (res[0] != 1) {
        st_pl_room_set(room);
        st_fade(0, 1, 0);
        c->state = 3;
    } else {
        c->state = 12;
        st_req_set_exit(9);
    }
}

void st_field_fade_out(void)
{
    st_fld_fade_out_wait();
    story_ctx.state = 4;
    gfx_overlay_enable(1);
}

/* opens the field of the current room with its characters, partner, events and pickups */
void st_field_open(void)
{
    StoryCtx *c = &story_ctx;
    int partner;

    st_snd_after_sub();
    st_mod_switch(0, 1);
    st_fld_setup(1, 0);
    st_fld_setup_room();
    st_fld_setup_objs();
    st_fld_setup_items();
    st_fld_place_chrs();
    if ((partner = st_pl_x14_get()) >= 0)
        st_fld_partner(partner);
    st_fld_events();
    st_fld_pickups();
    st_fld_night();
    st_bgm_play_room();
    st_fld_fade_in(1);
    gfx_overlay_enable(0);
    st_fade(1, 1, 0);
    while (!st_fld_fade_in_done())
        sys_frame_wait();
    st_req_set_night(0);
    c->prev = 12;
    if (st_ev2_get_4()) {
        st_ev2_set_4(0);
        st_event_start(9, -1, -1);
        st_ev2_set_0(1);
        c->state = 11;
    } else {
        c->state = 5;
    }
}

/* starts the free walk on the field (fld command 0x11) */
void st_field_walk(void)
{
    int arg;

    st_fld_wait_idle();
    fld_cmd_start(0x11, &arg);
    story_ctx.state = 6;
}

/* end of the free walk: stores the positions and goes where the result says */
void st_field_walk_poll(void)
{
    StoryCtx *c = &story_ctx;
    StFreeResult r;
    int i;
    int id;

    if (fld_cmd_poll(0x11, &r) == 0)
        return;
    st_pl_pos_set(r.x, r.y, r.z);
    if (st_sv_alt()) {
        for (i = 0; i < r.nchr; i++) {
            id = st_id_to_deck(r.chr[i].id);
            st_chr_set_pos_b(id, r.chr[i].x, r.chr[i].y);
            st_chr_set_dir_b(id, r.chr[i].dir);
        }
    } else {
        for (i = 0; i < r.nchr; i++)
            st_chr_set_pos(st_id_to_deck(r.chr[i].id), r.chr[i].x, r.chr[i].y, r.chr[i].dir);
    }
    st_pl_x18_set(-1);
    switch (r.kind) {
    default:
        switch (st_pl_room_get()) {
        case 7:
        case 8:
        case 9:
        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF:
            st_pl_room_set(6);
            break;
        case 0x1B:
            st_pl_room_set(0x1A);
            break;
        }
    case 0:
        switch (st_after_free()) {
        case 0:
        default:
            st_fade(0, 1, 0);
            gfx_overlay_enable(1);
            c->state = 7;
            break;
        case 1:
            st_ev_run(0x102);
            st_ev_clear();
            st_fade(0, 1, 0);
            gfx_overlay_enable(1);
            st_fld_fade_out_wait();
            st_fld_setup(7, 0);
            c->state = 4;
            break;
        case 2:
            st_fade(0, 1, 0);
            gfx_overlay_enable(1);
            st_fld_fade_out_wait();
            st_pl_room_set(6);
            c->state = 4;
            break;
        case 3:
            st_fade(0, 1, 0);
            gfx_overlay_enable(1);
            st_fld_fade_out_wait();
            st_pl_room_set(0x1A);
            c->state = 4;
            break;
        case 4:
            st_ev_clear();
            st_fade(0, 1, 0);
            gfx_overlay_enable(1);
            st_fld_fade_out_wait();
            st_fld_setup(2, 0);
            st_pl_room_set(2);
            st_flag666c_set(1);
            st_field_open();
            sys_frame_wait();
            st_event_start(0xC, -1, -1);
            c->state = 11;
            break;
        }
        break;
    case 2:
        id = st_id_to_deck(r.id);
        st_pl_x18_set(r.id);
        st_msg_load_chr(id);
        st_event_start(0, id, -1);
        c->state = 11;
        break;
    case 3:
        st_pl_x1c_set(r.arg);
        id = st_id_to_deck(r.id);
        st_pl_x18_set(r.id);
        st_msg_load_chr(id);
        st_event_start(1, id, -1);
        c->state = 11;
        break;
    case 4:
        st_pl_x20_set(r.arg);
        id = st_id_to_deck(r.id);
        st_pl_x18_set(r.id);
        st_msg_load_chr(id);
        st_event_start(2, id, -1);
        c->state = 11;
        break;
    case 5:
        id = st_id_to_deck(r.id);
        st_pl_x18_set(r.id);
        st_msg_load_chr(id);
        st_event_start(3, id, -1);
        c->state = 11;
        break;
    case 6:
        id = st_pl_x14_get();
        if (id < 0) {
            c->state = 5;
        } else {
            st_msg_load_chr(id);
            st_event_start(4, id, -1);
            c->state = 11;
        }
        break;
    case 7:
        id = st_pl_x14_get();
        if (id < 0) {
            c->state = 5;
        } else {
            st_msg_load_chr(id);
            st_event_start(5, id, -1);
            c->state = 11;
        }
        break;
    case 8:
        c->state = 12;
        st_req_set_exit(2);
        break;
    case 9:
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        c->state = 0x10;
        break;
    case 10:
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        st_fld_fade_out_wait();
        i = r.minute;
        id = r.hour;
        if (i != 0)
            i = i / 15;
        id = (id - 5) * 4;
        st_fld_setup(4, id + i - 2);
        c->state = 4;
        break;
    case 11:
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        st_fld_fade_out_wait();
        st_fld_setup(7, 0);
        c->state = 4;
        break;
    case 12:
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        st_fld_fade_out_wait();
        st_fld_setup(0xB, 0);
        c->state = 4;
        break;
    case 13:
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        c->state = 0x13;
        break;
    case 14:
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        c->state = 0x19;
        break;
    case 20:
        c->state = 12;
        st_req_set_exit(1);
        break;
    case 21:
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        st_fld_fade_out_wait();
        st_pl_room_set(st_bg_to_room(r.bg));
        c->state = 4;
        break;
    case 15:
        st_fade(0, 1, 0);
        c->state = 0x1F;
        break;
    case 16:
        st_fade(0, 1, 0);
        c->state = 0x1C;
        break;
    case 17:
        st_fade(0, 1, 0);
        st_req_set_lesson(-1);
        c->state = 0x25;
        break;
    case 18:
        st_fade(0, 1, 0);
        c->state = 0x2B;
        break;
    case 19:
        st_fade(0, 1, 0);
        c->state = 0x22;
        break;
    case 22:
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        st_fld_fade_out_wait();
        st_pl_room_set(0x1B);
        c->state = 4;
        break;
    }
}

/* back to the field after a sub-screen or an event */
void st_field_reopen(void)
{
    StoryCtx *c = &story_ctx;

    st_fld_fade_out_wait();
    if (story_mode_is1()) {
        if (sdate_check(0) == 1)
            st_fld_setup(1, 0);
        else
            st_fld_setup(2, 0);
    } else {
        st_fld_setup(1, 0);
    }
    c->state = 1;
}

/* runs the story script and acts on the menu entry it ended with */
void st_vm_menu(void)
{
    StoryCtx *c = &story_ctx;

    vm_step();
    switch (vm_update()) {
    case 100:
    case 101:
        break;
    case 102:
        st_ev_clear();
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        c->state = 0xD;
        break;
    case 103:
        st_ev_clear();
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        c->state = 0x16;
        break;
    case 104:
        st_ev_clear();
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        st_lesson_duel_start();
        c->state = 12;
        st_req_set_exit(6);
        st_pl_x18_set(-1);
        break;
    case 105:
        st_ev_clear();
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        st_lesson_duel_start2();
        c->state = 12;
        st_req_set_exit(8);
        break;
    case 106:
        st_ev_clear();
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        st_fld_fade_out_wait();
        st_pl_room_set(2);
        st_flag666c_set(1);
        st_field_open();
        sys_frame_wait();
        st_event_start(0xD, -1, -1);
        c->state = 11;
        st_pl_x18_set(-1);
        break;
    case 107:
        if (st_mod_loaded(0)) {
            if (st_check_ending() == 0) {
                st_ev_clear();
                gfx_overlay_enable(0);
                st_fade(1, 1, 0);
                if (st_duel_pending()) {
                    st_duel_prepare();
                    c->state = 12;
                    st_req_set_exit(4);
                } else {
                    c->state = 5;
                    st_pl_x18_set(-1);
                }
            }
        } else {
            c->state = 4;
            st_pl_x18_set(-1);
        }
        break;
    case 108:
        st_ev_clear();
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        st_pl_x18_set(-1);
        break;
    case 109:
        st_fld_fade_out_wait();
        st_field_open();
        sys_frame_wait();
        st_event_start(0xB, -1, -1);
        c->state = 11;
        st_pl_x18_set(-1);
        break;
    case 110:
        if (st_check_ending() == 0) {
            st_ev_clear();
            st_fade(0, 1, 0);
            gfx_overlay_enable(1);
            st_fld_fade_out_wait();
            c->state = 4;
            st_pl_x18_set(-1);
        }
        break;
    case 111:
        st_ev_clear();
        st_fade(0, 1, 0);
        gfx_overlay_enable(1);
        st_fld_fade_out_wait();
        st_fld_setup(7, 0);
        c->state = 4;
        st_pl_x18_set(-1);
        break;
    }
}

/* 1 when the story has ended (good or bad ending): goes to the staff roll states */
int st_check_ending(void)
{
    StoryCtx *c = &story_ctx;

    if (story_mode_is1()) {
        if (scal_get_cal_month() == 8) {
            st_pl_x18_set(-1);
            st_fade(0, 1, 0);
            gfx_overlay_enable(1);
            c->state = 0x32;
            return 1;
        }
    } else {
        switch (vm_var(1)) {
        case 1:
            break;
        case 2:
            st_pl_x18_set(-1);
            st_fade(0, 1, 0);
            gfx_overlay_enable(1);
            c->state = 0x32;
            return 1;
        case 3:
            st_pl_x18_set(-1);
            st_req_set_c(1);
            st_fade(0, 1, 0);
            gfx_overlay_enable(1);
            c->state = 0x2E;
            return 1;
        }
    }
    return 0;
}
