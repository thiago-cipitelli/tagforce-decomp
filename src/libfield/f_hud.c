// @range 0x08811BE0-0x08812FBC
// @rodata 0x0883892C-0x08838948
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

PadState *pad_get(void);
void fbag_init(void);
int fbag_update(int *picked);
void fld_dlg_open(unsigned short *text, int a, int b);
int fld_dlg_update(void);
void fld_sctab_reset(void);
void fld_sctab_open2(void);
int fld_day_part(void);
void *dl_alpha_test(void *list, int func, int ref, int mask);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_tex_func(void *list, int func, int comp);
void *dl_tex_mode(void *list, int mode, int a, int b, int c);
void *dl_tex_wrap(void *list, int u, int v);
void *fld_hud_name_draw(void *list);
void *fld_hud_clock_draw(void *list);
void *fld_hud_day_draw(void *list);
void *fld_hud_date_draw(void *list);
void *fld_hud_move_draw(void *list);
void *fld_hud_panel_draw(void *list);
void *fld_hud_draw_all(void *list);

/* the scene tabs: tab bar (0, closing 1), bag (2), the two choices (3, closing 4), the dialog (5),
   back to the bar (6). Sets *cmd/*arg to the scene command chosen (0 cancelled). */
void fld_sctab_update(int *cmd, int *arg, int *picked)
{
    FieldScTab *t = &field_scene_tab;
    int r;
    int id;
    int done;

    pad_get();
    switch (field_scene_tab.state) {
    case 0:
        r = menu_update(&t->w1);
        if (r == -1)
            break;
        id = t->idx[r];
        if (r == -2 || id == 0 || id == 1 || id == 4) {
            menu_finish(&t->w1);
            t->state = 1;
        }
        if (id == 2) {
            fbag_init();
            t->state = 2;
        } else if (id == 3) {
            fld_sctab_open2();
        } else if (id == 5) {
            fld_dlg_open(table_get_entry(fld_gim_get(5), 0x66), 3, 2);
            t->state = 5;
        }
        break;
    case 1:
        r = menu_update(&t->w1);
        if (t->w1.state != 6)
            break;
        id = t->idx[r];
        fld_sctab_reset();
        if (r == -2) {
            *cmd = 0;
        } else if (id == 0) {
            switch ((int)t->texts2[2]) {
            case 0:
                *cmd = 0x17;
                *arg = 2;
                break;
            case 1:
                *cmd = 0x11;
                break;
            case 2:
                *cmd = 0x13;
                break;
            case 3:
                *cmd = 0x17;
                *arg = 0xD;
                break;
            case 4:
                *cmd = 0x17;
                *arg = 0xE;
                break;
            }
        } else if (id == 1) {
            *cmd = 0x17;
            *arg = 5;
        } else if (id == 4) {
            *cmd = 0xB;
        }
        t->state = 0;
        break;
    case 2:
        switch (fbag_update(picked)) {
        case 1:
            t->state = 6;
            break;
        case 2:
            *cmd = 0x17;
            *arg = 3;
            fld_sctab_reset();
            t->state = 0;
            break;
        case 3:
            *cmd = 0x17;
            *arg = 4;
            fld_sctab_reset();
            t->state = 0;
            break;
        }
        break;
    case 3:
        r = menu_update(&t->w2);
        if (r == -1)
            break;
        switch (r) {
        case -2:
            menu_finish(&t->w2);
            t->state = 4;
            break;
        case 0:
        case 1:
            menu_finish(&t->w1);
            menu_finish(&t->w2);
            t->state = 4;
            break;
        }
        break;
    case 4:
        done = 0;
        r = menu_update(&t->w2);
        menu_update(&t->w1);
        if (r == -2) {
            if (t->w2.state == 6) {
                menu_reset(&t->w2);
                done = 1;
            }
        } else if (t->w1.state == 6 && t->w2.state == 6) {
            fld_sctab_reset();
            done = 1;
        }
        if (done) {
            switch (r) {
            case -2:
                t->state = 6;
                break;
            case 0:
                *cmd = 0x17;
                *arg = 6;
                t->state = 0;
                break;
            case 1:
                *cmd = 0x17;
                *arg = 7;
                t->state = 0;
                break;
            }
        }
        break;
    case 5:
        switch (fld_dlg_update()) {
        case -2:
        case 3:
            t->state = 6;
            break;
        case 2:
            fld_sctab_reset();
            *cmd = 0x17;
            *arg = 1;
            t->state = 0;
            break;
        }
        break;
    case 6:
        t->w1.no_cancel = 1;
        t->state = 0;
        t->w1.cancelled = t->w1.accepted = 0;
        break;
    }
}

void *fbag_draw(void *list);
void fld_hud_event(unsigned short **text, int *ev);
void fld_sctab_draw_text(void *list, unsigned short i, int x, int y);
void fld_sctab_draw_text2(void *list, unsigned short i, int x, int y);
extern const unsigned short field_sctab_item_w[][5]; /* 0x088388F0 width of the tab bar items by language and event */
void *fld_dlg_draw(void *list);

/* layer 3 of the scene tabs: the tab bar, then the page of the open tab */
void fld_sctab_draw(void)
{
    FieldScTab *t = &field_scene_tab;
    void *list = dl_select_layer(3, 0);

    menu_draw(&list, &field_scene_tab.w1);
    switch (field_scene_tab.state) {
    case 2:
        list = fbag_draw(list);
        break;
    case 3:
    case 4:
        menu_draw(&list, &t->w2);
        break;
    case 5:
        list = fld_dlg_draw(list);
        break;
    }
    dl_set_list(list);
}

/* opens the scene tabs: the six tabs (the sixth only when field_ctx.x14 is 0), their texts, the event
   of the cell and the tab bar widget */
void fld_sctab_open(void)
{
    FieldScTab *t = &field_scene_tab;
    MenuWidget *w = &field_scene_tab.w1;
    MenuItemDef *d = &field_scene_tab.w1.def;
    FieldCtx *ctx = fld_ctx_get();
    int lang = cfg_get_language();
    int on[6];
    int i;
    int n;
    int *p;
    int *q;

    on[3] = 0;
    on[0] = 1;
    on[1] = 1;
    on[2] = 1;
    on[4] = 1;
    on[5] = 1;
    if (ctx->x14 != 0)
        on[5] = 0;
    t->idx[0] = -1;
    t->idx[1] = -1;
    t->idx[2] = -1;
    t->idx[3] = -1;
    t->idx[4] = -1;
    t->idx[5] = -1;
    n = 0;
    for (i = 0, p = on, q = (int *)t; i < 6; i++, p++) {
        if (*p != 0) {
            ((FieldScTab *)q)->idx[0] = i; /* t->idx[n] = i through a pointer stepping over t */
            q++;
            n++;
        }
    }
    t->texts[0] = table_get_entry(fld_gim_get(5), 1);
    t->texts[2] = table_get_entry(fld_gim_get(5), 2);
    t->texts[1] = table_get_entry(fld_gim_get(5), 3);
    t->texts[3] = table_get_entry(fld_gim_get(5), 4);
    t->texts[4] = table_get_entry(fld_gim_get(5), 5);
    t->texts[5] = table_get_entry(fld_gim_get(5), 6);
    t->texts2[0] = table_get_entry(fld_gim_get(5), 1);
    t->texts2[1] = table_get_entry(fld_gim_get(5), 3);
    fld_hud_event(t->texts, (int *)&t->texts2[2]);
    memset(&t->w1, 0, sizeof(MenuWidget));
    d->visible = 1;
    d->auto_size = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->pad_t = 1;
    d->pad_b = 1;
    d->pad_l = 1;
    d->pad_r = 1;
    d->x = 4;
    d->y = 0x24;
    d->color = 0xFFFFFFFF;
    w->no_cancel = 1;
    w->cancel_blocked = 0;
    w->keep_open = 1;
    w->style = 2;
    w->buttons = 0;
    w->cursor = 0;
    w->button = 0;
    w->count = n;
    d->style = 0;
    w->item_w = ((const unsigned short *)((const char *)field_sctab_item_w + lang * 10))[(int)t->texts2[2]];
    w->scroll = 0;
    w->selected = 0;
    w->item_h = cfg_get_language() == 0 ? 0x11 : 0x13;
    w->cursor_h = cfg_get_language() == 0 ? 0xF : 0x11;
    w->draw_item = (MenuDrawItemFn)fld_sctab_draw_text;
    w->heap = fld_heap_get();
    menu_open(&t->w1);
    t->state = 0;
}

int fld_sctab_state(void)
{
    return field_scene_tab.state;
}

/* opens the page with the two choices of the scene tabs (state 3) */
void fld_sctab_open2(void)
{
    FieldScTab *t = &field_scene_tab;
    MenuWidget *w = &field_scene_tab.w2;
    MenuItemDef *d = &field_scene_tab.w2.def;

    memset(w, 0, sizeof(*w));
    d->visible = 1;
    d->auto_size = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->pad_t = 1;
    d->pad_b = 1;
    d->pad_l = 1;
    d->pad_r = 1;
    d->x = 0x4A;
    d->y = 0x51;
    d->color = 0xFFFFFFFF;
    w->no_cancel = 1;
    w->cancel_blocked = 0;
    w->keep_open = 1;
    w->count = 2;
    w->style = 2;
    w->buttons = 0;
    w->cursor = 0;
    d->style = 0;
    w->button = 0;
    w->item_w = 0x41;
    w->scroll = 0;
    w->selected = 0;
    w->item_h = cfg_get_language() == 0 ? 0x11 : 0x13;
    w->cursor_h = cfg_get_language() == 0 ? 0xF : 0x11;
    w->draw_item = (MenuDrawItemFn)fld_sctab_draw_text2;
    w->heap = fld_heap_get();
    menu_open(&t->w2);
    t->state = 3;
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

/* layer 3: the HUD parts, each skipped while it blinks off (blink_kind 0 menu, 5 day, 6 date) */
void fld_hud_draw(void)
{
    FieldHud *h = &field_hud;
    void *list;

    if (field_hud.on == 0)
        return;
    list = dl_select_layer(3, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_enable(list, 4);
    list = dl_alpha_test(list, 6, 0, 0xFF);
    list = dl_enable(list, 0);
    list = dl_disable(list, 1);
    list = dl_enable(list, 9);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_func(list, 0, 1);
    list = dl_tex_wrap(list, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    font_set_flag8000(0);
    if (h->blink_kind != 0 || h->blink_on == 0 || h->blink == 0)
        menu_layout(&list, &h->menu);
    list = fld_hud_name_draw(list);
    list = fld_hud_clock_draw(list);
    if (fld_day_part() < 2 && (h->blink_kind != 5 || h->blink_on == 0 || h->blink == 0))
        list = fld_hud_day_draw(list);
    if (fld_day_part() >= 2 && (h->blink_kind != 6 || h->blink_on == 0 || h->blink == 0)) {
        list = fld_hud_date_draw(list);
        list = fld_hud_move_draw(list);
    }
    list = fld_hud_panel_draw(list);
    list = fld_hud_draw_all(list);
    dl_set_list(list);
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

#ifdef NON_MATCHING /* lógica conferida com o asm. 61 palavras: o original materializa &field_hud, &name.state e &name.item em s1/s0/s3 antes da 1ª chamada (const, inline com parâmetros e volatile não reproduzem) */
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
