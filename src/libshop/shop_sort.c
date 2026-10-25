// @range 0x08813FF4-0x08814BE8
// @asm 0x088149A0-0x088149F4
// @data 0x08820494-0x088204F4
/* The sort dialog of the bought list: a window with the nine sort keys and, next to the cursor, a
   small window with the direction; the list is reordered live and restored on cancel. */
#include "libshop.h"

typedef struct ShopSort {       /* shop_sort 0x08F8EE08, heap allocated */
    unsigned short step;        /* +0x00 0 open, 1 closing */
    unsigned char focus : 2;    /* +0x02 0 keys, 1 direction */
    unsigned char pad02 : 6;
    unsigned char pad03;
    unsigned short *order;      /* +0x04 display order when the dialog opened */
    unsigned char key : 6;      /* +0x08 sort key and direction when the dialog opened */
    unsigned char dir : 2;
    unsigned char pad09[3];
    int y;                      /* +0x0C cursor y of the key list */
    MenuWidget *menu;           /* +0x10 keys */
    MenuWidget *sub;            /* +0x14 direction */
} ShopSort;

extern ShopSort *shop_sort;

ShopBox shop_sort_keys[14] = {
    { 0x2000, 0, 0, 0 },
    { 0x2027, 0x55, 0x22, 0x14 },
    { 0x2028, 0x00, 0x00, 0x12 },
    { 0x2027, 0x66, 0x22, 0x13 },
    { 0x2027, 0x44, 0x11, 0x15 },
    { 0x2024, 0x00, 0x20, 0x09 },
    { 0x2024, 0x44, 0x20, 0x0A },
    { 0x2024, 0x33, 0x20, 0x0B },
    { 0x2024, 0x11, 0x20, 0x0C },
    { 0x2024, 0x22, 0x20, 0x0D },
    { 0x2024, 0x66, 0x20, 0x0E },
    { 0x2024, 0x55, 0x20, 0x0F },
    { 0x2024, 0x44, 0x31, 0x10 },
    { 0x2024, 0x11, 0x31, 0x11 },
};

ShopBox shop_sort_dirs[2] = {
    { 0x2024, 0x33, 0x31, 0x16 },
    { 0x2024, 0x22, 0x31, 0x17 },
};

int menu_update(MenuWidget *w);
void menu_finish(MenuWidget *w);
void menu_open(MenuWidget *w);
void menu_reset(MenuWidget *w);
void menu_draw(void **dl, MenuWidget *w);
void shop_list_sort(unsigned short key, unsigned short dir);

void shop_sort_close(void);
void shop_sort_layout(void);
unsigned int *shop_sort_key_item(unsigned int *list, int i, int x, int y);
unsigned int *shop_sort_dir_item(unsigned int *list, int i, int x, int y);

void shop_sort_open(void)
{
    MenuItemDef *d;
    MenuWidget *w;
    int tw, tw2;

    shop_sort = heap_alloc(shop.heap, sizeof(ShopSort));
    if (shop_sort == 0)
        return;
    memset(shop_sort, 0, sizeof(ShopSort));
    shop_sort->menu = heap_alloc(shop.heap, sizeof(MenuWidget));
    w = shop_sort->menu;
    if (w != 0) {
        d = &w->def;
        memset(w, 0, sizeof(MenuWidget));
        d->visible = 1;
        d->style = 0;
        d->y = 0x3A;
        d->pad_t = 2;
        d->pad_b = 2;
        d->pad_l = 2;
        d->pad_r = 2;
        d->auto_size = 1;
        d->color = -1;
        d->open_anim = 2;
        d->close_anim = 2;
        d->show_title = 1;
        d->auto_title = 1;
        d->font_flag4 = 1;
        d->style2 = 2;
        d->title = table_get_entry(shop_str_get(0xC001), 0x18);
        d->title_font_size = 12;
        d->title_color = -1;
        w->heap = shop.heap;
        w->keep_open = 1;
        w->no_cancel = 1;
        w->style = 2;
        w->count = 13;
        w->rows = 9;
        w->item_w = 0x88;
        w->item_h = 0x12;
        w->draw_item = (MenuDrawItemFn)shop_sort_key_item;
        menu_open(w);
        d->x = (480 - d->w) / 2;
        shop_sort->y = menu_cursor_y(&w->menu, d) + 1;
    }
    shop_sort->sub = heap_alloc(shop.heap, sizeof(MenuWidget));
    w = shop_sort->sub;
    if (w != 0) {
        d = &w->def;
        memset(w, 0, sizeof(MenuWidget));
        d->visible = 1;
        d->style = 0;
        d->x = shop_sort->menu->def.x + shop_sort->menu->def.w - 4;
        d->pad_t = 2;
        d->pad_b = 2;
        d->pad_l = 2;
        d->pad_r = 2;
        d->auto_size = 1;
        d->color = -1;
        d->open_anim = 1;
        d->close_anim = 1;
        w->heap = shop.heap;
        w->keep_open = 1;
        w->style = 2;
        w->count = 2;
        font_set_size(12, 12);
        tw = font_measure_text(table_get_entry(shop_str_get(0xC001), 0x16));
        tw2 = font_measure_text(table_get_entry(shop_str_get(0xC001), 0x17));
        tw = tw > tw2 ? tw : tw2;
        w->item_w = (tw >> 6) + 0x19;
        w->item_h = 0x12;
        w->draw_item = (MenuDrawItemFn)shop_sort_dir_item;
        menu_open(w);
    }
    shop_sort->order = heap_alloc(shop.heap, shop.n_items * 2);
    if (shop_sort->order != 0)
        memcpy(shop_sort->order, shop.order, shop.n_items * 2);
    shop_sort->key = shop.sort;
    shop_sort->dir = shop.order_dir;
    shop_sort->menu->cursor = shop_sort->key - 1;
    w = shop_sort->menu;
    if (w->cursor >= w->rows) {
        /* scroll so the cursor line is visible */
        unsigned short c = w->cursor;
        if (c + w->rows < w->count) {
            w->scroll = c;
            shop_sort->menu->cursor = 0;
        } else {
            w->scroll = w->count - w->rows;
            w = shop_sort->menu;
            w->cursor = c - w->scroll;
        }
    }
    shop_sort->sub->cursor = shop_sort->dir;
    shop_sort_layout();
    shop.state = 12;
}

void shop_sort_update(void)
{
    PadState *pad = pad_get();
    int r;
    int key = shop_sort->menu->scroll + shop_sort->menu->cursor;
    int dir = shop_sort->sub->scroll + shop_sort->sub->cursor;

    switch (shop_sort->step) {
    case 0:
        shop_sort_layout();
        switch (shop_sort->focus) {
        case 0:
            r = menu_update(shop_sort->menu);
            if (pad->b.trig & 0x20) {
                shop_sort->menu->no_cancel = 0;
                shop_sort->sub->no_cancel = 1;
                shop_sort->focus = 1;
            }
            if (pad->b.trig & 0x300)
                shop_sort->sub->cursor = (shop_sort->sub->cursor + 1) & 1;
            break;
        case 1:
            r = menu_update(shop_sort->sub);
            if (pad->b.trig & 0x80) {
                shop_sort->sub->no_cancel = 0;
                shop_sort->menu->no_cancel = 1;
                shop_sort->focus = 0;
            }
            break;
        }
        if (key != shop_sort->menu->scroll + shop_sort->menu->cursor
            || dir != shop_sort->sub->scroll + shop_sort->sub->cursor) {
            memcpy(shop.order, shop_sort->order, shop.n_items * 2);
            shop_list_sort(shop_sort->menu->scroll + shop_sort->menu->cursor + 1,
                           shop_sort->sub->scroll + shop_sort->sub->cursor);
        }
        if (r != -1) {
            menu_finish(shop_sort->menu);
            menu_finish(shop_sort->sub);
            shop_sort->step = 1;
            if (r == -2) {
                memcpy(shop.order, shop_sort->order, shop.n_items * 2);
                shop.sort = shop_sort->key;
                shop.order_dir = shop_sort->dir;
            } else {
                memcpy(shop.order, shop_sort->order, shop.n_items * 2);
                shop_list_sort(shop_sort->menu->scroll + shop_sort->menu->cursor + 1,
                               shop_sort->sub->scroll + shop_sort->sub->cursor);
            }
        }
        break;
    case 1:
        menu_update(shop_sort->menu);
        menu_update(shop_sort->sub);
        if (shop_sort->menu->state == 6) {
            shop_sort_close();
            shop.state = 9;
        }
        break;
    }
}

void shop_sort_close(void)
{
    menu_reset(shop_sort->sub);
    menu_reset(shop_sort->menu);
    heap_set_level(shop.heap, shop_sort->sub);
    heap_set_level(shop.heap, shop_sort->menu);
    heap_set_level(shop.heap, shop_sort);
    shop_sort = 0;
}

unsigned int *shop_sort_draw(unsigned int *list)
{
    menu_draw((void **)&list, shop_sort->menu);
    menu_draw((void **)&list, shop_sort->sub);
    return list;
}

unsigned int *shop_sort_key_item(unsigned int *list, int i, int x, int y)
{
    return shop_draw_box(list, &shop_sort_keys[i + 1], x, y + 1);
}

unsigned int *shop_sort_dir_item(unsigned int *list, int i, int x, int y)
{
    return shop_draw_box(list, &shop_sort_dirs[i], x, y + 1);
}

#ifdef NON_MATCHING /* mesma conta; o original troca os registradores de y e da altura da linha */
/* the direction window follows the key cursor */
void shop_sort_layout(void)
{
    MenuWidget *m = shop_sort->menu;
    MenuWidget *s = shop_sort->sub;

    s->def.y = shop_sort->y + (m->item_h / 2 + m->cursor * m->item_h) - s->def.h / 2;
}
#endif

unsigned int *shop_draw_sort_tab(unsigned int *list, int i, int x, int y)
{
    int o = 17;
    ShopBox *b;
    void *gim;

    if (i == 0)
        return list;
    b = &shop_sort_keys[i];
    if (i == 2)
        o = 16;
    gim = shop_res_get(b->gim);
    if (gim != 0) {
        list = dl_ambient_color(list, -1);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(gim_set_texture(list, gim), o, o, o, o);
        list = draw_sprite(list, x, y, 0, b->x, b->y, 0, 0);
    }
    return list;
}

unsigned int *shop_draw_order_tab(unsigned int *list, int i, int x, int y)
{
    ShopBox *b = &shop_sort_dirs[i];
    void *gim = shop_res_get(b->gim);

    if (gim != 0) {
        list = dl_ambient_color(list, -1);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(gim_set_texture(list, gim), 17, 17, 17, 17);
        list = draw_sprite(list, x, y, 0, b->x, b->y, 0, 0);
    }
    return list;
}
