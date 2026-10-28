// @range 0x0881CE68-0x0881EDC8
// @asm 0x0881D180-0x0881D2B8
// @asm 0x0881E83C-0x0881E9D4
// @rodata 0x0883904C-0x088392E0
// @data 0x0883D3C0-0x0883D410
/* The bag window of the field (field_bag): two tabs (items with their count, cards one row per
   copy) switched with L/R, the help text of the entry under the cursor in the message window, and
   a small menu to pick an entry or throw a card away. */
#include "libfield.h"

PadState *pad_get(void);
unsigned short region_pick_value(int flag);
int font_measure_text(unsigned short *text);
int font_draw_wtext(int x, int y, int w, unsigned short *text);
void font_set_flag8000(int on);
void *gim_set_texture(void *list, void *gim);
void *dl_ambient_color(void *list, unsigned int color);
void *fld_draw_cell1(void *list, int x, int y, int u, int v, int w, int h);
void *fld_draw_frame(void *list, int x, int y, int u, int v, int w, int h, int w2, int h2, int l, int t,
                     int r, int b);
void fld_dlg_open(unsigned short *text, int a, int b);
int fld_dlg_update(void);
void *fld_dlg_draw(void *list);
FieldStory *fld_story_get(void);

void fbag_free(void);
void fbag_open(void);
void fbag_list0_open(void);
void fbag_list1_open(void);
void fbag_list0_resume(void);
void fbag_list0_update(void);
void fbag_list1_resume(void);
void fbag_list1_update(void);
void fbag_menu0_open(void);
void fbag_menu0_update(void);
void fbag_menu1_open(void);
int fbag_menu1_width(void);
void fbag_menu1_update(void);
void fbag_discard_ask(void);
void fbag_discard_update(void);
void fbag_close(void);
void *fbag_tab_draw(void *list, FieldBagTab *t);
void *fbag_tab_draw_off(void *list, FieldBagTab *t);
void *fbag_item0_draw(void *list, unsigned short i, int x, int y);
void *fbag_item1_draw(void *list, unsigned short i, int x, int y);
void *fbag_menu0_item(void *list, unsigned short i, int x, int y);
void *fbag_menu1_item(void *list, unsigned short i, int x, int y);
int fbag_count(int tab);
void fbag_build(void);
void fbag_help(void);

/* icon of each item / card in gim_icons */
const FieldOff fbag_icons0[21] = {
    { 0x0, 0x0 }, { 0x0, 0x78 }, { 0x18, 0x78 }, { 0x30, 0x78 }, { 0x48, 0x78 }, { 0x60, 0x78 },
    { 0x78, 0x78 }, { 0x90, 0x78 }, { 0xA8, 0x78 }, { 0xC0, 0x78 }, { 0xD8, 0x78 }, { 0x0, 0x90 },
    { 0x30, 0x78 }, { 0x0, 0x78 }, { 0x18, 0x78 }, { 0x30, 0x78 }, { 0x0, 0x78 }, { 0x18, 0x78 },
    { 0x30, 0x78 }, { 0x0, 0x78 }, { 0x18, 0x78 },
};
const FieldOff fbag_icons1[51] = {
    { 0x0, 0x0 }, { 0x0, 0x0 }, { 0x18, 0x0 }, { 0x30, 0x0 }, { 0x48, 0x0 }, { 0x60, 0x0 },
    { 0x78, 0x0 }, { 0x90, 0x0 }, { 0xA8, 0x0 }, { 0xC0, 0x0 }, { 0xD8, 0x0 }, { 0x0, 0x18 },
    { 0x18, 0x18 }, { 0x30, 0x18 }, { 0x48, 0x18 }, { 0x60, 0x18 }, { 0x78, 0x18 }, { 0x90, 0x18 },
    { 0xA8, 0x18 }, { 0xC0, 0x18 }, { 0xD8, 0x18 }, { 0x0, 0x30 }, { 0x18, 0x30 }, { 0x30, 0x30 },
    { 0x48, 0x30 }, { 0x60, 0x30 }, { 0x78, 0x30 }, { 0x90, 0x30 }, { 0xA8, 0x30 }, { 0xC0, 0x30 },
    { 0xD8, 0x30 }, { 0x0, 0x48 }, { 0x18, 0x48 }, { 0x30, 0x48 }, { 0x48, 0x48 }, { 0x60, 0x48 },
    { 0x78, 0x48 }, { 0x90, 0x48 }, { 0xA8, 0x48 }, { 0xC0, 0x48 }, { 0xD8, 0x48 }, { 0x0, 0x60 },
    { 0x18, 0x60 }, { 0x30, 0x60 }, { 0x48, 0x60 }, { 0x60, 0x60 }, { 0x78, 0x60 }, { 0x90, 0x60 },
    { 0xA8, 0x60 }, { 0xC0, 0x60 }, { 0xD8, 0x60 },
};
const FieldSpr fbag_slot = { 0xE8, 0, 0x18, 0x18 };

FieldBagTab fbag_tabs_jp[2] = {
    { 0, 0xEF, 0x28, { 0x99, 0x38, 0x5F, 0x12 } },
    { 0, 0x91, 0x28, { 0x99, 0x26, 0x5F, 0x12 } },
};
FieldBagTab fbag_tabs_en[2] = {
    { 0, 0xEF, 0x28, { 0x99, 0x38, 0x5F, 0x12 } },
    { 0, 0x69, 0x28, { 0x99, 0x26, 0x5F, 0x12 } },
};

void fbag_init(void)
{
    memset(&field_bag, 0, sizeof(field_bag));
    menu_init(&field_bag.menu, fld_heap_get());
    fld_story_flag(0x6216, 0);
}

void fbag_free(void)
{
    FieldBag *b = &field_bag;
    int i;

    menu_free_items(&field_bag.menu);
    for (i = 0; i < 4; i++)
        menu_reset(&b->w[i]);
}

/* returns the result once the window is closed (0 while open), with the id picked in *picked */
int fbag_update(int *picked)
{
    FieldBag *b = &field_bag;
    MenuItemDef *msg = fmsg_def();
    PadState *pad = pad_get();

    switch (field_bag.state) {
    case 4:
        if ((pad->b.trig & 0x20) || (pad->b.trig & 0x200) || (pad->b.trig & 0x8000)) {
            fld_story_flag(0x621B, 0);
            b->state = 2;
            b->tab = 0;
            fbag_help();
        }
        break;
    case 2:
        if ((pad->b.trig & 0x80) || (pad->b.trig & 0x100) || (pad->b.trig & 0x8000)) {
            fld_story_flag(0x621B, 0);
            b->tab = 1;
            b->state = 4;
            fbag_help();
        }
        break;
    }
    switch (b->state) {
    case 0:
        fbag_open();
        fmsg_reset(0x27, 400, 0xE, 0, 3);
        fmsg_open();
        msg->pad_l = 4;
        msg->pad_t = 2;
        break;
    case 1:
        fbag_list0_resume();
        b->state++;
        break;
    case 2:
        fbag_list0_update();
        break;
    case 3:
        fbag_list1_resume();
        b->state++;
        break;
    case 4:
        fbag_list1_update();
        break;
    case 5:
        fbag_menu0_open();
        b->state++;
        break;
    case 6:
        fbag_menu0_update();
        break;
    case 7:
        fbag_menu1_open();
        b->state++;
        break;
    case 8:
        fbag_menu1_update();
        break;
    case 9:
        fbag_discard_ask();
        b->state++;
        break;
    case 10:
        fbag_discard_update();
        break;
    case 11:
        fmsg_close();
        b->state = 12;
        fld_story_flag(0x6214, 0);
        break;
    case 12:
        if (fmsg_moving() == 0) {
            fbag_close();
            *picked = b->picked;
            return b->result;
        }
        break;
    }
    return 0;
}

#ifdef NON_MATCHING /* the original keeps a copy of the early return per state; every form tried merges them */
void *fbag_draw(void *list)
{
    FieldBag *b = &field_bag;
    FieldBagTab *tabs;

    if (field_bag.state != 12 && field_bag.state != 0) {
        tabs = cfg_get_language() != 0 ? fbag_tabs_en : fbag_tabs_jp;
        menu_layout(&list, &b->menu);
        switch (b->tab) {
        case 0:
            list = fbag_tab_draw_off(list, &tabs[1]);
            menu_draw(&list, &b->w[0]);
            list = fbag_tab_draw(list, &tabs[0]);
            break;
        case 1:
            list = fbag_tab_draw_off(list, &tabs[0]);
            menu_draw(&list, &b->w[1]);
            list = fbag_tab_draw(list, &tabs[1]);
            break;
        }
        menu_draw(&list, &b->w[2]);
        menu_draw(&list, &b->w[3]);
        list = fld_dlg_draw(list);
    }
    return list;
}
#endif

void fbag_open(void)
{
    FieldBag *b = &field_bag;
    FieldStory *st = fld_story_get();
    FieldBagTab *tabs;
    short x;
    MenuItemDef *d;

    switch (cfg_get_language()) {
    case 1:
        tabs = fbag_tabs_en;
        x = 0xBE;
        break;
    case 2:
        tabs = fbag_tabs_en;
        x = 0xBE;
        break;
    case 3:
        tabs = fbag_tabs_en;
        x = 0xBE;
        break;
    case 4:
        tabs = fbag_tabs_en;
        x = 0xBE;
        break;
    case 5:
        tabs = fbag_tabs_en;
        x = 0xBE;
        break;
    case 0:
        tabs = fbag_tabs_jp;
        x = 0x90;
        break;
    }
    b->sel[0] = -1;
    b->sel[1] = -1;
    b->tab = 1;
    memset(b->ids0, 0, sizeof(b->ids0));
    memset(b->ids1, 0, sizeof(b->ids1));
    b->gim_frame = fld_gim_get(0);
    b->gim_icons = fld_gim_get(3);
    b->counts0 = (int *)st->w[6];
    b->counts1 = (int *)st->w[7];
    b->use = (void (*)(int, int))st->w[11];
    fbag_build();
    d = &b->title;
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->style = 2;
    d->x = x;
    d->y = cfg_get_language() == 0 ? 3 : 10;
    d->auto_size = 1;
    d->color = 0xFFFFFFFF;
    d->text = table_get_entry(fld_gim_get(5), 0x12);
    d->font_flag4 = 1;
    d->font_size = 0xF;
    d->text_color = 0xFFFFFFFF;
    d->font_flag8000 = cfg_get_language() == 0;
    d->pad_t = 1;
    d->pad_b = -1;
    d->pad_l = 3;
    d->pad_r = 2;
    menu_add_item(&b->menu, d);
    d = &b->frame;
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->x = cfg_get_language() == 0 ? 0x8C : 0x64;
    d->y = 0x23;
    d->w = cfg_get_language() == 0 ? 0xC7 : 0x117;
    d->h = 0xAD;
    d->color = 0xFFFFFFFF;
    menu_add_item(&b->menu, d);
    fbag_list0_open();
    fbag_list1_open();
    tabs[0].text = table_get_entry(fld_gim_get(5), 0x13);
    tabs[1].text = table_get_entry(fld_gim_get(5), 0x14);
    fbag_list0_resume();
    fbag_list1_resume();
    b->state = 3;
}

void fbag_list0_open(void)
{
    MenuWidget *w = &field_bag.w[0];
    MenuItemDef *d = &field_bag.w[0].def;

    if (w != 0)
        menu_reset(w);
    memset(w, 0, sizeof(*w));
    d->visible = 1;
    d->x = cfg_get_language() == 0 ? 0x91 : 0x69;
    d->y = 0x37;
    d->w = cfg_get_language() == 0 ? 0xBD : 0x10D;
    d->h = 0x93;
    d->pad_l = 2;
    d->pad_t = 5;
    d->color = 0xFFFFFFFF;
    w->no_input = 0;
    w->no_cancel = 1;
    w->cancel_blocked = 0;
    w->keep_open = 1;
    w->cancel_resets = 0;
    w->style = 2;
    w->buttons = 0;
    w->count = fbag_count(0);
    w->rows = 5;
    w->arrow_off = -5;
    w->item_w = cfg_get_language() == 0 ? 0xBE : 0x10E;
    w->item_h = 0x1A;
    w->item_x = 0x1C;
    w->item_y = 0;
    w->cursor_w = cfg_get_language() == 0 ? 0x95 : 0xE5;
    w->cursor_h = 0x18;
    w->draw_item = (MenuDrawItemFn)fbag_item0_draw;
    w->heap = fld_heap_get();
    menu_open(w);
}

void fbag_list1_open(void)
{
    MenuWidget *w = &field_bag.w[1];
    MenuItemDef *d = &field_bag.w[1].def;

    if (w != 0)
        menu_reset(w);
    memset(w, 0, sizeof(*w));
    d->visible = 1;
    d->x = cfg_get_language() == 0 ? 0x91 : 0x69;
    d->y = 0x37;
    d->w = cfg_get_language() == 0 ? 0xBD : 0x10D;
    d->h = 0x93;
    d->pad_l = 2;
    d->pad_t = 5;
    d->color = 0xFFFFFFFF;
    w->no_input = 0;
    w->no_cancel = 1;
    w->cancel_blocked = 0;
    w->keep_open = 1;
    w->cancel_resets = 0;
    w->style = 2;
    w->buttons = 0;
    w->count = fbag_count(1);
    w->rows = 5;
    w->arrow_off = -5;
    w->item_w = cfg_get_language() == 0 ? 0xBE : 0x10E;
    w->item_h = 0x1A;
    w->item_x = 0x1C;
    w->item_y = 0;
    w->cursor_w = cfg_get_language() == 0 ? 0x95 : 0xE5;
    w->cursor_h = 0x18;
    w->draw_item = (MenuDrawItemFn)fbag_item1_draw;
    w->heap = fld_heap_get();
    menu_open(w);
}

void fbag_list0_resume(void)
{
    MenuWidget *w = &field_bag.w[0];

    w->no_cancel = 1;
    w->cancelled = w->accepted = 0;
}

void fbag_list0_update(void)
{
    FieldBag *b = &field_bag;
    MenuWidget *w = &field_bag.w[0];
    int i;

    if (w->count == 0) {
        if (pad_get()->b.trig & region_pick_value(1)) {
            b->result = 1;
            b->state = 11;
        }
    } else {
        switch (menu_update(w)) {
        case -2:
            b->result = 1;
            b->state = 11;
            break;
        default:
            b->state = 5;
            break;
        case -1:
            i = w->scroll + w->cursor;
            if (b->sel[0] != i)
                fmsg_set_text(table_get_entry(fld_gim_get(6), b->ids0[i] + 100), 0);
            b->sel[0] = w->scroll + w->cursor;
            break;
        }
    }
}

void fbag_list1_resume(void)
{
    MenuWidget *w = &field_bag.w[1];

    w->no_cancel = 1;
    w->cancelled = w->accepted = 0;
}

void fbag_list1_update(void)
{
    FieldBag *b = &field_bag;
    MenuWidget *w = &field_bag.w[1];
    int i;

    if (w->count == 0) {
        if (pad_get()->b.trig & region_pick_value(1)) {
            b->result = 1;
            b->state = 11;
        }
    } else {
        switch (menu_update(w)) {
        case -2:
            b->result = 1;
            b->state = 11;
            break;
        default:
            b->state = 7;
            break;
        case -1:
            i = w->scroll + w->cursor;
            if (b->sel[1] != i)
                fmsg_set_text(table_get_entry(fld_gim_get(6), b->ids1[i] + 0x96), 0);
            b->sel[1] = w->scroll + w->cursor;
            break;
        }
    }
}

/* the menu of an item: a single entry */
void fbag_menu0_open(void)
{
    MenuWidget *w = &field_bag.w[2];
    FieldBag *b = &field_bag;
    MenuItemDef *d = &field_bag.w[2].def;
    int off;

    memset(w, 0, sizeof(*w));
    d->visible = 1;
    d->x = cfg_get_language() == 0 ? 0x136 : 0x15E;
    d->y = b->w[0].cursor * b->w[0].item_h + 0x37;
    if (b->w[0].cursor < 3)
        off = 0x1F;
    else
        off = -0xD;
    d->y += (short)off;
    d->auto_size = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->pad_t = 1;
    d->pad_b = 1;
    d->pad_l = 1;
    d->pad_r = 1;
    w->count = 1;
    w->rows = 1;
    d->color = 0xFFFFFFFF;
    w->no_input = 0;
    w->no_cancel = 1;
    w->cancel_blocked = 0;
    w->keep_open = 0;
    w->cancel_resets = 0;
    w->style = 2;
    w->buttons = 0;
    w->item_w = (font_measure_text(table_get_entry(fld_gim_get(5), 0x15)) >> 6) + 9;
    w->item_h = cfg_get_language() == 0 ? 0x11 : 0x13;
    w->cursor_h = cfg_get_language() == 0 ? 0xF : 0x11;
    w->draw_item = (MenuDrawItemFn)fbag_menu0_item;
    w->heap = fld_heap_get();
    menu_open(w);
}

void fbag_menu0_update(void)
{
    MenuWidget *w = &field_bag.w[2];
    FieldBag *b = &field_bag;
    int r = menu_update(w);

    if (r == -1)
        return;
    menu_reset(w);
    switch (r) {
    case -2:
        b->state = 1;
        break;
    case 0:
        b->result = 2;
        b->picked = b->ids0[b->sel[0]];
        b->state = 11;
        break;
    }
}

/* the menu of a card: pick it or throw it away */
void fbag_menu1_open(void)
{
    MenuWidget *w = &field_bag.w[3];
    FieldBag *b = &field_bag;
    MenuItemDef *d = &field_bag.w[3].def;
    int off;

    memset(w, 0, sizeof(*w));
    d->visible = 1;
    d->x = cfg_get_language() == 0 ? 0x136 : 0x15E;
    d->y = b->w[1].cursor * b->w[0].item_h + 0x37;
    if (b->w[1].cursor < 3)
        off = 0x1F;
    else
        off = -0x1C;
    d->y += (short)off;
    d->auto_size = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->pad_t = 1;
    d->pad_b = 1;
    d->pad_l = 1;
    d->pad_r = 1;
    d->color = 0xFFFFFFFF;
    w->no_input = 0;
    w->no_cancel = 1;
    w->cancel_blocked = 0;
    w->keep_open = 0;
    w->cancel_resets = 0;
    w->count = 2;
    w->rows = 2;
    w->style = 2;
    w->buttons = 0;
    w->item_w = fbag_menu1_width() + 8;
    w->item_h = cfg_get_language() == 0 ? 0x11 : 0x13;
    w->cursor_h = cfg_get_language() == 0 ? 0xF : 0x11;
    w->draw_item = (MenuDrawItemFn)fbag_menu1_item;
    w->heap = fld_heap_get();
    menu_open(w);
}

int fbag_menu1_width(void)
{
    int i;
    int max = 0;
    int id;
    int t;

    font_set_size(0xF, 0xF);
    font_set_flag4(1);
    for (i = 0; i < 2; i++) {
        if (i != 1)
            id = 0x15;
        else
            id = 0x16;
        t = font_measure_text(table_get_entry(fld_gim_get(5), id)) >> 6;
        if (max < t)
            max = t;
    }
    return max + 1;
}

void fbag_menu1_update(void)
{
    MenuWidget *w = &field_bag.w[3];
    FieldBag *b = &field_bag;
    int r = menu_update(w);

    if (r == -1)
        return;
    menu_reset(w);
    switch (r) {
    case -2:
        b->state = 3;
        break;
    case 0:
        b->result = 3;
        b->picked = b->ids1[b->sel[1]];
        b->state = 11;
        break;
    case 1:
        b->state = 9;
        break;
    }
}

void fbag_discard_ask(void)
{
    fld_dlg_open(table_get_entry(fld_gim_get(5), 0x17), 3, 2);
}

void fbag_discard_update(void)
{
    FieldBag *b = &field_bag;
    FieldStory *st = fld_story_get();

    switch (fld_dlg_update()) {
    case -2:
    case 3:
        b->state = 3;
        break;
    case 2:
        ((void (*)(int, int))st->w[11])(b->ids1[b->sel[1]], -1);
        fbag_build();
        fbag_list1_open();
        fbag_help();
        b->state = 3;
        break;
    }
}

void fbag_close(void)
{
    menu_reset(&field_bag.w[0]);
    menu_reset(&field_bag.w[3]);
    fbag_free();
}

/* the tab in front */
void *fbag_tab_draw(void *list, FieldBagTab *t)
{
    int y;
    unsigned short *text;
    int x;

    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, field_bag.gim_frame);
    if (cfg_get_language() == 0)
        list = fld_draw_cell1(list, t->x, t->y, t->spr.u, t->spr.v, t->spr.w, t->spr.h);
    else
        list = fld_draw_frame(list, t->x, t->y, t->spr.u, t->spr.v, t->spr.w, t->spr.h, t->spr.w + 0x28,
                              t->spr.h, 3, 3, 3, 3);
    font_text_set(list);
    font_set_size(0xF, 0xF);
    font_set_flag4(1);
    font_set_flag8000(0);
    font_set_color(0xFF000000);
    y = t->y + 2;
    text = t->text;
    x = t->x + (cfg_get_language() == 0 ? 4 : 10);
    font_draw_text(x * 64, y * 64, (480 - x) * 64, text);
    return font_text_clear();
}

/* the tab behind, greyed and lower */
void *fbag_tab_draw_off(void *list, FieldBagTab *t)
{
    int y;
    unsigned short *text;
    int x;

    list = dl_ambient_color(list, 0xFFC0C0C0);
    list = gim_set_texture(list, field_bag.gim_frame);
    if (cfg_get_language() == 0)
        list = fld_draw_frame(list, t->x, t->y + 2, t->spr.u, t->spr.v, t->spr.w, t->spr.h, t->spr.w,
                              t->spr.h - 2, 3, 3, 3, 3);
    else
        list = fld_draw_frame(list, t->x, t->y + 2, t->spr.u, t->spr.v, t->spr.w, t->spr.h, t->spr.w + 0x28,
                              t->spr.h - 2, 3, 3, 3, 3);
    font_text_set(list);
    font_set_size(0xF, 0xF);
    font_set_flag4(1);
    font_set_flag8000(0);
    font_set_color(0xFF000000);
    y = t->y + 4;
    text = t->text;
    x = t->x + (cfg_get_language() == 0 ? 4 : 10);
    font_draw_text(x * 64, y * 64, (480 - x) * 64, text);
    return font_text_clear();
}

/* a row of the item list: icon, name and count */
void *fbag_item0_draw(void *list, unsigned short i, int x, int y)
{
    FieldBag *b = &field_bag;
    int id = field_bag.ids0[i];
    unsigned short *name = table_get_entry(fld_gim_get(6), id);
    int n = field_bag.counts0[id];
    int big = n >= 10;
    unsigned short num[4];
    int w;
    int ty;
    int tx;
    unsigned short *times;

    if (big) {
        num[0] = n / 10 + '0';
        num[1] = 0;
    }
    num[2] = n % 10 + '0';
    num[3] = 0;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, b->gim_frame);
    list = fld_draw_cell1(list, x, y, fbag_slot.u, fbag_slot.v, fbag_slot.w, fbag_slot.h);
    list = gim_set_texture(list, b->gim_icons);
    list = fld_draw_cell1(list, x, y, fbag_icons0[id].x, fbag_icons0[id].y, 0x18, 0x18);
    font_text_set(list);
    font_set_size(0xF, 0xF);
    font_set_flag4(1);
    font_set_flag8000(cfg_get_language() == 0);
    font_set_color(0xFF000000);
    w = cfg_get_language() == 0 ? 0x6E : 0xBE;
    font_draw_wtext((x + 0x21) * 64, (y + (cfg_get_language() == 0 ? 1 : 5)) * 64, w * 64, name);
    times = table_get_entry(fld_gim_get(5), 0x18);
    ty = y + (cfg_get_language() == 0 ? 1 : 5);
    tx = x + (cfg_get_language() == 0 ? 0x8B : 0xDB);
    font_draw_text(tx * 64, ty * 64, (480 - tx) * 64, times);
    if (big) {
        ty = y + (cfg_get_language() == 0 ? 1 : 5);
        tx = x + (cfg_get_language() == 0 ? 0x98 : 0xE8);
        font_draw_text(tx * 64, ty * 64, (480 - tx) * 64, &num[0]);
    }
    ty = y + (cfg_get_language() == 0 ? 1 : 5);
    tx = x + (cfg_get_language() == 0 ? 0xA3 : 0xF3);
    font_draw_text(tx * 64, ty * 64, (480 - tx) * 64, &num[2]);
    return font_text_clear();
}

/* a row of the card list: icon and name */
#ifdef NON_MATCHING /* the original keeps &field_bag in $s1 for the cursor_w read; mwcc rematerializes it */
void *fbag_item1_draw(void *list, unsigned short i, int x, int y)
{
    FieldBag *b = &field_bag;
    int id = b->ids1[i];
    unsigned short *name = table_get_entry(fld_gim_get(6), id + 0x32);
    int w;

    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, field_bag.gim_frame);
    list = fld_draw_cell1(list, x, y, fbag_slot.u, fbag_slot.v, fbag_slot.w, fbag_slot.h);
    list = gim_set_texture(list, field_bag.gim_icons);
    list = fld_draw_cell1(list, x, y, fbag_icons1[id].x, fbag_icons1[id].y, 0x18, 0x18);
    font_text_set(list);
    font_set_size(0xF, 0xF);
    font_set_flag4(1);
    font_set_flag8000(cfg_get_language() == 0);
    font_set_color(0xFF000000);
    w = b->w[1].cursor_w - 10;
    font_draw_wtext((x + 0x21) * 64, (y + (cfg_get_language() == 0 ? 1 : 5)) * 64, w * 64, name);
    return font_text_clear();
}
#endif

void *fbag_menu0_item(void *list, unsigned short i, int x, int y)
{
    unsigned short *text = 0;

    switch (i) {
    case 0:
        text = table_get_entry(fld_gim_get(5), 0x15);
        break;
    }
    font_text_set(list);
    font_set_size(0xF, 0xF);
    font_set_flag4(1);
    font_set_color(0xFF000000);
    font_draw_text((x + 4) * 64, y * 64, (480 - (x + 4)) * 64, text);
    return font_text_clear();
}

void *fbag_menu1_item(void *list, unsigned short i, int x, int y)
{
    unsigned short *text = 0;

    switch (i) {
    case 0:
        text = table_get_entry(fld_gim_get(5), 0x15);
        break;
    case 1:
        text = table_get_entry(fld_gim_get(5), 0x16);
        break;
    }
    font_text_set(list);
    font_set_size(0xF, 0xF);
    font_set_flag4(1);
    font_set_color(0xFF000000);
    font_draw_text((x + 4) * 64, y * 64, (480 - (x + 4)) * 64, text);
    return font_text_clear();
}

/* entries of a tab: kinds of items held, or cards held */
int fbag_count(int tab)
{
    FieldBag *b = &field_bag;
    int i = 1;
    int n = 0;

    switch (tab) {
    case 0:
        for (; i < 21; i++) {
            if (b->counts0[i] > 0)
                n++;
        }
        break;
    case 1:
        for (; i < 51; i++) {
            if (b->counts1[i] > 0)
                n += b->counts1[i];
        }
        break;
    }
    return n;
}

void fbag_build(void)
{
    FieldBag *b = &field_bag;
    FieldStory *st = fld_story_get();
    int i;
    int j;
    int n = 0;

    for (i = 1; i < 21; i++) {
        if (((int *)st->w[6])[i] > 0) {
            b->ids0[n] = i;
            n++;
        }
    }
    n = 0;
    for (i = 1; i < 51; i++) {
        if (b->counts1[i] > 0) {
            for (j = 0; j < b->counts1[i]; j++) {
                b->ids1[n] = i;
                n++;
            }
        }
    }
}

/* the help of the entry under the cursor in the message window */
void fbag_help(void)
{
    FieldBag *b = &field_bag;
    FieldStory *st = fld_story_get();
    int id;

    switch (field_bag.tab) {
    case 0:
        if (b->sel[0] >= 0) {
            id = b->ids0[b->sel[0]];
            if (((int *)st->w[6])[id] > 0)
                fmsg_set_text(table_get_entry(fld_gim_get(6), id + 100), 0);
            else
                fmsg_set_text(0, 0);
        }
        break;
    case 1:
        if (b->sel[1] >= 0) {
            id = b->ids1[b->sel[1]];
            if (((int *)st->w[7])[id] > 0)
                fmsg_set_text(table_get_entry(fld_gim_get(6), id + 0x96), 0);
            else
                fmsg_set_text(0, 0);
        }
        break;
    }
}
