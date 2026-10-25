// @range 0x0880EB20-0x0880F438
// @rodata 0x0882ADC8-0x0882AE08
/* Language selection of the options menu (state 19): a panel list of the five languages with the
   flag sprites; a new choice reloads the title pack and the card database. */
#include "libtitle.h"

void *tlang_label(void *list, int i0, int x, int y, int w);
void *tlang_flag(void *list, int cur);
void *tlang_marker(void *list, int i, int x, int y);

void tlang_enter(void)
{
    Title *const t = &title;
    TMenu *const m = &title.tm[0];

    title_goto_fade(0, 0xFFF, 0xFFE);
    tmenu_init(m);
    m->done_state = 3;
    m->n = 5;
    m->anim = 0;
    m->open_anim = 0;
    m->ok_anim = 0;
    m->cancel_anim = 0;
    m->b07 = 0;
    tmenu_set_fade(m, 4, 1);
    m->x = 140;
    m->y = 128;
    m->w = 200;
    m->h = 26;
    m->style = 7;
    m->style_hilite = 1;
    m->hilite = 1;
    m->style_sel = 0;
    if (title.lang != 0) {
        m->hilite_idx = (t->cfg_lang = t->lang) - 1;
        m->cursor = m->hilite_idx;
    } else {
        t->cfg_lang = 0;
        m->hilite_idx = 0;
    }
    m->b56 = 1;
    m->flags = 0xFF;
    m->draw = (TMenuDrawFn)tlang_label;
    m->style_sel = 0;
    title_ehf_load(5, 2, 0, 480);
    title_ehf_load(6, 2, 0, 480);
    title_ehf_load(13, 2, 0, 16);
}

void tlang_update(void)
{
    TMenu *const m = &title.tm[0];
    Title *const t = &title;
    int open = 1;
    unsigned int trig;
    int f;
    int r;

    if (m->mode == 1) {
        if (m->blink > 30) {
            m->mode = 0;
            m->blink = 0;
        } else if (t->sub == 2) {
            trig = pad_get()->b.trig;
            if ((trig & 0x10) || (trig & 0x40) || (trig & region_pick_value(open))) {
                m->mode = 0;
                m->blink = 0;
            }
        }
    }
    switch (t->sub) {
    case 0:
    case 4:
    case 6:
        break;
    case 1:
        f = title_ehf_frame(13);
        switch (f) {
        case 16:
            title_ehf_anim(13, 2, 17, 96);
            t->sub++;
            break;
        default:
            if (f < 16)
                open = 0;
            break;
        }
    case 2:
        r = tmenu_update(m, open);
        switch (r) {
        case -2:
            title_goto_fade(1, 0xFFF, 5);
            t->sub = 4;
            break;
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            m->blink = 0;
            t->cfg_lang = r + 1;
            m->hilite_idx = r;
            t->sub = 3;
            break;
        case -3:
        case -1:
            break;
        }
        break;
    case 3:
        if (m->mode == 0) {
            title_goto_fade(1, 0xFFF, 5);
            t->sub = 4;
        }
        break;
    case 5:
        if (t->lang != t->cfg_lang) {
            title_lang_load(1);
            card_db_request_load();
            t->sub = 6;
            gfx_overlay_enable(1);
        } else {
            title_set_state(9, 10);
        }
        break;
    case 7:
        if (card_db_load()) {
            gfx_overlay_enable(0);
            title_set_state(9, 10);
        }
        break;
    }
    t->prev_sub = t->sub;
}

void *tlang_draw(void *list)
{
    Title *const t = &title;
    TMenu *const m = &title.tm[0];

    if (title.sub < 5) {
        list = title_ehf_draw(list, 6);
        list = title_ehf_draw(list, 5);
        if (t->sub > 0) {
            list = title_ehf_draw(list, 13);
            list = title_screen_label(list);
        }
        list = tlang_flag(list, m->cursor);
        list = tmenu_draw(list, &t->tm[0]);
    }
    return list;
}

void *tlang_label(void *list, int i0, int x, int y, int w)
{
    char buf[0x200];
    TMenu *const m = &title.tm[0];
    unsigned short i = i0;
    int ty = y + 4;
    int id = i + 54;
    unsigned short *text;
    int tx;
    int tw;
    unsigned int c;

    memset(buf, 0, sizeof(buf));
    switch (i) {
    case 0:
        id = 54;
        break;
    case 1:
        id = 56;
        break;
    case 2:
        id = 55;
        break;
    case 3:
        id = 57;
        break;
    case 4:
        id = 58;
        break;
    }
    c = tmenu_text_color(m, i0);
    font_set_size(15, 15);
    font_set_color(c);
    font_set_flag4(1);
    title_msg_center(x, w, 0, id, &text, &tx, &tw);
    list = title_text_left(list, tx, ty, tw, text);
    return tlang_marker(list, i, x + 16, y + 4);
}

/* the flag of the language chosen (the cursor argument is not used) */
void *tlang_flag(void *list, int unused)
{
    short tw = title_sprites[35].tw;
    short th = title_sprites[35].th;
    short u = title_sprites[35].u;
    short v = title_sprites[35].v;
    unsigned int region = region_get();
    int k = title.tm[0].hilite_idx;

    switch (k) {
    case 0:
        if (region < 2) {
            u += k % 3 * tw;
            break;
        }
    case 1:
        u += (k + 1) % 3 * tw;
        break;
    case 2:
    case 3:
    case 4:
        u += (k - 2) % 3 * tw;
        v += th;
        break;
    }
    list = tmenu_gfx_setup(list, 1);
    list = gim_set_texture(list, ehp_find(title_pack(), title_sprite_name(35), 0));
    list = draw_set_offsets(list, tw, th, tw, th);
    return draw_sprite(list, 198, 53, 0, u, v, 0, 0);
}

/* the small flag before each language name */
void *tlang_marker(void *list, int i, int x, int y)
{
    short tw = title_sprites[36].tw;
    short th = title_sprites[36].th;
    short u = title_sprites[36].u;
    short v = title_sprites[36].v;
    unsigned int region = region_get();

    switch (i) {
    case 0:
        if (region < 2) {
            u += (tw + 1) * i;
            break;
        }
    case 1:
    case 2:
    case 3:
    case 4:
        u += (tw + 1) * (i + 1);
        break;
    }
    list = tmenu_gfx_setup(list, 1);
    list = gim_set_texture(list, ehp_find(title_pack(), title_sprite_name(36), 0));
    list = draw_set_offsets(list, tw, th, tw, th);
    return draw_sprite(list, x, y, 0, u, v, 0, 0);
}
