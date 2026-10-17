// @range 0x08843D78-0x08845AC0
// @rodata 0x08875718-0x08875734
// @asm 0x08843D78-0x088454DC
// @asm 0x088454DC-0x088455B0
/* Window rendering for the menu layer: open/close animation state machine, 9-slice frame built
   from window01.gim pieces into the layer-4 display list, title bar and text. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "game.h"
#include "menu.h"

void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
void *dl_alloc_inline(void *list, int bytes);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void *dl_clear_stencil(void *list, int s);
void *dl_clear(void *list, int flags);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_alpha_test(void *list, int func, int ref, int mask);
void *dl_depth_func(void *list, int func);
void *dl_depth_mask(void *list, int mask);
void *dl_tex_mode(void *list, int pixfmt, int maxmips, int a2, int swizzle);
void *dl_tex_func(void *list, int func, int rgba);
void *dl_tex_wrap(void *list, int u, int v);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *gim_set_texture();              /* (list, gim): called with one argument in the original when the item has its own */
void font_set_size(int w, int h);
int font_get_flag4(void);
int font_get_flag8000(void);
void font_set_flag4(int v);
void font_set_flag8000(int v);
void font_set_color(unsigned int c);
int font_measure_wtext2(int maxw, int maxh, unsigned short *text);
int font_measure_text(unsigned short *text);
int font_get_height(void);
void font_text_set(void *list);
void *font_text_clear(void);
void font_draw_wtext_ex(int x, int y, int w, int h, unsigned short *text, unsigned int color);
int font_draw_text(int x, int y, int w, unsigned short *text);
float truncf(float f);
void color_unpack(float *out, unsigned int rgba);
unsigned int color_pack(float *v);
void vec4_lerp2(float t, float *out, float *a, float *b);

/* 8-bit tab/edge texel coordinates of the variable-height side piece (.data) */
extern unsigned short wside_u;         /* 0x088EE8BC */
extern short wside_v;                  /* 0x088EE8BE */
extern short wside_w;                  /* 0x088EE8C0 */
extern short wside_h;                  /* 0x088EE8C2 */

#ifdef NON_MATCHING
/* The four functions below have the original's structure but mwcc allocates registers and the
   stack differently (menu_item_build/menu_item_build_frame: 2.4 KB each) or unrolls a counting
   loop the original does not (menu_item_sprite_count). They stay in asm; this C is for the port.
   The .rodata jump tables of the asm versions live at 0x088756D8..0x08875740. */
/* State machine of one item; (re)builds its frame when the definition changed. */
void menu_item_build(void **dl, MenuItem *it, Menu *m)
{
    MenuItemDef *d = it->def;
    int st = it->state;
    int c;
    int f4, f8;
    int cw, ch, mw, mh;
    unsigned int sz;
    short v;
    int tw, th;

    if (st == 3) {
        if (d->close_anim == 2) {
            c = it->counter;
            if (c == 4) {
                it->state = 4;
            } else if (c == 3) {
                d->visible = 0;
                d->y = it->saved_y;
                d->show_title = it->saved_show_title;
                d->text = it->saved_text;
                d->title = it->saved_title;
            } else if (c == 2) {
                it->saved_y = d->y;
                d->y = (short)(int)((float)d->y + (float)d->h * 0.25f);
                d->anchor = 0;
                d->scale_y = 0.5f;
                it->saved_show_title = d->show_title;
                d->show_title = 0;
                it->ghost = 1;
                it->x2 = d->x;
                it->y2 = (short)(int)((float)it->saved_y + (float)d->h * 0.125f);
                it->scale_x2 = 1.0f;
                it->scale_y2 = 0.75f;
                it->color2 = color_lerp(0.5f, d->color & 0xFFFFFF, d->color);
            } else if (c == 1) {
                it->saved_text = d->text;
                it->saved_title = d->title;
                d->text = 0;
                d->title = 0;
                d->text_color = it->saved_text_color;
                d->title_color = it->saved_title_color;
            } else if (c == 0) {
                it->saved_text_color = d->text_color;
                d->text_color = color_lerp(0.5f, d->text_color & 0xFFFFFF, d->text_color);
                it->saved_title_color = d->title_color;
                d->title_color = color_lerp(0.5f, d->title_color & 0xFFFFFF, d->title_color);
            }
            it->counter++;
        } else if (d->close_anim == 1) {
            c = it->counter;
            if (c == 4) {
                it->state = 4;
            } else if (c == 3) {
                d->visible = 0;
                d->show_title = it->saved_show_title;
                d->text = it->saved_text;
                d->title = it->saved_title;
            } else if (c == 2) {
                d->anchor = 0;
                d->scale_y = 0.5f;
                it->saved_show_title = d->show_title;
                d->show_title = 0;
                it->ghost = 1;
                it->x2 = d->x;
                it->y2 = d->y;
                it->scale_x2 = 1.0f;
                it->scale_y2 = 0.75f;
                it->color2 = color_lerp(0.5f, d->color & 0xFFFFFF, d->color);
            } else if (c == 1) {
                it->saved_text = d->text;
                it->saved_title = d->title;
                d->text = 0;
                d->title = 0;
                d->text_color = it->saved_text_color;
                d->title_color = it->saved_title_color;
            } else if (c == 0) {
                it->saved_text_color = d->text_color;
                d->text_color = color_lerp(0.5f, d->text_color & 0xFFFFFF, d->text_color);
                it->saved_title_color = d->title_color;
                d->title_color = color_lerp(0.5f, d->title_color & 0xFFFFFF, d->title_color);
            }
            it->counter++;
        } else {
            it->counter++;
        }
    } else if (st == 1) {
        if (d->open_anim == 2) {
            c = it->counter;
            if (c == 4) {
                it->state = 2;
            } else if (c == 3) {
                d->text_color = it->saved_text_color;
                d->title_color = it->saved_title_color;
            } else if (c == 2) {
                d->text = it->saved_text;
                d->title = it->saved_title;
                it->saved_text_color = d->text_color;
                d->text_color = color_lerp(0.5f, d->text_color & 0xFFFFFF, d->text_color);
                it->saved_title_color = d->title_color;
                d->title_color = color_lerp(0.5f, d->title_color & 0xFFFFFF, d->title_color);
            } else if (c == 1) {
                d->y = it->saved_y;
                d->scale_y = 1.0f;
                d->show_title = it->saved_show_title;
                it->ghost = 0;
            } else if (c == 0) {
                d->visible = 1;
                it->saved_y = d->y;
                d->y = (short)(int)((float)d->y + (float)d->h * 0.25f);
                d->anchor = 0;
                d->scale_y = 0.5f;
                it->saved_show_title = d->show_title;
                d->show_title = 0;
                it->ghost = 1;
                it->x2 = d->x;
                it->y2 = (short)(int)((float)it->saved_y + (float)d->h * 0.125f);
                it->scale_x2 = 1.0f;
                it->scale_y2 = 0.75f;
                it->color2 = color_lerp(0.5f, d->color & 0xFFFFFF, d->color);
                it->saved_text = d->text;
                it->saved_title = d->title;
                d->text = 0;
                d->title = 0;
            }
            it->counter++;
        } else if (d->open_anim == 1) {
            c = it->counter;
            if (c == 4) {
                it->state = 2;
            } else if (c == 3) {
                d->text_color = it->saved_text_color;
                d->title_color = it->saved_title_color;
            } else if (c == 2) {
                d->text = it->saved_text;
                d->title = it->saved_title;
                it->saved_text_color = d->text_color;
                d->text_color = color_lerp(0.5f, d->text_color & 0xFFFFFF, d->text_color);
                it->saved_title_color = d->title_color;
                d->title_color = color_lerp(0.5f, d->title_color & 0xFFFFFF, d->title_color);
            } else if (c == 1) {
                d->scale_y = 1.0f;
                d->show_title = it->saved_show_title;
                it->ghost = 0;
            } else if (c == 0) {
                d->visible = 1;
                d->anchor = 0;
                d->scale_y = 0.5f;
                it->saved_show_title = d->show_title;
                d->show_title = 0;
                it->ghost = 1;
                it->scale_x2 = 1.0f;
                it->x2 = d->x;
                it->y2 = d->y;
                it->scale_y2 = 0.75f;
                it->color2 = color_lerp(0.5f, d->color & 0xFFFFFF, d->color);
                it->saved_text = d->text;
                it->saved_title = d->title;
                d->text = 0;
                d->title = 0;
            }
            it->counter++;
        } else {
            it->counter++;
        }
    } else {
        if (st == 4 || st == 2 || st == 0) {
            if (st == 0) {
                if (d->scale_x == 0.0f)
                    d->scale_x = 1.0f;
                if (d->scale_y == 0.0f)
                    d->scale_y = 1.0f;
                d->visible = d->open_anim == 0;
                it->state = 2;
            }
            if (mem_differs(it->def, &it->cur, sizeof(MenuItemDef))) {
                if (d->auto_size != 0) {
                    if (d->text != 0) {
                        mw = d->pad_r + 2 + d->pad_l + MENU_PAD_X + MENU_PAD_W;
                        mh = d->pad_b + 2 + d->pad_t + MENU_PAD_Y + MENU_PAD_H;
                        f4 = font_get_flag4();
                        f8 = font_get_flag8000();
                        font_set_size(d->font_size, d->font_size);
                        font_set_flag4(d->font_flag4);
                        font_set_flag8000(d->font_flag8000);
                        cw = (d->max_w == 0 ? 480 : d->max_w) - mw;
                        ch = (d->max_h == 0 ? 272 : d->max_h) - mh;
                        sz = font_measure_wtext2(cw << 6, ch << 6, d->text);
                        d->w = (short)(int)((float)mw + truncf((float)(sz >> 16) / 64.0f));
                        d->h = (short)(int)((float)mh + truncf((float)(sz & 0xFFFF) / 64.0f));
                        font_set_flag4(f4);
                        font_set_flag8000(f8);
                    }
                    v = d->w;
                    if (v < d->min_w)
                        v = d->min_w;
                    d->w = v;
                    v = d->h;
                    if (v < d->min_h)
                        v = d->min_h;
                    d->h = v;
                }
                if (d->auto_title != 0 && d->title != 0) {
                    f4 = font_get_flag4();
                    f8 = font_get_flag8000();
                    font_set_size(d->title_font_size, d->title_font_size);
                    font_set_flag4(d->font_flag4);
                    font_set_flag8000(0);
                    tw = font_measure_text(d->title);
                    th = font_get_height();
                    d->title_w = (short)(int)((float)wtitle[2].w + (float)wtitle[1].w + truncf((float)tw / 64.0f + 1.0f) + 4.0f);
                    d->title_h = (short)(int)((float)wtitle[0].h + truncf((float)th / 64.0f + 1.0f) + -1.0f);
                    font_set_flag4(f4);
                    font_set_flag8000(f8);
                }
            }
        }
    }
    menu_item_build_frame(*dl, it, m, 0);
    if (it->ghost != 0)
        menu_item_build_frame(*dl, it, m, 1);
}

/* Emits the GE state, the frame sprites and the texts of one item into the display list. */
void menu_item_build_text(void **dl, MenuItem *it, Menu *m)
{
    MenuItemDef *d = it->def;
    float tl[2], br[2];
    int f4, f8;
    int ww, hh;
    float fx, fy;

    menu_anchor_rect(tl, br, d->x, d->y, (int)((float)d->w * d->scale_x), (int)((float)d->h * d->scale_y), d->anchor);
    *dl = dl_clear_stencil(*dl, 0);
    *dl = dl_clear(*dl, 0x14);
    *dl = dl_blend_func(*dl, 0, 2, 3, 0, 0);
    *dl = dl_enable(*dl, 4);
    *dl = dl_alpha_test(*dl, 6, 0, 0xFF);
    *dl = dl_enable(*dl, 0);
    *dl = dl_depth_func(*dl, 7);
    *dl = dl_enable(*dl, 1);
    *dl = dl_depth_mask(*dl, 0);
    *dl = dl_enable(*dl, 9);
    *dl = dl_tex_mode(*dl, 3, 0, 0, 0);
    *dl = dl_tex_func(*dl, 0, 1);
    *dl = dl_tex_wrap(*dl, 0, 0);
    *dl = dl_ambient_color(*dl, 0xFFFFFFFF);
    *dl = dl_set_tex_filter(*dl, 0, 0);
    if (d->gim != 0)
        *dl = gim_set_texture(*dl);
    else
        *dl = gim_set_texture(*dl, m->window_gim);
    if (it->ghost != 0)
        *dl = dl_draw_array(*dl, 6, 0x80011E, it->nverts2, 0, it->verts2);
    *dl = dl_draw_array(*dl, 6, 0x80011E, it->nverts, 0, it->verts);
    *dl = dl_disable(*dl, 1);
    font_text_set(*dl);
    if (d->text != 0) {
        f4 = font_get_flag4();
        f8 = font_get_flag8000();
        font_set_size(d->font_size, d->font_size);
        font_set_flag4(d->font_flag4);
        font_set_flag8000(d->font_flag8000);
        font_set_color(d->text_color);
        fx = (float)(MENU_PAD_X + 1) + tl[0] + (float)d->pad_l;
        fy = (float)(MENU_PAD_Y + 1) + tl[1] + (float)d->pad_t;
        ww = -2 - MENU_PAD_W - MENU_PAD_X - d->pad_r - d->pad_l;
        hh = -2 - MENU_PAD_H - MENU_PAD_Y - d->pad_b - d->pad_t;
        font_draw_wtext_ex((int)fx << 6, (int)fy << 6, (ww + d->w) * 64, (hh + d->h) * 64, d->text, 0xFFFFFFFF);
        font_set_flag4(f4);
        font_set_flag8000(f8);
    }
    if (d->title != 0 && d->show_title != 0) {
        f4 = font_get_flag4();
        f8 = font_get_flag8000();
        font_set_size(d->title_font_size, d->title_font_size);
        font_set_flag4(d->font_flag4);
        font_set_flag8000(0);
        font_set_color(d->title_color);
        font_draw_text((int)(tl[0] + 2.0f + (float)wtitle[1].w) << 6,
                       (int)((float)wtitle[0].h + (tl[1] - (float)d->title_h)) << 6,
                       (d->title_w - 4 - wtitle[1].w - wtitle[2].w) * 64, d->title);
        font_set_flag4(f4);
        font_set_flag8000(f8);
    }
    *dl = font_text_clear();
}

/* Builds the frame sprites (and the ghost copy when `ghost`) into inline display-list memory. */
void menu_item_build_frame(void *dl, MenuItem *it, Menu *m, int ghost)
{
    MenuItemDef *d = it->def;
    WVertex *v;
    short x, y;
    float sx, sy;
    unsigned int color;
    int w, h, n, k, step, rows, rest;
    float tl[2], br[2];
    const WPiece *fr;
    int uv[2], uvt[2];
    WPiece side;
    int tabflip, taby;
    unsigned int style;
    int ttw, tth, tty;
    float tyf;

    n = menu_item_sprite_count(it, ghost);
    if (ghost == 0) {
        v = dl_alloc_inline(dl, n << 4);
        it->verts = v;
        x = d->x;
        y = d->y;
        sx = d->scale_x;
        sy = d->scale_y;
        color = d->color;
    } else {
        v = dl_alloc_inline(dl, n << 4);
        it->verts2 = v;
        x = it->x2;
        y = it->y2;
        sx = it->scale_x2;
        sy = it->scale_y2;
        color = it->color2;
    }
    w = (int)((float)d->w * sx);
    h = (int)((float)d->h * sy);
    menu_anchor_rect(tl, br, x, y, w, h, d->anchor);
    style = d->style;
    switch (style) {
    case 0:
    case 1:
    case 2:
    case 3:
        fr = wframe_a;
        uv[0] = wstep_a[0] * style;
        uv[1] = wstep_a[1] * style;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        fr = wframe_b;
        uv[0] = wstep_b[0] * style;
        uv[1] = wstep_b[1] * style;
        break;
    }
    uvt[0] = wstep_a[0] * d->style2;
    uvt[1] = wstep_a[1] * d->style2;
    /* corners */
    menu_sprite(v, m->heap, &fr[0], (int)tl[0], (int)tl[1], d->z + 1, -1, -1, color, uv, 0);
    menu_sprite(v + 2, m->heap, &fr[1], (int)(br[0] - (float)fr[1].w), (int)tl[1], d->z + 1, -1, -1, color, uv, 0);
    menu_sprite(v + 4, m->heap, &fr[2], (int)tl[0], (int)(br[1] - (float)fr[2].h), d->z + 1, -1, -1, color, uv, 0);
    menu_sprite(v + 6, m->heap, &fr[3], (int)(br[0] - (float)fr[3].w), (int)(br[1] - (float)fr[3].h), d->z + 1, -1, -1, color, uv, 0);
    /* top, left, right, bottom edges */
    menu_sprite(v + 8, m->heap, &fr[4], (int)(tl[0] + (float)fr[0].w), (int)tl[1], d->z + 1, w - fr[0].w - fr[1].w, -1, color, uv, 0);
    menu_sprite(v + 10, m->heap, &fr[5], (int)tl[0], (int)(tl[1] + (float)fr[0].h), d->z + 1, -1, h - fr[0].h - fr[2].h, color, uv, 0);
    menu_sprite(v + 12, m->heap, &fr[6], (int)(br[0] - (float)fr[6].w), (int)(tl[1] + (float)fr[1].h), d->z + 1, -1, h - fr[1].h - fr[3].h, color, uv, 0);
    menu_sprite(v + 14, m->heap, &fr[7], (int)(tl[0] + (float)fr[2].w), (int)(br[1] - (float)fr[7].h), d->z + 1, w - fr[2].w - fr[3].w, -1, color, uv, 0);
    /* center, in rows of 8 texels */
    n = 16;
    h = h - fr[0].h - fr[2].h;
    rows = 0;
    if (h > 0) {
        k = 0;
        step = 0;
        do {
            rest = 8;
            if (h < (rows + 1) * 8)
                rest = h - k;
            side.x = fr[8].x;
            side.y = fr[8].y;
            side.w = wside_w;
            side.h = rest;
            menu_sprite(v + 16 + step, m->heap, &side, (int)(tl[0] + (float)fr[0].w), (int)((float)k + tl[1] + (float)fr[0].h), d->z + 1, w - fr[0].w - fr[1].w, rest, color, uv, 0);
            k += 8;
            step += 2;
            n += 2;
            rows++;
        } while (k < h);
    }
    /* tab */
    if (d->tab != 0) {
        if (style < 4) {
            if (d->tab == 4) {
                tabflip = 3;
                taby = (int)((float)fr[4].h + (tl[1] - (float)fr[9].h));
            } else if (d->tab == 3) {
                tabflip = 2;
                taby = (int)((float)fr[4].h + (tl[1] - (float)fr[9].h));
            } else if (d->tab == 2) {
                tabflip = 1;
                taby = (int)(br[1] - (float)fr[2].h);
            } else if (d->tab == 1) {
                tabflip = 0;
                taby = (int)(br[1] - (float)fr[2].h);
            }
            menu_sprite(v + n, m->heap, &fr[9], (int)(tl[0] + (float)d->tab_x), taby, d->z + 1, -1, -1, color, uv, tabflip);
            n += 2;
        }
    }
    /* title bar */
    if (d->title != 0 && d->show_title != 0) {
        ttw = wtitle[1].w;
        tth = wtitle[2].w;
        tyf = tl[1] - (float)d->title_h;
        tty = (int)tyf;
        menu_sprite(v + n, m->heap, &wtitle[0], (int)(tl[0] + (float)ttw), tty, d->z, d->title_w - ttw - tth, -1, color, uvt, 0);
        menu_sprite(v + n + 2, m->heap, &wtitle[1], (int)tl[0], tty, d->z, -1, -1, color, uvt, 0);
        menu_sprite(v + n + 4, m->heap, &wtitle[2], (int)((tl[0] + (float)d->title_w) - (float)tth), tty, d->z, -1, -1, color, uvt, 0);
        menu_sprite(v + n + 6, m->heap, &wtitle[3], (int)(tl[0] + (float)ttw), (int)(tyf + (float)wtitle[0].h), d->z, d->title_w - ttw - tth, wtitle[1].h - wtitle[0].h, color, uvt, 0);
        n += 8;
    }
    if (ghost == 0)
        it->nverts = n;
    else
        it->nverts2 = n;
}

/* Number of sprites (2 vertices each) the frame needs. */
int menu_item_sprite_count(MenuItem *it, int ghost)
{
    MenuItemDef *d = it->def;
    float sy;
    const WPiece *fr;
    int n, h, k;

    if (ghost != 0)
        sy = it->scale_y2;
    else
        sy = d->scale_y;
    h = (int)((float)d->h * sy);
    switch (d->style) {
    case 0:
    case 1:
    case 2:
    case 3:
        fr = wframe_a;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        fr = wframe_b;
        break;
    }
    n = 0;
    if (d->title != 0 && d->show_title != 0)
        n += 8;
    h = h - fr[0].h - fr[2].h;
    n += 16;
    k = 0;
    while (k < h) {
        k += 8;
        n += 2;
    }
    if (d->tab != 0 && d->style < 4)
        n += 2;
    return n;
}
#endif

/* Top-left and bottom-right of a w x h box placed at (x, y) according to the anchor mode. */
void menu_anchor_rect(float *tl, float *br, int x, int y, int w, int h, int anchor)
{
    switch (anchor) {
    case 0:
        tl[0] = (float)x;
        tl[1] = (float)y;
        break;
    case 1:
        tl[0] = (float)(x - w);
        tl[1] = (float)y;
        break;
    case 2:
        tl[0] = (float)x;
        tl[1] = (float)(y - h);
        break;
    case 3:
        tl[0] = (float)(x - w);
        tl[1] = (float)(y - h);
        break;
    case 4:
        tl[0] = (float)x - (float)w * 0.5f;
        tl[1] = (float)y;
        break;
    case 5:
        tl[0] = (float)x;
        tl[1] = (float)y - (float)h * 0.5f;
        break;
    case 6:
        tl[0] = (float)x - (float)w * 0.5f;
        tl[1] = (float)y - (float)h * 0.5f;
        break;
    }
    br[0] = tl[0] + (float)w;
    br[1] = tl[1] + (float)h;
}

/* One textured sprite (2 vertices) of piece `p` at (x, y); w/h < 0 use the piece size; flip 1/2/3 mirrors u/v/both. */
void menu_sprite(WVertex *v, void *heap, const WPiece *p, int x, int y, int z, int w, int h, unsigned int color, const int *uvoff, int flip)
{
    if (w < 0)
        w = p->w;
    if (h < 0)
        h = p->h;
    v[0].u = p->x + uvoff[0];
    v[0].v = p->y + uvoff[1];
    v[0].color = color;
    v[0].x = x;
    v[0].y = y;
    v[0].z = z;
    v[1].u = uvoff[0] + (p->x + p->w);
    v[1].v = uvoff[1] + (p->y + p->h);
    v[1].color = color;
    v[1].x = x + w;
    v[1].y = y + h;
    v[1].z = z;
    switch (flip) {
    case 0:
        break;
    case 1:
        mem_swap(heap, &v[0].u, &v[1].u, 2);
        break;
    case 2:
        mem_swap(heap, &v[0].v, &v[1].v, 2);
        break;
    case 3:
        mem_swap(heap, &v[0].u, &v[1].u, 2);
        mem_swap(heap, &v[0].v, &v[1].v, 2);
        break;
    }
}

int menu_find_free(Menu *m)
{
    int i;

    for (i = 0; i < 16; i++) {
        if (m->items[i] == 0)
            return i;
    }
    return -1;
}

int menu_find_item(Menu *m, MenuItemDef *def)
{
    int i;

    for (i = 0; i < 16; i++) {
        if (m->items[i] != 0 && def == m->items[i]->def)
            return i;
    }
    return -1;
}

/* OR of the byte differences between a and b (0 when equal). */
int mem_differs(const void *a, const void *b, unsigned int n)
{
    const unsigned char *pa = a;
    const unsigned char *pb = b;
    unsigned int r = 0;
    unsigned int i;

    for (i = 0; i < n; i++)
        r |= pa[i] ^ pb[i];
    return r;
}

void mem_swap(void *heap, void *a, void *b, int n)
{
    void *t = heap_alloc(heap, n);

    memcpy(t, a, n);
    memcpy(a, b, n);
    memcpy(b, t, n);
    if (t != 0)
        heap_set_level(heap, t);
}

unsigned int color_lerp(float t, unsigned int a, unsigned int b)
{
    float va[4], vb[4], vo[4];

    color_unpack(va, a);
    color_unpack(vb, b);
    vec4_lerp2(t, vo, va, vb);
    return color_pack(vo);
}
