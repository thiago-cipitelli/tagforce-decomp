// @range 0x08814D94-0x08815FD8
// @asm 0x08814D94-0x088158BC
// @asm 0x08815984-0x08815AA8
// @asm 0x08815C24-0x08815FD8
// @rodata 0x08838AC4-0x08838AE0
/* Sprite digits and strings of digits of the HUD, the icons of the day parts and the hands of the
   clock (rotated quads, still in asm). */
#include "libfield.h"

void *fld_clock_hand(float angle, void *list, int x, int y, int z, int u, int v, int w, int h);

void fld_hud_draw_free(void)
{
}

void *fld_digit(float scale, void *list, int x, int y, int z, int u, int v, int w, int h, int frame)
{
    list = draw_set_offsets(list, w, h, (int)(w * scale), (int)(h * scale));
    return draw_sprite(list, x, y, z, u + w * frame, v, 0, 0);
}

#ifdef NON_MATCHING /* só a ordem de um move de argumento (a3) e do sw do dígito na pilha difere */
void *fld_digits(void *list, char *s, int x, int y, int z, int u, int v, int w, int h, int gap,
                 float scale)
{
    int n = strlen(s) - 1;
    int i = n;
    float step;

    if (i >= 0) {
        step = gap + w * scale;
        do {
            list = fld_digit(scale, list, (int)(x - step * (n - i)), y, z, u, v, w, h, s[i] - '0');
        } while (--i >= 0);
    }
    return list;
}
#endif

void fld_day_icon(void *list, int n, int x, int y, int z)
{
    int u;
    int v;

    switch (n) {
    case 0:
    case 1:
    case 2:
    case 3:
        u = 0;
        v = 0;
        break;
    case 4:
    case 5:
    case 6:
        u = 0;
        v = 16;
        n -= 4;
        break;
    }
    fld_digit(1.0f, list, x, y, z, u, v, 64, 16, n);
}

void fld_clock_hands(void *list, int x, int y, int z, int h, int m)
{
    float a = 360.0f * (m / 60.0f);

    list = fld_clock_hand((3.1415927f * a) / 180.0f, list, x, y, z, 0xF8, 0x20, 3, 0x27);
    fld_clock_hand((3.1415927f * (360.0f * (h / 12.0f) + a / 12.0f)) / 180.0f, list, x, y, z, 0xFC,
                   0x20, 3, 0x27);
}
