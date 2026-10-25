// @range 0x0880B7D0-0x0880C0F0
// @asm 0x0880B7D0-0x0880B8CC
// @rodata 0x0882AD34-0x0882AD68
/* Title screen, second part: the new game prompt, continuing (save load), and the options screen
   (state 13: a tabbed panel of settings). */
#include "libtitle.h"

void bgm_fade_out(int x, int ms);

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

#ifdef NON_MATCHING /* os corpos do switch interno repetem o `li a0,1` que o original deixa só no delay slot */
/* sub 6: "start a new game?" */
void title_newgame_update(void)
{
    Title *const t = &title;
    MenuWidget *const w = &title.win[0];

    switch (title.step) {
    case 0:
        title_goto_fade(0, 0xFFF, 0xFFF);
        t->next_step = 2;
        t->step = 1;
        break;
    case 1:
        break;
    case 2:
        title_newgame_win_init();
        t->step = 3;
    case 3:
        switch (menu_update(w)) {
        case -1:
            break;
        case -2:
            title_goto_fade(1, 5, 0);
            t->step = 4;
            break;
        case -3:
            return;
        case 0:
            title_goto_fade(1, 8, 0);
            t->step = 4;
            break;
        }
        break;
    case 4:
        break;
    case 5:
        break;
    }
}
#endif

void *title_newgame_text(void *list, int x, int y)
{
    font_set_size(14, 14);
    font_set_color(0xFFFFFFFF);
    return title_msg_text(list, x, y, 256, 0, 16);
}

/* sub 7: read the save, then the options screen */
void title_continue_update(void)
{
    Title *const t = &title;

    switch (title.step) {
    case 0:
        if (msd_open(1, t->heap))
            t->step++;
        break;
    case 1:
        switch (msd_update()) {
        case 0:
        case 1:
            break;
        case 2:
            t->step++;
            break;
        case 3:
            t->step++;
            break;
        }
        break;
    case 2:
        t->step++;
        break;
    case 3:
        t->state = 8;
        t->sub = 0;
        t->step = 0;
        title_set_state(8, 0);
        break;
    case 4:
        break;
    }
}

/* sub 5: load the save and go to the main menu */
void title_load_save_update(void)
{
    Title *const t = &title;

    switch (title.step) {
    case 0:
    case 1:
    case 2:
        if (msd_open(4, t->heap) == 0)
            break;
        t->step = 3;
        break;
    case 3:
        switch (msd_update()) {
        case 0:
        case 1:
            break;
        case 2:
            title_apply_volumes();
            gfx_overlay_enable(1);
            card_db_request_load();
            t->step = 4;
            break;
        case 3:
            t->step = 5;
            break;
        }
        break;
    case 4:
        if (card_db_load()) {
            gfx_overlay_enable(0);
            title_set_state(9, 0);
        }
        break;
    case 5:
        title_set_state(5, 0);
        break;
    case 6:
        break;
    }
    t->prev_sub = t->sub;
}

void title_apply_volumes(void)
{
    snd_set_volume(2, cfg_get_byte3(2));
    snd_set_volume(0, cfg_get_byte3(0));
    snd_set_volume(1, cfg_get_byte3(1));
}

/* state 13: options */
void topt_enter(short state, short sub)
{
    title.state = state;
    title.sub = sub;
    title.step = 0;
    title.frame = 0;
    title.color0 = 0xFFFFFFFF;
    title.color1 = 0xFFFFFFFF;
    topt_panel_init();
    title_ehf_load(5, 2, 0, 480);
    title_ehf_load(6, 2, 0, 480);
    title_ehf_load(13, 2, 0, 16);
}

void topt_update(void)
{
    Title *const t = &title;
    TPanel *const p = &title.panel;
    int r;

    switch (title.sub) {
    case 0:
        title_goto_fade(0, 0xFFF, 2);
        t->sub = 1;
        break;
    case 1:
        break;
    case 2:
        if (title_ehf_frame(13) != 16)
            goto done;
        title_ehf_anim(13, 2, 17, 96);
        t->sub = 3;
        break;
    case 3:
        r = tpanel_update(p);
        switch (r) {
        case -1:
            break;
        case -2:
            title_goto_fade(1, 9, 11);
            t->frame = 0;
            t->sub = 4;
            break;
        case -3:
            break;
        default:
            title_goto_fade(1, 33, p->cur);
            t->next_step = r;
            t->frame = 0;
            t->sub = 4;
            break;
        }
        break;
    case 4:
        break;
    case 5:
        break;
    }
done:
    t->prev_sub = t->sub;
}

void *topt_draw(void *list)
{
    Title *const t = &title;
    TPanel *const p = &title.panel;
    TBox *b = &p->box;
    MenuWidget *w = &p->mw;

    list = title_ehf_draw(list, 6);
    list = title_ehf_draw(list, 5);
    if (title.sub >= 2) {
        list = title_ehf_draw(list, 13);
        list = title_screen_label(list);
    }
    b->no_cursor = 1;
    switch (t->sub) {
    case 3:
        w->draw_cursor = 0;
        b->no_cursor = 0;
        break;
    case 4:
        break;
    case 5:
        break;
    }
    list = tpanel_draw(list, p);
    t->frame++;
    return list;
}

void topt_panel_init(void)
{
    TPanel *const p = &title.panel;
    TBox *b = &p->box;
    MenuWidget *w = &p->mw;

    tpanel_init(p, title.heap);
    w->count = 20;
    w->rows = 8;
    b->count = w->count;
    b->n = w->rows;
    p->tab[0].draw = topt_tab_label;
    p->tab[1].draw = topt_tab_label;
    p->tab[2].draw = topt_tab_label;
    b->text = 0;
    b->draw = 0;
    w->draw_item = (MenuDrawItemFn)topt_item_label;
}

void *topt_tab_label(void *list, int i0, int x, int y, int w)
{
    TPanel *const p = &title.panel;
    unsigned short i = i0;
    TTab *tb = &p->tab[i];
    int tx = x + 8;
    int ty;

    if (p->cur == i) {
        ty = y + 4;
        list = dl_scissor(list, tx, ty, 81, 14);
    } else {
        ty = y + 5;
        list = dl_scissor(list, tx, ty, 81, 10);
    }
    list = dl_enable(list, 2);
    list = title_sprite(list, 1, i + 66, tx, ty);
    font_set_size(13, 13);
    if (p->cur == i)
        font_set_color(tb->text_color);
    else
        font_set_color(tb->text_color_off);
    font_set_flag4(1);
    list = title_msg_draw(list, tx + 14, ty, w, 0, i + 33, 0);
    return dl_disable(list, 2);
}

void *topt_item_label(void *list, int i0, int x, int y)
{
    unsigned int id = (unsigned short)i0;

    switch (title.panel.cur) {
    case 0:
        id += 16;
        break;
    case 1:
        id += 26;
        break;
    case 2:
        id += 33;
        break;
    }
    list = title_sprite(list, 1, 61, x + 1, y - 1);
    font_set_size(13, 13);
    font_set_color(0xFF000000);
    font_set_flag4(1);
    return title_msg_draw(list, x + 21, y + 1, 218, 0, id, 2);
}
