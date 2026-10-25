// @range 0x08813268-0x08815280
// @asm 0x08813B08-0x08813C24
// @data 0x0882300C-0x08823084
/* The card action menu and its sub-menus: info, favourite mark, sort/filter/search submenu, deck
   operations (rename, clear, reset, ...), pane layout picker (2x4 grid of layouts), "new" mark
   picker with its confirmation; the items drawn as button icons with labels. */
#include "libdeck.h"
#include "menu.h"

typedef struct DkButton {
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} DkButton;

typedef struct DkLineVtx {
    unsigned int color;
    short x, y, z;
    short pad;
} DkLineVtx;

int cfg_get_byte0_hi2(void);
void cfg_set_byte0_hi2(int v);
void *dk_res_get(unsigned short id);
void *func_088105BC(void *list, DkButton *b, int x, int y, unsigned int color);
void *dk_draw_card_pict(void *list, int x, int y, int shadow, unsigned int color, DkEnt *e, void *gim, void *gim_alt, void *alt);
void func_0880AF34(unsigned short mode);
int dk_mode_for(unsigned short panes, int narrow, unsigned short a, unsigned short b);
void dk_func_08817D18(DkList *l);
void func_08818BFC(DkList *l, unsigned short sort, unsigned short dir);
int dk_flight_undo(void);
void dk_deck_clear(void);
void dk_lists_reset(void);
void func_088192C8(void);
void func_088118F4(void);
void func_08810CD0(void);
void dk_action_free(void);
void dk_action_step0(void);
int dk_action_close(void);
void dk_action_step1(void);
void dk_action_step2(void);
void dk_action_step3(PadState *pad);
void dk_action_step4(void);
void dk_action_step5(void);
void dk_action_menu_sub(void);
void dk_action_deck_open(void);
void dk_action_layout_open(void);
void dk_action_mark_open(void);
void dk_action_confirm_open(void);
void dk_set_mode(int mode);
void *dk_action_draw_items(void *list, int x, int y, int w, int h);
void *dk_draw_action_item(void *list, unsigned short i, int x, int y);
void *dk_draw_layouts(void *list, int x, int y);
void *dk_draw_sub_item(void *list, unsigned short i, int x, int y);
void *dk_draw_deck_item(void *list, unsigned short i, int x, int y);
void *dk_draw_mark_item(void *list, unsigned short i, int x, int y);

/* opens the action menu on the card entry `ent` (0: on an empty slot) */
void dk_action_open(DkEnt *ent)
{
    int n = 7;
    int dy = 8;
    MenuWidget *m;
    MenuItemDef *def;

    dk_action = heap_alloc(dk_heap, 0x18);
    if (dk_action == 0)
        return;
    memset(dk_action, 0, 0x18);
    dk_action->ent = ent;
    if (ent == 0) {
        n -= 2;
        dy -= 4;
        dk_action->no_ent = 1;
    }
    dk_action->menu = heap_alloc(dk_heap, 0x120);
    m = dk_action->menu;
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
    def->title = table_get_entry(dk_res_get(0x8033), 0x1F);
    def->style2 = 2;
    def->title_font_size = 0xC;
    def->title_color = 0xFFFFFFFF;
    def->open_anim = 2;
    def->close_anim = 2;
    m->heap = dk_heap;
    m->no_cancel = 1;
    m->keep_open = 1;
    m->style = 2;
    m->count = n;
    m->item_w = 0xD4;
    m->item_h = 0x13;
    m->draw_all = (MenuDrawRectFn)dk_action_draw_items;
    if (dk_undo_count <= 0)
        m->disabled |= 0x40;
    if (dkv.pane[dkv.side].list->filter == 0)
        m->disabled |= 0x20;
    menu_open(m);
    if (def->w < 0xD4)
        def->w = 0xD4;
    def->h += (short)dy;
    def->x = (0x1E0 - def->w) / 2;
    dk_state = 2;
}

/* rebuilds the action menu for another entry, keeping the cursor on the same item */
void dk_action_reopen(DkEnt *ent)
{
    int was = 0;
    int pos = dk_action->menu->scroll + dk_action->menu->cursor;

    if (dk_action->no_ent == 1)
        was = 1;
    dk_action_free();
    dk_action_open(ent);
    if (dk_action->no_ent == 1) {
        if (was)
            dk_action->menu->cursor = pos;
        else if (pos >= 2)
            dk_action->menu->cursor = pos - 2;
    } else if (was) {
        dk_action->menu->cursor = pos + 2;
    } else {
        dk_action->menu->cursor = pos;
    }
}

/* per frame while the action menu is open; START opens the help page of the step */
int dk_action_update(PadState *pad)
{
    switch (dk_action->step) {
    case 0:
        if (pad->b.trig & 8)
            dk_help_page = 8;
        else
            dk_action_step0();
        break;
    case 1:
        if (pad->b.trig & 8)
            dk_help_page = 0x10;
        else
            dk_action_step1();
        break;
    case 2:
        if (pad->b.trig & 8)
            dk_help_page = 0x17;
        else
            dk_action_step2();
        break;
    case 10:
        return dk_action_close();
    case 3:
        if (pad->b.trig & 8)
            dk_help_page = 0xD;
        else
            dk_action_step3(pad);
        break;
    case 4:
        if (pad->b.trig & 8)
            dk_help_page = 0x15;
        else
            dk_action_step4();
        break;
    case 5:
        dk_action_step5();
        break;
    }
    return 0;
}

void *dk_action_draw(void *list)
{
    menu_draw(&list, dk_action->menu);
    if (dk_action->sub != 0)
        menu_draw(&list, dk_action->sub);
    return list;
}

void dk_action_free(void)
{
    if (dk_action->sub != 0) {
        menu_reset(dk_action->sub);
        heap_set_level(dk_heap, dk_action->sub);
        dk_action->sub = 0;
    }
    menu_reset(dk_action->menu);
    if (dk_action->menu != 0) {
        heap_set_level(dk_heap, dk_action->menu);
        dk_action->menu = 0;
    }
    heap_set_level(dk_heap, dk_action);
    dk_action = 0;
}

/* step 0: the action menu itself */
void dk_action_step0(void)
{
    int r = menu_update(dk_action->menu);
    unsigned short id;

    if (r == -1)
        return;
    if (r >= 0 && dk_action->no_ent == 1)
        r += 2;
    switch (r) {
    case -2:
    case 5:
    case 6:
        menu_finish(dk_action->menu);
        dk_action->step = 10;
        return;
    }
    dk_action->menu->no_cancel = 0;
    switch (r) {
    case 0:
        dk_flags.info = 1;
        dk_action->menu->cancelled = dk_action->menu->accepted = 0;
        break;
    case 1:
        id = dk_action->ent->id;
        if (inv_get_flag7(id))
            inv_set_flag7(id, 0);
        else
            inv_set_flag7(id, 1);
        dk_action->menu->cancelled = dk_action->menu->accepted = 0;
        break;
    case 2:
        dk_action_menu_sub();
        se_play(0x6216);
        dk_action->step = 1;
        break;
    case 4:
        dk_action_layout_open();
        se_play(0x6216);
        dk_action->step = 3;
        break;
    case 3:
        dk_action_deck_open();
        se_play(0x6216);
        dk_action->step = 2;
        break;
    }
}

#ifdef NON_MATCHING /* o original grava o estado no atraso de cada desvio para o retorno comum (lui içado); mesma lógica */
/* step 10: wait for the close, then run the chosen action */
int dk_action_close(void)
{
    int r = menu_update(dk_action->menu);
    DkList *l;

    if (dk_action->menu->state == 5)
        return 0;
    if (r >= 0 && dk_action->no_ent == 1)
        r += 2;
    dk_action_free();
    switch (r) {
    case 5:
        l = dkv.pane[dkv.side].list;
        dk_func_08817D18(l);
        func_08818BFC(l, l->sort, l->dir);
        break;
    case 6:
        if (dk_flight_undo())
            return 0;
        break;
    }
    dk_state = 0;
    return 0;
}
#endif

/* a sub-menu window beside the action menu */
int dk_sub_open(unsigned short title, unsigned short text, unsigned short count, MenuDrawItemFn draw)
{
    MenuWidget *m;
    MenuItemDef *def;

    if (dk_action->sub == 0)
        dk_action->sub = heap_alloc(dk_heap, 0x120);
    m = dk_action->sub;
    if (m == 0)
        return 0;
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    def->x = dk_action->menu->def.x + 0x28;
    def->y = 0x7C;
    def->auto_size = 1;
    def->color = 0xFFFFFFFF;
    if (text != 0)
        def->text = table_get_entry(dk_res_get(0x8033), text);
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
        def->title = table_get_entry(dk_res_get(0x8033), title);
    def->style2 = 2;
    def->title_font_size = 0xC;
    def->title_color = 0xFFFFFFFF;
    def->open_anim = 2;
    def->close_anim = 2;
    m->heap = dk_heap;
    m->no_cancel = 1;
    m->style = 2;
    m->count = count;
    m->item_w = 0xD4;
    m->item_h = 0x13;
    m->draw_item = draw;
    return 1;
}

/* the sort / filter / search / mark submenu */
void dk_action_menu_sub(void)
{
    if (dk_sub_open(0x18, 0, 4, (MenuDrawItemFn)dk_draw_sub_item)) {
        dk_action->sub->keep_open = 1;
        menu_open(dk_action->sub);
    }
}

/* step 1 */
void dk_action_step1(void)
{
    MenuWidget *m = dk_action->sub;
    int r = menu_update(m);

    if (r == -1)
        return;
    if (m->state == 3) {
        menu_finish(m);
        switch (r) {
        case 0:
        case 1:
        case 2:
            menu_finish(dk_action->menu);
            break;
        }
        return;
    }
    menu_reset(m);
    heap_set_level(dk_heap, dk_action->sub);
    dk_action->sub = 0;
    switch (r) {
    case 0:
        func_08810CD0();
        se_play(0x6216);
        dk_action_free();
        break;
    case 1:
        func_088118F4();
        se_play(0x6216);
        dk_action_free();
        break;
    case 2:
        func_088192C8();
        se_play(0x6216);
        dk_action_free();
        break;
    case 3:
        dk_action_mark_open();
        se_play(0x6216);
        dk_action->step = 4;
        break;
    case -2:
        dk_action->step = 0;
        dk_action->menu->cancelled = dk_action->menu->accepted = 0;
        break;
    }
}

/* the deck operations submenu (the save is disabled while the deck is out of the rules) */
void dk_action_deck_open(void)
{
    if (dk_sub_open(0x1D, 0, 5, (MenuDrawItemFn)dk_draw_deck_item)) {
        if ((unsigned int)dk_deck.lists[0].total > 0x50 || (unsigned int)dk_deck.lists[1].total > 0x1E || (unsigned int)dk_deck.lists[2].total > 0xF)
            dk_action->sub->disabled |= 2;
        menu_open(dk_action->sub);
    }
}

/* step 2 */
void dk_action_step2(void)
{
    int done = 0;
    MenuWidget *m = dk_action->sub;
    int r = menu_update(m);

    if (r == -1)
        return;
    menu_reset(m);
    heap_set_level(dk_heap, dk_action->sub);
    dk_action->sub = 0;
    switch (r) {
    case 0:
        dk_flags.deck_op = 2;
        break;
    case 1:
        dk_flags.deck_op = 1;
        break;
    case 2:
        dk_lists_reset();
        dk_undo_count = 0;
        done = 1;
        break;
    case 3:
        dk_deck_clear();
        dk_undo_count = 0;
        done = 1;
        break;
    case 4:
        dk_name_input_open(dk_deck_name, table_get_entry(dk_res_get(0x8033), 0x7F), 0);
        break;
    }
    if (done) {
        menu_finish(dk_action->menu);
        dk_action->step = 10;
    } else {
        dk_action->step = 0;
        dk_action->menu->cancelled = dk_action->menu->accepted = 0;
    }
}

/* the layout picker: 4 x 2 grid of pane layouts */
void dk_action_layout_open(void)
{
    MenuWidget *m;

    if (!dk_sub_open(0x1B, 0, 0, 0))
        return;
    m = dk_action->sub;
    m->def.auto_size = 0;
    m->def.w = 0x98;
    m->def.h = 0x30;
    m->def.x = dk_action->menu->def.x + 0x28;
    m->def.y = 0x7C;
    dk_action->mode = dk_mode_for(dkv.mode > 1 ? 2 : 1, dkv.mode < 4, dkv.pane[0].list_mode, dkv.pane[1].list_mode);
    m->cursor = dk_action->mode;
    if (dkv.side == 0) {
        dk_action->side = 1;
        dk_action->tab = dkv.pane[1].list->tab;
    } else {
        dk_action->side = 0;
        dk_action->tab = dkv.pane[0].list->tab;
    }
    m->no_input = 1;
    m->draw_all = (MenuDrawRectFn)dk_draw_layouts;
    menu_open(m);
}

/* step 3: d-pad picks a layout (previewed at once), accept keeps it, cancel restores */
void dk_action_step3(PadState *pad)
{
    MenuWidget *m = dk_action->sub;

    menu_update(m);
    if (m->state == 6) {
        if (m->cancelled) {
            dk_set_mode(dk_action->mode % 8);
            menu_reset(m);
            heap_set_level(dk_heap, dk_action->sub);
            dk_action->sub = 0;
            dk_action->step = 0;
            dk_action->menu->cancelled = dk_action->menu->accepted = 0;
        } else {
            if (dk_action->mode > 1 && dkv.mode < 2)
                dk_func_08817D18(&dk_deck.lists[dk_action->tab]);
            menu_finish(dk_action->menu);
            dk_action->step = 10;
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
        dk_set_mode(m->cursor % 8);
        se_play(0x6212);
        return;
    }
    if (pad->b.rep & 0x80) {
        m->cursor = (m->cursor % 4 + 3) % 4 + m->cursor / 4 * 4;
        dk_set_mode(m->cursor % 8);
        se_play(0x6212);
        return;
    }
    if (pad->b.rep & 0x20) {
        m->cursor = (m->cursor % 4 + 1) % 4 + m->cursor / 4 * 4;
        dk_set_mode(m->cursor % 8);
        se_play(0x6212);
    }
}

/* the "new" mark picker */
void dk_action_mark_open(void)
{
    MenuWidget *m;

    if (dk_sub_open(0x4B, 0x54, 3, (MenuDrawItemFn)dk_draw_mark_item)) {
        m = dk_action->sub;
        m->def.max_w = 0xF4;
        m->item_w = 0xF4;
        m->selected = 1 << cfg_get_byte0_hi2();
        menu_open(m);
    }
}

/* step 4 */
void dk_action_step4(void)
{
    MenuWidget *m = dk_action->sub;
    int r = menu_update(m);

    if (r == -1)
        return;
    menu_reset(m);
    switch (r) {
    case -2:
        dk_action_menu_sub();
        dk_action->step = 1;
        break;
    default:
        dk_action->mark = r;
        dk_action_confirm_open();
        se_play(0x621A);
        dk_action->step = 5;
        break;
    }
}

/* yes/no window for the mark */
void dk_action_confirm_open(void)
{
    MenuWidget *m;
    MenuItemDef *def;
    int x;

    if (dk_action->sub == 0)
        dk_action->sub = heap_alloc(dk_heap, 0x120);
    m = dk_action->sub;
    if (m == 0)
        return;
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    def->max_w = 0xF0;
    def->auto_size = 1;
    def->color = 0xFFFFFFFF;
    def->font_flag4 = 1;
    def->pad_l = 10;
    def->pad_t = 8;
    def->pad_r = 10;
    def->pad_b = 0;
    def->font_size = 0xC;
    def->text_color = 0xFF000000;
    def->text = table_get_entry(dk_res_get(0x8033), 0x53);
    def->open_anim = 2;
    def->close_anim = 2;
    m->heap = dk_heap;
    m->no_cancel = 1;
    m->buttons = 3;
    m->initial = 3;
    m->cancel_resets = 1;
    menu_open(m);
    x = 0x1E0 - def->w;
    def->x = x / 2;
    x = 0x110 - def->h;
    def->y = x / 2;
}

/* step 5: yes stores the mark and drops the marks of the trunk list */
void dk_action_step5(void)
{
    MenuWidget *m = dk_action->sub;
    int r = menu_update(m);
    DkList *l;

    if (r == -1)
        return;
    menu_reset(m);
    switch (r) {
    case -2:
    case 3:
        dk_action->step = 0;
        dk_action->menu->cancelled = dk_action->menu->accepted = 0;
        break;
    case 2:
        cfg_set_byte0_hi2(dk_action->mark);
        l = &dk_deck.lists[3];
        dk_func_08817D18(l);
        if (dkv.pane[0].list->tab == 3) {
            dkv.pane[0].cy = 0;
            dkv.pane[0].cx = 0;
            dkv.pane[0].top = 0;
        }
        if (dkv.pane[1].list->tab == 3) {
            dkv.pane[1].cy = 0;
            dkv.pane[1].cx = 0;
            dkv.pane[1].top = 0;
        }
        l->u18 = l->u1a = l->u1c_lo = l->u1c_hi = 0;
        heap_set_level(dk_heap, dk_action->sub);
        dk_action->sub = 0;
        menu_finish(dk_action->menu);
        dk_action->step = 10;
        break;
    }
}

/* a two-tone rule under a menu item */
void *dk_draw_rule(void *list, DkLineVtx *v, int x, int y, int w)
{
    int i;
    DkLineVtx *p;

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
void *dk_action_draw_items(void *list, int x, int y, int w, int h)
{
    MenuWidget *m = dk_action->menu;
    unsigned int first = 0;
    int n = 7;
    int i;
    unsigned int k;
    int dx;

    if (!dk_action->no_ent) {
        list = dk_draw_card_pict(list, x + 2, y + 2, 1, 0xFFFFFFFF, dk_action->ent, dk_res_get(0x8001), dk_res_get(0x8002), dk_res_get(0x8032));
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
        list = dk_draw_action_item(list, k, x + dx, y);
        y += m->item_h;
        switch (k) {
        case 1:
            list = dk_draw_rule(list, dl_alloc_inline(&list, 0x30), x, y + 1, m->item_w);
            y += 4;
            break;
        case 4:
            list = dk_draw_rule(list, dl_alloc_inline(&list, 0x30), x, y + 1, m->item_w);
            y += 4;
            break;
        }
    }
    return list;
}

DkButton dk_action_items[7] = {
    { 0x800B, 0x33, 0x11, 0x50 }, { 0x800B, 0x44, 0x11, 0x51 }, { 0x800B, 0x33, 0x22, 0x18 },
    { 0x800B, 0x44, 0x00, 0x1D }, { 0x800B, 0x22, 0x22, 0x1B }, { 0x800B, 0x55, 0x00, 0x70 },
    { 0x800B, 0x00, 0x00, 0x1A },
};

/* item i of the action menu, greyed when disabled */
void *dk_draw_action_item(void *list, unsigned short i, int x, int y)
{
    DkButton *b = &dk_action_items[i];
    unsigned int color = 0;

    switch (i) {
    case 6:
    case 5:
        if (dk_action->menu->disabled & 1 << i)
            color = 0xFF808080;
        break;
    }
    return func_088105BC(list, b, x, y + 1, color);
}

/* the layout picker contents: cursor and the 8 layout icons */
void *dk_draw_layouts(void *list, int x, int y)
{
    void *gim = dk_res_get(0x800A);
    int px = x;
    MenuWidget *m = dk_action->sub;
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

DkButton dk_sub_items[4] = {
    { 0x800B, 0x11, 0x00, 0x55 }, { 0x800B, 0x22, 0x00, 0x56 }, { 0x8008, 0x66, 0x31, 0x12 }, { 0x800B, 0x22, 0x11, 0x4B },
};

void *dk_draw_sub_item(void *list, unsigned short i, int x, int y)
{
    return func_088105BC(list, &dk_sub_items[i], x, y + 1, 0);
}

DkButton dk_deck_items[5] = {
    { 0x800B, 0x55, 0x44, 0x46 }, { 0x800B, 0x66, 0x44, 0x47 }, { 0x800B, 0x00, 0x11, 0x49 },
    { 0x800B, 0x11, 0x11, 0x4A }, { 0x800B, 0x66, 0x00, 0x48 },
};

/* item i of the deck operations submenu, greyed when disabled */
void *dk_draw_deck_item(void *list, unsigned short i, int x, int y)
{
    DkButton *b = &dk_deck_items[i];
    unsigned int color = 0;

    switch (i) {
    case 1:
        if (dk_action->sub->disabled & 1 << i)
            color = 0xFF808080;
        break;
    }
    return func_088105BC(list, b, x, y + 1, color);
}

DkButton dk_mark_items[3] = { { 0x800A, 0x11, 0x22, 0x4C }, { 0x800A, 0x22, 0x22, 0x4D }, { 0x800A, 0x00, 0x22, 0x4E } };

void *dk_draw_mark_item(void *list, unsigned short i, int x, int y)
{
    return func_088105BC(list, &dk_mark_items[i], x, y + 1, 0);
}

/* sets the pane layout; going from one list to two in the narrow layouts shows the list picked */
void dk_set_mode(int mode)
{
    unsigned short panes = dkv.mode > 1 ? 2 : 1;

    func_0880AF34(mode);
    if (dkv.side == 1 && dkv.mode > 1 && panes == 1)
        dk_select_tab(0, dk_action->tab, 2);
}
