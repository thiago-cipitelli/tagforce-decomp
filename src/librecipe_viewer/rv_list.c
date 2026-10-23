// @range 0x08807A44-0x08809274
// @asm 0x08807A44-0x08808490
// @asm 0x08808650-0x08808854
// @asm 0x08808988-0x08808BBC
// @asm 0x08808C30-0x08809274
/* Logic of the lists and of the shown recipe: loading the extra data lists, input of the list
   state (tabs, cursor with fast scrolling, open/menu), of the recipe grid, the card info state,
   the blink before a menu, and building the recipe grid (copies owned, partners, new cards). */
#include "librecipe_viewer.h"
#include "card_db.h"

int extdata_scan(int kind, int *count);
int extdata_load(void *out, int *count);
void pad_set_repeat_from_fps(void);
void pad_set_repeat(int delay, int rate);
unsigned int pad_get_repeat(void);
unsigned short region_pick_value(int flag);
int ehf_set_anim(void *e, int target, int limit);
int cardinfo_open(unsigned short card_id, void *window_gim, void *bg_gim);
void cardinfo_set_lists(void *list_gim, void *text_pack, void *index);
int cardinfo_update(void);
int cardinfo_has_alt_pict(void);
int inv_get_count(unsigned short id);
unsigned short rv_file_index(unsigned short id, unsigned short *tab);
int rv_kind_icon(unsigned short kind);
int rv_partner_count(int id);
void rv_scroll_pad(RvScroll *s);
void rv_menu_open(unsigned short kind);
int rv_grid_row_cols(int row);
RvEntry *rv_grid_entry(void);
void rv_grid_select(void);

void rv_set_title_anim(void);
void rv_set_tab_anim(void);
int rv_grid_cols(int rows, int n, int max);
void rv_select(void);
int rv_card_cmp(const void *a, const void *b);
void rv_build_list(RecipeData *d, RvRecipe *r);
void rv_fill_grid(RvCards *c, RecipeData *d);

#ifdef NON_MATCHING /* o original testa os passos com beql e os retornos caem em outro lugar; forma do switch não reproduzida */
/* state 0: lists found in the extra data */
void rv_load_lists(void)
{
    int i;

    switch (rv.step) {
    case 0:
        if (extdata_scan(1, &rv.n_ext))
            rv.step++;
        return;
    case 1:
        if (rv.n_ext) {
            if (rv.ext == 0)
                rv.ext = heap_alloc(rv.heap, 0x20800);
            memset(rv.ext, 0, 0x20800);
            rv.step++;
        } else {
            rv.step = 4;
        }
        return;
    case 2:
        if (extdata_load(rv.ext, &rv.n_ext))
            rv.step++;
        return;
    case 3:
        for (i = 0; i < rv.n_ext; i++) {
            rv.ext_info[i].index = i;
            rv_build_list(&rv.ext[i].recipe, &rv.ext_info[i]);
        }
    case 4:
        break;
    default:
        return;
    }
    rv.step = 0;
    rv.state = 1;
}
#endif

#ifdef NON_MATCHING /* cadeia longa de testes de botões; a ordem dos blocos e os $s diferem */
/* list state: cancel, accept, tabs, cursor */
void rv_input_list(void)
{
    PadState *pad = pad_get();
    RvPos *p = &rv.pos[rv.tab];
    int changed = 0;
    unsigned int trig;

    rv_scroll_pad(&rv.scroll);
    if (rv.held & pad->b.down) {
        rv.held = pad->b.down & 0x50;
        if (rv.hold_time < 120)
            rv.hold_time++;
        {
            int rate = 4 - rv.hold_time / 30;
            if (rate > 0)
                pad_set_repeat(pad_get_repeat() >> 16, rate);
        }
    } else {
        if (rv.held) {
            rv.held = 0;
            rv.hold_time = 0;
            pad_set_repeat_from_fps();
        }
        rv.held = pad->b.down & 0x50;
    }
    trig = pad->b.trig;
    if (trig & region_pick_value(1)) {
        rv.cancelled = 1;
        rv.state = 6;
        fade_start_in(1, 0xFF000000, 16);
        se_play(0x6214);
        if (rv.mode == 0)
            bgm_cmd(600);
    } else if (trig & region_pick_value(0)) {
        int i = p->top + p->cursor;
        switch (rv.mode) {
        case 1:
            if (((SaveRec *)sysstate_rec_get(i))->used == 0) {
                rv.state = 5;
                se_play(0x6213);
            } else {
                rv_menu_open(2);
                se_play(0x621A);
            }
            break;
        case 2:
            switch (rv.tab) {
            case 2:
                if (i < rv.n_ext) {
                    rv_menu_open(6);
                    se_play(0x621A);
                }
                break;
            case 1:
                if (i < rv.n_recipes) {
                    rv_menu_open(6);
                    se_play(0x621A);
                }
                break;
            case 0:
                if (i < 50 && ((SaveRec *)sysstate_rec_get(i))->used) {
                    rv_menu_open(1);
                    se_play(0x621A);
                }
                break;
            }
            break;
        case 0:
            if (rv.tab == 0 && ((SaveRec *)sysstate_rec_get(i))->used) {
                rv_menu_open(5);
                se_play(0x6216);
            }
            break;
        }
    } else if (trig & 0x8200) {
        if (rv.mode != 1) {
            unsigned int t = (rv.tab + 1) % 3;
            if (t == 2 && rv.n_ext < 1)
                t = 0;
            rv.tab = t;
            changed = 1;
            rv_set_tab_anim();
            se_play(0x621B);
        }
    } else if (trig & 0x100) {
        if (rv.mode != 1) {
            unsigned int t = (rv.tab + 2) % 3;
            if (t == 2 && rv.n_ext < 1)
                t = 1;
            rv.tab = t;
            changed = 1;
            rv_set_tab_anim();
            se_play(0x621B);
        }
    } else if (pad->b.rep & 0x20) {
        rv.state = 4;
        se_play(0x6212);
    } else if (pad->b.rep & 0x10) {
        if (p->cursor != 0) {
            p->cursor--;
            changed = 1;
        } else if (p->top != 0) {
            p->top--;
            changed = 1;
        } else if (trig & 0x10) {
            int total = 0;
            int few;
            switch (rv.tab) {
            case 2:
                total = rv.n_ext;
                few = total < 8;
                break;
            case 1:
                total = rv.n_recipes;
                few = total < 8;
                break;
            case 0:
                total = 50;
                few = total < 8;
                break;
            default:
                few = 1;
                break;
            }
            if (few) {
                p->top = 0;
                p->cursor = 7;
            } else {
                p->cursor = 7;
                p->top = total - p->cursor - 1;
            }
            changed = 1;
        }
    } else if (pad->b.rep & 0x40) {
        int total = 0;
        switch (rv.tab) {
        case 2:
            total = rv.n_ext;
            break;
        case 1:
            total = rv.n_recipes;
            break;
        case 0:
            total = 50;
            break;
        }
        if (p->cursor < 7) {
            p->cursor++;
            changed = 1;
        } else if (p->top < total - 8) {
            p->top++;
            changed = 1;
        } else if (trig & 0x40) {
            p->cursor = 0;
            p->top = 0;
            changed = 1;
        }
    }
    if (changed) {
        rv_select();
        rv.shown = 0;
        rv.grid.row = 0;
        rv.grid.col = 0;
        se_play(0x6212);
    }
    rv_grid_select();
}
#endif

#ifdef NON_MATCHING /* forma próxima; desvios e registradores dos testes do d-pad diferem */
/* recipe state: cursor in the grid, card info, back */
void rv_input_recipe(void)
{
    PadState *pad = pad_get();
    unsigned int rep;
    int n;

    rv_scroll_pad(&rv.scroll);
    if (pad->b.trig & region_pick_value(1)) {
        rv.state = 3;
        se_play(0x6214);
    } else if (pad->b.trig & region_pick_value(0)) {
        if (rv_grid_entry()) {
            rv.info = 1;
            se_play(0x6213);
        }
    } else {
        rep = pad->b.rep;
        if (rep & 0x80) {
            if (rv.grid.col == 0)
                rv.state = 3;
            else
                rv.grid.col--;
            se_play(0x6212);
        } else if (rep & 0x20) {
            if (rv.grid.col < rv_grid_row_cols(rv.grid.row) - 1) {
                rv.grid.col++;
                se_play(0x6212);
            }
        } else if (rep & 0x10) {
            if (rv.grid.row != 0) {
                n = rv_grid_row_cols(rv.grid.row - 1);
                if (n <= rv.grid.col)
                    rv.grid.col = n - 1;
                rv.grid.row--;
                se_play(0x6212);
            }
        } else if ((rep & 0x40) && rv.grid.row < 6) {
            n = rv_grid_row_cols(rv.grid.row + 1);
            if (n <= rv.grid.col)
                rv.grid.col = n - 1;
            rv.grid.row++;
            se_play(0x6212);
        }
    }
    rv_grid_select();
}
#endif

/* card info of the card under the grid cursor */
void rv_info_update(void)
{
    switch (rv.step) {
    case 0:
        fade_start_in(1, 0xFF000000, 16);
        rv.step++;
    case 1:
        if (!fade_is_done())
            break;
        cardinfo_open(rv_grid_entry()->id, rvres_get(0x800C), rvres_get(0x8012));
        cardinfo_set_lists(rvres_get(0x8015), rvres_get(0x8036), rvres_get(0x8037));
        cardinfo_has_alt_pict();
        rv.info_open = 1;
        rv.step++;
        break;
    case 2:
        if (!cardinfo_update())
            break;
        rv.info_open = 0;
        fade_start(1, 0xFF000000, 16);
        rv.step++;
    case 3:
        if (fade_is_done()) {
            rv.step = 0;
            rv.info = 0;
        }
        break;
    }
}

#ifdef NON_MATCHING /* forma próxima; ordem dos passos e registradores diferem */
/* menu state: the frame blinks, then the deck is reloaded and the confirmation opens */
void rv_input_menu(void)
{
    switch (rv.step) {
    case 0:
        if (rv.blink < 17) {
            rv.blink++;
            if (rv.blink & 2)
                rv.no_frame = 0;
            else
                rv.no_frame = 1;
        } else {
            rv.blink = 0;
            rv.step = 1;
            rv.no_frame = 0;
        }
        break;
    case 1:
        if (rv.on_delete) {
            int i = rv.pos[rv.tab].top + rv.pos[rv.tab].cursor;
            SaveRec *s;
            RvRecipe *r;

            rv.on_delete(i);
            s = sysstate_rec_get(i);
            r = &rv.decks[i];
            memset(r, 0, sizeof(*r));
            if (s->used) {
                r->index = i;
                rv_build_list(&s->recipe, r);
            }
        }
        rv_select();
        rv.shown = 0;
        rv.grid.row = 0;
        rv.grid.col = 0;
        rv.step++;
        break;
    case 2:
        rv_menu_open(3);
        se_play(0x621A);
        rv.state = 3;
        rv.step = 0;
        break;
    }
}
#endif

/* title animation of the mode */
void rv_set_title_anim(void)
{
    void *e = rvres_get(0x1011);
    unsigned short from;
    unsigned short to;

    if (e) {
        switch (rv.mode) {
        case 0:
            from = 160;
            to = 229;
            break;
        case 1:
            from = 0;
            to = 69;
            break;
        case 2:
            from = 80;
            to = 149;
            break;
        default:
            from = 0;
            to = 0;
            break;
        }
        ehf_set_anim(e, from, to);
    }
}

/* tab animation: the selected tab, 2 later when there are extra data lists, 5 when saving */
void rv_set_tab_anim(void)
{
    void *e = rvres_get(0x1010);
    int t = rv.tab;

    if (rv.mode == 1)
        t = 5;
    else if (rv.n_ext > 0)
        t += 2;
    if (e)
        ehf_set_anim(e, t, t + 1);
}

/* cards per row to show n cards in `rows` rows of at least `min` */
int rv_grid_cols(int rows, int n, int min)
{
    if (n < min)
        return n;
    if (n < min * rows)
        return min;
    return (n + rows - 1) / rows;
}

#ifdef NON_MATCHING /* o switch por aba e o laço de contagem de tipos saem com outros registradores */
/* loads the recipe under the list cursor into the grid and counts the types */
void rv_select(void)
{
    RvPos *p = &rv.pos[rv.tab];
    unsigned short i = p->top + p->cursor;
    int ok = 0;
    RecipeData rd;

    memset(rv.n_type, 0, sizeof(rv.n_type));
    switch (rv.tab) {
    case 2:
        if (i < rv.n_ext) {
            rv_fill_grid(&rv.grid.c, &rv.ext[i].recipe);
            ok = 1;
        }
        break;
    case 1:
        if (i < rv.n_recipes) {
            recipe_load(rv.recipes[i].index, &rd);
            rv_fill_grid(&rv.grid.c, &rd);
            ok = 1;
        }
        break;
    case 0:
        if (i < 50) {
            SaveRec *s = sysstate_rec_get(i);
            if (s->used) {
                rv_fill_grid(&rv.grid.c, &s->recipe);
                ok = 1;
            }
        }
        break;
    }
    if (ok) {
        int k;
        for (k = 0; k < rv.grid.c.n_main; k++)
            rv.n_type[rv_kind_icon(card_get_attribute_icon(rv.grid.c.main[k].id))]++;
        rv.n_type[3] = rv.grid.c.n_side;
    } else {
        memset(&rv.grid.c, 0, sizeof(rv.grid.c));
    }
    rv.grid.main_cols = rv_grid_cols(4, rv.grid.c.n_main, 7);
    rv.grid.side_cols = rv_grid_cols(2, rv.grid.c.n_side, 7);
}
#endif

/* by attribute icon, then by the database sort key */
int rv_card_cmp(const void *a, const void *b)
{
    unsigned short ida = *(const unsigned short *)a;
    unsigned short idb = *(const unsigned short *)b;
    unsigned short x = card_get_attribute_icon(ida);
    unsigned short y = card_get_attribute_icon(idb);

    if (x == y)
        return card_get_sort(ida) - card_get_sort(idb);
    return x - y;
}

#ifdef NON_MATCHING /* só o bloco do contador de cópias sai em ordem diferente (incremento antes do teste) */
/* how many cards of a recipe are owned: r->need cards, r->have owned */
void rv_build_list(RecipeData *d, RvRecipe *r)
{
    unsigned short ids[126];
    unsigned short n = 0;
    unsigned int i;
    unsigned short prev;
    unsigned short copies;
    int have;
    int j;

    for (i = 0; i < d->n_main; i++)
        ids[n++] = d->main_ids[i];
    for (i = 0; i < d->n_extra; i++)
        ids[n++] = d->extra_ids[i];
    for (i = 0; i < d->n_side; i++)
        ids[n++] = d->side_ids[i];
    qsort(ids, n, 2, rv_card_cmp);
    prev = 0;
    copies = 0;
    have = 0;
    for (j = 0; j < n; j++) {
        unsigned short id = ids[j];
        unsigned short owned = inv_get_count(id);
        if (prev == id) {
            if (owned - ++copies > 0)
                have++;
        } else {
            if (owned > 0)
                have++;
            copies = 0;
            prev = id;
        }
    }
    r->need = d->n_extra + d->n_main + d->n_side;
    r->have = have;
}
#endif

/* fills the grid with the sorted cards of recipe d; a card counts as owned while copies are
   left, or (marked) while partners hold it */
#ifdef NON_MATCHING /* mesma forma; os três laços sobre a cópia da receita usam outros $s e deslocamentos */
void rv_fill_grid(RvCards *c, RecipeData *src)
{
    RecipeData d;
    unsigned char used[0x994];
    unsigned int i;

    memcpy(&d, src, sizeof(d));
    qsort(d.main_ids, d.n_main, 2, rv_card_cmp);
    qsort(d.extra_ids, d.n_extra, 2, rv_card_cmp);
    qsort(d.side_ids, d.n_side, 2, rv_card_cmp);
    memset(c, 0, sizeof(*c));
    memset(used, 0, sizeof(used));
    c->n_main = d.n_main;
    for (i = 0; i < d.n_main; i++) {
        unsigned short id = d.main_ids[i];
        int have = inv_get_count(id);
        RvEntry *e = &c->main[i];
        unsigned char u;

        e->id = id;
        e->file = rv_file_index(id, rvres_get(0x8031));
        u = used[e->file];
        if (have - u > 0) {
            e->owned = 1;
            used[e->file]++;
        } else if (have - u + rv_partner_count(id) > 0) {
            e->owned = 1;
            e->flag7 = 1;
            used[e->file]++;
        } else {
            e->owned = 0;
        }
    }
    c->n_extra = d.n_extra;
    for (i = 0; i < d.n_extra; i++) {
        unsigned short id = d.extra_ids[i];
        int have = inv_get_count(id);
        RvEntry *e = &c->extra[i];
        unsigned char u;

        e->id = id;
        e->file = rv_file_index(id, rvres_get(0x8031));
        u = used[e->file];
        if (have - u > 0) {
            e->owned = 1;
            used[e->file]++;
        } else if (have - u + rv_partner_count(id) > 0) {
            e->owned = 1;
            e->flag7 = 1;
            used[e->file]++;
        } else {
            e->owned = 0;
        }
    }
    c->n_side = d.n_side;
    for (i = 0; i < d.n_side; i++) {
        unsigned short id = d.side_ids[i];
        int have = inv_get_count(id);
        RvEntry *e = &c->side[i];
        unsigned char u;

        e->id = id;
        e->file = rv_file_index(id, rvres_get(0x8031));
        u = used[e->file];
        if (have - u > 0) {
            e->owned = 1;
            used[e->file]++;
        } else if (have - u + rv_partner_count(id) > 0) {
            e->owned = 1;
            e->flag7 = 1;
            used[e->file]++;
        } else {
            e->owned = 0;
        }
    }
}
#endif
