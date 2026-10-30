// @range 0x088587EC-0x088589FC
// @data 0x08877878-0x08877880
/* Caption box of the duel: an entry of a text table shown in a dark box centred at the bottom
   of the screen while enabled. */
#include "dd_duelui.h"

typedef struct CaptionBox {            /* 0x08C5B2C4, 0xC */
    int on;
    void *texts;
    int index;
} CaptionBox;

extern CaptionBox caption_box;

int caption_max_w = 384;               /* box text area */
int caption_max_h = 217;

unsigned short font_get_flags(void);
void font_set_flags(unsigned short flags);
unsigned int font_measure_wtext2(int w, int h, unsigned short *text);
void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);

void caption_init(void)
{
    memset(&caption_box, 0, sizeof(caption_box));
}

void caption_term(void)
{
    memset(&caption_box, 0, sizeof(caption_box));
}

void caption_draw(void)
{
    CaptionBox *c = &caption_box;
    unsigned short *text;
    void *dl;
    unsigned int size;
    int w, h, x, y;
    int flags;

    if (caption_box.on == 0 || c->texts == 0)
        return;
    text = table_get_entry(c->texts, c->index);
    if (text == 0)
        return;
    dl = dl_select_layer(8, 0);
    flags = font_get_flags();
    font_set_size(12, 12);
    font_set_color(0xFFFFFFFF);
    size = font_measure_wtext2(caption_max_w << 6, caption_max_h << 6, text);
    w = size >> 16;
    h = size & 0xFFFF;
    x = (0x7800 - w) / 2;
    y = 0x4000 - h;
    dl = dl_disable(dl, 0);
    dl = dl_disable(dl, 1);
    dl = dl_enable(dl, 4);
    dl = dl_blend_func(dl, 0, 10, 10, 0, 0x80808080);
    dl = draw_set_offsets(dl, 0, 0, (w >> 6) + 8, (h >> 6) + 8);
    dl = draw_sprite(dl, (x >> 6) - 4, (y >> 6) - 4, 0, 0, 0, 0, 0);
    font_text_set(dl);
    font_draw_wtext_ex(x, y, w, h, text, -1);
    dl = font_text_clear();
    font_set_flags(flags);
    dl_set_list(dl);
}

void caption_set(void *texts, int index)
{
    caption_box.index = index;
    caption_box.texts = texts;
}

void caption_enable(int on)
{
    caption_box.on = on;
}
