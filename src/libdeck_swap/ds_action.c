// @range 0x08810D64-0x08812510
// @data 0x0881A2D4-0x0881A320
/* The card action menu and its sub-menus: favourite mark, card info, sort/filter submenu, clear
   or reset the list, pane layout picker (2x4 grid of layouts), the items drawn as button icons with
   labels (same source as libselect_card sc_action.c and libdeck dk_action.c). */
#include "libdeck_swap.h"
#include "menu.h"

typedef struct DsButton {              /* ds_draw_button */
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} DsButton;

typedef struct DsLineVtx {             /* see ds_draw.c */
    unsigned int color;
    short x, y, z;
    short pad;
} DsLineVtx;

int cfg_get_byte0_hi2(void);
void cfg_set_byte0_hi2(int v);
void *ds_draw_button(void *list, DsButton *b, int x, int y, unsigned int color);
void ds_lists_reset(void);
void ds_lists_clear(void);
int ds_input_move(void);
void ds_input_sort(void);
void ds_input_filter(void);
void ds_input_mark(void);
int ds_mode_for(unsigned short panes, int narrow, unsigned short a, unsigned short b);
void ds_set_mode_u(int mode);
void ds_set_mode(unsigned short mode);
void ds_action_free(void);
int ds_sub_open(unsigned short title, unsigned short text, unsigned short count, MenuDrawItemFn draw);
void ds_action_menu_sub(void);
void ds_action_clear_open(void);
void ds_action_layout_open(void);
void ds_action_mark_open(void);
void ds_action_confirm_open(void);
void *ds_draw_rule(void *list, DsLineVtx *v, int x, int y, int w);
void *ds_draw_action_item(void *list, unsigned short i, int x, int y);
void *ds_draw_layouts(void *list, int x, int y);
void *ds_draw_sub_item(void *list, unsigned short i, int x, int y);
void *ds_draw_clear_item(void *list, unsigned short i, int x, int y);
void *ds_draw_mark_item(void *list, unsigned short i, int x, int y);

void *ds_action_draw_items(void *list, int x, int y, int w, int h);
void ds_action_step0(void);
int ds_action_close(void);
void ds_action_step1(void);
void ds_action_step2(void);
void ds_action_step3(void);
void ds_word_open(void);
void ds_fmenu_open(void);
void ds_sort_menu_open(void);

/* opens the action menu on the card entry `ent` (0: on an empty slot) */
void ds_action_open(DsEnt *ent)
{
    int n = 7;
    int dy = 8;
    MenuWidget *m;
    MenuItemDef *def;

    ds_action = heap_alloc(ds.heap, 0x18);
    if (ds_action == 0)
        return;
    memset(ds_action, 0, 0x18);
    ds_action->ent = ent;
    if (ent == 0) {
        n -= 2;
        dy -= 4;
        ds_action->no_ent = 1;
    }
    ds_action->menu = heap_alloc(ds.heap, 0x120);
    m = ds_action->menu;
    if (m == 0)
        return;
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    def->y = 0x3A;
    def->color = 0xFFFFFFFF;
    def->auto_size = 1;
    def->font_flag4 = 1;
    def->pad_l = 2;
    def->pad_t = 2;
    def->pad_r = 2;
    def->pad_b = 2;
    def->show_title = 1;
    def->auto_title = 1;
    def->title = table_get_entry(res_get(0x8033), 0x8B);
    def->style2 = 2;
    def->title_font_size = 0xC;
    def->title_color = 0xFFFFFFFF;
    def->open_anim = 2;
    def->close_anim = 2;
    m->heap = ds.heap;
    m->no_cancel = 1;
    m->keep_open = 1;
    m->style = 2;
    m->count = n;
    m->item_w = 0xD4;
    m->item_h = 0x13;
    m->draw_all = (MenuDrawRectFn)ds_action_draw_items;
    if (ds.nhist <= 0)
        m->disabled |= 0x40;
    if (dsv.pane[dsv.side].list->filter == 0)
        m->disabled |= 0x20;
    menu_open(m);
    if (def->w < 0xD4)
        def->w = 0xD4;
    def->h += (short)dy;
    def->x = (0x1E0 - def->w) / 2;
    ds.state = 3;
}

/* state 3 */
int ds_input_action_menu(void)
{
    switch (ds_action->step) {
    case 0:
        ds_action_step0();
        break;
    case 1:
        ds_action_step1();
        break;
    case 2:
        ds_action_step2();
        break;
    case 10:
        return ds_action_close();
    case 3:
        ds_action_step3();
        break;
    }
    return 0;
}

void *ds_draw_action_menu(void *list)
{
    menu_draw(&list, ds_action->menu);
    if (ds_action->sub != 0)
        menu_draw(&list, ds_action->sub);
    return list;
}

void ds_action_free(void)
{
    if (ds_action->sub != 0) {
        menu_reset(ds_action->sub);
        heap_set_level(ds.heap, ds_action->sub);
        ds_action->sub = 0;
    }
    menu_reset(ds_action->menu);
    if (ds_action->menu != 0) {
        heap_set_level(ds.heap, ds_action->menu);
        ds_action->menu = 0;
    }
    heap_set_level(ds.heap, ds_action);
    ds_action = 0;
}

/* step 0: the action menu itself */
void ds_action_step0(void)
{
    int r = menu_update(ds_action->menu);
    unsigned short id;

    if (r == -1)
        return;
    if (r >= 0 && ds_action->no_ent == 1)
        r += 2;
    switch (r) {
    case -2:
    case 5:
    case 6:
        menu_finish(ds_action->menu);
        ds_action->step = 10;
        return;
    }
    ds_action->menu->no_cancel = 0;
    switch (r) {
    case 0:
        ds.info_open = 1;
        ds_action->menu->cancelled = ds_action->menu->accepted = 0;
        break;
    case 1:
        id = ds_action->ent->id;
        if (inv_get_flag7(id))
            inv_set_flag7(id, 0);
        else
            inv_set_flag7(id, 1);
        ds_action->menu->cancelled = ds_action->menu->accepted = 0;
        break;
    case 2:
        ds_action_menu_sub();
        se_play(0x6216);
        ds_action->step = 1;
        break;
    case 4:
        ds_action_layout_open();
        se_play(0x6216);
        ds_action->step = 3;
        break;
    case 3:
        ds_action_clear_open();
        se_play(0x6216);
        ds_action->step = 2;
        break;
    }
}


/* step 10: wait for the close, then run the chosen action */
int ds_action_close(void)
{
    int r = menu_update(ds_action->menu);
    DsList *l;

    if (ds_action->menu->state == 5)
        return 0;
    if (r >= 0 && ds_action->no_ent == 1)
        r += 2;
    ds_action_free();
    switch (r) {
    case 5:
        l = dsv.pane[dsv.side].list;
        ds_list_filter_reset(l);
        ds_list_sort(l, l->sort, l->dir);
        ds.state = 1;
        break;
    case 6:
        if (ds_input_move() == 0) {
            ds.state = 1;
            break;
        }
        return 0;
    default:
        ds.state = 1;
        break;
    }
    return 0;
}


/* a sub-menu window beside the action menu */
int ds_sub_open(unsigned short title, unsigned short text, unsigned short count, MenuDrawItemFn draw)
{
    MenuWidget *m;
    MenuItemDef *def;

    if (ds_action->sub == 0)
        ds_action->sub = heap_alloc(ds.heap, 0x120);
    m = ds_action->sub;
    if (m == 0)
        return 0;
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    def->x = ds_action->menu->def.x + 0x28;
    def->y = 0x7C;
    def->auto_size = 1;
    def->color = 0xFFFFFFFF;
    if (text != 0)
        def->text = table_get_entry(res_get(0x8033), text);
    def->font_size = 0xC;
    def->text_color = 0xFF000000;
    def->font_flag4 = 1;
    def->pad_l = 2;
    def->pad_t = 2;
    def->pad_r = 2;
    def->pad_b = 2;
    def->show_title = 1;
    def->auto_title = 1;
    if (title != 0)
        def->title = table_get_entry(res_get(0x8033), title);
    def->style2 = 2;
    def->title_font_size = 0xC;
    def->title_color = 0xFFFFFFFF;
    def->open_anim = 2;
    def->close_anim = 2;
    m->heap = ds.heap;
    m->no_cancel = 1;
    m->style = 2;
    m->count = count;
    m->item_w = 0xD4;
    m->item_h = 0x13;
    m->draw_item = draw;
    return 1;
}

/* the sort / filter / search / mark submenu */
void ds_action_menu_sub(void)
{
    if (ds_sub_open(0x18, 0, 3, (MenuDrawItemFn)ds_draw_sub_item)) {
        ds_action->sub->keep_open = 1;
        menu_open(ds_action->sub);
    }
}

/* step 1: the sort/filter/search submenu */
void ds_action_step1(void)
{
    MenuWidget *m = ds_action->sub;
    int r = menu_update(m);

    if (r == -1)
        return;
    if (m->state == 3) {
        menu_finish(m);
        switch (r) {
        case 0:
        case 1:
        case 2:
            menu_finish(ds_action->menu);
            break;
        }
        return;
    }
    menu_reset(m);
    heap_set_level(ds.heap, ds_action->sub);
    ds_action->sub = 0;
    switch (r) {
    case 0:
        ds_sort_menu_open();
        se_play(0x6216);
        ds_action_free();
        break;
    case 1:
        ds_fmenu_open();
        se_play(0x6216);
        ds_action_free();
        break;
    case 2:
        ds_word_open();
        se_play(0x6216);
        ds_action_free();
        break;
    case -2:
        ds_action->step = 0;
        ds_action->menu->cancelled = ds_action->menu->accepted = 0;
        break;
    }
}


void ds_action_clear_open(void)
{
    if (ds_sub_open(0x8C, 0, 2, (MenuDrawItemFn)ds_draw_clear_item))
        menu_open(ds_action->sub);
}

/* step 2: clear or reset the list */
void ds_action_step2(void)
{
    int done = 0;
    MenuWidget *m = ds_action->sub;
    int r = menu_update(m);

    if (r == -1)
        return;
    menu_reset(m);
    heap_set_level(ds.heap, ds_action->sub);
    ds_action->sub = 0;
    switch (r) {
    case 0:
        ds_lists_reset();
        ds.nhist = 0;
        done = 1;
        break;
    case 1:
        ds_lists_clear();
        ds.nhist = 0;
        done = 1;
        break;
    }
    if (done) {
        menu_finish(ds_action->menu);
        ds_action->step = 10;
    } else {
        ds_action->step = 0;
        ds_action->menu->cancelled = ds_action->menu->accepted = 0;
    }
}


/* the layout picker: 4 x 2 grid of pane layouts */
void ds_action_layout_open(void)
{
    MenuWidget *m;

    if (!ds_sub_open(0x1B, 0, 0, 0))
        return;
    m = ds_action->sub;
    m->def.auto_size = 0;
    m->def.w = 0x98;
    m->def.h = 0x30;
    m->def.x = ds_action->menu->def.x + 0x28;
    m->def.y = 0x7C;
    ds_action->mode = ds_mode_for(dsv.mode > 1 ? 2 : 1, dsv.mode < 4, dsv.pane[0].list_mode, dsv.pane[1].list_mode);
    m->cursor = ds_action->mode;
    if (dsv.side == 0) {
        ds_action->side = 1;
        ds_action->tab = dsv.pane[1].list->tab;
    } else {
        ds_action->side = 0;
        ds_action->tab = dsv.pane[0].list->tab;
    }
    m->no_input = 1;
    m->draw_all = (MenuDrawRectFn)ds_draw_layouts;
    menu_open(m);
}

/* step 3: the layout picker (d-pad moves in the 4 x 2 grid, the layout is shown live) */
void ds_action_step3(void)
{
    MenuWidget *m = ds_action->sub;
    PadState *pad = pad_get();

    menu_update(m);
    if (m->state == 6) {
        if (m->cancelled) {
            ds_set_mode_u(ds_action->mode % 8);
            menu_reset(m);
            heap_set_level(ds.heap, ds_action->sub);
            ds_action->sub = 0;
            ds_action->step = 0;
            ds_action->menu->cancelled = ds_action->menu->accepted = 0;
        } else {
            if (ds_action->mode > 1 && dsv.mode < 2)
                ds_list_filter_reset(&ds.list[ds_action->tab]);
            menu_finish(ds_action->menu);
            ds_action->step = 10;
        }
        return;
    }
    if (pad->b.trig & region_pick_value(1)) {
        m->cancelled = 1;
        menu_finish(m);
        se_play(0x6214);
        return;
    }
    if (pad->b.trig & region_pick_value(0)) {
        menu_set_cursor(m, m->cursor);
        se_play(0x6213);
        return;
    }
    if (pad->b.rep & 0x50) {
        m->cursor = (m->cursor + 4) % 8;
        ds_set_mode_u(m->cursor % 8);
        se_play(0x6212);
        return;
    }
    if (pad->b.rep & 0x80) {
        m->cursor = (m->cursor % 4 + 3) % 4 + m->cursor / 4 * 4;
        ds_set_mode_u(m->cursor % 8);
        se_play(0x6212);
        return;
    }
    if (pad->b.rep & 0x20) {
        m->cursor = (m->cursor % 4 + 1) % 4 + m->cursor / 4 * 4;
        ds_set_mode_u(m->cursor % 8);
        se_play(0x6212);
    }
}


/* a two-tone rule under a menu item */
void *ds_draw_rule(void *list, DsLineVtx *v, int x, int y, int w)
{
    int i;
    DsLineVtx *p;

    for (i = 0, p = v; i < 4; i++, p++) {
        p->x = x + w * (i % 2);
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

/* the items of the action menu: card picture on top, then the buttons with rules */
void *ds_action_draw_items(void *list, int x, int y, int w, int h)
{
    MenuWidget *m = ds_action->menu;
    unsigned int first = 0;
    int n = 7;
    int i;
    unsigned int k;
    int dx;

    if (!ds_action->no_ent) {
        list = ds_draw_card_pict(list, x + 2, y + 2, 1, 0xFFFFFFFF, ds_action->ent, res_get(0x8001), res_get(0x8002), res_get(0x8032));
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
        list = ds_draw_action_item(list, k, x + dx, y);
        y += m->item_h;
        switch (k) {
        case 1:
            list = ds_draw_rule(list, dl_alloc_inline(&list, 0x30), x, y + 1, m->item_w);
            y += 4;
            break;
        case 4:
            list = ds_draw_rule(list, dl_alloc_inline(&list, 0x30), x, y + 1, m->item_w);
            y += 4;
            break;
        }
    }
    return list;
}

DsButton ds_action_items[7] = {
    { 0x800B, 0x33, 0x11, 0x50 }, { 0x800B, 0x44, 0x11, 0x51 }, { 0x800B, 0x33, 0x22, 0x18 },
    { 0x800B, 0x44, 0x00, 0x8C }, { 0x800B, 0x22, 0x22, 0x1B }, { 0x800B, 0x55, 0x00, 0x70 },
    { 0x800B, 0x00, 0x00, 0x1A },
};

/* item i of the action menu, greyed when disabled */
void *ds_draw_action_item(void *list, unsigned short i, int x, int y)
{
    DsButton *b = &ds_action_items[i];
    unsigned int color = 0;

    switch (i) {
    case 6:
    case 5:
        if (ds_action->menu->disabled & 1 << i)
            color = 0xFF808080;
        break;
    }
    return ds_draw_button(list, b, x, y + 1, color);
}

/* the layout picker contents: cursor and the 8 layout icons */
void *ds_draw_layouts(void *list, int x, int y)
{
    void *gim = res_get(0x800A);
    int px = x;
    MenuWidget *m = ds_action->sub;
    unsigned char c = m->cursor;
    int i;
    int u;
    int v;

    list = menu_draw_cursor(list, m, x + c % 4 * 36 - 2, y + c / 4 * 20 - 2, 36, 20);
    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 32, 16, 32, 16);
    for (i = 0; i < 8; i++, px += 36) {
        u = i / 4 * 32 + 64;
        v = i % 4 * 16 + 64;
        if (i == 4) {
            px = x;
            y += 20;
        }
        list = draw_sprite(list, px, y, 0, u, v, 0, 0);
    }
    return list;
}

DsButton ds_sub_items[3] = {
    { 0x800B, 0x11, 0x00, 0x55 }, { 0x800B, 0x22, 0x00, 0x56 }, { 0x8008, 0x66, 0x31, 0x12 },
};

void *ds_draw_sub_item(void *list, unsigned short i, int x, int y)
{
    return ds_draw_button(list, &ds_sub_items[i], x, y + 1, 0);
}

DsButton ds_clear_items[2] = { { 0x800B, 0x00, 0x11, 0x49 }, { 0x800B, 0x11, 0x11, 0x4A } };

void *ds_draw_clear_item(void *list, unsigned short i, int x, int y)
{
    return ds_draw_button(list, &ds_clear_items[i], x, y + 1, 0);
}

void ds_set_mode_u(int mode)
{
    ds_set_mode(mode);
}

