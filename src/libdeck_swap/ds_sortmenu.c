// @range 0x0880AB28-0x0880B750
// @asm 0x0880B528-0x0880B57C
// @data 0x0881A1A0-0x0881A1E8
/* The sort menu (state 4): a list of the nine sort keys and a small up/down menu beside it; the
   pane is re-sorted live while the cursor moves, and restored on cancel. Sort/direction icons
   (same source as libselect_card sc_sort.c and libdeck dk_sortmenu.c). */
#include "libdeck_swap.h"
#include "menu.h"

typedef struct DsButton {
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} DsButton;

typedef struct DsSortMenu {            /* ds_sort_menu 0x0881A404, 0x18 bytes, heap allocated */
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
} DsSortMenu;

extern DsSortMenu *ds_sort_menu;

void *ds_draw_button(void *list, DsButton *b, int x, int y, unsigned int color);
void *ds_draw_shade(void *list, unsigned short side, unsigned short tab);
void ds_list_sort(DsList *l, unsigned short sort, unsigned short dir);
void ds_sort_menu_place(void);
void ds_sort_menu_free(void);
void *ds_draw_sort_item(void *list, int i, int x, int y);
void *ds_draw_dir_item(void *list, int i, int x, int y);
int font_measure_text(unsigned short *text);

void ds_sort_menu_open(void)
{
    DsList *l = dsv.pane[dsv.side].list;
    MenuItemDef *d;
    MenuWidget *m;
    int a;
    int b;

    ds_sort_menu = heap_alloc(ds.heap, 0x18);
    if (ds_sort_menu == 0)
        return;
    memset(ds_sort_menu, 0, 0x18);
    ds_sort_menu->keys = heap_alloc(ds.heap, 0x120);
    m = ds_sort_menu->keys;
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
        d->title = table_get_entry(res_get(0x8033), 0x55);
        d->title_font_size = 0xC;
        d->title_color = 0xFFFFFFFF;
        m->heap = ds.heap;
        m->keep_open = 1;
        m->no_cancel = 1;
        m->style = 2;
        m->count = 9;
        m->item_w = 0x88;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)ds_draw_sort_item;
        menu_open(m);
        d->x = (0x1E0 - d->w) / 2;
        ds_sort_menu->base_y = menu_cursor_y(&m->menu, d) + 1;
    }
    ds_sort_menu->dirs = heap_alloc(ds.heap, 0x120);
    m = ds_sort_menu->dirs;
    if (m != 0) {
        d = &m->def;
        memset(m, 0, 0x120);
        d->visible = 1;
        d->style = 0;
        d->x = ds_sort_menu->keys->def.x + ds_sort_menu->keys->def.w - 4;
        d->pad_t = 2;
        d->pad_b = 2;
        d->pad_l = 2;
        d->pad_r = 2;
        d->auto_size = 1;
        d->color = 0xFFFFFFFF;
        d->open_anim = 1;
        d->close_anim = 1;
        m->heap = ds.heap;
        m->keep_open = 1;
        m->style = 2;
        m->count = 2;
        font_set_size(12, 12);
        a = font_measure_text(table_get_entry(res_get(0x8033), 0x14));
        b = font_measure_text(table_get_entry(res_get(0x8033), 0x15));
        a = a > b ? a : b;
        m->item_w = (a >> 6) + 0x19;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)ds_draw_dir_item;
        menu_open(m);
    }
    ds_sort_menu->order = heap_alloc(ds.heap, l->count << 1);
    if (ds_sort_menu->order != 0)
        memcpy(ds_sort_menu->order, l->order, l->count << 1);
    ds_sort_menu->sort = l->sort;
    ds_sort_menu->dir = l->dir;
    ds_sort_menu->keys->cursor = ds_sort_menu->sort - 1;
    ds_sort_menu->dirs->cursor = ds_sort_menu->dir;
    ds_sort_menu_place();
    ds.state = 4;
    dsv.lock = 1;
}

/* state 4 */
void ds_sort_menu_update(void)
{
    PadState *pad = pad_get();
    DsList *l = dsv.pane[dsv.side].list;
    unsigned int k0 = ds_sort_menu->keys->cursor;
    unsigned int d0 = ds_sort_menu->dirs->cursor;
    int r;

    switch (ds_sort_menu->step) {
    case 0:
        ds_sort_menu_place();
        switch (ds_sort_menu->focus) {
        case 0:
            r = menu_update(ds_sort_menu->keys);
            if (pad->b.trig & 0x20) {
                ds_sort_menu->keys->no_cancel = 0;
                ds_sort_menu->dirs->no_cancel = 1;
                ds_sort_menu->focus = 1;
                se_play(0x6212);
            }
            if (pad->b.trig & 0x300) {
                ds_sort_menu->dirs->cursor = (ds_sort_menu->dirs->cursor + 1) & 1;
                se_play(0x6212);
            }
            break;
        case 1:
            r = menu_update(ds_sort_menu->dirs);
            if (pad->b.trig & 0x80) {
                ds_sort_menu->dirs->no_cancel = 0;
                ds_sort_menu->keys->no_cancel = 1;
                ds_sort_menu->focus = 0;
                se_play(0x6212);
            }
            break;
        }
        if (k0 != ds_sort_menu->keys->cursor || d0 != ds_sort_menu->dirs->cursor) {
            memcpy(l->order, ds_sort_menu->order, l->count << 1);
            ds_list_sort(l, ds_sort_menu->keys->cursor + 1, ds_sort_menu->dirs->cursor);
        }
        if (r != -1) {
            menu_finish(ds_sort_menu->keys);
            menu_finish(ds_sort_menu->dirs);
            ds_sort_menu->step = 1;
            if (r == -2) {
                memcpy(l->order, ds_sort_menu->order, l->count << 1);
                l->sort = ds_sort_menu->sort;
                l->dir = ds_sort_menu->dir;
            } else {
                memcpy(l->order, ds_sort_menu->order, l->count << 1);
                ds_list_sort(l, ds_sort_menu->keys->cursor + 1, ds_sort_menu->dirs->cursor);
            }
        }
        break;
    case 1:
        menu_update(ds_sort_menu->keys);
        menu_update(ds_sort_menu->dirs);
        if (ds_sort_menu->keys->state == 6) {
            ds_sort_menu_free();
            dsv.lock = 0;
            ds.state = 1;
        }
        break;
    }
}

void ds_sort_menu_free(void)
{
    menu_reset(ds_sort_menu->dirs);
    menu_reset(ds_sort_menu->keys);
    heap_set_level(ds.heap, ds_sort_menu->dirs);
    heap_set_level(ds.heap, ds_sort_menu->keys);
    heap_set_level(ds.heap, ds_sort_menu);
    ds_sort_menu = 0;
}

void *ds_sort_menu_draw(void *list)
{
    unsigned int side;

    if (dsv.mode > 1) {
        side = (dsv.side + 1) & 1;
        list = ds_draw_shade(list, side, dsv.pane[side].list->tab);
    }
    menu_draw(&list, ds_sort_menu->keys);
    menu_draw(&list, ds_sort_menu->dirs);
    return list;
}

DsButton ds_sort_icons[10] = {
    { 0x8000, 0x00, 0x00, 0x00 }, { 0x8008, 0x00, 0x20, 0x06 }, { 0x8008, 0x44, 0x20, 0x07 }, { 0x8008, 0x33, 0x20, 0x08 },
    { 0x8008, 0x11, 0x20, 0x09 }, { 0x8008, 0x22, 0x20, 0x0A }, { 0x8008, 0x66, 0x20, 0x0B }, { 0x8008, 0x55, 0x20, 0x0C },
    { 0x8008, 0x44, 0x31, 0x0D }, { 0x8008, 0x11, 0x31, 0x10 },
};

DsButton ds_dir_icons[2] = { { 0x8008, 0x33, 0x31, 0x14 }, { 0x8008, 0x22, 0x31, 0x15 } };

void *ds_draw_sort_item(void *list, int i, int x, int y)
{
    return ds_draw_button(list, &ds_sort_icons[i + 1], x, y + 1, 0);
}

void *ds_draw_dir_item(void *list, int i, int x, int y)
{
    return ds_draw_button(list, &ds_dir_icons[i], x, y + 1, 0);
}

#ifdef NON_MATCHING /* temporários a1/a2/a3 trocados */
/* keeps the direction menu beside the key under the cursor */
void ds_sort_menu_place(void)
{
    MenuWidget *k = ds_sort_menu->keys;
    MenuWidget *d = ds_sort_menu->dirs;
    int base = ds_sort_menu->base_y;
    unsigned int h = k->item_h;

    d->def.y = base + ((h >> 1) + k->cursor * h) - d->def.h / 2;
}
#endif

void *ds_draw_sort_icon(void *list, int sort, int x, int y)
{
    DsButton *b;
    void *gim;

    if (sort == 0)
        return list;
    {
        b = &ds_sort_icons[sort];
        gim = res_get(b->res);
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

void *ds_draw_dir_icon(void *list, int dir, int x, int y)
{
    DsButton *b = &ds_dir_icons[dir];
    void *gim = res_get(b->res);

    if (gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 17, 17, 17, 17);
        list = draw_sprite(list, x, y, 0, b->u, b->v, 0, 0);
    }
    return list;
}
