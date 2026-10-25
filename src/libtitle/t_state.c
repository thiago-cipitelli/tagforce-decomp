// @range 0x088062B4-0x08806EF0
// @rodata 0x0882AA70-0x0882AB28
/* Scene state switching of the title: the save prompts of the main menu, the transition that
   applies the queued state/sub-state after a fade, title_set_state (enters a state: opens its
   screen or starts its sub-screen module), and the new game / load flow (state 36). */
#include "libtitle.h"

int title_load_data(void);
int title_save_data(void);
void title_newgame_confirm(void);
void title_load_confirm(void);
/* called with the raw int arguments (no prototype in the original) */
void title_load_enter();
void title_limit_open();
void title_tlist_open();
void title_dtut_open();

/* sub 9 of the main menu: save the options, then back to the menu */
void title_menu_save_update(void)
{
    Title *const t = &title;

    switch (title.step) {
    case 0:
        bgm_cmd(600);
        title_goto_fade(0, 0xFFF, 0xFFF);
        t->step++;
        t->next_step = 2;
        break;
    case 1:
        break;
    case 2:
        if (msd_open(5, t->heap))
            t->step++;
    case 3:
        if (msd_update() != 1)
            t->step = 4;
        break;
    case 4:
        if (t->ret_pending) {
            t->ret_pending = 0;
            title_set_state(9, t->ret_menu);
        } else {
            title_set_state(9, 9);
        }
        break;
    }
    t->prev_sub = t->sub;
}

/* sub 14 of the main menu: the save data is broken, ask before going on */
void title_save_check_update(MenuWidget *unused)
{
    Title *const t = &title;
    MenuWidget *const w = &title.win[0];

    switch (title.step) {
    case 0:
        func_0880FBB8(75, 1, 1);
        t->step = 1;
        se_play(0x621A);
        break;
    case 1:
        if (w->state < 3) {
            menu_update(w);
        } else {
            switch (menu_update(w)) {
            case -2:
            case 3:
                menu_finish(w);
                t->step = 2;
                break;
            case 2:
                menu_finish(w);
                t->step = 3;
                title_goto_fade(1, 0xFFF, 9);
                break;
            }
        }
        break;
    case 2:
        menu_update(w);
        if (w->state == 6) {
            t->ret_pending = 0;
            title_menu_enter(9, t->ret_menu, 1);
        }
        break;
    case 3:
        break;
    case 4:
        break;
    }
}

void *title_save_check_draw(void *list)
{
    Title *t = &title;

    switch (title.step) {
    case 0:
        break;
    case 1:
    case 2:
        list = twin_draw_center(list, &t->win[0], 1);
        break;
    case 4:
        break;
    }
    return list;
}

/* applies the queued state changes once the fade is done; returns the scene state */
int title_transition(void)
{
    Title *const t = &title;

    if (!title.busy && title_fade_state() == 2) {
        func_088119D4(0);
        t->fade_state = 0;
        if (t->next_sub != 0xFFF) {
            if (t->next_sub == 0xFFE)
                t->sub++;
            else
                t->sub = t->next_sub;
            t->next_sub = 0xFFF;
        }
        if (t->next_step != 0xFFF) {
            if (t->next_step == 0xFFE)
                t->step++;
            else
                t->step = t->next_step;
            t->next_step = 0xFFF;
        }
        if (t->next_step2 != 0xFFF) {
            if (t->next_step2 == 0xFFE)
                t->step2++;
            else
                t->step2 = t->next_step2;
            t->next_step2 = 0xFFF;
        }
        if (t->next_state != 0xFFF) {
            t->state = t->next_state;
            return title_set_state(t->state, t->sub);
        }
    }
    return t->state;
}

/* fades out, then title_transition enters state/sub */
void title_goto(short state, short sub)
{
    title_state_init(2, 0);
    title.next_state = state;
    title.next_sub = sub;
}

/* enters a scene state; 0x28 = the state left the title loop (scene change) */
int title_set_state(int st, int sub)
{
    short state = st;

    switch (state) {
    case 0:
    case 1:
    case 7:
    case 10:
    case 11:
    case 15:
    case 16:
    case 18:
        break;
    case 2:
        title_state_init(st, 0);
        break;
    case 3:
        title_logo_enter(st, sub);
        break;
    case 4:
        title_movie_enter(st, 0);
        break;
    case 5:
        func_0880AFA0();
        break;
    case 6:
        func_08813088();
        break;
    case 8:
        func_0880F900();
        break;
    case 9:
        title_menu_enter(st, sub, 0);
        break;
    case 12:
        func_08821D3C();
        break;
    case 13:
        func_0880BB98();
        break;
    case 14:
        func_08813DF4();
        break;
    case 17:
        func_0880C998();
        break;
    case 19:
        func_0880EB20();
        break;
    case 20:
        func_08809DC4(4, sub);
        return 40;
    case 21:
        title_recipe_open();
        break;
    case 22:
        title_chara_open();
        break;
    case 23:
        title_album_open();
        break;
    case 24:
        title_tlist_open(st, sub);
        break;
    case 25:
        title_record_open();
        break;
    case 26:
        title_limit_open(st);
        break;
    case 27:
    case 28:
        func_0880AEF0(state - 27, 0);
        return 40;
    case 29:
    case 30:
        title_chara_open();
        break;
    case 31:
        newgame_init();
        func_08809DC4(2, 0);
        return 40;
    case 32:
        func_08809E24();
        return 40;
    case 33:
        func_0880A074();
        return 40;
    case 34:
        charalist_res_free();
        func_0880A934();
        func_0880A100();
        return 40;
    case 35:
        func_08815ADC();
        return 40;
    case 36:
        title_load_enter(st);
        break;
    case 37:
        sysvar_set_a90(1);
        func_08809DC4(14, 0);
        return 40;
    case 38:
        func_0880A390();
        return 40;
    case 39:
        title_dtut_open(st);
        break;
    }
    return state;
}

void title_load_enter(state)
short state;
{
    title.state = state;
    title.sub = 0;
    title.step = 0;
    title.step2 = 0;
    fade_start(1, 0xFF000000, 16);
    fade_finish();
}

/* state 36: read the save, then the new game or continue prompts */
int title_load_update(void)
{
    Title *t = &title;

    switch (title.sub) {
    case 0:
        t->sub++;
    case 1:
        title_load_data();
        break;

    case 2:
        title_newgame_confirm();
        break;
    case 3:
        title_load_confirm();
        break;
    case 4:
        title_save_data();
        break;
    case 5:
        title_set_state(37, 0);
        return 40;
    }
    return 36;
}

void title_newgame_confirm(void)
{
    Title *const t = &title;
    MenuWidget *const w = &title.win[0];
    unsigned int press = pad_get()->b.trig;

    switch (title.step) {
    case 0:
        se_play(0x621A);
        func_0880FBB8(73, 2, 0);
        t->step++;
    case 1:
        if (press & 8) {
            t->step = 4;
            menu_finish(w);
        } else {
            switch (menu_update(w)) {
            case -2:
            case 4:
                t->step = 3;
                break;
            case 1:
                t->step = 2;
                break;
            }
        }
        break;
    case 2:
    case 3:
    case 4:
        menu_update(w);
        if (w->state == 6) {
            if (t->step == 2) {
                t->step = 0;
                t->sub = 5;
            } else if (t->step == 3) {
                t->arg = 5;
                title_set_state(9, 6);
            } else {
                t->step = 0;
                t->sub = 4;
            }
        }
        break;
    }
}

int title_load_data(void)
{
    Title *const t = &title;

    switch (title.step) {
    case 0:
        if (msd_open(2, t->heap))
            t->step++;
        break;
    case 1:
        if (msd_update() != 1) {
            card_db_request_load();
            t->step++;
        }
        break;
    case 2:
        if (card_db_load()) {
            if (sysvar_get_a94()) {
                t->sub = 2;
                t->step = 0;
                return 1;
            } else {
                t->sub = 3;
                t->step = 0;
                return 1;
            }
        }
        break;
    }
    return 0;
}

int title_save_data(void)
{
    Title *const t = &title;

    switch (title.step) {
    case 0:
        if (msd_open(6, t->heap))
            t->step++;
        break;
    case 1:
        if (msd_update() != 1)
            t->step++;
        break;
    case 2:
        t->sub = 0;
        t->step = 0;
        return 1;
    }
    return 0;
}

void title_load_confirm(void)
{
    Title *const t = &title;
    MenuWidget *const w = &title.win[0];

    switch (title.step) {
    case 0:
        se_play(0x621A);
        func_0880FBB8(74, 3, 0);
        t->step++;
    case 1:
        switch (menu_update(w)) {
        case -2:
        case 3:
        case 1:
            t->step = 2;
            break;
        }
        break;
    case 2:
    case 3:
        menu_update(w);
        if (w->state == 6) {
            t->arg = 5;
            title_set_state(9, 6);
        }
        break;
    case 5:
        break;
    }
}

void *title_load_draw(void *list)
{
    Title *t = &title;
    MenuWidget *w = &title.win[0];

    switch (title.sub) {
    case 0:
        break;
    case 2:
        if (t->step > 0)
            list = twin_draw_center(list, w, 1);
        break;
    case 1:
        break;
    case 3:
        if (t->step > 0)
            list = twin_draw_center(list, w, 1);
        break;
    case 5:
        break;
    }
    return list;
}
