// @range 0x088195B4-0x0881BA64
// @rodata 0x0882B1D0-0x0882B2D4
// @asm 0x088195B4-0x088198EC
// @asm 0x0881A34C-0x0881AF0C
// @asm 0x0881B360-0x0881BA64
/* Free duel setup: the top of the ad hoc mode (own duelist, partner and the extra data step). */
#include "libtitle.h"

unsigned short *wcsncpy(unsigned short *dst, const unsigned short *src, int n);
void tpt_set_name(short i, int kind, int type2, int src, int deck, void *name);
void tpt_set_all(short i, int kind, int type2, int src, int deck, void *name, void *deckname);
void tfd_duelist_sprite(int *id, int *costume, int *x, int *y);
void tfd_adhoctop_init(void);
void tfd_duelist_icon(int on);
void tfd_rule_sprite(int n);
void tfd_load_charalist(void);
void tfd_unload_modules(void);
void tpt_anim_start(int on, int len);
int tpartner_update_net(void);
int tfd_adhoctop_handler(int tag);
int tfd_adhoctop_sprite(int r);
void tfd_adhoctop_draw(int step);
void tfd_adhoctop_icon2(void);
void tfd_adhoctop_draw2(void);
void extdata_open(void *heap);
int extdata_scan(int kind, int *count);
int extdata_load(void *out, int *count);
void func_08824DA8(void);
void func_08824F3C(int type);
int func_08825720(int type, int *dk, int *deck);

#ifdef NON_MATCHING /* o terceiro bloco (parceiro do outro lado) troca registradores: a2/a3/t0 e a ordem do sw de deck */
void tfd_adhoctop_init(void)
{
    int type;
    int var;
    int deck;
    int y;
    TDuelCfg *d = &title.duel;
    TExtList *e = &title.duel.tail.ext;
    TFdList *l = &d->list;
    int other = (d->u001 + 1) % 2;
    TFdDeck *k;

    type = 2;
    var = 0;
    deck_name_copy_out(e->deckname);
    k = &l->deck[d->u001];
    type = 1;
    k->kind = 1;
    var = d->u001;
    deck = 0;
    wcsncpy(k->deckname, e->deckname, 0x18);
    k->side = 0;
    k->type = type;
    k->u76 = var;
    k->dk = 0;
    k->deck = deck;
    tpt_set_name(0, type, var, 0, deck, k->name);

    k = &l->deck[d->u001 + 2];
    k->kind = 1;
    tfd_duelist_sprite(&type, &var, &deck, &y);
    if (type == 1 || type == 0x29) {
        wcsncpy(k->deckname, e->deckname, 0x18);
        k->dk = 0;
    } else {
        k->dk = 1;
    }
    k->side = 2;
    k->type = type;
    k->u76 = var;
    k->deck = deck;

    k = &l->deck[other];
    k->kind = 1;
    k->side = 0;
    k->type = 1;
    k->u76 = other;
    k->dk = 0;
    k->deck = 0;
    type = 1;
    var = other;
    deck = 0;
    tpt_set_name(1, type, var, 0, deck, k->name);

    k = &l->deck[other + 2];
    k->kind = 1;
    tfd_duelist_sprite(&type, &var, &deck, &y);
    if (type == 1 || type == 0x29) {
        wcsncpy(k->deckname, e->deckname, 0x18);
        k->dk = 0;
    } else {
        k->dk = 1;
    }
    k->side = 2;
    k->type = type;
    k->u76 = var;
    k->deck = deck;
}
#endif

/* the panel entry of a player: a saved deck or a recipe has its own deck name */
#define TFD_SET_ENTRY(k, kind, type2)                                                              \
    if ((k)->dk != 1 && ((k)->dk != 0 || (k)->deck != 0))                                          \
        tpt_set_all((k)->side, kind, type2, (k)->dk, (k)->deck, (k)->name, (k)->deckname);         \
    else                                                                                           \
        tpt_set_name((k)->side, kind, type2, (k)->dk, (k)->deck, (k)->name)

void tfd_adhoctop_update(void)
{
    Title *t = &title;
    TDuelCfg *d = &title.duel;
    TFdTail *q = &title.duel.tail;
    TFdList *l = &d->list;
    TFdDeck *me = &l->deck[d->u000];
    TFdDeck *k = &l->deck[d->u004];
    int f;
    int r;

    switch (t->step) {
    case 0:
    case 10:
    case 12: case 13: case 14: case 15: case 17: case 18: case 20: case 21: case 22: case 23:
    case 26: case 28: case 32: case 33: case 42: case 43: case 44: case 47: case 48:
        break;
    case 1:
        tfd_load_charalist();
        return;
    case 2:
        tfd_duelist_icon(0);
        tfd_rule_sprite(6);
        title_fade_out(0);
        t->step = 3;
        break;
    case 3:
        if (title_fade_state() == 0)
            t->step = 4;
        break;
    case 4:
        title_ehf_load(0xF, 2, 0, 0x11);
        title_ehf_load(0xC, 2, 100, 0x81);
        t->step = 5;
        break;
    case 5:
        if (title_ehf_frame(0xF) == 0x11)
            title_ehf_anim(0xF, 2, 0x12, 0x35);
        f = title_ehf_frame(0xC);
        switch (f) {
        case 0x81:
            title_ehf_anim(0xC, 2, 0x82, 0x9F);
        }
        if (f >= 0x9F)
            t->step = 6;
        break;
    case 6:
        if (d->u005 == 4) {
            if (!d->u004b6) {
                tfd_adhoctop_init();
                d->u004b6 = 1;
            }
            tpt_anim_start(1, 10);
            t->step = 7;
        }
        break;
    case 7:
        if (tpartner_update_net() == 1)
            t->step = 11;
        break;
    case 8:
        title_ehf_anim(0xF, 2, 0x12, 0x35);
        title_ehf_anim(0xC, 2, 0x82, 0x9F);
        title_ehf_load(10, 2, 0, 0x1DF);
        title_ehf_load(0xB, 2, 0, 0);
        title_fade_out(0);
        t->step = 9;
        break;
    case 9:
        if (title_fade_state() == 0)
            t->step = 11;
        break;
    case 11:
        r = tfd_adhoctop_handler(d->u004b5);
        switch (r) {
        case -2:
            t->step = 0x1B;
            title_module_stop();
            break;
        case -1:
            break;
        case 0:
        case 1:
        case 2:
        case 3:
            t->step = 0xE;
            t->arg = r;
            k = &l->deck[d->u001];
            k->side = tfd_adhoctop_sprite(r);
            k->kind = 1;
            k->u77 = d->u001;
            TFD_SET_ENTRY(k, 1, d->u001);
            me = k;
            k = &l->deck[d->u004];
            k->side = (me->side + 2) % 4;
            k->kind = 1;
            TFD_SET_ENTRY(k, k->type, k->u76);
            d->u004b4 = 1;
            k->u77 = d->u001;
            t->arg = 0;
            break;
        case 4:
            switch (t->arg) {
            case 0:
            case 1:
                d->u000 = d->u001;
                break;
            case 2:
            case 3:
                d->u000 = d->u004;
                break;
            }
            switch (t->arg) {
            case 0:
                se_play(0x6213);
                break;
            case 2:
                if (title_any_deck()) {
                    t->step = 0x1D;
                    q->u002 = t->arg;
                    t->next_sub = 0xFFF;
                    t->next_step = 0x2E;
                    se_play(0x6213);
                } else {
                    se_play(0x6215);
                    tframe_cursor_mode(0);
                }
                break;
            case 1:
                t->step = 0x1D;
                t->next_sub = 0xFFF;
                q->u002 = t->arg;
                t->next_step = 0x22;
                se_play(0x6213);
                break;
            case 3:
                if (title_any_deck()) {
                    t->step = 0x1D;
                    q->u002 = t->arg;
                    t->next_sub = 0xFFF;
                    t->next_step = 0x22;
                    se_play(0x6213);
                } else {
                    se_play(0x6215);
                    tframe_cursor_mode(0);
                }
                break;
            }
            break;
        case 5:
            t->step = 0x16;
            break;
        case 6:
            t->step = 0x11;
            break;
        }
        break;
    case 0x10:
        tpt_set_name(me->side, 1, d->u001, me->dk, me->deck, me->name);
        tpt_set_name(k->side, k->type, k->u76, k->dk, k->deck, k->name);
        t->arg = 0;
        t->step = 0xB;
        break;
    case 0x18:
    case 0x19:
        if (d->u005 == 0xF) {
            func_088225A0(0);
            title_module_stop();
            title_goto_fade(1, 0x23, 0);
            t->step = 0x1E;
        }
        break;
    case 0x1B:
        title_goto_fade(1, 9, 0);
        t->step = 0x1E;
        break;
    case 0x1D:
        title_goto_step(1, 0xFFF, t->next_sub, t->next_step);
        t->step = 0x1E;
        break;
    case 0x1E:
    case 0x1F:
        if (title_ehf_frame(0xB) == 0xE)
            title_ehf_anim(0xB, 0, 0xE, 0);
        break;
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
        tfd_adhoctop_draw(t->step);
        break;
    case 0x26:
        title_goto_step(0, 0xFFF, 0xFFF, 0x27);
        t->step = 0x2A;
        break;
    case 0x27:
        tfd_adhoctop_icon2();
        break;
    case 0x28:
        title_goto_step(1, 0xFFF, 0xFFF, 0x2B);
        t->step = 0x2A;
        break;
    case 0x29:
        title_goto_step(1, 0xFFF, 0xFFF, 0x2D);
        t->step = 0x2A;
        break;
    case 0x2D:
        tframe_cursor_mode(0);
        t->u86d = 2;
        t->arg = q->u002;
        t->step2 = 1;
        t->u86c = 0;
        title_fade_out(0);
        t->step = 8;
        break;
    case 0x2E:
        tfd_unload_modules();
        break;
    case 0x31:
        t->arg = q->u002;
        t->step2 = 1;
        t->u86c = 0;
        title_fade_out(0);
        t->step = 8;
        break;
    case 0x13:
        t->step = 0xB;
        break;
    }
}

void tfd_adhoctop_draw(int step)
{
    TPtEnt s;
    Title *t = &title;
    TExtList *e = &title.duel.tail.ext;
    int ok = 0;
    int size;

    fade_start(1, 0xFF000000, 0x10);
    fade_finish();
    tpt_get_ent(&s, t->duel.list.deck[title.duel.u000].side);
    switch (s.kind) {
    case 1:
    case 0x29:
        ok = 1;
        break;
    }
    if (ok) {
        switch (t->step) {
        case 0x22:
            func_088225A0(0);
            extdata_open(t->heap);
            t->step = 0x23;
            break;
        case 0x23:
            if (extdata_scan(1, &e->n))
                t->step = 0x24;
            break;
        case 0x24:
            if (e->n != 0) {
                size = e->n * sizeof(TExtRec);
                func_088225A0(0);
                e->rec = heap_alloc(t->heap, size);
                memset(e->rec, 0, size);
                e->b0 = 1;
                e->stage = 1;
                t->step = 0x25;
            } else {
                t->step = 0x26;
            }
            break;
        case 0x25:
            if (extdata_load(e->rec, &e->loaded)) {
                if (e->loaded >= 0x100)
                    e->loaded = 0x100;
                t->step = 0x26;
                e->stage = 2;
            }
            break;
        }
    } else {
        t->step = 0x26;
    }
    if (t->step == 0x26) {
        fade_start_in(1, 0xFF000000, 0x10);
        fade_finish();
        tfd_adhoctop_draw2();
    }
}

int tfd_adhoctop_icon(int i)
{
    switch (i) {
    case 0:
        return 1;
    case 1:
        return 0;
    case 2:
        return 3;
    case 3:
        return 2;
    }
    return 0;
}

int tfd_adhoctop_sprite(int r)
{
    switch (r) {
    case 0:
        return 1;
    case 1:
        return 0;
    case 2:
        return 3;
    case 3:
        return 2;
    case 4:
    case 5:
        break;
    }
    return 0;
}

int tfd_adhoctop_enter_init(int on)
{
    TPtEnt s;
    TDuelCfg *d = &title.duel;
    int i;
    int side;

    switch (title.duel.list.n) {
    case 2:
        return 1;
    case 3:
        if (on) {
            for (i = 0; i < 3; i++) {
                if (title.duel.list.deck[i].kind == 0)
                    break;
            }
            side = (title.duel.list.deck[d->u001].side + 2) % 4;
            tpt_get_ent(&s, side);
            d->u004 = side;
            if (s.kind != 1) {
                d->u004b4 = 1;
                return 1;
            }
        }
        break;
    case 4:
        break;
    }
    return 0;
}

void tfd_adhoctop_draw2(void)
{
    TPtEnt s;
    Title *t = &title;
    TFdTail *q = &title.duel.tail;
    TExtList *e = &q->ext;
    TDuelCfg *d = &title.duel;

    func_08824DA8();
    title.step2 = 0;
    title.frame = 0;
    title.u86c = 0;
    title.u86d = 1;
    title.arg = 0;
    title.u874 = 1;
    title.u878 = 0;
    q->u003 = d->u000;
    title.u879 = 6;
    tpt_get_ent(&s, t->duel.list.deck[d->u000].side);
    e->type = s.kind;
    switch (e->type) {
    case 1:
    case 0x29:
        if (e->stage == 2) {
            q->u004 = 2;
            t->u86d = 3;
        } else {
            t->u86d = 2;
            q->u004 = 0;
        }
        break;
    default:
        t->u86d = 1;
        q->u004 = 5;
        break;
    }
    title_ehf_load(4, 0, q->u004, q->u004);
    func_08824F3C(e->type);
}

void tfd_adhoctop_icon2(void)
{
    Title *t = &title;
    TFdTail *q = &title.duel.tail;
    TFdList *l = &title.duel.list;
    TExtList *e = &q->ext;
    int r;
    int i;

    switch (func_08825720(e->type, &e->dk, &e->deck)) {
    case -1:
        break;
    case -2:
        t->step = 0x29;
        break;
    default:
        tframe_cursor_mode(1);
        l->sel.u02 = e->u04;
        l->sel.type = e->type;
        l->sel.dk = e->dk;
        l->sel.deck = e->deck;
        t->step = 0x28;
        if (e->dk == 2) {
            i = e->type != 1;
            memcpy(q->decks[i].name, e->rec[e->deck].name, 0x40);
            q->decks[i].recipe = e->rec[e->deck].recipe;
        }
        break;
    }
}
