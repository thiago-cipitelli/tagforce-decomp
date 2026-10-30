// @range 0x0886A3F4-0x0886AE68
// @asm 0x0886A3F4-0x0886A794
// @asm 0x0886A7F8-0x0886A8FC
// @asm 0x0886A9D8-0x0886AB2C
// @asm 0x0886ABB8-0x0886AE68
// @data 0x0887CF74-0x0887CFF8
/* Field cursor guide: two d-pad pictures (texture 0x40026) with the caption of each direction
   around them, in a frame with a title; sized from the captions. */
#include "dd_duelui.h"

typedef struct GuideCaption {          /* caption around a d-pad picture */
    int msg;                           /* 0x0881DBC8 text */
    int align;                         /* 0 above, 1 below, 2 left, 3 right */
    int dx, dy;
} GuideCaption;

GuideCaption guide_captions[2][4] = {
    { { 56, 0, 0, -19 }, { 57, 1, 0, 19 }, { 58, 2, -23, 0 }, { 59, 3, 23, 0 } },
    { { 62, 0, 0, -19 }, { 61, 1, 0, 19 }, { 63, 2, -23, 0 }, { 60, 3, 23, 0 } },
};
unsigned short guide_title[2] = { 0xFF0B, 0 };

void *func_0881DBC8(int id);
void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *func_088044B0(int id);
unsigned int font_measure_wtext2(int w, int h, unsigned short *text);
int int_abs(int x);
int cfg_get_language(void);

void guide_size(int *w, int *h, GuideCaption *c);
void *guide_pad(void *dl, int x, int y, int which, GuideCaption *c);
void *guide_frame(void *dl, int x, int y, int w, int h, void *gim);

#ifdef NON_MATCHING /* alocação de registradores difere; não insistido */
void guide_draw(void)
{
    int w[2];
    int h[2];
    int i, tw, th, cx, cy;
    void *dl;
    void *gim;
    unsigned short title[2];
    int x, y, hw;

    font_set_size(12, 12);
    for (i = 0; i < 2; i++)
        guide_size(&w[i], &h[i], guide_captions[i]);
    if (cfg_get_language() == 0) {
        if (w[0] < w[1])
            w[0] = w[1];
        w[1] = w[0];
        if (h[0] < h[1])
            h[0] = h[1];
    } else {
        w[0] += 8;
        h[0] += 4;
    }
    tw = w[0] + w[1];
    th = h[0];
    if (th < h[1])
        th = h[1];
    if (cfg_get_language() == 0) {
        cx = 314;
        cy = 132;
    } else {
        cx = 240;
        cy = 136;
    }
    dl = dl_select_layer(8, 0);
    dl = dl_disable(dl, 1);
    gim = func_088044B0(0x40026);
    hw = tw / 2;
    if (gim != 0) {
        *(int *)title = *(int *)guide_title;
        x = cx - tw / 2;
        y = cy - th / 2;
        dl = guide_frame(dl, x - 5, y - 4, tw + 9, th + 9, gim);
        dl = gim_set_texture(dl, gim);
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
        dl = dl_set_tex_filter(dl, 0, 0);
        dl = draw_set_offsets(dl, 4, 16, 4, 16);
        dl = draw_sprite(dl, x - 5, y - 18, 0, 0, 38, 0, 0);
        dl = draw_set_offsets(dl, 1, 16, 36, 16);
        dl = draw_sprite(dl, x - 1, y - 18, 0, 4, 38, 0, 0);
        dl = draw_set_offsets(dl, 9, 16, 9, 16);
        dl = draw_sprite(dl, x + 35, y - 18, 0, 5, 38, 0, 0);
        dl = draw_set_offsets(dl, 23, 11, 23, 11);
        dl = draw_sprite(dl, x, y - 15, 0, 94, 22, 0, 0);
        font_text_set(dl);
        font_set_size(12, 12);
        font_draw_text((x + 24) << 6, (y - 15) << 6, (480 - (x + 24)) << 6, title);
        dl = font_text_clear();
    }
    for (i = 0; i < 2; i++)
        dl = guide_pad(dl, cx - hw + w[i] / 2 + (i == 0 ? 0 : w[0]), cy, i, guide_captions[i]);
    dl = dl_enable(dl, 1);
    dl_set_list(dl);
}
#else
void guide_draw(void);
#endif

void guide_text_size(int *w, int *h, GuideCaption *c)
{
    int s = font_measure_wtext2(0x7800, 0x4400, func_0881DBC8(c->msg));

    *w = (((s >> 16) & 0xFFFF) + 63) >> 6;
    *h = ((s & 0xFFFF) + 63) >> 6;
}

/* width: left and right captions with their offsets; height: top and bottom ones */
#ifdef NON_MATCHING /* alocação de registradores difere; não insistido */
void guide_size(int *w, int *h, GuideCaption *c)
{
    int tw, th;

    *w = 0;
    guide_text_size(&tw, &th, &c[2]);
    *w += tw + int_abs(c[2].dx);
    guide_text_size(&tw, &th, &c[3]);
    *w += tw + int_abs(c[3].dx);
    *h = 0;
    guide_text_size(&tw, &th, &c[0]);
    *h += th + int_abs(c[0].dy);
    guide_text_size(&tw, &th, &c[1]);
    *h += th + int_abs(c[1].dy);
}
#else
void guide_size(int *w, int *h, GuideCaption *c);
#endif

void *guide_pad_picture(void *dl, int x, int y, int which)
{
    void *gim = func_088044B0(0x40026);

    if (gim != 0) {
        dl = gim_set_texture(dl, gim);
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
        dl = dl_set_tex_filter(dl, 0, 0);
        dl = draw_set_offsets(dl, 47, 38, 47, 38);
        dl = draw_sprite(dl, x - 23, y - 19, 0, (1 - which) * 47, 0, 0, 0);
    }
    return dl;
}

#ifdef NON_MATCHING /* alocação de registradores difere; não insistido */
void *guide_caption(void *dl, int x, int y, GuideCaption *c)
{
    unsigned short *text = func_0881DBC8(c->msg);
    int w, h;

    x += c->dx;
    y += c->dy;
    guide_text_size(&w, &h, c);
    switch (c->align) {
    case 0:
        x -= w / 2;
        y -= h;
        break;
    case 1:
        x -= w / 2;
        break;
    case 2:
        x -= w;
        y -= h / 2;
        break;
    case 3:
        y -= h / 2;
        break;
    }
    font_text_set(dl);
    font_draw_wtext_ex(x << 6, y << 6, w << 6, h << 6, text, -1);
    return font_text_clear();
}
#else
void *guide_caption(void *dl, int x, int y, GuideCaption *c);
#endif

void *guide_pad(void *dl, int x, int y, int which, GuideCaption *c)
{
    dl = guide_pad_picture(dl, x, y, which);
    dl = guide_caption(dl, x, y, &c[0]);
    dl = guide_caption(dl, x, y, &c[1]);
    dl = guide_caption(dl, x, y, &c[2]);
    return guide_caption(dl, x, y, &c[3]);
}

/* frame of w x h from the 9 pieces of the texture */
#ifdef NON_MATCHING /* alocação de registradores difere; não insistido */
void *guide_frame(void *dl, int x, int y, int w, int h, void *gim)
{
    int x2, y2;

    if (gim == 0)
        return dl;
    dl = gim_set_texture(dl, gim);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = draw_set_offsets(dl, 4, 4, 4, 4);
    dl = draw_sprite(dl, x, y, 0, 0, 54, 0, 0);
    dl = draw_set_offsets(dl, 1, 4, w - 9, 4);
    dl = draw_sprite(dl, x + 4, y, 0, 4, 54, 0, 0);
    dl = draw_set_offsets(dl, 5, 4, 5, 4);
    x2 = x + w - 5;
    dl = draw_sprite(dl, x2, y, 0, 5, 54, 0, 0);
    dl = draw_set_offsets(dl, 4, 5, 4, 5);
    y2 = y + h - 5;
    dl = draw_sprite(dl, x, y2, 0, 0, 59, 0, 0);
    dl = draw_set_offsets(dl, 1, 5, w - 9, 5);
    dl = draw_sprite(dl, x + 4, y2, 0, 4, 59, 0, 0);
    dl = draw_set_offsets(dl, 5, 5, 5, 5);
    dl = draw_sprite(dl, x2, y2, 0, 5, 59, 0, 0);
    dl = draw_set_offsets(dl, 4, 1, 4, h - 9);
    dl = draw_sprite(dl, x, y + 4, 0, 0, 58, 0, 0);
    dl = draw_set_offsets(dl, 5, 1, 5, h - 9);
    dl = draw_sprite(dl, x2, y + 4, 0, 5, 58, 0, 0);
    dl = draw_set_offsets(dl, 1, 1, w - 9, h - 9);
    return draw_sprite(dl, x + 4, y + 4, 0, 4, 58, 0, 0);
}
#else
void *guide_frame(void *dl, int x, int y, int w, int h, void *gim);
#endif
