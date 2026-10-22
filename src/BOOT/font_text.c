// @range 0x08847008-0x08848C68
// @asm 0x08847768-0x08847FC8
// @asm 0x0884876C-0x08848C68
/* UCS-2 text layout and drawing. A FontLayout carries the pen; font_layout_wtext consumes one
   line into FontCtx.cmds (glyphs, size and color changes, ruby) and font_layout_end draws it. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "font.h"

int wcslen(const unsigned short *s);
int wcschr_index(const unsigned short *s, unsigned short c);
unsigned short towupper(unsigned short c);
int wc_is_lower(unsigned short c);

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

#ifdef NON_MATCHING /* forma do original, mas registradores/pilha diferentes (fica em asm; este C é a versão do port) */
/* Lays out one line of UCS-2 text into f->cmds. Tags: $R...(ruby) $f $F (lower/upper small caps)
   $Cn (color) $$ (literal). Returns 0 at end of text, 1 at a line break (when `wrap`),
   2 when max_glyphs is reached, -1 when the command buffer is full. */
int font_layout_wtext(FontCtx *f, FontLayout *st, unsigned short *text, int wrap)
{
    SceFontGlyphImage img;
    FontCmd cmd;
    unsigned short *p = text;
    unsigned short *next;
    unsigned short c;
    int r = 0;
    int pending_break = 0;
    int slot, adv, extra, pad, w;
    int yoff;
    int cls;
    unsigned int sx, sy;
    FontCmd *o;

    img.buf_w = 32;
    img.buf_h = 32;
    img.bytes_per_line = 16;
    img.pixel_format = 0;
    img.pad = 0;
    c = *p;
    for (;;) {
        next = p + 1;
        if (c == 0)
            break;
        pad = 0;
        if (!(st->flags & 0x20) && c == '$') {
            memset(&cmd, 0, sizeof cmd);
            next = font_parse_tag(f, st, &cmd, next);
            if (cmd.kind != 0) {
                memcpy(&f->cmds[f->ncmds], &cmd, sizeof cmd);
                f->ncmds++;
                if (f->ncmds > 0xFF) {
                    r = -1;
                    break;
                }
            }
            p = next;
            c = *p;
            continue;
        }
        if ((f->flags & 0x8000) && (st->flags & 2))
            pad = st->ruby_pad >> 1;
        if ((st->flags & 2) && c == '(') {
            if (f->flags & 0x8000) {
                if (f->ncmds > 0xE5) {
                    r = -1;
                    next = p;
                    break;
                }
                f->cmds[f->ncmds].kind = 4;
                f->ncmds++;
                next = font_ruby_layout(f, st, next);
                f->cmds[f->ncmds].kind = 5;
                f->ncmds++;
                if (st->ruby_y + 0x1C0 == st->y) {
                    if (st->x < st->ruby_x)
                        st->x = st->ruby_x;
                }
            } else {
                w = wcschr_index(next, ')');
                if (w < 0)
                    next = p + 2;
                else
                    next = next + w + 1;
            }
            st->flags &= ~2;
            p = next;
            c = *p;
            continue;
        }
        if ((f->flags & 0x4000) || st->force_upper != 0) {
            if (wc_is_lower(c) == 0) {
                if (st->flags & 0x10) {
                    o = &f->cmds[f->ncmds];
                    o->kind = 2;
                    o->a = st->size_x;
                    o->b = st->size_y;
                    f->ncmds++;
                    st->flags &= ~0x10;
                    st->cur_size_x = st->size_x;
                    st->cur_size_y = st->size_y;
                    st->yoff = 0;
                }
            } else {
                c = towupper(c);
                if (!(st->flags & 0x10)) {
                    sx = st->size_x * 10 / 12;
                    sy = st->size_y * 10 / 12;
                    o = &f->cmds[f->ncmds];
                    o->kind = 2;
                    o->a = sx & 0xFF;
                    o->b = sy & 0xFF;
                    f->ncmds++;
                    st->flags |= 0x10;
                    st->cur_size_x = sx;
                    st->cur_size_y = sy;
                    st->yoff = f->line_h * (f->size_y - (sy & 0xFF)) / 20 - 0x20;
                }
            }
        }
        if (c < 0x20) {
            if (c == '\n') {
                if (wrap == 0)
                    goto done;
                r = 1;
                break;
            }
            if (c == '\t')
                st->x += f->max_glyph_w * 4;
        } else {
            slot = font_glyph_cache(&img, c);
            yoff = st->yoff;
            extra = 0;
            st->x += pad;
            adv = f->slots[slot].advance * st->cur_size_x / 20;
            if (f->flags & 1)
                extra = font_word_width(f, st, &img, p, wrap);
            if (f->flags & 8) {
                cls = wc_break_class(c);
                if (pending_break) {
                    extra = font_run_width(f, st, &img, next);
                    pending_break = 0;
                } else if (cls == 1) {
                    cls = wc_break_class(wtext_next_char(f, next, 0));
                    if (cls < 1)
                        pending_break = 1;
                    else
                        extra = font_run_width(f, st, &img, next);
                } else if (cls == 0 || cls == 2) {
                    extra = font_run_width(f, st, &img, next);
                }
            }
            if (st->xmax < extra + adv + st->x + pad) {
                next = p;
                if (wrap != 0)
                    r = 1;
                break;
            }
            if (st->max_glyphs >= 0 && st->max_glyphs <= st->nglyphs) {
                r = 2;
                break;
            }
            o = &f->cmds[f->ncmds];
            o->kind = 1;
            o->a = st->x;
            o->b = st->y + yoff;
            o->c = slot;
            if (st->flags & 0x20)
                st->flags &= ~0x20;
            st->nglyphs++;
            st->x += adv + pad;
            f->ncmds++;
            if (f->ncmds > 0xFF) {
                r = -1;
                break;
            }
        }
        p = next;
        c = *p;
    }
    if ((st->flags & 2) && r >= 0)
        st->ruby_xmax = st->x;
done:
    st->consumed = (next - text);
    return r;
}

#endif

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

#ifdef NON_MATCHING /* forma do original, mas registradores/pilha diferentes (fica em asm; este C é a versão do port) */
/* Measures one line of text with the same rules as font_layout_wtext but emits no commands.
   Returns the pen x after the text (26.6). */
int font_layout_text(FontCtx *f, FontLayout *st, unsigned short *text, int wrap)
{
    SceFontGlyphImage img;
    FontCmd cmd;
    unsigned short *p, *next, *end;
    unsigned short c;
    int pending_break = 0;
    int slot, adv, extra, pad, w;
    int cls;
    unsigned int sx, sy;

    img.buf_w = 32;
    img.buf_h = 32;
    img.bytes_per_line = 16;
    img.pixel_format = 0;
    img.pad = 0;
    if (text == 0)
        return 0;
    c = *text;
    p = text;
    for (;;) {
        next = p + 1;
        end = next;
        if (c == 0)
            break;
        pad = 0;
        if (!(st->flags & 0x20) && c == '$') {
            next = font_parse_tag(f, st, &cmd, next);
        } else {
            if ((f->flags & 0x8000) && (st->flags & 2))
                pad = st->ruby_pad >> 1;
            if ((st->flags & 2) && c == '(') {
                if (f->flags & 0x8000) {
                    next = font_ruby_skip(f, st, next);
                    if (st->ruby_y + 0x1C0 == st->y) {
                        if (st->x < st->ruby_x)
                            st->x = st->ruby_x;
                    }
                } else {
                    w = wcschr_index(next, ')');
                    if (w < 0)
                        next = p + 2;
                    else
                        next = next + w + 1;
                }
                st->flags &= ~2;
            } else {
                if ((f->flags & 0x4000) || st->force_upper != 0) {
                    if (wc_is_lower(c) == 0) {
                        if (st->flags & 0x10) {
                            st->flags &= ~0x10;
                            st->cur_size_x = st->size_x;
                            st->cur_size_y = st->size_y;
                            st->yoff = 0;
                        }
                    } else {
                        c = towupper(c);
                        if (!(st->flags & 0x10)) {
                            st->flags |= 0x10;
                            st->cur_size_x = st->size_x * 10 / 12;
                            sy = st->size_y * 10 / 12;
                            st->cur_size_y = sy;
                            st->yoff = f->line_h * (f->size_y - (sy & 0xFF)) / 20 - 0x20;
                        }
                    }
                }
                if (c < 0x20) {
                    if (c == '\n')
                        goto done;
                    if (c == '\t')
                        st->x += f->max_glyph_w * 4;
                } else {
                    slot = font_glyph_cache(&img, c);
                    extra = 0;
                    st->x += pad;
                    adv = f->slots[slot].advance * st->cur_size_x / 20;
                    if (f->flags & 1)
                        extra = font_word_width(f, st, &img, p, wrap);
                    if (f->flags & 8) {
                        cls = wc_break_class(c);
                        if (pending_break) {
                            extra = font_run_width(f, st, &img, next);
                            pending_break = 0;
                        } else if (cls == 1) {
                            cls = wc_break_class(wtext_next_char(f, next, 0));
                            if (cls < 1)
                                pending_break = 1;
                            else
                                extra = font_run_width(f, st, &img, next);
                        } else if (cls == 0 || cls == 2) {
                            extra = font_run_width(f, st, &img, next);
                        }
                    }
                    end = p;
                    if (st->xmax < extra + adv + st->x + pad)
                        break;
                    st->x = st->x + adv + pad;
                    if (st->flags & 0x20)
                        st->flags &= ~0x20;
                }
            }
        }
        p = next;
        c = *p;
    }
    if (st->flags & 2)
        st->ruby_xmax = st->x;
done:
    st->consumed = (end - text);
    return st->x;
}
#endif
