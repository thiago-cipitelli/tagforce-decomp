/* Font layer (BOOT 0x08845AC0..0x0884A9B4): sceFont glyphs cached in a 4bpp atlas, drawn as
   textured quads on a display-list layer; UCS-2 and 8-bit text with layout tags. */
#ifndef FONT_H
#define FONT_H

typedef struct GlyphSlot {             /* FontCtx.slots[256], LRU doubly linked */
    unsigned short code;
    unsigned short prev;
    unsigned short next;
    short advance;                     /* 26.6 */
} GlyphSlot;

typedef struct GlyphIndex {            /* FontCtx.index[count], sorted by code */
    unsigned short code;
    unsigned short slot;
} GlyphIndex;

typedef struct FontCtx {               /* 0x51C54 bytes, in .bss; font_ctx_ptr points to it */
    unsigned char atlas[0x20000];      /* 0x00000 glyph texture, 16 x 16 cells of 32 x 32 4bpp */
    GlyphSlot slots[256];              /* 0x20000 */
    GlyphIndex index[256];             /* 0x20800 */
    int module_id;                     /* 0x20C00 libfont.prx */
    int lib;                           /* 0x20C04 */
    int font;                          /* 0x20C08 */
    int font2;                         /* 0x20C0C alternate font (non US/JP regions) */
    unsigned char scratch[0x30000];    /* 0x20C10 */
    unsigned char *scratch_ptr;        /* 0x50C10 */
    unsigned int color;                /* 0x50C14 */
    int line_h;                        /* 0x50C18 26.6 */
    int baseline;                      /* 0x50C1C 26.6 */
    void *dl;                          /* 0x50C20 current display list */
    unsigned short count;              /* 0x50C24 cached glyphs */
    unsigned short lru_head;           /* 0x50C26 least recently used slot */
    unsigned short lru_tail;           /* 0x50C28 most recently used slot */
    unsigned char size_x;              /* 0x50C2A 20 = 100% */
    unsigned char size_y;              /* 0x50C2B */
    char layer;                        /* 0x50C2C display-list layer, -1 = none */
    unsigned char pad50c2d[3];
    int max_glyph_w;                   /* 0x50C30 */
    int cursor_x;                      /* 0x50C34 26.6, after the last draw */
    int cursor_y;                      /* 0x50C38 */
    int unk50c3c;                      /* 0x50C3C */
    unsigned short flags;              /* 0x50C40 bit 2: shrink width, bit 14/15: ? */
    unsigned short pad50c42;
    void *saved_dl;                    /* 0x50C44 */
    unsigned int color2;               /* 0x50C48 shadow/outline color */
    unsigned char rest[0x51C54 - 0x50C4C];
} FontCtx;

typedef struct GlyphCmds {             /* 12 GE words: vertex format, 2 vertices, draw */
    unsigned int cmd0, cmd1, cmd2;
    unsigned int v0[4];                /* uv (u16 pair), x, y float, z */
    unsigned int v1[4];
    unsigned int cmd_draw;
} GlyphCmds;

typedef struct FontLayout {            /* 0x48, text layout state */
    int x;                             /* +0x00 26.6 */
    int y;                             /* +0x04 */
    int x0;                            /* +0x08 line start */
    int xmax;                          /* +0x0C */
    int lines;                         /* +0x10 */
    int unk14;
    unsigned int color;                /* +0x18 */
    unsigned int color2;               /* +0x1C */
    int unk20[3];
    int xmax2;                         /* +0x2C */
    int unk30[2];
    unsigned char size_x, size_y;      /* +0x38 */
    unsigned char base_size_x, base_size_y; /* +0x3A */
    int unk3c;
    int unk40;                         /* +0x40 */
    int unk44;
} FontLayout;

extern FontCtx *font_ctx_ptr;
extern unsigned int font_color_table[15];

/* sceFont (libfont.prx) */
typedef struct FontNewLibParams {
    void *user_data;
    unsigned int num_fonts;
    void *cache_data;
    void *(*alloc)(void *data, unsigned int size);
    void (*free)(void *data, void *p);
    void *open, *close, *read, *seek, *error, *io_finish;
} FontNewLibParams;

typedef struct SceFontStyle {          /* 0xA8 */
    float h, v, h_res, v_res, weight;
    unsigned short family, style, style_sub, language, region, country;
    char name[64];
    char file_name[64];
    unsigned int attributes, expire;
} SceFontStyle;

typedef struct SceFontInfo {           /* 0x108 */
    int max_glyph_w_i, max_glyph_h_i, max_ascender_i, max_descender_i, max_left_x_i, max_base_y_i,
        min_center_x_i, max_top_y_i, max_advance_x_i, max_advance_y_i;
    float max_glyph_w, max_glyph_h, max_ascender, max_descender, max_left_x, max_base_y,
        min_center_x, max_top_y, max_advance_x, max_advance_y;
    short max_glyph_w_s, max_glyph_h_s;
    SceFontStyle style;
    unsigned char bpp, pad[3];
    unsigned int reserved[2];
} SceFontInfo;

typedef struct SceFontCharInfo {       /* 0x3C */
    unsigned int bitmap_w, bitmap_h;
    int bitmap_left, bitmap_top;
    int w, h, ascender, descender, bearing_hx, bearing_hy, bearing_vx, bearing_vy;
    short advance_h;                   /* +0x30, 26.6 */
    short advance_v;
    unsigned short shadow_flags, shadow_id;
    int pad;
} SceFontCharInfo;

typedef struct SceFontGlyphImage {     /* 0x18 */
    int pixel_format;
    int x64, y64;
    unsigned short buf_w, buf_h, bytes_per_line, pad;
    void *buffer;
} SceFontGlyphImage;

int sceFontNewLib(FontNewLibParams *p, unsigned int *err);
int sceFontFindOptimumFont(int lib, SceFontStyle *style, unsigned int *err);
int sceFontOpen(int lib, int index, int mode, unsigned int *err);
int sceFontGetFontInfo(int font, SceFontInfo *info);
int sceFontSetAltCharacterCode(int lib, unsigned short code);
int sceFontGetCharInfo(int font, unsigned short code, SceFontCharInfo *info);
int sceFontGetCharGlyphImage(int font, unsigned short code, SceFontGlyphImage *img);

/* font.c */
int font_init(void *fs);
void font_mbs_reset(void);
int font_mbs_to_wcs(unsigned short *dst, unsigned int n, const char *src);
int font_set_layer(char layer);
unsigned int font_draw_wtext_simple(int x, int y, int x0, int xmax, unsigned short *text);
int font_glyph_find(unsigned short code);
void font_dl_begin(int layer);
void font_dl_end(void);
void font_glyph_quad(int x, int y, int slot);
void font_set_size(int w, int h);
void font_get_size(unsigned char *w, unsigned char *h);
unsigned int font_set_color(unsigned int c);
unsigned int font_get_color(void);
int font_measure_text(char *text);
int font_line_height(void);
unsigned int font_glyph_cache(SceFontGlyphImage *img, unsigned int code);
int wc_break_class(unsigned short c);
int wc_no_line_start(unsigned short c);
int wc_no_line_end(unsigned short c);
int u16_bsearch(unsigned short c, const unsigned short *table, int n);
/* font_draw.c */
void *font_dl_glyph_header(void *dl, unsigned int color);
unsigned int font_shadow_color(void);

#endif
