// @range 0x08808B44-0x08809D2C
// @asm 0x088090B8-0x088093C0
// @asm 0x088093D4-0x0880945C
// @rodata 0x0881C15C-0x0881C178
/* Message window of the shop: a menu item that slides in and out, the text typed a few pixels per
   frame on up to three lines (speed from the options, held button to hurry), pages and the blinking
   "next" arrow, and the font state it draws with. */
#include "libshop.h"

void se_play(int id);
int sysflag_test(unsigned int bit);
unsigned short region_pick_value(int flag);
unsigned int cfg_get_byte0_bits4_5(void);
GfxState *gfx_get_state(void);
void vec2_lerp(float t, float *out, float *a, float *b);
int wcslen(const unsigned short *s);
void font_set_flag1(int on);
void font_set_flag2(int on);
void font_set_flag4(int on);
void font_set_flag8(int on);
void font_set_flag8000(int on);
int font_get_flag8000(void);
void font_set_line_extra(int v);
unsigned short font_get_flags(void);
void font_set_flags(unsigned short flags);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
void *gim_set_texture(void *list, void *gim);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);

void shop_msgwin_reset(int x, int w, int size, int input, int style);
void shop_msgwin_set_size(int size);
void shop_msgwin_open(void);
int shop_msg_typing(void);
int shop_msgwin_moving(void);
void shop_msgwin_setup(void);
void shop_msg_blink(void);
void shop_msg_type(void);
void shop_msg_layout(void);
void shop_msgwin_place(void);
void shop_draw_icon(unsigned int **list, int x, int y, int z, int u, int v, int w, int h, int frame);
unsigned short shop_msg_font_push(void);
void shop_msg_font_pop(unsigned short flags);
void shop_msg_measure(void);

/* extra pixels between the lines: 1 in Japan and America, 3 in Europe */
static inline int shop_line_gap(void)
{
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
    return gap;
}

/* pixels typed per frame for each message speed */
static const int shop_msg_speed[3] = { 10, 20, 480 };

void shop_msgwin_init(void *heap, int x, int w, int size, int input, int style)
{
    memset(&shop_msg, 0, sizeof(shop_msg));
    shop_msg.heap = heap;
    shop_msg.x = x;
    shop_msg.w = w;
    shop_msg.font_size = size;
    shop_msg.input = input;
    shop_msg.style = style;
    shop_msgwin_setup();
}

void shop_msgwin_reset(int x, int w, int size, int input, int style)
{
    ShopMsgWin *m = &shop_msg;

    shop_msg.n_lines = 0;
    shop_msg.text = 0;
    shop_msg.speed = cfg_get_byte0_bits4_5();
    shop_msg.input = input;
    shop_msg.style = style;
    memset(shop_msg.lines, 0, sizeof(shop_msg.lines));
    shop_msg.line = 0;
    shop_msg.start = 0;
    shop_msg.done = 0;
    shop_msg.closed = 0;
    shop_msg.full = 0;
    shop_msg.shown = 0;
    shop_msg.narrow = 0;
    shop_msg.from[0] = x;
    shop_msg.from[1] = 300.0f;
    shop_msg.to[0] = x;
    shop_msg.to[1] = 210.0f;
    shop_msg.pos[0] = x;
    shop_msg.pos[1] = 300.0f;
    shop_msg.def.x = x;
    if (region_get() == 0) {
        m->n_lines = 2;
        m->narrow = 1;
    } else {
        m->n_lines = 3;
        m->narrow = 0;
    }
    shop_msgwin_set_size(size);
    shop_msg_set(0, 0);
    shop_msgwin_place();
    m->state = 0;
}

void shop_msgwin_free(void)
{
    shop_msg.icon = 0;
    menu_free_items(&shop_msg.menu);
    memset(&shop_msg, 0, sizeof(shop_msg));
}

/* per frame: slide in/out, type the text, advance on the button; 1 once the text was dismissed */
int shop_msgwin_update(void)
{
    ShopMsgWin *m = &shop_msg;
    unsigned int trig = pad_get()->b.trig;
    int i;
    int fast;

    shop_msg.speed = cfg_get_byte0_bits4_5();
    switch (shop_msg.state) {
    case 0:
        break;
    case 1:
        vec2_lerp(m->t, m->pos, m->from, m->to);
        if (m->t >= 1.0f) {
            m->state = 2;
        } else {
            m->t += 0.1f + 0.1f * m->t;
            if (m->t > 1.0f)
                m->t = 1.0f;
        }
        break;
    case 2:
        if (m->speed == 2) {
            for (i = 0; i < m->n_lines; i++)
                shop_msg_type();
        } else {
            shop_msg_type();
        }
        shop_msg_blink();
        if (m->input) {
            if (sysflag_test(7) && (pad_get()->b.down & 0x200) &&
                (pad_get()->b.down & region_pick_value(0)) && gfx_get_state()->frame % 3 == 0)
                fast = 1;
            else
                fast = 0;
            if (m->hold == 0 && ((trig & region_pick_value(0)) || fast)) {
                if (m->done) {
                    if (m->closed == 0) {
                        m->closed = 1;
                        se_play(0x6219);
                    }
                } else if (m->full) {
                    shop_msg_layout();
                    m->start = m->shown;
                    shop_msg_measure();
                    m->full = 0;
                    m->page++;
                    se_play(0x6219);
                } else {
                    for (i = 0; i < m->n_lines; i++) {
                        m->lines[m->line].w += shop_msg_speed[2];
                        shop_msg_type();
                        if (m->done || m->full)
                            break;
                    }
                }
            }
        }
        if (m->text == 0)
            m->closed = 1;
        break;
    case 3:
        vec2_lerp(m->t, m->pos, m->from, m->to);
        if (m->t <= 0.0f) {
            m->state = 0;
        } else {
            m->t -= 0.1f + 0.1f * m->t;
            if (m->t < 0.0f)
                m->t = 0.0f;
        }
        break;
    }
    if (m->state)
        shop_msgwin_place();
    return m->closed;
}

/* icon frames of the "next" arrow and its offset from the window corner */
const short shop_msg_icon_uv[4] = { 0x38, 0x4A, 0x0E, 0x0E };
const int shop_msg_icon_off[2] = { -20, -20 };

#ifdef NON_MATCHING /* muitos registradores salvos ($fp) e a conta da rolagem em float; alocação diferente */
void shop_msgwin_draw(unsigned int **list)
{
    ShopMsgWin *m = &shop_msg;
    MenuItemDef *d = &shop_msg.def;
    ShopMsgLine *l;
    unsigned short flags;
    int i;
    int y, h, gap;
    int x0, y0, w0, h0;

    if (shop_msg.state == 0)
        return;
    font_set_flag8000(0);
    menu_layout((void **)list, &m->menu);
    if (m->text) {
        flags = shop_msg_font_push();
        for (i = 0, l = m->lines; i <= m->line; i++, l++) {
            switch (region_get()) {
            case 1:
            case 0:
                y = l->y;
                gap = 1;
                h = l->h + 1;
                break;
            default:
                gap = 3;
                y = l->y - 2;
                h = l->h + 4;
                break;
            }
            if (l->w <= 0)
                continue;
            if (y < 272) {
                if (y + h >= 272)
                    h = 272 - y;
                *list = dl_scissor(*list, l->x, y, l->w + 1, h);
                *list = dl_enable(*list, 2);
            }
            if (y < 272) {
                int scroll = -(int)(m->page * (m->n_lines * (font_line_height() / 64.0f + gap)));
                menu_item_rect(&m->menu, d, &x0, &y0, &w0, &h0);
                font_text_set(*list);
                font_draw_wtext_ex(x0 << 6, (y0 + scroll) << 6, w0 << 6, 0x4400, m->text, -1);
                *list = font_text_clear();
            }
            *list = dl_disable(*list, 2);
        }
        shop_msg_font_pop(flags);
    }
    if (m->input && shop_msg_typing() == 0 && shop_msgwin_moving() == 0 && m->icon) {
        int x = d->x + d->w;
        int yy = d->y + d->h;
        *list = dl_ambient_color(*list, 0xFFFFFFFF);
        *list = gim_set_texture(*list, m->icon);
        shop_draw_icon(list, x + shop_msg_icon_off[0], yy + shop_msg_icon_off[1], 0, shop_msg_icon_uv[0],
                       shop_msg_icon_uv[1], shop_msg_icon_uv[2], shop_msg_icon_uv[3], m->blink / 10 & 1);
    }
}
#endif

void shop_msgwin_set_size(int size)
{
    MenuItemDef *d = &shop_msg.def;

    d->font_size = size;
    d->title_font_size = size;
}

#ifdef NON_MATCHING /* o original guarda &shop_msg em s0 antes das chamadas e relê o estado por ele; o mwcc rematerializa o endereço */
/* shows `text` (UCS-2) from its start; `title` nonzero shows the title bar */
void shop_msg_set(unsigned short *text, int title)
{
    MenuItemDef *d = &shop_msg.def;
    ShopMsgWin *const m = &shop_msg;

    d->show_title = title != 0;
    shop_msg.done = 0;
    shop_msg.closed = 0;
    shop_msg.full = 0;
    shop_msg.shown = 0;
    shop_msg.page = 0;
    d->title = (unsigned short *)title;
    shop_msg.text = text;
    shop_msg_layout();
    shop_msg_measure();
    if (m->state == 0)
        shop_msgwin_open();
}
#endif

void shop_msgwin_open(void)
{
    shop_msg.state = 1;
}

void shop_msgwin_close(void)
{
    shop_msg.state = 3;
}

/* 1 while the text is still being typed */
int shop_msg_typing(void)
{
    ShopMsgWin *m = &shop_msg;

    if (shop_msg.done)
        return 0;
    if (m->full)
        return 0;
    if (m->text == 0)
        return 0;
    return m->state == 2;
}

/* 1 while the window slides */
int shop_msgwin_moving(void)
{
    switch (shop_msg.state) {
    case 1:
    case 3:
        return 1;
    }
    return 0;
}

void shop_msgwin_setup(void)
{
    MenuItemDef *d = &shop_msg.def;

    menu_init(&shop_msg.menu, shop_msg.heap);
    memset(d, 0, sizeof(MenuItemDef));
    d->visible = 1;
    d->auto_title = 1;
    d->font_flag4 = 1;
    d->style = shop_msg.style;
    d->style2 = 2;
    d->pad_t = 2;
    d->show_title = 0;
    d->x = shop_msg.x;
    d->y = 0xD2;
    d->text = 0;
    d->title = 0;
    d->w = shop_msg.w;
    d->h = 0x3C;
    d->color = -1;
    d->title_color = -1;
    d->font_size = shop_msg.font_size;
    d->text_color = 0xFF000000;
    d->pad_l = 4;
    d->title_font_size = shop_msg.font_size;
    d->pad_r = 0x14;
    menu_add_item(&shop_msg.menu, d);
    shop_msgwin_reset(shop_msg.x, shop_msg.w, shop_msg.font_size, shop_msg.input, shop_msg.style);
}

void shop_msg_blink(void)
{
    ShopMsgWin *m = &shop_msg;

    if (shop_msg_typing() == 0)
        m->blink++;
}

/* types the current line a few pixels further; next line, page or end of the text */
void shop_msg_type(void)
{
    ShopMsgWin *m = &shop_msg;
    ShopMsgLine *l;
    unsigned short flags;
    int w;

    if (shop_msg.text == 0) {
        m->done = 1;
        return;
    }
    if (m->done)
        return;
    if (m->full)
        return;
    l = &m->lines[m->line];
    font_get_flag8000();
    flags = shop_msg_font_push();
    w = (font_measure_text(m->text + l->start) >> 6) + 1;
    w = w < m->line_w ? w : m->line_w;
    shop_msg_font_pop(flags);
    l->w += shop_msg_speed[m->speed];
    if (l->w < w)
        return;
    l->w = w;
    if (m->state != 2)
        return;
    m->shown += m->lines[m->line].len;
    if (m->shown - 1 == wcslen(m->text)) {
        m->blink = 0;
        m->done = 1;
    } else if (m->line == m->n_lines - 1) {
        m->blink = 0;
        m->full = 1;
    } else {
        m->line++;
    }
}

/* positions of the lines inside the window */
void shop_msg_layout(void)
{
    ShopMsgWin *m = &shop_msg;
    MenuItemDef *d = &shop_msg.def;
    ShopMsgLine *l;
    unsigned short flags = shop_msg_font_push();
    int gap;
    int lh;
    int i, y, extra;
    int x0, y0, w0, h0;

    gap = shop_line_gap();
    m->line = 0;
    m->start = 0;
    menu_item_rect(&m->menu, d, &x0, &y0, &w0, &h0);
    m->line_w = w0;
    lh = font_line_height() >> 6;
    for (i = 0; i < m->n_lines; i++) {
        m->lines[i].x = x0;
        m->lines[i].y = y0 + i * lh;
        m->lines[i].w = 0;
        m->lines[i].h = lh;
        m->lines[i].start = 0;
        if (region_get())
            m->lines[i].y += i * gap;
    }
    shop_msg_font_pop(flags);
}

/* moves the window to its animated position */
void shop_msgwin_place(void)
{
    ShopMsgWin *m = &shop_msg;
    MenuItemDef *d = &shop_msg.def;
    ShopMsgLine *l;
    unsigned short flags = shop_msg_font_push();
    int gap;
    int lh;
    int i, y, extra;
    int x0, y0, w0, h0;

    gap = shop_line_gap();
    m->def.x = m->pos[0];
    m->def.y = m->pos[1];
    menu_item_rect(&m->menu, d, &x0, &y0, &w0, &h0);
    lh = (font_line_height() >> 6) + 1;
    for (i = 0; i < m->n_lines; i++) {
        m->lines[i].x = x0;
        m->lines[i].y = y0 + i * lh;
        if (region_get())
            m->lines[i].y += i * gap;
    }
    shop_msg_font_pop(flags);
}

void shop_draw_icon(unsigned int **list, int x, int y, int z, int u, int v, int w, int h, int frame)
{
    *list = draw_set_offsets(*list, w, h, w, h);
    *list = draw_sprite(*list, x, y, z, u + w * frame, v, 0, 0);
}

/* sets the font of the window; returns the flags to restore */
unsigned short shop_msg_font_push(void)
{
    ShopMsgWin *m = &shop_msg;
    MenuItemDef *d = &shop_msg.def;
    unsigned short flags = font_get_flags();
    int gap;

    gap = shop_line_gap();
    if (region_get() == 0) {
        font_set_flag1(1);
        font_set_flag8000(m->narrow);
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

void shop_msg_font_pop(unsigned short flags)
{
    font_set_flags(flags);
    font_set_line_extra(0);
}

/* characters of each line of the page */
void shop_msg_measure(void)
{
    ShopMsgWin *m = &shop_msg;
    ShopMsgLine *l;
    unsigned short flags;
    int c;
    int i;

    if (shop_msg.text == 0)
        return;
    flags = shop_msg_font_push();
    font_set_color(0);
    c = m->start;
    for (i = 0; i < m->n_lines; i++) {
        m->lines[i].start = c;
        l = &m->lines[i];
        l->len = font_draw_wtext_ex(m->lines[i].x << 6, m->lines[i].y << 6, m->line_w << 6,
                                    (m->lines[i].h + 2) << 6, m->text + c, -1);
        if ((unsigned int)(c + l->len) < wcslen(m->text) && m->text[c + l->len] == ' ')
            l->len++;
        c += l->len;
    }
    shop_msg_font_pop(flags);
}
