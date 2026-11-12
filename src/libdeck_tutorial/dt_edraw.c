// @range 0x08805BE4-0x08808C6C
// @rodata 0x08820F08-0x08820F78
// @asm 0x08805BE4-0x08805FC0
// @asm 0x088071CC-0x0880779C
// @asm 0x08807F60-0x08808098
// @asm 0x088082AC-0x088087EC
// @asm 0x088087EC-0x08808924
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
void *cardlist_build(void *list, DtScroll *s);
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
int dt_sort_value(DtEnt *e, unsigned short sort);
unsigned int dt_tab_pos(unsigned short tab);
unsigned short dt_tab_rect(unsigned short i, unsigned short sel, DtRect *r);
int dt_kind_icon(unsigned short kind);
int dt_list_count_id(DtList *l, unsigned short id, int sub);
int dt_list_count_fixed(DtList *l, unsigned short id, unsigned short *n);
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
void *dt_draw_card_pict(void *list, int x, int y, int shadow, unsigned int color, DtEnt *e, void *gim, void *gim_alt, ScIdList *alt);
void *dt_draw_card_marks(void *list, int x, int y, DtEnt *e, int show_count, int blink, int show_fixed, unsigned short rank, void *gim_digits, void *gim);
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
void *dt_flight_draw(void *list);
void *dt_action_draw(void *list);
void *dt_sort_menu_draw(void *list);
void *dt_fmenu_draw(void *list);
void *dt_draw_word(void *list);
void *dt_popup_draw(void *list);

void *dt_draw_info(void *list, DtCursor *c);
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
void *dt_draw_count();    /* K&R: the callers pass x zero-extended (unsigned short) */
void *dt_draw_type_counts(void *list);
void *dt_draw_type_bar(void *list);
void *dt_draw_type_seg(void *list, int x, int y, int len, int dir, int kind);
void *dt_draw_panes(void *list);
void *dt_draw_rows(void *list, DtPane *p);
void *dt_draw_grid(void *list, DtPane *p);
void *dt_draw_row(void *list, unsigned short x, unsigned short y, unsigned short w, DtEnt *e, int blink);
void *dt_draw_rows_small(void *list, DtPane *p);
void *dt_draw_row_small(void *list, unsigned short x, unsigned short y, unsigned short w, DtEnt *e, int blink);

#ifdef NON_MATCHING /* 3 palavras: list fica em $s1 no original (aqui divide $s0 com r); ordem das declarações não resolve */
/* the strip under the panes: the card list's own strip in the deck modes, else name, icons, level,
   ATK/DEF and the description scrolling sideways */
void *dt_draw_info(void *list, DtCursor *c)
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
        if (inv_get_flag7(c->id))
            r = dt_draw_flag_icon(r, PT(0x7B, 0x87), 2, dt_eres_get(0x800A));
    } else {
    if (c->id == 0)
        return list;
    r = dt_draw_kind_icon(list, 0x146, 0x100, card_get_kind(c->id));
    if (card_is_monster(c->id)) {
        r = dt_draw_type_icon(r, 0x158, 0x100, card_get_type(c->id));
        level = card_get_level(c->id);
        r = dt_draw_level(r, PT(0x16B, 0x103), level, 0xFFFFFFFF, dt_eres_get(0x8009));
        r = dt_draw_stat(r, (unsigned short)card_get_atk_q(c->id) << 16 | 2, PT(0x187, 0x102), 0xC0A, 0xFFFFFFFF, dt_eres_get(0x8009));
        r = dt_draw_stat(r, (unsigned short)card_get_def_q(c->id) << 16 | 3, PT(0x1B3, 0x102), 0xC0A, 0xFFFFFFFF, dt_eres_get(0x8009));
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
    x2 = font_get_cursor_x() + 0x40;
    font_draw_wtext(x2, 0x3CC0, 0x6B00 - x2, card_info_fill(c->id));
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
    r = dl_disable(r, 2);
    }
    return r;
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

/* darkens pane side and tab button tab */
void *dt_draw_shade(void *list, unsigned short side, unsigned short tab, int with_tab)
{
    DtLayout *lay = &dtv.pane[side].lay;
    unsigned int pos = dt_tab_pos(tab);

    list = dl_disable(list, 9);
    list = dl_tex_func(list, 4, 0);
    list = dl_ambient_color(list, 0x80000000);
    list = draw_set_offsets(list, 0, 0, lay->w, lay->h);
    list = draw_sprite(list, lay->x, lay->y, 0, 0, 0, 0, 0);
    if (with_tab) {
        list = draw_set_offsets(list, 0, 0, (pos >> 16) & 0xFFFF, 0xF);
        list = draw_sprite(list, lay->x + (pos & 0xFFFF), 0x17, 0, 0, 0, 0, 0);
    }
    list = dl_enable(list, 9);
    list = dl_tex_func(list, 0, 1);
    return dl_ambient_color(list, 0xFFFFFFFF);
}

/* the value of the sort key of the cursor's card, in a bubble beside the scroll bar */
void *dt_draw_detail(void *list)
{
    DtPane *p = &dtv.pane[dtv.side];
    DtList *l = p->list;
    unsigned short i = dt_cur_index(dtv.side);
    DtEnt *e = SC_ENT(l, i);
    int sort = l->sort;
    unsigned int v;
    unsigned int sz;
    int x;
    int w;
    int r;
    int h;
    int cols;
    int hh;
    int y;
    int rows;
    int lw;

    if (e == 0)
        return list;
    v = dt_sort_value(e, sort);
    switch (sort) {
    case 3:
        w = 0x27;
        h = 0x14;
        break;
    case 4:
    case 5:
        w = 0x30;
        h = 0x13;
        break;
    case 1:
        font_set_size(14, 14);
        sz = font_sample_glyph_size();
        sz = (sz >> 16 & 0xFFFF) >> 6;
        w = sz + 13;
        h = sz + 11;
        break;
    case 2:
    case 6:
    case 7:
    case 8:
    case 9:
        w = 0x1B;
        h = 0x1B;
        break;
    }
    hh = p->lay.h - 0x2B;
    y = p->lay.y + 12;
    cols = p->lay.cols;
    rows = (l->count + (cols - 1)) / cols;
    r = i / cols;
    x = p->lay.x + p->lay.w - 10 - w;
    if (rows == 0)
        y += (hh - h) / 2;
    else
        y = y + hh * r / rows - h / 2;
    list = dt_draw_box(list, x, y, w, h, dt_eres_get(0x8013));
    if (v == 0xFFFFFFFE)
        return list;
    switch (sort) {
    case 0:
        break;
    case 1: {
        unsigned short ch[2];

        font_text_set(list);
        font_set_size(14, 14);
        font_set_color(0xFFFFFFFF);
        {
            unsigned char *d = (unsigned char *)ch;
            int n = 4;

            if (d)
                do {
                    *d++ = 0;
                } while (--n);
        }
        ch[0] = card_top_get_letter(v);
        lw = font_measure_text(ch);
        font_draw_text((x + 4 + (w - 9) / 2) * 64 - lw / 2, (y + 4) * 64, w << 6, ch);
        list = font_text_clear();
        break;
    }
    case 2:
        list = dt_draw_limit_icon(list, x + 5, y + 5, dt_kind_icon(v));
        break;
    case 9:
        list = dt_draw_pack_icon(list, x + 5, y + 5, v);
        break;
    case 3:
        list = dt_draw_level(list, PT(x + 5, y + 4), v, 0xFFFFFFFF, dt_eres_get(0x8009));
        break;
    case 4:
        if ((int)v > 0)
            v = (int)v / 100 * 100;
        else if (v == 0xFFFFFFFF)
            v = 0xFFFF;
        list = dt_draw_stat(list, (unsigned short)v << 16 | 2, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, dt_eres_get(0x8009));
        break;
    case 5:
        if ((int)v > 0)
            v = (int)v / 100 * 100;
        else if (v == 0xFFFFFFFF)
            v = 0xFFFF;
        list = dt_draw_stat(list, (unsigned short)v << 16 | 3, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, dt_eres_get(0x8009));
        break;
    case 8:
        list = dt_draw_attr_icon(list, x + 5, y + 5, v);
        break;
    case 6:
        list = dt_draw_type_icon(list, x + 5, y + 5, v);
        break;
    case 7:
        list = dt_draw_kind_icon(list, x + 5, y + 5, v);
        break;
    }
    return list;
}

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

/* focus frame and help window (help position away from the cursor), then the state's popup */
void *dt_draw_overlay(void *list)
{
    DtRect *f = &dts.focus;
    int hx;
    int hy;

    switch (dts.state) {
    case 6:
    case 7:
        if (dtv.mode > 1) {
            if (dtv.mode < 4) {
                hx = dtv.pane[0].lay.x + 2;
                if (f->y + f->h / 2 > 136)
                    hy = 54;
                else
                    hy = 162;
            } else {
                if (dtv.side == 0)
                    hx = 254;
                else
                    hx = 14;
                hy = 112;
            }
        } else {
            /* the view shifted by side panes: pane[0] is the cursor's pane */
            DtView *v = (DtView *)((char *)&dtv + dtv.side * sizeof(DtPane));

            if (v->pane[0].list_mode == 1)
                hx = 208;
            else if (f->x + f->w > v->pane[0].lay.x + v->pane[0].lay.w / 2)
                hx = v->pane[0].lay.x + 2;
            else
                hx = 208;
            if (f->y + f->h / 2 > 136)
                hy = 54;
            else
                hy = 162;
        }
    case 0:
        list = dt_draw_frame(list, f->x, f->y, f->w, f->h, dt_eres_get(0x8006), *(int *)gfx_get_state());
    }
    switch (dts.state) {
    case 0:
        break;
    case 1:
        list = dt_flight_draw(list);
        break;
    case 2:
        list = dt_action_draw(list);
        break;
    case 3:
        list = dt_sort_menu_draw(list);
        break;
    case 4:
        list = dt_fmenu_draw(list);
        break;
    case 5:
        list = dt_draw_word(list);
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
        list = dt_popup_draw(list);
    return list;
}


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
    return dt_draw_num_text(list, dt_count_x[cfg_get_language()], 5, dts.n_owned, 8, 4, 0);
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

#ifdef NON_MATCHING /* gêmea de dk_draw_headers, mesma forma (203 palavras): falta a alocação dos $s e o id u16 na pilha */
/* header of each pane: sort and direction icons, filter icon with the number of cards, "new" mark of
   the owned list, scroll bar */
void *dt_draw_headers(void *list)
{
    DtPane *p;
    DtList *l;
    DtLayout *lay;
    int x;
    int fx;
    int y;
    int by;
    int h;
    int sx;
    int show;
    int mark;
    int style;
    unsigned short id;
    unsigned short cols;

    p = &dtv.pane[0];
    l = p->list;
    lay = &p->lay;
    x = p->lay.x + 3;
    y = p->lay.y + p->lay.h - 18;
    list = dt_draw_sort_icon(list, l->sort, x, y);
    list = dt_draw_dir_icon(list, l->dir, x + 18, y);
    if (l->filter != 0) {
        show = 1;
        if (dtv.popup == 6 && dtv.side == 0 && (gfx_get_state()[0] >> 2 & 1))
            show = 0;
        if (show) {
            list = dt_draw_filter_icon(list, l->filter, x + 37, y);
            x += 56;
            if (l->count < 10)
                x -= 21;
            else if (l->count < 100)
                x -= 14;
            else if (l->count < 1000)
                x -= 7;
            list = dt_draw_digits(list, PT(x, y + 4), 0, l->count, 4, 0xFFFFFFFF, dt_eres_get(0x8009));
        }
    }
    fx = lay->x + lay->w - 20;
    if (l->tab == 3 && (mark = cfg_get_byte0_hi2()) != 0)
        list = dt_draw_flag_icon(list, PT(fx, y + 3), mark, dt_eres_get(0x800A));
    h = lay->h - 41;
    by = lay->y + 12;
    sx = lay->x + lay->w - 10;
    if (dtv.side == 0)
        style = 0;
    else
        style = 1;
    if (l->tab >= 3)
        style += 2;
    if (p->list_mode == 0) {
        cols = lay->cols;
        list = dt_draw_scrollbar(list, sx, by, h, p->top / cols, lay->grid_rows, (l->count + (cols - 1)) / cols, style,
                                 dt_eres_get(id = 0x8007));
    } else {
        list = dt_draw_scrollbar(list, sx, by, h, p->top, lay->rows, l->count, style, dt_eres_get(id = 0x8007));
    }
    if (dtv.mode > 1) {
        p = &dtv.pane[1];
        l = p->list;
        lay = &p->lay;
        x = p->lay.x + 3;
        y = p->lay.y + p->lay.h - 18;
        list = dt_draw_sort_icon(list, l->sort, x, y);
        list = dt_draw_dir_icon(list, l->dir, x + 18, y);
        if (l->filter != 0) {
            show = 1;
            if (dtv.popup == 6 && dtv.side == 1 && (gfx_get_state()[0] >> 2 & 1))
                show = 0;
            if (show) {
                list = dt_draw_filter_icon(list, l->filter, x + 37, y);
                x += 56;
                if (l->count < 10)
                    x -= 21;
                else if (l->count < 100)
                    x -= 14;
                else if (l->count < 1000)
                    x -= 7;
                list = dt_draw_digits(list, PT(x, y + 4), 0, l->count, 4, 0xFFFFFFFF, dt_eres_get(0x8009));
            }
        }
        fx = lay->x + lay->w - 20;
        if (l->tab == 3 && (mark = cfg_get_byte0_hi2()) != 0)
            list = dt_draw_flag_icon(list, PT(fx, y + 3), mark, dt_eres_get(0x800A));
        h = lay->h - 41;
        by = lay->y + 12;
        sx = lay->x + lay->w - 10;
        style = dtv.side == 1 ? 2 : 3;
        if (p->list_mode == 0) {
            cols = lay->cols;
            list = dt_draw_scrollbar(list, sx, by, h, p->top / cols, lay->grid_rows, (l->count + (cols - 1)) / cols, style,
                                     dt_eres_get(id));
        } else {
            list = dt_draw_scrollbar(list, sx, by, h, p->top, lay->rows, l->count, style, dt_eres_get(id));
        }
    }
    return dt_draw_counts(list);
}
#endif

unsigned short dt_count_digits[6] = { 2, 2, 2, 6, 2, 1 };   /* digits of the copy count of each tab */

/* the total of copies on the tab buttons of each pane (tab 4 is hidden in one-pane modes) */
void *dt_draw_counts(void *list)
{
    DtPane *p = &dtv.pane[0];
    unsigned short sel = p->list->tab;
    int i;
    unsigned short x;
    DtRect r;
#ifdef __MWERKS__ /* declaração só para o matching; o port usa o protótipo do arquivo */
    extern void *dt_draw_count(void *list, unsigned short x, DtRect *r, unsigned short ndig, int value);
#endif

    if (dtv.mode < 2) {
        for (i = 0; i < 5; i++) {
            if (i == 4)
                continue;
            x = dt_tab_rect(i, sel, &r);
            x += p->lay.x;
            r.x += p->lay.x;
            list = dt_draw_count(list, x, &r, dt_count_digits[i], dts.list[i].total);
        }
    } else {
        for (i = 0; i < 3; i++) {
            x = dt_tab_rect(i, sel, &r);
            x += p->lay.x;
            r.x += p->lay.x;
            list = dt_draw_count(list, x, &r, dt_count_digits[i], dts.list[i].total);
        }
        p = &dtv.pane[1];
        sel = p->list->tab;
        for (; i < 5; i++) {
            x = dt_tab_rect(i, sel, &r);
            x += p->lay.x;
            r.x += p->lay.x;
            list = dt_draw_count(list, x, &r, dt_count_digits[i], dts.list[i].total);
        }
    }
    return list;
}

void *dt_draw_count(list, x, r, ndig, value)
void *list;
short x;
DtRect *r;
unsigned short ndig;
int value;
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

/* the number of cards of each type in the deck */
void *dt_draw_type_counts(void *list)
{
    int i;
    int x;
    int v;
    int y;

    for (i = 0; i < 6; i++) {
        v = dts.n_type[i];
        if (dtv.mode < 4) {
            x = 122;
            y = i * 19 + 23;
        } else {
            x = i * 21 + 24;
            y = 3;
        }
        if (v < 10)
            x -= 3;
        list = dt_draw_digits(list, PT(x, y), 0, v, 2, 0xFFFFFFFF, dt_eres_get(0x8009));
    }
    return list;
}

/* the type proportions of the deck as a bar (vertical in the deck modes) */
void *dt_draw_type_bar(void *list)
{
    int total;
    int base;
    int len;
    int pos[7];
    int i;
    int sum;

    total = 0;
    for (i = 0; i < 6; i++)
        total += dts.n_type[i];
    if (dtv.mode < 4) {
        base = 0x14;
        len = 0x6F;
    } else {
        base = 0x17;
        len = 0x7A;
    }
    memset(pos, 0, 0x18);
    if (total > 0) {
        sum = 0;
        pos[0] = base;
        for (i = 0; i < 6; i++) {
            sum += dts.n_type[i];
            pos[i + 1] = base + len * sum / total;
        }
    }
    for (i = 0; i < 6; i++) {
        if (dts.n_type[i] != 0) {
            if (dtv.mode < 4)
                list = dt_draw_type_seg(list, 0x8D, pos[i], pos[i + 1] - pos[i], 0, i);
            else
                list = dt_draw_type_seg(list, pos[i], 0x12, pos[i + 1] - pos[i], 3, i);
        }
    }
    return list;
}

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
        w = 3;
        break;
    }
    list = draw_set_offsets(list, 3, 1, w, len);
    return draw_sprite(list, x, y, 0, 0, kind + 0x7A, 0, dir);
}

void *dt_draw_panes(void *list)
{
    DtPane *p = &dtv.pane[0];

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

#ifdef NON_MATCHING /* 12 palavras: o original calcula e (SC_ENT) com bnezl e só depois carrega row_w; aqui a carga de row_w vai para o delay do bnez */
/* list mode: one card per row */
void *dt_draw_rows(void *list, DtPane *p)
{
    DtList *l = p->list;
    DtLayout *lay = &p->lay;
    int blink = 1;
    unsigned short n;
    unsigned short x;
    unsigned short y;
    unsigned short idx;
    int i;
    DtEnt *e;

    if (l->tab == 5)
        blink = 0;
    idx = p->top;
    x = lay->x + lay->row_x;
    y = lay->y + lay->row_y;
    n = lay->rows;
    if (n + idx >= l->count)
        n = l->count - idx;
    for (i = 0; i < n; i++) {
        e = SC_ENT(l, idx);
        list = dt_draw_row(list, x, y, lay->row_w, e, blink);
        idx++;
        y += lay->row_h;
    }
    return list;
}
#endif

/* grid mode: card pictures with their marks */
void *dt_draw_grid(void *list, DtPane *p)
{
    DtList *l = p->list;
    DtLayout *lay = &p->lay;
    int blink = 1;
    unsigned short x;
    unsigned short idx;
    unsigned short y;
    int row;
    int col;
    DtEnt *e;

    if (l->tab == 5)
        blink = 0;
    y = lay->y + lay->grid_y;
    idx = p->top;
    for (row = 0; row < lay->grid_rows; row++) {
        if (idx >= l->count)
            break;
        x = lay->x + lay->grid_x;
        for (col = 0; col < lay->cols; col++) {
            e = SC_ENT(l, idx);
            list = dt_draw_card_pict(list, x, y, 1, 0xFFFFFFFF, e, dt_eres_get(0x8001), dt_eres_get(0x8002), dt_eres_get(0x8032));
            list = dt_draw_card_marks(list, x, y, e, 1, 0, blink, 0, dt_eres_get(0x8009), dt_eres_get(0x800A));
            x += lay->cell_w;
            idx++;
            if (idx >= l->count)
                break;
        }
        y += lay->cell_h;
    }
    return list;
}

/* the three counters of a row (template of dt_draw_row's local) */
DtCounter dt_row_counters[3] = { { 1, { 0 }, 0xFF000000, 0 }, { 2, { 0 }, 0xFF000000, 0 }, { 4, { 0 }, 0xFF000000, 0 } };

#ifdef NON_MATCHING
/* a wide row: frame, picture, name, icons, level, ATK/DEF, then the counters of the three decks */
void *dt_draw_row(void *list, unsigned short x, unsigned short y, unsigned short w, DtEnt *e, int blink)
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
            if (dt_list_count_fixed(&dts.list[1], e->id, (unsigned short *)&c[1].value))
                c[1].color = 0xFF0080FF;
        } else {
            if (dt_list_count_fixed(&dts.list[0], e->id, (unsigned short *)&c[0].value))
                c[0].color = 0xFF0080FF;
            if (dt_list_count_fixed(&dts.list[2], e->id, (unsigned short *)&c[1].value))
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

#ifdef NON_MATCHING /* 12 palavras: como dt_draw_rows */
/* small rows of the second pane (modes 2 and 3) */
void *dt_draw_rows_small(void *list, DtPane *p)
{
    DtList *l = p->list;
    DtLayout *lay = &p->lay;
    int blink = 1;
    unsigned short n;
    unsigned short x;
    unsigned short y;
    unsigned short idx;
    int i;
    DtEnt *e;

    if (l->tab == 5)
        blink = 0;
    idx = p->top;
    x = lay->x + lay->row_x;
    y = lay->y + lay->row_y;
    n = lay->rows;
    if (n + idx >= l->count)
        n = l->count - idx;
    for (i = 0; i < n; i++) {
        e = SC_ENT(l, idx);
        list = dt_draw_row_small(list, x, y, lay->row_w, e, blink);
        idx++;
        y += lay->row_h;
    }
    return list;
}
#endif

void *dt_draw_row_small(void *list, unsigned short x, unsigned short y, unsigned short w, DtEnt *e, int blink)
{
    unsigned short attr = card_get_attribute_icon(e->id);
    unsigned short atk;
    unsigned short def;
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
        list = dt_draw_stat(list, atk << 16 | 2, PT(tx, y + 13), 0xA09, 0xFF000000, dt_eres_get(0x8009));
        list = dt_draw_stat(list, def << 16 | 3, PT(tx, y + 23), 0xA09, 0xFF000000, dt_eres_get(0x8009));
        list = dt_draw_level(list, PT(x + 69, y + 20), level, 0xFF000000, dt_eres_get(0x8009));
    }
    if (inv_get_flag7(e->id))
        list = dt_draw_flag_icon(list, PT(x + 78, y + 13), 2, dt_eres_get(0x800A));
    return list;
}
