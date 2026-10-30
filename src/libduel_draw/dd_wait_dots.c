// @range 0x08825DDC-0x088260A4
/* Row of waiting dots in a duel window: the item callbacks that draw one dot per pending step
   (lit for the steps done, grey for the rest) and close the window a few frames after the count
   reaches zero. */
#include "dd_d3.h"

typedef struct DdWaitDots {
    int total;                         /* dots drawn (pending count when the window opened) */
    int delay;                         /* frames left before closing once nothing is pending */
} DdWaitDots;

extern DdWaitDots dd_wait_dots;

void func_0882308C(void);
void func_0882331C(void *draw);
void func_08823328(int (*done)(void));
MenuItemDef *func_08823310(void);
int dv_088B2094(void);               /* pending count (libduel_eng duel_088B2094) */
void *func_088044B0(int id);
void *gim_set_texture(void *list, void *gim);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
void req_flag_set(int i);

unsigned int *dd_wait_dots_draw(unsigned int *list, Menu *m, MenuItemDef *d);
int dd_wait_dots_done(void);

void dd_wait_dots_open(void)
{
    MenuItemDef *d;
    short dh;

    func_0882308C();
    func_0882331C(dd_wait_dots_draw);
    func_08823328(dd_wait_dots_done);
    d = func_08823310();
    dh = 40 - d->pad_b;
    d->h += dh;
    d->pad_b = 40;
    dd_wait_dots.total = dv_088B2094();
    dd_wait_dots.delay = 10;
}

unsigned int *dd_wait_dots_draw(unsigned int *list, Menu *m, MenuItemDef *d)
{
    DdWaitDots *w = &dd_wait_dots;
    int x, y, i, n;
    void *gim;

    if (menu_item_state(m, d) != 2) {
        req_flag_set(0x31);
        return list;
    }
    x = d->x + d->pad_l;
    y = d->y + d->h - d->pad_b;
    x += (d->w - d->pad_l - d->pad_r - w->total * 20) / 2;
    y += (d->pad_b - d->pad_t - 17) / 2;
    gim = func_088044B0(0x40005);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    n = w->total - dv_088B2094();
    list = dl_ambient_color(list, 0xFFFFFFFF);
    for (i = 0; i < n; i++) {
        list = draw_set_offsets(list, 17, 17, 17, 17);
        list = draw_sprite(list, x + i * 20, y, 0, 0x33, 0x20, 0, 0);
    }
    list = dl_ambient_color(list, 0xFF808080);
    for (; i < w->total; i++) {
        list = draw_set_offsets(list, 17, 17, 17, 17);
        list = draw_sprite(list, x + i * 20, y, 0, 0x33, 0x20, 0, 0);
    }
    return list;
}

int dd_wait_dots_done(void)
{
    DdWaitDots *w = &dd_wait_dots;

    if (dv_088B2094() <= 0) {
        if (w->delay <= 0)
            return 1;
        w->delay--;
    }
    return 0;
}
