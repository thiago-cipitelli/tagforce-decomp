// @range 0x08845AC0-0x0884A9B4
// @data 0x088EE8C4-0x088EEA58
// @asm 0x088469E8-0x08846EA0
/* One translation unit: font.c, font_text.c, font_flags.c and font_tags.c were merged here (round 34) because
   font_layout_end only matches with font_glyph_vertices as a static function defined earlier in the same file. */
/* Font layer: libfont.prx setup, the 256-slot glyph atlas with LRU replacement, glyph quads,
   size/color state and the UCS-2 line-breaking character classes. Text layout is in font_draw.c. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "stdarg.h"
#include "font.h"

int ehfile_load_module(void *fs, const char *name);
int region_get(void);
void mbs_set_tables(void *a, void *b);
int mbs_to_wcs(unsigned short *dst, unsigned int n, const char *src);
int vsprintf(char *buf, const char *fmt, va_list ap);
void *dl_select_layer(int layer, unsigned int *size_out);
void dl_set_list(void *list);
void *dl_push(void *list, void *words, int n);
void *dl_push1(void *list, unsigned int word);
void *font_alloc_cb(void *data, unsigned int size);
void font_free_cb(void *data, void *p);
void font_scratch_reset(void);
int font_layout_text(FontCtx *f, FontLayout *st, unsigned short *text, int wrap);
void mem_move(void *dst, void *src, unsigned int n);
extern const unsigned char font_half_width[0x2300];   /* .rodata 0x08875760: use font2 for these codes */
extern FontCtx font_ctx_storage;

unsigned int font_color_table[15] = {
    0x000000, 0x002945, 0x7F9900, 0x6400C4, 0x00EAFF, 0x6600FF, 0xFF8400, 0x0084FF,
    0xA40055, 0x004D0B, 0x0000BB, 0xB00C00, 0x00D957, 0x808080, 0xFFFFFF,
};
FontCtx *font_ctx_ptr = &font_ctx_storage;

/* o modelo de FontNewLibParams (0x088EE904) e "libfont.prx" saem do @data deste arquivo */
int font_init(void *fs)
{
    int status;
    unsigned int err;
    SceFontStyle style;
    SceFontInfo info;
    FontNewLibParams params;
    FontCtx *f;
    int best;

    params = (FontNewLibParams){ 0, 2, 0, font_alloc_cb, font_free_cb, 0, 0, 0, 0, 0, 0 };
    f = font_ctx_ptr;

    while ((f->module_id = ehfile_load_module(fs, "libfont.prx")) <= 0)
        ;
    sceKernelStartModule(f->module_id, 0, 0, &status, 0);
    memset(f, 0, sizeof(FontCtx));
    font_scratch_reset();
    f->lib = sceFontNewLib(&params, &err);
    memset(&style, 0, sizeof style);
    style.language = 2;
    style.v = 20.0f;
    style.h = 20.0f;
    style.family = 0;
    style.style = 0;
    best = sceFontFindOptimumFont(f->lib, &style, &err);
    f->font = sceFontOpen(f->lib, 0, 0, &err);
    switch (region_get()) {
    case 0:
    case 1:
        f->font2 = f->font;
        break;
    default:
        f->font2 = sceFontOpen(f->lib, best, 0, &err);
        break;
    }
    err = sceFontGetFontInfo(f->font, &info);
    f->max_glyph_w = info.max_glyph_w_i;
    f->line_h = info.max_glyph_h_i;
    f->baseline = info.max_ascender_i + 0x40;
    font_mbs_reset();
    err = sceFontSetAltCharacterCode(f->lib, 0x25A1);
    f->layer = -1;
    f->count = 0;
    f->lru_tail = 0xFFFF;
    f->lru_head = 0xFFFF;
    f->color = 0xFFFFFFFF;
    f->size_x = 20;
    f->size_y = 20;
    return 1;
}

void font_mbs_reset(void)
{
    mbs_set_tables(0, 0);
}

int font_mbs_to_wcs(unsigned short *dst, unsigned int n, const char *src)
{
    return mbs_to_wcs(dst, n, src);
}

int font_set_layer(char layer)
{
    char old = font_ctx_ptr->layer;

    font_ctx_ptr->layer = layer;
    return old;
}

/* Draws tag-free UCS-2 text at (x, y) in 26.6, wrapping at xmax back to x0; returns x << 16 | y. */
unsigned int font_draw_wtext_simple(int x, int y, int x0, int xmax, unsigned short *text)
{
    FontCtx *f = font_ctx_ptr;
    SceFontGlyphImage img;
    unsigned short c;
    int began = 0;
    unsigned int slot;
    int adv;

    img.pixel_format = 0;
    img.buf_w = 32;
    img.buf_h = 32;
    img.bytes_per_line = 16;
    img.pad = 0;
    while ((c = *text++) != 0) {
        if (c >= 0x20) {
            slot = font_glyph_cache(&img, c);
            if (!began) {
                began = 1;
                font_dl_begin(f->layer);
            }
            adv = f->slots[slot].advance * f->size_x / 20;
            if (x + adv > xmax) {
                y += f->line_h * f->size_y / 20;
                x = x0;
            }
            font_glyph_quad(x, y, slot);
            x += adv;
        } else {
            switch (c) {
            case '\n':
                y += f->line_h * f->size_y / 20;
                x = x0;
                break;
            }
        }
    }
    if (began)
        font_dl_end();
    return (x & 0xFFFF) << 16 | (y & 0xFFFF);
}

void font_printf(int x, int y, const char *fmt, ...)
{
    char buf[1024];
    unsigned short wbuf[1024];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    memset(wbuf, 0, sizeof wbuf);
    font_mbs_to_wcs(wbuf, 0x800, buf);
    font_draw_wtext_simple(x << 6, y << 6, 0, 480 << 6, wbuf);
}

void font_printf_box(int x, int y, int x0, int xmax, const char *fmt, ...)
{
    char buf[1024];
    unsigned short wbuf[1024];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    memset(wbuf, 0, sizeof wbuf);
    font_mbs_to_wcs(wbuf, 0x800, buf);
    font_draw_wtext_simple(x << 6, y << 6, x0 << 6, xmax << 6, wbuf);
}

void font_printf64(int x, int y, const char *fmt, ...)
{
    char buf[1024];
    unsigned short wbuf[1024];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    memset(wbuf, 0, sizeof wbuf);
    font_mbs_to_wcs(wbuf, 0x800, buf);
    font_draw_wtext_simple(x, y, 0, 480, wbuf);
}

void font_printf_box64(int x, int y, int x0, int xmax, const char *fmt, ...)
{
    char buf[1024];
    unsigned short wbuf[1024];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    memset(wbuf, 0, sizeof wbuf);
    font_mbs_to_wcs(wbuf, 0x800, buf);
    font_draw_wtext_simple(x, y, x0, xmax, wbuf);
}

/* Binary search in the sorted index: slot of `code`, or 0x100 + insertion position. */
int font_glyph_find(unsigned short code)
{
    FontCtx *f = font_ctx_ptr;
    int lo = 0;
    int hi = f->count + 1;
    int mid = hi / 2;
    int c = code;

    while (lo < mid) {
        int i = mid - 1;
        if (f->index[i].code < c)
            lo = mid;
        else if (c < f->index[i].code)
            hi = mid;
        else
            break;
        mid = (lo + hi) / 2;
    }
    if (lo >= mid)
        mid += 0x100;
    else
        mid--;
    return mid;
}

void font_dl_begin(int layer)
{
    FontCtx *f = font_ctx_ptr;

    f->dl = dl_select_layer(layer, 0);
    f->dl = font_dl_glyph_header(f->dl, f->color);
}

void font_dl_end(void)
{
    dl_set_list(dl_push1(font_ctx_ptr->dl, 0x13000000));
}

/* GE commands of one glyph quad: 2 vertices (u16 uv, float xyz) drawn with the atlas texture */
/* lido por um ponteiro const: assim a cópia sai em lwc1/swc1 como no original (definido como const iria para a .rodata) */
GlyphCmds glyph_cmds_template = { 0x14000000, 0x0100000C, 0x0800002C, { 0x00010001, 0, 0, 0 }, { 0x00150015, 0, 0, 0 }, 0x04060002 };

/* Emits a 2-vertex textured quad for atlas cell `slot` at (x, y) in 26.6. */
void font_glyph_quad(int x, int y, int slot)
{
    FontCtx *f = font_ctx_ptr;
    int u = (slot % 16) * 32 + 1;
    int v = (slot / 16) * 32 + 1;
    GlyphCmds cmds;
    unsigned int *q;
    float fx, fy;

    cmds = *(const GlyphCmds *)&glyph_cmds_template;
    q = (unsigned int *)((char *)f->dl + 12);
    f->dl = dl_push(f->dl, &cmds, 12);
    fx = (float)x * 0.015625f - (float)(f->size_x * 7) / 20.0f;
    fy = (float)y * 0.015625f - (float)(f->size_y * 7) / 20.0f;
    q[0] = (v & 0xFFFF) << 16 | (u & 0xFFFF);
    ((float *)q)[1] = fx;
    ((float *)q)[2] = fy;
    fx += (float)(unsigned int)f->size_x * 28.0f / 20.0f;
    fy += (float)(unsigned int)f->size_y * 28.0f / 20.0f;
    q[4] = ((v + 28) & 0xFFFF) << 16 | ((u + 28) & 0xFFFF);
    ((float *)q)[5] = fx;
    ((float *)q)[6] = fy;
}

/* Same quad written as two 16-byte vertices (u16 uv, float xyz) into v. `static`: font_layout_end (same translation
   unit) keeps v0 across its calls, which mwcc only does for a static callee whose register use it has seen. */
static void font_glyph_vertices(short *v, int x, int y, int slot)
{
    FontCtx *f = font_ctx_ptr;
    unsigned short u = (slot % 16) * 32 + 1;
    unsigned short vv = (slot / 16) * 32 + 1;
    float fx, fy;

    v[0] = u;
    v[1] = vv;
    v[8] = u + 28;
    v[9] = vv + 28;
    fx = (float)x * 0.015625f - (float)(f->size_x * 7) / 20.0f;
    fy = (float)y * 0.015625f - (float)(f->size_y * 7) / 20.0f;
    *(float *)(v + 2) = fx;
    *(float *)(v + 4) = fy;
    *(int *)(v + 6) = 0;
    *(float *)(v + 10) = fx + (float)(unsigned int)f->size_x * 28.0f / 20.0f;
    *(float *)(v + 12) = fy + (float)(unsigned int)f->size_y * 28.0f / 20.0f;
    *(int *)(v + 14) = 0;
}

void font_set_size(int w, int h)
{
    FontCtx *f = font_ctx_ptr;

    f->size_x = w;
    f->size_y = h;
}

void font_get_size(unsigned char *w, unsigned char *h)
{
    FontCtx *f = font_ctx_ptr;

    *w = f->size_x;
    *h = f->size_y;
}

unsigned int font_set_color(unsigned int c)
{
    FontCtx *f = font_ctx_ptr;

    f->color = c;
    f->color2 = font_shadow_color(c);
    return c;
}

unsigned int font_get_color(void)
{
    return font_ctx_ptr->color;
}

/* Width in 26.6 of a UCS-2 string laid out without limits. */
int font_measure_text(unsigned short *text)
{
    FontCtx *f = font_ctx_ptr;
    FontLayout st;

    memset(&st, 0, sizeof st);
    st.xmax = 0x7FFFFFFF;
    st.cur_size_x = f->size_x;
    st.size_x = f->size_x;
    st.cur_size_y = f->size_y;
    st.size_y = f->size_y;
    return font_layout_text(f, &st, text, 0);
}

int font_line_height(void)
{
    FontCtx *f = font_ctx_ptr;
    int h;

    if (f == 0)
        return 0;
    h = f->line_h * f->size_y / 20;
    if (f->flags & 0x8000)
        h += 0x1C0;
    return h;
}

#ifdef NON_MATCHING
/* Returns the atlas slot of `code`, rendering it with sceFont into the least recently used slot
   when absent; keeps the sorted index and the LRU list up to date. */
/* Fica em asm (55 palavras): forma igual, só f e font com s1/s5 trocados (e por isso os $s do laço de
   cópia deslocados). `code` é u16 que o original nunca mascara na entrada: os chamadores passam o
   registrador cru e font_glyph_find/sceFontGetChar* recebem u16 (com `unsigned int` neles o mwcc
   mascarava code uma vez na entrada: 302 palavras). Testados sem efeito na troca: ordem/tipos das
   declarações, font no bloco do if, f vinda de cópia, helpers inline (corpo inteiro, LRU), i = pos,
   pragmas opt_*. */
unsigned int font_glyph_cache(SceFontGlyphImage *img, unsigned short code)
{
    FontCtx *f = font_ctx_ptr;
    int i = font_glyph_find(code);
    int font = f->font;
    int slot;
    int pos;
    int evict;
    SceFontCharInfo ci;
    unsigned char bitmap[0x200] __attribute__((aligned(8)));
    unsigned char *src, *dst;
    int k;

    if (i >= 0x100) {
        pos = i - 0x100;
        slot = f->count;
        if (slot == 0x100) {
            evict = font_glyph_find(f->slots[f->lru_head].code);
            if (pos < evict) {
                mem_move(&f->index[pos + 1], &f->index[pos], (evict - pos) * 4);
            } else if (evict < pos) {
                pos--;
                if (pos != evict)
                    mem_move(&f->index[evict], &f->index[evict + 1], (pos - evict) * 4);
            }
            slot = f->lru_head;
            f->lru_head = f->slots[slot].next;
            f->slots[f->lru_head].prev = 0xFFFF;
            f->count--;
        } else if (slot != 0) {
            if (slot != pos)
                mem_move(&f->index[pos + 1], &f->index[pos], (slot - pos) * 4);
        } else {
            f->lru_head = 0;
        }
        f->slots[slot].code = code;
        f->index[pos].code = code;
        f->index[pos].slot = slot;
        if (code < 0x2300 && font_half_width[code] != 0)
            font = f->font2;
        sceFontGetCharInfo(font, code, &ci);
        f->slots[slot].advance = ci.advance_h;
        memset(bitmap, 0, sizeof bitmap);
        img->x64 = 0x200;
        img->y64 = 0x200;
        img->x64 += ci.bitmap_left * 64;
        img->y64 += f->baseline - ci.bitmap_top * 64;
        img->buffer = bitmap;
        sceFontGetCharGlyphImage(font, code, img);
        src = bitmap;
        dst = f->atlas + slot / 16 * 0x2000 + slot % 16 * 0x80;
        for (k = 0; k < 4; k++) {
            memcpy(dst, src, 0x80);
            src += 0x80;
            dst += 0x800;
        }
        f->count++;
    } else {
        slot = f->index[i].slot;
        if (slot != f->lru_tail) {
            if (slot == f->lru_head)
                f->lru_head = f->slots[slot].next;
            else
                f->slots[f->slots[slot].prev].next = f->slots[slot].next;
            f->slots[f->slots[slot].next].prev = f->slots[slot].prev;
        }
    }
    if (f->lru_tail != slot) {
        if (f->lru_tail != 0xFFFF)
            f->slots[f->lru_tail].next = slot;
        f->slots[slot].prev = f->lru_tail;
        f->slots[slot].next = 0xFFFF;
        f->lru_tail = slot;
    }
    return slot;
}
#endif

/* 0: breakable/space, 1: punctuation that may end a line, 2: opening quote, -1: other. */
int wc_break_class(unsigned short c)
{
    int r;

    if (c < 0x20) {
        r = 0;
    } else {
        switch (c) {
        case ' ':
            r = 0;
            break;
        case '!':
        case ',':
        case '.':
        case '/':
        case ':':
        case ';':
        case '?':
            r = 1;
            break;
        case '"':
        case 0x201C:
            r = 2;
            break;
        default:
            r = -1;
            break;
        }
    }
    return r;
}

/* sorted lists: characters that cannot start / end a line */
unsigned short wc_nostart[47] = {
    0x0029, 0x005D, 0x007D, 0x2026, 0x3001, 0x3002, 0x3009, 0x300B, 0x300D, 0x300F, 0x3011, 0x3015,
    0x3041, 0x3043, 0x3045, 0x3047, 0x3049, 0x3063, 0x3083, 0x3085, 0x3087, 0x308E, 0x30A1, 0x30A3,
    0x30A5, 0x30A7, 0x30A9, 0x30C3, 0x30E3, 0x30E5, 0x30E7, 0x30EE, 0x30F5, 0x30F6, 0x30FB, 0x30FC,
    0xFF01, 0xFF05, 0xFF06, 0xFF09, 0xFF0C, 0xFF0D, 0xFF0E, 0xFF0F, 0xFF1A, 0xFF1F, 0xFF5E,
};
unsigned short wc_noend[10] = {
    0x0028, 0x005B, 0x007B, 0x2018, 0x201C, 0x300C, 0x300E, 0x3010, 0x3014, 0xFF08,
};

int wc_no_line_start(unsigned short c)
{
    return !u16_bsearch(c, wc_nostart, 47);
}

int wc_no_line_end(unsigned short c)
{
    return !u16_bsearch(c, wc_noend, 10);
}

int u16_bsearch(unsigned short c, const unsigned short *table, int n)
{
    int lo = 0;
    int hi = n - 1;
    int mid;

    while (lo <= hi) {
        mid = (lo + hi) / 2;
        if (c == table[mid])
            return 1;
        if (table[mid] < c)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 0;
}

/* ---- text layout (was font_text.c) ---- */
/* UCS-2 text layout and drawing. A FontLayout carries the pen; font_layout_wtext consumes one
   line into FontCtx.cmds (glyphs, size and color changes, ruby) and font_layout_end draws it. */

int wcslen(const unsigned short *s);
int wcschr_index(const unsigned short *s, unsigned short c);
unsigned short towupper(int c);
int wc_is_lower(int c);

/* Draws `text` at (x, y) in 26.6 inside width w (26.6), one line, breaking words that do not fit.
   Returns the number of characters consumed. */
int font_draw_text(int x, int y, int w, unsigned short *text)
{
    FontCtx *f = font_ctx_ptr;
    unsigned short *p = text;
    int n = 0;
    FontLayout st, saved;
    void *dl;
    unsigned int color, color2;
    int r;

    if (w < 0)
        return 0;
    dl = f->dl;
    if (f->saved_dl == 0)
        font_dl_begin(f->layer);
    else
        f->dl = f->saved_dl;
    memset(&st, 0, sizeof st);
    st.xmax = x + w;
    st.y = y;
    st.x = x;
    st.size_x = st.cur_size_x = f->size_x;
    st.size_y = st.cur_size_y = f->size_y;
    st.color = f->color;
    st.color2 = f->color2;
    st.max_glyphs = -1;
    if (f->flags & 4)
        st.xmax = x + w - 0x40;
    if (f->flags & 0x8000) {
        st.y += 0x1C0;
        st.ruby_xmax = st.xmax;
    }
    color = st.color;
    color2 = st.color2;
    memcpy(&saved, &st, sizeof st);
    r = font_layout_wtext(f, &st, text, 0);
    while (r < 0) {
        font_layout_end(f, &saved);
        p += st.consumed;
        n += st.consumed;
        st.consumed = 0;
        if (p == text)
            break;
        st.nglyphs = saved.nglyphs;
        memcpy(&saved, &st, sizeof st);
        r = font_layout_wtext(f, &st, p, 0);
    }
    font_layout_end(f, &saved);
    n += st.consumed;
    if (f->saved_dl == 0) {
        font_dl_end();
    } else {
        f->saved_dl = f->dl;
        f->dl = dl;
    }
    if (f->flags & 0x8000)
        st.y -= 0x1C0;
    f->cursor_x = st.x;
    f->cursor_y = f->unk50c3c + (st.y + f->line_h * st.size_y / 20);
    f->color = color;
    f->color2 = color2;
    f->size_x = st.size_x;
    f->size_y = st.size_y;
    return n;
}

/* Draws word-wrapped text in the w x h box (26.6); at most max_glyphs glyphs (-1 = all).
   Returns the number of characters consumed. */
int font_draw_wtext_ex(int x, int y, int w, int h, unsigned short *text, int max_glyphs)
{
    FontCtx *f = font_ctx_ptr;
    int bottom = y + h;      /* lowest baseline allowed, 26.6 */
    unsigned short *p = text;
    int n = 0;
    FontLayout st, saved;
    void *dl;
    unsigned int color, color2;
    int line_h, step, lh;
    unsigned short flags;
    int more;                /* another line still fits below this one */
    int r;

    if (w < 0)
        return 0;
    dl = f->dl;
    if (f->saved_dl == 0)
        font_dl_begin(f->layer);
    else
        f->dl = f->saved_dl;
    memset(&st, 0, sizeof st);
    st.xmax = x + w;
    st.y = y;
    st.x = x;
    st.size_x = st.cur_size_x = f->size_x;
    st.cur_size_y = st.size_y = f->size_y;
    color = f->color;
    st.color = color;
    color2 = f->color2;
    st.max_glyphs = max_glyphs;
    st.flags |= 8;
    st.color2 = color2;
    lh = f->line_h * (int)st.size_y / 20;
    flags = f->flags;
    step = lh;
    if (flags & 4)
        step = lh + 0x40;
    line_h = f->unk50c3c + lh;
    if (flags & 4)
        st.xmax -= 0x40;
    if (f->flags & 0x8000) {
        st.line_x0 = x;
        st.y += 0x1C0;
        line_h += 0x1C0;
        st.ruby_xmax = st.xmax;
        st.line_h = line_h;
    }
    memcpy(&saved, &st, sizeof st);
    while (st.y + step <= bottom) {
        more = 1;
        if (step + (st.y + line_h) > bottom)
            more = 0;
        r = font_layout_wtext(f, &st, p, more);
        n += st.consumed;
        p += st.consumed;
        st.consumed = 0;
        if (p == text)
            break;
        if (r < 0) {
            font_layout_end(f, &saved);
            memcpy(&saved, &st, sizeof st);
        } else {
            if (r != 1)
                break;
            if ((f->flags & 0x8000) && (st.flags & 2) && st.ruby_x + (int)(st.ruby_pad / 2u) >= st.x) {
                st.ruby_x = x + (int)(st.ruby_w / 2u);
                st.ruby_y = st.y + line_h - 0x1C0;
            }
            if (f->flags & 8) {
                while (*p == ' ')
                    p++;
            }
            st.x = x;
            st.y += line_h;
        }
    }
    font_layout_end(f, &saved);
    if (f->saved_dl == 0) {
        font_dl_end();
    } else {
        f->saved_dl = f->dl;
        f->dl = dl;
    }
    if (f->flags & 0x8000)
        st.y -= 0x1C0;
    f->cursor_x = st.x;
    f->cursor_y = st.y + line_h;
    f->color = color;
    f->color2 = color2;
    f->size_x = st.size_x;
    f->size_y = st.size_y;
    return n;
}

/* Lays out one line of UCS-2 text into f->cmds. Tags: $R...(ruby) $f $F (lower/upper small caps)
   $Cn (color) $$ (literal). Returns 0 at end of text, 1 at a line break (when `wrap`),
   2 when max_glyphs is reached, -1 when the command buffer is full. */
int font_layout_wtext(FontCtx *f, FontLayout *st, unsigned short *text, int wrap)
{
    SceFontGlyphImage img;
    FontCmd cmd;
    int r = 0;
    unsigned short *p, *end;
    unsigned short c;
    unsigned short pad;
    int pending = 0;
    int slot, adv, extra, w, yoff;
    unsigned char sx, sy;
    unsigned short n;

    img.buf_w = 32;
    img.buf_h = 32;
    img.bytes_per_line = 16;
    img.pixel_format = 0;
    img.pad = 0;
    p = (unsigned short *)text; /* o cast redundante impede a propagação da cópia (move s3,a2 do original) */
    end = p;
    while ((c = *p++) != 0) {
        pad = 0;
        if (!(st->flags & 0x20) && c == '$') {
            {
                unsigned char *d = (unsigned char *)&cmd;
                int k = sizeof cmd;

                if (d)
                    do {
                        *d++ = 0;
                    } while (--k);
            }
            p = font_parse_tag(f, st, &cmd, p);
            end = p;
            if ((unsigned short)cmd.kind != 0) {
                memcpy(&f->cmds[f->ncmds++], &cmd, sizeof cmd);
                if (f->ncmds >= 0x100) {
                    r = -1;
                    goto out;
                }
            }
            continue;
        }
        if ((f->flags & 0x8000) && (st->flags & 2))
            pad = st->ruby_pad >> 1;
        if ((st->flags & 2) && c == '(') {
            if (f->flags & 0x8000) {
                if (f->ncmds >= 230) {
                    p = end;
                    r = -1;
                    goto out;
                }
                f->cmds[f->ncmds].kind = 4;
                f->ncmds++;
                p = font_ruby_layout(f, st, p);
                f->cmds[f->ncmds].kind = 5;
                f->ncmds++;
                if (st->ruby_y + 0x1C0 == st->y && st->x < st->ruby_x)
                    st->x = st->ruby_x;
            } else {
                w = wcschr_index(p, ')');
                if (w >= 0)
                    p += w + 1;
                else
                    p += 1;
            }
            st->flags &= ~2;
            end = p;
            continue;
        }
        if ((f->flags & 0x4000) || *(unsigned char *)&st->force_upper != 0) {
            if (wc_is_lower(c)) {
                n = towupper(c);
                if (!(st->flags & 0x10)) {
                    sx = st->size_x * 10 / 12;
                    sy = st->size_y * 10 / 12;
                    f->cmds[f->ncmds].kind = 2;
                    f->cmds[f->ncmds].a = sx;
                    f->cmds[f->ncmds].b = sy;
                    f->ncmds++;
                    st->flags |= 0x10;
                    st->cur_size_x = sx;
                    st->cur_size_y = sy;
                    st->yoff = f->line_h * (f->size_y - sy) / 20 - 0x20;
                }
                c = n;
            } else if (st->flags & 0x10) {
                f->cmds[f->ncmds].kind = 2;
                f->cmds[f->ncmds].a = st->size_x;
                f->cmds[f->ncmds].b = st->size_y;
                f->ncmds++;
                st->flags &= ~0x10;
                st->cur_size_x = st->size_x;
                st->cur_size_y = st->size_y;
                st->yoff = 0;
            }
        }
        if (c >= 0x20) {
            slot = font_glyph_cache(&img, c);
            adv = f->slots[slot].advance * st->cur_size_x / 20;
            yoff = st->yoff;
            st->x += pad;
            extra = 0;
            if (f->flags & 1)
                extra = font_word_width(f, st, &img, p - 1, wrap);
            if (f->flags & 8) {
                w = wc_break_class(c);
                if (pending) {
                    extra = font_run_width(f, st, &img, p);
                    pending = 0;
                } else if (w == 1) {
                    n = wtext_next_char(f, p, 0);
                    if (wc_break_class(n) > 0)
                        extra = font_run_width(f, st, &img, p);
                    else
                        pending = 1;
                } else if (w == 0 || w == 2) {
                    extra = font_run_width(f, st, &img, p);
                }
            }
            w = st->x + pad + adv + extra;
            if (st->xmax < w) {
                p = end;
                if (wrap != 0)
                    r = 1;
                goto out;
            }
            if (st->max_glyphs < 0 || st->nglyphs < st->max_glyphs) {
                f->cmds[f->ncmds].kind = 1;
                f->cmds[f->ncmds].a = st->x;
                f->cmds[f->ncmds].b = st->y + yoff;
                f->cmds[f->ncmds].c = slot;
                if (st->flags & 0x20)
                    st->flags &= ~0x20;
                st->nglyphs++;
            } else {
                r = 2;
                goto out;
            }
            st->x += adv + pad;
            f->ncmds++;
            if (f->ncmds >= 0x100) {
                r = -1;
                goto out;
            }
        } else if (c == '\n') {
            if (wrap != 0)
                r = 1;
            goto out;
        } else switch (c) {
        case '\r':
            break;
        case '\t':
            st->x += f->max_glyph_w * 4;
            break;
        }
        end = p;
    }
out:
    if ((st->flags & 2) && r >= 0)
        st->ruby_xmax = st->x;
    st->consumed = p - text;
    return r;
}


/* Single-line draw that shrinks the font until the text fits in w (26.6). */
int font_draw_wtext(int x, int y, int w, unsigned short *text)
{
    FontCtx *f = font_ctx_ptr;
    int k = 1;          /* size step */
    int limit;          /* width to fit, 26.6 */
    int r;
    unsigned char sx, sy;
    int tw;
    unsigned short flags;
    unsigned int scaled;

    if (w < 0)
        return 0;
    if (text == 0 || *text == 0)
        return 0;
    font_get_size(&sx, &sy);
    tw = font_measure_text(text);
    if (tw <= 0)
        return 0;
    flags = f->flags;
    limit = w;
    if (flags & 4)
        limit = w - 0x40;
    if (flags & 0x8000 || flags & 0x4000) {
        for (; tw > limit; k++) {
            if ((int)sx < k + 6)
                break;
            font_set_size(sx - k, sy);
            tw = font_measure_text(text);
        }
    } else {
        if (limit < tw) {
            do {
                if ((int)sx < k + 6)
                    break;
                scaled = sx - k;
                k++;
            } while (tw * (int)scaled / (int)sx > limit);
        }
        font_set_size(sx - k + 1, sy);
    }
    r = font_draw_text(x, y, w, text);
    font_set_size(sx, sy);
    return r;
}

/* Font width (percent / 5) that makes `text` fit in w (26.6): the current width when it already
   fits, 0 for no text. The font size is restored afterwards. */
int font_measure_wtext(int w, unsigned short *text)
{
    FontCtx *f = font_ctx_ptr;
    unsigned char sx, sy, rx, ry;
    int tw, k;
    unsigned short gw, n;

    if (w < 0)
        return 0;
    if (text == 0 || *text == 0)
        return 0;
    font_get_size(&sx, &sy);
    tw = font_measure_text(text);
    if (tw <= 0)
        return sx;
    if (tw + 0x40 <= w)
        return sx;
    if (w < tw + 0x40) {
        gw = f->max_glyph_w / 20;
        n = wcslen(text);
        k = (tw + 0x40 - w) / n / gw;
        k = k - 1;
        if ((int)sx < k + 6)
            k = sx - 6;
        for (; tw + 0x40 > w; k++) {
            if ((int)sx < k + 6)
                break;
            font_set_size(sx - k, sy);
            tw = font_measure_text(text);
        }
    }
    font_get_size(&rx, &ry);
    font_set_size(sx, sy);
    return rx;
}

/* Lays out `text` word-wrapped in a w x h box without drawing; returns width << 16 | height. */
unsigned int font_measure_wtext2(int w, int h, unsigned short *text)
{
    FontCtx *f = font_ctx_ptr;
    FontLayout st;
    int lw;
    unsigned short *p = text;
    int best = 0;       /* widest line */
    int line_h;
    int step;
    int more;           /* another line still fits below this one */
    unsigned short flags;
    int lh;             /* line height, 26.6 */

    if (w < 0)
        return 0;
    if (text == 0)
        return 0;
    memset(&st, 0, sizeof st);
    st.xmax = w;
    st.size_x = st.cur_size_x = f->size_x;
    st.size_y = st.cur_size_y = f->size_y;
    st.flags |= 8;
    lh = f->line_h * (int)f->size_y / 20;
    flags = f->flags;
    step = lh;
    if (flags & 4)
        step = lh + 0x40;
    line_h = f->unk50c3c + lh;
    if (flags & 4)
        st.xmax -= 0x40;
    if (f->flags & 0x8000) {
        st.line_x0 = 0;
        st.y += 0x1C0;
        line_h += 0x1C0;
        st.ruby_xmax = st.xmax;
        st.line_h = line_h;
    }
    while (st.y + step < h) {
        more = 1;
        if (step + (st.y + line_h) >= h)
            more = 0;
        lw = font_layout_text(f, &st, p, more);
        if (best < lw)
            best = lw;
        p += st.consumed;
        st.consumed = 0;
        if (p == text || p[-1] == 0 || !more)
            break;
        if (f->flags & 8) {
            while (*p == ' ')
                p++;
            if (*p == 0)
                break;
        }
        st.x = 0;
        st.y += line_h;
    }
    if (f->flags & 4)
        best += 0x40;
    return (best & 0xFFFF) << 16 | ((st.y + step) & 0xFFFF);
}

/* Height in 26.6 of `text` word-wrapped at width w. */
int font_wtext_lines(int w, unsigned short *text)
{
    FontCtx *f = font_ctx_ptr;
    FontLayout st;
    int lw;
    unsigned short *p = text;
    int best = 0;       /* widest line; computed but not returned */
    int line_h;
    int step;
    unsigned short flags;
    int h;              /* line height, 26.6 */

    if (w < 0)
        return 0;
    if (text == 0)
        return 0;
    memset(&st, 0, sizeof st);
    st.xmax = w;
    st.size_x = st.cur_size_x = f->size_x;
    st.size_y = st.cur_size_y = f->size_y;
    st.flags |= 8;
    h = f->line_h * (int)f->size_y / 20;
    flags = f->flags;
    step = h;
    if (flags & 4)
        step = h + 0x40;
    line_h = f->unk50c3c + h;
    if (flags & 4)
        st.xmax -= 0x40;
    if (f->flags & 0x8000) {
        st.line_x0 = 0;
        st.y += 0x1C0;
        line_h += 0x1C0;
        st.ruby_xmax = st.xmax;
        st.line_h = line_h;
    }
    for (;;) {
        lw = font_layout_text(f, &st, p, 1);
        if (best < lw)
            best = lw;
        p += st.consumed;
        st.consumed = 0;
        if (p == text || p[-1] == 0)
            break;
        if (f->flags & 8) {
            while (*p == ' ')
                p++;
            if (*p == 0)
                break;
        }
        st.x = 0;
        st.y += line_h;
    }
    return st.y + step;
}

/* Measures one line of text with the same rules as font_layout_wtext but emits no commands.
   Returns the pen x after the text (26.6). */
int font_layout_text(FontCtx *f, FontLayout *st, unsigned short *text, int wrap)
{
    SceFontGlyphImage img;
    FontCmd cmd;
    unsigned short *p, *end;
    unsigned short c;
    unsigned short pad;
    int pending = 0;
    int slot, adv, extra, w, sx, sy;
    unsigned short n;

    img.buf_w = 32;
    img.buf_h = 32;
    img.bytes_per_line = 16;
    img.pixel_format = 0;
    img.pad = 0;
    if (text == 0)
        return 0;
    p = (unsigned short *)text; /* o cast redundante impede a propagação da cópia (move s3,a2 do original) */
    end = p;
    while ((c = *p++) != 0) {
        pad = 0;
        if (!(st->flags & 0x20) && c == '$') {
            p = font_parse_tag(f, st, &cmd, p);
            end = p;
            continue;
        } else {
            if ((f->flags & 0x8000) && (st->flags & 2))
                pad = st->ruby_pad >> 1;
            if ((st->flags & 2) && c == '(') {
                if (f->flags & 0x8000) {
                    p = font_ruby_skip(f, st, p);
                    if (st->ruby_y + 0x1C0 == st->y && st->x < st->ruby_x)
                        st->x = st->ruby_x;
                } else {
                    w = wcschr_index(p, ')');
                    if (w >= 0)
                        p += w + 1;
                    else
                        p += 1;
                }
                st->flags &= ~2;
                end = p;
                continue;
            } else {
                if ((f->flags & 0x4000) || *(unsigned char *)&st->force_upper != 0) {
                    if (wc_is_lower(c)) {
                        n = towupper(c);
                        if (!(st->flags & 0x10)) {
                            sx = st->size_x * 10 / 12;
                            sy = st->size_y * 10 / 12;
                            st->flags |= 0x10;
                            st->cur_size_x = sx;
                            st->cur_size_y = sy;
                            st->yoff = f->line_h * (f->size_y - (sy & 0xFF)) / 20 - 0x20;
                        }
                        c = n;
                    } else if (st->flags & 0x10) {
                        st->flags &= ~0x10;
                        st->cur_size_x = st->size_x;
                        st->cur_size_y = st->size_y;
                        st->yoff = 0;
                    }
                }
                if (c >= 0x20) {
                    slot = font_glyph_cache(&img, c);
                    adv = f->slots[slot].advance * st->cur_size_x / 20;
                    st->x += pad;
                    extra = 0;
                    if (f->flags & 1)
                        extra = font_word_width(f, st, &img, p - 1, wrap);
                    if (f->flags & 8) {
                        w = wc_break_class(c);
                        if (pending) {
                            extra = font_run_width(f, st, &img, p);
                            pending = 0;
                        } else if (w == 1) {
                            n = wtext_next_char(f, p, 0);
                            if (wc_break_class(n) > 0)
                                extra = font_run_width(f, st, &img, p);
                            else
                                pending = 1;
                        } else if (w == 0 || w == 2) {
                            extra = font_run_width(f, st, &img, p);
                        }
                    }
                    w = st->x + pad + adv + extra;
                    if (st->xmax < w) {
                        p = end;
                        break;
                    }
                    st->x += adv + pad;
                    if (st->flags & 0x20)
                        st->flags &= ~0x20;
                } else switch (c) {
                case '\r':
                    break;
                case '\t':
                    st->x += f->max_glyph_w * 4;
                    break;
                case '\n':
                    goto out;
                }
            }
        }
        end = p;
    }
out:
    if (st->flags & 2)
        st->ruby_xmax = st->x;
    st->consumed = p - text;
    return st->x;
}

/* ---- layer state accessors (was font_flags.c) ---- */
/* Font layer state accessors: layout flags, extra line spacing, and the display list used
   while a menu item's text is being built. */

void dl_set_list(void *list);
void *dl_push1(void *list, unsigned int word);

void font_set_flags(unsigned short flags)
{
    font_ctx_ptr->flags = flags;
}

unsigned short font_get_flags(void)
{
    return font_ctx_ptr->flags;
}

void font_set_flag1(int on)
{
    FontCtx *f = font_ctx_ptr;

    f->flags &= ~1;
    if (on)
        f->flags |= 1;
}

int font_get_flag1(void)
{
    return (font_ctx_ptr->flags & 1) != 0;
}

void font_set_flag2(int on)
{
    FontCtx *f = font_ctx_ptr;

    f->flags &= ~2;
    if (on)
        f->flags |= 2;
}

int font_get_flag2(void)
{
    return (font_ctx_ptr->flags & 2) != 0;
}

void font_set_flag4(int on)
{
    FontCtx *f = font_ctx_ptr;

    f->flags &= ~4;
    if (on)
        f->flags |= 4;
}

int font_get_flag4(void)
{
    return (font_ctx_ptr->flags & 4) != 0;
}

void font_set_flag8(int on)
{
    FontCtx *f = font_ctx_ptr;

    f->flags &= ~8;
    if (on)
        f->flags |= 8;
}

int font_get_flag8(void)
{
    return (font_ctx_ptr->flags & 8) != 0;
}

void font_set_flag4000(int on)
{
    FontCtx *f = font_ctx_ptr;

    f->flags &= ~0x4000;
    if (on)
        f->flags |= 0x4000;
}

int font_get_flag4000(void)
{
    return (font_ctx_ptr->flags & 0x4000) != 0;
}

void font_set_flag8000(int on)
{
    FontCtx *f = font_ctx_ptr;

    f->flags &= ~0x8000;
    if (on)
        f->flags |= 0x8000;
}

int font_get_flag8000(void)
{
    return (font_ctx_ptr->flags & 0x8000) != 0;
}

void font_set_line_extra(int v)
{
    font_ctx_ptr->unk50c3c = v;
}

int font_get_line_extra(void)
{
    return font_ctx_ptr->unk50c3c;
}

/* Redirects glyph output into `list` (a menu item's display list) until font_text_clear. */
void font_text_set(void *list)
{
    FontCtx *f = font_ctx_ptr;

    f->saved_dl = list;
    f->saved_dl = font_dl_glyph_header(list, f->color);
}

void *font_text_clear(void)
{
    FontCtx *f = font_ctx_ptr;
    void *list = f->saved_dl;

    f->saved_dl = 0;
    return dl_push1(list, 0x13000000);
}

void font_text_flush(void)
{
    FontCtx *f = font_ctx_ptr;

    dl_set_list(dl_push1(f->saved_dl, 0x13000000));
    f->saved_dl = 0;
}

/* ---- tags and line emission (was font_tags.c) ---- */
/* Text tags and line emission: $R ruby (furigana over the base text), $C color, $f/$F small
   capitals, word-break measurement, and font_layout_end which turns FontCtx.cmds into glyph
   quads. Also the sceFont allocator callbacks over FontCtx.scratch. */

extern char font_rubi_gim_name[16];

int wcschr_index(const unsigned short *s, unsigned short c);
unsigned short *wcsncpy(unsigned short *d, const unsigned short *s, unsigned int n);
unsigned short towupper(int c);
int wc_is_upper(unsigned short c);
int cfg_get_language(void);
unsigned char *syspack_get(void);
void *ehp_find(void *pack, const char *name, int flags);
void *dl_alloc_inline(void *list, int bytes);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_push(void *list, void *words, int n);
void *dl_push1(void *list, unsigned int word);
void *gim_set_texture(void *list, void *gim);
extern const unsigned char font_clut[32];           /* .rodata 0x08875740: 16-entry 16-bit palette of the 4bpp atlas */

/* Next printable character after the tags at p; `in_ruby` skips a "(...)" ruby body. */
unsigned short wtext_next_char(FontCtx *f, unsigned short *p, int in_ruby)
{
    unsigned short c = 0;
    unsigned short *q;
    int k;

    while (*p) {
        c = *p;
        q = p + 1;
        if (c == '$') {
            switch (*q) {
            case 'C':
                p = q + 2;
                break;
            case 'F':
            case 'f':
                p = q + 1;
                break;
            case 'R':
                p = q + 1;
                in_ruby = 1;
                break;
            default:
                goto done;
            }
        } else {
            if (in_ruby == 0)
                break;
            if (c != '(')
                break;
            k = wcschr_index(q, ')');
            if (k < 0)
                break;
            p = q + (k + 1);
        }
    }
done:
    return c;
}

/* Forma que bate (rodada 33): o resultado de wtext_next_char passa por `ch` (unsigned short) antes de `c`; com
   `c` definido direto do jal ele ficava sempre no $s mais baixo, qualquer que fosse a ordem das declarações. */
/* Width in 26.6 of the word starting at p (up to the next breakable character), including the
   ruby padding, so that a word is not split across lines. */
int font_word_width(FontCtx *f, FontLayout *st, SceFontGlyphImage *img, unsigned short *p, int wrap)
{
    unsigned short pad = 0;
    unsigned short rtotal, rpad, rw;
    unsigned short *q;
    unsigned short c;
    int in_ruby;
    int in_word;
    int slot;
    unsigned short *next;
    int w;
    unsigned short ch;

    q = p;
    in_ruby = 0;
    in_word = 0;
    w = 0;
    if (st->flags & 2) {
        pad = st->ruby_pad;
        in_ruby = 1;
    }
    while ((c = *q++) != 0) {
        if (c >= 0x20) {
            if (c == '$') {
                switch (*q) {
                case 'C':
                    q += 2;
                    break;
                case 'F':
                case 'f':
                    q++;
                    break;
                case 'R':
                    q++;
                    if (f->flags & 0x8000) {
                        next = font_ruby_measure(f, q, &pad, &rpad, &rw, &rtotal);
                        if (wrap != 0) {
                            w += (unsigned int)pad >> 1;
                        } else {
                            q = next;
                            c = rtotal;
                            in_word = 0;
                            w += rw;
                            goto glyph;
                        }
                    }
                    in_ruby = 1;
                    break;
                default:
                    goto glyph;
                }
            } else {
            glyph:
                if (in_ruby && c == '(') {
                    slot = wcschr_index(q, ')');
                    if (slot >= 0)
                        q += slot + 1;
                    else
                        q++;
                    pad = 0;
                    in_ruby = 0;
                } else if (in_word) {
                    slot = font_glyph_cache(img, c);
                    w += f->slots[slot].advance * st->cur_size_x / 20;
                    w += pad;
                    if (wc_no_line_end(c) != 0) {
                        in_word = 0;
                        goto tail;
                    }
                } else if (wc_no_line_end(c) == 0) {
                    slot = font_glyph_cache(img, c);
                    if (p != q - 1)
                        w += f->slots[slot].advance * st->cur_size_x / 20;
                    in_word = 1;
                    w += pad;
                } else {
                tail:
                    ch = wtext_next_char(f, q, in_ruby);
                    c = ch;
                    if (!(st->flags & 8))
                        break;
                    if (wc_no_line_start(c) != 0)
                        break;
                    slot = font_glyph_cache(img, c);
                    w += f->slots[slot].advance * st->cur_size_x / 20;
                    w += pad;
                }
            }
        } else {
            break;
        }
    }
    return w;
}

static inline void font_fold_case(FontLayout *st, unsigned short *c, unsigned short *flags, unsigned char *sx)
{
    unsigned int ch = *c;

    if (wc_is_upper(ch) != 0) {
        if (*flags & 0x10) {
            *sx = st->size_x;
            *flags &= ~0x10;
        }
    } else {
        unsigned short r = towupper(ch);
        if (!(*flags & 0x10)) {
            *flags |= 0x10;
            *sx = st->size_x * 10 / 12;
        }
        *c = r;
    }
}

/* Width in 26.6 of the run of non-breakable characters starting at p. */
int font_run_width(FontCtx *f, FontLayout *st, SceFontGlyphImage *img, unsigned short *p)
{
    unsigned short c;
    int upper = (unsigned char)st->force_upper;
    unsigned char sx = st->cur_size_x;
    unsigned short flags = st->flags;
    int w = 0;
    unsigned short t;
    int cls, slot;

    while ((c = *p++) != 0) {
        if (c >= 0x20) {
            if (c == '$') {
                switch (*p) {
                case 'C':
                    p += 2;
                    continue;
                case 'F':
                    upper = 1;
                    p++;
                    continue;
                case 'f':
                    upper = 0;
                    p++;
                    continue;
                case 'R':
                    p++;
                    continue;
                }
            }
            if ((f->flags & 0x4000) || upper != 0)
                font_fold_case(st, &c, &flags, &sx);
            cls = wc_break_class(c);
            if (cls == 0)
                break;
            if (cls == 1) {
                t = wtext_next_char(f, p, 0);
                if ((f->flags & 0x4000) || upper != 0)
                    font_fold_case(st, &t, &flags, &sx);
                if (wc_break_class(t) <= 0) {
                    slot = font_glyph_cache(img, c);
                    w += f->slots[slot].advance * sx / 20;
                    break;
                }
            }
            slot = font_glyph_cache(img, c);
            w += f->slots[slot].advance * sx / 20;
        } else {
            break;
        }
    }
    return w;
}

/* Handles the character after '$'; fills *cmd for size/color changes. Returns the next position. */
unsigned short *font_parse_tag(FontCtx *f, FontLayout *st, FontCmd *cmd, unsigned short *p)
{
    unsigned short *next = p + 1;
    unsigned short dummy2, dummy1;
    unsigned int color;
    unsigned short n;

    switch (*p) {
    case '$':
        st->flags |= 0x20;
        return p;
    case 'C':
        n = *next;
        next++;
        if (f->flags & 2) {
            if (n >= '0' && n <= '9')
                n -= '0';
            else
                n += (unsigned short)-0x37;   /* hex letter: n - (0x41 - 10) */
            if (n == 0) {
                st->color = f->color;
                st->color2 = f->color2;
                cmd->kind = 3;
                cmd->a = f->color;
                cmd->b = f->color2;
            } else {
                color = (f->color & 0xFF000000) | font_color_table[n % 15];
                st->color = color;
                st->color2 = font_shadow_color(color);
                cmd->kind = 3;
                cmd->a = color;
                cmd->b = st->color2;
            }
        }
        break;
    case 'F':
        st->force_upper = 1;
        break;
    case 'f':
        st->force_upper = 0;
        if (st->flags & 0x10) {
            cmd->kind = 2;
            cmd->a = st->size_x;
            cmd->b = st->size_y;
            st->flags &= ~0x10;
            st->cur_size_x = st->size_x;
            st->cur_size_y = st->size_y;
            st->yoff = 0;
        }
        break;
    case 'R':
        if (f->flags & 0x8000) {
            font_ruby_measure(f, next, &st->ruby_pad, &st->ruby_w, &dummy1, &dummy2);
            st->ruby_x = st->x + st->ruby_w / 2u;
            st->ruby_y = st->y - 0x1C0;
        }
        st->flags |= 2;
        break;
    default:
        return p;
    }
    return next;
}

/* Measures a "$Rbase(ruby)" group at p: *pad = extra advance per base character, *w = extra
   width of the ruby beyond the base, *total = width of the wider of the two, *last = last base
   character. Returns the position after ')'. */
unsigned short *font_ruby_measure(FontCtx *f, unsigned short *p, unsigned short *pad, unsigned short *w, unsigned short *total, unsigned short *last)
{
    unsigned short base[24];
    int n, bw, k, rw;
    unsigned short *q;

    *last = 0;
    *total = 0;
    *w = 0;
    *pad = 0;
    n = wcschr_index(p, '(');
    if (n < 0) {
        return p;
    }
    wcsncpy(base, p, n);
    if (n > 0)
        *last = base[n - 1];
    base[n] = 0;
    bw = font_measure_text(base);
    q = p + (n + 1);
    k = wcschr_index(q, ')');
    if (k <= 0) {
        return p;
    }
    rw = k * 0x1C0;
    p = q + (k + 1);
    if (bw < rw) {
        *pad = (rw - bw) / n;
        *w = 0;
        *total = rw;
    } else {
        *pad = 0;
        *w = (bw - rw) / k;
        *total = bw;
    }
    return p;
}

/* GE state for drawing glyph quads from the atlas: texture (ctx base, 512x256 4bpp), color. */
void *font_dl_glyph_header(void *dl, unsigned int color)
{
    unsigned int cmds[20] = {
        0x21000001, 0xDF000032, 0x1E000001, 0xC9000100, 0xC7000101, 0xC6000101, 0, 0,
        0xB8000909, 0xC2000001, 0xC3000004, 0xCB000000, 0xC500FF02, 0, 0, 0xC4000001,
        0, 0, 0x10000000, 0x12800182,
    };

    cmds[6] = ((unsigned int)font_ctx_ptr & 0xFFFFFF) | 0xA0000000;
    cmds[7] = (((unsigned int)font_ctx_ptr >> 24) & 0xFF) << 16 | 0xA8000200;
    cmds[13] = ((unsigned int)font_clut & 0xFFFFFF) | 0xB0000000;
    cmds[14] = (((unsigned int)font_clut >> 24) & 0xFF) << 16 | 0xB1000000;
    cmds[16] = (color & 0xFFFFFF) | 0x55000000;
    cmds[17] = (color >> 24) | 0x58000000;
    return dl_push(dl, cmds, 20);
}

/* Shadow color for text color c: alpha halved for light colors, quartered for dark ones. */
unsigned int font_shadow_color(unsigned int c)
{
    unsigned int a = (c >> 24) & 0xFF;

    if (((c & 0xFF) * 299 + ((c & 0xFF00) >> 8) * 587 + ((c & 0xFF0000) >> 16) * 114) / 1000 < 0x80)
        return ((a << 6) / 255) << 24;
    return ((a << 7) / 255) << 24;
}
/* Emits the ruby characters of "(...)" at p as glyph commands above the base text. */
/* K&R local declaration: the original passes the raw c to wc_no_line_start (no unsigned short mask). */
unsigned short *font_ruby_layout(FontCtx *f, FontLayout *st, unsigned short *p)
{
    int c;
#ifdef __MWERKS__ /* declaração só para o matching; o port usa o protótipo do arquivo */
    int wc_no_line_start();
#endif

    while ((c = *p++) != ')') {
        if ((unsigned short)c < 0x20)
            break;
        if (st->ruby_x + 0x1C0 > st->ruby_xmax) {
            if (!(st->flags & 8) || wc_no_line_start(c) != 0) {
                st->ruby_y += st->line_h;
                st->ruby_x = st->line_x0 + st->ruby_w / 2u;
            }
        }
        f->cmds[f->ncmds].kind = 1;
        f->cmds[f->ncmds].a = st->ruby_x;
        f->cmds[f->ncmds].b = st->ruby_y;
        f->cmds[f->ncmds].c = c & 0xFFFF;
        st->ruby_x += st->ruby_w + 0x1C0;
        f->ncmds++;
        if (f->ncmds > 0xFF)
            break;
    }
    st->ruby_x -= (unsigned int)st->ruby_w >> 1;
    st->ruby_xmax = st->xmax;
    return p;
}

/* Same as font_ruby_layout without emitting commands (measurement). */
unsigned short *font_ruby_skip(FontCtx *f, FontLayout *st, unsigned short *p)
{
    unsigned short c;

    while ((c = *p++) != ')') {
        if (c < 0x20)
            break;
        if (st->ruby_x + 0x1C0 > st->ruby_xmax) {
            if (!(st->flags & 8) || wc_no_line_start(c) != 0) {
                st->ruby_y += st->line_h;
                st->ruby_x = st->line_x0 + st->ruby_w / 2u;
            }
        }
        st->ruby_x += st->ruby_w + 0x1C0;
    }
    st->ruby_x -= (unsigned int)st->ruby_w >> 1;
    st->ruby_xmax = st->xmax;
    return p;
}
/* Draws the commands of the current line: shadow pass (flag 4) then the main pass; ruby glyphs
   come from rubi_font00.gim. Resets the command count. The glyph slot is passed as unsigned (c->c is loaded
   before the other arguments). */
void font_layout_end(FontCtx *f, FontLayout *st)
{
    unsigned char sx = f->size_x;
    unsigned char sy = f->size_y;
    short *verts;
    int n;
    int i;
    int in_ruby;
    unsigned int color;
    FontCmd *c;

    verts = font_flush_quads(&f->dl, 0, 0);
    n = 0;
    if (f->flags & 4) {
        in_ruby = 0;
        f->dl = dl_ambient_color(f->dl, st->color2);
        for (i = 0; i < f->ncmds; i++) {
            c = &f->cmds[i];
            switch ((unsigned short)c->kind) {
            case 1:
                if (!in_ruby) {
                    font_glyph_vertices(verts + n * 16, c->a, c->b + 0x40, (unsigned int)c->c);
                    font_glyph_vertices(verts + (n + 1) * 16, c->a + 0x40, c->b + 0x40, (unsigned int)c->c);
                    n += 2;
                }
                break;
            case 3:
                if (!in_ruby) {
                    if (n > 0) {
                        verts = font_flush_quads(&f->dl, verts, n);
                        n = 0;
                    }
                    f->dl = dl_ambient_color(f->dl, c->b);
                }
                break;
            case 4:
                in_ruby = 1;
                break;
            case 5:
                in_ruby = 0;
                break;
            case 2:
                f->size_x = c->a;
                f->size_y = c->b;
                break;
            }
        }
    }
    if (n > 0) {
        verts = font_flush_quads(&f->dl, verts, n);
        n = 0;
    }
    f->size_x = sx;
    f->size_y = sy;
    color = st->color;
    in_ruby = 0;
    f->dl = dl_ambient_color(f->dl, color);
    for (i = 0; i < f->ncmds; i++) {
        c = &f->cmds[i];
        switch ((unsigned short)c->kind) {
        case 1:
            if (in_ruby) {
                font_rubi_quad(c->a, c->b, (unsigned short)c->c);
            } else {
                font_glyph_vertices(&verts[n * 16], c->a, c->b, (unsigned int)c->c);
                n++;
            }
            break;
        case 3:
            if (n > 0) {
                verts = font_flush_quads(&f->dl, verts, n);
                n = 0;
            }
            f->dl = dl_ambient_color(f->dl, c->a);
            color = c->a;
            break;
        case 4:
            in_ruby = 1;
            if (n > 0) {
                verts = font_flush_quads(&f->dl, verts, n);
                n = 0;
            }
            if (f->rubi_gim == 0)
                f->rubi_gim = ehp_find(syspack_get(), font_rubi_gim_name, 0);
            f->dl = dl_push1(f->dl, 0x13000000);
            f->dl = gim_set_texture(f->dl, f->rubi_gim);
            f->dl = dl_set_tex_filter(f->dl, 0, 0);
            break;
        case 5:
            in_ruby = 0;
            f->dl = font_dl_glyph_header(f->dl, color);
            verts = font_flush_quads(&f->dl, 0, 0);
            break;
        case 2:
            f->size_x = c->a;
            f->size_y = c->b;
            break;
        }
    }
    if (n > 0)
        font_flush_quads(&f->dl, verts, n);
    f->ncmds = 0;
}

/* With n == 0 returns where the next sprite vertices go (after the jump the real call emits);
   with n > 0 reserves the inline vertex memory and emits the draw. Returns *dl + 0x20. */
void *font_flush_quads(void **dl, void *verts, int n)
{
    if (n * 0x30 > 0) {
        dl_alloc_inline(dl, n * 0x30);
        *dl = dl_draw_array(*dl, 6, 0x800182, n * 2, 0, verts);
    }
    return (char *)*dl + 0x20;
}

int font_get_cursor_x(void)
{
    return font_ctx_ptr->cursor_x;
}

int font_get_cursor_y(void)
{
    return font_ctx_ptr->cursor_y;
}

/* One 8x8 ruby glyph (kana 0x3041..0x30FF from rubi_font00.gim) at (x, y) in 26.6. */
void font_rubi_quad(int x, int y, int code)
{
    FontCtx *f = font_ctx_ptr;
    short v[10];
    unsigned int cmds[11] = { 0 };
    unsigned int base;
    unsigned short k;
    int u, w;

    if ((unsigned short)code > 0x303F || (unsigned short)code < 0x3100) {
        k = (code & 0xFFFF) - 0x3040;
        u = (k & 0xF) * 8;
        w = (k >> 4) * 8;
        v[0] = u;
        v[1] = w;
        v[5] = u + 7;
        v[6] = w + 7;
        v[2] = (x + 0x20) >> 6;
        v[7] = v[2] + 7;
        v[3] = (y + 0x20) >> 6;
        v[8] = v[3] + 7;
        v[4] = 0;
        v[9] = 0;
        base = (unsigned int)f->dl + 0x1C;
        cmds[0] = (base >> 24 & 0xF) << 16 | 0x10000000;
        cmds[1] = (base & 0xFFFFFF) | 0x08000000;
        memcpy(&cmds[2], v, 0x14);
        cmds[7] = 0x12800102;
        base = (unsigned int)f->dl + 8;
        cmds[8] = (base >> 24 & 0xF) << 16 | 0x10000000;
        cmds[9] = (base & 0xFFFFFF) | 0x01000000;
        cmds[10] = 0x04060002;
        f->dl = dl_push(f->dl, cmds, 11);
    }
}

/* Copies src to dst dropping "$R" tags and the "(...)" ruby bodies (keeps the base text). */
void wtext_strip_tags(unsigned short *dst, unsigned short *src)
{
    unsigned short c;
    int in_ruby = 0;
    int k;

    while ((c = *src++) != 0) {
        if (c == '$') {
            c = *src++;
            if (c == 'R') {
                in_ruby = 1;
                continue;
            }
        }
        if (in_ruby && c == '(') {
            k = wcschr_index(src, ')');
            src += k + 1;
            in_ruby = 0;
        } else {
            *dst++ = c;
        }
    }
    *dst = 0;
}

/* Copies src to dst keeping only the ruby text of "$Rbase(ruby)" groups. */
void wtext_strip_tags2(unsigned short *dst, unsigned short *src)
{
    unsigned short c;
    int in_ruby = 0;
    int k;

    while ((c = *src++) != 0) {
        if (c == '$') {
            c = *src++;
            if (c == 'R') {
                k = wcschr_index(src, '(');
                src += k + 1;
                in_ruby = 1;
                continue;
            }
        }
        if (in_ruby && c == ')') {
            in_ruby = 0;
        } else {
            *dst++ = c;
        }
    }
    *dst = 0;
}

/* Number of visible characters in src (tags and ruby bodies excluded). */
int wtext_count_tags(unsigned short *text)
{
    unsigned short c;
    int n = 0;
    unsigned short *src = text;
    int in_ruby = 0;
    int k;

    while ((c = *src++) != 0) {
        if (c == '$') {
            c = *src++;
            switch (c) {
            case 'R':
                in_ruby = 1;
                continue;
            case 'C':
            case 'F':
            case 'f':
                src++;
                continue;
            }
        }
        if (in_ruby && c == '(') {
            k = wcschr_index(src, ')');
            src += k + 1;
            in_ruby = 0;
        } else {
            n++;
        }
    }
    return n;
}

/* Literal of font_layout_end, defined here so that it sits between the template of
   font_dl_glyph_header and the one of font_sample_glyph_size in .data. */
char font_rubi_gim_name[16] = "rubi_font00.gim";

/* Width and height (26.6) of the language's sample glyph: width << 16 | height. */
unsigned int font_sample_glyph_size(void)
{
    unsigned short samples[6][2] = { { 0x3042, 0 }, { 'A', 0 }, { 'A', 0 }, { 'A', 0 }, { 'A', 0 }, { 'A', 0 } };
    unsigned int w;

    w = (unsigned short)font_measure_text(samples[cfg_get_language()]) << 16;
    return w | (unsigned short)font_line_height();
}

void font_scratch_reset(void)
{
    FontCtx *f = font_ctx_ptr;

    f->scratch_ptr = f->scratch;
}

/* sceFont allocator: bump pointer in FontCtx.scratch, 4-byte granules. */
void *font_alloc_cb(void *data, unsigned int size)
{
    FontCtx *f = font_ctx_ptr;
    unsigned int words;
    unsigned char *p;

    if (size == 0)
        words = 1;
    else
        words = (size + 3) >> 2;
    p = f->scratch_ptr;
    f->scratch_ptr = p + words * 4;
    return p;
}

/* sceFont free: rewinds the bump pointer when the block is the last one. */
void font_free_cb(void *data, void *p)
{
    FontCtx *f = font_ctx_ptr;

    if ((unsigned char *)p > f->scratch_ptr)
        return;
    f->scratch_ptr = p;
}
