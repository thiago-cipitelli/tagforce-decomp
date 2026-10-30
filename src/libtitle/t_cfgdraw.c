// @range 0x0880D454-0x0880EB20
// @asm 0x0880D454-0x0880E934
// @data 0x0882E44C-0x0882E7F0
/* Drawing of the options screen (tcfg): the framed groups of rows, each row's label and its control
   (volume meter, left/right value with arrows, on/off check), and the option_msg_X.bin texts. */
#include "libtitle.h"

typedef struct TCfgSpr {               /* u, v, w, h of a panel piece */
    short u, v, w, h;
} TCfgSpr;

void *tcfg_row(void *list, TCfg *c, int group, int row, char sel, int x, int y, int w);
void *tcfg_meter(void *list, TCfg *c, int level, short x, short y);
void *tcfg_arrows(void *list, TCfg *c, short x, short y, int pos, int max);
void *tcfg_check(void *list, TCfg *c, int kind, int on, short x, short y);
void *tcfg_gfx(void *list, void *gim);
void *tcfg_text(void *list, int x, int y, int w, int id, int center);
void *tcfg_label(void *list, int x, int y, int w, int id);

/* templates of tcfg_draw (kept in asm) */
char tcfg_gims[2][64] = { "m_menu_panel01.gim", "menu_opt01.gim" };
char tcfg_gims_alt[2][64] = { "m_menu_panel02.gim", "menu_opt02.gim" };
TCfgSpr tcfg_spr[27] = {
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
int tcfg_group_style[4] = { 8, 9, 10, 11 };
unsigned char tcfg_group_tab[4] = { 0, 0, 0, 1 };
short tcfg_group_rows[4] = { 1, 1, 4, 5 };

#ifdef NON_MATCHING /* função de 3 KB com laços de peças; alocação de registradores e pilha diferem */
void *tcfg_draw(void *list)
{
    TCfg *c = tcfg;
    char gims[2][64];
    char gims_alt[2][64];
    TCfgSpr spr[27];
    int style[4];
    unsigned char tab[4];
    short rows[4];
    short gy[4];
    short gx[4];
    short top[4];
    int first = 0;
    int cur = c->step;
    int found = 0;
    int g, i, k;
    short sel_row = 0;
    short sel_group = 0;
    short n = 0;
    short rx, ry, rw, rh;
    short x, y;

    memcpy(gims, tcfg_gims, sizeof(gims));
    memcpy(gims_alt, tcfg_gims_alt, sizeof(gims_alt));
    memcpy(spr, tcfg_spr, sizeof(spr));
    memcpy(style, tcfg_group_style, sizeof(style));
    memcpy(tab, tcfg_group_tab, sizeof(tab));
    memcpy(rows, tcfg_group_rows, sizeof(rows));
    cfg_get_language();
    if (c->mode == 1)
        first = 2;
    if (cfg_get_language() != 0) {
        rows[2] = 3;
        if (c->step > 2)
            cur = c->step - 1;
    }
    if (c->mode == 1) {
        c->gim = ehp_find(c->pack, gims_alt[0], 0);
        c->gim2 = ehp_find(c->pack, gims_alt[1], 0);
    } else {
        c->gim = ehp_find(c->pack, gims[0], 0);
        c->gim2 = ehp_find(c->pack, gims[1], 0);
    }
    /* group and row of the cursor */
    for (g = 0; g < 4; g++) {
        if (rows[g] < 2) {
            sel_row = 0;
            if (cur == n)
                break;
            sel_row = 1;
            n++;
        } else {
            sel_row = 1;
            for (k = 0; k < rows[g] - 1; k++) {
                if (cur == n) {
                    found = 1;
                    break;
                }
                sel_row++;
                n++;
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
    for (g = first; g < 4; g++) {
        int pieces = rows[g] == 1 ? 3 : 9;

        /* frame */
        for (i = 0; i < pieces; i++) {
            TCfgSpr *s = &spr[i];

            list = tcfg_gfx(list, c->gim);
            switch (i % 3) {
            case 2:
                x = 349 - s->w;
                rw = s->w;
                break;
            case 1:
                x = s->u + 131;
                rw = 218 - (spr[0].w + spr[2].w);
                break;
            case 0:
                rw = s->w;
                x = s->u + 131;
                break;
            }
            switch (i / 3) {
            case 1:
                rh = s->h;
                y = gy[g] + (rows[g] - 1) * 17;
                break;
            case 2:
                y = gy[g] + 10;
                rh = rows[g] * 17 - 10;
                break;
            default:
                y = gy[g];
                rh = s->h;
                break;
            }
            list = draw_set_offsets(list, s->w, s->h, rw, rh);
            list = draw_sprite(list, x, y, 0, s->u + style[g] % 6 * 20, s->v + style[g] / 6 * 24, 0, 0);
        }
        /* tab of the group title */
        if (rows[g] > 1) {
            for (i = 0; i < 3; i++) {
                TCfgSpr *s = &spr[9 + i];

                list = tcfg_gfx(list, c->gim2);
                switch (i % 3) {
                case 2:
                    x = gx[g] + 213 - s->w;
                    rw = s->w;
                    break;
                case 1:
                    x = gx[g] + 4 + s->u;
                    rw = 209 - (spr[11].w + spr[13].w);
                    break;
                default:
                    rw = s->w;
                    x = gx[g] + 4 + s->u;
                    break;
                }
                list = draw_set_offsets(list, s->w, s->h, rw, s->h);
                list = draw_sprite(list, x, gy[g] + 4, 0, s->u, tab[g] * 16 + s->v, 0, 0);
            }
        }
        /* cursor bar */
        if (c->active && g == sel_group) {
            list = tcfg_gfx(list, c->gim);
            list = draw_set_offsets(list, spr[14].w, spr[14].h, 209, 14);
            list = draw_sprite(list, gx[g] + 4, gy[g] + 4 + sel_row * 17, 0, spr[14].u, spr[14].v, 0, 0);
        }
        for (i = 0; i < rows[g]; i++)
            list = tcfg_row(list, c, g, i, g == sel_group && i == sel_row, gx[g], gy[g], 218);
    }
    /* help box at the bottom */
    for (i = 0; i < 9; i++) {
        TCfgSpr *s = &spr[17 + i];
        short bx = 8;
        short bw = s->w;
        short by = 246;
        short bh = s->h;

        if (i % 3 == 1) {
            bw = 462 - spr[17].w - spr[19].w;
            if (bw < 0)
                bw = 0;
            bx = 8 + spr[17].w;
        } else if (i % 3 == 2) {
            bx = 8 + spr[17].w + (462 - spr[17].w - spr[19].w);
        }
        if (i / 3 == 1) {
            by = 246 + spr[17].h;
            bh = 20 - spr[17].h - spr[23].h;
            if (bh < 0)
                bh = 0;
        } else if (i / 3 == 2) {
            by = 246 + spr[17].h + (20 - spr[17].h - spr[23].h);
        }
        list = tcfg_gfx(list, c->gim2);
        list = draw_set_offsets(list, s->w, s->h, bw, bh);
        list = draw_sprite(list, bx, by, 0, s->u, s->v, 0, 0);
    }
    list = tcfg_gfx(list, c->gim2);
    list = draw_set_offsets(list, spr[26].w, spr[26].h, spr[26].w, spr[26].h);
    list = draw_sprite(list, 12, 250, 0, spr[26].u, spr[26].v, 0, 0);
    k = c->step + 19;
    if (c->step == 4)
        k = 22;
    else if (c->step == 3)
        k = 23;
    font_set_size(13, 13);
    font_set_color(0xFF000000);
    font_set_flag4(1);
    list = tcfg_label(list, 29, 251, 436, k);
    c->active = 0;
    return list;
}
#endif

/* templates of the row controls (the functions stay in asm) */
TCfgSpr tcfg_meter_spr[2] = { { 32, 16, 6, 12 }, { 38, 16, 6, 12 } };
TCfgSpr tcfg_arrow_spr[2] = { { 48, 0, 8, 12 }, { 56, 0, 8, 12 } };
TCfgSpr tcfg_check_spr[2] = { { 0, 32, 32, 13 }, { 0, 48, 32, 13 } };

#ifdef NON_MATCHING /* cópias de structs pelos modelos e registradores diferem */
/* row `row` of group `group`: the label and its control */
void *tcfg_row(void *list, TCfg *c, int group, int row, char sel, int x, int y, int w)
{
    TCfgVals *v = &c->cur;
    unsigned char level = 0;
    int kind = 0;
    int tx;
    int ty;
    int id;
    unsigned int color;
    int sel2;
    unsigned char on;
    int max;
    unsigned int val;

    if (row == 0) {
        tx = x + 8;
        ty = y + 5;
    } else {
        tx = x + 16;
        ty = y + 5 + row * 17;
    }
    if (group == 3) {
        w = 112;
        if (row == 4)
            id = 18;
        else if (row == 3)
            id = 11;
        else if (row == 2)
            id = 9;
        else if (row == 1)
            id = 8;
        else {
            id = 7;
            w = 202;
        }
    } else if (group == 2) {
        id = row + 3;
        w = 202;
        if (cfg_get_language() != 0 && id > 3)
            id = row + 4;
        if (id > 3)
            w = 75;
    } else if (group == 1) {
        id = row + 2;
        w = 120;
    } else if (group == 0) {
        id = row + 1;
        w = 120;
    }
    if (sel == 0)
        color = c->color;
    else if (c->active == 0)
        color = c->color;
    else
        color = c->color2;
    font_set_size(13, 13);
    font_set_color(color);
    font_set_flag4(1);
    list = tcfg_label(list, tx, ty, w, id);
    if (group == 3) {
        if (row == 1)
            kind = 2;
        else if (row > 1)
            kind = 1;
    } else if (group == 2) {
        if (row != 0)
            kind = 3;
    } else if (group == 1) {
        kind = 2;
    } else if (group == 0) {
        kind = 1;
    }
    font_set_flag4(1);
    if (kind == 3) {
        if (cfg_get_language() == 0) {
            if (row == 3)
                level = v->vol[2];
            else if (row == 2)
                level = v->vol[1];
            else if (row == 1)
                level = v->vol[0];
        } else if (row == 2) {
            level = v->vol[2];
        } else if (row == 1) {
            level = v->vol[1];
        } else {
            level = 0;
        }
        list = tcfg_meter(list, c, level, x + 94, ty);
    } else if (kind == 2) {
        max = 3;
        if (group == 1) {
            max = 2;
            val = v->speed;
            id = val + 15;
        } else {
            val = v->b134;
            id = val + 14;
        }
        list = tcfg_text(list, x + 144, ty, 54, id, 1);
        if (sel)
            list = tcfg_arrows(list, c, x + 132, ty, val, max);
    } else if (kind == 1) {
        if (group == 0) {
            sel2 = 1;
            on = v->mode1;
        } else {
            sel2 = 0;
            if (row == 4)
                on = v->flag3;
            else if (row == 3)
                on = v->flag2;
            else if (row == 2)
                on = v->flag0;
        }
        list = tcfg_check(list, c, sel2, on, x + 137, ty);
        list = tcfg_text(list, x + 136, ty, 30, 12, 1);
        list = tcfg_text(list, x + 171, ty, 30, 13, 1);
    }
    return list;
}

/* volume meter: 15 segments, `level` lit */
void *tcfg_meter(void *list, TCfg *c, int level, short x, short y)
{
    TCfgSpr s[2];
    TCfgSpr off;
    TCfgSpr cur;
    short i;

    memcpy(s, tcfg_meter_spr, sizeof(s));
    off = s[1];
    for (i = 0; i < 15; i++, x += 7) {
        if (i < (unsigned char)level)
            cur = s[0];
        else
            cur = off;
        list = tcfg_gfx(list, c->gim2);
        list = draw_set_offsets(list, cur.w, cur.h, s[0].w, s[0].h);
        list = draw_sprite(list, x, y, 0, cur.u, cur.v, 0, 0);
    }
    return list;
}

/* the blinking left/right arrows of a value */
void *tcfg_arrows(void *list, TCfg *c, short x, short y, int pos, int max)
{
    TCfgSpr s[2];
    TCfgSpr *p;
    short i;
    int show;
    int a;

    memcpy(s, tcfg_arrow_spr, sizeof(s));

    for (i = 0; i < 2; i++, x += 68) {
        show = 1;
        if (i == 0) {
            p = &s[0];
            if ((char)pos == 0)
                show = 0;
        } else {
            p = &s[1];
            if ((char)pos == (char)max)
                show = 0;
        }
        if (show) {
            a = c->blink % 16 / 8 ? 0xFF : 0;
            list = tcfg_gfx(list, c->gim2);
            list = dl_color(list, a << 24 | 0xFFFFFF);
            list = draw_set_offsets(list, p->w, p->h, s[0].w, s[0].h);
            list = draw_sprite(list, x, y, 0, p->u, p->v, 0, 0);
        }
    }
    return list;
}

/* 58 palavras: o original copia s[k] para uma struct na pilha (lwc1 de s[0], lh de s[1]) e o mwcc a escalariza; lógica conferida com o asm */
/* the check mark under ON (kind 1) or OFF */
void *tcfg_check(void *list, TCfg *c, int kind, int on, short x, short y)
{
    TCfgSpr s[2];
    TCfgSpr *p;
    short sx = x - 2;
    void *gim;

    memcpy(s, tcfg_check_spr, sizeof(s));

    if (kind == 0)
        p = &s[1];
    else
        p = &s[0];
    if ((char)on == 0)
        sx = x + 33;
    gim = c->gim2;
    list = tcfg_gfx(list, gim);
    list = draw_set_offsets(list, p->w, p->h, s[0].w, s[0].h);
    return draw_sprite(list, sx, y, 0, p->u, p->v, 0, 0);
}
#endif

void *tcfg_gfx(void *list, void *gim)
{
    list = dl_clear_color(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear_depth(list, 0);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_enable(list, 9);
    list = dl_disable(list, 1);
    list = dl_disable(list, 10);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_color(list, 0xFFFFFFFF);
    return gim_set_texture(list, gim);
}

/* text `id` of option_msg_X.bin, centered in w when `center` */
void *tcfg_text(void *list, int x, int y, int w, int id, int center)
{
    TCfg *c = tcfg;
    char names[6][64] = {
        "option_msg_j.bin", "option_msg_e.bin", "option_msg_g.bin",
        "option_msg_f.bin", "option_msg_i.bin", "option_msg_s.bin",
    };
    unsigned short *text;
    int n;

    text = table_get_entry(ehp_find(c->pack, names[cfg_get_language()], 0), id);
    if (center) {
        n = font_measure_text(text) >> 6;
        if (n < w) {
            int d = w - n;

            w = n;
            x += d / 2;
        }
    }
    font_text_set(list);
    font_draw_wtext(x << 6, y << 6, (w + 2) << 6, text);
    return font_text_clear();
}

void *tcfg_label(void *list, int x, int y, int w, int id)
{
    return tcfg_text(list, x, y, w, id, 0);
}
