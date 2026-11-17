/* sceLibFont (libfont.prx) on FreeType with a system TTF. The game (BOOT/font.c) opens font 0 of the library,
   reads the font metrics once, and renders each glyph through sceFontGetCharInfo + sceFontGetCharGlyphImage into
   a 32x32 4-bpp buffer (pixel format 0, bytes_per_line 16) at (x64, y64) = (8 + bitmap_left, 8 + ascender + 1 -
   bitmap_top) px, which it then copies into a 16x16-slot atlas; the quad for a glyph is the 28x28 px region from
   (1, 1) of the slot, drawn at (pen - 7, line_top - 7) px. So x64/y64 is the top-left of the glyph bitmap, the
   metrics are 26.6, and every glyph has to fit in 28x28 with the baseline at ascender + 1.

   Env: TAGFORCE_TTF (font file; default Liberation Sans, then DejaVu Sans), TAGFORCE_TTF_PX (pixel size,
   default 18: ascender + descender ~ 20 px, the size the game scales from), TAGFORCE_FONTDIR (hook for the PSP's
   own flash0:/font/*.pgf: not implemented, logs and falls back to the TTF). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "font.h"

static FT_Library ft;
static FT_Face face;
static unsigned short alt_code = 0x25A1;

static const char *ttf_candidates[] = {
    "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/TTF/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    0,
};

static int font_ready(void)
{
    const char *p, **c;
    int px;

    if (face)
        return 1;
    if ((p = getenv("TAGFORCE_FONTDIR")))
        fprintf(stderr, "font: TAGFORCE_FONTDIR=%s: PGF fonts not implemented, using a TTF\n", p);
    if (!ft && FT_Init_FreeType(&ft))
        return 0;
    if ((p = getenv("TAGFORCE_TTF")) && FT_New_Face(ft, p, 0, &face) == 0)
        goto ok;
    for (c = ttf_candidates; *c; c++)
        if (FT_New_Face(ft, *c, 0, &face) == 0) {
            p = *c;
            goto ok;
        }
    fprintf(stderr, "font: no TTF found (set TAGFORCE_TTF)\n");
    return 0;
ok:
    px = (p = getenv("TAGFORCE_TTF_PX")) ? atoi(p) : 18;
    FT_Set_Pixel_Sizes(face, 0, px);
    fprintf(stderr, "font: %s at %d px (ascender %ld, descender %ld, height %ld in 26.6)\n", face->family_name, px,
            (long)face->size->metrics.ascender, (long)face->size->metrics.descender, (long)face->size->metrics.height);
    return 1;
}

static int load_glyph(unsigned int code, int render)
{
    unsigned int idx = FT_Get_Char_Index(face, code);

    if (idx == 0) {
        static unsigned int seen[16];
        static int n;
        int i;

        for (i = 0; i < n && seen[i] != code; i++)
            ;
        if (i == n && n < 16) {
            seen[n++] = code;
            fprintf(stderr, "font: no glyph for U+%04X in %s, using U+%04X\n", code, face->family_name, alt_code);
        }
        idx = FT_Get_Char_Index(face, alt_code);
    }
    return FT_Load_Glyph(face, idx, FT_LOAD_DEFAULT | (render ? FT_LOAD_RENDER : 0)) == 0;
}

int sceFontNewLib(FontNewLibParams *p, unsigned int *err)
{
    (void)p;
    *err = font_ready() ? 0 : 0x80460001;
    return 1;
}

int sceFontDoneLib(int lib)
{
    (void)lib;
    return 0;
}

int sceFontFindOptimumFont(int lib, SceFontStyle *style, unsigned int *err)
{
    (void)lib;
    (void)style;
    *err = 0;
    return 0;
}

int sceFontOpen(int lib, int index, int mode, unsigned int *err)
{
    (void)lib;
    (void)index;
    (void)mode;
    *err = face ? 0 : 0x80460002;
    return 2;
}

int sceFontClose(int font)
{
    (void)font;
    return 0;
}

int sceFontSetAltCharacterCode(int lib, unsigned short code)
{
    (void)lib;
    alt_code = code;
    return 0;
}

int sceFontGetFontInfo(int font, SceFontInfo *info)
{
    FT_Size_Metrics *m;
    int asc, desc, adv, w, h;

    (void)font;
    memset(info, 0, sizeof *info);
    if (!face)
        return 0x80460002;
    m = &face->size->metrics;
    asc = (int)m->ascender;                 /* 26.6, positive up */
    desc = (int)-m->descender;              /* 26.6, positive down */
    adv = (int)m->max_advance;
    w = adv;
    h = asc + desc;
    info->max_glyph_w_i = w;
    info->max_glyph_h_i = h;
    info->max_ascender_i = asc;
    info->max_descender_i = desc;
    info->max_left_x_i = 0;
    info->max_base_y_i = asc;
    info->min_center_x_i = 0;
    info->max_top_y_i = asc;
    info->max_advance_x_i = adv;
    info->max_advance_y_i = h;
    info->max_glyph_w = w / 64.0f;
    info->max_glyph_h = h / 64.0f;
    info->max_ascender = asc / 64.0f;
    info->max_descender = desc / 64.0f;
    info->max_base_y = asc / 64.0f;
    info->max_top_y = asc / 64.0f;
    info->max_advance_x = adv / 64.0f;
    info->max_advance_y = h / 64.0f;
    info->max_glyph_w_s = (short)(w >> 6);
    info->max_glyph_h_s = (short)(h >> 6);
    info->style.h = info->style.v = (float)(h >> 6);
    info->style.h_res = info->style.v_res = 128.0f;
    info->style.language = 2;              /* latin */
    strncpy(info->style.name, face->family_name ? face->family_name : "ttf", sizeof info->style.name - 1);
    strncpy(info->style.file_name, "ltn0.pgf", sizeof info->style.file_name - 1);
    info->bpp = 4;
    return 0;
}

int sceFontGetCharInfo(int font, unsigned int code, SceFontCharInfo *ci)
{
    FT_Glyph_Metrics *g;

    (void)font;
    memset(ci, 0, sizeof *ci);
    if (!face || !load_glyph(code, 0))
        return 0x80460003;
    g = &face->glyph->metrics;
    ci->bitmap_w = (g->width + 63) >> 6;
    ci->bitmap_h = (g->height + 63) >> 6;
    ci->bitmap_left = g->horiBearingX >> 6;                  /* floor */
    ci->bitmap_top = (g->horiBearingY + 63) >> 6;            /* ceil */
    ci->w = g->width;
    ci->h = g->height;
    ci->ascender = g->horiBearingY;
    ci->descender = g->height - g->horiBearingY;
    ci->bearing_hx = g->horiBearingX;
    ci->bearing_hy = g->horiBearingY;
    ci->bearing_vx = g->vertBearingX;
    ci->bearing_vy = g->vertBearingY;
    ci->advance_h = g->horiAdvance;
    ci->advance_v = g->vertAdvance;
    return 0;
}

/* Writes the 8-bit coverage of the glyph as 4-bit (format 0, low nibble = even x) or 8-bit (format 2) pixels at
   (x64, y64) >> 6 of the buffer, clipped to buf_w x buf_h. */
int sceFontGetCharGlyphImage(int font, unsigned int code, SceFontGlyphImage *img)
{
    FT_Bitmap *bm;
    unsigned char *buf = img->buffer;
    int x0 = img->x64 >> 6, y0 = img->y64 >> 6;
    int x, y;

    (void)font;
    if (!face || !load_glyph(code, 1))
        return 0x80460003;
    bm = &face->glyph->bitmap;
    for (y = 0; y < (int)bm->rows; y++) {
        int py = y0 + y;
        if (py < 0 || py >= img->buf_h)
            continue;
        for (x = 0; x < (int)bm->width; x++) {
            int px = x0 + x;
            unsigned v = bm->buffer[y * bm->pitch + x];
            unsigned char *d;
            if (px < 0 || px >= img->buf_w)
                continue;
            if (img->pixel_format == 0 || img->pixel_format == 1) {   /* 4 bpp, _REV = high nibble first */
                int hi = (px & 1) ^ (img->pixel_format == 1);
                d = buf + py * img->bytes_per_line + (px >> 1);
                v = (v * 15 + 127) / 255;
                *d = hi ? (*d & 0x0F) | (v << 4) : (*d & 0xF0) | v;
            } else if (img->pixel_format == 2) {                    /* 8 bpp */
                buf[py * img->bytes_per_line + px] = (unsigned char)v;
            } else {                                                /* 24/32 bpp: not used by the game */
                return 0x80460004;
            }
        }
    }
    return 0;
}

int sceFontGetCharGlyphImage_Clip(int font, unsigned int code, SceFontGlyphImage *img, int cx, int cy, int cw, int ch)
{
    (void)cx;
    (void)cy;
    (void)cw;
    (void)ch;
    return sceFontGetCharGlyphImage(font, code, img);
}

/* window.c's on-screen keyboard line: s (ASCII) rasterised white on transparent into a w x h RGBA buffer (row 0 on
   top), left-aligned with a 6 px margin, vertically centred. Uses the face loaded above, at its current size. */
void host_text_bitmap(const char *s, unsigned char *rgba, int w, int h)
{
    int pen = 6, base;

    memset(rgba, 0, (size_t)w * h * 4);
    if (!font_ready())
        return;
    base = (h + (int)(face->size->metrics.ascender >> 6) + (int)(face->size->metrics.descender >> 6)) / 2;
    for (; *s; s++) {
        FT_Bitmap *b;
        int x, y;

        if (!load_glyph((unsigned char)*s, 1))
            continue;
        b = &face->glyph->bitmap;
        for (y = 0; y < (int)b->rows; y++)
            for (x = 0; x < (int)b->width; x++) {
                int px = pen + face->glyph->bitmap_left + x, py = base - face->glyph->bitmap_top + y;
                unsigned char a = b->buffer[y * b->pitch + x];

                if (px >= 0 && px < w && py >= 0 && py < h && a) {
                    unsigned char *d = rgba + (py * w + px) * 4;
                    d[0] = d[1] = d[2] = 255;
                    d[3] = a;
                }
            }
        pen += (int)(face->glyph->advance.x >> 6);
    }
}
