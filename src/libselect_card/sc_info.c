// @range 0x0880D54C-0x0880E64C
// @asm 0x0880D54C-0x0880D928
// @asm 0x0880DBFC-0x0880DD34
// @asm 0x0880DD34-0x0880E244
// @asm 0x0880E244-0x0880E51C
/* The card info strip under the panes (name, icons, level, ATK/DEF, scrolling description), the
   icon wrappers, a button icon with its label, the pane shade, the sort value bubble next to the
   cursor, and the card action menu (opened with Circle). */
#include "libselect_card.h"
#include "menu.h"

typedef struct ScButton {              /* sc_draw_button */
    unsigned short res;                /* resource id of the icon */
    unsigned char u, v;                /* +0x02 */
    unsigned short text;               /* +0x04 strTbl.bin entry */
} ScButton;


void font_set_flag4000(int on);
unsigned short *card_get_desc(int id);
unsigned short *card_info_fill(int id);
void *cardlist_build(void *list, ScScroll *s);
unsigned int font_sample_glyph_size(void);
unsigned short sc_cur_index(unsigned short side);
int sc_sort_value(ScEnt *e, unsigned int sort);
unsigned int sc_tab_pos(unsigned short i);
void *sc_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type);
void *sc_draw_attr_icon(void *list, unsigned short x, unsigned short y, unsigned short attr);
void *sc_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind);
void *sc_draw_limit_icon(void *list, unsigned short x, unsigned short y, unsigned short n);
void *sc_draw_pack_icon(void *list, unsigned short x, unsigned short y, unsigned short n);
void *sc_action_draw_items(void *list, int x, int y, int w, int h);
void sc_action_step0(void);
void sc_action_step1(void);
void sc_action_step2(void);
void sc_action_step3(void);
void sc_action_step4(void);
void sc_action_step5(void);
int sc_action_close(void);

#ifdef NON_MATCHING /* lista do parâmetro e do resultado em s trocados; atk/def mascarados no original */
/* the strip under the panes: the card list's own strip in one-pane modes, else name, icons, level,
   ATK/DEF and the description scrolling sideways */
void *sc_draw_info(void *list, ScCursor *c)
{
    void *r;
    int x;
    int w;
    int x2;
    unsigned short level;

    if (scv.mode < 4) {
        r = cardlist_build(list, &scv.cursor.scroll);
        if (c->id == 0)
            return r;
        if (inv_get_flag7(c->id) == 0)
            return r;
        r = sc_draw_flag_icon(r, PT(0x7B, 0x87), 2, res_get(0x800A));
        return r;
    }
    if (c->id == 0)
        return list;
    r = sc_draw_kind_icon(list, 0x146, 0x100, card_get_kind(c->id));
    if (card_is_monster(c->id)) {
        r = sc_draw_type_icon(r, 0x158, 0x100, card_get_type(c->id));
        level = card_get_level(c->id);
        r = sc_draw_level(r, PT(0x16B, 0x103), level, 0xFFFFFFFF, res_get(0x8009));
        r = sc_draw_stat(r, card_get_atk_q(c->id) << 16 | 2, PT(0x187, 0x102), 0xC0A, 0xFFFFFFFF, res_get(0x8009));
        r = sc_draw_stat(r, card_get_def_q(c->id) << 16 | 3, PT(0x1B3, 0x102), 0xC0A, 0xFFFFFFFF, res_get(0x8009));
    } else if (card_get_prop5_hi(c->id) != 0) {
        r = sc_draw_attr_icon(r, 0x158, 0x100, card_get_prop5_hi(c->id));
    }
    if (inv_get_flag7(c->id))
        r = sc_draw_flag_icon(r, PT(0x1CE, 0xF5), 2, res_get(0x800A));
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
    x = 0x180 - scv.scroll_pos;
    w = scv.scroll_pos + 0x5500;
    font_draw_text(x, 0x4080, w, card_get_desc(c->id));
    if (scv.scroll_max > 0) {
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

void *sc_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type)
{
    return sc_draw_icon_a(list, x | y << 16, type, res_get(0x8007));
}

void *sc_draw_attr_icon(void *list, unsigned short x, unsigned short y, unsigned short attr)
{
    return sc_draw_icon_b(list, x | y << 16, attr, res_get(0x8007));
}

void *sc_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind)
{
    return sc_draw_icon_c(list, x | y << 16, kind, res_get(0x8007));
}

void *sc_draw_limit_icon(void *list, unsigned short x, unsigned short y, unsigned short n)
{
    return sc_draw_icon_d(list, x | y << 16, n, res_get(0x800A));
}

void *sc_draw_pack_icon(void *list, unsigned short x, unsigned short y, unsigned short n)
{
    return sc_draw_icon_e(list, x | y << 16, n, res_get(0x800A));
}

/* a button icon and its label */
void *sc_draw_button(void *list, ScButton *b, int x, int y, unsigned int color)
{
    ScIconText t;

    memset(&t, 0, sizeof(t));
    t.w = 0x11;
    t.h = 0x11;
    t.u = b->u;
    t.v = b->v;
    t.gim = res_get(b->res);
    t.text = table_get_entry(res_get(0x8033), b->text);
    t.color = color;
    return sc_draw_icon_text(list, &t, x, y);
}

#ifdef NON_MATCHING /* campos do painel por índice; alocação diferente */
/* darkens pane side and tab button tab */
void *sc_draw_shade(void *list, unsigned short side, unsigned short tab)
{
    unsigned int pos = sc_tab_pos(tab);

    list = dl_disable(list, 9);
    list = dl_tex_func(list, 4, 0);
    list = dl_ambient_color(list, 0x80000000);
    list = draw_set_offsets(list, 0, 0, scv.pane[side].lay.w, scv.pane[side].lay.h);
    list = draw_sprite(list, scv.pane[side].lay.x, scv.pane[side].lay.y, 0, 0, 0, 0, 0);
    list = draw_set_offsets(list, 0, 0, pos >> 16, 0xF);
    list = draw_sprite(list, scv.pane[side].lay.x + (pos & 0xFFFF), 0x17, 0, 0, 0, 0, 0);
    list = dl_enable(list, 9);
    list = dl_tex_func(list, 0, 1);
    return dl_ambient_color(list, 0xFFFFFFFF);
}
#endif

#ifdef NON_MATCHING /* switch com tamanhos indefinidos e campos por índice; alocação diferente */
/* the value of the sort key of the cursor's card, in a bubble beside the scroll bar */
void *sc_draw_detail(void *list)
{
    unsigned int side = scv.side;
    ScList *l = scv.pane[side].list;
    unsigned short i = sc_cur_index(side);
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
    unsigned short letter[2];
    int lw;

    if (e == 0)
        return list;
    v = sc_sort_value(e, sort);
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
    cols = scv.pane[side].lay.cols;
    rows = (int)(l->count + cols - 1) / (int)cols;
    x = scv.pane[side].lay.x + scv.pane[side].lay.w - 10 - w;
    if (rows == 0) {
        y = scv.pane[side].lay.y + 12 + (scv.pane[side].lay.h - 0x2B - h) / 2;
    } else {
        half = h / 2;
        y = scv.pane[side].lay.y + 12 + (scv.pane[side].lay.h - 0x2B) * (i / cols) / rows - half;
    }
    list = sc_draw_box(list, x, y, w, h, res_get(0x8013));
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
        list = sc_draw_limit_icon(list, x + 5, y + 5, sc_kind_icon(v));
        break;
    case 3:
        list = sc_draw_level(list, PT(x + 5, y + 4), v, 0xFFFFFFFF, res_get(0x8009));
        break;
    case 4:
        if ((int)v > 0)
            v = (int)v / 100 * 100;
        else if (v == 0xFFFFFFFF)
            v = 0xFFFF;
        list = sc_draw_stat(list, v << 16 | 2, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, res_get(0x8009));
        break;
    case 5:
        if ((int)v > 0)
            v = (int)v / 100 * 100;
        else if (v == 0xFFFFFFFF)
            v = 0xFFFF;
        list = sc_draw_stat(list, v << 16 | 3, PT(x + 5, y + 4), 0xA09, 0xFFFFFFFF, res_get(0x8009));
        break;
    case 6:
        list = sc_draw_type_icon(list, x + 5, y + 5, v);
        break;
    case 7:
        list = sc_draw_kind_icon(list, x + 5, y + 5, v);
        break;
    case 8:
        list = sc_draw_attr_icon(list, x + 5, y + 5, v);
        break;
    case 9:
        list = sc_draw_pack_icon(list, x + 5, y + 5, v);
        break;
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* gravações dos campos do menu em outra ordem */
/* opens the card action menu on e (or the reduced one on an empty slot) */
void sc_action_open(ScEnt *e)
{
    unsigned short count = 7;
    short extra = 8;
    MenuWidget *m;
    int x;

    sc_action = heap_alloc(sc.heap, 0x18);
    if (sc_action == 0)
        return;
    memset(sc_action, 0, 0x18);
    sc_action->ent = e;
    if (e == 0) {
        count = 5;
        extra = 4;
        sc_action->no_ent = 1;
    }
    sc_action->menu = heap_alloc(sc.heap, 0x120);
    m = sc_action->menu;
    if (m == 0)
        return;
    memset(m, 0, 0x120);
    m->def.visible = 1;
    m->def.style = 0;
    m->def.y = 0x3A;
    m->def.color = 0xFFFFFFFF;
    m->def.auto_size = 1;
    m->def.show_title = 1;
    m->def.max_w = 2;
    m->def.min_w = 2;
    m->def.max_h = 2;
    m->def.min_h = 2;
    m->def.font_flag4 = 1;
    m->def.font_flag8000 = 1;
    if (sc.kind == 0)
        m->def.title = table_get_entry(res_get(0x8033), 0x8F);
    else
        m->def.title = table_get_entry(res_get(0x8033), 0x90);
    m->def.style2 = 2;
    m->def.title_font_size = 0xC;
    m->def.title_color = 0xFFFFFFFF;
    m->def.open_anim = 2;
    m->def.close_anim = 2;
    m->heap = sc.heap;
    m->no_cancel = 1;
    m->keep_open = 1;
    m->style = 2;
    m->count = count;
    m->item_w = 0xD4;
    m->item_h = 0x13;
    m->draw_all = sc_action_draw_items;
    if (sc.nhist == 0)
        m->disabled |= 0x40;
    if (scv.pane[scv.side].list->filter == 0)
        m->disabled |= 0x20;
    menu_open(m);
    if (m->def.w < 0xD4)
        m->def.w = 0xD4;
    m->def.h += extra;
    x = 0x1E0 - m->def.w;
    m->def.x = x / 2;
    sc.state = 3;
}
#endif

/* state 3 */
int sc_input_action_menu(void)
{
    switch (sc_action->step) {
    case 0:
        sc_action_step0();
        break;
    case 1:
        sc_action_step1();
        break;
    case 2:
        sc_action_step2();
        break;
    case 10:
        return sc_action_close();
    case 3:
        sc_action_step3();
        break;
    case 4:
        sc_action_step4();
        break;
    case 5:
        sc_action_step5();
        break;
    }
    return 0;
}

void *sc_draw_action_menu(void *list)
{
    menu_draw(&list, sc_action->menu);
    if (sc_action->sub != 0)
        menu_draw(&list, sc_action->sub);
    return list;
}
