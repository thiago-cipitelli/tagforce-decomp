// @range 0x08807630-0x08808040
// @asm 0x08807858-0x08807B44
/* Frame drawing: the export called once per frame (clear, background, panes, popups, fade), the
   popup layer over the panes (focus frame, help window, the popup of each state), and the background:
   bg.ehf, the window of the kind, the row lines of the panes and the title. */
#include "libselect_card.h"

typedef struct ScLineVtx {             /* dl_draw_array vertex: colour + position, 12 bytes */
    unsigned int color;
    unsigned short x, y, z;
    unsigned short pad;
} ScLineVtx;

void *sc_draw_bg(void *list);
void *sc_draw_cursor(void *list, void *cursor);
void *sc_draw_panes(void *list);
void *sc_draw_overlay(void *list);
void *sc_draw_bg_ehf(void *list);
void *sc_draw_window(void *list);
void *sc_draw_grid_lines(void *list, unsigned short mode);
void sc_line(ScLineVtx *v, unsigned short x, unsigned short y, unsigned short w);
void *sc_draw_title(void *list);
void *sc_draw_headers(void *list);
int sc_popup_done(int id);
void *sc_draw_sort_menu(void *list);
void *sc_draw_filter_menu(void *list);
void *sc_draw_search(void *list);
void *sc_draw_confirm(void *list);
void *sc_draw_result(void *list);
void *sc_draw_detail(void *list);
void *sc_draw_popup(void *list);

/* export: once per frame */
void libselect_card_08807630(void)
{
    void *list;

    switch (sc.state) {
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
        if (scv.info) {
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
        list = sc_draw_bg(list);
        list = sc_draw_cursor(list, &scv.cursor);
        list = sc_draw_panes(list);
        list = sc_draw_overlay(list);
        fade_apply(&list);
        dl_set_list(list);
        if (scv.busy && sc_popup_done(4))
            scv.busy = 0;
        break;
    }
}

#ifdef NON_MATCHING /* o original calcula a base de scv.pane[lado] uma vez e a reaproveita nos dois blocos; o mwcc atual a recalcula */
/* focus frame and help window (help position away from the cursor), then the state's popup */
void *sc_draw_overlay(void *list)
{
    ScRect *f = &sc.focus;
    int hx;
    int hy;
    void *gim;

    switch (sc.state) {
    case 7:
    case 8:
        if (scv.mode > 1) {
            if (scv.mode < 4) {
                hx = scv.pane[0].lay.x + 2;
                if (f->y + f->h / 2 > 0x88)
                    hy = 0x36;
                else
                    hy = 0xA2;
            } else {
                if (scv.side == 0)
                    hx = 0xFE;
                else
                    hx = 0x0E;
                hy = 0x70;
            }
        } else {
            if (scv.pane[scv.side].list_mode == 1)
                hx = 0xD0;
            else if (f->x + f->w > scv.pane[scv.side].lay.x + scv.pane[scv.side].lay.w / 2)
                hx = scv.pane[scv.side].lay.x + 2;
            else
                hx = 0xD0;
            if (f->y + f->h / 2 > 0x88)
                hy = 0x36;
            else
                hy = 0xA2;
        }
    case 1:
        gim = res_get(0x8006);
        list = sc_draw_frame(list, f->x, f->y, f->w, f->h, gim, gfx_get_state()[0]);
    }
    switch (sc.state) {
    case 0:
    case 1:
        break;
    case 2:
        list = sc_draw_sort_menu(list);
        break;
    case 3:
        list = sc_draw_filter_menu(list);
        break;
    case 4:
        list = sc_draw_search(list);
        break;
    case 5:
        list = sc_draw_confirm(list);
        break;
    case 6:
        list = sc_draw_result(list);
        break;
    case 7:
        list = sc_draw_help(list, hx, hy, 0, 0xF001, res_get(0x8013), res_get(0x8033));
        list = sc_draw_detail(list);
        break;
    case 8:
        list = sc_draw_help(list, hx, hy, 1, 0xF001, res_get(0x8013), res_get(0x8033));
        break;
    }
    if (scv.popup != 0)
        list = sc_draw_popup(list);
    return list;
}
#endif

void *sc_draw_bg(void *list)
{
    list = sc_draw_bg_ehf(list);
    list = sc_draw_window(list);
    list = sc_draw_grid_lines(list, scv.mode);
    list = sc_draw_title(list);
    return sc_draw_headers(list);
}

/* animated background (bg.ehf) */
void *sc_draw_bg_ehf(void *list)
{
    void *e = res_get(0x8021);
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

/* the window of the kind (window_<l>.ehf or t_window.ehf) */
void *sc_draw_window(void *list)
{
    void *e = 0;
    unsigned int opts[24];

    switch (sc.kind) {
    case 0:
        e = res_get(0x1010);
        break;
    case 1:
        e = res_get(0x2010);
        break;
    case 2:
        e = res_get(0x4010);
        break;
    }
    if (e == 0)
        return list;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()[0];
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, opts);
    return gfx_end_2d(list);
}

/* separator lines under the rows of the list panes */
void *sc_draw_grid_lines(void *list, unsigned short mode)
{
    ScLineVtx *v = dl_alloc_inline(&list, 0x240);
    ScLineVtx *p = v;
    unsigned short n = 0;
    unsigned short y;
    int i;

    if (mode == 0) {
        for (y = 41, i = 0; i < 7; i++, p += 4) {
            sc_line(p, 151, y, 317);
            y += 30;
            n += 4;
        }
    }
    if (mode == 4 || mode == 6) {
        for (y = 41, i = 0; i < 6; i++, p += 4) {
            sc_line(p, 3, y, 225);
            y += 30;
            n += 4;
        }
    }
    if (mode == 4 || mode == 7) {
        for (y = 41, i = 0; i < 6; i++, p += 4) {
            sc_line(p, 243, y, 225);
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
void sc_line(ScLineVtx *v, unsigned short x, unsigned short y, unsigned short w)
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

/* title of the kind (strTbl.bin 0x91..0x93) */
void *sc_draw_title(void *list)
{
    unsigned short x;

    if (scv.mode < 4)
        x = 0x99;
    else
        x = 5;
    font_text_set(list);
    font_set_size(12, 12);
    font_set_color(0xFFFFFFFF);
    switch (sc.kind) {
    case 0:
        font_draw_wtext(x << 6, 320, 0x5080, table_get_entry(res_get(0x8033), 0x91));
        break;
    case 1:
        font_draw_wtext(x << 6, 320, 0x5080, table_get_entry(res_get(0x8033), 0x92));
        break;
    case 2:
        font_draw_wtext(x << 6, 320, 0x3D80, table_get_entry(res_get(0x8033), 0x93));
        break;
    }
    return font_text_clear();
}
