// @range 0x0880DFFC-0x0880F3E0
// @asm 0x0880E194-0x0880E8D0
/* The word search (state 6): a window with the word, the last words used and an OK button; the
   word is typed in the system on-screen keyboard; the search runs incrementally with a progress
   bar and the matches become the pane's list (same source as libselect_card sc_word.c and libdeck
   dk_word.c). */
#include "libdeck_swap.h"
#include "menu.h"

typedef struct DsWordWin {             /* ds_word 0x0881A41C, 0x54 bytes */
    unsigned short step;               /* 0 edit, 1 history, 2 accept, 3 closing, 4 searching */
    unsigned short sstep;              /* +0x02 search step */
    unsigned char focus : 2;           /* +0x04 0 word, 2 OK */
    unsigned char pad04 : 6;
    unsigned char pad05;
    unsigned short word[0x20];         /* +0x06 */
    unsigned short progress;           /* +0x46 0..1000 */
    MenuWidget *menu;                  /* +0x48 */
    MenuWidget *hist;                  /* +0x4C */
    MenuWidget *bar;                   /* +0x50 */
} DsWordWin;

extern DsWordWin *ds_word;

void sysvar_set_a90(int v);
void *menu_draw_item(void *list, MenuWidget *m, int x, int y, int which, int lit);
int wcslen(const unsigned short *s);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
int font_measure_text(unsigned short *text);
void *ds_draw_shade(void *list, unsigned short side, unsigned short tab);
void ds_layout_scroll(short keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
void ds_search_start(DsList *l, unsigned short *word);
unsigned short ds_search_step(DsList *l);
void ds_search_finish(DsList *l);
void ds_popup_open(int id, int arg);
void ds_word_search(void);
void ds_word_free(void);
int ds_word_hist_open(void);
void ds_word_bar_open(void);
void *ds_draw_word_win(void *list, int x, int y);
void *ds_draw_hist_item(void *list, unsigned short i, int x, int y);
void *ds_draw_word_bar(void *list, int x, int y);
void ds_word_input_done(unsigned short *text, int r);
int ds_osk_open(unsigned short *target, unsigned short *desc, int len, void (*done)(unsigned short *, int));

/* opens the word search window */
void ds_word_open(void)
{
    MenuItemDef *def;
    MenuWidget *m;

    ds_word = heap_alloc(ds.heap, 0x54);
    if (ds_word == 0)
        return;
    memset(ds_word, 0, 0x54);
    ds_word->menu = heap_alloc(ds.heap, 0x120);
    m = ds_word->menu;
    if (m == 0)
        return;
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    def->x = 0x5E;
    def->y = 0x48;
    def->w = 0x124;
    def->h = 0x30;
    def->color = 0xFFFFFFFF;
    def->pad_l = 2;
    def->pad_t = 2;
    def->pad_r = 2;
    def->pad_b = 2;
    def->show_title = 1;
    def->auto_title = 1;
    def->font_flag4 = 1;
    def->style2 = 2;
    def->title = table_get_entry(res_get(0x8033), 0x12);
    def->title_font_size = 0xC;
    def->title_color = 0xFFFFFFFF;
    def->text = table_get_entry(res_get(0x8033), 0x42);
    def->font_flag4 = 1;
    def->font_size = 0xC;
    def->text_color = 0xFF000000;
    def->open_anim = 1;
    def->close_anim = 1;
    m->heap = ds.heap;
    m->no_input = 1;
    m->draw_all = (MenuDrawRectFn)ds_draw_word_win;
    menu_open(m);
    ds_word_hist_open();
    ds.state = 6;
}

#ifdef NON_MATCHING /* máquina de passos; alocação e ordem dos testes diferentes */
/* state 6 */
void ds_input_result(void)
{
    MenuWidget *hist = ds_word->hist;
    MenuWidget *m = ds_word->menu;
    PadState *pad = pad_get();
    unsigned char c;
    int r;

    switch (ds_word->step) {
    case 4:
        ds_word_search();
        break;
    case 3:
        menu_update(m);
        if (m->state == 6) {
            if (!m->cancelled) {
                menu_reset(m);
                if (hist != 0)
                    menu_reset(hist);
                ds_word->menu = 0;
                ds_word->hist = 0;
                ds_word->step = 4;
                ds_word->sstep = 0;
            } else {
                ds_word_free();
                ds.state = 1;
            }
        }
        break;
    case 2:
        menu_update(ds_word->menu);
        if (ds_word->menu->state != 4) {
            menu_finish(m);
            ds_word->step = 3;
        }
        break;
    case 1:
        c = hist->cursor;
        r = menu_update(ds_word->hist);
        if (r < 0) {
            if (r == -2) {
                ds_word->step = 0;
            } else {
                if (c != 0)
                    return;
                if (!(pad->b.rep & 0x10))
                    return;
                ds_word->step = 0;
                se_play(0x6212);
            }
        } else {
            wcscpy(ds_word->word, ds.words[r]);
            ds_word->step = 0;
            ds_word->focus = 2;
        }
        hist->accepted = 0;
        hist->cancelled = 0;
        hist->no_cancel = 0;
        hist->cursor = hist->count;
        break;
    case 0:
        if (pad->b.trig & region_pick_value(0)) {
            if (ds_word->focus == 2) {
                menu_set_cursor(ds_word->menu, 0);
                se_play(0x6213);
                ds_word->step = 2;
            } else if (ds_word->focus == 0) {
                if (ds_osk_open(ds_word->word, table_get_entry(res_get(0x8033), 0x19), 0x10, ds_osk_done) == 0) {
                    dsv.busy = 1;
                    se_play(0x6213);
                }
            }
        } else if (pad->b.trig & region_pick_value(1)) {
            m->cancelled = 1;
            menu_finish(m);
            se_play(0x6214);
            ds_word->step = 3;
        } else {
            if ((pad->b.rep & 0x80) && ds_word->focus != 0) {
                ds_word->focus = 0;
                se_play(0x6212);
            }
            if ((pad->b.rep & 0x20) && ds_word->focus != 2) {
                ds_word->focus = 2;
                se_play(0x6212);
            }
            if (ds_word->focus == 0 && (pad->b.rep & 0x40) && hist != 0) {
                ds_word->step = 1;
                hist->cursor = 0;
                hist->no_cancel = 1;
                se_play(0x6212);
            }
        }
        break;
    }
}
#endif

#ifdef NON_MATCHING /* passos da busca com outra ordem */
/* the search itself, a few entries a frame */
void ds_word_search(void)
{
    unsigned int side = dsv.side;
    DsList *l = dsv.pane[side].list;
    unsigned short view;
    unsigned short grid;

    switch (ds_word->sstep) {
    case 0:
        if (ds_word->word[0] == 0) {
            l->filter = 8;
            ds_word->sstep = 10;
        } else if (l->count != 0) {
            ds_word->progress = 0;
            ds_search_start(l, ds_word->word);
            ds_word_bar_open();
            ds_word->sstep++;
        } else {
            ds_search_start(l, ds_word->word);
            ds_search_finish(l);
            ds_word->sstep = 10;
        }
        return;
    case 1:
        menu_update(ds_word->bar);
        if (ds_word->bar->state == 3)
            ds_word->sstep++;
        return;
    case 2:
        ds_word->progress = ds_search_step(l);
        if (ds_word->progress >= 1000)
            ds_word->sstep++;
        return;
    case 3:
        ds_search_finish(l);
        view = dsv.pane[side].list_mode == 0 ? dsv.pane[side].lay.grid_rows : dsv.pane[side].lay.rows;
        grid = dsv.pane[side].list_mode == 0 ? dsv.pane[side].lay.cols : 1;
        ds_layout_scroll(0, 0, (grid & 0xFF) | view << 8, dsv.pane[side].list->count, &dsv.pane[side].top,
                         &dsv.pane[side].cx, &dsv.pane[side].cy);
        menu_finish(ds_word->bar);
        ds_word->sstep++;
        return;
    case 4:
        menu_update(ds_word->bar);
        if (ds_word->bar->state != 6)
            return;
        break;
    case 10:
        break;
    default:
        return;
    }
    ds_word_free();
    ds_popup_open(4, l->count);
    se_play(0x621A);
    ds.state = 1;
}
#endif

void ds_word_free(void)
{
    if (ds_word->bar != 0) {
        menu_reset(ds_word->bar);
        heap_set_level(ds.heap, ds_word->bar);
    }
    if (ds_word->hist != 0) {
        menu_reset(ds_word->hist);
        heap_set_level(ds.heap, ds_word->hist);
    }
    if (ds_word->menu != 0) {
        menu_reset(ds_word->menu);
        heap_set_level(ds.heap, ds_word->menu);
    }
    heap_set_level(ds.heap, ds_word);
    ds_word = 0;
}

/* the history list under the word (when there are words) */
int ds_word_hist_open(void)
{
    int n = 0;
    int i;
    MenuItemDef *def;
    MenuWidget *m;
    int y;

    for (i = 0; i < 8; i++) {
        if (ds.words[i][0] == 0)
            break;
        n++;
    }
    if (n == 0)
        return 0;
    m = ds_word->hist;
    if (m == 0) {
        ds_word->hist = heap_alloc(ds.heap, 0x120);
        m = ds_word->hist;
        if (m == 0)
            return 0;
    }
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    y = menu_cursor_y(&ds_word->menu->menu, &ds_word->menu->def);
    def->x = ds_word->menu->def.x + 6;
    def->y = y + 0xF;
    def->auto_size = 1;
    def->color = 0xFFFFFFFF;
    def->pad_l = 1;
    def->pad_t = 3;
    def->pad_r = 1;
    def->pad_b = 1;
    def->open_anim = 1;
    def->close_anim = 1;
    m->heap = ds.heap;
    m->no_cancel = 1;
    m->no_wrap = 1;
    m->keep_open = 1;
    m->style = 2;
    m->item_w = 0xB0;
    m->item_h = 0xE;
    m->count = n;
    m->cursor = n;
    m->draw_item = (MenuDrawItemFn)ds_draw_hist_item;
    menu_open(m);
    return 1;
}

/* the progress bar window */
void ds_word_bar_open(void)
{
    MenuItemDef *def;
    MenuWidget *m = ds_word->bar;

    if (m == 0) {
        ds_word->bar = heap_alloc(ds.heap, 0x120);
        m = ds_word->bar;
        if (m == 0)
            return;
    }
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 2;
    def->x = 0x8A;
    def->y = 0x70;
    def->w = 0xCC;
    def->h = 0x30;
    def->color = 0xFFFFFFFF;
    def->pad_l = 0xF;
    def->pad_t = 4;
    def->font_size = 0xC;
    def->text_color = 0xFFFFFFFF;
    def->text = table_get_entry(res_get(0x8033), 0x45);
    def->open_anim = 2;
    def->close_anim = 2;
    m->heap = ds.heap;
    m->no_input = 1;
    m->no_cancel = 1;
    m->count = 0;
    m->draw_all = (MenuDrawRectFn)ds_draw_word_bar;
    menu_open(m);
}

void *ds_draw_result(void *list)
{
    unsigned int side;

    if (dsv.mode > 1) {
        side = (dsv.side + 1) & 1;
        list = ds_draw_shade(list, side, dsv.pane[side].list->tab);
    }
    if (ds_word->menu != 0)
        menu_draw(&list, ds_word->menu);
    if (ds_word->bar != 0)
        menu_draw(&list, ds_word->bar);
    return list;
}

/* the word window contents: history, word field (blinking when focused), OK button */
void *ds_draw_word_win(void *list, int x, int y)
{
    MenuWidget *m = ds_word->menu;
    int v = 0;
    void *gim;
    unsigned short *label;
    int tx;

    if (ds_word->hist != 0) {
        menu_draw(&list, ds_word->hist);
        if (ds_word->focus != 2 && (gfx_get_state()[0] >> 3 & 1))
            v += 0x11;
    }
    gim = res_get(0x8006);
    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 4, 0x11, 4, 0x11);
    list = draw_sprite(list, x, y + 4, 0, 0xE0, v, 0, 0);
    list = draw_set_offsets(list, 0xF, 0x11, 0xF, 0x11);
    list = draw_sprite(list, x + 0xB9, y + 4, 0, 0xF1, v, 0, 0);
    list = draw_set_offsets(list, 4, 0x11, 0xB5, 0x11);
    list = draw_sprite(list, x + 4, y + 4, 0, 0xE4, v, 0, 0);
    if (ds_word->step == 0 && ds_word->focus == 0)
        list = menu_draw_cursor(list, ds_word->menu, x + 2, y + 6, 0xB7, 0xD);
    if (ds_word->word[0] != 0) {
        font_text_set(list);
        font_set_size(12, 12);
        font_set_color(0xFFFFFFFF);
        font_draw_text((x + 4) << 6, (y + 7) << 6, 0x2D40, ds_word->word);
        list = font_text_clear();
    }
    label = table_get_entry(res_get(0x8033), 0x17);
    tx = x + 0xD6;
    list = menu_draw_button(list, m, tx, y, 0, ds_word->focus == 2);
    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    tx += (0x3C - (font_measure_text(label) >> 6)) / 2;
    font_draw_text(tx << 6, (y + (0x18 - (font_line_height() >> 6)) / 2) << 6, (480 - tx) << 6, label);
    list = font_text_clear();
    return list;
}

void *ds_draw_hist_item(void *list, unsigned short i, int x, int y)
{
    unsigned int w = ds_word->hist->item_w;
    unsigned int h = ds_word->hist->item_h;

    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    font_draw_text(x << 6, (y + (int)(h - (font_line_height() >> 6)) / 2) << 6, w << 6, ds.words[i]);
    return font_text_clear();
}

/* the progress bar */
void *ds_draw_word_bar(void *list, int x, int y)
{
    void *gim = res_get(0x8006);
    unsigned short e;

    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 4, 7, 4, 7);
    list = draw_sprite(list, x, y + 8, 0, 0, 0xF8, 0, 0);
    list = draw_sprite(list, x + 0xA0, y + 8, 0, 0x1C, 0xF8, 0, 0);
    list = draw_set_offsets(list, 4, 7, 0x9C, 7);
    list = draw_sprite(list, x + 4, y + 8, 0, 4, 0xF8, 0, 0);
    e = ds_word->progress * 0x9A / 1000 + 8;
    list = draw_set_offsets(list, 4, 7, 4, 7);
    list = draw_sprite(list, x + e - 4, y + 8, 0, 0x3A, 0xF8, 0, 0);
    list = draw_sprite(list, x + 1, y + 8, 0, 0x20, 0xF8, 0, 0);
    if (e > 8) {
        list = draw_set_offsets(list, 4, 7, e - 8, 7);
        list = draw_sprite(list, x + 4, y + 8, 0, 0x24, 0xF8, 0, 0);
    }
    return list;
}

/* the name input closed: a non-empty word moves the focus to OK */
void ds_word_input_done(unsigned short *text, int r)
{
    if (ds_word != 0 && ds_word->step == 0 && ds_word->word[0] != 0)
        ds_word->focus = 2;
}

