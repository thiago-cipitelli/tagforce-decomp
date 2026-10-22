// @range 0x08804844-0x0880597C
// @rodata 0x0880B444-0x0880B460
/* Password screen drawing: background, monitor, message window (typed-out text, card name, the card
   image waving in), keypad and nixie digits (EHF animations), blinking text cursor. */
#include "libpassword.h"

typedef struct DrawOpts {              /* EhfOpts of BOOT ehf.c, 0x60 */
    int time;
    void *tex_cb;
    void *tex_arg;
    unsigned int flags;                /* +0x0C 0x10000: position */
    int angle;
    float x, y;                        /* +0x14 */
    int pad1c;
    float sx, sy;
    void *eval_cb;
    void *eval_arg;
    int mask[2];
    int pad38[2];
    int color[8];
} DrawOpts;

typedef struct LineVtx {               /* GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | through */
    float u, v;
    unsigned int color;
    float x, y, z;
} LineVtx;

extern unsigned int *dl_select_layer(int layer, unsigned int *size);
extern unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
extern unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
extern unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
extern unsigned int *dl_clear(unsigned int *list, unsigned int flags);
extern int dl_set_list(unsigned int *list);
extern unsigned int *dl_enable(unsigned int *list, int state);
extern unsigned int *dl_disable(unsigned int *list, int state);
extern unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
extern unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
extern unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
extern unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
extern unsigned int *dl_scissor(unsigned int *list, int x, int y, int w, int h);
extern unsigned int *dl_tex_flush(unsigned int *list);
extern unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle);
extern unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr);
extern unsigned int *dl_draw_array(unsigned int *list, int prim, unsigned int vtype, int count, void *indices, void *vertices);
extern void *dl_alloc_inline(unsigned int **list, int bytes);
extern unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
extern unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
extern unsigned int *gim_set_texture(unsigned int *list, void *gim);
extern unsigned int *gfx_set_draw_target(unsigned int *list, int base, unsigned int fbp, int w, int h, int psm);
extern unsigned int *gfx_set_default_target(unsigned int *list);
extern unsigned int *gfx_begin_2d(unsigned int *list);
extern unsigned int *gfx_end_2d(unsigned int *list);
extern unsigned int *gfx_get_state(void);
extern unsigned int *card_draw(unsigned int *list, int x, int y, unsigned int id);
extern float sinf_vfpu(float x);

unsigned int *draw_background(unsigned int *list);
unsigned int *draw_monitor(unsigned int *list);
unsigned int *draw_messages(unsigned int *list);
unsigned int *draw_keypad(unsigned int *list);
unsigned int *draw_digits(unsigned int *list);
void draw_card_help(int y);
unsigned int *draw_card_image(unsigned int *list, int x, int y, unsigned short id, unsigned short t, unsigned char alpha);

void password_draw(void)
{
    unsigned int *list;

    list = dl_select_layer(0, 0);
    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (pw.scene != 0) {
        list = dl_select_layer(4, 0);
        list = dl_enable(list, 9);
        list = dl_enable(list, 4);
        list = dl_tex_func(list, 0, 1);
        list = dl_set_tex_filter(list, 0, 0);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_disable(list, 1);
        list = draw_background(list);
        list = draw_monitor(list);
        list = draw_messages(list);
        list = draw_keypad(list);
        list = draw_digits(list);
        fade_apply((void **)&list);
        dl_set_list(list);
    }
}

unsigned int *draw_background(unsigned int *list)
{
    void *gim;
    int i;

    gim = res_get(RES_BG);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x40, 0x100, 0x40, 0x100);
    for (i = 0; i < 7; i++)
        list = draw_sprite(list, i * 0x40, 0, 0, i * 0x40, 0, 0, 0);
    list = draw_set_offsets(list, 0x20, 0x100, 0x20, 0x100);
    list = draw_sprite(list, i * 0x40, 0, 0, i * 0x40, 0, 0, 0);
    list = draw_set_offsets(list, 0x10, 0x40, 0x40, 0x10);
    for (i = 0; i < 4; i++) {
        list = draw_sprite(list, 0x100 - (i + 1) * 0x40, 0x100, 0, 0x1E0, i * 0x40, 0, 3);
        list = draw_sprite(list, 0x200 - (i + 1) * 0x40, 0x100, 0, 0x1F0, i * 0x40, 0, 3);
    }
    return list;
}

/* message window: what is shown depends on the scene and its step; the text scrolls up as a scene
   ends (pw.timer frames out of 30) */
unsigned int *draw_messages(unsigned int *list)
{
    int cursor = 1;
    int y = 83;
    unsigned char alpha;
    unsigned char w;
    unsigned char fw, fh;
    unsigned int cx, h;
    unsigned int x, ty;
    unsigned short gw, gh;
    unsigned int gs;

    list = dl_scissor(list, 25, 78, 280, 161);
    list = dl_enable(list, 2);
    font_set_size(14, 14);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    switch (pw.scene) {
    case 1:
        font_draw_wtext_ex(0x780, 0x14C0, 0x4380, 0x25C0, pw.text, pw.text_pos);
        break;
    case 2:
        font_draw_wtext_ex(0x780, 0x14C0, 0x4380, 0x25C0, pw.text, -1);
        break;
    case 3:
        if (pw.step == 0) {
            cursor = 0;
            y -= pw.timer * 151 / 30;
        }
        font_draw_wtext_ex(0x780, y << 6, 0x4380, 0x25C0, pw.text, pw.text_pos);
        break;
    case 4:
        if (pw.step == 4) {
            list = draw_card_image(font_text_clear(), 111, y, pw.card_id, pw.timer, 0);
            font_text_set(list);
            cursor = 0;
            break;
        } else if (pw.step == 5) {
            if (pw.timer < 60) {
                alpha = (60 - pw.timer) * 255 / 60 & 0xFF;
                if (alpha > 0) {
                    list = draw_card_image(font_text_clear(), 111, 83, pw.card_id, 120, alpha);
                    font_text_set(list);
                }
                cursor = 0;
                break;
            }
            font_set_color(((pw.timer - 60) * 255 / 60 & 0xFF) << 24 | 0xFFFFFF);
        } else if (pw.step == 3) {
            if (pw.timer >= 30)
                font_set_color(((60 - pw.timer) * 255 / 30 & 0xFF) << 24 | 0xFFFFFF);
        }
        switch (pw.step) {
        case 0:
        case 8:
            cursor = 0;
            y -= pw.timer * 151 / 30;
            break;
        }
        if (pw.step < 4) {
            font_draw_wtext_ex(0x780, y << 6, 0x4380, (151 - (y - 83)) << 6, pw.text, pw.text_pos);
            y = (font_get_cursor_y() >> 6) + 16;
        }
        if (pw.step == 2) {
            w = font_measure_wtext(0x4180, pw.name_full);
            font_get_size(&fw, &fh);
            font_set_size(w, fh);
            font_draw_text(0x880, y << 6, 0x4180, pw.name_shown);
            font_set_size(fw, fh);
        } else if (pw.step > 2) {
            font_draw_wtext(0x880, y << 6, 0x4180, pw.name_shown);
        }
        if (pw.step >= 6) {
            y = (font_get_cursor_y() >> 6) + 16;
            font_draw_wtext_ex(0x780, y << 6, 0x4380, (151 - (y - 83)) << 6, pw.text2, pw.text2_pos);
        }
        if (pw.step >= 7)
            draw_card_help((font_get_cursor_y() >> 6) + 4);
        break;
    case 5:
        switch (pw.step) {
        case 0:
        case 4:
            cursor = 0;
            y -= pw.timer * 151 / 30;
            break;
        }
        font_draw_wtext_ex(0x780, y << 6, 0x4380, 0x25C0, pw.text, pw.text_pos);
        if (pw.step >= 2) {
            y = (font_get_cursor_y() >> 6) + 16;
            font_draw_wtext_ex(0x780, y << 6, 0x4380, (151 - (y - 83)) << 6, pw.text2, pw.text2_pos);
        }
        if (pw.step >= 3)
            draw_card_help((font_get_cursor_y() >> 6) + 4);
        break;
    case 6:
        switch (pw.step) {
        case 0:
        case 3:
            cursor = 0;
            y -= pw.timer * 151 / 30;
            break;
        }
        font_draw_wtext_ex(0x780, y << 6, 0x4380, 0x25C0, pw.text, pw.text_pos);
        if (pw.step >= 2)
            draw_card_help((font_get_cursor_y() >> 6) + 4);
        break;
    }
    list = font_text_clear();
    if (cursor) {
        font_set_size(14, 14);
        if (pw.typing || (*gfx_get_state() >> 3 & 1)) {
            cx = font_get_cursor_x();
            h = font_line_height();
            h += font_get_line_extra();
            ty = (font_get_cursor_y() - h) >> 6;
            x = cx >> 6;
            gs = font_sample_glyph_size();
            gw = (unsigned short)(gs >> 16) >> 6;
            gh = (unsigned short)gs >> 6;
            if (x + gw > 300) {
                x = 30;
                ty += font_line_height() >> 6;
            }
            list = dl_disable(list, 9);
            list = dl_tex_func(list, 4, 0);
            list = dl_ambient_color(list, font_get_color());
            list = draw_set_offsets(list, 0, 0, gw - 2, gh);
            list = draw_sprite(list, x + 1, ty, 0, 0, 0, 0, 0);
            list = dl_enable(list, 9);
            list = dl_tex_func(list, 0, 1);
            list = dl_ambient_color(list, 0xFFFFFFFF);
        }
    }
    return dl_disable(list, 2);
}

unsigned int *draw_monitor(unsigned int *list)
{
    void *gim;

    gim = res_get(RES_MONITOR);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x80, 0x100, 0x118, 0x100);
    return draw_sprite(list, 25, 0, 0, 0, 0, 0, 0);
}

unsigned int *draw_keypad(unsigned int *list)
{
    void *ehf;
    DrawOpts opts;

    ehf = res_get(RES_KEYPAD);
    if (ehf == 0)
        return list;
    memset(&opts, 0, sizeof(opts));
    opts.time = *gfx_get_state();
    list = gfx_begin_2d(list);
    list = ehf_draw(list, ehf, &opts);
    return gfx_end_2d(list);
}

unsigned int *draw_digits(unsigned int *list)
{
    int i;
    int x;
    void *ehf;
    DrawOpts opts;

    for (i = 0, x = 0; i < 8; i++, x += 35) {
        ehf = res_get(RES_DIGIT0 + i);
        if (ehf != 0) {
            memset(&opts, 0, sizeof(opts));
            opts.time = *gfx_get_state();
            opts.flags = 0x10000;
            opts.x = x;
            list = gfx_begin_2d(list);
            list = ehf_draw(list, ehf, &opts);
            list = gfx_end_2d(list);
        }
    }
    return list;
}

/* the four help lines under the card (string table entries 5-8), two columns */
void draw_card_help(int y)
{
    int x = 30;
    unsigned short *s;
    unsigned int w1, w2;

    s = table_get_entry(res_get(RES_STRINGS), 5);
    w1 = font_measure_text(s);
    font_draw_text(0x780, y << 6, 0x7080, s);
    s = table_get_entry(res_get(RES_STRINGS), 6);
    w2 = font_measure_text(s);
    font_draw_text(0x780, (y + 16) << 6, 0x7080, s);
    if (w1 < w2)
        w1 = w2;
    x += (w1 >> 6) + 2;
    s = table_get_entry(res_get(RES_STRINGS), 7);
    font_draw_text(x << 6, y << 6, (0x1E0 - x) << 6, s);
    font_draw_text(x << 6, (y + 16) << 6, (0x1E0 - x) << 6, table_get_entry(res_get(RES_STRINGS), 8));
}

/* the found card, rendered to VRAM 0x160000 then drawn; while t < 120 it is drawn as 154 horizontal
   lines with a sine wobble that fades out */
unsigned int *draw_card_image(unsigned int *list, int x, int y, unsigned short id, unsigned short t, unsigned char alpha)
{
    int f;
    LineVtx *verts;
    LineVtx *v;
    int i;
    float amp;
    float phase;
    float s;
    float fx, fv, fy;
    unsigned int color;

    list = dl_disable(list, 2);
    font_set_line_extra(0);
    list = dl_tex_flush(list);
    list = gfx_set_draw_target(list, 0x160000, 0xFFFFFFFF, 0x80, 0x100, 3);
    list = card_draw(list, 0, 0, pw.card_id);
    list = gfx_set_default_target(list);
    list = dl_tex_flush(list);
    list = dl_tex_func(list, 0, 0);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_image(list, 0, 0x80, 0x100, 0x80, (char *)sceGeEdramGetAddr() + 0x160000);
    f = t & 0xFFFF;
    if (f >= 120) {
        if (alpha > 0)
            list = dl_ambient_color(list, alpha << 24 | 0xFFFFFF);
        else
            list = dl_ambient_color(list, 0xFFFFFFFF);
        list = draw_set_offsets(list, 0x6C, 0x9A, 0x6C, 0x9A);
        list = draw_sprite(list, x, y, 0, 0, 0, 0, 0);
    } else {
        verts = dl_alloc_inline(&list, 0x3000);
        v = verts;
        phase = 0.1f * t;
        color = (f * 255 / 120 & 0xFF) << 24 | 0xFFFFFF;
        amp = (120 - f) * 24 / 120;
        memset(verts, 0, 0x3000);
        for (i = 0; i < 154; i++, v += 2) {
            s = amp * sinf_vfpu(phase + 0.05f * i);
            v[0].u = 0;
            v[1].u = 108.0f;
            fv = 0.5f + i;
            v[0].v = fv;
            v[1].v = fv;
            v[0].color = color;
            v[1].color = color;
            fx = x + s;
            v[0].x = fx;
            v[1].x = 108.0f + fx;
            fy = y + i;
            v[0].y = fy;
            v[1].y = fy;
        }
        list = dl_draw_array(list, 1, 0x80019F, 308, 0, verts);
    }
    list = dl_tex_func(list, 0, 1);
    font_set_line_extra(0x80);
    return list;
}
