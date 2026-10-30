// @range 0x088632F4-0x08863998
// @asm 0x0886346C-0x08863734
/* Panels and scrollbar of the duel windows: a nine-patch panel (pulsing or flashing variants for
   the cursor) and the vertical scrollbar with its track, thumb and blinking arrows. */
#include "dd_duelui.h"
#include "allegrex.h"

void *ui_sprite(void *dl, int x, int y, int u, int v, int w, int h);
void *ui_nine_patch(void *dl, int x, int y, int u, int v, int cw, int ch, int w, int h, int sx, int sy);
float fabsf_vfpu(float x);

void *ui_bar3(void *dl, int x, int y, int u0, int v0, int w0, int h0, int u1, int v1, int w1, int th1, int h1, int u2, int v2, int w2, int h2);
void *ui_track(void *dl, int x, int y, int h);
void *ui_thumb(void *dl, int x, int y, int h, int active);
void *ui_track_caps(void *dl, int x0, int y0, int x1, int y1);
void *ui_arrow_up(void *dl, int x, int y);
void *ui_arrow_down(void *dl, int x, int y);

void *ui_panel(void *dl, void *gim, int x, int y, int w, int h, unsigned int color)
{
    if (color == 0)
        color = 0xFF389DFF;
    dl = dl_ambient_color(dl, color);
    dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
    dl = dl_enable(dl, 4);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = dl_enable(dl, 9);
    dl = gim_set_texture(dl, gim);
    return ui_nine_patch(dl, x, y, 0, 0x70, 11, 11, w, h, 1, 1);
}

/* alpha pulses with the frame counter */
void *ui_panel_pulse(void *dl, void *gim, int x, int y, int w, int h, unsigned int color, unsigned int t)
{
    if (color == 0)
        color = 0xFF389DFF;
    t = (t / 6) & 7;
    if (t > 4)
        t = 8 - t;
    return ui_panel(dl, gim, x, y, w, h, (color << 8 >> 8) | (unsigned char)(0xD0 - t * 16) << 24);
}

/* alpha blinks on/off every 2 frames */
void *ui_panel_flash(void *dl, void *gim, int x, int y, int w, int h, unsigned int color, unsigned int t)
{
    if (color == 0)
        color = 0xFF389DFF;
    return ui_panel(dl, gim, x, y, w, h, (color << 8 >> 8) | (unsigned char)(((t >> 1) & 1) * 255) << 24);
}

/* scrollbar of h pixels: pos (<= 0) is the scroll of a content of size total seen through view */
#ifdef NON_MATCHING /* só alocação de registradores difere (usa s8) */
void *ui_scrollbar(void *dl, void *gim, int x, int y, int h, int pos, int total, int view, int active)
{
    int range = ALLEGREX_MAX(view, total);
    int thumb = (float)(h - 12) * ((float)view / (float)range);
    float frac;
    int top;
    int bottom;
    int up, down;
    unsigned int alpha;

    if (pos == 0 || range == view)
        frac = 0.0f;
    else
        frac = (float)-pos / (float)(range - view);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    top = y + 6;
    dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
    dl = dl_enable(dl, 4);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = dl_enable(dl, 9);
    dl = gim_set_texture(dl, gim);
    dl = ui_track(dl, x, top, h - 12);
    bottom = y + h - 6;
    dl = ui_track_caps(dl, x + 1, y, x + 1, bottom);
    dl = ui_thumb(dl, x + 2, (float)top + frac * ((float)(y + h - thumb - 6) - (float)top), thumb, active);
    if (active) {
        up = 0;
        down = 0;
        alpha = fabsf_vfpu(255.0f * ((float)(int)(*gfx_get_state() % 30 - 15) / 15.0f));
        dl = dl_ambient_color(dl, alpha << 24 | 0xFFFFFF);
        if (range == view) {
            up = 0;
            down = 0;
        } else {
            if (frac > 0.0f)
                up = 1;
            if (frac < 1.0f)
                down = 1;
        }
        if (up)
            dl = ui_arrow_up(dl, x + 1, y);
        if (down)
            dl = ui_arrow_down(dl, x + 1, bottom);
    }
    return dl;
}
#else
void *ui_scrollbar(void *dl, void *gim, int x, int y, int h, int pos, int total, int view, int active);
#endif

/* vertical bar of three pieces: top, stretched middle (h1), bottom */
void *ui_bar3(void *dl, int x, int y, int u0, int v0, int w0, int h0, int u1, int v1, int w1, int th1, int h1, int u2, int v2, int w2, int h2)
{
    dl = draw_set_offsets(dl, w0, h0, w0, h0);
    dl = draw_sprite(dl, x, y, 0, u0, v0, 0, 0);
    dl = draw_set_offsets(dl, w1, th1, w1, h1);
    dl = draw_sprite(dl, x, y + h0, 0, u1, v1, 0, 0);
    dl = draw_set_offsets(dl, w2, h2, w2, h2);
    return draw_sprite(dl, x, h1 + (y + h0), 0, u2, v2, 0, 0);
}

void *ui_track(void *dl, int x, int y, int h)
{
    return ui_bar3(dl, x, y, 14, 0, 8, 4, 14, 4, 8, 1, h - 7, 14, 5, 8, 3);
}

void *ui_thumb(void *dl, int x, int y, int h, int active)
{
    int u = active ? 0 : 4;

    return ui_bar3(dl, x, y, u + 14, 8, 4, 3, u + 14, 11, 4, 1, h - 7, u + 14, 12, 4, 4);
}

void *ui_track_caps(void *dl, int x0, int y0, int x1, int y1)
{
    dl = ui_sprite(dl, x0, y0, 7, 0, 7, 6);
    return ui_sprite(dl, x1, y1, 7, 6, 7, 6);
}

void *ui_arrow_up(void *dl, int x, int y)
{
    return ui_sprite(dl, x, y, 0, 0, 7, 6);
}

void *ui_arrow_down(void *dl, int x, int y)
{
    return ui_sprite(dl, x, y, 0, 6, 7, 6);
}
