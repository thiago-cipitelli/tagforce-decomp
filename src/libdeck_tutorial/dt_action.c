// @range 0x08814D44-0x08816C34
// @asm 0x08815494-0x088155B0
// @data 0x088221E4-0x0882225C
/* (The code of libdeck dk_action.c.) The card action menu and its sub-menus: info, favourite mark, sort/filter/search submenu, deck
   operations (rename, clear, reset, ...), pane layout picker (2x4 grid of layouts), "new" mark
   picker with its confirmation; the items drawn as button icons with labels. */
#include "libdeck_tutorial.h"

typedef struct DtButton {
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} DtButton;

typedef struct DtLineVtx {
    unsigned int color;
    short x, y, z;
    short pad;
} DtLineVtx;

int cfg_get_byte0_hi2(void);
void cfg_set_byte0_hi2(int v);
void *dt_eres_get(unsigned short id);
void *dt_draw_ibutton(void *list, DtButton *b, int x, int y, unsigned int color);
void *dt_draw_card_pict(void *list, int x, int y, int shadow, unsigned int color, DtEnt *e, void *gim, void *gim_alt, void *alt);
void dt_set_mode(unsigned short mode);
int dt_mode_for(unsigned short panes, int narrow, unsigned short a, unsigned short b);
void dt_list_filter_reset(DtList *l);
void dt_list_sort(DtList *l, unsigned short sort, unsigned short dir);
int dt_flight_undo(void);
void dt_deck_clear(void);
void dt_lists_reset(void);
void dt_word_open(void);
void dt_fmenu_open(void);
void dt_sort_menu_open(void);
void dt_action_free(void);
void dt_action_step0(void);
int dt_action_close(void);
void dt_action_step1(void);
void dt_action_step2(void);
void dt_action_step3(PadState *pad);
void dt_action_step4(void);
void dt_action_step5(void);
void dt_action_menu_sub(void);
void dt_action_deck_open(void);
void dt_action_layout_open(void);
void dt_action_mark_open(void);
void dt_action_confirm_open(void);
void dt_action_set_mode(int mode);
void dt_menu_lock(MenuWidget *m);
void *dt_action_draw_items(void *list, int x, int y, int w, int h);
void *dt_draw_action_item(void *list, unsigned short i, int x, int y);
void *dt_draw_layouts(void *list, int x, int y);
void *dt_draw_sub_item(void *list, unsigned short i, int x, int y);
void *dt_draw_deck_item(void *list, unsigned short i, int x, int y);
void *dt_draw_mark_item(void *list, unsigned short i, int x, int y);

/* opens the action menu on the card entry `ent` (0: on an empty slot) */
void dt_action_open(DtEnt *ent)
{
    int n = 7;
    int dy = 8;
    MenuWidget *m;
    MenuItemDef *def;

    dt_action = heap_alloc(dts.heap, 0x18);
    if (dt_action == 0)
        return;
    memset(dt_action, 0, 0x18);
    dt_action->ent = ent;
    if (ent == 0) {
        n -= 2;
        dy -= 4;
        dt_action->no_ent = 1;
    }
    dt_action->menu = heap_alloc(dts.heap, 0x120);
    m = dt_action->menu;
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
    def->title = table_get_entry(dt_eres_get(0x8033), 0x1F);
    def->style2 = 2;
    def->title_font_size = 0xC;
    def->title_color = 0xFFFFFFFF;
    def->open_anim = 2;
    def->close_anim = 2;
    m->heap = dts.heap;
    m->no_cancel = 1;
    m->keep_open = 1;
    m->style = 2;
    m->count = n;
    m->item_w = 0xD4;
    m->item_h = 0x13;
    m->draw_all = (MenuDrawRectFn)dt_action_draw_items;
    if (dt_undo_count <= 0)
        m->disabled |= 0x40;
    if (dtv.pane[dtv.side].list->filter == 0)
        m->disabled |= 0x20;
    m->pad = (PadState *)&dt.pad;
    if (dt.step == 15)
        dt_menu_lock(m);
    menu_open(m);
    if (def->w < 0xD4)
        def->w = 0xD4;
    def->h += (short)dy;
    def->x = (0x1E0 - def->w) / 2;
    dts.state = 2;
}

/* per frame while the action menu is open; START opens the help page of the step */
int dt_action_update(PadState *pad)
{
    switch (dt_action->step) {
    case 0:
        if (!(pad->b.trig & 8))
            dt_action_step0();
        break;
    case 1:
        if (!(pad->b.trig & 8))
            dt_action_step1();
        break;
    case 2:
        if (!(pad->b.trig & 8))
            dt_action_step2();
        break;
    case 10:
        return dt_action_close();
    case 3:
        if (!(pad->b.trig & 8))
            dt_action_step3(pad);
        break;
    case 4:
        if (!(pad->b.trig & 8))
            dt_action_step4();
        break;
    case 5:
        dt_action_step5();
        break;
    }
    return 0;
}

void *dt_action_draw(void *list)
{
    menu_draw(&list, dt_action->menu);
    if (dt_action->sub != 0)
        menu_draw(&list, dt_action->sub);
    return list;
}

void dt_action_free(void)
{
    if (dt_action->sub != 0) {
        menu_reset(dt_action->sub);
        heap_set_level(dts.heap, dt_action->sub);
        dt_action->sub = 0;
    }
    menu_reset(dt_action->menu);
    if (dt_action->menu != 0) {
        heap_set_level(dts.heap, dt_action->menu);
        dt_action->menu = 0;
    }
    heap_set_level(dts.heap, dt_action);
    dt_action = 0;
}

/* step 0: the action menu itself */
void dt_action_step0(void)
{
    int r = menu_update(dt_action->menu);
    unsigned short id;

    if (r == -1)
        return;
    if (r >= 0 && dt_action->no_ent == 1)
        r += 2;
    switch (r) {
    case -2:
    case 5:
    case 6:
        menu_finish(dt_action->menu);
        dt_action->step = 10;
        return;
    }
    dt_action->menu->no_cancel = 0;
    switch (r) {
    case 0:
        dt_flags.info = 1;
        dt_action->menu->cancelled = dt_action->menu->accepted = 0;
        break;
    case 1:
        id = dt_action->ent->id;
        if (inv_get_flag7(id))
            inv_set_flag7(id, 0);
        else
            inv_set_flag7(id, 1);
        dt_action->menu->cancelled = dt_action->menu->accepted = 0;
        break;
    case 2:
        dt_action_menu_sub();
        se_play(0x6216);
        dt_action->step = 1;
        break;
    case 4:
        dt_action_layout_open();
        se_play(0x6216);
        dt_action->step = 3;
        break;
    case 3:
        dt_action_deck_open();
        se_play(0x6216);
        dt_action->step = 2;
        break;
    }
}

#ifdef NON_MATCHING /* o original grava o estado no atraso de cada desvio para o retorno comum (lui içado); mesma lógica */
/* step 10: wait for the close, then run the chosen action */
int dt_action_close(void)
{
    int r = menu_update(dt_action->menu);
    DtList *l;

    if (dt_action->menu->state == 5)
        return 0;
    if (r >= 0 && dt_action->no_ent == 1)
        r += 2;
    dt_action_free();
    switch (r) {
    case 5:
        l = dtv.pane[dtv.side].list;
        dt_list_filter_reset(l);
        dt_list_sort(l, l->sort, l->dir);
        break;
    case 6:
        if (dt_flight_undo())
            return 0;
        break;
    }
    dts.state = 0;
    return 0;
}
#endif

/* a sub-menu window beside the action menu */
int dt_sub_open(unsigned short title, unsigned short text, unsigned short count, MenuDrawItemFn draw)
{
    MenuWidget *m;
    MenuItemDef *def;

    if (dt_action->sub == 0)
        dt_action->sub = heap_alloc(dts.heap, 0x120);
    m = dt_action->sub;
    if (m == 0)
        return 0;
    def = &m->def;
    memset(m, 0, 0x120);
    def->visible = 1;
    def->style = 0;
    def->x = dt_action->menu->def.x + 0x28;
    def->y = 0x7C;
    def->auto_size = 1;
    def->color = 0xFFFFFFFF;
    if (text != 0)
        def->text = table_get_entry(dt_eres_get(0x8033), text);
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
        def->title = table_get_entry(dt_eres_get(0x8033), title);
    def->style2 = 2;
    def->title_font_size = 0xC;
    def->title_color = 0xFFFFFFFF;
    def->open_anim = 2;
    def->close_anim = 2;
    m->heap = dts.heap;
    m->no_cancel = 1;
    m->style = 2;
    m->count = count;
    m->item_w = 0xD4;
    m->item_h = 0x13;
    m->draw_item = draw;
    m->pad = (PadState *)&dt.pad;
    if (dt.step == 15)
        dt_menu_lock(m);
    return 1;
}

/* the sort / filter / search / mark submenu */
void dt_action_menu_sub(void)
{
    if (dt_sub_open(0x18, 0, 4, (MenuDrawItemFn)dt_draw_sub_item)) {
        dt_action->sub->keep_open = 1;
        menu_open(dt_action->sub);
    }
}

/* step 1 */
void dt_action_step1(void)
{
    MenuWidget *m = dt_action->sub;
    int r = menu_update(m);

    if (r == -1)
        return;
    if (m->state == 3) {
        menu_finish(m);
        switch (r) {
        case 0:
        case 1:
        case 2:
            menu_finish(dt_action->menu);
            break;
        }
        return;
    }
    menu_reset(m);
    heap_set_level(dts.heap, dt_action->sub);
    dt_action->sub = 0;
    switch (r) {
    case 0:
        dt_sort_menu_open();
        se_play(0x6216);
        dt_action_free();
        break;
    case 1:
        dt_fmenu_open();
        se_play(0x6216);
        dt_action_free();
        break;
    case 2:
        dt_word_open();
        se_play(0x6216);
        dt_action_free();
        break;
    case 3:
        dt_action_mark_open();
        se_play(0x6216);
        dt_action->step = 4;
        break;
    case -2:
        dt_action->step = 0;
        dt_action->menu->cancelled = dt_action->menu->accepted = 0;
        break;
    }
}

/* the deck operations submenu (the save is disabled while the deck is out of the rules) */
void dt_action_deck_open(void)
{
    if (dt_sub_open(0x1D, 0, 5, (MenuDrawItemFn)dt_draw_deck_item)) {
        if ((unsigned int)dts.list[0].total > 0x50 || (unsigned int)dts.list[1].total > 0x1E || (unsigned int)dts.list[2].total > 0xF)
            dt_action->sub->disabled |= 2;
        menu_open(dt_action->sub);
    }
}

/* step 2 */
void dt_action_step2(void)
{
    int done = 0;
    MenuWidget *m = dt_action->sub;
    int r = menu_update(m);

    if (r == -1)
        return;
    menu_reset(m);
    heap_set_level(dts.heap, dt_action->sub);
    dt_action->sub = 0;
    switch (r) {
    case 0:
        dt_flags.deck_op = 2;
        break;
    case 1:
        dt_flags.deck_op = 1;
        break;
    case 2:
        dt_lists_reset();
        dt_undo_count = 0;
        done = 1;
        break;
    case 3:
        dt_deck_clear();
        dt_undo_count = 0;
        done = 1;
        break;
    case 4:   /* no deck name input in the lesson */
        break;
    }
    if (done) {
        menu_finish(dt_action->menu);
        dt_action->step = 10;
    } else {
        dt_action->step = 0;
        dt_action->menu->cancelled = dt_action->menu->accepted = 0;
    }
}

/* the layout picker: 4 x 2 grid of pane layouts */
void dt_action_layout_open(void)
{
    MenuWidget *m;

    if (!dt_sub_open(0x1B, 0, 0, 0))
        return;
    m = dt_action->sub;
    m->def.auto_size = 0;
    m->def.w = 0x98;
    m->def.h = 0x30;
    m->def.x = dt_action->menu->def.x + 0x28;
    m->def.y = 0x7C;
    dt_action->mode = dt_mode_for(dtv.mode > 1 ? 2 : 1, dtv.mode < 4, dtv.pane[0].list_mode, dtv.pane[1].list_mode);
    m->cursor = dt_action->mode;
    if (dtv.side == 0) {
        dt_action->side = 1;
        dt_action->tab = dtv.pane[1].list->tab;
    } else {
        dt_action->side = 0;
        dt_action->tab = dtv.pane[0].list->tab;
    }
    m->no_input = 1;
    m->draw_all = (MenuDrawRectFn)dt_draw_layouts;
    menu_open(m);
}

/* step 3: d-pad picks a layout (previewed at once), accept keeps it, cancel restores */
void dt_action_step3(PadState *pad)
{
    MenuWidget *m = dt_action->sub;

    menu_update(m);
    if (m->state == 6) {
        if (m->cancelled) {
            dt_action_set_mode(dt_action->mode % 8);
            menu_reset(m);
            heap_set_level(dts.heap, dt_action->sub);
            dt_action->sub = 0;
            dt_action->step = 0;
            dt_action->menu->cancelled = dt_action->menu->accepted = 0;
        } else {
            if (dt_action->mode > 1 && dtv.mode < 2)
                dt_list_filter_reset(&dts.list[dt_action->tab]);
            menu_finish(dt_action->menu);
            dt_action->step = 10;
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
        dt_action_set_mode(m->cursor % 8);
        se_play(0x6212);
        return;
    }
    if (pad->b.rep & 0x80) {
        m->cursor = (m->cursor % 4 + 3) % 4 + m->cursor / 4 * 4;
        dt_action_set_mode(m->cursor % 8);
        se_play(0x6212);
        return;
    }
    if (pad->b.rep & 0x20) {
        m->cursor = (m->cursor % 4 + 1) % 4 + m->cursor / 4 * 4;
        dt_action_set_mode(m->cursor % 8);
        se_play(0x6212);
    }
}

/* the "new" mark picker */
void dt_action_mark_open(void)
{
    MenuWidget *m;

    if (dt_sub_open(0x4B, 0x54, 3, (MenuDrawItemFn)dt_draw_mark_item)) {
        m = dt_action->sub;
        m->def.max_w = 0xF4;
        m->item_w = 0xF4;
        m->selected = 1 << cfg_get_byte0_hi2();
        menu_open(m);
    }
}

/* step 4 */
void dt_action_step4(void)
{
    MenuWidget *m = dt_action->sub;
    int r = menu_update(m);

    if (r == -1)
        return;
    menu_reset(m);
    switch (r) {
    case -2:
        dt_action_menu_sub();
        dt_action->step = 1;
        break;
    default:
        dt_action->mark = r;
        dt_action_confirm_open();
        se_play(0x621A);
        dt_action->step = 5;
        break;
    }
}

/* yes/no window for the mark */
void dt_action_confirm_open(void)
{
    MenuWidget *m;
    MenuItemDef *def;
    int x;

    if (dt_action->sub == 0)
        dt_action->sub = heap_alloc(dts.heap, 0x120);
    m = dt_action->sub;
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
    def->text = table_get_entry(dt_eres_get(0x8033), 0x53);
    def->open_anim = 2;
    def->close_anim = 2;
    m->heap = dts.heap;
    m->no_cancel = 1;
    m->buttons = 3;
    m->initial = 3;
    m->cancel_resets = 1;
    m->pad = (PadState *)&dt.pad;
    if (dt.step == 15)
        dt_menu_lock(m);
    menu_open(m);
    x = 0x1E0 - def->w;
    def->x = x / 2;
    x = 0x110 - def->h;
    def->y = x / 2;
}

/* step 5: yes stores the mark and drops the marks of the trunk list */
void dt_action_step5(void)
{
    MenuWidget *m = dt_action->sub;
    int r = menu_update(m);
    DtList *l;

    if (r == -1)
        return;
    menu_reset(m);
    switch (r) {
    case -2:
    case 3:
        dt_action->step = 0;
        dt_action->menu->cancelled = dt_action->menu->accepted = 0;
        break;
    case 2:
        cfg_set_byte0_hi2(dt_action->mark);
        l = &dts.list[3];
        dt_list_filter_reset(l);
        if (dtv.pane[0].list->tab == 3) {
            dtv.pane[0].cy = 0;
            dtv.pane[0].cx = 0;
            dtv.pane[0].top = 0;
        }
        if (dtv.pane[1].list->tab == 3) {
            dtv.pane[1].cy = 0;
            dtv.pane[1].cx = 0;
            dtv.pane[1].top = 0;
        }
        l->u18 = l->u1a = l->u1c_lo = l->u1c_hi = 0;
        heap_set_level(dts.heap, dt_action->sub);
        dt_action->sub = 0;
        menu_finish(dt_action->menu);
        dt_action->step = 10;
        break;
    }
}

/* a two-tone rule under a menu item */
void *dt_draw_rule(void *list, DtLineVtx *v, int x, int y, int w)
{
    int i;
    DtLineVtx *p;

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
void *dt_action_draw_items(void *list, int x, int y, int w, int h)
{
    MenuWidget *m = dt_action->menu;
    unsigned int first = 0;
    int n = 7;
    int i;
    unsigned int k;
    int dx;

    if (!dt_action->no_ent) {
        list = dt_draw_card_pict(list, x + 2, y + 2, 1, 0xFFFFFFFF, dt_action->ent, dt_eres_get(0x8001), dt_eres_get(0x8002), dt_eres_get(0x8032));
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
        list = dt_draw_action_item(list, k, x + dx, y);
        y += m->item_h;
        switch (k) {
        case 1:
            list = dt_draw_rule(list, dl_alloc_inline(&list, 0x30), x, y + 1, m->item_w);
            y += 4;
            break;
        case 4:
            list = dt_draw_rule(list, dl_alloc_inline(&list, 0x30), x, y + 1, m->item_w);
            y += 4;
            break;
        }
    }
    return list;
}

DtButton dt_action_items[7] = {
    { 0x800B, 0x33, 0x11, 0x50 }, { 0x800B, 0x44, 0x11, 0x51 }, { 0x800B, 0x33, 0x22, 0x18 },
    { 0x800B, 0x44, 0x00, 0x1D }, { 0x800B, 0x22, 0x22, 0x1B }, { 0x800B, 0x55, 0x00, 0x70 },
    { 0x800B, 0x00, 0x00, 0x1A },
};

/* item i of the action menu, greyed when disabled */
void *dt_draw_action_item(void *list, unsigned short i, int x, int y)
{
    DtButton *b = &dt_action_items[i];
    unsigned int color = 0;

    switch (i) {
    case 6:
    case 5:
        if (dt_action->menu->disabled & 1 << i)
            color = 0xFF808080;
        break;
    }
    return dt_draw_ibutton(list, b, x, y + 1, color);
}

/* the layout picker contents: cursor and the 8 layout icons */
void *dt_draw_layouts(void *list, int x, int y)
{
    void *gim = dt_eres_get(0x800A);
    int px = x;
    MenuWidget *m = dt_action->sub;
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

DtButton dt_sub_items[4] = {
    { 0x800B, 0x11, 0x00, 0x55 }, { 0x800B, 0x22, 0x00, 0x56 }, { 0x8008, 0x66, 0x31, 0x12 }, { 0x800B, 0x22, 0x11, 0x4B },
};

void *dt_draw_sub_item(void *list, unsigned short i, int x, int y)
{
    return dt_draw_ibutton(list, &dt_sub_items[i], x, y + 1, 0);
}

DtButton dt_deck_items[5] = {
    { 0x800B, 0x55, 0x44, 0x46 }, { 0x800B, 0x66, 0x44, 0x47 }, { 0x800B, 0x00, 0x11, 0x49 },
    { 0x800B, 0x11, 0x11, 0x4A }, { 0x800B, 0x66, 0x00, 0x48 },
};

/* item i of the deck operations submenu, greyed when disabled */
void *dt_draw_deck_item(void *list, unsigned short i, int x, int y)
{
    DtButton *b = &dt_deck_items[i];
    unsigned int color = 0;

    switch (i) {
    case 1:
        if (dt_action->sub->disabled & 1 << i)
            color = 0xFF808080;
        break;
    }
    return dt_draw_ibutton(list, b, x, y + 1, color);
}

DtButton dt_mark_items[3] = { { 0x800A, 0x11, 0x22, 0x4C }, { 0x800A, 0x22, 0x22, 0x4D }, { 0x800A, 0x00, 0x22, 0x4E } };

void *dt_draw_mark_item(void *list, unsigned short i, int x, int y)
{
    return dt_draw_ibutton(list, &dt_mark_items[i], x, y + 1, 0);
}

/* sets the pane layout; going from one list to two in the narrow layouts shows the list picked */
void dt_action_set_mode(int mode)
{
    unsigned short panes = dtv.mode > 1 ? 2 : 1;

    dt_set_mode(mode);
    if (dtv.side == 1 && dtv.mode > 1 && panes == 1)
        dt_select_tab(0, dt_action->tab, 2);
}

