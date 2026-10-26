// @range 0x08813308-0x08814D94
// @asm 0x08813308-0x088135E8
// @asm 0x088135E8-0x08813914
// @asm 0x08813914-0x08813AA8
// @asm 0x08813AA8-0x08813D34
// @asm 0x08813D34-0x08813E2C
// @asm 0x08813F18-0x088140D8
// @asm 0x088141C4-0x08814404
// @asm 0x08814514-0x088147E0
// @asm 0x08814930-0x08814B5C
/* HUD parts sliding in and out: day and date, the moving icon (waits, then flies scaling down),
   the panel and the message box; their drawings are still in asm. */
#include "libfield.h"

extern Vec2 field_day_from;                    /* 0x0883C830 */
extern Vec2 field_day_to;                      /* 0x0883C838 */
extern Vec2 field_date_from;                   /* 0x0883C840 */
extern Vec2 field_date_to;                     /* 0x0883C848 */
extern Vec2 field_panel_from;                  /* 0x0883C850 */
extern Vec2 field_panel_to;                    /* 0x0883C858 */
extern Vec2 field_box_pos;                     /* 0x0883C860 */
extern Vec2 field_box_from;                    /* 0x0883C868 */
extern Vec2 field_box_to;                      /* 0x0883C870 */

void fld_hud_day_free(void)
{
}

void fld_hud_day_open(void)
{
    field_hud.day.state = 1;
}

void fld_hud_day_update(void)
{
    FieldHudSlide *s = &field_hud.day;
    Vec2 from = field_day_from;
    Vec2 to = field_day_to;

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
}

void fld_hud_date_free(void)
{
}

void fld_hud_date_open(void)
{
    field_hud.date.state = 1;
}

void fld_hud_date_update(void)
{
    FieldHudSlide *s = &field_hud.date;
    Vec2 from = field_date_from;
    Vec2 to = field_date_to;

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
}

void fld_hud_move_update(void)
{
    FieldHudMove *m = &field_hud.move;
    Vec2 to = *(Vec2 *)&field_hud.date.pos;

    switch (m->on) {
    case 1:
        if (m->wait < 10)
            m->wait++;
        else
            m->on = 2;
        break;
    case 2:
        to.x = to.x + 20.0f;
        to.y = to.y + 5.0f;
        vec2_lerp(&m->pos, &m->from, &to, m->t);
        m->s = 1.0f + -0.75f * m->t;
        if (m->t >= 1.0f) {
            m->on = 0;
        } else {
            m->t = m->t + (0.1f + 0.1f * m->t);
            if (m->t > 1.0f)
                m->t = 1.0f;
        }
        break;
    }
}

void fld_hud_panel_free(void)
{
}

void fld_hud_panel_open(void)
{
    field_hud.panel.state = 1;
}

void fld_hud_panel_update(void)
{
    FieldHudSlide *s = &field_hud.panel;
    Vec2 from = field_panel_from;
    Vec2 to = field_panel_to;

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
    case 3:
        vec2_lerp(&s->pos, &from, &to, s->t);
        if (s->t <= 0.0f) {
            s->state = 0;
        } else {
            s->t = s->t - (0.1f + 0.1f * s->t);
            if (s->t < 0.0f)
                s->t = 0.0f;
        }
        break;
    }
}

void fld_hud_box_free(void)
{
}

void fld_hud_box_init(void)
{
    MenuItemDef *d = &field_hud.boxw.item;
    FieldHudName *b = &field_hud.boxw;

    memset(d, 0, sizeof(MenuItemDef));
    d->visible = 1;
    d->show_title = 1;
    d->auto_title = 1;
    d->font_flag4 = 1;
    d->style2 = 2;
    d->x = 0x27;
    d->y = 0xF2;
    d->w = 400;
    d->h = 0x1B;
    d->color = 0xFFFFFFFF;
    d->title_color = 0xFFFFFFFF;
    d->title_font_size = 14;
    d->style = 0;
    d->title = 0;
    menu_add_item(&field_hud.menu, d);
    b->state = 0;
    {
        Vec2 t = field_box_pos;
        b->pos = t;
    }
}

void fld_hud_box_update(void)
{
    FieldHudName *s = &field_hud.boxw;
    Vec2 from = field_box_from;
    Vec2 to = field_box_to;

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
    case 3:
        vec2_lerp(&s->pos, &from, &to, s->t);
        if (s->t <= 0.0f) {
            s->state = 0;
        } else {
            s->t = s->t - (0.1f + 0.1f * s->t);
            if (s->t < 0.0f)
                s->t = 0.0f;
        }
        break;
    }
    s->item.x = s->pos.x;
    s->item.y = s->pos.y;
}
