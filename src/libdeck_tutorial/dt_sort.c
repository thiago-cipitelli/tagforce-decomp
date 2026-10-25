// @range 0x08817B78-0x08818E94
// @asm 0x088185A0-0x088185F4
// @asm 0x088187C8-0x08818E94
// @data 0x08822520-0x08822578
/* The sort menu (the code of libdeck dk_sortmenu.c, first half: a list of the sort keys and a small
   up/down menu beside it; the pane is re-sorted live while the cursor moves and restored on cancel)
   and the stable sort of a list's visible order by one of nine keys (dk_filter.c, second half). */
#include "libdeck_tutorial.h"

typedef struct DtButton {
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} DtButton;

typedef struct DtSortMenu {            /* dt_sort_menu 0x095D2714, 0x18 bytes, heap allocated */
    unsigned short step;               /* +0x00 0 open, 1 closing */
    unsigned char focus : 2;           /* +0x02 0 key list, 1 direction */
    unsigned char pad02 : 6;
    unsigned char pad03;
    unsigned short *order;             /* +0x04 order before the menu */
    unsigned char sort : 6;            /* +0x08 sort before the menu */
    unsigned char dir : 2;
    unsigned char pad09[3];
    int base_y;                        /* +0x0C */
    MenuWidget *keys;                  /* +0x10 */
    MenuWidget *dirs;                  /* +0x14 */
} DtSortMenu;

extern DtSortMenu *dt_sort_menu;

void *dt_draw_ibutton(void *list, DtButton *b, int x, int y, unsigned int color);
void *dt_draw_shade(void *list, unsigned short side, unsigned short tab, int dim);
void dt_list_sort(DtList *l, unsigned short sort, unsigned short dir);
void dt_sort_menu_place(void);
void dt_menu_lock(MenuWidget *m);
void dt_sort_menu_free(void);
void *dt_draw_sort_item(void *list, int i, int x, int y);
void *dt_draw_dir_item(void *list, int i, int x, int y);
void *dt_draw_fmenu_kind(void *list, int i, int x, int y);
void dt_fmenu_values(PadState *pad);
void dt_list_filter(DtList *l, unsigned int f, unsigned int *mask);
void dt_layout_scroll(int keep, int cur, unsigned short view, unsigned short total,
                      unsigned short *top, unsigned short *cx, unsigned short *cy);
void dt_popup_open(int kind, unsigned short n);

void dt_fmenu_free(void);
void dt_fmenu_open_values(unsigned short f);
int font_measure_text(unsigned short *text);
int card_get_flag31(int id);

/* opens the sort menu on the pane with the cursor */
void dt_sort_menu_open(void)
{
    DtList *l = dtv.pane[dtv.side].list;
    MenuItemDef *d;
    MenuWidget *m;
    int a;
    int b;

    dt_sort_menu = heap_alloc(dts.heap, 0x18);
    if (dt_sort_menu == 0)
        return;
    memset(dt_sort_menu, 0, 0x18);
    dt_sort_menu->keys = heap_alloc(dts.heap, 0x120);
    m = dt_sort_menu->keys;
    if (m != 0) {
        d = &m->def;
        memset(m, 0, 0x120);
        d->visible = 1;
        d->style = 0;
        d->y = 0x3A;
        d->pad_t = 2;
        d->pad_b = 2;
        d->pad_l = 2;
        d->pad_r = 2;
        d->auto_size = 1;
        d->color = 0xFFFFFFFF;
        d->open_anim = 2;
        d->close_anim = 2;
        d->show_title = 1;
        d->auto_title = 1;
        d->font_flag4 = 1;
        d->style2 = 2;
        d->title = table_get_entry(dt_eres_get(0x8033), 0x55);
        d->title_font_size = 0xC;
        d->title_color = 0xFFFFFFFF;
        m->heap = dts.heap;
        m->keep_open = 1;
        m->no_cancel = 1;
        m->style = 2;
        m->count = 9;
        m->item_w = 0x88;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)dt_draw_sort_item;
        m->pad = (PadState *)&dt.pad;
        if (dt.step == 15)
            dt_menu_lock(m);
        menu_open(m);
        d->x = (0x1E0 - d->w) / 2;
        dt_sort_menu->base_y = menu_cursor_y(&m->menu, d) + 1;
    }
    dt_sort_menu->dirs = heap_alloc(dts.heap, 0x120);
    m = dt_sort_menu->dirs;
    if (m != 0) {
        d = &m->def;
        memset(m, 0, 0x120);
        d->visible = 1;
        d->style = 0;
        d->x = dt_sort_menu->keys->def.x + dt_sort_menu->keys->def.w - 4;
        d->pad_t = 2;
        d->pad_b = 2;
        d->pad_l = 2;
        d->pad_r = 2;
        d->auto_size = 1;
        d->color = 0xFFFFFFFF;
        d->open_anim = 1;
        d->close_anim = 1;
        m->heap = dts.heap;
        m->keep_open = 1;
        m->style = 2;
        m->count = 2;
        font_set_size(12, 12);
        a = font_measure_text(table_get_entry(dt_eres_get(0x8033), 0x14));
        b = font_measure_text(table_get_entry(dt_eres_get(0x8033), 0x15));
        a = a > b ? a : b;
        m->item_w = (a >> 6) + 0x19;
        m->item_h = 0x12;
        m->draw_item = (MenuDrawItemFn)dt_draw_dir_item;
        menu_open(m);
    }
    dt_sort_menu->order = heap_alloc(dts.heap, l->count << 1);
    if (dt_sort_menu->order != 0)
        memcpy(dt_sort_menu->order, l->order, l->count << 1);
    dt_sort_menu->sort = l->sort;
    dt_sort_menu->dir = l->dir;
    dt_sort_menu->keys->cursor = dt_sort_menu->sort - 1;
    dt_sort_menu->dirs->cursor = dt_sort_menu->dir;
    dt_sort_menu_place();
    dts.state = 3;
    dtv.lock = 1;
}

/* state 3 */
void dt_sort_menu_update(PadState *pad)
{
    DtList *l = dtv.pane[dtv.side].list;
    unsigned int k0 = dt_sort_menu->keys->cursor;
    unsigned int d0 = dt_sort_menu->dirs->cursor;
    int r;

    switch (dt_sort_menu->step) {
    case 0:
        dt_sort_menu_place();
        switch (dt_sort_menu->focus) {
        case 0:
            r = menu_update(dt_sort_menu->keys);
            if (pad->b.trig & 0x20) {
                dt_sort_menu->keys->no_cancel = 0;
                dt_sort_menu->dirs->no_cancel = 1;
                dt_sort_menu->focus = 1;
                se_play(0x6212);
            }
            if (pad->b.trig & 0x300) {
                dt_sort_menu->dirs->cursor = (dt_sort_menu->dirs->cursor + 1) & 1;
                se_play(0x6212);
            }
            break;
        case 1:
            r = menu_update(dt_sort_menu->dirs);
            if (pad->b.trig & 0x80) {
                dt_sort_menu->dirs->no_cancel = 0;
                dt_sort_menu->keys->no_cancel = 1;
                dt_sort_menu->focus = 0;
                se_play(0x6212);
            }
            break;
        }
        if (k0 != dt_sort_menu->keys->cursor || d0 != dt_sort_menu->dirs->cursor) {
            memcpy(l->order, dt_sort_menu->order, l->count << 1);
            dt_list_sort(l, dt_sort_menu->keys->cursor + 1, dt_sort_menu->dirs->cursor);
        }
        if (r != -1) {
            menu_finish(dt_sort_menu->keys);
            menu_finish(dt_sort_menu->dirs);
            dt_sort_menu->step = 1;
            if (r == -2) {
                memcpy(l->order, dt_sort_menu->order, l->count << 1);
                l->sort = dt_sort_menu->sort;
                l->dir = dt_sort_menu->dir;
            } else {
                memcpy(l->order, dt_sort_menu->order, l->count << 1);
                dt_list_sort(l, dt_sort_menu->keys->cursor + 1, dt_sort_menu->dirs->cursor);
            }
        }
        break;
    case 1:
        menu_update(dt_sort_menu->keys);
        menu_update(dt_sort_menu->dirs);
        if (dt_sort_menu->keys->state == 6) {
            dt_sort_menu_free();
            dtv.lock = 0;
            dts.state = 0;
        }
        break;
    }
}

void dt_sort_menu_free(void)
{
    menu_reset(dt_sort_menu->dirs);
    menu_reset(dt_sort_menu->keys);
    heap_set_level(dts.heap, dt_sort_menu->dirs);
    heap_set_level(dts.heap, dt_sort_menu->keys);
    heap_set_level(dts.heap, dt_sort_menu);
    dt_sort_menu = 0;
}

void *dt_sort_menu_draw(void *list)
{
    unsigned int side;

    if (dtv.mode > 1) {
        side = (dtv.side + 1) & 1;
        list = dt_draw_shade(list, side, dtv.pane[side].list->tab, 1);
    }
    menu_draw(&list, dt_sort_menu->keys);
    menu_draw(&list, dt_sort_menu->dirs);
    return list;
}

DtButton dt_sort_icons[10] = {
    { 0x8000, 0x00, 0x00, 0x00 }, { 0x8008, 0x00, 0x20, 0x06 }, { 0x8008, 0x44, 0x20, 0x07 }, { 0x8008, 0x33, 0x20, 0x08 },
    { 0x8008, 0x11, 0x20, 0x09 }, { 0x8008, 0x22, 0x20, 0x0A }, { 0x8008, 0x66, 0x20, 0x0B }, { 0x8008, 0x55, 0x20, 0x0C },
    { 0x8008, 0x44, 0x31, 0x0D }, { 0x8008, 0x11, 0x31, 0x10 },
};

DtButton dt_dir_icons[2] = { { 0x8008, 0x33, 0x31, 0x14 }, { 0x8008, 0x22, 0x31, 0x15 } };

void *dt_draw_sort_item(void *list, int i, int x, int y)
{
    return dt_draw_ibutton(list, &dt_sort_icons[i + 1], x, y + 1, 0);
}

void *dt_draw_dir_item(void *list, int i, int x, int y)
{
    return dt_draw_ibutton(list, &dt_dir_icons[i], x, y + 1, 0);
}

#ifdef NON_MATCHING /* temporários a1/a2/a3 trocados (como sc_sort_menu_place) */
/* keeps the direction menu beside the key under the cursor */
void dt_sort_menu_place(void)
{
    MenuWidget *k = dt_sort_menu->keys;
    MenuWidget *d = dt_sort_menu->dirs;
    int base = dt_sort_menu->base_y;
    unsigned int h = k->item_h;

    d->def.y = base + ((h >> 1) + k->cursor * h) - d->def.h / 2;
}
#endif

void *dt_draw_sort_icon(void *list, int sort, int x, int y)
{
    DtButton *b;
    void *gim;

    if (sort == 0)
        return list;
    {
        b = &dt_sort_icons[sort];
        gim = dt_eres_get(b->res);
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

void *dt_draw_dir_icon(void *list, int dir, int x, int y)
{
    DtButton *b = &dt_dir_icons[dir];
    void *gim = dt_eres_get(b->res);

    if (gim != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 17, 17, 17, 17);
        list = draw_sprite(list, x, y, 0, b->u, b->v, 0, 0);
    }
    return list;
}


typedef struct DtKey {                 /* qsort record */
    unsigned short idx;                /* entry index */
    unsigned short pos;                /* position before sorting */
    int value;                         /* +0x04 dt_sort_key_value, -2 = none */
} DtKey;

typedef struct DtSearch {              /* dt_search 0x095D2720 */
    unsigned short pos;                /* entries scanned */
    unsigned short total;              /* +0x02 */
    unsigned short n;                  /* +0x04 matches */
    unsigned short pad06;
    unsigned short *out;               /* +0x08 */
    unsigned short *word;              /* +0x0C */
} DtSearch;

void qsort(void *base, unsigned int n, unsigned int size, int (*cmp)(const void *, const void *));
int dt_cmp_up(const void *pa, const void *pb);
int dt_cmp_down(const void *pa, const void *pb);
int dt_sort_key_value(DtEnt *e, int sort);

#ifdef NON_MATCHING /* contador e ponteiro em $s trocados */
* sorts the visible order of l by key sort, dir 0 up, 1 down */
void dt_list_sort(DtList *l, int sort, unsigned short dir)
{
    int i;
    unsigned int n = l->count;
    DtKey *k = heap_alloc(dts.heap, n << 3);
    DtKey *p;
    unsigned short idx;

    for (i = 0, p = k; i < (int)n; i++, p++) {
        idx = l->order[i];
        p->idx = idx;
        p->pos = i;
        p->value = dt_sort_key_value(&l->ents[idx], sort);
    }
    if (dir == 0)
        qsort(k, n, 8, dt_cmp_up);
    else if (dir == 1)
        qsort(k, n, 8, dt_cmp_down);
    for (i = 0; i < (int)n; i++)
        l->order[i] = k[i].idx;
    if (k != 0)
        heap_set_level(dts.heap, k);
    l->sort = sort;
    l->dir = dir;
}
#endif


#ifdef NON_MATCHING /* retornos em blocos próprios com b e o valor no atraso */
int dt_cmp_up(const void *pa, const void *pb)
{
    const DtKey *a = pa;
    const DtKey *b = pb;
    int r;

    if (a->value == b->value) {
        if (a->pos < b->pos)
            r = -1;
        else if (b->pos < a->pos)
            r = 1;
        else
            r = 0;
    } else if (a->value == -2)
        r = 1;
    else if (b->value == -2)
        r = -1;
    else if (a->value < b->value)
        r = -1;
    else if (b->value < a->value)
        r = 1;
    else
        r = 0;
    return r;
}
#endif


#ifdef NON_MATCHING /* idem dt_cmp_up */
int dt_cmp_down(const void *pa, const void *pb)
{
    const DtKey *a = pa;
    const DtKey *b = pb;
    int r;

    if (a->value == b->value) {
        if (b->pos < a->pos)
            r = -1;
        else if (a->pos < b->pos)
            r = 1;
        else
            r = 0;
    } else if (a->value == -2)
        r = 1;
    else if (b->value == -2)
        r = -1;
    else if (a->value < b->value)
        r = 1;
    else if (b->value < a->value)
        r = -1;
    else
        r = 0;
    return r;
}
#endif

unsigned char dt_spell_order[8] = { 9, 9, 2, 1, 4, 0, 3, 0 };
unsigned char dt_trap_order[8] = { 0x13, 0x0A, 0x13, 0x13, 0x0B, 0x13, 0x13, 0 };


#ifdef NON_MATCHING /* cadeia de testes da chave em outra ordem */
* value of key sort for a card (smaller sorts first), -2 none */
int dt_sort_key_value(DtEnt *e, int key)
{
    unsigned short sort = key;
    int v;

    switch (sort) {
    case 1:
        return card_get_sort(e->id);
    case 2:
        return card_get_attribute_icon(e->id);
    case 9:
        return card_get_limit(e->id);
    }
    switch (card_get_attribute_icon(e->id)) {
    case 7:
        if (sort == 8)
            return dt_spell_order[card_get_prop5_hi(e->id)];
        return -2;
    case 8:
        if (sort == 8)
            return dt_trap_order[card_get_prop5_hi(e->id)];
        break;
    default:
        switch (sort) {
        case 3:
            return 0xFFFF - card_get_level(e->id);
        case 4:
            v = card_get_atk_q(e->id);
            return v == 0xFFFF ? 0x10000 : 0xFFFF - v;
        case 5:
            v = card_get_def_q(e->id);
            return v == 0xFFFF ? 0x10000 : 0xFFFF - v;
        case 6:
            return card_get_type(e->id);
        case 7:
            return card_get_kind(e->id);
        }
        break;
    }
    return -2;
}
#endif


#ifdef NON_MATCHING /* cadeia de testes da chave em outra ordem */
* the value shown for key sort (-1 = "?", -2 none) */
int dt_sort_value(DtEnt *e, unsigned short sort)
{
    int v;

    if (sort == 9) {
        v = card_get_limit(e->id);
    } else if (sort == 2) {
        v = card_get_attribute_icon(e->id);
    } else if (sort == 1) {
        v = card_get_sort_position(e->id);
    } else {
        v = card_get_attribute_icon(e->id);
        if (v == 8 || v == 7) {
            if (sort == 8) {
                v = card_get_prop5_hi(e->id);
                if (v == 0)
                    v = 7;
            } else {
                v = -2;
            }
        } else if (sort == 7) {
            v = card_get_kind(e->id);
        } else if (sort == 6) {
            v = card_get_type(e->id);
        } else if (sort == 5) {
            v = card_get_def_q(e->id);
            if (v == 0xFFFF)
                v = -1;
        } else if (sort == 4) {
            v = card_get_atk_q(e->id);
            if (v == 0xFFFF)
                v = -1;
        } else if (sort == 3) {
            v = card_get_level(e->id);
        } else {
            v = -2;
        }
    }
    return v;
}
#endif

