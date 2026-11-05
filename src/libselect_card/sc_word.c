// @range 0x08813CF4-0x08815168
// @data 0x0881B510-0x0881B51C
/* The word search (state 6): a window with the word, the last words used and an OK button; the
   word is typed in the system on-screen keyboard; the search runs incrementally with a progress
   bar and the matches become the pane's list. */
#include "libselect_card.h"
#include "menu.h"

typedef struct ScWordWin {             /* sc_word 0x0881B9A0, 0x54 bytes */
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
} ScWordWin;

typedef struct ScOskData {             /* SceUtilityOskData */
    int unk00;
    int unk04;
    int language;
    int unk12;
    int inputtype;
    int lines;
    int unk24;
    unsigned short *desc;
    unsigned short *intext;
    int outtextlength;
    unsigned short *outtext;
    int result;
    int outtextlimit;
} ScOskData;

typedef struct ScOskParams {           /* SceUtilityOskParams */
    int size;
    int language;
    int button_swap;
    int graphics_thread;
    int access_thread;
    int font_thread;
    int sound_thread;
    int result;
    int reserved[4];
    int datacount;
    ScOskData *data;
    int state;
    int unk60;
} ScOskParams;

typedef struct ScOsk {                 /* sc_osk 0x0881B9A4, 0x1B0 bytes */
    unsigned char state;               /* +0x00 0 idle, 1 fade in, 2 open, 3 fade out, 4 done */
    unsigned char pad1[3];
    ScOskParams params;                /* +0x04 */
    ScOskData data;                    /* +0x44 */
    unsigned short intext[24];         /* +0x78 */
    unsigned short outtext[128];       /* +0xA8 */
    unsigned short *target;            /* +0x1A8 */
    void (*done)(unsigned short *text, int r); /* +0x1AC */
} ScOsk;

extern ScWordWin *sc_word;
extern ScOsk sc_osk;

void sysvar_set_a90(int v);
int sceUtilityOskInitStart(void *params);
int wcslen(const unsigned short *s);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
void *sc_draw_shade(void *list, unsigned short side, unsigned short tab);
void sc_layout_scroll(short keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
void sc_search_start(ScList *l, unsigned short *word);
unsigned short sc_search_step(ScList *l);
void sc_search_finish(ScList *l);
void sc_popup_open(int id, int arg);
void sc_word_search(void);
void sc_word_free(void);
int sc_word_reset(void);
void sc_word_bar_open(void);
int sc_osk_open(unsigned short *target, unsigned short *desc, int len, void (*done)(unsigned short *, int));
void sc_osk_done(unsigned short *text, int r);
void *sc_draw_hist_item(void *list, unsigned short i, int x, int y);
void *sc_draw_word_bar(void *list, int x, int y);

/* state 6 */
void sc_input_result(void)
{
    MenuWidget *m = sc_word->menu;
    MenuWidget *hist = sc_word->hist;
    PadState *pad = pad_get();
    int c;
    int r;

    switch (sc_word->step) {
    case 0:
        if (pad->b.trig & region_pick_value(0)) {
            switch (sc_word->focus) {
            case 0:
                if (sc_osk_open(sc_word->word, table_get_entry(res_get(0x8033), 0x19), 0x10, sc_osk_done) == 0) {
                    scv.busy = 1;
                    se_play(0x6213);
                }
                break;
            case 2:
                menu_set_cursor(sc_word->menu, 0);
                se_play(0x6213);
                sc_word->step = 2;
                break;
            }
        } else if (pad->b.trig & region_pick_value(1)) {
            m->cancelled = 1;
            menu_finish(m);
            se_play(0x6214);
            sc_word->step = 3;
        } else {
            if ((pad->b.rep & 0x80) && sc_word->focus != 0) {
                sc_word->focus = 0;
                se_play(0x6212);
            }
            if ((pad->b.rep & 0x20) && sc_word->focus != 2) {
                sc_word->focus = 2;
                se_play(0x6212);
            }
            if (sc_word->focus == 0 && (pad->b.rep & 0x40) && hist != 0) {
                sc_word->step = 1;
                hist->cursor = 0;
                hist->no_cancel = 1;
                se_play(0x6212);
            }
        }
        break;
    case 1:
        c = hist->cursor;
        r = menu_update(sc_word->hist);
        if (r >= 0) {
            wcscpy(sc_word->word, sc.words[r]);
            sc_word->step = 0;
            sc_word->focus = 2;
        } else if (r == -2) {
            sc_word->step = 0;
        } else {
            if (c != 0)
                return;
            if (!(pad->b.rep & 0x10))
                return;
            sc_word->step = 0;
            se_play(0x6212);
        }
        hist->cancelled = hist->accepted = 0;
        hist->no_cancel = 0;
        hist->cursor = hist->count;
        break;
    case 2:
        menu_update(sc_word->menu);
        if (sc_word->menu->state != 4) {
            menu_finish(m);
            sc_word->step = 3;
        }
        break;
    case 3:
        menu_update(m);
        if (m->state == 6) {
            if (m->cancelled) {
                sc_word_free();
                sc.state = 1;
            } else {
                menu_reset(m);
                if (hist != 0)
                    menu_reset(hist);
                sc_word->menu = 0;
                sc_word->hist = 0;
                sc_word->step = 4;
                sc_word->sstep = 0;
            }
        }
        break;
    case 4:
        sc_word_search();
        break;
    }
}

/* the search itself, a few entries a frame */
void sc_word_search(void)
{
    ScPane *p = &scv.pane[scv.side];
    ScList *l = p->list;
    unsigned short view;
    unsigned short cols;

    switch (sc_word->sstep) {
    case 0:
        if (sc_word->word[0] == 0) {
            l->filter = 8;
            sc_word->sstep = 10;
        } else if (l->count <= 0) {
            sc_search_start(l, sc_word->word);
            sc_search_finish(l);
            sc_word->sstep = 10;
        } else {
            sc_word->progress = 0;
            sc_search_start(l, sc_word->word);
            sc_word_bar_open();
            sc_word->sstep++;
        }
        break;
    case 1:
        menu_update(sc_word->bar);
        if (sc_word->bar->state == 3)
            sc_word->sstep++;
        break;
    case 2:
        sc_word->progress = sc_search_step(l);
        if (sc_word->progress >= 1000)
            sc_word->sstep++;
        break;
    case 3:
        sc_search_finish(l);
        view = (p->list_mode == 0) ? p->lay.grid_rows : p->lay.rows;
        cols = (p->list_mode == 0) ? p->lay.cols : 1;
        sc_layout_scroll(0, 0, (cols & 0xFF) | (view & 0xFF) << 8, p->list->count, &p->top, &p->cx, &p->cy);
        menu_finish(sc_word->bar);
        sc_word->sstep++;
        break;
    case 4:
        menu_update(sc_word->bar);
        if (sc_word->bar->state != 6)
            break;
    case 10:
        sc_word_free();
        sc_popup_open(4, l->count);
        se_play(0x621A);
        sc.state = 1;
        break;
    }
}

void sc_word_free(void)
{
    if (sc_word->bar != 0) {
        menu_reset(sc_word->bar);
        heap_set_level(sc.heap, sc_word->bar);
    }
    if (sc_word->hist != 0) {
        menu_reset(sc_word->hist);
        heap_set_level(sc.heap, sc_word->hist);
    }
    if (sc_word->menu != 0) {
        menu_reset(sc_word->menu);
        heap_set_level(sc.heap, sc_word->menu);
    }
    heap_set_level(sc.heap, sc_word);
    sc_word = 0;
}

/* the history list under the word (when there are words) */
int sc_word_reset(void)
{
    int n = 0;
    int i;
    MenuWidget *m;
    int y;
    MenuItemDef *def;

    for (i = 0; i < 8; i++) {
        if (sc.words[i][0] == 0)
            break;
        n++;
    }
    if (n == 0)
        return 0;
    m = sc_word->hist;
    if (m == 0) {
        sc_word->hist = heap_alloc(sc.heap, 0x120);
        m = sc_word->hist;
        if (m == 0)
            return 0;
    }
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    y = menu_cursor_y(&sc_word->menu->menu, &sc_word->menu->def);
    def->x = sc_word->menu->def.x + 6;
    def->y = y + 0xF;
    def->auto_size = 1;
    def->color = 0xFFFFFFFF;
    def->pad_l = 1;
    def->pad_t = 3;
    def->pad_r = 1;
    def->pad_b = 1;
    def->open_anim = 1;
    def->close_anim = 1;
    m->heap = sc.heap;
    m->no_cancel = 1;
    m->no_wrap = 1;
    m->keep_open = 1;
    m->style = 2;
    m->item_w = 0xB0;
    m->item_h = 0xE;
    m->count = n;
    m->cursor = n;
    m->draw_item = (MenuDrawItemFn)sc_draw_hist_item;
    menu_open(m);
    return 1;
}

/* the progress bar window */
void sc_word_bar_open(void)
{
    MenuItemDef *def;
    MenuWidget *m = sc_word->bar;

    if (m == 0) {
        sc_word->bar = heap_alloc(sc.heap, 0x120);
        m = sc_word->bar;
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
    m->heap = sc.heap;
    m->no_input = 1;
    m->no_cancel = 1;
    m->count = 0;
    m->draw_all = (MenuDrawRectFn)sc_draw_word_bar;
    menu_open(m);
}

void *sc_draw_result(void *list)
{
    unsigned int side;

    if (scv.mode > 1) {
        side = (scv.side + 1) & 1;
        list = sc_draw_shade(list, side, scv.pane[side].list->tab);
    }
    if (sc_word->menu != 0)
        menu_draw(&list, sc_word->menu);
    if (sc_word->bar != 0)
        menu_draw(&list, sc_word->bar);
    return list;
}

/* the word window contents: history, word field (blinking when focused), OK button */
void *sc_draw_word_win(void *list, int x, int y)
{
    MenuWidget *m = sc_word->menu;
    int v = 0;
    void *gim;
    unsigned short *label;
    int tx;

    if (sc_word->hist != 0) {
        menu_draw(&list, sc_word->hist);
        if (sc_word->focus != 2 && (gfx_get_state()[0] >> 3 & 1))
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
    if (sc_word->step == 0 && sc_word->focus == 0)
        list = menu_draw_cursor(list, sc_word->menu, x + 2, y + 6, 0xB7, 0xD);
    if (sc_word->word[0] != 0) {
        font_text_set(list);
        font_set_size(12, 12);
        font_set_color(0xFFFFFFFF);
        font_draw_text((x + 4) << 6, (y + 7) << 6, 0x2D40, sc_word->word);
        list = font_text_clear();
    }
    label = table_get_entry(res_get(0x8033), 0x17);
    tx = x + 0xD6;
    list = menu_draw_button(list, m, tx, y, 0, sc_word->focus == 2);
    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    tx += (0x3C - (font_measure_text(label) >> 6)) / 2;
    font_draw_text(tx << 6, (y + (0x18 - (font_line_height() >> 6)) / 2) << 6, (480 - tx) << 6, label);
    list = font_text_clear();
    return list;
}

void *sc_draw_hist_item(void *list, unsigned short i, int x, int y)
{
    unsigned int w = sc_word->hist->item_w;
    unsigned int h = sc_word->hist->item_h;

    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    font_draw_text(x << 6, (y + (int)(h - (font_line_height() >> 6)) / 2) << 6, w << 6, sc.words[i]);
    return font_text_clear();
}

/* the progress bar */
void *sc_draw_word_bar(void *list, int x, int y)
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
    e = sc_word->progress * 0x9A / 1000 + 8;
    list = draw_set_offsets(list, 4, 7, 4, 7);
    list = draw_sprite(list, x + e - 4, y + 8, 0, 0x3A, 0xF8, 0, 0);
    list = draw_sprite(list, x + 1, y + 8, 0, 0x20, 0xF8, 0, 0);
    if (e > 8) {
        list = draw_set_offsets(list, 4, 7, e - 8, 7);
        list = draw_sprite(list, x + 4, y + 8, 0, 0x24, 0xF8, 0, 0);
    }
    return list;
}

/* the keyboard closed: a non-empty word moves the focus to OK */
void sc_osk_done(unsigned short *text, int r)
{
    if (sc_word != 0 && sc_word->step == 0 && sc_word->word[0] != 0)
        sc_word->focus = 2;
}

/* OSK language of each game language */
unsigned short sc_osk_lang[6] = { 0, 1, 4, 2, 5, 3 };

/* starts the system on-screen keyboard on target ('$' doubled), len characters at most */
int sc_osk_open(unsigned short *text, unsigned short *desc, int maxlen, void (*done)(unsigned short *, int))
{
    unsigned short *in = sc_osk.intext;
    int lang = cfg_get_language();
    int dollar = 0;
    unsigned int i;
    unsigned short *s;
    unsigned short *o;
    ScOskParams *p;
    ScOskData *d;
    int r;

    if (lang < 0 || lang >= 6)
        lang = 1;
    memset(&sc_osk, 0, sizeof(sc_osk));
    sysvar_set_a90(1);
    sc_osk.target = text;
    sc_osk.done = done;
    for (i = 0, s = text, o = in; i < wcslen(text) && (int)i < 23; s++, i++) {
        unsigned short c = *s;
        if (c == '$') {
            if (dollar) {
                dollar = 0;
                continue;
            }
            dollar = 1;
        }
        *o = c;
        o++;
        if (c == 0)
            break;
    }
    p = (ScOskParams *)(char *)&sc_osk.params;
    p->size = 0x40;
    p->language = sc_osk_lang[lang];
    d = (ScOskData *)(char *)&sc_osk.data;
    if (region_pick_value(0) == 0x2000)
        p->button_swap = 0;
    else
        p->button_swap = 1;
    p->graphics_thread = 0x11;
    p->access_thread = 0x13;
    p->font_thread = 0x12;
    p->sound_thread = 0x10;
    p->reserved[0] = 0;
    p->reserved[1] = 0;
    p->data = d;
    p->datacount = 1;
    if (lang == 0)
        d->unk00 = 1;
    else
        d->unk00 = 0;
    d->lines = 1;
    d->unk24 = 1;
    d->outtext = sc_osk.outtext;
    d->desc = desc;
    d->intext = in;
    d->outtextlength = maxlen;
    d->unk04 = 0;
    d->language = 0;
    d->unk12 = 0;
    d->inputtype = 0;
    d->result = 0;
    d->outtextlimit = 0;
    r = sceUtilityOskInitStart(p);
    if (r == 0) {
        fade_start_in(2, 0xA0000000, 8);
        sc_osk.state = 1;
    }
    return r;
}
