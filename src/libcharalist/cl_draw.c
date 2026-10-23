// @range 0x08804F48-0x08807F68
// @asm 0x08804F48-0x08805354
// @asm 0x0880629C-0x08806364
// @asm 0x088073D8-0x088077D0
// @data 0x0880E01C-0x0880E138
/* Drawing of the screen: the window with the three tabs, four duelist lines (face, number and name,
   rank, stars of the deck level, win/draw/loss record), cursor and scroll bars, and the detail page
   (bust-up sliding in, profile text, rank, voice test bar). */
#include "libcharalist.h"
#include "allegrex.h"

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
unsigned int *dl_scissor(unsigned int *list, int x, int y, int w, int h);
unsigned int *gim_set_texture(unsigned int *list, void *gim);
unsigned int *gim_set_texture_sub(unsigned int *list, void *g, int frame, int cell, int pframe, int pcell);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
unsigned int *gfx_begin_2d(unsigned int *list);
unsigned int *gfx_end_2d(unsigned int *list);
unsigned int *ehf_draw(unsigned int *list, void *ehf, void *opts);
GfxState *gfx_get_state(void);
int wsprintf(unsigned short *out, const unsigned short *fmt, ...);
int font_draw_wtext(int x, int y, int w, unsigned short *text);
int font_draw_wtext_ex(int x, int y, int w, int h, unsigned short *text, int max_glyphs);
int font_get_cursor_x(void);
unsigned short *chara_get_name(int i, int j);
int deck_slot_get_level(int i);
unsigned short deck_slot_get_value(int i);
int value_to_tier(int v);
unsigned short sysstate2_rec_get_u16(int which, int i);

unsigned int *cl_draw_clear(unsigned int *list);
unsigned int *cl_draw_tabs(unsigned int *list, int x, int y, int w, int h);
unsigned int *cl_draw_entry(unsigned int *list, int x, int y, int w, int stars, int record, ClEntry *e);
unsigned int *cl_draw_card(unsigned int *list, int x, int y, ClEntry *e);
unsigned int *cl_draw_stars(unsigned int *list, int x, int y, ClEntry *e);
unsigned int *cl_draw_record(unsigned int *list, int x, int y, ClEntry *e);
void cl_draw_number(int x, int y, int n);
unsigned int *cl_draw_cursor(unsigned int *list, int x, int y, int w, int h);
void cl_draw_name(int x, int y, int w, unsigned short id);
unsigned int *cl_draw_rank(unsigned int *list, int x, int y, int dx, unsigned short rank);
unsigned int *cl_draw_arrow(unsigned int *list, int x, int y, unsigned char down, unsigned int period,
                            int blink, int base, unsigned int time, void *gim);
unsigned int *cl_draw_scrollbar(unsigned int *list, int x, int y, int h, int pos, int page, int total, int anim);
unsigned int *cl_draw_header(unsigned int *list);
unsigned int *cl_draw_dim(unsigned int *list, unsigned char alpha);
unsigned int *cl_draw_detail(unsigned int *list);
unsigned int *cl_draw_detail_frame(unsigned int *list, int x, int y);
unsigned int *cl_draw_bustup(unsigned int *list, ClEntry *e);
unsigned int *cl_draw_voicebar(unsigned int *list, int voice);
unsigned int *cl_draw_face(unsigned int *list, int x, int y, int face, int lit);
unsigned int *cl_draw_face_arrow(unsigned int *list, int x, int y, int face, int right);
unsigned int *cl_draw_portrait(unsigned int *list, unsigned short id, int x, int y, int scale, void *head, void *body);

#ifdef NON_MATCHING /* o original deixa a cópia morta do `addiu` do case 3 depois do `b` (beql; b; <cópia>), problema aberto */
/* 0x08804F48: export, once per frame */
void charalist_draw(void)
{
    unsigned int *list;
    int stars = 1;
    int record = 1;
    int w;
    int y0;
    int fw;
    int x;
    int i;
    int k;
    int y;

    list = dl_select_layer(0, 0);
    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (cl.state != 6 && cl.state != 0) {
        list = dl_select_layer(4, 0);
        list = dl_enable(list, 9);
        list = dl_enable(list, 4);
        list = dl_tex_func(list, 0, 1);
        list = dl_set_tex_filter(list, 0, 0);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_disable(list, 1);
        list = cl_draw_clear(list);
        w = 0x13C;
        y0 = 11;
        switch (cl.mode) {
        case 0:
        case 1:
            w += 0x90;
            list = cl_draw_header(list);
            y0 = 23;
            break;
        case 3:
            w += 0x65;
            record = 0;
            break;
        case 2:
            w += 0x2B;
            stars = 0;
            break;
        }
        fw = w + 20;
        x = (480 - fw) / 2;
        list = cl_draw_tabs(list, x, y0, fw, 0xF9);
        list = cl_draw_scrollbar(list, x + fw - 12, y0 + 27, 0xD1, cl.top[cl.tab], 4, cl.count[cl.tab], 1);
        for (i = 0, y = y0 + 20; i < 4; i++, y += 56) {
            k = cl.top[cl.tab] + i;
            if (k >= cl.count[cl.tab])
                break;
            list = cl_draw_entry(list, x + 5, y, w, stars, record, cl_entry(cl.tab, k));
        }
        switch (cl.state) {
        case 3:
            list = cl_draw_dim(list, cl.anim * 192 / 12);
            list = cl_draw_detail(list);
            break;
        default:
            if (cl.top[cl.tab] + cl.cursor[cl.tab] < cl.count[cl.tab])
                list = cl_draw_cursor(list, x + 6, y0 + 20 + cl.cursor[cl.tab] * 56, w - 2, 0x36);
            break;
        }
        dl_set_list(list);
        list = dl_select_layer(5, 0);
        list = dl_disable(list, 1);
        fade_apply((void **)&list);
        dl_set_list(list);
    }
}
#endif

unsigned int *cl_draw_clear(unsigned int *list)
{
    list = dl_clear_color(list, 0xFF383B60);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    return dl_clear(list, 0x17);
}

/* the list window with the three tabs on top; the current tab is lit */
unsigned int *cl_draw_tabs(unsigned int *list, int x, int y, int w, int h)
{
    void *tab = cl_res_get(0x1002);
    int y0;
    int v;
    int y1;
    int x1;
    int u0;
    int dy;
    int u1;
    int i;
    int tw;

    if (tab == 0)
        return list;
    list = gim_set_texture(list, tab);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    v = cl.tab * 32;
    y0 = y + 15;
    list = draw_set_offsets(list, 6, 6, 6, 6);
    list = draw_sprite(list, x, y0, 0, 64, v, 0, 0);
    y1 = y + h - 6;
    list = draw_sprite(list, x, y1, 0, 64, v + 11, 0, 0);
    list = draw_set_offsets(list, 14, 6, 14, 6);
    x1 = x + w - 14;
    list = draw_sprite(list, x1, y0, 0, 98, v, 0, 0);
    list = draw_sprite(list, x1, y1, 0, 98, v + 26, 0, 0);
    list = draw_set_offsets(list, 1, 6, w - 20, 6);
    list = draw_sprite(list, x + 6, y0, 0, 70, v, 0, 0);
    list = draw_sprite(list, x + 6, y1, 0, 70, v + 11, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, h - 27);
    list = draw_sprite(list, x, y0 + 6, 0, 64, v + 6, 0, 0);
    list = draw_set_offsets(list, 14, 1, 14, h - 27);
    list = draw_sprite(list, x1, y0 + 6, 0, 98, v + 6, 0, 0);
    list = draw_set_offsets(list, 1, 1, w - 20, h - 27);
    list = draw_sprite(list, x + 6, y0 + 6, 0, 70, v + 6, 0, 0);
    font_set_size(13, 13);
    i = 0;
    v = 0;
    do {
        tw = w / (3 - i);
        dy = 0;
        w -= tw;
        if (i == 0)
            u0 = 0;
        else
            u0 = 16;
        u1 = i == 2 ? 48 : 32;
        if (i == cl.tab) {
            list = dl_ambient_color(list, 0xFFFFFFFF);
            font_set_color(0xFFFFFFFF);
        } else {
            u0 += 8;
            u1 += 8;
            dy = 2;
            list = dl_ambient_color(list, 0x80FFFFFF);
            font_set_color(0xFF808080);
        }
        list = gim_set_texture(list, tab);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 8, 18, 8, 18);
        list = draw_sprite(list, x, y, 0, u0, v, 0, 0);
        list = draw_sprite(list, x + tw - 8, y, 0, u1, v, 0, 0);
        list = draw_set_offsets(list, 1, 18, tw - 16, 18);
        list = draw_sprite(list, x + 8, y, 0, u0 + 4, v, 0, 0);
        if (i != cl.tab) {
            list = dl_scissor(list, x, y + dy, tw - 8, 13);
            list = dl_enable(list, 2);
        }
        font_text_set(list);
        font_draw_text((x + 4) * 64, (y + 3 + dy) * 64, (tw - 8) * 64, table_get_entry(cl_res_get(CL_STRINGS), i + 0x60));
        list = font_text_clear();
        list = dl_disable(list, 2);
        i++;
        x += tw;
        v += 32;
    } while (i < 3);
    return list;
}

/* one line: card with face/number/name/rank, then the deck stars and the record when shown */
unsigned int *cl_draw_entry(unsigned int *list, int x, int y, int w, int stars, int record, ClEntry *e)
{
    void *parts = cl_res_get(0x1003);
    int xx;

    if (parts == 0)
        return list;
    list = gim_set_texture(list, parts);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 8, 55, 8, 55);
    list = draw_sprite(list, x, y, 0, 0xD4, 0x49, 0, 0);
    list = draw_sprite(list, x + w - 8, y, 0, 0xE4, 0x49, 0, 0);
    list = draw_set_offsets(list, 1, 55, w - 16, 55);
    list = draw_sprite(list, x + 8, y, 0, 0xDC, 0x49, 0, 0);
    list = cl_draw_card(list, x + 6, y + 4, e);
    xx = x + 0x135;
    if (stars) {
        list = cl_draw_stars(list, xx + 3, y + 5, e);
        xx += 0x65;
    }
    if (record)
        list = cl_draw_record(list, xx + 2, y + 5, e);
    return list;
}

/* rank -> string of its name */
unsigned short cl_rank_text[8] = { 0, 4, 5, 6, 7, 8, 9, 0 };

unsigned int *cl_draw_card(unsigned int *list, int x, int y, ClEntry *e)
{
    void *parts = cl_res_get(0x1003);
    int rank;
    int fx;
    int face;
    int lit;

    if (parts == 0)
        return list;
    list = gim_set_texture(list, parts);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    if (!e->simple)
        rank = e->u.info->rank[e->u.info->cur];
    else
        rank = e->u.s.rank;
    list = draw_set_offsets(list, 100, 46, 100, 46);
    list = draw_sprite(list, x, y, 0, 0, 0x51, 0, 0);
    list = draw_set_offsets(list, 1, 30, 1, 30);
    list = draw_sprite(list, x + 100, y, 0, 100, 0x51, 0, 0);
    list = draw_sprite(list, x + 0x12E, y, 0, 0x66, 0x51, 0, 0);
    list = draw_set_offsets(list, 1, 30, 0xC9, 30);
    list = draw_sprite(list, x + 0x65, y, 0, 0x65, 0x51, 0, 0);
    font_set_size(11, 13);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    {
        unsigned short fmt[8] = { 'N', 'o', '.', '%', '0', '3', 'd', 0 };
        unsigned short num[16];
        wsprintf(num, fmt, e->num);
        fx = (x + 0x34) * 64;
        font_draw_text(fx, y * 64, 0xB40, num);
    }
    font_set_color(0xFF000000);
    font_set_size(13, 13);
    font_draw_wtext(fx, (y + 15) * 64, 0xB40, table_get_entry(cl_res_get(CL_STRINGS), 1));
    font_draw_wtext(fx, (y + 32) * 64, 0xB40, table_get_entry(cl_res_get(CL_STRINGS), 2));
    font_set_size(16, 16);
    cl_draw_name(x + 0x6A, y + 13, 200, e->id);
    list = font_text_clear();
    font_set_size(13, 13);
    list = cl_draw_rank(list, x + 100, y + 30, 6, rank);
    list = dl_scissor(list, x + 3, y + 3, 40, 40);
    list = dl_enable(list, 2);
    if (!e->simple) {
        face = e->u.info->face + e->u.info->cur;
        lit = cl.state == 4 && e->id == cl.chosen;   /* the one just picked */
        list = cl_draw_face(list, x + 3, y + 3, face, lit);
        list = dl_disable(list, 2);
        if (e->u.info->cur > 0)
            list = cl_draw_face_arrow(list, x - 6, y + 35, face - 1, 0);
        if (e->u.info->cur < e->u.info->n - 1)
            list = cl_draw_face_arrow(list, x + 37, y + 35, face + 1, 1);
    } else {
        list = cl_draw_portrait(list, e->id, x - 2, y - 4, 100, cl_res_get(0x1006), cl_res_get(0x1007));
        list = dl_disable(list, 2);
    }
    return list;
}

/* the deck level as 1..3 stars; in the story mode the progress to the next level under them */
unsigned int *cl_draw_stars(unsigned int *list, int x, int y, ClEntry *e)
{
    void *parts = cl_res_get(0x1003);
    int i;
    int n;
    int v;
    int w;

    if (parts == 0)
        return list;
    list = gim_set_texture(list, parts);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x62, 0x2E, 0x62, 0x2E);
    list = draw_sprite(list, x, y, 0, 0x40, 0, 0, 0);
    v = deck_slot_get_level(e->id);
    switch (v) {
    case 3:
        n = 3;
        break;
    case 2:
        n = 2;
        break;
    case 1:
        n = 1;
        break;
    default:
        n = 0;
        break;
    }
    for (i = 0; i < n; i++) {
        list = draw_set_offsets(list, 30, 28, 30, 28);
        list = draw_sprite(list, x + i * 0x22, y, 0, 0x40 + i * 0x22, 0x2E, 0, 0);
    }
    if (cl.mode == 1 && value_to_tier(e->id) == 1) {
        v = deck_slot_get_value(e->id);
        w = v * 0x60 / 8000;
        if (w > 0) {
            if (v < 8000) {
                list = dl_scissor(list, x + 1, y + 30, w, 16);
                list = dl_enable(list, 2);
            }
            list = draw_set_offsets(list, 12, 12, 12, 12);
            for (i = 0; i < 8; i++)
                list = draw_sprite(list, x + 1 + i * 12, y + 32, 0, 0xCE, 0, 0, 0);
            list = dl_disable(list, 2);
        }
    } else {
        list = draw_set_offsets(list, 1, 14, 1, 14);
        list = draw_sprite(list, x, y + 30, 0, 0xA4, 0x2E, 0, 0);
        list = draw_sprite(list, x + 0x61, y + 30, 0, 0xA6, 0x2E, 0, 0);
        list = draw_set_offsets(list, 1, 14, 0x60, 14);
        list = draw_sprite(list, x + 1, y + 30, 0, 0xA5, 0x2E, 0, 0);
    }
    return list;
}

/* wins, draws and losses against the duelist */
unsigned int *cl_draw_record(unsigned int *list, int x, int y, ClEntry *e)
{
    void *parts = cl_res_get(0x1003);

    if (parts == 0)
        return list;
    list = gim_set_texture(list, parts);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x29, 0x2E, 0x29, 0x2E);
    list = draw_sprite(list, x, y, 0, 0xA4, 0, 0, 0);
    font_set_size(10, 12);
    font_set_color(0xFF000000);
    font_text_set(list);
    cl_draw_number(x + 0x27, y, sysstate2_rec_get_u16(0, e->id));
    cl_draw_number(x + 0x27, y + 15, sysstate2_rec_get_u16(1, e->id));
    cl_draw_number(x + 0x27, y + 30, sysstate2_rec_get_u16(2, e->id));
    return font_text_clear();
}

#ifdef NON_MATCHING /* o original copia os parâmetros para $s depois do laço de zeragem de ch; o mwcc os copia no prólogo */
/* n (at most 999) right-aligned before x */
void cl_draw_number(int x, int y, int n)
{
    int px;
    unsigned short ch[2] = { 0 };

    n = n > 999 ? 999 : n;
    px = x * 64;
    do {
        px -= 0x200;
        x -= 8;
        ch[0] = n % 10 + '0';
        font_draw_text(px, y * 64, (480 - x) * 64, ch);
        n /= 10;
    } while (n > 0);
}
#endif

/* cursor box; pulses by 2 pixels every 8 frames */
unsigned int *cl_draw_cursor(unsigned int *list, int x, int y, int w, int h)
{
    void *parts = cl_res_get(0x1003);
    int grow;
    int d;
    int y0;
    int x0;
    int y1;
    int x1;
    int ym;

    if (parts == 0)
        return list;
    grow = ((gfx_get_state()->frame >> 3) & 1) * 2;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, parts);
    list = draw_set_offsets(list, 6, 6, 6, 6);
    d = grow / 2;
    y0 = y - 4 - d;
    x0 = x - 4 - d;
    list = draw_sprite(list, x0, y0, 0, 25, 20, 0, 0);
    x1 = x + w - 2 + d;
    list = draw_sprite(list, x1, y0, 0, 40, 20, 0, 0);
    y1 = y + h - 2 + d;
    list = draw_sprite(list, x0, y1, 0, 25, 41, 0, 0);
    list = draw_sprite(list, x1, y1, 0, 40, 41, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, h + grow - 4);
    ym = y + 2 - d;
    list = draw_sprite(list, x0, ym, 0, 25, 26, 0, 0);
    list = draw_sprite(list, x1, ym, 0, 40, 26, 0, 0);
    list = draw_set_offsets(list, 1, 6, w + grow - 4, 6);
    d = x + 2 - d;
    list = draw_sprite(list, d, y0, 0, 31, 20, 0, 0);
    list = draw_sprite(list, d, y1, 0, 31, 41, 0, 0);
    return list;
}

/* duelist name; in Japanese the ruby line above it moves it down */
void cl_draw_name(int x, int y, int w, unsigned short id)
{
    int h;

    if (cfg_get_language() == 0) {
        h = font_line_height();
        font_set_flag8000(1);
        y -= (font_line_height() - h) >> 6;
    }
    font_draw_wtext(x * 64, y * 64, w * 64, chara_get_name(0, id));
    font_set_flag8000(0);
}

/* rank plate, colored by rank */
unsigned int *cl_draw_rank(unsigned int *list, int x, int y, int dx, unsigned short rank)
{
    void *parts = cl_res_get(0x1003);

    if (parts == 0)
        return list;
    list = gim_set_texture(list, parts);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    switch (rank) {
    case 1:
        list = dl_ambient_color(list, 0xFF7E7FFF);
        break;
    case 2:
        list = dl_ambient_color(list, 0xFF87E9F5);
        break;
    case 3:
        list = dl_ambient_color(list, 0xFFFFAFA0);
        break;
    default:
        list = dl_ambient_color(list, 0xFFA7D2E9);
        break;
    }
    list = draw_set_offsets(list, 1, 16, 1, 16);
    list = draw_sprite(list, x, y, 0, 100, 0x6F, 0, 0);
    list = draw_sprite(list, x + 0xCA, y, 0, 0x66, 0x6F, 0, 0);
    list = draw_set_offsets(list, 1, 16, 0xC9, 16);
    list = draw_sprite(list, x + 1, y, 0, 0x65, 0x6F, 0, 0);
    font_text_set(list);
    font_set_size(13, 13);
    font_draw_text((x + dx) * 64, (y + 2) * 64, 0x3200, table_get_entry(cl_res_get(CL_STRINGS), cl_rank_text[rank]));
    return font_text_clear();
}

/* scroll arrow (down = 1 for the lower one), blinking once per `period` frames */
unsigned int *cl_draw_arrow(unsigned int *list, int x, int y, unsigned char down, unsigned int period,
                            int blink, int base, unsigned int time, void *gim)
{
    unsigned int half = period >> 1;
    int v;
    float a;

    if (down == 1)
        v = 8;
    else
        v = 0;
    list = gim_set_texture(list, gim);
    list = dl_set_tex_filter(list, 0, 0);
    if (base) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = draw_set_offsets(list, 8, 8, 8, 8);
        list = draw_sprite(list, x, y, 0, 46, v + 48, 0, 0);
    }
    if (!blink)
        return list;
    time %= period;
    if (time < half)
        a = (float)time / (float)half;
    else
        a = (float)(period - time) / (float)half;
    if (a < 0.0f)
        a = 0.0f;
    if (a > 1.0f)
        a = 1.0f;
    list = dl_ambient_color(list, ((unsigned int)(255.0f * a) << 24) | 0xFFFFFF);
    list = draw_set_offsets(list, 8, 8, 8, 8);
    return draw_sprite(list, x, y, 0, 46, v, 0, 0);
}

/* scroll bar: knob size and position from the position, the window and the total; `anim` picks the
   knob frames */
unsigned int *cl_draw_scrollbar(unsigned int *list, int x, int y, int h, int pos, int page, int total, int anim)
{
    int track = h - 2;         /* becomes the knob offset */
    void *parts = cl_res_get(0x1003);
    int knob;
    int period_up;
    int period_down;
    int k;

    if (parts == 0)
        return list;
    if (total <= page) {
        knob = track;
        track = 0;
    } else {
        knob = ALLEGREX_MAX(page * track / total, 10);
        track = ALLEGREX_MIN(ALLEGREX_MAX(pos * (track - knob) / (total - page), 0), h - 2 - knob);
    }
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, parts);
    list = draw_set_offsets(list, 8, 5, 8, 5);
    list = draw_sprite(list, x, y, 0, 55, 0, 0, 0);
    list = draw_sprite(list, x, y + h - 5, 0, 55, 15, 0, 0);
    list = draw_set_offsets(list, 8, 5, 8, h - 10);
    list = draw_sprite(list, x, y + 5, 0, 55, 5, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, 5);
    k = anim & 1;
    list = draw_sprite(list, x + 2, y + track + 2, 0, 55, (k + 1) * 20, 0, 0);
    list = draw_sprite(list, x + 2, knob + (y + track - 3), 0, 55, k * 20 + 0x23, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, knob - 10);
    list = draw_sprite(list, x + 2, y + track + 7, 0, 55, k * 20 + 0x19, 0, 0);
    period_up = gfx_get_state()->fps;
    list = cl_draw_arrow(list, x, y - 7, 0, period_up, pos > 0, 1, gfx_get_state()->frame, parts);
    period_down = gfx_get_state()->fps;
    return cl_draw_arrow(list, x, y + h + 2, 1, period_down, pos < total - page, 1, gfx_get_state()->frame, parts);
}

unsigned int *cl_draw_header(unsigned int *list)
{
    void *parts = cl_res_get(0x1003);

    if (parts == 0)
        return list;
    list = gim_set_texture(list, parts);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 12, 23, 12, 23);
    list = draw_sprite(list, 0, 0, 0, 0, 20, 0, 0);
    list = draw_sprite(list, 468, 0, 0, 13, 20, 0, 0);
    list = draw_set_offsets(list, 1, 23, 456, 23);
    list = draw_sprite(list, 12, 0, 0, 12, 20, 0, 0);
    font_set_size(14, 14);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    font_draw_text(0x200, 0x100, 0x7600, table_get_entry(cl_res_get(CL_STRINGS), 3));
    return font_text_clear();
}

/* full screen black at `alpha` behind the detail page */
unsigned int *cl_draw_dim(unsigned int *list, unsigned char alpha)
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

/* detail page of the duelist under the cursor */
unsigned int *cl_draw_detail(unsigned int *list)
{
    ClEntry *e = cl_entry(cl.tab, cl.top[cl.tab] + cl.cursor[cl.tab]);
    unsigned short colon[2] = { ':', 0 };
    int x;
    int w;
    int nx;
    unsigned short *text;
    int px;
    int pw;

    list = cl_draw_bustup(list, e);
    x = 8;
    if (cl.anim < 12)
        x = 8 - (12 - cl.anim) * 299 / 12;
    list = cl_draw_detail_frame(list, x, 0x22);
    font_set_size(14, 14);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    text = table_get_entry(cl_res_get(CL_STRINGS), 10);
    px = x + 7;
    pw = (480 - px) * 64;
    px *= 64;
    font_draw_text(px, 0xC00, pw, text);
    w = font_get_cursor_x();
    font_draw_text(px, 0x1000, pw, table_get_entry(cl_res_get(CL_STRINGS), 11));
    if (w < font_get_cursor_x())
        w = font_get_cursor_x();
    font_draw_text(w, 0xBC0, 0x800, colon);
    font_draw_text(w, 0x1000, 0x800, colon);
    nx = font_get_cursor_x() >> 6;
    font_set_size(16, 16);
    cl_draw_name(nx + 7, 0x2E, 200, e->id);
    font_set_size(14, 14);
    font_set_color(0xFF163A4A);
    list = font_text_clear();
    text = e->u.info->text[e->u.info->cur];
    if (text) {
        if (cfg_get_language() == 0)
            font_set_flag8000(1);
        list = dl_scissor(list, 0, 0x55, 480, 0x99);
        list = dl_enable(list, 2);
        font_text_set(list);
        font_draw_wtext_ex((x + 11) * 64, 0x1540 - cl.scroll, 0x4180, cl.scroll_max + 0x2640, text, -1);
        list = font_text_clear();
        list = dl_disable(list, 2);
        list = cl_draw_scrollbar(list, x + 0x112, 0x5B, 0x8D, (unsigned int)cl.scroll, 0x2640, cl.scroll_max + 0x2640, 0);
        if (cfg_get_language() == 0)
            font_set_flag8000(0);
    }
    font_set_color(0xFF000000);
    list = cl_draw_rank(list, nx + 4, 0x3E, 3, e->u.info->rank[e->u.info->cur]);
    if (e->u.info->voice)
        list = cl_draw_voicebar(list, cl.voice);
    return list;
}

#ifdef NON_MATCHING /* o original soma 5/45 a x/y e desloca os sprites do quadro interno a partir deles; o mwcc dobra as somas nas constantes */
unsigned int *cl_draw_detail_frame(unsigned int *list, int x, int y)
{
    void *parts = cl_res_get(0x1003);

    if (parts == 0)
        return list;
    list = gim_set_texture(list, parts);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 6, 6, 6, 6);
    list = draw_sprite(list, x, y, 0, 0xEC, 0x52, 0, 0);
    list = draw_sprite(list, x + 0x11D, y, 0, 0xFA, 0x52, 0, 0);
    list = draw_sprite(list, x, y + 0xD2, 0, 0xEC, 0x60, 0, 0);
    list = draw_sprite(list, x + 0x11D, y + 0xD2, 0, 0xFA, 0x60, 0, 0);
    list = draw_set_offsets(list, 1, 6, 0x117, 6);
    list = draw_sprite(list, x + 6, y, 0, 0xF2, 0x52, 0, 0);
    list = draw_sprite(list, x + 6, y + 0xD2, 0, 0xF2, 0x60, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, 0xCC);
    list = draw_sprite(list, x, y + 6, 0, 0xEC, 0x58, 0, 0);
    list = draw_sprite(list, x + 0x11D, y + 6, 0, 0xFA, 0x58, 0, 0);
    list = draw_set_offsets(list, 1, 1, 0x117, 0xCC);
    list = draw_sprite(list, x + 6, y + 6, 0, 0xF2, 0x58, 0, 0);
    x += 5;                    /* the inner frame */
    y += 0x2D;
    list = draw_set_offsets(list, 6, 6, 6, 6);
    list = draw_sprite(list, x, y, 0, 0xEC, 0x66, 0, 0);
    list = draw_sprite(list, x, y + 0x9F, 0, 0xEC, 0x74, 0, 0);
    list = draw_set_offsets(list, 12, 6, 12, 6);
    list = draw_sprite(list, x + 0x10C, y, 0, 0xF4, 0x66, 0, 0);
    list = draw_sprite(list, x + 0x10C, y + 0x9F, 0, 0xF4, 0x74, 0, 0);
    list = draw_set_offsets(list, 1, 6, 0x106, 6);
    list = draw_sprite(list, x + 6, y, 0, 0xF2, 0x66, 0, 0);
    list = draw_sprite(list, x + 6, y + 0x9F, 0, 0xF2, 0x74, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, 0x99);
    list = draw_sprite(list, x, y + 6, 0, 0xEC, 0x6C, 0, 0);
    list = draw_set_offsets(list, 12, 1, 12, 0x99);
    list = draw_sprite(list, x + 0x10C, y + 6, 0, 0xF4, 0x6C, 0, 0);
    list = draw_set_offsets(list, 1, 1, 0x106, 0x99);
    return draw_sprite(list, x + 6, y + 6, 0, 0xF2, 0x6C, 0, 0);
}
#endif

/* the duelist's bust-up (body + mouth) sliding in with the page, cross-faded on costume change */
unsigned int *cl_draw_bustup(unsigned int *list, ClEntry *e)
{
    void *body = cl_bustup_get(e, 0);
    void *mouth;
    unsigned int opts[24];
    int x0;
    int k;
    int t;

    if (body == 0)
        return list;
    mouth = cl_bustup_get(e, 1);
    memset(opts, 0, sizeof(opts));
    opts[0] = gfx_get_state()->frame;
    opts[3] = 0x10000;
    if (cl.fade) {
        opts[3] = 0x10004;
        opts[19] = cl.alpha;
        opts[18] = 0xFF;
        opts[17] = 0xFF;
        opts[16] = 0xFF;
    }
    x0 = 400;
    k = 250;
    if (e->id == 0x11) {
        x0 = 0x208;
        k = 0x1EF;
    }
    t = cl.anim * k / 12;
    if (t < cl.slide) {
        if (cl.slide - 20 < t)
            cl.slide = t;
        else
            cl.slide = cl.slide - 20;
    } else if (cl.slide < t) {
        if (t < cl.slide + 20)
            cl.slide = t;
        else
            cl.slide = cl.slide + 20;
    }
    ((float *)opts)[5] = x0 - cl.slide;
    list = gfx_begin_2d(list);
    list = ehf_draw(list, body, opts);
    list = ehf_draw(list, mouth, opts);
    return gfx_end_2d(list);
}

/* voice index -> string of its name */
unsigned short cl_voice_text[124] = {
    54, 55, 56, 56, 56, 56, 57, 57, 57, 57, 58, 58, 58, 58, 59, 59, 59, 60, 60, 60,
    60, 61, 61, 61, 61, 61, 61, 62, 63, 63, 63, 63, 64, 64, 64, 65, 65, 65, 66, 66,
    66, 67, 67, 67, 68, 68, 68, 69, 69, 69, 70, 70, 70, 71, 71, 71, 72, 72, 72, 73,
    74, 75, 95, 94, 94, 94, 94, 76, 76, 76, 77, 77, 77, 77, 78, 78, 78, 79, 79, 79,
    80, 80, 80, 81, 81, 81, 81, 82, 82, 83, 83, 84, 84, 84, 84, 84, 84, 84, 84, 85,
    85, 85, 85, 86, 86, 87, 87, 87, 88, 88, 88, 88, 88, 88, 88, 88, 88, 89, 90, 91,
    92, 92, 92, 93,
};

/* voice test bar: number and name of the voice, blinking arrows */
unsigned int *cl_draw_voicebar(unsigned int *list, int voice)
{
    void *gim = cl_res_get(0x1008);
    int i;
    int y;
    int x;

    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    y = 0xFC;
    if (cl.anim < 12)
        y += (12 - cl.anim) * 20 / 12;
    list = draw_set_offsets(list, 64, 20, 64, 20);
    for (i = 0, x = 0; i < 4; i++, x += 64)
        list = draw_sprite(list, x, y, 0, (i & 1) << 6, (i >> 1) * 20, 0, 0);
    list = draw_set_offsets(list, 0x2A, 20, 0x2A, 20);
    list = draw_sprite(list, i * 64, y, 0, 0, 0x28, 0, 0);
    if (gfx_get_state()->frame >> 3 & 1) {
        if (voice > 0) {
            list = draw_set_offsets(list, 6, 10, 6, 10);
            list = draw_sprite(list, 0x65, y + 4, 0, 0x2A, 0x2C, 0, 0);
        }
        if (voice < cl.n_voices - 1) {
            list = draw_set_offsets(list, 6, 10, 6, 10);
            list = draw_sprite(list, 0x121, y + 4, 0, 0x30, 0x2C, 0, 0);
        }
    }
    if (cl.bgm_wait)
        return list;
    font_set_size(11, 14);
    font_set_color(0xFF000000);
    font_text_set(list);
    cl_draw_number(0x60, y + 3, voice + 1);
    font_set_size(14, 14);
    font_draw_wtext(0x1C00, (y + 3) * 64, 0x2B00, table_get_entry(cl_res_get(CL_STRINGS), cl_voice_text[cl.voices[voice]]));
    return font_text_clear();
}

/* face icon (main_face02.gim when lit) */
unsigned int *cl_draw_face(unsigned int *list, int x, int y, int face, int lit)
{
    void *gim;

    if (lit == 0)
        gim = cl_res_get(0x1004);
    else
        gim = cl_res_get(0x1005);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 40, 40, 40, 40);
    return draw_sprite(list, x, y, 0, face % 8 * 40, face / 8 * 40, 0, 0);
}

/* small preview of the previous/next costume with a blinking arrow */
unsigned int *cl_draw_face_arrow(unsigned int *list, int x, int y, int face, int right)
{
    void *parts = cl_res_get(0x1003);
    void *gim;

    if (parts == 0)
        return list;
    list = gim_set_texture(list, parts);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 13, 13, 13, 13);
    list = draw_sprite(list, x, y, 0, 0xE6, 0, 0, 0);
    if (gfx_get_state()->frame >> 3 & 1) {
        if (right == 0) {
            list = draw_set_offsets(list, 8, 11, 8, 11);
            list = draw_sprite(list, x + 12, y + 1, 0, 0xCD, 0x10, 0, 0);
        } else {
            list = draw_set_offsets(list, 8, 11, 8, 11);
            list = draw_sprite(list, x - 8, y + 1, 0, 0xD5, 0x10, 0, 0);
        }
    }
    gim = cl_res_get(0x1004);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    list = dl_set_tex_filter(list, 1, 1);
    list = draw_set_offsets(list, 40, 40, 10, 10);
    return draw_sprite(list, x + 1, y + 1, 0, face % 8 * 40, face / 8 * 40, 0, 0);
}
