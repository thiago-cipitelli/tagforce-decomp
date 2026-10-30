// @range 0x08817AA4-0x08818724
// @rodata 0x08838B04-0x08838B7C
// @data 0x0883C888-0x0883C88C
/* The money counter of the field HUD (field_money): the coin icon and the amount sliding in, the
   amount gained or spent popping up and flying to the counter, the counter rolling to the new value
   with random low digits, and the digit sprites. */
#include "libfield.h"

int sysstate_get_u32_54(int i);
void money_add(int n);
void *dl_ambient_color(void *list, unsigned int color);
void *gim_set_texture(void *list, void *gim);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_alpha_test(void *list, int func, int ref, int mask);
void *dl_tex_mode(void *list, int mode, int a, int b, int c);
void *dl_tex_func(void *list, int func, int rgba);
void *dl_tex_wrap(void *list, int u, int v);
void *dl_set_tex_filter(void *list, int min, int mag);
float powf_vfpu(float x, float y);
float ceilf_vfpu(float x);

void fmoney_update_count(void);
void *fmoney_draw_count(void *list);
void fmoney_nop(void);
void fmoney_update_pop(void);
void *fmoney_draw_pop(void *list);
void fmoney_nop2(void);
void *fmoney_sprite(void *list, int x, int y, int z, int u, int v, int w, int h, float scale);
void *fmoney_number(void *list, char *s, int x, int y, int z, int gap, float scale, int big);
int fmoney_ndigits(int n);

const FieldSpr fmoney_icon = { 0, 0, 0x1B, 0x10 };
const FieldSpr fmoney_digit = { 0x57, 0, 0xC, 0x10 };
const FieldSpr fmoney_sign = { 0xC8, 0x50, 0x28, 0x30 };
const FieldOff fmoney_icon_off = { 0x5D, 0 };
const FieldOff fmoney_text_off = { 0x50, 0 };
const struct {
    int u, v;
} fmoney_big[10] = { { 0, 0x20 }, { 0x28, 0x20 }, { 0x50, 0x20 }, { 0x78, 0x20 }, { 0xA0, 0x20 },
                     { 0, 0x50 }, { 0x28, 0x50 }, { 0x50, 0x50 }, { 0x78, 0x50 }, { 0xA0, 0x50 } };

void fmoney_init(int x0, int y0, int x1, int y1)
{
    memset(&field_money, 0, sizeof(field_money));
    field_money.from.x = x0;
    field_money.from.y = y0;
    field_money.to.x = x1;
    field_money.to.y = y1;
    field_money.shown = sysstate_get_u32_54(0);
    field_money.target = sysstate_get_u32_54(0);
}

void fmoney_free(void)
{
    fmoney_nop();
    fmoney_nop2();
}

void fmoney_update(void)
{
    fmoney_update_count();
    fmoney_update_pop();
}

void *fmoney_draw(void *list)
{
    if (field_money.gim == 0)
        return list;
    {
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
        list = fmoney_draw_count(list);
        list = fmoney_draw_pop(list);
    }
    return list;
}

void fmoney_set_gim(void *gim)
{
    field_money.gim = gim;
}

void fmoney_add(int amount, int x, int y)
{
    FieldMoney *const m = &field_money;
    FieldMoneyPop *const p = &field_money.pop;

    field_money.shown = sysstate_get_u32_54(0);
    m->target = amount + sysstate_get_u32_54(0) > 0 ? amount + sysstate_get_u32_54(0) : 0;
    money_add(amount);
    m->wait = 0;
    m->state = 3;
    m->step = (int)(amount / 10.0f);
    p->scale = 1.0f;
    m->delta = amount;
    p->pos.x = m->dfrom.x = x;
    p->t = 0;
    p->pos.y = m->dfrom.y = y;
    if (m->step == 0)
        m->step += amount > 0 ? 1 : -1;
    se_play(0x621D);
}

int fmoney_shown(void)
{
    return field_money.state == 2;
}

void fmoney_open(void)
{
    field_money.state = 1;
}

void fmoney_blink(int period, int duty)
{
    FieldMoney *const m = &field_money;

    if (period >= 0) {
        if (m->blink_on == 0) {
            m->blink_on = 1;
            m->period = period;
            m->duty = duty;
        }
    } else {
        m->blink_on = 0;
    }
}

void fmoney_update_count(void)
{
    FieldMoney *const m = &field_money;
    FieldMoneySlide *const s = &field_money.slide;

    switch (field_money.state) {
    case 1:
        vec2_lerp(&s->pos, &m->from, &m->to, s->t);
        if (s->t >= 1.0f) {
            m->state = 2;
        } else {
            s->t = s->t + (0.1f + 0.1f * s->t);
            if (s->t > 1.0f)
                s->t = 1.0f;
        }
        break;
    case 5:
        if (m->shown != m->target) {
            if (*gfx_get_state() % 3 == 0)
                se_play(0x621E);
            m->shown += m->step;
            if (m->step > 0 && m->target < m->shown) {
                m->shown = m->target;
                m->state = 2;
            } else if (m->step < 0 && m->shown < m->target) {
                m->shown = m->target;
                m->state = 2;
            }
        } else {
            m->state = 2;
        }
        break;
    }
}

void *fmoney_draw_count(void *list)
{
    FieldMoney *const m = &field_money;
    FieldMoneySlide *const s = &field_money.slide;
    char buf[256];
    int p;
    int v;

    if (field_money.state == 0)
        return list;
    if (m->blink_on && (int)(*gfx_get_state() % m->period) < m->duty)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, m->gim);
    {
        const FieldSpr *const sp = &fmoney_icon;
        const FieldOff *const o = &fmoney_icon_off;

        list = dl_ambient_color(list, 0xFF00FFFF);
        list = draw_set_offsets(list, sp->w, sp->h, sp->w, sp->h);
        list = draw_sprite(list, (int)(s->pos.x + o->x), (int)(s->pos.y + o->y), 0, sp->u, sp->v, 0, 0);
    }
    list = dl_ambient_color(list, 0xFFFFFFFF);
    switch (m->state) {
    case 5:
        p = (int)ceilf_vfpu(powf_vfpu(10.0f, fmoney_ndigits((int)fabsf_vfpu(0.1f * m->delta))));
        v = m->shown - m->shown % p;
        v += (int)(rand() & 0xFFFF) % p;
        sprintf(buf, "%d", v);
        break;
    case 3:
    case 4:
        sprintf(buf, "%d", m->shown);
        break;
    default:
        sprintf(buf, "%d", sysstate_get_u32_54(0));
        break;
    }
    return fmoney_number(list, buf, (int)(s->pos.x + fmoney_text_off.x), (int)(s->pos.y + fmoney_text_off.y), 0, -2,
                         1.0f, 0);
}

void fmoney_nop(void)
{
}

void fmoney_update_pop(void)
{
    FieldMoney *const m = &field_money;
    FieldMoneyPop *const p = &field_money.pop;

    switch (field_money.state) {
    case 3:
        if (m->wait < 10) {
            m->wait++;
            p->pos.x = m->dfrom.x;
            p->pos.y = m->dfrom.y;
            p->scale = 1.0f;
        } else {
            m->state = 4;
        }
        break;
    case 4:
        vec2_lerp(&p->pos, &m->dfrom, &m->to, p->t);
        p->scale = 1.0f + -0.75f * p->t;
        if (p->t >= 1.0f) {
            m->state = 5;
        } else {
            p->t = p->t + (0.1f + 0.1f * p->t);
            if (p->t > 1.0f)
                p->t = 1.0f;
        }
        break;
    }
}

void *fmoney_draw_pop(void *list)
{
    FieldMoney *const m = &field_money;
    FieldMoneyPop *const p = &field_money.pop;
    char buf[256];
    int n;

    switch (field_money.state) {
    case 3:
    case 4:
        list = dl_ambient_color(list, 0xFF0000FF);
        list = gim_set_texture(list, m->gim);
        if (m->delta < 0) {
            sprintf(buf, "%d", -m->delta);
            n = strlen(buf);
            list = fmoney_sprite(list, (int)((p->pos.x + fmoney_text_off.x) - 38.0f * p->scale * n),
                                 (int)(p->pos.y + fmoney_text_off.y), 0, fmoney_sign.u, fmoney_sign.v, fmoney_sign.w,
                                 fmoney_sign.h, p->scale);
        } else {
            sprintf(buf, "%d", m->delta);
        }
        return fmoney_number(list, buf, (int)(p->pos.x + fmoney_text_off.x), (int)(p->pos.y + fmoney_text_off.y), 0,
                             -2, p->scale, 1);
    }
    return list;
}

void fmoney_nop2(void)
{
}

void *fmoney_sprite(void *list, int x, int y, int z, int u, int v, int w, int h, float scale)
{
    list = dl_tex_wrap(list, 1, 1);
    list = draw_set_offsets(list, w, h, (int)(w * scale), (int)(h * scale));
    return draw_sprite(list, x, y, z, u, v, 0, 0);
}

void *fmoney_number(void *list, char *s, int x, int y, int z, int gap, float scale, int big)
{
    int last = strlen(s) - 1;
    int i;
    int d;
    int u, v, w, h;

    for (i = last; i >= 0; i--) {
        d = s[i] - '0';
        switch (big) {
        case 0:
            w = fmoney_digit.w;
            h = fmoney_digit.h;
            u = fmoney_digit.u + fmoney_digit.w * d;
            v = fmoney_digit.v;
            break;
        case 1:
            u = fmoney_big[d].u;
            v = fmoney_big[d].v;
            w = 0x28;
            h = 0x30;
            break;
        }
        list = fmoney_sprite(list, x - (int)(scale * (w + gap)) * (last - i), y, z, u, v, w, h, scale);
    }
    return list;
}

int fmoney_ndigits(int n)
{
    int c = 0;

loop:
    if (n > 0) {
        c++;
        n *= 0.1f;
        goto loop;
    }
    return c;
}
