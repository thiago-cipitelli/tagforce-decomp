// @range 0x08809D2C-0x0880A960
// @rodata 0x0881C178-0x0881C1F0
// @data 0x0881C970-0x0881C974
/* The duel points counter at the top of the screen: it slides in, counts up or down to the new total
   with a ticking sound, and the amount spent flies out of it as big red digits; plus the digit and
   scaled sprite helpers it draws with. */
#include "libshop.h"

typedef struct ShopSlide {      /* sliding position */
    float pos[2];
    float t;
} ShopSlide;

typedef struct ShopFly {        /* the amount that flies out */
    float pos[2];
    float t;
    float scale;
} ShopFly;

typedef struct ShopPoints {     /* shop_pts 0x08F8EDA4, 0x5C */
    int timer;                  /* +0x00 */
    int state;                  /* +0x04 0 off, 1 sliding in, 2 shown, 3 hold, 4 flying, 5 counting */
    int shown;                  /* +0x08 value on screen */
    int target;                 /* +0x0C */
    int delta;                  /* +0x10 */
    int step;                   /* +0x14 per frame while counting */
    float from[2];              /* +0x18 */
    float to[2];                /* +0x20 */
    float fly_from[2];          /* +0x28 */
    int blink;                  /* +0x30 */
    int period;                 /* +0x34 */
    int on;                     /* +0x38 */
    void *gim;                  /* +0x3C system.gim */
    ShopSlide slide;            /* +0x40 */
    ShopFly fly;                /* +0x4C */
} ShopPoints;

extern ShopPoints shop_pts;

void se_play(int id);
void money_add(int delta);
int sysstate_get_u32_54(int i);
GfxState *gfx_get_state(void);
void vec2_lerp(float t, float *out, float *a, float *b);
int rand(void);
int strlen(const char *s);
float fabsf_vfpu(float x);
float powf_vfpu(float x, float y);
float ceilf_vfpu(float x);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
void *gim_set_texture(void *list, void *gim);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);

void shop_pts_free(void);
void shop_pts_free2(void);
void shop_pts_slide_update(void);
void shop_pts_fly_update(void);
unsigned int *shop_pts_draw_counter(unsigned int *list);
unsigned int *shop_pts_draw_fly(unsigned int *list);
unsigned int *shop_draw_scaled(unsigned int *list, int x, int y, int z, int u, int v, int w, int h, float scale);
unsigned int *shop_draw_digits(unsigned int *list, char *s, int x, int y, int z, int gap, float scale, int style);
int shop_digits(int n);

/* sprites in system.gim: the "DP" label, the small digits, the minus sign, the big digits */
static const short shop_pts_label[4] = { 0, 0, 27, 16 };
static const short shop_pts_digit[4] = { 87, 0, 12, 16 };
static const short shop_pts_minus[4] = { 200, 80, 40, 48 };
static const int shop_pts_label_off[2] = { 93, 0 };
static const int shop_pts_num_off[2] = { 80, 0 };
static const int shop_pts_big[10][2] = {
    { 0, 32 }, { 40, 32 }, { 80, 32 }, { 120, 32 }, { 160, 32 },
    { 0, 80 }, { 40, 80 }, { 80, 80 }, { 120, 80 }, { 160, 80 },
};

void shop_tabs_setup(int x0, int y0, int x1, int y1)
{
    memset(&shop_pts, 0, sizeof(shop_pts));
    shop_pts.from[0] = x0;
    shop_pts.from[1] = y0;
    shop_pts.to[0] = x1;
    shop_pts.to[1] = y1;
    shop_pts.shown = sysstate_get_u32_54(0);
    shop_pts.target = sysstate_get_u32_54(0);
}

void shop_menu_free(void)
{
    shop_pts_free();
    shop_pts_free2();
}

void shop_tabs_update(void)
{
    shop_pts_slide_update();
    shop_pts_fly_update();
}

unsigned int *shop_pts_draw(unsigned int *list)
{
    if (shop_pts.gim == 0)
        return list;
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_enable(list, 4);
    list = dl_alpha_test(list, 6, 0, 0xFF);
    list = dl_enable(list, 0);
    list = dl_disable(list, 1);
    list = dl_enable(list, 9);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_func(list, 0, 1);
    list = dl_tex_wrap(list, 0, 0);
    list = dl_set_tex_filter(list, 0, 0);
    list = shop_pts_draw_counter(list);
    return shop_pts_draw_fly(list);
}

void shop_menu_init(void *gim)
{
    shop_pts.gim = gim;
}

/* spends (or gains) `delta` points; the amount flies from (x, y) */
void shop_points_add(int delta, int x, int y)
{
    ShopPoints *p = &shop_pts;
    ShopFly *f = &shop_pts.fly;

    shop_pts.shown = sysstate_get_u32_54(0);
    p->target = delta + sysstate_get_u32_54(0) > 0 ? delta + sysstate_get_u32_54(0) : 0;
    money_add(delta);
    p->state = 3;
    p->step = delta / 10.0f;
    f->scale = 1.0f;
    p->fly_from[0] = x;
    f->pos[0] = p->fly_from[0];
    p->timer = 0;
    p->delta = delta;
    f->t = 0.0f;
    p->fly_from[1] = y;
    f->pos[1] = p->fly_from[1];
    if (p->step == 0)
        p->step += delta > 0 ? 1 : -1;
    se_play(0x621D);
}

void shop_tabs_init(void)
{
    shop_pts.state = 1;
}

/* slides in, then counts the shown value to the target */
void shop_pts_slide_update(void)
{
    ShopPoints *p = &shop_pts;
    ShopSlide *s = &shop_pts.slide;

    switch (shop_pts.state) {
    case 1:
        vec2_lerp(s->t, s->pos, p->from, p->to);
        if (s->t >= 1.0f) {
            p->state = 2;
        } else {
            s->t += 0.1f + 0.1f * s->t;
            if (s->t > 1.0f)
                s->t = 1.0f;
        }
        break;
    case 5:
        if (p->shown != p->target) {
            if (gfx_get_state()->frame % 3 == 0)
                se_play(0x621E);
            p->shown += p->step;
            if (p->step > 0 && p->shown > p->target) {
                p->shown = p->target;
                p->state = 2;
            } else if (p->step < 0 && p->shown < p->target) {
                p->shown = p->target;
                p->state = 2;
            }
        } else {
            p->state = 2;
        }
        break;
    }
}

/* "DP" label and the counter (scrambled digits while counting) */
unsigned int *shop_pts_draw_counter(unsigned int *list)
{
    ShopPoints *p = &shop_pts;
    ShopSlide *s = &shop_pts.slide;
    const short *r;
    const int *o;
    char buf[256];
    int k;
    int base;
    int r2;

    if (shop_pts.state == 0)
        return list;
    if (p->blink && (int)(gfx_get_state()->frame % p->period) < p->on)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, p->gim);
    r = (const short *)(const char *)shop_pts_label;
    o = (const int *)(const char *)shop_pts_label_off;
    list = dl_ambient_color(list, 0xFF00FFFF);
    list = draw_set_offsets(list, r[2], r[3], r[2], r[3]);
    list = draw_sprite(list, s->pos[0] + o[0], s->pos[1] + o[1], 0, r[0], r[1], 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    switch (p->state) {
    case 5:
        k = ceilf_vfpu(powf_vfpu(10.0f, shop_digits(fabsf_vfpu(0.1f * p->delta))));
        base = p->shown;
        base -= base % k;
        sprintf(buf, "%d", base + (rand() & 0xFFFF) % k);
        break;
    case 3:
    case 4:
        sprintf(buf, "%d", p->shown);
        break;
    default:
        sprintf(buf, "%d", sysstate_get_u32_54(0));
        break;
    }
    return shop_draw_digits(list, buf, s->pos[0] + shop_pts_num_off[0], s->pos[1] + shop_pts_num_off[1],
                            0, -2, 1.0f, 0);
}

void shop_pts_free(void)
{
}

/* the spent amount flies out of the counter and shrinks */
void shop_pts_fly_update(void)
{
    ShopPoints *p = &shop_pts;
    ShopFly *f = &shop_pts.fly;

    switch (shop_pts.state) {
    case 3:
        if (p->timer < 10) {
            p->timer++;
            f->pos[0] = p->fly_from[0];
            f->pos[1] = p->fly_from[1];
            f->scale = 1.0f;
        } else {
            p->state = 4;
        }
        break;
    case 4:
        vec2_lerp(f->t, f->pos, p->fly_from, p->to);
        f->scale = 1.0f + -0.75f * f->t;
        if (f->t >= 1.0f) {
            p->state = 5;
        } else {
            f->t += 0.1f + 0.1f * f->t;
            if (f->t > 1.0f)
                f->t = 1.0f;
        }
        break;
    }
}

unsigned int *shop_pts_draw_fly(unsigned int *list)
{
    ShopPoints *p = &shop_pts;
    ShopFly *f = &shop_pts.fly;
    char buf[256];

    switch (shop_pts.state) {
    case 3:
    case 4:
        list = dl_ambient_color(list, 0xFF0000FF);
        list = gim_set_texture(list, p->gim);
        if (p->delta < 0) {
            int n;
            sprintf(buf, "%d", -p->delta);
            n = strlen(buf);
            list = shop_draw_scaled(list,
                                    f->pos[0] + shop_pts_num_off[0] - 38.0f * f->scale * n,
                                    f->pos[1] + shop_pts_num_off[1], 0, shop_pts_minus[0],
                                    shop_pts_minus[1], shop_pts_minus[2], shop_pts_minus[3], f->scale);
        } else {
            sprintf(buf, "%d", p->delta);
        }
        return shop_draw_digits(list, buf, f->pos[0] + shop_pts_num_off[0], f->pos[1] + shop_pts_num_off[1],
                                0, -2, f->scale, 1);
    }
    return list;
}

void shop_pts_free2(void)
{
}

unsigned int *shop_draw_scaled(unsigned int *list, int x, int y, int z, int u, int v, int w, int h, float scale)
{
    list = dl_tex_wrap(list, 1, 1);
    list = draw_set_offsets(list, w, h, (int)(w * scale), (int)(h * scale));
    return draw_sprite(list, x, y, z, u, v, 0, 0);
}

/* `s` right-aligned at x: small digits (style 0) or the big ones (style 1) */
unsigned int *shop_draw_digits(unsigned int *list, char *s, int x, int y, int z, int gap, float scale, int style)
{
    int last = strlen(s) - 1;
    int i;
    int u, v, w, h;
    int d;

    for (i = last; i >= 0; i--) {
        d = s[i] - '0';
        switch (style) {
        case 0:
            w = shop_pts_digit[2];
            h = shop_pts_digit[3];
            u = shop_pts_digit[0] + w * d;
            v = shop_pts_digit[1];
            break;
        case 1:
            u = shop_pts_big[d][0];
            v = shop_pts_big[d][1];
            w = 40;
            h = 48;
            break;
        }
        list = shop_draw_scaled(list, x - (int)(scale * (w + gap)) * (last - i), y, z, u, v, w, h, scale);
    }
    return list;
}

static inline float shop_scale(float x, float k)
{
    return x * k;
}

/* decimal digits of n */
int shop_digits(int n)
{
    int k = 0;

    for (;;) {
        if (n <= 0)
            break;
        k++;
        n = shop_scale(n, 0.1f);
    }
    return k;
}
