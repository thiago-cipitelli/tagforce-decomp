// @range 0x0883264C-0x08832B54
// @asm 0x08832808-0x08832B54
/* Rolling digits (LP counter style): a 19x22 digit sprite partly scrolled in or out, and a number
   that rolls from one value to another digit by digit, preceded by a sign sprite. */
#include "libduel_draw.h"

unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);

void dd_roll_digit(float t, int layer, int x, int y, int digit, int down)
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

#ifdef NON_MATCHING /* posição das matrizes locais na pilha e alocação do laço (forma igual) */
void dd_roll_number(float t, int x, int y, int from, int to)
{
    int val[2] = { from, to };
    int px[2] = { x + 19, x + 19 };
    float frac[2];
    int down[2];
    int i;

    dd_roll_digit(1.0f, 4, x, y, 10, 1);
    for (i = 0; i < 2; i++) {
        while (val[i] >= 10) {
            px[i] += 19;
            val[i] /= 10;
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
            dd_roll_digit(frac[1], 4, px[1], y, d[1], down[1]);
            px[1] -= 19;
            val[1] /= 10;
        } else if (px[1] < px[0]) {
            dd_roll_digit(frac[0], 4, px[0], y, d[0], down[0]);
            px[0] -= 19;
            val[0] /= 10;
        } else {
            if (d[0] == d[1]) {
                dd_roll_digit(1.0f, 4, px[0], y, d[0], down[0]);
            } else {
                dd_roll_digit(frac[0], 4, px[0], y, d[0], down[0]);
                dd_roll_digit(frac[1], 4, px[1], y, d[1], down[1]);
            }
            px[1] -= 19;
            val[0] /= 10;
            px[0] -= 19;
            val[1] /= 10;
        }
    } while (val[0] != 0 || val[1] != 0);
}
#endif
