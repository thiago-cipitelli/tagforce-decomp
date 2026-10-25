// @range 0x08812510-0x08813268
// @asm 0x088127B0-0x08812C90
// @asm 0x08812E84-0x08812F78
// @data 0x08822F94-0x0882300C
// @rodata 0x08822628-0x08822648
/* Filter menu, second half: the values window of a filter kind (grid of icons, picked values
   ticked, OK button), the value icons (limit, type, kind, flag, spell, pack, genre), the filter kind
   icon, the tick and the label of each value. */
#include "libdeck.h"
#include "menu.h"

typedef struct DkButton {
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} DkButton;

extern DkButton dk_filter_icons[9];    /* 0x08822F5C */

void *dk_res_get(unsigned short id);
void *func_088104E4(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dk_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dk_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short v);
void *func_08810550(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dk_draw_fmenu_values(void *list, int x, int y, int w, int h);
void *dk_draw_flag_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dk_draw_spell_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dk_draw_genre_value(void *list, unsigned short x, unsigned short y, unsigned short v);
void *dk_draw_check(void *list, unsigned short x, unsigned short y);
unsigned short *dk_filter_label(unsigned short f, unsigned short v);

DkFilterDef dk_filter_defs[9] = {
    { 0, 0, 0 },
    { 0, 5, (DkValueDraw)func_088104E4 },
    { 1, 20, (DkValueDraw)dk_draw_type_icon },
    { 1, 6, (DkValueDraw)dk_draw_kind_icon },
    { 0, 2, (DkValueDraw)dk_draw_flag_value },
    { 1, 9, (DkValueDraw)dk_draw_spell_value },
    { 0, 3, (DkValueDraw)func_08810550 },
    { 0, 35, (DkValueDraw)dk_draw_genre_value },
};

/* the values window of filter f */
void dk_fmenu_open_values(unsigned short f)
{
    DkFilterDef *d = &dk_filter_defs[f];
    int x = (0x1E0 - dk_fmenu->kinds->def.w) / 2;
    MenuWidget *m = dk_fmenu->vals;
    int n;
    MenuItemDef *def;

    if (m == 0) {
        dk_fmenu->vals = heap_alloc(dk_heap, 0x120);
        m = dk_fmenu->vals;
        if (m == 0)
            return;
    }
    def = &m->def;
    n = d->last - d->first + 1;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    def->x = x + 0x6D;
    def->y = dk_fmenu->kinds->def.y + 4 + f * 0x12;
    def->w = 0x8A;
    def->h = (n + 4) / 5 * 0x19 + 0x35;
    def->color = 0xFFFFFFFF;
    def->pad_l = 2;
    def->pad_t = 0x12;
    def->pad_r = 2;
    def->pad_b = 2;
    def->open_anim = 1;
    def->close_anim = 1;
    m->heap = dk_heap;
    m->no_input = 1;
    m->no_cancel = 1;
    m->item_w = 0x19;
    m->item_h = 0x19;
    m->count = 5;
    m->draw_all = (MenuDrawRectFn)dk_draw_fmenu_values;
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
        def->y = dk_fmenu->kinds->def.y - 0x16;
        m->count = 6;
        break;
    default:
        if (n < 5)
            m->count = n;
        break;
    }
    menu_open(m);
}

#ifdef NON_MATCHING /* x fica na pilha e cur em $s8 no original; alocação diferente */
/* the values grid: title strip with the label of the value under the cursor, the values with a
   tick on the picked ones, and the OK button */
void *dk_draw_fmenu_values(void *list, int x, int y, int w, int h)
{
    MenuWidget *m = dk_fmenu->vals;
    unsigned int f = dk_fmenu->kinds->cursor + 1;
    DkFilterDef *d = &dk_filter_defs[f];
    int cur = dk_fmenu->col + dk_fmenu->page * m->count;
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
    gim = dk_res_get(0x8006);
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
    if (dk_fmenu->page == 0xF)
        cur = -1;
    if (cur >= 0 && (label = dk_filter_label(f, d->first + cur)) != 0) {
        font_set_size(12, 12);
        font_set_color(0xFFFFFFFF);
        font_text_set(list);
        font_draw_wtext((bx + 4) << 6, (y - 0xF) << 6, (bw - 8) << 6, label);
        list = font_text_clear();
    }
    n = d->last - d->first + 1;
    if (!dk_fmenu->alt && f == 7)
        n--;
    for (i = 0; i < n; i++) {
        v = d->first + i;
        cx = x + i % m->count * m->item_w;
        cy = y + m->item_h * (i / m->count);
        if (dk_fmenu->mask[(int)v >> 5] & 1 << (v & 0x1F))
            list = dk_draw_check(list, cx, cy);
        if (cur == i)
            list = menu_draw_cursor(list, m, cx, cy, 0x18, 0x18);
        list = d->draw(list, cx + ox, cy + oy, v);
    }
    label = table_get_entry(dk_res_get(0x8033), 0x17);
    tx = x + (w - 0x3C) / 2;
    ty = y + h - 0x18;
    list = menu_draw_button(list, m, tx, ty, 0, dk_fmenu->page == 0xF);
    font_set_size(12, 12);
    font_text_set(list);
    font_set_color(0xFF000000);
    tx += (0x3C - (font_measure_text(label) >> 6)) / 2;
    font_draw_text(tx << 6, (ty + (0x18 - (font_line_height() >> 6)) / 2) << 6, (480 - tx) << 6, label);
    return font_text_clear();
}
#endif

void *dk_draw_flag_value(void *list, unsigned short x, unsigned short y, unsigned short v)
{
    unsigned short u = v * 17;
    void *gim = dk_res_get(0x800A);

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

void *dk_draw_spell_value(void *list, unsigned short x, unsigned short y, unsigned short v)
{
    unsigned short u = v % 7 * 17;
    unsigned short tv = v / 7 * 17 + 0x33;
    void *gim = dk_res_get(0x800B);

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
void *dk_draw_genre_value(void *list, unsigned short x, unsigned short y, unsigned short v)
{
    void *gim = dk_res_get(0x800C);

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
void *dk_draw_filter_icon(void *list, int filter, int x, int y)
{
    DkButton *b;
    void *gim;

    if (filter == 0)
        return list;
    {
        b = &dk_filter_icons[filter];
        gim = dk_res_get(b->res);
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

void *dk_draw_check(void *list, unsigned short x, unsigned short y)
{
    void *gim = dk_res_get(0x8006);

    if (gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 0x1A, 0x1A, 0x1A, 0x1A);
        list = draw_sprite(list, x, y, 0, 0x9A, 0xC0, 0, 0);
    }
    return list;
}

unsigned short dk_limit_labels[10] = { 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x0 };
unsigned short dk_spell_labels[14] = { 0x35, 0x36, 0x37, 0x38, 0x0, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41 };

/* label of value v of filter f */
unsigned short *dk_filter_label(unsigned short f, unsigned short v)
{
    unsigned short *s;

    switch (f) {
    case 1:
        s = table_get_entry(dk_res_get(0x8033), dk_limit_labels[v]);
        break;
    case 2:
        s = card_db_table_sel(v);
        break;
    case 3:
        s = dlg_table_sel(v);
        break;
    case 4:
        s = table_get_entry(dk_res_get(0x8033), dk_limit_labels[v + 6]);
        break;
    case 5:
        s = table_get_entry(dk_res_get(0x8033), dk_spell_labels[v + 4]);
        break;
    case 6:
        s = table_get_entry(dk_res_get(0x8033), dk_spell_labels[v]);
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
