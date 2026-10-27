// @range 0x08806EF0-0x08807C98
// @data 0x0882E3D4-0x0882E3E8
// @rodata 0x0882AB28-0x0882ABEC
/* Per-frame update and draw of the title scene: dispatch by scene state, the boot states (read the
   system save and the card database), the logo sequence with its skip handling, the opening movie,
   and the frame's display list with the sub-screen modules drawn on top. */
#include "libtitle.h"

void title_boot_update(void);
void title_wait_update(void);
void title_logo_update(void);
void title_movie_play(void);

int title_update(void)
{
    Title *const t = &title;
    int r;

    title.pad = pad_get();
    title.prev_state = title.state;
    title.prev_sub = title.sub;
    r = title_transition();
    switch (title.state) {
    case 0:
        title_boot_update();
        break;
    case 1:
    case 7:
    case 10:
    case 11:
    case 15:
    case 16:
    case 18:
    case 20:
    case 27:
    case 28:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 37:
    case 38:
        break;
    case 2:
        title_wait_update();
        break;
    case 3:
        title_logo_update();
        break;
    case 4:
        if (t->sub != 0)
            title_set_state(5, 0);
        break;
    case 5:
        t->skip = 0;
        title_screen_update();
        break;
    case 6:
        text_update();
        break;
    case 8:
        tname_update();
        break;
    case 9:
        title_menu_update();
        break;
    case 12:
        tdl_update();
        break;
    case 13:
        topt_update();
        break;
    case 14:
        tfd_update();
        break;
    case 17:
        tcfg_screen_update();
        break;
    case 19:
        tlang_update();
        break;
    case 21:
        title_recipe_update();
        break;
    case 22:
        title_chara_update();
        break;
    case 23:
        title_album_update();
        break;
    case 24:
        r = title_tlist_update(t->state);
        break;
    case 25:
        title_record_update();
        break;
    case 26:
        r = title_limit_update();
        break;
    case 29:
    case 30:
        r = title_chara_pick_update(t->state);
        break;
    case 39:
        title_dtut_update();
        break;
    case 36:
        r = title_load_update();
        break;
    }
    t->prev_state = t->state;
    t->ticks++;
    if (t->ticks >= 0x10000)
        t->ticks = 0;
    return r;
}

int title_get_ticks(void)
{
    return title.ticks;
}

/* state 0: read the system save, then the card database */
void title_boot_update(void)
{
    Title *const t = &title;

    switch (title.sub) {
    case 0:
        gfx_overlay_enable(1);
        title_sound_reset(1);
        t->sub++;
        gfx_overlay_enable(0);
        break;
    case 1:
        if (msd_open(1, t->heap))
            t->sub++;
        break;
    case 2:
        if (msd_update() != 1) {
            card_db_request_load();
            t->sub++;
        }
        break;
    case 3:
        if (card_db_load())
            title_set_state(2, 0);
        break;
    }
    t->prev_sub = t->sub;
}

void title_state_init(int state, int sub)
{
    title.state = state;
    title.next_state = 3;
    title.sub = sub;
    title.next_sub = 0;
    title.step = 0;
    title.frame = 0;
}

/* state 2: wait for the fade, then the queued state */
void title_wait_update(void)
{
    Title *const t = &title;

    switch (title.sub) {
    case 0:
        gfx_overlay_enable(1);
        title_sound_reset(0);
        title_lang_load(0);
        t->sub++;
        break;
    case 1:
        break;
    case 2:
        gfx_overlay_enable(0);
        title_set_state(t->next_state, t->next_sub);
        break;
    }
    t->prev_sub = t->sub;
}

void title_logo_enter(int state, int sub)
{
    title_fade_ex(4, 1, 0xFF000000, 30);
    title.state = state;
    title.sub = sub;
    title.next_sub = 0xFFE;
    title.step = 0;
    title.frame = 0;
    title.no_movie = 1;
}

/* state 3: the logos and the title screen; START or the accept button skips */
void title_logo_update(void)
{
    Title *const t = &title;
    unsigned int trig = pad_get()->b.trig;

    if (title.skip == 0 && t->sub >= 3 && ((trig & 8) || (trig & region_pick_value(0))))
        t->skip = 1;
    if (t->skippable && ((trig & 8) || (trig & region_pick_value(0))))
        t->skip = 1;
    switch (t->sub) {
    case 0:
        t->frame = 0;
        if (t->skip) {
            t->sub++;
            t->frame = 0;
        }
        break;
    case 1:
        if (t->skip) {
            fade_finish();
            t->fade_state = 0;
            t->frame = 0;
            title_goto_fade_n(1, 7, 5, 0);
            t->frame = 0;
            t->sub = 2;
        } else if (t->frame > 60) {
            t->sub++;
            t->frame = 0;
        } else {
            t->frame++;
        }
        break;
    case 2:
        if (t->fade_state == 0) {
            if (t->skip) {
                title_goto_fade_n(1, 30, 5, 0);
                t->frame = 0;
            } else {
                if (cfg_get_language() == 0)
                    title_goto_fade_n(3, 30, 0xFFF, 0xFFE);
                else
                    title_goto_fade_n(1, 30, 4, 0);
                t->frame = 0;
            }
        }
        break;
    case 3:
        t->skippable = 1;
        if (t->fade_state == 0) {
            title_goto_fade_n(2, 30, 0xFFF, 0xFFE);
            t->frame = 0;
        }
        if (t->skip) {
            t->frame = 0;
            t->sub++;
        }
        break;
    case 4:
        if (t->skip) {
            fade_finish();
            t->fade_state = 0;
            t->frame = 0;
            title_goto_fade_n(1, 7, 5, 0);
            t->frame = 0;
            t->sub = 5;
        } else if (t->frame > 60) {
            title_goto_fade_n(1, 30, 4, 0);
            t->frame = 0;
            t->sub = 5;
        } else {
            t->frame++;
        }
        break;
    case 5:
        t->skip = 0;
        break;
    case 6:
        t->skip = 0;
        break;
    case 7:
        if (t->frame < 90)
            t->skip = 0;
        if (t->frame > 300 || t->skip) {
            title_goto_fade_n(1, 30, 3, 0);
            t->frame = 0;
            t->sub = 8;
        } else {
            t->frame++;
        }
        break;
    case 8:
        t->skip = 0;
        break;
    }
    t->prev_sub = t->sub;
}

/* state 4: load the movie codecs into a 5 MB block of the heap */
void title_movie_enter(int state, int sub)
{
    Title *const t = &title;

    title.state = state;
    title.sub = 0;
    title.next_sub = 0;
    title.step = 0;
    title.frame = 0;
    title.movie_mem = heap_alloc(title.heap, 0x500000);
    duel_modules_load(0, 1);
    while (!modmgr_none_loading_alias())
        sys_frame_wait();
    movie_set_memory((unsigned int)t->movie_mem, 0x500000);
}

void title_movie_play(void)
{
    char path[256];
    const char *name = "movie/opening_a.pmf";

    ehfile_make_path_alias(path, 3, name);
    movie_play(path, 3, 64);
    memset(sceGeEdramGetAddr(), 0, 0x110000);
    movie_set_memory(0, 0);
    heap_set_level(title.heap, title.movie_mem);
    duel_modules_unload();
    title.sub = 1;
    title.no_movie = 1;
}

void title_draw(void)
{
    Title *const t = &title;
    void *list;

    list = dl_select_layer(0, 0);
    list = dl_clear(list, 23);
    list = tmenu_gfx_setup(list, 0);
    list = dl_color(list, 0xFFFFFFFF);
    font_set_flag1(1);
    font_set_flag4(1);
    switch (title.prev_state) {
    case 0:
        break;
    case 2:
        break;
    case 3:
        list = title_logo_draw(list);
        break;
    case 4:
        title_movie_play();
        break;
    case 5:
        list = title_screen_draw(list);
        break;
    case 6:
        list = text_draw(list);
        break;
    case 7:
        break;
    case 8:
        list = tname_draw(list);
        break;
    case 9:
        list = title_menu_draw(list);
        break;
    case 10:
        break;
    case 11:
        break;
    case 12:
        list = tdl_draw(list);
        break;
    case 13:
        list = topt_draw(list);
        break;
    case 14:
        list = tfd_config_draw_rect(list);
        break;
    case 15:
        break;
    case 17:
        list = tcfg_screen_draw(list);
        break;
    case 19:
        list = tlang_draw(list);
        break;
    case 22:
        break;
    case 23:
        break;
    case 24:
        break;
    case 25:
        break;
    case 26:
        break;
    case 36:
        list = title_load_draw(list);
        break;
    }
    if (fade_apply(&list) == 1 && t->fade_state == 1)
        t->fade_state = 2;
    dl_set_list(list);
    switch (t->prev_state) {
    case 0:
        if (t->sub < 3)
            msd_update_dialog();
        break;
    case 36:
        if (t->sub == 1 || t->sub == 4)
            msd_update_dialog();
    case 5:
        if (t->sub == 5 && t->step == 3)
            msd_update_dialog();
        break;
    case 9:
        if (t->sub == 9 && t->step == 3)
            msd_update_dialog();
        break;
    case 8:
        switch (t->sub) {
        case 2:
            tname_osk_draw();
            break;
        case 3:
            if (t->step == 3)
                msd_update_dialog();
            break;
        }
        break;
    case 21:
        if (t->sub_shown)
            recipe_viewer_draw();
        break;
    case 29:
    case 30:
    case 22:
    case 12:
        if (t->sub_shown)
            charalist_draw();
        break;
    case 23:
        title_album_draw();
        break;
    case 25:
        title_record_draw();
        break;
    case 26:
        title_limit_draw();
        break;
    case 14:
        tfd_net_lobby_machine();
        break;
    case 24:
        title_tlist_draw();
        break;
    case 39:
        title_dtut_draw();
        break;
    }
}
