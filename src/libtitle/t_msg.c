// @range 0x0880C0F0-0x0880C998
// @asm 0x0880C340-0x0880C3C8
// @asm 0x0880C3D8-0x0880C4EC
/* The message window of the title (tmsg): a system menu frame with up to three lines of text laid out
   by the font layer, the line positions and the text pagination. */
#include "libtitle.h"

void tmsg_reset(int x, int unused, int size, int a, int style);
void tmsg_set_size(int size);
void tmsg_set_text(unsigned short *text, int b);
void tmsg_mark_done(void);
void tmsg_create(void);
void tmsg_layout(void);
void tmsg_place(void);
unsigned short tmsg_font_begin(void);
void tmsg_font_end(unsigned short flags);
void tmsg_paginate(void);

void tmsg_init(void *heap, int x, int w, int size, int a, int style)
{
    memset(&tmsg, 0, sizeof(tmsg));
    tmsg.heap = heap;
    tmsg.x = x;
    tmsg.w0 = w;
    tmsg.size = size;
    tmsg.u0ec = a;
    tmsg.style = style;
    tmsg_create();
}

void tmsg_reset(int x, int unused, int size, int a, int style)
{
    TMsgWin *const m = &tmsg;
    float f;

    tmsg.nlines = 0;
    tmsg.text = 0;
    tmsg.speed = cfg_get_byte0_bits4_5();
    tmsg.u0ec = a;
    tmsg.style = style;
    memset(tmsg.line, 0, sizeof(tmsg.line));
    tmsg.u13c = 0;
    tmsg.pos = 0;
    tmsg.u148 = 0;
    tmsg.u14c = 0;
    tmsg.u150 = 0;
    tmsg.u154 = 0;
    tmsg.wide = 0;
    f = x;
    tmsg.fy2 = 300.0f;
    tmsg.fx2 = f;
    tmsg.fx3 = f;
    tmsg.fy3 = 210.0f;
    tmsg.fx = f;
    tmsg.fy = 300.0f;
    tmsg.def.x = x;
    if (region_get() == 0) {
        m->nlines = 2;
        m->wide = 1;
    } else {
        m->wide = 0;
        m->nlines = 3;
    }
    tmsg_set_size(size);
    tmsg_set_text(0, 0);
    tmsg_place();
    m->done = 0;
}

void tmsg_free(void)
{
    tmsg.u15c = 0;
    menu_free_items(&tmsg.menu);
    memset(&tmsg, 0, sizeof(tmsg));
}

void tmsg_set_size(int size)
{
    MenuItemDef *const d = &tmsg.def;

    d->font_size = size;
    d->title_font_size = size;
}

#ifdef NON_MATCHING /* o original guarda &tmsg em $s0 antes das chamadas; o mwcc dobra o endereço */
void tmsg_set_text(unsigned short *text, int b)
{
    MenuItemDef *const d = &tmsg.def;
    TMsgWin *const m = &tmsg;

    d->show_title = b != 0;
    tmsg.u148 = 0;
    tmsg.u14c = 0;
    tmsg.u150 = 0;
    tmsg.u154 = 0;
    tmsg.u144 = 0;
    d->title = (unsigned short *)b;
    tmsg.text = text;
    tmsg_layout();
    tmsg_paginate();
    if (m->done == 0)
        tmsg_mark_done();
}
#endif

void tmsg_mark_done(void)
{
    tmsg.done = 1;
}

#ifdef NON_MATCHING /* ordem dos lui/stores dos campos difere */
void tmsg_create(void)
{
    MenuItemDef *const d = &tmsg.def;

    menu_init(&tmsg.menu, tmsg.heap);
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->auto_title = 1;
    d->font_flag4 = 1;
    d->style2 = 2;
    d->pad_t = 2;
    d->style = tmsg.style;
    d->show_title = 0;
    d->y = 210;
    d->x = tmsg.x;
    d->text = 0;
    d->title = 0;
    d->h = 60;
    d->color = 0xFFFFFFFF;
    d->title_color = 0xFFFFFFFF;
    d->w = tmsg.w0;
    d->text_color = 0xFF000000;
    d->pad_l = 4;
    d->font_size = tmsg.size;
    d->title_font_size = tmsg.size;
    d->pad_r = 20;
    menu_add_item(&tmsg.menu, d);
    tmsg_reset(tmsg.x, tmsg.w0, tmsg.size, tmsg.u0ec, tmsg.style);
}
#endif

void tmsg_layout(void)
{
    TMsgWin *const m = &tmsg;
    MenuItemDef *const d = &tmsg.def;
    unsigned short flags = tmsg_font_begin();
    int gap;
    int lh;
    int i;
    int x, y, w, h;


    switch (region_get()) {
    case 0:
    case 1:
        gap = 1;
        break;
    default:
        gap = 3;
        break;
    }
    m->u13c = 0;
    m->pos = 0;
    menu_item_rect(&m->menu, d, &x, &y, &w, &h);
    m->w = w;
    lh = font_line_height() >> 6;
    for (i = 0; i < m->nlines; i++) {
        m->line[i].x = x;
        m->line[i].y = y + i * lh;
        m->line[i].u08 = 0;
        m->line[i].h = lh;
        m->line[i].start = 0;
        if (region_get())
            m->line[i].y += i * gap;
    }
    tmsg_font_end(flags);
}

void tmsg_place(void)
{
    TMsgWin *const m = &tmsg;
    MenuItemDef *const d = &tmsg.def;
    unsigned short flags = tmsg_font_begin();
    int x, y, w, h;
    int lh;
    int i;
    int gap;

    switch (region_get()) {
    case 0:
    case 1:
        gap = 1;
        break;
    default:
        gap = 3;
        break;
    }
    m->def.x = m->fx;
    m->def.y = m->fy;
    menu_item_rect(&m->menu, d, &x, &y, &w, &h);
    lh = (font_line_height() >> 6) + 1;
    for (i = 0; i < m->nlines; i++) {
        m->line[i].x = x;
        m->line[i].y = y + i * lh;
        if (region_get())
            m->line[i].y += i * gap;
    }
    tmsg_font_end(flags);
}

/* sets the font state of the window; returns the previous flags */
unsigned short tmsg_font_begin(void)
{
    TMsgWin *const m = &tmsg;
    MenuItemDef *const d = &tmsg.def;
    unsigned short flags = font_get_flags();
    int gap;

    switch (region_get()) {
    case 0:
    case 1:
        gap = 1;
        break;
    default:
        gap = 3;
        break;
    }
    if (region_get() == 0) {
        font_set_flag1(1);
        font_set_flag8000(m->wide);
    } else {
        font_set_flag8(1);
        font_set_flag8000(0);
    }
    font_set_flag2(1);
    font_set_size(d->font_size, d->font_size);
    font_set_flag4(d->font_flag4);
    font_set_color(d->text_color);
    font_set_line_extra(gap << 6);
    return flags;
}

void tmsg_font_end(unsigned short flags)
{
    font_set_flags(flags);
    font_set_line_extra(0);
}

/* how many characters of the text fit on each line from the current position */
void tmsg_paginate(void)
{
    TMsgWin *const m = &tmsg;
    TMsgLine *l;
    unsigned short flags;
    int pos;
    int i;
    unsigned int end;

    if (tmsg.text == 0)
        return;
    flags = tmsg_font_begin();
    font_set_color(0);
    for (i = 0, pos = m->pos; i < m->nlines; i++) {
        m->line[i].start = pos;
        l = &m->line[i];
        l->count = font_draw_wtext_ex(m->line[i].x << 6, m->line[i].y << 6, m->w << 6,
                                      (m->line[i].h + 2) << 6, m->text + pos, -1);
        end = pos + l->count;
        if (end < wcslen(m->text) && m->text[end] == ' ')
            l->count++;
        pos += l->count;
    }
    tmsg_font_end(flags);
}
