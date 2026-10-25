// @range 0x08807C98-0x08808D48
// @asm 0x08807D80-0x08807E44
// @data 0x0882E3E8-0x0882E404
// @rodata 0x0882ABEC-0x0882AC40
/* Title resources and drawing helpers: the logo screens, the per-language title_menu_X.ehp packs
   and their message tables, sprites from the sprite tables, a color fade, the screen fade with the
   queued state change, and the system window used for the prompts. */
#include "libtitle.h"

void *title_draw_konami(void *list);
void *title_draw_jump(void *list);
void *title_draw_e3(void *list);
void title_pack_load(int i);
int title_pack_loaded(void *buf, int size);
void title_pack_free(int i);

void *title_logo_draw(void *list)
{
    font_set_size(32, 32);
    font_set_color(0xFFFFFFFF);
    switch (title.sub) {
    case 0:
    case 1:
    case 2:
        list = title_draw_konami(list);
        break;
    case 3:
    case 4:
    case 5:
        list = title_draw_jump(list);
        break;
    case 6:
    case 7:
    case 8:
        list = title_draw_e3(list);
        break;
    }
    return list;
}

/* text id of the title message table (title_msg_X.bin) */
unsigned short *title_msg(int id)
{
    return table_get_entry(ehp_find(title_pack(), title_res_name(0), 0), id);
}

#ifdef NON_MATCHING /* &title e &title.duel trocam de $s (s2/s3) */
/* text id of message table `file`; 117/118 take the name of the card of the free duel */
unsigned short *title_text(int id, int file)
{
    TDuelCfg *const d = &title.duel;
    Title *const t = &title;
    void *tab = ehp_find(title_pack(), title_res_name(file), 0);

    switch (id) {
    case 117:
    case 118:
        wsprintf(t->msg_buf, table_get_entry(tab, id), card_get_name(d->card));
        return t->msg_buf;
    }
    return table_get_entry(tab, id);
}
#endif

/* loads the title_menu_X.ehp of the current language (mode 1: frees the previous one first) */
void title_lang_load(int mode)
{
    Title *const t = &title;

    if (mode != 0) {
        if (mode == 1) {
            title_pack_free(t->lang + 3);
            cfg_set_byte0_bits0_2(t->cfg_lang);
        }
    }
    t->lang = cfg_get_language();
    switch (t->lang) {
    case 0:
        title_pack_load(3);
        break;
    case 1:
        title_pack_load(4);
        break;
    case 2:
        title_pack_load(5);
        break;
    case 3:
        title_pack_load(6);
        break;
    case 4:
        title_pack_load(7);
        break;
    case 5:
        title_pack_load(8);
        break;
    }
}

void title_pack_load(int i)
{
    char path[256];
    Title *const t = &title;
    int size;
    void *buf;

    sprintf(path, title_pack_names[i]);
    size = ehfile_get_size(3, path);
    if (size > 0) {
        buf = heap_alloc(t->heap, size);
        t->packs[i] = buf;
        ehfile_read_async_alias(3, path, buf, size, title_pack_loaded, buf);
    }
}

int title_pack_loaded(void *buf, int size)
{
    Title *const t = &title;

    if (size > 0 && buf != 0) {
        ehp_mount(0, buf);
        t->sub++;
        return 1;
    }
    return 0;
}

void title_pack_free(int i)
{
    heap_set_level(title.heap, title.packs[i]);
}

void *title_pack(void)
{
    return title.packs[title.lang + 3];
}

char *title_sprite_name(int i)
{
    return title_sprites[i].name;
}

char *title_sprite_gim(TSpr16 *tab, int i)
{
    return title_gim_names[tab[i].gim];
}

char *title_gim_name(int i)
{
    return title_gim_names[i];
}

char *title_ehf_name(int i)
{
    return title_ehf_names[i];
}

char *title_res_name(int i)
{
    return title_res[title.lang + i].name;
}

void *title_sprite(void *list, int kind, int idx, int x, int y)
{
    return title_sprite_uv(list, kind, idx, x, y, 0, 0, -1, -1);
}

void *title_sprite_flip(void *list, int kind, int idx, int x, int y, short a, short b)
{
    return title_sprite_uv(list, kind, idx, x, y, 0, 0, a, b);
}

void *title_sprite_uv(void *list, int kind, int idx, int x, int y, int du, int dv, short a, short b)
{
    return title_sprite_ex(list, kind, idx, x, y, -1, -1, du, dv, a, b, -1);
}

/* sprite idx of title_sprites (kind 0) or title_sprites2; w/h, a/b < 0 = from the table */
void *title_sprite_ex(void *list, int kind, int idx, int x, int y, int w, int h, int du, int dv, short a,
                      short b, int color)
{
    TSprite s;
    TSpr16 s2;
    short u;
    short v;
    short tw;
    short th;
    short sa;
    short sb;
    void *gim;

    list = tmenu_gfx_setup(list, 0);
    if (color != -1)
        list = dl_color(list, color);
    if (kind == 0) {
        s = title_sprites[idx];
        u = s.u + du;
        v = s.v + dv;
        tw = s.tw;
        th = s.th;
        sa = s.a;
        sb = s.b;
        gim = ehp_find(title_pack(), title_sprite_gim((TSpr16 *)title_sprites, idx), 0);
    } else {
        s2 = title_sprites2[idx];
        u = s2.u + du;
        v = s2.v + dv;
        tw = s2.tw;
        th = s2.th;
        sa = s2.a;
        sb = s2.b;
        gim = ehp_find(title_pack(), title_sprite_gim(title_sprites2, idx), 0);
    }
    list = gim_set_texture(list, gim);
    if (a >= 0)
        sa = a;
    if (b >= 0)
        sb = b;
    if (w < 0)
        w = tw;
    if (h < 0)
        h = th;
    list = draw_set_offsets(list, tw, th, w, h);
    return draw_sprite(list, x, y, 0, u, v, sa, sb);
}

void *title_draw_konami(void *list)
{
    int ids[2] = { 0, 1 };
    TSprite s;
    int i;

    list = gim_set_texture(list, ehp_find(title_pack(), title_sprite_name(ids[0]), 0));
    for (i = 0; i < 2; i++) {
        s = title_sprites[ids[i]];
        list = draw_set_offsets(list, s.tw, s.th, s.w, s.h);
        list = draw_sprite(list, s.x, s.y, 0, s.u, s.v, s.a, s.b);
    }
    return list;
}

void *title_draw_jump(void *list)
{
    int ids[2] = { 2, 3 };
    TSprite s;
    int i;

    list = gim_set_texture(list, ehp_find(title_pack(), title_sprite_name(ids[0]), 0));
    for (i = 0; i < 2; i++) {
        s = title_sprites[ids[i]];
        list = draw_set_offsets(list, s.tw, s.th, s.w, s.h);
        list = draw_sprite(list, s.x, s.y, 0, s.u, s.v, s.a, s.b);
    }
    return list;
}

void *title_draw_e3(void *list)
{
    int ids[3] = { 4, 5, 6 };
    TSprite s;
    int i;

    cfg_get_language();
    list = gim_set_texture(list, ehp_find(title_pack(), title_gim_names[9], 0));
    for (i = 0; i < 3; i++) {
        s = title_sprites[ids[i]];
        list = draw_set_offsets(list, s.tw, s.th, s.w, s.h);
        list = draw_sprite(list, s.x, s.y, 0, s.u, s.v, s.a, s.b);
    }
    return list;
}

void tcfade_init(TColorFade *f, unsigned int *dst, unsigned int to, unsigned short len)
{
    f->dst = dst;
    f->from = *dst;
    f->to = to;
    f->len = len;
    f->t = 0;
    f->active = 0;
}

void tcfade_start(TColorFade *f)
{
    f->active = 1;
}

/* one step of the color interpolation; returns 1 while running */
unsigned short tcfade_step(TColorFade *f)
{
    unsigned short len;
    unsigned short inv;
    int n;

    if (f->active == 1) {
        n = f->len;
        f->t++;
        if (n != 0) {
            len = n;
            inv = len - f->t;
            *f->dst = ((unsigned short)((f->from & 0xFF000000) >> 24) * inv / len +
                       (unsigned short)((f->to & 0xFF000000) >> 24) * f->t / len & 0xFF) << 24 |
                      ((unsigned short)((f->from & 0xFF0000) >> 16) * inv / len +
                       (unsigned short)((f->to & 0xFF0000) >> 16) * f->t / len & 0xFF) << 16 |
                      ((unsigned short)((f->from & 0xFF00) >> 8) * inv / len +
                       (unsigned short)((f->to & 0xFF00) >> 8) * f->t / len & 0xFF) << 8 |
                      ((unsigned short)(f->from & 0xFF) * inv / len +
                       (unsigned short)(f->to & 0xFF) * f->t / len & 0xFF);
            if (len <= f->t) {
                f->len = 0;
                f->t = 0;
                f->active = 0;
            }
        } else {
            *f->dst = f->to;
            f->active = 0;
        }
    }
    return f->active;
}

void title_fade_out(int kind)
{
    title_fade(kind, 16);
}

void title_fade(int kind, int frames)
{
    title_fade_ex(kind, 1, 0xFF000000, frames);
}

/* kind: 0 out to black, 1 in from black, 2 out to white, 3 in from white, 4/5 out/in with `color` */
void title_fade_ex(int kind, int type, unsigned int color, int frames)
{
    Title *const t = &title;

    if (title.fade_state == 0) {
        switch (kind) {
        case 0:
            if ((unsigned short)frames == 16)
                fade_start(1, 0xFF000000, 16);
            else
                fade_start(type, 0xFF000000, frames);
            break;
        case 1:
            if ((unsigned short)frames == 16)
                fade_start_in(1, 0xFF000000, 16);
            else
                fade_start_in(type, 0xFF000000, frames);
            break;
        case 2:
            if ((unsigned short)frames == 16)
                fade_start(1, 0xFFFFFFFF, 16);
            else
                fade_start(type, 0xFFFFFFFF, frames);
            break;
        case 3:
            if ((unsigned short)frames == 16)
                fade_start_in(1, 0xFFFFFFFF, 16);
            else
                fade_start_in(type, 0xFFFFFFFF, frames);
            break;
        case 4:
            fade_start(type, color, frames);
            break;
        case 5:
            fade_start_in(type, color, frames);
            break;
        }
        t->fade_state = 1;
        t->next_state = 0xFFF;
    }
}

void title_goto_step(int kind, short state, short sub, short step)
{
    title_goto_fade(kind, state, sub);
    title.next_step = step;
    title.next_step2 = 0;
}

void title_goto_fade(int kind, short state, short sub)
{
    title_fade_out(kind);
    title.next_state = state;
    title.next_sub = sub;
    title.next_step = 0;
    title.next_step2 = 0;
}

void title_goto_fade_n(int kind, int frames, short state, short sub)
{
    title_fade(kind, frames);
    title.next_state = state;
    title.next_sub = sub;
    title.next_step = 0;
    title.next_step2 = 0;
}

int title_fade_state(void)
{
    return title.fade_state;
}

void twin_open(MenuWidget *w)
{
    menu_reset(w);
    menu_open(w);
}

/* the system window of the prompts (two-line message box at the bottom) */
void twin_init(MenuWidget *w)
{
    menu_reset(w);
    memset(w, 0, sizeof(*w));
    w->def.visible = 1;
    w->def.style = 0;
    w->def.style2 = 2;
    w->def.x = 39;
    w->def.y = 224;
    w->def.auto_size = 0;
    w->def.w = 401;
    w->def.h = 46;
    w->def.color = 0xFFFFFFFF;
    w->def.text = 0;
    w->def.font_flag4 = 1;
    w->def.pad_l = 12;
    w->def.pad_r = 12;
    w->def.pad_t = 6;
    w->def.pad_b = 6;
    w->def.font_size = 13;
    w->def.text_color = 0xFFFFFFFF;
    w->def.title = 0;
    w->def.title_font_size = 13;
    w->def.title_color = 0xFFFFFFFF;
    w->def.open_anim = 2;
    w->def.close_anim = 2;
    w->def.show_title = 0;
    w->def.auto_title = 1;
    w->heap = title.heap;
    w->no_input = 1;
    w->no_cancel = 0;
    w->cancel_blocked = 0;
    w->keep_open = 0;
    w->f0_4 = 0;
    w->cancel_resets = 0;
    w->style = 2;
    w->count = 2;
    w->rows = 2;
    w->buttons = 0;
    w->item_w = 377;
    w->item_h = 13;
    twin_open(w);
}

void *twin_draw(void *list, MenuWidget *w)
{
    menu_draw(&list, w);
    return list;
}
