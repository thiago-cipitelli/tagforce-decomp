// @range 0x08804348-0x088056EC
// @rodata 0x0882A97C-0x0882AA70
/* Main menu of the title (state 9): entering it (with the save check), the per-frame dispatch and
   draw, the main list and its three sub lists, each with the callback that prints the item labels. */
#include "libtitle.h"

int title_menu_anim(void);
void title_main_menu_update(void);
void *title_main_label(void *list, int i, int x, int y, int w);
void title_sub_update_play(void);
void *title_sub_label_play(void *list, int i, int x, int y, int w);
void title_sub_update_extra(void);
void *title_sub_label_extra(void *list, int i, int x, int y, int w);
void title_sub_update_opt(void);
void *title_sub_label_opt(void *list, int i, int x, int y, int w);

/* menu: 0 = main list, 5/6 = a sub list, 14 = save check first */
void title_menu_enter(int state, int menu, int no_check)
{
    Title *const t = &title;
    TMenu *const m1 = &title.tm[1];
    short m;

    if (no_check == 0 && sysstate_checksum()) {
        t->ret_menu = menu;
        menu = 14;
        t->next_sub = menu;
    }
    m = menu;
    switch (m) {
    case 14:
        title_fade_out(0);
        title_menu_open(t->ret_menu - 3);
        t->ret_pending = 1;
        t->sub = 13;
        t->state = state;
        t->next_sub = menu;
        t->step = 0;
        t->step2 = 0;
        t->frame = 0;
        t->color0 = -1;
        t->color1 = -1;
        break;
    default:
        if (no_check == 0) {
            title_fade_out(0);
            t->state = state;
            t->sub = 0;
            t->next_sub = 0xFFE;
        } else {
            t->sub = 1;
            t->next_sub = 0xFFF;
        }
        t->step = 0;
        t->step2 = 0;
        t->frame = 0;
        t->color0 = -1;
        t->color1 = -1;
        if (menu == 0)
            title_menu_open(0);
        else
            title_menu_open(m - 3);
        break;
    case 5:
        title_menu_open(m - 3);
        title_sub_open_extra();
        m1->cursor = t->arg;
        t->arg = 0;
        if (no_check == 0) {
            title_fade_out(0);
            t->state = state;
            t->next_sub = menu;
            t->sub = 15;
        } else {
            t->state = state;
            t->sub = menu;
        }
        t->step = 0;
        t->step2 = 0;
        t->frame = 0;
        t->color0 = -1;
        t->color1 = -1;
        break;
    case 6:
        title_menu_open(m - 3);
        title_sub_open_opt();
        m1->cursor = t->arg;
        t->arg = 0;
        if (no_check == 0) {
            title_fade_out(0);
            t->state = state;
            t->next_sub = menu;
            t->sub = 15;
        } else {
            t->state = state;
            t->sub = menu;
        }
        t->step = 0;
        t->step2 = 0;
        t->frame = 0;
        t->color0 = -1;
        t->color1 = -1;
        break;
    }
    if (no_check == 0) {
        title_ehf_load(5, 2, 0, 480);
        title_ehf_load(6, 2, 0, 480);
        bgm_play(5);
    }
}

void title_menu_update(void)
{
    Title *const t = &title;
    MenuWidget *w = &title.win[2];

    switch (title.sub) {
    case 1:
        twin_init(w);
        t->sub++;
        break;
    case 2:
        title_main_menu_update();
        break;
    case 3:
        title_sub_update_play();
        break;
    case 5:
        title_sub_update_extra();
        break;
    case 6:
        title_sub_update_opt();
        break;
    case 9:
        title_menu_save_update();
        break;
    case 14:
        title_save_check_update(w);
        break;
    case 0:
    case 4:
    case 7:
    case 8:
    case 10:
    case 11:
    case 12:
    case 13:
    case 15:
        break;
    }
    t->prev_sub = t->sub;
}

void *title_menu_draw(void *list)
{
    Title *const t = &title;

    if (title.sub != 9) {
        list = title_ehf_draw(list, 6);
        list = title_ehf_draw(list, 5);
        if (t->sub >= 2 && t->sub != 14 && t->sub != 13 && t->sub != 15)
            list = title_screen_label(title_ehf_draw(list, 13));
        switch (t->sub) {
        case 0:
        case 1:
            list = tmenu_draw(list, &t->tm[0]);
            break;
        case 2:
            list = tmenu_draw(list, &t->tm[0]);
            break;
        case 3:
        case 5:
        case 6:
        case 10:
        case 15:
            list = tmenu_draw(list, &t->tm[0]);
            list = tmenu_draw(list, &t->tm[1]);
            break;
        case 14:
            list = title_save_check_draw(list);
            break;
        case 4:
        case 7:
        case 8:
        case 9:
        case 11:
        case 12:
        case 13:
            break;
        }
        t->frame++;
    }
    return list;
}

/* the menu panel EHF (layer 13): 0 not shown yet, 1 shown this frame, 2 sliding in, 3 in place */
int title_menu_anim(void)
{
    int f = title_ehf_frame(13);

    if (f < 16)
        return 0;
    if (f == 16) {
        title_ehf_anim(13, 2, 17, 96);
        return 1;
    }
    if (f < 96)
        return 2;
    return 3;
}

void title_menu_open(int cursor)
{
    TMenu *const m = &title.tm[0];

    title.step = 0;
    title.frame = 0;
    tmenu_init(m);
    m->style = 1;
    m->n = 9;
    m->b07 = 0;
    if (title_any_deck() == 0)
        m->item[1] = 1;
    if (title_flag8() == 0)
        m->item[8] = 1;
    if (cfg_get_language() == 0) {
        m->n = 8;
        m->x = 160;
        m->w = 160;
        m->y = 61;
        m->h = 26;
    } else {
        m->n = 8;
        m->x = 140;
        m->y = 61;
        m->w = 200;
        m->h = 26;
    }
    m->n--;
    tmenu_set_fade(m, 4, 1);
    m->draw = (TMenuDrawFn)title_main_label;
    m->cursor = cursor;
    tmenu_update(m, 1);
    title_ehf_load(13, 2, 0, 16);
}

void title_main_menu_update(void)
{
    TMenu *const m = &title.tm[0];
    Title *const t = &title;
    int open = 1;
    int a;

    m->color = 0xFFFFFFFF;
    m->text_color = 0xFFFFFFFF;
    m->sel_color = 0xFF000000;
    a = title_menu_anim();
    if (a < 3) {
        if (t->step == 0) {
            if (a == 0)
                open = 0;
            switch (tmenu_update(m, open)) {
            case -2:
                m->blink = 0;
                m->mode = 2;
                bgm_cmd(600);
                title_goto_fade(1, 5, 0);
                t->frame = 0;
                t->step++;
                break;
            case 0:
                title_sub_open_play();
                break;
            case 1:
                title_goto_fade(1, 12, 0);
                t->step++;
                break;
            case 8:
                bgm_cmd(600);
                title_goto_fade(1, 13, 0);
                t->step++;
                break;
            case 4:
                title_goto_fade(1, 14, 0);
                t->step++;
                break;
            case 2:
                title_sub_open_extra();
                break;
            case 3:
                title_sub_open_opt();
                break;
            case 5:
                title_goto_fade(1, 17, 0);
                t->step++;
                break;
            case 6:
                title_goto_fade(1, 0xFFF, 9);
                t->step++;
                break;
            case 7:
                title_goto_fade(1, 19, 0);
                t->step++;
                break;
            case -3:
            case -1:
                break;
            }
        }
    }
}

void *title_main_label(void *list, int i, int x, int y, int w)
{
    char buf[0x200];
    TMenu *const m = &title.tm[0];
    int ty = y + 4;
    unsigned int c;
    int text;

    memset(buf, 0, sizeof(buf));
    if (m->state >= 2 || (m->fade_color[(unsigned short)i] & 0xFF000000) == 0xFF000000) {
        text = (unsigned short)i + 20;
        c = tmenu_text_color(m, i);
        font_set_size(15, 15);
        font_set_color(c);
        font_set_flag4(1);
        if (text >= 24)
            text++;
        list = title_msg_draw(list, x + 8, ty, w - 16, 0, text, 3);
    }
    return list;
}

void title_sub_open_play(void)
{
    TMenu *const m = &title.tm[1];
    TMenu *const m0 = &title.tm[0];
    DeckFlags f;
    short v[2];

    title.sub = 3;
    title.step = 0;
    title.frame = 0;
    m0->color = 0xFF808080;
    m0->text_color = 0xFF808080;
    m0->sel_color = 0xFF000000;
    tmenu_init(m);
    m->n = 4;
    m->b07 = 0;
    tmenu_set_fade(m, 4, 1);
    if (cfg_get_language() == 0) {
        m->x = 180;
        m->y = 81;
        m->w = 160;
        m->h = 26;
    } else {
        m->x = 160;
        m->y = 70;
        m->w = 200;
        m->h = 26;
    }
    m->style = 4;
    m->draw = (TMenuDrawFn)title_sub_label_play;
    deck_cur_get_u16_0(v);
    *(short *)&f = v[0];
    if (!f.b0)
        m->item[0] = 1;
    switch (sysstate_get_u32_97e8()) {
    case 0:
    case 1:
        m->item[2] = 1;
        m->item[3] = 1;
        break;
    case 2:
        m->item[3] = 1;
        break;
    }
}

void title_sub_update_play(void)
{
    TMenu *const m0 = &title.tm[0];
    Title *const t = &title;
    TMenu *const m1 = &title.tm[1];
    int r;

    tmenu_update(m0, 2);
    if (title.step == 0) {
        r = tmenu_update(m1, 1);
        switch (r) {
        case -2:
            m0->blink = 0;
            m0->mode = 0;
            tmenu_back(m0);
            t->frame = 0;
            t->sub = 2;
            break;
        case 0:
        case 1:
        case 2:
        case 3:
            bgm_cmd(600);
            t->step++;
            t->frame = 0;
            title_goto_fade(1, r + 27, 0);
            break;
        case -3:
        case -1:
            break;
        }
    }
}

void *title_sub_label_play(void *list, int i, int x, int y, int w)
{
    char buf[0x200];
    TMenu *const m = &title.tm[1];
    int ty = y + 4;
    unsigned int c;

    memset(buf, 0, sizeof(buf));
    if (m->state >= 2 || (m->fade_color[(unsigned short)i] & 0xFF000000) == 0xFF000000) {
        c = tmenu_text_color(m, i);
        font_set_size(15, 15);
        font_set_color(c);
        font_set_flag4(1);
        list = title_msg_draw(list, x + 8, ty, w - 16, 0, (unsigned short)i + 29, 3);
    }
    return list;
}

void title_sub_open_extra(void)
{
    TMenu *const m = &title.tm[1];
    TMenu *const m0 = &title.tm[0];

    title.sub = 5;
    title.step = 0;
    title.frame = 0;
    m0->color = 0xFF808080;
    m0->text_color = 0xFF808080;
    m0->sel_color = 0xFF000000;
    tmenu_init(m);
    m->n = 2;
    m->b07 = 0;
    tmenu_set_fade(m, 4, 1);
    if (cfg_get_language() == 0) {
        m->x = 180;
        m->y = 133;
        m->w = 160;
        m->h = 26;
    } else {
        m->x = 160;
        m->y = 118;
        m->w = 200;
        m->h = 26;
    }
    m->style = 4;
    m->draw = (TMenuDrawFn)title_sub_label_extra;
}

void title_sub_update_extra(void)
{
    Title *const t = &title;
    TMenu *const m0 = &title.tm[0];
    TMenu *const m1 = &title.tm[1];
    int open = 1;
    int a;

    a = title_menu_anim();
    tmenu_update(m0, 2);
    if (a < 3 && t->step == 0) {
        if (a == 0)
            open = 0;
        switch (tmenu_update(m1, open)) {
        case -3:
            break;
        case -2:
            m0->blink = 0;
            m0->mode = 0;
            tmenu_back(m0);
            t->frame = 0;
            t->sub = 2;
            break;
        case -1:
            break;
        case 0:
            bgm_cmd(600);
            title_goto_fade(1, 20, 0);
            t->step++;
            break;
        case 1:
            bgm_cmd(600);
            title_goto_fade(1, 21, 0);
            t->step++;
            break;
        }
    }
}

void *title_sub_label_extra(void *list, int i, int x, int y, int w)
{
    char buf[0x200];
    TMenu *const m = &title.tm[1];
    int ty = y + 4;
    unsigned int c;

    memset(buf, 0, sizeof(buf));
    if (m->state >= 2 || (m->fade_color[(unsigned short)i] & 0xFF000000) == 0xFF000000) {
        c = tmenu_text_color(m, i);
        font_set_size(15, 15);
        font_set_color(c);
        font_set_flag4(1);
        list = title_msg_draw(list, x + 8, ty, w - 16, 0, (unsigned short)i + 45, 3);
    }
    return list;
}

void title_sub_open_opt(void)
{
    TMenu *const m = &title.tm[1];
    TMenu *const m0 = &title.tm[0];

    title.sub = 6;
    title.step = 0;
    title.frame = 0;
    m0->color = 0xFF808080;
    m0->text_color = 0xFF808080;
    m0->sel_color = 0xFF000000;
    tmenu_init(m);
    m->n = 7;
    m->b07 = 0;
    tmenu_set_fade(m, 4, 1);
    m->item[5] = 2;
    m->item[6] = 2;
    if (cfg_get_language() == 0) {
        m->x = 180;
        m->y = 81;
        m->w = 160;
        m->h = 26;
    } else {
        m->x = 160;
        m->y = 70;
        m->w = 200;
        m->h = 26;
    }
    m->style = 4;
    m->draw = (TMenuDrawFn)title_sub_label_opt;
    tmenu_update(m, 1);
}

void title_sub_update_opt(void)
{
    Title *const t = &title;
    TMenu *const m0 = &title.tm[0];
    TMenu *const m1 = &title.tm[1];
    int open = 1;
    int a;

    a = title_menu_anim();
    tmenu_update(m0, 2);
    if (a < 3 && t->step == 0) {
        if (a == 0)
            open = 0;
        switch (tmenu_update(m1, open)) {
        case -2:
            m0->blink = 0;
            m0->mode = 0;
            tmenu_back(m0);
            t->frame = 0;
            t->sub = 2;
            break;
        case 0:
            bgm_cmd(600);
            title_goto_fade(1, 22, 0);
            t->step++;
            break;
        case 1:
            title_goto_fade(1, 23, 0);
            t->step++;
            break;
        case 2:
            bgm_cmd(600);
            title_goto_fade(1, 24, -1);
            t->step++;
            break;
        case 3:
            title_goto_fade(1, 25, 0);
            t->step++;
            break;
        case 4:
            bgm_cmd(600);
            title_goto_fade(1, 26, 0);
            t->step++;
            break;
        case 5:
            bgm_cmd(600);
            title_goto_fade(1, 36, 0);
            t->step++;
            break;
        case 6:
            title_goto_fade(1, 6, 0);
            t->step++;
            break;
        case -3:
        case -1:
            break;
        }
    }
}

void *title_sub_label_opt(void *list, int i, int x, int y, int w)
{
    char buf[0x200];
    TMenu *const m = &title.tm[1];
    int ty = y + 4;
    unsigned int c;

    memset(buf, 0, sizeof(buf));
    if (m->state >= 2 || (m->fade_color[(unsigned short)i] & 0xFF000000) == 0xFF000000) {
        c = tmenu_text_color(m, i);
        font_set_size(15, 15);
        font_set_color(c);
        font_set_flag4(1);
        list = title_msg_draw(list, x + 8, ty, w - 16, 0, (unsigned short)i + 47, 3);
    }
    return list;
}
