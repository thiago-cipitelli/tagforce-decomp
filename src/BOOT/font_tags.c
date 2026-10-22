// @range 0x0884900C-0x0884A9B4
// @data 0x088EE9E0-0x088EEA30
// @asm 0x0884900C-0x08849B14
// @asm 0x08849C9C-0x0884A430
// @asm 0x0884A654-0x0884A91C
/* Text tags and line emission: $R ruby (furigana over the base text), $C color, $f/$F small
   capitals, word-break measurement, and font_layout_end which turns FontCtx.cmds into glyph
   quads. Also the sceFont allocator callbacks over FontCtx.scratch. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "font.h"

int wcschr_index(const unsigned short *s, unsigned short c);
unsigned short *wcsncpy(unsigned short *d, const unsigned short *s, unsigned int n);
unsigned short towupper(unsigned short c);
int wc_is_upper(unsigned short c);
int wc_is_lower(unsigned short c);
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
void font_glyph_vertices(short *v, int x, int y, int slot);
extern const unsigned char font_clut[32];           /* .rodata 0x08875740: 16-entry 16-bit palette of the 4bpp atlas */

#ifdef NON_MATCHING
/* Kept in asm: register allocation differs (sizes match within a few words). */
/* Next printable character after the tags at p; `in_ruby` skips a "(...)" ruby body. */
unsigned short wtext_next_char(FontCtx *f, unsigned short *p, int in_ruby)
{
    int c = 0;
    unsigned short *q;
    int k;

    while (*p != 0) {
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
                return c;
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
    return c;
}

/* Width in 26.6 of the word starting at p (up to the next breakable character), including the
   ruby padding, so that a word is not split across lines. */
int font_word_width(FontCtx *f, FontLayout *st, SceFontGlyphImage *img, unsigned short *p, int wrap)
{
    unsigned short pad = 0;
    unsigned short rpad, rw, rtotal, rlast;
    int in_ruby = (st->flags & 2) != 0;
    int in_word = 0;
    int w = 0;
    unsigned short c, t;
    unsigned short *q, *next;
    int slot;

    if (in_ruby)
        pad = st->ruby_pad;
    c = *p;
    q = p;
    for (;;) {
        if (c == 0)
            return w;
        next = q + 1;
        if (c < 0x20)
            return w;
        if (c == '$') {
            t = *next;
            if (t == 'R') {
                q += 2;
                if (f->flags & 0x8000) {
                    next = font_ruby_measure(f, q, &pad, &rpad, &rw, &rtotal);
                    if (wrap == 0) {
                        in_word = 0;
                        w += rw;
                        c = rtotal;
                        goto glyph;
                    }
                    w += pad >> 1;
                }
                in_ruby = 1;
            } else if (t == 'f') {
                q += 2;
            } else if (t == 'F') {
                q += 2;
            } else {
                if (t != 'C')
                    goto glyph;
                q += 3;
            }
        } else {
        glyph:
            if (in_ruby && c == '(') {
                slot = wcschr_index(next, ')');
                if (slot < 0)
                    q = next + 1;
                else
                    q = next + slot + 1;
                pad = 0;
                in_ruby = 0;
            } else {
                q = next;
                if (in_word) {
                    slot = font_glyph_cache(img, c);
                    w += f->slots[slot].advance * st->cur_size_x / 20 + pad;
                    if (wc_no_line_end(c) != 0) {
                        in_word = 0;
                        goto tail;
                    }
                } else if (wc_no_line_end(c) == 0) {
                    slot = font_glyph_cache(img, c);
                    if (p != next - 1)
                        w += f->slots[slot].advance * st->cur_size_x / 20;
                    in_word = 1;
                    w += pad;
                } else {
                tail:
                    t = wtext_next_char(f, next, in_ruby);
                    if (!(st->flags & 8))
                        return w;
                    if (wc_no_line_start(t) != 0)
                        return w;
                    slot = font_glyph_cache(img, t);
                    w += f->slots[slot].advance * st->cur_size_x / 20 + pad;
                }
            }
        }
        c = *q;
    }
}

/* Width in 26.6 of the run of non-breakable characters starting at p. */
int font_run_width(FontCtx *f, FontLayout *st, SceFontGlyphImage *img, unsigned short *p)
{
    unsigned short c = *p;
    char upper = st->force_upper;
    unsigned int sx = st->cur_size_x;
    unsigned short flags = st->flags;
    int w = 0;
    unsigned short *next;
    unsigned short t;
    int cls, slot;

    for (;;) {
        if (c == 0)
            return w;
        next = p + 1;
        if (c < 0x20)
            return w;
        if (c == '$') {
            t = *next;
            if (t == 'R') {
                next = p + 2;
            } else if (t == 'f') {
                upper = 0;
                next = p + 2;
            } else if (t == 'F') {
                upper = 1;
                next = p + 2;
            } else {
                if (t != 'C')
                    goto glyph;
                next = p + 3;
            }
        } else {
        glyph:
            if ((f->flags & 0x4000) || upper != 0) {
                if (wc_is_upper(c) == 0) {
                    c = towupper(c);
                    if (!(flags & 0x10)) {
                        flags |= 0x10;
                        sx = st->size_x * 10 / 12 & 0xFF;
                    }
                } else if (flags & 0x10) {
                    sx = st->size_x;
                    flags = 0;
                }
            }
            cls = wc_break_class(c);
            if (cls == 0)
                return w;
            if (cls == 1) {
                t = wtext_next_char(f, next, 0);
                if ((f->flags & 0x4000) || upper != 0) {
                    if (wc_is_upper(t) == 0) {
                        t = towupper(t);
                        if (!(flags & 0x10)) {
                            flags |= 0x10;
                            sx = st->size_x * 10 / 12 & 0xFF;
                        }
                    } else if (flags & 0x10) {
                        sx = st->size_x;
                        flags = 0;
                    }
                }
                cls = wc_break_class(t);
                if (cls < 1) {
                    slot = font_glyph_cache(img, c);
                    return w + f->slots[slot].advance * (int)sx / 20;
                }
            }
            slot = font_glyph_cache(img, c);
            w += f->slots[slot].advance * (int)sx / 20;
        }
        c = *next;
        p = next;
    }
}

/* Handles the character after '$'; fills *cmd for size/color changes. Returns the next position. */
unsigned short *font_parse_tag(FontCtx *f, FontLayout *st, FontCmd *cmd, unsigned short *p)
{
    unsigned short c = *p;
    unsigned short *next = p + 1;
    unsigned short dummy1, dummy2;
    unsigned int color;
    unsigned short n;

    if (c == 'R') {
        if (f->flags & 0x8000) {
            font_ruby_measure(f, next, &st->ruby_pad, &st->ruby_w, &dummy1, &dummy2);
            st->ruby_x = st->x + (st->ruby_w >> 1);
            st->ruby_y = st->y - 0x1C0;
        }
        st->flags |= 2;
    } else if (c == 'f') {
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
    } else if (c == 'F') {
        st->force_upper = 1;
    } else if (c == 'C') {
        n = *next;
        next = p + 2;
        if (f->flags & 2) {
            if (n < '0' || n > '9')
                n = n - 0x37;
            else
                n = n - '0';
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
    } else {
        next = p;
        if (c == '$')
            st->flags |= 0x20;
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
    if (n >= 0) {
        wcsncpy(base, p, n);
        if (n > 0)
            *last = base[n - 1];
        base[n] = 0;
        bw = font_measure_text(base);
        q = p + (n + 1);
        k = wcschr_index(q, ')');
        if (k > 0) {
            rw = k * 0x1C0;
            p = q + (k + 1);
            if (bw < rw) {
                *pad = (rw - bw) / n;
                *w = 0;
                *total = rw;
            } else {
                *pad = 0;
                *w = (bw - k * 0x1C0) / k;
                *total = bw;
            }
        }
    }
    return p;
}
#endif

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

/* Kept in asm: register allocation differs; font_layout_end is 1.2 KB. */
/* Shadow color for text color c: alpha halved for light colors, quartered for dark ones. */
unsigned int font_shadow_color(unsigned int c)
{
    unsigned int a = (c >> 24) & 0xFF;

    if (((c & 0xFF) * 299 + ((c & 0xFF00) >> 8) * 587 + ((c & 0xFF0000) >> 16) * 114) / 1000 < 0x80)
        return ((a << 6) / 255) << 24;
    return ((a << 7) / 255) << 24;
}
#ifdef NON_MATCHING

/* Emits the ruby characters of "(...)" at p as glyph commands above the base text. */
unsigned short *font_ruby_layout(FontCtx *f, FontLayout *st, unsigned short *p)
{
    unsigned short c = *p;
    unsigned int ch;
    FontCmd *o;

    for (;;) {
        ch = c;
        p++;
        if (ch == ')')
            break;
        if (ch < 0x20)
            break;
        if (st->ruby_xmax < st->ruby_x + 0x1C0) {
            if (!(st->flags & 8) || wc_no_line_start(ch) != 0) {
                st->ruby_y += st->line_h;
                st->ruby_x = st->line_x0 + (st->ruby_w >> 1);
            }
        }
        o = &f->cmds[f->ncmds];
        o->kind = 1;
        o->a = st->ruby_x;
        o->b = st->ruby_y;
        o->c = ch;
        st->ruby_x += st->ruby_w + 0x1C0;
        f->ncmds++;
        if (f->ncmds > 0xFF)
            break;
        c = *p;
    }
    st->ruby_x -= st->ruby_w >> 1;
    st->ruby_xmax = st->xmax;
    return p;
}

/* Same as font_ruby_layout without emitting commands (measurement). */
unsigned short *font_ruby_skip(FontCtx *f, FontLayout *st, unsigned short *p)
{
    unsigned short c = *p;

    for (;;) {
        p++;
        if (c == ')')
            break;
        if (c < 0x20)
            break;
        if (st->ruby_xmax < st->ruby_x + 0x1C0) {
            if (!(st->flags & 8) || wc_no_line_start(c) != 0) {
                st->ruby_y += st->line_h;
                st->ruby_x = st->line_x0 + (st->ruby_w >> 1);
            }
        }
        st->ruby_x += st->ruby_w + 0x1C0;
        c = *p;
    }
    st->ruby_x -= st->ruby_w >> 1;
    st->ruby_xmax = st->xmax;
    return p;
}

/* Draws the commands of the current line: shadow pass (flag 4) then the main pass; ruby glyphs
   come from rubi_font00.gim. Resets the command count. */
void font_layout_end(FontCtx *f, FontLayout *st)
{
    unsigned char sx = f->size_x;
    unsigned char sy = f->size_y;
    short *verts;
    int n = 0;
    int i;
    int in_ruby;
    unsigned int color;
    FontCmd *c;

    verts = font_flush_quads(&f->dl, 0, 0);
    if (f->flags & 4) {
        in_ruby = 0;
        f->dl = dl_ambient_color(f->dl, st->color2);
        for (i = 0, c = f->cmds; i < f->ncmds; i++, c++) {
            if (c->kind == 2) {
                f->size_x = c->a;
                f->size_y = c->b;
            } else if (c->kind == 5) {
                in_ruby = 0;
            } else if (c->kind == 4) {
                in_ruby = 1;
            } else if (c->kind == 3) {
                if (!in_ruby) {
                    if (n > 0) {
                        verts = font_flush_quads(&f->dl, verts, n);
                        n = 0;
                    }
                    f->dl = dl_ambient_color(f->dl, c->b);
                }
            } else if (c->kind == 1 && !in_ruby) {
                font_glyph_vertices(verts + n * 16, c->a, c->b + 0x40, c->c);
                font_glyph_vertices(verts + (n + 1) * 16, c->a + 0x40, c->b + 0x40, c->c);
                n += 2;
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
    for (i = 0, c = f->cmds; i < f->ncmds; i++, c++) {
        if (c->kind == 2) {
            f->size_x = c->a;
            f->size_y = c->b;
        } else if (c->kind == 5) {
            in_ruby = 0;
            f->dl = font_dl_glyph_header(f->dl, color);
            verts = font_flush_quads(&f->dl, 0, 0);
        } else if (c->kind == 4) {
            in_ruby = 1;
            if (n > 0) {
                verts = font_flush_quads(&f->dl, verts, n);
                n = 0;
            }
            if (f->rubi_gim == 0)
                f->rubi_gim = ehp_find(syspack_get(), "rubi_font00.gim", 0);
            f->dl = dl_push1(f->dl, 0x13000000);
            f->dl = gim_set_texture(f->dl, f->rubi_gim);
            f->dl = dl_set_tex_filter(f->dl, 0, 0);
        } else if (c->kind == 3) {
            if (n > 0) {
                verts = font_flush_quads(&f->dl, verts, n);
                n = 0;
            }
            f->dl = dl_ambient_color(f->dl, c->a);
            color = c->a;
        } else if (c->kind == 1) {
            if (in_ruby) {
                font_rubi_quad(c->a, c->b, c->c);
            } else {
                font_glyph_vertices(verts + n * 16, c->a, c->b, c->c);
                n++;
            }
        }
    }
    if (n > 0)
        font_flush_quads(&f->dl, verts, n);
    f->ncmds = 0;
}
#endif

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

/* Kept in asm: register allocation differs. The "rubi_font00.gim" literal, the ruby sample strings
   and the glyph header template live in the asm .data (0x088EE9E0..0x088EEA58). */
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
#ifdef NON_MATCHING

/* Copies src to dst dropping "$R" tags and the "(...)" ruby bodies (keeps the base text). */
void wtext_strip_tags(unsigned short *dst, unsigned short *src)
{
    unsigned short c = *src;
    int in_ruby = 0;
    unsigned short *next;
    int k;

    for (;;) {
        if (c == 0) {
            *dst = 0;
            return;
        }
        next = src + 1;
        if (c == '$') {
            c = *next;
            next = src + 2;
            if (c != 'R')
                goto copy;
            in_ruby = 1;
        } else {
        copy:
            if (in_ruby) {
                if (c == '(') {
                    k = wcschr_index(next, ')');
                    next = next + k + 1;
                    in_ruby = 0;
                    goto skip;
                }
                *dst = c;
            } else {
                *dst = c;
            }
            dst++;
        }
    skip:
        c = *next;
        src = next;
    }
}

/* Copies src to dst keeping only the ruby text of "$Rbase(ruby)" groups. */
void wtext_strip_tags2(unsigned short *dst, unsigned short *src)
{
    unsigned short c = *src;
    int in_ruby = 0;
    unsigned short *next;
    int k;

    for (;;) {
        if (c == 0) {
            *dst = 0;
            return;
        }
        next = src + 1;
        if (c == '$') {
            c = *next;
            next = src + 2;
            if (c != 'R')
                goto copy;
            k = wcschr_index(next, '(');
            next = next + k + 1;
            in_ruby = 1;
        } else {
        copy:
            if (in_ruby) {
                if (c == ')') {
                    in_ruby = 0;
                    goto skip;
                }
                *dst = c;
            } else {
                *dst = c;
            }
            dst++;
        }
    skip:
        c = *next;
        src = next;
    }
}

/* Number of visible characters in src (tags and ruby bodies excluded). */
int wtext_count_tags(unsigned short *src)
{
    unsigned short c = *src;
    int n = 0;
    int in_ruby = 0;
    unsigned short *next;
    int k;

    for (;;) {
        if (c == 0)
            return n;
        next = src + 1;
        if (c == '$') {
            c = *next;
            next = src + 2;
            if (c == 'f') {
                next = src + 3;
            } else if (c == 'F') {
                next = src + 3;
            } else if (c == 'C') {
                next = src + 3;
            } else {
                if (c != 'R')
                    goto count;
                in_ruby = 1;
            }
        } else {
        count:
            if (in_ruby) {
                if (c == '(') {
                    k = wcschr_index(next, ')');
                    next = next + k + 1;
                    in_ruby = 0;
                } else {
                    n++;
                }
            } else {
                n++;
            }
        }
        c = *next;
        src = next;
    }
}

/* Width and height (26.6) of the language's sample glyph: width << 16 | height. */
unsigned int font_sample_glyph_size(void)
{
    unsigned short samples[6][2] = { { 0x3042, 0 }, { 'A', 0 }, { 'A', 0 }, { 'A', 0 }, { 'A', 0 }, { 'A', 0 } };
    int w;

    w = font_measure_text(samples[cfg_get_language()]);
    return (unsigned short)w << 16 | (unsigned short)font_line_height();
}
#endif

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
