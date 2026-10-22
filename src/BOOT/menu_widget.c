// @range 0x0884A9B4-0x0884C8D0
// @data 0x088EEA58-0x088EEBA0
// @asm 0x0884A9B4-0x0884BCF0
// @asm 0x0884BE28-0x0884C15C
/* Menu widget: a window (menu.c) hosting a scrollable list with a blinking cursor and/or a row of
   OK / YES / NO / CANCEL buttons (buttom_icon1.gim), driven by the pad. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "game.h"
#include "font.h"
#include "menu.h"

unsigned char *syspack_get(void);
void *ehp_find(void *pack, const char *name, int flags);
unsigned int *gfx_get_state(void);
void *gim_set_texture(void *list, void *gim);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
unsigned short region_pick_value(int flag);
int cfg_get_language(void);
void se_play(int id);

static unsigned short s_ok[] = { 'O', 'K', 0 };
static unsigned short s_aceptar[] = { 'A', 'C', 'E', 'P', 'T', 'A', 'R', 0 };
static unsigned short s_cancel[] = { 'C', 'A', 'N', 'C', 'E', 'L', 0 };
static unsigned short s_abbrechen[] = { 'A', 'B', 'B', 'R', 'E', 'C', 'H', 'E', 'N', 0 };
static unsigned short s_annuler[] = { 'A', 'N', 'N', 'U', 'L', 'E', 'R', 0 };
static unsigned short s_annulla[] = { 'A', 'N', 'N', 'U', 'L', 'L', 'A', 0 };
static unsigned short s_cancelar[] = { 'C', 'A', 'N', 'C', 'E', 'L', 'A', 'R', 0 };
static unsigned short s_yes[] = { 'Y', 'E', 'S', 0 };
static unsigned short s_ja[] = { 'J', 'A', 0 };
static unsigned short s_oui[] = { 'O', 'U', 'I', 0 };
static unsigned short s_si_it[] = { 'S', 0xCC, 0 };
static unsigned short s_si_es[] = { 'S', 0xCD, 0 };
static unsigned short s_no[] = { 'N', 'O', 0 };
static unsigned short s_nein[] = { 'N', 'E', 'I', 'N', 0 };
static unsigned short s_non[] = { 'N', 'O', 'N', 0 };
/* [language][button]: 1 OK, 2 YES, 3 NO, 4 CANCEL */
unsigned short *menu_button_labels[6][5] = {
    { 0, s_ok, s_yes, s_no, s_cancel },
    { 0, s_ok, s_yes, s_no, s_cancel },
    { 0, s_ok, s_ja, s_nein, s_abbrechen },
    { 0, s_ok, s_oui, s_non, s_annuler },
    { 0, s_ok, s_si_it, s_no, s_annulla },
    { 0, s_aceptar, s_si_es, s_no, s_cancelar },
};
/* buttons present per button set (bit n = button n) */
unsigned char menu_button_bits[5] = { 0x00, 0x02, 0x12, 0x0C, 0x1C };
char buttom_icon1_gim[] = "buttom_icon1.gim"; /* (sic) the icon sheet of the buttons and arrows */

#ifdef NON_MATCHING
/* menu_open, menu_draw and menu_update: structure follows the original, register allocation does not.
   Kept in asm. */
/* Sizes the window to its list and buttons, loads the icons and opens it. */
void menu_open(MenuWidget *w)
{
    MenuItemDef *d = &w->def;
    unsigned int rows, n, style;
    int cw, ch, min_w;
    unsigned char set, b, i, k;

    w->state = 1;
    menu_init(&w->menu, w->heap);
    menu_add_item(&w->menu, d);
    if (d->auto_size != 0) {
        rows = w->rows;
        cw = 0;
        min_w = 0;
        if (rows == 0)
            rows = w->count;
        n = w->count;
        if (n >= rows)
            n = rows;
        style = w->style;
        if (style != 0) {
            if (style & 1)
                k = w->item_w * n;
            else
                k = w->item_w;
            cw = d->pad_l + d->pad_r + 9 + (style & 1 ? w->item_w * n : w->item_w);
        }
        set = w->buttons;
        if (set != 0) {
            if (set == 4)
                min_w = 0xE3;
            else if (set == 3)
                min_w = 0xA7;
            else if (set == 2)
                min_w = 0xA7;
            else if (set == 1)
                min_w = 0x57;
        }
        if (cw < min_w)
            cw = min_w;
        if (d->w < cw)
            d->w = cw;
        ch = menu_cursor_y(&w->menu, d) - d->y + d->pad_b + 5;
        if (style != 0) {
            if (style & 1)
                ch += w->item_h;
            else
                ch += w->item_h * n;
        }
        if (w->buttons != 0)
            ch += 0x2A;
        if (d->h < ch)
            d->h = ch;
        d->auto_size = 0;
    }
    if (w->cursor_w == 0)
        w->cursor_w = w->item_w;
    if (w->cursor_h == 0)
        w->cursor_h = w->item_h;
    if (w->icon_gim == 0) {
        if (syspack_get() != 0)
            w->icon_gim = ehp_find(syspack_get(), buttom_icon1_gim, 0);
    }
    if (w->buttons != 0) {
        b = w->button;
        k = b;
        for (i = 0; i < 8; i++) {
            if (menu_button_bits[w->buttons] & (1 << (k & 7))) {
                b = (b + i) & 7;
                break;
            }
            k++;
        }
        w->button = b;
    }
    if (w->style != 0) {
        if (w->skip_disabled && (w->disabled >> (w->scroll + w->cursor)) & 1)
            menu_cursor_next(w);
    }
    if (d->open_anim < 1) {
        w->state = 3;
    } else {
        menu_item_open(&w->menu, d);
        w->state = 2;
    }
}

/* Draws the window, its list (with cursor, highlights and arrows) and the buttons; the window
   alpha follows the open/close animation of the item. */
void menu_draw(void **dl, MenuWidget *w)
{
    MenuItemDef *d = &w->def;
    void *start = *dl;
    unsigned int alpha = 0xFF;
    unsigned int c;
    int x0, y0, cx, cy, cw, chh, k, span;
    unsigned int n, rows, style, i, bit;
    unsigned int *p;
    int ix, iy;

    menu_layout(dl, &w->menu);
    if (w->keep_dl)
        *dl = start;
    switch (w->state) {
    case 4:
        alpha = 0xFF;
        break;
    case 3:
        break;
    case 5:
        c = menu_item_counter(w);
        alpha = c < 2 ? 0x80 : 0;
        break;
    case 2:
        c = menu_item_counter(w);
        if (c < 3)
            alpha = 0;
        else if (c == 3)
            alpha = 0x80;
        break;
    default:
        return;
    }
    if (alpha == 0 || d->visible == 0)
        return;
    x0 = d->x + d->pad_l + 4;
    y0 = menu_cursor_y(&w->menu, d) + 1;
    if (w->center) {
        style = w->style;
        if (style != 0) {
            rows = w->rows;
            n = w->count;
            if (rows != 0 && rows < n)
                n = rows;
            if (!(style & 1)) {
                span = w->item_h * n;
                k = d->y + d->h - d->pad_b - 4 - y0;
                if (span < k)
                    y0 += (k - span) / 2;
            } else {
                span = w->item_w * n;
                k = d->x + d->w - d->pad_r - 4 - x0;
                if (span < k)
                    x0 += (k - span) / 2;
            }
        }
    }
    cw = d->x + d->w - x0 - d->pad_r - 5;
    chh = d->y + d->h - y0 - d->pad_b - 5;
    start = *dl;
    if (w->draw_all != 0) {
        *dl = w->draw_all(start, x0, y0, cw, chh);
    } else {
        n = w->count;
        if (w->draw_before != 0)
            *dl = w->draw_before(start, x0, y0, cw, chh);
        rows = w->rows;
        if (rows != 0 && rows < n)
            n = rows;
        ix = x0;
        iy = y0;
        if (w->style != 0) {
            for (i = 0; i < n; i++) {
                bit = 1 << ((i + w->scroll) & 0x1F);
                if (w->selected & bit) {
                    if (w->draw_selected == 0)
                        menu_draw_selected(*dl, w, ix + w->item_x, iy + w->item_y, w->cursor_w, w->cursor_h);
                    else
                        *dl = w->draw_selected(*dl, w, ix + w->item_x, iy + w->item_y, w->cursor_w, w->cursor_h);
                }
                if (w->cursor == i) {
                    if (w->no_cancel) {
                        if (w->draw_cursor == 0)
                            *dl = menu_draw_cursor(*dl, w, ix + w->item_x, iy + w->item_y, w->cursor_w, w->cursor_h);
                        else
                            *dl = w->draw_cursor(*dl, w, ix + w->item_x, iy + w->item_y, w->cursor_w, w->cursor_h);
                    } else if (w->draw_selected == 0) {
                        menu_draw_selected(*dl, w, ix + w->item_x, iy + w->item_y, w->cursor_w, w->cursor_h);
                    } else {
                        *dl = w->draw_selected(*dl, w, ix + w->item_x, iy + w->item_y, w->cursor_w, w->cursor_h);
                    }
                }
                if (w->draw_item != 0)
                    *dl = w->draw_item(*dl, (i + w->scroll) & 0xFFFF, ix, iy);
                if (w->selected & bit) {
                    if (w->draw_selected_post != 0)
                        *dl = w->draw_selected_post(*dl, w, ix + w->item_x, iy + w->item_y, w->cursor_w, w->cursor_h);
                }
                if (w->cursor == i) {
                    if (w->no_cancel) {
                        if (w->draw_cursor_post != 0)
                            *dl = w->draw_cursor_post(*dl, w, ix + w->item_x, iy + w->item_y, w->cursor_w, w->cursor_h);
                    } else if (w->draw_selected_post != 0) {
                        *dl = w->draw_selected_post(*dl, w, ix + w->item_x, iy + w->item_y, w->cursor_w, w->cursor_h);
                    }
                }
                if ((w->style & 1))
                    ix += w->item_w;
                else
                    iy += w->item_h;
            }
        }
        if (w->scroll != 0) {
            *dl = gim_set_texture(*dl, w->icon_gim);
            if ((w->style & 1))
                *dl = menu_draw_arrow(*dl, x0 - w->arrow_off, y0 + (w->item_h >> 1), 2);
            else
                *dl = menu_draw_arrow(*dl, x0 + (w->item_w >> 1), y0 - w->arrow_off, 1);
        }
        if (w->rows != 0) {
            if (w->scroll + w->rows < w->count) {
                *dl = gim_set_texture(*dl, w->icon_gim);
                if ((w->style & 1))
                    *dl = menu_draw_arrow(*dl, ix + w->arrow_off, y0 + (w->item_h >> 1), 3);
                else
                    *dl = menu_draw_arrow(*dl, x0 + (w->item_w >> 1), iy + w->arrow_off, 0);
            }
        }
        switch (w->buttons) {
        case 4:
            *dl = menu_draw_button(*dl, w, d->x + d->w / 2 - 0x60, iy + 10, 2, w->button == 2);
            *dl = menu_draw_button(*dl, w, d->x + (d->w - 0x3C) / 2, iy + 10, 3, w->button == 3);
            *dl = menu_draw_button(*dl, w, d->x + d->w / 2 + 0x24, iy + 10, 4, w->button == 4);
            break;
        case 3:
            *dl = menu_draw_button(*dl, w, d->x + d->w / 2 - 0x46, iy + 10, 2, w->button == 2);
            *dl = menu_draw_button(*dl, w, d->x + d->w / 2 + 10, iy + 10, 3, w->button == 3);
            break;
        case 2:
            *dl = menu_draw_button(*dl, w, d->x + d->w / 2 - 0x46, iy + 10, 1, w->button == 1);
            *dl = menu_draw_button(*dl, w, d->x + d->w / 2 + 10, iy + 10, 4, w->button == 4);
            break;
        case 1:
            *dl = menu_draw_button(*dl, w, d->x + (d->w - 0x3C) / 2, iy + 10, 1, w->button == 1);
            break;
        }
        if (w->draw_after != 0)
            *dl = w->draw_after(*dl, x0, y0, cw, chh);
    }
    /* fade: scale every ambient alpha command (0x58) emitted above */
    if (alpha < 0xFF && (unsigned int *)start < (unsigned int *)*dl) {
        for (p = start; p < (unsigned int *)*dl; p++) {
            if (*p >> 24 == 0x58)
                *p = (((*p & 0xFF) * alpha) / 0xFF & 0xFF) | 0x58000000;
        }
    }
}

/* Advances the widget one frame. Returns -3 when idle, -2 when cancelled, the selected list index
   or button when accepted, -1 otherwise. */
int menu_update(MenuWidget *w)
{
    PadState *pad = w->pad;
    unsigned int pressed, accept, cancel, style;
    unsigned char bits, b, nb, i;
    int k;

    if (pad == 0)
        pad = pad_get();
    switch (w->state) {
    case 4:
        if (w->blink++ > 8) {
            w->blink = 0;
            if (w->keep_open) {
                w->state = 3;
                if (w->cancelled)
                    return -2;
                if (w->style != 0)
                    return w->scroll + w->cursor;
                if (w->buttons != 0)
                    return w->button;
                return -1;
            }
            menu_finish(w);
        }
        return -1;
    case 3:
        break;
    case 6:
        break;
    case 5:
        if (menu_item_is_open(&w->menu, &w->def) == 0)
            return -1;
        w->state = 6;
        break;
    case 2:
        if (menu_item_is_open(&w->menu, &w->def) != 0)
            w->state = 3;
        return -1;
    default:
        return -3;
    }
    if (w->state == 6 || w->state == 5) {
        if (w->cancelled)
            return -2;
        if (w->style != 0)
            return w->scroll + w->cursor;
        if (w->buttons != 0)
            return w->button;
    }
    if (w->cancelled)
        return -2;
    if (w->accepted) {
        if (w->style != 0)
            return w->scroll + w->cursor;
        if (w->buttons != 0)
            return w->button;
    } else if (w->no_input) {
        return -1;
    }
    pressed = pad->b.dir_rep;
    accept = region_pick_value(0);
    cancel = region_pick_value(1);
    if (pressed & cancel & 0xFFFF) {
        if (w->cancel_blocked) {
            se_play(0x6215);
            return -1;
        }
        se_play(0x6214);
        w->cancelled = 1;
        if (!(w->cancel_resets)) {
            if (!(w->keep_open)) {
                menu_finish(w);
                return -1;
            }
            return -2;
        }
        if (w->style != 0) {
            if (w->rows == 0) {
                w->cursor = w->initial;
            } else if (w->initial < w->scroll + w->rows) {
                w->cursor = w->initial;
            } else {
                w->cursor = w->rows - 1;
                w->scroll = w->initial - w->cursor;
            }
            w->state = 4;
        }
        if (w->buttons != 0) {
            w->button = w->initial & 0xF;
            w->state = 4;
        }
        return -1;
    }
    if (pressed & accept & 0xFFFF) {
        if (w->accept_blocked)
            return -1;
        if (w->style != 0 && (w->disabled >> (w->scroll + w->cursor)) & 1) {
            se_play(0x6215);
            return -1;
        }
        se_play(0x6213);
        w->accepted = 1;
        w->state = 4;
        return -1;
    }
    style = w->style;
    if (style != 0) {
        if (style & 1) {
            if (pad->b.trig & 0x80) {
                menu_cursor_prev(w);
                se_play(0x6212);
            }
            if (pad->b.trig & 0x20) {
                menu_cursor_next(w);
                se_play(0x6212);
            }
        } else {
            if (pad->b.trig & 0x10) {
                menu_cursor_prev(w);
                se_play(0x6212);
            }
            if (pad->b.trig & 0x40) {
                menu_cursor_next(w);
                se_play(0x6212);
            }
        }
        return -1;
    }
    if (w->buttons == 0)
        return -1;
    bits = menu_button_bits[w->buttons];
    b = w->button;
    nb = b;
    if (pad->b.trig & 0x80) {
        for (i = 1; i < 8; i++) {
            if (bits & (1 << ((b + 8 - i) & 7))) {
                nb = (b + 8 - i) & 7;
                break;
            }
        }
        if (b != nb) {
            se_play(0x6212);
            w->button = nb;
        }
    }
    if (pad->b.trig & 0x20) {
        k = nb;
        for (i = 1; i < 8; i++) {
            k++;
            if (bits & (1 << (k & 7))) {
                nb = (nb + i) & 7;
                break;
            }
        }
        if (w->button != nb) {
            se_play(0x6212);
            w->button = nb;
        }
    }
    return -1;
}
#endif

void menu_set_button(MenuWidget *w, unsigned char b)
{
    w->button = b;
    w->accepted = 1;
    w->state = 4;
}

/* Moves the cursor (and scroll) to list entry `index` and accepts it. */
void menu_set_cursor(MenuWidget *w, int index)
{
    int rows = w->rows;

    if (rows > 0 && index >= rows) {
        w->cursor = rows - 1;
        w->scroll = index - w->cursor;
    } else {
        w->cursor = index;
    }
    w->accepted = 1;
    w->state = 4;
}

void menu_finish(MenuWidget *w)
{
    menu_item_close(&w->menu, &w->def);
    if (w->def.close_anim < 1)
        w->state = 6;
    else
        w->state = 5;
}

void menu_reset(MenuWidget *w)
{
    menu_remove_item(&w->menu, &w->def);
    menu_free_items(&w->menu);
    w->state = 0;
}

#ifdef NON_MATCHING
/* Kept in asm (register allocation). */
/* One button (60 x 24 from buttom_icon1.gim) with its localized label; `lit` draws the
   highlighted frame and, while blinking, the flashing overlay. */
void *menu_draw_button(void *dl, MenuWidget *w, int x, int y, int which, int lit)
{
    int show = lit;
    unsigned short *label;
    unsigned int t;
    int u, v, tw, th, dx, dy;
    unsigned char sx;

    if (w->state == 4) {
        if (w->blink & 2) {
            dl = gim_set_texture(dl, w->icon_gim);
            goto frame;
        }
        if (lit == 1)
            show = 0;
    }
    dl = gim_set_texture(dl, w->icon_gim);
frame:
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = draw_set_offsets(dl, 60, 24, 60, 24);
    if (show == 0) {
        dl = draw_sprite(dl, x, y, 0, 0, 0, 0, 0);
    } else {
        if (which < 3) {
            if (which == 0) {
                u = 60;
                v = 24;
            } else {
                u = 60;
                v = 0;
            }
        } else {
            u = 0;
            v = 24;
        }
        dl = draw_sprite(dl, x, y, 0, u, v, 0, 0);
        dl = draw_sprite(dl, x, y, 0, 0, 48, 0, 0);
    }
    if (lit != 0 && w->state != 4) {
        t = *gfx_get_state() % 33;
        if (t < 17)
            t <<= 7;
        else
            t = (33 - t) * 0x80;
        t = (t & 0xFFF) >> 4;
        dl = dl_ambient_color(dl, t | t << 8 | t << 24 | t << 16);
        dl = dl_blend_func(dl, 0, 2, 10, 0xFFFFFFFF, 0xFFFFFFFF);
        dl = draw_set_offsets(dl, 60, 24, 60, 24);
        dl = draw_sprite(dl, x, y, 0, 60, 48, 0, 0);
        dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
    }
    label = menu_button_labels[cfg_get_language()][which];
    if (label != 0) {
        font_set_size(10, 12);
        font_set_color(0xFF000000);
        sx = font_measure_wtext(0xF00, label);
        font_set_size(sx, 12);
        tw = font_measure_text(label);
        font_text_set(dl);
        th = font_line_height();
        dy = (24 - (th >> 6)) / 2;
        dx = (60 - (tw >> 6)) / 2;
        x += dx;
        font_draw_text(x * 64, (y + dy + 1) * 64, (480 - x) * 64, label);
        dl = font_text_clear();
    }
    return dl;
}

#endif

/* The list cursor: a pulsing light-blue box (menu_draw_box); hidden on the off phase of the
   accept blink. */
void *menu_draw_cursor(void *dl, MenuWidget *w, int x, int y, int cw, int ch)
{
    unsigned int color = 0xA00080FF;
    unsigned int t;

    if (w->state == 4) {
        if (!((w->blink >> 1) & 1))
            return dl;
    } else {
        t = *gfx_get_state() % 33;
        if (t > 16)
            t = 33 - t;
        color = (unsigned char)(((t << 7) >> 4) + 0x7F) << 24 | 0x80FF;
    }
    dl = gim_set_texture(dl, w->menu.window_gim);
    dl = dl_ambient_color(dl, color);
    return menu_draw_box(dl, x, y, cw, ch);
}

/* Highlight box of a selected entry (steady color). */
void menu_draw_selected(void *dl, MenuWidget *w, int x, int y, int cw, int ch)
{
    dl = gim_set_texture(dl, w->menu.window_gim);
    dl = dl_ambient_color(dl, 0x803A768D);
    menu_draw_box(dl, x, y, cw, ch);
}

/* 9-slice box from the 12 x 12 corner pieces at (0..12, 112..124) of the window texture; returns
   the list (menu_draw_cursor passes it on). */
void *menu_draw_box(void *dl, int x, int y, int w, int h)
{
    int y1, x1;

    dl = dl_set_tex_filter(dl, 0, 0);
    dl = draw_set_offsets(dl, 4, 4, 4, 4);
    dl = draw_sprite(dl, x, y, 0, 0, 0x70, 0, 0);
    x1 = x + w - 4;
    dl = draw_sprite(dl, x1, y, 0, 8, 0x70, 0, 0);
    y1 = y + h - 4;
    dl = draw_sprite(dl, x, y1, 0, 0, 0x78, 0, 0);
    dl = draw_sprite(dl, x1, y1, 0, 8, 0x78, 0, 0);
    if (w > 8) {
        dl = draw_set_offsets(dl, 4, 4, w - 8, 4);
        dl = draw_sprite(dl, x + 4, y, 0, 4, 0x70, 0, 0);
        dl = draw_sprite(dl, x + 4, y1, 0, 4, 0x78, 0, 0);
    }
    if (h > 8) {
        dl = draw_set_offsets(dl, 4, 4, 4, h - 8);
        dl = draw_sprite(dl, x, y + 4, 0, 0, 0x74, 0, 0);
        dl = draw_sprite(dl, x1, y + 4, 0, 8, 0x74, 0, 0);
    }
    if (w > 8 && h > 8) {
        dl = draw_set_offsets(dl, 4, 4, w - 8, h - 8);
        dl = draw_sprite(dl, x + 4, y + 4, 0, 4, 0x74, 0, 0);
    }
    return dl;
}

/* Animation counter of the widget's window item (0 when not found). */
int menu_item_counter(MenuWidget *w)
{
    int i;

    for (i = 0; i < 16; i++) {
        if (w->menu.items[i]->def == &w->def)
            return w->menu.items[i]->counter;
    }
    return 0;
}

void menu_cursor_prev(MenuWidget *w)
{
    int i;

    if (w->skip_disabled) {
        for (i = 0; i < (int)w->count; i++) {
            menu_cursor_prev1(w);
            if (!((w->disabled >> (w->scroll + w->cursor)) & 1))
                return;
        }
    } else {
        menu_cursor_prev1(w);
    }
}

/* One entry up: cursor, then scroll, then (unless no_wrap) to the last entry. */
void menu_cursor_prev1(MenuWidget *w)
{
    int rows = w->rows;
    int cur = w->cursor;
    int count = w->count;
    int scroll = w->scroll;
    int vis = rows;

    if (count < rows)
        vis = count;
    if (cur == 0) {
        if (scroll == 0) {
            if (!w->no_wrap) {
                if (rows == 0) {
                    cur = count - 1;
                } else {
                    scroll = count - vis;
                    cur = vis - 1;
                }
            }
        } else {
            scroll--;
        }
    } else {
        cur--;
    }
    w->cursor = cur;
    w->scroll = scroll;
}

void menu_cursor_next(MenuWidget *w)
{
    int i;

    if (w->skip_disabled) {
        for (i = 0; i < (int)w->count; i++) {
            menu_cursor_next1(w);
            if (!((w->disabled >> (w->scroll + w->cursor)) & 1))
                return;
        }
    } else {
        menu_cursor_next1(w);
    }
}

/* One entry down: cursor, then scroll, then (unless no_wrap) back to the first entry. */
void menu_cursor_next1(MenuWidget *w)
{
    int cur = w->cursor;
    int rows = w->rows;
    int scroll = w->scroll;
    int count = w->count;
    int vis = rows;

    if (count < rows)
        vis = count;
    if (rows == 0) {
        if (w->no_wrap && cur >= count - 1) {
            cur = count - 1;
        } else if (count == 0) {
            cur = 0;
        } else {
            cur = (cur + 1) % count;
        }
    } else if (cur >= vis - 1) {
        if (scroll >= count - vis) {
            if (!w->no_wrap) {
                cur = 0;
                scroll = 0;
            }
        } else {
            scroll++;
        }
    } else {
        cur++;
    }
    w->cursor = cur;
    w->scroll = scroll;
}

/* Scroll arrow (12 x 12 from the icon texture row at v = 72), blinking with the frame counter. */
void *menu_draw_arrow(void *dl, int x, int y, int dir)
{
    if (((*gfx_get_state() >> 3) & 3) == 0)
        return dl;
    switch (dir) {
    case 2:
        x -= 12;
        y -= 6;
        break;
    case 3:
        y -= 6;
        break;
    case 1:
        x -= 6;
        y -= 12;
        break;
    case 0:
        x -= 6;
        break;
    }
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = draw_set_offsets(dl, 12, 12, 12, 12);
    return draw_sprite(dl, x, y, 0, dir * 12, 0x48, 0, 0);
}
