// @range 0x08848C68-0x0884900C
/* Font layer state accessors: layout flags, extra line spacing, and the display list used
   while a menu item's text is being built. */
#include "psp.h"
#include "ehsys.h"
#include "font.h"

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
