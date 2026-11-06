// @range 0x0880D454-0x0880EB20
// @asm 0x0880D454-0x0880E0B4
// @asm 0x0880E508-0x0880E828
// @data 0x0882E44C-0x0882E7F0
/* Drawing of the options screen (tcfg): the framed groups of rows, each row's label and its control
   (volume meter, left/right value with arrows, on/off check), and the option_msg_X.bin texts. */
#include "libtitle.h"

typedef struct TCfgSpr {               /* u, v, w, h of a panel piece */
    short u, v, w, h;
} TCfgSpr;

typedef struct TCfgSpr2 {              /* a pair of pieces, copied whole from a template */
    TCfgSpr s[2];
} TCfgSpr2;

typedef struct TCfgSprA {              /* TCfgSpr copied as two words (lwc1/swc1) */
    short u, v, w, h;
} __attribute__((aligned(4))) TCfgSprA;

void *tcfg_row(void *list, TCfg *c, int group, int row, unsigned char sel, int x, int y, int w, int h);
void *tcfg_meter(void *list, TCfg *c, unsigned char level, int x, int y);
void *tcfg_arrows(void *list, TCfg *c, int x, int y, unsigned char pos, unsigned char max);
void *tcfg_check(void *list, TCfg *c, unsigned char kind, unsigned char on, int x, int y);
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

#ifdef NON_MATCHING /* 3168 B como o original e mesma forma (283 palavras): x/y/rw/rh inicializados (os switch sem default os deixam vivos), índices sem sinal nos laços da aba e da caixa de ajuda (com índice afim o mwcc reduz o endereço a um ponteiro e deixa de içar spr[0].w etc. do laço dos grupos), switch (step) no fim; falta a alocação dos $s (x/rw/rh um registrador acima, cur em $s1 em vez de $s4) */
void *tcfg_draw(void *list)
{
    TCfg *c = tcfg;
    int first = 0;
    char gims[2][64] = { "m_menu_panel01.gim", "menu_opt01.gim" };
    char gims_alt[2][64] = { "m_menu_panel02.gim", "menu_opt02.gim" };
    TCfgSpr spr[27] = {
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
    unsigned char cur = c->step;
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
        for (i = 0; i < pieces; i++) {
            u = spr[i].u + style[first] % 6 * 20;
            v = spr[i].v + style[first] / 6 * 24;
            list = tcfg_gfx(list, c->gim);
            switch (i % 3) {
            case 0:
                rw = spr[i].w;
                x = spr[i].u + 131;
                break;
            case 1:
                rw = 218 - (spr[0].w + spr[2].w);
                x = spr[i].u + 131;
                break;
            case 2:
                rw = spr[i].w;
                x = 349 - rw;
                break;
            }
            switch (i / 3) {
            case 0:
                y = gy[first];
                rh = spr[i].h;
                break;
            case 2:
                y = gy[first] + 10;
                rh = rows[first] * 17 - 10;
                break;
            case 1:
                rh = spr[i].h;
                y = gy[first] + (rows[first] - 1) * 17;
                break;
            }
            list = draw_set_offsets(list, spr[i].w, spr[i].h, rw, rh);
            list = draw_sprite(list, x, y, 0, u, v, 0, 0);
        }
        /* tab of the group title */
        if (rows[first] > 1) {
            y = gy[first] + 4;
            for (i = 0; i < 3; i++) {
                u = spr[(unsigned int)(9 + i)].u;
                v = tab[first] * 16 + spr[(unsigned int)(9 + i)].v;
                list = tcfg_gfx(list, c->gim2);
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
        if (c->active && first == sel_group) {
            x = gx[first] + 4;
            y = gy[first] + 4 + sel_row * 17;
            rw = 209;
            rh = 14;
            list = tcfg_gfx(list, c->gim);
            list = draw_set_offsets(list, spr[12].w, spr[12].h, rw, rh);
            list = draw_sprite(list, x, y, 0, spr[12].u, spr[12].v, 0, 0);
        }
        for (i = 0; i < rows[first]; i++) {
            if (first == sel_group && i == sel_row)
                sel = 1;
            else
                sel = 0;
            list = tcfg_row(list, c, first, i, sel, gx[first], gy[first], 218, rows[first] * 17);
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
        list = tcfg_gfx(list, c->gim2);
        list = draw_set_offsets(list, spr[(unsigned int)(17 + i)].w, spr[(unsigned int)(17 + i)].h, rw, rh);
        list = draw_sprite(list, x, y, 0, u, v, 0, 0);
    }
    u = spr[26].u;
    v = spr[26].v;
    rw = spr[26].w;
    rh = spr[26].h;
    list = tcfg_gfx(list, c->gim2);
    list = draw_set_offsets(list, rw, rh, rw, rh);
    list = draw_sprite(list, 12, 250, 0, u, v, 0, 0);
    k = c->step + 19;
    switch (c->step) {
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
    list = tcfg_label(list, 29, 251, 436, k);
    c->active = 0;
    return list;
}
#endif

/* templates of the row controls (the functions stay in asm) */
TCfgSpr tcfg_meter_spr[2] = { { 32, 16, 6, 12 }, { 38, 16, 6, 12 } };
TCfgSpr tcfg_arrow_spr[2] = { { 48, 0, 8, 12 }, { 56, 0, 8, 12 } };
TCfgSpr tcfg_check_spr[2] = { { 0, 32, 32, 13 }, { 0, 48, 32, 13 } };

/* row `row` of group `group`: the label and its control */
void *tcfg_row(void *list, TCfg *c, int group, int row, unsigned char sel, int x, int y, int w, int h)
{
    TCfgVals *v;
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
    if (sel && c->active)
        color = c->color2;
    else
        color = c->color;
    font_set_size(13, 13);
    font_set_color(color);
    font_set_flag4(1);
    list = tcfg_label(list, tx, ty, w, id);
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
            on = v->mode1;
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
        list = tcfg_check(list, c, sel2, on, cx, ty);
        cx = x + 136;
        list = tcfg_text(list, cx, ty, 30, 12, 1);
        list = tcfg_text(list, cx + 35, ty, 30, 13, 1);
        break;
    case 2:
        max = 3;
        if (group == 1) {
            max = 2;
            val = v->speed;
            id = val + 15;
        } else {
            val = v->b134;
            id = val + 14;
        }
        cx = x + 144;
        list = tcfg_text(list, cx, ty, 54, id, 1);
        if (sel)
            list = tcfg_arrows(list, c, cx - 12, ty, val, max);
        break;
    case 3:
        cx = x + 94;
        if (cfg_get_language() != 0) {
            switch (row) {
            case 1:
                level = v->vol[1];
                break;
            case 2:
                level = v->vol[2];
                break;
            }
        } else {
            switch (row) {
            case 1:
                level = v->vol[0];
                break;
            case 2:
                level = v->vol[1];
                break;
            case 3:
                level = v->vol[2];
                break;
            }
        }
        list = tcfg_meter(list, c, level, cx, ty);
        break;
    }
    return list;
}

#ifdef NON_MATCHING
/* 14 palavras: a cópia off = s[1] sai com os lh/sh noutra ordem (u,v,w antes do sh no original); o resto bate.
   Tentado: off campo a campo, inicializado na declaração, s como TCfgSpr2 */
/* volume meter: 15 segments, `level` lit */
void *tcfg_meter(void *list, TCfg *c, unsigned char level, int x, int y)
{
    TCfgSpr s[2];
    TCfgSprA cur;
    TCfgSpr off;
    short w;
    short h;
    short sx;
    short sy;
    short i;

    *(TCfgSpr2 *)s = *(TCfgSpr2 *)tcfg_meter_spr;
    off = s[1];
    w = s[0].w;
    h = s[0].h;
    sy = y;
    for (i = 0; i < 15; i++) {
        sx = x + i * 7;
        if (i < (unsigned char)level)
            cur = *(TCfgSprA *)&s[0];
        else
            cur = *(TCfgSprA *)&off;
        list = tcfg_gfx(list, c->gim2);
        list = draw_set_offsets(list, cur.w, cur.h, w, h);
        list = draw_sprite(list, sx, sy, 0, cur.u, cur.v, 0, 0);
    }
    return list;
}

/* 14 palavras: só a ordem dos lh/sh da cópia off = s[1] (como tcfg_meter) */
/* the blinking left/right arrows of a value */
void *tcfg_arrows(void *list, TCfg *c, int x, int y, unsigned char pos, unsigned char max)
{
    TCfgSpr s[2];
    TCfgSprA cur;
    TCfgSpr off;
    unsigned char a;
    short w, h;
    short sx, sy;
    int show;
    short i;

    *(TCfgSpr2 *)s = *(TCfgSpr2 *)tcfg_arrow_spr;
    off = s[1];
    w = s[0].w;
    h = s[0].h;
    sy = y;
    for (i = 0; i < 2; i++) {
        show = 1;
        sx = x + i * 68;
        if (!i) {
            cur = *(TCfgSprA *)&s[0];
            if ((unsigned char)pos == 0)
                show = 0;
        } else {
            cur = *(TCfgSprA *)&off;
            if ((unsigned char)pos == (unsigned char)max)
                show = 0;
        }
        if (show) {
            a = c->blink % 16 / 8 ? 0xFF : 0;
            list = tcfg_gfx(list, c->gim2);
            list = dl_color(list, a << 24 | 0xFFFFFF);
            list = draw_set_offsets(list, cur.w, cur.h, w, h);
            list = draw_sprite(list, sx, sy, 0, cur.u, cur.v, 0, 0);
        }
    }
    return list;
}

#endif

/* the check mark under ON (kind 1) or OFF */
void *tcfg_check(void *list, TCfg *c, unsigned char kind, unsigned char on, int x, int y)
{
    TCfgSpr s[2];
    TCfgSprA cur;
    short w, h;
    short sx;
    short sy;

    *(TCfgSpr2 *)s = *(TCfgSpr2 *)tcfg_check_spr;
    sy = y;
    w = s[0].w;
    h = s[0].h;
    sx = x - 2;
    if (kind)
        cur = *(TCfgSprA *)&s[0];
    else
        *(TCfgSpr *)&cur = s[1];
    if ((unsigned char)on == 0)
        sx = x + 33;
    list = tcfg_gfx(list, c->gim2);
    list = draw_set_offsets(list, cur.w, cur.h, w, h);
    return draw_sprite(list, sx, sy, 0, cur.u, cur.v, 0, 0);
}

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
