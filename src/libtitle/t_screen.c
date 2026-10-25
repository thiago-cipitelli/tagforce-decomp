// @range 0x0880AFA0-0x0880B7D0
// @rodata 0x0882ACA0-0x0882AD1C
/* The title screen (state 5: press start, then NEW GAME / CONTINUE / ... with the title call voice)
   and the new game prompt window. */
#include "libtitle.h"

void bgm_fade_out(int x, int ms);
int cfg_get_byte3(int i);
void snd_set_volume(int channel, int vol);

void title_start_menu_init(void);
void title_start_menu_update(void);
void title_newgame_update(void);
void title_continue_update(void);
void title_load_save_update(void);
void title_apply_volumes(void);
void topt_panel_init(void);
void *topt_tab_label(void *list, int i0, int x, int y, int w);
void *topt_item_label(void *list, int i0, int x, int y);
int tpanel_update(TPanel *p);
void *tpanel_draw(void *list, TPanel *p);
void *title_start_label(void *list, int i, int x, int y, int w);
void *title_newgame_text(void *list, int x, int y);

void title_screen_enter(short state, short sub)
{
    Title *const t = &title;

    cfg_get_language();
    switch (sub) {
    case 0:
        t->color0 = 0xFFFFFFFF;
        t->color1 = 0xFFFFFFFF;
        title_goto_fade_n(0, 30, 0xFFF, 0xFFE);
        t->state = state;
        t->sub = sub;
        t->step = 0;
        t->frame = 0;
        break;
    case 11:
        t->color0 = 0xFF404040;
        t->color1 = 0xFFFFFFFF;
        title_goto_fade(0, 5, 4);
        title_start_menu_init();
        title_start_menu_update();
        t->state = state;
        t->sub = sub;
        t->step = 0;
        t->frame = 0;
        break;
    }
    title_ehf_load(0, 2, 0, 89);
    title_ehf_load(1, 2, 0, 119);
    if (sub != 8)
        bgm_play(4);
}

void title_screen_update(void)
{
    Title *const t = &title;
    unsigned int trig = pad_get()->b.trig;

    if (title_ehf_frame(0) == 89)
        title_ehf_anim(0, 2, 90, 129);
    switch (t->sub) {
    case 0:
    case 8:
    case 9:
    case 11:
        break;
    case 1:
        if (cfg_get_language() == 0) {
            t->voice = (int)rand_next() % 43 + 0x61C1;
            se_play(t->voice);
        }
        t->sub++;
    case 2:
        if (tcfade_step(&t->cfade) == 0) {
            if ((trig & 8) || (trig & region_pick_value(0))) {
                se_play(0x6213);
                title_ehf_load(1, 2, 120, 164);
                tcfade_init(&t->cfade, &t->color0, 0xFF404040, 15);
                tcfade_start(&t->cfade);
                t->sub++;
                t->next_sub = 4;
            } else if (t->frame >= 990) {
                if (t->no_movie) {
                    title_goto_fade(1, 32, 0);
                    t->sub = 0;
                    t->frame = 0;
                    bgm_cmd(600);
                } else {
                    title_goto_fade(1, 3, 0);
                    t->sub = 0;
                    t->frame = 0;
                    bgm_cmd(600);
                }
            }
        } else {
            t->frame = 0;
        }
        break;
    case 3:
        if (tcfade_step(&t->cfade) == 0) {
            title_start_menu_init();
            t->sub = t->next_sub;
        }
        break;
    case 4:
        title_start_menu_update();
        break;
    case 5:
        title_load_save_update();
        break;
    case 6:
        title_newgame_update();
        break;
    case 7:
        title_continue_update();
        break;
    case 10:
        if (t->step == 0) {
            title_goto_fade(1, 31, 0);
            t->step++;
        }
        break;
    }
    t->prev_sub = t->sub;
}

void *title_screen_draw(void *list)
{
    Title *const t = &title;

    if (title.sub != 8) {
        cfg_get_language();
        list = title_ehf_draw(list, 0);
        switch (t->sub) {
        case 0:
        case 1:
        case 5:
        case 7:
        case 8:
        case 9:
            break;
        case 2:
        case 3:
        case 10:
            list = title_ehf_draw(list, 1);
            break;
        case 4:
        case 11:
            list = tmenu_draw(list, &t->tm[0]);
            break;
        case 6:
            if (t->step >= 3)
                list = twin_draw(list, &t->win[0]);
            break;
        }
        t->frame++;
    }
    return list;
}

void title_start_menu_init(void)
{
    TMenu *const m = &title.tm[0];

    tmenu_init(m);
    m->b07 = 0;
    if (sysvar_get_a94())
        m->cursor = 1;
    else
        m->cursor = 0;
    m->n = 2;
    m->x = 140;
    m->y = 198;
    m->w = 200;
    m->h = 24;
    tmenu_set_fade(m, 4, 1);
    m->draw = (TMenuDrawFn)title_start_label;
}

void title_start_menu_update(void)
{
    Title *const t = &title;
    TMenu *const m = &title.tm[0];

    cfg_get_language();
    switch (title.step) {
    case 0:
    case 1:
        switch (tmenu_update(m, 1)) {
        case -2:
            tcfade_init(&t->cfade, &t->color0, 0xFFFFFFFF, 15);
            tcfade_start(&t->cfade);
            t->sub = 2;
            t->frame = 0;
            title_ehf_anim(1, 2, 0, 119);
            break;
        case 0:
            title_goto_fade(1, 8, 0);
            t->step = 2;
            newgame_init();
            break;
        case 1:
            title_goto_fade(1, 0xFFF, 5);
            t->step = 2;
            break;
        case 2:
            title_goto_fade(1, 0xFFF, 8);
            t->step++;
            break;
        case 3:
            title_goto_fade(1, 6, 0);
            t->step++;
            break;
        case -3:
        case -1:
            break;
        }
        break;
    case 2:
        if (t->voice != 0) {
            bgm_fade_out(t->voice, 1000);
            t->voice = 0;
        }
        break;
    case 3:
        break;
    }
}

void *title_start_label(void *list, int i, int x, int y, int w)
{
    TMenu *m = &title.tm[0];
    int ty = y + 4;
    unsigned short k = i;
    int id = k + 16;
    unsigned int c;

    if (m->state >= 2 || (m->fade_color[k] & 0xFF000000) == 0xFF000000) {
        c = tmenu_text_color(m, i);
        font_set_size(15, 15);
        font_set_color(c);
        font_set_flag4(1);
        list = title_msg_draw(list, x, ty, w, 0, id, 1);
    }
    return list;
}

void title_newgame_win_init(void)
{
    MenuWidget *const w = &title.win[0];
    MenuItemDef *d = &w->def;

    twin_init(w);
    d->style = 2;
    d->open_anim = 2;
    d->close_anim = 2;
    d->x = 80;
    d->y = 96;
    d->h = 96;
    d->w = 300;
    w->no_input = 0;
    w->no_cancel = 0;
    w->cancel_blocked = 0;
    w->keep_open = 0;
    w->f0_4 = 0;
    w->cancel_resets = 0;
    w->draw_all = (MenuDrawRectFn)title_newgame_text;
    d->title = 0;
    twin_open(w);
}
