// @range 0x088162CC-0x08818A88
// @rodata 0x0882AFD8-0x0882B118
// @asm 0x08817F98-0x08818480
/* Free duel: drawing dispatch, the vs CPU screens, the cursor and regulation sub-screens. */
#include "libtitle.h"

void *tfd_icon_setup(void *list, MenuWidget *w, int x, int y, int cw, int ch);
void *tfd_entry_draw(void *list);
void *tfd_vscpu_draw(void *list);
void tfd_cursor_init(void);
void tfd_cursor_handler(void);
void *tfd_cursor_icon(void *list);
int tfd_sprite_slidein(void);
void tfd_regulation_init(void);
void tfd_regulation_handler(void);
int tfd_regulation_input(void);
int tfd_regulation_select();
void text_ext_reset(void);
int net_get_state(void);
void *tfd_rule_init(void *list);
void *tfd_rule_draw(void *list, int x, int y, int w, int h, int lit);
void *tfd_icon_draw(void *list, int id, int x, int y);
void *title_text_frame(void *list, int x, int y, int w, unsigned short *text, int align);
void tpt_set_name(short i, int kind, int type2, int src, int deck, void *name);
void tpt_set_mode(int mode);
void *tbox_draw(void *list, TBox *b);
void *tfd_rule_item(void *list, int i, int x, int y, int w, int h);
void net_get_host_info(TFdImp *out);
void *tfd_cursor_item(void *dl, int index, int x, int y);
unsigned short *sysstate_get_name(void);
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
void *tdl_cursor(void *list, MenuWidget *w, int x, int y, int cw, int ch);

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
        w->draw_cursor = tdl_cursor;
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

void tfd_cursor_init(void)
{
    TDuelCfg *d = &title.duel;
    MenuWidget *w = &title.win[0];
    TDuelOpts *o = &d->opts;

    title.step = 0;
    title.step2 = 1;
    d->opts.kind = 0;
    title.frame = 0;
    title.u86c = 0;
    title.u86d = 2;
    title.color0 = 0xFFFFFFFF;
    title.color1 = 0xFFFFFFFF;
    twin_init(w);
    w->def.x = 4;
    w->def.y = 98;
    w->def.h = 132;
    w->def.w = 320;
    w->def.pad_l = 1;
    w->def.pad_r = 2;
    w->item_w = w->def.w - (w->def.pad_l + w->def.pad_r);
    w->def.pad_t = 18;
    w->def.color = 0xFFFFFFFF;
    w->rows = 10;
    w->count = 2;
    w->no_input = 0;
    w->no_cancel = 1;
    w->keep_open = 1;
    w->draw_cursor = (MenuDrawCursorFn)tpanel_cursor;
    w->item_h = 14;
    w->cursor_h = 14;
    w->cursor_w = w->item_w - 8;
    w->draw_item = tfd_cursor_item;
    w->def.pad_b = 0;
    w->def.open_anim = 0;
    w->def.close_anim = 0;
    w->draw_cursor_post = 0;
    w->item_x = 0;
    w->item_y = 0;
    twin_open(w);
    d->u001 = 0;
    o->name = sysstate_get_name();
    o->side = 2;
    title_ehf_load(8, 2, 0, 0x1E0);
    title_ehf_load(14, 2, 0, 0x11);
}

void tfd_cursor_update(void)
{
    Title *t = &title;
    MenuWidget *w = &title.win[0];

    switch (title.step) {
    case 0:
        t->step = 1;
    case 1:
        tfd_cursor_init();
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
        w->draw_cursor = 0;
        tfd_cursor_handler();
        switch (t->step2) {
        case 0:
            w->count = 1;
            break;
        case 1:
            w->count = 2;
            break;
        case 2:
            w->count = 2;
            break;
        case 3:
            w->count = 2;
            break;
        case 4:
            w->count = 5;
            break;
        case 5:
            w->count = 5;
            break;
        case 6:
            w->count = 0;
            break;
        case 7:
        case 8:
        case 9:
            break;
        }
        break;
    case 0x1B:
        w->draw_cursor = (MenuDrawCursorFn)tpanel_cursor;
        title_goto_fade(1, 0xFFF, 4);
        t->step = 0x1E;
        break;
    case 0x1D:
        title_goto_fade(1, 0xFFF, t->next_sub);
        t->step = 0x1E;
        tfd_evt_handler(3, 0);
        break;
    case 0x1E:
    case 0x1F:
        if (title_fade_state() == 0)
            t->step = 0x21;
        break;
    case 0x21:
        break;
    }
}

void tfd_cursor_handler(void)
{
    Title *t = &title;
    MenuWidget *w = &title.win[0];
    TDuelOpts *o = &title.duel.opts;
    int r;

    r = menu_update(w);
    switch (r) {
    case -1:
        break;
    case -2:
        if (t->step2 > 1) {
            if (t->step2 > 3) {
                if (!o->b7)
                    t->step2 = 3;
                else
                    t->step2--;
            } else {
                t->step2--;
            }
            w->cancelled = w->accepted = 0;
            w->scroll = 0;
            w->cursor = 0;
            switch (t->step2) {
            case 0:
                w->cursor = o->kind;
                break;
            case 1:
                w->cursor = o->tag;
                break;
            case 2:
                w->cursor = o->swap;
                break;
            case 3:
                w->cursor = o->b7;
                break;
            case 4:
                w->cursor = o->lp;
                break;
            case 5:
                w->cursor = o->time;
                break;
            case 6:
            case 7:
            case 8:
                break;
            }
        } else {
            t->step = 0x1B;
            t->step2 = 1;
        }
        break;
    case -3:
        break;
    default:
        switch (t->step2) {
        case 0:
            o->kind = r;
            switch (r) {
            case 0:
                t->step2++;
                break;
            case 1:
                t->step2 = 6;
                break;
            case 2:
                t->step2 = 6;
                break;
            case 3:
                t->step2 = 6;
                break;
            }
            break;
        case 1:
            t->step2++;
            o->tag = r;
            break;
        case 2:
            t->step2++;
            o->swap = r;
            break;
        case 3:
            switch (r) {
            case 0:
                o->b7 = 0;
                o->lp = 2;
                o->time = 0;
                t->step2 = 6;
                break;
            case 1:
                o->b7 = 1;
                t->step2++;
                break;
            }
            break;
        case 4:
            t->step2++;
            o->lp = r;
            break;
        case 5:
            o->time = r;
            t->step2 = 6;
            break;
        case 7:
            t->step = 0x1D;
            t->next_sub = 6;
            break;
        case 8:
            t->step = 0x1D;
            t->next_sub = 6;
            break;
        case 9:
            t->step = 0x1D;
            t->next_sub = 6;
            break;
        case 6:
            t->step = 0x1D;
            tframe_cursor_mode(1);
            switch (o->kind) {
            case 0:
            case 2:
                t->next_sub = 6;
                break;
            case 1:
                t->next_sub = 0x10;
                break;
            case 3:
                t->next_sub = 0x10;
                break;
            case 4:
                t->next_sub = 0x10;
                break;
            }
            break;
        }
        w->cancelled = w->accepted = 0;
        w->scroll = 0;
        w->cursor = 0;
        break;
    }
}

void *tfd_cursor_draw(void *list)
{
    Title *t = &title;
    MenuWidget *w = &title.win[0];
    TDuelOpts *o = &title.duel.opts;
    int n = 0;

    list = title_ehf_draw(list, 8);
    if (title.step >= 5) {
        list = title_ehf_draw(list, 14);
        list = title_screen_label(list);
    }
    list = tbox_header(list, tfd_show_duelinfo);
    list = tbox_corner(list, tfd_sprite_anim);
    list = tfd_config_draw_text(list, 1, 9);
    list = tbox_help(list, tfd_sprite_slidein(), 12);
    menu_draw(&list, w);
    if (t->step2 != 6)
        list = tfd_cursor_icon(list);
    if (t->step2 != 0) {
        if (o->kind == 0) {
            n = (short)(int)t->step2;
            if (n <= 3)
                n -= 1;
            else
                n -= 2;
        } else {
            n = 1;
        }
    }
    list = tbox_side(list, 0x14, n, tfd_cursor_sprite);
    if (t->step2 == 6)
        list = tframe_cursor(list, 0xFFFFFFFF, 0x147, 0xF2, 0x97, 0x17);
    return list;
}

void *tfd_cursor_icon(void *list)
{
    Title *t = &title;
    int id = 0;

    switch (title.step2) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        id = t->step2 + 0x4C;
        break;
    case 6:
    case 7:
    case 8:
        break;
    }
    return tbox_small(list, id, 9, 0x67, 0x135, 0x10);
}

void *tfd_cursor_item(void *list, int index, int x, int y)
{
    int k = 0;

    switch (title.step2) {
    case 0:
        k = 0x52;
        break;
    case 1:
        k = 0x58;
        break;
    case 2:
        k = 0x5A;
        break;
    case 3:
        k = 0x5C;
        break;
    case 4:
        k = 0x68;
        break;
    case 5:
        k = 0x6D;
        break;
    case 6:
    case 7:
    case 8:
        break;
    }
    if (k != 0) {
        k += (unsigned short)index;
        list = title_msg_font(list, x, y, 0x100, 12, k, 0, 15, 15, 0xFF002020);
    }
    return list;
}

void *tfd_cursor_sprite(void *list, int i, int x, int y, int w, int h)
{
    TDuelOpts *o = &title.duel.opts;
    int n = 0;

    if (o->kind == 0) {
        switch ((unsigned short)i) {
        case 0:
            n = o->tag + 0x58;
            break;
        case 1:
            n = o->swap + 0x31;
            break;
        case 2:
            n = o->lp + 0x34;
            break;
        case 3:
            n = o->time + 0x39;
            break;
        }
    } else if (o->kind == 1) {
        n = 0x17;
    } else {
        n = o->kind + 0x51;
    }
    if (n != 0)
        list = title_msg_font(list, x, y, w, 12, n, 2, 13, 13, 0xFF002020);
    return list;
}

void tfd_regulation_init(void)
{
    TBox *b = &title.box3;
    TDuelCfg *d = &title.duel;

    title.step = 0;
    title.step2 = 0;
    title.frame = 0;
    title.u86c = 0;
    title.u86d = 2;
    title.color0 = 0xFFFFFFFF;
    title.color1 = 0xFFFFFFFF;
    tbox_init(b);
    b->x = 4;
    b->ix = b->x + 3;
    b->y = 98;
    b->iy = b->y + 23;
    b->w = 320;
    b->h = 132;
    b->frame = 0x54;
    b->style = 9;
    b->f1 = 1;
    b->no_cells = 1;
    b->f4 = 1;
    b->no_scroll = 1;
    b->iw = 154;
    b->ih = 25;
    b->f6 = 1;
    b->n = 8;
    b->count = b->n;
    b->cols = 2;
    b->draw = tfd_rule_item;
    b->pos = 0;
    b->cursor = 0;
    twin_help_init(&title.win[0], 14, tfd_sprite_slidein(), 12);
    tfd_show_msgbox(&title.win[1], 0x72);
    if (title.sub == 6) {
        d->u001 = 0;
    } else {
        net_get_host_info(&d->host);
        d->opts.name = d->host.name;
        d->u001 = 1;
    }
    title_ehf_load(14, 2, 0, 0x11);
}

void tfd_regulation_update(void)
{
    Title *t = &title;
    MenuWidget *win = &title.win[1];

    switch (title.step) {
    case 0:
        t->step = 1;
    case 1:
        tfd_regulation_init();
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
        tfd_regulation_handler();
        break;
    case 0x14:
        tframe_cursor_mode(0);
        if (menu_update(win) != -1)
            t->step = 0x16;
        break;
    case 0x1B:
        title_goto_fade(1, 0xFFF, 3);
        t->step = 0x1E;
        break;
    case 0x1C:
        break;
    case 0x1D:
        title_goto_fade(1, 0xFFF, t->next_sub);
        t->step2 = 0;
        t->step = 0x1E;
        break;
    case 0x1E:
    case 0x1F:
        if (title_fade_state() == 0)
            t->step = 0x21;
        break;
    case 0x21:
        break;
    }
}

void tfd_regulation_handler(void)
{
    Title *t = &title;
    TDuelOpts *o = &title.duel.opts;
    int r;

    switch (r = tfd_regulation_input()) {
    case -3:
        break;
    case -2:
        t->step = 0x1B;
        break;
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        tfd_regulation_select(r);
        return;
    case 9:
        if (tfd_regulation_select(r)) {
            if (o->kind == 0) {
                t->step = 0x1C;
                tframe_cursor_mode(1);
                if (o->tag == 0)
                    t->next_sub = 8;
                else
                    t->next_sub = 9;
                text_ext_reset();
            } else {
                t->step = 0x1D;
                t->next_sub = 14;
            }
        }
        break;
    }
}

#ifdef NON_MATCHING /* o original mantém a constante 1 em $a0 (lim = 1 e o teste n < 1 com slt); o mwcc a propaga (li a3,1 / blez) */
/* lim stays whatever the caller left in $a3 unless the kind sets it (the original reads an uninitialized value) */
int tfd_regulation_select(int i, int unused1, int unused2, int lim)
{
    TDuelCfg *d = &title.duel;
    TFdList *l = &d->list;
    int n = 0;
    int k;

    if (d->opts.kind == 0)
        lim = 1;
    else if (d->opts.kind == 2)
        lim = 1;
    for (k = 0; k < 8; k++) {
        if (title.duel.list.deck[k].kind != 0)
            n++;
    }
    if (i < 9) {
        TFdDeck *p = &l->deck[i];

        if (n < lim) {
            if (p->on) {
                tfd_evt_handler(6, i);
                return 1;
            }
            return 0;
        }
    } else if (i == 9) {
        if (n >= 1) {
            se_play(0x6213);
            tfd_evt_handler(7, 0);
            title.duel.list.n = n;
            return 1;
        }
        se_play(0x6215);
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* o se_play(0x6215) do original desvia para o bloco comum do return -1 (b; nop); o mwcc devolve -1 ali mesmo (b epílogo; li -1) */
/* the player grid of the regulation screen (2 columns) and the duel start button below it */
int tfd_regulation_input(void)
{
    int last;
    Title *t = &title;
    TBox *b = &title.box3;
    int cols;
    PadState *pad;
    int rows;
    unsigned int rep;
    unsigned int trig;

    cols = b->cols;
    last = 0;
    pad = pad_get();
    rep = pad->b.rep;
    trig = pad->b.trig;
    if (title.sub == 6) {
        if (b->count > 0)
            last = b->count - 1;
        if (cols == 0)
            cols = 1;
        rows = (b->n + b->n % 2) / cols;
        if ((trig & 0x20) || (rep & 0x20)) {
            if (t->u86c) {
                if (trig & 0x20) {
                    t->u86c = 0;
                    b->pos = b->pos % rows;
                    se_play(0x6212);
                }
            } else {
                if (b->pos < rows) {
                    b->pos += rows;
                    if (b->pos > last)
                        b->pos = last;
                } else {
                    t->u86c++;
                }
                se_play(0x6212);
            }
        } else if ((trig & 0x80) || (rep & 0x80)) {
            if (t->u86c) {
                b->pos = rows + b->pos % rows;
                t->u86c = 0;
                if (b->pos > last)
                    b->pos = last;
                se_play(0x6212);
            } else if (rows <= b->pos) {
                b->pos -= rows;
                se_play(0x6212);
            } else if (trig & 0x80) {
                t->u86c++;
                se_play(0x6212);
            }
        }
        if (t->u86c == 0) {
            if ((trig & 0x40) || (rep & 0x40)) {
                if (b->pos < last) {
                    b->pos++;
                    se_play(0x6212);
                } else if (trig & 0x40) {
                    b->pos = 0;
                    se_play(0x6212);
                }
            } else if ((trig & 0x10) || (rep & 0x10)) {
                if (b->pos != 0) {
                    b->pos--;
                    se_play(0x6212);
                } else if (trig & 0x10) {
                    b->pos = last;
                    se_play(0x6212);
                }
            }
        }
        b->cursor = b->pos;
        if (b->cursor > b->n - 1)
            b->cursor = b->n - 1;
        if (rep & region_pick_value(0)) {
            if (t->u86c)
                return 9;
            if (title.duel.list.deck[b->pos].kind != 0) {
                se_play(0x6214);
                tfd_evt_handler(6, b->pos);
                return -1;
            }
            if (title.duel.list.deck[b->pos].on == 0) {
                se_play(0x6215);
            } else {
                se_play(0x6213);
                return b->pos;
            }
        } else {
            if (rep & region_pick_value(1)) {
                se_play(0x6214);
                return -2;
            }
            return -1;
        }
    } else if (net_get_state() == 0x15 && (rep & region_pick_value(1))) {
        se_play(0x6214);
        tfd_evt_handler(2, 0);
        return -2;
    }
    return -1;
}
#endif

void *tfd_regulation_draw(void *list)
{
    Title *t = &title;
    TBox *b = &title.box3;
    TDuelCfg *d = &title.duel;
    MenuWidget *w = &title.win[0];
    TDuelOpts *o = &d->opts;
    MenuWidget *win = &title.win[1];

    if (d->u001)
        list = title_ehf_draw(list, 7);
    else
        list = title_ehf_draw(list, 8);
    if (t->step >= 5) {
        list = title_ehf_draw(list, 14);
        list = title_screen_label(list);
    }
    list = tbox_header(list, tfd_show_duelinfo);
    if (t->sub == 6)
        list = tbox_corner(list, tfd_sprite_anim);
    list = tfd_config_draw_text(list, 1, 9);
    menu_draw(&list, w);
    if (t->sub == 6) {
        if (t->u86c != 0 || t->step <= 10)
            b->no_cursor = 1;
        else
            b->no_cursor = 0;
        list = tbox_draw(list, b);
        list = tfd_rule_init(list);
    }
    list = tbox_side(list, 0x14, o->kind == 0 ? 5 : 1, tfd_cursor_sprite);
    if (t->u86c)
        list = tframe_cursor(list, 0xFFFFFFFF, 0x147, 0xF2, 0x97, 0x17);
    if (t->step == 0x14)
        list = twin_draw_center(list, win, 1);
    return list;
}

void *tfd_rule_init(void *list)
{
    return tbox_small(list, 0x74, 9, 0x67, 0x135, 0x10);
}

/* the flag byte of a TFdDeck (+0x7C), reached through its own pointer in tfd_rule_item */
typedef struct TFdDeckFlags {
    unsigned char dk : 2;
    unsigned char kind : 3;
    unsigned char side : 2;
    unsigned char on : 1;
} TFdDeckFlags;

/* a cell of the regulation grid: a player of the list */
void *tfd_rule_item(void *list, int i, int x, int y, int w, int h)
{
    unsigned short n = i;
    int tx = x + 0x34;
    int ty = y + 6;
    int lit = 0;
    int id = n + 0x5F;
    Title *t = &title;
    unsigned short *name = title.duel.list.deck[n].name;
    TFdDeckFlags *f = (TFdDeckFlags *)((char *)t + n * sizeof(TFdDeck) + 0xA8C);

    if (f->on)
        lit = 1;
    list = tfd_rule_draw(list, x, y, w, h, lit);
    list = title_msg_font(list, x + 6, y + 5, 0x100, 12, id, 0, 15, 15, 0xFFFFFFFF);
    if (name) {
        font_set_size(13, 13);
        font_set_color(0xFF004040);
        font_set_flag4(1);
        list = title_text_frame(list, tx, ty, 0x61, name, 2);
    }
    if (f->kind)
        list = tfd_icon_draw(list, 5, x + 0x1D, y + 4);
    return list;
}

void *tfd_rule_draw(void *list, int x, int y, int w, int h, int lit)
{
    short u = title_sprites2[0x49].u;
    short v = title_sprites2[0x49].v;
    short tw = title_sprites2[0x49].tw;
    short th = title_sprites2[0x49].th;
    short a = title_sprites2[0x49].a;
    short b = title_sprites2[0x49].b;
    short sx = x;
    short sy = y;

    list = tmenu_gfx_setup(list, 0);
    if (lit)
        list = dl_color(list, 0xFFFFFFFF);
    else
        list = dl_color(list, 0xA0FFFFFF);
    list = gim_set_texture(list, ehp_find(title_pack(), title_sprite_gim(title_sprites2, 0x4A), 0));
    list = draw_set_offsets(list, tw, th, tw, th);
    return draw_sprite(list, sx, sy, 0, u, v, a, b);
}

void tfd_rule_icon(int net)
{
    TDuelCfg *d = &title.duel;

    memset(d->ready, 0, sizeof(d->ready));
    title.step = 0;
    title.step2 = 0;
    title.frame = 0;
    title.u86c = 0;
    title.u86d = 2;
    d->u004b6 = 0;
    if (net)
        d->u004b6 = 1;
    d->u000 = d->u001;
    if (!net) {
        title_ehf_load(10, 2, 0, 0x1DF);
        title_ehf_load(12, 2, 0, 0);
        title_ehf_load(15, 2, 0, 0x11);
    }
}

void tfd_rule_sprite(int mode)
{
    int i;

    for (i = 0; i < 4; i++)
        tpt_set_name(i, 0, i, 3, 0, 0);
    tpt_set_mode(mode);
}
