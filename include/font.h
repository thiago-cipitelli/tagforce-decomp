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

typedef struct FontCmd {               /* FontCtx.cmds[], one per glyph or state change */
    short kind;                        /* 1 glyph (x, y, slot), 2 size (sx, sy), 3 color (c, c2), 4/5 ruby begin/end */
    short pad;
    int a, b, c;
} FontCmd;

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
    FontCmd cmds[256];                 /* 0x50C4C deferred commands of the current line */
    int ncmds;                         /* 0x51C4C */
    void *rubi_gim;                    /* 0x51C50 rubi_font00.gim */
} FontCtx;

typedef struct GlyphCmds {             /* 12 GE words: vertex format, 2 vertices, draw */
    unsigned int cmd0, cmd1, cmd2;
    unsigned int v0[4];                /* uv (u16 pair), x, y float, z */
    unsigned int v1[4];
    unsigned int cmd_draw;
} GlyphCmds;

typedef struct FontLayout {            /* 0x48, text layout state passed to font_layout_*text */
    int x;                             /* +0x00 26.6 pen position */
    int y;                             /* +0x04 */
    int yoff;                          /* +0x08 baseline shift of small capitals */
    int xmax;                          /* +0x0C */
    int consumed;                      /* +0x10 characters consumed by the last call */
    unsigned short flags;              /* +0x14 2: inside $R ruby, 0x10: small caps, 0x20: literal '$' */
    unsigned short pad16;
    unsigned int color;                /* +0x18 */
    unsigned int color2;               /* +0x1C shadow color */
    unsigned short ruby_pad;           /* +0x20 extra advance per base char under a ruby */
    unsigned short ruby_w;             /* +0x22 */
    int ruby_x;                        /* +0x24 */
    int ruby_y;                        /* +0x28 */
    int ruby_xmax;                     /* +0x2C */
    int line_x0;                       /* +0x30 */
    int line_h;                        /* +0x34 */
    unsigned char size_x, size_y;      /* +0x38 */
    unsigned char cur_size_x, cur_size_y; /* +0x3A */
    int nglyphs;                       /* +0x3C */
    int max_glyphs;                    /* +0x40 -1 = unlimited */
    char force_upper;                  /* +0x44 $F / $f */
    unsigned char pad45[3];
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
    int advance_h;                     /* +0x30, 26.6 */
    int advance_v;
    unsigned short shadow_flags, shadow_id;
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
int sceFontGetCharInfo(int font, unsigned int code, SceFontCharInfo *info);
int sceFontGetCharGlyphImage(int font, unsigned int code, SceFontGlyphImage *img);

/* font.c */
int font_init(void *fs);
void font_mbs_reset(void);
int font_mbs_to_wcs(unsigned short *dst, unsigned int n, const char *src);
int font_set_layer(char layer);
unsigned int font_draw_wtext_simple(int x, int y, int x0, int xmax, unsigned short *text);
int font_glyph_find(unsigned int code);
void font_dl_begin(int layer);
void font_dl_end(void);
void font_glyph_quad(int x, int y, int slot);
void font_set_size(int w, int h);
void font_get_size(unsigned char *w, unsigned char *h);
unsigned int font_set_color(unsigned int c);
unsigned int font_get_color(void);
int font_measure_text(unsigned short *text);
int font_line_height(void);
unsigned int font_glyph_cache(SceFontGlyphImage *img, unsigned int code);
int wc_break_class(unsigned short c);
int wc_no_line_start(unsigned short c);
int wc_no_line_end(unsigned short c);
int u16_bsearch(unsigned short c, const unsigned short *table, int n);
/* font_flags.c */
void font_set_flags(unsigned short flags);
unsigned short font_get_flags(void);
void font_set_flag1(int on);
int font_get_flag1(void);
void font_set_flag2(int on);
int font_get_flag2(void);
void font_set_flag4(int on);
int font_get_flag4(void);
void font_set_flag8(int on);
int font_get_flag8(void);
void font_set_flag4000(int on);
int font_get_flag4000(void);
void font_set_flag8000(int on);
int font_get_flag8000(void);
void font_set_line_extra(int v);
int font_get_line_extra(void);
void font_text_set(void *list);
void *font_text_clear(void);
void font_text_flush(void);
/* font_text.c */
int font_draw_text(int x, int y, int w, unsigned short *text);
int font_draw_wtext_ex(int x, int y, int w, int h, unsigned short *text, int max_glyphs);
int font_layout_wtext(FontCtx *f, FontLayout *st, unsigned short *text, int wrap);
int font_draw_wtext(int x, int y, int w, unsigned short *text);
int font_measure_wtext(int w, unsigned short *text);
unsigned int font_measure_wtext2(int w, int h, unsigned short *text);
int font_wtext_lines(int w, unsigned short *text);
int font_layout_text(FontCtx *f, FontLayout *st, unsigned short *text, int wrap);
/* font_tags.c */
unsigned short wtext_next_char(FontCtx *f, unsigned short *p, int in_ruby);
int font_word_width(FontCtx *f, FontLayout *st, SceFontGlyphImage *img, unsigned short *p, int wrap);
int font_run_width(FontCtx *f, FontLayout *st, SceFontGlyphImage *img, unsigned short *p);
unsigned short *font_parse_tag(FontCtx *f, FontLayout *st, FontCmd *cmd, unsigned short *p);
unsigned short *font_ruby_measure(FontCtx *f, unsigned short *p, unsigned short *pad, unsigned short *w, unsigned short *total, unsigned short *last);
void *font_dl_glyph_header(void *dl, unsigned int color);
unsigned int font_shadow_color(unsigned int c);
unsigned short *font_ruby_layout(FontCtx *f, FontLayout *st, unsigned short *p);
unsigned short *font_ruby_skip(FontCtx *f, FontLayout *st, unsigned short *p);
void font_layout_end(FontCtx *f, FontLayout *st);
void *font_flush_quads(void **dl, void *verts, int n);
int font_get_cursor_x(void);
int font_get_cursor_y(void);
void font_rubi_quad(int x, int y, int code);
void wtext_strip_tags(unsigned short *dst, unsigned short *src);
void wtext_strip_tags2(unsigned short *dst, unsigned short *src);
int wtext_count_tags(unsigned short *src);
unsigned int font_sample_glyph_size(void);
void font_scratch_reset(void);
void *font_alloc_cb(void *data, unsigned int size);
void font_free_cb(void *data, void *p);

#endif
