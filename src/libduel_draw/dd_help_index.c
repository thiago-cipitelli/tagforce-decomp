// @range 0x088621A0-0x08862A60
/* Duel help index: a second list window (entries first..last of a table pack) whose items are
   either a heading (marker entry 3 at the start of the text) or a bulleted line (entry 4 as
   bullet), with a frame that follows the list and a scrollbar. */
#include "libduel_draw.h"

typedef struct HelpIndex {             /* 0x08C5B56C, 0x118 */
    int state;                         /* 0x000 */
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
} HelpIndex;

extern HelpIndex help_index;

unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
void *ui_nine_patch(void *dl, int x, int y, int u, int v, int cw, int ch, int w, int h, int sx, int sy);
void *ui_rect(void *dl, int x, int y, int w, int h, unsigned int color);
void *ui_panel(void *dl, void *gim, int x, int y, int w, int h, unsigned int color);
void *ui_panel_pulse(void *dl, void *gim, int x, int y, int w, int h, unsigned int color, int t);
void *ui_scrollbar(void *dl, void *gim, int x, int y, int h, int total, int scroll, int len, int on);

void hidx_reset(int first, int last);
void *hidx_draw_bg(void *dl, int x, int y, int w, int h);
void *hidx_draw_item(void *dl, int i, int x, int y, int w, int h);
void *hidx_draw_item_bg(void *dl, int i, int x, int y, int w, int h);
void *hidx_draw_cursor(void *dl, int x, int y, int w, int h);
int hidx_item_h(int i, int w);
void *hidx_scrollbar(void *dl, int x, int y, int h, int total, int scroll);
void hidx_font(void);
int hidx_is_heading(unsigned short *text);

void hidx_init(void *heap, void *frame_gim, void *parts_gim, void *texts, void *pack, int first, int last)
{
    DdList *l = &help_index.list;
    MenuItemDef *d = &help_index.def;

    memset(&help_index, 0, sizeof(help_index));
    help_index.texts = texts;
    help_index.pack = pack;
    help_index.heap = heap;
    help_index.frame_gim = frame_gim;
    help_index.parts_gim = parts_gim;
    menu_init(&help_index.menu, heap);
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->color = 0xFFFFFFFF;
    d->style = 0;
    menu_add_item(&help_index.menu, d);
    memset(l, 0, sizeof(*l));
    l->pad_t = 1;
    l->no_cancel = 1;
    l->no_accept = 1;
    l->x = 7;
    l->y = 31;
    l->w = 191;
    l->h = 205;
    l->pad_b = 2;
    l->pad_r = 16;
    l->draw_bg = hidx_draw_bg;
    l->draw_item = hidx_draw_item;
    l->draw_item_bg = hidx_draw_item_bg;
    l->draw_cursor = hidx_draw_cursor;
    l->item_h = hidx_item_h;
    l->pad_l = 0;
    list_reset(l);
    hidx_reset(first, last);
}

void hidx_reset(int first, int last)
{
    DdList *l = &help_index.list;

    help_index.state = 0;
    help_index.first = first;
    l->count = last - first + 1;
    l->cursor = 0;
    help_index.last = last;
}

void hidx_update(void)
{
    list_layout(&help_index.list);
}

void *hidx_draw(void *dl)
{
    return list_draw(dl, &help_index.list);
}

void hidx_term(void)
{
    list_term(&help_index.list);
    menu_free_items(&help_index.menu);
    memset(&help_index, 0, sizeof(help_index));
}

void hidx_set_active(int on)
{
    list_set_active(&help_index.list, on);
}

int hidx_cursor(void)
{
    return list_cursor(&help_index.list);
}

void hidx_set_cursor(int c)
{
    list_set_cursor(&help_index.list, c);
}

void *hidx_draw_bg(void *dl, int x, int y, int w, int h)
{
    DdList *l = &help_index.list;

    return hidx_scrollbar(dl, 185, 32, 205, l->scroll, l->total);
}

void *hidx_draw_item(void *dl, int i, int x, int y, int w, int h)
{
    unsigned short buf[0x100];
    HelpIndex *hi = &help_index;
    unsigned short *text = table_get_entry(help_index.texts, i + help_index.first);
    int heading = hidx_is_heading(text);
    unsigned short *mark;
    int mw;

    hidx_font();
    font_text_set(dl);
    if (heading) {
        mark = table_get_entry(hi->pack, 3);
        mw = ceilf_vfpu((float)font_measure_text(mark) / 64.0f);
        wcscpy(buf, text + 1);
        font_draw_text(x << 6, (y + 1) << 6, (480 - x) << 6, mark);
        font_draw_wtext_ex((x + mw) << 6, (y + 1) << 6, (w - mw + 1) << 6, (h + 1) << 6, buf, -1);
    } else {
        mark = table_get_entry(hi->pack, 4);
        mw = ceilf_vfpu((float)font_measure_text(mark) / 64.0f);
        font_draw_text(x << 6, (y + 1) << 6, (480 - x) << 6, mark);
        font_draw_wtext_ex((x + mw) << 6, (y + 1) << 6, (w - mw + 1) << 6, (h + 1) << 6, text, -1);
    }
    return font_text_clear();
}

void *hidx_draw_item_bg(void *dl, int i, int x, int y, int w, int h)
{
    HelpIndex *hi = &help_index;

    if (hidx_is_heading(table_get_entry(help_index.texts, i + help_index.first))) {
        dl = dl_ambient_color(dl, 0xFF8DB6C5);
        dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
        dl = dl_enable(dl, 4);
        dl = dl_set_tex_filter(dl, 0, 0);
        dl = dl_enable(dl, 9);
        dl = gim_set_texture(dl, hi->parts_gim);
        dl = ui_nine_patch(dl, x, y + 1, 0, 0x70, 11, 11, w, h - 1, 1, 1);
    }
    return ui_rect(dl, x, y + h - 1, w, 1, 0xFF879FA5);
}

void *hidx_draw_cursor(void *dl, int x, int y, int w, int h)
{
    HelpIndex *hi = &help_index;

    if (help_index.list.active)
        return ui_panel_pulse(dl, hi->parts_gim, x, y + 1, w, h - 1, 0, *gfx_get_state());
    return ui_panel(dl, hi->parts_gim, x, y, w, h - 1, 0);
}

void *hidx_scrollbar(void *dl, int x, int y, int h, int total, int scroll)
{
    MenuItemDef *d = &help_index.def;

    d->w = 13;
    d->h = h - 1;
    d->x = x;
    d->y = y;
    menu_layout(&dl, &help_index.menu);
    dl = ui_scrollbar(dl, help_index.frame_gim, x + 2, y + 6, h - 11, total, scroll, h - 3, help_index.list.active);
    return dl;
}

int hidx_item_h(int i, int w)
{
    unsigned short *text = table_get_entry(help_index.texts, i + help_index.first);
    unsigned short *mark = table_get_entry(help_index.pack, 3);
    int mw;

    hidx_font();
    mw = ceilf_vfpu((float)font_measure_text(mark) / 64.0f);
    return ceilf_vfpu((float)font_wtext_lines((w - mw + 1) << 6, text) / 64.0f);
}

void hidx_font(void)
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

/* a heading starts with the marker character of entry 3 */
int hidx_is_heading(unsigned short *text)
{
    unsigned short *mark = table_get_entry(help_index.pack, 3);

    return *text == *mark;
}
