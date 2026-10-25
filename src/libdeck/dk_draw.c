// @range 0x0880DBD0-0x0880E4D0
// @rodata 0x088225B8-0x088225D8
// @data 0x08822ED8-0x08822EE4
/* Frame drawing: the draw callback (clear, background, card info, panes, popups, fade or the system
   keyboard), the popup layer over the panes (focus frame, help window placed away from the cursor,
   the popup of each state), and the background: bg.ehf, d_window0.ehf, the deck name and owned
   card count, and the row lines of the list panes. */
#include "libdeck.h"

typedef struct DkLineVtx {             /* dl_draw_array vertex: colour + position, 12 bytes */
    unsigned int color;
    unsigned short x, y, z;
    unsigned short pad;
} DkLineVtx;

void *dl_clear_color(void *list, unsigned int color);
void *dl_clear_depth(void *list, unsigned int z);
void *dl_clear_stencil(void *list, unsigned int s);
void *dl_clear(void *list, unsigned int flags);
void font_text_set(void *list);
void *font_text_clear(void);

extern unsigned short dk_owned_kinds;   /* 0x095D2438 card kinds owned */
void *dk_draw_headers(void *list);
void *dk_draw_info(void *list, DkCursor *c);
void *dk_draw_panes(void *list);
void *dk_draw_overlay(void *list);
void *dk_draw_bg_ehf(void *list);
void *dk_draw_window(void *list);
void *dk_draw_title(void *list);
void *dk_draw_grid_lines(void *list, unsigned short mode);
void dk_line(DkLineVtx *v, unsigned short x, unsigned short y, unsigned short w);
void *dk_draw_bg(void *list);
void *dk_draw_attr_counts(void *list);
void *dk_draw_attr_bars(void *list);
void *dk_flight_draw(void *list);
void *dk_action_draw(void *list);
void *dk_sort_menu_draw(void *list);
void *dk_fmenu_draw(void *list);
void *func_08819F80(void *list);
void *func_088107C0(void *list);
void *func_0880AF08(void *list);
void *dk_draw_frame(void *list, int x, int y, int w, int h, void *gim, int style);
void *dk_draw_help(void *list, int x, int y, int alt, unsigned int flags, void *gim, void *strtbl);
void *dk_draw_num_text(void *list, int x, int y, unsigned int value, int w, int ndig, int pad);

/* sys_state_set_callbacks draw callback */
void dk_draw_scene(void)
{
    void *list;

    list = dl_select_layer(0, 0);
    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (dkv.loading)
        return;
    list = dl_select_layer(4, 0);
    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 1);
    list = dk_draw_bg(list);
    list = dk_draw_info(list, &dkv.cursor);
    list = dk_draw_panes(list);
    list = dk_draw_overlay(list);
    dl_set_list(list);
    if (dkv.busy) {
        dk_name_input_update();
    } else {
        list = dl_select_layer(5, 0);
        list = dl_disable(list, 1);
        fade_apply(&list);
        dl_set_list(list);
    }
}

/* focus frame and help window (help position away from the cursor), then the state's popup */
void *dk_draw_overlay(void *list)
{
    DkRect *f = &dk_focus;
    int hx;
    int hy;

    switch (dk_state) {
    case 6:
    case 7:
        if (dkv.mode > 1) {
            if (dkv.mode < 4) {
                hx = dkv.pane[0].lay.x + 2;
                if (f->y + f->h / 2 > 136)
                    hy = 54;
                else
                    hy = 162;
            } else {
                if (dkv.side == 0)
                    hx = 254;
                else
                    hx = 14;
                hy = 112;
            }
        } else {
            /* the view shifted by side panes: pane[0] is the cursor's pane */
            DkView *v = (DkView *)((char *)&dkv + dkv.side * sizeof(DkPane));

            if (v->pane[0].list_mode == 1)
                hx = 208;
            else if (f->x + f->w > v->pane[0].lay.x + v->pane[0].lay.w / 2)
                hx = v->pane[0].lay.x + 2;
            else
                hx = 208;
            if (f->y + f->h / 2 > 136)
                hy = 54;
            else
                hy = 162;
        }
    case 0:
        list = dk_draw_frame(list, f->x, f->y, f->w, f->h, dk_res_get(0x8006), *(int *)gfx_get_state());
    }
    switch (dk_state) {
    case 0:
        break;
    case 1:
        list = dk_flight_draw(list);
        break;
    case 2:
        list = dk_action_draw(list);
        break;
    case 3:
        list = dk_sort_menu_draw(list);
        break;
    case 4:
        list = dk_fmenu_draw(list);
        break;
    case 5:
        list = func_08819F80(list);
        break;
    case 6:
        list = dk_draw_help(list, hx, hy, 0, 0xF001, dk_res_get(0x8013), dk_res_get(0x8033));
        list = func_088107C0(list);
        break;
    case 7:
        list = dk_draw_help(list, hx, hy, 1, 0xF001, dk_res_get(0x8013), dk_res_get(0x8033));
        break;
    }
    if (dkv.popup != 0)
        list = func_0880AF08(list);
    return list;
}

void *dk_draw_bg(void *list)
{
    list = dk_draw_bg_ehf(list);
    list = dk_draw_window(list);
    list = dk_draw_grid_lines(list, dkv.mode);
    list = dk_draw_title(list);
    list = dk_draw_headers(list);
    list = dk_draw_attr_counts(list);
    return dk_draw_attr_bars(list);
}

/* animated background (bg.ehf) */
void *dk_draw_bg_ehf(void *list)
{
    void *e = dk_res_get(0x8021);
    unsigned int opts[24];

    if (e == 0)
        return list;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()[0];
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, opts);
    list = gfx_end_2d(list);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    return dl_blend_func(list, 0, 2, 3, 0, 0);
}

/* the editor window (d_window0.ehf) */
void *dk_draw_window(void *list)
{
    void *e = dk_res_get(0x8022);
    unsigned int opts[24];

    if (e == 0)
        return list;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()[0];
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, opts);
    return gfx_end_2d(list);
}

short dk_title_x[6] = { 0x19D, 0x1AA, 0x1AA, 0x1AA, 0x1AA, 0x1AA };

/* deck name, the "owned" label (strTbl.bin 107) and the count of cards owned */
void *dk_draw_title(void *list)
{
    font_text_set(list);
    font_set_size(12, 12);
    font_set_color(0xFFFFFFFF);
    font_draw_wtext(0x2980, 320, 0x1BC0, (unsigned short *)dk_deck_name);
    font_set_size(11, 11);
    font_draw_wtext(0x7040, 0x300, 0x740, table_get_entry(dk_res_get(0x8033), 107));
    list = font_text_clear();
    font_set_size(10, 12);
    font_set_color(0xFF000000);
    return dk_draw_num_text(list, dk_title_x[cfg_get_language()], 5, dk_owned_kinds, 8, 4, 0);
}

/* separator lines under the rows of the list panes */
void *dk_draw_grid_lines(void *list, unsigned short mode)
{
    DkLineVtx *v = dl_alloc_inline(&list, 0x240);
    DkLineVtx *p = v;
    unsigned short n = 0;
    unsigned short y;
    int i;

    if (mode == 0) {
        for (y = 41, i = 0; i < 7; i++, p += 4) {
            dk_line(p, 151, y, 317);
            y += 30;
            n += 4;
        }
    }
    if (mode == 4 || mode == 6) {
        for (y = 41, i = 0; i < 6; i++, p += 4) {
            dk_line(p, 3, y, 225);
            y += 30;
            n += 4;
        }
    }
    if (mode == 4 || mode == 7) {
        for (y = 41, i = 0; i < 6; i++, p += 4) {
            dk_line(p, 243, y, 225);
            y += 30;
            n += 4;
        }
    }
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_disable(list, 9);
    list = dl_draw_array(list, 1, 0x80011C, n, 0, v);
    list = dl_enable(list, 9);
    return list;
}

/* a dark line w wide at (x, y) with a lighter one under it */
void dk_line(DkLineVtx *v, unsigned short x, unsigned short y, unsigned short w)
{
    v[0].x = x;
    v[0].y = y;
    v[0].z = 0;
    v[0].color = 0x33000000;
    v[1].x = x + w;
    v[1].y = y;
    v[1].z = 0;
    v[1].color = 0x33000000;
    v[2].x = x;
    v[2].y = y + 1;
    v[2].z = 0;
    v[2].color = 0x19000000;
    v[3].x = x + w;
    v[3].y = y + 1;
    v[3].z = 0;
    v[3].color = 0x19000000;
}
