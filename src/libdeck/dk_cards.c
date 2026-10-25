// @range 0x0880F1E4-0x088103A0
// @asm 0x0880F284-0x0880F3BC
// @asm 0x0880F5E4-0x0880FC6C
// @asm 0x0880FFC4-0x088103A0
// @data 0x08822EF0-0x08822F14
/* The cards of the two panes, as a grid of pictures or as rows (picture, name, kind/attribute
   icons, level, ATK/DEF and the copies in the decks), and the strip under the panes with the card
   under the cursor (name, icons, stats and the description scrolling sideways). */
#include "libdeck.h"

#define PT(x, y) ((unsigned short)(x) | (unsigned short)(y) << 16)
#define DK_ENT(l, i) ((i) >= (l)->count ? 0 : &(l)->ents[(l)->order[i]])

typedef struct DkRowCounter {          /* dk_draw_counters, 12 bytes */
    unsigned char icon;                /* row of the counter icons, 0 = none */
    unsigned char pad01[3];
    unsigned int color;                /* +0x04 */
    unsigned char value;               /* +0x08 */
    unsigned char pad09[3];
} DkRowCounter;

typedef struct DkRowCounters {         /* copied as one struct */
    DkRowCounter c[3];
} DkRowCounters;

unsigned short *card_get_desc(int id);
unsigned short *card_info_fill(int id);
void *cardlist_build(void *list, DkScroll *s);
void font_text_set(void *list);
void *font_text_clear(void);
void font_set_flag4000(int on);
int font_draw_wtext(int x, int y, int w, unsigned short *text);
int inv_get_state(unsigned short id);
void *dk_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type);
void *dk_draw_attr_icon(void *list, unsigned short x, unsigned short y, unsigned short attr);
void *dk_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind);
void *dk_draw_card_pict(void *list, int x, int y, int shadow, unsigned int color, DkEnt *e, void *gim, void *gim_alt, void *alt);
void *dk_draw_card_marks(void *list, int x, int y, DkEnt *e, int show_count, int blink, int show_fixed, unsigned short rank, void *gim_digits, void *gim);
void *dk_draw_row_frame(void *list, int pos, unsigned short w, unsigned short kind, void *gim);
void *dk_draw_flag_icon(void *list, int pos, unsigned short kind, void *gim);
void *dk_draw_counters(void *list, int pos, DkRowCounter *c, void *gim_strip, void *gim);
void *dk_draw_level(void *list, int pos, unsigned short level, unsigned int color, void *gim);
void *dk_draw_stat(void *list, unsigned int st, int pos, unsigned short size, unsigned int color, void *gim);
int dk_list_count_fixed(DkList *l, unsigned short id, unsigned char *out);
void *dk_draw_rows(void *list, DkPane *p);
void *dk_draw_grid(void *list, DkPane *p);
void *dk_draw_row(void *list, unsigned short x, unsigned short y, unsigned short w, DkEnt *e, int show);
void *dk_draw_rows_small(void *list, DkPane *p);
void *dk_draw_row_small(void *list, unsigned short x, unsigned short y, unsigned short w, DkEnt *e, int show);

void *dk_draw_panes(void *list)
{
    DkPane *p = &dkv.pane[0];

    if (p->list_mode == 0)
        list = dk_draw_grid(list, p);
    else
        list = dk_draw_rows(list, p);
    p = &dkv.pane[1];
    if (dkv.mode > 1) {
        if (p->list_mode == 0)
            list = dk_draw_grid(list, p);
        else if (dkv.mode < 4)
            list = dk_draw_rows_small(list, p);
        else
            list = dk_draw_rows(list, p);
    }
    return list;
}

#ifdef NON_MATCHING /* o original carrega a largura da linha depois da junção do ternário; o mwcc atual a sobe para o atraso do desvio (como sc_draw_rows) */
/* list mode: one card per row (the copies are not shown for the rented list) */
void *dk_draw_rows(void *list, DkPane *p)
{
    DkList *l = p->list;
    DkLayout *lay = &p->lay;
    int show = 1;
    unsigned short y;
    unsigned short x;
    unsigned short idx;
    unsigned short n;
    int i;
    DkEnt *e;

    if (l->tab == 5)
        show = 0;
    idx = p->top;
    x = lay->x + lay->row_x;
    y = lay->y + lay->row_y;
    n = lay->rows;
    if (n + idx >= l->count)
        n = l->count - idx;
    for (i = 0; i < n; i++) {
        e = DK_ENT(l, idx);
        list = dk_draw_row(list, x, y, lay->row_w, e, show);
        idx++;
        y += lay->row_h;
    }
    return list;
}
#endif

/* grid mode: card pictures with their marks */
void *dk_draw_grid(void *list, DkPane *p)
{
    DkList *l = p->list;
    DkLayout *lay = &p->lay;
    int show = 1;
    unsigned short y;
    int row;
    unsigned short x;
    DkEnt *e;
    unsigned short idx;
    int col;

    if (l->tab == 5)
        show = 0;
    y = lay->y + lay->grid_y;
    idx = p->top;
    for (row = 0; row < lay->grid_rows; row++) {
        if (idx >= l->count)
            break;
        x = lay->x + lay->grid_x;
        for (col = 0; col < lay->cols; col++) {
            e = DK_ENT(l, idx);
            list = dk_draw_card_pict(list, x, y, 1, 0xFFFFFFFF, e, dk_res_get(0x8001), dk_res_get(0x8002), dk_res_get(0x8032));
            list = dk_draw_card_marks(list, x, y, e, 1, inv_get_state(e->id) == 1, show, 0, dk_res_get(0x8009), dk_res_get(0x800A));
            x += lay->cell_w;
            idx++;
            if (idx >= l->count)
                break;
        }
        y += lay->cell_h;
    }
    return list;
}

/* the three counters of a row: copies in the deck / side / extra (blue when rented to the partner) */
DkRowCounters dk_row_counters = { { { 1, { 0 }, 0xFF000000, 0 }, { 2, { 0 }, 0xFF000000, 0 }, { 4, { 0 }, 0xFF000000, 0 } } };

#ifdef NON_MATCHING /* o modelo dos contadores é copiado com lwc1/swc1 como no original, mas a alocação de s/pilha difere (como sc_draw_row) */
/* a wide row: frame, picture, name, icons, level, ATK/DEF, then the counters */
void *dk_draw_row(void *list, unsigned short x, unsigned short y, unsigned short w, DkEnt *e, int show)
{
    int full = 0;
    unsigned short attr;
    unsigned short tx;
    unsigned short type;
    unsigned short kind;
    unsigned short atk;
    unsigned short def;
    unsigned short level;
    unsigned short cx;

    attr = card_get_attribute_icon(e->id);
    list = dk_draw_row_frame(list, PT(x + 23, y), w, attr, dk_res_get(0x8006));
    list = dk_draw_card_pict(list, x, y, 1, 0xFFFFFFFF, e, dk_res_get(0x8001), dk_res_get(0x8002), dk_res_get(0x8032));
    tx = x + 29;
    font_text_set(list);
    font_set_size(12, 12);
    font_set_color(0xFF000000);
    if (cfg_get_language() != 0) {
        font_set_flag4000(1);
        font_set_flag8(0);
    }
    font_draw_text(tx << 6, (y + 2) << 6, (w - 54) << 6, card_get_name(e->id));
    if (cfg_get_language() != 0) {
        font_set_flag4000(0);
        font_set_flag8(1);
    }
    list = font_text_clear();
    if (attr - 7 < 2u) {
        type = card_get_prop5_hi(e->id);
        kind = card_get_kind(e->id);
        list = dk_draw_kind_icon(list, tx, y + 14, kind);
        list = dk_draw_attr_icon(list, tx + 18, y + 14, type);
    } else {
        type = card_get_type(e->id);
        kind = card_get_kind(e->id);
        atk = card_get_atk_q(e->id);
        def = card_get_def_q(e->id);
        level = card_get_level(e->id);
        list = dk_draw_kind_icon(list, tx, y + 14, kind);
        list = dk_draw_type_icon(list, tx + 18, y + 14, type);
        list = dk_draw_level(list, PT(tx + 37, y + 16), level, 0xFF000000, dk_res_get(0x8009));
        list = dk_draw_stat(list, atk << 16, PT(tx + 73, y + 16), 0xC0A, 0xFF000000, dk_res_get(0x8009));
        list = dk_draw_stat(list, def << 16 | 1, PT(tx + 133, y + 16), 0xC0A, 0xFF000000, dk_res_get(0x8009));
    }
    cx = tx + 193;
    if (x + w < cx + 50) {
        full = 1;
    } else {
        DkRowCounters c = dk_row_counters;

        if (card_attribute_icon_is3(e->id)) {
            c.c[0].icon = 0;
            c.c[1].icon = 3;
            if (dk_list_count_fixed(&dk_lists[1], e->id, &c.c[1].value))
                c.c[1].color = 0xFF0080FF;
        } else {
            if (dk_list_count_fixed(&dk_lists[0], e->id, &c.c[0].value))
                c.c[0].color = 0xFF0080FF;
            if (dk_list_count_fixed(&dk_lists[2], e->id, &c.c[1].value))
                c.c[1].color = 0xFF0080FF;
        }
        c.c[2].value = dk_list_count_id(&dk_lists[3], e->id, 0);
        list = dk_draw_counters(list, PT(cx + 7, y + 14), c.c, dk_res_get(0x8006), dk_res_get(0x800A));
    }
    if (inv_get_flag7(e->id))
        list = dk_draw_flag_icon(list, PT(x + w - 24, y + 3), 2, dk_res_get(0x800A));
    return dk_draw_card_marks(list, x, y, e, full, inv_get_state(e->id) == 1, show, 0, dk_res_get(0x8009), dk_res_get(0x800A));
}
#endif

#ifdef NON_MATCHING /* mesmo problema de dk_draw_rows */
/* small rows of the second pane (modes 2 and 3) */
void *dk_draw_rows_small(void *list, DkPane *p)
{
    DkList *l = p->list;
    DkLayout *lay = &p->lay;
    int show = 1;
    unsigned short y;
    unsigned short x;
    unsigned short idx;
    unsigned short n;
    int i;
    DkEnt *e;

    if (l->tab == 5)
        show = 0;
    idx = p->top;
    x = lay->x + lay->row_x;
    y = lay->y + lay->row_y;
    n = lay->rows;
    if (n + idx >= l->count)
        n = l->count - idx;
    for (i = 0; i < n; i++) {
        e = DK_ENT(l, idx);
        list = dk_draw_row_small(list, x, y, lay->row_w, e, show);
        idx++;
        y += lay->row_h;
    }
    return list;
}
#endif

void *dk_draw_row_small(void *list, unsigned short x, unsigned short y, unsigned short w, DkEnt *e, int show)
{
    unsigned short attr;
    unsigned short atk;
    unsigned short def;
    unsigned short level;
    unsigned short tx;

    attr = card_get_attribute_icon(e->id);
    list = dk_draw_row_frame(list, PT(x + 26, y), w, attr, dk_res_get(0x8006));
    list = dk_draw_card_pict(list, x, y + 1, 1, 0xFFFFFFFF, e, dk_res_get(0x8001), dk_res_get(0x8002), dk_res_get(0x8032));
    list = dk_draw_card_marks(list, x, y + 1, e, 1, inv_get_state(e->id) == 1, show, 0, dk_res_get(0x8009), dk_res_get(0x800A));
    if (cfg_get_language() != 0)
        font_set_flag8(0);
    font_text_set(list);
    font_set_size(12, 12);
    font_set_color(0xFF000000);
    if (cfg_get_language() != 0) {
        font_set_flag4000(1);
        font_set_flag8(0);
    }
    font_draw_text((x + 27) << 6, y << 6, (w - 27) << 6, card_get_name(e->id));
    if (cfg_get_language() != 0) {
        font_set_flag4000(0);
        font_set_flag8(1);
    }
    list = font_text_clear();
    if (cfg_get_language() != 0)
        font_set_flag8(1);
    if (card_is_monster(e->id)) {
        atk = card_get_atk_q(e->id);
        def = card_get_def_q(e->id);
        level = card_get_level(e->id);
        tx = x + 25;
        list = dk_draw_stat(list, atk << 16 | 2, PT(tx, y + 13), 0xA09, 0xFF000000, dk_res_get(0x8009));
        list = dk_draw_stat(list, def << 16 | 3, PT(tx, y + 23), 0xA09, 0xFF000000, dk_res_get(0x8009));
        list = dk_draw_level(list, PT(x + 69, y + 20), level, 0xFF000000, dk_res_get(0x8009));
    }
    if (inv_get_flag7(e->id))
        list = dk_draw_flag_icon(list, PT(x + 78, y + 13), 2, dk_res_get(0x800A));
    return list;
}

#ifdef NON_MATCHING /* lista do parâmetro e do resultado em s trocados (como sc_draw_info) */
/* the strip under the panes: the card list's own strip in one-pane modes, else name, icons, level,
   ATK/DEF and the description scrolling sideways */
void *dk_draw_info(void *list, DkCursor *c)
{
    void *r;
    int x;
    int w;
    int x2;
    unsigned short level;

    if (dkv.mode < 4) {
        r = cardlist_build(list, &dkv.cursor.scroll);
        if (c->id == 0)
            return r;
        if (inv_get_flag7(c->id))
            r = dk_draw_flag_icon(r, PT(0x7B, 0x87), 2, dk_res_get(0x800A));
        return r;
    }
    if (c->id == 0)
        return list;
    r = dk_draw_kind_icon(list, 0x146, 0x100, card_get_kind(c->id));
    if (card_is_monster(c->id)) {
        r = dk_draw_type_icon(r, 0x158, 0x100, card_get_type(c->id));
        level = card_get_level(c->id);
        r = dk_draw_level(r, PT(0x16B, 0x103), level, 0xFFFFFFFF, dk_res_get(0x8009));
        r = dk_draw_stat(r, (unsigned short)card_get_atk_q(c->id) << 16 | 2, PT(0x187, 0x102), 0xC0A, 0xFFFFFFFF, dk_res_get(0x8009));
        r = dk_draw_stat(r, (unsigned short)card_get_def_q(c->id) << 16 | 3, PT(0x1B3, 0x102), 0xC0A, 0xFFFFFFFF, dk_res_get(0x8009));
    } else if (card_get_prop5_hi(c->id) != 0) {
        r = dk_draw_attr_icon(r, 0x158, 0x100, card_get_prop5_hi(c->id));
    }
    if (inv_get_flag7(c->id))
        r = dk_draw_flag_icon(r, PT(0x1CE, 0xF5), 2, dk_res_get(0x800A));
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
    x = 0x180 - dkv.scroll_pos;
    w = dkv.scroll_pos + 0x5500;
    font_draw_text(x, 0x4080, w, card_get_desc(c->id));
    if (dkv.scroll_max > 0) {
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
