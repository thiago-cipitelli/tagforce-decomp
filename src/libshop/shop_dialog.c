// @range 0x08814BE8-0x08814F58
/* The yes/no dialogs of the shop: leave the shop, buy the packs, open the bought list. */
#include "libshop.h"

extern MenuWidget shop_dialog;

int menu_update(MenuWidget *w);
void menu_open(MenuWidget *w);
void menu_reset(MenuWidget *w);
void menu_draw(void **dl, MenuWidget *w);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
void bgm_cmd(int cmd);
void shop_clerk_voice(int what);
void shop_stock_save(void);
void shop_items_commit(void);

/* 1 leave, 2 buy, 3 bought list */
void shop_dialog_open(unsigned short what)
{
    MenuWidget *w = &shop_dialog;
    MenuItemDef *d = &w->def;
    unsigned char buttons = 1;

    memset(w, 0, sizeof(MenuWidget));
    switch (what) {
    case 1:
        d->text = table_get_entry(shop_str_get(0xC001), 0x19);
        w->initial = 3;
        buttons = 3;
        w->cancel_resets = 1;
        break;
    case 2:
        d->text = table_get_entry(shop_str_get(0xC001), 0x1A);
        w->initial = 1;
        d->auto_size = 1;
        w->cancel_resets = 1;
        d->min_w = 0xA0;
        d->min_h = 0x36;
        break;
    case 3:
        d->text = table_get_entry(shop_str_get(0xC001), 0x1C);
        w->initial = 3;
        buttons = 3;
        w->cancel_resets = 1;
        break;
    }
    d->visible = 1;
    d->auto_size = 1;
    d->font_flag4 = 1;
    d->max_w = 0xF0;
    d->color = -1;
    d->pad_l = 10;
    d->pad_r = 10;
    d->pad_t = 8;
    d->font_size = 12;
    d->text_color = 0xFF000000;
    d->open_anim = 2;
    d->close_anim = 2;
    w->heap = shop.heap;
    w->no_cancel = 1;
    w->buttons = buttons;
    d->style = 0;
    d->pad_b = 0;
    menu_open(w);
    d->x = (480 - d->w) / 2;
    d->y = (272 - d->h) / 2;
    shop.hud = what;
}

void shop_dialog_update(void)
{
    int r = menu_update(&shop_dialog);

    if (r == -1)
        return;
    switch (shop.hud) {
    case 1:
        if (r == 2) {
            shop_clerk_voice(6);
            shop_stock_save();
            fade_start_in(1, 0xFF000000, 16);
            bgm_cmd(600);
            shop.state = 6;
        }
        break;
    case 3:
        if (r == 2) {
            shop_items_commit();
            fade_start_in(1, 0xFF000000, 16);
            shop.state = 14;
        }
        break;
    }
    menu_reset(&shop_dialog);
    shop.hud = 0;
}

unsigned int *shop_dialog_draw(unsigned int *list)
{
    menu_draw((void **)&list, &shop_dialog);
    return list;
}
