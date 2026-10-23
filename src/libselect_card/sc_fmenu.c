// @range 0x088121D0-0x08813CF4
// @asm 0x088121D0-0x08812464
// @asm 0x08812464-0x088125E4
// @asm 0x088125E4-0x08812CB0
// @asm 0x08812E04-0x088130A4
// @asm 0x088130A4-0x08813584
// @asm 0x08813778-0x0881386C
// @asm 0x08813A20-0x08813B5C
// @asm 0x08813B5C-0x08813CF4
// @data 0x0881B460-0x0881B510
/* The filter menu (state 5): a list of the seven filter kinds and a grid of the values of the
   chosen one (toggled with Circle, page by page), then the filter applied to the pane; the value
   icons and labels; the opening of the word search window. */
#include "libselect_card.h"
#include "menu.h"

typedef struct ScButton {
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} ScButton;

typedef void *(*ScValueDraw)(void *list, unsigned short x, unsigned short y, unsigned short v);

typedef struct ScFilterDef {           /* values of a filter kind */
    unsigned short first;
    unsigned short last;
    ScValueDraw draw;                  /* +0x04 */
} ScFilterDef;

typedef struct ScFilterMenu {          /* sc_fmenu 0x0881B99C, 0x18 bytes */
    unsigned short step;               /* 0 kinds, 1 values, 2 closing */
    unsigned char sub;                 /* +0x02 step inside the values window */
    unsigned char alt : 1;             /* +0x03 some card has an alternate picture */
    unsigned char col : 4;             /*       cursor column */
    unsigned char pad03 : 3;
    unsigned char page : 4;            /* +0x04 cursor row, 0xF = the OK button */
    unsigned char pad04 : 4;
    unsigned char pad05[3];
    unsigned int mask[2];              /* +0x08 values picked */
    MenuWidget *kinds;                 /* +0x10 */
    MenuWidget *vals;                  /* +0x14 */
} ScFilterMenu;

typedef struct ScWordWin {             /* sc_word 0x0881B9A0, 0x54 bytes */
    unsigned char pad[0x48];
    MenuWidget *menu;                  /* +0x48 */
    unsigned char pad4c[8];
} ScWordWin;

extern ScFilterMenu *sc_fmenu;
extern ScWordWin *sc_word;

void *sc_draw_button(void *list, ScButton *b, int x, int y, unsigned int color);
void *sc_draw_shade(void *list, unsigned short side, unsigned short tab);
void *sc_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind);
void *sc_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type);
void *sc_draw_limit_icon(void *list, unsigned short x, unsigned short y, unsigned short n);
void *sc_draw_pack_icon(void *list, unsigned short x, unsigned short y, unsigned short n);
void sc_layout_scroll(short keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
unsigned short sc_list_filter(ScList *l, unsigned short f, unsigned int *arg);
void sc_popup_open(int id, int arg);
void *menu_draw_item(void *list, MenuWidget *m, int x, int y, int which, int lit);
void sc_fmenu_free(void);
void sc_fmenu_values(void);
void sc_fmenu_open_values(unsigned short f);
void *sc_draw_fmenu_values(void *list, int x, int y, int w, int h);
void *sc_draw_fmenu_kind(void *list, int i, int x, int y);
void *sc_draw_check(void *list, unsigned short x, unsigned short y);
unsigned short *sc_filter_label(unsigned short f, unsigned short v);
void *sc_draw_word_win(void *list, int x, int y, int w, int h);
void sc_word_reset(void);

/* filter kind icons and labels */
ScButton sc_filter_icons[9] = { { 0x8000, 0x00, 0x00, 0x00 }, { 0x8008, 0x44, 0x20, 0x07 }, { 0x8008, 0x66, 0x20, 0x0B }, { 0x8008, 0x55, 0x20, 0x0C }, { 0x8008, 0x33, 0x20, 0x08 }, { 0x800B, 0x00, 0x33, 0x2A }, { 0x8008, 0x11, 0x31, 0x10 }, { 0x800B, 0x44, 0x22, 0x2B }, { 0x8008, 0x66, 0x31, 0x12 } };

#ifdef NON_MATCHING /* gravações dos campos do menu em outra ordem */
/* opens the filter menu (kinds list) */
void sc_input_filter(void)
{
    ScList *l = scv.pane[scv.side].list;
    MenuWidget *m;
    int id;
    int f;

    sc_fmenu = heap_alloc(sc.heap, 0x18);
    if (sc_fmenu == 0)
        return;
    memset(sc_fmenu, 0, 0x18);
    sc_fmenu->kinds = heap_alloc(sc.heap, 0x120);
    m = sc_fmenu->kinds;
    if (m != 0) {
        memset(m, 0, 0x120);
        m->def.visible = 1;
        m->def.style = 0;
        m->def.y = 0x3A;
        m->def.pad_t = 2;
        m->def.pad_b = 2;
        m->def.pad_l = 2;
        m->def.pad_r = 2;
        m->def.auto_size = 1;
        m->def.color = 0xFFFFFFFF;
        m->def.open_anim = 2;
        m->def.close_anim = 2;
        m->def.show_title = 1;
        m->def.auto_title = 1;
        m->def.font_flag4 = 1;
        m->def.style2 = 2;
        m->def.title = table_get_entry(res_get(0x8033), 0x56);
        m->def.title_font_size = 0xC;
        m->def.title_color = 0xFFFFFFFF;
        m->heap = sc.heap;
        m->keep_open = 1;
        m->no_cancel = 1;
        m->style = 2;
        m->count = 7;
        m->item_w = 0xA4;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)sc_draw_fmenu_kind;
        menu_open(m);
        m->def.x = (0x1E0 - m->def.w) / 2;
    }
    for (id = 0xFA7; id < 0x1B2E; id++) {
        if (card_get_flag31(id) && inv_get_bit3(id)) {
            sc_fmenu->alt = 1;
            break;
        }
    }
    f = l->filter - 1;
    if (f < 0)
        f = 0;
    else if (f > 6)
        f = 0;
    sc_fmenu->kinds->cursor = f;
    sc.state = 5;
}
#endif

#ifdef NON_MATCHING /* testes do passo e zeramentos com outra ordem */
/* state 5 */
void sc_input_confirm(void)
{
    int r;

    pad_get();
    switch (sc_fmenu->step) {
    case 0:
        r = menu_update(sc_fmenu->kinds);
        if (r == -1)
            break;
        if (r == -2) {
            menu_finish(sc_fmenu->kinds);
            sc_fmenu->step = 2;
        } else {
            sc_fmenu->kinds->no_cancel = 0;
            sc_fmenu_open_values(r + 1);
            se_play(0x6216);
            sc_fmenu->page = 0;
            sc_fmenu->col = 0;
            sc_fmenu->mask[0] = 0;
            sc_fmenu->mask[1] = 0;
            sc_fmenu->sub = 0;
            sc_fmenu->step = 1;
        }
        break;
    case 1:
        sc_fmenu_values();
        break;
    case 2:
        if (menu_update(sc_fmenu->kinds) != -1) {
            sc_fmenu_free();
            sc.state = 1;
        }
        break;
    }
}
#endif

void *sc_draw_flag_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *sc_draw_spell_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *sc_draw_genre_value(void *list, unsigned short x, unsigned short y, unsigned short v);

ScFilterDef sc_filter_defs[9] = {
    { 0, 0, 0 },
    { 0, 5, (ScValueDraw)sc_draw_limit_icon },
    { 1, 20, (ScValueDraw)sc_draw_type_icon },
    { 1, 6, (ScValueDraw)sc_draw_kind_icon },
    { 0, 2, (ScValueDraw)sc_draw_flag_value },
    { 1, 9, (ScValueDraw)sc_draw_spell_value },
    { 0, 3, (ScValueDraw)sc_draw_pack_icon },
    { 0, 35, (ScValueDraw)sc_draw_genre_value },
};

#ifdef NON_MATCHING /* máquina de passos grande; alocação e blocos diferentes */
/* the values window of filter kind (cursor of the kinds list) + 1 */
void sc_fmenu_values(void)
{
    MenuWidget *m = sc_fmenu->vals;
    PadState *pad = pad_get();
    unsigned int f = sc_fmenu->kinds->cursor + 1;
    unsigned int side;
    ScList *l;
    int n;
    unsigned int cols;
    int rows;
    unsigned int old;
    unsigned int v;
    unsigned short view;
    unsigned short grid;

    switch (sc_fmenu->sub) {
    case 3:
        menu_update(m);
        if (m->state == 6) {
            menu_reset(m);
            sc_fmenu->vals = 0;
            sc_fmenu->kinds->accepted = 0;
            sc_fmenu->kinds->cancelled = 0;
            sc_fmenu->kinds->no_cancel = 1;
            sc_fmenu->step = 0;
        }
        break;
    case 2:
        menu_update(m);
        if (m->state == 6) {
            side = scv.side;
            l = scv.pane[side].list;
            if (l->count == 0) {
                l->filter = f;
            } else {
                sc_list_filter(l, f, sc_fmenu->mask);
                view = scv.pane[side].list_mode == 0 ? scv.pane[side].lay.grid_rows : scv.pane[side].lay.rows;
                grid = scv.pane[side].list_mode == 0 ? scv.pane[side].lay.cols : 1;
                sc_layout_scroll(0, 0, (grid & 0xFF) | view << 8, scv.pane[side].list->count, &scv.pane[side].top,
                                 &scv.pane[side].cx, &scv.pane[side].cy);
            }
            sc_fmenu_free();
            sc_popup_open(4, l->count);
            se_play(0x621A);
            sc.state = 1;
        }
        break;
    case 1:
        menu_update(sc_fmenu->vals);
        if (sc_fmenu->vals->state != 4) {
            menu_finish(m);
            menu_finish(sc_fmenu->kinds);
            sc_fmenu->sub = 2;
        }
        break;
    case 0:
        if (pad->b.trig & region_pick_value(0)) {
            if (sc_fmenu->page == 0xF) {
                menu_set_cursor(sc_fmenu->vals, 0);
                sc_fmenu->sub = 1;
            } else {
                v = sc_filter_defs[f].first + sc_fmenu->col + sc_fmenu->page * m->count;
                sc_fmenu->mask[(int)v >> 5] ^= 1 << (v & 0x1F);
            }
            se_play(0x6213);
            break;
        }
        if (pad->b.trig & region_pick_value(1)) {
            m->cancelled = 1;
            menu_finish(m);
            sc_fmenu->sub = 3;
            se_play(0x6214);
            break;
        }
        n = sc_filter_defs[f].last - sc_filter_defs[f].first + 1;
        if (!sc_fmenu->alt && f == 7)
            n--;
        cols = m->count;
        rows = (int)(n + cols - 1) / (int)cols;
        if (pad->b.rep & 0x10) {
            old = sc_fmenu->page;
            if (old == 0) {
                sc_fmenu->page = 0xF;
                se_play(0x6212);
            } else {
                if (old == 0xF)
                    sc_fmenu->page = rows - 1;
                else
                    sc_fmenu->page = old - 1;
                if ((int)(sc_fmenu->col + sc_fmenu->page * cols) >= n)
                    sc_fmenu->page = sc_fmenu->page - 1;
                if (old != sc_fmenu->page)
                    se_play(0x6212);
            }
        }
        if (pad->b.rep & 0x40) {
            old = sc_fmenu->page;
            if (old == 0xF)
                sc_fmenu->page = 0;
            else
                sc_fmenu->page = old + 1;
            if ((int)(sc_fmenu->col + sc_fmenu->page * cols) >= n)
                sc_fmenu->page = 0xF;
            if (old != sc_fmenu->page)
                se_play(0x6212);
        }
        if (sc_fmenu->page == rows - 1u) {
            cols = n % (int)m->count;
            if (cols == 0)
                cols = m->count;
        }
        if (sc_fmenu->page != 0xF) {
            old = sc_fmenu->col;
            if (pad->b.rep & 0x80)
                sc_fmenu->col = (int)(old + cols - 1) % (int)cols;
            if (pad->b.rep & 0x20)
                sc_fmenu->col = (int)(sc_fmenu->col + 1) % (int)cols;
            if (old != sc_fmenu->col)
                se_play(0x6212);
        }
        break;
    }
}
#endif

void sc_fmenu_free(void)
{
    if (sc_fmenu->vals != 0) {
        menu_reset(sc_fmenu->vals);
        heap_set_level(sc.heap, sc_fmenu->vals);
    }
    menu_reset(sc_fmenu->kinds);
    heap_set_level(sc.heap, sc_fmenu->kinds);
    heap_set_level(sc.heap, sc_fmenu);
    sc_fmenu = 0;
}

void *sc_draw_confirm(void *list)
{
    unsigned int side;

    if (scv.mode > 1) {
        side = (scv.side + 1) & 1;
        list = sc_draw_shade(list, side, scv.pane[side].list->tab);
    }
    menu_draw(&list, sc_fmenu->kinds);
    if (sc_fmenu->vals != 0)
        menu_draw(&list, sc_fmenu->vals);
    return list;
}


void *sc_draw_fmenu_kind(void *list, int i, int x, int y)
{
    return sc_draw_button(list, &sc_filter_icons[i + 1], x, y + 1, 0);
}

#ifdef NON_MATCHING /* gravações dos campos do menu em outra ordem */
/* the values window of filter f */
void sc_fmenu_open_values(unsigned short f)
{
    int x = (0x1E0 - sc_fmenu->kinds->def.w) / 2;
    MenuWidget *m = sc_fmenu->vals;
    int n;

    if (m == 0) {
        sc_fmenu->vals = heap_alloc(sc.heap, 0x120);
        m = sc_fmenu->vals;
        if (m == 0)
            return;
    }
    n = sc_filter_defs[f].last - sc_filter_defs[f].first + 1;
    memset(m, 0, 0x120);
    m->def.visible = 1;
    m->def.style = 0;
    m->def.x = x + 0x6D;
    m->def.y = sc_fmenu->kinds->def.y + 4 + f * 0x12;
    m->def.w = 0x8A;
    m->def.h = (sc_filter_defs[f].last - sc_filter_defs[f].first + 5) / 5 * 0x19 + 0x35;
    m->def.anchor = -1;
    m->def.pad_l = 2;
    m->def.pad_t = 0x12;
    m->def.pad_r = 2;
    m->def.pad_b = 2;
    m->def.open_anim = 1;
    m->def.close_anim = 1;
    m->heap = sc.heap;
    m->no_input = 1;
    m->no_cancel = 1;
    m->item_w = 0x19;
    m->item_h = 0x19;
    m->count = 5;
    m->draw_all = (MenuDrawRectFn)sc_draw_fmenu_values;
    if (f == 7) {
        m->def.w = 0xB7;
        m->def.h = 0xCB;
        m->def.pad_l += 10;
        m->def.pad_r += 10;
        m->def.y = sc_fmenu->kinds->def.y - 0x16;
        m->count = 6;
    } else if (f == 5) {
        m->def.w = 0xA3;
        m->count = 6;
    } else if (f == 3) {
        m->def.w = 0xA3;
        m->def.h = 0x4E;
        m->count = 6;
    } else if (f == 1) {
        m->count = 4;
    } else if (n < 5) {
        m->count = n;
    }
    menu_open(m);
}
#endif

#ifdef NON_MATCHING /* alocação diferente */
/* the values grid: title strip with the label of the value under the cursor, the values with a
   tick on the picked ones, and the OK button */
void *sc_draw_fmenu_values(void *list, int x, int y, int w, int h)
{
    MenuWidget *m = sc_fmenu->vals;
    unsigned int f = sc_fmenu->kinds->cursor + 1;
    ScFilterDef *d = &sc_filter_defs[f];
    int ox = (m->item_w - 0x11) / 2;
    int cur = sc_fmenu->col + sc_fmenu->page * m->count;
    int oy = (m->item_h - 0x11) / 2;
    int mw = m->def.w;
    int bw = mw - 0xD;
    int bx = x;
    void *gim;
    unsigned short *label;
    int n;
    int i;
    unsigned int v;
    unsigned int cx;
    unsigned int cy;
    int tx;
    int ty;

    if (w < bw)
        bx = x - (bw - w) / 2;
    gim = res_get(0x8006);
    if (gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 4, 0x11, 4, 0x11);
        list = draw_sprite(list, bx, y - 0x12, 0, 0xE0, 0, 0, 0);
        list = draw_sprite(list, bx + bw - 4, y - 0x12, 0, 0xE0, 0, 1, 0);
        list = draw_set_offsets(list, 4, 0x11, mw - 0x15, 0x11);
        list = draw_sprite(list, bx + 4, y - 0x12, 0, 0xE4, 0, 0, 0);
    }
    if (sc_fmenu->page == 0xF)
        cur = -1;
    if (cur >= 0 && (label = sc_filter_label(f, d->first + cur)) != 0) {
        font_set_size(12, 12);
        font_set_color(0xFFFFFFFF);
        font_text_set(list);
        font_draw_wtext((bx + 4) << 6, (y - 0xF) << 6, (mw - 0x15) << 6, label);
        list = font_text_clear();
    }
    n = d->last - d->first + 1;
    if (!sc_fmenu->alt && f == 7)
        n = d->last - d->first;
    for (i = 0; i < n; i++) {
        v = d->first + i;
        cx = x + i % m->count * m->item_w;
        cy = y + m->item_h * (i / m->count);
        if (sc_fmenu->mask[(int)v >> 5] & 1 << (v & 0x1F))
            list = sc_draw_check(list, cx, cy);
        if (cur == i)
            list = menu_draw_cursor(list, m, cx, cy, 0x18, 0x18);
        list = d->draw(list, cx + ox, cy + oy, v);
    }
    label = table_get_entry(res_get(0x8033), 0x17);
    tx = x + (w - 0x3C) / 2;
    ty = y + h - 0x18;
    list = menu_draw_item(list, m, tx, ty, 0, sc_fmenu->page == 0xF);
    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    tx += (0x3C - (font_measure_text(label) >> 6)) / 2;
    font_draw_text(tx << 6, (ty + (0x18 - (font_line_height() >> 6)) / 2) << 6, (480 - tx) << 6, label);
    return font_text_clear();
}
#endif

void *sc_draw_flag_value(void *list, unsigned short x, unsigned short y, unsigned short v)
{
    unsigned short u = v * 17;
    void *gim = res_get(0x800A);

    if (gim == 0)
        return list;
    {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 17, 17, 17, 17);
        return draw_sprite(list, x, y, 0, u, 0, 0, 0);
    }
    return list;
}

void *sc_draw_spell_value(void *list, unsigned short x, unsigned short y, unsigned short v)
{
    unsigned short u = v % 7 * 17;
    unsigned short tv = v / 7 * 17 + 0x33;
    void *gim = res_get(0x800B);

    if (gim == 0)
        return list;
    {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 17, 17, 17, 17);
        return draw_sprite(list, x, y, 0, u, tv, 0, 0);
    }
    return list;
}

#ifdef NON_MATCHING /* o original tem blocos mortos depois do desvio */
void *sc_draw_genre_value(void *list, unsigned short x, unsigned short y, unsigned short v)
{
    void *gim = res_get(0x800C);

    if (gim == 0)
        return list;
    {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 16, 16, 16, 16);
        return draw_sprite(list, x, y, 0, (v & 7) << 4, (v >> 3 & 0xFFF) << 4, 0, 0);
    }
    return list;
}
#endif


void *sc_draw_filter_icon(void *list, int filter, int x, int y)
{
    ScButton *b;
    void *gim;

    if (filter == 0)
        return list;
    {
        b = &sc_filter_icons[filter];
        gim = res_get(b->res);
        if (gim != 0) {
            list = dl_ambient_color(list, 0xFFFFFFFF);
            list = dl_set_tex_filter(list, 0, 0);
            list = gim_set_texture(list, gim);
            list = draw_set_offsets(list, 17, 17, 17, 17);
            list = draw_sprite(list, x, y, 0, b->u, b->v, 0, 0);
        }
    }
    return list;
}

void *sc_draw_check(void *list, unsigned short x, unsigned short y)
{
    void *gim = res_get(0x8006);

    if (gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 0x1A, 0x1A, 0x1A, 0x1A);
        list = draw_sprite(list, x, y, 0, 0x9A, 0xC0, 0, 0);
    }
    return list;
}

unsigned short sc_limit_labels[10] = { 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x0 };
unsigned short sc_spell_labels[14] = { 0x35, 0x36, 0x37, 0x38, 0x0, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41 };

#ifdef NON_MATCHING /* tabela de saltos com default primeiro; corpos em outra ordem */
/* label of value v of filter f */
unsigned short *sc_filter_label(unsigned short f, unsigned short v)
{
    switch (f) {
    default:
        return 0;
    case 1:
        return table_get_entry(res_get(0x8033), sc_limit_labels[v]);
    case 2:
        return card_db_table_sel(v);
    case 3:
        return dlg_table_sel(v);
    case 4:
        return table_get_entry(res_get(0x8033), sc_limit_labels[v + 6]);
    case 5:
        return table_get_entry(res_get(0x8033), sc_spell_labels[v + 4]);
    case 6:
        return table_get_entry(res_get(0x8033), sc_spell_labels[v]);
    case 7:
        return dlg_get_text_bounded(v);
    }
}
#endif

#ifdef NON_MATCHING /* gravações dos campos do menu em outra ordem */
/* opens the word search window */
void sc_input_mark(void)
{
    MenuWidget *m;

    sc_word = heap_alloc(sc.heap, 0x54);
    if (sc_word == 0)
        return;
    memset(sc_word, 0, 0x54);
    sc_word->menu = heap_alloc(sc.heap, 0x120);
    m = sc_word->menu;
    if (m == 0)
        return;
    memset(m, 0, 0x120);
    m->def.visible = 1;
    m->def.style = 0;
    m->def.x = 0x5E;
    m->def.y = 0x48;
    m->def.w = 0x124;
    m->def.h = 0x30;
    m->def.anchor = -1;
    m->def.pad_l = 2;
    m->def.pad_t = 2;
    m->def.pad_r = 2;
    m->def.pad_b = 2;
    m->def.font_flag4 = 1;
    m->def.font_flag8000 = 1;
    m->def.show_title = 1;
    m->def.style2 = 2;
    m->def.title = table_get_entry(res_get(0x8033), 0x12);
    m->def.title_font_size = 0xC;
    m->def.title_color = 0xFFFFFFFF;
    m->def.text = table_get_entry(res_get(0x8033), 0x42);
    m->def.show_title = 1;
    m->def.font_size = 0xC;
    m->def.text_color = 0xFF000000;
    m->def.open_anim = 1;
    m->def.close_anim = 1;
    m->heap = sc.heap;
    m->no_input = 1;
    m->draw_all = (MenuDrawRectFn)sc_draw_word_win;
    menu_open(m);
    sc_word_reset();
    sc.state = 6;
}
#endif
