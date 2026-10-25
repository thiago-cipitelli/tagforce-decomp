// @range 0x08813088-0x08813AC4
// @rodata 0x0882AEE8-0x0882AF38
/* Extra data cards (state 6): the .YGC files found on the memory stick are listed in the panel; a
   card not owned yet can be taken (three copies). */
#include "libtitle.h"

void extdata_open(void *heap);
int extdata_scan(int kind, int *count);
int extdata_load(void *out, int *count);
void *extdata_draw(void *dl);
int inv_get_bit7_default1(unsigned short id);
void inv_set_bit7(unsigned short id);
void inv_adjust_count(int id, int dir);
void text_ext_reset(void);
void tpad_accel(int on);
void *title_text_frame(void *list, int x, int y, int w, unsigned short *text, int align);

void text_list_init(void);
void *text_legend(void *list);
void *text_tab_label(void *list, int i, int x, int y, int w);
void *text_item(void *list, int i, int x, int y);
void *text_head(void *list, int i, int x, int y, int w);

void text_enter(int state)
{
    title.state = state;
    title.sub = 0;
    title.step = 0;
    title.frame = 0;
    title.color0 = 0xFFFFFFFF;
    title.color1 = 0xFFFFFFFF;
    title_ehf_load(5, 2, 0, 480);
    title_ehf_load(6, 2, 0, 480);
    title_ehf_load(13, 2, 0, 16);
    title.u3044 = 0;
    title.u3048 = 0;
    fade_start(1, 0xFF000000, 16);
    fade_finish();
}

void text_update(void)
{
    Title *const t = &title;
    TPanel *p = &title.panel;
    TBox *b = &p->box;
    TExtList *e = &title.duel.ext;
    TExtCard card;
    unsigned int trig;
    int r;

    switch (title.sub) {
    case 0:
        text_ext_reset();
        t->sub = 1;
    case 1:
        extdata_open(t->heap);
        t->sub = 2;
        break;
    case 2:
        if (extdata_scan(0, &e->n) == 0)
            break;
        func_088225A0(1);
        if (e->n != 0) {
            e->card = heap_alloc(t->heap, e->n * sizeof(TExtCard));
            e->b0 = 1;
            e->stage = 1;
            t->sub = 3;
        } else {
            t->sub = 4;
        }
        break;
    case 3:
        if (extdata_load(e->card, &e->loaded)) {
            t->sub = 4;
            e->stage = 2;
        }
        break;
    case 4:
        text_list_init();
        title_goto_fade(0, 0xFFF, 6);
        t->sub = 5;
        break;
    case 6:
        if (title_ehf_frame(13) == 16) {
            title_ehf_anim(13, 2, 17, 96);
            t->sub = 7;
        }
        break;
    case 7:
        tpad_accel(1);
        r = tpanel_update(p);
        switch (r) {
        case -2:
            t->sub = 8;
            t->frame = 0;
            tpad_accel(0);
            break;
        case -1:
        default:
            trig = pad_get()->b.trig;
            if (trig & region_pick_value(0)) {
                if (b->pos < e->n) {
                    memcpy(&card, &e->card[b->pos], sizeof(card));
                    if (card.invalid) {
                        se_play(0x6215);
                    } else if (inv_get_bit7_default1(card.id)) {
                        se_play(0x6215);
                    } else {
                        tframe_cursor_mode(2);
                        inv_set_bit7(card.id);
                        inv_adjust_count(card.id, 1);
                        inv_adjust_count(card.id, 1);
                        inv_adjust_count(card.id, 1);
                        se_play(0x6213);
                    }
                } else {
                    se_play(0x6215);
                }
            }
            break;
        }
        if (t->sub == 8) {
            title_goto_fade(1, 0xFFF, 9);
            t->arg = 6;
        }
        break;
    case 9:
        func_088225A0(1);
        t->arg = 6;
        title_set_state(9, 6);
        break;
    }
    t->prev_sub = t->sub;
}

void *text_draw(void *list)
{
    Title *const t = &title;
    TPanel *p = &title.panel;
    TBox *b = &p->box;

    if (title.sub < 7)
        b->no_cursor = 1;
    else
        b->no_cursor = 0;
    switch (t->sub) {
    case 0:
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        list = extdata_draw(list);
        break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        list = title_ehf_draw(list, 6);
        list = title_ehf_draw(list, 5);
        if (t->sub >= 6) {
            list = title_ehf_draw(list, 13);
            list = title_screen_label(list);
        }
        list = tpanel_draw(list, p);
        list = text_legend(list);
        t->frame++;
        break;
    }
    return list;
}

/* the three marks at the bottom: new, owned, invalid */
void *text_legend(void *list)
{
    list = title_sprite(list, 1, 60, 3, 255);
    list = title_sprite(list, 1, 61, 116, 256);
    list = title_sprite(list, 1, 62, 330, 255);
    font_set_size(12, 12);
    font_set_color(0xFFFFFFFF);
    font_set_flag4(1);
    list = title_msg_draw(list, 22, 258, 91, 0, 4, 2);
    list = title_msg_draw(list, 135, 258, 192, 0, 5, 2);
    return title_msg_draw(list, 348, 258, 128, 0, 6, 2);
}

void text_list_init(void)
{
    TPanel *p = &title.panel;
    TBox *b = &p->box;
    MenuWidget *w = &p->mw;

    tpanel_init(p, title.heap);
    p->f0 = 2;
    p->draw2 = (TBoxDrawFn)text_head;
    p->n = 1;
    p->w = 480;
    b->w = p->w;
    b->iw = b->w - 21;
    p->h = 220;
    b->h = p->h - 16;
    p->y = 33;
    b->y = p->y + 16;
    b->iy = p->y + 22;
    p->x = 0;
    b->ix = p->x + 5;
    b->ih = 23;
    b->x = p->x;
    tpanel_list_init(p, title.heap);
    w->count = title.duel.ext.n;
    w->rows = 8;
    if (w->count < w->rows)
        w->count = 8;
    b->count = w->count;
    b->n = w->rows;
    w->draw_cursor = (MenuDrawCursorFn)tpanel_cursor;
    w->accept_blocked = 1;
    p->tab[0].draw = (TPanelDrawFn)text_tab_label;
    p->tab[1].draw = (TPanelDrawFn)text_tab_label;
    p->tab[2].draw = (TPanelDrawFn)text_tab_label;
    b->text = 0;
    b->draw = 0;
    w->draw_cursor_post = 0;
    w->draw_item = (MenuDrawItemFn)text_item;
}

void *text_tab_label(void *list, int i, int x, int y, int w)
{
    TPanel *p = &title.panel;
    TTab *t = &p->tab[(unsigned short)i];
    int tx = x + 4;
    int ty = y + 4;

    font_set_size(15, 15);
    if (p->cur == (unsigned short)i)
        font_set_color(t->text_color);
    else
        font_set_color(t->text_color_off);
    font_set_flag4(1);
    return title_msg_draw(list, tx, ty, w, 0, (unsigned short)i + 33, 0);
}

/* a card of the list: the mark, the file name and the card name */
void *text_item(void *list, int i, int x, int y)
{
    TExtList *e = &title.duel.ext;
    TExtCard card;

    if ((unsigned short)i < e->n) {
        int sx, sy;

        memcpy(&card, &e->card[(unsigned short)i], sizeof(card));
        sx = x + 1;
        sy = y - 1;
        if (card.invalid)
            list = title_sprite(list, 1, 62, sx, sy);
        else if (inv_get_bit7_default1(card.id))
            list = title_sprite(list, 1, 60, sx, sy);
        else
            list = title_sprite(list, 1, 61, sx, sy);
        font_set_size(13, 13);
        font_set_color(0xFF000000);
        font_set_flag4(1);
        list = title_text_frame(list, x + 23, y, 82, card.name, 2);
        if (!card.invalid) {
            font_set_size(13, 13);
            font_set_color(0xFF000000);
            font_set_flag4(1);
            list = title_text_frame(list, x + 116, y, 332, card_get_name(card.id), 2);
        }
    }
    return list;
}

void *text_head(void *list, int i0, int x, int y, int w)
{
    unsigned short i = i0;

    font_set_size(15, 15);
    font_set_color(0xFFFFFFFF);
    font_set_flag4(1);
    return title_msg_draw(list, x + 4, y + 4, w - 4, 0, i + 2, 2);
}
