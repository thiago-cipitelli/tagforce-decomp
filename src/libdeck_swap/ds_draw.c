// @range 0x08806654-0x08806F0C
// @asm 0x0880687C-0x08806B68
// @asm 0x08806B68-0x08806F0C
/* Frame drawing: the export called once per frame (clear, background, info strip, panes, popups,
   fade), the popup layer over the panes (focus frame, help window, the popup of each state) and the
   card info strip (same source as libselect_card sc_draw.c and sc_info.c). */
#include "libdeck_swap.h"

void *ds_draw_bg(void *list);
void *ds_draw_info(void *list, DsCursor *c);
void *ds_draw_panes(void *list);
void *ds_draw_overlay(void *list);
int ds_osk_update(int id);
void *ds_draw_sort_menu(void *list);
void *ds_draw_action_menu(void *list);
void *ds_sort_menu_draw(void *list);
void *ds_draw_confirm(void *list);
void *ds_draw_result(void *list);
void *ds_draw_detail(void *list);
void *ds_popup_draw(void *list);
void *cardlist_build(void *list, DsScroll *s);
unsigned short *card_info_fill(int id);
unsigned short *card_get_desc(int id);
int font_get_cursor_x(void);
void *ds_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type);
void *ds_draw_attr_icon(void *list, unsigned short x, unsigned short y, unsigned short attr);
void *ds_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind);

/* export: once per frame */
void libdeck_swap_08806654(void)
{
    void *list;

    switch (ds.state) {
    case 0:
    case 10:
        list = dl_select_layer(0, 0);
        list = dl_clear_color(list, 0xFF000000);
        list = dl_clear_depth(list, 0);
        list = dl_clear_stencil(list, 0);
        list = dl_clear(list, 0x17);
        dl_set_list(list);
        break;
    default:
        if (dsv.info) {
            cardinfo_draw();
            break;
        }
        list = dl_select_layer(0, 0);
        list = dl_clear_color(list, 0xFF000000);
        list = dl_clear_depth(list, 0);
        list = dl_clear_stencil(list, 0);
        list = dl_clear(list, 0x17);
        dl_set_list(list);
        list = dl_select_layer(4, 0);
        list = dl_enable(list, 9);
        list = dl_enable(list, 4);
        list = dl_tex_func(list, 0, 1);
        list = dl_set_tex_filter(list, 0, 0);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_disable(list, 1);
        list = ds_draw_bg(list);
        list = ds_draw_info(list, &dsv.cursor);
        list = ds_draw_panes(list);
        list = ds_draw_overlay(list);
        fade_apply(&list);
        dl_set_list(list);
        if (dsv.busy && ds_osk_update(4))
            dsv.busy = 0;
        break;
    }
}

#ifdef NON_MATCHING /* o original calcula a base de dsv.pane[lado] uma vez e a reaproveita nos dois blocos; o mwcc atual a recalcula */
/* focus frame and help window (help position away from the cursor), then the state's popup */
void *ds_draw_overlay(void *list)
{
    DsRect *f = &ds.focus;
    int hx;
    int hy;
    void *gim;

    switch (ds.state) {
    case 7:
    case 8:
        if (dsv.mode > 1) {
            if (dsv.mode < 4) {
                hx = dsv.pane[0].lay.x + 2;
                if (f->y + f->h / 2 > 0x88)
                    hy = 0x36;
                else
                    hy = 0xA2;
            } else {
                if (dsv.side == 0)
                    hx = 0xFE;
                else
                    hx = 0x0E;
                hy = 0x70;
            }
        } else {
            if (dsv.pane[dsv.side].list_mode == 1)
                hx = 0xD0;
            else if (f->x + f->w > dsv.pane[dsv.side].lay.x + dsv.pane[dsv.side].lay.w / 2)
                hx = dsv.pane[dsv.side].lay.x + 2;
            else
                hx = 0xD0;
            if (f->y + f->h / 2 > 0x88)
                hy = 0x36;
            else
                hy = 0xA2;
        }
    case 1:
        gim = res_get(0x8006);
        list = ds_draw_frame(list, f->x, f->y, f->w, f->h, gim, gfx_get_state()[0]);
    }
    switch (ds.state) {
    case 0:
    case 1:
        break;
    case 2:
        list = ds_draw_sort_menu(list);
        break;
    case 3:
        list = ds_draw_action_menu(list);
        break;
    case 4:
        list = ds_sort_menu_draw(list);
        break;
    case 5:
        list = ds_draw_confirm(list);
        break;
    case 6:
        list = ds_draw_result(list);
        break;
    case 7:
        list = ds_draw_help(list, hx, hy, 0, 0xF001, res_get(0x8013), res_get(0x8033));
        list = ds_draw_detail(list);
        break;
    case 8:
        list = ds_draw_help(list, hx, hy, 1, 0xF001, res_get(0x8013), res_get(0x8033));
        break;
    }
    if (dsv.popup != 0)
        list = ds_popup_draw(list);
    return list;
}
#endif

#ifdef NON_MATCHING /* lista do parâmetro e do resultado em s trocados; atk/def mascarados no original (como sc_draw_info) */
/* the strip under the panes: the card list's own strip in one-pane modes, else name, icons, level,
   ATK/DEF and the description scrolling sideways */
void *ds_draw_info(void *list, DsCursor *c)
{
    void *r;
    int x;
    int w;
    int x2;
    unsigned short level;

    if (dsv.mode < 4) {
        r = cardlist_build(list, &dsv.cursor.scroll);
        if (c->id == 0)
            return r;
        if (inv_get_flag7(c->id) == 0)
            return r;
        r = ds_draw_flag_icon(r, PT(0x7B, 0x87), 2, res_get(0x800A));
        return r;
    }
    if (c->id == 0)
        return list;
    r = ds_draw_kind_icon(list, 0x146, 0x100, card_get_kind(c->id));
    if (card_is_monster(c->id)) {
        r = ds_draw_type_icon(r, 0x158, 0x100, card_get_type(c->id));
        level = card_get_level(c->id);
        r = ds_draw_level(r, PT(0x16B, 0x103), level, 0xFFFFFFFF, res_get(0x8009));
        r = ds_draw_stat(r, card_get_atk_q(c->id) << 16 | 2, PT(0x187, 0x102), 0xC0A, 0xFFFFFFFF, res_get(0x8009));
        r = ds_draw_stat(r, card_get_def_q(c->id) << 16 | 3, PT(0x1B3, 0x102), 0xC0A, 0xFFFFFFFF, res_get(0x8009));
    } else if (card_get_prop5_hi(c->id) != 0) {
        r = ds_draw_attr_icon(r, 0x158, 0x100, card_get_prop5_hi(c->id));
    }
    font_text_set(r);
    font_set_size(12, 12);
    font_set_color(0xFF00FFFF);
    if (cfg_get_language() != 0)
        font_set_flag4000(1);
    font_draw_text(0x180, 0x3CC0, 0x6C80, card_get_name(c->id));
    if (cfg_get_language() != 0)
        font_set_flag4000(0);
    font_set_color(0xFF00FF00);
    x = font_get_cursor_x() + 0x40;
    font_draw_wtext(x, 0x3CC0, 0x6B00 - x, card_info_fill(c->id));
    r = font_text_clear();
    r = dl_scissor(r, 6, 0x100, 0x13D, 0x110);
    r = dl_enable(r, 2);
    font_text_set(r);
    font_set_color(0xFFFFFFFF);
    if (cfg_get_language() != 0)
        font_set_flag8(0);
    x = 0x180 - dsv.scroll_pos;
    w = dsv.scroll_pos + 0x5500;
    font_draw_text(x, 0x4080, w, card_get_desc(c->id));
    if (dsv.scroll_max > 0) {
        x2 = font_get_cursor_x() + 0x2780;
        if (x2 < w + x)
            font_draw_text(x2, 0x4080, w - (x2 - x), card_get_desc(c->id));
    }
    if (cfg_get_language() != 0)
        font_set_flag8(1);
    r = font_text_clear();
    return dl_disable(r, 2);
}
#endif
