// @range 0x08857100-0x08858224
// @asm 0x08857100-0x08857280
// @asm 0x0885731C-0x08857614
// @asm 0x08857614-0x08857960
// @asm 0x08857960-0x08857A40
// @asm 0x08857A40-0x08857C60
// @asm 0x08857C60-0x08857CC8
// @asm 0x08857DC0-0x0885809C
// @rodata 0x08870D3C-0x08870DF0
// @data 0x08877850-0x08877878
/* LP counting object of the duel (continues dd_lp_panel.c): draws the panel with the five
   rolling digits (3D drums in Japan, 2D sprites and a damage bar elsewhere), counts the LP from
   one value to the other and rolls each digit to its new value. */
#include "dd_duelui.h"

typedef struct LpDigit {               /* a rolling digit, 0x10 */
    int rolling;                       /* +0x00 */
    float t;                           /* +0x04 0..1 */
    int cur;                           /* +0x08 -1 = blank */
    int next;                          /* +0x0C */
} LpDigit;

typedef struct DdLpCount {             /* LP counting object, 0x90 (as in dd_lp_panel.c) */
    int x, y;                          /* +0x00 */
    int from;                          /* +0x08 */
    int to;                            /* +0x0C */
    int cur;                           /* +0x10 */
    int speed;                         /* +0x14 */
    int sx, sy;                        /* +0x18 top-left of the panel */
    int active;                        /* +0x20 0 done, 1 wait, 2 count, 3 hold */
    LpDigit digits[5];                 /* +0x24 */
    unsigned int t;                    /* +0x74 counting frames */
    unsigned int wait;                 /* +0x78 */
    float bar_a;                       /* +0x7C */
    float bar_b;                       /* +0x80 */
    float da;                          /* +0x84 */
    float db;                          /* +0x88 */
    int wide;                          /* +0x8C 5 digits */
} DdLpCount;

typedef struct DdLpMgr {               /* 0x08C5B294 (as in dd_lp_panel.c) */
    void *obj;
    float fovy, aspect, near, far;
    void *pnl01, *pnl02, *nmb01, *nmb02;
    void *pnl_a01, *nmb01_a;
    void *ehf;
} DdLpMgr;

extern DdLpMgr dd_lp;

const float lp_digit_speed[5] = { 0.26f, 0.22f, 0.18f, 0.14f, 0.1f };
const int lp_digit3d_uv[10][2] = {
    { 1, 0 }, { 26, 0 }, { 51, 0 }, { 76, 0 }, { 101, 0 },
    { 1, 34 }, { 26, 34 }, { 51, 34 }, { 76, 34 }, { 101, 34 },
};
const int lp_digit2d_uv[10][2] = {
    { 0, 0 }, { 18, 0 }, { 36, 0 }, { 54, 0 }, { 72, 0 },
    { 90, 0 }, { 108, 0 }, { 126, 0 }, { 144, 0 }, { 162, 0 },
};

float lp_drum_pos[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
int lp_heart_uv[3][2] = { { 184, 0 }, { 184, 16 }, { 184, 32 } };

void *func_088562C4(void *dl, int x, int y);
void *func_088566EC(void *dl, int x, int y);
void *func_0885650C(void *dl);
void func_088565F8(float x, float y, float z, float w, float h, void *v, int tx, int ty, int tw, int th, int texw,
                   int texh, unsigned int color);
int func_0881E330(void);
int func_0881CA64(void *p);
void *dl_alloc_inline(void **list, int bytes);
void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_load(DdMat4 *m);
void mat4_identity(DdMat4 *m);
DdMat4 *mat4_mul_rot_x(DdMat4 *out, DdMat4 *m, float a);
void mat4_inverse_ortho(DdMat4 *out, DdMat4 *m);
void mat4_translate(DdMat4 *out, DdMat4 *m, ChrVec4 *v);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void ehf_seek(void *e, int frame);
void *gfx_begin_2d(void *list);
void *ehf_draw(void *list, void *ehf, void *opts);
void *gfx_end_2d(void *list);
float powf_vfpu(float x, float y);
unsigned int rand(void);
void bgm_stop(int x);

void *lpc_back(void *dl, DdLpCount *c);
void *lpc_drum(void *dl, LpDigit *d, int i);
void *lpc_panel_2d(void *dl, DdLpCount *c);
void *lpc_digit_2d(void *dl, DdLpCount *c, int i);
void lpc_digits_set(LpDigit *d, int value);
void lpc_digit_start(LpDigit *d, int next, int roll);
int lpc_digit_of(int value, int i, int blank);
void lpc_digit_update(DdLpCount *c, int i);
void lpc_pulse(DdLpCount *c);

#ifdef NON_MATCHING /* alocação de registradores/forma difere; não insistido */
void lpc_draw(void *o, DdLpCount *c)
{
    void *dl = dl_select_layer(8, 0);
    int i;

    if (region_get() == 0) {
        dl = func_088562C4(dl, c->sx, c->sy);
        dl = func_088566EC(dl, c->sx, c->sy);
        dl = lpc_back(dl, c);
        dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
        dl = dl_enable(dl, 4);
        dl = dl_set_tex_filter(dl, 1, 1);
        dl = gim_set_texture(dl, dd_lp.nmb01);
        for (i = 0; i < 5; i++)
            dl = lpc_drum(dl, &c->digits[i], i);
        dl = func_0885650C(dl);
    } else {
        dl = func_088562C4(dl, c->sx, c->sy);
        dl = lpc_panel_2d(dl, c);
        dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
        dl = dl_set_tex_filter(dl, 1, 1);
        dl = gim_set_texture(dl, dd_lp.nmb01_a);
        for (i = 0; i < 5; i++)
            dl = lpc_digit_2d(dl, c, i);
        dl = func_0885650C(dl);
    }
    dl_set_list(dl);
}
#else
void lpc_draw(void *o, DdLpCount *c);
#endif

void *lpc_back(void *dl, DdLpCount *c)
{
    dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = gim_set_texture(dl, dd_lp.nmb01);
    dl = draw_set_offsets(dl, 48, 34, 48, 34);
    return draw_sprite(dl, c->sx + 15, c->sy + 10, 0, 0, 68, 0, 0);
}

#ifdef NON_MATCHING /* desenho 3D do tambor (matrizes); não tentado a fundo */
/* drum of digit i: the current face, and the next one turned by t half a turn */
void *lpc_drum(void *dl, LpDigit *d, int i)
{
    void *cur = 0;
    void *next = 0;
    ChrVec4 pos;
    DdMat4 rot2, mv, view, rot, m;

    if (d->cur != -1) {
        cur = dl_alloc_inline(&dl, 0x60);
        func_088565F8(0.0f, 0.0f, 0.0f, 0.3f, 0.5f, cur, lp_digit3d_uv[d->cur][0], lp_digit3d_uv[d->cur][1], 24,
                      34, 128, 128, 0xFFFFFFFF);
    }
    if (d->next != -1) {
        next = dl_alloc_inline(&dl, 0x60);
        func_088565F8(0.0f, 0.0f, 0.0f, 0.3f, 0.5f, next, lp_digit3d_uv[d->next][0], lp_digit3d_uv[d->next][1],
                      24, 34, 128, 128, 0xFFFFFFFF);
    }
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    pos = *(ChrVec4 *)lp_drum_pos;
    pos.x = (float)(5 - i) * 0.3f + -3.4f;
    mat4_identity(&rot);
    mat4_mul_rot_x(&rot, &rot, d->t * 3.1415927f);
    mtx_set_mode(1);
    mtx_get34(&view);
    mat4_inverse_ortho(&view, &view);
    mat4_identity(&mv);
    mat4_translate(&mv, &mv, &pos);
    mat4_identity(&m);
    mat4_mul(&m, &m, &mv);
    mat4_mul(&m, &m, &view);
    mat4_mul(&m, &m, &rot);
    mtx_set_mode(2);
    mtx_load(&m);
    if (cur != 0)
        dl = draw_array(dl, 4, 0x19F, 4, 0, cur);
    if (next != 0) {
        mat4_identity(&rot2);
        mat4_mul_rot_x(&rot2, &rot2, 3.1415927f);
        mat4_mul(&m, &m, &rot2);
        mtx_load(&m);
        dl = draw_array(dl, 4, 0x19F, 4, 0, next);
    }
    mtx_pop();
    return dl;
}

/* 2D panel: background, damage effect (ehf frame by the LP left), two bars and the heart */
void *lpc_panel_2d(void *dl, DdLpCount *c)
{
    int uv[3][2];
    char opts[0x60];
    int cur = c->cur;
    int max = c->speed;
    unsigned int frame;
    int w;

    memcpy(uv, lp_heart_uv, sizeof(uv));
    if (cur < 0)
        cur = 0;
    if (max < cur)
        cur = max;
    dl = dl_blend_func(dl, 0, 2, 3, 0, 0);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = gim_set_texture(dl, dd_lp.pnl_a01);
    dl = draw_set_offsets(dl, 184, 122, 184, 122);
    dl = draw_sprite(dl, c->sx, c->sy, 0, 0, 0, 0, 0);
    memset(opts, 0, sizeof(opts));
    frame = (1.0f - (float)cur / (float)max) * 99.0f;
    ehf_seek(dd_lp.ehf, frame);
    dl = gfx_begin_2d(dl);
    dl = ehf_draw(dl, dd_lp.ehf, opts);
    dl = gfx_end_2d(dl);
    dl = dl_enable(dl, 9);
    dl = dl_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 0);
    dl = gim_set_texture(dl, dd_lp.pnl_a01);
    w = c->bar_a * 56.0f;
    dl = draw_set_offsets(dl, 56, 8, w, 8);
    dl = draw_sprite(dl, c->sx + (int)(100.0f - (float)w * 0.5f), c->sy + 12, 0, 184, 187, 0, 0);
    w = c->bar_b * 56.0f;
    dl = draw_set_offsets(dl, 56, 8, w, 8);
    dl = draw_sprite(dl, c->sx + (int)(100.0f - (float)w * 0.5f), c->sy + 22, 0, 184, 219, 0, 0);
    frame = (*gfx_get_state() >> 1) % 3;
    dl = draw_set_offsets(dl, 65, 16, 65, 16);
    return draw_sprite(dl, c->sx + 63, c->sy + 87, 0, uv[frame][0], uv[frame][1], 0, 0);
}
#else
void *lpc_drum(void *dl, LpDigit *d, int i);
void *lpc_panel_2d(void *dl, DdLpCount *c);
#endif

#ifdef NON_MATCHING /* alocação de registradores/forma difere; não insistido */
void *lpc_digit_2d(void *dl, DdLpCount *c, int i)
{
    int w, n, d;

    if (c->wide == 0) {
        w = 18;
        n = 4;
    } else {
        w = 13;
        n = 5;
    }
    d = c->digits[i].next;
    if (d != -1) {
        dl = draw_set_offsets(dl, 18, 37, w, 37);
        dl = draw_sprite(dl, c->sx + 62 + w * (n - 1 - i), c->sy + 38, 0, lp_digit2d_uv[d][0], lp_digit2d_uv[d][1],
                         0, 0);
    }
    return dl;
}
#else
void *lpc_digit_2d(void *dl, DdLpCount *c, int i);
#endif

#ifdef NON_MATCHING /* alocação de registradores/forma difere; não insistido */
void lpc_update(void *o, DdLpCount *c)
{
    int i;

    switch (c->active) {
    case 1:
        if (++c->wait >= 10) {
            c->wait = 0;
            c->active = 2;
            se_play(0x622D);
        }
        break;
    case 2:
        if (c->t < 36) {
            c->cur = (float)c->from + (float)c->t / 35.0f * ((float)c->to - (float)c->from);
            c->t++;
        } else if (c->digits[0].rolling == 0 && c->digits[1].rolling == 0 && c->digits[2].rolling == 0 &&
                   c->digits[3].rolling == 0 && c->digits[4].rolling == 0) {
            lpc_digits_set(c->digits, c->to);
            c->active = 3;
            bgm_stop(0x622D);
            if (c->to == 0)
                se_play(0x622B);
            else
                se_play(0x622E);
        }
        for (i = 0; i < 5; i++)
            lpc_digit_update(c, i);
        break;
    case 3:
        if (++c->wait > 44)
            c->active = 0;
        break;
    }
    if (region_get() != 0) {
        switch (c->active) {
        case 1:
        case 2:
        case 3:
            lpc_pulse(c);
            break;
        }
    }
    if (c->active == 0)
        func_0881E330();
}
#else
void lpc_update(void *o, DdLpCount *c);
#endif

#ifdef NON_MATCHING /* alocação de registradores/forma difere; não insistido */
void lpc_digits_set(LpDigit *d, int value)
{
    int i;

    for (i = 0; i < 5; i++) {
        lpc_digit_start(d, lpc_digit_of(value, i, i != 0), 0);
        d++;
    }
}
#else
void lpc_digits_set(LpDigit *d, int value);
#endif

void lpc_digit_start(LpDigit *d, int next, int roll)
{
    d->rolling = 0;
    d->cur = next;
    d->next = next;
    if (roll != 0)
        d->t = d->t - 1.0f;
    else
        d->t = 0;
}

/* digit i of value, -1 for a leading zero when blank */
int lpc_digit_of(int value, int i, int blank)
{
    int p, d;

    p = ceilf_vfpu(powf_vfpu(10.0f, (float)i));
    d = value % (p * 10) / p;
    if ((region_get() == 0 || i >= 4) && blank && value / p < 1)
        d = -1;
    return d;
}

#ifdef NON_MATCHING /* laço de OR desenrolado pelo compilador; não tentado a fundo */
void lpc_digit_update(DdLpCount *c, int i)
{
    LpDigit *d = &c->digits[i];
    unsigned int above = 0;
    int k, n, blank;

    for (k = i + 1; k < 5; k++)
        above |= c->digits[k].rolling;
    if (d->rolling == 0) {
        blank = d->next == 0 && above == 0;
        if (i == 0)
            blank = 0;
        n = lpc_digit_of(c->cur, i, blank);
        if (n != 0 || d->next != -1) {
            if (n != d->next) {
                d->rolling = 1;
                d->cur = n;
            }
        }
        if (d->rolling == 0 && above != 0) {
            d->rolling = 1;
            d->cur = (rand() & 0xFFFF) % 10;
        }
    }
    if (d->rolling != 0) {
        if (region_get() == 0)
            d->t += lp_digit_speed[i];
        else
            d->t = 1.0f;
        if (d->t >= 1.0f) {
            if (d->cur == 0 && above == 0) {
                lpc_digit_start(d, lpc_digit_of(c->cur, i, 1), 0);
                return;
            }
            n = lpc_digit_of(c->cur, i, 0);
            lpc_digit_start(d, d->cur, n != d->cur);
        }
    }
}
#else
void lpc_digit_update(DdLpCount *c, int i);
#endif

/* the two bars pulse between 0.5 and 1 */
void lpc_pulse(DdLpCount *c)
{
    c->bar_a += c->da;
    if (c->da > 0.0f) {
        if (c->bar_a > 1.0f) {
            c->bar_a = 1.0f;
            c->da *= -1.0f;
        }
    } else if (c->da < 0.0f && c->bar_a < 0.5f) {
        c->bar_a = 0.5f;
        c->da *= -1.0f;
    }
    c->bar_b += c->db;
    if (c->db > 0.0f) {
        if (c->bar_b > 1.0f) {
            c->bar_b = 1.0f;
            c->db *= -1.0f;
        }
    } else if (c->db < 0.0f && c->bar_b < 0.5f) {
        c->bar_b = 0.5f;
        c->db *= -1.0f;
    }
}

void lpc_free(void *o, DdLpCount *c)
{
    bgm_stop(0x622D);
    func_0881CA64(c);
    dd_lp.obj = 0;
}
