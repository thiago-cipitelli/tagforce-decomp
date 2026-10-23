// @range 0x08804B50-0x08805FF8
// @asm 0x08804B50-0x08804F40
// @asm 0x088054D8-0x08805BA4
// @data 0x0880CBBC-0x0880CC1C
/* 2D part of the machine and the per-frame draw export: background, machine panel, the EHF
   animations of the panel, the message window with its cursor, the card coming out (rendered to a
   texture, then drawn zoomed and fading), the white flash, and the name of the new card. */
#include "libconv_machine.h"

typedef struct CmVtx {          /* GE vertex: uv, color, xyz (0x80019F) */
    float u, v;
    unsigned int color;
    float x, y, z;
} CmVtx;

unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
unsigned int *dl_clear(unsigned int *list, unsigned int flags);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
unsigned int *dl_tex_flush(unsigned int *list);
unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle);
unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr);
unsigned int *dl_draw_array(unsigned int *list, int prim, unsigned int vtype, int count, void *indices, void *vertices);
void *dl_alloc_inline(unsigned int **list, int bytes);
unsigned int *gfx_set_draw_target(unsigned int *list, int base, unsigned int fbp, int w, int h, int psm);
unsigned int *gfx_set_default_target(unsigned int *list);
unsigned int *gfx_begin_2d(unsigned int *list);
unsigned int *gfx_end_2d(unsigned int *list);
unsigned int *card_draw(unsigned int *list, int x, int y, unsigned int id);
unsigned int *gim_set_texture(unsigned int *list, void *gim);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
unsigned int *ehf_draw(unsigned int *list, void *ehf, void *opts);
int ehf_get_frame_count(void *e);
int ehf_get_data_ptr(void *e);
int ehf_set_frame_alias(void *e, unsigned int frame);
GfxState *gfx_get_state(void);
char *sceGeEdramGetAddr(void);
void mtx_set_mode(int mode);
void cardinfo_draw(void);
void libselect_card_08807630(void);
int font_draw_wtext_ex(int x, int y, int w, int h, unsigned short *text, int max_glyphs);
unsigned short *card_get_name(int id);

unsigned int *cm_draw_bg(unsigned int *list);
unsigned int *cm_draw_machine(unsigned int *list);
unsigned int *cm_draw_anim(unsigned int *list, int which);
unsigned int *cm_draw_message(unsigned int *list);
unsigned int *cm_draw_cards(unsigned int *list);
unsigned int *cm_draw_flash(unsigned int *list, unsigned char alpha);
unsigned int *cm_draw_dim(unsigned int *list, unsigned char alpha);
unsigned int *cm_draw_name(unsigned int *list, int x, int y);
unsigned int *cm_draw_hint(unsigned int *list);
void cm_anim_get(int which, unsigned short *id, unsigned short *speed);
void cm_gfx_begin3d(void);
void cm_draw_lid(void);
void cm_draw_card3d(CmCard *c);
void cm_draw_result3d(void);

#ifdef NON_MATCHING /* o original deixa a cópia morta da primeira instrução do ramo depois do `b` (beql; b; <cópia>), problema aberto */
/* 0x08804B50: export, once per frame (libselect_card draws itself while it is up) */
void conv_draw(void)
{
    unsigned int *list;
    int i;

    if (cm.selecting) {
        libselect_card_08807630();
        return;
    }
    list = dl_select_layer(0, 0);
    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (cm.state == 0)
        return;
    if (cm.detail) {
        cardinfo_draw();
        return;
    }
    list = dl_select_layer(1, 0);
    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 1);
    list = cm_draw_cards(list);
    list = cm_draw_bg(list);
    list = cm_draw_anim(list, 0);
    list = cm_draw_anim(list, 1);
    list = cm_draw_anim(list, 2);
    list = cm_draw_anim(list, 3);
    list = cm_draw_anim(list, 5);
    list = cm_draw_machine(list);
    list = cm_draw_message(list);
    dl_set_list(list);
    cm_gfx_begin3d();
    mtx_set_mode(2);
    cm_draw_lid();
    switch (cm.state) {
    case 4:
    case 5:
        if (cm.dim) {
            list = dl_select_layer(2, 0);
            list = cm_draw_dim(list, cm.dim);
            dl_set_list(list);
        }
        cm_draw_result3d();
        break;
    case 3:
        for (i = 0; i < cm.n_cards; i++)
            cm_draw_card3d(&cm.flying[i]);
        break;
    }
    list = dl_select_layer(4, 0);
    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 1);
    if (cm.state == 5) {
        list = cm_draw_name(list, cm.name_x, 0xDD);
        if (cm.name_shown)
            list = cm_draw_hint(list);
    }
    list = cm_draw_anim(list, 6);
    list = cm_draw_anim(list, 7);
    list = cm_draw_anim(list, 8);
    dl_set_list(list);
    list = dl_select_layer(5, 0);
    list = dl_disable(list, 1);
    fade_apply((void **)&list);
    dl_set_list(list);
}
#endif

/* 480x272 background in 64-pixel strips; the bottom 16 lines come from the right edge, rotated */
unsigned int *cm_draw_bg(unsigned int *list)
{
    void *gim = cm_res_get(0x1001);
    int i;
    int x;
    int v;
    int j;

    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 64, 256, 64, 256);
    for (i = 0, x = 0; i < 7; i++, x += 64)
        list = draw_sprite(list, x, 0, 0, x, 0, 0, 0);
    list = draw_set_offsets(list, 32, 256, 32, 256);
    list = draw_sprite(list, i * 64, 0, 0, i * 64, 0, 0, 0);
    list = draw_set_offsets(list, 16, 64, 64, 16);
    for (j = 0, v = 0; j < 4; j++, v += 64) {
        list = draw_sprite(list, 256 - (j + 1) * 64, 256, 0, 480, v, 0, 3);
        list = draw_sprite(list, 512 - (j + 1) * 64, 256, 0, 496, v, 0, 3);
    }
    return list;
}

/* the machine panel (cover.gim) */
unsigned int *cm_draw_machine(unsigned int *list)
{
    void *gim = cm_res_get(0x1002);
    int i;
    int u;
    int x;

    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 64, 160, 64, 160);
    for (i = 0, u = 0, x = 0x83; i < 3; i++, u += 64, x += 64)
        list = draw_sprite(list, x, 0x5A, 0, u, 0, 0, 0);
    return list;
}

/* animation slot `which` (cm_anims): plays at its speed, wrapping before the end */
unsigned int *cm_draw_anim(unsigned int *list, int which)
{
    unsigned short id;
    unsigned short speed;
    unsigned int opts[27];     /* EhfOpts, 0x60 used */
    void *e;
    int f;

    cm_anim_get(which, &id, &speed);
    if (speed == 0)
        return list;
    e = cm_res_get(id);
    if (e == 0)
        return list;
    memset(opts, 0, 0x60);
    opts[0] = gfx_get_state()->frame;
    list = gfx_begin_2d(list);
    if (speed == 1) {
        list = ehf_draw(list, e, opts);
    } else {
        f = ehf_get_frame_count(e);
        f += speed;
        if (f >= ehf_get_data_ptr(e) - 1)
            f %= ehf_get_data_ptr(e) - 1;
        ehf_set_frame_alias(e, f);
        list = ehf_draw(list, e, opts);
    }
    list = gfx_end_2d(list);
    list = dl_enable(list, 4);
    return dl_blend_func(list, 0, 2, 3, 0, 0);
}

/* message window text, typed out, with the blinking block cursor after it */
unsigned int *cm_draw_message(unsigned int *list)
{
    unsigned int cx;
    unsigned int cy;
    unsigned int x;
    unsigned int y;
    unsigned short ch[2];
    unsigned short w;
    unsigned short h;

    font_text_set(list);
    font_set_color(0xFFFFFFFF);
    font_set_size(14, 14);
    font_draw_wtext_ex(0x940, 0x740, 0x4700, 0x780, cm.text, cm.text_pos);
    list = font_text_clear();
    if (cm.state == 5)
        return list;
    font_set_size(14, 14);
    if (cm.typing || (gfx_get_state()->frame >> 3 & 1)) {
        cx = font_get_cursor_x();
        cy = font_get_cursor_y();
        y = (cy - font_line_height()) >> 6;
        x = cx >> 6;
        memcpy(ch, table_get_entry(cm_res_get(CM_STRINGS), 1), 2);
        ch[1] = 0;
        w = font_measure_text(ch) >> 6;
        h = font_line_height() >> 6;
        if (x + w > 0x141) {
            x = 0x25;
            y += font_line_height() >> 6;
        }
        list = dl_disable(list, 9);
        list = dl_tex_func(list, 4, 0);
        list = dl_ambient_color(list, font_get_color());
        list = draw_set_offsets(list, 0, 0, w - 2, h);
        list = draw_sprite(list, x + 1, y, 0, 0, 0, 0, 0);
        list = dl_enable(list, 9);
        list = dl_tex_func(list, 0, 1);
        list = dl_ambient_color(list, 0xFFFFFFFF);
    }
    return list;
}

/* corners of the card picture (u, v, then x, y on a 108x154 card) */
float cm_card_vtx[6][4] = {
    { 108.0f, 0.0f, 108.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f },
    { 108.0f, 77.0f, 108.0f, 77.0f },
    { 0.0f, 77.0f, 0.0f, 77.0f },
    { 108.0f, 154.0f, 108.0f, 154.0f },
    { 0.0f, 154.0f, 0.0f, 154.0f },
};

#ifdef NON_MATCHING /* não testado para matching */
/* the converted card (zooming, then folding open) and the cards burning in the machine */
unsigned int *cm_draw_cards(unsigned int *list)
{
    CmVtx *v;
    int z;
    int w;
    int h;
    int x;
    int y;
    int i;
    int d;
    float f;
    CmCard *c;
    int t;

    if (cm.state == 4 && cm.result != 0) {
        v = dl_alloc_inline(&list, 0x90);
        z = cm.zoom;
        if (z >= 0x81)
            z = 0x80;
        w = (z * 4 * 8 + z * 7) * 2 / 128;
        h = z * 0x70 / 128;
        x = (0xB9 - w) / 2 + 0x83;
        y = (0x8D - h) / 2 + 0x5A;
        list = gfx_set_draw_target(list, 0x160000, 0xFFFFFFFF, 0x80, 0x100, 3);
        list = card_draw(list, 0, 0, cm.result);
        list = gfx_set_default_target(list);
        list = dl_tex_flush(list);
        list = dl_tex_func(list, 0, 0);
        list = dl_tex_mode(list, 3, 0, 0, 0);
        list = dl_tex_image(list, 0, 0x80, 0x100, 0x80, sceGeEdramGetAddr() + 0x160000);
        list = dl_ambient_color(list, 0xFFFFFF);
        memset(v, 0, 0x90);
        for (i = 0; i < 6; i++) {
            v[i].x = x + w * cm_card_vtx[i][0] / 108.0f;
            v[i].y = y + h * cm_card_vtx[i][1] / 154.0f;
            v[i].u = cm_card_vtx[i][2];
            v[i].v = cm_card_vtx[i][3];
            v[i].color = cm.glow << 24 | 0xFFFFFF;
        }
        if (cm.zoom > 0x80) {
            d = (cm.zoom - 0x80) * 0x6B / 8;
            v[2].x += d;
            v[3].y -= d;
            d = (cm.zoom - 0x80) * 0x70 / 32;
            v[0].y += d;
            v[1].y += d;
            v[4].y -= d;
            v[5].y -= d;
        }
        list = dl_draw_array(list, 4, 0x80019F, 6, 0, v);
    }
    list = dl_tex_func(list, 0, 1);
    switch (cm.flash_fade) {
    case 2:
        if (cm.flash < 9) {
            cm.flash = 0;
            cm.flash_fade = 0;
        } else {
            cm.flash -= 8;
        }
        break;
    case 1:
        if (cm.flash >= 0xFB) {
            cm.flash = 0xFF;
            cm.flash_fade = 0;
        } else {
            cm.flash += 4;
        }
        break;
    }
    list = cm_draw_flash(list, cm.flash);
    if (cm.state == 4) {
        for (i = 0; i < cm.n_cards; i++) {
            c = &cm.flying[i];
            if (c->state == 3 && c->t > 0) {
                t = 0x15 - c->t;
                w = t * t * 0x4E / 0x1B9;
                h = t * t * 0x70 / 0x1B9;
                list = dl_tex_flush(list);
                list = gfx_set_draw_target(list, 0x160000, 0xFFFFFFFF, 0x80, 0x100, 3);
                list = card_draw(list, 0, 0, cm.cards[i]);
                list = gfx_set_default_target(list);
                list = dl_tex_flush(list);
                list = dl_tex_func(list, 0, 0);
                list = dl_tex_mode(list, 3, 0, 0, 0);
                list = dl_tex_image(list, 0, 0x80, 0x100, 0x80, sceGeEdramGetAddr() + 0x160000);
                list = dl_ambient_color(list, (t * 0xFF / 0x15) << 24 | 0xFFFFFF);
                list = draw_set_offsets(list, 0x6C, 0x9A, w, h);
                list = draw_sprite(list, c->x + 220.0f - w / 2, c->y + 160.0f - h / 2, 0, 0, 0, 0, 0);
            }
        }
    }
    return dl_tex_func(list, 0, 1);
}
#endif

/* full screen white flash at `alpha` over the panel animation 4 */
unsigned int *cm_draw_flash(unsigned int *list, unsigned char alpha)
{
    unsigned int *l;

    if (alpha == 0)
        return list;
    if (alpha == 0xFF)
        return cm_draw_anim(list, 4);
    list = dl_tex_flush(list);
    list = gfx_set_draw_target(list, 0x174000, 0xFFFFFFFF, 0x200, 0x200, 3);
    list = dl_disable(list, 9);
    list = dl_tex_func(list, 4, 0);
    list = dl_ambient_color(list, 0xFF000000);
    list = draw_set_offsets(list, 0, 0, 480, 272);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    list = dl_enable(list, 9);
    list = dl_tex_func(list, 0, 1);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = cm_draw_anim(list, 4);
    list = gfx_set_default_target(list);
    list = dl_tex_flush(list);
    list = dl_tex_func(list, 0, 0);
    l = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_image(l, 0, 0x200, 0x200, 0x200, sceGeEdramGetAddr() + 0x174000);
    list = dl_ambient_color(list, alpha << 24 | 0xFFFFFF);
    list = draw_set_offsets(list, 480, 272, 480, 272);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    return dl_tex_func(list, 0, 1);
}

/* full screen black at `alpha` */
unsigned int *cm_draw_dim(unsigned int *list, unsigned char alpha)
{
    list = dl_disable(list, 1);
    list = dl_disable(list, 9);
    list = dl_tex_func(list, 4, 0);
    list = dl_ambient_color(list, alpha << 24);
    list = draw_set_offsets(list, 0, 0, 480, 272);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    list = dl_enable(list, 9);
    list = dl_tex_func(list, 0, 1);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    return dl_enable(list, 1);
}

/* name of the new card, red outlined */
unsigned int *cm_draw_name(unsigned int *list, int x, int y)
{
    unsigned short *name = card_get_name(cm.result);
    int py;
    int px;

    font_set_flag4(0);
    font_text_set(list);
    if (name) {
        font_set_size(cm.name_size, 18);
        font_set_color(0xFF0000FF);
        py = (y + 1) * 64;
        font_draw_text(x * 64, py, 0x7800, name);
        font_draw_text((x + 2) * 64, py, 0x7800, name);
        px = (x + 1) * 64;
        font_draw_text(px, y * 64, 0x7800, name);
        font_draw_text(px, (y + 2) * 64, 0x7800, name);
        font_set_color(0xFFFFFFFF);
        font_draw_text(px, py, 0x7800, name);
    }
    font_set_flag4(1);
    return font_text_clear();
}

/* "press a button" hint in the corner */
unsigned int *cm_draw_hint(unsigned int *list)
{
    unsigned short *text = table_get_entry(cm_res_get(CM_STRINGS), 7);
    int w;
    int h;

    font_set_size(14, 14);
    w = font_measure_text(text) + 0x40;
    h = font_line_height() + 0x40;
    font_text_set(list);
    font_draw_text(0x7700 - w, 0x4300 - h, w, text);
    return font_text_clear();
}
