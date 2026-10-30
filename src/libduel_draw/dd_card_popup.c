// @range 0x08866EC8-0x088676D8
// @asm 0x0886722C-0x08867550
// @data 0x0887C994-0x0887C9D4
/* Action menu of a card on the field: lists the actions that apply (card detail, two engine
   commands, the field browse), places the window next to the card projected to the screen, and
   runs the chosen action. */
#include "dd_duelui.h"

typedef struct PopupItem {             /* icon and caption of an action */
    int f0;
    int u, v;                          /* icon in texture 0x40005 */
    int msg;                           /* 0x0881DBC8 caption */
} PopupItem;

PopupItem popup_items[4] = {
    { 0, 0, 66, 4 },
    { 1, 91, 92, 50 },
    { 2, 78, 92, 52 },
    { 3, 52, 79, 51 },
};

void func_08825824(void *panel, int a, int b, void (*setup)(MenuWidget *w, int init));
void func_0881CB90(int mode);
void func_0881D044(int mode);
void func_088259FC(void (*cb)(void));
void func_088259E4(void *(*cb)(void *dl));
void func_08825B7C(int v);
int func_08825A14(void);
int dv_08835924(void);
int func_08806D04(void);
int hl_allowed(void);
void *func_0881DBC8(int id);
void func_0881D858(int card);
void dv_08836028(void);
void opt_open(void);
void hl_open(void);
void *func_0880B7B8(int side, int zone, int index);
void func_0880A8E0(DdVec4A *pos, DdVec4A *rot, short loc);
void *func_088044B0(int id);

void popup_build(void);
void popup_menu_setup(MenuWidget *w, int init);
void popup_select(void);
void *popup_track(void *dl);
void *popup_draw_item(void *dl, int index, int x, int y);

void popup_open(DdPopupArg *arg)
{
    popup.arg = *arg;
    popup_build();
    if (popup.count != 0) {
        func_08825824(0, 0, 32, popup_menu_setup);
        func_0881CB90(4);
        func_0881D044(49);
        func_088259FC(popup_select);
        func_088259E4(popup_track);
        func_08825B7C(1);
        se_play(0x6216);
    }
}

void popup_build(void)
{
    DdPopup *p = &popup;
    int n = 0;
    int i;
    int w;

    if (p->arg.card > 0) {
        p->items[n] = 0;
        n++;
    }
    if ((unsigned int)(dv_08835924() - 2) < 2) {
        p->items[n] = 1;
        n++;
    }
    if (func_08806D04() == 0) {
        p->items[n] = 2;
        n++;
    }
    if (hl_allowed() != 0) {
        p->items[n] = 3;
        n++;
    }
    p->count = n;
    p->text_w = 0;
    font_set_size(12, 12);
    for (i = 0; i < n; i++) {
        w = font_measure_text(func_0881DBC8(popup_items[p->items[i]].msg)) >> 6;
        if (p->text_w < w)
            p->text_w = w;
    }
}

void popup_menu_setup(MenuWidget *w, int init)
{
    DdPopup *p = &popup;
    MenuItemDef *d = &w->def;

    if (init) {
        p->menu = w;
        popup_build();
        w->cancel_blocked = 0;
        w->count = p->count;
        w->rows = p->count;
        w->item_w = 100;
        w->item_h = 15;
        w->draw_item = popup_draw_item;
        d->tab = 1;
        d->pad_t = 2;
        d->pad_b = 2;
        d->pad_l = 2;
        d->pad_r = 2;
    } else {
        w->item_w = p->text_w + 18;
        w->cursor_w = w->item_w;
        d->w = w->item_w + 8 + d->pad_l + d->pad_r;
        if (d->y < 0)
            d->y = 0;
    }
}

void popup_select(void)
{
    DdPopup *p = &popup;
    int i = func_08825A14();

    if (i >= 0) {
        switch (p->items[i]) {
        case 0:
            func_0881D858(p->arg.card);
            func_0881CB90(5);
            break;
        case 1:
            dv_08836028();
            break;
        case 2:
            opt_open();
            break;
        case 3:
            hl_open();
            break;
        }
    }
}

/* places the window above the card (screen projection of its model position) */
#ifdef NON_MATCHING /* o original grava os bitfields de loc com lhu/sh; o mwcc estreita para lbu/sb */
void popup_place(MenuWidget *w, int side, int zone, int index)
{
    DdVec4A pos;
    DdVec4A scr;
    DdVec4A rot;
    DdMat4 vp;
    DdMat4 proj;
    DdMat4 view;
    DdMat4 model;
    char *e;
    MenuItemDef *d;

    if (w == 0)
        return;
    e = func_0880B7B8(side, zone, index);
    if (e == 0 || zone != 0xB) {
        PopupLoc loc = { 0 };

        loc.side = side;
        loc.zone = zone;
        loc.index = index;
        func_0880A8E0(&pos, &rot, *(short *)&loc);
    } else {
        pos = *(DdVec4A *)(e + 0x10);
        rot = *(DdVec4A *)(e + 0x40);
    }
    mtx_set_mode(2);
    mtx_get34(&proj);
    mtx_set_mode(1);
    mtx_get34(&view);
    mtx_set_mode(0);
    mtx_get34(&model);
    mat4_mul(&vp, &view, &proj);
    mat4_mul(&vp, &model, &vp);
    mtx_set_mode(2);
    pos.w = 1.0f;
    mat4_transform_vec4(&scr, &vp, &pos);
    scr.x = scr.x * 480.0f / scr.w;
    scr.y = -272.0f * scr.y / scr.w;
    scr.x += 2122.0f;
    scr.y += 2044.0f;
    scr.x -= 1808.0f;
    scr.y -= 1912.0f;
    scr.y -= 20.0f;
    d = &w->def;
    d->x = scr.x - (float)(d->w / 2);
    d->y = scr.y - (float)d->h;
    if (d->x + d->w > 480)
        d->x = 480 - d->w;
    if (d->x < 148)
        d->x = 148;
    if (d->y < 0)
        d->y = 0;
    d->tab_x = scr.x - (float)d->x;
    if (d->tab_x < 4)
        d->tab_x = 4;
    if (d->tab_x > d->w - 4)
        d->tab_x = d->w - 4;
}
#else
void popup_place(MenuWidget *w, int side, int zone, int index);
#endif

void *popup_track(void *dl)
{
    PopupLoc loc;

    popup.arg.get_loc(&loc);
    popup_place(popup.menu, loc.side, loc.zone, loc.index);
    return dl;
}

void *popup_draw_item(void *dl, int index, int x, int y)
{
    int item = popup.items[(unsigned short)index];
    PopupItem *it = &popup_items[item];
    void *msg;

    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 1);
    dl = gim_set_texture(dl, func_088044B0(0x40005));
    dl = draw_set_offsets(dl, 13, 13, 13, 13);
    dl = draw_sprite(dl, x, y + 1, 0, it->u, it->v, 0, 0);
    font_set_size(12, 12);
    msg = func_0881DBC8(it->msg);
    font_text_set(dl);
    font_set_color(0xFF000000);
    font_draw_text((x + 15) << 6, (y + 1) << 6, (480 - (x + 15)) << 6, msg);
    return font_text_clear();
}
