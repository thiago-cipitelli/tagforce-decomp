// @range 0x08807CD0-0x08808E5C
// @data 0x0880DC50-0x0880DCB8
/* The album contents and the page stack: the card list in sort order (owned cards plus the empty
   slots of the missing ones, the special cards appended on extra pages), jumps to a card index or to
   the next/previous group of the sort order, the card detail screen, fast turning while L/R is held,
   the skip-menu animation, and the list of page objects (created, shifted, freed, placed by slot). */
#include "libcardalbum.h"

typedef struct CaSlot {        /* position of each stack slot */
    float x, y, z;
} CaSlot;

int card_get_sort(int id);
int card_get_flag31(int id);
int card_is_attribute9(int id);
int card_get_sort_position(int id);
int ca_is_extra(unsigned short id);
int ca_cur_card(void);
unsigned short ca_cur_index(void);
CaPage *ca_cur_page(void);
CaPage *ca_page_find(unsigned short slot);
void ca_page_set_kind(CaPage *p, int kind, float speed);
void ca_turn_back(float speed);
void ca_turn_fwd(float speed);
void ca_pages_place(void);
void ca_cursor_fix(void);
int cardinfo_open(unsigned short card_id, void *window_gim, void *bg_gim);
int cardinfo_has_alt_pict(void);
void cardinfo_set_lists(void *list_gim, void *text_pack, void *index);
int cardinfo_update(void);
int ehf_set_anim(void *e, int target, int limit);
int ehf_get_frame_count(void *e);
void ehf_get_size(void *e, int *target, int *limit);

/* cards that go to the extra pages instead of their place in the sort order */
unsigned short ca_extra_ids[3] = { 0x143D, 0x1445, 0x1446 };

int ca_is_extra(unsigned short id)
{
    int lo = 0;
    int hi = 2;
    int mid;
    unsigned short v;

    while (lo <= hi) {
        mid = (lo + hi) / 2;
        v = ca_extra_ids[mid];
        if (id == v)
            return 1;
        if (v < id)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 0;
}

/* every card in sort order: owned ones, 0 for a missing slot; the extra cards owned go after the
   last normal page */
void ca_list_build(void)
{
    unsigned short sorted[0x9B0];
    unsigned short extra[500];
    int n_extra;
    unsigned short c;
    unsigned short *src;
    unsigned short *dst;
    int i;
    int n;
    int owned;
    int id;
    int k;

    memset(sorted, 0, sizeof(sorted));
    for (id = 0xFA7; id < 0x1B2E; id++) {
        k = card_get_sort(id);
        if (k > 0)
            sorted[k - 1] = id;
    }
    n = 0;
    n_extra = 0;
    owned = 0;
    memset(ca.cards, 0, 0x1326);
    for (i = 0, src = sorted, dst = extra; i < 0x9B0; i++, src++) {
        c = *src;
        if (c != 0 && card_get_flag31(c)) {
            if (ca_is_extra(c)) {
                if (inv_get_state(c)) {
                    *dst = c;
                    dst++;
                    n_extra++;
                }
            } else if (!card_is_attribute9(c)) {
                if (inv_get_state(c) == 0) {
                    ca.cards[n] = 0;
                    n++;
                } else {
                    ca.cards[n] = c;
                    n++;
                    owned++;
                }
            }
        }
    }
    ca.n_cards = n;
    ca.permille = owned * 1000 / n;
    if (n_extra > 0) {
        n = CA_PAGES(n) * CA_PER_PAGE;
        memcpy(&ca.cards[n], extra, n_extra * 2);
    }
    ca.n_total = n + n_extra;
}

/* puts the cursor on the card at `index`, turning (animated) to its page when needed */
int ca_goto_index(int index)
{
    int page;
    int cell;
    int k;
    int cur;
    CaPage *t;

    if (index < 0)
        return 0;
    if (index >= ca.n_total)
        return 0;
    page = index / CA_PER_PAGE;
    k = index % CA_PER_PAGE;
    if (k < 9) {
        cell = k % 3 + k / 3 * 6 + 3;
        t = ca_page_find(4);
        if (t)
            cur = t->page;
        else
            cur = CA_PAGES(ca.n_total);
    } else {
        k -= 9;
        cell = k + k / 3 * 3;
        t = ca_page_find(3);
        if (t)
            cur = t->page;
        else
            cur = -1;
    }
    if (page == cur) {
        ca.cur.pos = cell;
        ca_cursor_fix();
        return 1;
    }
    ca.jump.active = 1;
    ca.jump.pos = cell;
    ca.jump.page = page;
    if (page < cur) {
        k = cur - page;
        if (k > 0x32)
            ca.jump.speed = 12.5f;
        else if (k > 0x14)
            ca.jump.speed = 6.25f;
        else
            ca.jump.speed = 3.125f;
        ca_turn_back(ca.jump.speed);
    } else if (cur < page) {
        k = page - cur;
        if (k > 0x32)
            ca.jump.speed = 12.5f;
        else if (k > 0x14)
            ca.jump.speed = 6.25f;
        else
            ca.jump.speed = 3.125f;
        ca_turn_fwd(ca.jump.speed);
    }
    return 1;
}

/* card detail screen (BOOT cardinfo) over a fade */
void ca_detail_update(void)
{
    switch (ca.step) {
    case 0:
        fade_start_in(1, 0xFF000000, 16);
        ca.step++;
    case 1:
        if (fade_is_done()) {
            cardinfo_open(ca_cur_card(), ca_res_get(0x1006), ca_res_get(0x1009));
            cardinfo_has_alt_pict();
            cardinfo_set_lists(ca_res_get(0x100A), ca_res_get(0x1031), ca_res_get(0x1032));
            ca.detail = 1;
            ca.step++;
        }
        break;
    case 2:
        if (cardinfo_update() == 0)
            break;
        ca.detail = 0;
        fade_start(1, 0xFF000000, 16);
        ca.step++;
    case 3:
        if (fade_is_done()) {
            ca.step = 0;
            ca.state = 3;
        }
        break;
    }
}

/* holding the turn button speeds the turning up to 5x */
void ca_hold_update(void)
{
    PadState *pad = pad_get();
    unsigned int held = ca.held;

    if (held) {
        if (held & pad->b.down) {
            if (held & pad->b.rep) {
                if (held == 0x80 || held == 0x100) {
                    ca.speed_back += 0.5f;
                    if (ca.speed_back > 5.0f)
                        ca.speed_back = 5.0f;
                }
                if (ca.held == 0x20 || ca.held == 0x200) {
                    ca.speed_fwd += 0.5f;
                    if (ca.speed_fwd > 5.0f)
                        ca.speed_fwd = 5.0f;
                }
                if (ca.held == region_pick_value(0)) {
                    if (ca.cur.side == 0) {
                        ca.speed_back += 0.5f;
                        if (ca.speed_back > 5.0f)
                            ca.speed_back = 5.0f;
                    } else {
                        ca.speed_fwd += 0.5f;
                        if (ca.speed_fwd > 5.0f)
                            ca.speed_fwd = 5.0f;
                    }
                }
            }
        } else {
            ca.held = 0;
            ca.speed_fwd = 1.0f;
            ca.speed_back = 1.0f;
        }
    }
}

/* moves the cursor to the other side when its side has no page (first and last pages) */
void ca_cursor_fix(void)
{
    unsigned char d;

    if (ca_cur_page() == 0) {
        if (ca.cur.pos % 6 < 3) {
            if (ca_page_find(4)) {
                d = 3 - ca.cur.pos % 3;
                ca.cur.pos = ca.cur.pos + d;
                if (ca.cur.area == 1)
                    ca.cur.area = 0;
            }
        } else {
            if (ca_page_find(3)) {
                d = ca.cur.pos % 3 + 1;
                ca.cur.pos = ca.cur.pos - d;
                if (ca.cur.area == 1)
                    ca.cur.area = 0;
            }
        }
    }
}

/* first card of the next (back = 0) or previous (back = 1) group of the sort order; -1 if none */
int ca_find_group(int back)
{
    int cur = (unsigned short)ca_cur_index();
    int n = ca.n_total;
    int found;
    int v;
    int i;
    int group = -1;
    int k;

    if (back) {
        if (cur <= 0)
            return group;
        for (i = cur - 1; i > 0; i--) {
            if (i < ca.n_cards) {
                v = ca.cards[i];
                if (v != 0) {
                    group = card_get_sort_position(v);
                    break;
                }
            }
        }
        if (i <= 0)
            return -1;
        for (i--; i >= 0; i--) {
            found = 0;
            v = ca.cards[i];
            if (group == 0) {
                found = 1;
                i = 0;
            } else if ((unsigned short)v != 0) {
                k = card_get_sort_position((unsigned short)v);
                if (k != group)
                    found = 1;
            }
            if (found) {
                i++;
                while (i < n) {
                    if (ca.cards[i] != 0)
                        break;
                    i++;
                }
                break;
            }
        }
        if (i < 0)
            return -1;
    } else {
        if (ca.cards[cur] != 0)
            group = card_get_sort_position(ca.cards[cur]);
        for (i = cur + 1; i < n; i++) {
            if (i >= ca.n_cards)
                group = -1;
            if (ca.cards[i] != 0) {
                k = card_get_sort_position(ca.cards[i]);
                if (k != group)
                    break;
            }
        }
        if (i >= n)
            return -1;
    }
    if (i == ca_cur_index())
        return -1;
    return i;
}

/* skip menu animation: 6 opening, 7 idle, 8 hidden, 0..5 pressing an entry */
void ca_skipmenu_anim(int which)
{
    void *e = ca_res_get(CA_SKIPMENU);
    int to;
    int from;

    if (e) {
        switch (which) {
        case 6:
            from = 0;
            to = 9;
            break;
        case 7:
            to = 9;
            from = to;
            break;
        case 8:
            to = 0;
            from = 0;
            break;
        default:
            from = which * 5 + 10;
            to = from + 4;
            break;
        }
        ehf_set_anim(e, from, to + 1);
    }
}

/* holds the last frame once an animation has played */
void ca_skipmenu_update(void)
{
    void *e = ca_res_get(CA_SKIPMENU);
    int target;
    int limit;
    int n;

    if (e) {
        ehf_get_size(e, &target, &limit);
        n = ehf_get_frame_count(e);
        if (limit > 0 && n >= limit - 1)
            ehf_set_anim(e, limit - 1, limit);
    }
}

/* where each of the 8 stack slots sits (0 and 7 off screen) */
CaSlot ca_slots[8] = {
    { -0.12f, -0.12f, 0.0f },
    { -0.12f, -0.08f, 0.0f },
    { -0.06f, -0.04f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
    { 0.06f, -0.04f, 0.0f },
    { 0.12f, -0.08f, 0.0f },
    { 0.12f, -0.12f, 0.0f },
};

/* new page object for album page `page` in `slot`, inserted in the list by its y */
CaPage *ca_page_new(int slot, int kind, int page)
{
    CaPage *q = ca.pages;      /* walks the list, then becomes the new page */
    float y = ca_slots[slot].y;
    CaPage *prev = 0;
    CaPage *next = 0;
    int i;
    unsigned short id;
    int base;
    int k;

    if (page < 0 || page >= CA_PAGES(ca.n_total))
        return 0;
    for (; q; q = q->next) {
        if (y < q->pos[1]) {
            next = q;
            break;
        }
        prev = q;
    }
    q = heap_alloc(ca.heap, 0xC0);
    memset(q, 0, 0xC0);
    if (prev)
        prev->next = q;
    else
        ca.pages = q;
    q->prev = prev;
    q->next = next;
    if (next)
        next->prev = q;
    q->slot = slot;
    q->live = 1;
    base = page * CA_PER_PAGE;
    for (i = 0; i < CA_PER_PAGE; i++) {
        k = base + i;
        if (k >= ca.n_total) {
            q->cards[i] = 0;
        } else {
            id = ca.cards[k];
            inv_get_hi(id);
            q->cards[i] = id;
        }
    }
    q->page = page;
    q->pos[0] = ca_slots[slot].x;
    q->pos[1] = y;
    q->pos[2] = ca_slots[slot].z;
    ca_page_set_kind(q, kind, 1.0f);
    return q;
}

void ca_pages_free(void)
{
    CaPage *p = ca.pages;
    CaPage *next;

    ca.pages = 0;
    while (p) {
        next = p->next;
        heap_set_level(ca.heap, p);
        p = next;
    }
}

/* after a page turned back: every page moves one slot left, the ones leaving slot 0 are freed */
void ca_pages_shift_back(void)
{
    CaPage *prev = 0;
    CaPage *p = ca.pages;
    CaPage *cur;

    while (p) {
        if (p->slot < 2) {
            cur = p;
            p = p->next;
            if (prev == 0) {
                ca.pages = p;
                p->prev = 0;
            } else {
                prev->next = p;
                if (p)
                    p->prev = prev;
            }
            heap_set_level(ca.heap, cur);
        } else {
            p->slot--;
            prev = p;
            p = p->next;
        }
    }
    ca_pages_place();
}

/* after a page turned forward: every page moves one slot right, the ones leaving slot 7 are freed */
void ca_pages_shift_fwd(void)
{
    CaPage *prev = 0;
    CaPage *p = ca.pages;
    CaPage *cur;

    while (p) {
        if (p->slot >= 6) {
            cur = p;
            p = p->next;
            if (prev == 0) {
                ca.pages = p;
                p->prev = 0;
            } else {
                prev->next = p;
                if (p)
                    p->prev = prev;
            }
            heap_set_level(ca.heap, cur);
        } else {
            p->slot++;
            prev = p;
            p = p->next;
        }
    }
    ca_pages_place();
}

void ca_pages_place(void)
{
    CaPage *p;

    for (p = ca.pages; p; p = p->next) {
        p->pos[0] = ca_slots[p->slot].x;
        p->pos[1] = ca_slots[p->slot].y;
        p->pos[2] = ca_slots[p->slot].z;
    }
}

CaPage *ca_page_find(unsigned short slot)
{
    CaPage *p = ca.pages;

    if (p) {
        do {
            if (p->slot == slot)
                return p;
            p = p->next;
        } while (p);
    }
    return 0;
}
