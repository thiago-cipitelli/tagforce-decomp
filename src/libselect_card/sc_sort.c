// @range 0x0880FFCC-0x08810BF4
// @asm 0x0880FFCC-0x08810440
// @asm 0x08810440-0x08810868
// @asm 0x088109CC-0x08810A20
// @data 0x0881B3DC-0x0881B424
/* The sort menu (state 4): a list of the nine sort keys and a small up/down menu beside it; the
   pane is re-sorted live while the cursor moves, and restored on cancel. Sort/direction icons. */
#include "libselect_card.h"
#include "menu.h"

typedef struct ScButton {
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} ScButton;

typedef struct ScSortMenu {            /* sc_sort_menu 0x0881B988, 0x18 bytes */
    unsigned char step;                /* +0x00 0 open, 1 closing */
    unsigned char focus : 2;           /* +0x01 0 key list, 1 direction */
    unsigned char pad01 : 6;
    unsigned short pad02;
    unsigned short *order;             /* +0x04 order before the menu */
    unsigned char sort : 6;            /* +0x08 sort before the menu */
    unsigned char dir : 2;
    unsigned char pad09[3];
    int base_y;                        /* +0x0C */
    MenuWidget *keys;                  /* +0x10 */
    MenuWidget *dirs;                  /* +0x14 */
} ScSortMenu;

extern ScSortMenu *sc_sort_menu;

void *sc_draw_button(void *list, ScButton *b, int x, int y, unsigned int color);
void *sc_draw_shade(void *list, unsigned short side, unsigned short tab);
void sc_sort_menu_place(void);
void sc_sort_menu_free(void);
void *sc_draw_sort_item(void *list, int i, int x, int y);
void *sc_draw_dir_item(void *list, int i, int x, int y);

#ifdef NON_MATCHING /* gravações dos campos dos dois menus em outra ordem */
/* opens the sort menu on the pane with the cursor */
void sc_input_sort(void)
{
    ScList *l = scv.pane[scv.side].list;
    MenuWidget *m;
    int a;
    int b;

    sc_sort_menu = heap_alloc(sc.heap, 0x18);
    if (sc_sort_menu == 0)
        return;
    memset(sc_sort_menu, 0, 0x18);
    sc_sort_menu->keys = heap_alloc(sc.heap, 0x120);
    m = sc_sort_menu->keys;
    if (m != 0) {
        memset(m, 0, 0x120);
        m->def.visible = 1;
        m->def.style = 0;
        m->def.y = 0x3A;
        m->def.pad_t = 2;
        m->def.pad_b = 2;
        m->def.pad_l = 2;
        m->def.pad_r = 2;
        m->def.auto_size = 1;
        m->def.color = 0xFFFFFFFF;
        m->def.open_anim = 2;
        m->def.close_anim = 2;
        m->def.show_title = 1;
        m->def.auto_title = 1;
        m->def.font_flag4 = 1;
        m->def.style2 = 2;
        m->def.title = table_get_entry(res_get(0x8033), 0x55);
        m->def.title_font_size = 0xC;
        m->def.title_color = 0xFFFFFFFF;
        m->heap = sc.heap;
        m->keep_open = 1;
        m->no_cancel = 1;
        m->style = 2;
        m->count = 9;
        m->item_w = 0x88;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)sc_draw_sort_item;
        menu_open(m);
        m->def.x = (0x1E0 - m->def.w) / 2;
        sc_sort_menu->base_y = menu_cursor_y(&m->menu, &m->def) + 1;
    }
    sc_sort_menu->dirs = heap_alloc(sc.heap, 0x120);
    m = sc_sort_menu->dirs;
    if (m != 0) {
        memset(m, 0, 0x120);
        m->def.visible = 1;
        m->def.style = 0;
        m->def.x = sc_sort_menu->keys->def.x + sc_sort_menu->keys->def.w - 4;
        m->def.pad_t = 2;
        m->def.pad_b = 2;
        m->def.pad_l = 2;
        m->def.pad_r = 2;
        m->def.auto_size = 1;
        m->def.color = 0xFFFFFFFF;
        m->def.open_anim = 1;
        m->def.close_anim = 1;
        m->heap = sc.heap;
        m->keep_open = 1;
        m->style = 2;
        m->count = 2;
        font_set_size(12, 12);
        a = font_measure_text(table_get_entry(res_get(0x8033), 0x14));
        b = font_measure_text(table_get_entry(res_get(0x8033), 0x15));
        if (a <= b)
            a = b;
        m->item_w = (a >> 6) + 0x19;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)sc_draw_dir_item;
        menu_open(m);
    }
    sc_sort_menu->order = heap_alloc(sc.heap, l->count << 1);
    if (sc_sort_menu->order != 0)
        memcpy(sc_sort_menu->order, l->order, l->count << 1);
    sc_sort_menu->sort = l->sort;
    sc_sort_menu->dir = l->dir;
    sc_sort_menu->keys->cursor = sc_sort_menu->sort - 1;
    sc_sort_menu->dirs->cursor = sc_sort_menu->dir;
    sc_sort_menu_place();
    sc.state = 4;
    scv.lock = 1;
}
#endif

#ifdef NON_MATCHING /* alocação e ordem dos testes diferentes */
/* state 4 */
void sc_input_search(void)
{
    PadState *pad = pad_get();
    ScList *l = scv.pane[scv.side].list;
    unsigned char k0 = sc_sort_menu->keys->cursor;
    unsigned char d0 = sc_sort_menu->dirs->cursor;
    int r;

    if (sc_sort_menu->step == 1) {
        menu_update(sc_sort_menu->keys);
        menu_update(sc_sort_menu->dirs);
        if (sc_sort_menu->keys->state != 6)
            return;
        sc_sort_menu_free();
        scv.lock = 0;
        sc.state = 1;
        return;
    }
    if (sc_sort_menu->step != 0)
        return;
    sc_sort_menu_place();
    if (sc_sort_menu->focus == 1) {
        r = menu_update(sc_sort_menu->dirs);
        if (pad->b.trig & 0x80) {
            sc_sort_menu->dirs->no_cancel = 0;
            sc_sort_menu->keys->no_cancel = 1;
            sc_sort_menu->focus = 0;
            se_play(0x6212);
        }
    } else if (sc_sort_menu->focus == 0) {
        r = menu_update(sc_sort_menu->keys);
        if (pad->b.trig & 0x20) {
            sc_sort_menu->keys->no_cancel = 0;
            sc_sort_menu->dirs->no_cancel = 1;
            sc_sort_menu->focus = 1;
            se_play(0x6212);
        }
        if (pad->b.trig & 0x300) {
            sc_sort_menu->dirs->cursor = (sc_sort_menu->dirs->cursor + 1) & 1;
            se_play(0x6212);
        }
    }
    if (k0 != sc_sort_menu->keys->cursor || d0 != sc_sort_menu->dirs->cursor) {
        memcpy(l->order, sc_sort_menu->order, l->count << 1);
        sc_list_sort(l, sc_sort_menu->keys->cursor + 1, sc_sort_menu->dirs->cursor);
    }
    if (r != -1) {
        menu_finish(sc_sort_menu->keys);
        menu_finish(sc_sort_menu->dirs);
        sc_sort_menu->step = 1;
        if (r == -2) {
            memcpy(l->order, sc_sort_menu->order, l->count << 1);
            l->sort = sc_sort_menu->sort;
            l->dir = sc_sort_menu->dir;
        } else {
            memcpy(l->order, sc_sort_menu->order, l->count << 1);
            sc_list_sort(l, sc_sort_menu->keys->cursor + 1, sc_sort_menu->dirs->cursor);
        }
    }
}
#endif

void sc_sort_menu_free(void)
{
    menu_reset(sc_sort_menu->dirs);
    menu_reset(sc_sort_menu->keys);
    heap_set_level(sc.heap, sc_sort_menu->dirs);
    heap_set_level(sc.heap, sc_sort_menu->keys);
    heap_set_level(sc.heap, sc_sort_menu);
    sc_sort_menu = 0;
}

void *sc_draw_search(void *list)
{
    unsigned int side;

    if (scv.mode > 1) {
        side = (scv.side + 1) & 1;
        list = sc_draw_shade(list, side, scv.pane[side].list->tab);
    }
    menu_draw(&list, sc_sort_menu->keys);
    menu_draw(&list, sc_sort_menu->dirs);
    return list;
}

ScButton sc_sort_icons[10] = {
    { 0x8000, 0x00, 0x00, 0x00 }, { 0x8008, 0x00, 0x20, 0x06 }, { 0x8008, 0x44, 0x20, 0x07 }, { 0x8008, 0x33, 0x20, 0x08 },
    { 0x8008, 0x11, 0x20, 0x09 }, { 0x8008, 0x22, 0x20, 0x0A }, { 0x8008, 0x66, 0x20, 0x0B }, { 0x8008, 0x55, 0x20, 0x0C },
    { 0x8008, 0x44, 0x31, 0x0D }, { 0x8008, 0x11, 0x31, 0x10 },
};

ScButton sc_dir_icons[2] = { { 0x8008, 0x33, 0x31, 0x14 }, { 0x8008, 0x22, 0x31, 0x15 } };

void *sc_draw_sort_item(void *list, int i, int x, int y)
{
    return sc_draw_button(list, &sc_sort_icons[i + 1], x, y + 1, 0);
}

void *sc_draw_dir_item(void *list, int i, int x, int y)
{
    return sc_draw_button(list, &sc_dir_icons[i], x, y + 1, 0);
}

#ifdef NON_MATCHING /* temporários a1/a2/a3 trocados */
/* keeps the direction menu beside the key under the cursor */
void sc_sort_menu_place(void)
{
    MenuWidget *k = sc_sort_menu->keys;
    MenuWidget *d = sc_sort_menu->dirs;
    int base = sc_sort_menu->base_y;
    unsigned int h = k->item_h;

    d->def.y = base + ((h >> 1) + k->cursor * h) - d->def.h / 2;
}
#endif

void *sc_draw_sort_icon(void *list, int sort, int x, int y)
{
    ScButton *b;
    void *gim;

    if (sort == 0)
        return list;
    {
        b = &sc_sort_icons[sort];
        gim = res_get(b->res);
        if (gim != 0) {
            list = dl_ambient_color(list, 0xFFFFFFFF);
            list = dl_set_tex_filter(list, 0, 0);
            list = gim_set_texture(list, gim);
            list = draw_set_offsets(list, 17, 17, 17, 17);
            list = draw_sprite(list, x, y, 0, b->u, b->v, 0, 0);
        }
    }
    return list;
}

void *sc_draw_dir_icon(void *list, int dir, int x, int y)
{
    ScButton *b = &sc_dir_icons[dir];
    void *gim = res_get(b->res);

    if (gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 17, 17, 17, 17);
        list = draw_sprite(list, x, y, 0, b->u, b->v, 0, 0);
    }
    return list;
}
