// @range 0x088299C8-0x0882A94C
// @asm 0x08829BBC-0x08829E2C
// @asm 0x0882A1EC-0x0882A294
// @asm 0x0882A594-0x0882A6A4
// @asm 0x0882A6E8-0x0882A83C
// @asm 0x0882A880-0x0882A94C
// @data 0x088769E0-0x088769F0
/* Grid picker of the duel: a window listing the entries first..last in a grid; the player marks
   between min and max of them (the marked ones get a frame) and confirms with an OK button
   window below the grid. */
#include "dd_d3.h"

typedef unsigned int *(*DdPickItemFn)(unsigned int *list, int x, int y, int index);
typedef unsigned int *(*DdPickMarkFn)(unsigned int *list, int x, int y, int w, int h);

typedef struct DdPickCfg {             /* caller's description, 0x2C */
    int first;                         /* +0x00 first entry */
    int last;                          /* +0x04 one past the last entry */
    int cols;                          /* +0x08 */
    unsigned short *title;             /* +0x0C */
    unsigned short *(*name)(int index); /* +0x10 text of the entry under the cursor */
    DdPickItemFn draw_item;            /* +0x14 */
    DdPickMarkFn draw_mark;            /* +0x18 */
    int cell_w;                        /* +0x1C */
    int cell_h;                        /* +0x20 */
    int min_pick;                      /* +0x24 */
    int max_pick;                      /* +0x28 */
} DdPickCfg;

typedef struct DdPick {                /* 0x08C5A888, 0x150 */
    DdPickCfg cfg;                     /* +0x000 */
    int w_cache;                       /* +0x02C */
    int h_cache;                       /* +0x030 */
    int cur_x;                         /* +0x034 */
    int cur_y;                         /* +0x038 9: the OK button */
    int picked[32];                    /* +0x03C entries marked, in order */
    int n_picked;                      /* +0x0BC */
    int state;                         /* +0x0C0 0 picking, 1 on OK, 2 confirmed, 3 cancelled */
    MenuWidget *confirm;               /* +0x0C4 OK button window */
    unsigned short title[64];          /* +0x0C8 */
    int grid_w;                        /* +0x148 */
    int grid_h;                        /* +0x14C */
} DdPick;

extern DdPick dd_pick;

void *dd_alloc(int size);
void dd_free(void *p);
void *dd_heap_get(void);             /* duel heap */
PadState *dd_pad(void);
int func_08825764(void);               /* the picker has the input */
unsigned short region_pick_value(int flag);
unsigned short *wcsncpy(unsigned short *d, const unsigned short *s, unsigned int n);
unsigned char *syspack_get(void);
void *ehp_find(void *pack, const char *name, int *size);
void *gim_set_texture(void *list, void *gim);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
void font_set_size(int w, int h);
unsigned int font_set_color(unsigned int c);
void font_text_set(void *list);
int font_draw_text(int x, int y, int w, unsigned short *text);
void *font_text_clear(void);
int font_measure_text(unsigned short *text);
int font_line_height(void);

void dd_pick_confirm_free(void);
void dd_pick_confirm_open(int x, int y, int w, int h);
unsigned int *dd_pick_confirm_draw(unsigned int *list, int x, int y, int w, int h);
unsigned int *dd_pick_draw_title(unsigned int *list, int x, int y, int w, int h, int index);
int dd_pick_width(MenuWidget *w);
int dd_pick_height(MenuWidget *w);
int dd_pick_has(int index);
void dd_pick_toggle(int index, int add);

void dd_pick_term(void)
{
    dd_pick_confirm_free();
}

void dd_pick_init(DdPickCfg *cfg)
{
    memset(&dd_pick, 0, sizeof(dd_pick));
    dd_pick.cfg = *cfg;
    wcsncpy(dd_pick.title, cfg->title, 64);
    dd_pick.title[63] = 0;
    dd_pick.cfg.title = dd_pick.title;
}

void dd_pick_setup(MenuWidget *w)
{
    int n = dd_pick.cfg.last - dd_pick.cfg.first;
    int rows = (n + dd_pick.cfg.cols - 1) / dd_pick.cfg.cols;

    w->no_input = 1;
    w->no_cancel = 1;
    w->item_w = dd_pick.cfg.cell_w;
    w->item_h = dd_pick.cfg.cell_h;
    w->count = dd_pick.cfg.cols;
    w->def.auto_size = 0;
    dd_pick.grid_w = w->count * w->item_w;
    dd_pick.grid_h = rows * w->item_h;
    w->def.w = dd_pick.grid_w + (w->def.pad_l + w->def.pad_r) + 9;
    w->def.h = dd_pick.grid_h + (w->def.font_size + (w->def.pad_t + w->def.pad_b) + 50);
}

void dd_pick_open_confirm(void)
{
    dd_pick_confirm_open(0, 0, 100, 21);
}

#ifdef NON_MATCHING /* alocação de registradores (forma igual) */
unsigned int *dd_pick_draw(unsigned int *list, MenuWidget *w, int x, int y)
{
    int first = dd_pick.cfg.first;
    int last = dd_pick.cfg.last;
    DdPick *p = &dd_pick;
    int ox, oy;
    int sel;
    int n;
    int i;
    int cx, cy;
    int ty, by;
    int gw;

    ox = (w->item_w - 17) / 2;
    oy = (w->item_h - 17) / 2;
    if (dd_pick.cur_y == 9)
        sel = -1;
    else
        sel = p->cur_x + p->cur_y * w->count;
    list = dd_pick_draw_title(list, x, y, p->grid_w, 20, sel >= 0 ? sel + first : -1);
    n = last - first;
    ty = y + 20;
    for (i = 0; i < n; i++) {
        cx = x + (i % w->count) * w->item_w;
        cy = ty + w->item_h * (i / w->count);
        if (sel == i)
            list = menu_draw_cursor(list, w, cx, cy, p->cfg.cell_w - 1, p->cfg.cell_h - 1);
        list = p->cfg.draw_item(list, cx + ox, cy + oy, i + first);
        if (dd_pick_has(i))
            list = p->cfg.draw_mark(list, cx, cy, p->cfg.cell_w - 1, p->cfg.cell_h - 1);
    }
    by = ty + p->grid_h;
    if (p->confirm != 0) {
        gw = dd_pick_width(w);
        dd_pick_height(w);
        p->confirm->def.x = x + (gw - p->confirm->def.w) / 2 + 1;
        p->confirm->def.y = by;
        menu_draw((void **)&list, p->confirm);
    }
    return list;
}
#endif

void dd_pick_update(MenuWidget *w)
{
    DdPick *p = &dd_pick;
    PadState *pad = dd_pad();
    int first = dd_pick.cfg.first;
    int last = dd_pick.cfg.last;
    int n, rows, cols, len;
    int idx;

    switch (dd_pick.state) {
    case 0:
        if (func_08825764() == 0) {
            menu_update(w);
            break;
        }
        if (pad->b.trig & region_pick_value(0)) {
            if (p->cur_y == 9) {
                if (p->n_picked >= p->cfg.min_pick) {
                    menu_set_cursor(p->confirm, 0);
                    p->state = 1;
                }
            } else {
                idx = p->cur_x + p->cur_y * w->count;
                if (dd_pick_has(idx)) {
                    dd_pick_toggle(idx, 0);
                } else if (p->n_picked < p->cfg.max_pick && !(w->disabled & (1 << idx))) {
                    dd_pick_toggle(idx, 1);
                    if (p->n_picked >= p->cfg.max_pick)
                        p->cur_y = 9;
                }
            }
            break;
        }
        if (pad->b.trig & region_pick_value(1)) {
            if (p->n_picked > 0) {
                dd_pick_toggle(p->picked[p->n_picked - 1], 0);
            } else if (!w->cancel_blocked) {
                w->cancelled = 1;
                menu_finish(w);
                p->state = 3;
                break;
            }
        }
        cols = w->count;
        n = last - first;
        rows = (n + cols - 1) / cols;
        if (pad->b.rep & 0x10) {
            if (p->cur_y == 0) {
                p->cur_y = 9;
            } else {
                if (p->cur_y == 9)
                    p->cur_y = rows - 1;
                else
                    p->cur_y = p->cur_y - 1;
                if (p->cur_x + p->cur_y * cols >= n)
                    p->cur_y = p->cur_y - 1;
            }
        }
        if (pad->b.rep & 0x40) {
            if (p->cur_y == 9)
                p->cur_y = 0;
            else
                p->cur_y = p->cur_y + 1;
            if (p->cur_x + p->cur_y * cols >= n)
                p->cur_y = 9;
        }
        len = cols;
        if (p->cur_y == rows - 1) {
            len = n % w->count;
            if (len == 0)
                len = w->count;
        }
        if (p->cur_y != 9) {
            if (pad->b.rep & 0x80)
                p->cur_x = (p->cur_x + len - 1) % len;
            if (pad->b.rep & 0x20)
                p->cur_x = (p->cur_x + 1) % len;
        }
        break;
    case 1:
        menu_update(p->confirm);
        if (p->confirm->state != 4) {
            menu_finish(w);
            p->state = 2;
        }
        break;
    case 2:
    case 3:
        menu_update(w);
        if (w->state == 6)
            dd_pick_confirm_free();
        break;
    }
}

#ifdef NON_MATCHING /* alocação de registradores (forma igual) */
int dd_pick_width(MenuWidget *w)
{
    DdPick *p = &dd_pick;
    int v;

    if (p->w_cache > 0)
        return p->w_cache;
    v = 0;
    v += p->cfg.cell_w * p->cfg.cols;
    p->w_cache = v;
    return v;
}
#endif

#ifdef NON_MATCHING /* alocação de registradores (forma igual) */
int dd_pick_height(MenuWidget *w)
{
    DdPick *p = &dd_pick;
    int v;
    int n;

    if (p->h_cache > 0)
        return p->h_cache;
    v = 0;
    v += 20;
    n = p->cfg.last - p->cfg.first;
    v += p->cfg.cell_h * ((n + p->cfg.cols - 1) / p->cfg.cols) + 1;
    v += 21;
    p->h_cache = v;
    return v;
}
#endif

unsigned int dd_pick_mask(void)
{
    DdPick *p = &dd_pick;
    unsigned int m = 0;
    int i;

    for (i = 0; i < p->n_picked; i++)
        m |= 1 << p->picked[i];
    return m << p->cfg.first;
}

unsigned int *dd_pick_draw_title(unsigned int *list, int x, int y, int w, int h, int index)
{
    DdPick *p = &dd_pick;
    void *gim;
    unsigned short *text;

    y += (h - 16) / 2;
    gim = ehp_find(syspack_get(), "window01.gim", 0);
    if (gim != 0) {
        list = dl_enable(list, 9);
        list = gim_set_texture(list, gim);
    }
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = draw_set_offsets(list, 4, 16, 4, 16);
    list = draw_sprite(list, x, y, 0, 0, 96, 0, 0);
    list = draw_set_offsets(list, 4, 16, w - 9, 16);
    list = draw_sprite(list, x + 4, y, 0, 4, 96, 0, 0);
    list = draw_set_offsets(list, 5, 16, 5, 16);
    list = draw_sprite(list, x + w - 5, y, 0, 11, 96, 0, 0);
    if (index >= 0) {
        text = p->cfg.name(index);
        if (text != 0) {
            font_set_size(12, 12);
            font_set_color(0xFFFFFFFF);
            font_text_set(list);
            font_draw_text((x + 4) << 6, (y + 2) << 6, (480 - (x + 4)) << 6, text);
            list = font_text_clear();
        }
    }
    return list;
}

#ifdef NON_MATCHING /* alocação de registradores (forma igual) */
void dd_pick_confirm_open(int x, int y, int w, int h)
{
    MenuWidget *m;
    MenuItemDef *d;

    dd_pick.confirm = dd_alloc(sizeof(MenuWidget));
    if ((m = dd_pick.confirm) != 0) {
        d = &m->def;
        memset(m, 0, sizeof(MenuWidget));
        d->visible = 1;
        d->style = 0;
        d->x = x;
        d->y = y;
        d->w = w;
        d->h = h;
        d->color = 0xFFFFFFFF;
        m->heap = dd_heap_get();
        m->no_input = 1;
        m->keep_open = 1;
        m->no_cancel = 1;
        m->count = 1;
        m->draw_all = (MenuDrawRectFn)dd_pick_confirm_draw;
        menu_open(m);
    }
}
#endif

void dd_pick_confirm_free(void)
{
    DdPick *p = &dd_pick;

    if (dd_pick.confirm != 0) {
        menu_reset(p->confirm);
        dd_free(p->confirm);
        p->confirm = 0;
    }
}

#ifdef NON_MATCHING /* alocação de registradores (forma igual) */
unsigned int *dd_pick_confirm_draw(unsigned int *list, int x, int y, int w, int h)
{
    MenuWidget *c = dd_pick.confirm;
    int bw = c->def.w - 9;
    int bh = c->def.h - 9;
    DdPick *p = &dd_pick;
    unsigned short *text = dd_pick.title;

    if (dd_pick.cur_y == 9)
        list = menu_draw_cursor(list, p->confirm, x - 1, y - 2, bw + 2, bh + 2);
    font_set_size(12, 12);
    font_text_set(list);
    if (p->n_picked >= p->cfg.min_pick)
        font_set_color(0xFF000000);
    else
        font_set_color(0xFF808080);
    x += (bw - (font_measure_text(text) >> 6)) / 2;
    y += (bh - (font_line_height() >> 6)) / 2;
    font_draw_text(x << 6, y << 6, (480 - x) << 6, text);
    font_text_clear();
}
#endif

int dd_pick_has(int index)
{
    DdPick *p = &dd_pick;
    int i;

    for (i = 0; i < dd_pick.n_picked; i++) {
        if (p->picked[i] == index)
            return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* o original chama dd_pick_has como função static (a0/a3 vivos no jal); dd_pick_has deixou de ser static porque dd_pick_draw em asm a chama */
void dd_pick_toggle(int index, int add)
{
    DdPick *p = &dd_pick;
    int i;

    if (add) {
        if (!dd_pick_has(index)) {
            p->picked[p->n_picked] = index;
            p->n_picked++;
        }
    } else if (dd_pick_has(index)) {
        for (i = 0; i < p->n_picked; i++) {
            if (index == p->picked[i])
                break;
        }
        for (i++; i < p->n_picked; i++)
            p->picked[i - 1] = p->picked[i];
        p->n_picked--;
    }
}
#endif
