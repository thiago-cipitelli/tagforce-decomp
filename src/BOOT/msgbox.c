// @range 0x0886C904-0x0886CF50
// @asm 0x0886C904-0x0886C9FC
// @asm 0x0886CC40-0x0886CD04
/* In-game message box: a centred window with UCS-2 text, either plain (closes on demand,
   type 0) or with buttons through the menu widget (type 1 OK, type 2 YES/NO). One box at a
   time; msgbox_update returns 1 while it is open, then 2 closed, 3 yes/ok, 4 no, 5 other. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "game.h"
#include "menu.h"

typedef struct MsgBox {                /* 0x1E8 */
    int kind;                          /* +0x000 0 plain menu item, 1 widget with buttons */
    Menu menu;                         /* +0x004 */
    MenuItemDef def;                   /* +0x050 */
    MenuWidget widget;                 /* +0x0C8 */
} MsgBox;

extern int msgbox_active;              /* 0x08E34A00 */
extern MsgBox msgbox;                  /* 0x08E34A04 */

void *dl_select_layer(int layer, int flags);
void dl_set_list(void *dl);
int region_get(void);
void font_set_flag1(int v);
void font_set_flag8(int v);
void se_play(int id);

int msgbox_kind(int type);
void msgbox_open_plain(void *heap, unsigned short *text);
void msgbox_open_buttons(void *heap, unsigned short *text, int buttons, int initial);
int msgbox_update_plain(void **dl);
int msgbox_update_buttons(void **dl);

#ifdef NON_MATCHING /* the type tests take their constants in other registers and delay slots */
int msgbox_open(unsigned short *text, void *heap, int type, int flag)
{
    MsgBox *m = &msgbox;

    if (msgbox_active != 0) {
        return 0;
    }
    memset(m, 0, 0x1E8);
    m->kind = msgbox_kind(type);
    switch (type) {
    default:
        msgbox_open_plain(heap, text);
        break;
    case 1:
        msgbox_open_buttons(heap, text, 1, 0);
        break;
    case 2:
        msgbox_open_buttons(heap, text, 3, flag != 1 ? 2 : 3);
        break;
    }
    msgbox_active = 1;
    se_play(0x621A);
    return 1;
}
#endif

/* `dl` 0: draw into a list of layer 4 of our own. */
int msgbox_update(void **dl)
{
    MsgBox *m = &msgbox;
    int r;
    int mine = 0;
    void *own = 0;

    if (msgbox_active == 0) {
        return 0;
    }
    if (dl == 0) {
        own = dl_select_layer(4, 0);
        dl = &own;
        mine = 1;
    }
    if (m->kind != 1) {
        r = msgbox_update_plain(dl);
    } else {
        r = msgbox_update_buttons(dl);
    }
    if (mine) {
        dl_set_list(own);
    }
    return r;
}

/* Starts closing a plain box (button boxes close themselves). */
int msgbox_close(void)
{
    MsgBox *m = &msgbox;

    int r = 0;

    if (m->kind != 1) {
        menu_item_close(&m->menu, &m->def);
        r = 1;
    }
    return r;
}

int msgbox_kind(int type)
{
    switch (type) { default: return 0; case 1: return 1; case 2: return 1; }
}

void msgbox_open_plain(void *heap, unsigned short *text)
{
    MenuItemDef *d = &msgbox.def;

    menu_init(&msgbox.menu, heap);
    memset(d, 0, 0x78);
    d->visible = 1;
    d->font_flag4 = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->auto_size = 1;
    d->style = 2;
    d->color = 0xFFFFFFFF;
    d->text_color = 0xFFFFFFFF;
    d->pad_t = 8;
    d->pad_b = 8;
    d->pad_l = 10;
    d->pad_r = 10;
    d->font_size = 13;
    d->max_w = 400;
    d->min_w = 100;
    d->x = 0;
    d->y = 0;
    d->text = text;
    menu_add_item(&msgbox.menu, d);
    menu_item_open(&msgbox.menu, d);
    d->x = 240.0f - d->w * 0.5f;
    d->y = 136.0f - d->h * 0.5f;
}

#ifdef NON_MATCHING /* the original keeps a dead `move v1,s1` before font_set_flag8(1) */
int msgbox_update_plain(void **dl)
{
    MsgBox *m = &msgbox;
    MenuItemDef *d = &msgbox.def;
    int r = 1;

    if (region_get() != 0) {
        font_set_flag8(1);
    } else {
        font_set_flag1(1);
    }
    menu_layout(dl, &m->menu);
    if (menu_item_state(&m->menu, d) != 2) {
        if (menu_item_is_open(&m->menu, d) != 0) {
            r = 2;
            menu_free_items(&m->menu);
            msgbox_active = 0;
        }
    }
    return r;
}
#endif

void msgbox_open_buttons(void *heap, unsigned short *text, int buttons, int initial)
{
    MenuWidget *w = &msgbox.widget;
    MenuItemDef *d = &msgbox.widget.def;

    memset(w, 0, 0x120);
    d->visible = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->font_flag4 = 1;
    d->auto_size = 1;
    d->style = 2;
    d->pad_t = 8;
    d->pad_l = 10;
    d->pad_r = 10;
    d->color = 0xFFFFFFFF;
    d->text_color = 0xFFFFFFFF;
    d->font_size = 13;
    d->max_w = 400;
    d->min_w = 100;
    w->no_cancel = 1;
    w->cancel_blocked = 1;
    w->buttons = buttons;
    w->button = initial;
    d->x = 0;
    d->y = 0;
    d->pad_b = 0;
    d->text = text;
    w->heap = heap;
    menu_open(w);
    d->x = 240.0f - d->w * 0.5f;
    d->y = 136.0f - d->h * 0.5f;
}

int msgbox_update_buttons(void **dl)
{
    MenuWidget *w = &msgbox.widget;
    MsgBox *m = &msgbox;
    int r = 1;
    int res = menu_update(w);

    if (res != -1) {
        menu_reset(w);
        msgbox_active = 0;
        switch (res) {
        case 1:
        default:
            r = 5;
            break;
        case 2:
            r = 3;
            break;
        case 3:
            r = 4;
            break;
        }
    }
    menu_draw(dl, &m->widget);
    return r;
}
