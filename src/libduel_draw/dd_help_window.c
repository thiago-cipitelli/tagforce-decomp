// @range 0x08861238-0x088621A0
// @asm 0x08861494-0x08861638
// @asm 0x088617A8-0x088617E4
// @asm 0x08861DD0-0x08861F24
// @asm 0x08861F24-0x08862180
/* Duel help window: a framed list of help pages (text entries first..last of a table pack),
   opened with an animation, moved with the d-pad, closed by confirm/cancel; plus the scrolling
   list widget it is built on (layout of the items, scrolling to the cursor, drawing). */
#include "dd_duelui.h"

typedef struct HelpWin {               /* 0x08C5B44C, 0x120 */
    int state;                         /* 0x000 0 idle, 1 open, 2 opening, 3 active, 4 blink, 5 close, 6 closing, 7 closed */
    void *heap;                        /* 0x004 */
    void *frame_gim;                   /* 0x008 */
    void *parts_gim;                   /* 0x00C */
    void *texts;                       /* 0x010 */
    void *pack;                        /* 0x014 */
    DdList list;                       /* 0x018 */
    Menu menu;                         /* 0x04C */
    MenuItemDef def;                   /* 0x098 */
    int first;                         /* 0x110 */
    int last;                          /* 0x114 */
    unsigned int blink;                /* 0x118 */
    int initial;                       /* 0x11C */
} HelpWin;

extern HelpWin help_win;

void *ui_sprite(void *dl, int x, int y, int u, int v, int tw, int th);
void *ui_nine_patch(void *dl, int x, int y, int u, int v, int cw, int ch, int w, int h, int sx, int sy);
void *ui_rect(void *dl, int x, int y, int w, int h, unsigned int color);
void *ui_panel(void *dl, void *gim, int x, int y, int w, int h, unsigned int color);
void *ui_panel_pulse(void *dl, void *gim, int x, int y, int w, int h, unsigned int color, int t);
void *ui_panel_flash(void *dl, void *gim, int x, int y, int w, int h, unsigned int color, int t);
void *ui_scrollbar(void *dl, void *gim, int x, int y, int h, int total, int scroll, int len, int on);

void help_reset(int first, int last, int initial);
void *help_draw_bg(void *dl, int x, int y, int w, int h);
void *help_draw_item(void *dl, int i, int x, int y, int w, int h);
void *help_draw_item_bg(void *dl, int i, int x, int y, int w, int h);
void *help_draw_cursor(void *dl, int x, int y, int w, int h);
int help_item_h(int i, int w);
void *help_scrollbar(void *dl, int x, int y, int h, int total, int scroll);
void help_font_white(void);
void help_font_black(void);

void help_init(void *heap, void *frame_gim, void *parts_gim, void *texts, void *pack, int first, int last, int initial)
{
    DdList *l = &help_win.list;

    memset(&help_win, 0, sizeof(help_win));
    help_win.texts = texts;
    help_win.pack = pack;
    help_win.heap = heap;
    help_win.frame_gim = frame_gim;
    help_win.parts_gim = parts_gim;
    memset(l, 0, sizeof(*l));
    l->x = 75;
    l->y = 30;
    l->w = 401;
    l->h = 208;
    l->pad_t = 4;
    l->pad_b = 5;
    l->pad_l = 5;
    l->pad_r = 12;
    l->draw_bg = help_draw_bg;
    l->draw_item = help_draw_item;
    l->draw_item_bg = help_draw_item_bg;
    l->draw_cursor = help_draw_cursor;
    l->item_h = help_item_h;
    list_reset(l);
    help_reset(first, last, initial);
}

void help_reset(int first, int last, int initial)
{
    DdList *l;
    MenuItemDef *d;

    l = &help_win.list;
    help_win.state = 0;
    help_win.first = first;
    help_win.last = last;
    l->count = last - first + 1;
    help_win.blink = 0;
    help_win.initial = initial;
    d = &help_win.def;
    l->active = 1;
    l->cursor = initial;
    l->result = 0;
    menu_free_items(&help_win.menu);
    menu_init(&help_win.menu, help_win.heap);
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->x = 75;
    d->y = 30;
    d->w = 401;
    d->h = 208;
    d->color = 0xFFFFFFFF;
    d->style = 0;
    menu_add_item(&help_win.menu, d);
}

#ifdef NON_MATCHING /* só alocação de registradores difere */
void help_update(void)
{
    HelpWin *w = &help_win;
    DdList *l = &help_win.list;
    MenuItemDef *d = &help_win.def;

    list_layout(l);
    switch (help_win.state) {
    case 0:
    case 7:
        break;
    case 1:
        menu_item_open(&w->menu, d);
        w->state = 2;
        se_play(0x6216);
        break;
    case 2:
        if (menu_item_is_open(&w->menu, d)) {
            l->active = 0;
            w->state = 3;
        }
        break;
    case 3:
        switch (l->result) {
        case 0:
            break;
        case 1:
            w->state = 4;
            l->active = 1;
            break;
        case 2:
            l->active = 1;
            w->state = 5;
            l->cursor = w->initial;
            break;
        }
        break;
    case 4:
        w->blink++;
        if (w->blink >= 8)
            w->state = 5;
        break;
    case 5:
        menu_item_close(&w->menu, d);
        w->state = 6;
        break;
    case 6:
        if (menu_item_is_open(&w->menu, d))
            w->state = 7;
        break;
    }
}
#else
void help_update(void);
#endif

void *help_draw(void *dl)
{
    DdList *l = &help_win.list;
    unsigned short *text;

    dl = list_draw(dl, l);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_enable(dl, 9);
    dl = gim_set_texture(dl, help_win.frame_gim);
    dl = ui_nine_patch(dl, 75, 4, 22, 10, 5, 5, 401, 24, 2, 2);
    help_font_white();
    font_text_set(dl);
    text = table_get_entry(help_win.texts, l->cursor + help_win.first);
    font_draw_text(80 << 6, (region_get() == 0 ? 6 : 10) << 6, 400 << 6, text);
    return font_text_clear();
}

void help_term(void)
{
    list_term(&help_win.list);
    menu_free_items(&help_win.menu);
    memset(&help_win, 0, sizeof(help_win));
}

void help_set_active(int on)
{
    help_win.list.active = on;
}

void help_open(void)
{
    help_win.state = 1;
}

int help_cursor(void)
{
    return list_cursor(&help_win.list);
}

/* 0 open, 1 closed by cancel, 2 closed by accept */
#ifdef NON_MATCHING /* só alocação de registradores difere */
int help_result(void)
{
    DdList *l = &help_win.list;

    if (help_win.state != 7)
        return 0;
    if (l->result == 2)
        return 2;
    return 1;
}
#else
int help_result(void);
#endif

void *help_draw_bg(void *dl, int x, int y, int w, int h)
{
    DdList *l = &help_win.list;

    menu_layout(&dl, &help_win.menu);
    dl = help_scrollbar(dl, 465, 30, 208, l->scroll, l->total);
    return dl;
}

/* the window content is drawn only while open (states 3..5) */
#define IF_HIDDEN_RETURN(dl) \
    switch (help_win.state) { \
    case 0: case 1: case 2: case 6: case 7: \
        return dl; \
    }

void *help_draw_item(void *dl, int i, int x, int y, int w, int h)
{
    HelpWin *hw = &help_win;
    unsigned short *text;

    IF_HIDDEN_RETURN(dl);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_enable(dl, 9);
    dl = gim_set_texture(dl, hw->frame_gim);
    dl = ui_sprite(dl, x + 3, y + (region_get() == 0 ? 6 : 0), 42, 0, 16, 16);
    help_font_black();
    font_text_set(dl);
    text = table_get_entry(hw->texts, i + hw->first);
    font_draw_wtext_ex((x + 21) << 6, y << 6, (w - 19) << 6, h << 6, text, -1);
    return font_text_clear();
}

void *help_draw_item_bg(void *dl, int i, int x, int y, int w, int h)
{
    HelpWin *hw = &help_win;

    IF_HIDDEN_RETURN(dl);
    return ui_panel(dl, hw->parts_gim, x, y + 1, w, h - 2, 0xFFA1F9FF);
}

void *help_draw_cursor(void *dl, int x, int y, int w, int h)
{
    HelpWin *hw = &help_win;
    DdList *l = &help_win.list;

    IF_HIDDEN_RETURN(dl);
    if (hw->state == 4)
        dl = ui_panel_flash(dl, hw->parts_gim, x, y, w, h, 0, *gfx_get_state());
    else if (l->active)
        dl = ui_panel_pulse(dl, hw->parts_gim, x, y, w, h, 0, *gfx_get_state());
    else
        dl = ui_panel(dl, hw->parts_gim, x, y, w, h, 0);
    return dl;
}

void *help_scrollbar(void *dl, int x, int y, int h, int total, int scroll)
{
    HelpWin *hw = &help_win;

    IF_HIDDEN_RETURN(dl);
    dl = ui_rect(dl, x, y + 3, 1, 202, 0xFF479FC1);
    return ui_scrollbar(dl, hw->frame_gim, x, y + 3, h - 6, total, scroll, h - 9, 1);
}

int help_item_h(int i, int w)
{
    unsigned short *text = table_get_entry(help_win.texts, i + help_win.first);

    table_get_entry(help_win.pack, 2);
    help_font_black();
    return ceilf_vfpu((float)font_wtext_lines((w - 19) << 6, text) / 64.0f);
}

void help_font_flags(void)
{
    if (region_get() == 0) {
        font_set_flag1(1);
        font_set_flag8000(1);
    } else {
        font_set_flag8(1);
        font_set_flag8000(0);
    }
    font_set_flag4(1);
    font_set_flag2(1);
}

void help_font_white(void)
{
    help_font_flags();
    font_set_size(14, 14);
    font_set_color(0xFFFFFFFF);
}

void help_font_black(void)
{
    help_font_flags();
    font_set_size(14, 14);
    font_set_color(0xFF000000);
}

void list_reset(DdList *l)
{
    listw_input_reset(l);
    l->scroll = 0;
    l->total = 0;
}

/* input, then scrolls so the cursor item is inside the window and sums the item heights */
#ifdef NON_MATCHING /* só alocação de registradores difere */
void list_layout(DdList *l)
{
    int w = l->w - l->pad_l - l->pad_r;
    int above, cur, top, bottom, end;
    int i;

    listw_input(l);
    above = 0;
    for (i = 0; i < l->cursor; i++)
        above += l->item_h(i, w);
    cur = above + l->item_h(l->cursor, w);
    top = l->y + l->pad_t;
    end = l->pad_t + (l->scroll + (l->y + above));
    if (end < top)
        l->scroll += (short)(top - end);
    end = l->pad_t + (l->scroll + (l->y + cur));
    bottom = l->y + l->h - l->pad_b;
    if (end > bottom)
        l->scroll += (short)(bottom - end);
    l->total = 0;
    for (i = 0; i < l->count; i++)
        l->total += l->item_h(i, w);
}
#else
void list_layout(DdList *l);
#endif

#ifdef NON_MATCHING /* só alocação de registradores difere */
void *list_draw(void *dl, DdList *l)
{
    int w, x0, y0, x, y, cw, ch;
    int sum, yy, h;
    int i;

    if (l->draw_bg != 0)
        dl = l->draw_bg(dl, l->x, l->y, l->w, l->h);
    if (l->draw_item == 0 && l->draw_item_bg == 0 && l->draw_cursor == 0)
        return dl;
    x0 = l->x;
    w = l->w - l->pad_l - l->pad_r;
    sum = 0;
    if (x0 < 480) {
        y0 = l->y;
        if (y0 < 272) {
            x = x0 + l->pad_l;
            if (x < 0)
                x = 0;
            y = y0 + l->pad_t;
            if (y < 0)
                y = 0;
            ch = l->h - l->pad_t - l->pad_b;
            cw = w;
            if (x + w > 480)
                cw = 480 - x0;
            if (y + ch > 272)
                ch = 272 - y0;
            dl = dl_scissor(dl, x, y, cw, ch);
            dl = dl_enable(dl, 2);
        }
    }
    for (i = 0; i < l->count; i++) {
        yy = l->scroll + sum + l->y + l->pad_t;
        h = l->item_h(i, w);
        sum += h;
        if (yy + h > l->y && yy < l->y + l->h) {
            void *d2 = l->draw_item_bg(dl, i, l->x + l->pad_l, yy, w, h);

            if (l->cursor == i && l->draw_cursor != 0)
                d2 = l->draw_cursor(d2, l->x + l->pad_l, yy, w, h);
            dl = l->draw_item(d2, i, l->x + l->pad_l, yy, w, h);
        }
    }
    return dl_disable(dl, 2);
}
#else
void *list_draw(void *dl, DdList *l);
#endif

void list_term(DdList *l)
{
    listw_term(l);
}

void list_set_active(DdList *l, int on)
{
    listw_set_active(l, on);
}

int list_cursor(DdList *l)
{
    return listw_cursor(l);
}

void list_set_cursor(DdList *l, int c)
{
    listw_set_cursor(l, c);
}
