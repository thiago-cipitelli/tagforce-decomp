// @range 0x08808A54-0x08809804
// @asm 0x08808AF4-0x08808C08
// @asm 0x08808E18-0x088094B4
// @data 0x0881A17C-0x0881A1A0
/* The cards of the two panes, as a grid of pictures or as rows (picture, name, kind/attribute
   icons, level, ATK/DEF and the copies in the main and extra decks), small rows for the narrow
   second pane (same source as libselect_card sc_panes.c). */
#include "libdeck_swap.h"

int inv_get_state(unsigned short id);
int inv_get_count(unsigned short id);
int card_attribute_icon_is3(int id);
void font_set_flag4000(int on);
unsigned int ds_list_count_id(DsList *l, unsigned short id);
void *ds_draw_type_icon(void *list, unsigned short x, unsigned short y, unsigned short type);
void *ds_draw_attr_icon(void *list, unsigned short x, unsigned short y, unsigned short attr);
void *ds_draw_kind_icon(void *list, unsigned short x, unsigned short y, unsigned short kind);
void *ds_draw_rows(void *list, DsPane *p);
void *ds_draw_grid(void *list, DsPane *p);
void *ds_draw_row(void *list, unsigned short x, unsigned short y, unsigned short w, DsEnt *e);
void *ds_draw_rows_small(void *list, DsPane *p);
void *ds_draw_row_small(void *list, unsigned short x, unsigned short y, unsigned short w, DsEnt *e);

void *ds_draw_panes(void *list)
{
    DsPane *p = &dsv.pane[0];

    if (p->list_mode == 0)
        list = ds_draw_grid(list, p);
    else
        list = ds_draw_rows(list, p);
    p = &dsv.pane[1];
    if (dsv.mode > 1) {
        if (p->list_mode == 0)
            list = ds_draw_grid(list, p);
        else if (dsv.mode < 4)
            list = ds_draw_rows_small(list, p);
        else
            list = ds_draw_rows(list, p);
    }
    return list;
}

#ifdef NON_MATCHING /* o original carrega a largura da linha depois da junção do ternário; o mwcc atual a sobe para o atraso do desvio */
/* list mode: one card per row */
void *ds_draw_rows(void *list, DsPane *p)
{
    DsList *l = p->list;
    DsLayout *lay = &p->lay;
    unsigned short x = p->lay.x + p->lay.row_x;
    unsigned short y = p->lay.y + p->lay.row_y;
    unsigned short idx = p->top;
    unsigned short n = p->lay.rows;
    int i;
    DsEnt *e;

    if (n + idx >= l->count)
        n = l->count - idx;
    for (i = 0; i < n; i++) {
        if (idx >= l->count)
            e = 0;
        else
            e = &l->ents[l->order[idx]];
        list = ds_draw_row(list, x, y, lay->row_w, e);
        idx++;
        y += lay->row_h;
    }
    return list;
}
#endif

/* grid mode: card pictures with their marks */
void *ds_draw_grid(void *list, DsPane *p)
{
    DsList *l = p->list;
    DsLayout *lay = &p->lay;
    unsigned short y;
    int row;
    unsigned short x;
    DsEnt *e;
    unsigned short idx;
    int col;

    y = lay->y + lay->grid_y;
    idx = p->top;
    for (row = 0; row < lay->grid_rows; row++) {
        if (idx >= l->count)
            break;
        x = lay->x + lay->grid_x;
        for (col = 0; col < lay->cols; col++) {
            e = DS_ENT(l, idx);
            list = ds_draw_card_pict(list, x, y, 1, 0xFFFFFFFF, e, res_get(0x8001), res_get(0x8002), res_get(0x8032));
            list = ds_draw_card_marks(list, x, y, e, 1, inv_get_state(e->id == 1), 1, 0, res_get(0x8009), res_get(0x800A));
            x += (unsigned short)lay->cell_w;
            idx++;
            if (idx >= l->count)
                break;
        }
        y += (unsigned short)lay->cell_h;
    }
    return list;
}


typedef struct DsRowCounters {         /* copied as one struct */
    DsCounter c[3];
} DsRowCounters;

/* the three counters of a row: copies in the main deck, in the extra deck and the free ones (blue
   when a partner's) */
DsRowCounters ds_row_counters = { { { 1, { 0 }, 0xFF000000, 0 }, { 2, { 0 }, 0xFF000000, 0 }, { 4, { 0 }, 0xFF000000, 0 } } };

#ifdef NON_MATCHING /* o modelo dos contadores é copiado palavra a palavra e a alocação de s/pilha difere (como dk_draw_row) */
/* a wide row: frame, picture, name, icons, level, ATK/DEF, then the counters */
void *ds_draw_row(void *list, unsigned short x, unsigned short y, unsigned short w, DsEnt *e)
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
    unsigned int a;
    unsigned int b;

    attr = card_get_attribute_icon(e->id);
    list = ds_draw_row_frame(list, PT(x + 23, y), w, attr, res_get(0x8006));
    list = ds_draw_card_pict(list, x, y, 1, 0xFFFFFFFF, e, res_get(0x8001), res_get(0x8002), res_get(0x8032));
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
        list = ds_draw_kind_icon(list, tx, y + 14, kind);
        list = ds_draw_attr_icon(list, tx + 18, y + 14, type);
    } else {
        type = card_get_type(e->id);
        kind = card_get_kind(e->id);
        atk = card_get_atk_q(e->id);
        def = card_get_def_q(e->id);
        level = card_get_level(e->id);
        list = ds_draw_kind_icon(list, tx, y + 14, kind);
        list = ds_draw_type_icon(list, tx + 18, y + 14, type);
        list = ds_draw_level(list, PT(tx + 37, y + 16), level, 0xFF000000, res_get(0x8009));
        list = ds_draw_stat(list, atk << 16, PT(tx + 73, y + 16), 0xC0A, 0xFF000000, res_get(0x8009));
        list = ds_draw_stat(list, def << 16 | 1, PT(tx + 133, y + 16), 0xC0A, 0xFF000000, res_get(0x8009));
    }
    cx = tx + 193;
    if (x + w < cx + 50) {
        full = 1;
    } else {
        DsRowCounters c = ds_row_counters;

        if (card_attribute_icon_is3(e->id)) {
            b = ds_list_count_id(&ds.list[1], e->id);
            c.c[0].icon = 0;
            c.c[1].icon = 3;
            if (b >> 16)
                c.c[1].color = 0xFF0080FF;
            c.c[1].value = (b >> 16) + b;
            c.c[2].value = inv_get_count(e->id) - b;
        } else {
            a = ds_list_count_id(&ds.list[0], e->id);
            b = ds_list_count_id(&ds.list[2], e->id);
            if (a >> 16)
                c.c[0].color = 0xFF0080FF;
            if (b >> 16)
                c.c[1].color = 0xFF0080FF;
            c.c[0].value = (a >> 16) + a;
            c.c[1].value = (b >> 16) + b;
            c.c[2].value = inv_get_count(e->id) - a - b;
        }
        list = ds_draw_counters(list, PT(cx + 7, y + 14), c.c, res_get(0x8006), res_get(0x800A));
    }
    if (inv_get_flag7(e->id))
        list = ds_draw_flag_icon(list, PT(x + w - 24, y + 3), 2, res_get(0x800A));
    return ds_draw_card_marks(list, x, y, e, full, inv_get_state(e->id == 1), 1, 0, res_get(0x8009), res_get(0x800A));
}
#endif

#ifdef NON_MATCHING /* o original carrega a largura da linha depois da junção do ternário; o mwcc atual a sobe para o atraso do desvio */
/* small rows of the second pane (modes 2 and 3) */
void *ds_draw_rows_small(void *list, DsPane *p)
{
    DsList *l = p->list;
    DsLayout *lay = &p->lay;
    unsigned short x = p->lay.x + p->lay.row_x;
    unsigned short y = p->lay.y + p->lay.row_y;
    unsigned short idx = p->top;
    unsigned short n = p->lay.rows;
    int i;
    DsEnt *e;

    if (n + idx >= l->count)
        n = l->count - idx;
    for (i = 0; i < n; i++) {
        e = DS_ENT(l, idx);
        list = ds_draw_row_small(list, x, y, lay->row_w, e);
        idx++;
        y += lay->row_h;
    }
    return list;
}
#endif

void *ds_draw_row_small(void *list, unsigned short x, unsigned short y, unsigned short w, DsEnt *e)
{
    unsigned short attr;
    unsigned short atk;
    unsigned short def;
    unsigned short level;
    unsigned short tx;

    attr = card_get_attribute_icon(e->id);
    list = ds_draw_row_frame(list, PT(x + 26, y), w, attr, res_get(0x8006));
    list = ds_draw_card_pict(list, x, y + 1, 1, 0xFFFFFFFF, e, res_get(0x8001), res_get(0x8002), res_get(0x8032));
    list = ds_draw_card_marks(list, x, y + 1, e, 1, inv_get_state(e->id == 1), 1, 0, res_get(0x8009), res_get(0x800A));
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
        list = ds_draw_stat(list, atk << 16 | 2, PT(tx, y + 13), 0xA09, 0xFF000000, res_get(0x8009));
        list = ds_draw_stat(list, def << 16 | 3, PT(tx, y + 23), 0xA09, 0xFF000000, res_get(0x8009));
        list = ds_draw_level(list, PT(x + 69, y + 20), level, 0xFF000000, res_get(0x8009));
    }
    if (inv_get_flag7(e->id))
        list = ds_draw_flag_icon(list, PT(x + 78, y + 13), 2, res_get(0x800A));
    return list;
}

