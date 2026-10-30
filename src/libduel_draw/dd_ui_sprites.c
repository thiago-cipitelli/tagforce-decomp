// @range 0x08862A60-0x08862DCC
// @asm 0x08862B9C-0x08862DCC
/* Sprite helpers of the duel windows: a filled rectangle, a (scaled) sprite from the bound
   texture and a nine-patch frame built from corner, edge and center pieces. */
#include "libduel_draw.h"

void *ui_sprite_scaled(void *dl, int x, int y, int u, int v, int tw, int th, int w, int h);

void *ui_rect(void *dl, int x, int y, int w, int h, unsigned int color)
{
    dl = dl_disable(dl, 9);
    dl = dl_color(dl, color);
    dl = draw_set_offsets(dl, 1, 1, w, h);
    return draw_sprite(dl, x, y, 0, 0, 0, 0, 0);
}

void *ui_sprite(void *dl, int x, int y, int u, int v, int w, int h)
{
    return ui_sprite_scaled(dl, x, y, u, v, w, h, w, h);
}

void *ui_sprite_scaled(void *dl, int x, int y, int u, int v, int tw, int th, int w, int h)
{
    dl = draw_set_offsets(dl, tw, th, w, h);
    return draw_sprite(dl, x, y, 0, u, v, 0, 0);
}

/* frame of w x h at (x, y) from the cw x ch texture block at (u, v); sx, sy: border size */
#ifdef NON_MATCHING /* só alocação de registradores difere */
void *ui_nine_patch(void *dl, int x, int y, int u, int v, int cw, int ch, int w, int h, int sx, int sy)
{
    int x2, y2, u2, v2;

    dl = ui_sprite(dl, x, y, u, v, sx, sy);
    v2 = v + ch - sy;
    y2 = y + h - sy;
    dl = ui_sprite(dl, x, y2, u, v2, sx, sy);
    u2 = u + cw - sx;
    x2 = x + w - sx;
    dl = ui_sprite(dl, x2, y, u2, v, sx, sy);
    dl = ui_sprite(dl, x2, y2, u2, v2, sx, sy);
    w -= sx * 2;
    cw -= sx * 2;
    dl = ui_sprite_scaled(dl, x + sx, y, u + sx, v, cw, sy, w, sy);
    dl = ui_sprite_scaled(dl, x + sx, y2, u + sx, v2, cw, sy, w, sy);
    v += sy;
    y += sy;
    ch -= sy * 2;
    h -= sy * 2;
    dl = ui_sprite_scaled(dl, x, y, u, v, sx, ch, sx, h);
    dl = ui_sprite_scaled(dl, x2, y, u2, v, sx, ch, sx, h);
    return ui_sprite_scaled(dl, x + sx, y, u + sx, v, cw, ch, w, h);
}
#else
void *ui_nine_patch(void *dl, int x, int y, int u, int v, int cw, int ch, int w, int h, int sx, int sy);
#endif
