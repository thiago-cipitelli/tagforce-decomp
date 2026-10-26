// @range 0x08812FBC-0x08813308
/* HUD parts: the room name and the clock sliding in, and the clock drawing dispatch. */
#include "libfield.h"

int fld_day_part(void);
void *func_08813308(void *list);
void *func_088135E8(void *list);
void *func_08813AA8(void *list);
void *func_08813D34(void *list);

extern Vec2 field_name_from;                   /* 0x0883C7CC */
extern Vec2 field_name_to;                     /* 0x0883C7D4 */
extern Vec2 field_clock_from;                  /* 0x0883C7DC */
extern Vec2 field_clock_to;                    /* 0x0883C7E4 */
extern Vec2 field_clock_night;                 /* 0x0883C7EC */

void fld_hud_name_update(void)
{
    FieldHudName *s = &field_hud.name;
    Vec2 from = field_name_from;
    Vec2 to = field_name_to;

    switch (s->state) {
    case 1:
        vec2_lerp(&s->pos, &from, &to, s->t);
        if (s->t >= 1.0f) {
            s->state = 2;
        } else {
            s->t = s->t + (0.1f + 0.1f * s->t);
            if (s->t > 1.0f)
                s->t = 1.0f;
        }
        break;
    case 2:
        break;
    }
    s->item.x = s->pos.x;
    s->item.y = s->pos.y;
}

void *fld_hud_name_draw(void *list)
{
    return list;
}

void fld_hud_clock_free(void)
{
}

void fld_hud_clock_open(void)
{
    field_hud.clock.state = 1;
}

void fld_hud_clock_update(void)
{
    FieldHudSlide *s = &field_hud.clock;
    Vec2 from = field_clock_from;
    Vec2 to = field_clock_to;

    switch (s->state) {
    case 1:
        vec2_lerp(&s->pos, &from, &to, s->t);
        switch (fld_day_part()) {
        case 2:
        case 3: {
            Vec2 d = field_clock_night;
            vec2_add(&s->pos, &s->pos, &d);
            break;
        }
        }
        if (s->t >= 1.0f) {
            s->state = 2;
        } else {
            s->t = s->t + (0.1f + 0.1f * s->t);
            if (s->t > 1.0f)
                s->t = 1.0f;
        }
        break;
    case 2:
        break;
    }
}

void *fld_hud_clock_draw(void *list)
{
    FieldHud *h = &field_hud;

    if (field_hud.clock.state == 0)
        return list;
    switch (region_get()) {
    case 1:
    default:
        list = func_088135E8(list);
        break;
    case 0:
        list = func_08813308(list);
        break;
    }
    switch (fld_day_part()) {
    case 2:
    case 3:
        return list;
    }
    if (h->blink_kind != 3 || !h->blink_on || !h->blink)
        list = func_08813D34(list);
    if (h->blink_kind != 4 || !h->blink_on || !h->blink)
        list = func_08813AA8(list);
    return list;
}
