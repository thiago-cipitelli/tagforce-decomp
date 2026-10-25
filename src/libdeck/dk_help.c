// @range 0x0881D918-0x0881E870
// @asm 0x0881D918-0x0881DB9C
// @rodata 0x088226C0-0x08822A00
// @data 0x08823184-0x08823940
/* Help viewer (deck editor and duel rules): the topic list, the pages of the topic and the page
   text, with the picture of the page read from the "help" folder in the background; entry from a
   help id (topic and page tables), per-frame input/fade state machine, drawing and teardown. */
#include "libdeck.h"
#include "menu.h"

typedef struct DkHelpSection {         /* rows of a topic in the page list (strtbl indices) */
    unsigned short first;
    unsigned short last;
} DkHelpSection;

typedef struct DkHelp {                /* dk_help 0x095D2CD8, 0x128 */
    int topic;                         /* +0x00 13..23 */
    void *heap;                        /* +0x04 */
    void *gim;                         /* +0x08 icons */
    void *window;                      /* +0x0C window01.gim of the system pack */
    void *strtbl;                      /* +0x10 help texts */
    void *strtbl_ui;                   /* +0x14 */
    int state;                         /* +0x18 0 fade in, 1 running, 2 fade out, 3 done */
    int on_pages;                      /* +0x1C 1 the page list has the focus, 0 the topic list */
    int topic_row;                     /* +0x20 topic list row seen last */
    int page;                          /* +0x24 page list row */
    int saved_page;                    /* +0x28 page row when the topic list was opened */
    void *image_mem;                   /* +0x2C picture being shown/read */
    int image;                         /* +0x30 page of the picture, -1 reading, -2 none */
    int image_want;                    /* +0x34 page whose picture should be shown */
    int image_read;                    /* +0x38 page being read */
    int topics_first;                  /* +0x3C topic list rows */
    int topics_last;                   /* +0x40 */
    int u44;                           /* +0x44 */
    int pages_first;                   /* +0x48 strtbl row of the first page */
    int u4c;                           /* +0x4C */
    int u50;                           /* +0x50 */
    int topic_base;                    /* +0x54 first topic id (0 duel help, 13 deck help) */
    int text_base;                     /* +0x58 strtbl row of the first page text */
    const DkHelpSection *sections;     /* +0x5C page rows of each topic */
    char **images;                     /* +0x60 picture of each page (0 none) */
    Menu menu;                         /* +0x64 full-screen window */
    MenuItemDef def;                   /* +0xB0 */
} DkHelp;

extern DkHelp dk_help;

int sprintf(char *buf, const char *fmt, ...);
void *syspack_get(void);
int fade_is_done(void);
void *dl_color(void *list, unsigned int color);

/* dk_help_ui.c */
void dk_help_bar_init(void *heap, void *gim, void *strtbl);
void *dk_help_bar_draw(void *list, int no_select);
void dk_help_bar_free(void);
void dk_help_topics_init(void *heap, void *gim, void *gim_box, void *strtbl, void *strtbl_ui, int first, int last, int sel);
void dk_help_topics_set(int first, int last, int sel);
void dk_help_topics_update(void);
void *dk_help_topics_draw_title(void *list);
void dk_help_topics_free(void);
void dk_help_topics_set_active(int on);
void dk_help_topics_open(void);
int dk_help_topics_get_cur(void);
int dk_help_topics_result(void);
void dk_help_pages_init(void *heap, void *gim, void *gim_box, void *strtbl, void *strtbl_ui, int first, int last);
void dk_help_pages_set(int first, int last);
void dk_help_pages_update(void);
void *dk_help_pages_draw(void *list);
void dk_help_pages_free(void);
void dk_help_pages_set_active(int on);
int dk_help_pages_get_cur(void);
void dk_help_pages_set_cur(int i);
void *dk_help_draw_rect(void *list, int x, int y, int w, int h, unsigned int color);
void *dk_help_sprite(void *list, int x, int y, int u, int v, int w, int h);
void dk_help_text_init(void *heap, void *gim, void *gim_box, unsigned short *text);
void dk_help_text_set(unsigned short *text);
void dk_help_text_update(void);
void *dk_help_text_draw(void *list);
void dk_help_text_free(void);
void dk_help_text_scroll(void);
void dk_help_text_set_image(void *gim);

/* this file */
int dk_help_is_deck(int topic);
int dk_help_topic_id(int row);
int dk_help_topic_row(int topic);
int dk_help_topic_of(int id);
int dk_help_page_of(int id);
int dk_help_input(void);
void dk_help_load_pages(int topic, int keep);
void dk_help_load_text(int topic, int keep);
void dk_help_image_index(void);
void *dk_help_draw_header(void *list);
void dk_help_header_font(void);
void dk_help_font_reset(void);
int dk_help_image_cb(int *page, int size);

/* page rows of each topic */
const DkHelpSection dk_help_duel_sections[13] = {
    { 17, 30 }, { 31, 58 }, { 59, 60 }, { 61, 62 }, { 63, 63 }, { 64, 64 }, { 65, 77 },
    { 78, 83 }, { 84, 84 }, { 85, 87 }, { 88, 93 }, { 94, 103 }, { 104, 105 },
};
const DkHelpSection dk_help_deck_sections[11] = {
    { 15, 19 }, { 20, 22 }, { 23, 30 }, { 31, 36 }, { 37, 43 }, { 44, 45 }, { 46, 73 },
    { 74, 76 }, { 77, 82 }, { 83, 92 }, { 93, 94 },
};

/* picture of each page in the "help" folder */
char *dk_help_duel_images[89] = {
    0, 0, 0, 0, "help_duelimg_005.gim", 0, 0, 0, "help_duelimg_009.gim", "help_duelimg_010.gim",
    "help_duelimg_011.gim", "help_duelimg_012.gim", "help_duelimg_013.gim", "help_duelimg_014.gim",
    0, "help_duelimg_016.gim", 0, "help_duelimg_018.gim", "help_duelimg_019.gim",
    "help_duelimg_020.gim", "help_duelimg_021.gim", 0, 0, 0, 0, 0, 0, 0, "help_duelimg_029.gim", 0,
    0, "help_duelimg_032.gim", "help_duelimg_033.gim", "help_duelimg_034.gim",
    "help_duelimg_035.gim", "help_duelimg_036.gim", 0, "help_duelimg_038.gim", 0, 0,
    "help_duelimg_041.gim", "help_duelimg_042.gim", 0, 0, 0, 0, "help_duelimg_047.gim",
    "help_duelimg_048.gim", "help_duelimg_049.gim", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    "help_duelimg_061.gim", "help_duelimg_062.gim", 0, 0, 0, 0, 0, "help_duelimg_068.gim",
    "help_duelimg_069.gim", 0, 0, 0, 0, 0, 0, "help_duelimg_076.gim", "help_duelimg_077.gim", 0, 0,
    "help_duelimg_080.gim", 0, 0, "help_duelimg_083.gim", 0, 0, 0, 0, 0, 0,
};
char *dk_help_deck_images[80] = {
    0, 0, "help_deckimg_003.gim", 0, 0, 0, 0, "help_deckimg_008.gim", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "help_deckimg_033.gim", 0, "help_deckimg_035.gim",
    "help_deckimg_036.gim", "help_deckimg_037.gim", "help_deckimg_038.gim", 0, 0, 0, 0, 0, 0, 0,
    "help_deckimg_046.gim", 0, 0, "help_deckimg_049.gim", "help_deckimg_050.gim",
    "help_deckimg_051.gim", "help_deckimg_052.gim", "help_deckimg_053.gim", 0,
    "help_deckimg_055.gim", 0, 0, "help_deckimg_058.gim", "help_deckimg_059.gim",
    "help_deckimg_060.gim", 0, 0, 0, 0, 0, 0, "help_deckimg_067.gim", "help_deckimg_068.gim", 0, 0,
    "help_deckimg_071.gim", 0, 0, "help_deckimg_074.gim", 0, 0, 0, 0, 0, 0,
};

char dk_help_window_gim[] = "window01.gim";   /* literal of dk_help_init, kept here for the asm */

#ifdef NON_MATCHING /* no ramo da ajuda de duelo o original carrega o 1 de topics_first de novo; o nosso reaproveita o registrador da comparação */
/* builds the help screen on `topic` */
void dk_help_init(int topic, void *heap, void *gim, void *strtbl, void *strtbl_ui)
{
    DkHelp *h = &dk_help;
    MenuItemDef *d;

    memset(h, 0, sizeof(*h));
    h->topic = topic;
    h->heap = heap;
    h->gim = gim;
    h->window = ehp_find(syspack_get(), dk_help_window_gim, 0);
    h->strtbl = strtbl;
    h->strtbl_ui = strtbl_ui;
    h->state = 0;
    h->on_pages = 1;
    h->topic_row = 0;
    h->page = 0;
    h->image = -2;
    h->image_want = -2;
    h->image_read = -2;
    switch (dk_help_is_deck(topic)) {
    default:
            h->topics_first = 1;
            h->topics_last = 15;
            h->u44 = 13;
            h->pages_first = 16;
            h->u4c = 106;
            h->u50 = 89;
            h->topic_base = 0;
            h->text_base = 91;
            h->sections = dk_help_duel_sections;
            h->images = dk_help_duel_images;
        break;
    case 1:
            h->topics_first = 1;
            h->topics_last = 13;
            h->u44 = 11;
            h->pages_first = 14;
            h->u4c = 95;
            h->u50 = 80;
            h->topic_base = 13;
            h->text_base = 82;
            h->sections = dk_help_deck_sections;
            h->images = dk_help_deck_images;
        break;
    }
    d = &h->def;
    menu_init(&h->menu, heap);
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->w = 480;
    d->h = 272;
    d->color = 0xFFFFFFFF;
    d->style = 0;
    d->x = 0;
    d->y = 0;
    menu_add_item(&h->menu, d);
    dk_help_topics_init(h->heap, h->gim, h->window, h->strtbl, h->strtbl_ui, h->topics_first + 1, h->topics_last - 1, dk_help_topic_row(h->topic));
    dk_help_load_pages(h->topic, 0);
    dk_help_load_text(h->topic, 0);
    dk_help_bar_init(h->heap, h->gim, h->strtbl_ui);
    dk_help_topics_set_active(0);
    dk_help_pages_set_active(0);
    fade_start(1, 0xFF000000, 16);
}
#endif

/* opens the help on the page of help id `id` */
void dk_help_open(int id, void *heap, void *gim, void *strtbl, void *strtbl_ui)
{
    int topic = dk_help_topic_of(id);
    int page = dk_help_page_of(id);

    dk_help_init(topic, heap, gim, strtbl, strtbl_ui);
    dk_help_pages_set_cur(page);
}

/* per frame; 1 once the help has faded out */
int dk_help_update(void)
{
    char path[256];
    DkHelp *h = &dk_help;
    int done = 0;
    int row;
    int topic;
    int r;

    dk_help_topics_update();
    dk_help_pages_update();
    dk_help_text_update();
    switch (h->state) {
    default:
        if (fade_is_done())
            h->state = 1;
        break;
    case 1:
        switch (dk_help_input()) {
        case 0:
            break;
        case 1:
            h->saved_page = dk_help_pages_get_cur();
            dk_help_topics_set(h->topics_first + 1, h->topics_last - 1, dk_help_topic_row(h->topic));
            dk_help_topics_open();
            h->topic_row = dk_help_topics_get_cur();
            h->on_pages = 0;
            break;
        case 2:
            h->on_pages = 1;
            break;
        case 3:
            fade_start_in(1, 0xFF000000, 16);
            h->state = 2;
            break;
        }
        if (h->on_pages != 1) {
            row = dk_help_topics_get_cur();
            topic = dk_help_topic_id(row);
            dk_help_topics_set_active(1);
            dk_help_pages_set_active(0);
            if (row != h->topic_row) {
                h->topic_row = row;
                dk_help_load_pages(topic, 1);
                dk_help_load_text(topic, 1);
            }
            r = dk_help_topics_result();
            if (r != 0) {
                h->on_pages = 1;
                if (topic != h->topic)
                    h->topic = topic;
                switch (r) {
                case 1:
                    dk_help_load_pages(topic, 1);
                    dk_help_load_text(topic, 1);
                    break;
                case 2:
                    dk_help_pages_set_cur(h->saved_page);
                    break;
                }
            }
        } else {
            dk_help_topics_set_active(0);
            dk_help_pages_set_active(1);
            dk_help_text_scroll();
        }
        break;
    case 2:
        if (fade_is_done())
            h->state = 3;
        break;
    case 3:
        done = 1;
        break;
    }
    row = dk_help_pages_get_cur();
    if (row != h->page) {
        h->page = row;
        dk_help_load_text(h->topic, 1);
    }
    if (h->image != -1) {
        dk_help_image_index();
        if (h->image_want != h->image && h->on_pages == 1) {
            if (h->image_mem) {
                heap_set_level(h->heap, h->image_mem);
                h->image_mem = 0;
            }
            dk_help_text_set_image(0);
            if (h->images[h->image_want]) {
                int size;

                sprintf(path, "%s/%s", "help", h->images[h->image_want]);
                size = ehfile_get_size(3, path);
                h->image_mem = heap_alloc(h->heap, size);
                h->image_read = h->image_want;
                ehfile_read_async_alias(3, path, h->image_mem, size, (int (*)(void *, int))dk_help_image_cb, &h->image_read);
                h->image = -1;
            } else {
                h->image = -2;
            }
        }
    }
    return done;
}

void *dk_help_draw(void *list)
{
    DkHelp *h = &dk_help;

    list = dk_help_draw_header(list);
    list = dk_help_text_draw(list);
    list = dk_help_pages_draw(list);
    if (dk_help.on_pages == 0)
        list = dk_help_draw_rect(list, 3, 30, 473, 238, 0x80000000);
    list = dk_help_bar_draw(list, h->on_pages == 0);
    list = dk_help_topics_draw_title(list);
    fade_apply(&list);
    return list;
}

/* frees everything once no picture is being read; 1 when done */
int dk_help_close(void)
{
    DkHelp *h = &dk_help;

    if (h->image == -1)
        return 0;
    dk_help_topics_free();
    dk_help_pages_free();
    dk_help_text_free();
    dk_help_bar_free();
    menu_free_items(&h->menu);
    dk_help_font_reset();
    if (h->image_mem) {
        heap_set_level(h->heap, h->image_mem);
        h->image_mem = 0;
    }
    memset(&dk_help, 0, sizeof(dk_help));
    return 1;
}

/* 3 leave (START, or cancel in the page list), 1 open the topic list (TRIANGLE) */
int dk_help_input(void)
{
    DkHelp *h = &dk_help;
    int r = 0;
    unsigned int trig = pad_get()->b.trig;

    if (trig & 8)
        r = 3;
    switch (h->on_pages) {
    case 0:
        break;
    case 1:
        if (trig & region_pick_value(1))
            r = 3;
        else if (trig & 0x1000)
            r = 1;
        break;
    }
    return r;
}

/* page list of a topic (new window, or `keep` the window and change the rows) */
void dk_help_load_pages(int topic, int keep)
{
    DkHelp *h = &dk_help;
    const DkHelpSection *s = &h->sections[dk_help_topic_row(topic)];

    if (keep)
        dk_help_pages_set(s->first, s->last);
    else
        dk_help_pages_init(h->heap, h->gim, h->window, h->strtbl, h->strtbl_ui, s->first, s->last);
    h->page = 0;
}

/* text of the current page of a topic */
void dk_help_load_text(int topic, int keep)
{
    DkHelp *h = &dk_help;
    int first = h->sections[topic - h->topic_base].first;
    int i = h->page + (first + h->text_base);


    if (keep)
        dk_help_text_set(table_get_entry(h->strtbl, i));
    else
        dk_help_text_init(h->heap, h->gim, h->window, table_get_entry(h->strtbl, i));
    dk_help_image_index();
}

/* picture index of the current page */
void dk_help_image_index(void)
{
    int row = dk_help_topic_row(dk_help.topic);

    dk_help.image_want = dk_help.page + (dk_help.sections[row].first - (dk_help.pages_first + 1));
}

/* title bar */
void *dk_help_draw_header(void *list)
{
    menu_layout(&list, &dk_help.menu);
    list = dk_help_draw_rect(list, 3, 3, 473, 26, 0xFFA0CEDF);
    list = dk_help_draw_rect(list, 3, 29, 473, 1, 0xFF48A3BE);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_enable(list, 9);
    list = gim_set_texture(list, dk_help.gim);
    list = dk_help_sprite(list, 10, 10, 32, 0, 9, 14);
    dk_help_header_font();
    font_text_set(list);
    font_draw_wtext(0x580, 0x280, 0xD40, table_get_entry(dk_help.strtbl_ui, 1));
    list = font_text_clear();
    return list;
}

void dk_help_header_font(void)
{
    if (region_get() == 0)
        font_set_flag1(1);
    else
        font_set_flag8(1);
    font_set_flag8000(0);
    font_set_flag4(1);
    font_set_size(15, 15);
    font_set_color(0xFF000000);
}

void dk_help_font_reset(void)
{
    font_set_flag8000(0);
}

/* the picture of `*page` was read: show it if the page is still current, else drop it */
int dk_help_image_cb(int *page, int size)
{
    DkHelp *h = &dk_help;

    if (size > 0) {
        if (h->image_want == *page) {
            dk_help_text_set_image(h->image_mem);
            h->image = *page;
        } else {
            if (h->image_mem) {
                heap_set_level(h->heap, h->image_mem);
                h->image_mem = 0;
            }
            dk_help_text_set_image(0);
            h->image = -2;
        }
        return 1;
    }
    return 0;
}

/* topics 13..23 are the deck editor's */
int dk_help_is_deck(int topic)
{
    int r;

    switch (topic) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    default:
        r = 0;
        break;
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
        r = 1;
        break;
    }
    return r;
}

int dk_help_topic_id(int row)
{
    return dk_help.topic_base + row;
}

int dk_help_topic_row(int topic)
{
    return topic - dk_help.topic_base;
}

/* help topic (13..23) of a help id */
int dk_help_topic_of(int id)
{
    switch (id) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    default:
        return 13;
    case 5:
    case 6:
    case 7:
        return 14;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        return 15;
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
        return 16;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
        return 17;
    case 29:
    case 30:
        return 18;
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
        return 19;
    case 59:
    case 60:
    case 61:
        return 20;
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
        return 21;
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
        return 22;
    case 78:
    case 79:
        return 23;
    }
}

/* page of a help id inside its topic */
int dk_help_page_of(int id)
{
    switch (id) {
    case 0:
    default:
        return 0;
    case 1:
        return 1;
    case 2:
        return 2;
    case 3:
        return 3;
    case 4:
        return 4;
    case 5:
        return 0;
    case 6:
        return 1;
    case 7:
        return 2;
    case 8:
        return 0;
    case 9:
        return 1;
    case 10:
        return 2;
    case 11:
        return 3;
    case 12:
        return 4;
    case 13:
        return 5;
    case 14:
        return 6;
    case 15:
        return 7;
    case 16:
        return 0;
    case 17:
        return 1;
    case 18:
        return 2;
    case 19:
        return 3;
    case 20:
        return 4;
    case 21:
        return 5;
    case 22:
        return 0;
    case 23:
        return 1;
    case 24:
        return 2;
    case 25:
        return 3;
    case 26:
        return 4;
    case 27:
        return 5;
    case 28:
        return 6;
    case 29:
        return 0;
    case 30:
        return 1;
    case 31:
        return 0;
    case 32:
        return 1;
    case 33:
        return 2;
    case 34:
        return 3;
    case 35:
        return 4;
    case 36:
        return 5;
    case 37:
        return 6;
    case 38:
        return 7;
    case 39:
        return 8;
    case 40:
        return 9;
    case 41:
        return 10;
    case 42:
        return 11;
    case 43:
        return 12;
    case 44:
        return 13;
    case 45:
        return 14;
    case 46:
        return 15;
    case 47:
        return 16;
    case 48:
        return 17;
    case 49:
        return 18;
    case 50:
        return 19;
    case 51:
        return 20;
    case 52:
        return 21;
    case 53:
        return 22;
    case 54:
        return 23;
    case 55:
        return 24;
    case 56:
        return 25;
    case 57:
        return 26;
    case 58:
        return 27;
    case 59:
        return 0;
    case 60:
        return 1;
    case 61:
        return 2;
    case 62:
        return 0;
    case 63:
        return 1;
    case 64:
        return 2;
    case 65:
        return 3;
    case 66:
        return 4;
    case 67:
        return 5;
    case 68:
        return 0;
    case 69:
        return 1;
    case 70:
        return 2;
    case 71:
        return 3;
    case 72:
        return 4;
    case 73:
        return 5;
    case 74:
        return 6;
    case 75:
        return 7;
    case 76:
        return 8;
    case 77:
        return 9;
    case 78:
        return 0;
    case 79:
        return 1;
    }
}
