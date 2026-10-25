// @range 0x0881AB64-0x0881D918
// @asm 0x0881B728-0x0881B764
// @asm 0x0881D3EC-0x0881D6B4
// @rodata 0x08822688-0x088226C0
/* Help viewer widgets: the button bar at the bottom, a generic scrolling list of rows of varying
   height (DkPager: input, layout, clipped drawing through callbacks), the topic list, the page list
   of a topic and the text pane, plus the frame/scroll-bar sprite helpers they share. */
#include "libdeck.h"
#include "menu.h"
#include "allegrex.h"

typedef struct DkPager DkPager;
typedef void *(*DkPagerRectFn)(void *list, int x, int y, int w, int h);
typedef void *(*DkPagerRowFn)(void *list, int i, int x, int y, int w, int h);
typedef int (*DkPagerHeightFn)(int i, int w);

struct DkPager {                       /* 0x34: vertical list of rows of varying height */
    unsigned short n;                  /* +0x00 rows */
    unsigned short cur;                /* +0x02 cursor row */
    unsigned char active : 1;          /* +0x04 takes input */
    unsigned char no_ok : 1;           /*       the OK button is ignored */
    unsigned char no_cancel : 1;       /*       the cancel button is ignored */
    unsigned char no_move : 1;         /*       up/down are ignored */
    unsigned char pad04 : 4;
    unsigned char pad05[3];
    int result;                        /* +0x08 1 OK, 2 cancel */
    short x, y;                        /* +0x0C frame */
    unsigned short w, h;               /* +0x10 */
    short pad_t, pad_b, pad_l, pad_r;  /* +0x14 margins of the rows inside the frame */
    DkPagerRectFn draw_bg;             /* +0x1C frame */
    DkPagerRowFn draw_row;             /* +0x20 row contents */
    DkPagerRowFn draw_row_bg;          /* +0x24 row background */
    DkPagerRectFn draw_cursor;         /* +0x28 cursor box on the current row */
    DkPagerHeightFn row_height;        /* +0x2C height of row i for a width */
    short scroll;                      /* +0x30 offset of the first row */
    unsigned short total;              /* +0x32 height of all rows */
};

typedef struct DkHelpBar {             /* dk_help_bar 0x095D28E4, 0xD0: button legend at the bottom */
    void *heap;
    void *gim;                         /* +0x04 */
    void *strtbl;                      /* +0x08 */
    Menu menu;                         /* +0x0C */
    MenuItemDef def;                   /* +0x58 */
} DkHelpBar;

extern DkHelpBar dk_help_bar;

typedef struct DkHelpTopics {          /* dk_help_topics 0x095D29B4, 0x120: list of the topics */
    int state;                         /* +0x000 0 idle, 1 opening, 2 open, 3 input, 4 picked, 5/6 closing, 7 closed */
    void *heap;                        /* +0x004 */
    void *gim;                         /* +0x008 icons */
    void *gim_box;                     /* +0x00C box sprites */
    void *strtbl;                      /* +0x010 help texts */
    void *strtbl_ui;                   /* +0x014 */
    DkPager pager;                     /* +0x018 */
    Menu menu;                         /* +0x04C */
    MenuItemDef def;                   /* +0x098 */
    int first;                         /* +0x110 text of the first row */
    int last;                          /* +0x114 */
    unsigned int timer;                /* +0x118 delay after picking */
    int sel;                           /* +0x11C row selected when (re)opened */
} DkHelpTopics;

extern DkHelpTopics dk_help_topics;

typedef struct DkHelpPages {           /* dk_help_pages 0x095D2AD4, 0x118: pages of the current topic */
    int state;                         /* +0x000 */
    void *heap;                        /* +0x004 */
    void *gim;                         /* +0x008 icons */
    void *gim_box;                     /* +0x00C box sprites */
    void *strtbl;                      /* +0x010 help texts */
    void *strtbl_ui;                   /* +0x014 */
    DkPager pager;                     /* +0x018 */
    Menu menu;                         /* +0x04C */
    MenuItemDef def;                   /* +0x098 */
    int first;                         /* +0x110 text of the first row */
    int last;                          /* +0x114 */
} DkHelpPages;

extern DkHelpPages dk_help_pages;

typedef struct DkHelpText {            /* dk_help_text 0x095D2BEC, 0xEC: text of the current page */
    void *heap;
    void *gim;                         /* +0x04 icons */
    void *gim_box;                     /* +0x08 */
    void *image;                       /* +0x0C picture under the text (help EHP gim), 0 none */
    Menu menu;                         /* +0x10 */
    MenuItemDef def;                   /* +0x5C */
    unsigned short *text;              /* +0xD4 */
    int scroll;                        /* +0xD8 <= 0 */
    int text_h;                        /* +0xDC */
    int image_w;                       /* +0xE0 */
    int image_h;                       /* +0xE4 */
    int total;                         /* +0xE8 text, picture and margin */
} DkHelpText;

extern DkHelpText dk_help_text;

typedef struct DkGimImage {            /* gim_find_block(gim, 4) */
    unsigned short pad[4];
    unsigned short w;                  /* +0x08 */
    unsigned short h;                  /* +0x0A */
} DkGimImage;

void *dk_help_sprite(void *list, int x, int y, int u, int v, int w, int h);
void *dk_help_draw_rect(void *list, int x, int y, int w, int h, unsigned int color);
void *dk_help_box(void *list, void *gim, int x, int y, int w, int h, unsigned int color);
void *dk_help_box_blink(void *list, void *gim, int x, int y, int w, int h, unsigned int color, unsigned int frame);
void *dk_help_box_flash(void *list, void *gim, int x, int y, int w, int h, unsigned int color, unsigned int frame);
void *dk_help_scrollbar(void *list, void *gim, int x, int y, int h, int scroll, int total, int view, int arrows);
void *dk_help_frame(void *list, int x, int y, int u, int v, int cw, int ch, int w, int h, int bw, int bh);
void dk_pager_init(DkPager *p);
void dk_pager_update(DkPager *p);
void *dk_pager_draw(void *list, DkPager *p);
void dk_pager_free_(DkPager *p);
int dk_pager_get_cur_(DkPager *p);
void dk_help_font_white(void);
void dk_help_font_black(void);
float ceilf_vfpu(float x);
void *dk_help_topics_draw_bg(void *list);
void *dk_help_topics_draw_row(void *list, int i, int x, int y, int w, int h);
void *dk_help_topics_draw_row_bg(void *list, int i, int x, int y, int w, int h);
void *dk_help_topics_draw_cursor(void *list, int x, int y, int w, int h);
void *dk_help_topics_draw_scroll(void *list, int x, int y, int h, int scroll, int total);
int dk_help_topics_row_height(int i, int w);
void *dk_help_pages_draw_bg(void *list);
void *dk_help_pages_draw_row(void *list, int i, int x, int y, int w, int h);
void *dk_help_pages_draw_row_bg(void *list, int i, int x, int y, int w, int h);
void *dk_help_pages_draw_cursor(void *list, int x, int y, int w, int h);
void *dk_help_pages_draw_frame(void *list, int x, int y, int h, int scroll, int total);
int dk_help_pages_row_height(int i, int w);
void dk_help_pages_font(void);
int dk_help_pages_is_heading(unsigned short *text);
int wcscpy(unsigned short *dst, const unsigned short *src);
void dk_pager_set_active_(DkPager *p, int on);
void dk_pager_set_cur_(DkPager *p, int i);
void *dl_color(void *list, unsigned int color);
void *dk_help_sprite_ex(void *list, int x, int y, int u, int v, int tw, int th, int w, int h);
void *gim_find_block(void *g, int id);
void dk_help_text_set(unsigned short *text);
void dk_help_text_measure(void);
void dk_help_text_font(void);
float fabsf_vfpu(float x);
void *dk_help_scroll_track(void *list, int x, int y, int h);
void *dk_help_scroll_thumb(void *list, int x, int y, int h, int lit);
void *dk_help_scroll_arrows(void *list, int x1, int y1, int x2, int y2);
void *dk_help_scroll_up(void *list, int x, int y);
void *dk_help_scroll_down(void *list, int x, int y);

/* the bar frame, in the menu of its own */
void dk_help_bar_init(void *heap, void *gim, void *strtbl)
{
    MenuItemDef *d;

    memset(&dk_help_bar, 0, sizeof(dk_help_bar));
    dk_help_bar.heap = heap;
    dk_help_bar.gim = gim;
    dk_help_bar.strtbl = strtbl;
    d = &dk_help_bar.def;
    menu_init(&dk_help_bar.menu, heap);
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->x = 5;
    d->y = 0xEE;
    d->w = 0x1D6;
    d->h = 0x1E;
    d->color = 0xFFFFFFFF;
    d->style = 0;
    menu_add_item(&dk_help_bar.menu, d);
}

void dk_help_bar_font(void);

/* button icons and their legends (the third one, "select", only when `no_select` is 0) */
void *dk_help_bar_draw(void *list, int no_select)
{
    DkHelpBar *b = &dk_help_bar;
    int dx1, dx2, dx3;
    int w0, w1, w2, w3;
    unsigned short *t;

    switch (cfg_get_language()) {
    case 0:
    default:
        w0 = 480;
        dx1 = 0;
        dx2 = 0;
        dx3 = 0;
        w1 = 480;
        w2 = 480;
        w3 = 480;
        break;
    case 1:
        dx1 = 15;
        dx2 = 50;
        dx3 = 25;
        w0 = 100;
        w1 = 125;
        w2 = 57;
        w3 = 101;
        break;
    case 2:
        dx1 = 15;
        dx2 = 50;
        dx3 = 25;
        w0 = 100;
        w1 = 125;
        w2 = 57;
        w3 = 101;
        break;
    case 3:
        dx1 = 15;
        dx2 = 50;
        dx3 = 25;
        w0 = 100;
        w1 = 125;
        w2 = 57;
        w3 = 101;
        break;
    case 4:
        dx1 = 15;
        dx2 = 50;
        dx3 = 25;
        w0 = 100;
        w1 = 125;
        w2 = 57;
        w3 = 101;
        break;
    case 5:
        dx1 = 15;
        dx2 = 50;
        dx3 = 25;
        w0 = 100;
        w1 = 125;
        w2 = 57;
        w3 = 101;
        break;
    }
    menu_layout(&list, &b->menu);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_enable(list, 9);
    list = gim_set_texture(list, b->gim);
    list = dk_help_sprite(list, 13, 244, 0, 16, 9, 19);
    list = dk_help_sprite(list, dx1 + 109, 244, 13, 16, 15, 19);
    if (!no_select)
        list = dk_help_sprite(list, dx2 + 215, 244, 46, 35, 18, 18);
    list = dk_help_sprite(list, dx3 + 315, 250, 0, 35, 26, 11);
    dk_help_bar_font();
    font_text_set(list);
    t = table_get_entry(b->strtbl, 5);
    font_draw_wtext(0x600, ((region_get() ? 5 : 0) + 242) << 6, w0 << 6, t);
    t = table_get_entry(b->strtbl, 6);
    font_draw_wtext((dx1 + 125) << 6, ((region_get() ? 5 : 0) + 242) << 6, w1 << 6, t);
    if (!no_select) {
        t = table_get_entry(b->strtbl, 7);
        font_draw_wtext((dx2 + 235) << 6, ((region_get() ? 5 : 0) + 242) << 6, w2 << 6, t);
    }
    t = table_get_entry(b->strtbl, 8);
    font_draw_wtext((dx3 + 345) << 6, ((region_get() ? 5 : 0) + 242) << 6, w3 << 6, t);
    list = font_text_clear();
    return list;
}

void dk_help_bar_free(void)
{
    menu_free_items(&dk_help_bar.menu);
    memset(&dk_help_bar, 0, sizeof(dk_help_bar));
}

/* 14-pixel black text */
void dk_help_bar_font(void)
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
    font_set_size(14, 14);
    font_set_color(0xFF000000);
}

void dk_pager_clear_result(DkPager *p)
{
    p->result = 0;
}

/* OK / cancel / up-down (wrapping) on the active list */
void dk_pager_input(DkPager *p)
{
    PadState *pad;

    if (p->result != 2 && p->result != 1 && p->active) {
        pad = pad_get();
        if (pad->b.trig & region_pick_value(0)) {
            if (!p->no_ok) {
                p->result = 1;
                se_play(0x6213);
            }
        } else if (pad->b.trig & region_pick_value(1)) {
            if (!p->no_cancel) {
                p->result = 2;
                se_play(0x6214);
            }
        } else if (!p->no_move) {
            if (pad->b.rep & 0x40) {
                if (p->cur < p->n - 1)
                    p->cur++;
                else
                    p->cur = 0;
                se_play(0x6212);
            }
            if (pad->b.rep & 0x10) {
                if (p->cur > 0)
                    p->cur--;
                else
                    p->cur = p->n - 1;
                se_play(0x6212);
            }
        }
    }
}

void dk_pager_free(DkPager *p)
{
}

static inline void pager_set_active(DkPager *p, int on)
{
    p->active = on;
}

void dk_pager_set_active(DkPager *p, int on)
{
    pager_set_active(p, on);
}

int dk_pager_get_cur(DkPager *p)
{
    return p->cur;
}

void dk_pager_set_cur(DkPager *p, int i)
{
    p->cur = i;
}

void dk_help_topics_set(int first, int last, int sel);

/* topic list window: rows first..last of strtbl, cursor on `sel` */
void dk_help_topics_init(void *heap, void *gim, void *gim_box, void *strtbl, void *strtbl_ui, int first, int last, int sel)
{
    DkPager *p = &dk_help_topics.pager;

    memset(&dk_help_topics, 0, sizeof(dk_help_topics));
    dk_help_topics.strtbl = strtbl;
    dk_help_topics.strtbl_ui = strtbl_ui;
    dk_help_topics.heap = heap;
    dk_help_topics.gim = gim;
    dk_help_topics.gim_box = gim_box;
    memset(p, 0, sizeof(*p));
    p->x = 75;
    p->y = 30;
    p->w = 401;
    p->h = 208;
    p->pad_t = 4;
    p->pad_b = 5;
    p->pad_l = 5;
    p->pad_r = 12;
    p->draw_bg = (DkPagerRectFn)dk_help_topics_draw_bg;
    p->draw_row = dk_help_topics_draw_row;
    p->draw_row_bg = dk_help_topics_draw_row_bg;
    p->draw_cursor = dk_help_topics_draw_cursor;
    p->row_height = dk_help_topics_row_height;
    dk_pager_init(p);
    dk_help_topics_set(first, last, sel);
}

/* (re)fills the list with rows first..last and rebuilds its window */
void dk_help_topics_set(int first, int last, int sel)
{
    DkPager *p = &dk_help_topics.pager;
    MenuItemDef *d;

    dk_help_topics.state = 0;
    dk_help_topics.first = first;
    dk_help_topics.last = last;
    p->n = last - first + 1;
    dk_help_topics.timer = 0;
    dk_help_topics.sel = sel;
    p->active = 1;
    p->cur = sel;
    p->result = 0;
    d = &dk_help_topics.def;
    menu_free_items(&dk_help_topics.menu);
    menu_init(&dk_help_topics.menu, dk_help_topics.heap);
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
    menu_add_item(&dk_help_topics.menu, d);
}

/* window open/close animation and the pick */
void dk_help_topics_update(void)
{
    DkHelpTopics *t = &dk_help_topics;
    DkPager *p = &dk_help_topics.pager;
    MenuItemDef *d = &dk_help_topics.def;

    dk_pager_update(p);
    switch (t->state) {
    case 0:
    case 7:
        break;
    case 1:
        menu_item_open(&t->menu, d);
        t->state = 2;
        se_play(0x6216);
        break;
    case 2:
        if (menu_item_is_open(&t->menu, d)) {
            pager_set_active(p, 0);
            t->state = 3;
        }
        break;
    case 3:
        switch (p->result) {
        case 0:
            break;
        case 1:
            pager_set_active(p, 1);
            t->state = 4;
            break;
        case 2:
            p->active = 1;
            p->cur = t->sel;
            t->state = 5;
            break;
        }
        break;
    case 4:
        t->timer++;
        if (t->timer >= 8)
            t->state = 5;
        break;
    case 5:
        menu_item_close(&t->menu, d);
        t->state = 6;
        break;
    case 6:
        if (menu_item_is_open(&t->menu, d))
            t->state = 7;
        break;
    }
}

/* title of the current row above the list */
void *dk_help_topics_draw_title(void *list)
{
    DkPager *p = &dk_help_topics.pager;
    unsigned short *t;

    list = dk_pager_draw(list, p);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_enable(list, 9);
    list = gim_set_texture(list, dk_help_topics.gim);
    list = dk_help_frame(list, 75, 4, 22, 10, 5, 5, 401, 24, 2, 2);
    dk_help_font_white();
    font_text_set(list);
    t = table_get_entry(dk_help_topics.strtbl, p->cur + dk_help_topics.first);
    font_draw_text(0x1400, (region_get() == 0 ? 6 : 10) << 6, 0x6400, t);
    return font_text_clear();
}

void dk_help_topics_free(void)
{
    dk_pager_free_(&dk_help_topics.pager);
    menu_free_items(&dk_help_topics.menu);
    memset(&dk_help_topics, 0, sizeof(dk_help_topics));
}

void dk_help_topics_set_active(int on)
{
    dk_help_topics.pager.active = on;
}

void dk_help_topics_open(void)
{
    dk_help_topics.state = 1;
}

int dk_help_topics_get_cur(void)
{
    return dk_pager_get_cur_(&dk_help_topics.pager);
}

#ifdef NON_MATCHING /* o original repete o `li v0,1` do ramo (cópia morta do delay slot), forma igual */
/* 0 still open, 1 a topic was picked, 2 cancelled */
int dk_help_topics_result(void)
{
    DkPager *p = &dk_help_topics.pager;
    int r = 0;

    if (dk_help_topics.state == 7)
        r = p->result != 2 ? 1 : 2;
    return r;
}
#endif

/* window frame and the scroll bar */
void *dk_help_topics_draw_bg(void *list)
{
    DkPager *p = &dk_help_topics.pager;

    menu_layout(&list, &dk_help_topics.menu);
    list = dk_help_topics_draw_scroll(list, 465, 30, 208, p->scroll, p->total);
    return list;
}

/* row i: bullet icon and its text */
void *dk_help_topics_draw_row(void *list, int i, int x, int y, int w, int h)
{
    DkHelpTopics *t = &dk_help_topics;

    switch (t->state) {
    case 0:
    case 1:
    case 2:
    case 6:
    case 7:
        return list;
    }
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_enable(list, 9);
    list = gim_set_texture(list, t->gim);
    list = dk_help_sprite(list, x + 3, y + (region_get() == 0 ? 6 : 0), 42, 0, 16, 16);
    dk_help_font_black();
    font_text_set(list);
    font_draw_wtext_ex((x + 21) << 6, y << 6, (w - 19) << 6, h << 6, table_get_entry(t->strtbl, i + t->first), -1);
    return font_text_clear();
}

void *dk_help_topics_draw_row_bg(void *list, int i, int x, int y, int w, int h)
{
    DkHelpTopics *t = &dk_help_topics;

    switch (t->state) {
    case 0:
    case 1:
    case 2:
    case 6:
    case 7:
        return list;
    }
    return dk_help_box(list, t->gim_box, x, y + 1, w, h - 2, 0xFFA1F9FF);
}

/* cursor box: flashing once picked, blinking while the list has the focus */
void *dk_help_topics_draw_cursor(void *list, int x, int y, int w, int h)
{
    DkHelpTopics *t = &dk_help_topics;
    DkPager *p = &dk_help_topics.pager;

    switch (t->state) {
    case 0:
    case 1:
    case 2:
    case 6:
    case 7:
        return list;
    }
    if (t->state == 4)
        list = dk_help_box_flash(list, t->gim_box, x, y, w, h, 0, *gfx_get_state());
    else if (p->active)
        list = dk_help_box_blink(list, t->gim_box, x, y, w, h, 0, *gfx_get_state());
    else
        list = dk_help_box(list, t->gim_box, x, y, w, h, 0);
    return list;
}

/* separator line and scroll bar right of the list */
void *dk_help_topics_draw_scroll(void *list, int x, int y, int h, int scroll, int total)
{
    DkHelpTopics *t = &dk_help_topics;

    switch (t->state) {
    case 0:
    case 1:
    case 2:
    case 6:
    case 7:
        return list;
    }
    list = dk_help_draw_rect(list, x, y + 3, 1, 202, 0xFF479FC1);
    return dk_help_scrollbar(list, t->gim, x, y + 3, h - 6, scroll, total, h - 9, 1);
}

/* lines of row i's text at width w */
int dk_help_topics_row_height(int i, int w)
{
    unsigned short *text = table_get_entry(dk_help_topics.strtbl, i + dk_help_topics.first);

    table_get_entry(dk_help_topics.strtbl_ui, 2);
    dk_help_font_black();
    return ceilf_vfpu(font_wtext_lines((w - 19) << 6, text) / 64.0f);
}

void dk_help_font_flags(void)
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

void dk_help_font_white(void)
{
    dk_help_font_flags();
    font_set_size(14, 14);
    font_set_color(0xFFFFFFFF);
}

void dk_help_font_black(void)
{
    dk_help_font_flags();
    font_set_size(14, 14);
    font_set_color(0xFF000000);
}

void dk_pager_init(DkPager *p)
{
    dk_pager_clear_result(p);
    p->scroll = 0;
    p->total = 0;
}

/* input, then scrolls so that the cursor row is inside the frame and sums the rows' heights */
void dk_pager_update(DkPager *p)
{
    int w = p->w - p->pad_l - p->pad_r;
    int sum;
    int i;
    int top;
    int y;
    int end;

    dk_pager_input(p);
    sum = 0;
    for (i = 0; i < p->cur; i++)
        sum += p->row_height(i, w);
    end = sum + p->row_height(p->cur, w);
    y = p->pad_t + (p->scroll + (p->y + sum));
    top = p->y + p->pad_t;
    if (y < top)
        p->scroll += (short)(top - y);
    y = p->pad_t + (p->scroll + (p->y + end));
    top = p->y + p->h - p->pad_b;
    if (y > top)
        p->scroll += (short)(top - y);
    p->total = 0;
    for (i = 0; i < p->n; i++)
        p->total += p->row_height(i, w);
}

/* frame, then the visible rows clipped to it (background, cursor box, contents) */
void *dk_pager_draw(void *list, DkPager *p)
{
    int w;
    int sum;
    int i;
    int y;
    int h;
    int cw;

    if (p->draw_bg)
        list = p->draw_bg(list, p->x, p->y, p->w, p->h);
    if (p->draw_row || p->draw_row_bg || p->draw_cursor) {
        w = cw = p->w - p->pad_l - p->pad_r;
        sum = 0;
        if (p->x < 480 && p->y < 272) {
            int cx = p->x + p->pad_l;
            int cy;
            int ch;

            cx = cx >= 0 ? cx : 0;
            cy = p->y + p->pad_t;
            cy = cy >= 0 ? cy : 0;
            ch = p->h - p->pad_t - p->pad_b;
            if (cx + cw > 480)
                cw = 480 - p->x;
            if (cy + ch > 272)
                ch = 272 - p->y;
            list = dl_scissor(list, cx, cy, cw, ch);
            list = dl_enable(list, 2);
        }
        for (i = 0; i < p->n; i++) {
            y = p->scroll + (sum + (p->y + p->pad_t));
            h = p->row_height(i, w);
            sum += h;
            if (p->y < y + h && y < p->y + p->h) {
                void *l = p->draw_row_bg(list, i, p->x + p->pad_l, y, w, h);
                if (p->cur == i && p->draw_cursor)
                    l = p->draw_cursor(l, p->x + p->pad_l, y, w, h);
                list = p->draw_row(l, i, p->x + p->pad_l, y, w, h);
            }
        }
        list = dl_disable(list, 2);
    }
    return list;
}

void dk_pager_free_(DkPager *p)
{
    dk_pager_free(p);
}

void dk_pager_set_active_(DkPager *p, int on)
{
    dk_pager_set_active(p, on);
}

int dk_pager_get_cur_(DkPager *p)
{
    return dk_pager_get_cur(p);
}

void dk_pager_set_cur_(DkPager *p, int i)
{
    dk_pager_set_cur(p, i);
}

void dk_help_pages_set(int first, int last);

/* page list window of a topic: rows first..last of strtbl */
void dk_help_pages_init(void *heap, void *gim, void *gim_box, void *strtbl, void *strtbl_ui, int first, int last)
{
    DkPager *p = &dk_help_pages.pager;
    MenuItemDef *d = &dk_help_pages.def;

    memset(&dk_help_pages, 0, sizeof(dk_help_pages));
    dk_help_pages.strtbl = strtbl;
    dk_help_pages.strtbl_ui = strtbl_ui;
    dk_help_pages.heap = heap;
    dk_help_pages.gim = gim;
    dk_help_pages.gim_box = gim_box;
    menu_init(&dk_help_pages.menu, heap);
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->color = 0xFFFFFFFF;
    d->style = 0;
    menu_add_item(&dk_help_pages.menu, d);
    memset(p, 0, sizeof(*p));
    p->pad_t = 1;
    p->no_ok = 1;
    p->no_cancel = 1;
    p->x = 7;
    p->y = 31;
    p->w = 191;
    p->h = 205;
    p->pad_b = 2;
    p->pad_r = 16;
    p->draw_bg = (DkPagerRectFn)dk_help_pages_draw_bg;
    p->draw_row = dk_help_pages_draw_row;
    p->draw_row_bg = dk_help_pages_draw_row_bg;
    p->draw_cursor = dk_help_pages_draw_cursor;
    p->row_height = dk_help_pages_row_height;
    p->pad_l = 0;
    dk_pager_init(p);
    dk_help_pages_set(first, last);
}

void dk_help_pages_set(int first, int last)
{
    DkPager *p = &dk_help_pages.pager;

    dk_help_pages.state = 0;
    dk_help_pages.first = first;
    p->n = last - first + 1;
    p->cur = 0;
    dk_help_pages.last = last;
}

void dk_help_pages_update(void)
{
    dk_pager_update(&dk_help_pages.pager);
}

void *dk_help_pages_draw(void *list)
{
    return dk_pager_draw(list, &dk_help_pages.pager);
}

void dk_help_pages_free(void)
{
    dk_pager_free_(&dk_help_pages.pager);
    menu_free_items(&dk_help_pages.menu);
    memset(&dk_help_pages, 0, sizeof(dk_help_pages));
}

void dk_help_pages_set_active(int on)
{
    dk_pager_set_active_(&dk_help_pages.pager, on);
}

int dk_help_pages_get_cur(void)
{
    return dk_pager_get_cur_(&dk_help_pages.pager);
}

void dk_help_pages_set_cur(int i)
{
    dk_pager_set_cur_(&dk_help_pages.pager, i);
}

void *dk_help_pages_draw_bg(void *list)
{
    DkPager *p = &dk_help_pages.pager;

    return dk_help_pages_draw_frame(list, 185, 32, 205, p->scroll, p->total);
}

/* row i: a heading (text starting with the heading mark) or a bulleted page title */
void *dk_help_pages_draw_row(void *list, int i, int x, int y, int w, int h)
{
    unsigned short buf[256];
    DkHelpPages *pg = &dk_help_pages;
    unsigned short *text = table_get_entry(dk_help_pages.strtbl, i + dk_help_pages.first);
    int heading = dk_help_pages_is_heading(text);
    unsigned short *mark;
    int indent;

    dk_help_pages_font();
    font_text_set(list);
    if (heading) {
        mark = table_get_entry(pg->strtbl_ui, 3);
        indent = ceilf_vfpu(font_measure_text(mark) / 64.0f);
        wcscpy(buf, text + 1);
        font_draw_text(x << 6, (y + 1) << 6, (480 - x) << 6, mark);
        font_draw_wtext_ex((x + indent) << 6, (y + 1) << 6, (w - indent + 1) << 6, (h + 1) << 6, buf, -1);
    } else {
        mark = table_get_entry(pg->strtbl_ui, 4);
        indent = ceilf_vfpu(font_measure_text(mark) / 64.0f);
        font_draw_text(x << 6, (y + 1) << 6, (480 - x) << 6, mark);
        font_draw_wtext_ex((x + indent) << 6, (y + 1) << 6, (w - indent + 1) << 6, (h + 1) << 6, text, -1);
    }
    return font_text_clear();
}

/* headings get a filled box; every row a separator line under it */
void *dk_help_pages_draw_row_bg(void *list, int i, int x, int y, int w, int h)
{
    DkHelpPages *pg = &dk_help_pages;

    if (dk_help_pages_is_heading(table_get_entry(dk_help_pages.strtbl, i + dk_help_pages.first))) {
        list = dl_ambient_color(list, 0xFF8DB6C5);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_enable(list, 4);
        list = dl_set_tex_filter(list, 0, 0);
        list = dl_enable(list, 9);
        list = gim_set_texture(list, pg->gim_box);
        list = dk_help_frame(list, x, y + 1, 0, 0x70, 11, 11, w, h - 1, 1, 1);
    }
    return dk_help_draw_rect(list, x, y + h - 1, w, 1, 0xFF879FA5);
}

void *dk_help_pages_draw_cursor(void *list, int x, int y, int w, int h)
{
    DkHelpPages *pg = &dk_help_pages;

    if (dk_help_pages.pager.active)
        return dk_help_box_blink(list, pg->gim_box, x, y + 1, w, h - 1, 0, *gfx_get_state());
    return dk_help_box(list, pg->gim_box, x, y, w, h - 1, 0);
}

/* window frame (sized to the list) and the scroll bar */
void *dk_help_pages_draw_frame(void *list, int x, int y, int h, int scroll, int total)
{
    MenuItemDef *d = &dk_help_pages.def;

    d->w = 13;
    d->x = x;
    d->y = y;
    d->h = h - 1;
    menu_layout(&list, &dk_help_pages.menu);
    list = dk_help_scrollbar(list, dk_help_pages.gim, x + 2, y + 6, h - 11, scroll, total, h - 3, dk_help_pages.pager.active);
    return list;
}

int dk_help_pages_row_height(int i, int w)
{
    unsigned short *text = table_get_entry(dk_help_pages.strtbl, i + dk_help_pages.first);
    unsigned short *mark = table_get_entry(dk_help_pages.strtbl_ui, 3);
    int indent;

    dk_help_pages_font();
    indent = ceilf_vfpu(font_measure_text(mark) / 64.0f);
    return ceilf_vfpu(font_wtext_lines((w - indent + 1) << 6, text) / 64.0f);
}

void dk_help_pages_font(void)
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
    font_set_size(14, 14);
    font_set_color(0xFF000000);
}

/* heading rows start with the mark character of strtbl_ui entry 3 */
int dk_help_pages_is_heading(unsigned short *text)
{
    return *text == *table_get_entry(dk_help_pages.strtbl_ui, 3);
}

/* flat rectangle */
void *dk_help_draw_rect(void *list, int x, int y, int w, int h, unsigned int color)
{
    list = dl_disable(list, 9);
    list = dl_color(list, color);
    list = draw_set_offsets(list, 1, 1, w, h);
    return draw_sprite(list, x, y, 0, 0, 0, 0, 0);
}

/* w x h texels at (u, v) drawn unscaled at (x, y) */
void *dk_help_sprite(void *list, int x, int y, int u, int v, int w, int h)
{
    return dk_help_sprite_ex(list, x, y, u, v, w, h, w, h);
}

/* tw x th texels at (u, v) stretched to w x h at (x, y) */
void *dk_help_sprite_ex(void *list, int x, int y, int u, int v, int tw, int th, int w, int h)
{
    list = draw_set_offsets(list, tw, th, w, h);
    return draw_sprite(list, x, y, 0, u, v, 0, 0);
}

/* nine-slice box: the cw x ch texel frame at (u, v) with bw x bh corners, drawn as w x h at (x, y) */
void *dk_help_frame(void *list, int x, int y, int u, int v, int cw, int ch, int w, int h, int bw, int bh)
{
    int v2, u2, y2, x2;
    int w2, cw2, u3, x3;
    int v3, y3, ch2, h2;

    list = dk_help_sprite(list, x, y, u, v, bw, bh);
    v2 = v + ch - bh;
    y2 = y + h - bh;
    list = dk_help_sprite(list, x, y2, u, v2, bw, bh);
    u2 = u + cw - bw;
    x2 = x + w - bw;
    list = dk_help_sprite(list, x2, y, u2, v, bw, bh);
    list = dk_help_sprite(list, x2, y2, u2, v2, bw, bh);
    w2 = w - bw * 2;
    cw2 = cw - bw * 2;
    u3 = u + bw;
    x3 = x + bw;
    list = dk_help_sprite_ex(list, x3, y, u3, v, cw2, bh, w2, bh);
    list = dk_help_sprite_ex(list, x3, y2, u3, v2, cw2, bh, w2, bh);
    v3 = v + bh;
    y3 = y + bh;
    ch2 = ch - bh * 2;
    h2 = h - bh * 2;
    list = dk_help_sprite_ex(list, x, y3, u, v3, bw, ch2, bw, h2);
    list = dk_help_sprite_ex(list, x2, y3, u2, v3, bw, ch2, bw, h2);
    return dk_help_sprite_ex(list, x3, y3, u3, v3, cw2, ch2, w2, h2);
}

/* text pane right of the page list */
void dk_help_text_init(void *heap, void *gim, void *gim_box, unsigned short *text)
{
    MenuItemDef *d = &dk_help_text.def;

    memset(&dk_help_text, 0, sizeof(dk_help_text));
    dk_help_text.heap = heap;
    dk_help_text.gim = gim;
    dk_help_text.gim_box = gim_box;
    menu_init(&dk_help_text.menu, heap);
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->x = 200;
    d->y = 32;
    d->w = 275;
    d->h = 205;
    d->color = 0xFFFFFFFF;
    d->style = 0;
    menu_add_item(&dk_help_text.menu, d);
    dk_help_text_set(text);
}

/* new text, back to the top, no picture */
void dk_help_text_set(unsigned short *text)
{
    dk_help_text.image = 0;
    dk_help_text.text = text;
    dk_help_text.scroll = 0;
    dk_help_text.image_h = 0;
    dk_help_text.total = 0;
    dk_help_text_font();
    dk_help_text.text_h = font_wtext_lines(0x4080, dk_help_text.text) / 64.0f;
    dk_help_text_measure();
}

void dk_help_text_update(void)
{
}

/* frame, scroll bar, the text clipped to the pane and the picture under it */
void *dk_help_text_draw(void *list)
{
    DkHelpText *t = &dk_help_text;
    unsigned short *text;
    int scroll;
    int h;

    menu_layout(&list, &dk_help_text.menu);
    list = dk_help_scrollbar(list, dk_help_text.gim, 463, 38, 194, dk_help_text.scroll, dk_help_text.total, 195, 1);
    list = dk_help_draw_rect(list, 462, 34, 1, 200, 0xFF479FC1);
    list = dl_scissor(list, 205, 37, 258, 195);
    list = dl_enable(list, 2);
    dk_help_text_font();
    font_text_set(list);
    text = dk_help_text.text;
    scroll = dk_help_text.scroll;
    h = 195 - scroll + font_line_height();
    font_draw_wtext_ex(0x3340, (scroll + 37) << 6, 0x4080, h << 6, text, -1);
    list = font_text_clear();
    if (dk_help_text.image) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_enable(list, 9);
        list = gim_set_texture(list, t->image);
        list = dk_help_sprite(list, 205, t->scroll + 47 + t->text_h, 0, 0, t->image_w, t->image_h);
    }
    list = dl_disable(list, 2);
    return list;
}

void dk_help_text_free(void)
{
    menu_free_items(&dk_help_text.menu);
    memset(&dk_help_text, 0, sizeof(dk_help_text));
}

/* analog stick scrolls the text */
void dk_help_text_scroll(void)
{
    DkHelpText *t = &dk_help_text;
    PadState *pad = pad_get();

    if (pad->b.analog_y > 158)
        t->scroll -= ((pad->b.analog_y - 158) * 4) / 64.0f;
    if (pad->b.analog_y < 98)
        t->scroll -= ((pad->b.analog_y - 98) * 4) / 64.0f;
    if (t->scroll > 0)
        t->scroll = 0;
    if (t->total > 195) {
        if (t->scroll < 195 - t->total)
            t->scroll = 195 - t->total;
    } else
        t->scroll = 0;

}

void dk_help_text_set_image(void *gim)
{
    DkHelpText *t = &dk_help_text;

    dk_help_text.image = gim;
    if (gim) {
        DkGimImage *img = gim_find_block(gim, 4);
        t->image_w = img->w;
        t->image_h = img->h;
    } else {
        t->image_w = 0;
        t->image_h = 0;
    }
    dk_help_text_measure();
}

void dk_help_text_measure(void)
{
    dk_help_text.total = dk_help_text.text_h + dk_help_text.image_h + 10;
}

void dk_help_text_font(void)
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
    font_set_size(14, 14);
    font_set_color(0xFF000000);
}

/* window01-style box (texels (0, 112), 11x11 with 1-pixel borders) tinted `color` (0: light blue) */
void *dk_help_box(void *list, void *gim, int x, int y, int w, int h, unsigned int color)
{
    if (color == 0)
        color = 0xFF389DFF;
    list = dl_ambient_color(list, color);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_enable(list, 4);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_enable(list, 9);
    list = gim_set_texture(list, gim);
    return dk_help_frame(list, x, y, 0, 112, 11, 11, w, h, 1, 1);
}

/* box whose alpha pulses with the frame counter */
void *dk_help_box_blink(void *list, void *gim, int x, int y, int w, int h, unsigned int color, unsigned int frame)
{
    unsigned int alpha;

    if (color == 0)
        color = 0xFF389DFF;
    frame = frame / 6 % 8;
    if (frame > 4)
        frame = 8 - frame;
    alpha = (unsigned char)(208 - frame * 16);
    color = (color & 0xFFFFFF) | alpha << 24;
    return dk_help_box(list, gim, x, y, w, h, color);
}

/* box flashing on and off every two frames */
void *dk_help_box_flash(void *list, void *gim, int x, int y, int w, int h, unsigned int color, unsigned int frame)
{
    unsigned int alpha;

    if (color == 0)
        color = 0xFF389DFF;
    alpha = (unsigned char)((frame >> 1 & 1) * 255);
    color = (color & 0xFFFFFF) | alpha << 24;
    return dk_help_box(list, gim, x, y, w, h, color);
}

#ifdef NON_MATCHING /* forma igual; o original usa nove registradores $s (top em s8), o nosso reaproveita um */
/* vertical scroll bar: track of height h at (x, y), thumb sized view/total, blinking arrows */
void *dk_help_scrollbar(void *list, void *gim, int x, int y, int h, int scroll, int total, int view, int arrows)
{
    int max = ALLEGREX_MAX(view, total);
    int thumb = (float)(h - 12) * ((float)view / (float)max);
    int top;
    int bottom;
    int end;

    float pos;

    if (scroll == 0 || max == view)
        pos = 0.0f;
    else
        pos = (float)-scroll / (float)(max - view);
    top = y + 6;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_enable(list, 4);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_enable(list, 9);
    list = gim_set_texture(list, gim);
    list = dk_help_scroll_track(list, x, top, h - 12);
    end = y + h - 6;
    list = dk_help_scroll_arrows(list, x + 1, y, x + 1, end);
    list = dk_help_scroll_thumb(list, x + 2, (float)top + pos * ((float)(y + h - thumb - 6) - (float)top), thumb, arrows);
    if (arrows) {
        int up = 0;
        int down = 0;
        unsigned int alpha = fabsf_vfpu((float)(int)(*gfx_get_state() % 30 - 15) / 15.0f * 255.0f);

        list = dl_ambient_color(list, alpha << 24 | 0xFFFFFF);
        if (max == view) {
            up = 0;
            down = 0;
        } else {
            if (pos > 0.0f)
                up = 1;
            if (pos < 1.0f)
                down = 1;
        }
        if (up)
            list = dk_help_scroll_up(list, x + 1, y);
        if (down)
            list = dk_help_scroll_down(list, x + 1, y + h - 6);
    }
    return list;
}
#endif

/* three-piece vertical strip: (u0, v0) w0 x h0 cap, (u1, v1) w1 x th1 stretched to h1, (u2, v2) w2 x h2 cap */
void *dk_help_slice3(void *list, int x, int y, int u0, int v0, int w0, int h0, int u1, int v1, int w1, int th1, int h1, int u2, int v2, int w2, int h2)
{
    list = draw_set_offsets(list, w0, h0, w0, h0);
    list = draw_sprite(list, x, y, 0, u0, v0, 0, 0);
    list = draw_set_offsets(list, w1, th1, w1, h1);
    list = draw_sprite(list, x, y + h0, 0, u1, v1, 0, 0);
    list = draw_set_offsets(list, w2, h2, w2, h2);
    return draw_sprite(list, x, h1 + (y + h0), 0, u2, v2, 0, 0);
}

void *dk_help_scroll_track(void *list, int x, int y, int h)
{
    return dk_help_slice3(list, x, y, 14, 0, 8, 4, 14, 4, 8, 1, h - 7, 14, 5, 8, 3);
}

void *dk_help_scroll_thumb(void *list, int x, int y, int h, int lit)
{
    int u = lit ? 0 : 4;

    return dk_help_slice3(list, x, y, u + 14, 8, 4, 3, u + 14, 11, 4, 1, h - 7, u + 14, 12, 4, 4);
}

void *dk_help_scroll_arrows(void *list, int x1, int y1, int x2, int y2)
{
    list = dk_help_sprite(list, x1, y1, 7, 0, 7, 6);
    return dk_help_sprite(list, x2, y2, 7, 6, 7, 6);
}

void *dk_help_scroll_up(void *list, int x, int y)
{
    return dk_help_sprite(list, x, y, 0, 0, 7, 6);
}

void *dk_help_scroll_down(void *list, int x, int y)
{
    return dk_help_sprite(list, x, y, 0, 6, 7, 6);
}
