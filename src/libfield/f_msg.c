// @range 0x08815FD8-0x08817218
// @asm 0x088168B0-0x08816938
// @asm 0x088169EC-0x08816B00
// @rodata 0x08838AE0-0x08838AFC
/* The message window of the field (field_msg; libtitle's tmsg is a trimmed copy): a system menu frame
   sliding in and out, up to three lines typed at the text speed of the options, the pages, the
   "next" cursor and the input that skips or advances the text. */
#include "libfield.h"

int sysflag_test(unsigned int bit);
unsigned short region_pick_value(int flag);
int cfg_get_byte0_bits4_5(void);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_ambient_color(void *list, unsigned int color);
void *gim_set_texture(void *list, void *gim);

void fmsg_create(void);
void fmsg_blink_tick(void);
void fmsg_type(void);
void fmsg_layout(void);
void fmsg_place(void);
void fmsg_sprite(void **list, int x, int y, int z, int u, int v, int w, int h, int frame);
unsigned short fmsg_font_begin(void);
void fmsg_font_end(unsigned short flags);
void fmsg_paginate(void);

const int fmsg_speeds[3] = { 10, 20, 480 };  /* characters per frame by text speed */
const struct {
    short u, v, w, h;
    int dx, dy;
} fmsg_cursor_spr = { 0x38, 0x4A, 14, 14, -20, -20 };

void fmsg_init(void *heap, int x, int w, int size, int input, int style)
{
    memset(&field_msg, 0, sizeof(field_msg));
    field_msg.heap = heap;
    field_msg.x = x;
    field_msg.w0 = w;
    field_msg.size = size;
    field_msg.input = input;
    field_msg.style = style;
    fmsg_create();
}

void fmsg_reset(int x, int w, int size, int input, int style)
{
    FieldMsg *const m = &field_msg;
    float f;

    field_msg.nlines = 0;
    field_msg.text = 0;
    field_msg.speed = cfg_get_byte0_bits4_5();
    field_msg.input = input;
    field_msg.style = style;
    memset(field_msg.line, 0, sizeof(field_msg.line));
    field_msg.cur = 0;
    field_msg.pos = 0;
    field_msg.all = 0;
    field_msg.done = 0;
    field_msg.more = 0;
    field_msg.typed = 0;
    field_msg.wide = 0;
    f = x;
    field_msg.from.y = 300.0f;
    field_msg.from.x = f;
    field_msg.to.x = f;
    field_msg.to.y = 210.0f;
    field_msg.pos2.x = f;
    field_msg.pos2.y = 300.0f;
    field_msg.def.x = x;
    if (region_get() == 0) {
        m->nlines = 2;
        m->wide = 1;
    } else {
        m->wide = 0;
        m->nlines = 3;
    }
    fmsg_set_size(size);
    fmsg_set_text(0, 0);
    fmsg_place();
    m->state = 0;
}

void fmsg_free(void)
{
    field_msg.cursor = 0;
    menu_free_items(&field_msg.menu);
    memset(&field_msg, 0, sizeof(field_msg));
}

int fmsg_update(void)
{
    FieldMsg *const m = &field_msg;
    unsigned int trig = pad_get()->b.trig;
    int skip;
    int i;

    switch (field_msg.speed = cfg_get_byte0_bits4_5(), field_msg.state) {
    case 0:
        break;
    case 1:
        vec2_lerp(&m->pos2, &m->from, &m->to, m->t);
        if (m->t >= 1.0f) {
            m->state = 2;
        } else {
            m->t = m->t + (0.1f + 0.1f * m->t);
            if (m->t > 1.0f)
                m->t = 1.0f;
        }
        break;
    case 2:
        if (m->speed == 2) {
            for (i = 0; i < m->nlines; i++)
                fmsg_type();
        } else {
            fmsg_type();
        }
        fmsg_blink_tick();
        if (m->input != 0) {
            if (sysflag_test(7) && (pad_get()->b.down & 0x200) && (pad_get()->b.down & region_pick_value(0)) &&
                *gfx_get_state() % 3 == 0)
                skip = 1;
            else
                skip = 0;
            if (m->lock == 0 && ((trig & region_pick_value(0)) || skip)) {
                if (m->all) {
                    if (m->done == 0) {
                        m->done = 1;
                        se_play(0x6219);
                    }
                } else if (m->more) {
                    fmsg_layout();
                    m->pos = m->typed;
                    fmsg_paginate();
                    m->more = 0;
                    m->page++;
                    se_play(0x6219);
                } else {
                    for (i = 0; i < m->nlines; i++) {
                        m->line[m->cur].shown += fmsg_speeds[2];
                        fmsg_type();
                        if (m->all || m->more)
                            break;
                    }
                }
            }
        }
        if (m->text == 0)
            m->done = 1;
        break;
    case 3:
        vec2_lerp(&m->pos2, &m->from, &m->to, m->t);
        if (m->t <= 0.0f) {
            m->state = 0;
        } else {
            m->t = m->t - (0.1f + 0.1f * m->t);
            if (m->t < 0.0f)
                m->t = 0.0f;
        }
        break;
    }
    if (m->state != 0)
        fmsg_place();
    return m->done;
}

void fmsg_draw(void **list)
{
    FieldMsg *const m = &field_msg;
    MenuItemDef *const d = &field_msg.def;
    unsigned short flags;
    FieldMsgLine *l;
    int i;
    int gap;
    int y;
    int h;
    int off;
    float fh;
    int x, yy, w, hh;

    if (field_msg.state == 0)
        return;
    font_set_flag8000(0);
    menu_layout(list, &m->menu);
    if (m->text != 0) {
        flags = fmsg_font_begin();
        for (i = 0; i <= m->cur; i++) {
            l = &m->line[i];
            switch (region_get()) {
            default:
                gap = 3;
                y = l->y - 2;
                h = l->h + 4;
                break;
            case 0:
            case 1:
                y = l->y;
                gap = 1;
                h = l->h + 1;
                break;
            }
            if (l->shown > 0) {
                if (y < 0x110) {
                    h = y + h < 0x110 ? h : 0x110 - y;
                    *list = dl_scissor(*list, l->x, y, l->shown + 1, h);
                    *list = dl_enable(*list, 2);
                }
                if (y < 0x110) {
                    fh = font_line_height() / 64.0f;
                    off = -(int)(m->page * (m->nlines * (fh + gap)));
                    menu_item_rect(&m->menu, &m->def, &x, &yy, &w, &hh);
                    font_text_set(*list);
                    font_draw_wtext_ex(x << 6, (yy + off) << 6, w << 6, 0x4400, m->text, -1);
                    *list = font_text_clear();
                }
                *list = dl_disable(*list, 2);
            }
        }
        fmsg_font_end(flags);
    }
    if (m->input != 0 && fmsg_waiting() == 0 && fmsg_moving() == 0 && m->cursor != 0) {
        y = d->x + d->w;
        h = d->y + d->h;
        *list = dl_ambient_color(*list, 0xFFFFFFFF);
        *list = gim_set_texture(*list, m->cursor);
        fmsg_sprite(list, y + fmsg_cursor_spr.dx, h + fmsg_cursor_spr.dy, 0, fmsg_cursor_spr.u, fmsg_cursor_spr.v,
                    fmsg_cursor_spr.w, fmsg_cursor_spr.h, (m->blink / 10) & 1);
    }
}

int fmsg_done(void)
{
    return field_msg.done;
}

void fmsg_set_cursor(void *gim)
{
    field_msg.cursor = gim;
}

MenuItemDef *fmsg_def(void)
{
    return &field_msg.def;
}

Vec2 *fmsg_pos(void)
{
    return &field_msg.pos2;
}

void fmsg_lock(int on)
{
    field_msg.lock = on;
}

void fmsg_set_size(int size)
{
    MenuItemDef *const d = &field_msg.def;

    d->font_size = size;
    d->title_font_size = size;
}

void fmsg_set_input(int on)
{
    field_msg.input = on;
}

#ifdef NON_MATCHING /* the original keeps &field_msg in $s0 across the calls; mwcc rebuilds the address */
void fmsg_set_text(unsigned short *text, int title)
{
    MenuItemDef *const d = &field_msg.def;
    FieldMsg *const m = &field_msg;

    d->show_title = title != 0;
    field_msg.all = 0;
    field_msg.done = 0;
    field_msg.more = 0;
    field_msg.typed = 0;
    field_msg.page = 0;
    d->title = (unsigned short *)title;
    field_msg.text = text;
    fmsg_layout();
    fmsg_paginate();
    if (m->state == 0)
        fmsg_open();
}
#endif

void fmsg_open(void)
{
    field_msg.state = 1;
}

void fmsg_close(void)
{
    field_msg.state = 3;
}

int fmsg_active(void)
{
    return field_msg.state != 0;
}

int fmsg_waiting(void)
{
    FieldMsg *m = &field_msg;

    if (m->all)
        return 0;
    if (m->more)
        return 0;
    if (m->text == 0)
        return 0;
    return m->state == 2;
}

int fmsg_moving(void)
{
    switch (field_msg.state) {
    case 1:
    case 3:
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* lógica conferida. 34 palavras: o mwcc adianta as leituras de x/w0/size/style para antes dos stores constantes; o original lê cada uma junto do uso (cast por char * e base FieldMsg * não mudam) */
void fmsg_create(void)
{
    MenuItemDef *d = &field_msg.def;

    menu_init(&field_msg.menu, field_msg.heap);
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->auto_title = 1;
    d->font_flag4 = 1;
    d->style2 = 2;
    d->pad_t = 2;
    d->style = field_msg.style;
    d->show_title = 0;
    d->y = 210;
    d->x = field_msg.x;
    d->text = 0;
    d->title = 0;
    d->h = 60;
    d->color = 0xFFFFFFFF;
    d->title_color = 0xFFFFFFFF;
    d->w = field_msg.w0;
    d->text_color = 0xFF000000;
    d->pad_l = 4;
    d->font_size = field_msg.size;
    d->title_font_size = field_msg.size;
    d->pad_r = 20;
    menu_add_item(&field_msg.menu, d);
    fmsg_reset(field_msg.x, field_msg.w0, field_msg.size, field_msg.input, field_msg.style);
}
#endif

void fmsg_blink_tick(void)
{
    FieldMsg *const m = &field_msg;

    if (fmsg_waiting() == 0)
        m->blink++;
}

void fmsg_type(void)
{
    FieldMsg *const m = &field_msg;
    FieldMsgLine *l;
    unsigned short flags;
    int w;

    if (field_msg.text == 0) {
        m->all = 1;
    } else if (m->all == 0 && m->more == 0) {
        l = &m->line[m->cur];
        font_get_flag8000();
        flags = fmsg_font_begin();
        w = (font_measure_text(m->text + l->start) >> 6) + 1;
        w = w < m->w ? w : m->w;
        fmsg_font_end(flags);
        l->shown += fmsg_speeds[m->speed];
        if (l->shown >= w) {
            l->shown = w;
            if (m->state == 2) {
                m->typed += m->line[m->cur].count;
                if (m->typed - 1 == wcslen(m->text)) {
                    m->blink = 0;
                    m->all = 1;
                } else if (m->cur == m->nlines - 1) {
                    m->blink = 0;
                    m->more = 1;
                } else {
                    m->cur++;
                }
            }
        }
    }
}

void fmsg_layout(void)
{
    FieldMsg *const m = &field_msg;
    MenuItemDef *const d = &field_msg.def;
    unsigned short flags = fmsg_font_begin();
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
    m->cur = 0;
    m->pos = 0;
    menu_item_rect(&m->menu, d, &x, &y, &w, &h);
    m->w = w;
    lh = font_line_height() >> 6;
    for (i = 0; i < m->nlines; i++) {
        m->line[i].x = x;
        m->line[i].y = y + i * lh;
        m->line[i].shown = 0;
        m->line[i].h = lh;
        m->line[i].start = 0;
        if (region_get())
            m->line[i].y += i * gap;
    }
    fmsg_font_end(flags);
}

void fmsg_place(void)
{
    FieldMsg *const m = &field_msg;
    MenuItemDef *const d = &field_msg.def;
    unsigned short flags = fmsg_font_begin();
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
    m->def.x = m->pos2.x;
    m->def.y = m->pos2.y;
    menu_item_rect(&m->menu, d, &x, &y, &w, &h);
    lh = (font_line_height() >> 6) + 1;
    for (i = 0; i < m->nlines; i++) {
        m->line[i].x = x;
        m->line[i].y = y + i * lh;
        if (region_get())
            m->line[i].y += i * gap;
    }
    fmsg_font_end(flags);
}

void fmsg_sprite(void **list, int x, int y, int z, int u, int v, int w, int h, int frame)
{
    *list = draw_set_offsets(*list, w, h, w, h);
    *list = draw_sprite(*list, x, y, z, u + w * frame, v, 0, 0);
}

/* sets the font state of the window; returns the previous flags */
unsigned short fmsg_font_begin(void)
{
    FieldMsg *const m = &field_msg;
    MenuItemDef *const d = &field_msg.def;
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

void fmsg_font_end(unsigned short flags)
{
    font_set_flags(flags);
    font_set_line_extra(0);
}

/* how many characters of the text fit on each line from the current position */
void fmsg_paginate(void)
{
    FieldMsg *const m = &field_msg;
    FieldMsgLine *l;
    unsigned short flags;
    int pos;
    int i;
    unsigned int end;

    if (field_msg.text == 0)
        return;
    flags = fmsg_font_begin();
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
    fmsg_font_end(flags);
}
