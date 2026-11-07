// @range 0x088682AC-0x0886A3F4
// @rodata 0x08872B98-0x08872BE0
// @asm 0x08868B30-0x08869790
// @data 0x0887CBD0-0x0887CF74
/* Duel options window (m_menu_panel / menu_opt textures, option_msg_<lang>.bin captions): four
   groups of rows (auto-advance, speed, volumes, display mode, flags), edited with the d-pad;
   one button keeps the new values, the other restores the saved ones. Opened from the duel menu. */
#include "dd_duelui.h"

typedef struct OptValues {             /* the settings edited by the window, 4 bytes */
    unsigned char auto_on : 1;         /* cfg_is_mode1 */
    unsigned char speed : 2;           /* cfg byte0 bits 4-5 */
    unsigned char mode : 2;            /* cfg bits 1, 3, 4 */
    unsigned char flag0 : 1;           /* cfg flag 0 */
    unsigned char flag3 : 1;           /* cfg flag 3 */
    unsigned char flag2 : 1;           /* cfg flag 2 */
    unsigned char bgm;                 /* volume of channel 2 */
    unsigned char se;                  /* channel 0 */
    unsigned char voice;               /* channel 1 */
} OptValues;

typedef struct Options {               /* 0x30, heap */
    int full;                          /* 0x00 1: all groups, 0: the duel-only rows */
    int cursor;                        /* 0x04 row 0..8 */
    OptValues cur;                     /* 0x08 */
    OptValues saved;                   /* 0x0C */
    unsigned char drawn;               /* 0x10 */
    char pad11[3];
    void *heap;                        /* 0x14 */
    void *pack;                        /* 0x18 */
    void *panel_gim;                   /* 0x1C */
    void *opt_gim;                     /* 0x20 */
    int frame;                         /* 0x24 */
    unsigned int color;                /* 0x28 */
    unsigned int color_sel;            /* 0x2C */
} Options;

typedef struct OptRect {
    short u, v, w, h;
} OptRect;

typedef struct OptRect2 {               /* a pair of pieces, copied whole from a template */
    OptRect s[2];
} OptRect2;

typedef struct OptRectA {               /* OptRect copied as two words (lwc1/swc1) */
    short u, v, w, h;
} __attribute__((aligned(4))) OptRectA;

extern Options *options;               /* 0x08C5B9B8 */
extern int opt_prev_mode;              /* 0x08C5B9BC */
extern int opt_state;                  /* 0x08C5B9C0 */

char opt_gims_a[2][0x40] = { "m_menu_panel01.gim", "menu_opt01.gim" };
char opt_gims_b[2][0x40] = { "m_menu_panel02.gim", "menu_opt02.gim" };
OptRect opt_rects[27] = {
    { 0, 0, 6, 24 }, { 6, 0, 1, 24 }, { 17, 0, 3, 24 }, { 0, 0, 6, 24 }, { 6, 0, 1, 24 }, { 17, 0, 3, 24 },
    { 0, 10, 6, 1 }, { 6, 10, 1, 1 }, { 17, 10, 3, 1 }, { 0, 0, 4, 14 }, { 4, 0, 1, 14 }, { 31, 0, 1, 14 },
    { 10, 12, 1, 1 }, { 48, 0, 8, 12 }, { 56, 0, 8, 12 }, { 32, 16, 6, 12 }, { 38, 16, 6, 12 }, { 32, 0, 4, 4 },
    { 38, 0, 1, 4 }, { 40, 0, 5, 4 }, { 32, 6, 4, 1 }, { 38, 6, 1, 1 }, { 40, 6, 5, 1 }, { 32, 8, 4, 5 },
    { 38, 8, 1, 5 }, { 40, 8, 5, 5 }, { 48, 12, 16, 14 },
};
/* per group: title icon, title color row, number of rows (read one by one by opt_draw) */
int opt_group_icon[4] = { 8, 9, 10, 11 };
unsigned char opt_group_c[4] = { 0, 0, 0, 1 };
short opt_group_rows[4] = { 1, 1, 4, 5 };
OptRect opt_gauge[2] = { { 32, 16, 6, 12 }, { 38, 16, 6, 12 } };
OptRect opt_arrows[2] = { { 48, 0, 8, 12 }, { 56, 0, 8, 12 } };
OptRect opt_switch[2] = { { 0, 32, 32, 13 }, { 0, 48, 32, 13 } };
char opt_msg_files[6][0x40] = {
    "option_msg_j.bin", "option_msg_e.bin", "option_msg_g.bin",
    "option_msg_f.bin", "option_msg_i.bin", "option_msg_s.bin",
};

int cfg_is_mode1(void);
unsigned int cfg_get_byte0_bits4_5(void);
int cfg_get_byte3(int i);
unsigned int cfg_get_bits_1_3_4(void);
unsigned int cfg_get_flag(int which);
void cfg_set_byte0_bit3(int on);
void cfg_set_byte0_bits4_5(int v);
void snd_set_volume(int channel, int vol);
void cfg_set_bits_1_3_4(int v);
void cfg_set_flag(int which, int on);
void *gim_set_texture_sub(void *list, void *gim, int a, int b, int c, int frame);
void *dl_clear_color(void *list, unsigned int c);
void *dl_clear_stencil(void *list, int v);
void *dl_clear_depth(void *list, int v);
void *heap_alloc(void *h, int n);
int heap_set_level(void *h, void *p);
void *ehp_find(void *pack, const char *name, int *size);
int font_draw_wtext(int x, int y, int w, unsigned short *text);
int cfg_get_language(void);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int dd_mode_get(void);
void dd_mode_push(int mode);
void dd_mode_set_flag(int mode);
void *dd_mode_draw_one(void *dl, int mode);
void dd_mode_pop(void);
void *dd_heap_get(void);
void *dd_pack_get(int id);
PadState *dd_pad(void);

void opt_read(OptValues *v);
void *opt_row(void *list, Options *c, int group, int row, unsigned char sel, int x, int y, int w, int h);
void *opt_gauge_draw(void *list, Options *c, unsigned char level, int x, int y);
void *opt_arrows_draw(void *list, Options *c, int x, int y, unsigned char pos, unsigned char max);
void *opt_switch_draw(void *list, Options *c, unsigned char kind, unsigned char on, int x, int y);
void *opt_tex(void *dl, void *gim);
void *opt_text(void *dl, int x, int y, int w, int msg, int center);
void *opt_text_left(void *dl, int x, int y, int w, int msg);

void opt_init(void *heap, void *pack, int full)
{
    Options *o;

    o = options = heap_alloc(heap, sizeof(Options));
    memset(o, 0, sizeof(Options));
    o->heap = heap;
    o->pack = pack;
    o->full = full;
    opt_read(&o->cur);
    opt_read(&o->saved);
    if (full == 0) {
        o->cursor = 0;
        o->color = 0xFFFFFFFF;
        o->color_sel = 0xFF000000;
    } else {
        o->cursor = 3;
        o->color = 0xFF000000;
        o->color_sel = 0xFF000000;
    }
}

void opt_read(OptValues *v)
{
    v->auto_on = cfg_is_mode1();
    v->speed = cfg_get_byte0_bits4_5();
    v->bgm = cfg_get_byte3(2);
    v->se = cfg_get_byte3(0);
    v->voice = cfg_get_byte3(1);
    v->mode = cfg_get_bits_1_3_4();
    v->flag2 = cfg_get_flag(2);
    v->flag0 = cfg_get_flag(0);
    v->flag3 = cfg_get_flag(3);
}

void opt_write(OptValues *v)
{
    cfg_set_byte0_bit3(v->auto_on);
    cfg_set_byte0_bits4_5(v->speed);
    snd_set_volume(2, v->bgm);
    snd_set_volume(0, v->se);
    snd_set_volume(1, v->voice);
    cfg_set_bits_1_3_4(v->mode);
    cfg_set_flag(2, v->flag2);
    cfg_set_flag(0, v->flag0);
    cfg_set_flag(3, v->flag3);
}

void opt_free(void)
{
    heap_set_level(options->heap, options);
}

/* 1 while open, 2 when closed (the same code as tcfg_update of libtitle) */
int opt_update(void)
{
    Options *c = options;
    OptValues *v = &options->cur;
    PadState *pad = pad_get();
    unsigned int rep;
    unsigned int trig;
    int first;

    c->drawn = 1;
    first = 0;
    if (c->full == 1)
        first = 3;
    rep = pad->b.rep;
    trig = pad->b.trig;
    if (rep & region_pick_value(0)) {
        se_play(0x6213);
        opt_write(v);
        return 2;
    }
    if (rep & region_pick_value(1)) {
        se_play(0x6214);
        c->cur = c->saved;
        opt_write(v);
        return 2;
    }
    if (rep & 0x80) {
        switch (c->cursor) {
        case 0:
            se_play(0x6212);
            v->auto_on = 1;
            break;
        case 1:
            if (v->speed != 0) {
                se_play(0x6212);
                v->speed--;
            }
            break;
        case 2:
            if (v->bgm != 0) {
                se_play(0x6218);
                v->bgm--;
            }
            snd_set_volume(2, v->bgm);
            break;
        case 3:
            if (v->se != 0) {
                se_play(0x6212);
                v->se--;
            }
            snd_set_volume(0, v->se);
            break;
        case 4:
            if (v->voice != 0) {
                se_play(0x6212);
                v->voice--;
            }
            snd_set_volume(1, v->voice);
            break;
        case 5:
            if (v->mode != 0) {
                se_play(0x6212);
                v->mode--;
            }
            break;
        case 6:
            se_play(0x6212);
            v->flag0 = 1;
            break;
        case 7:
            se_play(0x6212);
            v->flag2 = 1;
            break;
        case 8:
            se_play(0x6212);
            v->flag3 = 1;
            break;
        }
    } else if (rep & 0x20) {
        switch (c->cursor) {
        case 0:
            se_play(0x6212);
            v->auto_on = 0;
            break;
        case 1:
            if (v->speed < 2) {
                se_play(0x6212);
                v->speed++;
            }
            break;
        case 2:
            if (v->bgm < 15) {
                se_play(0x6218);
                v->bgm++;
            }
            snd_set_volume(2, v->bgm);
            break;
        case 3:
            if (v->se < 15) {
                se_play(0x6212);
                v->se++;
            }
            snd_set_volume(0, v->se);
            break;
        case 4:
            if (v->voice < 15) {
                se_play(0x6212);
                v->voice++;
            }
            snd_set_volume(1, v->voice);
            break;
        case 5:
            if (v->mode < 3) {
                se_play(0x6212);
                v->mode++;
            }
            break;
        case 6:
            se_play(0x6212);
            v->flag0 = 0;
            break;
        case 7:
            se_play(0x6212);
            v->flag2 = 0;
            break;
        case 8:
            se_play(0x6212);
            v->flag3 = 0;
            break;
        }
    } else if ((trig & 0x10) || (rep & 0x10)) {
        if (c->cursor > first) {
            if (c->cursor == 3) {
                se_play(0x6212);
                c->cursor -= 2;
            } else {
                se_play(0x6212);
                c->cursor--;
            }
        } else if (trig & 0x10) {
            se_play(0x6212);
            c->cursor = 8;
        }
    } else if ((trig & 0x40) || (rep & 0x40)) {
        if (c->cursor < 8) {
            se_play(0x6212);
            c->cursor++;
            if (cfg_get_language() != 0 && c->cursor == 2)
                c->cursor++;
        } else if (trig & 0x40) {
            se_play(0x6212);
            c->cursor = first;
        }
    }
    c->frame++;
    c->frame %= 0x1000000;
    return 1;
}

#ifdef NON_MATCHING /* gerado de tcfg_draw (libtitle, mesmo código): ver o comentário lá */
/* the window: per group a frame of three by three pieces, a title strip, the cursor and the rows;
   at the bottom a nine-piece help box and the help line of the current row */
void *opt_draw(void *list)
{
    int piece;
    Options *c = options;
    int first = 0;
    char gims[2][64] = { "m_menu_panel01.gim", "menu_opt01.gim" };
    char gims_alt[2][64] = { "m_menu_panel02.gim", "menu_opt02.gim" };
    OptRect spr[27] = {
        { 0, 0, 6, 24 }, { 6, 0, 1, 24 }, { 17, 0, 3, 24 },
        { 0, 0, 6, 24 }, { 6, 0, 1, 24 }, { 17, 0, 3, 24 },
        { 0, 10, 6, 1 }, { 6, 10, 1, 1 }, { 17, 10, 3, 1 },
        { 0, 0, 4, 14 }, { 4, 0, 1, 14 }, { 31, 0, 1, 14 },
        { 10, 12, 1, 1 }, { 48, 0, 8, 12 }, { 56, 0, 8, 12 },
        { 32, 16, 6, 12 }, { 38, 16, 6, 12 }, { 32, 0, 4, 4 },
        { 38, 0, 1, 4 }, { 40, 0, 5, 4 }, { 32, 6, 4, 1 },
        { 38, 6, 1, 1 }, { 40, 6, 5, 1 }, { 32, 8, 4, 5 },
        { 38, 8, 1, 5 }, { 40, 8, 5, 5 }, { 48, 12, 16, 14 },
    };
    int style[4] = { 8, 9, 10, 11 };
    /* piece being drawn (the switches below leave them as they were for an index none of their cases covers) */
    short x = 131;
    short y = 34;
    short rw = 218;
    short rh = 17;
    short sel_group = 0;
    unsigned char tab[4] = { 0, 0, 0, 1 };
    short n = 0;
    short top[4];
    short rows[4] = { 1, 1, 4, 5 };
    unsigned char cur = c->cursor;
    int found = 0;
    short gx[4];
    short gy[4];
    int g, i, k;
    short sel_row;
    unsigned char sel;
    short pieces;
    short u, v;
    short base_x, base_y, mw, mh;

    cfg_get_language();
    if (c->full == 1)
        first = 2;
    if (cfg_get_language() != 0) {
        rows[2] = 3;
        if (c->cursor > 2)
            cur = c->cursor - 1;
    }
    if (c->full == 1) {
        c->panel_gim = ehp_find(c->pack, gims_alt[0], 0);
        c->opt_gim = ehp_find(c->pack, gims_alt[1], 0);
    } else {
        c->panel_gim = ehp_find(c->pack, gims[0], 0);
        c->opt_gim = ehp_find(c->pack, gims[1], 0);
    }
    /* group and row of the cursor */
    for (g = 0; g < 4; g++) {
        if (rows[g] < 2) {
            sel_row = 0;
            if (cur == n)
                break;
            sel_row++;
            n++;
        } else {
            sel_row = 1;
            for (k = 0; k < rows[g] - 1; k++) {
                if (cur != n) {
                    sel_row++;
                    n++;
                    continue;
                }
                found = 1;
                break;
            }
        }
        if (found)
            break;
        sel_group++;
    }
    /* group positions */
    for (g = first; g < 4; g++) {
        if (g == first)
            top[g] = 0;
        else
            top[g] = top[g - 1] + rows[g - 1];
        gx[g] = 131;
        gy[g] = g * 6 + top[g] * 17 + 34;
        if (cfg_get_language() != 0)
            gy[g] = g * 6 + top[g] * 17 + 42;
    }
    for (; first < 4; first++) {
        pieces = rows[first] == 1 ? 3 : 9;

        /* frame */
        for (piece = 0; piece < pieces; piece++) {
            u = spr[piece].u + style[first] % 6 * 20;
            v = spr[piece].v + style[first] / 6 * 24;
            list = opt_tex(list, c->panel_gim);
            switch (piece % 3) {
            case 0:
                rw = spr[piece].w;
                x = spr[piece].u + 131;
                break;
            case 1:
                rw = 218 - (spr[0].w + spr[2].w);
                x = spr[piece].u + 131;
                break;
            case 2:
                rw = spr[piece].w;
                x = 349 - rw;
                break;
            }
            switch (piece / 3) {
            case 0:
                y = gy[first];
                rh = spr[piece].h;
                break;
            case 2:
                y = gy[first] + 10;
                rh = rows[first] * 17 - 10;
                break;
            case 1:
                rh = spr[piece].h;
                y = gy[first] + (rows[first] - 1) * 17;
                break;
            }
            list = draw_set_offsets(list, spr[piece].w, spr[piece].h, rw, rh);
            list = draw_sprite(list, x, y, 0, u, v, 0, 0);
        }
        /* tab of the group title */
        if (rows[first] > 1) {
            y = gy[first] + 4;
            for (i = 0; i < 3; i++) {
                u = spr[(unsigned int)(9 + i)].u;
                v = tab[first] * 16 + spr[(unsigned int)(9 + i)].v;
                list = opt_tex(list, c->opt_gim);
                switch (i % 3) {
                case 0:
                    rw = spr[(unsigned int)(9 + i)].w;
                    x = gx[first] + 4 + u;
                    break;
                case 1:
                    rw = 209 - (spr[9].w + spr[11].w);
                    x = gx[first] + 4 + u;
                    break;
                case 2:
                    rw = spr[(unsigned int)(9 + i)].w;
                    x = gx[first] + 213 - rw;
                    break;
                }
                rh = spr[(unsigned int)(9 + i)].h;
                list = draw_set_offsets(list, spr[(unsigned int)(9 + i)].w, spr[(unsigned int)(9 + i)].h, rw, rh);
                list = draw_sprite(list, x, y, 0, u, v, 0, 0);
            }
        }
        /* cursor bar */
        if (c->drawn && first == sel_group) {
            x = gx[first] + 4;
            y = gy[first] + 4 + sel_row * 17;
            rw = 209;
            rh = 14;
            list = opt_tex(list, c->panel_gim);
            list = draw_set_offsets(list, spr[12].w, spr[12].h, rw, rh);
            list = draw_sprite(list, x, y, 0, spr[12].u, spr[12].v, 0, 0);
        }
        for (i = 0; i < rows[first]; i++) {
            if (first == sel_group && i == sel_row)
                sel = 1;
            else
                sel = 0;
            list = opt_row(list, c, first, i, sel, gx[first], gy[first], 218, rows[first] * 17);
        }
    }
    /* help box at the bottom: 3x3 pieces; the stretched middle uses the pieces beside it in the template
       (s[-1], s[1]) for both axes, as the original does */
    base_x = spr[17].w + 8;
    base_y = spr[17].h + 246;
    mw = 0;
    mh = 0;
    for (i = 0; i < 9; i++) {
        u = spr[(unsigned int)(17 + i)].u;
        v = spr[(unsigned int)(17 + i)].v;
        switch (i) {
        case 0: case 3: case 6:
            rw = spr[(unsigned int)(17 + i)].w;
            x = 8;
            break;
        }
        switch (i) {
        case 1: case 4: case 7:
            x = base_x;
            mw = 462 - spr[(unsigned int)(17 + i) - 1].w - spr[(unsigned int)(17 + i) + 1].w;
            if (mw < 0)
                mw = 0;
            rw = mw;
            break;
        }
        switch (i) {
        case 2: case 5: case 8:
            rw = spr[(unsigned int)(17 + i)].w;
            x = base_x + mw;
            break;
        }
        switch (i) {
        case 0: case 1: case 2:
            rh = spr[(unsigned int)(17 + i)].h;
            y = 246;
            break;
        }
        switch (i) {
        case 3: case 4: case 5:
            y = base_y;
            mh = 20 - spr[(unsigned int)(17 + i) - 1].h - spr[(unsigned int)(17 + i) + 1].h;
            if (mh < 0)
                mh = 0;
            rh = mh;
            break;
        }
        switch (i) {
        case 6: case 7: case 8:
            rh = spr[(unsigned int)(17 + i)].h;
            y = base_y + mh;
            break;
        }
        list = opt_tex(list, c->opt_gim);
        list = draw_set_offsets(list, spr[(unsigned int)(17 + i)].w, spr[(unsigned int)(17 + i)].h, rw, rh);
        list = draw_sprite(list, x, y, 0, u, v, 0, 0);
    }
    u = spr[26].u;
    v = spr[26].v;
    rw = spr[26].w;
    rh = spr[26].h;
    list = opt_tex(list, c->opt_gim);
    list = draw_set_offsets(list, rw, rh, rw, rh);
    list = draw_sprite(list, 12, 250, 0, u, v, 0, 0);
    k = c->cursor + 19;
    switch (c->cursor) {
    case 3:
        k = 23;
        break;
    case 4:
        k = 22;
        break;
    }
    font_set_size(13, 13);
    font_set_color(0xFF000000);
    font_set_flag4(1);
    list = opt_text_left(list, 29, 251, 436, k);
    c->drawn = 0;
    return list;
}
#else
void *opt_draw(void *list);
#endif

/* one row: caption, then its control (gauge, arrows with a value caption, or an on/off switch) */
void *opt_row(void *list, Options *c, int group, int row, unsigned char sel, int x, int y, int w, int h)
{
    OptValues *v;
    int kind;
    unsigned int color;
    int ty;
    int id;
    int tx;
    unsigned char level;
    unsigned char sel2;
    unsigned char on;
    unsigned char max;
    unsigned char val;
    int cx;

    v = &c->cur;
    level = 0;
    kind = 0;
    if (row == 0) {
        tx = x + 8;
        ty = y + 5;
    } else {
        tx = x + 16;
        ty = y + 5 + row * 17;
    }
    switch (group) {
    case 0:
        id = row + 1;
        w = 120;
        break;
    case 1:
        id = row + 2;
        w = 120;
        break;
    case 2:
        id = row + 3;
        w = 202;
        if (cfg_get_language() != 0 && id > 3)
            id++;
        if (id > 3)
            w = 75;
        break;
    case 3:
        w = 112;
        switch (row) {
        default:
            id = 7;
            w = 202;
            break;
        case 1:
            id = 8;
            break;
        case 2:
            id = 9;
            break;
        case 3:
            id = 11;
            break;
        case 4:
            id = 18;
            break;
        }
        break;
    }
    if (sel && c->drawn)
        color = c->color_sel;
    else
        color = c->color;
    font_set_size(13, 13);
    font_set_color(color);
    font_set_flag4(1);
    list = opt_text_left(list, tx, ty, w, id);
    switch (group) {
    case 0:
        kind = 1;
        break;
    case 1:
        kind = 2;
        break;
    case 2:
        if (row != 0)
            kind = 3;
        break;
    case 3:
        if (row == 1)
            kind = 2;
        else if (row > 1)
            kind = 1;
        break;
    }
    font_set_flag4(1);
    switch (kind) {
    case 0:
        break;
    case 1:
        cx = x + 137;
        if (group == 0) {
            sel2 = 1;
            on = v->auto_on;
        } else {
            sel2 = 0;
            switch (row) {
            case 2:
                on = v->flag0;
                break;
            case 3:
                on = v->flag2;
                break;
            case 4:
                on = v->flag3;
                break;
            }
        }
        list = opt_switch_draw(list, c, sel2, on, cx, ty);
        cx = x + 136;
        list = opt_text(list, cx, ty, 30, 12, 1);
        list = opt_text(list, cx + 35, ty, 30, 13, 1);
        break;
    case 2:
        max = 3;
        if (group == 1) {
            max = 2;
            val = v->speed;
            id = val + 15;
        } else {
            val = v->mode;
            id = val + 14;
        }
        cx = x + 144;
        list = opt_text(list, cx, ty, 54, id, 1);
        if (sel)
            list = opt_arrows_draw(list, c, cx - 12, ty, val, max);
        break;
    case 3:
        cx = x + 94;
        if (cfg_get_language() != 0) {
            switch (row) {
            case 1:
                level = v->se;
                break;
            case 2:
                level = v->voice;
                break;
            }
        } else {
            switch (row) {
            case 1:
                level = v->bgm;
                break;
            case 2:
                level = v->se;
                break;
            case 3:
                level = v->voice;
                break;
            }
        }
        list = opt_gauge_draw(list, c, level, cx, ty);
        break;
    }
    return list;
}

/* volume gauge: 15 cells, the first level ones lit */
void *opt_gauge_draw(void *list, Options *c, unsigned char level, int x, int y)
{
    OptRect s[2];
    OptRectA cur;
    OptRect off;
    short w;
    short h;
    short sx;
    short sy;
    short i;

    *(OptRect2 *)s = *(OptRect2 *)opt_gauge;
    sy = y;
    w = s[0].w;
    h = s[0].h;
    off = ((OptRect2 *)s)->s[1];
    for (i = 0; i < 15; i++) {
        sx = x + i * 7;
        if (i < (unsigned char)level)
            cur = *(OptRectA *)&s[0];
        else
            cur = *(OptRectA *)&off;
        list = opt_tex(list, c->opt_gim);
        list = draw_set_offsets(list, cur.w, cur.h, w, h);
        list = draw_sprite(list, sx, sy, 0, cur.u, cur.v, 0, 0);
    }
    return list;
}

/* blinking left/right arrows, hidden at the ends */
void *opt_arrows_draw(void *list, Options *c, int x, int y, unsigned char pos, unsigned char max)
{
    OptRect s[2];
    OptRectA cur;
    OptRect off;
    unsigned char a;
    short w, h;
    short sx, sy;
    int show;
    short i;

    *(OptRect2 *)s = *(OptRect2 *)opt_arrows;
    sy = y;
    w = s[0].w;
    h = s[0].h;
    off = ((OptRect2 *)s)->s[1];
    for (i = 0; i < 2; i++) {
        show = 1;
        sx = x + i * 68;
        if (!i) {
            cur = *(OptRectA *)&s[0];
            if ((unsigned char)pos == 0)
                show = 0;
        } else {
            cur = *(OptRectA *)&off;
            if ((unsigned char)pos == (unsigned char)max)
                show = 0;
        }
        if (show) {
            a = c->frame % 16 / 8 ? 0xFF : 0;
            list = opt_tex(list, c->opt_gim);
            list = dl_color(list, a << 24 | 0xFFFFFF);
            list = draw_set_offsets(list, cur.w, cur.h, w, h);
            list = draw_sprite(list, sx, sy, 0, cur.u, cur.v, 0, 0);
        }
    }
    return list;
}

/* on/off switch */
void *opt_switch_draw(void *list, Options *c, unsigned char kind, unsigned char on, int x, int y)
{
    OptRect s[2];
    OptRectA cur;
    short w, h;
    short sx;
    short sy;

    *(OptRect2 *)s = *(OptRect2 *)opt_switch;
    sy = y;
    w = s[0].w;
    h = s[0].h;
    sx = x - 2;
    if (kind)
        cur = *(OptRectA *)&s[0];
    else
        *(OptRect *)&cur = s[1];
    if ((unsigned char)on == 0)
        sx = x + 33;
    list = opt_tex(list, c->opt_gim);
    list = draw_set_offsets(list, cur.w, cur.h, w, h);
    return draw_sprite(list, sx, sy, 0, cur.u, cur.v, 0, 0);
}

void *opt_tex(void *dl, void *gim)
{
    dl = dl_clear_color(dl, 0);
    dl = dl_clear_stencil(dl, 0);
    dl = dl_clear_depth(dl, 0);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = dl_enable(dl, 9);
    dl = dl_disable(dl, 1);
    dl = dl_disable(dl, 10);
    dl = dl_enable(dl, 4);
    dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
    dl = dl_color(dl, 0xFFFFFFFF);
    return gim_set_texture(dl, gim);
}

/* caption msg of option_msg_<language>.bin; center: centered in w when narrower */
typedef struct { char s[6][0x40]; } OptNames;

void *opt_text(void *dl, int x, int y, int w, int msg, int center)
{
    Options *o = options;
    OptNames names = *(OptNames *)opt_msg_files;
    unsigned short *text;
    int tw;

    text = table_get_entry(ehp_find(o->pack, names.s[cfg_get_language()], 0), msg);
    if (center != 0) {
        tw = font_measure_text(text) >> 6;
        if (tw < w) {
            int d = w - tw;

            w = tw;
            x += d / 2;
        }
    }
    font_text_set(dl);
    font_draw_wtext(x << 6, y << 6, (w + 2) << 6, text);
    return font_text_clear();
}

void *opt_text_left(void *dl, int x, int y, int w, int msg)
{
    return opt_text(dl, x, y, w, msg, 0);
}

int opt_open(void)
{
    int m = dd_mode_get();

    switch (m) {
    case 6:
    case 9:
    case 5:
        return 0;
    }
    opt_prev_mode = dd_mode_get();
    dd_mode_push(9);
    dd_mode_set_flag(0);
    return 1;
}

void opt_start(void)
{
    opt_state = 1;
    fade_start_in(1, 0xFF000000, 16);
    opt_init(dd_heap_get(), dd_pack_get(0x80001), 1);
}

void opt_term(void)
{
    opt_free();
}

void opt_draw_layer(void *dl)
{
    opt_draw(dd_mode_draw_one(dl, opt_prev_mode));
}

void opt_tick(void)
{
    int r;

    switch (opt_state) {
    case 1:
    case 2:
        r = opt_update();
        font_set_flag8000(0);
        if (r == 2)
            opt_state = 3;
        break;
    case 3:
        if (((int *)dd_pad())[6] == 0) {
            opt_state = 0;
            dd_mode_pop();
        }
        break;
    }
}

int opt_is_open(void)
{
    return dd_mode_get() == 9;
}

void opt_close(void)
{
    if (dd_mode_get() == 9)
        dd_mode_pop();
}
