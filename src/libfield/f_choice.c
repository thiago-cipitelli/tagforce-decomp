// @range 0x0882A3A8-0x0882A918
// @asm 0x0882A844-0x0882A918
// @rodata 0x08839A4C-0x08839A74
/* Menu of five choices of the scene commands (field_choice): a title window and a list of the
   texts 0x2B..0x2F of the field table; returns 1..5 for the choice, 0 when cancelled. */
#include "libfield.h"

int font_measure_text(unsigned short *text);

void fcho_title_setup(void);
void fcho_list_setup(void);
int fcho_list_width(void);
void *fcho_list_item(void *list, unsigned int i, int x, int y);

const unsigned short fcho_texts[5] = { 0x2B, 0x2C, 0x2D, 0x2E, 0x2F };

void fcho_open(void)
{
    memset(&field_choice, 0, sizeof(field_choice));
    menu_init(&field_choice.menu, fld_heap_get());
    fcho_title_setup();
    fcho_list_setup();
    fld_story_flag(0x6216, 0);
}

void fcho_close(void)
{
    menu_free_items(&field_choice.menu);
    menu_reset(&field_choice.w);
    memset(&field_choice, 0, sizeof(field_choice));
}

int fcho_update(void)
{
    FieldChoice *c = &field_choice;
    int ret = -1;
    int r = menu_update(&field_choice.w);

    if (r != ret) {
        menu_reset(&c->w);
        switch (r) {
        default:
        case -2:
        case -1:
            ret = 0;
            break;
        case 0:
            ret = 1;
            break;
        case 1:
            ret = 2;
            break;
        case 2:
            ret = 3;
            break;
        case 3:
            ret = 4;
            break;
        case 4:
            ret = 5;
            break;
        }
    }
    return ret;
}

void *fcho_draw(void *list)
{
    menu_layout(&list, &field_choice.menu);
    menu_draw(&list, &field_choice.w);
    return list;
}

void fcho_title_setup(void)
{
    MenuItemDef *d = &field_choice.title;

    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->auto_size = 1;
    d->x = 0xAA;
    d->y = 0x3C;
    d->color = 0xFFFFFFFF;
    d->style = 2;
    d->text = table_get_entry(fld_gim_get(5), 0x2A);
    d->font_flag4 = 1;
    d->pad_t = 1;
    d->font_size = 15;
    d->text_color = 0xFFFFFFFF;
    d->pad_b = -1;
    d->pad_l = 3;
    d->pad_r = 2;
    d->font_flag8000 = 0;
    menu_add_item(&field_choice.menu, d);
    d->x = 240.0f - d->w * 0.5f;
}

void fcho_list_setup(void)
{
    MenuWidget *w = &field_choice.w;
    MenuItemDef *d = &field_choice.w.def;

    memset(w, 0, sizeof(*w));
    d->visible = 1;
    d->auto_size = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->pad_t = 1;
    d->pad_b = 1;
    d->pad_l = 1;
    d->pad_r = 1;
    d->x = 0xAA;
    d->y = 0x55;
    d->color = 0xFFFFFFFF;
    w->no_cancel = 1;
    w->style = 2;
    w->buttons = 0;
    w->count = 5;
    w->item_w = fcho_list_width() + 8;
    /* the second test of the language is in the original */
    w->item_h = cfg_get_language() == 0 ? 0x19 : cfg_get_language() == 0 ? 0x11 : 0x13;
    w->cursor_h = cfg_get_language() == 0 ? 0x17 : cfg_get_language() == 0 ? 0xF : 0x11;
    w->draw_item = (MenuDrawItemFn)fcho_list_item;
    w->heap = fld_heap_get();
    menu_open(w);
    d->x = 240.0f - d->w * 0.5f;
}

int fcho_list_width(void)
{
    int i;
    int max = 0;
    int n;
    const unsigned short *t;

    font_set_size(15, 15);
    font_set_flag4(1);
    for (i = 0, t = fcho_texts; i < 5; i++, t++) {
        n = font_measure_text(table_get_entry(fld_gim_get(5), *t)) >> 6;
        if (max < n)
            max = n;
    }
    return max + 1;
}

#ifdef NON_MATCHING /* the original loads the text id before the calls, in the prologue */
void *fcho_list_item(void *list, unsigned int i, int x, int y)
{
    int id = fcho_texts[i & 0xFFFF];

    font_set_flag8000(cfg_get_language() == 0);
    font_text_set(list);
    font_set_size(15, 15);
    font_set_flag4(1);
    font_set_flag2(1);
    font_set_color(0xFF000000);
    font_draw_text((x + 4) << 6, (y + 1) << 6, (0x1E0 - (x + 4)) << 6, table_get_entry(fld_gim_get(5), id));
    return font_text_clear();
}
#endif
