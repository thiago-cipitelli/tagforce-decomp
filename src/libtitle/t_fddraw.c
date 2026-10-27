// @range 0x0881E0CC-0x0881F02C
// @data 0x0882F384-0x0882F398
// @rodata 0x0882B344-0x0882B40C
// @asm 0x0881EA4C-0x0881ECB4
/* Free duel setup, the option list: the player rows, the box titles, the icons, the character pick
   (libcharalist) and the entry message. */
#include "libtitle.h"

void *tpt_sprite_def(void *list, int i, short x, short y);
void font_set_size(int w, int h);
unsigned int font_set_color(unsigned int c);
void font_set_flag4(int on);
void *title_text_frame(void *list, int x, int y, int w, unsigned short *text, int align);
void *cip_open_h(void *handle, int bufsize, int prefetch);
void *cip_open_m(void *handle, int bufsize, int prefetch);
int cip_is_loaded(void *c);
unsigned int *cip_get_image(unsigned int *list, void *handle, int id, int variant, int *loaded_out);
int inv_get_hi(unsigned short id);
void cardlist_init(TCardListCfg *cfg);
void cardlist_clear(void);
void *cardlist_build(void *list, TCardPanel *p);
void select_card_release(void);
void libselect_card_08804008(int mode, void *cfg, int *n, unsigned short *cards);
int libselect_card_088041C4(void);
void libselect_card_08807630(void);
PadState *pad_get(void);
void *tbox_draw(void *list, TBox *b);
void *tframe_cursor(void *list, unsigned int color, int x, int y, int w, int h);
void *tframe_box(void *list, unsigned int color, int x, int y, int w, int h, int kind, int style, int mode);
void tscroll_calc(short *pos, short *len, TBox *b, short x, short y, short h);
void *tscroll_draw(void *list, unsigned int color, int x, int y, int h, int pos, int len);
void tfd_show_msgbox(MenuWidget *w, int msg);
int tfd_evt_handler(int a, int b);
int net_get_state(void);
void inv_set_bit7(unsigned short id);
void inv_adjust_count(unsigned short id, int dir);
void bgm_stop(int x);
void *tfd_draw(void *list);
void *tfd_cursor_sprite(void *list, int i, int x, int y, int w, int h);
int tdl_deck_default(int id);

void *tfd_duelinfo_stub(void *list);
void *tfd_duelinfo_update(void *list);
void *tfd_duelinfo_draw(void *list);
void *tfd_duelinfo_icon(void *list);
void *tfd_duelinfo_enter_init(void *list);
void tfd_net_load_charalist(void);
int tfd_net_cb(void *arg, int id);
int tfd_lobby_handler(void);
void tfd_lobby_draw(void);
void tfd_pass_update(void);
int tfd_vscpu_entry_draw(void);
void tfd_net_mode_update(void);
void *tfd_config_draw_label(void *list);
void *tfd_main_text(void *list, int i, int x, int y, int w, int h);
int tfd_config_draw_sprite(void);
void *tfd_config_draw_text(void *list, int n, int style);
void *tfd_show_duelinfo(void *list, int i, int x, int y, int w, int h);
void *tfd_sprite_draw(void *list, unsigned short i, int x, int y, int w, int h);
int tfd_sprite_slidein(void);
void *tfd_sprite_anim(void *list, int i, int x, int y, int w, int h);
void *tfd_icon_draw(void *list, int id, int x, int y);
int tfd_load_selectcard(void *arg, int id);

/* a row of the player list (TBoxDrawFn): kind, deck, life points, time, name */
void *tfd_sprite_draw(void *list, unsigned short i, int x, int y, int w, int h)
{
    TDuelCfg *d = &title.duel;
    Title *t = &title;
    MenuWidget *mw = &title.panel.mw;
    TFdList *l = &d->list;
    TFdSeat *e = (TFdSeat *)&d->opts;
    unsigned short *text;
    int bw = w;
    int ix = x;
    int iy = y;
    int tx;
    int ty;
    int k;
    int tw = w;
    int fstyle = 0;
    unsigned int color = 0;
    int style;
    int bx;
    int by;
    int fk;
    int icon;
    int id;
    int kind = 5;

    if (title.sub == 4)
        e = &l->seat[i + mw->scroll];
    style = 9;
    if (t->sub == 4) {
        switch (e->kind) {
        case 0:
            if (e->b5)
                style = 5;
            else
                style = 4;
            break;
        case 1:
            style = 6;
            break;
        case 2:
            style = 5;
            break;
        case 3:
            style = 6;
            break;
        case 4:
            style = 7;
            break;
        }
        if (e->side == 0)
            style = 0xB;
    }
    list = tframe_box(list, 0xFFFFFFFF, x, y, w, h, 0x54, style, 0);
    for (k = 0; k < 8; k++) {
        bx = x;
        by = y;
        tx = x;
        ty = y;
        fk = -1;
        icon = 0xB;
        id = 0;
        text = 0;
        switch (k) {
        case 0:
            bx = x + 0x16;
            bw = 0x65;
            kind = e->kind;
            ix = bx - 0x12;
            tx = bx + 3;
            by = y + 4;
            iy = by;
            ty = by + 2;
            tw = 0x5F;
            if (kind == 0)
                id = e->b5 + 0x2F;
            else
                id = kind + 0x52;
            fstyle = 0xD;
            fk = 0x5D;
            color = 0xFF000000;
            switch (kind) {
            case 0:
                if (e->b5)
                    icon = 8;
                else
                    icon = 3;
                break;
            case 1:
                icon = 9;
                id = 0x57;
                break;
            case 2:
                icon = 4;
                break;
            case 3:
                icon = 6;
                break;
            case 4:
                icon = 7;
                break;
            }
            if (e->side == 0 || (t->sub == 5 && t->step2 < 2)) {
                icon = 0xB;
                id = 0;
            }
            break;
        case 1:
            bx = x + 0x7D;
            bw = 0xAF;
            by = y + 4;
            tx = bx + 3;
            ty = by + 2;
            color = 0xFFFFFFFF;
            tw = 0x38;
            fk = 0x5D;
            fstyle = 0xF;
            if (kind == 0) {
                id = e->b6 + 10;
            } else if (kind == 1) {
                text = e->card_name;
                tw = 0xA7;
            }
            if (e->side == 0 || (t->sub == 5 && t->step2 < 3)) {
                icon = 0xB;
                id = 0;
                text = 0;
            }
            break;
        case 2:
            bx = x + 0x7D;
            by = y + 4;
            bw = 0xAF;
            if (kind == 0) {
                tx = bx + 0x3A;
                ty = by + 2;
                tw = 0x38;
                id = e->lo + 0x68;
            }
            if (e->side == 0 || (t->sub == 5 && t->step2 < 5)) {
                icon = 0xB;
                id = 0;
            }
            break;
        case 3:
            bx = x + 0x7D;
            by = y + 4;
            bw = 0xAF;
            if (kind == 0) {
                tx = x + 0xB4;
                tw = 0x28;
            }
            break;
        case 4:
            if (kind == 0) {
                tx = x + 0xDE;
                tw = 0x20;
            }
            break;
        case 5:
            bx = x + 0x7D;
            by = y + 4;
            bw = 0xAF;
            if (kind == 0) {
                tx = bx + 0x78;
                ty = by + 2;
                tw = 0x34;
                id = e->hi + 0x6D;
            }
            if (e->side == 0 || (t->sub == 5 && t->step2 < 6)) {
                icon = 0xB;
                id = 0;
            }
            break;
        case 6:
            bx = x + 0x16;
            bw = 0xA3;
            ix = bx - 0x12;
            tx = bx + 3;
            by = y + 0x15;
            ty = by + 2;
            iy = by;
            tw = 0x9D;
            switch (t->sub) {
            case 5:
                id = 0x10;
                break;
            case 6:
            case 7:
                if (kind != 1)
                    id = 0x11;
                else
                    id = 0x17;
                break;
            default:
                id = 0xE;
                if (e->side != 0)
                    id = 0x11;
                break;
            }
            fstyle = 0xE;
            color = 0xFF004040;
            fk = 0x5D;
            if (t->sub == 5)
                icon = 2;
            else if (id > 0xE)
                icon = 1;
            break;
        case 7:
            bx = x + 0xC7;
            bw = 0x65;
            ix = bx - 0xF;
            tx = bx + 3;
            by = y + 0x15;
            ty = by + 2;
            iy = by;
            fstyle = 0xE;
            tw = 0x5F;
            fk = 0x5D;
            color = 0xFF004040;
            icon = 0;
            break;
        }
        if (fk >= 0)
            list = tframe_box(list, 0xFFFFFFFF, bx, by, bw, 0x10, fk, fstyle, 0);
        if (icon != 0xB)
            list = tfd_icon_draw(list, icon, ix, iy);
        if (k != 7) {
            if (id > 0) {
                font_set_size(13, 13);
                font_set_color(color);
                font_set_flag4(1);
                list = title_msg_draw(list, tx, ty, tw, 0xC, id, 2);
            } else if (text != 0) {
                font_set_size(13, 13);
                font_set_color(color);
                font_set_flag4(1);
                list = title_text_frame(list, tx, ty, tw, text, 2);
            }
        } else {
            text = e->name;
            font_set_size(13, 13);
            font_set_color(color);
            font_set_flag4(1);
            if (text != 0)
                list = title_text_frame(list, tx, ty, tw, text, 2);
        }
    }
    return list;
}

/* text of the help box */
int tfd_sprite_slidein(void)
{
    Title *t = &title;
    MenuWidget *w = &title.win[0];
    int id = 0;

    switch (title.sub) {
    case 4:
        break;
    case 5:
        if (t->step == 0xB) {
            switch (t->step2) {
            case 0:
                break;
            case 1:
                id = w->cursor + 0x43;
                break;
            case 2:
                id = w->cursor + 0x45;
                break;
            case 3:
                id = w->cursor + 0x47;
                break;
            case 4:
                id = 0x49;
                break;
            case 5:
                id = 0x4A;
                break;
            case 6:
                id = 0x4B;
                break;
            case 7:
            case 8:
            case 9:
                break;
            }
        }
        break;
    case 6:
        id = 0x3E;
        break;
    case 7:
        id = 0x42;
        break;
    case 8:
    case 9:
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
    case 0x10:
        break;
    case 0x11:
        id = 0x25;
        break;
    case 0x12:
        id = 0x26;
        break;
    case 0x13:
        id = 0x27;
        break;
    }
    return id;
}

/* title of the corner box */
void *tfd_sprite_anim(void *list, int i, int x, int y, int w, int h)
{
    Title *t = &title;
    TDuelOpts *o = &title.duel.opts;
    int id = 0x1B;
    unsigned int color = 0xFFFFFFFF;
    int j;
    int n;
    int need;

    switch (title.sub) {
    case 4:
        id = 0x19;
        break;
    case 5:
        id = 0x1A;
        if (t->step2 < 6)
            color = 0xFF808080;
        break;
    case 6:
    case 7:
        n = 0;
        if (o->kind == 0)
            need = 1;
        else if (o->kind == 2)
            need = 1;
        for (j = 0; j < 8; j++) {
            if (title.duel.list.deck[j].kind != 0)
                n++;
        }
        if (n < need)
            color = 0xFF808080;
        break;
    case 8:
        id = 0x21;
        break;
    case 9:
        id = 0x21;
        break;
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
        break;
    case 0xE:
        id = 0x20;
        break;
    case 0xF:
    case 0x10:
        break;
    case 0x11:
        id = 0x1D;
        break;
    case 0x12:
        id = 0x1C;
        break;
    case 0x13:
        id = 0x1E;
        break;
    }
    if (id != 0)
        list = title_msg_font(list, x, y, w, 0xC, id, 3, 0xD, 0xD, color);
    return list;
}

void *tfd_icon_setup(void *list, MenuWidget *w, int x, int y, int cw, int ch)
{
    if (title.u86c != 0)
        return tframe_cursor(list, 0xFFFFFFFF, 0x147, 0xF2, 0x97, 0x17);
    return tframe_cursor(list, 0xFFFFFFFF, x - 4, y - 4, cw, ch - 1);
}

#ifdef NON_MATCHING /* ordem das cargas da tabela de sprites e alocação de $s */
/* an icon of the player list (title_sprites2[0x4A], 5 per row) */
void *tfd_icon_draw(void *list, int id, int x, int y)
{
    short tw = title_sprites2[0x4A].tw;
    short th = title_sprites2[0x4A].th;
    short a = title_sprites2[0x4A].a;
    short u = title_sprites2[0x4A].u;
    short sx = x;
    short sy = y;
    TDuelCfg *d = &title.duel;
    short b = title_sprites2[0x4A].b;
    short v = title_sprites2[0x4A].v;
    short col = id % 5;
    short row = id / 5;

    list = tmenu_gfx_setup(list, 0);
    list = dl_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, ehp_find(title_pack(), title_sprite_gim(title_sprites2, 0x4A), 0));
    switch (id) {
    case 0:
    case 5:
        break;
    case 1:
    case 2:
        u += (short)(col * 0x11 + (col - 1) * 0x22);
        u += (short)(d->blink / 10 * 0x11);
        break;
    case 9:
        u += 0x88;
        row = 1;
        break;
    default:
        u += (short)(col * 0x11 + (col - 1) * 0x22);
        break;
    }
    list = draw_set_offsets(list, tw, th, tw, th);
    return draw_sprite(list, sx, sy, 0, u, (short)(v + (short)(row * 0x11)), a, b);
}
#endif

void tfd_load_charalist(void)
{
    Title *t = &title;

    switch (title.step2) {
    case 0:
        ehfile_request_module(2, "libcharalist.prx", tfd_load_selectcard, 0);
        gfx_overlay_enable(1);
        t->step2++;
        t->b3038_5 = 1;
        break;
    case 1:
        break;
    }
}

int tfd_load_selectcard(void *arg, int id)
{
    Title *t = &title;

    if (title_module_start(arg, id)) {
        t->sub_mod = id;
        gfx_overlay_enable(0);
        t->step2 = 0;
        t->step++;
        return 1;
    }
    return 0;
}

/* the character pick of libcharalist */
void tfd_unload_modules(void)
{
    Title *t = &title;
    TFdTail *tl = &title.duel.tail;
    TDuelCfg *d = &title.duel;
    TFdList *l = &d->list;
    int id;
    int cos;
    int deck;

    switch (title.step2) {
    case 0:
        charalist_init(2, t->heap);
        charalist_update();
        t->sub_shown = 1;
        tl->u003 = d->u000;
        t->step2++;
        break;
    case 1:
        switch (charalist_update()) {
        case 0:
            break;
        case 1:
            id = charalist_result_id();
            cos = charalist_result_costume();
            deck = tdl_deck_default(id);
            l->sel.u02 = cos;
            l->sel.type = id;
            if (id == 0x29)
                l->sel.dk = 0;
            else
                l->sel.dk = 1;
            l->sel.deck = deck;
            t->sub_shown = 0;
            t->step = 0x2F;
            break;
        case 2:
            t->sub_shown = 0;
            t->step = 0x31;
            break;
        }
        break;
    }
}

void tfd_entry_update(void)
{
    Title *t = &title;
    MenuWidget *w = &title.win[1];

    switch (title.step) {
    case 0:
    case 1:
    case 2:
    case 3:
        fade_start(1, 0xFF000000, 0x10);
        fade_finish();
        tfd_show_msgbox(w, 0x78);
        t->step = 0x14;
        break;
    case 0x14:
        if (menu_update(w) != -1)
            t->step = 0x1D;
        break;
    case 0x16:
    case 0x1B:
        break;
    case 0x1D:
        fade_start_in(1, 0xFF000000, 0x10);
        fade_finish();
        t->step = 0;
        t->sub = 1;
        break;
    case 0x1E:
    case 0x1F:
    case 0x21:
        break;
    }
}

void *tfd_entry_draw(void *list)
{
    MenuWidget *w = &title.win[1];

    if (title.step == 0x14)
        list = twin_draw_center(list, w, 1);
    return list;
}
