// @range 0x08863998-0x088649D4
// @asm 0x08863998-0x08863C1C
// @asm 0x08863C1C-0x08863F74
// @asm 0x088640B4-0x08864154
// @asm 0x0886452C-0x088645FC
// @asm 0x088645FC-0x0886492C
// @data 0x08878EA0-0x0887965C
// @rodata 0x088727B4-0x08872874
/* Duel help screen (window01.gim): category list, index and text page with its picture, loaded
   from help/<image>.gim on demand; duel help (categories 0..12) and deck help (13..23) differ
   only by their tables. Plus the button guide at the bottom. */
#include "dd_duelui.h"

typedef struct HelpRange {             /* index entries of a category */
    unsigned short first;
    unsigned short last;
} HelpRange;

typedef struct HelpScene {             /* 0x08C5B770, 0x128 */
    int category;                      /* 0x000 */
    void *heap;                        /* 0x004 */
    void *frame_gim;                   /* 0x008 */
    void *parts_gim;                   /* 0x00C window01.gim */
    void *texts;                       /* 0x010 */
    void *pack;                        /* 0x014 */
    int state;                         /* 0x018 0 fade in, 1 run, 2 fade out, 3 done */
    int focus;                         /* 0x01C 1 index, 0 category window */
    int cat_cursor;                    /* 0x020 */
    int idx_cursor;                    /* 0x024 */
    int saved_idx;                     /* 0x028 */
    void *image;                       /* 0x02C loaded picture */
    int image_req;                     /* 0x030 -1 loading, -2 none */
    int image_want;                    /* 0x034 */
    int image_loading;                 /* 0x038 */
    int first;                         /* 0x03C */
    int last;                          /* 0x040 */
    int idx_first;                     /* 0x044 */
    int idx_base;                      /* 0x048 */
    int f4c;
    int f50;
    int cat_base;                      /* 0x054 */
    int text_base;                     /* 0x058 */
    const HelpRange *ranges;           /* 0x05C */
    char **images;                     /* 0x060 */
    Menu menu;                         /* 0x064 */
    MenuItemDef def;                   /* 0x0B0 */
} HelpScene;

typedef struct HelpGuide {             /* 0x08C5B898, 0xD0: button guide */
    void *heap;
    void *frame_gim;
    void *pack;
    Menu menu;                         /* 0x0C */
    MenuItemDef def;                   /* 0x58 */
} HelpGuide;

extern HelpScene help_scene;
extern HelpGuide help_guide;

void *syspack_get(void);
void *ehp_find(void *pack, const char *name, int *size);
int fade_is_done(void);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_apply(void **list);
int heap_set_level(void *h, void *p);
void *heap_alloc(void *h, int n);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int font_draw_wtext(int x, int y, int w, unsigned short *text);
int cfg_get_language(void);
int sprintf(char *, const char *, ...);
void *ui_rect(void *dl, int x, int y, int w, int h, unsigned int color);
void *ui_sprite(void *dl, int x, int y, int u, int v, int w, int h);

void help_init(void *heap, void *frame_gim, void *parts_gim, void *texts, void *pack, int first, int last, int initial);
void help_reset(int first, int last, int initial);
void help_update(void);
void *help_draw(void *dl);
void help_term(void);
void help_set_active(int on);
void help_open(void);
int help_cursor(void);
int help_result(void);
void hidx_init(void *heap, void *frame_gim, void *parts_gim, void *texts, void *pack, int first, int last);
void hidx_reset(int first, int last);
void hidx_update(void);
void *hidx_draw(void *dl);
void hidx_term(void);
void hidx_set_active(int on);
int hidx_cursor(void);
void hidx_set_cursor(int c);
void htext_init(void *heap, void *frame_gim, void *parts_gim, unsigned short *text);
void htext_set(unsigned short *text);
void htext_update(void);
void *htext_draw(void *dl);
void htext_term(void);
void htext_scroll(void);
void htext_set_image(void *gim);

void hscene_load_index(int category, int reset);
void hscene_load_text(int category, int reset);
void hscene_image_want(void);
void *hscene_draw_top(void *dl);
void hscene_top_font(void);
void hscene_font_reset(void);
int hscene_image_done(int *want, int size);
int hscene_is_deck(int category);
int hscene_cat_from_index(int i);
int hscene_index_from_cat(int category);
void hguide_init(void *heap, void *frame_gim, void *pack);
void *hguide_draw(void *dl, int index_only);
void hguide_term(void);
void hguide_font(void);
int hscene_input(void);

const HelpRange help_duel_ranges[13] = {
    { 17, 30 }, { 31, 58 }, { 59, 60 }, { 61, 62 }, { 63, 63 }, { 64, 64 }, { 65, 77 },
    { 78, 83 }, { 84, 84 }, { 85, 87 }, { 88, 93 }, { 94, 103 }, { 104, 105 },
};

const HelpRange help_deck_ranges[11] = {
    { 15, 19 }, { 20, 22 }, { 23, 30 }, { 31, 36 }, { 37, 43 }, { 44, 45 },
    { 46, 73 }, { 74, 76 }, { 77, 82 }, { 83, 92 }, { 93, 94 },
};

char *help_duel_images[89] = {
    0, 0, 0, 0,
    "help_duelimg_005.gim", 0, 0, 0,
    "help_duelimg_009.gim", "help_duelimg_010.gim", "help_duelimg_011.gim", "help_duelimg_012.gim",
    "help_duelimg_013.gim", "help_duelimg_014.gim", 0, "help_duelimg_016.gim",
    0, "help_duelimg_018.gim", "help_duelimg_019.gim", "help_duelimg_020.gim",
    "help_duelimg_021.gim", 0, 0, 0,
    0, 0, 0, 0,
    "help_duelimg_029.gim", 0, 0, "help_duelimg_032.gim",
    "help_duelimg_033.gim", "help_duelimg_034.gim", "help_duelimg_035.gim", "help_duelimg_036.gim",
    0, "help_duelimg_038.gim", 0, 0,
    "help_duelimg_041.gim", "help_duelimg_042.gim", 0, 0,
    0, 0, "help_duelimg_047.gim", "help_duelimg_048.gim",
    "help_duelimg_049.gim", 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    "help_duelimg_061.gim", "help_duelimg_062.gim", 0, 0,
    0, 0, 0, "help_duelimg_068.gim",
    "help_duelimg_069.gim", 0, 0, 0,
    0, 0, 0, "help_duelimg_076.gim",
    "help_duelimg_077.gim", 0, 0, "help_duelimg_080.gim",
    0, 0, "help_duelimg_083.gim", 0,
    0, 0, 0, 0,
    0,
};

char *help_deck_images[80] = {
    0, 0, "help_deckimg_003.gim", 0,
    0, 0, 0, "help_deckimg_008.gim",
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    "help_deckimg_033.gim", 0, "help_deckimg_035.gim", "help_deckimg_036.gim",
    "help_deckimg_037.gim", "help_deckimg_038.gim", 0, 0,
    0, 0, 0, 0,
    0, "help_deckimg_046.gim", 0, 0,
    "help_deckimg_049.gim", "help_deckimg_050.gim", "help_deckimg_051.gim", "help_deckimg_052.gim",
    "help_deckimg_053.gim", 0, "help_deckimg_055.gim", 0,
    0, "help_deckimg_058.gim", "help_deckimg_059.gim", "help_deckimg_060.gim",
    0, 0, 0, 0,
    0, 0, "help_deckimg_067.gim", "help_deckimg_068.gim",
    0, 0, "help_deckimg_071.gim", 0,
    0, "help_deckimg_074.gim", 0, 0,
    0, 0, 0, 0,
};

/* used by the functions kept in asm */
char help_window_gim[] = "window01.gim";
char help_path_fmt[] = "%s/%s";
char help_dir[] = "help";

#ifdef NON_MATCHING /* alocação de registradores/ordem dos testes difere; não insistido */
void hscene_init(int category, void *heap, void *frame_gim, void *texts, void *pack)
{
    HelpScene *s = &help_scene;
    MenuItemDef *d = &help_scene.def;

    memset(&help_scene, 0, sizeof(help_scene));
    s->category = category;
    s->heap = heap;
    s->frame_gim = frame_gim;
    s->parts_gim = ehp_find(syspack_get(), help_window_gim, 0);
    s->state = 0;
    s->focus = 1;
    s->cat_cursor = 0;
    s->idx_cursor = 0;
    s->image_req = -2;
    s->image_want = -2;
    s->image_loading = -2;
    s->texts = texts;
    s->pack = pack;
    if (hscene_is_deck(category) == 1) {
        s->last = 13;
        s->cat_base = 13;
        s->idx_first = 11;
        s->idx_base = 14;
        s->f4c = 95;
        s->f50 = 80;
        s->text_base = 82;
        s->ranges = help_deck_ranges;
        s->images = help_deck_images;
    } else {
        s->last = 15;
        s->idx_first = 13;
        s->idx_base = 16;
        s->f4c = 106;
        s->f50 = 89;
        s->text_base = 91;
        s->ranges = help_duel_ranges;
        s->cat_base = 0;
        s->images = help_duel_images;
    }
    s->first = 1;
    menu_init(&s->menu, heap);
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->w = 480;
    d->h = 272;
    d->color = 0xFFFFFFFF;
    d->style = 0;
    d->x = 0;
    d->y = 0;
    menu_add_item(&s->menu, d);
    help_init(s->heap, s->frame_gim, s->parts_gim, s->texts, s->pack, s->first + 1, s->last - 1,
              hscene_index_from_cat(s->category));
    hscene_load_index(s->category, 0);
    hscene_load_text(s->category, 0);
    hguide_init(s->heap, s->frame_gim, s->pack);
    help_set_active(0);
    hidx_set_active(0);
    fade_start(1, 0xFF000000, 16);
}
#else
void hscene_init(int category, void *heap, void *frame_gim, void *texts, void *pack);
#endif

/* 1 when the screen is over */
#ifdef NON_MATCHING /* alocação de registradores/ordem dos testes difere; não insistido */
int hscene_update(void)
{
    HelpScene *s = &help_scene;
    int r = 0;
    int in, cur, cat, res;
    char path[0x100];
    int size;

    help_update();
    hidx_update();
    htext_update();
    switch (s->state) {
    case 0:
        if (fade_is_done())
            s->state = 1;
        break;
    case 1:
        in = hscene_input();
        switch (in) {
        case 1:
            s->saved_idx = hidx_cursor();
            help_reset(s->first + 1, s->last - 1, hscene_index_from_cat(s->category));
            help_open();
            s->cat_cursor = help_cursor();
            s->focus = 0;
            break;
        case 2:
            s->focus = 1;
            break;
        case 3:
            fade_start_in(1, 0xFF000000, 16);
            s->state = 2;
            break;
        }
        if (s->focus == 1) {
            help_set_active(0);
            hidx_set_active(1);
            htext_scroll();
        } else {
            cur = help_cursor();
            cat = hscene_cat_from_index(cur);
            help_set_active(1);
            hidx_set_active(0);
            if (cur != s->cat_cursor) {
                s->cat_cursor = cur;
                hscene_load_index(cat, 1);
                hscene_load_text(cat, 1);
            }
            res = help_result();
            if (res != 0) {
                s->focus = 1;
                if (cat != s->category)
                    s->category = cat;
                switch (res) {
                case 1:
                    hscene_load_index(cat, 1);
                    hscene_load_text(cat, 1);
                    break;
                case 2:
                    hidx_set_cursor(s->saved_idx);
                    break;
                }
            }
        }
        break;
    case 2:
        if (fade_is_done())
            s->state = 3;
        break;
    case 3:
        r = 1;
        break;
    }
    cur = hidx_cursor();
    if (cur != s->idx_cursor) {
        s->idx_cursor = cur;
        hscene_load_text(s->category, 1);
    }
    if (s->image_req != -1) {
        hscene_image_want();
        if (s->image_want != s->image_req && s->focus == 1) {
            if (s->image != 0) {
                heap_set_level(s->heap, s->image);
                s->image = 0;
            }
            htext_set_image(0);
            if (s->images[s->image_want] == 0) {
                s->image_req = -2;
            } else {
                sprintf(path, help_path_fmt, help_dir, s->images[s->image_want]);
                size = ehfile_get_size(3, path);
                s->image = heap_alloc(s->heap, size);
                s->image_loading = s->image_want;
                ehfile_read_async_alias(3, path, s->image, size, (int (*)(void *, int))hscene_image_done,
                                        &s->image_loading);
                s->image_req = -1;
            }
        }
    }
    return r;
}
#else
int hscene_update(void);
#endif

void *hscene_draw(void *dl)
{
    HelpScene *s = &help_scene;

    dl = hscene_draw_top(dl);
    dl = htext_draw(dl);
    dl = hidx_draw(dl);
    if (help_scene.focus == 0)
        dl = ui_rect(dl, 3, 30, 473, 238, 0x80000000);
    dl = hguide_draw(dl, s->focus == 0);
    dl = help_draw(dl);
    fade_apply(&dl);
    return dl;
}

/* 1 when everything is released (waits for a pending picture load) */
int hscene_term(void)
{
    HelpScene *s = &help_scene;
    int r;

    if (help_scene.image_req == -1) {
        r = 0;
    } else {
        help_term();
        hidx_term();
        htext_term();
        hguide_term();
        menu_free_items(&s->menu);
        hscene_font_reset();
        if (s->image != 0) {
            heap_set_level(s->heap, s->image);
            s->image = 0;
        }
        memset(&help_scene, 0, sizeof(help_scene));
        r = 1;
    }
    return r;
}

/* 3 quit (START, or cancel on the index), 1 open the category window (square) */
#ifdef NON_MATCHING /* alocação de registradores/ordem dos testes difere; não insistido */
int hscene_input(void)
{
    int r = 0;
    PadState *pad = pad_get();
    unsigned int trig = pad->b.trig;

    if (trig & 8)
        r = 3;
    if (help_scene.focus == 1) {
        if (trig & region_pick_value(0))
            r = 3;
        else if (trig & 0x1000)
            r = 1;
    }
    return r;
}
#else
int hscene_input(void);
#endif

void hscene_load_index(int category, int reset)
{
    HelpScene *s = &help_scene;
    const HelpRange *r = &help_scene.ranges[hscene_index_from_cat(category)];

    if (reset != 0)
        hidx_reset(r->first, r->last);
    else
        hidx_init(s->heap, s->frame_gim, s->parts_gim, s->texts, s->pack, r->first, r->last);
    s->idx_cursor = 0;
}

void hscene_load_text(int category, int reset)
{
    HelpScene *s = &help_scene;
    int first = help_scene.ranges[category - help_scene.cat_base].first;
    int i = first + help_scene.text_base + help_scene.idx_cursor;

    if (reset != 0)
        htext_set(table_get_entry(s->texts, i));
    else
        htext_init(s->heap, s->frame_gim, s->parts_gim, table_get_entry(s->texts, i));
    hscene_image_want();
}

void hscene_image_want(void)
{
    int i = hscene_index_from_cat(help_scene.category);

    help_scene.image_want = help_scene.idx_cursor + (help_scene.ranges[i].first - (help_scene.idx_base + 1));
}

void *hscene_draw_top(void *dl)
{
    menu_layout(&dl, &help_scene.menu);
    dl = ui_rect(dl, 3, 3, 473, 26, 0xFFA0CEDF);
    dl = ui_rect(dl, 3, 29, 473, 1, 0xFF48A3BE);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_enable(dl, 9);
    dl = gim_set_texture(dl, help_scene.frame_gim);
    dl = ui_sprite(dl, 10, 10, 32, 0, 9, 14);
    hscene_top_font();
    font_text_set(dl);
    font_draw_wtext(0x580, 0x280, 0xD40, table_get_entry(help_scene.pack, 1));
    dl = font_text_clear();
    return dl;
}

void hscene_top_font(void)
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

void hscene_font_reset(void)
{
    font_set_flag8000(0);
}

/* read callback of the picture */
int hscene_image_done(int *want, int size)
{
    HelpScene *s = &help_scene;

    if (size > 0) {
        if (s->image_want == *want) {
            htext_set_image(s->image);
            s->image_req = *want;
        } else {
            if (s->image != 0) {
                heap_set_level(s->heap, s->image);
                s->image = 0;
            }
            htext_set_image(0);
            s->image_req = -2;
        }
        return 1;
    }
    return 0;
}

int hscene_is_deck(int category)
{
    int r;

    switch (category) {
    case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
    default:
        r = 0;
        break;
    case 13: case 14: case 15: case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
        r = 1;
        break;
    }
    return r;
}

int hscene_cat_from_index(int i)
{
    return help_scene.cat_base + i;
}

int hscene_index_from_cat(int category)
{
    return category - help_scene.cat_base;
}

#ifdef NON_MATCHING /* alocação de registradores/ordem dos testes difere; não insistido */
void hguide_init(void *heap, void *frame_gim, void *pack)
{
    MenuItemDef *d;

    memset(&help_guide, 0, sizeof(help_guide));
    help_guide.heap = heap;
    help_guide.frame_gim = frame_gim;
    help_guide.pack = pack;
    menu_init(&help_guide.menu, heap);
    d = &help_guide.def;
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->x = 5;
    d->y = 238;
    d->w = 470;
    d->h = 30;
    d->color = 0xFFFFFFFF;
    d->style = 0;
    menu_add_item(&help_guide.menu, d);
}
#else
void hguide_init(void *heap, void *frame_gim, void *pack);
#endif

#define GUIDE_Y(base) (((region_get() == 0 ? 0 : 5) + (base)) << 6)

/* button guide: per language offsets of the four captions */
#ifdef NON_MATCHING /* alocação de registradores/ordem dos testes difere; não insistido */
void *hguide_draw(void *dl, int index_only)
{
    int w0, dx1, dx2, dx3, w1, w2, w3;

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
        dx1 = 15; dx2 = 50; dx3 = 25; w0 = 100; w1 = 125; w2 = 57; w3 = 101;
        break;
    case 2:
        dx1 = 15; dx2 = 50; dx3 = 25; w0 = 100; w1 = 125; w2 = 57; w3 = 101;
        break;
    case 3:
        dx1 = 15; dx2 = 50; dx3 = 25; w0 = 100; w1 = 125; w2 = 57; w3 = 101;
        break;
    case 4:
        dx1 = 15; dx2 = 50; dx3 = 25; w0 = 100; w1 = 125; w2 = 57; w3 = 101;
        break;
    case 5:
        dx1 = 15; dx2 = 50; dx3 = 25; w0 = 100; w1 = 125; w2 = 57; w3 = 101;
        break;
    }
    menu_layout(&dl, &help_guide.menu);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_enable(dl, 9);
    dl = gim_set_texture(dl, help_guide.frame_gim);
    dl = ui_sprite(dl, 13, 244, 0, 16, 9, 19);
    dl = ui_sprite(dl, dx1 + 109, 244, 13, 16, 15, 19);
    if (index_only == 0)
        dl = ui_sprite(dl, dx2 + 215, 244, 46, 35, 18, 18);
    dl = ui_sprite(dl, dx3 + 315, 250, 0, 35, 26, 11);
    hguide_font();
    font_text_set(dl);
    font_draw_wtext(0x600, GUIDE_Y(242), w0 << 6, table_get_entry(help_guide.pack, 5));
    font_draw_wtext((dx1 + 125) << 6, GUIDE_Y(242), w1 << 6, table_get_entry(help_guide.pack, 6));
    if (index_only == 0)
        font_draw_wtext((dx2 + 235) << 6, GUIDE_Y(242), w2 << 6, table_get_entry(help_guide.pack, 7));
    font_draw_wtext((dx3 + 345) << 6, GUIDE_Y(242), w3 << 6, table_get_entry(help_guide.pack, 8));
    return font_text_clear();
}
#else
void *hguide_draw(void *dl, int index_only);
#endif

void hguide_term(void)
{
    menu_free_items(&help_guide.menu);
    memset(&help_guide, 0, sizeof(help_guide));
}

void hguide_font(void)
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
