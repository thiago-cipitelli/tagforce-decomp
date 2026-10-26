// @range 0x088162CC-0x08816E5C
// @rodata 0x0882AFD8-0x0882B028
/* Free duel: drawing dispatch, the vs CPU screens, the cursor and regulation sub-screens. */
#include "libtitle.h"

void *tfd_icon_setup(void *list, MenuWidget *w, int x, int y, int cw, int ch);
void *tfd_entry_draw(void *list);
void *tfd_vscpu_draw(void *list);
void *tfd_cursor_draw(void *list);
void *tfd_regulation_draw(void *list);
void *tfd_duelist_draw2(void *list);
void *tfd_adhoctop_handler2(void *list);
void *tfd_lobby_draw2(void *list);
void *tfd_vscpu_entry_draw2(void *list);
void tfd_vscpu_panel_init(TPanel *p);
void tfd_show_msgbox(MenuWidget *w, int msg);
void tfd_vscpu_init(void);
void tfd_clear(void);
int tfd_evt_handler(int a, int b);
int tfd_config_draw_line(void);
void *tfd_config_draw_text(void *list, int a, int b);
void *tfd_config_draw_bg(void *list);
void *tfd_show_duelinfo(void *list, int i, int x, int y, int w, int h);
void *tfd_sprite_anim(void *list, int i, int x, int y, int w, int h);
void *tfd_cursor_sprite(void *list, int i, int x, int y, int w, int h);
void *tframe_cursor(void *list, unsigned int color, int x, int y, int w, int h);
void *func_08823248(void *list, MenuWidget *w, int x, int y, int cw, int ch);

void *tfd_draw(void *list)
{
    switch (title.sub) {
    case 0:
        list = tfd_entry_draw(list);
        break;
    case 1:
    case 2:
    case 3:
    case 10:
    case 11:
    case 12:
    case 13:
    case 15:
        break;
    case 4:
        list = tfd_vscpu_draw(list);
        break;
    case 5:
        list = tfd_cursor_draw(list);
        break;
    case 6:
    case 7:
        list = tfd_regulation_draw(list);
        break;
    case 8:
        list = tfd_duelist_draw2(list);
        break;
    case 9:
        list = tfd_adhoctop_handler2(list);
        break;
    case 14:
        list = tfd_lobby_draw2(list);
        break;
    case 16:
    case 17:
    case 18:
    case 19:
        list = tfd_vscpu_entry_draw2(list);
        break;
    }
    return list;
}

void tfd_vscpu_init(void)
{
    TPanel *const p = &title.panel;
    MenuWidget *w = &p->mw;

    memset(&title.duel.opts, 0, sizeof(TDuelOpts));
    title.step = 0;
    title.step2 = 0;
    title.frame = 0;
    title.u86c = 0;
    title.u86d = 2;
    title.color0 = 0xFFFFFFFF;
    title.color1 = 0xFFFFFFFF;
    tfd_vscpu_panel_init(p);
    w->cancel_blocked = 1;
    w->accept_blocked = 1;
    w->draw_cursor = (MenuDrawCursorFn)tpanel_cursor;
    tfd_show_msgbox(&title.win[1], 0x72);
    title_ehf_load(7, 2, 0, 0x1E0);
    title_ehf_load(14, 2, 0, 0x11);
}

void tfd_vscpu_panel_init(TPanel *p)
{
    MenuWidget *w = &p->mw;
    TBox *b = &p->box;
    MenuItemDef *def = &w->def;

    tpanel_init(p, title.heap);
    p->f0 = 0;
    p->n = 1;
    b->x = 4;
    b->y = 52;
    b->w = 305;
    b->h = 42;
    def->x = b->x;
    def->y = b->y;
    def->w = b->w;
    def->h = b->h;
    def->pad_l = 0;
    def->pad_r = 0;
    def->pad_t = 0;
    def->pad_b = 0;
    def->color = 0xFFFFFF;
    w->count = 8;
    w->rows = 5;
    w->draw_cursor = (MenuDrawCursorFn)tfd_icon_setup;
    w->draw_cursor_post = 0;
    w->item_w = b->w - (def->pad_l + def->pad_r);
    w->item_h = b->h + 1;
    w->item_x = 0;
    w->item_y = 0;
    w->cursor_w = w->item_w;
    w->cursor_h = b->y - 8 - w->item_y * 2;
    b->count = w->count;
    b->n = w->rows;
    b->text = 0;
    b->draw = 0;
    w->accept_blocked = 1;
}

void tfd_show_msgbox(MenuWidget *w, int msg)
{
    MenuItemDef *d = &w->def;

    twin_init(w);
    d->style = 2;
    d->open_anim = 2;
    d->close_anim = 2;
    d->auto_size = 1;
    d->pad_l = 10;
    d->pad_t = 8;
    d->pad_r = 10;
    d->pad_b = 0;
    d->font_size = 12;
    d->x = 60;
    d->y = 100;
    d->w = 360;
    d->h = 80;
    d->color = 0xFFFFFFFF;
    d->title = 0;
    d->text = title_text(msg, 12);
    w->no_input = 0;
    w->no_cancel = 0;
    w->cancel_blocked = 0;
    w->keep_open = 0;
    w->f0_4 = 0;
    w->cancel_resets = 0;
    w->count = 0;
    w->rows = 0;
    w->button = 1;
    w->style = 0;
    w->buttons = 1;
    w->initial = 1;
    w->cancel_blocked = 1;
    twin_open(w);
}

void tfd_vscpu_update(void)
{
    TPanel *const p = &title.panel;
    Title *t = &title;
    MenuWidget *w = &p->mw;
    int r;
    TBox *b = &p->box;
    TDuelCfg *d = &title.duel;
    TFdList *l = &d->list;
    TDuelOpts *o = &d->opts;
    int c;
    int m;
    MenuWidget *win = &title.win[1];

    switch (title.step) {
    case 0:
        t->step = 1;
    case 1:
        tfd_vscpu_init();
        tfd_clear();
        t->step = 2;
        title_fade_out(0);
        break;
    case 2:
    case 3:
        if (title_fade_state() == 0)
            t->step = 4;
        break;
    case 4:
        title_ehf_load(14, 2, 0, 0x11);
        t->step = 5;
        break;
    case 5:
        if (title_ehf_frame(14) == 0x11) {
            title_ehf_anim(14, 2, 0x12, 0x35);
            t->step = 11;
        }
        break;
    case 11:
        r = tfd_config_draw_line();
        switch (r) {
        case -1:
            *(TFdSeat *)o = l->seat[b->pos];
            break;
        case -2:
            t->step = 0x1B;
            break;
        case -3:
            break;
        default:
            tframe_cursor_mode(1);
            if (r == b->count + t->u86c) {
                t->step = 0x1D;
                t->next_sub = 5;
            } else {
                switch (title.duel.list.seat[b->pos].kind) {
                case 0:
                case 2:
                    t->step = 12;
                    t->next_sub = 7;
                    tfd_evt_handler(4, 0);
                    break;
                case 1:
                    c = title.duel.list.seat[b->pos].card;
                    d->card = c;
                    if (inv_get_bit7_default1(c)) {
                        d->card_ok = 0;
                        c = 0;
                    } else {
                        d->card_ok = 1;
                    }
                    tfd_evt_handler(5, c);
                    t->step = 12;
                    t->next_sub = 0x13;
                    break;
                case 3:
                case 4:
                    t->step = 12;
                    t->next_sub = 0x10;
                    break;
                }
                *(TFdSeat *)&d->opts = l->seat[b->pos];
                title.duel.u003 = b->pos;
            }
            break;
        }
        break;
    case 12:
        if (d->anim < 90) {
            d->anim++;
        } else {
            m = 0x72;
            tframe_cursor_mode(0);
            t->step = 0x15;
            d->anim = 0;
            se_play(0x621A);
            if (t->next_sub == 0x13)
                m = 0x77;
            tfd_show_msgbox(win, m);
        }
        break;
    case 13:
        break;
    case 0x14:
    case 0x15:
        d->anim = 0;
        tframe_cursor_mode(0);
        if (menu_update(win) != -1) {
            short cur;
            short scroll;

            switch (t->step) {
            case 0x14:
                t->step = 0x16;
                break;
            case 0x15:
                tfd_evt_handler(9, 0);
                t->step = 11;
                break;
            }
            scroll = w->scroll;
            cur = w->cursor;
            tfd_vscpu_panel_init(p);
            w->cursor = cur;
            w->scroll = scroll;
        }
        break;
    case 0x16:
        break;
    case 0x1B:
        w->draw_cursor = (MenuDrawCursorFn)tpanel_cursor;
        d->anim = 0;
        title_goto_fade(1, 0xFFF, 0xFFF);
        t->next_step = 0x1F;
        t->step = 0x1E;
        break;
    case 0x1D:
        d->anim = 0;
        title_goto_fade(1, 0xFFF, t->next_sub);
        t->step = 0x1E;
        break;
    case 0x1E:
        break;
    case 0x1F:
        tfd_evt_handler(2, 0);
        t->step = 0x21;
        break;
    case 0x21:
        break;
    }
}

void *tfd_vscpu_draw(void *list)
{
    TPanel *const p = &title.panel;
    Title *t = &title;
    MenuWidget *w = &p->mw;
    MenuWidget *win = &title.win[1];
    TDuelOpts *o = &title.duel.opts;
    int n = 0;
    TBoxDrawFn fn = 0;

    list = title_ehf_draw(list, 7);
    if (title.u86c == 0 && t->step >= 11)
        w->draw_cursor = func_08823248;
    else
        w->draw_cursor = (MenuDrawCursorFn)tpanel_cursor;
    if (t->step >= 5) {
        list = title_ehf_draw(list, 14);
        list = title_screen_label(list);
    }
    list = tbox_header(list, tfd_show_duelinfo);
    if (o->side) {
        if (o->kind == 0)
            n = 5;
        else
            n = 1;
        fn = tfd_cursor_sprite;
    }
    list = tbox_side(list, 0x14, n, fn);
    list = tbox_corner(list, tfd_sprite_anim);
    list = tfd_config_draw_text(list, 8, 4);
    list = tfd_config_draw_bg(list);
    list = tpanel_draw(list, p);
    if (t->u86c)
        list = tframe_cursor(list, 0xFFFFFFFF, 0x147, 0xF2, 0x97, 0x17);
    if ((unsigned int)(t->step - 0x14) < 2)
        list = twin_draw_center(list, win, 1);
    return list;
}
