// @range 0x088682AC-0x0886A3F4
// @asm 0x08868524-0x08868B30
// @asm 0x08868B30-0x08869790
// @asm 0x08869790-0x08869BE4
// @asm 0x08869BE4-0x08869D34
// @asm 0x08869D34-0x08869F04
// @asm 0x08869F04-0x0886A010
// @asm 0x0886A0C4-0x0886A1F4
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
    unsigned int frame;                /* 0x24 */
    unsigned int color;                /* 0x28 */
    unsigned int color_sel;            /* 0x2C */
} Options;

typedef struct OptRect {
    short u, v, w, h;
} OptRect;

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
int func_0881CB84(void);
void func_0881CB90(int mode);
void func_0881D044(int mode);
void *func_0881CD80(void *dl, int mode);
void func_0881CC60(void);
void *func_0881CA74(void);
void *func_088045C0(int id);
PadState *func_0881DE50(void);

void opt_read(OptValues *v);
void *opt_row(void *dl, Options *o, int group, int row, int sel, int x, int y, int w);
void *opt_gauge_draw(void *dl, Options *o, unsigned int level, short x, short y);
void *opt_arrows_draw(void *dl, Options *o, short x, short y, int pos, int last);
void *opt_switch_draw(void *dl, Options *o, int on, int which, short x, short y);
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

/* 1 while open, 2 when closed */
#ifdef NON_MATCHING /* cópia de modelos locais e alocação diferem; não insistido */
int opt_update(void)
{
    Options *o = options;
    OptValues *v = &options->cur;
    PadState *pad = pad_get();
    int first;
    unsigned int rep, trig;

    o->drawn = 1;
    first = 0;
    if (o->full == 1)
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
        o->cur = o->saved;
        opt_write(v);
        return 2;
    }
    if (rep & 0x80) {
        switch (o->cursor) {
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
        switch (o->cursor) {
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
        if (first < o->cursor) {
            se_play(0x6212);
            if (o->cursor == 3)
                o->cursor -= 2;
            else
                o->cursor--;
        } else if (trig & 0x10) {
            se_play(0x6212);
            o->cursor = 8;
        }
    } else if ((trig & 0x40) || (rep & 0x40)) {
        if (o->cursor < 8) {
            se_play(0x6212);
            o->cursor++;
            if (cfg_get_language() != 0 && o->cursor == 2)
                o->cursor = 3;
        } else if (trig & 0x40) {
            se_play(0x6212);
            o->cursor = first;
        }
    }
    o->frame++;
    if ((int)o->frame < 0 && (o->frame & 0xFFFFFF) == 0)
        o->frame = 0;
    else
        o->frame &= 0xFFFFFF;
    return 1;
}
#else
int opt_update(void);
#endif

#ifdef NON_MATCHING /* função grande de desenho (layout em três fatias); não tentada a fundo */
/* the window: per group a frame of three by three pieces, a title icon, the rows and the cursor;
   at the bottom the help line of the current row */
void opt_draw(void *dl)
{
    Options *o = options;
    char gims_a[2][0x40];
    char gims_b[2][0x40];
    OptRect r[27];
    int icon[4];
    unsigned char c[4];
    short rows[4];
    short gy[4], gx[4], gyy[4], gstart[4];
    int group, start, cur, row, n, i, k, msg;
    short sel = 0;
    int found = 0;
    short x, y, w, h;

    memcpy(gims_a, opt_gims_a, sizeof(gims_a));
    memcpy(gims_b, opt_gims_b, sizeof(gims_b));
    memcpy(r, opt_rects, sizeof(r));
    memcpy(icon, opt_group_icon, sizeof(icon));
    memcpy(c, opt_group_c, sizeof(c));
    memcpy(rows, opt_group_rows, sizeof(rows));
    cur = o->cursor & 0xFF;
    cfg_get_language();
    start = o->full == 1 ? 2 : 0;
    if (cfg_get_language() != 0) {
        rows[2] = 3;
        if (o->cursor > 2)
            cur = (o->cursor - 1) & 0xFF;
    }
    if (o->full == 1) {
        o->panel_gim = ehp_find(o->pack, gims_b[0], 0);
        o->opt_gim = ehp_find(o->pack, gims_b[1], 0);
    } else {
        o->panel_gim = ehp_find(o->pack, gims_a[0], 0);
        o->opt_gim = ehp_find(o->pack, gims_a[1], 0);
    }
    /* group and row of the cursor */
    k = 0;
    for (group = 0; group < 4; group++) {
        if (rows[group] < 2) {
            sel = 0;
            if (cur == k)
                break;
            sel = 1;
            k++;
        } else {
            sel = 1;
            for (i = 0; i < rows[group] - 1; i++) {
                if (cur == k) {
                    found = 1;
                    break;
                }
                sel++;
                k++;
            }
        }
        if (found)
            break;
    }
    /* layout of the groups */
    for (i = start; i < 4; i++) {
        gstart[i] = i == start ? 0 : gstart[i - 1] + rows[i - 1];
        gx[i] = 131;
        gy[i] = i * 6 + gstart[i] * 17 + (cfg_get_language() != 0 ? 42 : 34);
    }
    for (i = start; i < 4; i++) {
        n = rows[i] == 1 ? 3 : 9;
        for (k = 0; k < n; k++) {
            OptRect *p = &r[k];

            dl = opt_tex(dl, o->panel_gim);
            switch (k % 3) {
            case 0: x = p->u + 131; w = p->w; break;
            case 1: x = p->u + 131; w = 218 - (r[0].w + r[2].w); break;
            case 2: x = 349 - p->w; w = p->w; break;
            }
            switch (k / 3) {
            case 0: y = gy[i]; h = p->h; break;
            case 1: y = gy[i] + 10; h = rows[i] * 17 - 10; break;
            default: y = gy[i] + (rows[i] - 1) * 17; h = p->h; break;
            }
            dl = draw_set_offsets(dl, p->w, p->h, w, h);
            dl = draw_sprite(dl, x, y, 0, p->u + icon[i] % 6 * 20, p->v + icon[i] / 6 * 24, 0, 0);
        }
        if (rows[i] > 1) {
            for (k = 0; k < 3; k++) {
                OptRect *p = &r[9 + k];

                dl = opt_tex(dl, o->opt_gim);
                switch (k % 3) {
                case 0: x = gx[i] + 4 + p->u; w = p->w; break;
                case 1: x = gx[i] + 4 + p->u; w = 209 - (r[18].w + r[26].w); break;
                default: x = gx[i] + 213 - p->w; w = p->w; break;
                }
                dl = draw_set_offsets(dl, p->w, p->h, w, p->h);
                dl = draw_sprite(dl, x, gy[i] + 4, 0, p->u, p->v + c[i] * 16, 0, 0);
            }
        }
        if (o->drawn && i == group) {
            dl = opt_tex(dl, o->panel_gim);
            dl = draw_set_offsets(dl, r[26].w, r[26].h, 209, 14);
            dl = draw_sprite(dl, gx[i] + 4, gy[i] + 4 + sel * 17, 0, r[26].u, r[26].v, 0, 0);
        }
        for (k = 0; k < rows[i]; k++)
            dl = opt_row(dl, o, i, k, i == group && k == sel, gx[i], gy[i], 218);
    }
    /* help line */
    dl = opt_tex(dl, o->opt_gim);
    dl = draw_set_offsets(dl, r[20].w, r[20].h, r[20].w, r[20].h);
    dl = draw_sprite(dl, 12, 250, 0, r[19].u, r[19].v, 0, 0);
    msg = o->cursor + 19;
    if (o->cursor == 4)
        msg = 22;
    else if (o->cursor == 3)
        msg = 23;
    font_set_size(13, 13);
    font_set_color(0xFF000000);
    font_set_flag4(1);
    opt_text_left(dl, 29, 251, 436, msg);
    o->drawn = 0;
}
#else
void opt_draw(void *dl);
#endif

/* one row: caption, then its control (gauge, arrows with a value caption, or an on/off switch) */
#ifdef NON_MATCHING /* cópia de modelos locais e alocação diferem; não insistido */
void *opt_row(void *dl, Options *o, int group, int row, int sel, int x, int y, int w)
{
    OptValues *v = &o->cur;
    unsigned char level = 0;
    unsigned char on;
    int tx, ty, msg, kind, which, last, pos;
    unsigned int color;

    kind = 0;
    if (row == 0) {
        tx = x + 8;
        ty = y + 5;
    } else {
        tx = x + 16;
        ty = y + 5 + row * 17;
    }
    switch (group) {
    case 3:
        w = 0x70;
        if (row == 4)
            msg = 18;
        else if (row == 3)
            msg = 11;
        else if (row == 2)
            msg = 9;
        else if (row == 1)
            msg = 8;
        else {
            msg = 7;
            w = 0xCA;
        }
        break;
    case 2:
        msg = row + 3;
        w = 0xCA;
        if (cfg_get_language() != 0 && msg > 3)
            msg = row + 4;
        if (msg > 3)
            w = 0x4B;
        break;
    case 1:
        msg = row + 2;
        w = 0x78;
        break;
    case 0:
        msg = row + 1;
        w = 0x78;
        break;
    }
    if (!sel || !o->drawn)
        color = o->color;
    else
        color = o->color_sel;
    font_set_size(13, 13);
    font_set_color(color);
    font_set_flag4(1);
    dl = opt_text_left(dl, tx, ty, w, msg);
    switch (group) {
    case 3:
        if (row == 1)
            kind = 2;
        else if (row > 1)
            kind = 1;
        break;
    case 2:
        if (row != 0)
            kind = 3;
        break;
    case 1:
        kind = 2;
        break;
    case 0:
        kind = 1;
        break;
    }
    font_set_flag4(1);
    if (kind == 3) {
        if (cfg_get_language() == 0) {
            if (row == 3)
                level = v->voice;
            else if (row == 2)
                level = v->se;
            else if (row == 1)
                level = v->bgm;
        } else if (row == 2) {
            level = v->voice;
        } else if (row == 1) {
            level = v->se;
        } else {
            level = 0;
        }
        dl = opt_gauge_draw(dl, o, level, x + 0x5E, ty);
    } else if (kind == 2) {
        last = 3;
        if (group == 1) {
            last = 2;
            pos = v->speed;
            msg = pos + 15;
        } else {
            pos = v->mode;
            msg = pos + 14;
        }
        dl = opt_text(dl, x + 0x90, ty, 0x36, msg, 1);
        if (sel)
            dl = opt_arrows_draw(dl, o, x + 0x84, ty, pos, last);
    } else if (kind == 1) {
        if (group == 0) {
            which = 1;
            on = v->auto_on;
        } else {
            which = 0;
            if (row == 4)
                on = v->flag3;
            else if (row == 3)
                on = v->flag2;
            else if (row == 2)
                on = v->flag0;
        }
        dl = opt_switch_draw(dl, o, which, on, x + 0x89, ty);
        dl = opt_text(dl, x + 0x88, ty, 0x1E, 12, 1);
        dl = opt_text(dl, x + 0xAB, ty, 0x1E, 13, 1);
    }
    return dl;
}
#else
void *opt_row(void *dl, Options *o, int group, int row, int sel, int x, int y, int w);
#endif

/* volume gauge: 15 cells, the first level ones lit */
#ifdef NON_MATCHING /* cópia de modelos locais e alocação diferem; não insistido */
void *opt_gauge_draw(void *dl, Options *o, unsigned int level, short x, short y)
{
    OptRect r[2];
    OptRect *p;
    short i;

    r[0] = opt_gauge[0];
    r[1] = opt_gauge[1];
    for (i = 0; i < 15; i++) {
        p = i < (int)(level & 0xFF) ? &r[0] : &r[1];
        dl = opt_tex(dl, o->opt_gim);
        dl = draw_set_offsets(dl, p->w, p->h, r[0].w, r[0].h);
        dl = draw_sprite(dl, x, y, 0, p->u, p->v, 0, 0);
        x += 7;
    }
    return dl;
}
#else
void *opt_gauge_draw(void *dl, Options *o, unsigned int level, short x, short y);
#endif

/* blinking left/right arrows, hidden at the ends */
#ifdef NON_MATCHING /* cópia de modelos locais e alocação diferem; não insistido */
void *opt_arrows_draw(void *dl, Options *o, short x, short y, int pos, int last)
{
    OptRect r[2];
    OptRect *p;
    short i;
    int show, a;

    r[0] = opt_arrows[0];
    r[1] = opt_arrows[1];
    for (i = 0; i < 2; i++) {
        show = 1;
        if (i == 0) {
            p = &r[0];
            if ((char)pos == 0)
                show = 0;
        } else {
            p = &r[1];
            if ((char)pos == (char)last)
                show = 0;
        }
        if (show) {
            a = (int)(o->frame % 16) / 8 == 0 ? 0 : 0xFF;
            dl = opt_tex(dl, o->opt_gim);
            dl = dl_color(dl, a << 24 | 0xFFFFFF);
            dl = draw_set_offsets(dl, p->w, p->h, r[0].w, r[0].h);
            dl = draw_sprite(dl, x, y, 0, p->u, p->v, 0, 0);
        }
        x += 0x44;
    }
    return dl;
}
#else
void *opt_arrows_draw(void *dl, Options *o, short x, short y, int pos, int last);
#endif

/* on/off switch */
#ifdef NON_MATCHING /* cópia de modelos locais e alocação diferem; não insistido */
void *opt_switch_draw(void *dl, Options *o, int which, int on, short x, short y)
{
    OptRect r[2];
    OptRect *p;
    short sx = x - 2;

    r[0] = opt_switch[0];
    r[1] = opt_switch[1];
    p = which != 0 ? &r[0] : &r[1];
    if ((char)on == 0)
        sx = x + 0x21;
    dl = opt_tex(dl, o->opt_gim);
    dl = draw_set_offsets(dl, p->w, p->h, r[0].w, r[0].h);
    return draw_sprite(dl, sx, y, 0, p->u, p->v, 0, 0);
}
#else
void *opt_switch_draw(void *dl, Options *o, int which, int on, short x, short y);
#endif

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
#ifdef NON_MATCHING /* cópia de modelos locais e alocação diferem; não insistido */
void *opt_text(void *dl, int x, int y, int w, int msg, int center)
{
    Options *o = options;
    char names[6][0x40];
    unsigned short *text;
    int tw;

    memcpy(names, opt_msg_files, sizeof(names));
    text = table_get_entry(ehp_find(o->pack, names[cfg_get_language()], 0), msg);
    if (center != 0) {
        tw = font_measure_text(text) >> 6;
        if (tw < w) {
            x += (w - tw) / 2;
            w = tw;
        }
    }
    font_text_set(dl);
    font_draw_wtext(x << 6, y << 6, (w + 2) << 6, text);
    return font_text_clear();
}
#else
void *opt_text(void *dl, int x, int y, int w, int msg, int center);
#endif

void *opt_text_left(void *dl, int x, int y, int w, int msg)
{
    return opt_text(dl, x, y, w, msg, 0);
}

int opt_open(void)
{
    int m = func_0881CB84();

    switch (m) {
    case 6:
    case 9:
    case 5:
        return 0;
    }
    opt_prev_mode = func_0881CB84();
    func_0881CB90(9);
    func_0881D044(0);
    return 1;
}

void opt_start(void)
{
    opt_state = 1;
    fade_start_in(1, 0xFF000000, 16);
    opt_init(func_0881CA74(), func_088045C0(0x80001), 1);
}

void opt_term(void)
{
    opt_free();
}

void opt_draw_layer(void *dl)
{
    opt_draw(func_0881CD80(dl, opt_prev_mode));
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
        if (((int *)func_0881DE50())[6] == 0) {
            opt_state = 0;
            func_0881CC60();
        }
        break;
    }
}

int opt_is_open(void)
{
    return func_0881CB84() == 9;
}

void opt_close(void)
{
    if (func_0881CB84() == 9)
        func_0881CC60();
}
