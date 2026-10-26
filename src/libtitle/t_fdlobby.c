// @range 0x0881BA64-0x0881E0CC
// @data 0x0882F370-0x0882F384
// @rodata 0x0882B2D4-0x0882B344
// @asm 0x0881BA64-0x0881BB7C
// @asm 0x0881C0E0-0x0881C474
// @asm 0x0881CC08-0x0881CDAC
// @asm 0x0881D94C-0x0881D994
/* End of the free duel setup: the password lobby (cards picked with libselect_card), the duel info
   panel, the vs CPU entry, the option list drawing, the character pick (libcharalist) and the entry
   message. */
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
int func_0882527C(int id);

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

/* the frame of a box of the lobby: no list, no scroll bar */
#define TFD_BOX(b, X, Y, W, H) \
    (b)->x = (X);              \
    (b)->ix = (b)->x + 6;      \
    (b)->y = (Y);              \
    (b)->iy = (b)->y + 6;      \
    (b)->w = (W);              \
    (b)->iw = (b)->w - 6;      \
    (b)->h = (H);              \
    (b)->ih = (b)->h - 6;      \
    (b)->color = 0xFFFFFFFF;   \
    (b)->color2 = 0xFFFFFFFF;  \
    (b)->style = 3;            \
    (b)->no_frame = 0;         \
    (b)->n = 1;                \
    (b)->count = 1;            \
    (b)->f1 = 1;               \
    (b)->no_cursor = 1;        \
    (b)->no_cells = 1;         \
    (b)->f4 = 1;               \
    (b)->no_scroll = 1;        \
    (b)->state = 0;            \
    (b)->frame = 0;            \
    (b)->pos = 0;              \
    (b)->cursor = 0;           \
    (b)->text = 0;             \
    (b)->draw = 0

#ifdef NON_MATCHING /* os contadores x+0x22 e o ponteiro do deck trocam $s0/$s1 */
/* names of the players of the ad hoc list */
void *tfd_adhoc_pass_init(void *list, int mode)
{
    int i;
    unsigned short *name;

    if (mode == 2) {
        for (i = 0; i < title.duel.list.n; i++) {
            list = tpt_sprite_def(list, 0x16, i * 0x74 + 8, 0x34);
            list = tpt_sprite_def(list, i + 0x12, i * 0x74 + 10, 0x36);
            if ((name = title.duel.list.deck[i].name) != 0) {
                font_set_size(11, 11);
                font_set_color(0xFFFFFFFF);
                font_set_flag4(1);
                list = title_text_frame(list, i * 0x74 + 0x22, 0x36, 0x40, name, 2);
            }
        }
    }
    return list;
}
#endif

void tfd_pass_init(void)
{
}

void tfd_pass_setup_init(void)
{
}

void tfd_pass_join_init(void)
{
}

void tfd_pass_select_init(void)
{
}

/* loads libselect_card, the card images and the card list */
void tfd_pass_update(void)
{
    TDuelCfg *d = &title.duel;
    Title *t = &title;
    TFdPassSel *ps = &d->psel;
    TFdPass *pa = &d->pass;
    TCardListCfg cfg;

    switch (title.step2) {
    case 0:
        memset(pa, 0, sizeof(TFdPass));
        ps->loaded = 1;
        t->u86d = 3;
        t->u874 = 10;
        ps->cip = 0;
        ps->mem = 0;
        t->u86c = 0;
        t->arg = 0;
        t->u87a = 0;
        ehfile_request_module(2, "libselect_card.prx", tfd_net_cb, 0);
        gfx_overlay_enable(1);
        t->step2++;
        break;
    case 1:
        break;
    case 2:
        if (ps->cip != 0)
            break;
        ps->mem = heap_alloc(t->heap, 0x132000);
        ps->cip = cip_open_h(ps->mem, 0x132000, 0);
        t->step2++;
        break;
    case 3:
        if (cip_is_loaded(ps->cip)) {
            tfd_net_load_charalist();
            t->step2++;
        }
        break;
    case 4:
        t->step2 = 4;
        memset(&cfg, 0, sizeof(cfg));
        cfg.gim = ehp_find(title_pack(), title_gim_name(12), 0);
        cfg.digits_gim = ehp_find(title_pack(), title_gim_name(13), 0);
        cfg.frame_ehf = ehp_find(title_pack(), title_ehf_name(3), 0);
        cfg.attr_gim = ehp_find(title_pack(), title_gim_name(14), 0);
        cfg.kind_gim = ehp_find(title_pack(), title_gim_name(15), 0);
        cfg.cip = ps->cip;
        cardlist_init(&cfg);
        if (pa->cip == 0) {
            pa->mem = heap_alloc(t->heap, 0x5E2000);
            pa->cip = cip_open_m(pa->mem, 0x5E2000, 0);
            t->step2++;
        }
    case 5:
        if (cip_is_loaded(pa->cip)) {
            gfx_overlay_enable(0);
            t->step++;
            t->step2 = 0;
            title_ehf_load(9, 2, 0, 0x1E0);
        }
        break;
    }
}

void tfd_lobby_update(void)
{
    TDuelCfg *d = &title.duel;
    Title *t = &title;
    TFdPassSel *ps = &d->psel;
    TFdPass *pa = &d->pass;

    switch (title.step) {
    case 0:
        t->step = 1;
    case 1:
        tfd_pass_update();
        t->step = 2;
        break;
    case 2:
        gfx_overlay_enable(0);
        title_fade_out(0);
        t->step++;
        break;
    case 3:
        if (title_fade_state() == 0)
            t->step = 0xB;
        break;
    case 4:
        break;
    case 5:
        break;
    case 0xB:
        pa->panel.id = -1;
        switch (tfd_lobby_handler()) {
        case -3:
            break;
        case -2:
            t->step = 0x1B;
            break;
        case -1:
            break;
        case 0:
            break;
        case 1:
            title_goto_fade(1, 0xFFF, 0xF);
            t->step = 0x1E;
            t->step2 = 0;
            ps->loaded = 1;
            break;
        case 2:
            break;
        }
        break;
    case 0x1B:
        title_fade_out(1);
        t->step = 0x1E;
        break;
    case 0x1D:
        title_goto_fade(1, 0xFFF, t->next_sub);
        t->step2 = 0;
        t->step = 0x1E;
        break;
    case 0x1E:
        if (title_fade_state() != 0)
            break;
        t->step = 0x1F;
    case 0x1F:
    case 0x21:
        tfd_lobby_draw();
        break;
    }
}

/* leaves the lobby: frees libselect_card and the card images */
void tfd_lobby_draw(void)
{
    TDuelCfg *d = &title.duel;
    Title *t = &title;
    TFdPassSel *ps = &d->psel;
    TFdPass *pa = &d->pass;

    if (d->psel.loaded) {
        select_card_release();
        title_module_stop();
        heap_set_level(t->heap, ps->mem);
    }
    cardlist_clear();
    heap_set_level(t->heap, pa->mem);
    title_set_state(t->state, 4);
}

/* input of the lobby: the five cards (two rows), then the buttons; returns the card chosen,
   1 to start, -2 to leave, -1 otherwise */
#ifdef NON_MATCHING /* forma dos desvios dos testes de pad diferente */
int tfd_lobby_handler(void)
{
    Title *t = &title;
    TFdPass *pa = &title.duel.pass;
    PadState *pad = pad_get();
    unsigned int rep = pad->b.rep;
    unsigned int trig = pad->b.trig;

    if (t->u86c == 0) {
        if (t->arg < 5) {
            if (trig & region_pick_value(0)) {
                pa->slot[t->arg].on = 1;
                return t->arg;
            }
            if (trig & region_pick_value(1)) {
                pa->slot[t->arg].on = 0;
            } else if (rep & 0x20) {
                if (t->arg < 4)
                    t->arg++;
                else
                    t->arg = 0;
            } else if (rep & 0x80) {
                if (t->arg > 0)
                    t->arg--;
                else
                    t->arg = 4;
            } else if (rep & 0x40) {
                t->arg += 5;
            }
        } else if (t->arg < 10) {
            if (!(trig & region_pick_value(0)) && !(trig & region_pick_value(1))) {
                if (rep & 0x20) {
                    if (t->arg < 9)
                        t->arg++;
                    else
                        t->arg = 5;
                } else if (rep & 0x80) {
                    if (t->arg > 5)
                        t->arg--;
                    else
                        t->arg = 9;
                } else if (rep & 0x10) {
                    t->arg -= 5;
                } else if (rep & 0x40) {
                    t->u86c++;
                }
            }
        }
    } else if (t->u87a == 0) {
        if (trig & region_pick_value(0)) {
            if (t->u86c == 2)
                t->u87a = 1;
            else if (t->u86c == 1)
                return 1;
        } else if (trig & region_pick_value(1)) {
            t->u87a = 1;
        } else if (rep & 0x20) {
            if (t->u86c < 2)
                t->u86c++;
            else
                t->u86c = 1;
        } else if (rep & 0x80) {
            if (t->u86c > 1)
                t->u86c--;
            else
                t->u86c = 2;
        } else if (rep & 0x10) {
            t->u86c = 0;
        }
    } else {
        if (trig & region_pick_value(0)) {
            t->u87a = 0;
            return -2;
        }
        if (trig & region_pick_value(1))
            t->u87a = 0;
    }
    return -1;
}
#endif

void *tfd_lobby_draw2(void *list)
{
    list = title_ehf_draw(list, 9);
    switch (title.step) {
    case 1:
        break;
    case 2:
    case 3:
    case 0xB:
    case 0x1B:
    case 0x1D:
    case 0x1E:
        list = tfd_duelinfo_stub(list);
        list = tfd_duelinfo_update(list);
        list = tfd_duelinfo_draw(list);
        list = tfd_duelinfo_icon(list);
        list = tfd_duelinfo_enter_init(list);
        break;
    case 0x1F:
    case 0x21:
        break;
    }
    return list;
}

/* frames of the lobby */
void *tfd_duelinfo_update(void *list)
{
    TBox *b = &title.panel.box;

    TFD_BOX(b, 0x95, 2, 0x128, 0x17);
    list = tbox_draw(list, b);
    TFD_BOX(b, 0x95, 0xF2, 0xEC, 0x17);
    list = tbox_draw(list, b);
    TFD_BOX(b, 0x183, 0xF2, 0x5B, 0x17);
    list = tbox_draw(list, b);
    TFD_BOX(b, 0x95, 0x1A, 0x149, 0xD6);
    list = tbox_draw(list, b);
    list = title_sprite(list, 1, 0x69, 0x9B, 0x7C);
    list = title_sprite_flip(list, 1, 0x69, 0x16E, 0x75, 0, 2);
    TFD_BOX(b, 0x9B, 0x1F, 0x13D, 0x5C);
    list = tbox_draw(list, b);
    TFD_BOX(b, 0x9B, 0x8E, 0x13D, 0x5C);
    return tbox_draw(list, b);
}

#ifdef NON_MATCHING /* alocação de $s nos dois laços */
/* the empty card frames and the cards chosen */
void *tfd_duelinfo_draw(void *list)
{
    TDuelCfg *d = &title.duel;
    TFdPass *pa = &d->pass;
    TFdPassSel *ps = &d->psel;
    int y;
    int j;
    int x;
    int i;
    int loaded;

    y = 0x25;
    for (j = 0; j < 2; j++) {
        if (j != 0)
            y = 0x95;
        for (i = 0, x = 0xA6; i < 5; i++, x += 0x3C)
            list = title_sprite(list, 1, 0x6C, x, y);
    }
    {
        int y2;
        int j2;
        int n;
        int i2;
        int x2;
        unsigned short *c;
        int id;

        y2 = 0x25;
        n = ps->n;
        for (j2 = 0; j2 < 2; j2++) {
            if (j2 != 0) {
                y2 = 0x95;
                n = ps->n;
            }
            for (i2 = 0, x2 = 0xA6, c = ps->card; i2 < n; i2++, c++, x2 += 0x3C) {
                id = *c;
                list = cip_get_image(list, pa->cip, id, inv_get_hi(id), &loaded);
                list = dl_set_tex_filter(list, 1, 1);
                list = draw_set_offsets(list, 0x40, 0x40, 0x36, 0x4D);
                list = draw_sprite(list, x2, y2, 0, 0, 0, 0, 0);
            }
        }
    }
    return list;
}
#endif

/* all the images of the chosen cards are loaded */
int tfd_duelinfo_check(void)
{
    TDuelCfg *d = &title.duel;
    TFdPass *pa = &d->pass;
    TFdPassSel *ps = &d->psel;
    int ok = 1;
    void *list = dl_select_layer(0, 0);
    int i;
    int id;
    int loaded;

    for (i = 0; i < ps->n; i++) {
        id = ps->card[i];
        cip_get_image(list, pa->cip, id, inv_get_hi(id), &loaded);
        if (loaded == 0)
            ok = loaded;
    }
    dl_set_list(list);
    return ok != 0;
}

/* cursor and marks of the lobby */
void *tfd_duelinfo_icon(void *list)
{
    Title *t = &title;
    TFdPass *pa = &title.duel.pass;
    int x;
    int y;
    int w = 0x3E;
    int h = 0x55;
    int i;
    int sx;

    if (title.u86c == 0) {
        if (t->arg < 5) {
            x = t->arg * 0x3C + 0xA2;
            y = 0x21;
        } else if (t->arg < 10) {
            x = (t->arg - 5) * 0x3C + 0xA2;
            y = 0x91;
        }
    } else if (t->u86c == 1) {
        x = 0xB9;
        y = 0xF2;
        w = 0xC8;
        h = 0x17;
    } else {
        x = 0x183;
        y = 0xF2;
        w = 0x5B;
        h = 0x17;
    }
    if (t->u87a == 0)
        list = tframe_cursor(list, 0xFFFFFFFF, x, y, w, h);
    for (i = 0, sx = 0xBF; i < 5; i++, sx += 0x3A) {
        if (pa->slot[i].on)
            list = title_sprite(list, 1, 0x6A, sx, 0x20);
    }
    return list;
}

void *tfd_duelinfo_stub(void *list)
{
    return cardlist_build(list, &title.duel.pass.panel);
}

/* the confirmation window */
void *tfd_duelinfo_enter_init(void *list)
{
    TBox *b = &title.panel.box;

    if (title.u87a != 0) {
        TFD_BOX(b, 0x78, 0x60, 0xF0, 0x50);
        list = tbox_draw(list, b);
    }
    return list;
}

void tfd_net_load_charalist(void)
{
    TFdPassSel *ps = &title.duel.psel;
    struct {
        void *heap;
        void *cip;
        int u08;
        void *pack;
    } cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.heap = title.heap;
    cfg.cip = ps->cip;
    cfg.pack = title_pack();
    libselect_card_08804008(2, &cfg, &ps->n, ps->card);
}

int tfd_net_cb(void *arg, int id)
{
    Title *t = &title;

    if (title_module_start(arg, id)) {
        t->step2++;
        t->sub_mod = id;
        return 1;
    }
    return 0;
}

void tfd_net_event_handler(void)
{
    Title *t = &title;

    switch (title.step) {
    case 0:
        t->step = 1;
    case 1:
        t->step = 2;
    case 2:
        gfx_overlay_enable(1);
        tfd_net_load_charalist();
        t->step = 0xB;
    case 3:
    case 0xB:
        if (libselect_card_088041C4() != 1)
            break;
        t->step = 0x1E;
    case 0x1B:
    case 0x1D:
    case 0x1E:
        if (tfd_duelinfo_check())
            gfx_overlay_enable(0);
        else
            gfx_overlay_enable(1);
        t->step = 0x1F;
        break;
    case 0x1F:
        if (tfd_duelinfo_check()) {
            title_fade_out(0);
            gfx_overlay_enable(0);
            t->sub = 0xE;
            t->step2 = 0;
            t->step = 3;
        }
        break;
    case 0x21:
        break;
    }
}

void tfd_net_lobby_machine(void)
{
    Title *t = &title;

    if (title.sub == 0xF) {
        switch (t->step) {
        case 0:
        case 1:
        case 2:
        case 3:
            break;
        case 0xB:
        case 0x1B:
            libselect_card_08807630();
            break;
        case 0x1D:
        case 0x1E:
        case 0x1F:
        case 0x21:
            break;
        }
    } else if (t->sub == 9 && t->sub_shown) {
        charalist_draw();
    }
}

void tfd_net_mode_update(void)
{
    title.step = 0;
    title.step2 = 0;
    title.frame = 0;
    title.u86c = 0;
    title.u86d = 2;
    title.color0 = 0xFFFFFFFF;
    title.color1 = 0xFFFFFFFF;
    title_ehf_load(8, 2, 0, 0x1E0);
    title_ehf_load(0xE, 2, 0, 0x11);
}

void tfd_vscpu_entry_update(void)
{
    Title *t = &title;
    TDuelCfg *d = &title.duel;
    MenuWidget *w = &title.win[1];
    int card;

    switch (title.step) {
    case 0:
        t->step = 1;
    case 1:
        tfd_net_mode_update();
        title_fade_out(0);
        t->step = 2;
        break;
    case 2:
    case 3:
        if (title_fade_state() == 0)
            t->step = 4;
        break;
    case 4:
        title_ehf_load(0xE, 2, 0, 0x11);
        t->step = 5;
        break;
    case 5:
        if (title_ehf_frame(0xE) == 0x11) {
            title_ehf_anim(0xE, 2, 0x12, 0x35);
            if (t->sub == 0x13) {
                card = d->card;
                if (d->card_win) {
                    inv_set_bit7(card);
                    inv_adjust_count(card, 1);
                    inv_adjust_count(card, 1);
                    inv_adjust_count(card, 1);
                    tfd_show_msgbox(w, 0x75);
                } else {
                    tfd_show_msgbox(w, 0x76);
                }
                t->step = 0x14;
                se_play(0x621A);
            } else {
                t->step = 0xB;
            }
        }
        break;
    case 0xB:
        if (tfd_vscpu_entry_draw())
            t->step = 0x1B;
        break;
    case 0x14:
        if (menu_update(w) != -1) {
            if (net_get_state() == 0x15)
                tfd_evt_handler(2, 0);
            else
                tfd_evt_handler(8, 0);
            t->step = 0x1B;
        }
        break;
    case 0x1B:
        title_goto_fade(1, 0xFFF, 3);
        t->step = 0x1E;
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

int tfd_vscpu_entry_draw(void)
{
    unsigned int trig = pad_get()->b.trig;

    if (trig & region_pick_value(0))
        return 1;
    return (trig & region_pick_value(1)) != 0;
}

void *tfd_vscpu_entry_draw2(void *list)
{
    Title *t = &title;
    TDuelOpts *o = &title.duel.opts;
    MenuWidget *w = &title.win[1];
    int n = 0;

    list = title_ehf_draw(list, 8);
    if (title.step >= 5) {
        list = title_ehf_draw(list, 0xE);
        list = title_screen_label(list);
    }
    list = tbox_header(list, tfd_show_duelinfo);
    if (t->sub != 0x13)
        list = tbox_corner(list, tfd_sprite_anim);
    list = tfd_config_draw_text(list, 1, 9);
    if (t->sub != 0x13) {
        list = tbox_help(list, tfd_sprite_slidein(), 0xC);
        list = tbox_main(list, tfd_main_text);
        if (t->step2 != 6)
            list = tfd_config_draw_label(list);
    }
    if (t->step2 != 0) {
        if (o->kind == 0) {
            int s = t->step2;
            if ((short)s < 4)
                n = (short)s - 1;
            else
                n = (short)s - 2;
        } else {
            n = 1;
        }
    }
    if (t->sub == 0x13)
        n = 1;
    list = tbox_side(list, 0x14, n, tfd_cursor_sprite);
    if (t->step == 0x14)
        list = twin_draw_center(list, w, 1);
    return list;
}

void *tfd_config_draw_label(void *list)
{
    int id = 0;

    switch (title.sub) {
    case 0x10:
        id = 0x4C;
        break;
    case 0x11:
        id = 0x4C;
        break;
    case 0x12:
        id = 0x4C;
        break;
    case 0x13:
        id = 0x4C;
        break;
    }
    return tbox_small(list, id, 9, 0x67, 0x135, 0x10);
}

#ifdef NON_MATCHING /* o original calcula y + 0x10 antes de mover x */
void *tfd_main_text(void *list, int i, int x, int y, int w, int h)
{
    return title_msg_font(list, x, y + 0x10, 0x12C, 0xC, 0x22, 0, 0xF, 0xF, 0xFF404040);
}
#endif

/* input of the option list */
int tfd_config_draw_line(void)
{
    Title *t = &title;
    TPanel *p = &title.panel;
    MenuWidget *w = &p->mw;
    TBox *b = &p->box;
    unsigned int trig;

    tfd_config_draw_sprite();
    trig = pad_get()->b.trig;
    if (title.u86c == 0 && p->f2) {
        menu_update(w);
        b->pos = w->scroll + w->cursor;
        b->cursor = w->cursor;
        if (trig & region_pick_value(0)) {
            bgm_stop(0x6213);
            if (title.duel.list.seat[b->pos].side != 0) {
                se_play(0x6213);
                return b->pos;
            }
            se_play(0x6215);
            return -1;
        }
        if (trig & region_pick_value(1)) {
            se_play(0x6214);
            return -2;
        }
        return -1;
    }
    if (trig & region_pick_value(0)) {
        se_play(0x6213);
        return b->count + t->u86c;
    }
    if (trig & region_pick_value(1)) {
        se_play(0x6214);
        return -2;
    }
    return -1;
}

/* left/right on the buttons row */
int tfd_config_draw_sprite(void)
{
    Title *t = &title;
    int max = 0;
    unsigned int trig = pad_get()->b.trig;

    if (title.u86d > 0)
        max = t->u86d - 1;
    if (trig & 0x20) {
        se_play(0x6212);
        if (t->u86c < max)
            t->u86c++;
        else
            t->u86c = 0;
    } else if (trig & 0x80) {
        se_play(0x6212);
        if (t->u86c > 0)
            t->u86c--;
        else
            t->u86c = max;
    }
    if (trig & region_pick_value(0)) {
        se_play(0x6213);
        return t->u86c;
    }
    if (trig & region_pick_value(1)) {
        se_play(0x6214);
        return -2;
    }
    return -1;
}

void *tfd_config_draw_rect(void *list)
{
    list = tfd_draw(list);
    title.frame++;
    return list;
}

/* the player list */
void *tfd_config_draw_text(void *list, int n, int style)
{
    TBox *b = &title.panel.box;

    b->x = 4;
    b->y = 0x35;
    b->w = 0x131;
    b->h = 0x2A;
    b->color = 0xFFFFFFFF;
    b->color2 = 0xFFFFFFFF;
    b->frame = 0x54;
    b->style = style + n % 5;
    b->no_frame = 1;
    b->f1 = 1;
    b->no_cursor = 1;
    b->no_cells = 1;
    b->f4 = 1;
    b->no_scroll = 1;
    b->state = 0;
    b->pos = 0;
    b->cursor = 0;
    if (n > 5)
        b->n = 5;
    else
        b->n = n;
    b->count = title.duel.list.n;
    b->text = 0;
    b->ix = b->x;
    b->iy = b->y;
    b->iw = b->w;
    b->ih = b->h;
    b->draw = (TBoxDrawFn)tfd_sprite_draw;
    return tbox_draw(list, b);
}

/* the deck list with its scroll bar */
void *tfd_config_draw_bg(void *list)
{
    TPanel *p = &title.panel;
    TBox *b;
    short x;
    short y;
    short h;
    short pos;
    short len;

    p->box.x = 0x136;
    p->box.ix = p->box.x + 6;
    p->box.y = 0x35;
    p->box.iy = p->box.y + 6;
    p->box.w = 0xF;
    p->box.iw = p->box.w - 6;
    p->box.h = 0xD6;
    p->box.ih = p->box.h - 6;
    p->box.color = 0xFFFFFFFF;
    p->box.color2 = 0xFFFFFFFF;
    p->box.frame = 0x54;
    p->box.style = 9;
    p->box.no_frame = 0;
    p->box.f1 = 1;
    p->box.no_cursor = 1;
    p->box.no_cells = 1;
    p->box.f4 = 1;
    p->box.no_scroll = 1;
    p->box.pos = p->mw.cursor + p->mw.scroll;
    p->box.n = 5;
    p->box.state = 0;
    p->box.text = 0;
    p->box.draw = 0;
    p->box.cursor = p->mw.cursor;
    p->box.count = 8;
    b = &p->box;
    list = tbox_draw(list, b);
    x = b->x + 4;
    y = b->y + 10;
    h = b->h - 20;
    tscroll_calc(&pos, &len, b, x, y, h);
    return tscroll_draw(list, 0xFFFFFFFF, x, y, h, pos, len);
}

/* title of the header box */
void *tfd_show_duelinfo(void *list, int i, int x, int y, int w, int h)
{
    Title *t = &title;
    TDuelCfg *d = &title.duel;
    int id = 0x22;

    switch (title.sub) {
    case 4:
        if (t->step >= 0xC && t->step < 0x1E && t->step != 0x1B && t->next_step != 0x1F && t->next_sub != 5)
            id = 0x28;
        break;
    case 5:
        id = 0x23;
        break;
    case 6:
        id = 0x24;
        break;
    case 7:
        id = 0x29;
        break;
    case 8:
        if (t->step < 0x18)
            id = 0x2B;
        else if (d->u005 < 6)
            id = 0x2D;
        else
            id = 0x2E;
        break;
    case 9:
        if (t->step < 0x18) {
            if (t->step2 == 0)
                id = 0x2A;
            else
                id = 0x2C;
        } else if (d->u005 < 6) {
            id = 0x2D;
        } else {
            id = 0x2E;
        }
        break;
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
        break;
    case 0x10:
        id = 0x25;
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
    return title_msg_font(list, x, y, w, 0xC, id, 0, 0xD, 0xD, 0xFFFFFFFF);
}

