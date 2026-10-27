// @range 0x08821D3C-0x08824DA8
// @asm 0x08822D64-0x08822EB4
// @data 0x08830140-0x08830214
// @rodata 0x0882B4E4-0x0882B6D0
/* Duel against the computer (state 12, main menu item 1): the rules (tag duel, deck swap, life
   points, time), the partner, the opponents, a character from libcharalist and the decks, all over
   the partner / versus panel. title.sub is the screen (0 load libcharalist, 1 first entry, 2 rules,
   3 partner, 4 opponents, 5 versus, 6 character, 7 deck), title.step the step inside it. */
#include "libtitle.h"

int tpartner_update_net(void);
void *tpartner_draw(void *list);
void tpt_anim_start(int on, int len);
void tpt_set_mode(int mode);
void tpt_set_name(short i, int kind, int type2, int src, int deck, void *name);
void tpt_set_all(short i, int kind, int type2, int src, int deck, void *name, void *deckname);
unsigned short *chara_get_name(int set, int id);
unsigned short *sysstate_get_name(void);
void extdata_open(void *heap);
int extdata_scan(int kind, int *count);
int extdata_load(void *out, int *count);
void *extdata_draw(void *dl);
void *tframe_cursor(void *list, unsigned int color, int x, int y, int w, int h);
void tfd_memset_big(void *p);

typedef struct TDlEnt {                /* a partner panel entry (TPtEnt) as 4 words: copied as a block */
    int w[4];
} TDlEnt;
#define ENT(x) ((TPtEnt *)&(x))

void text_ext_reset(void);
void tdl_ext_free(int keep);
void tdl_seats_init(void);
void tdl_sub_enter(short sub);
void tdl_ext_load(short sub);
void tdl_sub_anim(short sub);
void tdl_rules_enter(void);
void tdl_rules_update(void);
void tdl_rules_anim(void);
void *tdl_rules_draw(void *list);
void *tdl_rules_help(void *list);
void *tdl_rules_item(void *list, int i, int x, int y);
void *tdl_rules_value(void *list, unsigned short i, int x, int y, int w);
int tdl_rules_help_id(void);
void *tdl_cursor(void *list, MenuWidget *w, int x, int y, int cw, int ch);
void tdl_partner_enter(void);
void tdl_partner_swap(void);
void tdl_partner_anim(short sub);
void tdl_partner_update(void);
int tdl_partner_input(int cpu);
void *tdl_partner_draw(void *list);
void *tdl_random_label(void *list);
void *tdl_corner_label(void *list, int i, int x, int y, int w);
void tdl_vs_enter(void);
void tdl_vs_update(void);
int tdl_vs_side(void);
int tdl_vs_input(void);
void *tdl_vs_draw(void *list);
void tdl_nop(void);
void *tdl_pt_draw(void *list);
void tdl_chara_load(void);
void tdl_chara_update(void);
void tdl_random_seat(int seat);
int tdl_seat_of(int sub, int i);
void *tdl_header_label(void *list, int i, int x, int y, int w);
void tdl_decks_clear(void);
void tdl_deck_enter(void);
int tdl_deck_default(int id);
void tdl_deck_update(void);
void *tdl_deck_draw(void *list);

void tdl_enter(int state, int sub)
{
    TPanel *p = &title.panel;
    MenuWidget *w = &p->mw;
    TBox *b = &p->box;
    MenuItemDef *def = &w->def;

    title.state = state;
    title.sub = sub;
    title.step = 0;
    title.step2 = 0;
    title.frame = 0;
    title.u86c = 0;
    title.u86d = 2;
    title.color0 = 0xFFFFFFFF;
    title.color1 = 0xFFFFFFFF;
    tpanel_init(p, title.heap);
    p->f0 = 0;
    p->n = 1;
    b->x = 4;
    b->y = 52;
    b->w = 305;
    b->h = 42;
    def->x = b->x;
    def->y = b->y;
    def->w = b->w;
    def->h = b->h;
    def->pad_l = 0;
    def->pad_r = 0;
    def->pad_t = 0;
    def->pad_b = 0;
    def->color = 0xFFFFFF;
    w->count = 20;
    w->rows = 5;
    w->draw_cursor = tdl_cursor;
    w->draw_cursor_post = 0;
    w->item_w = b->w - (def->pad_l + def->pad_r);
    w->item_h = b->h + 1;
    w->item_x = 0;
    w->item_y = 0;
    w->cursor_w = w->item_w;
    w->cursor_h = b->y - 8 - w->item_y * 2;
    b->count = w->count;
    b->n = w->rows;
    b->text = 0;
    b->draw = 0;
    tpartner_init(title.heap, title_pack(), 0);
    memset(&title.duel.uf00, 0, 0x4C);
    memset(&title.duel.tail, 0, 0x1510);
    title_ehf_load(10, 2, 0, 0x1DF);
    text_ext_reset();
}

void tdl_seats_init(void)
{
    int i;
    unsigned short *name;

    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            name = sysstate_get_name();
            tpt_set_name(i, 1, 0, 0, tdl_deck_default(1), name);
            break;
        default:
            tdl_random_seat(i);
            break;
        }
    }
    tpt_set_mode(4);
}

void tdl_update(void)
{
    Title *t = &title;
    MenuWidget *w = &title.win[0];

    tdl_sub_anim(t->sub);
    switch (t->step) {
    case 16:
        tframe_cursor_mode(0);
        tdl_sub_enter(t->sub);
        title_fade_out(0);
        t->next_step = 9;
        t->step = 6;
        title_ehf_anim(15, 2, 0x12, 0x35);
        if (t->sub == 3)
            title_ehf_anim(12, 2, 0x1E, 0x3B);
        else
            title_ehf_anim(12, 2, 0x82, 0x9F);
        break;
    case 0:
        if (t->sub >= 2)
            tframe_cursor_mode(0);
        tdl_sub_enter(t->sub);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        tdl_ext_load(t->sub);
        break;
    case 5:
        title_fade_out(0);
        t->next_step = 7;
        t->step = 6;
        break;
    case 9:
        switch (t->sub) {
        case 2:
            tdl_rules_update();
            break;
        case 3:
            tdl_partner_update();
            break;
        case 4:
            tdl_vs_update();
            break;
        case 5:
            tdl_nop();
            break;
        case 6:
            tdl_chara_update();
            break;
        case 7:
            tdl_deck_update();
            break;
        }
        break;
    case 10:
        title_fade_out(1);
        t->step++;
        break;
    case 12:
        w->draw_cursor = (MenuDrawCursorFn)tpanel_cursor;
        title_goto_fade(1, 0xFFF, 0xFFF);
        t->next_step = 0xFFF;
        t->step = 13;
        break;
    case 13:
        if (title_fade_state() == 0) {
            charalist_res_free();
            title_module_stop();
            tdl_ext_free(0);
            tpartner_free();
            title_set_state(9, 4);
        }
        break;
    case 14:
        tdl_ext_free(0);
        title_goto_fade(1, 0x22, 0);
        t->next_step = 0;
        t->step = 15;
        break;
    case 11:
    case 15:
    case 6:
    case 7:
    case 8:
        break;
    }
    t->prev_sub = t->sub;
}

void tdl_sub_enter(short sub)
{
    Title *t = &title;
    TFdTail *q = &title.duel.tail;

    t->sub_shown = 0;
    switch (sub) {
    case 0:
        tdl_chara_load();
        break;
    case 1:
        tdl_seats_init();
        t->sub = 2;
    case 2:
        tdl_rules_enter();
        t->step = 5;
        break;
    case 3:
        tdl_partner_enter();
        t->step = 5;
        break;
    case 4:
        tdl_vs_enter();
        t->step = 5;
        break;
    case 5:
        t->step = 5;
        break;
    case 6:
        tdl_seat_of(q->u001, q->u002);
        charalist_init(2, t->heap);
        charalist_update();
        t->sub_shown = 1;
        t->step = 5;
        break;
    case 7:
        t->step = 1;
        break;
    }
}

void tdl_ext_load(short sub)
{
    TFdTail *q = &title.duel.tail;
    Title *t = &title;
    TExtList *e = &q->ext;
    TPtEnt s;
    int load = 0;
    int size;

    fade_start(1, 0xFF000000, 16);
    fade_finish();
    tdl_seat_of(q->u001, q->u002);
    tpt_get_ent(&s, q->u003);
    if (sub == 7) {
        switch (s.kind) {
        case 1:
        case 0x29:
            load = 1;
        }
    }
    if (load) {
        switch (t->step) {
        case 1:
            tdl_ext_free(0);
            extdata_open(t->heap);
            t->step = 2;
            break;
        case 2:
            if (extdata_scan(1, &e->n))
                t->step = 3;
            break;
        case 3:
            if (e->n != 0) {
                size = e->n * sizeof(TExtRec);
                tdl_ext_free(0);
                e->rec = heap_alloc(t->heap, size);
                memset(e->rec, 0, size);
                e->b0 = 1;
                e->stage = 1;
                t->step = 4;
            } else {
                t->step = 5;
            }
            break;
        case 4:
            if (extdata_load(e->rec, &e->loaded)) {
                if (e->loaded >= 0x100)
                    e->loaded = 0x100;
                t->step = 5;
                e->stage = 2;
            }
            break;
        }
    } else {
        t->step = 5;
    }
    if (t->step == 5) {
        fade_start_in(1, 0xFF000000, 16);
        fade_finish();
        tdl_deck_enter();
    }
}

void text_ext_reset(void)
{
    title.duel.tail.ext.stage = 0;
}

void tdl_ext_free(int keep)
{
    TExtList *e = &title.duel.tail.ext;
    Title *t = &title;

    if (e->stage) {
        if (keep == 0) {
            heap_set_level(t->heap, e->rec);
            memset(e->file, 0, sizeof(e->file));
        } else {
            heap_set_level(t->heap, e->card);
        }
        e->n = 0;
        e->b0 = 0;
        e->stage = 0;
        e->loaded = 0;
    }
}

void tdl_sub_anim(short sub)
{
    Title *t = &title;

    switch (sub) {
    case 2:
        tdl_rules_anim();
        break;
    case 3:
    case 4:
        tdl_partner_anim(sub);
        break;
    case 6:
    case 7:
        if (t->step == 7)
            t->step = 9;
        break;
    case 0:
    case 1:
    case 5:
        break;
    }
}

void tdl_rules_enter(void)
{
    MenuWidget *w = &title.win[0];

    title.step = 0;
    title.step2 = 0;
    title.frame = 0;
    title.u86c = 0;
    title.u86d = 2;
    title.color0 = 0xFFFFFFFF;
    title.color1 = 0xFFFFFFFF;
    twin_init(w);
    w->def.x = 4;
    w->def.y = 0x35;
    w->def.h = 0x85;
    w->def.pad_l = 1;
    w->def.w = 0x140;
    w->def.pad_r = 2;
    w->def.pad_t = 0x12;
    w->def.color = 0xFFFFFFFF;
    w->rows = 10;
    w->no_input = 0;
    w->def.open_anim = 0;
    w->item_w = w->def.w - (w->def.pad_l + w->def.pad_r);
    w->def.close_anim = 0;
    w->no_cancel = 1;
    w->keep_open = 1;
    w->draw_cursor = (MenuDrawCursorFn)tpanel_cursor;
    w->item_h = 16;
    w->cursor_w = w->item_w - 8;
    w->cursor_h = 15;
    w->draw_cursor_post = 0;
    w->count = 2;
    w->draw_item = (MenuDrawItemFn)tdl_rules_item;
    w->def.pad_b = 0x12;
    w->item_x = 0;
    w->item_y = 0;
    twin_open(w);
    tfd_memset_big(&title.duel);
    title_ehf_load(15, 2, 0, 0x11);
}

void tdl_rules_update(void)
{
    Title *t = &title;
    TFdTail *q = &title.duel.tail;
    MenuWidget *w = &title.win[0];
    TDuelOpts *o = &title.duel.opts;
    short k;
    int r;

    if (title.step != 9)
        return;
    w->draw_cursor = 0;
    switch (r = menu_update(w)) {
    case -1:
        break;
    case -2:
        if ((k = t->step2) != 0) {
            if (k == 5 && o->b7 == 0)
                t->step2 = 2;
            else
                t->step2--;
            w->cancelled = w->accepted = 0;
            w->cursor = 0;
            w->scroll = 0;
            switch (t->step2) {
            case 0:
                w->cursor = o->tag;
                break;
            case 1:
                w->cursor = o->swap;
                break;
            case 2:
                w->cursor = o->b7;
                break;
            case 3:
                w->cursor = o->lp;
                break;
            case 4:
                w->cursor = o->time;
                break;
            case 5:
                break;
            }
        } else {
            t->step = 12;
        }
        break;
    case -3:
        break;
    default:
        switch (t->step2) {
        case 0:
            t->step2++;
            o->tag = r;
            break;
        case 1:
            t->step2++;
            o->swap = r;
            break;
        case 2:
            o->b7 = r;
            switch (r) {
            case 0:
                o->lp = 2;
                o->time = 0;
                t->step2 = 5;
                break;
            case 1:
                t->step2++;
                break;
            }
            break;
        case 3:
            t->step2++;
            o->lp = r;
            break;
        case 4:
            o->time = r;
            t->step2 = 5;
            break;
        case 5:
            t->step = 10;
            tframe_cursor_mode(1);
            if (o->tag)
                t->next_sub = 4;
            else
                t->next_sub = 3;
            q->u002 = 0;
            t->next_step = 0;
            t->next_step2 = 0;
            break;
        }
        w->cancelled = w->accepted = 0;
        w->scroll = 0;
        w->cursor = 0;
        break;
    }
    switch (t->step2) {
    case 0:
        w->count = 2;
        break;
    case 1:
        w->count = 2;
        break;
    case 2:
        w->count = 2;
        break;
    case 3:
        w->count = 5;
        break;
    case 4:
        w->count = 5;
        break;
    case 5:
        w->count = 0;
        break;
    }
}

void tdl_rules_anim(void)
{
    Title *t = &title;

    switch (t->step) {
    case 7:
        title_ehf_load(15, 2, 0, 0x11);
        t->step = 8;
        break;
    case 8:
        if (title_ehf_frame(15) == 0x11) {
            title_ehf_anim(15, 2, 0x12, 0x35);
            t->step = 9;
        }
        break;
    case 11:
        if (title_ehf_frame(15) == 0x3A)
            title_ehf_anim(15, 2, 0, 0);
        break;
    }
}

#ifdef NON_MATCHING /* the n-- after the side tab count is a beqzl with the decrement in the delay slot */
void *tdl_rules_draw(void *list)
{
    Title *t = &title;
    MenuWidget *w = &title.win[0];
    int n = 0;
    short k;

    if (t->step >= 8) {
        list = title_ehf_draw(list, 15);
        list = title_screen_label(list);
    }
    list = tbox_header(list, (TBoxDrawFn)tdl_header_label);
    list = tbox_corner(list, (TBoxDrawFn)tdl_corner_label);
    list = tbox_help(list, tdl_rules_help_id(), 12);
    menu_draw(&list, w);
    if (t->step2 != 5)
        list = tdl_rules_help(list);
    if ((k = t->step2) != 0) {
        n = k;
        if (n > 2)
            n--;
    }
    list = tbox_side(list, 0x14, n, (TBoxDrawFn)tdl_rules_value);
    if (t->step2 == 5)
        list = tframe_cursor(list, 0xFFFFFFFF, 0x147, 0xF2, 0x97, 0x17);
    return list;
}
#endif

void *tdl_rules_help(void *list)
{
    Title *t = &title;
    int id = 0;

    switch (t->step2) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        id = t->step2 + 0x4D;
        break;
    case 5:
        break;
    }
    return tbox_small(list, id, 9, 0x3A, 0x135, 16);
}

void *tdl_rules_item(void *list, int i, int x, int y)
{
    int id = 0;

    switch (title.step2) {
    case 0:
        id = 88;
        break;
    case 1:
        id = 90;
        break;
    case 2:
        id = 92;
        break;
    case 3:
        id = 104;
        break;
    case 4:
        id = 109;
        break;
    }
    if (id != 0) {
        id += (unsigned short)i;
        list = title_msg_font(list, x, y, 0x100, 12, id, 0, 15, 15, 0xFF002020);
    }
    return list;
}

void *tdl_rules_value(void *list, unsigned short i, int x, int y, int w)
{
    TDuelOpts *o = &title.duel.opts;
    int id = 0;

    if (i >= 2)
        i++;
    switch (i) {
    case 0:
        id = 0x58 + o->tag;
        break;
    case 1:
        id = 0x5A + o->swap;
        break;
    case 2:
    case 3:
        id = o->lp + 0x34;
        break;
    case 4:
        id = o->time + 0x39;
        break;
    case 5:
        break;
    }
    if (id != 0)
        list = title_msg_font(list, x, y, w, 12, id, 2, 13, 13, 0xFF002020);
    return list;
}

void *tdl_draw(void *list)
{
    Title *t = &title;

    switch (t->step) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        break;
    default:
        list = title_ehf_draw(list, 10);
        break;
    }
    switch (t->sub) {
    case 0:
    case 1:
    case 6:
        break;
    case 2:
        list = tdl_rules_draw(list);
        break;
    case 3:
        list = tdl_partner_draw(list);
        break;
    case 4:
        list = tdl_vs_draw(list);
        break;
    case 5:
        list = tdl_pt_draw(list);
        break;
    case 7:
        if (t->step > 0 && t->step < 5)
            list = extdata_draw(list);
        else
            list = tdl_deck_draw(list);
        break;
    }
    t->frame++;
    return list;
}

int tdl_rules_help_id(void)
{
    Title *t = &title;
    MenuWidget *w = &title.win[0];
    int id = 0;

    if (t->step == 9) {
        switch (t->step2) {
        case 0:
            id = w->cursor + 0x43;
            break;
        case 1:
            id = w->cursor + 0x45;
            break;
        case 2:
            id = w->cursor + 0x47;
            break;
        case 3:
            id = 0x49;
            break;
        case 4:
            id = 0x4A;
            break;
        case 5:
            break;
        }
    }
    return id;
}

void *tdl_cursor(void *list, MenuWidget *w, int x, int y, int cw, int ch)
{
    if (title.u86c != 0)
        list = tframe_cursor(list, 0xFFFFFFFF, 0x147, 0xF4, 0x97, 0x17);
    else
        list = tframe_cursor(list, 0xFFFFFFFF, x - 4, y - 4, cw, ch - 1);
    return list;
}

void tdl_partner_enter(void)
{
    title.step2 = 0;
    title.frame = 0;
    title.u86c = title.duel.tail.u002;
    title.u86d = 4;
    tdl_partner_swap();
    tpt_set_mode(1);
    if (title.step != 16)
        tpt_anim_start(1, 10);
    title_ehf_load(12, 2, 0, 0);
    title_ehf_load(15, 2, 0, 0);
}

void tdl_partner_swap(void)
{
    TFdTail *q = &title.duel.tail;
    TDlEnt ent[4];
    TDeckRec d[4];
    TDlEnt tmp;

    tpt_get_ent(&ent[0], 0);
    if (ENT(ent[0])->kind == 1)
        return;
    d[0] = q->decks[0];
    d[2] = q->decks[2];
    tpt_get_ent(&ent[0], 0);
    tpt_get_ent(&tmp, 2);
    ent[2] = tmp;
    q->decks[0] = d[2];
    q->decks[2] = d[0];
    tpt_set_all(0, ENT(ent[2])->kind, ENT(ent[2])->type2, ENT(ent[2])->src, ENT(ent[2])->deck, ENT(ent[2])->name,
                q->decks[0].name);
    tpt_set_all(2, ENT(ent[0])->kind, ENT(ent[0])->type2, ENT(ent[0])->src, ENT(ent[0])->deck, ENT(ent[0])->name,
                q->decks[2].name);
}

void tdl_partner_anim(short sub)
{
    Title *t = &title;
    int f;

    if (t->step == 7) {
        if (sub == 3)
            title_ehf_load(12, 2, 0, 0x1D);
        else
            title_ehf_load(12, 2, 100, 0x81);
        title_ehf_load(15, 2, 0, 0x11);
        t->step = 8;
    } else if (t->step == 8) {
        if (title_ehf_frame(15) == 0x11)
            title_ehf_anim(15, 2, 0x12, 0x35);
        if (sub == 3) {
            f = title_ehf_frame(12);
            switch (f) {
            case 0x1D:
                title_ehf_anim(12, 2, 0x1E, 0x3B);
                break;
            }
            if (f >= 0x1D && tpartner_update_net() == 1)
                t->step = 9;
        } else {
            f = title_ehf_frame(12);
            switch (f) {
            case 0x81:
                title_ehf_anim(12, 2, 0x82, 0x9F);
                break;
            }
            if (f >= 0x81 && tpartner_update_net() == 1)
                t->step = 9;
        }
    } else if (t->step == 11) {
        if (sub == 3) {
            if (title_ehf_frame(12) == 0x5D)
                title_ehf_anim(12, 2, 0x5D, 0x5D);
        } else {
            if (title_ehf_frame(12) == 0xC1)
                title_ehf_anim(12, 2, 0xC1, 0xC1);
        }
        if (title_ehf_frame(15) == 0x3A)
            title_ehf_anim(15, 2, 0, 0);
    }
}

void tdl_partner_update(void)
{
    Title *t = &title;
    TFdTail *q = &title.duel.tail;

    switch (tdl_partner_input(1)) {
    case -2:
        t->step = 10;
        t->next_step = 0;
        t->next_step2 = 0;
        t->next_sub = 2;
        break;
    case 0:
        q->u002 = t->u86c;
        t->next_step = 0;
        q->u001 = 3;
        t->step = 10;
        t->next_sub = 6;
        t->next_step2 = 0;
        break;
    case 1:
    case 2:
        q->u002 = t->u86c;
        t->next_step = 0;
        q->u001 = 3;
        t->step = 10;
        t->next_sub = 7;
        t->next_step2 = 0;
        break;
    case 3:
        bgm_cmd(600);
        t->step = 14;
        t->next_sub = 0;
        t->next_step = 0;
        t->next_step2 = 0;
        title_ehf_anim(12, 2, 0x4B, 0x5D);
        break;
    case -1:
        break;
    }
}

int tdl_partner_input(int cpu)
{
    Title *t = &title;
    TFdTail *q = &title.duel.tail;
    int last = 0;
    PadState *pad = pad_get();
    short w;
    unsigned int trig = pad->b.trig;
    unsigned int rep = pad->b.rep;

    if (title.u86d > 0)
        last = t->u86d - 1;
    if ((w = t->u3450) == 0) {
        if ((trig & 0x40) || (rep & 0x40)) {
            if (t->u86c < last) {
                t->u86c++;
                se_play(0x6212);
            } else if (trig & 0x40) {
                t->u86c = 0;
                se_play(0x6212);
            }
        } else if ((trig & 0x10) || (rep & 0x10)) {
            if (t->u86c > 0) {
                t->u86c--;
                se_play(0x6212);
            } else if (trig & 0x10) {
                t->u86c = last;
                se_play(0x6212);
            }
        } else if (trig & 0x20) {
            if (t->u86c < last) {
                t->u86c = last;
                se_play(0x6212);
            }
        } else if (trig & 0x80) {
            if (t->u86c == last) {
                t->u86c--;
                se_play(0x6212);
            }
        }
        if (cpu != 0 && t->u86c == 0 && (trig & 0x8000)) {
            tframe_cursor_mode(1);
            se_play(0x6213);
            tdl_seat_of(3, q->u002 = t->u86c);
            tdl_random_seat(q->u003);
            t->u3450 = 1;
            return -1;
        }
        if (trig & region_pick_value(0)) {
            tframe_cursor_mode(1);
            se_play(0x6213);
            return t->u86c;
        }
        if (trig & region_pick_value(1)) {
            if (cpu != 0)
                se_play(0x6214);
            return -2;
        }
        return -1;
    }
    if (w < 60) {
        tdl_random_seat(q->u003);
        se_play(0x6212);
        t->u3450++;
    } else {
        tframe_cursor_mode(0);
        se_play(0x6213);
        t->u3450 = 0;
    }
    return -1;
}

void *tdl_partner_draw(void *list)
{
    Title *t = &title;
    int sel = title.u86c;

    list = title_ehf_draw(list, 12);
    list = title_ehf_draw(list, 15);
    list = title_screen_label(list);
    if (title_ehf_frame(12) == 0x17)
        se_play(0x628C);
    else if (title_ehf_frame(12) == 0x4B)
        se_play(0x628D);
    if (t->step >= 9) {
        list = tbox_header(list, (TBoxDrawFn)tdl_header_label);
        list = tbox_corner(list, (TBoxDrawFn)tdl_corner_label);
        if (t->u86c == 0)
            list = tdl_random_label(list);
    }
    list = tpartner_draw(list);
    if (t->step >= 9) {
        int r[4][4] = { { 0xEF, 0x55, 0x84, 0x17 }, { 0xDF, 0x6D, 0x94, 0x17 },
                        { 0x6E, 0xCE, 0x94, 0x17 }, { 0x147, 0xF2, 0x97, 0x17 } };

        list = tframe_cursor(list, 0xFFFFFFFF, r[sel][0], r[sel][1], r[sel][2], r[sel][3]);
    }
    return list;
}

void *tdl_random_label(void *list)
{
    return title_msg_font(list, 0x25, 0xF6, 0x100, 0, 0x4C, 2, 15, 15, 0xFFFFFFFF);
}

void *tdl_corner_label(void *list, int i, int x, int y, int w)
{
    Title *t = &title;
    int id = 0;
    unsigned int color = 0xFFFFFFFF;

    switch (t->sub) {
    case 2:
        id = 10;
        if (t->step2 < 5)
            color = 0xFF808080;
        break;
    case 3:
    case 4:
    case 5:
        id = 11;
        break;
    case 0:
    case 1:
    case 6:
    case 7:
        break;
    }
    if (id != 0)
        list = title_msg_font(list, x, y, w, 0, id, 3, 13, 13, color);
    return list;
}

void tdl_vs_enter(void)
{
    Title *t = &title;
    TFdTail *q = &title.duel.tail;
    TDlEnt ent[4];
    TDlEnt tmp;

    if (title.step2 == 0) {
        t->u86c = 0;
        t->u86d = 0;
        t->arg = 0;
        tpt_get_ent(&ent[0], 0);
        tpt_get_ent(&tmp, 2);
        ent[2] = tmp;
        t->arg = 0;
        if (ENT(ent[2])->kind == 1)
            t->arg = 1;
        t->u874 = 2;
        tpt_set_mode(3);
        if (t->step != 16)
            tpt_anim_start(1, 10);
        title_ehf_load(12, 2, 0, 0);
        title_ehf_load(11, 2, 0, 0);
        title_ehf_load(15, 2, 0, 0);
    } else {
        t->arg = q->u002;
        t->u86c = 0;
        t->u86d = 2;
        t->u874 = 8;
        t->frame = 0;
        tpt_set_mode(4);
        if (t->step != 16)
            tpt_anim_start(1, 10);
        title_ehf_load(12, 2, 0, 0);
        title_ehf_load(11, 2, 0, 0);
        title_ehf_load(15, 2, 0, 0);
    }
}

void tdl_vs_update(void)
{
    Title *t = &title;
    TFdTail *q = &title.duel.tail;
    int r;
    TDlEnt ent[4];
    TDlEnt tmp;
    TDlEnt tmp2;

    if (title.step2 == 0) {
        r = tdl_vs_side();
        switch (r) {
        case -1:
            break;
        case -2:
            t->step = 10;
            t->next_step = 0;
            t->next_step2 = 0;
            t->next_sub = 2;
            break;
        case 0:
        case 1:
            tframe_cursor_mode(2);
            tpt_set_mode(4);
            if (r == 0)
                q->u000 = 2;
            else
                q->u000 = 6;
            t->u86c = 0;
            t->arg = q->u000;
            t->u874 = 8;
            t->u86d = 2;
            t->step2 = 1;
            break;
        }
    } else if (t->step2 == 1) {
        if (title_ehf_frame(11) == 14)
            title_ehf_anim(11, 0, 14, 0);
        switch (tdl_vs_input()) {
        case -1:
            break;
        case -2:
            tpt_set_mode(3);
            t->u86c = 0;
            t->u86d = 0;
            tpt_get_ent(&ent[0], 0);
            tpt_get_ent(&tmp, 2);
            ent[2] = tmp;
            t->arg = 0;
            if (ENT(ent[2])->kind == 1)
                t->arg = 1;
            t->step2 = 0;
            t->u874 = 2;
            break;
        case 0:
            q->u001 = 4;
            q->u002 = t->arg;
            if (t->arg == q->u000) {
                tframe_cursor_mode(2);
                tpt_set_mode(3);
                t->u86c = 0;
                t->u86d = 0;
                tpt_get_ent(&ent[0], 0);
                tpt_get_ent(&tmp2, 2);
                ent[2] = tmp2;
                t->arg = 0;
                if (ENT(ent[2])->kind == 1)
                    t->arg = 1;
                t->step2 = 0;
                t->u874 = 2;
            } else {
                tframe_cursor_mode(1);
                switch (t->arg) {
                case 0:
                case 2:
                case 4:
                case 6:
                    t->step = 10;
                    t->next_step = 0;
                    t->next_step2 = 0;
                    t->next_sub = 6;
                    break;
                case 1:
                case 3:
                case 5:
                case 7:
                    t->step = 10;
                    t->next_step = 0;
                    t->next_step2 = 0;
                    t->next_sub = 7;
                    break;
                }
            }
            break;
        case 1:
            tframe_cursor_mode(1);
            bgm_cmd(600);
            t->step = 14;
            t->next_sub = 0;
            t->next_step = 0;
            t->next_step2 = 0;
            title_ehf_anim(12, 2, 0xAF, 0xC1);
            title_ehf_anim(11, 2, 0, 14);
            break;
        }
    }
}

int tdl_vs_side(void)
{
    Title *t = &title;
    TDeckRec d[4];
    int i;
    TFdTail *q = &title.duel.tail;
    int last = 0;
    int moved = 0;
    TDlEnt ent[4];
    TDlEnt tmp;
    unsigned int trig = pad_get()->b.trig;

    if (trig != 0)
        tframe_cursor_mode(0);
    if (t->u874 > 0)
        last = t->u874 - 1;
    if (trig & 0x20) {
        se_play(0x6212);
        moved = 1;
        if (t->arg < last)
            t->arg++;
        else
            t->arg = 0;
    } else if (trig & 0x80) {
        se_play(0x6212);
        moved = 1;
        if (t->arg > 0)
            t->arg--;
        else
            t->arg = last;
    }
    if (moved) {
        i = 0;
        if (t->arg != 0)
            i = 2;
        tpt_get_ent(&ent[0], 0);
        tpt_get_ent(&tmp, 2);
        ent[2] = tmp;
        d[0] = q->decks[0];
        d[2] = q->decks[2];
        if (ENT(ent[i])->kind != 1) {
            q->decks[0] = d[2];
            q->decks[2] = d[0];
            if (ENT(ent[2])->src == 2)
                tpt_set_all(0, ENT(ent[2])->kind, ENT(ent[2])->type2, ENT(ent[2])->src, ENT(ent[2])->deck, ENT(ent[2])->name,
                            q->decks[0].name);
            else
                tpt_set_name(0, ENT(ent[2])->kind, ENT(ent[2])->type2, ENT(ent[2])->src, ENT(ent[2])->deck,
                             ENT(ent[2])->name);
            if (ENT(ent[0])->src == 2)
                tpt_set_all(2, ENT(ent[0])->kind, ENT(ent[0])->type2, ENT(ent[0])->src, ENT(ent[0])->deck, ENT(ent[0])->name,
                            q->decks[2].name);
            else
                tpt_set_name(2, ENT(ent[0])->kind, ENT(ent[0])->type2, ENT(ent[0])->src, ENT(ent[0])->deck,
                             ENT(ent[0])->name);
        }
    }
    if (trig & region_pick_value(0)) {
        se_play(0x6213);
        return t->arg;
    }
    if (trig & region_pick_value(1)) {
        se_play(0x6214);
        return -2;
    }
    return -1;
}

int tdl_vs_input(void)
{
    Title *t = &title;
    TFdTail *q = &title.duel.tail;
    int half = title.u874 / 2;
    PadState *pad = pad_get();
    unsigned int trig = pad->b.trig;
    unsigned int rep = pad->b.rep;
    TPtEnt s;
    int r;
    short w;

    if (title.u3450 == 0 && trig != 0)
        tframe_cursor_mode(0);
    if ((w = t->u3450) == 0) {
        if (t->u86c == 0) {
            if (trig & 0x8000) {
                switch (t->arg) {
                case 0:
                case 2:
                case 4:
                case 6:
                    tpt_get_ent(&s, tdl_seat_of(4, t->arg));
                    if (s.kind != 1) {
                        tframe_cursor_mode(1);
                        se_play(0x6213);
                        q->u002 = t->arg;
                        tdl_random_seat(q->u003);
                        t->u3450 = 1;
                    } else {
                        se_play(0x6215);
                    }
                    break;
                default:
                    se_play(0x6215);
                    break;
                }
                return -1;
            }
            if ((rep & 0x20) || (rep & 0x80) || (trig & 0x20) || (trig & 0x80)) {
                if (t->arg / half != 0) {
                    if ((rep & 0x80) || (trig & 0x80) || (trig & 0x20)) {
                        t->arg -= half;
                        se_play(0x6212);
                    }
                } else {
                    if ((rep & 0x20) || (trig & 0x80) || (trig & 0x20)) {
                        t->arg += half;
                        se_play(0x6212);
                    }
                }
            } else if (rep & 0x40) {
                if (t->arg % half == 3) {
                    t->u86c++;
                    se_play(0x6212);
                } else {
                    t->arg++;
                    se_play(0x6212);
                }
            } else if ((trig & 0x10) || (rep & 0x10)) {
                if (t->arg % half == 0) {
                    if (trig & 0x10) {
                        t->u86c++;
                        se_play(0x6212);
                    }
                } else {
                    t->arg--;
                    se_play(0x6212);
                }
            }
        } else if (trig & 0x40) {
            se_play(0x6212);
            t->u86c = 0;
            t->arg = t->arg / half * 4;
        } else if (rep & 0x10) {
            se_play(0x6212);
            t->u86c = 0;
            t->arg = t->arg / half * 4 + 3;
        }
        if (trig & region_pick_value(0)) {
            se_play(0x6213);
            r = t->u86c;
        } else if (trig & region_pick_value(1)) {
            se_play(0x6214);
            r = -2;
        } else {
            r = -1;
        }
    } else {
        if (w < 60) {
            tdl_random_seat(q->u003);
            se_play(0x6212);
            t->u3450++;
        } else {
            tframe_cursor_mode(0);
            se_play(0x6213);
            t->u3450 = 0;
        }
        r = -1;
    }
    return r;
}

void *tdl_vs_draw(void *list)
{
    Title *t = &title;
    TPtEnt s;

    list = title_ehf_draw(list, 12);
    list = title_ehf_draw(list, 15);
    list = title_screen_label(list);
    if (title_ehf_frame(12) == 0x7B)
        se_play(0x628C);
    else if (title_ehf_frame(12) == 0xAF)
        se_play(0x628D);
    if (t->step >= 9) {
        list = tbox_header(list, (TBoxDrawFn)tdl_header_label);
        list = tbox_corner(list, (TBoxDrawFn)tdl_corner_label);
        if (t->step2 == 1) {
            switch (t->arg) {
            case 0:
            case 2:
            case 4:
            case 6:
                tpt_get_ent(&s, tdl_seat_of(4, t->arg));
                if (s.kind != 1)
                    list = tdl_random_label(list);
                break;
            }
        }
    }
    list = tpartner_draw(list);
    if (t->step2 == 1)
        list = title_ehf_draw(list, 11);
    if (t->step >= 9) {
        if (t->u86c != 0) {
            list = tframe_cursor(list, 0xFFFFFFFF, 0x147, 0xF2, 0x97, 0x17);
        } else {
            int r[8][4] = { { 90, 95, 132, 23 }, { 90, 119, 148, 23 }, { 90, 192, 132, 23 },
                            { 90, 216, 148, 23 }, { 259, 95, 132, 23 }, { 243, 119, 148, 23 },
                            { 259, 192, 132, 23 }, { 243, 216, 148, 23 } };
            int i = t->step2 == 0 ? (t->arg == 0 ? 2 : 6) : t->arg;

            list = tframe_cursor(list, 0xFFFFFFFF, r[i][0], r[i][1], r[i][2], r[i][3]);
        }
    }
    return list;
}

void tdl_nop(void)
{
}

void *tdl_pt_draw(void *list)
{
    return tpartner_draw(list);
}

int tdl_chara_loaded(void *arg, int id);

void tdl_chara_load(void)
{
    Title *t = &title;

    switch (t->step2) {
    case 0:
        ehfile_request_module(2, "libcharalist.prx", tdl_chara_loaded, 0);
        gfx_overlay_enable(1);
        t->step2++;
        break;
    case 1:
        break;
    }
}

int tdl_chara_loaded(void *arg, int id)
{
    Title *t = &title;

    if (title_module_start(arg, id) != 0) {
        t->sub_mod = id;
        gfx_overlay_enable(0);
        t->step = 0;
        t->step2 = 0;
        t->sub = 1;
        return 1;
    }
    return 0;
}

void tdl_chara_update(void)
{
    TFdTail *q = &title.duel.tail;
    unsigned char seat = q->u003;
    Title *t = &title;
    int id;
    int deck;

    switch (charalist_update()) {
    case 0:
        break;
    case 1:
        id = charalist_result_id();
        deck = tdl_deck_default(id);
        tpt_set_name(seat, id, charalist_result_costume(), 0, deck, chara_get_name(0, id));
    case 2:
        t->sub_shown = 0;
        t->sub = q->u001;
        t->step = 16;
        t->step2 = 0;
        if (t->sub == 4)
            t->step2 = 1;
        break;
    }
}

void tdl_random_seat(int seat)
{
    int id;
    int type2;
    int deck;
    Title *t = &title;
    int src = 1;

    if (title_any_deck()) {
        charalist_random(t->heap);
        id = charalist_result_id();
        type2 = charalist_result_costume();
        deck = tdl_deck_default(id);
        if (id == 0x29)
            src = 0;
    } else {
        id = 2;
        type2 = 0;
        deck = tdl_deck_default(2);
    }
    tpt_set_name(seat, id, type2, src, deck, chara_get_name(0, id));
    t->u874 = 8;
}

int tdl_seat_of(int sub, int i)
{
    TFdTail *q = &title.duel.tail;

    if (sub == 4) {
        switch (i) {
        case 2:
        case 3:
            q->u003 = 0;
            break;
        case 0:
        case 1:
            q->u003 = 1;
            break;
        case 6:
        case 7:
            q->u003 = 2;
            break;
        case 4:
        case 5:
            q->u003 = 3;
            break;
        }
    } else {
        switch (i) {
        case 0:
        case 1:
            q->u003 = 1;
            break;
        case 2:
            q->u003 = 0;
            break;
        }
    }
    return q->u003;
}

void *tdl_header_label(void *list, int i, int x, int y, int w)
{
    Title *t = &title;
    int id = 0;

    switch (t->sub) {
    case 2:
        id = 7;
        break;
    case 3:
        id = 8;
        break;
    case 4:
        if (t->step2 == 0)
            id = 13;
        else
            id = 9;
        break;
    case 0:
    case 1:
    case 5:
    case 6:
    case 7:
        break;
    }
    return title_msg_font(list, x, y, w, 0, id, 0, 13, 13, 0xFFFFFFFF);
}
