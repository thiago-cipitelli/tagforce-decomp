// @range 0x08813808-0x08813FF4
// @data 0x08820480-0x08820494
/* The card menu of the bought list (Triangle on a card): a window with the card picture and three
   lines (card info, mark/unmark, help); the mark toggles the card's flag in the collection. */
#include "libshop.h"

typedef struct ShopDetail {     /* shop_detail 0x08F8EE04, heap allocated */
    unsigned short step;        /* +0x00 0 menu, 1 closing */
    unsigned short pad02;
    ShopItem *item;             /* +0x04 */
    MenuWidget *menu;           /* +0x08 */
} ShopDetail;

typedef struct ShopLineVtx {    /* colored vertex of the separator line, 12 bytes */
    unsigned int color;
    short x, y, z;
    short pad;
} ShopLineVtx;

extern ShopDetail *shop_detail;

int menu_update(MenuWidget *w);
void menu_finish(MenuWidget *w);
void menu_open(MenuWidget *w);
void menu_reset(MenuWidget *w);
void menu_draw(void **dl, MenuWidget *w);
void *menu_draw_cursor(void *dl, MenuWidget *w, int x, int y, int cw, int ch);
int inv_get_flag7(unsigned short id);
void inv_set_flag7(unsigned short id, int on);
void *dl_alloc_inline(unsigned int **list, int bytes);
unsigned int *dl_draw_array(unsigned int *list, int prim, unsigned int vtype, int count, void *indices, void *vertices);
void *shop_ui_draw_card_pict(void *list, int x, int y, int shadow, unsigned int color, UiEnt *e, void *gim, void *gim_alt, UiIdList *alt);
void shop_sort_open(void);

void shop_detail_close(void);
void shop_detail_menu(void);
int shop_detail_wait(void);
unsigned int *shop_detail_line(unsigned int *list, ShopLineVtx *v, int x, int y, int w);
unsigned int *shop_detail_draw_all(unsigned int *list, int x, int y);
unsigned int *shop_detail_label(unsigned int *list, unsigned short line, int x, int y);

void shop_card_detail(ShopItem *it)
{
    MenuWidget *w;
    MenuItemDef *d;

    shop_detail = heap_alloc(shop.heap, sizeof(ShopDetail));
    if (shop_detail == 0)
        return;
    memset(shop_detail, 0, sizeof(ShopDetail));
    shop_detail->item = it;
    shop_detail->menu = heap_alloc(shop.heap, sizeof(MenuWidget));
    w = shop_detail->menu;
    if (w == 0)
        return;
    d = &w->def;
    memset(w, 0, sizeof(MenuWidget));
    d->visible = 1;
    d->style = 0;
    d->y = 0x3A;
    d->color = -1;
    d->auto_size = 1;
    d->font_flag4 = 1;
    d->pad_l = 2;
    d->pad_t = 2;
    d->pad_r = 2;
    d->pad_b = 2;
    d->show_title = 1;
    d->auto_title = 1;
    d->title = table_get_entry(shop_str_get(0xC001), 6);
    d->style2 = 2;
    d->title_font_size = 12;
    d->title_color = -1;
    d->open_anim = 2;
    d->close_anim = 2;
    w->heap = shop.heap;
    w->no_cancel = 1;
    w->keep_open = 1;
    w->style = 2;
    w->count = 3;
    w->item_w = 0xD4;
    w->item_h = 0x13;
    w->draw_all = (MenuDrawRectFn)shop_detail_draw_all;
    menu_open(w);
    if (d->w < 0xD4)
        d->w = 0xD4;
    d->h += 4;
    d->x = (480 - d->w) / 2;
    shop.state = 13;
}

int shop_menu_input(void)
{
    switch (shop_detail->step) {
    case 0:
        shop_detail_menu();
        break;
    case 1:
        return shop_detail_wait();
    }
    return 0;
}

unsigned int *shop_detail_draw(unsigned int *list)
{
    menu_draw((void **)&list, shop_detail->menu);
    return list;
}

void shop_detail_close(void)
{
    menu_reset(shop_detail->menu);
    if (shop_detail->menu) {
        heap_set_level(shop.heap, shop_detail->menu);
        shop_detail->menu = 0;
    }
    heap_set_level(shop.heap, shop_detail);
    shop_detail = 0;
}

/* 0 card info, 1 mark/unmark, cancel or 2 help */
void shop_detail_menu(void)
{
    int r = menu_update(shop_detail->menu);
    unsigned short id;

    if (r == -1)
        return;
    switch (r) {
    case -2:
    case 2:
        menu_finish(shop_detail->menu);
        shop_detail->step = 1;
        return;
    }
    shop_detail->menu->no_cancel = 0;
    switch (r) {
    case 0:
        shop.info = 1;
        shop.step = 0;
        shop_detail->menu->cancelled = shop_detail->menu->accepted = 0;
        break;
    case 1:
        id = shop_detail->item->id;
        if (inv_get_flag7(id))
            inv_set_flag7(id, 0);
        else
            inv_set_flag7(id, 1);
        shop_detail->menu->cancelled = shop_detail->menu->accepted = 0;
        break;
    }
}

/* waits for the window to close; help or back to the list */
int shop_detail_wait(void)
{
    int r = menu_update(shop_detail->menu);

    if (shop_detail->menu->state == 5)
        return 0;
    shop_detail_close();
    switch (r) {
    case 2:
        shop_sort_open();
        se_play(0x6216);
        break;
    default:
        shop.state = 9;
        break;
    }
    return 0;
}

/* two-pixel separator line between the info line and the others */
unsigned int *shop_detail_line(unsigned int *list, ShopLineVtx *v, int x, int y, int w)
{
    int i;
    ShopLineVtx *p;

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

/* window content: card picture, cursor, the three lines */
unsigned int *shop_detail_draw_all(unsigned int *list, int x, int y)
{
    MenuWidget *w = shop_detail->menu;
    UiEnt e;
    ShopItem *it;
    int i;
    int dx;

    memset(&e, 0, sizeof(e));
    it = shop_detail->item;
    e.id = it->id;
    e.key = it->key;
    e.count = it->count;
    list = shop_ui_draw_card_pict(list, x + 2, y + 2, 1, 0xFFFFFFFF, &e, shop_res_get(0x2020), shop_res_get(0x2021),
                                  shop_str_get(0xC004));
    for (i = 0; i < 3; i++) {
        dx = 0;
        switch (i) {
        case 0:
        case 1:
            dx += 0x1D;
            break;
        }
        if (w->cursor == i)
            list = menu_draw_cursor(list, w, w->item_x + (x + dx), y + w->item_y, w->cursor_w - dx, w->cursor_h);
        list = shop_detail_label(list, i, x + dx, y);
        y += w->item_h;
        switch (i) {
        case 1: {
            ShopLineVtx *v = dl_alloc_inline(&list, 0x30);
            list = shop_detail_line(list, v, x, y + 1, w->item_w);
            y += 4;
            break;
        }
        }
    }
    return list;
}

/* the lines as shelf-style boxes (info, mark, help) */
ShopBox shop_detail_lines[3] = {
    { 0x2027, 0x33, 0x11, 4 },
    { 0x2027, 0x44, 0x11, 5 },
    { 0x2027, 0x11, 0x00, 7 },
};

unsigned int *shop_detail_label(unsigned int *list, unsigned short line, int x, int y)
{
    return shop_draw_box(list, &shop_detail_lines[line], x, y + 1);
}
