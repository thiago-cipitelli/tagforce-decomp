// @range 0x08811BE0-0x08812FBC
// @asm 0x08811BE0-0x08812080
// @asm 0x08812080-0x08812140
// @asm 0x08812140-0x08812460
// @asm 0x0881246C-0x08812608
// @asm 0x088129AC-0x08812BD0
// @asm 0x08812EB0-0x08812FBC
/* HUD of the field: the room name window, the event icon of the cell the player stands on, the
   blinking of the HUD parts, the slide-in of a part, the text callbacks of the menus. */
#include "libfield.h"

int fld_bg_event(int bg, int cx, int cz);      /* 0x0880CF74 */
int fld_bg_group(int x);
void fld_hud_name_init(void);
void fld_hud_name_update(void);
void fld_hud_clock_free(void);
void fld_hud_clock_open(void);
void fld_hud_clock_update(void);
void fld_hud_day_free(void);
void fld_hud_day_open(void);
void fld_hud_day_update(void);
void fld_hud_date_free(void);
void fld_hud_date_open(void);
void fld_hud_date_update(void);
void fld_hud_move_update(void);
void fld_hud_panel_free(void);
void fld_hud_panel_open(void);
void fld_hud_panel_update(void);
void fld_hud_box_free(void);
void fld_hud_box_init(void);
void fld_hud_box_update(void);
void fld_hud_draw_free(void);

int fld_sctab_state(void)
{
    return field_scene_tab.state;
}

void fld_sctab_draw_text(void *list, unsigned short i, int x, int y)
{
    int k = field_scene_tab.idx[i];

    font_text_set(list);
    font_set_size(15, 15);
    font_set_flag4(1);
    font_set_color(0xFF000000);
    font_draw_text((x + 4) << 6, y << 6, (480 - (x + 4)) << 6, field_scene_tab.texts[k]);
    font_text_clear();
}

void fld_sctab_draw_text2(void *list, unsigned short i, int x, int y)
{
    font_text_set(list);
    font_set_size(15, 15);
    font_set_flag4(1);
    font_set_color(0xFF000000);
    font_draw_text((x + 4) << 6, y << 6, (480 - (x + 4)) << 6, field_scene_tab.texts2[i]);
    font_text_clear();
}

/* event of the cell under the player and its label */
void fld_hud_event(unsigned short **text, int *ev)
{
    Vec3 *p = &fld_player_get()->pos;
    int e = fld_bg_event(fld_bg_id(), p->x / 0.5f, p->z / 0.5f);

    *ev = e;
    switch (e) {
    case 0:
        *text = table_get_entry(fld_gim_get(5), 1);
        break;
    case 1:
        *text = table_get_entry(fld_gim_get(5), 8);
        break;
    case 2:
        *text = table_get_entry(fld_gim_get(5), 9);
        break;
    case 3:
        *text = table_get_entry(fld_gim_get(5), 10);
        break;
    case 4:
        *text = table_get_entry(fld_gim_get(5), 11);
        break;
    }
}

void fld_sctab_reset(void)
{
    menu_reset(&field_scene_tab.w1);
    menu_reset(&field_scene_tab.w2);
}

void fld_hud_init(void)
{
    memset(&field_hud, 0, sizeof(FieldHud));
}

void fld_hud_free(void)
{
    fld_hud_clock_free();
    fld_hud_day_free();
    fld_hud_date_free();
    fld_hud_panel_free();
    fld_hud_box_free();
    fld_hud_draw_free();
    menu_free_items(&field_hud.menu);
}

void fld_hud_update(void)
{
    FieldHud *h = &field_hud;

    if (field_hud.on) {
        fld_hud_name_update();
        fld_hud_clock_update();
        fld_hud_day_update();
        fld_hud_date_update();
        fld_hud_move_update();
        fld_hud_panel_update();
        fld_hud_box_update();
        if (h->blink_on) {
            if ((int)(*gfx_get_state() % h->period) < h->duty)
                h->blink = 1;
            else
                h->blink = 0;
        }
    }
}

void fld_hud_open(int story)
{
    FieldHud *h = &field_hud;

    menu_init(&field_hud.menu, fld_heap_get());
    field_hud.gim0 = fld_gim_get(0);
    field_hud.gim1 = fld_gim_get(1);
    field_hud.gim19 = fld_gim_get(0x13);
    switch (story) {
    case 0:
        fld_hud_name_init();
        fld_hud_clock_open();
        fld_hud_day_open();
        fld_hud_date_open();
        fld_hud_panel_open();
        fld_hud_box_init();
        break;
    case 1:
        fld_hud_clock_open();
        fld_hud_day_open();
        fld_hud_date_open();
        break;
    }
    h->on = 1;
}

void fld_hud_box_open(unsigned short *title, int a, int b, int c, int d, int e)
{
    int *p = field_hud.p;
    int *st = &field_hud.boxw.state;
    MenuItemDef *box = &field_hud.boxw.item;

    if (*st != 1) {
        box->title = title;
        *st = 1;
        p[0] = a;
        p[1] = b;
        p[2] = c;
        p[3] = d;
        p[4] = e;
    }
}

void fld_hud_box_close(void)
{
    int *st = &field_hud.boxw.state;

    if (*st != 3)
        *st = 3;
}

int fld_hud_flash(void)
{
    FieldHud *h = &field_hud;

    if (field_hud.t < 60) {
        h->phase = (h->t / 5) % 2;
        h->t++;
    } else {
        h->phase = 0;
        h->t = 0;
        return 0;
    }
    return 1;
}

void fld_hud_move(int arg, int x, int y)
{
    FieldHudMove *m = &field_hud.move;

    m->on = 1;
    m->arg = arg;
    m->from.y = y;
    m->pos.y = y;
    m->t = 0.0f;
    m->wait = 0;
    m->s = 1.0f;
    m->from.x = x;
    m->pos.x = x;
    fld_story_flag(0x621D, 0);
}

int fld_hud_move_done(void)
{
    return field_hud.move.on == 0;
}

void fld_hud_show(void)
{
    int *st = &field_hud.panel.state;

    if (*st != 1)
        *st = 1;
}

void fld_hud_hide(void)
{
    int *st = &field_hud.panel.state;

    if (*st != 3)
        *st = 3;
}

int fld_hud_hidden(void)
{
    int r;

    switch (field_hud.panel.state) {
    case 0:
        r = 1;
        break;
    case 3:
        r = 0;
        break;
    default:
        r = 0;
        break;
    }
    return r;
}

void fld_hud_blink(int period, int duty, int kind)
{
    FieldHud *h = &field_hud;

    if (period >= 0) {
        if (h->blink_on == 0) {
            h->period = period;
            h->duty = duty;
            h->blink_kind = kind;
            h->blink_on = 1;
        }
    } else {
        h->blink_on = 0;
    }
}

#ifdef NON_MATCHING /* o original materializa &field_hud, &item_on e &item em s1/s0/s3 no início; aqui só o último */
void fld_hud_name_init(void)
{
    FieldHud *h = (FieldHud *)(char *)&field_hud;
    int *on = (int *)(char *)&field_hud.name.state;
    MenuItemDef *d = (MenuItemDef *)(char *)&field_hud.name.item;
    int g;

    fld_story_get();
    g = fld_bg_group(fld_bg_id());
    memset(d, 0, sizeof(MenuItemDef));
    d->visible = 1;
    d->auto_size = 1;
    d->style = 2;
    d->x = 4;
    d->y = 4;
    d->color = 0xFFFFFFFF;
    d->text = table_get_entry(fld_gim_get(8), g);
    d->font_flag4 = 1;
    d->font_flag8000 = cfg_get_language() == 0;
    d->font_size = 15;
    d->text_color = 0xFFFFFFFF;
    d->pad_b = -1;
    d->pad_t = 1;
    d->pad_l = 3;
    d->pad_r = 2;
    menu_add_item(&h->menu, d);
    *on = 1;
}
#endif
