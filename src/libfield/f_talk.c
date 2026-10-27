// @range 0x08817218-0x08817AA4
// @asm 0x08817510-0x08817770
// @rodata 0x08838AFC-0x08838B04
// @data 0x0883C878-0x0883C888
/* The talk and choice commands of the field scripts (field_talk): a line said by a slot in the
   message window (closing it when the text is done) and a numbered choice list centered on the
   screen; R held slides the message window out of the way. */
#include "libfield.h"

int cfg_get_language(void);
int fmsg_done(void);
void fmsg_close(void);
void fmsg_lock(int on);
int menu_update(MenuWidget *w);

void ftalk_clear(void);
void ftalk_nop(void);
void *ftalk_draw_msg(void *list);
void *ftalk_draw_choice(void *list);
void *ftalk_item(void *list, unsigned short i, int x, int y);
int ftalk_max_width(unsigned short **text, int n);
void ftalk_slide(void);
void *fld_dl_clear(void *list, int a, unsigned int color);

static const unsigned short ftalk_sep[4] = { '.', ' ', ' ', 0 };

void ftalk_clear(void)
{
    memset(&field_talk, 0, sizeof(field_talk));
}

void ftalk_free(void)
{
    ftalk_nop();
    ftalk_clear();
}

void ftalk_nop(void)
{
}

void ftalk_update(void)
{
    FieldTalk *const t = &field_talk;
    FieldCtx *c = fld_ctx_get();
    FieldTalkMsg *const m = &field_talk.msg;
    FieldTalkChoice *const ch = &field_talk.ch;
    int r;

    switch (field_talk.state) {
    case 1:
        if (m->closing == 0 && fmsg_done()) {
            if (m->close != 0) {
                fmsg_close();
                m->closing = 1;
            } else {
                t->active = 0;
                t->state = 0;
            }
        } else if (m->closing != 0 && fmsg_moving() == 0) {
            t->active = 0;
            t->state = 0;
        } else {
            if (fmsg_waiting() != 0)
                c->speaker = m->speaker;
            else
                c->speaker = -1;
            if (fmsg_moving() == 0)
                ftalk_slide();
        }
        break;
    case 2:
        r = menu_update(&ch->w);
        if (r != -1) {
            menu_reset(&ch->w);
            ch->result = r;
            t->choosing = 0;
            t->state = 0;
        }
        break;
    }
}

void ftalk_draw(void)
{
    void *list = dl_select_layer(3, 0);

    switch (field_talk.state) {
    case 1:
        list = ftalk_draw_msg(list);
        break;
    case 2:
        list = ftalk_draw_choice(list);
        break;
    }
    dl_set_list(list);
}

void ftalk_msg(FieldTalkCmd *a)
{
    FieldTalk *const t = &field_talk;
    FieldTalkMsg *const m = &field_talk.msg;
    MenuItemDef *d = fmsg_def();

    field_talk.active = 1;
    if (fmsg_active() == 0 && a->text == 0) {
        t->active = 0;
    } else {
        memset(m, 0, sizeof(*m));
        m->speaker = a->speaker;
        fmsg_set_input(1);
        d->pad_l = 4;
        d->pad_t = 2;
        if (a->text == 0) {
            m->close = 1;
        } else {
            fmsg_set_text(a->text, a->title);
            m->t = 1.0f;
        }
        t->state = 1;
    }
    d->pad74 = (d->pad74 != 0) ^ 1;
}

int ftalk_msg_done(void)
{
    return (field_talk.active != 0) ^ 1;
}

#ifdef NON_MATCHING /* same instructions; mwcc schedules the item_w store before the widget flags */
void ftalk_choice(FieldList *l)
{
    FieldTalk *const t = &field_talk;
    FieldTalkChoice *const ch = &field_talk.ch;
    MenuWidget *const w = &field_talk.ch.w;
    MenuItemDef *const d = &field_talk.ch.w.def;
    int i;
    int width;
    MenuItemDef *d2;

    field_talk.choosing = 1;
    for (i = 0; i < l->n; i++)
        ch->texts[i] = ((unsigned short **)l->items)[i];
    width = ftalk_max_width(ch->texts, l->n);
    memset(w, 0, sizeof(*w));
    d->visible = 1;
    d->auto_size = 1;
    d->open_anim = 1;
    d->close_anim = 1;
    d->pad_t = 1;
    d->pad_b = 1;
    d->pad_l = 1;
    d->pad_r = 1;
    d->y = 60;
    d->color = 0xFFFFFFFF;
    w->no_cancel = 1;
    w->cancel_blocked = 1;
    w->style = 2;
    w->buttons = 0;
    d->style = 0;
    d->x = 0;
    w->item_w = width + 0x23;
    w->cursor = 0;
    w->count = l->n;
    w->scroll = 0;
    w->button = 0;
    w->selected = 0;
    w->item_h = cfg_get_language() == 0 ? 0x19 : cfg_get_language() == 0 ? 0x11 : 0x13;
    w->cursor_h = cfg_get_language() == 0 ? 0x17 : cfg_get_language() == 0 ? 0xF : 0x11;
    w->draw_item = (MenuDrawItemFn)ftalk_item;
    w->heap = fld_heap_get();
    menu_open(w);
    d->x = 240.0f - 0.5f * d->w;
    d2 = fmsg_def();
    d2->pad74 = (d2->pad74 != 0) ^ 1;
    t->state = 2;
}
#endif

int ftalk_choice_done(int *out)
{
    *out = field_talk.ch.result;
    return (field_talk.choosing != 0) ^ 1;
}

void *ftalk_draw_msg(void *list)
{
    return list;
}

void *ftalk_draw_choice(void *list)
{
    list = fld_dl_clear(list, 0, 0x80000000);
    menu_draw(&list, &field_talk.ch.w);
    return list;
}

void *ftalk_item(void *list, unsigned short i, int x, int y)
{
    unsigned short buf[256];

    buf[0] = i + '1';
    buf[1] = 0;
    wcscat(buf, ftalk_sep);
    wcscat(buf, field_talk.ch.texts[i]);
    font_set_flag8000(cfg_get_language() == 0);
    font_text_set(list);
    font_set_size(15, 15);
    font_set_flag4(1);
    font_set_flag2(1);
    font_set_color(0xFF000000);
    font_draw_text((x + 4) * 64, (y + 1) * 64, (480 - (x + 4)) * 64, buf);
    return font_text_clear();
}

int ftalk_max_width(unsigned short **text, int n)
{
    int i;
    int max = 0;
    int w;

    font_set_size(15, 15);
    font_set_flag4(1);
    for (i = 0; i < n; i++, text++) {
        w = font_measure_text(*text) >> 6;
        if (max < w)
            max = w;
    }
    return max;
}

void ftalk_slide(void)
{
    FieldTalkMsg *const m = &field_talk.msg;
    Vec2 *p = fmsg_pos();
    Vec2 from = { 39.0f, 300.0f };
    Vec2 to = { 39.0f, 210.0f };

    if (pad_get()->b.down & 0x100) {
        m->t = m->t - (0.1f + 0.1f * m->t);
        if (m->t < 0.0f)
            m->t = 0.0f;
        vec2_lerp(p, &from, &to, m->t);
    } else if (!(m->t >= 1.0f)) {
        m->t = m->t + (0.1f + 0.1f * m->t);
        if (m->t > 1.0f)
            m->t = 1.0f;
        vec2_lerp(p, &from, &to, m->t);
    }
    if (m->t >= 1.0f)
        fmsg_lock(0);
    else
        fmsg_lock(1);
}
