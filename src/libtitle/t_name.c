// @range 0x0880F900-0x088106D8
// @rodata 0x0882AE08-0x0882AEA0
/* New game (state 8): the player's name is typed with the on-screen keyboard, confirmed with a
   YES/NO prompt, and the new save is written. Also the message prompts (title_prompt) used by the
   other title states. */
#include "libtitle.h"

int tosk_open(unsigned short *text, unsigned short *desc, int len, void (*done)(unsigned short *, int));
int tosk_update(int layer);
unsigned short *sysstate_get_name(void);
void *title_text_frame(void *list, int x, int y, int w, unsigned short *text, int align);

void tname_setup(void);
void tname_win_init(void);
void tname_update_input(void);
int tname_button(void);
void tname_osk_start(void);
void tname_osk_update(void);
void tname_osk_done(unsigned short *text, int r);
int tname_save(void);
int tname_valid(void);

void tname_enter(void)
{
    title.state = 8;
    title.sub = 0;
    title.step = 0;
    title.step2 = 0;
    title.frame = 0;
    tname_setup();
    title.sub = 1;
}

void tname_update(void)
{
    switch (title.sub) {
    case 0:
        break;
    case 1:
        tname_update_input();
        break;
    case 2:
        tname_osk_update();
        break;
    case 3:
        if (tname_save()) {
            sysstate_checksum();
            title_set_state(9, 0);
        }
        break;
    case 4:
        break;
    }
}

void tname_setup(void)
{
    tname_win_init();
    title_ehf_load(5, 2, 0, 480);
    title_ehf_load(6, 2, 0, 480);
    title_ehf_load(13, 2, 0, 16);
    title_ehf_load(16, 2, 0, 20);
    bgm_play(5);
}

/* the "enter your name" message at the bottom */
void tname_win_init(void)
{
    MenuWidget *w = &title.win[0];
    MenuItemDef *d = &w->def;

    twin_init(w);
    d->open_anim = 1;
    d->close_anim = 1;
    d->x = 40;
    d->y = 224;
    d->w = 401;
    d->h = 46;
    d->pad_l = 6;
    d->pad_r = 6;
    d->pad_t = 5;
    d->pad_b = 5;
    d->color = 0xFFFFFFFF;
    d->text_color = 0xFF000000;
    d->style = 0;
    d->title = 0;
    d->text = title_msg(69);
    w->style = 0;
    w->no_input = 0;
    w->no_cancel = 0;
    w->cancel_blocked = 0;
    w->keep_open = 0;
    w->f0_4 = 0;
    w->cancel_resets = 0;
    twin_open(w);
}

/* a prompt window with message `id`; kind 1 YES/NO, 2 OK/CANCEL, 3 OK; `no` puts the cursor on NO */
void title_prompt(int id, int kind, int no)
{
    MenuWidget *w = &title.win[0];
    MenuItemDef *d = &w->def;

    twin_init(w);
    d->style = 2;
    d->open_anim = 2;
    d->close_anim = 2;
    d->auto_size = 1;
    d->pad_l = 10;
    d->pad_r = 10;
    d->pad_t = 8;
    d->font_size = 12;
    d->x = 60;
    d->y = 100;
    d->w = 360;
    d->h = 104;
    d->color = 0xFFFFFFFF;
    d->pad_b = 0;
    d->title = 0;
    wsprintf(title.msg_buf, title_msg(id), 0x2122, 0x2122);
    d->text = title.msg_buf;
    w->no_input = 0;
    w->no_cancel = 0;
    w->cancel_blocked = 0;
    w->keep_open = 0;
    w->f0_4 = 0;
    w->cancel_resets = 1;
    if (kind == 1) {
        w->style = 0;
        w->count = 0;
        w->buttons = 3;
        if (no == 0)
            w->button = 3;
        else
            w->button = 2;
        w->initial = 3;
        w->arrow_off = 256;
    } else if (kind == 2) {
        w->style = 0;
        w->count = 0;
        w->buttons = 2;
        if (no == 0)
            w->button = 1;
        else
            w->button = 4;
        w->initial = 4;
        w->arrow_off = 256;
    } else if (kind == 3) {
        w->initial = 1;
        w->style = 0;
        w->count = 0;
        w->buttons = 1;
        w->button = 1;
        w->arrow_off = 256;
        w->cancel_blocked = 1;
    }
    twin_open(w);
}

void tname_update_input(void)
{
    Title *const t = &title;
    MenuWidget *const w = &title.win[0];

    switch (title.step) {
    case 0:
        tname_setup();
        title_goto_fade(0, 0xFFF, 0xFFF);
        t->next_step = 2;
        t->step = 1;
        break;
    case 1:
        break;
    case 2:
        if (title_ehf_frame(16) != 20)
            return;
        title_ehf_anim(16, 2, 21, 29);
        t->step++;
        break;
    case 3:
        if (title_ehf_frame(13) == 16) {
            title_ehf_anim(13, 2, 17, 96);
            t->step++;
        }
        break;
    case 4:
        menu_update(w);
        if (w->state >= 3) {
            switch (tname_button()) {
            case 0:
                t->step = 5;
                menu_finish(w);
                menu_update(w);
                break;
            case 1:
                title_goto_fade(1, 0xFFF, 2);
                t->step = 9;
                break;
            case 2:
                title_goto_fade(1, 5, 0);
                t->step = 9;
                title_ehf_anim(16, 2, 30, 35);
                break;
            }
        }
        break;
    case 5:
        menu_update(w);
        if (w->state == 6) {
            if (tname_valid() != 0) {
                if (sysvar_get_a94() != 0)
                    title_prompt(72, 1, 0);
                else
                    title_prompt(71, 1, 0);
                t->step = 6;
            } else {
                title_prompt(15, 3, 0);
                t->step = 7;
            }
            se_play(0x621A);
        }
        break;
    case 6:
        switch (menu_update(w)) {
        case -2:
        case 3:
            sysstate_checksum();
            title_goto_fade(1, 9, 0);
            t->step = 9;
            title_ehf_anim(16, 2, 30, 35);
            break;
        case 2:
            title_goto_fade(1, 0xFFF, 3);
            t->step = 9;
            title_ehf_anim(16, 2, 30, 35);
            break;
        }
        break;
    case 7:
        switch (menu_update(w)) {
        case -2:
        case 1:
            t->step = 8;
            break;
        }
        break;
    case 8:
        menu_update(w);
        if (w->state == 6) {
            tname_win_init();
            t->step = 4;
        }
        break;
    case 9:
        if (title_ehf_frame(16) == 34)
            title_ehf_anim(16, 0, 35, 35);
        break;
    case 10:
        break;
    }
}

/* START: done (0), accept: rename (1), cancel (2); -1 nothing */
int tname_button(void)
{
    unsigned int trig = pad_get()->b.trig;

    if (trig & 8) {
        se_play(0x6213);
        return 0;
    }
    if (trig & region_pick_value(0)) {
        se_play(0x6213);
        return 1;
    }
    if (trig & region_pick_value(1)) {
        se_play(0x6214);
        return 2;
    }
    return -1;
}

void *tname_draw(void *list)
{
    Title *const t = &title;

    if (title.sub == 1) {
        list = title_ehf_draw(list, 6);
        list = title_ehf_draw(list, 5);
        list = title_ehf_draw(list, 16);
        if (t->step == 4) {
            list = title_sprite(list, 1, 116, 175, 197);
            font_set_size(15, 15);
            font_set_color(0xFF000000);
            font_set_flag4(1);
            list = title_text_frame(list, 194, 201, 108, sysstate_get_name(), 2);
        }
        if (t->step >= 3) {
            list = title_ehf_draw(list, 13);
            list = title_screen_label(list);
        }
        switch (t->step) {
        case 0:
        case 1:
        case 2:
        case 3:
            break;
        case 4:
        case 5:
        case 9:
        case 10:
            list = twin_draw_center(list, &t->win[0], 0);
            break;
        case 6:
        case 7:
        case 8:
            list = twin_draw_center(list, &t->win[0], 1);
            break;
        }
    }
    return list;
}

void tname_osk_start(void)
{
    unsigned short *desc = table_get_entry(ehp_find(title_pack(), title_res_name(0), 0), 14);

    tosk_open(sysstate_get_name(), desc, 9, tname_osk_done);
    title.step = 3;
}

void tname_osk_update(void)
{
    switch (title.step) {
    case 0:
        tname_osk_start();
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        break;
    }
}

void tname_osk_done(unsigned short *text, int r)
{
    Title *const t = &title;

    switch (title.step) {
    case 3:
        t->step = 4;
        break;
    case 0:
    case 1:
    case 2:
    case 4:
    case 5:
    case 6:
    case 7:
        break;
    }
}

/* draws the keyboard over the scene; back to the name screen when it is closed */
void tname_osk_draw(void)
{
    Title *const t = &title;

    if (title.sub == 2 && t->step >= 3 && tosk_update(4)) {
        if (t->step == 4) {
            t->step = 0;
            t->sub = 1;
        } else {
            t->step = 0;
        }
    }
}

/* writes the save; 1 when done */
int tname_save(void)
{
    int r = 0;
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
        r = 1;
        break;
    }
    t->prev_sub = t->sub;
    return r;
}

/* the name has something else than spaces */
int tname_valid(void)
{
    unsigned short name[64];
    int i;
    unsigned short *p;

    memset(name, 0, sizeof(name));
    memcpy(name, sysstate_get_name(), 16);
    if (name[0] != 0) {
        for (i = 0, p = name; i < 8; i++, p++) {
            if (*p != 0x3000 && *p != ' ' && *p != 0)
                return 1;
        }
    }
    return 0;
}
