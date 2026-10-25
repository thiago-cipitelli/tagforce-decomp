// @range 0x08808D48-0x08809DC4
// @asm 0x08808F00-0x088094DC
// @asm 0x088095F8-0x08809C4C
// @data 0x0882E404-0x0882E414
/* The title's panel menus (TMenu): a column of panels from the title sprites that fade in one after
   the other, a cursor moved with the pad (skipping disabled items), accept/cancel with a blink, and the
   per-item label callback; plus the centered system window and the GE state they draw with. */
#include "libtitle.h"

int tmenu_fade_step(TMenu *m, unsigned int color);
int tmenu_anim_instant(TMenu *m);

void *twin_draw_center(void *list, MenuWidget *w, int center)
{
    MenuItemDef *d = &w->def;

    if (center) {
        d->x = (480 - d->w) / 2;
        d->y = (272 - d->h) / 2;
    }
    menu_draw(&list, w);
    return list;
}

void tmenu_init(TMenu *m)
{
    int i;

    m->state = 0;
    m->b07 = 0;
    m->n = 8;
    m->done_state = 4;
    m->open_anim = 0;
    m->ok_anim = 0;
    m->cancel_anim = 0;
    m->anim = m->open_anim;
    m->open_anim = 1;
    m->open_anim = 2;
    tmenu_set_fade(m, 4, 1);
    for (i = 0; i < 10; i++)
        m->item[i] = 0;
    m->x = 128;
    m->y = 40;
    m->w = 240;
    m->h = 32;
    m->style = 1;
    m->style_mark = 5;
    m->cursor = 0;
    m->style_sel = 0;
    m->flags = 0xFF;
    m->hilite = 0;
    m->hilite_idx = 0;
    m->style_hilite = 4;
    m->b56 = 1;
    m->flags = 0xFF;
    m->style_sel = 0;
    m->color = 0xFFFFFFFF;
    m->text_color = 0xFFFFFFFF;
    m->sel_color = 0xFF000000;
    m->off_color = 0xFF808080;
    m->off_text_color = 0xFF808080;
    m->u58 = 0xFF;
    m->cursor = 0;
    m->blink = 0;
    m->mode = 0;
    m->draw = 0;
}

#ifdef NON_MATCHING /* forma quase igual; blocos de retorno e registradores dos laços de busca diferem */
/* mode 1 = takes input, 2 = only animates; returns the accepted item, -2 on cancel, -1 while running,
   -3 when idle or finished */
int tmenu_update(TMenu *m, int mode)
{
    unsigned char last = 0;
    PadState *pad;
    unsigned int trig;
    unsigned int dir;
    int i;
    int k;
    int found;
    int r;

    if (mode == 2 && m->state > 2)
        m->state = 2;
    switch (m->state) {
    case 0:
        m->state++;
    case 1:
        switch (m->open_anim) {
        case 0:
            m->state++;
            break;
        case 1:
            if (tmenu_anim_instant(m))
                m->state++;
            break;
        case 2:
            if (tmenu_fade_step(m, m->color))
                m->state++;
            break;
        case 3:
            r = -1;
            goto done;
        }
        r = -1;
        break;
    case 2:
        if (mode != 1)
            return -3;
        m->state++;
        k = 0;
        found = 0;
        if (m->n != 0)
            last = (m->n & 0xFF) - 1;
        for (i = m->cursor; i <= last; i++, k++) {
            if (m->item[i] != 1) {
                found = 1;
                break;
            }
        }
        if (found)
            m->cursor += (unsigned char)k;
    case 3:
        pad = pad_get();
        trig = pad->b.trig;
        dir = (m->flags & 2) ? pad->b.rep : trig;
        if (m->n != 0)
            last = (m->n & 0xFF) - 1;
        if (pad->b.trig & region_pick_value(0)) {
            if (m->item[m->cursor] != 1) {
                se_play(0x6213);
                m->mode = 1;
                m->anim = m->ok_anim;
                switch (m->anim) {
                case 0:
                    m->state = m->done_state;
                    return m->cursor;
                case 1:
                    m->state = 5;
                    m->next = 4;
                    break;
                case 3:
                    m->state = 5;
                    m->next = 4;
                    break;
                }
            } else {
                se_play(0x6215);
            }
        } else if (pad->b.trig & region_pick_value(1)) {
            m->mode = 2;
            se_play(0x6214);
            m->state = 5;
            m->next = 6;
            m->anim = m->cancel_anim;
        } else if ((trig & 0x10) || (dir & 0x10)) {
            if (m->cursor > 0) {
                k = 1;
                found = 0;
                for (i = m->cursor - 1; i >= 0; i--, k++) {
                    if (m->item[i] != 1) {
                        found = 1;
                        break;
                    }
                }
                if (found) {
                    m->cursor -= (unsigned char)k;
                    se_play(0x6212);
                } else {
                    se_play(0x6215);
                }
            } else if (m->flags & 1) {
                if (trig & 0x10) {
                    found = 0;
                    if (m->item[last] != 1)
                        found = 1;
                    if (found) {
                        m->cursor = last;
                        se_play(0x6212);
                    } else {
                        se_play(0x6215);
                    }
                }
            } else {
                m->cursor = 0;
                se_play(0x6212);
            }
        } else if ((trig & 0x40) || (dir & 0x40)) {
            if (m->cursor < last) {
                k = 1;
                found = 0;
                for (i = m->cursor + 1; i <= last; i++, k++) {
                    if (m->item[i] != 1) {
                        found = 1;
                        break;
                    }
                }
                if (found) {
                    m->cursor += (unsigned char)k;
                    se_play(0x6212);
                } else {
                    se_play(0x6215);
                }
            } else if (m->flags & 1) {
                if (trig & 0x40) {
                    found = 0;
                    if (m->item[0] != 1)
                        found = 1;
                    if (found) {
                        m->cursor = 0;
                        se_play(0x6212);
                    } else {
                        se_play(0x6215);
                    }
                }
            } else {
                m->cursor = last;
                se_play(0x6212);
            }
        }
        r = -1;
        break;
    case 4:
        switch (m->anim) {
        case 0:
            m->state = 7;
            return m->cursor;
        case 1:
            if (tmenu_anim_instant(m)) {
                m->state = 7;
                return m->cursor;
            }
            break;
        case 3:
            m->state = 7;
            break;
        }
        r = -1;
        break;
    case 5:
        switch (m->anim) {
        case 0:
            m->state = m->next;
            break;
        case 1:
            if (tmenu_anim_instant(m))
                m->state = m->next;
            break;
        case 3:
            r = -1;
            goto done;
        }
        r = -1;
        break;
    case 6:
        m->state = 7;
        r = -2;
        break;
    case 7:
        m->state = 0;
        r = -3;
        break;
    default:
        r = -3;
        break;
    }
done:
    return r;
}
#endif

/* goes back from the accepted state to input */
void tmenu_back(TMenu *m)
{
    switch (m->state) {
    case 4:
        m->state = 3;
        break;
    }
}

void tmenu_set_fade(TMenu *m, short len, short step)
{
    m->fade_len = len;
    m->fade_step = step;
    m->fade_t = 0;
    m->fade_end = m->fade_len + m->fade_step * m->n;
    memset(m->fade_color, 0, sizeof(m->fade_color));
}

/* one frame of the staggered fade-in; 1 when done */
int tmenu_fade_step(TMenu *m, unsigned int color)
{
    unsigned int c = color & 0xFFFFFF;
    int step = m->fade_step;
    int i;
    int a;
    int d;

    if (m->fade_t < m->fade_end) {
        i = 0;
        if (m->n > 0) {
            d = 0;
            do {
                a = m->fade_t - d;
                if (m->fade_len <= a)
                    a = m->fade_len;
                else if (a <= 0)
                    a = 0;
                m->fade_color[i] = c | a * 255 / m->fade_len << 24;
                i++;
                d += step;
            } while (i < m->n);
        }
        m->fade_t++;
        return 0;
    }
    m->fade_t = 0;
    m->fade_end = 0;
    return 1;
}

int tmenu_anim_instant(TMenu *m)
{
    return 1;
}

/* template of the panel sprites (left, middle, right) */
int tmenu_draw_ids[4] = { 10, 11, 12 };

#ifdef NON_MATCHING /* registradores e spills do laço de três sprites diferem */
void *tmenu_draw(void *list, TMenu *m)
{
    int *ids = tmenu_draw_ids;
    unsigned char style = m->style;
    unsigned char sel = m->style_sel;
    unsigned char hil = m->style_hilite;
    int n = m->n;
    short su = style % 6 * 20;
    short sv = style / 6 * 24;
    short selu = sel % 6 * 20;
    short selv = sel / 6 * 24;
    short hu = hil % 6 * 20;
    short hv = hil / 6 * 24;
    TSprite s;
    int i;
    int j;
    int k;
    int on;
    short x;
    short y;
    short w;
    short sx;
    short sw;
    short u;
    short v;
    unsigned int a;

    if (m->state < 3) {
        m->blink = 0;
    } else {
        m->blink = m->blink + 1;
        if (m->blink >= 60)
            m->blink = 0;
    }
    for (i = 0; i < n; i++) {
        x = m->x;
        y = m->y + m->h * i;
        list = tmenu_gfx_setup(list, 0);
        if (m->open_anim == 2 && m->state < 2)
            list = dl_color(list, m->fade_color[i]);
        else if (m->item[i] == 1)
            list = dl_color(list, m->off_color);
        else
            list = dl_color(list, m->color);
        list = gim_set_texture(list, ehp_find(title_pack(), title_sprite_name(ids[0]), 0));
        if (m->state < 3)
            on = 0;
        else if (i == m->cursor)
            on = 1;
        else
            on = 0;
        for (j = 0; j <= on; j++) {
            if (j != 0) {
                if (m->mode == 0) {
                    a = m->blink;
                    if (a < 20)
                        a = (unsigned char)(a * 48 / 20 + 207);
                    else if (a < 40)
                        a = 255;
                    else
                        a = (unsigned char)(255 - (a - 40) * 48 / 20);
                } else if (m->mode == 1) {
                    a = m->blink % 6 < 3 ? 255 : 64;
                } else {
                    a = 0;
                }
                a = (unsigned char)a;
                list = dl_color(list, (a | (a << 16 | 0xFF000000) | a << 8) & (a << 24 | 0xFFFFFF));
            }
            for (k = 0; k < 3; k++) {
                s = title_sprites[ids[k]];
                w = m->w;
                if (j != 0) {
                    u = s.u + selu;
                    v = s.v + selv;
                } else if (m->hilite && i == m->hilite_idx) {
                    u = s.u + hu;
                    v = s.v + hv;
                } else if (m->item[i] == 2) {
                    u = s.u + (short)(m->style_mark % 6 * 20);
                    v = s.v + (short)(m->style_mark / 6 * 24);
                } else {
                    u = s.u + su;
                    v = s.v + sv;
                }
                if (w < 9)
                    w = 9;
                sx = s.x;
                sw = s.w;
                if (k == 2)
                    sx = w - 3;
                else if (k == 1)
                    sw = w - 9;
                list = draw_set_offsets(list, s.tw, s.th, sw, s.h);
                list = draw_sprite(list, (short)(sx + x), (short)(s.y + y), 0, u, v, s.a, s.b);
            }
        }
        if (m->draw)
            list = m->draw(list, i, x, y, m->w, m->h);
    }
    return list;
}
#endif

unsigned int tmenu_text_color(TMenu *m, int i0)
{
    unsigned short i = i0;

    if (m->item[i] == 1)
        return m->off_text_color;
    if (m->state >= 3 && i == m->cursor && m->mode < 2)
        return m->sel_color;
    return m->text_color;
}

/* GE state for the 2D panels (mode 2: untextured shadow) */
void *tmenu_gfx_setup(void *list, int mode)
{
    list = dl_clear_color(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear_depth(list, 0);
    if (mode == 2) {
        list = dl_disable(list, 9);
        list = dl_disable(list, 1);
        list = dl_tex_func(list, 4, 0);
        list = dl_ambient_color(list, 0x60000000);
    } else {
        list = dl_enable(list, 9);
        list = dl_enable(list, 1);
        list = dl_tex_func(list, 0, 1);
        list = dl_ambient_color(list, 0xFFFFFFFF);
    }
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_disable(list, 1);
    list = dl_disable(list, 10);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    return dl_color(list, 0xFFFFFFFF);
}
