// @range 0x08862DCC-0x088632F4
// @asm 0x08862F38-0x088630E0
/* Duel help page: a framed text (wrapped) with an optional picture below it, scrolled with the
   analog stick, with a scrollbar. */
#include "dd_duelui.h"

typedef struct HelpText {              /* 0x08C5B684, 0xEC */
    void *heap;                        /* 0x00 */
    void *frame_gim;                   /* 0x04 */
    void *parts_gim;                   /* 0x08 */
    void *image;                       /* 0x0C picture under the text (gim), 0 = none */
    Menu menu;                         /* 0x10 */
    MenuItemDef def;                   /* 0x5C */
    unsigned short *text;              /* 0xD4 */
    int scroll;                        /* 0xD8 <= 0 */
    int text_h;                        /* 0xDC */
    int image_w;                       /* 0xE0 */
    int image_h;                       /* 0xE4 */
    int total_h;                       /* 0xE8 */
} HelpText;

extern HelpText help_text;

void *ui_rect(void *dl, int x, int y, int w, int h, unsigned int color);
void *ui_sprite(void *dl, int x, int y, int u, int v, int w, int h);
void *ui_scrollbar(void *dl, void *gim, int x, int y, int h, int pos, int total, int view, int active);
unsigned short *gim_find_block(void *gim, int type);
int font_line_height(void);

void htext_set(unsigned short *text);
void htext_font(void);
void htext_total(void);

void htext_init(void *heap, void *frame_gim, void *parts_gim, unsigned short *text)
{
    MenuItemDef *d = &help_text.def;

    memset(&help_text, 0, sizeof(help_text));
    help_text.heap = heap;
    help_text.frame_gim = frame_gim;
    help_text.parts_gim = parts_gim;
    menu_init(&help_text.menu, heap);
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->x = 200;
    d->y = 32;
    d->w = 275;
    d->h = 205;
    d->color = 0xFFFFFFFF;
    d->style = 0;
    menu_add_item(&help_text.menu, d);
    htext_set(text);
}

void htext_set(unsigned short *text)
{
    help_text.image = 0;
    help_text.text = text;
    help_text.scroll = 0;
    help_text.image_h = 0;
    help_text.total_h = 0;
    htext_font();
    help_text.text_h = (float)font_wtext_lines(0x4080, help_text.text) / 64.0f;
    htext_total();
}

void htext_update(void)
{
}

#ifdef NON_MATCHING /* o mwcc propaga a cópia de help_text.text e relê depois da chamada */
void htext_draw(void *dl)
{
    HelpText *h = &help_text;
    unsigned short *text;
    int scroll;

    menu_layout(&dl, &help_text.menu);
    dl = ui_scrollbar(dl, help_text.frame_gim, 463, 38, 194, help_text.scroll, help_text.total_h, 195, 1);
    dl = ui_rect(dl, 462, 34, 1, 200, 0xFF479FC1);
    dl = dl_scissor(dl, 205, 37, 258, 195);
    dl = dl_enable(dl, 2);
    htext_font();
    font_text_set(dl);
    text = help_text.text;
    scroll = help_text.scroll;
    font_draw_wtext_ex(0x3340, (scroll + 37) << 6, 0x4080, (195 - scroll + font_line_height()) << 6, text, -1);
    dl = font_text_clear();
    if (h->image != 0) {
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
        dl = dl_enable(dl, 9);
        dl = gim_set_texture(dl, h->image);
        dl = ui_sprite(dl, 205, h->scroll + 47 + h->text_h, 0, 0, h->image_w, h->image_h);
    }
    dl = dl_disable(dl, 2);
}
#else
void htext_draw(void *dl);
#endif

void htext_term(void)
{
    menu_free_items(&help_text.menu);
    memset(&help_text, 0, sizeof(help_text));
}

/* the analog stick scrolls the page */
void htext_scroll(void)
{
    HelpText *h = &help_text;
    PadState *pad = pad_get();

    if (pad->b.analog_y > 158)
        h->scroll = (float)h->scroll - (float)((pad->b.analog_y - 158) * 4) / 64.0f;
    if (pad->b.analog_y < 98)
        h->scroll = (float)h->scroll - (float)((pad->b.analog_y - 98) * 4) / 64.0f;
    if (h->scroll > 0)
        h->scroll = 0;
    if (h->total_h > 195) {
        if (h->scroll < 195 - h->total_h)
            h->scroll = 195 - h->total_h;
    } else {
        h->scroll = 0;
    }
}

void htext_set_image(void *gim)
{
    HelpText *h = &help_text;
    unsigned short *b;

    help_text.image = gim;
    if (gim != 0) {
        b = gim_find_block(gim, 4);
        h->image_w = b[4];
        h->image_h = b[5];
    } else {
        h->image_w = 0;
        h->image_h = 0;
    }
    htext_total();
}

void htext_total(void)
{
    help_text.total_h = help_text.text_h + help_text.image_h + 10;
}

void htext_font(void)
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
