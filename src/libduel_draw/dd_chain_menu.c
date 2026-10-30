// @range 0x0882308C-0x08823674
// @asm 0x08823344-0x088235C4
/* Text panel shown beside the field (used by the field browser and the event commands): a single menu
   window that slides to the side opposite to the cursor (cursor listener), closes when its condition
   says so, and exports for the duel manager. */
#include "dd_field_ui.h"

typedef struct DdChainWin {            /* 0x08C58538, 0x2D8 */
    Menu menu;                         /* +0x000 */
    MenuItemDef def;                   /* +0x04C */
    unsigned short text[256];          /* +0x0C4 */
    int state;                         /* +0x2C4 0 idle, 1-4 open (3/4: upper, 1/2: lower half), 5 closing, 6 opening */
    int x;                             /* +0x2C8 */
    int y;                             /* +0x2CC */
    int (*cond)(void);                 /* +0x2D0 nonzero: the panel may stay open */
    void *(*draw)(void *dl, Menu *m, MenuItemDef *d); /* +0x2D4 extra drawing over the window */
} DdChainWin;

extern DdChainWin dd_chain_win;

int dv_08832D08();
void *dd_heap_get(void);
void dd_cursor_listen(DdCursorCb cb);
void dd_cursor_unlisten(DdCursorCb cb);
MenuItemDef *func_08823310(void);
void func_0882331C(void *(*draw)(void *dl, Menu *m, MenuItemDef *d));
void func_08823344(void);
void func_088235C4(DdLoc loc, int card);
int func_08823650(void);

/* opens the panel with a copy of text */
void func_0882308C(const unsigned short *text)
{
    DdChainWin *w = &dd_chain_win;
    MenuItemDef *d = &dd_chain_win.def;
    DdLoc cur;

    if (w->state == 0) {
        menu_init(&w->menu, dd_heap_get());
        w->state = 6;
        w->cond = func_08823650;
        w->x = 0;
        w->y = 0;
        w->draw = 0;
        memcpy(w->text, text, sizeof(w->text));
        memset(d, 0, sizeof(MenuItemDef));
        d->visible = 1;
        d->auto_size = 1;
        d->font_flag4 = 1;
        d->open_anim = 1;
        d->close_anim = 1;
        d->color = 0xFFFFFFFF;
        d->max_w = 265;
        d->text = w->text;
        d->pad_l = 10;
        d->pad_r = 10;
        d->pad_t = 8;
        d->pad_b = 8;
        d->font_size = 12;
        d->text_color = 0xFF202020;
        d->scale_x = 1.0f;
        d->scale_y = 1.0f;
        d->anchor = 6;
        d->gim = 0;
        d->style = 0;
        d->style2 = 0;
        d->tab = 0;
        d->x = 0;
        d->y = 0;
        d->z = 0;
        d->w = 0;
        d->h = 0;
        d->tab_x = 0;
        d->show_title = 0;
        d->auto_title = 0;
        d->title_w = 0;
        d->title_h = 0;
        d->title = 0;
        d->title_font_size = 0;
        d->title_color = 0;
        menu_add_item(&w->menu, d);
        menu_item_open(&w->menu, d);
        dd_cursor_get(&cur);
        if (!cur.player)
            w->state = 3;
        else
            w->state = 2;
        dd_cursor_listen(func_088235C4);
        se_play(0x621A);
    }
}

/* per frame: update and draw on layer 8 */
void func_08823234(void)
{
    DdChainWin *w = &dd_chain_win;
    void *dl;

    switch (w->state) {
    case 0:
        return;
    case 6:
        if (menu_item_state(&w->menu, &w->def) == 4)
            return;
        break;
    }
    func_08823344();
    dl = dl_select_layer(8, 0);
    menu_layout(&dl, &w->menu);
    if (w->draw)
        dl = w->draw(dl, &w->menu, &w->def);
    dl_set_list(dl);
}

/* closed (or still opening) */
int func_088232E0(void)
{
    int r;

    switch (dd_chain_win.state) {
    case 0:
    case 6:
        r = 1;
        break;
    default:
        r = 0;
        break;
    }
    return r;
}

/* the window, for the caller to edit */
MenuItemDef *func_08823310(void)
{
    return &dd_chain_win.def;
}

/* sets the extra drawing */
void func_0882331C(void *(*draw)(void *dl, Menu *m, MenuItemDef *d))
{
    dd_chain_win.draw = draw;
}

/* sets the stay-open condition (0: the default one) */
void func_08823328(int (*f)(void))
{
    DdChainWin *w = &dd_chain_win;

    if (f == 0)
        f = func_08823650;
    w->cond = f;
}

#ifdef NON_MATCHING /* estados em switch; o 132 que o original mantém em $s0 (sem dobrar) não reproduzido */
/* state machine: placement by half of the screen, open/close from the condition */
void func_08823344(void)
{
    DdChainWin *w = &dd_chain_win;
    int top = 132;

    if (menu_item_state(&w->menu, &w->def) == 4) {
        switch (w->state) {
        case 1:
            w->state = 3;
            break;
        case 4:
            w->state = 2;
            break;
        case 5:
            w->state = 6;
            break;
        }
    }
    switch (w->state) {
    case 1:
    case 2:
        w->y = 202 - w->def.h / 2;
        break;
    case 3:
    case 4:
        w->y = top / 2 - w->def.h / 2;
        break;
    }
    w->def.y = w->y;
    w->x = 314.0f - (float)(w->def.w / 2);
    w->def.x = w->x;
    if (menu_item_state(&w->menu, &w->def) == 4) {
        switch (w->state) {
        case 2:
        case 3:
            menu_item_open(&w->menu, &w->def);
            break;
        }
    }
    if (w->cond && w->cond()) {
        switch (w->state) {
        case 0:
        case 5:
            break;
        case 6:
            if (menu_item_state(&w->menu, &w->def) == 4) {
                menu_free_items(&w->menu);
                dd_cursor_unlisten(func_088235C4);
                w->state = 0;
            }
            break;
        default:
            if (menu_item_is_open(&w->menu, &w->def)) {
                menu_item_close(&w->menu, &w->def);
                w->state = 5;
            }
            break;
        }
    } else {
        switch (w->state) {
        case 1:
        case 4:
            if (menu_item_is_open(&w->menu, &w->def))
                menu_item_close(&w->menu, &w->def);
            break;
        }
    }
}
#endif

/* cursor listener: the panel moves to the half of the screen away from the cursor's player */
void func_088235C4(DdLoc loc, int card)
{
    DdChainWin *w = &dd_chain_win;

    if (!loc.player) {
        switch (w->state) {
        case 2:
            w->state = 1;
            break;
        case 4:
            w->state = 3;
            break;
        }
    } else {
        switch (w->state) {
        case 1:
            w->state = 2;
            break;
        case 3:
            w->state = 4;
            break;
        }
    }
}

int func_08823650(void)
{
    return dv_08832D08() == 0;
}
