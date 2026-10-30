// @range 0x08825088-0x08825DDC
// @asm 0x088250E8-0x088252B0
// @asm 0x08825674-0x08825728
// @asm 0x08825A14-0x08825A70
/* The message window (one MenuWidget, dd_msg_win, shared by the duel's questions and choices:
   open/update/draw/close, heap copies of its texts, hooks), the number choice drawn in it, and the
   bouncing marks over the monsters that can still attack. */
#include "dd_field_ui.h"

extern FxVtx D_08876228[];             /* quad of the attack mark */

int dd_free(void *p);
void *dd_alloc(int n);
void *dd_heap_get(void);
int dd_input_on(void);
void *dd_gim_get(int id);
DdCobj *dd_cobj_at(int player, int zone, int index);
void dd_slot_xform(float *pos, float *rot, DdPos at);
int dd_mark_is_at(int player, int zone);
int dv_phase(void);
int dv_turn_side(void);
int dv_can_attack_any(int side, int z, int arg);
float sinf_vfpu(float x);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
int wsprintf(unsigned short *out, const unsigned short *fmt, ...);
int wcslen(const unsigned short *s);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
int func_0882092C(void);
unsigned short *func_08825088(int i);
void func_088252B0(void);
void func_088252B8(void);
void func_08825394(int player, int zone);
void *func_088254FC(void *dl);
void func_08825580(void);
int func_08825728(void);
int func_08825764(void);
void func_088257B8(void);
int func_088257C4(void);
MenuWidget *func_08825818(void);
void func_088259D8(void);
void func_088259E4(void *(*fn)(void *dl));
void func_088259F0(void (*fn)(MenuWidget *w, int old));
void func_088259FC(void (*fn)(void));
void func_08825A08(int (*fn)(void));
void func_08825A70(int v);
void func_08825A7C(unsigned short *title);
void func_08825AFC(unsigned short *text);
void func_08825B7C(int v);
void func_08825B88(int on);
void func_08825B9C(MenuWidget *w);

/* text of entry i of the number choice */
unsigned short *func_08825088(int i)
{
    wsprintf(dd_num_text, dlg_get_text(dd_num_prompt.fmt), dd_num_prompt.min + i * dd_num_prompt.step);
    return dd_num_text;
}

#ifdef NON_MATCHING /* registradores dos argumentos das setas diferentes; o +1 dobra com o +20 */
/* MenuWidget draw_all of the number choice: the number in a cursor box, arrows when it can change */
void func_088250E8(void *dl, int x, int y, int w_, int h_)
{
    MenuWidget *w = dd_num_prompt.w;
    int tx;
    int i = w->scroll + w->cursor;
    int cx = (w->def.w - w->def.pad_l - w->def.pad_r) / 2;
    unsigned short *text;
    int tw, th, ty;

    font_set_size(12, 12);
    font_set_color(0xFF000000);
    text = func_08825088(i);
    tw = font_measure_text(text) >> 6;
    th = font_line_height() >> 6;
    ty = y + (20 - th / 2);
    tx = x + (cx - tw / 2);
    dl = menu_draw_cursor(dl, w, tx - 10, ty - 5, tw + 20, th + 10);
    font_text_set(dl);
    font_draw_text(tx << 6, ty << 6, (480 - tx) << 6, text);
    dl = gim_set_texture(font_text_clear(), w->icon_gim);
    if (i > 0)
        dl = menu_draw_arrow(dl, x + cx - 4, y + 20 - th / 2, 1);
    if (i < w->count - 1)
        menu_draw_arrow(dl, x + cx - 4, y + 20 + th / 2 + 1, 0);
}
#endif

void func_088252B0(void)
{
}

void func_088252B8(void)
{
}

/* battle phase: a mark over every monster of the turn player that can still attack */
void func_088252C0(void)
{
    int side;
    int z;
    DdCobj *o;

    if (dv_phase() == 3) {
        switch (func_0882092C()) {
        case 0:
            side = dv_turn_side();
            for (z = 0; z <= 4; z++) {
                if (dv_can_attack_any(side, z, 1) != 0 && (o = dd_cobj_at(side, z, 0)) != 0 && !(o->t < o->dur)
                    && dd_mark_is_at(side, z) == 0)
                    func_08825394(side, z);
            }
            break;
        }
    }
}

/* the bouncing mark over one zone */
void func_08825394(int player, int zone)
{
    DdPos at;
    FxVec4 pos;
    FxVec4 rot;
    FxMat4 m;
    void *dl;

    at.player = player;
    at.zone = zone;
    at.f14 = 0;
    at.f15 = 0;
    at.idx = 0;
    dd_slot_xform(&pos.x, &rot.x, at);
    quat_to_mat4(m.m, &rot.x);
    pos.y += 10.0f + 5.0f * sinf_vfpu((float)*(unsigned int *)gfx_get_state() / 10.0f);
    mtx_push();
    mtx_identity();
    mtx_translate(&pos.x);
    mtx_mult(m.m);
    dl = dl_select_layer(4, 0);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = func_088254FC(dl);
    dl_set_list(dl);
    mtx_pop();
}

void *func_088254FC(void *dl)
{
    dl = gim_set_texture(dl, dd_gim_get(0x40006));
    dl = dl_enable(dl, 4);
    dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
    return draw_array(dl, 4, 0x183, 4, 0, D_08876228);
}

void func_08825580(void)
{
}

/* closes the window and frees its texts */
void func_08825588(void)
{
    DdMsgWin *m = &dd_msg_win;

    menu_reset(&m->w);
    if (dd_msg_win.title) {
        dd_free(m->title);
        m->title = 0;
    }
    if (m->text) {
        dd_free(m->text);
        m->text = 0;
    }
}

/* draws the window (Japanese font flag by language) */
void *func_088255E8(void *dl)
{
    DdMsgWin *m = &dd_msg_win;
    int flags = font_get_flags();

    if (cfg_get_language() == 0)
        font_set_flag1(1);
    else
        font_set_flag8(1);
    if (m->draw_hook)
        dl = m->draw_hook(dl);
    menu_draw(&dl, &m->w);
    font_set_flags(flags);
    return dl;
}

#ifdef NON_MATCHING /* alocação de registradores diferente (forma igual) */
/* per frame: menu update while the input is on; returns nonzero when the window is done */
int func_08825674(void)
{
    int old;
    DdMsgWin *m = &dd_msg_win;

    if (dd_msg_win.update)
        return m->update();
    old = m->w.scroll + m->w.cursor;
    if (dd_input_on())
        m->result = menu_update(&m->w);
    if (old != m->w.scroll + m->w.cursor && m->on_move)
        m->on_move(&m->w, old);
    return !func_08825728();
}
#endif

/* the window is open (not idle, opening or closed) */
int func_08825728(void)
{
    switch (dd_msg_win.w.state) {
    case 0:
    case 1:
    case 6:
        return 0;
    }
    return 1;
}

int func_08825764(void)
{
    return dd_msg_win.w.state == 3;
}

/* calls the close hook once */
void func_08825778(void)
{
    DdMsgWin *m = &dd_msg_win;

    if (dd_msg_win.on_close) {
        m->on_close();
        m->on_close = 0;
    }
}

void func_088257B8(void)
{
    menu_finish(&dd_msg_win.w);
}

int func_088257C4(void)
{
    return dd_msg_win.x13c;
}

int func_088257D0(void)
{
    return !dd_msg_win.hidden;
}

/* moves the window */
void func_088257E4(int dx, int dy)
{
    dd_msg_win.w.def.x += (short)dx;
    dd_msg_win.w.def.y += (short)dy;
}

MenuWidget *func_08825818(void)
{
    return &dd_msg_win.w;
}

/* opens the window: kind 1/3 = OK / YES-NO buttons, 0x20/0x30 = list styles; setup(w, 1) before
   and setup(w, 0) after the layout; centred on the screen */
void func_08825824(unsigned short *title, unsigned short *text, int kind, void (*setup)(MenuWidget *w, int open))
{
    DdMsgWin *m = &dd_msg_win;
    int flags;

    memset(m, 0, sizeof(DdMsgWin));
    func_08825B9C(&m->w);
    func_08825A7C(title);
    func_08825AFC(text);
    switch (kind) {
    case 0:
        break;
    case 1:
    case 3:
        m->w.buttons = kind;
        break;
    case 0x20:
    case 0x30:
        m->w.style = kind / 16;
        break;
    }
    m->w.def.max_w = 265;
    if (setup)
        setup(&m->w, 1);
    flags = font_get_flags();
    if (cfg_get_language() == 0)
        font_set_flag1(1);
    else
        font_set_flag8(1);
    menu_open(&m->w);
    font_set_flags(flags);
    m->w.def.x = 314 - m->w.def.w / 2;
    m->w.def.y = 132 - m->w.def.h / 2;
    if (setup)
        setup(&m->w, 0);
}

void func_088259D8(void)
{
    menu_reset(&dd_msg_win.w);
}

void func_088259E4(void *(*fn)(void *dl))
{
    dd_msg_win.draw_hook = fn;
}

void func_088259F0(void (*fn)(MenuWidget *w, int old))
{
    dd_msg_win.on_move = fn;
}

void func_088259FC(void (*fn)(void))
{
    dd_msg_win.on_close = fn;
}

void func_08825A08(int (*fn)(void))
{
    dd_msg_win.update = fn;
}

#ifdef NON_MATCHING /* o original tem um "b; move v0,zero" morto a mais, não reproduzido */
/* the answer: with buttons only 1-3 count */
int func_08825A14(void)
{
    DdMsgWin *m = &dd_msg_win;

    if (dd_msg_win.w.buttons) {
        switch (m->result) {
        case 1:
        case 2:
        case 3:
            return m->result;
        }
        return 0;
    }
    return m->result;
}
#endif

void func_08825A70(int v)
{
    dd_msg_win.result = v;
}

/* heap copy of the title */
void func_08825A7C(unsigned short *title)
{
    DdMsgWin *m = &dd_msg_win;

    if (dd_msg_win.title) {
        dd_free(m->title);
        m->title = 0;
    }
    if (title) {
        m->title = dd_alloc((wcslen(title) + 1) * 2);
        if (m->title)
            wcscpy(m->title, title);
    }
    m->w.def.title = m->title;
}

/* heap copy of the text */
void func_08825AFC(unsigned short *text)
{
    DdMsgWin *m = &dd_msg_win;

    if (dd_msg_win.text) {
        dd_free(m->text);
        m->text = 0;
    }
    if (text) {
        m->text = dd_alloc((wcslen(text) + 1) * 2);
        if (m->text)
            wcscpy(m->text, text);
    }
    m->w.def.text = m->text;
}

void func_08825B7C(int v)
{
    dd_msg_win.x13c = v;
}

void func_08825B88(int on)
{
    dd_msg_win.hidden = !on;
}

/* default settings of the widget */
void func_08825B9C(MenuWidget *w)
{
    MenuItemDef *d = &w->def;

    memset(w, 0, sizeof(MenuWidget));
    d->visible = 1;
    d->gim = 0;
    d->style = 0;
    d->style2 = 2;
    d->tab = 0;
    d->x = 0;
    d->y = 0;
    d->z = 0;
    d->auto_size = 1;
    d->w = 0;
    d->h = 0;
    d->color = 0xFFFFFFFF;
    d->tab_x = 0;
    d->show_title = 1;
    d->auto_title = 1;
    d->title_w = 0;
    d->title_h = 0;
    d->text = 0;
    d->font_flag4 = 1;
    d->pad_l = 10;
    d->pad_r = 10;
    d->pad_t = 8;
    d->pad_b = 8;
    d->font_size = 12;
    d->text_color = 0xFF000000;
    d->title = 0;
    d->title_font_size = 12;
    d->title_color = 0xFFFFFFFF;
    d->open_anim = 2;
    d->close_anim = 2;
    d->scale_x = 1.0f;
    d->scale_y = 1.0f;
    d->anchor = 0;
    w->heap = dd_heap_get();
    w->no_input = 0;
    w->no_cancel = 1;
    w->cancel_blocked = 1;
    w->keep_open = 0;
    w->cancel_resets = 0;
    w->accept_blocked = 0;
    w->no_wrap = 0;
    w->skip_disabled = 0;
    w->style = 0;
    w->buttons = 0;
    w->count = 0;
    w->rows = 0;
    w->scroll = 0;
    w->cursor = 0;
    w->button = 0;
    w->initial = 0;
    w->selected = 0;
    w->disabled = 0;
    w->arrow_off = 0;
    w->item_w = 0;
    w->item_h = 0;
    w->item_x = 0;
    w->item_y = 0;
    w->cursor_w = 0;
    w->cursor_h = 0;
    w->draw_item = 0;
    w->draw_all = 0;
    w->draw_cursor = 0;
    w->draw_cursor_post = 0;
    w->draw_selected = 0;
    w->draw_selected_post = 0;
}
