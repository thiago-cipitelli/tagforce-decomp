// @range 0x0880681C-0x08807CD0
// @asm 0x088077B8-0x08807CD0
// @rodata 0x0880D694-0x0880D6AC
/* Input of the browse state (card cells, page corners, bottom menu), the prefetch of the pictures of
   the visible sides, the cursor -> card lookups and the page turning (one page at a time, or an
   animated jump to a page that can be skipped with confirm). */
#include "libcardalbum.h"

void se_play(int id);
void ca_index_open(void);
void ca_skipmenu_update(void);
void ca_cursor_fix(void);
void ca_hold_update(void);
int ca_select_update(void);
int ca_cur_card(void);
CaPage *ca_cur_page(void);
CaPage *ca_page_find(unsigned short slot);
CaPage *ca_page_new(int slot, int kind, int page);
void ca_page_set_kind(CaPage *p, int kind, float speed);
void ca_list_build(void);
int ca_goto_index(int index);
int ca_find_group(int back);
void ca_input_cards(void);
void ca_input_corner(void);
void ca_input_menu(void);
void ca_turn_back(float speed);
void ca_turn_fwd(float speed);

void ca_build(void)
{
    ca_list_build();
    ca_skipmenu_anim(8);
    ca.held = 0;
    ca.speed_fwd = 1.0f;
    ca.speed_back = 1.0f;
}

/* browse state: stick scrolls the card text, cancel leaves, select opens the index, L/R turn pages */
void ca_input(void)
{
    PadState *pad = pad_get();
    int y = pad->b.analog_y;
    CaPanel *pn = &ca.panel;
    int d = 0;
    unsigned int trig;

    if (y > 0x9E)
        d = (y - 0x9E) * 4;
    if (y < 0x62)
        d = (y - 0x62) * 4;
    d = (unsigned int)pn->scroll + d;
    if (d <= 0)
        pn->scroll = 0;
    else if ((unsigned int)d >= (unsigned int)pn->desc_max)
        pn->scroll = pn->desc_max;
    else
        pn->scroll = d;
    trig = pad->b.trig;
    if (trig & region_pick_value(1)) {
        ca.state = 8;
        se_play(0x6214);
    } else if (trig & 0x1000) {                /* select */
        ca_index_open();
        se_play(0x6216);
    } else if (pad->b.rep & 0x100) {           /* L */
        ca.held = 0x100;
        ca_turn_back(ca.speed_back);
        se_play(0x621B);
    } else if (pad->b.rep & 0x200) {           /* R */
        ca.held = 0x200;
        ca_turn_fwd(ca.speed_fwd);
        se_play(0x621B);
    } else {
        switch (ca.cur.area) {
        case 0:
            ca_input_cards();
            break;
        case 1:
            ca_input_corner();
            break;
        case 2:
            ca_input_menu();
            break;
        }
    }
    ca_hold_update();
}

/* cursor over the 6x3 card cells; leaving the edges goes to the corners or the menu */
void ca_input_cards(void)
{
    PadState *pad = pad_get();
    int col = ca.cur.pos % 6;
    int row = ca.cur.pos / 6;
    unsigned int trig;

    if (pad->b.rep & 0x80) {                   /* left */
        se_play(0x6212);
        if (col <= 0) {
            ca.cur.area = 1;
            ca.cur.side = 0;
            return;
        }
        col--;
    }
    if (pad->b.rep & 0x20) {                   /* right */
        se_play(0x6212);
        if (col >= 5) {
            ca.cur.area = 1;
            ca.cur.side = 1;
            return;
        }
        col++;
    }
    if (pad->b.rep & 0x10) {                   /* up */
        if (row > 0) {
            row--;
            se_play(0x6212);
        }
    }
    if (pad->b.rep & 0x40) {                   /* down */
        se_play(0x6212);
        if (row >= 2) {
            ca.cur.area = 2;
            ca.cur.item = col;
            return;
        }
        row++;
    }
    ca.cur.pos = col + row * 6;
    ca_cursor_fix();
    ca_select_update();
    trig = pad->b.trig;
    if (trig & region_pick_value(0)) {
        if ((unsigned short)ca_cur_card() != 0) {
            ca.state = 6;
            se_play(0x6213);
        }
    } else if (trig & 0x8000) {                /* square: to the menu */
        ca.cur.area = 2;
        se_play(0x6212);
    }
}

/* cursor on a page corner: pressing towards the edge or confirm turns the page */
void ca_input_corner(void)
{
    PadState *pad = pad_get();

    if (pad->b.rep & 0x80) {
        if (ca.cur.side == 0) {
            ca.held = 0x80;
            ca_turn_back(ca.speed_back);
            se_play(0x621B);
        } else {
            ca.cur.area = 0;
            se_play(0x6212);
        }
    }
    if (pad->b.rep & 0x20) {
        if (ca.cur.side == 1) {
            ca.held = 0x20;
            ca_turn_fwd(ca.speed_fwd);
            se_play(0x621B);
        } else {
            ca.cur.area = 0;
            se_play(0x6212);
        }
    }
    if (pad->b.rep & 0x10) {
        ca.cur.area = 0;
        se_play(0x6212);
    }
    if (pad->b.rep & 0x40) {
        if (ca.cur.side == 0)
            ca.cur.item = 0;
        else
            ca.cur.item = 5;
        ca.cur.area = 2;
        se_play(0x6212);
    }
    if (pad->b.trig & region_pick_value(0)) {
        if (ca.cur.side == 0)
            ca_turn_back(ca.speed_back);
        else
            ca_turn_fwd(ca.speed_fwd);
        ca.held = region_pick_value(0);
        se_play(0x621B);
    }
    ca_select_update();
}

/* bottom menu: first page, previous group, previous page, next page, next group, last page */
void ca_input_menu(void)
{
    PadState *pad = pad_get();
    int i;

    if (pad->b.rep & 0x80) {
        ca.cur.item = (ca.cur.item + 5) % 6;
        se_play(0x6212);
    }
    if (pad->b.rep & 0x20) {
        ca.cur.item = (ca.cur.item + 1) % 6;
        se_play(0x6212);
    }
    if (pad->b.rep & 0x10) {
        ca.cur.area = 0;
        ca_skipmenu_anim(7);
        se_play(0x6212);
    }
    if (pad->b.rep & region_pick_value(0)) {
        se_play(0x6213);
        switch (ca.cur.item) {
        case 0:
            ca_goto_index(0);
            break;
        case 1:
            i = ca_find_group(1);
            if (i >= 0)
                ca_goto_index(i);
            break;
        case 2:
            ca_turn_back(ca.speed_back);
            break;
        case 3:
            ca_turn_fwd(ca.speed_fwd);
            break;
        case 4:
            i = ca_find_group(0);
            if (i >= 0)
                ca_goto_index(i);
            break;
        case 5:
            ca_goto_index(ca.n_total - 1);
            break;
        }
        if (ca.state == 4)
            ca_skipmenu_anim(ca.cur.item);
    } else if (pad->b.trig & 0x8000) {
        ca.cur.area = 0;
        ca_skipmenu_anim(7);
        se_play(0x6212);
    }
    ca_select_update();
}

/* queues the pictures of the open sides (the facing side of each page first) */
void ca_prefetch_update(void)
{
    CaPage *p = ca.pages;
    unsigned char right[9];
    unsigned char left[9];
    int i;
    int j;
    unsigned short id;

    if (p != 0 && ca.state == 3) {
        ca_queue_clear();
        memset(right, 0, sizeof(right));
        memset(left, 0, sizeof(left));
        while (p->next)
            p = p->next;
        for (;;) {
            switch (p->kind) {
            case 1:
                for (i = 17; i >= 0; i--) {
                    id = p->cards[i];
                    if (id != 0) {
                        if (i >= 9) {
                            j = 2 - i % 3 + (i / 3 * 3 - 9);
                            if (right[j] == 0) {
                                right[j] = 1;
                                ca_queue_add(id);
                            }
                        } else {
                            right[i] = 1;
                        }
                    }
                }
                break;
            case 2:
                for (i = 0; i < 18; i++) {
                    id = p->cards[i];
                    if (id != 0) {
                        if (i < 9) {
                            if (left[i] == 0) {
                                left[i] = 1;
                                ca_queue_add(id);
                            }
                        } else {
                            /* left[2 - i % 3 + i / 3 * 3 - 9], written so the address sums in the original order */
                            (&left[2 - i % 3])[i / 3 * 3 - 9] = 1;
                        }
                    }
                }
                break;
            }
            if (p == ca.pages)
                break;
            p = p->prev;
        }
    }
    ca_queue_prefetch();
    ca_skipmenu_update();
}

/* page under the cursor column */
CaPage *ca_cur_page(void)
{
    return ca_page_find(ca.cur.pos % 6 < 3 ? 3 : 4);
}

int ca_cur_card(void)
{
    CaPage *p = ca_cur_page();
    int col;
    int row;
    int i;

    if (p == 0)
        return 0;
    col = ca.cur.pos % 6;
    row = ca.cur.pos / 6;
    if (col < 3)
        i = col + (row * 3 + 9);
    else {
        i = (row - 1) * 3;
        i += col;
    }
    return p->cards[i];
}

/* index in ca.cards of the card under the cursor */
unsigned short ca_cur_index(void)
{
    CaPage *p = ca_cur_page();
    int col;
    int row;
    int i;

    if (p == 0)
        return 0;
    col = ca.cur.pos % 6;
    row = ca.cur.pos / 6;
    if (col < 3)
        i = col + (row * 3 + 9);
    else {
        i = (row - 1) * 3;
        i += col;
    }
    return i + p->page * CA_PER_PAGE;
}

/* shows the card under the cursor on the panel; 1 if it changed */
int ca_select_update(void)
{
    int old = ca.panel.id;
    unsigned short id = ca_cur_card();

    if (id == 0)
        return 0;
    if (old == id)
        return 0;
    ca.panel.id = id;
    ca.panel.scroll = 0;
    return 1;
}

/* the right page turns to the left; a new page enters at the left end of the stack */
void ca_turn_back(float speed)
{
    CaPage *p = ca.pages;
    int page = -1;
    int min = 8;
    CaPage *prev;
    CaPage *next;

    for (; p; p = p->next) {
        if (p->slot == 3) {
            prev = p->prev;
            next = p->next;
            ca_page_set_kind(p, 5, speed);
            if (prev != 0 || next != 0) {      /* move it to the end of the list (drawn last) */
                if (prev)
                    prev->next = next;
                else
                    ca.pages = next;
                if (next)
                    next->prev = prev;
                while (next) {
                    prev = next;
                    next = next->next;
                }
                prev->next = p;
                p->prev = prev;
                p->next = 0;
            }
            ca.state = 4;
            break;
        }
    }
    for (p = ca.pages; p; p = p->next) {
        if (p->slot < min) {
            min = p->slot;
            page = p->page;
        }
    }
    if (min == 1 && page > 0)
        ca_page_new(0, 1, page - 1);
}

/* the left page turns to the right; a new page enters at the right end of the stack */
void ca_turn_fwd(float speed)
{
    CaPage *p = ca.pages;
    int page = -1;
    int max = 0;
    CaPage *prev;
    CaPage *next;

    for (; p; p = p->next) {
        if (p->slot == 4) {
            prev = p->prev;
            next = p->next;
            ca_page_set_kind(p, 4, speed);
            if (prev != 0 || next != 0) {
                if (prev)
                    prev->next = next;
                else
                    ca.pages = next;
                if (next)
                    next->prev = prev;
                while (next) {
                    prev = next;
                    next = next->next;
                }
                prev->next = p;
                p->prev = prev;
                p->next = 0;
            }
            ca.state = 4;
            break;
        }
    }
    for (p = ca.pages; p; p = p->next) {
        if (p->slot > max) {
            max = p->slot;
            page = p->page;
        }
    }
    if (max == 6 && page < CA_PAGES(ca.n_total) - 1)
        ca_page_new(7, 2, page + 1);
}

#ifdef NON_MATCHING /* o último laço (afastar as outras páginas) usa os temporários em outra ordem (p em v1, constantes 4/5 em a1/a2); forma igual */
/* turning state: keeps turning towards the jump target, moves the other pages aside meanwhile */
void ca_turn_update(void)
{
    CaPage *p = ca.pages;
    CaPage *turning = 0;
    PadState *pad;
    int base;
    int i;
    CaPage *t;
    float dx;
    float dz;

    ca_hold_update();
    if (ca.jump.active) {
        pad = pad_get();
        if (pad->b.trig & region_pick_value(0)) {  /* skip: open the target page at once */
            ca_pages_free();
            if (ca.jump.pos % 6 < 3)
                base = ca.jump.page - 3;
            else
                base = ca.jump.page - 4;
            for (i = 1; i < 7; i++) {
                if (base + i >= 0 && base + i < CA_PAGES(ca.n_total))
                    ca_page_new(i, i < 4 ? 1 : 2, base + i);
            }
            ca.jump.active = 0;
            ca.cur.pos = ca.jump.pos;
            se_play(0x6213);
        } else {
            pad = pad_get();
            if (pad->b.trig & region_pick_value(1)) {
                ca.jump.active = 0;
                se_play(0x6214);
            }
        }
    }
    for (; p; p = p->next) {
        switch (p->kind) {
        case 4:
            turning = p;
            if (!ca.jump.active)
                p->speed = ca.speed_fwd;
            break;
        case 5:
            turning = p;
            if (!ca.jump.active)
                p->speed = ca.speed_back;
            break;
        }
    }
    if (turning == 0) {
        if (ca.jump.active) {
            if (ca.jump.pos % 6 < 3)
                t = ca_page_find(3);
            else
                t = ca_page_find(4);
            if (t == 0) {
                ca.jump.active = 0;
                ca.cur.pos = ca.jump.pos;
                ca_cursor_fix();
                ca.state = 3;
            } else if (t->page < ca.jump.page) {
                ca_turn_fwd(ca.jump.speed);
                se_play(0x621B);
            } else if (ca.jump.page < t->page) {
                ca_turn_back(ca.jump.speed);
                se_play(0x621B);
            } else {
                ca.jump.active = 0;
                ca.cur.pos = ca.jump.pos;
                ca_cursor_fix();
                ca.state = 3;
            }
        } else {
            ca.state = 3;
            ca_cursor_fix();
        }
        return;
    }
    dx = turning->speed * 0.06f / 25.0f;
    dz = turning->speed * 0.04f / 25.0f;
    for (t = ca.pages; t; t = t->next) {
        if (t != turning) {
            switch (turning->kind) {
            case 4:
                if (t->slot < 4)
                    t->pos[1] -= dz;
                else
                    t->pos[1] += dz;
                if (t->slot > 1 && t->slot < 7)
                    t->pos[0] -= dx;
                break;
            case 5:
                if (t->slot < 4)
                    t->pos[1] += dz;
                else
                    t->pos[1] -= dz;
                if (t->slot > 0 && t->slot < 6)
                    t->pos[0] += dx;
                break;
            }
        }
    }
}
#endif
