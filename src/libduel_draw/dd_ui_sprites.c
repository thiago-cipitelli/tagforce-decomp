// @range 0x08862A60-0x08862DCC
/* Sprite helpers of the duel windows: a filled rectangle, a (scaled) sprite from the bound
   texture and a nine-patch frame built from corner, edge and center pieces. */
#include "dd_duelui.h"

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
void *ui_nine_patch(void *dl, int x, int y, int u, int v, int cw, int ch, int w, int h, int bw, int bh)
{
    int v2, u2, y2, x2;
    int w2, cw2, u3, x3;
    int v3, y3, ch2, h2;

    dl = ui_sprite(dl, x, y, u, v, bw, bh);
    v2 = v + ch - bh;
    y2 = y + h - bh;
    dl = ui_sprite(dl, x, y2, u, v2, bw, bh);
    u2 = u + cw - bw;
    x2 = x + w - bw;
    dl = ui_sprite(dl, x2, y, u2, v, bw, bh);
    dl = ui_sprite(dl, x2, y2, u2, v2, bw, bh);
    w2 = w - bw * 2;
    cw2 = cw - bw * 2;
    u3 = u + bw;
    x3 = x + bw;
    dl = ui_sprite_scaled(dl, x3, y, u3, v, cw2, bh, w2, bh);
    dl = ui_sprite_scaled(dl, x3, y2, u3, v2, cw2, bh, w2, bh);
    v3 = v + bh;
    y3 = y + bh;
    ch2 = ch - bh * 2;
    h2 = h - bh * 2;
    dl = ui_sprite_scaled(dl, x, y3, u, v3, bw, ch2, bw, h2);
    dl = ui_sprite_scaled(dl, x2, y3, u2, v3, bw, ch2, bw, h2);
    return ui_sprite_scaled(dl, x3, y3, u3, v3, cw2, ch2, w2, h2);
}
