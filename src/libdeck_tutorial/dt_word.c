// @range 0x08818E94-0x0881A20C
// @asm 0x0881902C-0x08819488
// @asm 0x08819488-0x08819710
/* (The code of libdeck dk_word.c; the lesson sets the word instead of the system keyboard.) The word
   search (scene state 5): a window with the word, the last words used and an OK button;
   the word is typed with the name input (system keyboard); the search runs incrementally with a
   progress bar and the matches become the pane's list. */
#include "libdeck_tutorial.h"

typedef struct DtWordWin {             /* dt_word 0x095D2730, 0x54 bytes */
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
} DtWordWin;

/* the search word history: 8 words of 32 characters */
#define DK_WORDS ((unsigned short (*)[32])dt_profile)
typedef struct DtWordsView {           /* the same words seen from the dt_deck block (+0x116) */
    unsigned char pad[0x116];
    unsigned short words[8][32];
} DtWordsView;

extern DtWordWin *dt_word;

void *menu_draw_item(void *list, MenuWidget *m, int x, int y, int which, int lit);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
void *dt_draw_shade(void *list, unsigned short side, unsigned short tab, int show_tab);
void dt_layout_scroll(int keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
void dt_search_start(DtList *l, unsigned short *word);
unsigned short dt_search_step(DtList *l);
void dt_search_finish(DtList *l);
void dt_popup_open(int id, int arg);
void dt_word_search(void);
void dt_word_free(void);
int dt_word_hist_open(void);
void dt_word_bar_open(void);
void *dt_draw_word_win(void *list, int x, int y);
void *dt_draw_hist_item(void *list, unsigned short i, int x, int y);
void *dt_draw_word_bar(void *list, int x, int y);

/* opens the word search window */
void dt_word_open(void)
{
    MenuItemDef *def;
    MenuWidget *m;

    dt_word = heap_alloc(dts.heap, 0x54);
    if (dt_word == 0)
        return;
    memset(dt_word, 0, 0x54);
    dt_word->menu = heap_alloc(dts.heap, 0x120);
    m = dt_word->menu;
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
    def->title = table_get_entry(dt_eres_get(0x8033), 0x12);
    def->title_font_size = 0xC;
    def->title_color = 0xFFFFFFFF;
    def->text = table_get_entry(dt_eres_get(0x8033), 0x42);
    def->font_flag4 = 1;
    def->font_size = 0xC;
    def->text_color = 0xFF000000;
    def->open_anim = 1;
    def->close_anim = 1;
    m->heap = dts.heap;
    m->no_input = 1;
    m->draw_all = (MenuDrawRectFn)dt_draw_word_win;
    menu_open(m);
    dt_word_hist_open();
    dts.state = 5;
}

#ifdef NON_MATCHING /* máquina de passos (dk_word_update sem o teclado do sistema); testes e alocação diferentes */
/* scene state 5 */
void dt_word_update(PadState *pad)
{
    MenuWidget *m = dt_word->menu;
    MenuWidget *hist = dt_word->hist;
    unsigned char c;
    int r;

    switch (dt_word->step) {
    case 0:
        if (pad->b.trig & region_pick_value(0)) {
            if (dt_word->focus == 2) {
                menu_set_cursor(m, 0);
                se_play(0x6213);
                dt_word->step = 2;
            }
        } else if (pad->b.trig & region_pick_value(1)) {
            m->cancelled = 1;
            menu_finish(m);
            se_play(0x6214);
            if (hist != 0)
                menu_finish(hist);
            dt_word->step = 3;
        } else {
            if ((pad->b.rep & 0x80) && dt_word->focus != 0) {
                dt_word->focus = 0;
                se_play(0x6212);
            }
            if ((pad->b.rep & 0x20) && dt_word->focus != 2) {
                dt_word->focus = 2;
                se_play(0x6212);
            }
            if (dt_word->focus == 0 && (pad->b.rep & 0x40) && hist != 0) {
                dt_word->step = 1;
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
                dt_word->step = 0;
            } else {
                if (c != 0)
                    return;
                if (!(pad->b.rep & 0x10))
                    return;
                dt_word->step = 0;
                se_play(0x6212);
            }
        } else {
            wcscpy(dt_word->word, DK_WORDS[r]);
            dt_word->step = 0;
            dt_word->focus = 2;
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
            dt_word->step = 3;
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
                dt_word->menu = 0;
                dt_word->hist = 0;
                dt_word->step = 4;
                dt_word->sstep = 0;
            } else {
                dt_word_free();
                dts.state = 0;
            }
        }
        break;
    case 4:
        dt_word_search();
        break;
    }
}
#endif

#ifdef NON_MATCHING /* passos da busca com outra ordem de blocos */
/* the search itself, 16 entries a frame */
void dt_word_search(void)
{
    DtPane *p = &dtv.pane[dtv.side];
    DtList *l = p->list;

    switch (dt_word->sstep) {
    case 0:
        if (dt_word->word[0] == 0) {
            l->filter = 8;
            dt_word->sstep = 10;
        } else if (l->count <= 0) {
            dt_search_start(l, dt_word->word);
            dt_search_finish(l);
            dt_word->sstep = 10;
        } else {
            dt_word->progress = 0;
            dt_search_start(l, dt_word->word);
            dt_word_bar_open();
            dt_word->sstep++;
        }
        return;
    case 1:
        menu_update(dt_word->bar);
        if (dt_word->bar->state == 3)
            dt_word->sstep++;
        return;
    case 2:
        dt_word->progress = dt_search_step(l);
        if (dt_word->progress >= 1000)
            dt_word->sstep++;
        return;
    case 3:
        dt_search_finish(l);
        dt_layout_scroll(0, 0, (p->list_mode == 0 ? p->lay.cols : 1) & 0xFF | (p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows) << 8,
                         p->list->count, &p->top, &p->cx, &p->cy);
        menu_finish(dt_word->bar);
        dt_word->sstep++;
        return;
    case 4:
        menu_update(dt_word->bar);
        if (dt_word->bar->state != 6)
            return;
        break;
    case 10:
        break;
    default:
        return;
    }
    dt_word_free();
    dt_popup_open(6, l->count);
    se_play(0x621A);
    dts.state = 0;
}
#endif

void dt_word_free(void)
{
    if (dt_word->bar != 0) {
        menu_reset(dt_word->bar);
        heap_set_level(dts.heap, dt_word->bar);
    }
    if (dt_word->hist != 0) {
        menu_reset(dt_word->hist);
        heap_set_level(dts.heap, dt_word->hist);
    }
    if (dt_word->menu != 0) {
        menu_reset(dt_word->menu);
        heap_set_level(dts.heap, dt_word->menu);
    }
    heap_set_level(dts.heap, dt_word);
    dt_word = 0;
}

/* the history list under the word (when there are words) */
int dt_word_hist_open(void)
{
    int n = 0;
    int i;
    MenuItemDef *def;
    MenuWidget *m;
    int y;

    for (i = 0; i < 8; i++) {
        if (((DtWordsView *)&dts)->words[i][0] == 0)
            break;
        n++;
    }
    if (n == 0)
        return 0;
    m = dt_word->hist;
    if (m == 0) {
        dt_word->hist = heap_alloc(dts.heap, 0x120);
        m = dt_word->hist;
        if (m == 0)
            return 0;
    }
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    y = menu_cursor_y(&dt_word->menu->menu, &dt_word->menu->def);
    def->x = dt_word->menu->def.x + 6;
    def->y = y + 0x11;
    def->auto_size = 1;
    def->color = 0xFFFFFFFF;
    def->pad_l = 1;
    def->pad_t = 3;
    def->pad_r = 1;
    def->pad_b = 1;
    def->open_anim = 1;
    def->close_anim = 1;
    m->heap = dts.heap;
    m->no_cancel = 0;
    m->no_wrap = 1;
    m->keep_open = 1;
    m->style = 2;
    m->item_w = 0xB0;
    m->item_h = 0xE;
    m->count = n;
    m->cursor = n;
    m->draw_item = (MenuDrawItemFn)dt_draw_hist_item;
    menu_open(m);
    return 1;
}

/* the progress bar window */
void dt_word_bar_open(void)
{
    MenuItemDef *def;
    MenuWidget *m = dt_word->bar;

    if (m == 0) {
        dt_word->bar = heap_alloc(dts.heap, 0x120);
        m = dt_word->bar;
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
    def->text = table_get_entry(dt_eres_get(0x8033), 0x45);
    def->open_anim = 2;
    def->close_anim = 2;
    m->heap = dts.heap;
    m->no_input = 1;
    m->no_cancel = 1;
    m->count = 0;
    m->draw_all = (MenuDrawRectFn)dt_draw_word_bar;
    menu_open(m);
}

void *dt_draw_word(void *list)
{
    unsigned int side;

    if (dtv.mode > 1) {
        side = (dtv.side + 1) & 1;
        list = dt_draw_shade(list, side, dtv.pane[side].list->tab, 1);
    }
    if (dt_word->menu != 0)
        menu_draw(&list, dt_word->menu);
    if (dt_word->bar != 0)
        menu_draw(&list, dt_word->bar);
    return list;
}

/* the word window contents: history, word field (blinking when focused), OK button */
void *dt_draw_word_win(void *list, int x, int y)
{
    MenuWidget *m = dt_word->menu;
    int v = 0;
    void *gim;
    unsigned short *label;
    int tx;

    if (dt_word->hist != 0) {
        menu_draw(&list, dt_word->hist);
        if (dt_word->focus != 2 && (gfx_get_state()[0] >> 3 & 1))
            v += 0x11;
    }
    gim = dt_eres_get(0x8006);
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
    if (dt_word->step == 0 && dt_word->focus == 0)
        list = menu_draw_cursor(list, dt_word->menu, x + 2, y + 7, 0xB7, 0xD);
    if (dt_word->word[0] != 0) {
        font_text_set(list);
        font_set_size(12, 12);
        font_set_color(0xFFFFFFFF);
        font_draw_text((x + 4) << 6, (y + 7) << 6, 0x2D40, dt_word->word);
        list = font_text_clear();
    }
    label = table_get_entry(dt_eres_get(0x8033), 0x17);
    tx = x + 0xD6;
    list = menu_draw_button(list, m, tx, y, 0, dt_word->focus == 2);
    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    tx += (0x3C - (font_measure_text(label) >> 6)) / 2;
    font_draw_text(tx << 6, (y + (0x18 - (font_line_height() >> 6)) / 2) << 6, (480 - tx) << 6, label);
    list = font_text_clear();
    return list;
}

void *dt_draw_hist_item(void *list, unsigned short i, int x, int y)
{
    unsigned int w = dt_word->hist->item_w;
    unsigned int h = dt_word->hist->item_h;

    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    font_draw_text(x << 6, (y + (int)(h - (font_line_height() >> 6)) / 2) << 6, w << 6, ((DtWordsView *)&dts)->words[i]);
    return font_text_clear();
}

/* the progress bar */
void *dt_draw_word_bar(void *list, int x, int y)
{
    void *gim = dt_eres_get(0x8006);
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
    e = dt_word->progress * 0x9A / 1000 + 8;
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
/* the lesson types the word itself */
void dt_word_set(unsigned short *text)
{
    if (dt_word != 0)
        wcscpy(dt_word->word, text);
}
