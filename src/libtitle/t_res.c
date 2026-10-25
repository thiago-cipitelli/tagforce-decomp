// @range 0x0880A41C-0x0880AFA0
// @asm 0x0880A5FC-0x0880A8AC
/* Title helpers: the EHF background layers (load, animate, draw), the label of the current screen,
   starting and stopping the sub-screen modules, sound reset, menu availability checks, texts of the
   message tables drawn with alignment, and the story start with the chosen character. */
#include "libtitle.h"

void *title_text_left(void *list, int x, int y, int w, unsigned short *text);
void *title_text_wide(void *list, int x, int y, int w, unsigned short *text);
void *title_text_box(void *list, int x, int y, int w, int a, unsigned short *text);
void *title_text_draw(void *list, int x, int y, int w, int a, unsigned short *text, int align);
int title_ehf_frame(int i);
void *title_msg_draw(void *list, int x, int y, int w, int file, int id, int align);
void title_ehf_anim(int i, int mode, int from, int to);

/* loads EHF layer i of the title pack and starts its animation */
void title_ehf_load(int i, int mode, int from, int to)
{
    void *e = ehp_find(title_pack(), title_ehf_names[i], 0);

    TITLE_EHF(i) = e;

    title_ehf_anim(i, mode, from, to);
}

void title_ehf_anim(int i, int mode, int from, int to)
{
    void *e = TITLE_EHF(i);


    if (e == 0)
        return;
    switch (mode) {
    case 0:
        ehf_set_anim(e, from, from + 1);
        break;
    case 1:
        ehf_set_anim(e, from, to + 1);
        break;
    case 2:
        ehf_set_anim(e, from, to + 1);
        break;
    }
}

int title_ehf_frame(int i)
{
    void *e = TITLE_EHF(i);

    if (e != 0)
        return ehf_get_frame_count(e);
    return -1;

}

void *title_ehf_draw(void *list, int i)
{
    void **e = &title.ehf[i];
    unsigned int opts[24];

    if (*e != 0) {
        memset(opts, 0, sizeof(opts));
        opts[0] = *gfx_get_state();
        list = gfx_begin_2d(list);
        list = gfx_end_2d(ehf_draw(list, *e, opts));
    }
    return list;
}

#ifdef NON_MATCHING /* os testes do estado saem em outra ordem; a tabela de saltos fica no asm */
/* the name plate of the current screen, shown while the menu layer (13) is in place */
void *title_screen_label(void *list)
{
    int layer = 17;
    int u = 112;
    int y = 7;
    int idx = 117;
    int v = 0;
    int end = 97;
    int f;
    int show;
    int h;
    unsigned int color;

    if (title.state == 12) {
        end = 54;
        layer = 15;
        idx = 118;
    } else if (title.state == 14) {
        layer = 14;
        end = 54;
        idx = 118;
    } else if (title.state == 19) {
        layer = 13;
    } else if (title.state == 17 || title.state == 13 || title.state == 6 || title.state == 8 ||
               title.state == 9) {
        layer = 13;
    }
    if (title.state == 14) {
        u = 15;
        y = 4;
        switch (title.sub) {
        case 4:
            v = 0;
            break;
        case 5:
            v = 21;
            break;
        case 6:
        case 7:
            v = 42;
            break;
        case 8:
            v = 189;
            break;
        case 9:
            v = 210;
            break;
        case 14:
            v = 84;
            break;
        case 16:
            v = 126;
            break;
        case 17:
            v = 105;
            break;
        case 18:
            v = 147;
            break;
        case 19:
            v = 126;
            break;
        }
    } else if (title.state == 12) {
        y = 4;
        u = 15;
        if (title.sub == 4)
            v = 210;
        else if (title.sub == 3)
            v = 189;
        else if (title.sub == 2)
            v = 168;
    } else if (title.state == 19) {
        v = 105;
    } else if (title.state == 17) {
        v = 84;
    } else if (title.state == 13) {
        v = 63;
    } else if (title.state == 6) {
        v = 42;
    } else if (title.state == 8) {
        v = 21;
    } else if (title.state == 9) {
        v = 0;
    }
    if (layer == 17)
        return list;
    f = title_ehf_frame(layer);
    h = -1;
    color = 0xFFFFFFFF;
    show = 0;
    if (f == 6 || f == end) {
        show = 1;
        h = 10;
        color = 0x80FFFFFF;
        y += 5;
    } else if (f >= 7 && f < end) {
        show = 1;
    }
    if (show)
        list = title_sprite_ex(list, 1, idx, u, y, -1, h, 0, v, -1, -1, color);
    return list;
}
#endif

/* load callback of the sub-screen modules: starts the module */
int title_module_start(void *arg, int id)
{
    Title *const t = &title;
    int a = 1;

    if (id <= 0)
        return 0;
    t->mod_loaded = 1;
    t->b3038_5 = 0;
    t->sub_mod = id;
    sceKernelStartModule(id, 4, &a, 0, 0);
    return 1;
}

void title_module_stop(void)
{
    Title *const t = &title;
    int status;

    if (title.mod_loaded) {
        t->mod_loaded = 0;
        sceKernelStopModule(t->sub_mod, 0, 0, &status, 0);
        sceKernelUnloadModule(t->sub_mod);
    }
}

/* waits for the sounds to stop and loads the title banks */
void title_sound_reset(int keep)
{
    snd_cmd_fefb_0();
    while (snd_count_bgm() + snd_count_se() != 0)
        sys_frame_wait();
    snd_cmd_fefb();
    bgm_fade(330);
    if (keep == 0) {
        bgm_fade(327);
        bgm_fade(337);
    }
}

/* some story character has a deck */
int title_any_deck(void)
{
    int i;

    for (i = 2; i < 152; i++) {
        if (deck_slot_get(i)->count > 0)
            return 1;
    }
    return 0;
}

int title_flag8(void)
{
    return sysflag_test(8) != 0;
}

void *title_msg_text(void *list, int x, int y, int w, int file, int id)
{
    return title_text_left(list, x, y, w,
                           table_get_entry(ehp_find(title_pack(), title_res_name(file), 0), id));
}

/* text of a message table and where it starts when centered in w */
void title_msg_center(int x, int w, int file, int id, unsigned short **text, int *tx, int *tw)
{
    unsigned short *s = table_get_entry(ehp_find(title_pack(), title_res_name(file), 0), id);
    int n;

    *text = s;
    n = font_measure_text(s) >> 6;
    *tx = x + (w - n) / 2;
    *tw = n;
}

void *title_msg_font(void *list, int x, int y, int w, int file, int id, int align, int fw, int fh,
                     unsigned int color)
{
    font_set_size(fw, fh);
    font_set_color(color);
    font_set_flag4(1);
    return title_msg_draw(list, x, y, w, file, id, align);
}

void *title_msg_draw(void *list, int x, int y, int w, int file, int id, int align)
{
    return title_text_draw(list, x, y, w, 0,
                           table_get_entry(ehp_find(title_pack(), title_res_name(file), 0), id), align);
}

/* align: 0 left, 1 centered, 2 wide, 3 wide centered if it fits, 4 boxed */
void *title_text_draw(void *list, int x, int y, int w, int a, unsigned short *text, int align)
{
    int n;
    int tw = w;

    switch (align) {
    case 1:
        tw = font_measure_text(text) >> 6;
        x += (w - tw) / 2;
        break;
    case 3:
        n = font_measure_text(text) >> 6;
        if (n < w) {
            tw = n;
            x += (w - n) / 2;
        }
        break;
    }
    switch (align) {
    case 0:
    case 1:
        list = title_text_left(list, x, y, tw, text);
        break;
    case 2:
    case 3:
        list = title_text_wide(list, x, y, tw, text);
    case 4:
        list = title_text_box(list, x, y, tw, a, text);
        break;
    }
    return list;
}

/* starts the story with character `chr` (0 = the one saved) */
void title_story_start(int kind, int chr)
{
    DeckFlags f;
    short v[2];
    int set = kind;

    deck_cur_get_u16_0(v);
    *(short *)&f = v[0];
    if (kind == 0 && f.b0)
        kind = f.kind;
    if (chr == 0)
        chr = f.chara;
    deck_slot_set_level(chr, deck_slot_get_level(chr));
    if (set != 0)
        sysflag_apply(kind, chr);
    title_scene_push(2, 0);
}
