// @range 0x08808E5C-0x08809B30
// @data 0x0880DCB8-0x0880DD08
/* The index window (select button): a grid of initial letters (kana in Japanese, A-Z elsewhere), dimmed
   when no owned card starts with it, plus an entry for the extra pages; choosing one jumps to the
   first card of that letter. */
#include "libcardalbum.h"

extern CaIndex ca_index;

unsigned short card_top_count(void);
unsigned short card_top_get_letter(unsigned short i);
int card_get_sort_position(int id);
int ca_goto_index(int index);
void ca_index_free(void);
int ca_index_find(int letter);
void *ca_index_draw_cb(void *dl, int x, int y, int w, int h);

/* letter of each cell: Japanese 10x5 grid (0xFF = empty cell), 7x4 alphabet */
unsigned char ca_index_jp[50] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
    0x05, 0x06, 0x07, 0x08, 0x09, 0x1E, 0x1F, 0x20, 0x21, 0x22,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x23, 0xFF, 0x24, 0xFF, 0x25,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x26, 0x27, 0x28, 0x29, 0x2A,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x2B, 0x2C, 0x2D, 0xFF, 0xFF,
};
unsigned char ca_index_en[28] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
    0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0xFF,
};

void ca_index_open(void)
{
    int cols;
    MenuWidget *w;
    MenuItemDef *d;
    int i;

    memset(&ca_index, 0, sizeof(ca_index));
    ca_index.has = heap_alloc(ca.heap, card_top_count());
    if (ca_index.has == 0) {
        ca_index_free();
        return;
    }
    memset(ca_index.has, 0, card_top_count());
    for (i = 0; i < ca.n_cards; i++) {
        if (ca.cards[i] != 0)
            ca_index.has[card_get_sort_position(ca.cards[i])] = 1;
    }
    if (cfg_get_language() == 0) {
        ca_index.cols = 10;
        ca_index.rows = 5;
        ca_index.layout = ca_index_jp;
    } else {
        ca_index.cols = 7;
        ca_index.rows = 4;
        ca_index.layout = ca_index_en;
    }
    if (CA_PAGES(ca.n_total) - CA_PAGES(ca.n_cards) > 0)
        ca_index.extra = 1;
    w = ca_index.menu = heap_alloc(ca.heap, 0x120);
    if (w == 0) {
        ca_index_free();
        return;
    }
    cols = ca_index.cols;
    d = &w->def;
    if (cfg_get_language() == 0)
        cols++;                                /* gap between the two kana halves */
    memset(w, 0, 0x120);
    d->visible = 1;
    d->style = 0;
    d->w = cols * 22 + 17;
    d->h = ca_index.rows * 22 + 17;
    d->x = (480 - d->w) / 2;
    d->y = (272 - d->h) / 2;
    if (ca_index.extra)
        d->h += 22;
    d->color = 0xFFFFFFFF;
    d->pad_l = 8;
    d->pad_t = 8;
    d->pad_r = 8;
    d->pad_b = 8;
    d->show_title = 1;
    d->auto_title = 1;
    d->title = table_get_entry(ca_res_get(CA_STRINGS), 3);
    d->style2 = 2;
    d->title_font_size = 12;
    d->title_color = 0xFFFFFFFF;
    d->open_anim = 2;
    d->close_anim = 2;
    w->heap = ca.heap;
    w->no_cancel = 1;
    w->no_input = 1;
    w->draw_all = ca_index_draw_cb;
    menu_open(w);
    ca.state = 5;
}

void ca_index_free(void)
{
    if (ca_index.has) {
        heap_set_level(ca.heap, ca_index.has);
        ca_index.has = 0;
    }
    if (ca_index.menu) {
        heap_set_level(ca.heap, ca_index.menu);
        ca_index.menu = 0;
    }
}

void ca_index_update(void)
{
    MenuWidget *w = ca_index.menu;
    PadState *pad = pad_get();
    int c;

    menu_update(w);
    switch (w->state) {
    case 3:
        if (pad->b.rep & 0x10) {
            if (ca_index.on_extra) {
                ca_index.on_extra = 0;
                ca_index.row = ca_index.rows - 1;
            } else if (ca_index.row > 0) {
                ca_index.row--;
            } else if (ca_index.extra) {
                ca_index.on_extra = 1;
            } else {
                ca_index.row = ca_index.rows - 1;
            }
            se_play(0x6212);
        }
        if (pad->b.rep & 0x40) {
            if (ca_index.on_extra) {
                ca_index.on_extra = 0;
                ca_index.row = 0;
            } else if (ca_index.row < ca_index.rows - 1) {
                ca_index.row++;
            } else if (ca_index.extra) {
                ca_index.on_extra = 1;
            } else {
                ca_index.row = 0;
            }
            se_play(0x6212);
        }
        if (pad->b.rep & 0x80) {
            if (ca_index.col > 0)
                ca_index.col--;
            else
                ca_index.col = ca_index.cols - 1;
            se_play(0x6212);
        }
        if (pad->b.rep & 0x20) {
            if (ca_index.col < ca_index.cols - 1)
                ca_index.col++;
            else
                ca_index.col = 0;
            se_play(0x6212);
        }
        if (pad->b.trig & region_pick_value(1)) {
            w->cancelled = 1;
            menu_finish(w);
            se_play(0x6214);
        } else if (pad->b.trig & region_pick_value(0)) {
            c = ca_index.col + ca_index.row * ca_index.cols;
            c = ca_index.layout[c];         /* letter under the cursor */
            if (ca_index.on_extra || (c != 0xFF && ca_index.has[c]))
                menu_set_cursor(w, 0);
            se_play(0x6213);
        }
        break;
    case 6:
        ca.state = 3;
        if (!w->cancelled) {
            if (ca_index.on_extra)
                c = CA_PAGES(ca.n_cards) * CA_PER_PAGE;
            else {
                c = ca_index.col + ca_index.row * ca_index.cols;
                c = ca_index_find(ca_index.layout[c]);
            }
            if (c >= 0)
                ca_goto_index(c);
        }
        ca_index_free();
        break;
    }
}

unsigned int *ca_index_draw(unsigned int *list)
{
    menu_draw((void **)&list, ca_index.menu);
    return list;
}

/* content of the window: cursor box, the letters, the extra-pages entry */
void *ca_index_draw_cb(void *dl, int x, int y, int w, int h)
{
    unsigned short ch[2] = { 0 };
    int col;
    int r;
    int c;
    int k;
    int pos;
    int letter;
    int tw;
    int px;

    if (ca_index.on_extra) {
        dl = menu_draw_cursor(dl, ca_index.menu, x + (w - 78) / 2, y + ca_index.rows * 22 - 4, 78, 22);
    } else {
        col = ca_index.col;
        if (cfg_get_language() == 0 && col >= (int)(ca_index.cols / 2u))
            col++;
        dl = menu_draw_cursor(dl, ca_index.menu, x + col * 22 - 4, y + ca_index.row * 22 - 4, 22, 22);
    }
    font_set_size(14, 14);
    font_text_set(dl);
    k = 0;
    for (r = 0; r < ca_index.rows; r++) {
        for (c = 0; c < ca_index.cols; c++, k++) {
            letter = ca_index.layout[k];
            pos = c;
            if (ca_index.has[letter])
                font_set_color(0xFF000000);
            else
                font_set_color(0x80000000);
            ch[0] = card_top_get_letter(letter);
            tw = font_measure_text(ch) >> 6;
            if (cfg_get_language() == 0 && c >= (int)(ca_index.cols / 2u))
                pos++;
            px = ((unsigned int)(14 - tw) >> 1) + (x + pos * 22);
            font_draw_text(px * 64, (y + r * 22) * 64, (480 - px) * 64, ch);
        }
    }
    if (ca_index.extra) {
        font_set_color(0xFF000000);
        tw = font_measure_text(table_get_entry(ca_res_get(CA_STRINGS), 4)) >> 6;
        {
            unsigned short *s = table_get_entry(ca_res_get(CA_STRINGS), 4);
            px = x + ((unsigned int)(w - tw) >> 1);
            font_draw_text(px * 64, (y + ca_index.rows * 22) * 64, (480 - px) * 64, s);
        }
    }
    return font_text_clear();
}

/* first card whose initial is `letter` */
int ca_index_find(int letter)
{
    int i;

    for (i = 0; i < ca.n_cards; i++) {
        if (ca.cards[i] != 0 && letter == card_get_sort_position(ca.cards[i]))
            break;
    }
    return i;
}
