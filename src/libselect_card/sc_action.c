// @range 0x0880E64C-0x0880FFCC
// @asm 0x0880E95C-0x0880EA84
// @asm 0x0880EA84-0x0880EC54
// @asm 0x0880EC54-0x0880ECD4
// @asm 0x0880F678-0x0880F808
// @asm 0x0880FA68-0x0880FB4C
// @data 0x0881B378-0x0881B3DC
/* The card action menu and its sub-menus: favourite mark, card info, sort/filter submenu, clear
   or reset the list, pane layout picker (2x4 grid of layouts), "new" mark picker, the items drawn
   as button icons with labels. */
#include "libselect_card.h"
#include "menu.h"

typedef struct ScButton {              /* sc_draw_button */
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} ScButton;

typedef struct ScLineVtx {             /* see sc_draw.c */
    unsigned int color;
    short x, y, z;
    short pad;
} ScLineVtx;

int cfg_get_byte0_hi2(void);
void cfg_set_byte0_hi2(int v);
void *sc_draw_button(void *list, ScButton *b, int x, int y, unsigned int color);
void sc_lists_reset(void);
void sc_lists_clear(void);
int sc_input_move(void);
void sc_input_sort(void);
void sc_input_filter(void);
void sc_input_mark(void);
int sc_mode_for(unsigned short panes, int narrow, unsigned short a, unsigned short b);
void sc_set_mode_u(int mode);
void sc_set_mode(unsigned short mode);
void sc_action_free(void);
int sc_sub_open(short title, short text, unsigned short count, MenuDrawItemFn draw);
void sc_action_menu_sub(void);
void sc_action_clear_open(void);
void sc_action_layout_open(void);
void sc_action_mark_open(void);
void sc_action_confirm_open(void);
void *sc_draw_rule(void *list, ScLineVtx *v, int x, int y, int w);
void *sc_draw_action_item(void *list, unsigned short i, int x, int y);
void *sc_draw_layouts(void *list, int x, int y);
void *sc_draw_sub_item(void *list, unsigned short i, int x, int y);
void *sc_draw_clear_item(void *list, unsigned short i, int x, int y);
void *sc_draw_mark_item(void *list, unsigned short i, int x, int y);

void sc_action_free(void)
{
    if (sc_action->sub != 0) {
        menu_reset(sc_action->sub);
        heap_set_level(sc.heap, sc_action->sub);
        sc_action->sub = 0;
    }
    menu_reset(sc_action->menu);
    if (sc_action->menu != 0) {
        heap_set_level(sc.heap, sc_action->menu);
        sc_action->menu = 0;
    }
    heap_set_level(sc.heap, sc_action);
    sc_action = 0;
}

/* step 0: the action menu itself */
void sc_action_step0(void)
{
    int r = menu_update(sc_action->menu);
    unsigned short id;

    if (r == -1)
        return;
    if (r >= 0 && sc_action->no_ent == 1)
        r += 2;
    switch (r) {
    case -2:
    case 5:
    case 6:
        menu_finish(sc_action->menu);
        sc_action->step = 10;
        return;
    }
    sc_action->menu->no_cancel = 0;
    switch (r) {
    case 0:
        sc.info_open = 1;
        sc_action->menu->cancelled = sc_action->menu->accepted = 0;
        break;
    case 1:
        id = sc_action->ent->id;
        if (inv_get_flag7(id))
            inv_set_flag7(id, 0);
        else
            inv_set_flag7(id, 1);
        sc_action->menu->cancelled = sc_action->menu->accepted = 0;
        break;
    case 2:
        sc_action_menu_sub();
        se_play(0x6216);
        sc_action->step = 1;
        break;
    case 4:
        sc_action_layout_open();
        se_play(0x6216);
        sc_action->step = 3;
        break;
    case 3:
        sc_action_clear_open();
        se_play(0x6216);
        sc_action->step = 2;
        break;
    }
}

#ifdef NON_MATCHING /* o original duplica a gravação do estado depois do mover (bloco esvaziado pelo desvio likely) */
/* step 10: wait for the close, then run the chosen action */
int sc_action_close(void)
{
    int r = menu_update(sc_action->menu);
    ScList *l;

    if (sc_action->menu->state == 5)
        return 0;
    if (r >= 0 && sc_action->no_ent == 1)
        r += 2;
    sc_action_free();
    switch (r) {
    case 5:
        l = scv.pane[scv.side].list;
        sc_list_filter_reset(l);
        sc_list_sort(l, l->sort, l->dir);
        break;
    case 6:
        if (sc_input_move())
            return 0;
        break;
    }
    sc.state = 1;
    return 0;
}
#endif

#ifdef NON_MATCHING /* gravações dos campos do menu em outra ordem */
/* a sub-menu window beside the action menu */
int sc_sub_open(short title, short text, unsigned short count, MenuDrawItemFn draw)
{
    MenuWidget *m;

    if (sc_action->sub == 0)
        sc_action->sub = heap_alloc(sc.heap, 0x120);
    m = sc_action->sub;
    if (m == 0)
        return 0;
    memset(m, 0, 0x120);
    m->def.visible = 1;
    m->def.style = 0;
    m->def.x = sc_action->menu->def.x + 0x28;
    m->def.y = 0x7C;
    m->def.auto_size = 1;
    m->def.color = 0xFFFFFFFF;
    if (text != 0)
        m->def.text = table_get_entry(res_get(0x8033), text);
    m->def.font_size = 0xC;
    m->def.text_color = 0xFF000000;
    m->def.font_flag4 = 1;
    m->def.pad_l = 2;
    m->def.pad_t = 2;
    m->def.pad_r = 2;
    m->def.pad_b = 2;
    m->def.show_title = 1;
    m->def.auto_title = 1;
    if (title != 0)
        m->def.title = table_get_entry(res_get(0x8033), title);
    m->def.style2 = 2;
    m->def.title_font_size = 0xC;
    m->def.title_color = 0xFFFFFFFF;
    m->def.open_anim = 2;
    m->def.close_anim = 2;
    m->heap = sc.heap;
    m->no_cancel = 1;
    m->style = 2;
    m->count = count;
    m->item_w = 0xD4;
    m->item_h = 0x13;
    m->draw_item = draw;
    return 1;
}
#endif

#ifdef NON_MATCHING /* a contagem u16 vai para outro registrador */
/* the sort / filter / information submenu */
void sc_action_menu_sub(void)
{
    unsigned short n = 4;

    if (sc.kind == 0)
        n--;
    if (sc_sub_open(0x18, 0, n, (MenuDrawItemFn)sc_draw_sub_item)) {
        sc_action->sub->keep_open = 1;
        menu_open(sc_action->sub);
    }
}
#endif

/* step 1 */
void sc_action_step1(void)
{
    MenuWidget *m = sc_action->sub;
    int r = menu_update(m);

    if (r == -1)
        return;
    if (m->state == 3) {
        menu_finish(m);
        switch (r) {
        case 0:
        case 1:
        case 2:
            menu_finish(sc_action->menu);
            break;
        }
        return;
    }
    menu_reset(m);
    heap_set_level(sc.heap, sc_action->sub);
    sc_action->sub = 0;
    switch (r) {
    case 0:
        sc_input_sort();
        se_play(0x6216);
        sc_action_free();
        break;
    case 1:
        sc_input_filter();
        se_play(0x6216);
        sc_action_free();
        break;
    case 2:
        sc_input_mark();
        se_play(0x6216);
        sc_action_free();
        break;
    case 3:
        sc_action_mark_open();
        se_play(0x6216);
        sc_action->step = 4;
        break;
    case -2:
        sc_action->step = 0;
        sc_action->menu->cancelled = sc_action->menu->accepted = 0;
        break;
    }
}

void sc_action_clear_open(void)
{
    if (sc_sub_open(0x94, 0, 2, (MenuDrawItemFn)sc_draw_clear_item))
        menu_open(sc_action->sub);
}

/* step 2: clear the list (1) or reset it (0) */
void sc_action_step2(void)
{
    int done = 0;
    MenuWidget *m = sc_action->sub;
    int r = menu_update(m);

    if (r == -1)
        return;
    menu_reset(m);
    heap_set_level(sc.heap, sc_action->sub);
    sc_action->sub = 0;
    switch (r) {
    case 0:
        sc_lists_reset();
        sc.nhist = 0;
        done = 1;
        break;
    case 1:
        sc_lists_clear();
        sc.nhist = 0;
        done = 1;
        break;
    }
    if (done) {
        menu_finish(sc_action->menu);
        sc_action->step = 10;
    } else {
        sc_action->step = 0;
        sc_action->menu->cancelled = sc_action->menu->accepted = 0;
    }
}

/* the layout picker: 4 x 2 grid of pane layouts */
void sc_action_layout_open(void)
{
    MenuWidget *m;

    if (!sc_sub_open(0x1B, 0, 0, 0))
        return;
    m = sc_action->sub;
    m->def.auto_size = 0;
    m->def.w = 0x98;
    m->def.h = 0x30;
    m->def.x = sc_action->menu->def.x + 0x28;
    m->def.y = 0x7C;
    sc_action->mode = sc_mode_for(scv.mode > 1 ? 2 : 1, scv.mode < 4, scv.pane[0].list_mode, scv.pane[1].list_mode);
    m->cursor = sc_action->mode;
    if (scv.side == 0) {
        sc_action->side = 1;
        sc_action->tab = scv.pane[1].list->tab;
    } else {
        sc_action->side = 0;
        sc_action->tab = scv.pane[0].list->tab;
    }
    m->no_input = 1;
    m->draw_all = (MenuDrawRectFn)sc_draw_layouts;
    menu_open(m);
}

/* step 3: d-pad picks a layout (previewed at once), accept keeps it, cancel restores */
void sc_action_step3(void)
{
    MenuWidget *m = sc_action->sub;
    PadState *pad = pad_get();
    unsigned int rep;
    int c;

    menu_update(m);
    if (m->state == 6) {
        if (m->cancelled) {
            sc_set_mode_u(sc_action->mode % 8);
            menu_reset(m);
            heap_set_level(sc.heap, sc_action->sub);
            sc_action->sub = 0;
            sc_action->step = 0;
            sc_action->menu->cancelled = sc_action->menu->accepted = 0;
        } else {
            if (sc_action->mode > 1 && scv.mode < 2)
                sc_list_filter_reset(&sc.list[sc_action->tab]);
            menu_finish(sc_action->menu);
            sc_action->step = 10;
        }
        return;
    }
    if (pad->b.trig & region_pick_value(1)) {
        m->cancelled = 1;
        menu_finish(m);
        se_play(0x6214);
    } else if (pad->b.trig & region_pick_value(0)) {
        menu_set_cursor(m, m->cursor);
        se_play(0x6213);
    } else {
        rep = pad->b.rep;
        if (rep & 0x50) {
            m->cursor = (m->cursor + 4) % 8;
            sc_set_mode_u(m->cursor % 8);
            se_play(0x6212);
        } else if (rep & 0x80) {
            c = m->cursor;
            m->cursor = (c % 4 + 3) % 4 + (unsigned char)c / 4 * 4;
            sc_set_mode_u(m->cursor % 8);
            se_play(0x6212);
        } else if (rep & 0x20) {
            c = m->cursor;
            m->cursor = (c % 4 + 1) % 4 + (unsigned char)c / 4 * 4;
            sc_set_mode_u(m->cursor % 8);
            se_play(0x6212);
        }
    }
}

/* the "new" mark picker */
void sc_action_mark_open(void)
{
    MenuWidget *m;

    if (sc_sub_open(0x4B, 0x54, 3, (MenuDrawItemFn)sc_draw_mark_item)) {
        m = sc_action->sub;
        m->def.max_w = 0xF4;
        m->item_w = 0xF4;
        m->selected = 1 << cfg_get_byte0_hi2();
        menu_open(m);
    }
}

/* step 4 */
void sc_action_step4(void)
{
    MenuWidget *m = sc_action->sub;
    int r = menu_update(m);

    if (r == -1)
        return;
    menu_reset(m);
    switch (r) {
    case -2:
        sc_action_menu_sub();
        sc_action->step = 1;
        break;
    default:
        sc_action->mark = r;
        sc_action_confirm_open();
        se_play(0x621A);
        sc_action->step = 5;
        break;
    }
}

#ifdef NON_MATCHING /* gravações dos campos do menu em outra ordem */
/* yes/no window for the mark */
void sc_action_confirm_open(void)
{
    MenuWidget *m;
    int x;

    if (sc_action->sub == 0)
        sc_action->sub = heap_alloc(sc.heap, 0x120);
    m = sc_action->sub;
    if (m == 0)
        return;
    memset(m, 0, 0x120);
    m->def.visible = 1;
    m->def.style = 0;
    m->def.max_w = 0xF0;
    m->def.auto_size = 1;
    m->def.color = 0xFFFFFFFF;
    m->def.font_flag4 = 1;
    m->def.pad_l = 10;
    m->def.pad_t = 8;
    m->def.pad_r = 10;
    m->def.pad_b = 0;
    m->def.font_size = 0xC;
    m->def.text_color = 0xFF000000;
    m->def.text = table_get_entry(res_get(0x8033), 0x53);
    m->def.open_anim = 2;
    m->def.close_anim = 2;
    m->heap = sc.heap;
    m->no_cancel = 1;
    m->buttons = 3;
    m->initial = 3;
    m->cancel_resets = 1;
    menu_open(m);
    x = 0x1E0 - m->def.w;
    m->def.x = x / 2;
    x = 0x110 - m->def.h;
    m->def.y = x / 2;
}
#endif

/* step 5: yes stores the mark and drops the lists' marks */
void sc_action_step5(void)
{
    MenuWidget *m = sc_action->sub;
    int r = menu_update(m);
    ScList *l;
    int a;
    int b;

    if (r == -1)
        return;
    menu_reset(m);
    switch (r) {
    case -2:
    case 3:
        sc_action->step = 0;
        sc_action->menu->cancelled = sc_action->menu->accepted = 0;
        break;
    case 2:
        cfg_set_byte0_hi2(sc_action->mark);
        l = 0;
        a = 0;
        b = 0;
        switch (sc.kind) {
        case 1:
            l = &sc.list[0];
            if (scv.pane[0].list->tab == 0)
                a = 1;
            if (scv.pane[1].list->tab == 0)
                b = 1;
            break;
        case 2:
            l = &sc.list[0];
            if (scv.pane[0].list->tab == 0)
                a = 1;
            if (scv.pane[1].list->tab == 0)
                b = 1;
            break;
        }
        if (l != 0) {
            sc_list_filter_reset(l);
            if (a) {
                scv.pane[0].cy = 0;
                scv.pane[0].cx = 0;
                scv.pane[0].top = 0;
            }
            if (b) {
                scv.pane[1].cy = 0;
                scv.pane[1].cx = 0;
                scv.pane[1].top = 0;
            }
            l->u18 = l->u1a = l->u1c_lo = l->u1c_hi = 0;
        }
        heap_set_level(sc.heap, sc_action->sub);
        sc_action->sub = 0;
        menu_finish(sc_action->menu);
        sc_action->step = 10;
        break;
    }
}

#ifdef NON_MATCHING /* laço de vértices com outra alocação */
/* a two-tone rule under a menu item */
void *sc_draw_rule(void *list, ScLineVtx *v, int x, int y, int w)
{
    int i;
    ScLineVtx *p;

    for (i = 0, p = v; i < 4; i++, p++) {
        p->x = x + w * (short)(i % 2);
        p->z = 0;
        if (i < 2) {
            p->y = y;
            p->color = 0xFF479FC1;
        } else {
            p->y = y + 1;
            p->color = 0xFFA9D7E9;
        }
    }
    list = dl_disable(list, 9);
    list = dl_draw_array(list, 1, 0x80011C, 2, 0, v);
    list = dl_draw_array(list, 1, 0x80011C, 2, 0, v + 2);
    return dl_enable(list, 9);
}
#endif

/* the items of the action menu: card picture on top, then the buttons with rules */
void *sc_action_draw_items(void *list, int x, int y, int w, int h)
{
    MenuWidget *m = sc_action->menu;
    int first = 0;
    int n = 7;
    int i;
    int k;
    int dx;

    if (!sc_action->no_ent) {
        list = sc_draw_card_pict(list, x + 2, y + 2, 1, 0xFFFFFFFF, sc_action->ent, res_get(0x8001), res_get(0x8002), res_get(0x8032));
    } else {
        first = 2;
        n -= 2;
    }
    for (i = 0; i < n; i++) {
        k = i + first;
        dx = 0;
        switch (k) {
        case 0:
        case 1:
            dx += 0x1D;
            break;
        }
        if (m->cursor == i)
            list = menu_draw_cursor(list, m, m->item_x + (x + dx), y + m->item_y, m->cursor_w - dx, m->cursor_h);
        list = sc_draw_action_item(list, k, x + dx, y);
        y += m->item_h;
        switch (k) {
        case 1:
            list = sc_draw_rule(list, dl_alloc_inline(&list, 0x30), x, y + 1, m->item_w);
            y += 4;
            break;
        case 4:
            list = sc_draw_rule(list, dl_alloc_inline(&list, 0x30), x, y + 1, m->item_w);
            y += 4;
            break;
        }
    }
    return list;
}

ScButton sc_action_items[7] = {
    { 0x800B, 0x33, 0x11, 0x50 }, { 0x800B, 0x44, 0x11, 0x51 }, { 0x800B, 0x33, 0x22, 0x18 },
    { 0x800B, 0x44, 0x00, 0x94 }, { 0x800B, 0x22, 0x22, 0x1B }, { 0x800B, 0x55, 0x00, 0x70 },
    { 0x800B, 0x00, 0x00, 0x1A },
};

/* item i of the action menu, greyed when disabled */
void *sc_draw_action_item(void *list, unsigned short i, int x, int y)
{
    ScButton *b = &sc_action_items[i];
    unsigned int color = 0;

    switch (i) {
    case 6:
    case 5:
        if (sc_action->menu->disabled & 1 << i)
            color = 0xFF808080;
        break;
    }
    return sc_draw_button(list, b, x, y + 1, color);
}

/* the layout picker contents: cursor and the 8 layout icons */
void *sc_draw_layouts(void *list, int x, int y)
{
    void *gim = res_get(0x800A);
    int px = x;
    MenuWidget *m = sc_action->sub;
    int c = m->cursor;
    int i;
    int u;
    int v;

    list = menu_draw_cursor(list, m, x + c % 4 * 36 - 2, y + (unsigned char)c / 4 * 20 - 2, 36, 20);
    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 32, 16, 32, 16);
    for (i = 0; i < 8; i++) {
        u = i / 4 * 32 + 64;
        v = i % 4 * 16 + 64;
        if (i == 4) {
            px = x;
            y += 20;
        }
        list = draw_sprite(list, px, y, 0, u, v, 0, 0);
        px += 36;
    }
    return list;
}

ScButton sc_sub_items[4] = {
    { 0x800B, 0x11, 0x00, 0x55 }, { 0x800B, 0x22, 0x00, 0x56 }, { 0x8008, 0x66, 0x31, 0x12 }, { 0x800B, 0x22, 0x11, 0x4B },
};

void *sc_draw_sub_item(void *list, unsigned short i, int x, int y)
{
    return sc_draw_button(list, &sc_sub_items[i], x, y + 1, 0);
}

ScButton sc_clear_items[2] = { { 0x800B, 0x00, 0x11, 0x95 }, { 0x800B, 0x11, 0x11, 0x96 } };

void *sc_draw_clear_item(void *list, unsigned short i, int x, int y)
{
    return sc_draw_button(list, &sc_clear_items[i], x, y + 1, 0);
}

ScButton sc_mark_items[3] = { { 0x800A, 0x11, 0x22, 0x4C }, { 0x800A, 0x22, 0x22, 0x4D }, { 0x800A, 0x00, 0x22, 0x4E } };

void *sc_draw_mark_item(void *list, unsigned short i, int x, int y)
{
    return sc_draw_button(list, &sc_mark_items[i], x, y + 1, 0);
}

void sc_set_mode_u(int mode)
{
    sc_set_mode(mode);
}
