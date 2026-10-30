// @range 0x08813CF4-0x08815168
// @asm 0x088144FC-0x088146E0
// @asm 0x088146E0-0x08814828
// @asm 0x088148D0-0x08814C14
// @asm 0x08814F40-0x08815168
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

typedef struct ScOsk {                 /* sc_osk 0x0881B9A4, 0x1B0 bytes */
    unsigned char state;               /* 0 idle, 1 fade in, 2 running, 3 fade out, 4 done */
    unsigned char pad01[3];
    unsigned int base[12];             /* +0x04 SceUtilityOskParams base */
    int datacount;                     /* +0x34 */
    unsigned int *data;                /* +0x38 */
    int pstate;                        /* +0x3C */
    int unk40;
    unsigned int odata[13];            /* +0x44 SceUtilityOskData */
    unsigned short in[0x18];           /* +0x78 */
    unsigned short out[0x80];          /* +0xA8 */
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
void *menu_draw_item(void *list, MenuWidget *m, int x, int y, int which, int lit);
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

#ifdef NON_MATCHING /* gravações dos campos do menu em outra ordem */
/* the history list under the word (when there are words) */
int sc_word_reset(void)
{
    int n = 0;
    int i;
    MenuWidget *m;
    short y;

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
    memset(m, 0, 0x120);
    m->def.visible = 1;
    m->def.style = 0;
    y = menu_cursor_y(&sc_word->menu->menu, &sc_word->menu->def);
    m->def.x = sc_word->menu->def.x + 6;
    m->def.y = y + 0xF;
    m->def.auto_size = 1;
    m->def.anchor = -1;
    m->def.pad_l = 1;
    m->def.pad_t = 3;
    m->def.pad_r = 1;
    m->def.pad_b = 1;
    m->def.open_anim = 1;
    m->def.close_anim = 1;
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
#endif

#ifdef NON_MATCHING /* gravações dos campos do menu em outra ordem */
/* the progress bar window */
void sc_word_bar_open(void)
{
    MenuWidget *m = sc_word->bar;

    if (m == 0) {
        sc_word->bar = heap_alloc(sc.heap, 0x120);
        m = sc_word->bar;
        if (m == 0)
            return;
    }
    memset(m, 0, 0x120);
    m->def.visible = 1;
    m->def.style = 2;
    m->def.x = 0x8A;
    m->def.y = 0x70;
    m->def.w = 0xCC;
    m->def.h = 0x30;
    m->def.anchor = -1;
    m->def.pad_l = 0xF;
    m->def.pad_t = 4;
    m->def.font_size = 0xC;
    m->def.text_color = 0xFFFFFFFF;
    m->def.text = table_get_entry(res_get(0x8033), 0x45);
    m->def.open_anim = 2;
    m->def.close_anim = 2;
    m->heap = sc.heap;
    m->no_input = 1;
    m->no_cancel = 1;
    m->count = 0;
    m->draw_all = (MenuDrawRectFn)sc_draw_word_bar;
    menu_open(m);
}
#endif

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

#ifdef NON_MATCHING /* o original relê o menu da estrutura e guarda x + 0xD6 em s0; quase igual */
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
            v = 0x11;
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
        list = menu_draw_cursor(list, m, x + 2, y + 6, 0xB7, 0xD);
    if (sc_word->word[0] != 0) {
        font_text_set(list);
        font_set_size(12, 12);
        font_set_color(0xFFFFFFFF);
        font_draw_text((x + 4) << 6, (y + 7) << 6, 0x2D40, sc_word->word);
        list = font_text_clear();
    }
    label = table_get_entry(res_get(0x8033), 0x17);
    list = menu_draw_item(list, m, x + 0xD6, y, 0, sc_word->focus == 2);
    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    tx = x + 0xD6 + (0x3C - (font_measure_text(label) >> 6)) / 2;
    font_draw_text(tx << 6, (y + (0x18 - (font_line_height() >> 6)) / 2) << 6, (480 - tx) << 6, label);
    return font_text_clear();
}
#endif

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

#ifdef NON_MATCHING /* lógica conferida com o asm; 63 palavras: o original grava os campos 0/1 (região, idioma)
                       com desvios, o mwcc gera sltu/sltiu; ordem das gravações dos parâmetros */
/* starts the system on-screen keyboard on target ('$' doubled), len characters at most */
int sc_osk_open(unsigned short *target, unsigned short *desc, int len, void (*done)(unsigned short *, int))
{
    unsigned short *in = sc_osk.in;
    int lang = cfg_get_language();
    int dollar = 0;
    unsigned int i;
    unsigned short *s;
    unsigned short *d;
    unsigned short c;
    unsigned int *b;
    unsigned int *o;
    int r;

    if (lang < 0 || lang >= 6)
        lang = 1;
    memset(&sc_osk, 0, 0x1B0);
    sysvar_set_a90(1);
    sc_osk.target = target;
    sc_osk.done = done;
    for (i = 0, s = target, d = in; i < wcslen(target) && (int)i < 0x17; i++, s++) {
        c = *s;
        if (c == '$') {
            if (dollar) {
                dollar = 0;
                continue;
            }
            dollar = 1;
        }
        *d++ = c;
        if (c == 0)
            break;
    }
    b = sc_osk.base;
    b[0] = 0x40;
    b[1] = sc_osk_lang[lang];
    o = sc_osk.odata;
    b[2] = (unsigned short)region_pick_value(0) != 0x2000 ? 1 : 0;
    b[3] = 0x11;
    b[4] = 0x13;
    b[5] = 0x12;
    b[6] = 0x10;
    b[8] = 0;
    b[9] = 0;
    sc_osk.data = o;
    sc_osk.datacount = 1;
    o[0] = lang == 0 ? 1 : 0;
    o[5] = 1;
    o[6] = 1;
    o[10] = (unsigned int)sc_osk.out;
    o[7] = (unsigned int)desc;
    o[8] = (unsigned int)in;
    o[9] = len;
    o[1] = 0;
    o[2] = 0;
    o[3] = 0;
    o[4] = 0;
    o[11] = 0;
    o[12] = 0;
    r = sceUtilityOskInitStart(sc_osk.base);
    if (r == 0) {
        fade_start_in(2, 0xA0000000, 8);
        sc_osk.state = 1;
    }
    return r;
}
#endif
