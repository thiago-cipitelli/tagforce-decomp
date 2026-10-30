// @range 0x08813308-0x08814D94
// @asm 0x08813308-0x088135E8
// @asm 0x088135E8-0x08813914
// @asm 0x08813914-0x08813AA8
// @asm 0x08813AA8-0x08813D34
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

void *dl_ambient_color(void *list, unsigned int color);
FieldStory *fld_story_get(void);
void *dl_set_tex_filter(void *list, int min, int mag);
void *fld_draw_cell1(void *list, int x, int y, int u, int v, int w, int h);
void *fld_clock_hands(void *list, int x, int y, int z, int h, int m);
void *fld_day_icon(void *list, int n, int x, int y, int z);

#ifdef NON_MATCHING /* C novo, conferido com o asm. 43 palavras: o teste de blink_kind sai bnel/beqz sem likely no original e a lista volta em v0 sem cópia (aqui com return explícito) */
/* the clock face of the HUD slid to field_hud.clock.pos: the two plates, then the weekday icon unless
   it blinks off (blink_kind 2) */
void *fld_hud_clock_face(void *list, int dx1, int dx2, int dx3)
{
    FieldHud *h = &field_hud;
    FieldHudSlide *s = &field_hud.clock;
    FieldStory *st = fld_story_get();

    list = gim_set_texture(list, field_hud.gim0);
    list = fld_draw_cell1(list, s->pos.x + (float)dx1, s->pos.y, 0, 0x58, 0xC0, 0x16);
    list = fld_draw_cell1(list, s->pos.x + (float)dx2, s->pos.y + 21.0f, 0x1C, 0x6E, 0xA4, 0x12);
    if (field_hud.blink_kind == 2 && h->blink_on != 0 && h->blink != 0)
        return list;
    if (st->w[2] != 0) {
        list = gim_set_texture(list, h->gim1);
        list = fld_day_icon(list, *(int *)st->w[2], s->pos.x + (float)dx3, s->pos.y + 22.0f, 0);
    }
    return list;
}
#endif

/* the hands of the small clock of the HUD at the hour and minute of the story */
void *fld_hud_clock_mark(void *list)
{
    FieldHudSlide *s = &field_hud.clock;
    FieldStory *st = fld_story_get();

    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, field_hud.gim0);
    list = fld_draw_cell1(list, s->pos.x + 79.0f, s->pos.y, 0, 0x20, 0x2A, 0x2A);
    return fld_clock_hands(list, s->pos.x + 97.0f, s->pos.y + 1.0f, 0, *(int *)st->w[3], *(int *)st->w[4]);
}

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

typedef struct FieldHudCell {           /* sprite of a HUD part, 0xC */
    short u, v, w, h;
    int gim;                           /* index in field_hud.gim0.. */
} FieldHudCell;

typedef struct FieldHudDayPos {         /* weekday plate by language, 8 */
    signed char x, y;
    unsigned char u, v, w, h;
    signed char dx, dy;                /* the day number */
} FieldHudDayPos;

extern const FieldHudDayPos field_day_pos[];   /* 0x08838984 */
extern const FieldHudCell field_day_digits;    /* 0x088389B4 */
extern const char str_d[];                     /* 0x0883C7F4 "%d" */
void *fld_digits(void *list, char *s, int x, int y, int z, int u, int v, int w, int h, int gap,
                 float scale);

/* the weekday plate of the HUD and the day of the month */
void *fld_hud_day_draw(void *list)
{
    FieldHud *h = &field_hud;
    FieldHudSlide *s = &field_hud.day;
    FieldStory *st = fld_story_get();
    const FieldHudDayPos *p;
    char buf[256];

    if (s->state == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, h->gim1);
    p = &field_day_pos[cfg_get_language()];
    list = fld_draw_cell1(list, s->pos.x + (float)p->x, s->pos.y + (float)p->y, p->u, p->v, p->w, p->h);
    if (st->w[5] != 0) {
        sprintf(buf, str_d, *(int *)st->w[5]);
        list = gim_set_texture(list, (&h->gim0)[field_day_digits.gim]);
        list = fld_digits(list, buf, s->pos.x + (float)p->dx, s->pos.y + (float)p->dy, 0, field_day_digits.u,
                          field_day_digits.v, field_day_digits.w, field_day_digits.h, -1, 1.0f);
    }
    return list;
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
