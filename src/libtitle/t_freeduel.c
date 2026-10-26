// @range 0x08813AC4-0x088141F4
// @rodata 0x0882AF38-0x0882AF94
/* Free duel setup (state 3 and its sub-screens): the player list, the deck list, the options panel. */
#include "libtitle.h"

unsigned short *chara_get_name(int set, int id);
void *tfd_icon_setup(void *list, MenuWidget *w, int x, int y, int cw, int ch);
int tfd_evt_handler(int a, int b);
void tfd_entry_update(void);
void net_adhoc_start(void);
void tfd_vscpu_update(void);
void tfd_cursor_update(void);
void tfd_regulation_update(void);
void tfd_duelist_update(void);
void tfd_adhoctop_update(void);
void tfd_pass_init(void);
void tfd_pass_setup_init(void);
void tfd_pass_join_init(void);
void tfd_pass_select_init(void);
void tfd_lobby_update(void);
void tfd_net_event_handler(void);
void tfd_vscpu_entry_update(void);

void tfd_clear(void)
{
    memset(&title.duel.list, 0, sizeof(TFdList));
}

int tfd_populate_import(int mode)
{
    TFdList *const l = &title.duel.list;
    TFdImpList *ip = &l->imps;
    TFdImp *e;
    TFdSeat *s;
    int i;

    if (mode == 4) {
        for (i = 0; i < 9; i++) {
            s = &l->seat[i];
            e = &ip->e[i];
            l->seat[i].side = e->side;
            l->seat[i].kind = e->kind;
            l->seat[i].b5 = e->b5;
            l->seat[i].b6 = e->b6;
            l->seat[i].lo = e->lo;
            l->seat[i].hi = e->hi;
            l->seat[i].name = e->name;
            l->seat[i].st = 1;
            if (e->kind == 1) {
                s->card = e->card;
                s->card_name = card_get_name(s->card);
            }
            if (s->side != 0)
                l->n++;
        }
        if (l->n < 5)
            l->n = 5;
    } else {
        int n;
        l->n = 9;
        n = l->n;
        for (i = 0; i < n; i++)
            l->seat[i].name = chara_get_name(0, i + 1);
    }
    return 1;
}

void tfd_populate_config(TFdDeckSrc *src)
{
    TFdList *const l = &title.duel.list;
    TFdDeckInfo *d;
    int i;

    memset(l->deck, 0, sizeof(l->deck));
    l->n = 9;
    for (i = 0; i < 9; i++) {
        d = &src[i].d;
        l->deck[i].on = d->on;
        l->deck[i].name = d->name;
        l->deck[i].kind = d->kind;
    }
}

void tfd_memset_big(void *p)
{
    memset(p, 0, 0x1180);
}

void tfd_config_init(int state, int sub)
{
    TPanel *const p = &title.panel;
    MenuWidget *w = &p->mw;
    TBox *b = &p->box;
    TDuelCfg *d = &title.duel;
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
    d->anim = 0;
    tpanel_init(p, title.heap);
    p->f0 = 0;
    p->n = 1;
    b->x = 4;
    def->x = 4;
    b->y = 52;
    def->y = 52;
    b->w = 305;
    b->h = 42;
    def->h = 42;
    w->item_h = b->h + 1;
    def->color = 0xFFFFFF;
    w->count = 20;
    w->rows = 5;
    w->draw_cursor = (MenuDrawCursorFn)tfd_icon_setup;
    def->pad_l = 0;
    def->pad_r = 0;
    def->pad_t = 0;
    def->pad_b = 0;
    w->item_w = 305 - (def->pad_l + def->pad_r);
    w->item_y = 0;
    def->w = 305;
    w->cursor_w = w->item_w;
    w->cursor_h = b->y - 8 - w->item_y * 2;
    b->count = w->count;
    b->n = w->rows;
    w->draw_cursor_post = 0;
    w->item_x = 0;
    b->text = 0;
    b->draw = 0;
    tpartner_init(title.heap, title_pack(), 5);
    memset(d->uf00, 0, sizeof(d->uf00));
    memset(d->uf4c, 0, sizeof(d->uf4c));
}

void tfd_update(void)
{
    Title *t = &title;
    TDuelCfg *d = &title.duel;

    if (t->sub < 22) {
        if (t->sub >= 3) {
            if (!tfd_evt_handler(0, 0))
                t->busy = 1;
            else
                t->busy = 0;
        }
        if (!t->busy) {
            switch (t->sub) {
            case 0:
                tfd_entry_update();
                break;
            case 1:
            case 2:
                net_adhoc_start();
                t->sub = 3;
                break;
            case 4:
                tfd_vscpu_update();
                break;
            case 5:
                tfd_cursor_update();
                break;
            case 6:
            case 7:
                tfd_regulation_update();
                break;
            case 8:
                tfd_duelist_update();
                break;
            case 9:
                tfd_adhoctop_update();
                break;
            case 10:
                tfd_pass_init();
                break;
            case 11:
                tfd_pass_setup_init();
                break;
            case 12:
                tfd_pass_join_init();
                break;
            case 13:
                tfd_pass_select_init();
                break;
            case 14:
                tfd_lobby_update();
                break;
            case 15:
                tfd_net_event_handler();
                break;
            case 16:
            case 17:
            case 18:
            case 19:
                tfd_vscpu_entry_update();
                break;
            case 20:
                if (t->b3038_5)
                    break;
                t->sub = 21;
            case 21:
                tpartner_free();
                title_module_stop();
                func_088225A0(0);
                title_goto_fade(1, 9, 7);
                t->sub = 22;
                break;
            case 22:
                break;
            }
        }
    }
    t->prev_sub = t->sub;
    d->blink++;
    if (d->blink >= 30)
        d->blink = 0;
}
