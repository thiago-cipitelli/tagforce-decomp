// @range 0x0883264C-0x08832B54
/* Rolling digits (LP counter style): a 19x22 digit sprite partly scrolled in or out, and a number
   that rolls from one value to another digit by digit, preceded by a sign sprite. */
#include "dd_d3.h"

unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);

void dd_roll_digit(int layer, int x, int y, int digit, int down, float t)
{
    unsigned int *list;

    if (t == 0.0f)
        return;
    list = dl_select_layer(layer, 0);
    if (t == 1.0f) {
        list = draw_set_offsets(list, 19, 22, 19, 22);
        list = draw_sprite(list, x, y, 0, digit * 19, 0, 0, 0);
    } else if (down == 1) {
        list = draw_set_offsets(list, 19, 22, 19, 22.0f * t);
        list = draw_sprite(list, x, y, 0, digit * 19, 0, 0, 0);
    } else {
        list = draw_set_offsets(list, 19, 22, 19, 22.0f * t);
        list = draw_sprite(list, x, y + 22.0f * (1.0f - t), 0, digit * 19, 0, 0, 0);
    }
    dl_set_list(list);
}

/* rolls a number from `from` to `to` (t: 0..1), digit by digit from the right, after the sign sprite */
void dd_roll_number(int x, int y, int from, int to, float t)
{
    int val[2] = { from, to };
    int px[2] = { x + 19, x + 19 };
    int down[2];
    int i;
    float frac[2];

    dd_roll_digit(4, x, y, 10, 1, 1.0f);
    for (i = 0; i < 2; i++) {
        while (val[i] >= 10) {
            val[i] /= 10;
            px[i] += 19;
        }
        if (i == 0) {
            val[i] = from;
            if (to < from)
                down[i] = 1;
            else
                down[i] = 0;
            frac[i] = 1.0f - t;
        } else {
            val[i] = to;
            if (to < from)
                down[i] = 0;
            else
                down[i] = 1;
            frac[i] = t;
        }
    }
    do {
        int d[2] = { val[0] % 10, val[1] % 10 };

        if (px[0] < px[1]) {
            dd_roll_digit(4, px[1], y, d[1], down[1], frac[1]);
            val[1] /= 10;
            px[1] -= 19;
        } else if (px[1] < px[0]) {
            dd_roll_digit(4, px[0], y, d[0], down[0], frac[0]);
            val[0] /= 10;
            px[0] -= 19;
        } else {
            if (d[0] == d[1]) {
                dd_roll_digit(4, px[0], y, d[0], down[0], 1.0f);
            } else {
                dd_roll_digit(4, px[0], y, d[0], down[0], frac[0]);
                dd_roll_digit(4, px[1], y, d[1], down[1], frac[1]);
            }
            val[0] /= 10;
            px[0] -= 19;
            val[1] /= 10;
            px[1] -= 19;
        }
    } while (val[0] != 0 || val[1] != 0);
}
