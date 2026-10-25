// @range 0x088192C8-0x0881A69C
// @asm 0x08819460-0x088198FC
// @asm 0x088198FC-0x08819B84
/* The word search (scene state 5): a window with the word, the last words used and an OK button;
   the word is typed with the name input (system keyboard); the search runs incrementally with a
   progress bar and the matches become the pane's list. */
#include "libdeck.h"
#include "menu.h"

typedef struct DkWordWin {             /* dk_word 0x095D2730, 0x54 bytes */
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
} DkWordWin;

/* the search word history: 8 words of 32 characters */
#define DK_WORDS ((unsigned short (*)[32])dk_profile)
typedef struct DkWordsView {           /* the same words seen from the dk_deck block (+0x116) */
    unsigned char pad[0x116];
    unsigned short words[8][32];
} DkWordsView;

extern DkWordWin *dk_word;

void *menu_draw_item(void *list, MenuWidget *m, int x, int y, int which, int lit);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
void *dk_draw_shade(void *list, unsigned short side, unsigned short tab, int show_tab);
void dk_layout_scroll(int keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
void dk_search_start(DkList *l, unsigned short *word);
unsigned short dk_search_step(DkList *l);
void dk_search_finish(DkList *l);
void dk_popup_open(int id, int arg);
void dk_word_search(void);
void dk_word_free(void);
int dk_word_hist_open(void);
void dk_word_bar_open(void);
void *dk_draw_word_win(void *list, int x, int y);
void *dk_draw_hist_item(void *list, unsigned short i, int x, int y);
void *dk_draw_word_bar(void *list, int x, int y);
void dk_word_input_done(unsigned short *text, int r);

/* opens the word search window */
void dk_word_open(void)
{
    MenuItemDef *def;
    MenuWidget *m;

    dk_word = heap_alloc(dk_heap, 0x54);
    if (dk_word == 0)
        return;
    memset(dk_word, 0, 0x54);
    dk_word->menu = heap_alloc(dk_heap, 0x120);
    m = dk_word->menu;
    if (m == 0)
        return;
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    def->x = 0x5E;
    def->y = 0x48;
    def->w = 0x124;
    def->h = 0x34;
    def->color = 0xFFFFFFFF;
    def->pad_l = 2;
    def->pad_t = 2;
    def->pad_r = 2;
    def->pad_b = 2;
    def->show_title = 1;
    def->auto_title = 1;
    def->font_flag4 = 1;
    def->style2 = 2;
    def->title = table_get_entry(dk_res_get(0x8033), 0x12);
    def->title_font_size = 0xC;
    def->title_color = 0xFFFFFFFF;
    def->text = table_get_entry(dk_res_get(0x8033), 0x42);
    def->font_flag4 = 1;
    def->font_size = 0xC;
    def->text_color = 0xFF000000;
    def->open_anim = 1;
    def->close_anim = 1;
    m->heap = dk_heap;
    m->no_input = 1;
    m->draw_all = (MenuDrawRectFn)dk_draw_word_win;
    menu_open(m);
    dk_word_hist_open();
    dk_state = 5;
}

#ifdef NON_MATCHING /* máquina de passos; testes e alocação diferentes */
/* scene state 5 */
void dk_word_update(PadState *pad)
{
    MenuWidget *m = dk_word->menu;
    MenuWidget *hist = dk_word->hist;
    unsigned char c;
    int r;

    switch (dk_word->step) {
    case 0:
        if (pad->b.trig & region_pick_value(0)) {
            if (dk_word->focus == 2) {
                menu_set_cursor(m, 0);
                se_play(0x6213);
                dk_word->step = 2;
            } else if (dk_word->focus == 0) {
                dk_name_input_open(dk_word->word, table_get_entry(dk_res_get(0x8033), 0x19), dk_word_input_done);
                se_play(0x6213);
            }
        } else if (pad->b.trig & region_pick_value(1)) {
            m->cancelled = 1;
            menu_finish(m);
            se_play(0x6214);
            if (hist != 0)
                menu_finish(hist);
            dk_word->step = 3;
        } else {
            if ((pad->b.rep & 0x80) && dk_word->focus != 0) {
                dk_word->focus = 0;
                se_play(0x6212);
            }
            if ((pad->b.rep & 0x20) && dk_word->focus != 2) {
                dk_word->focus = 2;
                se_play(0x6212);
            }
            if (dk_word->focus == 0 && (pad->b.rep & 0x40) && hist != 0) {
                dk_word->step = 1;
                hist->cursor = 0;
                hist->no_cancel = 1;
                se_play(0x6212);
            }
        }
        break;
    case 1:
        c = hist->cursor;
        r = menu_update(hist);
        if (r < 0) {
            if (r == -2) {
                dk_word->step = 0;
            } else {
                if (c != 0)
                    return;
                if (!(pad->b.rep & 0x10))
                    return;
                dk_word->step = 0;
                se_play(0x6212);
            }
        } else {
            wcscpy(dk_word->word, DK_WORDS[r]);
            dk_word->step = 0;
            dk_word->focus = 2;
        }
        hist->cancelled = hist->accepted = 0;
        hist->no_cancel = 0;
        hist->cursor = hist->count;
        break;
    case 2:
        menu_update(m);
        if (m->state != 4) {
            menu_finish(m);
            if (hist != 0)
                menu_finish(hist);
            dk_word->step = 3;
        }
        break;
    case 3:
        menu_update(m);
        if (hist != 0)
            menu_update(hist);
        if (m->state == 6) {
            if (!m->cancelled) {
                menu_reset(m);
                if (hist != 0)
                    menu_reset(hist);
                dk_word->menu = 0;
                dk_word->hist = 0;
                dk_word->step = 4;
                dk_word->sstep = 0;
            } else {
                dk_word_free();
                dk_state = 0;
            }
        }
        break;
    case 4:
        dk_word_search();
        break;
    }
}
#endif

#ifdef NON_MATCHING /* passos da busca com outra ordem de blocos */
/* the search itself, 16 entries a frame */
void dk_word_search(void)
{
    DkPane *p = &dkv.pane[dkv.side];
    DkList *l = p->list;

    switch (dk_word->sstep) {
    case 0:
        if (dk_word->word[0] == 0) {
            l->filter = 8;
            dk_word->sstep = 10;
        } else if (l->count <= 0) {
            dk_search_start(l, dk_word->word);
            dk_search_finish(l);
            dk_word->sstep = 10;
        } else {
            dk_word->progress = 0;
            dk_search_start(l, dk_word->word);
            dk_word_bar_open();
            dk_word->sstep++;
        }
        return;
    case 1:
        menu_update(dk_word->bar);
        if (dk_word->bar->state == 3)
            dk_word->sstep++;
        return;
    case 2:
        dk_word->progress = dk_search_step(l);
        if (dk_word->progress >= 1000)
            dk_word->sstep++;
        return;
    case 3:
        dk_search_finish(l);
        dk_layout_scroll(0, 0, (p->list_mode == 0 ? p->lay.cols : 1) & 0xFF | (p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows) << 8,
                         p->list->count, &p->top, &p->cx, &p->cy);
        menu_finish(dk_word->bar);
        dk_word->sstep++;
        return;
    case 4:
        menu_update(dk_word->bar);
        if (dk_word->bar->state != 6)
            return;
        break;
    case 10:
        break;
    default:
        return;
    }
    dk_word_free();
    dk_popup_open(6, l->count);
    se_play(0x621A);
    dk_state = 0;
}
#endif

void dk_word_free(void)
{
    if (dk_word->bar != 0) {
        menu_reset(dk_word->bar);
        heap_set_level(dk_heap, dk_word->bar);
    }
    if (dk_word->hist != 0) {
        menu_reset(dk_word->hist);
        heap_set_level(dk_heap, dk_word->hist);
    }
    if (dk_word->menu != 0) {
        menu_reset(dk_word->menu);
        heap_set_level(dk_heap, dk_word->menu);
    }
    heap_set_level(dk_heap, dk_word);
    dk_word = 0;
}

/* the history list under the word (when there are words) */
int dk_word_hist_open(void)
{
    int n = 0;
    int i;
    MenuItemDef *def;
    MenuWidget *m;
    int y;

    for (i = 0; i < 8; i++) {
        if (((DkWordsView *)&dk_deck)->words[i][0] == 0)
            break;
        n++;
    }
    if (n == 0)
        return 0;
    m = dk_word->hist;
    if (m == 0) {
        dk_word->hist = heap_alloc(dk_heap, 0x120);
        m = dk_word->hist;
        if (m == 0)
            return 0;
    }
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    y = menu_cursor_y(&dk_word->menu->menu, &dk_word->menu->def);
    def->x = dk_word->menu->def.x + 6;
    def->y = y + 0x11;
    def->auto_size = 1;
    def->color = 0xFFFFFFFF;
    def->pad_l = 1;
    def->pad_t = 3;
    def->pad_r = 1;
    def->pad_b = 1;
    def->open_anim = 1;
    def->close_anim = 1;
    m->heap = dk_heap;
    m->no_cancel = 0;
    m->no_wrap = 1;
    m->keep_open = 1;
    m->style = 2;
    m->item_w = 0xB0;
    m->item_h = 0xE;
    m->count = n;
    m->cursor = n;
    m->draw_item = (MenuDrawItemFn)dk_draw_hist_item;
    menu_open(m);
    return 1;
}

/* the progress bar window */
void dk_word_bar_open(void)
{
    MenuItemDef *def;
    MenuWidget *m = dk_word->bar;

    if (m == 0) {
        dk_word->bar = heap_alloc(dk_heap, 0x120);
        m = dk_word->bar;
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
    def->text = table_get_entry(dk_res_get(0x8033), 0x45);
    def->open_anim = 2;
    def->close_anim = 2;
    m->heap = dk_heap;
    m->no_input = 1;
    m->no_cancel = 1;
    m->count = 0;
    m->draw_all = (MenuDrawRectFn)dk_draw_word_bar;
    menu_open(m);
}

void *dk_draw_word(void *list)
{
    unsigned int side;

    if (dkv.mode > 1) {
        side = (dkv.side + 1) & 1;
        list = dk_draw_shade(list, side, dkv.pane[side].list->tab, 1);
    }
    if (dk_word->menu != 0)
        menu_draw(&list, dk_word->menu);
    if (dk_word->bar != 0)
        menu_draw(&list, dk_word->bar);
    return list;
}

/* the word window contents: history, word field (blinking when focused), OK button */
void *dk_draw_word_win(void *list, int x, int y)
{
    MenuWidget *m = dk_word->menu;
    int v = 0;
    void *gim;
    unsigned short *label;
    int tx;

    if (dk_word->hist != 0) {
        menu_draw(&list, dk_word->hist);
        if (dk_word->focus != 2 && (gfx_get_state()[0] >> 3 & 1))
            v += 0x11;
    }
    gim = dk_res_get(0x8006);
    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 4, 0x11, 4, 0x11);
    list = draw_sprite(list, x, y + 5, 0, 0xE0, v, 0, 0);
    list = draw_set_offsets(list, 0xF, 0x11, 0xF, 0x11);
    list = draw_sprite(list, x + 0xB9, y + 5, 0, 0xF1, v, 0, 0);
    list = draw_set_offsets(list, 4, 0x11, 0xB5, 0x11);
    list = draw_sprite(list, x + 4, y + 5, 0, 0xE4, v, 0, 0);
    if (dk_word->step == 0 && dk_word->focus == 0)
        list = menu_draw_cursor(list, dk_word->menu, x + 2, y + 7, 0xB7, 0xD);
    if (dk_word->word[0] != 0) {
        font_text_set(list);
        font_set_size(12, 12);
        font_set_color(0xFFFFFFFF);
        font_draw_text((x + 4) << 6, (y + 7) << 6, 0x2D40, dk_word->word);
        list = font_text_clear();
    }
    label = table_get_entry(dk_res_get(0x8033), 0x17);
    tx = x + 0xD6;
    list = menu_draw_button(list, m, tx, y, 0, dk_word->focus == 2);
    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    tx += (0x3C - (font_measure_text(label) >> 6)) / 2;
    font_draw_text(tx << 6, (y + (0x18 - (font_line_height() >> 6)) / 2) << 6, (480 - tx) << 6, label);
    list = font_text_clear();
    return list;
}

void *dk_draw_hist_item(void *list, unsigned short i, int x, int y)
{
    unsigned int w = dk_word->hist->item_w;
    unsigned int h = dk_word->hist->item_h;

    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    font_draw_text(x << 6, (y + (int)(h - (font_line_height() >> 6)) / 2) << 6, w << 6, ((DkWordsView *)&dk_deck)->words[i]);
    return font_text_clear();
}

/* the progress bar */
void *dk_draw_word_bar(void *list, int x, int y)
{
    void *gim = dk_res_get(0x8006);
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
    e = dk_word->progress * 0x9A / 1000 + 8;
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
void dk_word_input_done(unsigned short *text, int r)
{
    if (dk_word != 0 && dk_word->step == 0 && dk_word->word[0] != 0)
        dk_word->focus = 2;
}
