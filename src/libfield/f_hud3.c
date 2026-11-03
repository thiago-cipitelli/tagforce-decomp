// @range 0x08813308-0x08814D94
/* HUD parts sliding in and out: day and date, the moving icon (waits, then flies scaling down),
   the panel and the message box; their drawings are still in asm. */
#include "libfield.h"
#include "allegrex.h"

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

typedef struct FieldHudCell {           /* sprite of a HUD part, 0xC */
    short u, v, w, h;
    int gim;                           /* index in field_hud.gim0.. */
} FieldHudCell;

void *fld_hud_clock_face(void *list, int dx1, int dx2, int dx3);
extern const char str_d[];
void *fld_digits(void *list, char *s, int x, int y, int z, int u, int v, int w, int h, int gap,
                 float scale);
extern const FieldHudCell field_clock_cells[3];  /* 0x08838948 the two plates and the digits */
extern const int field_clock_pos[3][2];         /* 0x0883896C */

/* clock of the HUD, layout A: the face, then (unless blinking off, blink_kind 1) the two plates and
   st->w[0] and st->w[1] in digits */
void *fld_hud_clock_draw_a(void *list)
{
    FieldHud *h = &field_hud;
    FieldHudSlide *s = &field_hud.clock;
    FieldStory *st = fld_story_get();
    int i;
    const FieldHudCell *c;
    const int *p;
    char buf[256];

    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = fld_hud_clock_face(list, 5, 30, 38);
    if (field_hud.blink_kind != 1 || h->blink_on == 0 || h->blink == 0) {
        for (i = 0, c = field_clock_cells, p = field_clock_pos[0]; i < 2; i++, c++, p += 2) {
            list = gim_set_texture(list, (&h->gim0)[c->gim]);
            list = fld_draw_cell1(list, s->pos.x + (float)p[0], s->pos.y + (float)p[1], c->u, c->v, c->w, c->h);
        }
        if (st->w[0] != 0) {
            sprintf(buf, str_d, *(int *)st->w[0]);
            list = gim_set_texture(list, (&h->gim0)[field_clock_cells[2].gim]);
            list = fld_digits(list, buf, s->pos.x + (float)field_clock_pos[2][0],
                              s->pos.y + (float)field_clock_pos[2][1], 0, field_clock_cells[2].u,
                              field_clock_cells[2].v, field_clock_cells[2].w, field_clock_cells[2].h, -4, 1.0f);
        }
        if (st->w[1] != 0) {
            sprintf(buf, str_d, *(int *)st->w[1]);
            list = gim_set_texture(list, (&h->gim0)[field_clock_cells[2].gim]);
            list = fld_digits(list, buf, s->pos.x + (float)field_clock_pos[2][0] + 38.0f,
                              s->pos.y + (float)field_clock_pos[2][1], 0, field_clock_cells[2].u,
                              field_clock_cells[2].v, field_clock_cells[2].w, field_clock_cells[2].h, -4, 1.0f);
        }
    }
    return list;
}

typedef struct FieldMonthW { int w[4]; } FieldMonthW;
typedef struct FieldMonthUv { int uv[4][2]; } FieldMonthUv;
extern const FieldMonthW field_month_w;         /* 0x0883C7F8 width of the month names */
extern const FieldMonthUv field_month_uv;       /* 0x0883C808 */
extern const FieldHudCell field_day_suffix[4];  /* 0x08838A7C st, nd, rd, th */

/* clock of the HUD, layout B: the face, then (unless blinking off) the month name (st->w[0], 5..8)
   and the day st->w[1] with its ordinal suffix */
void *fld_hud_clock_draw_b(void *list)
{
    char buf[256];
    FieldHud *h = &field_hud;
    FieldHudSlide *s = &field_hud.clock;
    FieldStory *st = fld_story_get();
    int m = *(int *)st->w[0] - 5;
    FieldMonthW w = field_month_w;
    FieldMonthUv uv = field_month_uv;
    const FieldHudCell *c;
    int d;

    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = fld_hud_clock_face(list, -20, 18, 24);
    if (field_hud.blink_kind != 1 || h->blink_on == 0 || h->blink == 0) {
        list = gim_set_texture(list, h->gim1);
        if (st->w[0] != 0)
            list = fld_draw_cell1(list, 31.0f + s->pos.x, 3.0f + s->pos.y, uv.uv[m][0], uv.uv[m][1], w.w[m], 16);
        if (st->w[1] != 0) {
            sprintf(buf, str_d, *(int *)st->w[1]);
            list = fld_digits(list, buf, s->pos.x - 3.0f, 3.0f + s->pos.y - 1.0f, 0, 0, 32, 16, 16, -4, 1.0f);
            switch (d = *(int *)st->w[1]) {
            case 11:
            case 12:
            case 13:
                c = &field_day_suffix[3];
                break;
            default:
                switch (d % 10) {
                case 1:
                    c = &field_day_suffix[0];
                    break;
                case 2:
                    c = &field_day_suffix[1];
                    break;
                case 3:
                    c = &field_day_suffix[2];
                    break;
                default:
                    c = &field_day_suffix[3];
                    break;
                }
                break;
            }
            list = fld_draw_cell1(list, 11.0f + s->pos.x, 3.0f + s->pos.y, c->u, c->v, c->w, c->h);
        }
    }
    return list;
}

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
    if (field_hud.blink_kind != 2 || h->blink_on == 0 || h->blink == 0) {
        if (st->w[2] != 0) {
            list = gim_set_texture(list, h->gim1);
            list = fld_day_icon(list, *(int *)st->w[2], s->pos.x + (float)dx3, s->pos.y + 22.0f, 0);
        }
    }
    return list;
}

void *fld_digit(float scale, void *list, int x, int y, int z, int u, int v, int w, int h, int frame);
extern const char str_02d[];                   /* 0x0883C828 "%02d" */
extern const char str_d[];
void *fld_digits(void *list, char *s, int x, int y, int z, int u, int v, int w, int h, int gap,
                 float scale);

/* the digital time of the HUD clock: the colon, AM/PM (hour >= 12), the hour % 12 and the minutes */
void *fld_hud_clock_time(void *list)
{
    FieldHud *h = &field_hud;
    FieldHudSlide *s = &field_hud.clock;
    FieldStory *st = fld_story_get();
    int cx;
    int ax;
    int hx;
    char buf[256];

    switch (region_get()) {
    case 1:
    default:
        cx = 66;
        ax = 95;
        hx = 54;
        break;
    case 0:
        cx = 90;
        ax = 44;
        hx = 78;
        break;
    }
    list = gim_set_texture(list, h->gim0);
    list = fld_draw_cell1(list, s->pos.x + (float)cx, 40.0f + s->pos.y, 81, 0, 6, 16);
    if (st->w[3] != 0)
        list = fld_digit(1.0f, list, s->pos.x + (float)ax, 40.0f + s->pos.y, 0, 27, 0, 27, 16,
                         *(int *)st->w[3] >= 12);
    if (st->w[3] != 0) {
        sprintf(buf, str_d, *(int *)st->w[3] % 12);
        list = fld_digits(list, buf, s->pos.x + (float)hx, 40.0f + s->pos.y, 0, 87, 0, 12, 16, -1, 1.0f);
    }
    if (st->w[4] != 0) {
        sprintf(buf, str_02d, *(int *)st->w[4]);
        list = fld_digits(list, buf, 30.0f + (s->pos.x + (float)hx), 40.0f + s->pos.y, 0, 87, 0, 12, 16, -1,
                          1.0f);
    }
    return list;
}

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

/* the counter of the HUD (st->w[15]() clamped to 0..999): its plate, the digits and the label,
   all right-aligned on the number */
void *fld_hud_date_draw(void *list)
{
    FieldHud *h = &field_hud;
    FieldHudSlide *s = &field_hud.date;
    FieldStory *st = fld_story_get();
    void *gim = fld_gim_get(2);
    int n = ((int (*)(void))st->w[15])();
    int w;
    int x;
    unsigned short *text;
    char buf[8];

    if (s->state == 0)
        return list;
    sprintf(buf, str_d, ALLEGREX_MIN(ALLEGREX_MAX(n, 0), 999));
    w = strlen(buf) * 10;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, h->gim0);
    list = fld_draw_cell1(list, s->pos.x - (float)w, -14.0f + s->pos.y, 0x7E, 0x20, 0x11, 0x1D);
    list = gim_set_texture(list, gim);
    if (st != 0 && st->w[15] != 0)
        list = fld_digits(list, buf, 18.0f + s->pos.x, -1.0f + s->pos.y, 0, 0x57, 0, 0xC, 0x10, -2, 1.0f);
    font_text_set(list);
    font_set_size(15, 15);
    font_set_flag4(1);
    font_set_color(0xFFFFFFFF);
    text = table_get_entry(fld_gim_get(5), 0x18);
    x = 15.0f + s->pos.x - (float)w;
    font_draw_text(x << 6, (int)s->pos.y << 6, (480 - x) << 6, text);
    return font_text_clear();
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

extern const unsigned char field_move_digits[10][2]; /* 0x088389C0 big digit cells (u, v), 40 x 48 */
extern const FieldHudCell field_move_minus;     /* 0x088389D4 */
void *fld_digit(float scale, void *list, int x, int y, int z, int u, int v, int w, int h, int frame);

/* the step count of the HUD (move.arg, scaled by move.s): its plate, a minus when negative and the
   big digits */
void *fld_hud_move_draw(void *list)
{
    FieldHud *h = &field_hud;
    FieldHudMove *m = &field_hud.move;
    void *gim = fld_gim_get(2);
    float off;
    int i;
    char buf[8];

    switch (m->on) {
    case 1:
    case 2:
        list = dl_set_tex_filter(list, 0, 0);
        list = gim_set_texture(list, h->gim0);
        if (m->arg < 0)
            off = (float)(field_move_minus.w - 2) * m->s;
        else
            off = 0.0f;
        list = draw_set_offsets(list, 17, 29, 17, 29);
        list = draw_sprite(list, -10.0f + (m->pos.x - 8.5f) - (float)(int)off, m->pos.y - 14.5f, 0, 126, 32, 0, 0);
        list = gim_set_texture(list, gim);
        if (m->arg < 0) {
            sprintf(buf, str_d, -m->arg);
            list = fld_digit(m->s, list, m->pos.x - (float)(field_move_minus.w - 2) * m->s,
                             m->pos.y - 0.5f * ((float)field_move_minus.h * m->s), 0, field_move_minus.u,
                             field_move_minus.v, field_move_minus.w, field_move_minus.h, 0);
        } else {
            sprintf(buf, str_d, m->arg);
        }
        for (i = strlen(buf) - 1; i >= 0; i--) {
            int d = buf[i] - '0';
            float k = 38.0f;
            int dx = (int)(m->s * k);

            list = fld_digit(m->s, list, m->pos.x + (float)(dx * i),
                             m->pos.y - 0.5f * (48.0f * m->s), 0, field_move_digits[d][0], field_move_digits[d][1],
                             40, 48, 0);
        }
        return list;
    }
    return list;
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

extern const FieldHudCell field_panel_cells[3];  /* 0x088389E0 the two plates and the digits */
extern const int field_panel_pos[3][2];         /* 0x08838A04 */

/* the panel of the HUD: plate 0, plate 1 and the count of st->w[8] unless hidden (phase) or zero */
void *fld_hud_panel_draw(void *list)
{
    FieldHud *h = &field_hud;
    FieldHudSlide *s = &field_hud.panel;
    FieldStory *st = fld_story_get();
    int i;
    const FieldHudCell *c;
    const int *p;
    char buf[256];

    if (s->state == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    for (i = 0, c = field_panel_cells, p = field_panel_pos[0]; i < 2; i++, c++, p += 2) {
        if (i == 1) {
            if (h->phase != 0)
                continue;
            if (st->w[8] != 0 && *(int *)st->w[8] <= 0)
                continue;
        }
        list = gim_set_texture(list, (&h->gim0)[c->gim]);
        list = fld_draw_cell1(list, s->pos.x + (float)p[0], s->pos.y + (float)p[1], c->u, c->v, c->w, c->h);
    }
    if (h->phase == 0 && st->w[8] != 0 && *(int *)st->w[8] > 0) {
        sprintf(buf, str_d, *(int *)st->w[8]);
        list = gim_set_texture(list, (&h->gim0)[field_panel_cells[2].gim]);
        list = fld_digits(list, buf, s->pos.x + (float)field_panel_pos[2][0], s->pos.y + (float)field_panel_pos[2][1],
                          0, field_panel_cells[2].u, field_panel_cells[2].v, field_panel_cells[2].w,
                          field_panel_cells[2].h, -1, 1.0f);
    }
    return list;
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
