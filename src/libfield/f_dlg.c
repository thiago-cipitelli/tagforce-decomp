// @range 0x0882B7C8-0x0882BE80
// @rodata 0x08839A74-0x08839AD4
/* Yes/no dialog of the field (field_dlg) and the panel of the face buttons of the scene commands:
   which buttons are offered and which one was pressed. */
#include "libfield.h"

int font_measure_text(unsigned short *text);
unsigned short region_pick_value(int flag);
void *gim_set_texture(void *list, void *gim);
void *fld_draw_cell(void *list, int x, int y, int u, int v, int sw, int sh, int dw, int dh);
void *fld_draw_cell1(void *list, int x, int y, int u, int v, int w, int h);
void *fld_draw_frame(void *list, int x, int y, int u, int v, int cw, int ch, int w, int h, int l, int t,
                     int r, int b);

/* text offsets of the four buttons in the panel, by language */
const FdlgPos fdlg_pos[6][4] = {
    { { 109, 47 }, { 61, 20 }, { 7, 47 }, { 61, 75 } },
    { { 66, 75 }, { 66, 20 }, { 24, 47 }, { 109, 47 } },
    { { 66, 75 }, { 66, 20 }, { 24, 47 }, { 109, 47 } },
    { { 66, 75 }, { 66, 20 }, { 24, 47 }, { 109, 47 } },
    { { 66, 75 }, { 66, 20 }, { 24, 47 }, { 109, 47 } },
    { { 66, 75 }, { 66, 20 }, { 24, 47 }, { 109, 47 } },
};

void fld_dlg_open(unsigned short *text, int buttons, int button)
{
    MenuWidget *w = &field_dlg.w;
    MenuItemDef *d = &field_dlg.w.def;
    int init;
    int tw;
    int x;
    int y;
    int wd;
    int h;

    if (field_dlg.open != 0)
        return;
    memset(w, 0, sizeof(*w));
    switch (buttons) {
    case 2:
    default:
        init = 4;
        break;
    case 3:
        init = 3;
        break;
    }
    d->visible = 1;
    d->auto_size = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->font_flag4 = 1;
    d->pad_t = 8;
    d->pad_l = 10;
    d->pad_r = 10;
    d->color = 0xFFFFFFFF;
    d->style = 0;
    d->x = 0;
    d->y = 0;
    d->pad_b = 0;
    d->text = text;
    d->font_flag8000 = cfg_get_language() == 0;
    d->font_size = 0xD;
    d->text_color = 0xFF000000;
    w->no_cancel = 1;
    w->cancel_resets = 1;
    w->buttons = buttons;
    w->button = button;
    w->initial = init;
    w->heap = fld_heap_get();
    menu_open(w);
    d->x = 240.0f - d->w * 0.5f;
    d->y = 136.0f - d->h * 0.5f;
    font_set_flag8000(d->font_flag8000);
    font_set_size(d->font_size, d->font_size);
    font_set_flag4(d->font_flag4);
    tw = (font_measure_text(text) >> 6) + 1;
    menu_item_rect(&w->menu, d, &x, &y, &wd, &h);
    if (tw < wd)
        d->pad_l += (short)(int)((wd - tw) * 0.5f);
    fld_story_flag(0x621A, 0);
    field_dlg.open = 1;
}

int fld_dlg_update(void)
{
    MenuWidget *w = &field_dlg.w;
    int r = menu_update(w);

    if (r != -1) {
        menu_reset(w);
        field_dlg.open = 0;
    }
    return r;
}

void *fld_dlg_draw(void *list)
{
    MenuWidget *w = &field_dlg.w;

    if (field_dlg.open == 0)
        return list;
    menu_draw(&list, w);
    return list;
}

/* index of the face button of mask pressed in pad (0 ok, 1 triangle, 2 square, 3 cancel), or -1 */
int fdlg_button(PadState *pad, unsigned int mask)
{
    int ret = -1;
    int ok;
    int tri;
    int sq;
    int cancel;

    switch (region_get()) {
    case 1:
    default:
        ok = 0x4000;
        tri = 0x1000;
        sq = 0x8000;
        cancel = 0x2000;
        break;
    case 0:
        ok = 0x2000;
        tri = 0x1000;
        sq = 0x8000;
        cancel = 0x4000;
        break;
    }
    if ((mask & 1) && (pad->b.trig & ok))
        ret = 0;
    if ((mask & 2) && (pad->b.trig & tri))
        ret = 1;
    if ((mask & 4) && (pad->b.trig & sq))
        ret = 2;
    if ((mask & 8) && (pad->b.trig & cancel))
        ret = 3;
    return ret;
}

void *fdlg_draw_buttons(void *list, unsigned int mask)
{
    const FdlgPos *p = fdlg_pos[cfg_get_language()];
    unsigned short *text;

    list = gim_set_texture(list, fld_gim_get(0));
    list = fld_draw_frame(list, 0xA0, 0x9F, 0x59, 0x3B, 10, 10, 0xA1, 0x50, 4, 5, 4, 5);
    list = fld_draw_cell1(list, 0xA0, 0x91, 0x59, 0x2B, 3, 0x10);
    list = fld_draw_cell1(list, 0xCA, 0x91, 0x5F, 0x2B, 8, 0x10);
    list = fld_draw_cell(list, 0xA3, 0x91, 0x5C, 0x2B, 3, 0x10, 0x27, 0x10);
    list = fld_draw_cell1(list, 0xD9, 0xB4, 0x2A, 0x20, 0x2F, 0x26);
    list = fld_draw_cell1(list, 0xA5, 0x94, 0x59, 0x20, 0x17, 0xB);
    font_text_set(list);
    font_set_size(0xD, 0xD);
    font_set_flag4(1);
    font_set_color(0xFFFFFFFF);
    if (mask & 1) {
        text = table_get_entry(fld_gim_get(5), 1);
        font_draw_text((p[0].x + 0xA0) << 6, (p[0].y + 0x91) << 6, (0x1E0 - (p[0].x + 0xA0)) << 6, text);
    }
    if (mask & 2) {
        text = table_get_entry(fld_gim_get(5), 3);
        font_draw_text((p[1].x + 0xA0) << 6, (p[1].y + 0x91) << 6, (0x1E0 - (p[1].x + 0xA0)) << 6, text);
    }
    if (mask & 4) {
        text = table_get_entry(fld_gim_get(5), 2);
        font_draw_text((p[2].x + 0xA0) << 6, (p[2].y + 0x91) << 6, (0x1E0 - (p[2].x + 0xA0)) << 6, text);
    }
    if (mask & 8) {
        text = table_get_entry(fld_gim_get(5), 6);
        font_draw_text((p[3].x + 0xA0) << 6, (p[3].y + 0x91) << 6, (0x1E0 - (p[3].x + 0xA0)) << 6, text);
    }
    font_draw_text(0xBD << 6, 0x93 << 6, (0x1E0 - 0xBD) << 6, table_get_entry(fld_gim_get(5), 0x69));
    return font_text_clear();
}
