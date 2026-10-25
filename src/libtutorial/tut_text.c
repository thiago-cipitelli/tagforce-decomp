// @range 0x08809A5C-0x0880AC6C
// @data 0x0880F938-0x0880F954
// @asm 0x08809B00-0x08809C5C
// @asm 0x08809F94-0x0880A29C
// @asm 0x0880A2D4-0x0880A35C
// @asm 0x0880A3CC-0x0880A400
// @asm 0x0880A440-0x0880A554
// @asm 0x0880A590-0x0880A708
// @asm 0x0880A708-0x0880A840
// @asm 0x0880A840-0x0880A984
// @asm 0x0880AA3C-0x0880AB18
/* Text box of the talk windows: a menu window that slides in and out, the message typed line by
   line at the speed of the game options (or skipped with a button), pages of two or three lines
   and the blinking "next" icon while the box waits for the player. */
#include "libtutorial.h"

typedef struct TutLine {         /* one line of the page, 0x18 bytes */
    int x, y;
    int w;                       /* +0x08 width typed so far */
    int h;
    int start;                   /* +0x10 first character */
    int count;                   /* +0x14 characters on the line */
} TutLine;

typedef struct TutText {         /* tut_tb 0x08817930, 0x180 bytes */
    int state;                   /* +0x000 0 hidden, 1 opening, 2 shown, 3 closing */
    float t;                     /* +0x004 slide 0..1 */
    Menu menu;                   /* +0x008 */
    MenuItemDef def;             /* +0x054 */
    void *heap;                  /* +0x0CC */
    int x;                       /* +0x0D0 */
    int w;                       /* +0x0D4 */
    int size;                    /* +0x0D8 font size */
    int lines;                   /* +0x0DC lines per page (2 in Japanese, 3 otherwise) */
    int text_w;                  /* +0x0E0 */
    unsigned short *text;        /* +0x0E4 */
    int speed;                   /* +0x0E8 game option 0..2 */
    int wait;                    /* +0x0EC wait for the button at the end */
    int style;                   /* +0x0F0 */
    TutLine line[3];             /* +0x0F4 */
    int cur;                     /* +0x13C line being typed */
    int pos;                     /* +0x140 first character of the page */
    int page;                    /* +0x144 */
    int finished;                /* +0x148 whole text typed */
    int done;                    /* +0x14C button pressed at the end */
    int page_full;               /* +0x150 */
    int next_pos;                /* +0x154 */
    int flag8000;                /* +0x158 */
    void *icon;                  /* +0x15C tuto_duel01.gim */
    unsigned int blink;          /* +0x160 */
    int no_skip;                 /* +0x164 */
    float pos2[2];               /* +0x168 current position */
    float from[2];               /* +0x170 */
    float to[2];                 /* +0x178 */
} TutText;

extern TutText tut_tb;

void *vec2_lerp(float *out, const float *a, const float *b, float t);
unsigned int cfg_get_byte0_bits4_5(void);
int region_get(void);
int sysflag_test(unsigned int bit);
unsigned int *gfx_get_state(void);
void font_set_line_extra(int v);
unsigned int wcslen(const unsigned short *s);
void *gim_set_texture(void *list, void *g);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_ambient_color(void *list, unsigned int color);

void tut_text_reset(int x, int w, int size, int wait, int style);
void tut_text_build(void);
void tut_text_blink(void);
void tut_text_advance(void);
void tut_text_layout(void);
void tut_text_relayout(void);
void tut_text_icon(void **list, int x, int y, int z, int u, int v, int w, int h, int frame);
unsigned short tut_text_font_begin(void);
void tut_text_font_end(unsigned short flags);
void tut_text_paginate(void);
void tut_text_set_size(int size);
void tut_text_set(unsigned short *text, unsigned short *name);
void tut_text_open(void);
int tut_text_typing(void);
int tut_text_anim_busy(void);

int tut_text_speeds[3] = { 10, 20, 480 };    /* width typed per frame by speed option */
short tut_text_icon_rect[4] = { 0x38, 0x4A, 14, 14 };
int tut_text_icon_off[2] = { -20, -20 };

void tut_text_init(void *heap, int x, int w, int size, int wait, int style)
{
    memset(&tut_tb, 0, 0x180);
    tut_tb.heap = heap;
    tut_tb.x = x;
    tut_tb.w = w;
    tut_tb.size = size;
    tut_tb.wait = wait;
    tut_tb.style = style;
    tut_text_build();
}

#ifdef NON_MATCHING
/* acessos absolutos ao tut_tb com um lui por campo e a ordem das gravações das posições float não saem iguais */
void tut_text_reset(int x, int w, int size, int wait, int style)
{
    TutText *t = &tut_tb;

    tut_tb.lines = 0;
    tut_tb.text = 0;
    tut_tb.speed = cfg_get_byte0_bits4_5();
    tut_tb.wait = wait;
    tut_tb.style = style;
    memset(tut_tb.line, 0, 0x48);
    tut_tb.cur = 0;
    tut_tb.pos = 0;
    tut_tb.finished = 0;
    tut_tb.done = 0;
    tut_tb.page_full = 0;
    tut_tb.next_pos = 0;
    tut_tb.flag8000 = 0;
    tut_tb.pos2[0] = x;
    tut_tb.from[1] = 300.0f;
    tut_tb.to[1] = 210.0f;
    tut_tb.pos2[1] = 300.0f;
    tut_tb.def.x = x;
    tut_tb.from[0] = tut_tb.pos2[0];
    tut_tb.to[0] = tut_tb.pos2[0];
    if (region_get() == 0) {
        t->lines = 2;
        t->flag8000 = 1;
    } else {
        t->flag8000 = 0;
        t->lines = 3;
    }
    tut_text_set_size(size);
    tut_text_set(0, 0);
    tut_text_relayout();
    t->state = 0;
}
#endif

/* per frame: slide, typing (speed 2 types every line at once), skip with the button; returns
   nonzero once the player confirmed the end of the text */
int tut_text_update(void)
{
    TutText *t = &tut_tb;
    unsigned int trig = pad_get()->b.trig;
    int i;
    int skip;
    int step;

    tut_tb.speed = cfg_get_byte0_bits4_5();
    switch (tut_tb.state) {
    case 0:
        break;
    case 1:
        vec2_lerp(t->pos2, t->from, t->to, t->t);
        if (t->t >= 1.0f) {
            t->state = 2;
        } else {
            t->t += 0.1f + 0.1f * t->t;
            if (t->t > 1.0f)
                t->t = 1.0f;
        }
        break;
    case 2:
        if (tut_tb.speed == 2) {
            for (i = 0; i < t->lines; i++)
                tut_text_advance();
        } else {
            tut_text_advance();
        }
        tut_text_blink();
        if (t->wait != 0) {
            if (sysflag_test(7) && (pad_get()->b.down & 0x200) &&
                (pad_get()->b.down & region_pick_value(0)) && *gfx_get_state() % 3 == 0)
                skip = 1;
            else
                skip = 0;
            if (t->no_skip == 0 && ((trig & region_pick_value(0)) || skip)) {
                if (t->finished != 0) {
                    if (t->done == 0) {
                        t->done = 1;
                        se_play(0x6219);
                    }
                } else if (t->page_full != 0) {
                    tut_text_layout();
                    t->pos = t->next_pos;
                    tut_text_paginate();
                    t->page_full = 0;
                    t->page++;
                    se_play(0x6219);
                } else {
                    i = 0;
                    if (t->lines > 0) {
                        step = tut_text_speeds[2];
                        do {
                            t->line[t->cur].w += step;
                            tut_text_advance();
                            if (t->finished != 0 || t->page_full != 0)
                                break;
                            i++;
                        } while (i < t->lines);
                    }
                }
            }
        }
        if (t->text == 0)
            t->done = 1;
        break;
    case 3:
        vec2_lerp(t->pos2, t->from, t->to, t->t);
        if (t->t <= 0.0f) {
            t->state = 0;
        } else {
            t->t -= 0.1f + 0.1f * t->t;
            if (t->t < 0.0f)
                t->t = 0.0f;
        }
        break;
    }
    if (t->state != 0)
        tut_text_relayout();
    return t->done;
}

#ifdef NON_MATCHING
/* teste da região (região 0 e 1 com corpos separados) e alocação dos $s no laço das linhas */
void tut_text_draw(void **list)
{
    int r;
    TutText *t = &tut_tb;
    MenuItemDef *d = &tut_tb.def;
    unsigned short flags;
    int i;
    TutLine *l;
    int g;
    int y;
    int h;
    int x0, y0, w0, h0;
    int off;

    if (tut_tb.state == 0)
        return;
    font_set_flag8000(0);
    menu_layout(list, &t->menu);
    if (t->text != 0) {
        flags = tut_text_font_begin();
        for (i = 0; i <= t->cur; i++) {
            l = &t->line[i];
            r = region_get();
            if (r == 1) {
                y = l->y;
                g = 1;
                h = l->h + 1;
            } else if (r == 0) {
                y = l->y;
                g = 1;
                h = l->h + 1;
            } else {
                g = 3;
                y = l->y - 2;
                h = l->h + 4;
            }
            if (l->w > 0) {
                if (y < 0x110) {
                    if (y + h >= 0x110)
                        h = 0x110 - y;
                    *list = dl_scissor(*list, l->x, y, l->w + 1, h);
                    *list = dl_enable(*list, 2);
                }
                if (y < 0x110) {
                    off = (int)((float)t->page * ((float)t->lines * ((float)font_line_height() / 64.0f + (float)g)));
                    menu_item_rect(&t->menu, d, &x0, &y0, &w0, &h0);
                    font_text_set(*list);
                    font_draw_wtext_ex(x0 << 6, (y0 - off) << 6, w0 << 6, 0x4400, t->text, -1);
                    *list = font_text_clear();
                }
                *list = dl_disable(*list, 2);
            }
        }
        tut_text_font_end(flags);
    }
    if (t->wait != 0 && tut_text_typing() == 0 && tut_text_anim_busy() == 0 && t->icon != 0) {
        int x = d->x + d->w;
        int yy = d->y + d->h;

        *list = dl_ambient_color(*list, 0xFFFFFFFF);
        *list = gim_set_texture(*list, t->icon);
        tut_text_icon(list, x + tut_text_icon_off[0], yy + tut_text_icon_off[1], 0, tut_text_icon_rect[0],
                      tut_text_icon_rect[1], tut_text_icon_rect[2], tut_text_icon_rect[3], t->blink / 10 & 1);
    }
}
#endif

int tut_text_done(void)
{
    return tut_tb.done;
}

void tut_text_set_icon(void *gim)
{
    tut_tb.icon = gim;
}

void tut_text_set_size(int size)
{
    MenuItemDef *d = &tut_tb.def;

    d->font_size = size;
    d->title_font_size = size;
}

void tut_text_set_wait(int on)
{
    tut_tb.wait = on;
}

/* name (the speaker) goes in the title bar */
#ifdef NON_MATCHING
/* o original guarda &tut_tb em $s0 antes das chamadas; o mwcc dobra o ponteiro em lui + deslocamento */
void tut_text_set(unsigned short *text, unsigned short *name)
{
    MenuItemDef *d = &tut_tb.def;
    TutText *t;

    d->show_title = name != 0;
    tut_tb.finished = 0;
    tut_tb.done = 0;
    tut_tb.page_full = 0;
    tut_tb.next_pos = 0;
    tut_tb.page = 0;
    d->title = name;
    tut_tb.text = text;
    t = (TutText *)(char *)&tut_tb;
    tut_text_layout();
    tut_text_paginate();
    if (t->state == 0)
        tut_text_open();
}
#endif

void tut_text_open(void)
{
    tut_tb.state = 1;
}

void tut_text_close(void)
{
    tut_tb.state = 3;
}

int tut_text_typing(void)
{
    TutText *t = &tut_tb;

    if (tut_tb.finished != 0)
        return 0;
    if (t->page_full != 0)
        return 0;
    if (t->text == 0)
        return 0;
    return t->state == 2;
}

#ifdef NON_MATCHING
/* o case 1 do original salta para um bloco li v0,1 próprio (1 palavra) */
int tut_text_anim_busy(void)
{
    int r;

    switch (tut_tb.state) {
    default:
        r = 0;
        break;
    case 1:
        r = 1;
        break;
    case 3:
        r = 1;
        break;
    }
    return r;
}
#endif

/* slide target: x, y and width of the box */
void tut_text_place(int x, int y, int w)
{
    tut_tb.pos2[0] = x;
    tut_tb.pos2[1] = y;
    tut_tb.w = w;
    tut_tb.state = 2;
    tut_tb.def.w = w;
}

#ifdef NON_MATCHING
/* ordem das gravações nos campos do MenuItemDef e registradores */
void tut_text_build(void)
{
    MenuItemDef *d = &tut_tb.def;

    menu_init(&tut_tb.menu, tut_tb.heap);
    memset(d, 0, 0x78);
    d->visible = 1;
    d->auto_title = 1;
    d->font_flag4 = 1;
    d->style2 = 2;
    d->pad_t = 2;
    d->style = tut_tb.style;
    d->show_title = 0;
    d->y = 0xD2;
    d->x = tut_tb.x;
    d->text = 0;
    d->title = 0;
    d->h = 0x3C;
    d->color = 0xFFFFFFFF;
    d->title_color = 0xFFFFFFFF;
    d->w = tut_tb.w;
    d->text_color = 0xFF000000;
    d->pad_l = 4;
    d->font_size = tut_tb.size;
    d->title_font_size = tut_tb.size;
    d->pad_r = 0x14;
    menu_add_item(&tut_tb.menu, d);
    tut_text_reset(tut_tb.x, tut_tb.w, tut_tb.size, tut_tb.wait, tut_tb.style);
}
#endif

void tut_text_blink(void)
{
    TutText *t = &tut_tb;

    if (tut_text_typing() == 0)
        t->blink++;
}

/* type more of the current line; at its end go to the next line, the next page or stop */
#ifdef NON_MATCHING
/* base da linha corrente e ordem dos testes do fim de linha/página */
void tut_text_advance(void)
{
    TutText *t = &tut_tb;
    TutLine *l;
    unsigned short flags;
    int w;
    int n;

    if (tut_tb.text == 0) {
        t->finished = 1;
        return;
    }
    if (t->finished != 0)
        return;
    if (t->page_full != 0)
        return;
    l = &t->line[t->cur];
    font_get_flag8000();
    flags = tut_text_font_begin();
    w = (font_measure_text(t->text + l->start) >> 6) + 1;
    if (w >= t->text_w)
        w = t->text_w;
    tut_text_font_end(flags);
    l->w += tut_text_speeds[t->speed];
    if (l->w < w)
        return;
    l->w = w;
    if (t->state != 2)
        return;
    t->next_pos += t->line[t->cur].count;
    n = t->next_pos - 1;
    if (n == wcslen(t->text)) {
        t->blink = 0;
        t->finished = 1;
    } else if (t->cur == t->lines - 1) {
        t->blink = 0;
        t->page_full = 1;
    } else {
        t->cur++;
    }
}
#endif

#ifdef NON_MATCHING
/* teste da região (região 0 e 1 com corpos separados) e alocação dos $s do laço */
void tut_text_layout(void)
{
    int r;
    TutText *t = &tut_tb;
    MenuItemDef *d = &tut_tb.def;
    unsigned short flags = tut_text_font_begin();
    int g;
    int x, y, w, h;
    int lh;
    int i;
    TutLine *l;
    int dy;
    int gap;

    r = region_get();
    g = (r == 1 || r == 0) ? 1 : 3;
    t->cur = 0;
    t->pos = 0;
    menu_item_rect(&t->menu, d, &x, &y, &w, &h);
    t->text_w = w;
    lh = font_line_height() >> 6;
    for (i = 0, l = t->line, dy = 0, gap = 0; i < t->lines; i++, l++, dy += lh, gap += g) {
        l->x = x;
        l->y = y + dy;
        l->w = 0;
        l->h = lh;
        l->start = 0;
        if (region_get() != 0)
            l->y += gap;
    }
    tut_text_font_end(flags);
}
#endif

#ifdef NON_MATCHING
/* teste da região (região 0 e 1 com corpos separados) e alocação dos $s do laço */
void tut_text_relayout(void)
{
    int r;
    TutText *t = &tut_tb;
    MenuItemDef *d = &tut_tb.def;
    unsigned short flags = tut_text_font_begin();
    int g;
    int x, y, w, h;
    int lh;
    int i;
    TutLine *l;
    int dy;
    int gap;

    r = region_get();
    g = (r == 1 || r == 0) ? 1 : 3;
    t->def.x = (int)t->pos2[0];
    t->def.y = (int)t->pos2[1];
    menu_item_rect(&t->menu, d, &x, &y, &w, &h);
    lh = (font_line_height() >> 6) + 1;
    for (i = 0, l = t->line, dy = 0, gap = 0; i < t->lines; i++, l++, dy += lh, gap += g) {
        l->x = x;
        l->y = y + dy;
        if (region_get() != 0)
            l->y += gap;
    }
    tut_text_font_end(flags);
}
#endif

/* frame-th w x h cell of a strip */
void tut_text_icon(void **list, int x, int y, int z, int u, int v, int w, int h, int frame)
{
    *list = draw_set_offsets(*list, w, h, w, h);
    *list = draw_sprite(*list, x, y, z, u + w * frame, v, 0, 0);
}

/* font state of the box; returns the previous flags for tut_text_font_end */
#ifdef NON_MATCHING
/* forma igual, $s0/$s2 trocados entre o MenuItemDef e o espaçamento */
unsigned short tut_text_font_begin(void)
{
    TutText *t;
    MenuItemDef *d;
    unsigned short flags;
    int g;
    int r;

    t = &tut_tb;
    d = &tut_tb.def;
    flags = font_get_flags();
    r = region_get();
    g = (r == 1 || r == 0) ? 1 : 3;
    if (region_get() == 0) {
        font_set_flag1(1);
        font_set_flag8000(t->flag8000);
    } else {
        font_set_flag8(1);
        font_set_flag8000(0);
    }
    font_set_flag2(1);
    font_set_size(d->font_size, d->font_size);
    font_set_flag4(d->font_flag4);
    font_set_color(d->text_color);
    font_set_line_extra(g << 6);
    return flags;
}
#endif

void tut_text_font_end(unsigned short flags)
{
    font_set_flags(flags);
    font_set_line_extra(0);
}

/* split the text from pos into the lines of a page (a space at a line end is skipped) */
void tut_text_paginate(void)
{
    TutText *t = &tut_tb;
    unsigned short flags;
    int pos;
    int i;
    int e;

    if (tut_tb.text == 0)
        return;
    flags = tut_text_font_begin();
    font_set_color(0);
    pos = t->pos;
    for (i = 0; i < t->lines; i++) {
        TutLine *l = &t->line[i];

        l->start = pos;
        l->count = font_draw_wtext_ex(l->x << 6, l->y << 6, t->text_w << 6, (l->h + 2) << 6, t->text + pos, -1);
        e = pos + l->count;
        if (e < wcslen(t->text)) {
            if (t->text[e] == ' ')
                l->count++;
        }
        pos += l->count;
    }
    tut_text_font_end(flags);
}
