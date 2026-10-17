// @range 0x088433F0-0x08843D78
// @rodata 0x08875600-0x088756D8
// @asm 0x08843600-0x08843918
// @asm 0x088439FC-0x08843A68
/* Menu/window layer: item registry (16 slots on the caller's heap), draw-order layout, item state
   and geometry. Rendering itself is in menu_draw.c. The paddings live in .rodata (asm). */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "game.h"
#include "menu.h"

unsigned char *syspack_get(void);
void *ehp_find(void *pack, const char *name, int flags);
void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
void *dl_select_layer(int layer, unsigned int *size_out);
void dl_set_list(void *list);
void font_set_size(int w, int h);
int font_get_flag4(void);
int font_get_flag8000(void);
void font_set_flag4(int v);
void font_set_flag8000(int v);
int font_measure_wtext2(int maxw, int maxh, unsigned short *text);

const char window01_gim[] = "window01.gim";
const WPiece wframe_a[10] = {
    { 0, 0, 3, 3 }, { 28, 0, 4, 3 }, { 0, 13, 3, 4 }, { 28, 13, 4, 4 }, { 3, 0, 1, 3 },
    { 0, 3, 3, 1 }, { 28, 3, 4, 1 }, { 3, 13, 1, 4 }, { 3, 3, 1, 8 }, { 9, 13, 7, 10 },
};
const WPiece wtitle[4] = { { 36, 0, 1, 4 }, { 32, 0, 4, 24 }, { 52, 0, 12, 24 }, { 36, 4, 1, 1 } };
const int wstep_a[2] = { 0, 24 };
const WPiece wframe_b[9] = {
    { 0, 96, 2, 2 }, { 13, 96, 3, 2 }, { 0, 109, 2, 3 }, { 13, 109, 3, 3 }, { 2, 96, 1, 2 },
    { 0, 98, 2, 1 }, { 13, 99, 3, 1 }, { 3, 109, 1, 3 }, { 2, 98, 1, 11 },
};
const int wstep_b[2] = { 16, 0 };

void menu_init(Menu *m, void *heap)
{
    memset(m, 0, sizeof(Menu));
    m->window_gim = ehp_find(syspack_get(), window01_gim, 0);
    m->heap = heap;
}

int menu_add_item(Menu *m, MenuItemDef *def)
{
    MenuItem *it;
    int slot;
    void *dl;

    if (def == 0)
        return 0;
    if (m->count >= 16)
        return 0;
    it = heap_alloc(m->heap, sizeof(MenuItem));
    if (it == 0) {
        if (it != 0)
            heap_set_level(m->heap, it);
        return 0;
    }
    memset(it, 0, sizeof(MenuItem));
    it->verts = 0;
    it->nverts = 0;
    it->nverts2 = 0;
    it->def = def;
    it->state = 0;
    it->counter = 0;
    dl = dl_select_layer(4, 0);
    menu_item_build(&dl, it, m);
    dl_set_list(dl);
    it->cur = *it->def;
    slot = menu_find_free(m);
    if (slot == -1) {
        if (it != 0)
            heap_set_level(m->heap, it);
        return 0;
    }
    m->items[slot] = it;
    m->count++;
    return 1;
}

void menu_free_items(Menu *m)
{
    int i;

    for (i = 0; i < 16; i++)
        menu_slot_free(&m->items[i], m->heap);
    memset(m, 0, sizeof(Menu));
}

#ifdef NON_MATCHING
/* Structure matches (register-agnostic diff: 198 = 198 instructions); mwcc assigns the loop
   temporaries to other registers (i->a0, k->v1 in the original). Kept in asm. */
/* Builds every item in ascending def->order (insertion sort into a local index list). */
void menu_layout(void **dl, Menu *m)
{
    int order[16];
    int count = 0;
    int i, k, j;
    MenuItem *it;

    memset(order, -1, sizeof order);
    for (i = 0; i < 16; i++) {
        if (m->items[i] == 0)
            continue;
        for (k = 0; k < count; k++) {
            if (m->items[i]->def->order < m->items[k]->def->order)
                break;
        }
        for (j = count - 1; j >= k; j--)
            order[j + 1] = order[j];
        order[k] = i;
        count++;
    }
    for (i = 0; i < count; i++) {
        it = m->items[order[i]];
        menu_item_build(dl, it, m);
        if (it->def->has_text != 0)
            menu_item_build_text(dl, it, m);
        it->cur = *it->def;
    }
}
#endif

int menu_remove_item(Menu *m, MenuItemDef *def)
{
    int i = menu_find_item(m, def);

    if (i != -1) {
        menu_slot_free(&m->items[i], m->heap);
        m->count--;
        return 1;
    }
    return 0;
}

int menu_item_open(Menu *m, MenuItemDef *def)
{
    int i = menu_find_item(m, def);
    MenuItem *it;

    if (i != -1) {
        it = m->items[i];
        it->state = 1;
        it->counter = 0;
        return 1;
    }
    return 0;
}

int menu_item_close(Menu *m, MenuItemDef *def)
{
    int i = menu_find_item(m, def);
    MenuItem *it;

    if (i != -1) {
        it = m->items[i];
        it->state = 3;
        it->counter = 0;
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING
/* The original emits two separate `return 1` blocks for states 4 and 2 and a jump to the next
   instruction; no if/switch form reproduces it. Kept in asm. */
int menu_item_is_open(Menu *m, MenuItemDef *def)
{
    int i = menu_find_item(m, def);

    if (i == -1)
        return 0;
    switch (m->items[i]->state) {
    case 2:
    case 4:
        return 1;
    default:
        return 0;
    }
}
#endif

/* Baseline y of the text cursor after the item's text, in pixels. */
int menu_cursor_y(Menu *m, MenuItemDef *def)
{
    int i = menu_find_item(m, def);
    MenuItemDef *d;
    int f4, f8;
    int w;
    int ww, hh, py;

    if (i != -1) {
        d = m->items[i]->def;
        if (d->text == 0)
            return d->pad_t + (d->y + (MENU_PAD_Y + 1));
        font_set_size(d->font_size, d->font_size);
        f4 = font_get_flag4();
        f8 = font_get_flag8000();
        font_set_flag4(d->font_flag4);
        font_set_flag8000(d->font_flag8000);
        py = MENU_PAD_Y;
        ww = -2 - MENU_PAD_W - MENU_PAD_X - d->pad_r - d->pad_l;
        hh = -2 - MENU_PAD_H - py - d->pad_b - d->pad_t;
        w = font_measure_wtext2((ww + d->w) * 64, (hh + d->h) * 64, d->text); /* 26.6 fixed */
        font_set_flag4(f4);
        font_set_flag8000(f8);
        return (int)((float)(int)(unsigned short)w / 64.0f + (float)(d->pad_t + (d->y + (py + 1))));
    }
    return -1;
}

/* Inner (content) rectangle of the item's window. */
void menu_item_rect(Menu *m, MenuItemDef *def, int *x, int *y, int *w, int *h)
{
    int i = menu_find_item(m, def);
    MenuItemDef *d;
    int t;

    if (i != -1) {
        d = m->items[i]->def;
        *x = (MENU_PAD_X + 1) + (d->x + d->pad_l);
        *y = (MENU_PAD_Y + 1) + (d->y + d->pad_t);
        t = -2 - MENU_PAD_W - MENU_PAD_X - d->pad_r - d->pad_l;
        *w = t + d->w;
        t = -2 - MENU_PAD_H - MENU_PAD_Y - d->pad_b - d->pad_t;
        *h = t + d->h;
    }
}

int menu_item_state(Menu *m, MenuItemDef *def)
{
    int i = menu_find_item(m, def);

    if (i != -1)
        return m->items[i]->state;
    return -1;
}

void menu_slot_free(MenuItem **slot, void *heap)
{
    MenuItem *it = *slot;

    if (it != 0 && it != 0) {
        heap_set_level(heap, it);
        *slot = 0;
    }
}
