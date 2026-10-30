// @range 0x08858224-0x088587EC
// @asm 0x088583D8-0x08858614
// @asm 0x08858614-0x088586F0
// @rodata 0x08870DF0-0x08870E90
/* LP display object of the duel (continues dd_lp_panel.c): the panel with the LP of both
   players as text ('?' hidden, padded), drawn digit by digit from the number texture. */
#include "dd_duelui.h"
#include "allegrex.h"

typedef struct DdLpShow {              /* LP display object, 0x34 (as in dd_lp_panel.c) */
    int x, y;                          /* +0x00 */
    int lp[2];                         /* +0x08 */
    int sx, sy;                        /* +0x10 */
    int active;                        /* +0x18 */
    unsigned int t;                    /* +0x1C */
    char text[2][5];                   /* +0x20 digits, least significant first */
    short pad2a;
    int wide[2];                       /* +0x2C */
} DdLpShow;

typedef struct DdLpMgr {               /* 0x08C5B294 (as in dd_lp_panel.c) */
    void *obj;
    float fovy, aspect, near, far;
    void *pnl01, *pnl02, *nmb01, *nmb02;
    void *pnl_a01, *nmb01_a;
    void *ehf;
} DdLpMgr;

extern DdLpMgr dd_lp;

const int lp_glyph_uv_j[10][2] = {
    { 0, 0 }, { 17, 0 }, { 34, 0 }, { 51, 0 }, { 68, 0 },
    { 0, 24 }, { 17, 24 }, { 34, 24 }, { 51, 24 }, { 68, 24 },
};
const int lp_glyph_uv[10][2] = {
    { 0, 37 }, { 20, 37 }, { 40, 37 }, { 60, 37 }, { 80, 37 },
    { 100, 37 }, { 120, 37 }, { 140, 37 }, { 160, 37 }, { 180, 37 },
};

void *func_088562C4(void *dl, int x, int y);
void *func_088566EC(void *dl, int x, int y);
void *func_0885650C(void *dl);
int func_0881E330(void);
int func_0881CA64(void *p);
void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);

void *lps_back(void *dl, DdLpShow *s);
void *lps_digits(void *dl, DdLpShow *s, int top);
void *lps_glyph(void *dl, int x, int y, char c);
void *lps_back_2d(void *dl, DdLpShow *s);

void lps_draw(void *o, DdLpShow *s)
{
    void *dl = dl_select_layer(8, 0);

    if (region_get() == 0) {
        dl = func_088562C4(dl, s->sx, s->sy);
        dl = func_088566EC(dl, s->sx, s->sy);
        dl = lps_back(dl, s);
        dl = lps_digits(dl, s, 1);
        dl = lps_digits(dl, s, 0);
        dl = func_0885650C(dl);
    } else {
        dl = func_088562C4(dl, s->sx, s->sy);
        dl = lps_back_2d(dl, s);
        dl = lps_digits(dl, s, 1);
        dl = lps_digits(dl, s, 0);
        dl = func_0885650C(dl);
    }
    dl_set_list(dl);
}

void *lps_back(void *dl, DdLpShow *s)
{
    dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = gim_set_texture(dl, dd_lp.nmb02);
    dl = draw_set_offsets(dl, 43, 17, 43, 17);
    dl = draw_sprite(dl, s->sx + 15, s->sy + 7, 0, 85, 0, 0, 0);
    return draw_sprite(dl, s->sx + 15, s->sy + 48, 0, 85, 17, 0, 0);
}

#ifdef NON_MATCHING /* alocação de registradores difere; não insistido */
/* the five characters of one player (top: player 0) */
void *lps_digits(void *dl, DdLpShow *s, int top)
{
    int x0, w0, w1, n0, n1, gw, gh, dy0, dy1, sp0, sp1, i;
    void *gim;

    if (region_get() == 0) {
        w0 = 17;
        gh = 24;
        gim = dd_lp.nmb02;
        n0 = 5;
        w1 = 17;
        gw = 17;
        x0 = 25;
        dy1 = 24;
        dy0 = 65;
        sp0 = 0;
        sp1 = 0;
        n1 = 5;
    } else {
        gw = 20;
        gh = 27;
        gim = dd_lp.nmb01_a;
        dy1 = 22;
        dy0 = 72;
        sp0 = 3;
        sp1 = 3;
        x0 = 69;
        if (s->wide[0] == 0) {
            w0 = 20;
            n0 = 4;
        } else {
            w0 = 17;
            n0 = 5;
        }
        if (s->wide[1] == 0) {
            w1 = 20;
            n1 = 4;
        } else {
            w1 = 17;
            n1 = 5;
        }
    }
    dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
    dl = dl_set_tex_filter(dl, 1, 1);
    dl = gim_set_texture(dl, gim);
    for (i = 0; i < 5; i++) {
        if (top == 0) {
            dl = draw_set_offsets(dl, gw, gh, w1, gh);
            dl = lps_glyph(dl, s->sx + x0 + (w1 - sp1) * (n1 - 1 - i), s->sy + dy0, s->text[1][i]);
        } else {
            dl = draw_set_offsets(dl, gw, gh, w0, gh);
            dl = lps_glyph(dl, s->sx + x0 + (w0 - sp0) * (n0 - 1 - i), s->sy + dy1, s->text[0][i]);
        }
    }
    return dl;
}
#else
void *lps_digits(void *dl, DdLpShow *s, int top);
#endif

#ifdef NON_MATCHING /* só o teste de '?' sai beql em vez de beq */
void *lps_glyph(void *dl, int x, int y, char c)
{
    const int (*uv)[2];
    int u, v, d;

    if (region_get() == 0) {
        uv = lp_glyph_uv_j;
        u = 111;
        v = 40;
    } else {
        uv = lp_glyph_uv;
        u = 200;
        v = 37;
    }
    switch (c) {
    case '?':
        break;
    default:
        if (c < '0' || c >= '9' + 1)
            return dl;
        d = ALLEGREX_MIN(ALLEGREX_MAX(c - '0', 0), 9);
        u = uv[d][0];
        v = uv[d][1];
        break;
    }
    return draw_sprite(dl, x, y, 0, u, v, 0, 0);
}
#else
void *lps_glyph(void *dl, int x, int y, char c);
#endif

void *lps_back_2d(void *dl, DdLpShow *s)
{
    dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = gim_set_texture(dl, dd_lp.pnl_a01);
    dl = draw_set_offsets(dl, 184, 122, 184, 122);
    return draw_sprite(dl, s->sx, s->sy, 0, 0, 122, 0, 0);
}

void lps_update(void *o, DdLpShow *s)
{
    if (++s->t >= 45)
        s->active = 0;
    if (s->active == 0)
        func_0881E330();
}

void lps_free(void *o, DdLpShow *s)
{
    func_0881CA64(s);
    dd_lp.obj = 0;
}
