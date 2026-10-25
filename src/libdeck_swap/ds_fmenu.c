// @range 0x0880F3E0-0x08810D64
// @asm 0x0880F7EC-0x0880FEB8
// @asm 0x088102AC-0x0881078C
// @asm 0x08810980-0x08810A74
// @rodata 0x08819CBC-0x08819CDC
// @data 0x0881A224-0x0881A2D4
/* The filter menu (state 5): a list of the seven filter kinds and a grid of the values of the
   chosen one (toggled with Circle, page by page), then the filter applied to the pane; the value
   icons and labels (same source as libselect_card sc_fmenu.c and libdeck dk_fmenu.c). */
#include "libdeck_swap.h"
#include "menu.h"

typedef struct DsButton {
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} DsButton;

typedef void *(*DsValueDraw)(void *list, unsigned short x, unsigned short y, unsigned short v);

typedef struct DsFilterDef {           /* values of a filter kind */
    unsigned short first;
    unsigned short last;
    DsValueDraw draw;                  /* +0x04 */
} DsFilterDef;

typedef struct DsFilterMenu {          /* ds_fmenu 0x0881B99C, 0x18 bytes */
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
} DsFilterMenu;

extern DsFilterMenu *ds_fmenu;

void *ds_draw_button(void *list, DsButton *b, int x, int y, unsigned int color);
void *ds_draw_shade(void *list, unsigned short side, unsigned short tab);
void *ds_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind);
void *ds_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type);
void *ds_draw_limit_icon(void *list, unsigned short x, unsigned short y, unsigned short n);
void *ds_draw_pack_icon(void *list, unsigned short x, unsigned short y, unsigned short n);
void ds_layout_scroll(short keep, unsigned short count, unsigned short view, unsigned short total, unsigned short *top, unsigned short *cx, unsigned short *cy);
unsigned short ds_list_filter(DsList *l, unsigned short f, unsigned int *arg);
void ds_popup_open(int id, int arg);
void *menu_draw_item(void *list, MenuWidget *m, int x, int y, int which, int lit);
void ds_fmenu_free(void);
void ds_fmenu_values(void);
void ds_fmenu_open_values(unsigned short f);
void *ds_draw_fmenu_values(void *list, int x, int y, int w, int h);
void *ds_draw_fmenu_kind(void *list, int i, int x, int y);
void *ds_draw_check(void *list, unsigned short x, unsigned short y);
unsigned short *ds_filter_label(unsigned short f, unsigned short v);

int card_get_flag31(int id);
void ds_fmenu_update(void);

/* filter kind icons and labels */
DsButton ds_filter_icons[9] = { { 0x8000, 0x00, 0x00, 0x00 }, { 0x8008, 0x44, 0x20, 0x07 }, { 0x8008, 0x66, 0x20, 0x0B }, { 0x8008, 0x55, 0x20, 0x0C }, { 0x8008, 0x33, 0x20, 0x08 }, { 0x800B, 0x00, 0x33, 0x2A }, { 0x8008, 0x11, 0x31, 0x10 }, { 0x800B, 0x44, 0x22, 0x2B }, { 0x8008, 0x66, 0x31, 0x12 } };

/* opens the filter menu (kinds list) */
void ds_fmenu_open(void)
{
    DsList *l = dsv.pane[dsv.side].list;
    MenuWidget *m;
    MenuItemDef *d;
    int id;
    int f;

    ds_fmenu = heap_alloc(ds.heap, 0x18);
    if (ds_fmenu == 0)
        return;
    memset(ds_fmenu, 0, 0x18);
    ds_fmenu->kinds = heap_alloc(ds.heap, 0x120);
    m = ds_fmenu->kinds;
    if (m != 0) {
        d = &m->def;
        memset(m, 0, 0x120);
        d->visible = 1;
        d->style = 0;
        d->y = 0x3A;
        d->pad_t = 2;
        d->pad_b = 2;
        d->pad_l = 2;
        d->pad_r = 2;
        d->auto_size = 1;
        d->color = 0xFFFFFFFF;
        d->open_anim = 2;
        d->close_anim = 2;
        d->show_title = 1;
        d->auto_title = 1;
        d->font_flag4 = 1;
        d->style2 = 2;
        d->title = table_get_entry(res_get(0x8033), 0x56);
        d->title_font_size = 0xC;
        d->title_color = 0xFFFFFFFF;
        m->heap = ds.heap;
        m->keep_open = 1;
        m->no_cancel = 1;
        m->style = 2;
        m->count = 7;
        m->item_w = 0xA4;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)ds_draw_fmenu_kind;
        menu_open(m);
        d->x = (0x1E0 - d->w) / 2;
    }
    for (id = 0xFA7; id < 0x1B2E; id++) {
        if (card_get_flag31(id) && inv_get_bit3(id)) {
            ds_fmenu->alt = 1;
            break;
        }
    }
    f = l->filter - 1;
    if (f >= 0) {
        if (f >= 7)
            f = 0;
    } else
        f = 0;
    ds_fmenu->kinds->cursor = f;
    ds.state = 5;
}

/* state 5 */
void ds_fmenu_update(void)
{
    int r;

    switch (ds_fmenu->step) {
    case 0:
        r = menu_update(ds_fmenu->kinds);
        if (r == -1)
            break;
        if (r == -2) {
            menu_finish(ds_fmenu->kinds);
            ds_fmenu->step = 2;
        } else {
            ds_fmenu->kinds->no_cancel = 0;
            ds_fmenu_open_values(r + 1);
            se_play(0x6216);
            ds_fmenu->col = ds_fmenu->page = 0;
            ds_fmenu->mask[0] = 0;
            ds_fmenu->mask[1] = 0;
            ds_fmenu->sub = 0;
            ds_fmenu->step = 1;
        }
        break;
    case 1:
        ds_fmenu_values();
        break;
    case 2:
        if (menu_update(ds_fmenu->kinds) != -1) {
            ds_fmenu_free();
            ds.state = 1;
        }
        break;
    }
}

void *ds_draw_flag_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *ds_draw_spell_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *ds_draw_genre_value(void *list, unsigned short x, unsigned short y, unsigned short v);

DsFilterDef ds_filter_defs[9] = {
    { 0, 0, 0 },
    { 0, 5, (DsValueDraw)ds_draw_limit_icon },
    { 1, 20, (DsValueDraw)ds_draw_type_icon },
    { 1, 6, (DsValueDraw)ds_draw_kind_icon },
    { 0, 2, (DsValueDraw)ds_draw_flag_value },
    { 1, 9, (DsValueDraw)ds_draw_spell_value },
    { 0, 3, (DsValueDraw)ds_draw_pack_icon },
    { 0, 35, (DsValueDraw)ds_draw_genre_value },
};

#ifdef NON_MATCHING /* máquina de passos grande; alocação e blocos diferentes */
/* the values window of filter kind (cursor of the kinds list) + 1 */
void ds_fmenu_values(void)
{
    MenuWidget *m = ds_fmenu->vals;
    PadState *pad = pad_get();
    unsigned int f = ds_fmenu->kinds->cursor + 1;
    unsigned int side;
    DsList *l;
    int n;
    unsigned int cols;
    int rows;
    unsigned int old;
    unsigned int v;
    unsigned short view;
    unsigned short grid;

    switch (ds_fmenu->sub) {
    case 3:
        menu_update(m);
        if (m->state == 6) {
            menu_reset(m);
            ds_fmenu->vals = 0;
            ds_fmenu->kinds->accepted = 0;
            ds_fmenu->kinds->cancelled = 0;
            ds_fmenu->kinds->no_cancel = 1;
            ds_fmenu->step = 0;
        }
        break;
    case 2:
        menu_update(m);
        if (m->state == 6) {
            side = dsv.side;
            l = dsv.pane[side].list;
            if (l->count == 0) {
                l->filter = f;
            } else {
                ds_list_filter(l, f, ds_fmenu->mask);
                view = dsv.pane[side].list_mode == 0 ? dsv.pane[side].lay.grid_rows : dsv.pane[side].lay.rows;
                grid = dsv.pane[side].list_mode == 0 ? dsv.pane[side].lay.cols : 1;
                ds_layout_scroll(0, 0, (grid & 0xFF) | view << 8, dsv.pane[side].list->count, &dsv.pane[side].top,
                                 &dsv.pane[side].cx, &dsv.pane[side].cy);
            }
            ds_fmenu_free();
            ds_popup_open(4, l->count);
            se_play(0x621A);
            ds.state = 1;
        }
        break;
    case 1:
        menu_update(ds_fmenu->vals);
        if (ds_fmenu->vals->state != 4) {
            menu_finish(m);
            menu_finish(ds_fmenu->kinds);
            ds_fmenu->sub = 2;
        }
        break;
    case 0:
        if (pad->b.trig & region_pick_value(0)) {
            if (ds_fmenu->page == 0xF) {
                menu_set_cursor(ds_fmenu->vals, 0);
                ds_fmenu->sub = 1;
            } else {
                v = ds_filter_defs[f].first + ds_fmenu->col + ds_fmenu->page * m->count;
                ds_fmenu->mask[(int)v >> 5] ^= 1 << (v & 0x1F);
            }
            se_play(0x6213);
            break;
        }
        if (pad->b.trig & region_pick_value(1)) {
            m->cancelled = 1;
            menu_finish(m);
            ds_fmenu->sub = 3;
            se_play(0x6214);
            break;
        }
        n = ds_filter_defs[f].last - ds_filter_defs[f].first + 1;
        if (!ds_fmenu->alt && f == 7)
            n--;
        cols = m->count;
        rows = (int)(n + cols - 1) / (int)cols;
        if (pad->b.rep & 0x10) {
            old = ds_fmenu->page;
            if (old == 0) {
                ds_fmenu->page = 0xF;
                se_play(0x6212);
            } else {
                if (old == 0xF)
                    ds_fmenu->page = rows - 1;
                else
                    ds_fmenu->page = old - 1;
                if ((int)(ds_fmenu->col + ds_fmenu->page * cols) >= n)
                    ds_fmenu->page = ds_fmenu->page - 1;
                if (old != ds_fmenu->page)
                    se_play(0x6212);
            }
        }
        if (pad->b.rep & 0x40) {
            old = ds_fmenu->page;
            if (old == 0xF)
                ds_fmenu->page = 0;
            else
                ds_fmenu->page = old + 1;
            if ((int)(ds_fmenu->col + ds_fmenu->page * cols) >= n)
                ds_fmenu->page = 0xF;
            if (old != ds_fmenu->page)
                se_play(0x6212);
        }
        if (ds_fmenu->page == rows - 1u) {
            cols = n % (int)m->count;
            if (cols == 0)
                cols = m->count;
        }
        if (ds_fmenu->page != 0xF) {
            old = ds_fmenu->col;
            if (pad->b.rep & 0x80)
                ds_fmenu->col = (int)(old + cols - 1) % (int)cols;
            if (pad->b.rep & 0x20)
                ds_fmenu->col = (int)(ds_fmenu->col + 1) % (int)cols;
            if (old != ds_fmenu->col)
                se_play(0x6212);
        }
        break;
    }
}
#endif

void ds_fmenu_free(void)
{
    if (ds_fmenu->vals != 0) {
        menu_reset(ds_fmenu->vals);
        heap_set_level(ds.heap, ds_fmenu->vals);
    }
    menu_reset(ds_fmenu->kinds);
    heap_set_level(ds.heap, ds_fmenu->kinds);
    heap_set_level(ds.heap, ds_fmenu);
    ds_fmenu = 0;
}

void *ds_draw_confirm(void *list)
{
    unsigned int side;

    if (dsv.mode > 1) {
        side = (dsv.side + 1) & 1;
        list = ds_draw_shade(list, side, dsv.pane[side].list->tab);
    }
    menu_draw(&list, ds_fmenu->kinds);
    if (ds_fmenu->vals != 0)
        menu_draw(&list, ds_fmenu->vals);
    return list;
}

void *ds_draw_fmenu_kind(void *list, int i, int x, int y)
{
    return ds_draw_button(list, &ds_filter_icons[i + 1], x, y + 1, 0);
}

/* the values window of filter f */
void ds_fmenu_open_values(unsigned short f)
{
    DsFilterDef *d = &ds_filter_defs[f];
    int x = (0x1E0 - ds_fmenu->kinds->def.w) / 2;
    MenuWidget *m = ds_fmenu->vals;
    int n;
    MenuItemDef *def;

    if (m == 0) {
        ds_fmenu->vals = heap_alloc(ds.heap, 0x120);
        m = ds_fmenu->vals;
        if (m == 0)
            return;
    }
    def = &m->def;
    n = d->last - d->first + 1;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    def->x = x + 0x6D;
    def->y = ds_fmenu->kinds->def.y + 4 + f * 0x12;
    def->w = 0x8A;
    def->h = (n + 4) / 5 * 0x19 + 0x35;
    def->color = 0xFFFFFFFF;
    def->pad_l = 2;
    def->pad_t = 0x12;
    def->pad_r = 2;
    def->pad_b = 2;
    def->open_anim = 1;
    def->close_anim = 1;
    m->heap = ds.heap;
    m->no_input = 1;
    m->no_cancel = 1;
    m->item_w = 0x19;
    m->item_h = 0x19;
    m->count = 5;
    m->draw_all = (MenuDrawRectFn)ds_draw_fmenu_values;
    switch (f) {
    case 1:
        m->count = 4;
        break;
    case 3:
        def->w = 0xA3;
        def->h = 0x4E;
        m->count = 6;
        break;
    case 5:
        def->w = 0xA3;
        m->count = 6;
        break;
    case 7:
        def->w = 0xB7;
        def->h = 0xCB;
        def->pad_l += 10;
        def->pad_r += 10;
        def->y = ds_fmenu->kinds->def.y - 0x16;
        m->count = 6;
        break;
    default:
        if (n < 5)
            m->count = n;
        break;
    }
    menu_open(m);
}

#ifdef NON_MATCHING /* alocação diferente */
/* the values grid: title strip with the label of the value under the cursor, the values with a
   tick on the picked ones, and the OK button */
void *ds_draw_fmenu_values(void *list, int x, int y, int w, int h)
{
    MenuWidget *m = ds_fmenu->vals;
    unsigned int f = ds_fmenu->kinds->cursor + 1;
    DsFilterDef *d = &ds_filter_defs[f];
    int ox = (m->item_w - 0x11) / 2;
    int cur = ds_fmenu->col + ds_fmenu->page * m->count;
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
    if (ds_fmenu->page == 0xF)
        cur = -1;
    if (cur >= 0 && (label = ds_filter_label(f, d->first + cur)) != 0) {
        font_set_size(12, 12);
        font_set_color(0xFFFFFFFF);
        font_text_set(list);
        font_draw_wtext((bx + 4) << 6, (y - 0xF) << 6, (mw - 0x15) << 6, label);
        list = font_text_clear();
    }
    n = d->last - d->first + 1;
    if (!ds_fmenu->alt && f == 7)
        n = d->last - d->first;
    for (i = 0; i < n; i++) {
        v = d->first + i;
        cx = x + i % m->count * m->item_w;
        cy = y + m->item_h * (i / m->count);
        if (ds_fmenu->mask[(int)v >> 5] & 1 << (v & 0x1F))
            list = ds_draw_check(list, cx, cy);
        if (cur == i)
            list = menu_draw_cursor(list, m, cx, cy, 0x18, 0x18);
        list = d->draw(list, cx + ox, cy + oy, v);
    }
    label = table_get_entry(res_get(0x8033), 0x17);
    tx = x + (w - 0x3C) / 2;
    ty = y + h - 0x18;
    list = menu_draw_item(list, m, tx, ty, 0, ds_fmenu->page == 0xF);
    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    tx += (0x3C - (font_measure_text(label) >> 6)) / 2;
    font_draw_text(tx << 6, (ty + (0x18 - (font_line_height() >> 6)) / 2) << 6, (480 - tx) << 6, label);
    return font_text_clear();
}
#endif

void *ds_draw_flag_value(void *list, unsigned short x, unsigned short y, unsigned short v)
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

void *ds_draw_spell_value(void *list, unsigned short x, unsigned short y, unsigned short v)
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
void *ds_draw_genre_value(void *list, unsigned short x, unsigned short y, unsigned short v)
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

void *ds_draw_filter_icon(void *list, int filter, int x, int y)
{
    DsButton *b;
    void *gim;

    if (filter == 0)
        return list;
    {
        b = &ds_filter_icons[filter];
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

void *ds_draw_check(void *list, unsigned short x, unsigned short y)
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

unsigned short ds_limit_labels[10] = { 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x0 };
unsigned short ds_spell_labels[14] = { 0x35, 0x36, 0x37, 0x38, 0x0, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41 };

/* label of value v of filter f */
unsigned short *ds_filter_label(unsigned short f, unsigned short v)
{
    unsigned short *s;

    switch (f) {
    case 1:
        s = table_get_entry(res_get(0x8033), ds_limit_labels[v]);
        break;
    case 2:
        s = card_db_table_sel(v);
        break;
    case 3:
        s = dlg_table_sel(v);
        break;
    case 4:
        s = table_get_entry(res_get(0x8033), ds_limit_labels[v + 6]);
        break;
    case 5:
        s = table_get_entry(res_get(0x8033), ds_spell_labels[v + 4]);
        break;
    case 6:
        s = table_get_entry(res_get(0x8033), ds_spell_labels[v]);
        break;
    case 7:
        s = dlg_get_text_bounded(v);
        break;
    default:
        s = 0;
        break;
    }
    return s;
}

