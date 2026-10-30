// @range 0x08805874-0x08807A44
// @asm 0x08805874-0x08805D40
// @asm 0x08805F6C-0x088064F8
// @asm 0x08806F08-0x08806FC0
// @asm 0x088075E4-0x0880776C
// @data 0x0880E88C-0x0880E8BC
/* The screen: background and title animations, the per-type counters, the recipe grid (main deck,
   extra deck, side deck) with the cursor, the lists of saved decks / recipes / extra data lists
   with their scrollbar, the cursor frame, the name entry and the fade. */
#include "librecipe_viewer.h"
#include "allegrex.h"
#include "game.h"
#include "card_db.h"

#define RV_XY(x, y) ((unsigned short)(short)(x) | (unsigned short)(short)(y) << 16)

void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_clear_color(void *list, unsigned int rgba);
void *dl_clear_depth(void *list, int z);
void *dl_clear_stencil(void *list, int s);
void *dl_clear(void *list, int flags);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_tex_func(void *list, int func, int rgba);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *gfx_begin_2d(void *list);
void *gfx_end_2d(void *list);
void *ehf_draw(void *list, void *e, void *opts);
void *cardlist_build(void *list, RvScroll *s);
void cardinfo_draw(void);
void *extdata_draw(void *dl);
unsigned short *table_get_entry(void *t, unsigned int i);
unsigned short *chara_get_name(int i, int j);

void *rv_draw_digits(void *list, int pos, int zeros, unsigned int n, unsigned short fmt, unsigned int color, void *gim);
void *rv_draw_number(void *list, short x, short y, int n, int step, unsigned short digits, int zeros);
void *rv_draw_frame(void *list, int x, int y, int w, int h, void *gim, unsigned int flags);
void *rv_draw_card(void *list, int x, int y, int shadow, unsigned int color, RvEntry *e, void *gim,
                   void *alt_gim, int *alt);
void *rv_draw_marks(void *list, int x, int y, RvEntry *e, int show_count, int blink, int alt, unsigned short n,
                    void *digits_gim, void *icons_gim);
void *rv_draw_arrow(void *list, int x, int y, unsigned char kind, unsigned int period, int blink, int base,
                    unsigned int time, void *gim);
void *rv_osk_draw(void *dl);
int rv_osk_update(int layer);

void *rv_draw_bg(void *dl);
void *rv_draw_title(void *dl);
void *rv_draw_tabs(void *dl);
void *rv_draw_type_counts(void *dl);
void *rv_draw_type_bars(void *dl);
void *rv_draw_recipe(void *dl);
void *rv_draw_lists(void *dl);
void *rv_draw_deck_row(void *dl, int i, int x, int y);
void *rv_draw_recipe_row(void *dl, int i, int x, int y);
void *rv_draw_ext_row(void *dl, int i, int x, int y);
void *rv_draw_type_bar(void *dl, int x, int y, int h, int type);
void *rv_draw_date(void *dl, int x, int y, unsigned short *date);
void *rv_draw_grid_card(void *dl, int x, int y, RvEntry *e);
void *rv_draw_grid_cursor(void *dl, unsigned short x, unsigned short y);
void *rv_draw_scrollbar(void *dl, int x, int y, int h, int pos, int page, int total);

#ifdef NON_MATCHING /* o original deixa uma cópia morta do `li a0,4` depois do `b` do ramo da ficha da carta (problema aberto, como tlist_draw) */
/* 0x08805874: export */
void recipe_viewer_draw(void)
{
    void *dl = dl_select_layer(0, 0);

    dl = dl_clear_color(dl, 0xFF000000);
    dl = dl_clear_depth(dl, 0);
    dl = dl_clear_stencil(dl, 0);
    dl = dl_clear(dl, 0x17);
    dl_set_list(dl);
    if (rv.state != 7 && rv.state != 1) {
        if (rv.info_open) {
            cardinfo_draw();
        } else {
            dl = dl_select_layer(4, 0);
            dl = dl_enable(dl, 9);
            dl = dl_enable(dl, 4);
            dl = dl_tex_func(dl, 0, 1);
            dl = dl_set_tex_filter(dl, 0, 0);
            dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
            dl = dl_disable(dl, 1);
            if (rv.state == 0) {
                dl = extdata_draw(dl);
                dl_set_list(dl);
            } else {
                dl = rv_draw_bg(dl);
                dl = rv_draw_title(dl);
                dl = rv_draw_tabs(dl);
                dl = rv_draw_type_counts(dl);
                dl = rv_draw_type_bars(dl);
                dl = rv_draw_recipe(dl);
                dl = rv_draw_lists(dl);
                if (rv.state == 4) {
                    int y = rv.pos[rv.tab].cursor * 29;
                    dl = dl_disable(dl, 9);
                    dl = dl_tex_func(dl, 4, 0);
                    dl = dl_ambient_color(dl, 0x80000000);
                    dl = draw_set_offsets(dl, 0, 0, 146, y + 18);
                    dl = draw_sprite(dl, 148, 18, 0, 0, 0, 0, 0);
                    dl = draw_set_offsets(dl, 0, 0, 10, 29);
                    dl = draw_sprite(dl, 284, y + 36, 0, 0, 0, 0, 0);
                    dl = draw_set_offsets(dl, 0, 0, 146, 271 - (y + 65));
                    dl = draw_sprite(dl, 148, y + 65, 0, 0, 0, 0, 0);
                    dl = dl_enable(dl, 9);
                    dl = dl_tex_func(dl, 0, 1);
                    dl = dl_ambient_color(dl, 0xFFFFFFFF);
                }
                if (!rv.no_frame) {
                    unsigned int t = 0;
                    if (rv.state != 4)
                        t = gfx_get_state()->frame;
                    dl = rv_draw_frame(dl, 151, rv.pos[rv.tab].cursor * 29 + 36, 132, 29, rvres_get(0x8006), t);
                }
                dl = cardlist_build(dl, &rv.scroll);
                if (rv.osk)
                    dl = rv_osk_draw(dl);
                dl_set_list(dl);
                if (rv.busy) {
                    rv.busy = rv_osk_update(4) == 0;
                } else {
                    dl = dl_select_layer(4, 0);
                    dl = dl_disable(dl, 1);
                    fade_apply(&dl);
                    dl_set_list(dl);
                }
            }
        }
    }
}
#endif

void *rv_draw_bg(void *dl)
{
    void *ehf = rvres_get(0x8021);
    unsigned int opts[24];

    if (ehf == 0)
        return dl;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()->frame;
    dl = gfx_begin_2d(dl);
    dl = ehf_draw(dl, ehf, opts);
    dl = gfx_end_2d(dl);
    dl = dl_enable(dl, 4);
    dl = dl_tex_func(dl, 0, 1);
    dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
    return dl;
}

unsigned short rv_title_ids[4] = { 0x9C, 0x47, 0x46, 0 };   /* strTbl entry of the title per mode */

/* title bar (animated) with the title of the mode */
void *rv_draw_title(void *dl)
{
    void *ehf = rvres_get(0x1011);
    unsigned int opts[24];

    if (ehf == 0)
        return dl;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()->frame;
    dl = gfx_begin_2d(dl);
    dl = ehf_draw(dl, ehf, opts);
    dl = gfx_end_2d(dl);
    font_text_set(dl);
    font_set_color(0xFFFFFFFF);
    font_set_size(12, 12);
    font_draw_text(157 << 6, 3 << 6, 323 << 6, table_get_entry(rvres_get(0x8033), rv_title_ids[rv.mode]));
    return font_text_clear();
}

/* tab titles (animated) */
void *rv_draw_tabs(void *dl)
{
    void *ehf = rvres_get(0x1010);
    unsigned int opts[24];

    if (ehf == 0)
        return dl;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()->frame;
    dl = gfx_begin_2d(dl);
    dl = ehf_draw(dl, ehf, opts);
    dl = gfx_end_2d(dl);
    return dl;
}

/* the shown recipe: main deck (4 rows), side deck (2 rows), extra deck (1 row), revealed up to
   rv.shown cards, with the cursor in the recipe state */
#ifdef NON_MATCHING /* forma próxima; contadores e limites das três seções em outros $s e na pilha */
void *rv_draw_recipe(void *dl)
{
    RvGrid *g = &rv.grid;
    RvEntry *sel = 0;
    int sel_y = 0;
    int sel_x = 0;
    int done = 0;
    int y = -18;
    int k = 0;
    int limit;
    int row;
    int i;
    int n;

    dl = rv_draw_digits(dl, 0x301CD, 0, g->c.n_main, 2, 0xFFFFFFFF, rvres_get(0x8009));
    limit = rv.shown;
    if (g->c.n_main < limit)
        limit = g->c.n_main;
    for (row = 0; row < 4; row++) {
        y += 33;
        if (g->row == row)
            sel_y = y;
        for (i = 0; i < g->main_cols; i++) {
            int x;
            RvEntry *e;

            if (done + i >= limit)
                break;
            if (g->main_cols < 8)
                x = i * 175 / 7 + 300;
            else
                x = i * 152 / (g->main_cols - 1) + 300;
            e = &g->c.main[done + i];
            if (rv.state == 4 && g->col == i && g->row == row) {
                sel_x = x;
                sel = e;
            } else {
                dl = rv_draw_grid_card(dl, x, y, e);
            }
            k++;
        }
        done = k;
    }
    k = 0;
    done = 0;
    y += 11;
    dl = rv_draw_digits(dl, 461 | (unsigned short)(y + 21) << 16, 0, g->c.n_side, 2, 0xFFFFFFFF, rvres_get(0x8009));
    limit = rv.shown - g->c.n_main;
    if (g->c.n_side < limit)
        limit = g->c.n_side;
    for (; row < 6; row++) {
        y += 33;
        if (g->row == row)
            sel_y = y;
        for (i = 0; i < g->side_cols; i++) {
            int x;
            RvEntry *e;

            if (done + i >= limit)
                break;
            if (g->side_cols < 8)
                x = i * 175 / 7 + 300;
            else
                x = i * 152 / (g->side_cols - 1) + 300;
            e = &g->c.side[done + i];
            if (rv.state == 4 && g->col == i && g->row == row) {
                sel_x = x;
                sel = e;
            } else {
                dl = rv_draw_grid_card(dl, x, y, e);
            }
            k++;
        }
        done = k;
    }
    dl = rv_draw_digits(dl, 461 | (unsigned short)(y + 32) << 16, 0, g->c.n_extra, 2, 0xFFFFFFFF, rvres_get(0x8009));
    n = rv.shown - g->c.n_main - g->c.n_side;
    if (g->c.n_extra < n)
        n = g->c.n_extra;
    if (g->row == row)
        sel_y = y + 44;
    for (i = 0; i < n; i++) {
        int x;

        if (g->c.n_extra < 8)
            x = i * 175 / 7 + 300;
        else
            x = i * 152 / (g->c.n_extra - 1) + 300;
        if (rv.state == 4 && g->col == i && g->row == 6) {
            sel_x = x;
            sel = &g->c.extra[i];
        } else {
            dl = rv_draw_grid_card(dl, x, y + 44, &g->c.extra[i]);
        }
    }
    if (rv.state == 4) {
        if (sel_x == 0)
            sel_x = g->col * 175 / 7 + 300;
        if (sel)
            dl = rv_draw_grid_card(dl, sel_x, sel_y, sel);
        return rv_draw_grid_cursor(dl, sel_x, sel_y);
    }
    return dl;
}
#endif

/* row drawers per tab, the template rv_draw_lists copies */
void *(*rv_row_draw[3])(void *dl, int i, int x, int y) = {
    rv_draw_deck_row, rv_draw_recipe_row, rv_draw_ext_row,
};

typedef struct RvRowDraw { void *(*f[3])(void *dl, int i, int x, int y); } RvRowDraw;

/* the 8 visible rows of the current tab and its scrollbar */
void *rv_draw_lists(void *dl)
{
    RvPos *p = &((RecipeViewer *)(char *)&rv)->pos[rv.tab];
    RvRowDraw draw = *(RvRowDraw *)rv_row_draw;
    void *(*f)(void *dl, int i, int x, int y);
    int i;
    int y;
    int total;

    f = draw.f[rv.tab];
    for (i = 0, y = 36; i < 8; i++, y += 29)
        dl = f(dl, p->top + i, 151, y);
    total = 0;
    switch (rv.tab) {
    case 0:
        total = 50;
        break;
    case 1:
        total = rv.n_recipes;
        break;
    case 2:
        total = rv.n_ext;
        break;
    }
    return rv_draw_scrollbar(dl, 284, 45, 214, p->top, 8, total);
}

/* row of a saved deck: frame, name and date (or "- NO DATA -"), complete-deck mark */
void *rv_draw_deck_row(void *dl, int i, int x, int y)
{
    int need;
    SaveRec *s = sysstate_rec_get(i);
    void *gim;

    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 0);
    gim = rvres_get(0x1001);
    if (gim) {
        dl = gim_set_texture(dl, gim);
        dl = draw_set_offsets(dl, 103, 30, 103, 30);
        dl = draw_sprite(dl, x, y, 0, 0, 0, 0, 0);
        dl = draw_set_offsets(dl, 24, 30, 24, 30);
        dl = draw_sprite(dl, x + 103, y, 0, 5, 0, 0, 0);
        dl = draw_set_offsets(dl, 6, 30, 6, 30);
        dl = draw_sprite(dl, x + 127, y, 0, 103, 0, 0, 0);
    }
    font_text_set(dl);
    font_set_color(0xFF000000);
    font_set_size(12, 12);
    if (s->used == 0) {
        unsigned short txt[12] = { '-', ' ', 'N', 'O', ' ', 'D', 'A', 'T', 'A', ' ', '-', 0 };
        int w = (font_measure_text(txt) >> 6) + 1;
        font_draw_text((x + 16 + ((100U - w) >> 1)) << 6, (y + 9) << 6, (w + 1) << 6, txt);
        dl = font_text_clear();
    } else {
        font_draw_wtext((x + 16) << 6, (y + 3) << 6, 7040, s->name);
        dl = font_text_clear();
        dl = rv_draw_date(dl, x + 16, y + 16, (unsigned short *)s->unk44);
    }
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 0);
    gim = rvres_get(0x800A);
    if (gim) {
        dl = gim_set_texture(dl, gim);
        dl = draw_set_offsets(dl, 10, 14, 10, 14);
        dl = draw_sprite(dl, x + 3, y + 2, 0, 54, 35, 0, 0);
        need = rv.decks[i].need;
        if (need > 0 && rv.decks[i].have == need) {
            dl = draw_set_offsets(dl, 22, 24, 22, 24);
            dl = draw_sprite(dl, x + 110, y + 4, 0, 102, 17, 0, 0);
        }
    }
    return dl;
}

/* row of a recipe: name, author, number and complete-deck mark */
void *rv_draw_recipe_row(void *dl, int i, int x, int y)
{
    int need;
    void *gim;
    unsigned short *name;

    if (i >= rv.n_recipes)
        return dl;
    {
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
        dl = dl_set_tex_filter(dl, 0, 0);
        gim = rvres_get(0x1001);
        if (gim) {
            dl = gim_set_texture(dl, gim);
            dl = draw_set_offsets(dl, 103, 30, 103, 30);
            dl = draw_sprite(dl, x, y, 0, 0, 32, 0, 0);
            dl = draw_set_offsets(dl, 24, 30, 24, 30);
            dl = draw_sprite(dl, x + 103, y, 0, 5, 32, 0, 0);
            dl = draw_set_offsets(dl, 6, 30, 6, 30);
            dl = draw_sprite(dl, x + 127, y, 0, 103, 32, 0, 0);
        }
        font_text_set(dl);
        font_set_color(0xFF000000);
        font_set_size(12, 12);
        font_draw_wtext((x + 16) << 6, (y + 3) << 6, 7040, recipe_get_name(rv.recipes[i].index));
        font_set_size(10, 10);
        name = chara_get_name(0, rv.recipes[i].kind);
        if (font_measure_text(name) >> 6 > 92)
            name = chara_get_name(1, rv.recipes[i].kind);
        font_draw_wtext((x + 20) << 6, (y + 16) << 6, 5888, name);
        dl = font_text_clear();
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
        dl = dl_set_tex_filter(dl, 0, 0);
        gim = rvres_get(0x800A);
        if (gim) {
            dl = gim_set_texture(dl, gim);
            dl = draw_set_offsets(dl, 10, 14, 10, 14);
            dl = draw_sprite(dl, x + 3, y + 2, 0, 71, 35, 0, 0);
            need = rv.recipes[i].need;
        if (need > 0 && rv.recipes[i].have == need) {
                dl = draw_set_offsets(dl, 22, 24, 22, 24);
                dl = draw_sprite(dl, x + 110, y + 4, 0, 102, 17, 0, 0);
            }
        }
        return rv_draw_digits(dl, (unsigned short)x | (unsigned short)(y + 17) << 16, 1, rv.recipes[i].number, 0x303,
                              0xFFFFFFFF, rvres_get(0x8009));
    }
}

/* row of a list from the extra data */
void *rv_draw_ext_row(void *dl, int i, int x, int y)
{
    int need;
    void *gim;

    if (i >= rv.n_ext)
        return dl;
    {
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
        dl = dl_set_tex_filter(dl, 0, 0);
        gim = rvres_get(0x1001);
        if (gim) {
            dl = gim_set_texture(dl, gim);
            dl = draw_set_offsets(dl, 103, 30, 103, 30);
            dl = draw_sprite(dl, x, y, 0, 0, 64, 0, 0);
            dl = draw_set_offsets(dl, 24, 30, 24, 30);
            dl = draw_sprite(dl, x + 103, y, 0, 5, 64, 0, 0);
            dl = draw_set_offsets(dl, 6, 30, 6, 30);
            dl = draw_sprite(dl, x + 127, y, 0, 103, 64, 0, 0);
        }
        font_text_set(dl);
        font_set_color(0xFF000000);
        font_set_size(12, 12);
        font_draw_wtext((x + 16) << 6, (y + 3) << 6, 7040, rv.ext[i].name);
        dl = font_text_clear();
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
        dl = dl_set_tex_filter(dl, 0, 0);
        gim = rvres_get(0x800A);
        if (gim) {
            dl = gim_set_texture(dl, gim);
            dl = draw_set_offsets(dl, 10, 14, 10, 14);
            dl = draw_sprite(dl, x + 3, y + 2, 0, 88, 35, 0, 0);
            need = rv.ext_info[i].need;
        if (need > 0 && rv.ext_info[i].have == need) {
                dl = draw_set_offsets(dl, 22, 24, 22, 24);
                dl = draw_sprite(dl, x + 110, y + 4, 0, 102, 17, 0, 0);
            }
        }
    }
    return dl;
}

#ifdef NON_MATCHING /* só a alocação de $s difere (y, contador e ponteiro) */
/* number of cards of each type */
void *rv_draw_type_counts(void *dl)
{
    int i;
    int y;
    int x;
    int n;

    for (i = 0, y = 9; i < 6; y += 19, i++) {
        n = rv.n_type[i];
        x = 122;
        if (n < 10)
            x -= 3;
        dl = rv_draw_digits(dl, (unsigned short)x | (unsigned short)y << 16, 0, n, 2, 0xFFFFFFFF, rvres_get(0x8009));
    }
    return dl;
}
#endif

/* stacked bar of the types (111 pixels) */
void *rv_draw_type_bars(void *dl)
{
    int total = 0;
    int b[7];
    int i;
    int acc;

    for (i = 0; i < 6; i++)
        total += rv.n_type[i];
    memset(b, 0, 0x18);
    if (total > 0) {
        b[0] = 6;
        acc = 0;
        for (i = 0; i < 6; i++) {
            acc += rv.n_type[i];
            b[i + 1] = acc * 111 / total + 6;
        }
    }
    for (i = 0; i < 6; i++) {
        if (rv.n_type[i])
            dl = rv_draw_type_bar(dl, 141, b[i], b[i + 1] - b[i], i);
    }
    return dl;
}

void *rv_draw_type_bar(void *dl, int x, int y, int h, int type)
{
    void *gim = rvres_get(0x800A);

    if (gim == 0)
        return dl;
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = gim_set_texture(dl, gim);
    dl = draw_set_offsets(dl, 3, 1, 3, h);
    return draw_sprite(dl, x, y, 0, 0, type + 122, 0, 0);
}

/* save date: yy/mm/dd hh:mm */
void *rv_draw_date(void *dl, int x, int y, unsigned short *d)
{
    unsigned short sep[2] = { '/', 0 };

    font_set_size(9, 10);
    font_set_color(0xFF000000);
    dl = rv_draw_number(dl, x + 12, y, d[0] % 100, 6, 2, 1);
    font_text_set(dl);
    font_draw_text((x + 12) << 6, y << 6, 640, sep);
    dl = font_text_clear();
    dl = rv_draw_number(dl, x + 28, y, d[1], 6, 2, 1);
    font_text_set(dl);
    font_draw_text((x + 28) << 6, y << 6, 640, sep);
    dl = font_text_clear();
    dl = rv_draw_number(dl, x + 44, y, d[2], 6, 2, 1);
    dl = rv_draw_number(dl, x + 60, y, d[3], 6, 2, 1);
    sep[0] = ':';
    font_text_set(dl);
    font_draw_text((x + 61) << 6, (y - 1) << 6, 640, sep);
    dl = font_text_clear();
    return rv_draw_number(dl, x + 78, y, d[4], 6, 2, 1);
}

/* a card of the recipe grid, dimmed when not owned */
void *rv_draw_grid_card(void *dl, int x, int y, RvEntry *e)
{
    dl = rv_draw_card(dl, x, y, 1, 0xFFFFFFFF, e, rvres_get(0x8001), rvres_get(0x8002), rvres_get(0x8032));
    dl = rv_draw_marks(dl, x, y, e, 0, 0, 1, 0, rvres_get(0x8009), rvres_get(0x800A));
    if (!e->owned) {
        dl = dl_disable(dl, 9);
        dl = dl_tex_func(dl, 4, 0);
        dl = dl_ambient_color(dl, 0x80000000);
        dl = draw_set_offsets(dl, 0, 0, 22, 30);
        dl = draw_sprite(dl, x, y, 0, 0, 0, 0, 0);
        dl = dl_enable(dl, 9);
        dl = dl_tex_func(dl, 0, 1);
    }
    return dl;
}

#ifdef NON_MATCHING /* o original mascara x num registrador novo e reaproveita o do parâmetro para x0 */
/* the four pulsing corners of the grid cursor */
void *rv_draw_grid_cursor(void *dl, unsigned short x, unsigned short y)
{
    void *gim;
    int x0;
    int d = (gfx_get_state()->frame >> 1) % 6;
    int y0;

    int x1;
    int y1;

    if (d > 3)
        d = 6 - d;
    gim = rvres_get(0x8008);
    if (gim == 0)
        return dl;
    {

        dl = dl_ambient_color(dl, 0xFFFFFFFF);
        dl = dl_set_tex_filter(dl, 0, 0);
        dl = gim_set_texture(dl, gim);
        dl = draw_set_offsets(dl, 10, 10, 10, 10);
        y0 = y - d - 2;
        x0 = x - d - 2;
        dl = draw_sprite(dl, x0, y0, 0, 0, 92, 0, 0);
        x1 = x + 16 + d;
        dl = draw_sprite(dl, x1, y0, 0, 17, 92, 0, 0);
        y1 = y + 24 + d;
        dl = draw_sprite(dl, x0, y1, 0, 0, 108, 0, 0);
        dl = draw_sprite(dl, x1, y1, 0, 17, 108, 0, 0);
    }
    return dl;
}
#endif

void *rv_draw_scrollbar(void *dl, int x, int y, int h, int pos, int page, int total)
{
    int track = h - 2;         /* becomes the knob offset */
    void *gim = rvres_get(0x8007);
    int knob;
    int period_up;
    int period_down;

    if (gim == 0)
        return dl;
    if (total <= page) {
        knob = track;
        track = 0;
    } else {
        knob = ALLEGREX_MAX(page * track / total, 10);
        track = ALLEGREX_MIN(ALLEGREX_MAX(pos * (track - knob) / (total - page), 0), h - 2 - knob);
    }
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = gim_set_texture(dl, gim);
    dl = draw_set_offsets(dl, 8, 5, 8, 5);
    dl = draw_sprite(dl, x, y, 0, 119, 0, 0, 0);
    dl = draw_sprite(dl, x, y + h - 5, 0, 119, 15, 0, 0);
    dl = draw_set_offsets(dl, 8, 5, 8, h - 10);
    dl = draw_sprite(dl, x, y + 5, 0, 119, 5, 0, 0);
    dl = draw_set_offsets(dl, 4, 5, 4, 5);
    dl = draw_sprite(dl, x + 2, y + track + 2, 0, 119, 20, 0, 0);
    dl = draw_sprite(dl, x + 2, knob + (y + track - 3), 0, 119, 35, 0, 0);
    dl = draw_set_offsets(dl, 4, 5, 4, knob - 10);
    dl = draw_sprite(dl, x + 2, y + track + 7, 0, 119, 25, 0, 0);
    period_up = gfx_get_state()->fps;
    dl = rv_draw_arrow(dl, x, y - 7, 0, period_up, pos > 0, 1, gfx_get_state()->frame, gim);
    period_down = gfx_get_state()->fps;
    return rv_draw_arrow(dl, x, y + h + 2, 1, period_down, pos < total - page, 1, gfx_get_state()->frame, gim);
}
