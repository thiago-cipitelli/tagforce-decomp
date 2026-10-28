// @range 0x0880740C-0x08808934
// @asm 0x08808030-0x08808418
// @data 0x0880D634-0x0880D63C
/* The three lists (forbidden, limited, semi-limited) built from the card database, list/grid
   cursor movement with paging, tab switching, the card info state and the deck check run when
   leaving (first card in the player's decks over its limit). */
#include "liblimitlist.h"
#include "card_db.h"

typedef struct LlCount {       /* card of the deck check and its copies */
    unsigned short id;
    unsigned short n;
} LlCount;

int sysflag_test(unsigned int bit);
unsigned short region_pick_value(int flag);
int ehf_set_anim(void *e, int target, int limit);
int cardinfo_open(unsigned short card_id, void *window_gim, void *bg_gim);
void cardinfo_set_lists(void *list_gim, void *text_pack, void *index);
int cardinfo_update(void);
unsigned short ll_file_index(unsigned short id, void *tab);
void ll_scroll_pad(LlScroll *s);
void ll_menu_open_help(void);
void ll_menu_open_illegal(unsigned short id);

void ll_set_tab_anim(int tab);
void ll_move_list(PadBits *pad);
void ll_move_grid(PadBits *pad);
void ll_update_cursor(void);
int ll_select_card(void);
int ll_card_cmp(const void *a, const void *b);
unsigned short ll_find_illegal(void);
int ll_count_add(LlCount *c, unsigned short id, int n);
void ll_page_fit(int pos, int layout, int count, unsigned short *top, unsigned short *col, unsigned short *row);

/* entry under the cursor of list `tab`, 0 past the end */
static inline LlEntry *ll_entry(int tab)
{
    unsigned char t = tab;
    int i = ll.top[t] + ll.cursor[t];

    if (i >= ll.count[t])
        return 0;
    return &ll.list[t][i];
}

void ll_build(void)
{
    int j;
    int n;
    int k;
    LlEntry *e;
    unsigned short c;
    unsigned short f;

    unsigned short *ids[3];
    int i;
    int id;

    ll_free_lists();
    for (i = 0; i < 3; i++) {
        ids[i] = heap_alloc(ll.heap, 0x1326);
        ll.count[i] = 0;
    }
    for (id = 4007; id < 6958; id++) {
        if (card_get_flag31(id)) {
            int lim = card_get_limit(id);
            if (lim < 3) {
                j = ll.count[lim]++;
                ids[lim][j] = id;
            }
        }
    }
    for (i = 0; i < 3; i++)
        qsort(ids[i], ll.count[i], 2, ll_card_cmp);
    for (i = 0; i < 3; i++) {
        n = ll.count[i];
        ll.list[i] = heap_alloc(ll.heap, n * sizeof(LlEntry));
        e = ll.list[i];
        memset(e, 0, n * sizeof(LlEntry));
        for (k = 0; k < n; k++, e++) {
            c = ids[i][k];
            f = ll_file_index(c, llres_get(0x8031));
            e->id = c;
            e->file = f;
            e->lines = 1;
        }
    }
    for (i = 0; i < 3; i++)
        heap_set_level(ll.heap, ids[i]);
    ll.top[0] = 0;
    ll.cursor[0] = 0;
    ll.top[1] = 0;
    ll.cursor[1] = 0;
    ll.top[2] = 0;
    ll.cursor[2] = 0;
    ll_set_tab_anim(ll.tab);
    ll_update_cursor();
    ll_select_card();
}

void ll_free_lists(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        if (ll.list[i]) {
            heap_set_level(ll.heap, ll.list[i]);
            ll.list[i] = 0;
        }
    }
}

/* input of the list state */
void ll_input(void)
{
    PadState *pad = pad_get();
    unsigned int trig = pad->b.trig;
    int changed = 0;

    if (trig & region_pick_value(1)) {
        unsigned short id = ll_find_illegal();
        if (id == 0) {
            fade_start_in(1, 0xFF000000, 16);
            bgm_cmd(600);
            ll.state = 6;
            se_play(0x6214);
        } else {
            ll_menu_open_illegal(id);
            se_play(0x621A);
        }
    } else if (trig & region_pick_value(0)) {
        if (ll_entry(ll.tab)) {
            ll.state = 4;
            ll.step = 0;
            se_play(0x6213);
        }
    } else if (trig & 0x1000) {
        ll_menu_open_help();
        se_play(0x6216);
    } else if (trig & 0x100) {
        ll.tab = (ll.tab + 2) % 3;
        ll_set_tab_anim(ll.tab);
        se_play(0x621B);
        changed = 1;
    } else if (trig & 0x8200) {
        ll.tab = (ll.tab + 1) % 3;
        ll_set_tab_anim(ll.tab);
        se_play(0x621B);
        changed = 1;
    } else if (trig & 1) {
        unsigned short top = ll.top[ll.tab];
        unsigned short col;
        unsigned short row;

        ll.grid = (ll.grid + 1) & 1;
        se_play(0x6213);
        if (!ll.grid) {
            col = 1;
            row = ll.cursor[ll.tab];
            ll_page_fit(ll.top[ll.tab] + ll.cursor[ll.tab], 0x701, ll.count[ll.tab], &top, &col, &row);
            ll.top[ll.tab] = top;
            ll.cursor[ll.tab] = row;
        } else {
            unsigned short c = ll.cursor[ll.tab];
            col = c % 11;
            row = c / 11;
            ll_page_fit(ll.top[ll.tab] + c, 0x60B, ll.count[ll.tab], &top, &col, &row);
            ll.top[ll.tab] = top;
            ll.cursor[ll.tab] = col + row * 11;
        }
    } else if (ll.grid == 0) {
        ll_move_list(&pad->b);
    } else if (ll.grid == 1) {
        ll_move_grid(&pad->b);
    }
    if (changed) {
        unsigned char t = ll.tab;
        unsigned short top = ll.top[t];
        unsigned short col;
        unsigned short row;

        if (!ll.grid) {
            col = 1;
            row = ll.cursor[t];
            ll_page_fit(top + ll.cursor[t], 0x701, ll.count[t], &top, &col, &row);
            ll.top[ll.tab] = top;
            ll.cursor[ll.tab] = row;
        } else {
            unsigned short c = ll.cursor[t];
            col = c % 11;
            row = c / 11;
            ll_page_fit(top + c, 0x60B, ll.count[t], &top, &col, &row);
            ll.top[ll.tab] = top;
            ll.cursor[ll.tab] = col + row * 11;
        }
    }
    ll_update_cursor();
    ll_select_card();
    ll_scroll_pad(&ll.scroll);
}

/* card info state: fade, BOOT card info screen, fade back to the list */
void ll_info_update(void)
{
    LlEntry *e;
    void *win;

    switch (ll.step) {
    case 0:
        fade_start_in(1, 0xFF000000, 16);
        ll.step++;
    case 1:
        if (!fade_is_done())
            break;
        e = ll_entry(ll.tab);
        if (e == 0) {
            ll.info = 0;
            fade_start(1, 0xFF000000, 16);
            ll.step = 3;
        } else {
            cardinfo_open(e->id, llres_get(0x800C), llres_get(0x8012));
            cardinfo_set_lists(llres_get(0x8015), llres_get(0x8036), llres_get(0x8037));
            ll.info = 1;
            ll.step++;
        }
        break;
    case 2:
        if (!cardinfo_update())
            break;
        ll.info = 0;
        fade_start(1, 0xFF000000, 16);
        ll.step++;
    case 3:
        if (fade_is_done()) {
            ll.step = 0;
            ll.state = 2;
        }
        break;
    }
}

/* title animation of the tab */
void ll_set_tab_anim(int tab)
{
    void *e = llres_get(0x1010);

    if (e)
        ehf_set_anim(e, tab, tab + 1);
}

#ifdef NON_MATCHING /* forma igual (o clamp do topo como `top + c > 0 ? top + c : 0` dá o blezl); só a ordem dos loads iniciais (top, rep, count, cur) e os temporários t* diferem */
/* cursor of the name list (7 rows), wrapping at both ends */
void ll_move_list(PadBits *pad)
{
    unsigned int rep = pad->rep;
    unsigned int t = ll.tab;
    int top = ll.top[t];
    int cur = ll.cursor[t];
    int count = ll.count[t];

    int changed = 0;
    int d = 0;
    int c;

    if (rep & 0x40)
        d++;
    if (rep & 0x10)
        d--;
    if (top + cur == 0 && (pad->trig & 0x10)) {
        top = count >= 7 ? count - 7 : 0;
        c = 6;
    } else if (top + cur >= count - 1 && cur == 6 && (pad->trig & 0x40)) {
        c = 0;
        top = 0;
    } else {
        c = cur + d;
        if (c < 0) {
            top = top + c > 0 ? top + c : 0;
            c = 0;
        } else if (c >= 7) {
            if (count < 7)
                top = 0;
            else if (top + c >= count)
                top = count - 7;
            else
                top += c - 6;
            c = 6;
        }
    }
    if (cur != c) {
        ll.cursor[t] = c;
        changed = 1;
    }
    if (ll.top[ll.tab] != top) {
        ll.top[ll.tab] = top;
        changed = 1;
    }
    if (changed)
        se_play(0x6212);
}
#endif

#ifdef NON_MATCHING /* forma igual (clamp `top + row > 0 ? top + row : 0`, `rows > 6`, col antes de row); só sobra a troca t3/t4 entre t*2 (com dy) e changed no prólogo */
/* cursor of the picture grid (11 x 6), wrapping at both ends */
void ll_move_grid(PadBits *pad)
{
    unsigned int t = ll.tab;
    unsigned int rep = pad->rep;
    int cur = ll.cursor[t];
    int col = cur % 11;
    int row = cur / 11;
    int top = (ll.top[t] + 10) / 11;
    int rows = (ll.count[t] + 10) / 11;
    int dx;
    int dy;
    int c;
    int changed;

    dy = 0;
    dx = 0;
    changed = 0;
    if (rep & 0x40)
        dy++;
    if (rep & 0x10)
        dy--;
    if (rep & 0x20)
        dx++;
    if (rep & 0x80)
        dx--;
    col = (col + dx + 11) % 11;
    if (top + row == 0 && (pad->trig & 0x10)) {
        top = rows >= 6 ? rows - 6 : 0;
        row = 5;
    } else if (top + row >= rows - 1 && row == 5 && (pad->trig & 0x40)) {
        row = 0;
        top = 0;
    } else {
        row += dy;
        if (row < 0) {
            top = top + row > 0 ? top + row : 0;
            row = 0;
        } else if (row >= 6) {
            if (rows > 6) {
                if (top + row >= rows)
                    top = rows - 6;
                else
                    top += row - 5;
            }
            row = 5;
        }
    }
    c = col + row * 11;
    if (cur != c) {
        ll.cursor[t] = c;
        changed = 1;
    }
    top *= 11;
    if (ll.top[ll.tab] != top) {
        ll.top[ll.tab] = top;
        changed = 1;
    }
    if (changed)
        se_play(0x6212);
}
#endif

void ll_update_cursor(void)
{
    LlRect *r = &ll.cur;
    int c = ll.cursor[ll.tab];

    if (!ll.grid) {
        r->x = 152;
        r->y = c * 30 + 42;
        r->w = 315;
        r->h = 29;
    } else {
        r->x = c % 11 * 28 + 156;
        r->y = c / 11 * 34 + 46;
        r->w = 23;
        r->h = 31;
    }
}

/* 1 when the cursor moved to another card (its text starts again from the top) */
int ll_select_card(void)
{
    int card = ll.scroll.card;
    LlEntry *e = ll_entry(ll.tab);
    int id;

    if (e == 0)
        return 0;
    id = e->id;
    if (id == 0)
        return 0;
    if (card == id)
        return 0;
    ll.scroll.card = id;
    ll.scroll.pos = 0;
    return 1;
}

/* by attribute icon, then by the database sort key */
int ll_card_cmp(const void *a, const void *b)
{
    unsigned short x = card_get_attribute_icon(*(const unsigned short *)a);
    unsigned short y = card_get_attribute_icon(*(const unsigned short *)b);

    if (x < y)
        return -1;
    if (x > y)
        return 1;
    x = card_get_sort(*(const unsigned short *)a);
    y = card_get_sort(*(const unsigned short *)b);
    return x - y;
}

/* first card of the player's three decks with more copies than allowed (0 = all legal) */
unsigned short ll_find_illegal(void)
{
    int forbidden;
    int i;
    int j;
    int n;
    int k;

    LlCount cards[126];
    unsigned short decks[3] = { 1, 3, 2 };
    int extra = sysflag_test(0) != 0;

    forbidden = 0;
    memset(cards, 0, sizeof(cards));
    n = 0;
    for (i = 0; i < 3; i++) {
        k = deck_count(decks[i]);
        for (j = 0; j < k; j++)
            n = ll_count_add(cards, (int)deck_get_card(decks[i], j), n);
    }
    for (k = 0; k < n; k++) {
        unsigned short id = cards[k].id;
        int lim = card_get_limit(id);
        int copies = cards[k].n;
        if (lim < 3) {
            if (lim == 0) {
                forbidden++;
                if (forbidden > extra)
                    return id;
            } else if (lim < copies) {
                return id;
            }
        }
    }
    return 0;
}

/* adds a copy of `id` to the n counted cards; the new count */
int ll_count_add(LlCount *c, unsigned short id, int n)
{
    int i;

    for (i = 0; i < n; i++) {
        if (c[i].id == id) {
            c[i].n++;
            return n;
        }
    }
    c[n].id = id;
    c[n].n = 1;
    return n + 1;
}

/* keeps entry `pos` visible in a page of layout (rows << 8 | cols): first entry, column, row */
void ll_page_fit(int pos, int layout, int count, unsigned short *top, unsigned short *col, unsigned short *row)
{
    int cols = layout & 0xFF;
    int rows = (unsigned short)layout >> 8 & 0xFF;
    int n = (unsigned short)count;
    unsigned short page = cols * rows;
    int old = *top;

    if (n < 2) {
        *row = 0;
        *col = 0;
        *top = 0;
        return;
    }
    if (page >= n)
        *top = 0;
    else if (old + page - cols >= n)
        *top = cols * ((n + cols - 1) / cols - rows);
    if (old != *top)
        *row += (short)((short)old - (short)*top) / cols;
    if (pos >= n)
        pos = n - 1;
    if (pos >= 0) {
        unsigned short r = pos / cols;
        if (r < rows)
            *top = 0;
        else
            *top = cols * (r - (rows - 1));
        *col = pos % cols;
        *row = r - *top / cols;
    }
}
