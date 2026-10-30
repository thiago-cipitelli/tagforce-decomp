// @range 0x08827FE4-0x088290BC
// @asm 0x088280F8-0x0882834C
// @asm 0x088289AC-0x08828EC4
// @rodata 0x08839924-0x088399B4
/* Menu of the hour to wait until (field_hour_menu): the title window, the choice list with its help
   text and the hour picker that moves by quarters of hour between the current time and 23:45. */
#include "libfield.h"

int sysstate_get_u32_97e8(void);
FieldStory *fld_story_get(void);
void *gim_set_texture(void *list, void *gim);
void *dl_ambient_color(void *list, unsigned int color);
void *fld_draw_cell(void *list, int x, int y, int u, int v, int sw, int sh, int dw, int dh);
void *fld_draw_cell1(void *list, int x, int y, int u, int v, int w, int h);
int font_measure_text(unsigned short *text);

int fclk_list_width(void);
void fclk_time_setup(void);
void *fclk_list_item(void *list, unsigned int i, int x, int y);
void *fclk_label_jp(void *list, int x, int y);
void *fclk_label(void *list, int x, int y);
void fclk_index_to_time(int *hour, int *minute, int i);
void fclk_time_to_index(int *i, int hour, int minute);

/* positions inside the hour picker, by region: two labels, AM/PM, hour, ':', minutes, frame and
   the two arrows */
const FclkPos fclk_pos[2][9] = {
    { { 0x0D, 0x0C }, { 0x90, 0x0C }, { 0x03, 0x0C }, { 0x1E, 0x0C }, { 0x35, 0x0B },
      { 0x3A, 0x0C }, { 0x00, 0x11 }, { 0x30, 0x06 }, { 0x30, 0x23 } },
    { { 0x0D, 0x0C }, { 0x90, 0x0C }, { 0x37, 0x0C }, { 0x03, 0x0C }, { 0x1A, 0x0B },
      { 0x1F, 0x0C }, { 0x00, 0x11 }, { 0x16, 0x06 }, { 0x16, 0x23 } },
};

void fclk_title_setup(void)
{
    FieldHourMenu *c = &field_hour_menu;
    MenuItemDef *d = &field_hour_menu.title;

    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->x = 0;
    d->y = cfg_get_language() == 0 ? 0x43 : 0x4B;
    d->auto_size = 1;
    d->color = 0xFFFFFFFF;
    d->style = 2;
    d->text = table_get_entry(fld_gim_get(5), 0x1D);
    d->font_flag4 = 1;
    d->font_flag8000 = cfg_get_language() == 0;
    d->font_size = 15;
    d->text_color = 0xFFFFFFFF;
    d->pad_b = -1;
    d->pad_t = 1;
    d->pad_l = 3;
    d->pad_r = 2;
    menu_add_item(&c->menu, d);
    d->x = 240.0f - d->w * 0.5f;
}

#ifdef NON_MATCHING /* the stores of x, y and color are scheduled after the bitfields; the original has them first */
void fclk_list_setup(void)
{
    MenuWidget *w = &field_hour_menu.w[0];
    MenuItemDef *d = &field_hour_menu.w[0].def;
    FieldStory *st = fld_story_get();
    int more = 0;

    field_hour_menu.texts[0] = table_get_entry(fld_gim_get(5), 0x1E);
    field_hour_menu.texts[1] = table_get_entry(fld_gim_get(5), 0x1F);
    field_hour_menu.texts[2] = table_get_entry(fld_gim_get(5), 0x20);
    if (sysstate_get_u32_97e8() >= 2) {
        if (*(int *)st->w[0] == 7) {
            if (*(int *)st->w[1] != 0x1F)
                more = 1;
        } else {
            more = 1;
        }
    }
    memset(w, 0, sizeof(*w));
    d->x = 0x9D;
    d->y = 100;
    d->color = 0xFFFFFFFF;
    w->no_cancel = 1;
    w->keep_open = 1;
    w->style = 2;
    w->buttons = 0;
    d->visible = 1;
    d->auto_size = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->pad_t = 1;
    d->pad_b = 1;
    d->pad_l = 1;
    d->pad_r = 1;
    w->count = more == 1 ? 3 : 2;
    w->item_w = fclk_list_width() + 8;
    /* the second test of the language is in the original */
    w->item_h = cfg_get_language() == 0 ? 0x19 : cfg_get_language() == 0 ? 0x11 : 0x13;
    w->cursor_h = cfg_get_language() == 0 ? 0x17 : cfg_get_language() == 0 ? 0xF : 0x11;
    w->draw_item = (MenuDrawItemFn)fclk_list_item;
    w->heap = fld_heap_get();
    menu_open(w);
    d->x = 240.0f - d->w * 0.5f;
}
#endif

int fclk_list_width(void)
{
    FieldHourMenu *c = &field_hour_menu;
    MenuWidget *w = &field_hour_menu.w[0];
    int i;
    int max = 0;
    int n;

    font_set_size(15, 15);
    font_set_flag4(1);
    for (i = 0; i < w->count; i++) {
        n = font_measure_text(c->texts[0]) >> 6;
        if (max < n)
            max = n;
        c = (FieldHourMenu *)((int *)c + 1);
    }
    return max + 1;
}

void fclk_time_setup(void)
{
    MenuWidget *w = &field_hour_menu.w[1];
    MenuItemDef *d = &field_hour_menu.w[1].def;

    memset(w, 0, sizeof(*w));
    d->visible = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->x = 0xB9;
    d->y = 0x78;
    d->w = 0xE2;
    d->h = 0x53;
    d->color = 0xFFFFFFFF;
    d->pad_t = 0x20;
    d->font_size = 0xD;
    w->no_cancel = 1;
    w->style = 0;
    w->buttons = 2;
    w->heap = fld_heap_get();
    menu_open(w);
}

void fclk_hint_init(void)
{
    FieldHourMenu *c = &field_hour_menu;
    MenuWidget *w = &field_hour_menu.w[0];
    MenuItemDef *m = fmsg_def();

    w->cancelled = w->accepted = 0;
    field_hour_menu.hint = w->scroll + w->cursor;
    fmsg_set_input(0);
    m->pad_l = 4;
    m->pad_t = 2;
    if (field_hour_menu.open != 0)
        fmsg_set_text(table_get_entry(fld_gim_get(5), c->hint + 0x21), 0);
}

void fclk_list_update(void)
{
    FieldHourMenu *c = &field_hour_menu;
    MenuWidget *w = &field_hour_menu.w[0];
    MenuItemDef *m = fmsg_def();
    int i;
    int q;

    switch (menu_update(w)) {
    case -2:
        c->ok = 0;
        c->state = 9;
        break;
    case 0:
        fclk_time_to_index(&q, 0x18, 0);
        if (c->min_sel < q) {
            c->ok = 1;
            c->state = 2;
        } else {
            fmsg_set_input(1);
            m->pad_l = 4;
            m->pad_t = 2;
            fmsg_set_text(table_get_entry(fld_gim_get(5), 0x29), 0);
            c->state = 4;
        }
        break;
    case 1:
        c->state = 5;
        break;
    case 2:
        c->state = 7;
        break;
    case -1:
        i = w->scroll + w->cursor;
        if (c->hint != i) {
            c->hint = i;
            fmsg_set_text(table_get_entry(fld_gim_get(5), i + 0x21), 0);
        }
        break;
    }
}

void fclk_time_open(void)
{
    menu_reset(&field_hour_menu.w[1]);
    fclk_time_setup();
}

void fclk_time_update(void)
{
    FieldHourMenu *c = &field_hour_menu;
    MenuWidget *w = &field_hour_menu.w[1];
    unsigned int rep = pad_get()->b.rep;
    unsigned int down = pad_get()->b.down;
    unsigned int trig = pad_get()->b.trig;

    field_hour_menu.left = 1;
    field_hour_menu.right = 1;
    if (down & 0x10)
        c->left = 2;
    if (down & 0x40)
        c->right = 2;
    if (trig & 0x10)
        c->blink = 0x10;
    if (trig & 0x40)
        c->blink = 0x10;
    c->blink++;
    if ((rep & 0x10) && c->sel > c->min_sel) {
        c->sel--;
        fld_story_flag(0x6212, 0);
    }
    if ((rep & 0x40) && c->sel < 0x5F) {
        c->sel++;
        fld_story_flag(0x6212, 0);
    }
    fclk_index_to_time(&c->hour, &c->minute, c->sel);
    if (c->sel <= c->min_sel)
        c->left = 0;
    if (c->sel >= 0x5F)
        c->right = 0;
    switch (menu_update(w)) {
    case -2:
    case 4:
        c->open = 0;
        c->state = 0;
        break;
    case 1:
        c->state = 9;
        break;
    case -1:
        break;
    }
}

void fclk_finish(void)
{
    menu_finish(&field_hour_menu.w[0]);
    menu_finish(&field_hour_menu.w[1]);
    fmsg_close();
}

int fclk_closed(void)
{
    FieldHourMenu *c = &field_hour_menu;
    int r;

    if (c->w[0].state == 6) {
        if (c->w[1].state == 6)
            r = 1;
        else
            r = 0;
    } else {
        r = 0;
    }
    return r;
}

void fclk_free(void)
{
    FieldHourMenu *c = &field_hour_menu;
    int i;

    menu_free_items(&field_hour_menu.menu);
    for (i = 0; i < 2; i++) {
        menu_reset(&c->w[0]);
        c = (FieldHourMenu *)((char *)c + sizeof(MenuWidget));
    }
}

#ifdef NON_MATCHING /* fclk_list_item (lógica conferida, 50 palavras): o original materializa &field_hour_menu em $s0 no início e o mantém nas chamadas; fclk_time_draw: o argumento de pilha de fld_draw_cell é guardado uma instrução antes */
void *fclk_list_item(void *list, unsigned int i, int x, int y)
{
    FieldHourMenu *c = &field_hour_menu;
    int flag = font_get_flag8000();

    font_text_set(list);
    font_set_size(15, 15);
    font_set_flag4(1);
    font_set_flag8000(cfg_get_language() == 0);
    font_set_color(0xFF000000);
    font_draw_text((x + 4) << 6, (y + 1) << 6, (0x1E0 - (x + 4)) << 6, c->texts[i & 0xFFFF]);
    list = font_text_clear();
    font_set_flag8000(flag);
    return list;
}

void *fclk_time_draw(void *list)
{
    FieldHourMenu *c = &field_hour_menu;
    MenuWidget *w = &field_hour_menu.w[1];
    MenuItemDef *d = &field_hour_menu.w[1].def;
    const FclkPos *p;
    int x = d->x;
    int y = d->y;
    int flag = font_get_flag8000();
    int dx;
    unsigned short *text;
    int h;
    unsigned short hh[3];
    unsigned short mm[3];
    unsigned char a;
    unsigned int color;

    switch (cfg_get_language()) {
    case 1:
    default:
        list = fclk_label(list, x, y);
        dx = 0x82;
        break;
    case 0:
        list = fclk_label_jp(list, x, y);
        dx = 0x39;
        break;
    }
    switch (region_get()) {
    case 1:
    default:
        p = fclk_pos[1];
        break;
    case 0:
        p = fclk_pos[0];
        break;
    }
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, w->menu.window_gim);
    list = fld_draw_cell(list, dx + (x + p[6].x), y + p[6].y, 0x10, 0x37, 1, 1, 0x53, 0x12);
    if (c->hour < 12)
        text = table_get_entry(fld_gim_get(5), 0x26);
    else
        text = table_get_entry(fld_gim_get(5), 0x27);
    font_text_set(list);
    font_set_color(0xFFFFFFFF);
    font_draw_text((dx + (x + p[2].x)) << 6, (y + p[2].y) << 6, (0x1E0 - (dx + (x + p[2].x))) << 6, text);
    text = table_get_entry(fld_gim_get(5), 0x28);
    font_draw_text((dx + (x + p[4].x)) << 6, (y + p[4].y) << 6, (0x1E0 - (dx + (x + p[4].x))) << 6, text);
    list = font_text_clear();
    h = c->hour % 12;
    hh[2] = 0;
    hh[0] = h / 10 + '0';
    hh[1] = h % 10 + '0';
    font_text_set(list);
    font_set_color(0xFFFFFFFF);
    font_draw_text((dx + (x + p[3].x)) << 6, (y + p[3].y) << 6, (0x1E0 - (dx + (x + p[3].x))) << 6, hh);
    list = font_text_clear();
    mm[2] = 0;
    mm[0] = c->minute / 10 + '0';
    mm[1] = c->minute % 10 + '0';
    font_text_set(list);
    font_set_color(0xFFFFFFFF);
    font_draw_text((dx + (x + p[5].x)) << 6, (y + p[5].y) << 6, (0x1E0 - (dx + (x + p[5].x))) << 6, mm);
    list = font_text_clear();
    if ((int)(c->blink % 24) < 16)
        a = 0xFF;
    else
        a = 0;
    list = gim_set_texture(list, c->gim);
    switch (c->left) {
    case 1:
        color = 0xFFFFFFFF;
        break;
    case 2:
        color = (a << 24) | 0xFFFFFF;
        break;
    default:
        color = 0;
        break;
    }
    list = dl_ambient_color(list, color);
    list = fld_draw_cell1(list, dx + (x + p[7].x), y + p[7].y, 100, 0x49, 0xE, 0xE);
    switch (c->right) {
    case 1:
        color = 0xFFFFFFFF;
        break;
    case 2:
        color = (a << 24) | 0xFFFFFF;
        break;
    default:
        color = 0;
        break;
    }
    list = dl_ambient_color(list, color);
    list = fld_draw_cell1(list, dx + (x + p[8].x), y + p[8].y, 0x72, 0x49, 0xE, 0xE);
    font_set_flag8000(flag);
    return list;
}
#endif

void *fclk_label_jp(void *list, int x, int y)
{
    unsigned short *text;

    font_text_set(list);
    font_set_size(15, 15);
    font_set_flag4(1);
    font_set_flag8000(1);
    font_set_color(0xFF000000);
    text = table_get_entry(fld_gim_get(5), 0x24);
    font_draw_text((x + fclk_pos[0][0].x) << 6, (y + fclk_pos[0][0].y) << 6, (0x1E0 - (x + fclk_pos[0][0].x)) << 6, text);
    text = table_get_entry(fld_gim_get(5), 0x25);
    font_draw_text((x + fclk_pos[0][1].x) << 6, (y + fclk_pos[0][1].y) << 6, (0x1E0 - (x + fclk_pos[0][1].x)) << 6, text);
    return font_text_clear();
}

void *fclk_label(void *list, int x, int y)
{
    unsigned short *text;

    font_text_set(list);
    font_set_size(15, 15);
    font_set_flag4(1);
    font_set_flag8000(1);
    font_set_color(0xFF000000);
    text = table_get_entry(fld_gim_get(5), 0x25);
    font_draw_text((x + fclk_pos[0][0].x) << 6, (y + fclk_pos[0][0].y) << 6, (0x1E0 - (x + fclk_pos[0][0].x)) << 6, text);
    return font_text_clear();
}

/* quarters of hour since 0:00 <-> hour and minutes */
void fclk_index_to_time(int *hour, int *minute, int i)
{
    *hour = i / 4;
    *minute = i % 4 * 15;
}

void fclk_time_to_index(int *i, int hour, int minute)
{
    *i = minute / 15 + hour * 4;
}
