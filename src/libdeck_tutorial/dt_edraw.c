// @range 0x08805BE4-0x08808C6C
// @asm 0x08805BE4-0x08805FC0
// @asm 0x08806294-0x088063E0
// @asm 0x088063E0-0x088068F0
// @asm 0x08806A6C-0x08806D54
// @asm 0x088071CC-0x0880779C
// @asm 0x0880779C-0x08807994
// @asm 0x08807A5C-0x08807B58
// @asm 0x08807B58-0x08807DB8
// @asm 0x08807DB8-0x08807EC0
// @asm 0x08807F60-0x08808098
// @asm 0x08808098-0x088082AC
// @asm 0x088082AC-0x088087EC
// @asm 0x088087EC-0x08808924
// @asm 0x08808924-0x08808C6C
// @data 0x08821E34-0x08821E70
/* Drawing of the deck editor (the libdeck screen the lesson runs on): the card info strip, the icon
   wrappers, the pane shade, the sort value bubble, the frame export (clear, background, panes, focus
   frame and popups), the background (bg.ehf, window, row lines, deck name and card count), the pane
   headers, the copy counts of the tabs, the type bar and the card rows and grid. */
#include "libdeck_tutorial.h"

typedef struct DtLineVtx {       /* dl_draw_array vertex: colour + position, 12 bytes */
    unsigned int color;
    unsigned short x, y, z;
    unsigned short pad;
} DtLineVtx;

typedef struct DtButton {        /* dt_draw_ibutton */
    unsigned short res;          /* resource id of the icon */
    unsigned char u, v;          /* +0x02 */
    unsigned short text;         /* +0x04 strTbl.bin entry */
} DtButton;

typedef struct DtCounter {       /* dt_draw_counters, 12 bytes x 3 */
    unsigned char icon;
    unsigned char pad01[3];
    unsigned int color;
    unsigned char value;
    unsigned char pad09[3];
} DtCounter;

void font_set_flag4000(int on);
int font_get_cursor_x(void);
unsigned short *card_get_name(int id);
unsigned short *card_get_desc(int id);
unsigned short *card_info_fill(int id);
void *cardlist_build(void *list, ScScroll *s);
unsigned int font_sample_glyph_size(void);
int font_measure_text(unsigned short *text);
int card_get_attribute_icon(int id);
int card_attribute_icon_is3(int id);
void *gfx_begin_2d(void *list);
void *gfx_end_2d(void *list);
void *ehf_draw(void *list, void *ehf, void *opts);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void *dl_alloc_inline(void **list, int bytes);
void *dl_clear_color(void *list, unsigned int color);
void *dl_clear_depth(void *list, unsigned int z);
void *dl_clear_stencil(void *list, unsigned int s);
void *dl_clear(void *list, unsigned int flags);

void *dt_eres_get(unsigned short id);
unsigned short dt_cur_index(unsigned short side);
int dt_sort_value(ScEnt *e, unsigned int sort);
unsigned int dt_pane_size(unsigned short tab);
unsigned short dt_tab_rect(unsigned short i, unsigned short sel, ScRect *r);
int dt_kind_icon(unsigned short kind);
int dt_list_count_id(ScList *l, unsigned short id, int sub);
int dt_list_has(ScList *l, unsigned short id, unsigned char *value);
void *dt_draw_digits(void *list, ScPt pos, int pad, unsigned int value, unsigned short fmt, unsigned int color, void *gim);
void *dt_draw_num_text(void *list, int x, int y, int value, int step, unsigned short n, int pad);
void *dt_draw_frame(void *list, int x, int y, int w, int h, void *gim, int style);
void *dt_draw_icon_a(void *list, ScPt pos, unsigned short id, void *gim);
void *dt_draw_icon_b(void *list, ScPt pos, unsigned short id, void *gim);
void *dt_draw_icon_c(void *list, ScPt pos, unsigned short id, void *gim);
void *dt_draw_icon_d(void *list, ScPt pos, unsigned short id, void *gim);
void *dt_draw_icon_e(void *list, ScPt pos, unsigned short id, void *gim);
void *dt_draw_level(void *list, ScPt pos, unsigned short level, unsigned int color, void *gim);
void *dt_draw_stat(void *list, unsigned int st, ScPt pos, unsigned short size, unsigned int color, void *gim);
void *dt_draw_card_pict(void *list, int x, int y, int shadow, unsigned int color, ScEnt *e, void *gim, void *gim_alt, ScIdList *alt);
void *dt_draw_card_marks(void *list, int x, int y, ScEnt *e, int show_count, int blink, int show_fixed, unsigned short rank, void *gim_digits, void *gim);
void *dt_draw_row_frame(void *list, ScPt pos, unsigned short w, unsigned short kind, void *gim);
void *dt_draw_flag_icon(void *list, ScPt pos, unsigned short kind, void *gim);
void *dt_draw_counters(void *list, ScPt pos, DtCounter *c, void *gim_strip, void *gim);
void *dt_draw_icon_text(void *list, ScIconText *t, int x, int y);
void *dt_draw_scrollbar(void *list, int x, int y, int h, int pos, int first, int total, int frame, void *gim);
void *dt_draw_box(void *list, int x, int y, int w, int h, void *gim);
void *dt_draw_help(void *list, int x, int y, int alt, unsigned int flags, void *gim, void *strtbl);
void *dt_draw_sort_icon(void *list, unsigned short sort, int x, int y);
void *dt_draw_dir_icon(void *list, unsigned short dir, int x, int y);
void *dt_draw_filter_icon(void *list, unsigned short filter, int x, int y);
void *dt_draw_sort_menu(void *list);
void *dt_draw_action_menu(void *list);
void *dt_draw_search(void *list);
void *dt_draw_confirm(void *list);
void *dt_draw_result(void *list);
void *dt_draw_popup(void *list);

void *dt_draw_info(void *list, ScCursor *c);
void *dt_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type);
void *dt_draw_attr_icon(void *list, unsigned short x, unsigned short y, unsigned short attr);
void *dt_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind);
void *dt_draw_limit_icon(void *list, unsigned short x, unsigned short y, unsigned short n);
void *dt_draw_pack_icon(void *list, unsigned short x, unsigned short y, unsigned short n);
void *dt_draw_detail(void *list);
void *dt_draw_overlay(void *list);
void *dt_draw_bg(void *list);
void *dt_draw_bg_ehf(void *list);
void *dt_draw_window(void *list);
void *dt_draw_title(void *list);
void *dt_draw_grid_lines(void *list, unsigned short mode);
void dt_line(DtLineVtx *v, unsigned short x, unsigned short y, unsigned short w);
void *dt_draw_headers(void *list);
void *dt_draw_counts(void *list);
void *dt_draw_count(void *list, short x, ScRect *r, unsigned short ndig, int value);
void *dt_draw_type_counts(void *list);
void *dt_draw_type_bar(void *list);
void *dt_draw_type_seg(void *list, int x, int y, int len, int dir, int kind);
void *dt_draw_panes(void *list);
void *dt_draw_rows(void *list, ScPane *p);
void *dt_draw_grid(void *list, ScPane *p);
void *dt_draw_row(void *list, unsigned short x, unsigned short y, unsigned short w, ScEnt *e, int blink);
void *dt_draw_rows_small(void *list, ScPane *p);
void *dt_draw_row_small(void *list, unsigned short x, unsigned short y, unsigned short w, ScEnt *e, int blink);

#ifdef NON_MATCHING
/* the strip under the panes: the card list's own strip in the deck modes, else name, icons, level,
   ATK/DEF and the description scrolling sideways */
void *dt_draw_info(void *list, ScCursor *c)
{
    void *r;
    int x;
    int w;
    int x2;
    unsigned short level;

    if (dtv.mode < 4) {
        r = cardlist_build(list, &dtv.cursor.scroll);
        if (c->id == 0)
            return r;
        if (inv_get_flag7(c->id) == 0)
            return r;
        return dt_draw_flag_icon(r, PT(0x7B, 0x87), 2, dt_eres_get(0x800A));
    }
    if (c->id == 0)
        return list;
    r = dt_draw_kind_icon(list, 0x146, 0x100, card_get_kind(c->id));
    if (card_is_monster(c->id)) {
        r = dt_draw_type_icon(r, 0x158, 0x100, card_get_type(c->id));
        level = card_get_level(c->id);
        r = dt_draw_level(r, PT(0x16B, 0x103), level, 0xFFFFFFFF, dt_eres_get(0x8009));
        r = dt_draw_stat(r, card_get_atk_q(c->id) << 16 | 2, PT(0x187, 0x102), 0xC0A, 0xFFFFFFFF, dt_eres_get(0x8009));
        r = dt_draw_stat(r, card_get_def_q(c->id) << 16 | 3, PT(0x1B3, 0x102), 0xC0A, 0xFFFFFFFF, dt_eres_get(0x8009));
    } else if (card_get_prop5_hi(c->id) != 0) {
        r = dt_draw_attr_icon(r, 0x158, 0x100, card_get_prop5_hi(c->id));
    }
    if (inv_get_flag7(c->id))
        r = dt_draw_flag_icon(r, PT(0x1CE, 0xF5), 2, dt_eres_get(0x800A));
    font_text_set(r);
    font_set_size(12, 12);
    font_set_color(0xFF00FFFF);
    if (cfg_get_language() != 0)
        font_set_flag4000(1);
    font_draw_text(0x180, 0x3CC0, 0x6C80, card_get_name(c->id));
    if (cfg_get_language() != 0)
        font_set_flag4000(0);
    font_set_color(0xFF00FF00);
    x = font_get_cursor_x() + 0x40;
    font_draw_wtext(x, 0x3CC0, 0x6B00 - x, card_info_fill(c->id));
    r = font_text_clear();
    r = dl_scissor(r, 6, 0x100, 0x13D, 0x110);
    r = dl_enable(r, 2);
    font_text_set(r);
    font_set_color(0xFFFFFFFF);
    if (cfg_get_language() != 0)
        font_set_flag8(0);
    x = 0x180 - dtv.scroll_pos;
    w = dtv.scroll_pos + 0x5500;
    font_draw_text(x, 0x4080, w, card_get_desc(c->id));
    if (dtv.scroll_max > 0) {
        x2 = font_get_cursor_x() + 0x2780;
        if (x2 < w + x)
            font_draw_text(x2, 0x4080, w - (x2 - x), card_get_desc(c->id));
    }
    if (cfg_get_language() != 0)
        font_set_flag8(1);
    r = font_text_clear();
    return dl_disable(r, 2);
}
#endif

void *dt_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type)
{
    return dt_draw_icon_a(list, x | y << 16, type, dt_eres_get(0x8007));
}

void *dt_draw_attr_icon(void *list, unsigned short x, unsigned short y, unsigned short attr)
{
    return dt_draw_icon_b(list, x | y << 16, attr, dt_eres_get(0x8007));
}

void *dt_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind)
{
    return dt_draw_icon_c(list, x | y << 16, kind, dt_eres_get(0x8007));
}

void *dt_draw_limit_icon(void *list, unsigned short x, unsigned short y, unsigned short n)
{
    return dt_draw_icon_d(list, x | y << 16, n, dt_eres_get(0x800A));
}

void *dt_draw_pack_icon(void *list, unsigned short x, unsigned short y, unsigned short n)
{
    return dt_draw_icon_e(list, x | y << 16, n, dt_eres_get(0x800A));
}

/* a button icon and its label */
void *dt_draw_ibutton(void *list, DtButton *b, int x, int y, unsigned int color)
{
    ScIconText t;

    memset(&t, 0, sizeof(t));
    t.w = 0x11;
    t.h = 0x11;
    t.u = b->u;
    t.v = b->v;
    t.gim = dt_eres_get(b->res);
    t.text = table_get_entry(dt_eres_get(0x8033), b->text);
    t.color = color;
    return dt_draw_icon_text(list, &t, x, y);
}

#ifdef NON_MATCHING
/* darkens pane side and tab button tab */
void *dt_draw_shade(void *list, unsigned short side, unsigned short tab, int with_tab)
{
    unsigned int pos = dt_pane_size(tab);

    list = dl_disable(list, 9);
    list = dl_tex_func(list, 4, 0);
    list = dl_ambient_color(list, 0x80000000);
    list = draw_set_offsets(list, 0, 0, dtv.pane[side].lay.w, dtv.pane[side].lay.h);
    list = draw_sprite(list, dtv.pane[side].lay.x, dtv.pane[side].lay.y, 0, 0, 0, 0, 0);
    if (with_tab) {
        list = draw_set_offsets(list, 0, 0, pos >> 16, 0xF);
        list = draw_sprite(list, dtv.pane[side].lay.x + (pos & 0xFFFF), 0x17, 0, 0, 0, 0, 0);
    }
    list = dl_enable(list, 9);
    list = dl_tex_func(list, 0, 1);
    return dl_ambient_color(list, 0xFFFFFFFF);
}
#endif

#ifdef NON_MATCHING
/* the value of the sort key of the cursor's card, in a bubble beside the scroll bar */
void *dt_draw_detail(void *list)
{
    unsigned int side = dtv.side;
    ScList *l = dtv.pane[side].list;
    unsigned short i = dt_cur_index(side);
    ScEnt *e = SC_ENT(l, i);
    unsigned char sort = l->sort;
    unsigned int v;
    int w;
    int h;
    unsigned int cols;
    int rows;
    int x;
    int y;
    int half;
    unsigned int sz;
    int lw;

    if (e == 0)
        return list;
    v = dt_sort_value(e, sort);
    switch (sort) {
    case 1:
        font_set_size(14, 14);
        sz = font_sample_glyph_size();
        w = (sz >> 22) + 13;
        h = (sz >> 22) + 11;
        break;
    case 2:
    case 6:
    case 7:
    case 8:
    case 9:
        w = 0x1B;
        h = 0x1B;
        break;
    case 3:
        w = 0x27;
        h = 0x14;
        break;
    case 4:
    case 5:
        w = 0x30;
        h = 0x13;
        break;
    }
    cols = dtv.pane[side].lay.cols;
    rows = (int)(l->count + cols - 1) / (int)cols;
    x = dtv.pane[side].lay.x + dtv.pane[side].lay.w - 10 - w;
    if (rows == 0) {
        y = dtv.pane[side].lay.y + 12 + (dtv.pane[side].lay.h - 0x2B - h) / 2;
    } else {
        half = h / 2;
        y = dtv.pane[side].lay.y + 12 + (dtv.pane[side].lay.h - 0x2B) * (i / cols) / rows - half;
    }
    list = dt_draw_box(list, x, y, w, h, dt_eres_get(0x8013));
    if (v == 0xFFFFFFFE)
        return list;
    switch (sort) {
    case 0:
        break;
    case 1: {
        unsigned short ch[2] = { 0 };

        font_text_set(list);
        font_set_size(14, 14);
        font_set_color(0xFFFFFFFF);
        ch[0] = card_top_get_letter(v);
        lw = font_measure_text(ch);
        font_draw_text((x + 4 + (w - 9) / 2) * 64 - lw / 2, (y + 4) * 64, w << 6, ch);
        list = font_text_clear();
        break;
    }
    case 2:
        list = dt_draw_limit_icon(list, x + 5, y + 5, dt_kind_icon(v));
        break;
    case 3:
        list = dt_draw_level(list, PT(x + 5, y + 4), v, 0xFFFFFFFF, dt_eres_get(0x8009));
        break;
    case 4:
        if ((int)v > 0)
            v = (int)v / 100 * 100;
        else if (v == 0xFFFFFFFF)
            v = 0xFFFF;
        list = dt_draw_stat(list, v << 16 | 2, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, dt_eres_get(0x8009));
        break;
    case 5:
        if ((int)v > 0)
            v = (int)v / 100 * 100;
        else if (v == 0xFFFFFFFF)
            v = 0xFFFF;
        list = dt_draw_stat(list, v << 16 | 3, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, dt_eres_get(0x8009));
        break;
    case 6:
        list = dt_draw_type_icon(list, x + 5, y + 5, v);
        break;
    case 7:
        list = dt_draw_kind_icon(list, x + 5, y + 5, v);
        break;
    case 8:
        list = dt_draw_attr_icon(list, x + 5, y + 5, v);
        break;
    case 9:
        list = dt_draw_pack_icon(list, x + 5, y + 5, v);
        break;
    }
    return list;
}
#endif

/* the editor frame (called by dt_draw before the lesson overlays) */
void dt_editor_draw(void)
{
    void *list = dl_select_layer(0, 0);

    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (!dtv.no_draw) {
        list = dl_select_layer(4, 0);
        list = dl_enable(list, 9);
        list = dl_enable(list, 4);
        list = dl_tex_func(list, 0, 1);
        list = dl_set_tex_filter(list, 0, 0);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_disable(list, 1);
        list = dt_draw_bg(list);
        list = dt_draw_info(list, &dtv.cursor);
        list = dt_draw_panes(list);
        list = dt_draw_overlay(list);
        dl_set_list(list);
        list = dl_select_layer(5, 0);
        list = dl_disable(list, 1);
        fade_apply(&list);
        dl_set_list(list);
    }
}

#ifdef NON_MATCHING
/* focus frame and help window (help position away from the cursor), then the state's popup */
void *dt_draw_overlay(void *list)
{
    ScRect *f = &dts.focus;
    int hx;
    int hy;
    void *gim;

    switch (dts.state) {
    case 6:
    case 7:
        if (dtv.mode < 2) {
            if (dtv.pane[dtv.side].list_mode == 1)
                hx = 0xD0;
            else if (dtv.pane[dtv.side].lay.x + dtv.pane[dtv.side].lay.w / 2 < f->x + f->w)
                hx = dtv.pane[dtv.side].lay.x + 2;
            else
                hx = 0xD0;
            if (f->y + f->h / 2 < 0x89)
                hy = 0xA2;
            else
                hy = 0x36;
        } else if (dtv.mode < 4) {
            hx = dtv.pane[0].lay.x + 2;
            if (f->y + f->h / 2 < 0x89)
                hy = 0xA2;
            else
                hy = 0x36;
        } else {
            if (dtv.side == 0)
                hx = 0xFE;
            else
                hx = 0x0E;
            hy = 0x70;
        }
    case 0:
        gim = dt_eres_get(0x8006);
        list = dt_draw_frame(list, f->x, f->y, f->w, f->h, gim, gfx_get_state()[0]);
    }
    switch (dts.state) {
    case 1:
        list = dt_draw_sort_menu(list);
        break;
    case 2:
        list = dt_draw_action_menu(list);
        break;
    case 3:
        list = dt_draw_search(list);
        break;
    case 4:
        list = dt_draw_confirm(list);
        break;
    case 5:
        list = dt_draw_result(list);
        break;
    case 6:
        list = dt_draw_help(list, hx, hy, 0, 0xF001, dt_eres_get(0x8013), dt_eres_get(0x8033));
        list = dt_draw_detail(list);
        break;
    case 7:
        list = dt_draw_help(list, hx, hy, 1, 0xF001, dt_eres_get(0x8013), dt_eres_get(0x8033));
        break;
    }
    if (dtv.popup != 0)
        list = dt_draw_popup(list);
    return list;
}
#endif

void *dt_draw_bg(void *list)
{
    list = dt_draw_bg_ehf(list);
    list = dt_draw_window(list);
    list = dt_draw_grid_lines(list, dtv.mode);
    list = dt_draw_title(list);
    list = dt_draw_headers(list);
    list = dt_draw_type_counts(list);
    return dt_draw_type_bar(list);
}

/* animated background (bg.ehf) */
void *dt_draw_bg_ehf(void *list)
{
    void *e = dt_eres_get(0x8021);
    unsigned int opts[24];

    if (e == 0)
        return list;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()[0];
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, opts);
    list = gfx_end_2d(list);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    return dl_blend_func(list, 0, 2, 3, 0, 0);
}

/* the window (d_window0.ehf) */
void *dt_draw_window(void *list)
{
    void *e = dt_eres_get(0x8022);
    unsigned int opts[24];

    if (e == 0)
        return list;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()[0];
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, opts);
    return gfx_end_2d(list);
}

short dt_count_x[6] = { 0x19D, 0x1AA, 0x1AA, 0x1AA, 0x1AA, 0x1AA };   /* by cfg_get_language() */

/* deck name and the number of cards in the deck */
void *dt_draw_title(void *list)
{
    font_text_set(list);
    font_set_size(12, 12);
    font_set_color(0xFFFFFFFF);
    font_draw_wtext(0x2980, 0x140, 0x1BC0, dts.name);
    font_set_size(11, 11);
    font_draw_wtext(0x7040, 0x300, 0x740, table_get_entry(dt_eres_get(0x8033), 0x6B));
    list = font_text_clear();
    font_set_size(10, 12);
    font_set_color(0xFF000000);
    return dt_draw_num_text(list, dt_count_x[cfg_get_language()], 5, dts.n_deck, 8, 4, 0);
}

/* separator lines under the rows of the list panes */
void *dt_draw_grid_lines(void *list, unsigned short mode)
{
    DtLineVtx *v = dl_alloc_inline(&list, 0x240);
    DtLineVtx *p = v;
    unsigned short n = 0;
    unsigned short y;
    int i;

    if (mode == 0) {
        for (y = 41, i = 0; i < 7; i++, p += 4) {
            dt_line(p, 151, y, 317);
            y += 30;
            n += 4;
        }
    }
    if (mode == 4 || mode == 6) {
        for (y = 41, i = 0; i < 6; i++, p += 4) {
            dt_line(p, 3, y, 225);
            y += 30;
            n += 4;
        }
    }
    if (mode == 4 || mode == 7) {
        for (y = 41, i = 0; i < 6; i++, p += 4) {
            dt_line(p, 243, y, 225);
            y += 30;
            n += 4;
        }
    }
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_disable(list, 9);
    list = dl_draw_array(list, 1, 0x80011C, n, 0, v);
    list = dl_enable(list, 9);
    return list;
}

/* a dark line w wide at (x, y) with a lighter one under it */
void dt_line(DtLineVtx *v, unsigned short x, unsigned short y, unsigned short w)
{
    v[0].x = x;
    v[0].y = y;
    v[0].z = 0;
    v[0].color = 0x33000000;
    v[1].x = x + w;
    v[1].y = y;
    v[1].z = 0;
    v[1].color = 0x33000000;
    v[2].x = x;
    v[2].y = y + 1;
    v[2].z = 0;
    v[2].color = 0x19000000;
    v[3].x = x + w;
    v[3].y = y + 1;
    v[3].z = 0;
    v[3].color = 0x19000000;
}

#ifdef NON_MATCHING
/* header of each pane: sort and direction icons, filter icon with the number of cards, "new" mark of
   the owned list, scroll bar */
void *dt_draw_headers(void *list)
{
    ScPane *p;
    ScList *l;
    int x;
    int y;
    int show;
    unsigned int mark;
    int style;
    unsigned short cols;

    p = &dtv.pane[0];
    l = p->list;
    x = p->lay.x;
    y = p->lay.y + p->lay.h;
    list = dt_draw_sort_icon(list, l->sort, x + 3, y - 18);
    list = dt_draw_dir_icon(list, l->dir, x + 21, y - 18);
    if (l->filter != 0) {
        show = 1;
        if (dtv.popup == 6 && dtv.side == 0 && (gfx_get_state()[0] >> 2 & 1))
            show = 0;
        if (show) {
            list = dt_draw_filter_icon(list, l->filter, x + 40, y - 18);
            if (l->count < 10)
                x += 38;
            else if (l->count < 100)
                x += 45;
            else if (l->count < 1000)
                x += 52;
            else
                x += 59;
            list = dt_draw_digits(list, PT(x, y - 14), 0, l->count, 4, 0xFFFFFFFF, dt_eres_get(0x8009));
        }
    }
    if (l->tab == 3 && (mark = cfg_get_byte0_hi2()) != 0)
        list = dt_draw_flag_icon(list, PT(p->lay.x + p->lay.w - 20, y - 15), mark, dt_eres_get(0x800A));
    style = dtv.side != 0;
    if (l->tab > 2)
        style += 2;
    if (p->list_mode == 0) {
        cols = p->lay.cols;
        list = dt_draw_scrollbar(list, p->lay.x + p->lay.w - 10, p->lay.y + 12, p->lay.h - 41, p->top / cols,
                                 p->lay.grid_rows, (l->count + cols - 1) / cols, style, dt_eres_get(0x8007));
    } else {
        list = dt_draw_scrollbar(list, p->lay.x + p->lay.w - 10, p->lay.y + 12, p->lay.h - 41, p->top,
                                 p->lay.rows, l->count, style, dt_eres_get(0x8007));
    }
    if (dtv.mode > 1) {
        p = &dtv.pane[1];
        l = p->list;
        x = p->lay.x;
        y = p->lay.y + p->lay.h;
        list = dt_draw_sort_icon(list, l->sort, x + 3, y - 18);
        list = dt_draw_dir_icon(list, l->dir, x + 21, y - 18);
        if (l->filter != 0) {
            show = 1;
            if (dtv.popup == 6 && dtv.side == 1 && (gfx_get_state()[0] >> 2 & 1))
                show = 0;
            if (show) {
                list = dt_draw_filter_icon(list, l->filter, x + 40, y - 18);
                if (l->count < 10)
                    x += 38;
                else if (l->count < 100)
                    x += 45;
                else if (l->count < 1000)
                    x += 52;
                else
                    x += 59;
                list = dt_draw_digits(list, PT(x, y - 14), 0, l->count, 4, 0xFFFFFFFF, dt_eres_get(0x8009));
            }
        }
        if (l->tab == 3 && (mark = cfg_get_byte0_hi2()) != 0)
            list = dt_draw_flag_icon(list, PT(p->lay.x + p->lay.w - 20, y - 15), mark, dt_eres_get(0x800A));
        style = dtv.side == 1 ? 2 : 3;
        if (p->list_mode == 0) {
            cols = p->lay.cols;
            list = dt_draw_scrollbar(list, p->lay.x + p->lay.w - 10, p->lay.y + 12, p->lay.h - 41, p->top / cols,
                                     p->lay.grid_rows, (l->count + cols - 1) / cols, style, dt_eres_get(0x8007));
        } else {
            list = dt_draw_scrollbar(list, p->lay.x + p->lay.w - 10, p->lay.y + 12, p->lay.h - 41, p->top,
                                     p->lay.rows, l->count, style, dt_eres_get(0x8007));
        }
    }
    return dt_draw_counts(list);
}
#endif

unsigned short dt_count_digits[6] = { 2, 2, 2, 6, 2, 1 };   /* digits of the copy count of each tab */

#ifdef NON_MATCHING
/* the total of copies on the tab buttons of each pane (tab 4 is hidden in one-pane modes) */
void *dt_draw_counts(void *list)
{
    ScPane *p = &dtv.pane[0];
    unsigned short sel = p->list->tab;
    unsigned int i;
    short x;
    ScRect r;

    if (dtv.mode < 2) {
        for (i = 0; i < 5; i++) {
            if (i != 4) {
                x = dt_tab_rect(i, sel, &r);
                r.x += dtv.pane[0].lay.x;
                list = dt_draw_count(list, x + dtv.pane[0].lay.x, &r, dt_count_digits[i], dts.list[i].total);
            }
        }
    } else {
        for (i = 0; i < 3; i++) {
            x = dt_tab_rect(i, sel, &r);
            r.x += dtv.pane[0].lay.x;
            list = dt_draw_count(list, x + dtv.pane[0].lay.x, &r, dt_count_digits[i], dts.list[i].total);
        }
        sel = dtv.pane[1].list->tab;
        for (; i < 5; i++) {
            x = dt_tab_rect(i, sel, &r);
            r.x += dtv.pane[1].lay.x;
            list = dt_draw_count(list, x + dtv.pane[1].lay.x, &r, dt_count_digits[i], dts.list[i].total);
        }
    }
    return list;
}
#endif

void *dt_draw_count(void *list, short x, ScRect *r, unsigned short ndig, int value)
{
    if (r->w <= 0 || r->h <= 0)
        return list;
    list = dl_scissor(list, r->x, r->y, r->w, r->h);
    list = dl_enable(list, 2);
    font_set_size(11, 12);
    font_set_color(0xFF000000);
    list = dt_draw_num_text(list, x, (short)(r->y + 1), value, 8, ndig, 0);
    return dl_disable(list, 2);
}

#ifdef NON_MATCHING
/* the number of cards of each type in the deck */
void *dt_draw_type_counts(void *list)
{
    int i;
    int y = 0x17;
    unsigned int x2 = 0x18;
    unsigned short c;
    unsigned int x;
    int yy;

    for (i = 0; i < 6; i++, y += 0x13, x2 += 0x15) {
        c = dts.n_type[i];
        if (dtv.mode < 4) {
            x = 0x7A;
            yy = y;
        } else {
            yy = 3;
            x = x2;
        }
        if (c < 10)
            x -= 3;
        list = dt_draw_digits(list, x | yy << 16, 0, c, 2, 0xFFFFFFFF, dt_eres_get(0x8009));
    }
    return list;
}
#endif

#ifdef NON_MATCHING
/* the type proportions of the deck as a bar (vertical in the deck modes) */
void *dt_draw_type_bar(void *list)
{
    unsigned int total = dts.n_type[0] + dts.n_type[1] + dts.n_type[2] + dts.n_type[3] + dts.n_type[4] + dts.n_type[5];
    int base;
    int len;
    int pos[7];
    int i;
    unsigned short *n;
    int *p;

    if (dtv.mode < 4) {
        base = 0x14;
        len = 0x6F;
    } else {
        base = 0x17;
        len = 0x7A;
    }
    memset(pos, 0, 0x18);
    if (total != 0) {
        unsigned int s1 = dts.n_type[0] + dts.n_type[1];
        unsigned int s2 = s1 + dts.n_type[2];
        unsigned int s3 = s2 + dts.n_type[3];
        unsigned int s4 = s3 + dts.n_type[4];

        pos[0] = base;
        pos[1] = base + len * dts.n_type[0] / total;
        pos[2] = base + len * s1 / total;
        pos[3] = base + len * s2 / total;
        pos[4] = base + len * s3 / total;
        pos[5] = base + len * s4 / total;
        pos[6] = base + len * (s4 + dts.n_type[5]) / total;
    }
    for (i = 0, n = dts.n_type, p = pos; i < 6; i++, n++, p++) {
        if (*n != 0) {
            if (dtv.mode < 4)
                list = dt_draw_type_seg(list, 0x8D, p[0], p[1] - p[0], 0, i);
            else
                list = dt_draw_type_seg(list, p[0], 0x12, p[1] - p[0], 3, i);
        }
    }
    return list;
}
#endif

#ifdef NON_MATCHING
/* one segment of the bar: a 3-texel strip of icon05.gim row 0x7A + kind, rotated by dir */
void *dt_draw_type_seg(void *list, int x, int y, int len, int dir, int kind)
{
    void *gim = dt_eres_get(0x800A);
    int w;

    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    w = 3;
    switch (dir) {
    case 1:
    case 3:
        w = len;
        len = 3;
        break;
    default:
        break;
    }
    list = draw_set_offsets(list, 3, 1, w, len);
    return draw_sprite(list, x, y, 0, 0, kind + 0x7A, 0, dir);
}
#endif

void *dt_draw_panes(void *list)
{
    ScPane *p = &dtv.pane[0];

    if (p->list_mode == 0)
        list = dt_draw_grid(list, p);
    else
        list = dt_draw_rows(list, p);
    p = &dtv.pane[1];
    if (dtv.mode > 1) {
        if (p->list_mode == 0)
            list = dt_draw_grid(list, p);
        else if (dtv.mode < 4)
            list = dt_draw_rows_small(list, p);
        else
            list = dt_draw_rows(list, p);
    }
    return list;
}

#ifdef NON_MATCHING
/* list mode: one card per row */
void *dt_draw_rows(void *list, ScPane *p)
{
    ScList *l = p->list;
    unsigned short tab = l->tab;
    unsigned short idx = p->top;
    short x = p->lay.x;
    short dx = p->lay.row_x;
    short y = p->lay.y + p->lay.row_y;
    unsigned short n = p->lay.rows;
    int i;
    ScEnt *e;

    if (n + idx >= l->count)
        n = l->count - idx;
    for (i = 0; i < n; i++) {
        e = SC_ENT(l, idx);
        list = dt_draw_row(list, x + dx, y, p->lay.row_w, e, tab != 5);
        idx++;
        y += p->lay.row_h;
    }
    return list;
}
#endif

#ifdef NON_MATCHING
/* grid mode: card pictures with their marks */
void *dt_draw_grid(void *list, ScPane *p)
{
    ScList *l = p->list;
    unsigned short tab = l->tab;
    unsigned short idx = p->top;
    short y = p->lay.y + p->lay.grid_y;
    int row = 0;
    int col;
    short x;
    ScEnt *e;

    if (p->lay.grid_rows == 0)
        return list;
    while (idx < l->count) {
        col = 0;
        x = p->lay.x + p->lay.grid_x;
        if (p->lay.cols != 0) {
            for (;;) {
                e = SC_ENT(l, idx);
                list = dt_draw_card_pict(list, x, y, 1, 0xFFFFFFFF, e, dt_eres_get(0x8001), dt_eres_get(0x8002), dt_eres_get(0x8032));
                list = dt_draw_card_marks(list, x, y, e, 1, 0, tab != 5, 0, dt_eres_get(0x8009), dt_eres_get(0x800A));
                idx++;
                x += p->lay.cell_w;
                if (idx >= l->count)
                    break;
                col++;
                if (col >= p->lay.cols)
                    break;
            }
        }
        row++;
        y += p->lay.cell_h;
        if (row >= p->lay.grid_rows)
            return list;
    }
    return list;
}
#endif

/* the three counters of a row (template of dt_draw_row's local) */
DtCounter dt_row_counters[3] = { { 1, { 0 }, 0xFF000000, 0 }, { 2, { 0 }, 0xFF000000, 0 }, { 4, { 0 }, 0xFF000000, 0 } };

#ifdef NON_MATCHING
/* a wide row: frame, picture, name, icons, level, ATK/DEF, then the counters of the three decks */
void *dt_draw_row(void *list, unsigned short x, unsigned short y, unsigned short w, ScEnt *e, int blink)
{
    unsigned short attr = card_get_attribute_icon(e->id);
    int full = 0;
    unsigned short tx;
    unsigned short cx;
    unsigned short type;
    unsigned short kind;
    int atk;
    int def;
    unsigned short level;
    unsigned int pos;
    unsigned short right;

    list = dt_draw_row_frame(list, PT(x + 23, y), w, attr, dt_eres_get(0x8006));
    list = dt_draw_card_pict(list, x, y, 1, 0xFFFFFFFF, e, dt_eres_get(0x8001), dt_eres_get(0x8002), dt_eres_get(0x8032));
    tx = x + 29;
    font_text_set(list);
    font_set_size(12, 12);
    font_set_color(0xFF000000);
    if (cfg_get_language() != 0) {
        font_set_flag4000(1);
        font_set_flag8(0);
    }
    font_draw_text(tx << 6, (y + 2) << 6, (w - 54) << 6, card_get_name(e->id));
    if (cfg_get_language() != 0) {
        font_set_flag4000(0);
        font_set_flag8(1);
    }
    list = font_text_clear();
    if (attr - 7U < 2) {
        type = card_get_prop5_hi(e->id);
        kind = card_get_kind(e->id);
        list = dt_draw_kind_icon(list, tx, y + 14, kind);
        list = dt_draw_attr_icon(list, tx + 18, y + 14, type);
    } else {
        type = card_get_type(e->id);
        kind = card_get_kind(e->id);
        atk = card_get_atk_q(e->id);
        def = card_get_def_q(e->id);
        level = card_get_level(e->id);
        list = dt_draw_kind_icon(list, tx, y + 14, kind);
        list = dt_draw_type_icon(list, tx + 18, y + 14, type);
        pos = (y + 16) << 16;
        list = dt_draw_level(list, (unsigned short)(tx + 37) | pos, level, 0xFF000000, dt_eres_get(0x8009));
        list = dt_draw_stat(list, atk << 16, (unsigned short)(tx + 73) | pos, 0xC0A, 0xFF000000, dt_eres_get(0x8009));
        list = dt_draw_stat(list, def << 16 | 1, (unsigned short)(tx + 133) | pos, 0xC0A, 0xFF000000, dt_eres_get(0x8009));
    }
    cx = tx + 193;
    right = x + w;
    if (right < cx + 50) {
        full = 1;
    } else {
        DtCounter c[3];

        c[0] = dt_row_counters[0];
        c[1] = dt_row_counters[1];
        c[2] = dt_row_counters[2];
        if (card_attribute_icon_is3(e->id)) {
            c[0].icon = 0;
            c[1].icon = 3;
            if (dt_list_has(&dts.list[1], e->id, &c[1].value))
                c[1].color = 0xFF0080FF;
        } else {
            if (dt_list_has(&dts.list[0], e->id, &c[0].value))
                c[0].color = 0xFF0080FF;
            if (dt_list_has(&dts.list[2], e->id, &c[1].value))
                c[1].color = 0xFF0080FF;
        }
        c[2].value = dt_list_count_id(&dts.list[3], e->id, 0);
        list = dt_draw_counters(list, PT(cx + 7, y + 14), c, dt_eres_get(0x8006), dt_eres_get(0x800A));
    }
    if (inv_get_flag7(e->id))
        list = dt_draw_flag_icon(list, PT(right - 24, y + 3), 2, dt_eres_get(0x800A));
    return dt_draw_card_marks(list, x, y, e, full, 0, blink, 0, dt_eres_get(0x8009), dt_eres_get(0x800A));
}
#endif

#ifdef NON_MATCHING
/* small rows of the second pane (modes 2 and 3) */
void *dt_draw_rows_small(void *list, ScPane *p)
{
    ScList *l = p->list;
    unsigned short tab = l->tab;
    unsigned short idx = p->top;
    short x = p->lay.x;
    short dx = p->lay.row_x;
    short y = p->lay.y + p->lay.row_y;
    unsigned short n = p->lay.rows;
    int i;
    ScEnt *e;

    if (n + idx >= l->count)
        n = l->count - idx;
    for (i = 0; i < n; i++) {
        e = SC_ENT(l, idx);
        list = dt_draw_row_small(list, x + dx, y, p->lay.row_w, e, tab != 5);
        idx++;
        y += p->lay.row_h;
    }
    return list;
}
#endif

#ifdef NON_MATCHING
void *dt_draw_row_small(void *list, unsigned short x, unsigned short y, unsigned short w, ScEnt *e, int blink)
{
    unsigned short attr = card_get_attribute_icon(e->id);
    int atk;
    int def;
    unsigned short level;
    unsigned short tx;

    list = dt_draw_row_frame(list, PT(x + 26, y), w, attr, dt_eres_get(0x8006));
    list = dt_draw_card_pict(list, x, y + 1, 1, 0xFFFFFFFF, e, dt_eres_get(0x8001), dt_eres_get(0x8002), dt_eres_get(0x8032));
    list = dt_draw_card_marks(list, x, y + 1, e, 1, 0, blink, 0, dt_eres_get(0x8009), dt_eres_get(0x800A));
    if (cfg_get_language() != 0)
        font_set_flag8(0);
    font_text_set(list);
    font_set_size(12, 12);
    font_set_color(0xFF000000);
    if (cfg_get_language() != 0) {
        font_set_flag4000(1);
        font_set_flag8(0);
    }
    font_draw_text((x + 27) << 6, y << 6, (w - 27) << 6, card_get_name(e->id));
    if (cfg_get_language() != 0) {
        font_set_flag4000(0);
        font_set_flag8(1);
    }
    list = font_text_clear();
    if (cfg_get_language() != 0)
        font_set_flag8(1);
    if (card_is_monster(e->id)) {
        atk = card_get_atk_q(e->id);
        def = card_get_def_q(e->id);
        level = card_get_level(e->id);
        tx = x + 25;
        list = dt_draw_stat(list, atk << 16 | 2, tx | (y + 13) << 16, 0xA09, 0xFF000000, dt_eres_get(0x8009));
        list = dt_draw_stat(list, def << 16 | 3, tx | (y + 23) << 16, 0xA09, 0xFF000000, dt_eres_get(0x8009));
        list = dt_draw_level(list, (unsigned short)(x + 69) | (y + 20) << 16, level, 0xFF000000, dt_eres_get(0x8009));
    }
    if (inv_get_flag7(e->id))
        list = dt_draw_flag_icon(list, (unsigned short)(x + 78) | (y + 13) << 16, 2, dt_eres_get(0x800A));
    return list;
}
#endif
