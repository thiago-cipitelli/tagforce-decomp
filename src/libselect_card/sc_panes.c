// @range 0x08808040-0x088099CC
// @asm 0x08808040-0x08808694
// @asm 0x08808694-0x088088B4
// @asm 0x08808A1C-0x08808D88
// @asm 0x08808D88-0x08809638
// @data 0x0881AE28-0x0881AE58
/* The two card panes: header icons (sort, direction, filter and its count, the "new" mark), the
   scroll bars, the copy counts of the tabs, and the cards themselves, as a grid of pictures or as
   rows (picture, name, kind/attribute icons, level, ATK/DEF and the rental counters). */
#include "libselect_card.h"

unsigned char cfg_get_byte0_hi2(void);
void font_set_flag4000(int on);
void *sc_draw_sort_icon(void *list, int sort, int x, int y);
void *sc_draw_dir_icon(void *list, int dir, int x, int y);
void *sc_draw_filter_icon(void *list, int filter, int x, int y);
unsigned short sc_tab_rect(unsigned short tab, unsigned short sel, ScRect *r);
int sc_list_count_id(ScList *l, short id, int same_name);
void *sc_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type);
void *sc_draw_attr_icon(void *list, unsigned short x, unsigned short y, unsigned short attr);
void *sc_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind);
void *sc_draw_counts(void *list);
void *sc_draw_count(void *list, unsigned short x, ScRect *r, unsigned short ndig, int value);
void *sc_draw_rows(void *list, ScPane *p);
void *sc_draw_grid(void *list, ScPane *p);
void *sc_draw_row(void *list, unsigned short x, unsigned short y, unsigned short w, ScEnt *e);
void *sc_draw_rows_small(void *list, ScPane *p);
void *sc_draw_row_small(void *list, unsigned short x, unsigned short y, unsigned short w, ScEnt *e);

#ifdef NON_MATCHING /* o original guarda o id do recurso (0x8007) num u16 na pilha e o relê no segundo painel; a alocação dos s difere */
/* header of each pane: sort and direction icons, filter icon with the number of cards, "new"
   mark of the owned list, scroll bar */
void *sc_draw_headers(void *list)
{
    ScPane *p;
    ScList *l;
    ScLayout *lay;
    int x;
    int y;
    int show;
    unsigned char mark;
    int style;
    unsigned short id;
    unsigned short cols;

    p = &scv.pane[0];
    l = p->list;
    lay = &p->lay;
    x = p->lay.x + 3;
    y = p->lay.y + p->lay.h - 18;
    list = sc_draw_sort_icon(list, l->sort, x, y);
    list = sc_draw_dir_icon(list, l->dir, x + 18, y);
    if (l->filter != 0) {
        show = 1;
        if (scv.popup == 4 && scv.side == 0 && (gfx_get_state()[0] >> 2 & 1))
            show = 0;
        if (show) {
            list = sc_draw_filter_icon(list, l->filter, x + 37, y);
            x += 56;
            if (l->count < 10)
                x -= 21;
            else if (l->count < 100)
                x -= 14;
            else if (l->count < 1000)
                x -= 7;
            list = sc_draw_digits(list, PT(x, y + 4), 0, l->count, 4, 0xFFFFFFFF, res_get(0x8009));
        }
    }
    x = lay->x + lay->w - 20;
    show = 0;
    switch (sc.kind) {
    case 1:
        if (l->tab == 0)
            show = 1;
        break;
    case 2:
        if (l->tab == 0)
            show = 1;
        break;
    }
    if (show && (mark = cfg_get_byte0_hi2()) != 0)
        list = sc_draw_flag_icon(list, PT(x, y + 3), mark, res_get(0x800A));
    style = scv.side != 0 ? 1 : 0;
    if (l->tab > 0)
        style += 2;
    if (p->list_mode == 0) {
        id = 0x8007;
        cols = lay->cols;
        list = sc_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top / cols, lay->grid_rows,
                                 (l->count + cols - 1) / cols, style, res_get(id));
    } else {
        id = 0x8007;
        list = sc_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top, lay->rows, l->count, style,
                                 res_get(id));
    }
    if (scv.mode > 1) {
        p = &scv.pane[1];
        l = p->list;
        lay = &p->lay;
        x = p->lay.x + 3;
        y = p->lay.y + p->lay.h - 18;
        list = sc_draw_sort_icon(list, l->sort, x, y);
        list = sc_draw_dir_icon(list, l->dir, x + 18, y);
        if (l->filter != 0) {
            show = 1;
            if (scv.popup == 4 && scv.side == 1 && (gfx_get_state()[0] >> 2 & 1))
                show = 0;
            if (show) {
                list = sc_draw_filter_icon(list, l->filter, x + 37, y);
                x += 56;
                if (l->count < 10)
                    x -= 21;
                else if (l->count < 100)
                    x -= 14;
                else if (l->count < 1000)
                    x -= 7;
                list = sc_draw_digits(list, PT(x, y + 4), 0, l->count, 4, 0xFFFFFFFF, res_get(0x8009));
            }
        }
        x = lay->x + lay->w - 20;
        show = 0;
        switch (sc.kind) {
        case 1:
            if (l->tab == 0)
                show = 1;
            break;
        case 2:
            if (l->tab == 0)
                show = 1;
            break;
        }
        if (show && (mark = cfg_get_byte0_hi2()) != 0)
            list = sc_draw_flag_icon(list, PT(x, y + 3), mark, res_get(0x800A));
        style = scv.side == 1 ? 2 : 3;
        if (p->list_mode == 0) {
            cols = lay->cols;
            list = sc_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top / cols, lay->grid_rows,
                                     (l->count + cols - 1) / cols, style, res_get(id));
        } else {
            list = sc_draw_scrollbar(list, lay->x + lay->w - 10, lay->y + 12, lay->h - 41, p->top, lay->rows, l->count,
                                     style, res_get(id));
        }
    }
    return sc_draw_counts(list);
}
#endif

/* digits of the copy count on each tab */
unsigned short sc_count_digits[3][2] = { { 4, 1 }, { 6, 2 }, { 6, 1 } };

#ifdef NON_MATCHING /* o original mantém o 2 do teste de sc.kind num registrador s como limite dos laços */
/* the total of copies on the tab buttons of each pane */
void *sc_draw_counts(void *list)
{
    ScPane *p = &scv.pane[0];
    unsigned short sel = p->list->tab;
    int n = 2;
    unsigned short *d;
    int i;
    unsigned short x;
    ScRect r;

    switch (sc.kind) {
    case 0:
        d = sc_count_digits[0];
        break;
    case 1:
        d = sc_count_digits[1];
        break;
    case 2:
        d = sc_count_digits[2];
        break;
    default:
        return list;
    }
    if (scv.mode < 2) {
        for (i = 0; i < n; i++) {
            x = sc_tab_rect(i, sel, &r);
            r.x += p->lay.x;
            list = sc_draw_count(list, x + p->lay.x, &r, d[i], sc.list[i].total);
        }
    } else {
        i = 0;
        x = sc_tab_rect(i, sel, &r);
        r.x += p->lay.x;
        list = sc_draw_count(list, x + p->lay.x, &r, d[i], sc.list[i].total);
        p = &scv.pane[1];
        sel = p->list->tab;
        for (i++; i < n; i++) {
            x = sc_tab_rect(i, sel, &r);
            r.x += p->lay.x;
            list = sc_draw_count(list, x + p->lay.x, &r, d[i], sc.list[i].total);
        }
    }
    return list;
}

#endif

void *sc_draw_count(void *list, unsigned short x, ScRect *r, unsigned short ndig, int value)
{
    if (r->w <= 0 || r->h <= 0)
        return list;
    list = dl_scissor(list, r->x, r->y, r->w, r->h);
    list = dl_enable(list, 2);
    font_set_size(11, 12);
    font_set_color(0xFF000000);
    list = sc_draw_num_text(list, (short)x, (short)(r->y + 1), value, 8, ndig, 0);
    return dl_disable(list, 2);
}

void *sc_draw_panes(void *list)
{
    ScPane *p = &scv.pane[0];

    if (p->list_mode == 0)
        list = sc_draw_grid(list, p);
    else
        list = sc_draw_rows(list, p);
    p = &scv.pane[1];
    if (scv.mode > 1) {
        if (p->list_mode == 0)
            list = sc_draw_grid(list, p);
        else if (scv.mode < 4)
            list = sc_draw_rows_small(list, p);
        else
            list = sc_draw_rows(list, p);
    }
    return list;
}

#ifdef NON_MATCHING /* o original carrega a largura da linha depois da junção do ternário; o mwcc atual a sobe para o atraso do desvio */
/* list mode: one card per row */
void *sc_draw_rows(void *list, ScPane *p)
{
    ScList *l = p->list;
    ScLayout *lay = &p->lay;
    unsigned short x = p->lay.x + p->lay.row_x;
    unsigned short y = p->lay.y + p->lay.row_y;
    unsigned short idx = p->top;
    unsigned short n = p->lay.rows;
    int i;
    ScEnt *e;

    if (n + idx >= l->count)
        n = l->count - idx;
    for (i = 0; i < n; i++) {
        if (idx >= l->count)
            e = 0;
        else
            e = &l->ents[l->order[idx]];
        list = sc_draw_row(list, x, y, lay->row_w, e);
        idx++;
        y += lay->row_h;
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* contadores de linha/coluna em pilha e registradores diferentes */
/* grid mode: card pictures with their marks */
void *sc_draw_grid(void *list, ScPane *p)
{
    ScList *l = p->list;
    ScLayout *lay = &p->lay;
    short y = p->lay.y + p->lay.grid_y;
    unsigned short idx = p->top;
    int row = 0;
    int col;
    unsigned short x;
    ScEnt *e;
    unsigned int color;
    unsigned short rank;
    unsigned char r;

    if (p->lay.grid_rows == 0)
        return list;
    do {
        if (idx >= l->count)
            return list;
        x = lay->x + lay->grid_x;
        for (col = 0; col < lay->cols; col++) {
            e = SC_ENT(l, idx);
            color = 0xFFFFFFFF;
            rank = 0;
            if (sc.kind == 0 && (r = e->rank) != 4) {
                rank = r;
                color = 0xA0FFFFFF;
            }
            list = sc_draw_card_pict(list, x, y, 1, color, e, res_get(0x8001), res_get(0x8002), res_get(0x8032));
            list = sc_draw_card_marks(list, x, y, e, 1, inv_get_state(e->id == 1), 0, rank, res_get(0x8009), res_get(0x800A));
            idx++;
            x += lay->cell_w;
            if (idx >= l->count)
                break;
        }
        row++;
        y += lay->cell_h;
    } while (row < lay->grid_rows);
    return list;
}
#endif

/* the three counters of a row (template of sc_draw_row's local) */
ScCounter sc_row_counters[3] = { { 1, { 0 }, 0xFF000000, 0 }, { 2, { 0 }, 0xFF000000, 0 }, { 4, { 0 }, 0xFF000000, 0 } };

#ifdef NON_MATCHING /* o modelo dos contadores é copiado com lwc1/swc1 e a alocação de s/pilha difere */
/* a wide row: frame, picture, name, icons, level, ATK/DEF, then the counters or the copies */
void *sc_draw_row(void *list, unsigned short x, unsigned short y, unsigned short w, ScEnt *e)
{
    unsigned short attr;
    unsigned short tx;
    int full = 0;
    unsigned short rank = 0;
    unsigned int color;
    unsigned short type;
    unsigned short kind;
    unsigned short atk;
    unsigned short def;
    unsigned short level;
    unsigned short cx;
    int i;
    unsigned short id;

    attr = card_get_attribute_icon(e->id);
    list = sc_draw_row_frame(list, PT(x + 23, y), w, attr, res_get(0x8006));
    color = 0xFFFFFFFF;
    if (sc.kind == 0 && e->rank != 4) {
        rank = e->rank;
        color = 0xA0FFFFFF;
    }
    list = sc_draw_card_pict(list, x, y, 1, color, e, res_get(0x8001), res_get(0x8002), res_get(0x8032));
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
        list = sc_draw_kind_icon(list, tx, y + 14, kind);
        list = sc_draw_attr_icon(list, tx + 18, y + 14, type);
    } else {
        type = card_get_type(e->id);
        kind = card_get_kind(e->id);
        atk = card_get_atk_q(e->id);
        def = card_get_def_q(e->id);
        level = card_get_level(e->id);
        list = sc_draw_kind_icon(list, tx, y + 14, kind);
        list = sc_draw_type_icon(list, tx + 18, y + 14, type);
        list = sc_draw_level(list, PT(tx + 37, y + 16), level, 0xFF000000, res_get(0x8009));
        list = sc_draw_stat(list, atk << 16, PT(tx + 73, y + 16), 0xC0A, 0xFF000000, res_get(0x8009));
        list = sc_draw_stat(list, def << 16 | 1, PT(tx + 133, y + 16), 0xC0A, 0xFF000000, res_get(0x8009));
    }
    cx = tx + 193;
    if (x + w < cx + 50) {
        full = 1;
    } else {
        ScCounter c[3];

        memcpy(c, sc_row_counters, sizeof(c));

        if (sc.kind == 0) {
            c[0].icon = 0;
            c[1].icon = 5;
            c[1].value = sc_list_count_id(&sc.list[1], e->id, 0);
            c[2].icon = 6;
            c[2].value = inv_get_count(e->id);
        } else {
            if (card_attribute_icon_is3(e->id)) {
                c[1].icon = 3;
                c[0].icon = 0;
                for (i = 0; i < deck_count(3); i++) {
                    if (e->id == deck_get_card(3, i))
                        c[1].value++;
                }
                for (i = 0; i < partner_count_get(); i++) {
                    if (e->id == partner_id_get(i) && partner_state_get(i) == 3)
                        c[1].color = 0xFF0080FF;
                }
            } else {
                for (i = 0; i < deck_count(1); i++) {
                    if (e->id == deck_get_card(1, i))
                        c[0].value++;
                }
                for (i = 0; i < deck_count(2); i++) {
                    if (e->id == deck_get_card(2, i))
                        c[1].value++;
                }
                for (i = 0; i < partner_count_get(); i++) {
                    if (e->id == partner_id_get(i)) {
                        if (partner_state_get(i) == 1)
                            c[0].color = 0xFF0080FF;
                        if (partner_state_get(i) == 2)
                            c[1].color = 0xFF0080FF;
                    }
                }
            }
            c[2].value = sc_list_count_id(&sc.list[0], e->id, 0);
            if (scv.pane[scv.side].list->tab == 1)
                full = 1;
        }
        list = sc_draw_counters(list, PT(cx + 7, y + 14), c, res_get(0x8006), res_get(0x800A));
    }
    if (inv_get_flag7(e->id))
        list = sc_draw_flag_icon(list, PT(x + w - 24, y + 3), 2, res_get(0x800A));
    return sc_draw_card_marks(list, x, y, e, full, inv_get_state(e->id == 1), 0, rank, res_get(0x8009), res_get(0x800A));
}
#endif

#ifdef NON_MATCHING /* o original carrega a largura da linha depois da junção do ternário; o mwcc atual a sobe para o atraso do desvio */
/* small rows of the second pane (modes 2 and 3) */
void *sc_draw_rows_small(void *list, ScPane *p)
{
    ScList *l = p->list;
    ScLayout *lay = &p->lay;
    unsigned short x = p->lay.x + p->lay.row_x;
    unsigned short y = p->lay.y + p->lay.row_y;
    unsigned short idx = p->top;
    unsigned short n = p->lay.rows;
    int i;
    ScEnt *e;

    if (n + idx >= l->count)
        n = l->count - idx;
    for (i = 0; i < n; i++) {
        e = SC_ENT(l, idx);
        list = sc_draw_row_small(list, x, y, lay->row_w, e);
        idx++;
        y += lay->row_h;
    }
    return list;
}
#endif

void *sc_draw_row_small(void *list, unsigned short x, unsigned short y, unsigned short w, ScEnt *e)
{
    unsigned short attr;
    unsigned int color;
    unsigned short rank;
    unsigned short atk;
    unsigned short def;
    unsigned short level;
    unsigned short tx;

    attr = card_get_attribute_icon(e->id);
    list = sc_draw_row_frame(list, PT(x + 26, y), w, attr, res_get(0x8006));
    color = 0xFFFFFFFF;
    rank = 0;
    if (sc.kind == 0 && e->rank != 4) {
        color = 0xA0FFFFFF;
        rank = e->rank;
    }
    list = sc_draw_card_pict(list, x, y + 1, 1, color, e, res_get(0x8001), res_get(0x8002), res_get(0x8032));
    list = sc_draw_card_marks(list, x, y + 1, e, 1, inv_get_state(e->id == 1), 0, rank, res_get(0x8009), res_get(0x800A));
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
        list = sc_draw_stat(list, atk << 16 | 2, PT(tx, y + 13), 0xA09, 0xFF000000, res_get(0x8009));
        list = sc_draw_stat(list, def << 16 | 3, PT(tx, y + 23), 0xA09, 0xFF000000, res_get(0x8009));
        list = sc_draw_level(list, PT(x + 69, y + 20), level, 0xFF000000, res_get(0x8009));
    }
    if (inv_get_flag7(e->id))
        list = sc_draw_flag_icon(list, PT(x + 78, y + 13), 2, res_get(0x800A));
    return list;
}
