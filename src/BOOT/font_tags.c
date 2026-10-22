// @range 0x0884900C-0x0884A9B4
// @data 0x088EE9E0-0x088EEA58
// @asm 0x088490EC-0x08849778
// @asm 0x08849C9C-0x08849E54
// @asm 0x08849F44-0x0884A430
/* Text tags and line emission: $R ruby (furigana over the base text), $C color, $f/$F small
   capitals, word-break measurement, and font_layout_end which turns FontCtx.cmds into glyph
   quads. Also the sceFont allocator callbacks over FontCtx.scratch. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "font.h"

extern char font_rubi_gim_name[16];

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

#ifdef NON_MATCHING /* font_word_width e font_run_width: o original usa s7/s8 para cópias mascaradas de c e t e por isso guarda w na pilha; com c unsigned short ou int e o teste de maiúscula em if/else invertido fica em ~150 palavras */
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
    unsigned short c;
    unsigned char upper = st->force_upper;
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
            if ((f->flags & 0x4000) || upper != 0) {
                if (wc_is_upper(c) == 0) {
                    c = towupper(c);
                    if (!(flags & 0x10)) {
                        flags |= 0x10;
                        sx = st->size_x * 10 / 12;
                    }
                } else if (flags & 0x10) {
                    sx = st->size_x;
                    flags &= ~0x10;
                }
            }
            cls = wc_break_class(c);
            if (cls == 0)
                return w;
            if (cls == 1) {
                t = wtext_next_char(f, p, 0);
                if ((f->flags & 0x4000) || upper != 0) {
                    if (wc_is_upper(t) == 0) {
                        t = towupper(t);
                        if (!(flags & 0x10)) {
                            flags |= 0x10;
                            sx = st->size_x * 10 / 12;
                        }
                    } else if (flags & 0x10) {
                        sx = st->size_x;
                        flags &= ~0x10;
                    }
                }
                if (wc_break_class(t) <= 0) {
                    slot = font_glyph_cache(img, c);
                    w += f->slots[slot].advance * sx / 20;
                    return w;
                }
            }
            slot = font_glyph_cache(img, c);
            w += f->slots[slot].advance * sx / 20;
        } else {
            return w;
        }
    }
    return w;
}

#endif
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

/* Kept in asm: register allocation differs; font_layout_end is 1.2 KB. */
/* Shadow color for text color c: alpha halved for light colors, quartered for dark ones. */
unsigned int font_shadow_color(unsigned int c)
{
    unsigned int a = (c >> 24) & 0xFF;

    if (((c & 0xFF) * 299 + ((c & 0xFF00) >> 8) * 587 + ((c & 0xFF0000) >> 16) * 114) / 1000 < 0x80)
        return ((a << 6) / 255) << 24;
    return ((a << 7) / 255) << 24;
}
/* Emits the ruby characters of "(...)" at p as glyph commands above the base text. */
#ifdef NON_MATCHING /* 3 palavras: o original carrega c direto em s0 e passa o valor cru a wc_no_line_start (`move a0,s0` no atraso), refazendo a máscara no teste (v0); com c int o mwcc reaproveita a máscara do teste (a0) na chamada; com c unsigned short ele mascara na cópia para s0 (48 palavras) */
unsigned short *font_ruby_layout(FontCtx *f, FontLayout *st, unsigned short *p)
{
    int c;

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

#endif
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
#ifdef NON_MATCHING

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
                f->rubi_gim = ehp_find(syspack_get(), font_rubi_gim_name, 0);
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

/* Literal of font_layout_end (kept in asm), defined here so that it sits between the template of
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
