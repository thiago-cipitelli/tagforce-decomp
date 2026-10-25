// @range 0x08810CD0-0x08812510
// @asm 0x088116CC-0x08811720
// @asm 0x08811CFC-0x088123B8
// @data 0x08822F14-0x08822F94
/* The sort menu (a list of the sort keys and a small up/down menu beside it; the pane is re-sorted
   live while the cursor moves and restored on cancel) and the filter menu's kinds list (open,
   per-frame update, values window, teardown), with their item icons. */
#include "libdeck.h"
#include "menu.h"

typedef struct DkButton {
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} DkButton;

typedef struct DkSortMenu {            /* dk_sort_menu 0x095D2714, 0x18 bytes, heap allocated */
    unsigned short step;               /* +0x00 0 open, 1 closing */
    unsigned char focus : 2;           /* +0x02 0 key list, 1 direction */
    unsigned char pad02 : 6;
    unsigned char pad03;
    unsigned short *order;             /* +0x04 order before the menu */
    unsigned char sort : 6;            /* +0x08 sort before the menu */
    unsigned char dir : 2;
    unsigned char pad09[3];
    int base_y;                        /* +0x0C */
    MenuWidget *keys;                  /* +0x10 */
    MenuWidget *dirs;                  /* +0x14 */
} DkSortMenu;

extern DkSortMenu *dk_sort_menu;

void *dk_draw_button(void *list, DkButton *b, int x, int y, unsigned int color);
void *dk_draw_shade(void *list, unsigned short side, unsigned short tab, int dim);
void dk_list_sort(DkList *l, unsigned short sort, unsigned short dir);
void dk_sort_menu_place(void);
void dk_sort_menu_free(void);
void *dk_draw_sort_item(void *list, int i, int x, int y);
void *dk_draw_dir_item(void *list, int i, int x, int y);
void *dk_draw_fmenu_kind(void *list, int i, int x, int y);
void dk_fmenu_values(PadState *pad);
void dk_list_filter(DkList *l, unsigned int f, unsigned int *mask);
void dk_layout_scroll(int keep, int cur, unsigned short view, unsigned short total,
                      unsigned short *top, unsigned short *cx, unsigned short *cy);
void dk_popup_open(int kind, unsigned short n);

void dk_fmenu_free(void);
void dk_fmenu_open_values(unsigned short f);
int font_measure_text(unsigned short *text);
int card_get_flag31(int id);

/* opens the sort menu on the pane with the cursor */
void dk_sort_menu_open(void)
{
    DkList *l = dkv.pane[dkv.side].list;
    MenuItemDef *d;
    MenuWidget *m;
    int a;
    int b;

    dk_sort_menu = heap_alloc(dk_heap, 0x18);
    if (dk_sort_menu == 0)
        return;
    memset(dk_sort_menu, 0, 0x18);
    dk_sort_menu->keys = heap_alloc(dk_heap, 0x120);
    m = dk_sort_menu->keys;
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
        d->title = table_get_entry(dk_res_get(0x8033), 0x55);
        d->title_font_size = 0xC;
        d->title_color = 0xFFFFFFFF;
        m->heap = dk_heap;
        m->keep_open = 1;
        m->no_cancel = 1;
        m->style = 2;
        m->count = 9;
        m->item_w = 0x88;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)dk_draw_sort_item;
        menu_open(m);
        d->x = (0x1E0 - d->w) / 2;
        dk_sort_menu->base_y = menu_cursor_y(&m->menu, d) + 1;
    }
    dk_sort_menu->dirs = heap_alloc(dk_heap, 0x120);
    m = dk_sort_menu->dirs;
    if (m != 0) {
        d = &m->def;
        memset(m, 0, 0x120);
        d->visible = 1;
        d->style = 0;
        d->x = dk_sort_menu->keys->def.x + dk_sort_menu->keys->def.w - 4;
        d->pad_t = 2;
        d->pad_b = 2;
        d->pad_l = 2;
        d->pad_r = 2;
        d->auto_size = 1;
        d->color = 0xFFFFFFFF;
        d->open_anim = 1;
        d->close_anim = 1;
        m->heap = dk_heap;
        m->keep_open = 1;
        m->style = 2;
        m->count = 2;
        font_set_size(12, 12);
        a = font_measure_text(table_get_entry(dk_res_get(0x8033), 0x14));
        b = font_measure_text(table_get_entry(dk_res_get(0x8033), 0x15));
        a = a > b ? a : b;
        m->item_w = (a >> 6) + 0x19;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)dk_draw_dir_item;
        menu_open(m);
    }
    dk_sort_menu->order = heap_alloc(dk_heap, l->count << 1);
    if (dk_sort_menu->order != 0)
        memcpy(dk_sort_menu->order, l->order, l->count << 1);
    dk_sort_menu->sort = l->sort;
    dk_sort_menu->dir = l->dir;
    dk_sort_menu->keys->cursor = dk_sort_menu->sort - 1;
    dk_sort_menu->dirs->cursor = dk_sort_menu->dir;
    dk_sort_menu_place();
    dk_state = 3;
    dkv.lock = 1;
}

/* state 3 */
void dk_sort_menu_update(PadState *pad)
{
    DkList *l = dkv.pane[dkv.side].list;
    unsigned int k0 = dk_sort_menu->keys->cursor;
    unsigned int d0 = dk_sort_menu->dirs->cursor;
    int r;

    switch (dk_sort_menu->step) {
    case 0:
        dk_sort_menu_place();
        switch (dk_sort_menu->focus) {
        case 0:
            r = menu_update(dk_sort_menu->keys);
            if (pad->b.trig & 0x20) {
                dk_sort_menu->keys->no_cancel = 0;
                dk_sort_menu->dirs->no_cancel = 1;
                dk_sort_menu->focus = 1;
                se_play(0x6212);
            }
            if (pad->b.trig & 0x300) {
                dk_sort_menu->dirs->cursor = (dk_sort_menu->dirs->cursor + 1) & 1;
                se_play(0x6212);
            }
            break;
        case 1:
            r = menu_update(dk_sort_menu->dirs);
            if (pad->b.trig & 0x80) {
                dk_sort_menu->dirs->no_cancel = 0;
                dk_sort_menu->keys->no_cancel = 1;
                dk_sort_menu->focus = 0;
                se_play(0x6212);
            }
            break;
        }
        if (k0 != dk_sort_menu->keys->cursor || d0 != dk_sort_menu->dirs->cursor) {
            memcpy(l->order, dk_sort_menu->order, l->count << 1);
            dk_list_sort(l, dk_sort_menu->keys->cursor + 1, dk_sort_menu->dirs->cursor);
        }
        if (r != -1) {
            menu_finish(dk_sort_menu->keys);
            menu_finish(dk_sort_menu->dirs);
            dk_sort_menu->step = 1;
            if (r == -2) {
                memcpy(l->order, dk_sort_menu->order, l->count << 1);
                l->sort = dk_sort_menu->sort;
                l->dir = dk_sort_menu->dir;
            } else {
                memcpy(l->order, dk_sort_menu->order, l->count << 1);
                dk_list_sort(l, dk_sort_menu->keys->cursor + 1, dk_sort_menu->dirs->cursor);
            }
        }
        break;
    case 1:
        menu_update(dk_sort_menu->keys);
        menu_update(dk_sort_menu->dirs);
        if (dk_sort_menu->keys->state == 6) {
            dk_sort_menu_free();
            dkv.lock = 0;
            dk_state = 0;
        }
        break;
    }
}

void dk_sort_menu_free(void)
{
    menu_reset(dk_sort_menu->dirs);
    menu_reset(dk_sort_menu->keys);
    heap_set_level(dk_heap, dk_sort_menu->dirs);
    heap_set_level(dk_heap, dk_sort_menu->keys);
    heap_set_level(dk_heap, dk_sort_menu);
    dk_sort_menu = 0;
}

void *dk_sort_menu_draw(void *list)
{
    unsigned int side;

    if (dkv.mode > 1) {
        side = (dkv.side + 1) & 1;
        list = dk_draw_shade(list, side, dkv.pane[side].list->tab, 1);
    }
    menu_draw(&list, dk_sort_menu->keys);
    menu_draw(&list, dk_sort_menu->dirs);
    return list;
}

DkButton dk_sort_icons[10] = {
    { 0x8000, 0x00, 0x00, 0x00 }, { 0x8008, 0x00, 0x20, 0x06 }, { 0x8008, 0x44, 0x20, 0x07 }, { 0x8008, 0x33, 0x20, 0x08 },
    { 0x8008, 0x11, 0x20, 0x09 }, { 0x8008, 0x22, 0x20, 0x0A }, { 0x8008, 0x66, 0x20, 0x0B }, { 0x8008, 0x55, 0x20, 0x0C },
    { 0x8008, 0x44, 0x31, 0x0D }, { 0x8008, 0x11, 0x31, 0x10 },
};

DkButton dk_dir_icons[2] = { { 0x8008, 0x33, 0x31, 0x14 }, { 0x8008, 0x22, 0x31, 0x15 } };

void *dk_draw_sort_item(void *list, int i, int x, int y)
{
    return dk_draw_button(list, &dk_sort_icons[i + 1], x, y + 1, 0);
}

void *dk_draw_dir_item(void *list, int i, int x, int y)
{
    return dk_draw_button(list, &dk_dir_icons[i], x, y + 1, 0);
}

#ifdef NON_MATCHING /* temporários a1/a2/a3 trocados (como sc_sort_menu_place) */
/* keeps the direction menu beside the key under the cursor */
void dk_sort_menu_place(void)
{
    MenuWidget *k = dk_sort_menu->keys;
    MenuWidget *d = dk_sort_menu->dirs;
    int base = dk_sort_menu->base_y;
    unsigned int h = k->item_h;

    d->def.y = base + ((h >> 1) + k->cursor * h) - d->def.h / 2;
}
#endif

void *dk_draw_sort_icon(void *list, int sort, int x, int y)
{
    DkButton *b;
    void *gim;

    if (sort == 0)
        return list;
    {
        b = &dk_sort_icons[sort];
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

void *dk_draw_dir_icon(void *list, int dir, int x, int y)
{
    DkButton *b = &dk_dir_icons[dir];
    void *gim = dk_res_get(b->res);

    if (gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 17, 17, 17, 17);
        list = draw_sprite(list, x, y, 0, b->u, b->v, 0, 0);
    }
    return list;
}

/* opens the filter menu (kinds list) */
void dk_fmenu_open(void)
{
    DkList *l = dkv.pane[dkv.side].list;
    MenuWidget *m;
    MenuItemDef *d;
    int id;
    int f;

    dk_fmenu = heap_alloc(dk_heap, 0x18);
    if (dk_fmenu == 0)
        return;
    memset(dk_fmenu, 0, 0x18);
    dk_fmenu->kinds = heap_alloc(dk_heap, 0x120);
    m = dk_fmenu->kinds;
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
        d->title = table_get_entry(dk_res_get(0x8033), 0x56);
        d->title_font_size = 0xC;
        d->title_color = 0xFFFFFFFF;
        m->heap = dk_heap;
        m->keep_open = 1;
        m->no_cancel = 1;
        m->style = 2;
        m->count = 7;
        m->item_w = 0xA4;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)dk_draw_fmenu_kind;
        menu_open(m);
        d->x = (0x1E0 - d->w) / 2;
    }
    for (id = 0xFA7; id < 0x1B2E; id++) {
        if (card_get_flag31(id) && inv_get_bit3(id)) {
            dk_fmenu->alt = 1;
            break;
        }
    }
    f = l->filter - 1;
    if (f >= 0) {
        if (f >= 7)
            f = 0;
    } else
        f = 0;
    dk_fmenu->kinds->cursor = f;
    dk_state = 4;
}

/* state 4 */
void dk_fmenu_update(PadState *pad)
{
    int r;

    switch (dk_fmenu->step) {
    case 0:
        r = menu_update(dk_fmenu->kinds);
        if (r == -1)
            break;
        if (r == -2) {
            menu_finish(dk_fmenu->kinds);
            dk_fmenu->step = 2;
        } else {
            dk_fmenu->kinds->no_cancel = 0;
            dk_fmenu_open_values(r + 1);
            se_play(0x6216);
            dk_fmenu->col = dk_fmenu->page = 0;
            dk_fmenu->mask[0] = 0;
            dk_fmenu->mask[1] = 0;
            dk_fmenu->sub = 0;
            dk_fmenu->step = 1;
        }
        break;
    case 1:
        dk_fmenu_values(pad);
        break;
    case 2:
        if (menu_update(dk_fmenu->kinds) != -1) {
            dk_fmenu_free();
            dk_state = 0;
        }
        break;
    }
}

#ifdef NON_MATCHING /* máquina de passos grande; forma próxima, alocação e blocos diferentes (como sc_fmenu_values) */
/* the values window of filter kind (cursor of the kinds list) + 1 */
void dk_fmenu_values(PadState *pad)
{
    MenuWidget *m = dk_fmenu->vals;
    unsigned int f = dk_fmenu->kinds->cursor + 1;
    DkFilterDef *def = &dk_filter_defs[f];
    unsigned int side;
    DkList *l;
    int n;
    unsigned int cols;
    int rows;
    unsigned int old;
    unsigned int v;
    unsigned short view;
    unsigned short grid;

    switch (dk_fmenu->sub) {
    case 0:
        if (pad->b.trig & region_pick_value(0)) {
            if (dk_fmenu->page == 0xF) {
                menu_set_cursor(dk_fmenu->vals, 0);
                dk_fmenu->sub = 1;
            } else {
                v = def->first + dk_fmenu->col + dk_fmenu->page * m->count;
                dk_fmenu->mask[(int)v >> 5] ^= 1 << (v & 0x1F);
            }
            se_play(0x6213);
            break;
        }
        if (pad->b.trig & region_pick_value(1)) {
            m->cancelled = 1;
            menu_finish(m);
            dk_fmenu->sub = 3;
            se_play(0x6214);
            break;
        }
        n = def->last - def->first + 1;
        if (!dk_fmenu->alt && f == 7)
            n--;
        cols = m->count;
        rows = (int)(n + cols - 1) / (int)cols;
        if (pad->b.rep & 0x10) {
            old = dk_fmenu->page;
            if (old == 0) {
                dk_fmenu->page = 0xF;
                se_play(0x6212);
            } else {
                if (old == 0xF)
                    dk_fmenu->page = rows - 1;
                else
                    dk_fmenu->page = old - 1;
                if ((int)(dk_fmenu->col + dk_fmenu->page * cols) >= n)
                    dk_fmenu->page = dk_fmenu->page - 1;
                if (old != dk_fmenu->page)
                    se_play(0x6212);
            }
        }
        if (pad->b.rep & 0x40) {
            old = dk_fmenu->page;
            if (old == 0xF)
                dk_fmenu->page = 0;
            else
                dk_fmenu->page = old + 1;
            if ((int)(dk_fmenu->col + dk_fmenu->page * cols) >= n)
                dk_fmenu->page = 0xF;
            if (old != dk_fmenu->page)
                se_play(0x6212);
        }
        if (dk_fmenu->page == rows - 1u) {
            cols = n % (int)m->count;
            if (cols == 0)
                cols = m->count;
        }
        if (dk_fmenu->page != 0xF) {
            old = dk_fmenu->col;
            if (pad->b.rep & 0x80)
                dk_fmenu->col = (int)(old + cols - 1) % (int)cols;
            if (pad->b.rep & 0x20)
                dk_fmenu->col = (int)(dk_fmenu->col + 1) % (int)cols;
            if (old != dk_fmenu->col)
                se_play(0x6212);
        }
        break;
    case 1:
        menu_update(m);
        if (dk_fmenu->vals->state != 4) {
            menu_finish(m);
            menu_finish(dk_fmenu->kinds);
            dk_fmenu->sub = 2;
        }
        break;
    case 2:
        menu_update(m);
        if (m->state == 6) {
            side = dkv.side;
            l = dkv.pane[side].list;
            if (l->count == 0) {
                l->filter = f;
            } else {
                dk_list_filter(l, f, dk_fmenu->mask);
                view = dkv.pane[side].list_mode == 0 ? dkv.pane[side].lay.grid_rows : dkv.pane[side].lay.rows;
                grid = dkv.pane[side].list_mode == 0 ? dkv.pane[side].lay.cols : 1;
                dk_layout_scroll(0, 0, (grid & 0xFF) | view << 8, dkv.pane[side].list->count, &dkv.pane[side].top,
                                 &dkv.pane[side].cx, &dkv.pane[side].cy);
            }
            dk_fmenu_free();
            dk_popup_open(6, l->count);
            se_play(0x621A);
            dk_state = 0;
        }
        break;
    case 3:
        menu_update(m);
        if (m->state == 6) {
            menu_reset(m);
            dk_fmenu->vals = 0;
            dk_fmenu->kinds->accepted = 0;
            dk_fmenu->kinds->cancelled = 0;
            dk_fmenu->kinds->no_cancel = 1;
            dk_fmenu->step = 0;
        }
        break;
    }
}
#endif

void dk_fmenu_free(void)
{
    if (dk_fmenu->vals != 0) {
        menu_reset(dk_fmenu->vals);
        heap_set_level(dk_heap, dk_fmenu->vals);
    }
    menu_reset(dk_fmenu->kinds);
    heap_set_level(dk_heap, dk_fmenu->kinds);
    heap_set_level(dk_heap, dk_fmenu);
    dk_fmenu = 0;
}

void *dk_fmenu_draw(void *list)
{
    unsigned int side;

    if (dkv.mode > 1) {
        side = (dkv.side + 1) & 1;
        list = dk_draw_shade(list, side, dkv.pane[side].list->tab, 1);
    }
    menu_draw(&list, dk_fmenu->kinds);
    if (dk_fmenu->vals != 0)
        menu_draw(&list, dk_fmenu->vals);
    return list;
}

DkButton dk_filter_icons[9] = {
    { 0x8000, 0x00, 0x00, 0x00 }, { 0x8008, 0x44, 0x20, 0x07 }, { 0x8008, 0x66, 0x20, 0x0B }, { 0x8008, 0x55, 0x20, 0x0C },
    { 0x8008, 0x33, 0x20, 0x08 }, { 0x800B, 0x00, 0x33, 0x2A }, { 0x8008, 0x11, 0x31, 0x10 }, { 0x800B, 0x44, 0x22, 0x2B },
    { 0x8008, 0x66, 0x31, 0x12 },
};

void *dk_draw_fmenu_kind(void *list, int i, int x, int y)
{
    return dk_draw_button(list, &dk_filter_icons[i + 1], x, y + 1, 0);
}
