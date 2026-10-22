// @range 0x08845AC0-0x08847008
// @asm 0x08845AC0-0x08845DDC
// @data 0x088EE8C4-0x088EE904
// @asm 0x088462E8-0x0884638C
// @asm 0x088469E8-0x08846EA0
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

#ifdef NON_MATCHING
/* Kept in asm: register allocation and the double copy of the FontNewLibParams template differ. */
int font_init(void *fs)
{
    FontNewLibParams params = { 0, 2, 0, font_alloc_cb, font_free_cb, 0, 0, 0, 0, 0, 0 };
    FontCtx *f = font_ctx_ptr;
    SceFontInfo info;
    SceFontStyle style;
    int best;
    unsigned int err;

    do {
        f->module_id = ehfile_load_module(fs, "libfont.prx");
    } while (f->module_id <= 0);
    sceKernelStartModule(f->module_id, 0, 0, (int *)&err, 0);
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
    if (region_get() == 1 || region_get() == 0)
        f->font2 = f->font;
    else
        f->font2 = sceFontOpen(f->lib, best, 0, &err);
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
#endif

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

/* Draws tag-free UCS-2 text at (x, y) in 26.6, wrapping at xmax back to x0; returns x << 16 | y.
   c is int (font_glyph_cache gets it unmasked); the tests compare its low 16 bits. */
unsigned int font_draw_wtext_simple(int x, int y, int x0, int xmax, unsigned short *text)
{
    FontCtx *f = font_ctx_ptr;
    SceFontGlyphImage img;
    int c;
    int began = 0;
    unsigned int slot;
    int adv;

    img.pixel_format = 0;
    img.buf_w = 32;
    img.buf_h = 32;
    img.bytes_per_line = 16;
    img.pad = 0;
    while ((c = *text++) != 0) {
        if ((unsigned short)c >= 0x20) {
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
            switch ((unsigned short)c) {
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
#ifdef NON_MATCHING /* 29 palavras: o original calcula (mid - 1) * 4 sem dobrar o -1 no deslocamento e materializa lo < mid em v1 (slt v1, não at); índice ou ponteiro local, >=/> nos testes e do-while não mudam. O parâmetro é unsigned int (o chamador font_glyph_cache passa code sem máscara) */
int font_glyph_find(unsigned int code)
{
    FontCtx *f = font_ctx_ptr;
    int lo = 0;
    int hi = f->count + 1;
    int mid = hi / 2;
    unsigned short c = code;

    while (lo < mid) {
        if (f->index[mid - 1].code < c)
            lo = mid;
        else if (c < f->index[mid - 1].code)
            hi = mid;
        else
            break;
        mid = (lo + hi) / 2;
    }
    if (lo < mid)
        return mid - 1;
    return mid + 0x100;
}
#endif

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
#ifdef NON_MATCHING
static const GlyphCmds glyph_cmds_template = { 0x14000000, 0x0100000C, 0x0800002C, { 0x00010001, 0, 0, 0 }, { 0x00150015, 0, 0, 0 }, 0x04060002 };
#else
extern const GlyphCmds glyph_cmds_template; /* 0x088EE93C, no .data em asm (fora do @data deste arquivo) */
#endif

/* Emits a 2-vertex textured quad for atlas cell `slot` at (x, y) in 26.6. */
void font_glyph_quad(int x, int y, int slot)
{
    FontCtx *f = font_ctx_ptr;
    int u = (slot % 16) * 32 + 1;
    int v = (slot / 16) * 32 + 1;
    GlyphCmds cmds;
    unsigned int *q;
    float fx, fy;

    cmds = glyph_cmds_template;
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

/* Same quad written as two 16-byte vertices (u16 uv, float xyz) into v. */
void font_glyph_vertices(short *v, int x, int y, int slot)
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
/* Fica em asm (60 palavras): forma igual, mas o original dá f a s1 e font a s5 (aqui trocados) e
   mascara code mais tarde; nenhuma ordem de declaração muda isso. */
unsigned int font_glyph_cache(SceFontGlyphImage *img, unsigned int code)
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
        if ((unsigned short)code < 0x2300 && font_half_width[(unsigned short)code] != 0)
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

/* sorted lists in .data (asm): characters that cannot start / end a line */
extern const unsigned short wc_nostart[47];
extern const unsigned short wc_noend[10];

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
