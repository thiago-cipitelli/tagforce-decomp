// @range 0x08859DF0-0x0885C688
// @data 0x088F00E4-0x088F0114
// @asm 0x08859E98-0x0885ADD0
// @asm 0x0885AFB0-0x0885B498
// @asm 0x0885BE3C-0x0885C000
/* Card detail viewer: full-screen panel with the card picture, name, description / genre list /
   deck-list tab, ATK/DEF, level stars and scroll bar. Driven by 0x50 bytes of state at 0x08BEA44C
   and drawn on its own display-list layer; opened/closed with a black fade. */
#include "psp.h"
#include "libc.h"
#include "game.h"
#include "ehsys.h"
#include "font.h"
#include "card_db.h"

/* Viewer state: separate globals laid out consecutively from 0x08BEA44C (cardinfo_open clears the
   0x50 bytes in one memset). They are not one struct in the original: &cardinfo_entry is a direct
   address, where a struct member would compile to base + offset. */
typedef struct CardInfoFlags {
    unsigned char has_lists : 1;       /* cardinfo_set_lists called */
    unsigned char has_alt : 1;         /* alternate pictures available (cardinfo_has_alt_pict) */
    unsigned char pad : 6;
} CardInfoFlags;

typedef struct CardInfoEntry {         /* index entry: deck lists mentioning the card */
    unsigned short id;
    unsigned short count;              /* lines at index + offset, each pict:4 text_id:12 */
    unsigned int offset;
} CardInfoEntry;

extern unsigned short cardinfo_card_id;     /* 0x08BEA44C */
extern CardInfoFlags cardinfo_flags;        /* 0x08BEA44E */
extern void *cardinfo_window_gim;           /* 0x08BEA450 frame / icon sheet */
extern void *cardinfo_bg_gim;               /* 0x08BEA454 480x272 background */
extern void *cardinfo_list_gim;             /* 0x08BEA458 deck-list box sheet */
extern unsigned int *cardinfo_text_pack;    /* 0x08BEA45C {count, table_off, data_off}, entries off:28 kind:4 */
extern unsigned short *cardinfo_index;      /* 0x08BEA460 {count, CardInfoEntry[count]} */
extern CardInfoEntry cardinfo_entry;        /* 0x08BEA464 copy of the entry for card_id */
extern unsigned short cardinfo_view_state;  /* 0x08BEA46C 1 fade in, 2 active, 3 fade out, 4 done */
extern unsigned short cardinfo_tab;         /* 0x08BEA46E 0 description, 1 genres, 2 lists */
extern unsigned short cardinfo_n_pict;      /* 0x08BEA470 */
extern unsigned int cardinfo_scroll;        /* 0x08BEA474 26.6 */
extern unsigned int cardinfo_scroll_max;    /* 0x08BEA478 */
extern unsigned short cardinfo_layer;       /* 0x08BEA47C display-list layer */
extern unsigned int cardinfo_panel[7];      /* 0x08BEA480 card panel record, [0] = card id */

void *dl_select_layer(int layer, unsigned int *size_out);
void dl_set_list(void *list);
void *dl_clear_color(void *list, unsigned int rgba);
void *dl_clear_depth(void *list, int z);
void *dl_clear_stencil(void *list, int s);
void *dl_clear(void *list, int flags);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_tex_func(void *list, int func, int rgba);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_tex_mode(void *list, int pixfmt, int maxmips, int a2, int swizzle);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
void *gim_set_texture(void *list, void *gim);
unsigned int *gfx_get_state(void);
void *ehp_find(void *pack, const char *name, int flags);
unsigned char *syspack_get(void);
int ceil_to_int(float f);
unsigned short region_pick_value();
void se_play(int id);
unsigned short *sysmsg_get(int i);
int inv_get_bit3(unsigned short id);
void inv_set_hi(unsigned short id, int v);
int inv_get_hi(unsigned short id);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_is_done(void);
int fade_apply(void **list);
void *card_panel_draw(void *list, int x, int y, unsigned int *panel);
void *card_draw_frame_special(void *list, int x, int y, int attr);
void *cardlist_draw_icon(void *list, int x, int y, int attr, int prop);
void *cardlist_draw_stars(void *list, int x, int y, int n, int color);
void *card_draw_kind_icon(void *list, int x, int y, int kind);
void *card_draw_type_icon(void *list, int x, int y, int type);
void *cardlist_draw_scrollbar(void *list, int x, int y, int h, int pos, int size);
void *cardlist_draw_arrow(void *list, int x, int y, int dir, unsigned int a, int active, int b, unsigned int c);
unsigned short *card_info_fill(int id);

void cardinfo_calc_scroll(void);
void *cardinfo_draw_bg(void *list);
void *cardinfo_draw_window(void *list, unsigned int style, int x, int y, int w, int h);
void *cardinfo_draw_frame(void *list, int x, int y, int w, int h);
void *cardinfo_draw_tabs(void *list, int x, int y);
void cardinfo_draw_stat(int x, int y, int is_def, int value);
void cardinfo_draw_number(int x, int y, int value);
void *cardinfo_draw_title_bar(void *list, int x, int y, int w, unsigned short *text);
void *cardinfo_draw_type_row(void *list, int x, int y, int type_class);
void *cardinfo_draw_genre_row(void *list, int x, int y, int h, int genre);
void *cardinfo_draw_genre_icon(void *list, int x, int y, int genre);
void *cardinfo_draw_list_entry(void *list, int x, int y, unsigned short text_id);
void *cardinfo_draw_list_box(void *list, int x, int y, int lines);
unsigned short *cardinfo_find_entry(unsigned short id);
unsigned short *cardinfo_get_text(unsigned int id);
unsigned int cardinfo_get_text_kind(unsigned int id);
void *cardinfo_draw_hint(void *list, int x, int y, unsigned short *text, unsigned char arrows);
void *cardinfo_draw_lr_arrows(void *list);

int cardinfo_open(unsigned short card_id, void *window_gim, void *bg_gim)
{
    memset(&cardinfo_card_id, 0, 0x50);
    cardinfo_layer = 4;
    cardinfo_card_id = card_id;
    cardinfo_n_pict = 1;
    cardinfo_panel[0] = card_id;
    cardinfo_bg_gim = bg_gim;
    cardinfo_window_gim = window_gim;
    cardinfo_calc_scroll();
    fade_start(1, 0xFF000000, 16);
    cardinfo_view_state = 1;
    return 1;
}

#ifdef NON_MATCHING /* cardinfo_update: two scratch registers differ (constant 1 in a0, state++ in v1); cardinfo_draw: the original keeps the duplicated instruction after every likely branch to an else block (8 dead copies) where this compile moves it */
/* Input and fades; returns 1 once the viewer has closed. */
int cardinfo_update(void)
{
    PadState *pad = pad_get();
    unsigned short pict;
    unsigned int s;
    int n, delta, ay;

    switch (cardinfo_view_state) {
    case 0:
    case 4:
        return 1;
    case 1:
    case 3:
        if (fade_is_done())
            cardinfo_view_state++;
        return 0;
    }
    {
        if (pad->held & region_pick_value()) {
            fade_start_in(1, 0xFF000000, 16);
            cardinfo_view_state = 3;
            se_play(0x6214);
        }
        if (cardinfo_flags.has_alt) {
            pict = inv_get_hi(cardinfo_card_id);
            if (pad->held & 0x100) {
                pict = (pict + cardinfo_n_pict - 1) % cardinfo_n_pict;
                inv_set_hi(cardinfo_card_id, pict);
                if (cardinfo_flags.has_lists)
                    cardinfo_calc_scroll();
                se_play(0x621B);
            }
            if (pad->held & 0x200) {
                pict = (pict + 1) % cardinfo_n_pict;
                inv_set_hi(cardinfo_card_id, pict);
                if (cardinfo_flags.has_lists)
                    cardinfo_calc_scroll();
                se_play(0x621B);
            }
        }
        if (pad->held & 0x80) {
            if (cardinfo_flags.has_lists)
                n = 3;
            else
                n = 2;
            cardinfo_tab = (cardinfo_tab + n - 1) % n;
            cardinfo_calc_scroll();
            se_play(0x6212);
        }
        if (pad->held & 0x20) {
            if (cardinfo_flags.has_lists)
                n = 3;
            else
                n = 2;
            cardinfo_tab = (cardinfo_tab + 1) % n;
            cardinfo_calc_scroll();
            se_play(0x6212);
        }
        ay = pad->analog_y;
        delta = 0;
        if (ay > 0x9E)
            delta = (ay - 0x9E) * 4;
        if (ay < 0x62)
            delta = (ay - 0x62) * 4;
        if (pad->down & 0x10)
            delta = -0x80;
        if (pad->down & 0x40)
            delta = 0x80;
        s = cardinfo_scroll + delta;
        if ((int)s <= 0)
            cardinfo_scroll = 0;
        else if (s >= cardinfo_scroll_max)
            cardinfo_scroll = cardinfo_scroll_max;
        else
            cardinfo_scroll = s;
    }
    return 0;
}

void cardinfo_draw(void)
{
    void *list;
    unsigned short id;
    unsigned short saved_flags;
    unsigned short *hint_l = 0;
    unsigned short *hint_r = 0;
    unsigned short *title = 0;
    unsigned short *line;
    unsigned short v;
    unsigned int *gs;
    unsigned int pos, range;
    int y, i, n, attr, cls, pict;

    list = dl_select_layer(cardinfo_layer, 0);
    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (cardinfo_view_state != 4 && cardinfo_view_state != 0) {

        list = dl_select_layer(cardinfo_layer, 0);
        list = dl_enable(list, 9);
        list = dl_enable(list, 4);
        list = dl_tex_func(list, 0, 1);
        list = dl_set_tex_filter(list, 0, 0);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_disable(list, 1);
        id = cardinfo_card_id;
        list = cardinfo_draw_bg(list);
        list = card_panel_draw(list, 5, 4, cardinfo_panel);
        list = dl_tex_mode(list, 3, 0, 0, 0);
        list = dl_tex_func(list, 0, 1);
        list = dl_set_tex_filter(list, 0, 0);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = cardinfo_draw_window(list, 2, 0x76, 4, 0x165, 0x30);
        list = cardinfo_draw_frame(list, 0x7B, 8, 0x136, 0x1B);
        list = cardinfo_draw_window(list, 0, 0x76, 0x35, 0x165, 0xD9);
        list = cardinfo_draw_tabs(list, 3, 0xA4);
        if (card_is_monster(id) != 0) {
            list = cardlist_draw_stars(list, 0x1B5, 8, card_get_level(id), -1);
            list = card_draw_kind_icon(list, 0x1B2, 0x13, card_get_kind(id));
            list = card_draw_type_icon(list, 0x1C3, 0x13, card_get_type(id));
        } else {
            attr = card_get_attribute(id);
            list = card_draw_frame_special(list, 0x1B2, 0x13, attr);
            list = cardlist_draw_icon(list, 0x1C3, 0x13, attr, card_get_prop5_hi(id));
        }
        saved_flags = font_get_flags();
        font_set_flag4(1);
        if (cfg_get_language() == 0) {
            font_set_flag8000(1);
            font_set_flag1(1);
        } else {
            font_set_flag8(1);
        }
        font_set_color(0xFFFFFFFF);
        font_set_size(14, 14);
        font_set_line_extra(0x80);
        font_text_set(list);
        if (cfg_get_language() == 0) {
            font_draw_wtext(8000, 0x2C0, 0x4C80, card_get_name(id));
        } else {
            font_set_flag4000(1);
            font_draw_wtext(8000, 0x480, 0x4C80, card_get_name(id));
            font_set_flag4000(0);
        }
        font_set_color(0xFF000000);
        list = font_text_clear();
        list = dl_scissor(list, 0x7A, 0x48, 0x155, 0xC0);
        list = dl_enable(list, 2);
        switch (cardinfo_tab) {
        case 0:
            font_text_set(list);
            font_draw_wtext_ex(0x1F00, 0x1200 - cardinfo_scroll, 0x5300, cardinfo_scroll_max + 0x3000, card_get_desc(id), -1);
            list = font_text_clear();
            if (card_is_monster(id) != 0)
                title = sysmsg_get(0x10);
            else
                title = sysmsg_get(0x11);
            hint_r = sysmsg_get(0x12);
            hint_l = hint_r;
            if (cardinfo_flags.has_lists)
                hint_l = sysmsg_get(0x13);
            break;
        case 1:
            y = 0x48 - (cardinfo_scroll >> 6);
            for (i = 1; i < 0x23; i++) {
                if (card_has_genre_bit(id, i)) {
                    list = cardinfo_draw_genre_row(list, 0x80, y, 0x18, i);
                    y += 0x18;
                }
            }
            if (inv_get_bit3(cardinfo_card_id) && card_get_samepict(cardinfo_card_id) > 1)
                list = cardinfo_draw_genre_row(list, 0x80, y, 0x18, 0x23);
            title = sysmsg_get(0x12);
            if (card_is_monster(id) != 0)
                hint_l = sysmsg_get(0x10);
            else
                hint_l = sysmsg_get(0x11);
            hint_r = hint_l;
            if (cardinfo_flags.has_lists)
                hint_r = sysmsg_get(0x13);
            break;
        case 2:
            if (cardinfo_entry.count != 0) {
                line = (unsigned short *)((unsigned char *)cardinfo_index + cardinfo_entry.offset);
                y = 0x48 - (cardinfo_scroll >> 6);
                font_set_line_extra(0);
                for (i = 0; i < cardinfo_entry.count; i++) {
                    v = *line++;
                    pict = (v >> 12) & 0xF;
                    if (pict == inv_get_hi(cardinfo_card_id)) {
                        list = cardinfo_draw_list_entry(list, 0x7A, y, v & 0xFFF);
                        y = (font_get_cursor_y() >> 6) + 6;
                    }
                }
            }
            title = sysmsg_get(0x13);
            hint_l = sysmsg_get(0x12);
            if (card_is_monster(id) != 0)
                hint_r = sysmsg_get(0x10);
            else
                hint_r = sysmsg_get(0x11);
            break;
        }
        font_set_flags(saved_flags);
        list = dl_disable(list, 2);
        font_set_line_extra(0);
        font_set_size(12, 12);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        if (cardinfo_window_gim != 0) {
            list = dl_set_tex_filter(list, 0, 0);
            list = gim_set_texture(list, cardinfo_window_gim);
            list = draw_set_offsets(list, 8, 8, 8, 0xCA);
            list = draw_sprite(list, 0x1CE, 0x40, 0, 0x4F, 0x65, 0, 0);
        }
        range = (cardinfo_scroll_max >> 6) + 0xC0;
        pos = cardinfo_scroll >> 6;
        list = cardlist_draw_scrollbar(list, 0x1D0, 0x4E, 0xB4, pos * 0xB2 / range, ceil_to_int(34176.0f / (float)range));
        gs = gfx_get_state();
        list = cardlist_draw_arrow(list, 0x1D0, 0x46, 1, gfx_get_state()[1], cardinfo_scroll != 0, 0, gs[0]);
        gs = gfx_get_state();
        list = cardlist_draw_arrow(list, 0x1D0, 0x103, 0, gfx_get_state()[1],
                                   cardinfo_scroll < cardinfo_scroll_max, 0, gs[0]);
        if ((card_is_monster(id) == 0 || card_attribute_to_group(id) != 0) && (cls = card_get_type_class(id)) > 0)
            list = cardinfo_draw_type_row(list, 8, 0xCE, cls);
        n = 0;
        for (i = 1; i < 0x23; i++) {
            if (card_has_genre_bit(id, i)) {
                list = cardinfo_draw_genre_icon(list, (n % 6) * 0x11 + 9, (n / 6) * 0x11 + 0xAA, i);
                n++;
            }
        }
        if (inv_get_bit3(cardinfo_card_id) && card_get_samepict(cardinfo_card_id) > 1)
            list = cardinfo_draw_genre_icon(list, (n % 6) * 0x11 + 9, (n / 6) * 0x11 + 0xAA, 0x23);
        if (hint_l != 0 || hint_r != 0) {
            if (hint_l == hint_r) {
                list = cardinfo_draw_hint(list, 3, 0xFA, hint_l, 3);
            } else {
                if (hint_l != 0)
                    list = cardinfo_draw_hint(list, 3, 0xE5, hint_l, 1);
                if (hint_r != 0)
                    list = cardinfo_draw_hint(list, 3, 0xFA, hint_r, 2);
            }
        }
        if (cardinfo_flags.has_alt)
            list = cardinfo_draw_lr_arrows(list);
        list = cardinfo_draw_title_bar(list, 0x79, 0x38, 0x15E, title);
        font_text_set(list);
        if (cfg_get_language() != 0)
            font_set_flag4000(1);
        if (card_is_monster(id) == 0) {
            font_set_color(0xFF00FF00);
            font_draw_wtext(0x1F00, 0x900, 0x2200, card_info_fill(id));
            font_set_flag4000(0);
        } else {
            font_set_color(0xFF00FF00);
            font_draw_wtext(0x1F00, 0x900, 0x2300, card_info_fill(id));
            font_set_flag4000(0);
            font_set_color(0xFF00FFFF);
            font_draw_wtext(0x4300, 0x900, 0xE00, dlg_table_sel(card_get_kind(id)));
            cardinfo_draw_stat(0x14A, 0x24, 0, card_get_atk_q(id));
            cardinfo_draw_stat(0x195, 0x24, 1, card_get_def_q(id));
        }
        list = font_text_clear();
        if (cardinfo_view_state == 3 || cardinfo_view_state == 1)
            fade_apply(&list);
        dl_set_list(list);
    }
}

#endif

/* Enables the L/R picture switch when the card has alternate pictures the player has seen. */
int cardinfo_has_alt_pict(void)
{
    unsigned short n;

    if (inv_get_bit3(cardinfo_card_id) != 0 && (n = card_get_samepict(cardinfo_card_id)) > 1) {
        cardinfo_flags.has_alt = 1;
        cardinfo_n_pict = n;
        return 1;
    }
    return 0;
}

/* Deck-list tab data: box sheet, text pack and per-card index. */
void cardinfo_set_lists(void *list_gim, unsigned int *text_pack, unsigned short *index)
{
    unsigned short *e;

    if (list_gim != 0 && text_pack != 0 && index != 0) {
        cardinfo_list_gim = list_gim;
        cardinfo_text_pack = text_pack;
        cardinfo_index = index;
        cardinfo_flags.has_lists = 1;
        e = cardinfo_find_entry(cardinfo_card_id);
        if (e != 0)
            memcpy(&cardinfo_entry, e, 8);
    }
}

int cardinfo_is_active(void)
{
    return cardinfo_view_state == 2;
}

void cardinfo_close(void)
{
    fade_start_in(1, 0xFF000000, 16);
    cardinfo_view_state = 3;
}

void cardinfo_set_layer(unsigned short layer)
{
    cardinfo_layer = layer;
}

char cardinfo_window01_gim[] = "window01.gim";   /* .data 0x088F00E4; referenced by the asm of cardinfo_draw_window */

void *cardinfo_draw_bg(void *list)
{
    if (cardinfo_bg_gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, cardinfo_bg_gim);
        list = draw_set_offsets(list, 0x80, 0x80, 0x1E0, 0x110);
        list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    }
    return list;
}

#ifdef NON_MATCHING /* cardinfo_draw_window: y5/v5 allocation and x+5 rematerialisation; cardinfo_draw_frame: order of three argument moves */
/* 9-slice window from window01.gim (style = row of 24 px in the sheet). */
void *cardinfo_draw_window(void *list, unsigned int style, int x, int y, int w, int h)
{
    void *gim = ehp_find(syspack_get(), cardinfo_window01_gim, 0);
    int v = (style & 0xFFFF) * 0x18;
    int x1, y1, v2, x5, y5, v5;

    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 5, 5, 5, 5);
    list = draw_sprite(list, x, y, 0, 0, v, 0, 0);
    x1 = x + w - 5;
    list = draw_sprite(list, x1, y, 0, 0x1B, v, 0, 0);
    y1 = y + h - 5;
    v2 = v + 0xC;
    list = draw_sprite(list, x1, y1, 0, 0x1B, v2, 0, 0);
    list = draw_sprite(list, x, y1, 0, 0, v2, 0, 0);
    list = draw_set_offsets(list, 1, 5, w - 10, 5);
    x5 = x + 5;
    list = draw_sprite(list, x5, y, 0, 5, v, 0, 0);
    list = draw_sprite(list, x5, y1, 0, 5, v + 0xC, 0, 0);
    list = draw_set_offsets(list, 5, 1, 5, h - 10);
    y5 = y + 5;
    v5 = v + 5;
    list = draw_sprite(list, x, y5, 0, 0, v5, 0, 0);
    list = draw_sprite(list, x1, y5, 0, 0x1B, v5, 0, 0);
    list = draw_set_offsets(list, 1, 1, w - 10, h - 10);
    return draw_sprite(list, x + 5, y5, 0, 5, v5, 0, 0);
}

/* 9-slice frame from the window sheet (pieces at 0x33,0x65). */
void *cardinfo_draw_frame(void *list, int x, int y, int w, int h)
{
    int y1, x1, x5, y5;

    if (cardinfo_window_gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, cardinfo_window_gim);
    list = draw_set_offsets(list, 5, 5, 5, 5);
    list = draw_sprite(list, x, y, 0, 0x33, 0x65, 0, 0);
    x1 = x + w - 5;
    list = draw_sprite(list, x1, y, 0, 0x49, 0x65, 0, 0);
    y1 = y + h - 5;
    list = draw_sprite(list, x1, y1, 0, 0x49, 0x7B, 0, 0);
    list = draw_sprite(list, x, y1, 0, 0x33, 0x7B, 0, 0);
    list = draw_set_offsets(list, 1, 5, w - 10, 5);
    x5 = x + 5;
    list = draw_sprite(list, x5, y, 0, 0x38, 0x65, 0, 0);
    list = draw_sprite(list, x5, y1, 0, 0x38, 0x7B, 0, 0);
    list = draw_set_offsets(list, 5, 1, 5, h - 10);
    y5 = y + 5;
    list = draw_sprite(list, x, y5, 0, 0x33, 0x6A, 0, 0);
    list = draw_sprite(list, x1, y5, 0, 0x49, 0x6A, 0, 0);
    list = draw_set_offsets(list, 1, 1, w - 10, h - 10);
    return draw_sprite(list, x5, y5, 0, 0x38, 0x6A, 0, 0);
}

#endif

/* Tab strip: left cap, stretched middle, right cap. */
void *cardinfo_draw_tabs(void *list, int x, int y)
{
    if (cardinfo_window_gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, cardinfo_window_gim);
    list = draw_set_offsets(list, 8, 0x3B, 8, 0x3B);
    list = draw_sprite(list, x, y, 0, 0x58, 0x45, 0, 0);
    list = draw_sprite(list, x + 0x69, y, 0, 0x78, 0x45, 0, 0);
    list = draw_set_offsets(list, 1, 0x3B, 0x61, 0x3B);
    return draw_sprite(list, x + 8, y, 0, 0x60, 0x45, 0, 0);
}

/* Height of the current tab's content, in 26.6, minus the visible area. */
void cardinfo_calc_scroll(void)
{
    unsigned short saved_flags;
    int h = 0;
    int i, pict;
    unsigned short v;
    unsigned short *line;

    saved_flags = font_get_flags();
    font_set_flag4(1);
    if (cfg_get_language() == 0) {
        font_set_flag8000(1);
        font_set_flag1(1);
    } else {
        font_set_flag8(1);
    }
    font_set_size(14, 14);
    font_set_line_extra(0x80);
    switch (cardinfo_tab) {
    case 0:
        h = font_wtext_lines(0x5300, card_get_desc(cardinfo_card_id));
        break;
    case 1:
        for (i = 1; i < 0x23; i++) {
            if (card_has_genre_bit(cardinfo_card_id, i))
                h += 0x600;
        }
        if (inv_get_bit3(cardinfo_card_id) && card_get_samepict(cardinfo_card_id) > 1)
            h += 0x600;
        break;
    case 2:
        if ((int)cardinfo_entry.count > 0) {
            line = (unsigned short *)((unsigned char *)cardinfo_index + cardinfo_entry.offset);
            font_set_line_extra(0);
            for (i = 0; i < cardinfo_entry.count; i++) {
                v = *line++;
                pict = (v >> 12) & 0xF;
                if (pict == inv_get_hi(cardinfo_card_id)) {
                    h += 2;
                    h += font_wtext_lines(0x4D40, cardinfo_get_text(v & 0xFFF)) >> 6;
                    h += 6;
                }
            }
            h -= 6;
        }
        h *= 0x40;
        break;
    }
    font_set_flags(saved_flags);
    font_set_line_extra(0);
    if (h - 0x3000 > 0)
        cardinfo_scroll_max = h - 0x3000;
    else
        cardinfo_scroll_max = 0;
    cardinfo_scroll = 0;
}

/* "ATK/" or "DEF/" followed by the value. */
void cardinfo_draw_stat(int x, int y, int is_def, int value)
{
    unsigned short atk[5] = { 'A', 'T', 'K', '/', 0 };
    unsigned short def[5] = { 'D', 'E', 'F', '/', 0 };
    unsigned short *s;

    if (is_def != 0)
        s = def;
    else
        s = atk;
    font_set_color(0xFFFFFFFF);
    font_set_size(12, 12);
    font_draw_text(x << 6, y << 6, (0x1E0 - x) * 0x40, s);
    cardinfo_draw_number(x + 0xE, y, value);
}

/* Right-aligned decimal (0..9999) ending at x + 40; "?" for 0xFFFF. */
void cardinfo_draw_number(int x, int y, int value)
{
    int px;

    font_set_size(12, 12);
    if (value == 0xFFFF) {
        unsigned short q[2] = { '?', 0 };
        font_draw_text((x + 0x28) * 0x40, y << 6, (0x1E0 - (x + 0x28)) * 0x40, q);
    } else {
        if (value < 0)
            value = 0;
        else if (value > 9999)
            value = 9999;
        x += 0x28;
        px = x * 0x40;
        do {
            char digit[4] = { 0 };
            digit[0] = value % 10 + '0';
            font_draw_text(px, y << 6, (0x1E0 - x) * 0x40, (unsigned short *)digit);
            px -= 0x200;
            value /= 10;
            x -= 8;
        } while (value > 0);
    }
}

/* Title bar: caps + stretched middle (row 0x73 of the window sheet) with centred text. */
void *cardinfo_draw_title_bar(void *list, int x, int y, int w, unsigned short *text)
{
    if (cardinfo_window_gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, cardinfo_window_gim);
        list = draw_set_offsets(list, 4, 0xD, 4, 0xD);
        list = draw_sprite(list, x, y, 0, 1, 0x73, 0, 0);
        list = draw_sprite(list, x + w - 4, y, 0, 0xD, 0x73, 0, 0);
        list = draw_set_offsets(list, 4, 0xD, w - 8, 0xD);
        list = draw_sprite(list, x + 4, y, 0, 5, 0x73, 0, 0);
    }
    font_set_size(12, 12);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    font_draw_text((x + 3) * 0x40, (y + 1) * 0x40, (w - 3) * 0x40, text);
    return font_text_clear();
}

/* "Type:" label (sysmsg 0x14) and the type-class icon. */
void *cardinfo_draw_type_row(void *list, int x, int y, int type_class)
{
    void *dl;

    font_text_set(list);
    font_set_size(12, 12);
    font_set_color(0xFF000000);
    font_draw_wtext(0x200, (y + 1) * 0x40, 0x1400, sysmsg_get(0x14));
    dl = font_text_clear();
    if (cardinfo_window_gim != 0) {
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
        dl = dl_set_tex_filter(dl, 0, 0);
        dl = gim_set_texture(dl, cardinfo_window_gim);
        dl = draw_set_offsets(dl, 0x16, 10, 0x16, 10);
        dl = draw_sprite(dl, x + 0x50, y + 1, 0, 0, (type_class - 1) * 0xC + 0x50, 0, 0);
    }
    return dl;
}

/* Genre row of the genre tab: icon centred in h plus its name. */
void *cardinfo_draw_genre_row(void *list, int x, int y, int h, int genre)
{
    int iy, ty, d;

    if (cfg_get_language() == 0)
        iy = y + h - 0x10;
    else
        iy = y + (h - 0x10) / 2;
    list = cardinfo_draw_genre_icon(list, x, iy, genre);
    font_text_set(list);
    font_set_color(0xFF000000);
    font_set_size(14, 14);
    if (cfg_get_language() == 0) {
        ty = (y + h - 1) - (font_line_height() >> 6);
    } else {
        d = h - (font_line_height() >> 6);
        ty = y + d / 2;
    }
    font_draw_text((x + 0x14) * 0x40, ty << 6, (0x1E0 - (x + 0x14)) * 0x40, dlg_get_text_bounded(genre));
    return font_text_clear();
}

/* 16x16 genre icon from the window sheet (8 per row). */
void *cardinfo_draw_genre_icon(void *list, int x, int y, int genre)
{
    if (cardinfo_window_gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, cardinfo_window_gim);
        list = draw_set_offsets(list, 0x10, 0x10, 0x10, 0x10);
        list = draw_sprite(list, x, y, 0, (genre % 8) << 4, (genre / 8) << 4, 0, 0);
    }
    return list;
}

#ifdef NON_MATCHING /* s-register assignment: the original keeps list/x/y/text_id in s4..s1 and re-extends text_id at every call */
/* One deck-list entry: box sized to the text, kind icon and the text. */
void *cardinfo_draw_list_entry(void *list, int x, int y, unsigned short text_id)
{
    unsigned short id = text_id;         /* a u16 local is re-extended at every use, a parameter is not */
    unsigned short ty = 3;
    int lines, kind;

    font_set_color(0xFF000000);
    font_set_size(14, 14);
    if (cardinfo_list_gim != 0) {
        lines = font_wtext_lines(0x4D40, cardinfo_get_text(id));
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, cardinfo_list_gim);
        list = cardinfo_draw_list_box(list, x, y, lines / font_line_height());
        kind = cardinfo_get_text_kind(id) & 0xFF;
        if (kind < 5) {
            list = draw_set_offsets(list, 0x11, 0x13, 0x11, 0x13);
            list = draw_sprite(list, x + 4, y + 5, 0, kind * 0x11, 0x2D, 0, 0);
        }
    }
    font_text_set(list);
    if (cfg_get_language() != 0) {
        ty += 3;
        font_set_line_extra(0x1C0);
    }
    font_draw_wtext_ex((x + 0x17) * 0x40, (y + ty) * 0x40, 0x4D40, 0x7800, cardinfo_get_text(id), -1);
    if (cfg_get_language() != 0)
        font_set_line_extra(0);
    return font_text_clear();
}

#endif

/* Box with `lines` rows: top row 0x16 high, middle rows 0x15, bottom 5. */
void *cardinfo_draw_list_box(void *list, int x, int y, int lines)
{
    int i, ry;

    list = draw_set_offsets(list, 0x18, 0x16, 0x18, 0x16);
    list = draw_sprite(list, x, y, 0, 0x30, 0, 0, 0);
    list = draw_set_offsets(list, 8, 0x16, 8, 0x16);
    list = draw_sprite(list, x + 0x14B, y, 0, 0x50, 0, 0, 0);
    list = draw_set_offsets(list, 1, 0x16, 0x133, 0x16);
    list = draw_sprite(list, x + 0x18, y, 0, 0x48, 0, 0, 0);
    for (i = 1; i < lines; i++) {
        ry = y + i * 0x16;                 /* strength-reduced by the compiler: preheader y + 0x16, += 0x16 */
        list = draw_set_offsets(list, 0x18, 0x15, 0x18, 0x15);
        list = draw_sprite(list, x, ry, 0, 0x58, 0x16, 0, 0);
        list = draw_set_offsets(list, 8, 0x15, 8, 0x15);
        list = draw_sprite(list, x + 0x14B, ry, 0, 0x78, 0x16, 0, 0);
        list = draw_set_offsets(list, 1, 0x15, 0x133, 0x15);
        list = draw_sprite(list, x + 0x18, ry, 0, 0x70, 0x16, 0, 0);
    }
    list = draw_set_offsets(list, 0x18, 5, 0x18, 5);
    ry = y + 0x16 + (lines - 1) * 0x15;
    list = draw_sprite(list, x, ry, 0, 0x30, 0x16, 0, 0);
    list = draw_set_offsets(list, 8, 5, 8, 5);
    list = draw_sprite(list, x + 0x14B, ry, 0, 0x50, 0x16, 0, 0);
    list = draw_set_offsets(list, 1, 5, 0x133, 5);
    return draw_sprite(list, x + 0x18, ry, 0, 0x48, 0x16, 0, 0);
}

/* Binary search of the index ({count, entries[count]{id, count, offset}}) for a card id. */
unsigned short *cardinfo_find_entry(unsigned short id)
{
    unsigned short *tab;
    int lo = 0, hi, mid;

    hi = cardinfo_index[0] - 1;
    tab = cardinfo_index + 1;
    while (lo <= hi) {
        mid = (lo + hi) / 2;
        if (id == tab[mid * 4])
            return &tab[mid * 4];
        if (tab[mid * 4] < id)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 0;
}

unsigned short *cardinfo_get_text(unsigned int id)
{
    unsigned int *p = cardinfo_text_pack;
    unsigned int *tbl;
    unsigned char *data;

    if (p != 0) {
        tbl = (unsigned int *)((unsigned char *)p + p[1]);
        data = (unsigned char *)p + p[2];
        if ((id & 0xFFFF) < p[0])
            return (unsigned short *)(data + (tbl[id & 0xFFFF] & 0x0FFFFFFF));
    }
    return 0;
}

unsigned int cardinfo_get_text_kind(unsigned int id)
{
    unsigned int *p = cardinfo_text_pack;
    unsigned int *tbl;

    if (p != 0) {
        tbl = (unsigned int *)((unsigned char *)p + p[1]);
        if ((id & 0xFFFF) < p[0])
            return (tbl[id & 0xFFFF] >> 28) & 0xF;
    }
    return 0;
}

/* Button hint box: small window, optional left/right arrows (bobbing with the frame counter) and centred text. */
void *cardinfo_draw_hint(void *list, int x, int y, unsigned short *text, unsigned char arrows)
{
    unsigned char bob[4] = { 0, 1, 2, 1 };
    unsigned int *gs;
    unsigned short off;
    unsigned char sz;

    list = cardinfo_draw_window(list, 2, x, y, 0x71, 0x14);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, cardinfo_window_gim);
    list = draw_set_offsets(list, 7, 0xD, 7, 0xD);
    gs = gfx_get_state();
    off = bob[(gs[0] >> 2) & 3];
    if (arrows & 1)
        list = draw_sprite(list, x + 3 + off, y + 3, 0, 0x11, 0x73, 0, 0);
    if (arrows & 2)
        list = draw_sprite(list, x + 0x66 - off, y + 3, 0, 0x19, 0x73, 0, 0);
    font_set_color(0xFFFFFFFF);
    font_set_size(12, 12);
    sz = font_measure_wtext(0x1640, text);
    font_set_size(sz, 12);
    font_text_set(list);
    x += (0x71 - ((font_measure_text(text) >> 6) + 1)) / 2;
    font_draw_text(x * 0x40, (y + 4) * 0x40, (0x1E0 - x) * 0x40, text);
    return font_text_clear();
}

/* L / R button icons at the top when the picture can be switched. */
void *cardinfo_draw_lr_arrows(void *list)
{
    if (cardinfo_window_gim != 0 && ((gfx_get_state()[0] >> 3) & 1)) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, cardinfo_window_gim);
        list = draw_set_offsets(list, 0x17, 0xB, 0x17, 0xB);
        list = draw_sprite(list, 4, 3, 0, 0x41, 0x4F, 0, 0);
        list = draw_sprite(list, 0x5C, 3, 0, 0x41, 0x5A, 0, 0);
    }
    return list;
}
