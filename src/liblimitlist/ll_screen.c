// @range 0x08806490-0x0880740C
// @asm 0x08806490-0x088066C8
// @asm 0x08806DC0-0x0880740C
// @data 0x0880D5EC-0x0880D634
/* The screen: background animation, tab titles with the list sizes, header with the list name,
   the list rows (kind banner, picture, name, icons, ATK/DEF and the deck/partner/owned counters)
   or the picture grid, the cursor frame, menus and the fade. */
#include "liblimitlist.h"
#include "card_db.h"

typedef struct LlCounter {     /* see ll_draw.c */
    unsigned char icon;
    unsigned char pad1[3];
    unsigned int color;
    unsigned char value;
    unsigned char pad9[3];
} LlCounter;

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
void *dl_scissor(void *list, int x, int y, int w, int h);
void *gfx_begin_2d(void *list);
void *gfx_end_2d(void *list);
void *ehf_draw(void *list, void *e, void *opts);
void *cardlist_build(void *list, LlScroll *s);
void cardinfo_draw(void);
unsigned short *sysstate_get_player_name_ptr(void);
int inv_get_count(unsigned short id);
int partner_count_get(void);
unsigned short partner_id_get(int i);
int partner_state_get(int i);

void *ll_menu_draw(void *list);
void *ll_draw_frame(void *list, int x, int y, int w, int h, void *gim, unsigned int flags);
void *ll_draw_number(void *list, short x, short y, int n, int step, unsigned short digits, int zeros);
void *ll_draw_scrollbar(void *list, int x, int y, int h, int pos, int page, int total, int style, void *gim);
void *ll_draw_card(void *list, int x, int y, int shadow, unsigned int color, LlEntry *e, void *gim,
                   void *alt_gim, int *alt);
void *ll_draw_kind(void *list, int pos, unsigned short width, unsigned short kind, void *gim);
void *ll_draw_race(void *list, int pos, unsigned short icon, void *gim);
void *ll_draw_type(void *list, int pos, unsigned short icon, void *gim);
void *ll_draw_attr(void *list, int pos, unsigned short icon, void *gim);
void *ll_draw_badge(void *list, int pos, unsigned short n, unsigned int color, void *gim);
void *ll_draw_stat(void *list, int stat, int pos, unsigned short size, unsigned int color, void *gim);
void *ll_draw_counters(void *list, int pos, LlCounter *c, void *bar_gim, void *icon_gim);

void *ll_draw_bg(void *dl);
void *ll_draw_tabs(void *dl);
void *ll_draw_header(void *dl);
void *ll_draw_entries(void *dl);
void *ll_draw_list(void *dl);
void *ll_draw_grid(void *dl);
void *ll_draw_row(void *dl, unsigned short x, unsigned short y, LlEntry *e);

/* entry i of list t, 0 past the end */
static inline LlEntry *ll_entry_at(LimitList *l, unsigned int t, unsigned short i)
{
    if (i >= l->count[t])
        return 0;
    return &l->list[t][i];
}

#ifdef NON_MATCHING /* o original deixa uma cópia morta do `li a0,4` depois do `b` do ramo da ficha da carta (problema aberto, como tlist_draw/duelrecord_draw) */
/* 0x08806490: export */
void limitlist_draw(void)
{
    void *dl = dl_select_layer(0, 0);

    dl = dl_clear_color(dl, 0xFF000000);
    dl = dl_clear_depth(dl, 0);
    dl = dl_clear_stencil(dl, 0);
    dl = dl_clear(dl, 0x17);
    dl_set_list(dl);
    if (ll.state != 7 && ll.state != 0) {
        if (ll.info) {
            cardinfo_draw();
        } else {
            dl = dl_select_layer(4, 0);
            dl = dl_enable(dl, 9);
            dl = dl_enable(dl, 4);
            dl = dl_tex_func(dl, 0, 1);
            dl = dl_set_tex_filter(dl, 0, 0);
            dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
            dl = dl_disable(dl, 1);
            dl = ll_draw_bg(dl);
            dl = cardlist_build(dl, &ll.scroll);
            dl = ll_draw_tabs(dl);
            dl = ll_draw_header(dl);
            dl = ll_draw_entries(dl);
            if (ll.state == 5 || ll.state == 3)
                dl = ll_menu_draw(dl);
            else {
                LlRect *r = &ll.cur;
                void *gim = llres_get(0x8006);
                dl = ll_draw_frame(dl, r->x, r->y, r->w, r->h, gim, gfx_get_state()->frame);
            }
            dl_set_list(dl);
            dl = dl_select_layer(5, 0);
            dl = dl_disable(dl, 1);
            fade_apply(&dl);
            dl_set_list(dl);
        }
    }
}
#endif

void *ll_draw_bg(void *dl)
{
    void *ehf = llres_get(0x8021);
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

unsigned short ll_tab_x[6 * 3] = {   /* x of the list sizes in the tab titles, 3 per language */
    0xFC, 0x16A, 0x1D9, 0xFC, 0x16A, 0x1D9, 0xFC, 0x16A, 0x1D9,
    0xFC, 0x16A, 0x1D9, 0xFC, 0x16A, 0x1D9, 0xFC, 0x16A, 0x1D9,
};

/* initial counters of a row (main deck / side deck / rest), the template ll_draw_row copies */
LlCounter ll_row_counters[3] = {
    { 1, { 0 }, 0xFF000000, 0 },
    { 2, { 0 }, 0xFF000000, 0 },
    { 4, { 0 }, 0xFF000000, 0 },
};

/* tab titles (animated), the size of each list and the scrollbar */
void *ll_draw_tabs(void *dl)
{
    void *ehf = llres_get(0x1010);
    unsigned int opts[24];
    int i;
    int page;

    if (ehf == 0)
        return dl;
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()->frame;
    dl = gfx_begin_2d(dl);
    dl = ehf_draw(dl, ehf, opts);
    dl = gfx_end_2d(dl);
    font_set_size(11, 12);
    font_set_color(0xFF000000);
    dl = dl_scissor(dl, 0, 26, 480, 12);
    dl = dl_enable(dl, 2);
    for (i = 0; i < 3; i++) {
        unsigned short x = ll_tab_x[i + cfg_get_language() * 3];
        int y = 29;
        if (i == ll.tab)
            y -= 2;
        dl = ll_draw_number(dl, x, y, ll.count[i], 8, 3, 0);
    }
    dl = dl_disable(dl, 2);
    page = 7;
    if (ll.grid == 1)
        page = 66;
    return ll_draw_scrollbar(dl, 469, 50, 192, ll.top[ll.tab], page, ll.count[ll.tab], 2, llres_get(0x8007));
}

/* header bar with the name of the list */
void *ll_draw_header(void *dl)
{
    void *gim = llres_get(0x1002);

    if (gim == 0)
        return dl;
    dl = gim_set_texture(dl, gim);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = draw_set_offsets(dl, 12, 23, 12, 23);
    dl = draw_sprite(dl, 148, 0, 0, 0, 20, 0, 0);
    dl = draw_sprite(dl, 467, 0, 0, 13, 20, 0, 0);
    dl = draw_set_offsets(dl, 1, 23, 307, 23);
    dl = draw_sprite(dl, 160, 0, 0, 12, 20, 0, 0);
    font_set_size(14, 14);
    font_set_color(0xFFFFFFFF);
    font_text_set(dl);
    font_draw_text(156 << 6, 4 << 6, 324 << 6, sysstate_get_player_name_ptr());
    return font_text_clear();
}

void *ll_draw_entries(void *dl)
{
    if (ll.grid == 0)
        return ll_draw_list(dl);
    else
        return ll_draw_grid(dl);
}

/* name list: up to 7 rows from the top entry */
void *ll_draw_list(void *dl)
{
    unsigned short y = 41;
    unsigned int t = ll.tab;
    unsigned short i = ll.top[t];
    unsigned short n = 7;
    int k;

    if (i + 7 >= ll.count[t])
        n = ll.count[t] - i;
    for (k = 0; k < n; k++) {
        dl = ll_draw_row(dl, 151, y, ll_entry_at((LimitList *)(char *)&ll, t, i));
        i++;
        y += 30;
    }
    return dl;
}

/* picture grid: 6 rows of 11 */
void *ll_draw_grid(void *dl)
{
    unsigned short y = 46;
    unsigned int t = ll.tab;
    unsigned short x;
    unsigned short i = ll.top[t];
    unsigned short count = ll.count[t];
    int row;

    for (row = 0; row < 6; row++) {
        int col;

        if (i >= count)
            break;
        x = 156;
        col = 0;
        do {
            LlEntry *e = ll_entry_at((LimitList *)(char *)&ll, t, i);
            if (e)
                dl = ll_draw_card(dl, x, y, 1, 0xFFFFFFFF, e, llres_get(0x8001), llres_get(0x8002),
                                  llres_get(0x8032));
            x += 28;
            i++;
        } while (i < count && ++col < 11);
        y += 34;
    }
    return dl;
}

#ifdef NON_MATCHING /* mesma forma; com cnt inicializado localmente (o original copia ll_row_counters como modelo, via lwc1), x = x + 29 e o nome num temporário, o prólogo e a cópia batem; sobram x re-mascarado em s0 no ramo do monstro e o x += 193 duplicado nos dois ramos (88 palavras) */
/* one row of the name list */
void *ll_draw_row(void *dl, unsigned short x, unsigned short y, LlEntry *e)
{
    unsigned short icon = card_get_attribute_icon(e->id);
    int yy;

    dl = ll_draw_kind(dl, (unsigned short)(x + 23) | y << 16, 318, icon, llres_get(0x8006));
    dl = ll_draw_card(dl, x, y, 1, 0xFFFFFFFF, e, llres_get(0x8001), llres_get(0x8002), llres_get(0x8032));
    x = x + 29;
    font_text_set(dl);
    font_set_size(12, 12);
    font_set_color(0xFF000000);
    if (cfg_get_language() != 0)
        font_set_flag4000(1);
    {
        unsigned short *name = card_get_name(e->id);
        font_draw_text(x << 6, (y + 2) << 6, 16896, name);
    }
    font_set_flag4000(0);
    dl = font_text_clear();
    if (icon == 7 || icon == 8) {
        unsigned short type = card_get_prop5_hi(e->id);
        unsigned short kind = card_get_kind(e->id);
        yy = (unsigned short)(y + 14) << 16;
        dl = ll_draw_race(dl, x | yy, kind, llres_get(0x8007));
        dl = ll_draw_type(dl, (unsigned short)(x + 18) | yy, type, llres_get(0x8007));
    } else {
        unsigned short attr = card_get_type(e->id);
        unsigned short kind = card_get_kind(e->id);
        unsigned short atk = card_get_atk_q(e->id);
        unsigned short def = card_get_def_q(e->id);
        unsigned short level = card_get_level(e->id);
        int y2;
        yy = (unsigned short)(y + 14) << 16;
        dl = ll_draw_race(dl, x | yy, kind, llres_get(0x8007));
        dl = ll_draw_attr(dl, (unsigned short)(x + 18) | yy, attr, llres_get(0x8007));
        y2 = (unsigned short)(y + 16) << 16;
        dl = ll_draw_badge(dl, (unsigned short)(x + 37) | y2, level, 0xFF000000, llres_get(0x8009));
        dl = ll_draw_stat(dl, atk << 16, (unsigned short)(x + 73) | y2, 0xC0A, 0xFF000000, llres_get(0x8009));
        dl = ll_draw_stat(dl, def << 16 | 1, (unsigned short)(x + 133) | y2, 0xC0A, 0xFF000000, llres_get(0x8009));
    }
    x += 193;
    {
        LlCounter cnt[3] = { { 1, { 0 }, 0xFF000000, 0 }, { 2, { 0 }, 0xFF000000, 0 }, { 4, { 0 }, 0xFF000000, 0 } };
        int i;

        if (card_attribute_icon_is3(e->id)) {
            /* extra deck card: the extra deck, partners holding it, then the rest */
            cnt[1].icon = 3;
            cnt[0].icon = 0;
            for (i = 0; i < deck_count(3); i++) {
                if (e->id == deck_get_card(3, i))
                    cnt[1].value++;
            }
            for (i = 0; i < partner_count_get(); i++) {
                if (e->id == partner_id_get(i) && partner_state_get(i) == 3) {
                    cnt[1].color = 0xFF0080FF;
                    cnt[1].value++;
                }
            }
            cnt[2].value = inv_get_count(e->id) - cnt[1].value;
        } else {
            /* main deck, side deck (with the partners holding it), then the rest */
            for (i = 0; i < deck_count(1); i++) {
                if (e->id == deck_get_card(1, i))
                    cnt[0].value++;
            }
            for (i = 0; i < deck_count(2); i++) {
                if (e->id == deck_get_card(2, i))
                    cnt[1].value++;
            }
            for (i = 0; i < partner_count_get(); i++) {
                if (e->id == partner_id_get(i)) {
                    if (partner_state_get(i) == 1) {
                        cnt[0].color = 0xFF0080FF;
                        cnt[0].value++;
                    }
                    if (partner_state_get(i) == 2) {
                        cnt[1].color = 0xFF0080FF;
                        cnt[1].value++;
                    }
                }
            }
            cnt[2].value = inv_get_count(e->id) - cnt[0].value - cnt[1].value;
        }
        return ll_draw_counters(dl, (unsigned short)(x + 7) | yy, cnt, llres_get(0x8006), llres_get(0x800A));
    }
}
#endif
