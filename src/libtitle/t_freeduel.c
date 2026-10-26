// @range 0x08813AC4-0x088148A8
// @rodata 0x0882AF38-0x0882AF94
/* Free duel setup (state 3 and its sub-screens): the player list, the deck list, the options panel. */
#include "libtitle.h"

unsigned short *chara_get_name(int set, int id);
void *tfd_icon_setup(void *list, MenuWidget *w, int x, int y, int cw, int ch);
int tfd_evt_handler(int a, int b);
void tfd_comm_machine(int a, int b);
void tfd_show_msgbox(MenuWidget *w, int msg);
void text_ext_reset(void);
unsigned short *sysstate_get_name(void);
unsigned short *wcsncpy(unsigned short *dst, const unsigned short *src, int n);
void net_leave(void);
void net_adhoc_stop(void);
void net_get_hosts(TFdImpList *out);
void net_host(TFdImp *me);
void net_join(int i, TFdScanEnt *me);
int net_join_tagged(int i, void *me);
void net_get_scan(TFdScan *out);
void net_toggle_pick(int i);
void net_start_game(void);
void net_get_host_players(TFdScan *out);
void net_get_host_info(TFdImp *out);
void tfd_entry_update(void);
void net_adhoc_start(void);
int net_get_state(void);
void net_set_state(int s);
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

void tfd_populate_config(TFdScan *src)
{
    TFdList *const l = &title.duel.list;
    TFdScanEnt *d;
    int i;

    memset(l->deck, 0, sizeof(l->deck));
    l->n = 9;
    for (i = 0; i < 9; i++) {
        d = &src->e[i];
        l->deck[i].on = d->valid;
        l->deck[i].name = d->name;
        l->deck[i].kind = d->joined;
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

/* the host entries are indexed by byte offset in the original (pointer first in the address sum) */
#define TFD_HOST(l, i) ((TFdImp *)((char *)(l) + (i) * sizeof(TFdImp) + 4))

int tfd_evt_handler(int a, int b)
{
    TFdImp me;
    TFdScanEnt se;
    struct {
        unsigned short tag;
        unsigned short pad;
        unsigned short name[0x18];
    } jt;
    int r;
    Title *t = &title;
    TDuelCfg *d = &title.duel;
    TFdList *l = &d->list;
    TFdImpList *imps = &l->imps;
    TFdScan *scan = &l->scan;
    TDuelOpts *o = &d->opts;
    TBox *box = &title.panel.box;
    MenuWidget *win = &title.win[1];
    int m;

    r = 1;
    if (a == 8)
        net_set_state(11);
    if (a == 9)
        net_leave();
    switch (net_get_state()) {
    case 10:
        return 0;
    case -1:
        net_adhoc_stop();
        d->anim = 0;
        t->sub = 20;
        break;
    case 0:
        if (t->sub == 3)
            net_set_state(4);
        else if (t->sub == 20)
            net_adhoc_stop();
        break;
    case 5:
        break;
    case 11:
        if (a == 2) {
            net_set_state(0x21);
        } else if (a == 0) {
            title.duel.list.n = imps->n;
            switch (t->sub) {
            case 3:
                t->sub = 4;
                break;
            case 20:
                net_set_state(0x13);
                break;
            }
            net_get_hosts(imps);
            tfd_populate_import(4);
        } else if (a == 3) {
            me.kind = o->kind;
            me.b5 = o->tag;
            me.b6 = o->swap;
            me.lo = o->lp;
            me.hi = o->time;
            wcsncpy(me.name, o->name, 15);
            net_host(&me);
        } else if (a == 4) {
            if (TFD_HOST(imps, box->pos)->side) {
                wcsncpy(se.name, sysstate_get_name(), 15);
                net_join(box->pos, &se);
            }
        } else if (a == 5) {
            if (TFD_HOST(imps, box->pos)->side) {
                jt.tag = b;
                wcsncpy(jt.name, sysstate_get_name(), 15);
                r = net_join_tagged(box->pos, &jt);
            }
        }
        break;
    case 0x1E:
        if (a == 0) {
            net_get_scan(scan);
            tfd_populate_config(scan);
            if (t->sub == 3)
                net_set_state(0x13);
        } else if (a == 6) {
            net_toggle_pick(b);
            tfd_populate_config(scan);
        } else if (a == 7) {
            net_start_game();
        }
        break;
    case 0x15:
        if (t->sub == 4 && t->step == 12) {
            t->step = 0x1D;
        } else if (t->sub == 7) {
            net_get_host_info(&d->host);
            o->name = d->host.name;
            o->kind = d->host.kind;
            o->tag = d->host.b5;
            o->swap = d->host.b6;
            o->lp = d->host.lo;
            o->time = d->host.hi;
            if (a == 2)
                net_leave();
        } else {
            net_get_host_players(scan);
            if (a == 2)
                net_leave();
        }
        break;
    case 14:
    case 15:
    case 16:
    case 17:
        switch (t->step) {
        case 11:
        case 12:
            t->step = 20;
            m = 0x72;
            se_play(0x621A);
            if (t->next_sub == 0x13)
                m = 0x77;
            tfd_show_msgbox(win, m);
            break;
        case 20:
            break;
        case 0x16:
            net_set_state(11);
            switch (t->sub) {
            case 4:
                t->step = 11;
                break;
            case 5:
            case 7:
                t->step = 0x1B;
                break;
            }
            break;
        }
        break;
    case 0x23:
        if (t->step == 0x16) {
            net_set_state(0x13);
            t->step = 0x1B;
        } else if (t->step != 20) {
            t->step = 20;
            se_play(0x621A);
            tfd_show_msgbox(win, 0x73);
        }
        break;
    case 8:
        d->u005 = 0;
        if (d->u001 && t->step == 11) {
            t->step = 0x1D;
            if (!o->tag)
                t->next_sub = 8;
            else
                t->next_sub = 9;
            text_ext_reset();
        }
        break;
    case 0x1F:
        switch (t->sub) {
        case 6:
            if (t->step == 0x1C)
                t->step = 0x1D;
            break;
        default:
            if (t->step == 0)
                t->step++;
            tfd_comm_machine(a, b);
            if (a == 2)
                net_set_state(0x21);
            break;
        }
        break;
    case 7:
        break;
    case 9:
        break;
    default:
        if (a == 2)
            net_set_state(0x21);
        break;
    }
    return r;
}
