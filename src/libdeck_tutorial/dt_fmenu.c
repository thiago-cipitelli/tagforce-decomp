// @range 0x08811BB8-0x088135A8
// @asm 0x08811FEC-0x088126D0
// @asm 0x08812AF0-0x08812FD0
// @asm 0x088131C4-0x088132B8
// @rodata 0x08821030-0x08821050
// @data 0x08821F74-0x08822024
/* The filter menu (the code of libdeck dk_sortmenu.c second half and dk_fmenu.c): the kinds list and
   the values window of a kind (grid of icons, picked values ticked, OK button), the value icons, the
   tick and the label of each value; the lesson gives both menus their input and may lock their
   buttons. */
#include "libdeck_tutorial.h"

typedef struct DtButton {
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} DtButton;

void *dt_draw_ibutton(void *list, DtButton *b, int x, int y, unsigned int color);
void dt_menu_lock(MenuWidget *m);
void *dt_eres_get(unsigned short id);
void *dt_draw_limit_icon(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_pack_icon(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_fmenu_values(void *list, int x, int y, int w, int h);
void *dt_draw_flag_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_spell_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_genre_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_check(void *list, unsigned short x, unsigned short y);
unsigned short *dt_filter_label(unsigned short f, unsigned short v);
void *dt_draw_shade(void *list, unsigned short side, unsigned short tab, int dim);
void dt_list_sort(DtList *l, unsigned short sort, unsigned short dir);
void dt_sort_menu_place(void);
void dt_sort_menu_free(void);
void *dt_draw_sort_item(void *list, int i, int x, int y);
void *dt_draw_dir_item(void *list, int i, int x, int y);
void *dt_draw_fmenu_kind(void *list, int i, int x, int y);
void dt_fmenu_values(PadState *pad);
void dt_list_filter(DtList *l, unsigned int f, unsigned int *mask);
void dt_layout_scroll(int keep, int cur, unsigned short view, unsigned short total,
                      unsigned short *top, unsigned short *cx, unsigned short *cy);
void dt_popup_open(int kind, unsigned short n);
void dt_fmenu_free(void);
void dt_fmenu_open_values(unsigned short f);
int font_measure_text(unsigned short *text);
int card_get_flag31(int id);
/* opens the sort menu on the pane with the cursor */

void dt_fmenu_open(void)
{
    DtList *l = dtv.pane[dtv.side].list;
    MenuWidget *m;
    MenuItemDef *d;
    int id;
    int f;

    dt_fmenu = heap_alloc(dts.heap, 0x18);
    if (dt_fmenu == 0)
        return;
    memset(dt_fmenu, 0, 0x18);
    dt_fmenu->kinds = heap_alloc(dts.heap, 0x120);
    m = dt_fmenu->kinds;
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
        d->title = table_get_entry(dt_eres_get(0x8033), 0x56);
        d->title_font_size = 0xC;
        d->title_color = 0xFFFFFFFF;
        m->heap = dts.heap;
        m->keep_open = 1;
        m->no_cancel = 1;
        m->style = 2;
        m->count = 7;
        m->item_w = 0xA4;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)dt_draw_fmenu_kind;
        m->pad = (PadState *)&dt.pad;
        if (dt.step == 15)
            dt_menu_lock(m);
        menu_open(m);
        d->x = (0x1E0 - d->w) / 2;
    }
    for (id = 0xFA7; id < 0x1B2E; id++) {
        if (card_get_flag31(id) && inv_get_bit3(id)) {
            dt_fmenu->alt = 1;
            break;
        }
    }
    f = l->filter - 1;
    if (f >= 0) {
        if (f >= 7)
            f = 0;
    } else
        f = 0;
    dt_fmenu->kinds->cursor = f;
    dts.state = 4;
}

/* state 4 */
void dt_fmenu_update(PadState *pad)
{
    int r;

    switch (dt_fmenu->step) {
    case 0:
        r = menu_update(dt_fmenu->kinds);
        if (r == -1)
            break;
        if (r == -2) {
            menu_finish(dt_fmenu->kinds);
            dt_fmenu->step = 2;
        } else {
            dt_fmenu->kinds->no_cancel = 0;
            dt_fmenu_open_values(r + 1);
            se_play(0x6216);
            dt_fmenu->col = dt_fmenu->page = 0;
            dt_fmenu->mask[0] = 0;
            dt_fmenu->mask[1] = 0;
            dt_fmenu->sub = 0;
            dt_fmenu->step = 1;
        }
        break;
    case 1:
        dt_fmenu_values(pad);
        break;
    case 2:
        if (menu_update(dt_fmenu->kinds) != -1) {
            dt_fmenu_free();
            dts.state = 0;
        }
        break;
    }
}

#ifdef NON_MATCHING /* máquina de passos grande (dk_fmenu_values com o teste da lição no OK); alocação e blocos diferentes, como sc_fmenu_values */
/* the values window of filter kind (cursor of the kinds list) + 1 */
void dt_fmenu_values(PadState *pad)
{
    MenuWidget *m = dt_fmenu->vals;
    unsigned int f = dt_fmenu->kinds->cursor + 1;
    DtFilterDef *def = &dt_filter_defs[f];
    unsigned int side;
    DtList *l;
    int n;
    unsigned int cols;
    int rows;
    unsigned int old;
    unsigned int v;
    unsigned short view;
    unsigned short grid;

    switch (dt_fmenu->sub) {
    case 0:
        if (pad->b.trig & region_pick_value(0)) {
            if (dt_fmenu->page == 0xF) {
                if ((dt.show & 4) && dt_fmenu->mask[0] != ~m->disabled)
                    return;   /* lesson: OK only with the values asked for */
                menu_set_cursor(dt_fmenu->vals, 0);
                dt_fmenu->sub = 1;
            } else {
                v = def->first + dt_fmenu->col + dt_fmenu->page * m->count;
                dt_fmenu->mask[(int)v >> 5] ^= 1 << (v & 0x1F);
            }
            se_play(0x6213);
            break;
        }
        if (pad->b.trig & region_pick_value(1)) {
            m->cancelled = 1;
            menu_finish(m);
            dt_fmenu->sub = 3;
            se_play(0x6214);
            break;
        }
        n = def->last - def->first + 1;
        if (!dt_fmenu->alt && f == 7)
            n--;
        cols = m->count;
        rows = (int)(n + cols - 1) / (int)cols;
        if (pad->b.rep & 0x10) {
            old = dt_fmenu->page;
            if (old == 0) {
                dt_fmenu->page = 0xF;
                se_play(0x6212);
            } else {
                if (old == 0xF)
                    dt_fmenu->page = rows - 1;
                else
                    dt_fmenu->page = old - 1;
                if ((int)(dt_fmenu->col + dt_fmenu->page * cols) >= n)
                    dt_fmenu->page = dt_fmenu->page - 1;
                if (old != dt_fmenu->page)
                    se_play(0x6212);
            }
        }
        if (pad->b.rep & 0x40) {
            old = dt_fmenu->page;
            if (old == 0xF)
                dt_fmenu->page = 0;
            else
                dt_fmenu->page = old + 1;
            if ((int)(dt_fmenu->col + dt_fmenu->page * cols) >= n)
                dt_fmenu->page = 0xF;
            if (old != dt_fmenu->page)
                se_play(0x6212);
        }
        if (dt_fmenu->page == rows - 1u) {
            cols = n % (int)m->count;
            if (cols == 0)
                cols = m->count;
        }
        if (dt_fmenu->page != 0xF) {
            old = dt_fmenu->col;
            if (pad->b.rep & 0x80)
                dt_fmenu->col = (int)(old + cols - 1) % (int)cols;
            if (pad->b.rep & 0x20)
                dt_fmenu->col = (int)(dt_fmenu->col + 1) % (int)cols;
            if (old != dt_fmenu->col)
                se_play(0x6212);
        }
        break;
    case 1:
        menu_update(m);
        if (dt_fmenu->vals->state != 4) {
            menu_finish(m);
            menu_finish(dt_fmenu->kinds);
            dt_fmenu->sub = 2;
        }
        break;
    case 2:
        menu_update(m);
        if (m->state == 6) {
            side = dtv.side;
            l = dtv.pane[side].list;
            if (l->count == 0) {
                l->filter = f;
            } else {
                dt_list_filter(l, f, dt_fmenu->mask);
                view = dtv.pane[side].list_mode == 0 ? dtv.pane[side].lay.grid_rows : dtv.pane[side].lay.rows;
                grid = dtv.pane[side].list_mode == 0 ? dtv.pane[side].lay.cols : 1;
                dt_layout_scroll(0, 0, (grid & 0xFF) | view << 8, dtv.pane[side].list->count, &dtv.pane[side].top,
                                 &dtv.pane[side].cx, &dtv.pane[side].cy);
            }
            dt_fmenu_free();
            dt_popup_open(6, l->count);
            se_play(0x621A);
            dts.state = 0;
        }
        break;
    case 3:
        menu_update(m);
        if (m->state == 6) {
            menu_reset(m);
            dt_fmenu->vals = 0;
            dt_fmenu->kinds->accepted = 0;
            dt_fmenu->kinds->cancelled = 0;
            dt_fmenu->kinds->no_cancel = 1;
            dt_fmenu->step = 0;
        }
        break;
    }
}
#endif

void dt_fmenu_free(void)
{
    if (dt_fmenu->vals != 0) {
        menu_reset(dt_fmenu->vals);
        heap_set_level(dts.heap, dt_fmenu->vals);
    }
    menu_reset(dt_fmenu->kinds);
    heap_set_level(dts.heap, dt_fmenu->kinds);
    heap_set_level(dts.heap, dt_fmenu);
    dt_fmenu = 0;
}

void *dt_fmenu_draw(void *list)
{
    unsigned int side;

    if (dtv.mode > 1) {
        side = (dtv.side + 1) & 1;
        list = dt_draw_shade(list, side, dtv.pane[side].list->tab, 1);
    }
    menu_draw(&list, dt_fmenu->kinds);
    if (dt_fmenu->vals != 0)
        menu_draw(&list, dt_fmenu->vals);
    return list;
}

DtButton dt_filter_icons[9] = {
    { 0x8000, 0x00, 0x00, 0x00 }, { 0x8008, 0x44, 0x20, 0x07 }, { 0x8008, 0x66, 0x20, 0x0B }, { 0x8008, 0x55, 0x20, 0x0C },
    { 0x8008, 0x33, 0x20, 0x08 }, { 0x800B, 0x00, 0x33, 0x2A }, { 0x8008, 0x11, 0x31, 0x10 }, { 0x800B, 0x44, 0x22, 0x2B },
    { 0x8008, 0x66, 0x31, 0x12 },
};

void *dt_draw_fmenu_kind(void *list, int i, int x, int y)
{
    return dt_draw_ibutton(list, &dt_filter_icons[i + 1], x, y + 1, 0);
}


void *dt_eres_get(unsigned short id);
void *dt_draw_limit_icon(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_pack_icon(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_fmenu_values(void *list, int x, int y, int w, int h);
void *dt_draw_flag_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_spell_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_genre_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dt_draw_check(void *list, unsigned short x, unsigned short y);
unsigned short *dt_filter_label(unsigned short f, unsigned short v);

DtFilterDef dt_filter_defs[9] = {
    { 0, 0, 0 },
    { 0, 5, (DtValueDraw)dt_draw_limit_icon },
    { 1, 20, (DtValueDraw)dt_draw_type_icon },
    { 1, 6, (DtValueDraw)dt_draw_kind_icon },
    { 0, 2, (DtValueDraw)dt_draw_flag_value },
    { 1, 9, (DtValueDraw)dt_draw_spell_value },
    { 0, 3, (DtValueDraw)dt_draw_pack_icon },
    { 0, 35, (DtValueDraw)dt_draw_genre_value },
};

/* the values window of filter f */
void dt_fmenu_open_values(unsigned short f)
{
    DtFilterDef *d = &dt_filter_defs[f];
    int x = (0x1E0 - dt_fmenu->kinds->def.w) / 2;
    MenuWidget *m = dt_fmenu->vals;
    int n;
    MenuItemDef *def;

    if (m == 0) {
        dt_fmenu->vals = heap_alloc(dts.heap, 0x120);
        m = dt_fmenu->vals;
        if (m == 0)
            return;
    }
    def = &m->def;
    n = d->last - d->first + 1;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    def->x = x + 0x6D;
    def->y = dt_fmenu->kinds->def.y + 4 + f * 0x12;
    def->w = 0x8A;
    def->h = (n + 4) / 5 * 0x19 + 0x35;
    def->color = 0xFFFFFFFF;
    def->pad_l = 2;
    def->pad_t = 0x12;
    def->pad_r = 2;
    def->pad_b = 2;
    def->open_anim = 1;
    def->close_anim = 1;
    m->heap = dts.heap;
    m->no_input = 1;
    m->no_cancel = 1;
    m->item_w = 0x19;
    m->item_h = 0x19;
    m->count = 5;
    m->draw_all = (MenuDrawRectFn)dt_draw_fmenu_values;
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
        def->y = dt_fmenu->kinds->def.y - 0x16;
        m->count = 6;
        break;
    default:
        if (n < 5)
            m->count = n;
        break;
    }
    m->pad = (PadState *)&dt.pad;
    if (dt.step == 15)
        dt_menu_lock(m);
    menu_open(m);
}

#ifdef NON_MATCHING /* x fica na pilha e cur em $s8 no original; alocação diferente */
/* the values grid: title strip with the label of the value under the cursor, the values with a
   tick on the picked ones, and the OK button */
void *dt_draw_fmenu_values(void *list, int x, int y, int w, int h)
{
    MenuWidget *m = dt_fmenu->vals;
    unsigned int f = dt_fmenu->kinds->cursor + 1;
    DtFilterDef *d = &dt_filter_defs[f];
    int cur = dt_fmenu->col + dt_fmenu->page * m->count;
    int ox = (m->item_w - 0x11) / 2;
    int oy = (m->item_h - 0x11) / 2;
    int bx = x;
    int bw = m->def.w - 0xD;
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
    gim = dt_eres_get(0x8006);
    if (gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 4, 0x11, 4, 0x11);
        list = draw_sprite(list, bx, y - 0x12, 0, 0xE0, 0, 0, 0);
        list = draw_sprite(list, bx + bw - 4, y - 0x12, 0, 0xE0, 0, 1, 0);
        list = draw_set_offsets(list, 4, 0x11, bw - 8, 0x11);
        list = draw_sprite(list, bx + 4, y - 0x12, 0, 0xE4, 0, 0, 0);
    }
    if (dt_fmenu->page == 0xF)
        cur = -1;
    if (cur >= 0 && (label = dt_filter_label(f, d->first + cur)) != 0) {
        font_set_size(12, 12);
        font_set_color(0xFFFFFFFF);
        font_text_set(list);
        font_draw_wtext((bx + 4) << 6, (y - 0xF) << 6, (bw - 8) << 6, label);
        list = font_text_clear();
    }
    n = d->last - d->first + 1;
    if (!dt_fmenu->alt && f == 7)
        n--;
    for (i = 0; i < n; i++) {
        v = d->first + i;
        cx = x + i % m->count * m->item_w;
        cy = y + m->item_h * (i / m->count);
        if (dt_fmenu->mask[(int)v >> 5] & 1 << (v & 0x1F))
            list = dt_draw_check(list, cx, cy);
        if (cur == i)
            list = menu_draw_cursor(list, m, cx, cy, 0x18, 0x18);
        list = d->draw(list, cx + ox, cy + oy, v);
    }
    label = table_get_entry(dt_eres_get(0x8033), 0x17);
    tx = x + (w - 0x3C) / 2;
    ty = y + h - 0x18;
    list = menu_draw_button(list, m, tx, ty, 0, dt_fmenu->page == 0xF);
    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    tx += (0x3C - (font_measure_text(label) >> 6)) / 2;
    font_draw_text(tx << 6, (ty + (0x18 - (font_line_height() >> 6)) / 2) << 6, (480 - tx) << 6, label);
    return font_text_clear();
}
#endif

void *dt_draw_flag_value(void *list, unsigned short x, unsigned short y, unsigned short v)
{
    unsigned short u = v * 17;
    void *gim = dt_eres_get(0x800A);

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

void *dt_draw_spell_value(void *list, unsigned short x, unsigned short y, unsigned short v)
{
    unsigned short u = v % 7 * 17;
    unsigned short tv = v / 7 * 17 + 0x33;
    void *gim = dt_eres_get(0x800B);

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
void *dt_draw_genre_value(void *list, unsigned short x, unsigned short y, unsigned short v)
{
    void *gim = dt_eres_get(0x800C);

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

/* icon of filter kind `filter` (0 none) */
void *dt_draw_filter_icon(void *list, int filter, int x, int y)
{
    DtButton *b;
    void *gim;

    if (filter == 0)
        return list;
    {
        b = &dt_filter_icons[filter];
        gim = dt_eres_get(b->res);
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

void *dt_draw_check(void *list, unsigned short x, unsigned short y)
{
    void *gim = dt_eres_get(0x8006);

    if (gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 0x1A, 0x1A, 0x1A, 0x1A);
        list = draw_sprite(list, x, y, 0, 0x9A, 0xC0, 0, 0);
    }
    return list;
}

unsigned short dt_limit_labels[10] = { 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x0 };
unsigned short dt_spell_labels[14] = { 0x35, 0x36, 0x37, 0x38, 0x0, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41 };

/* label of value v of filter f */
unsigned short *dt_filter_label(unsigned short f, unsigned short v)
{
    unsigned short *s;

    switch (f) {
    case 1:
        s = table_get_entry(dt_eres_get(0x8033), dt_limit_labels[v]);
        break;
    case 2:
        s = card_db_table_sel(v);
        break;
    case 3:
        s = dlg_table_sel(v);
        break;
    case 4:
        s = table_get_entry(dt_eres_get(0x8033), dt_limit_labels[v + 6]);
        break;
    case 5:
        s = table_get_entry(dt_eres_get(0x8033), dt_spell_labels[v + 4]);
        break;
    case 6:
        s = table_get_entry(dt_eres_get(0x8033), dt_spell_labels[v]);
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

