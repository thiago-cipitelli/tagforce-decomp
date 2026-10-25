// @range 0x088106D8-0x08813088
// @asm 0x088108A0-0x08810ADC
// @asm 0x08810CDC-0x08811658
// @asm 0x08811A8C-0x08811FBC
// @asm 0x08812200-0x08812674
// @asm 0x08812910-0x08812AB8
/* The title's panel widgets: a framed box of cells with a scrollbar (TBox), tabs (TTab), the tabbed
   window combining them with a system list (TPanel), the nine-piece frames drawn from title_sprites2,
   the blinking cursor frame, and preset boxes and help windows used by the duel setup screens. */
#include "libtitle.h"

void tbox_init(TBox *b);
void ttab_init(TTab *t);
void tpanel_list_init(TPanel *p, void *heap);
void *tbox_draw(void *list, TBox *b);
void *ttab_draw(void *list, TTab *t, unsigned short i, int a, int sel);
void *tpanel_draw_split(void *list, TPanel *p);
void *tframe_box(void *list, unsigned int color, int x, int y, int w, int h, int kind, int style, int mode);
void *tframe_cursor(void *list, unsigned int color, int x, int y, int w, int h);
void tframe_cursor_mode(int mode);
void *tframe_bar(void *list, int sel, unsigned int color, int x, int y, int w, int h);
void *tframe(void *list, unsigned int color, int x, int y, int w, int h, int kind, int style,
             int mode);
void *tframe_window(void *list, unsigned int color, int x, int y, int w, int h);
void *tframe_tab(void *list, unsigned int color, short x, short y);
void tscroll_calc(short *pos, short *len, TBox *b, short x, short y, short h);
void tscroll_calc2(short *pos, short *len, int count, int n, int top, int sel, int x, short y, short h);
void *tscroll_draw(void *list, unsigned int color, int x, int y, int h, int pos, int len);
void *tpanel_cursor(void *list);

void tbox_init(TBox *b)
{
    memset(b, 0, sizeof(*b));
    b->state = 0;
    b->x = 50;
    b->y = 50;
    b->w = 380;
    b->h = 170;
    b->color = 0xFFFFFFFF;
    b->color2 = 0xFFFFFFFF;
    b->style = 1;
    b->no_frame = 0;
    b->f1 = 0;
    b->no_cursor = 0;
    b->no_cells = 0;
    b->f4 = 0;
    b->no_scroll = 0;
    b->f6 = 0;
    b->pos = 0;
    b->cursor = 0;
    b->n = 8;
    b->count = 8;
    b->cols = 1;
    b->ix = b->x + 5;
    b->iy = b->y + 5;
    b->iw = 391;
    b->ih = 34;
    b->text = 0;
    b->draw = 0;
}

void ttab_init(TTab *t)
{
    t->state = 0;
    t->x = 0;
    t->y = 0;
    t->w = 64;
    t->h = 15;
    t->pos = 0;
    t->cap = 1;
    t->color = 0xFFFFFFFF;
    t->text_color = 0xFFFFFFFF;
    t->color_off = 0xFF808080;
    t->text_color_off = 0xFF808080;
    t->u1c = 0;
    t->draw = 0;
}

#ifdef NON_MATCHING /* cópia de p->n para o divisor sai com andi no original; registradores trocados */
void tpanel_init(TPanel *p, void *heap)
{
    TTab *t;
    TBox *b;
    int i;
    int n;

    p->f0 = 1;
    p->f2 = 1;
    p->x = 106;
    p->y = 39;
    p->w = 268;
    p->h = 218;
    p->cur = 0;
    p->n = 2;
    t = p->tab;
    b = &p->box;
    n = 1;
    if (p->n != 0)
        n = p->n;
    for (i = 0; i < 4; i++, t++) {
        ttab_init(t);
        if (i == 0) {
            t->pos = 0;
            t->cap = 1;
        } else if (i < p->n - 1) {
            t->pos = 1;
            t->cap = 2;
        } else {
            t->pos = 2;
            t->cap = 3;
        }
        t->w = p->w / n;
        t->h = 15;
        t->x = p->x + t->w * i;
        t->y = p->y;
    }
    tbox_init(b);
    b->x = p->x;
    b->y = p->y + p->tab[0].h;
    b->w = p->w;
    b->h = p->h - p->tab[0].h;
    b->ix = b->x + 5;
    b->iy = b->y + 6;
    b->iw = 248;
    b->ih = 23;
    if (b->h < 0)
        b->h = 0;
    tpanel_list_init(p, heap);
}
#endif

/* the system list of the panel */
void tpanel_list_init(TPanel *p, void *heap)
{
    TBox *b = &p->box;
    MenuWidget *w = &p->mw;
    MenuItemDef *d = &w->def;

    menu_reset(w);
    memset(w, 0, sizeof(*w));
    d->visible = 1;
    d->style = 0;
    d->style2 = 2;
    d->x = b->x;
    d->y = b->y;
    d->auto_size = 13;
    d->w = b->w;
    d->h = b->h;
    d->color = 0xFFFFFF;
    d->text = 0;
    d->font_flag4 = 1;
    d->pad_l = 5;
    d->pad_r = 4;
    d->pad_t = 6;
    d->pad_b = 0;
    d->font_size = 0;
    d->text_color = 0xFFFFFFFF;
    d->title = 0;
    d->title_font_size = 14;
    d->title_color = 0xFFFFFFFF;
    d->open_anim = 0;
    d->close_anim = 0;
    d->show_title = 1;
    d->auto_title = 1;
    w->heap = heap;
    w->no_input = 0;
    w->no_cancel = 1;
    w->cancel_blocked = 0;
    w->keep_open = 1;
    w->f0_4 = 0;
    w->cancel_resets = 0;
    w->style = 2;
    w->count = 20;
    w->rows = 8;
    w->arrow_off = 136;
    w->item_w = b->iw - (d->pad_l + d->pad_r);
    w->item_h = b->ih + 1;
    w->cursor_w = w->item_w;
    w->cursor_h = 14;
    w->draw_item = 0;
    w->draw_cursor = (MenuDrawCursorFn)tpanel_cursor;
    twin_open(w);
}

#ifdef NON_MATCHING /* ordem de avaliação dos argumentos dos laços de células (registradores a1/a2) */
/* box frame, the cells, the text, the scrollbar, the cell callbacks and the cursor */
void *tbox_draw(void *list, TBox *b)
{
    int sel = b->cursor;
    short cols = b->cols;
    int mode = 0;
    short rows;
    short cw, ch;
    short pos, len;
    short sx, sy, sh;
    int i;

    if (cols == 0)
        cols = 1;
    rows = b->n / cols;
    if (b->n <= sel)
        sel = b->n - 1;
    if (!b->no_scroll)
        mode = 1;
    if (!b->no_frame)
        list = tframe_box(list, b->color, b->x, b->y, b->w, b->h, b->frame, b->style, mode);
    if (!b->no_cells) {
        cw = b->iw;
        ch = b->ih;
        for (i = 0; i < b->n; i++)
            list = tframe_box(list, b->color, (short)(b->ix + (b->iw + 3) * (i / rows)),
                              (short)(b->iy + (b->ih + 1) * (i % rows)), cw, ch, 0, 3, 0);
    }
    if (b->text)
        list = title_text_draw(list, b->ix, b->iy, b->iw, b->ih, b->text, 4);
    if (!b->no_scroll) {
        sx = b->x + b->w - 11;
        sy = b->y + 12;
        sh = b->h - 24;
        tscroll_calc(&pos, &len, b, sx, sy, sh);
        list = tscroll_draw(list, 0xFFFFFFFF, sx, sy, sh, pos, len);
    }
    if (b->draw) {
        cw = b->iw;
        ch = b->ih;
        for (i = 0; i < b->n; i++)
            list = b->draw(list, (unsigned short)i, (short)(b->ix + (b->iw + 3) * (i / rows)),
                           (short)(b->iy + (b->ih + 1) * (i % rows)), cw, ch);
    }
    if (!b->no_cursor)
        list = tframe_cursor(list, b->color, (short)(b->ix + (b->iw + 3) * (sel / rows)),
                             (short)(b->iy + (b->ih + 1) * (sel % rows)), b->iw, b->ih);
    return list;
}
#endif

#ifdef NON_MATCHING /* tamanho certo; registradores e seh da largura de cada peça diferem */
/* one tab: three pieces, coloured by the selection, then its label callback */
void *ttab_draw(void *list, TTab *t, unsigned short i, int a, int sel)
{
    TSpr16 s;
    int show = 1;
    int k = 16;
    int j;

    switch (a) {
    case 0:
        break;
    case 1:
        if (!sel)
            show = 0;
        break;
    case 2:
        if (sel)
            show = 0;
        break;
    }
    if (show) {
        switch (t->pos) {
        case 0:
            if (sel)
                k = 12;
            else
                k = 21;
            break;
        case 1:
            if (sel)
                k = 15;
            else
                k = 24;
            break;
        case 2:
            if (sel)
                k = 18;
            else
                k = 27;
            break;
        }
        list = tmenu_gfx_setup(list, 0);
        if (sel)
            list = dl_color(list, t->color);
        else
            list = dl_color(list, t->color_off);
        list = gim_set_texture(list, ehp_find(title_pack(), title_sprite_gim(title_sprites2, 16), 0));
        for (j = 0; j < 3; j++) {
            short v, w, x, y, h, u, px, py;
            int pw;

            s = title_sprites2[k + j];
            v = s.v + (short)(t->cap << 5);
            w = t->w;
            x = s.x;
            y = s.y;
            pw = s.w;
            h = s.h;
            u = s.u;
            if (w < 7)
                w = 7;
            switch (j) {
            case 1:
                pw = w - 7;
                break;
            case 2:
                x = w - 4;
                break;
            }
            px = x + t->x;
            py = y + t->y;
            list = draw_set_offsets(list, s.tw, s.th, pw, h);
            list = draw_sprite(list, px, py, 0, u, v, s.a, s.b);
        }
        if (t->draw)
            list = ((void *(*)(void *, int, int, int, int, int))t->draw)(list, i, t->x, t->y, t->w, t->h);
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* registradores do laço das abas e das células (o original chama tframe_tab com int) */
/* the two-column layout of the panel (f0 == 2) */
void *tpanel_draw_split(void *list, TPanel *p)
{
    TBox *b = &p->box;
    int sel = b->cursor;
    short ix, iy, iw, ih;
    short pos, len;
    short sx, sy, sh;
    int x, w;
    int i;

    if (b->n <= sel)
        sel = b->n - 1;
    list = tframe_window(list, 0xFFFFFFFF, p->x, p->y, p->w, p->h);
    for (i = 0; i < 2; i++) {
        if (i == 0) {
            w = 83;
            x = p->x + 27;
        } else {
            w = 355;
            x = p->x + 115;
        }
        list = tframe_tab(list, 0xFFFFFFFF, x, p->y);
        if (p->draw2)
            list = p->draw2(list, (unsigned short)i, x + 2, p->y, w, 0);
    }
    if (!b->no_cells) {
        ix = b->ix;
        iw = b->iw;
        ih = b->ih;
        for (i = 0; i < b->n; i++) {
            iy = b->iy + i * (b->ih + 1);
            list = tframe_box(list, b->color, ix, iy, iw, ih, 0, 3, 0);
            list = tframe_bar(list, 0, 0xFFFFFFFF, ix + 23, iy + 4, 88, 14);
        }
    }
    if (!b->no_cursor) {
        iy = b->iy + sel * (b->ih + 1);
        list = tframe_bar(list, 1, 0xFFFFFFFF, b->ix + 23, iy + 4, 88, 14);
        list = tframe_cursor(list, b->color, (short)(b->ix - 1), (short)(b->iy - 1 + sel * (b->ih + 1)),
                             (short)(b->iw + 2), (short)(b->ih + 2));
    }
    if (!b->no_scroll) {
        sx = b->x + b->w - 12;
        sy = b->y + 12;
        sh = b->h - 24;
        tscroll_calc(&pos, &len, b, sx, sy, sh);
        list = tscroll_draw(list, 0xFFFFFFFF, sx, sy, sh, pos, len);
    }
    if (b->draw) {
        ix = b->ix;
        iy = b->iy;
        iw = b->iw;
        ih = b->ih;
        for (i = 0; i < b->n; i++) {
            list = b->draw(list, (unsigned short)i, ix, iy, iw, ih);
            iy += b->ih;
        }
    }
    return list;
}
#endif

/* L/R change the tab, the list takes the rest; -1 while running (see topt_update) */
int tpanel_update(TPanel *p)
{
    TBox *b = &p->box;
    MenuWidget *w = &p->mw;
    int last = 0;
    unsigned int trig = pad_get()->b.trig;
    int r;

    if (trig)
        tframe_cursor_mode(0);
    if (p->n > 0)
        last = p->n - 1;
    if (trig & 0x200) {
        if (p->cur < last)
            p->cur++;
    } else if (trig & 0x100) {
        if (p->cur > 0)
            p->cur--;
    }
    if (p->f2) {
        r = menu_update(w);
        b->pos = w->scroll + w->cursor;
        b->cursor = w->cursor;
    }
    return r;
}

void *tpanel_draw(void *list, TPanel *p)
{
    TBox *b = &p->box;
    MenuWidget *w = &p->mw;
    unsigned short i;
    TTab *t;

    if (p->n)
        b->style = p->tab[p->cur].cap;
    t = p->tab;
    if (p->f0 == 1) {
        list = tbox_draw(list, b);
        for (i = 0; i < p->n; i++, t++)
            list = ttab_draw(list, t, i, 0, p->cur == i);
    } else if (p->f0 == 2) {
        list = tpanel_draw_split(list, p);
    }
    if (p->f2)
        menu_draw(&list, w);
    return list;
}

void *tframe_box(void *list, unsigned int color, int x, int y, int w, int h, int kind, int style, int mode)
{
    return tframe(list, color, x, y, w, h, kind, style, mode);
}

/* the cursor frame; it blinks while cursor_mode is set (mode 2 stops after a few frames) */
void *tframe_cursor(void *list, unsigned int color, int x, int y, int w, int h)
{
    Title *const t = &title;
    int k = title.ticks % 16 / 8 + 1;

    x -= k;
    y -= k;
    w += k * 2;
    h += k * 2;
    if (title.cursor_mode) {
        if (t->cursor_blink % 4 >= 2)
            list = tframe(list, color, x, y, w, h, 40, 0, 0);
        t->cursor_blink++;
        if (t->cursor_blink >= 0x1000)
            t->cursor_blink = 0;
        if (t->cursor_mode == 2 && t->cursor_blink >= 15)
            tframe_cursor_mode(0);
    } else {
        list = tframe(list, color, x, y, w, h, 40, 0, 0);
    }
    return list;
}

void tframe_cursor_mode(int mode)
{
    Title *const t = &title;

    title.cursor_blink = 0;
    if (mode)
        t->cursor_mode = mode;
    else
        t->cursor_mode = 0;
}

void *tframe_bar(void *list, int sel, unsigned int color, int x, int y, int w, int h)
{
    if (sel)
        return tframe(list, color, x, y, w, h, 70, 1, 0);
    else
        return tframe(list, color, x, y, w, h, 70, 0, 0);
}

#ifdef NON_MATCHING /* quadro de nove peças com switch; registradores e spills diferem */
/* nine-piece frame from title_sprites2[kind..]; style selects the texture row, mode 1 the shadow */
void *tframe(void *list, unsigned int color, int x, int y, int w, int h, int kind, int style,
             int mode)
{
    TSpr16 s;
    int passes = 1;
    int n = 9;
    int last = 2;
    short du = 0;
    short dv = 0;
    short x1, x2, y1, y2;
    short px, py, pw, ph;
    short uv;
    int pass;
    int i;

    if (kind == 70) {
        du = style * 14;
        mode = 0;
        n = 3;
    } else if (kind == 73) {
        n = 1;
        mode = 0;
    } else if (kind == 102) {
        mode = 0;
        n = 3;
        passes = 2;
    } else if (kind == 93) {
        if (style < 13 || style > 16)
            style = 13;
        dv = (style - 13) * 16;
        mode = 0;
    } else if (kind == 84 || kind == 75) {
        if (style < 4 || style > 12)
            style = 4;
        style -= 4;
        du = style / 3 * 13;
        mode = 0;
        dv = style % 3 * 13;
    } else {
        dv = style << 5;
    }
    if (mode == 1) {
        last = 9;
        n = 12;
    }
    x1 = x + title_sprites2[kind].tw;
    w -= title_sprites2[kind].tw + title_sprites2[kind + last].tw;
    if (w < 0)
        w = 0;
    x2 = x1 + w;
    y1 = y + title_sprites2[kind].th;
    h -= title_sprites2[kind].th + title_sprites2[kind + 6].th;
    if (h < 0)
        h = 0;
    y2 = y1 + h;
    list = tmenu_gfx_setup(list, 0);
    list = dl_color(list, color);
    list = gim_set_texture(list, ehp_find(title_pack(), title_sprite_gim(title_sprites2, kind), 0));
    for (pass = 0, uv = 0; pass < passes; pass++, uv += 32) {
        if (pass != 0 && kind == 102)
            dv = uv;
        for (i = kind; i < kind + n; i++) {
            s = title_sprites2[i];
            pw = s.tw;
            ph = s.th;
            px = x;
            py = y;
            switch (i - kind) {
            case 1:
                pw = w;
                px = x1;
                break;
            case 2:
                px = x2;
                break;
            case 3:
                ph = h;
                py = y1;
                break;
            case 4:
                pw = w;
                ph = h;
                px = x1;
                py = y1;
                break;
            case 5:
                ph = h;
                px = x2;
                py = y1;
                break;
            case 6:
                py = y2;
                break;
            case 7:
                pw = w;
                px = x1;
                py = y2;
                break;
            case 8:
                px = x2;
                py = y2;
                break;
            case 9:
                px = x2;
                break;
            case 10:
                ph = h;
                px = x2;
                py = y1;
                break;
            case 11:
                px = x2;
                py = y2;
                break;
            }
            list = draw_set_offsets(list, s.tw, s.th, pw, ph);
            list = draw_sprite(list, px, py, 0, (short)(s.u + du), (short)(s.v + dv), s.a, s.b);
        }
    }
    return list;
}
#endif

void *tframe_window(void *list, unsigned int color, int x, int y, int w, int h)
{
    return tframe(list, color, x, y, w, h, 30, 0, 0);
}

/* a tab head of the two-column layout */
void *tframe_tab(void *list, unsigned int color, short x, short y)
{
    short u, v, a, b, th, tw;

    list = tmenu_gfx_setup(list, 0);
    list = dl_color(list, color);
    list = gim_set_texture(list, ehp_find(title_pack(), title_sprite_gim(title_sprites2, 30), 0));
    u = title_sprites2[39].u;
    v = title_sprites2[39].v;
    a = title_sprites2[39].a;
    b = title_sprites2[39].b;
    th = title_sprites2[39].th;
    tw = title_sprites2[39].tw;
    list = draw_set_offsets(list, tw, th, tw, th);
    return draw_sprite(list, x, y, 0, u, v, a, b);
}

/* scrollbar thumb of a box: position and length */
void tscroll_calc(short *pos, short *len, TBox *b, short x, short y, short h)
{
    int hidden = b->count - b->n;
    int range = b->count - 1;
    int top = b->pos - b->cursor;
    int in, span;

    if (range <= 0)
        range = 1;
    if (hidden <= 0)
        hidden = 0;
    in = h - 2;
    span = in - 8;
    *len = span + 8 - span * hidden / range;
    if (top < 0)
        top = 0;
    if (hidden <= 0)
        hidden = 1;
    *pos = y + 1 + top * (in - *len) / hidden;
}

void tscroll_calc2(short *pos, short *len, int count, int n, int top, int sel, int x, short y, short h)
{
    int range;
    int hidden;
    int in, span;

    top -= sel;
    range = count - 1;
    hidden = count - n;
    if (range <= 0)
        range = 1;
    if (hidden <= 0)
        hidden = 0;
    in = h - 2;
    span = in - 8;
    *len = span + 8 - span * hidden / range;
    if (top < 0)
        top = 0;
    if (hidden <= 0)
        hidden = 1;
    *pos = y + 1 + top * (in - *len) / hidden;
}

#ifdef NON_MATCHING /* o original guarda quase todos os temporários na pilha (sw/lh); forma não reproduzida */
/* vertical scrollbar: track, thumb and the two arrows (blinking when there is more to scroll) */
void *tscroll_draw(void *list, unsigned int color, int x, int y, int h, int pos, int len)
{
    TSpr16 s;
    short x1 = x + 1;
    short y0 = y;
    short th = h - 4;
    short ty;
    short tl;
    short px, py, ph;
    int show;
    int i;

    if (th < 0)
        th = 0;
    ty = pos + 3;
    tl = len - 6;
    if (tl < 0)
        tl = 0;
    list = tmenu_gfx_setup(list, 0);
    list = dl_color(list, color);
    list = gim_set_texture(list, ehp_find(title_pack(), title_sprite_gim(title_sprites2, 49), 0));
    for (i = 53; i < 59; i++) {
        s = title_sprites2[i];
        ph = s.th;
        px = x;
        py = y0;
        switch (i) {
        case 54:
            ph = th;
            py = y0 + 2;
            break;
        case 55:
            py = y0 + 2 + th;
            break;
        case 56:
            px = x1;
            py = pos;
            break;
        case 57:
            ph = tl;
            px = x1;
            py = ty;
            break;
        case 58:
            px = x1;
            py = ty + tl;
            break;
        }
        list = draw_set_offsets(list, s.tw, s.th, s.tw, ph);
        list = draw_sprite(list, px, py, 0, s.u, s.v, s.a, s.b);
    }
    for (i = 49; i < 53; i++) {
        s = title_sprites2[i];
        px = s.x + x;
        py = s.y + y0;
        show = 0;
        switch (i) {
        case 49:
            show = 1;
        case 51:
            if (y + 1 != pos)
                show = 1;
            py += -5;
            break;
        case 50:
            show = 1;
        case 52:
            if (y + h - 1 != pos + len)
                show = 1;
            py += (short)h + 1;
            break;
        }
        if ((unsigned int)(i - 51) < 2)
            list = dl_color(list, (0xFF - title_get_ticks() % 16 * 0xFF / 15) << 24 | 0xFFFFFF);
        else
            list = dl_color(list, color);
        if (show) {
            list = draw_set_offsets(list, s.tw, s.th, s.tw, s.th);
            list = draw_sprite(list, px, py, 0, s.u, s.v, s.a, s.b);
        }
    }
    return list;
}
#endif

/* the small box of the setup screens (title.panel.box2) with an optional message */
void *tbox_small(void *list, int id, int x, int y, int w, int h)

{
    TBox *b = &title.panel.box2;

    b->x = x;
    b->ix = b->x + 3;
    b->y = y;
    b->iy = b->y + 2;
    b->w = w;
    b->iw = b->w - 6;
    b->h = h;
    b->ih = b->h - 4;
    b->color = 0xFFFFFFFF;
    b->color2 = 0xFFFFFFFF;
    b->frame = 93;
    b->style = 14;
    b->no_frame = 0;
    b->n = 1;
    b->count = 1;
    b->f1 = 1;
    b->no_cursor = 1;
    b->no_cells = 1;
    b->f4 = 1;
    b->no_scroll = 1;
    b->state = 0;
    b->pos = 0;
    b->cursor = 0;
    b->text = 0;
    b->draw = 0;
    list = tbox_draw(list, b);
    if (id)
        list = title_msg_font(list, b->ix, b->iy, b->iw, 12, id, 0, 13, 13, 0xFF002020);
    return list;
}

/* the box at the bottom right of the setup screens */
void *tbox_corner(void *list, TBoxDrawFn draw)
{
    TBox *b = &title.panel.box;

    b->x = 327;
    b->ix = b->x + 5;
    b->y = 242;
    b->iy = b->y + 5;
    b->w = 151;
    b->iw = b->w - 11;
    b->h = 23;
    b->ih = b->h - 11;
    b->color = 0xFFFFFFFF;
    b->color2 = 0xFFFFFFFF;
    b->frame = 84;
    b->style = 10;
    b->no_frame = 0;
    b->n = 1;
    b->count = 1;
    b->f1 = 1;
    b->no_cursor = 1;
    b->no_cells = 1;
    b->f4 = 1;
    b->no_scroll = 1;
    b->state = 0;
    b->pos = 0;
    b->cursor = 0;
    b->text = 0;
    b->draw = draw;
    return tbox_draw(list, b);
}

#ifdef NON_MATCHING /* andi extra na releitura de title.duel.anim e ordem do pulso */
/* the title bar of the setup screens, pulsing with title.duel.anim */
void *tbox_header(void *list, TBoxDrawFn draw)
{
    TBox *b = &title.panel.box;
    unsigned short a;
    int k;

    b->x = 4;
    b->ix = b->x + 5;
    b->y = 28;
    b->iy = b->y + 5;
    b->w = 473;
    b->iw = b->w - 11;
    b->h = 23;
    b->ih = b->h - 11;
    b->color = 0xFFFFFFFF;
    b->color2 = 0xFFFFFFFF;
    b->frame = 84;
    b->style = 10;
    b->no_frame = 0;
    b->n = 1;
    b->count = 1;
    b->f1 = 1;
    b->no_cursor = 1;
    b->no_cells = 1;
    b->f4 = 1;
    b->no_scroll = 1;
    b->state = 0;
    b->pos = 0;
    b->cursor = 0;
    if (title.duel.anim != 0) {
        short d, d2;

        k = title.duel.anim / 5 % 4;
        if (k >= 3)
            k = 1;
        d = k;
        d2 = k * 2;
        b->x -= d;
        b->y -= d;
        b->w += d2;
        b->h += d2;
    }
    b->text = 0;
    b->draw = draw;
    return tbox_draw(list, b);
}
#endif

/* the help line at the bottom (higher in state 14) */
void twin_help_init(MenuWidget *w, int state, int id, int file)
{
    MenuItemDef *d = &w->def;

    twin_init(w);
    d->style = 0;
    d->open_anim = 0;
    d->close_anim = 0;
    if (state == 14) {
        d->x = 4;
        d->y = 231;
        d->w = 320;
        d->h = 34;
    } else {
        d->x = 4;
        d->y = 187;
        d->w = 320;
        d->h = 34;
    }
    d->color = 0xFFFFFFFF;
    d->title = 0;
    d->text = title_text(id, file);
    d->font_flag4 = 1;
    d->pad_l = 1;
    d->pad_r = 0;
    d->pad_t = 1;
    d->pad_b = 0;
    d->font_size = 11;
    d->text_color = 0xFF004040;
    w->no_input = 0;
    w->no_cancel = 0;
    w->cancel_blocked = 0;
    w->keep_open = 0;
    w->f0_4 = 0;
    w->cancel_resets = 0;
    w->count = 0;
    w->rows = 0;
    twin_open(w);
}

/* the help text box at the bottom */
void *tbox_help(void *list, int id, int file)
{
    TBox *b = &title.panel.box;

    b->state = 0;
    if (title.state == 14) {
        b->x = 4;
        b->y = 231;
        b->w = 320;
        b->h = 34;
    } else {
        b->x = 4;
        b->y = 187;
        b->w = 320;
        b->h = 34;
    }
    b->color = 0xFFFFFFFF;
    b->color2 = 0xFFFFFFFF;
    b->style = 3;
    b->no_frame = 0;
    b->n = 1;
    b->count = 1;
    b->f1 = 1;
    b->no_cursor = 1;
    b->no_cells = 1;
    b->f4 = 1;
    b->no_scroll = 1;
    b->ix = b->x + 5;
    b->iy = b->y + 5;
    b->iw = b->w - 11;
    b->ih = b->h - 11;
    b->frame = 0;
    b->pos = 0;
    b->cursor = 0;
    b->text = title_text(id, file);
    b->draw = 0;
    font_set_size(12, 12);
    font_set_color(0xFF004040);
    font_set_flag4(1);
    return tbox_draw(list, b);
}

/* the list box at the right with a message box on top */
void *tbox_side(void *list, int id, int n, TBoxDrawFn draw)
{
    TBox *b = &title.panel.box2;

    b->x = 327;
    b->ix = b->x + 6;
    b->y = 53;
    b->iy = b->y + 25;
    b->w = 151;
    b->h = 185;
    b->color = 0xFFFFFFFF;
    b->color2 = 0xFFFFFFFF;
    b->style = 3;
    b->no_frame = 0;
    b->count = 1;
    b->f1 = 1;
    b->no_cursor = 1;
    b->no_cells = 1;
    b->f4 = 1;
    b->no_scroll = 1;
    b->iw = 138;
    b->ih = 15;
    b->state = 0;
    b->frame = 0;
    b->pos = 0;
    b->cursor = 0;
    b->text = 0;
    b->n = n;
    b->draw = draw;
    list = tbox_draw(list, b);
    return tbox_small(list, id, b->x + 5, b->y + 5, 140, 16);
}

void *tpanel_cursor(void *list)
{
    return list;
}

/* the main box of the setup screens */
void *tbox_main(void *list, TBoxDrawFn draw)
{
    TBox *b = &title.panel.box;

    b->x = 4;
    b->ix = b->x + 5;
    b->y = 98;
    b->iy = b->y + 5;
    b->w = 320;
    b->iw = b->w - 11;
    b->h = 132;
    b->ih = b->h - 11;
    b->color = 0xFFFFFFFF;
    b->color2 = 0xFFFFFFFF;
    b->frame = 84;
    b->style = 9;
    b->no_frame = 0;
    b->n = 1;
    b->count = 1;
    b->f1 = 1;
    b->no_cursor = 1;
    b->no_cells = 1;
    b->f4 = 1;
    b->no_scroll = 1;
    b->state = 0;
    b->pos = 0;
    b->cursor = 0;
    b->text = 0;
    b->draw = draw;
    return tbox_draw(list, b);
}
