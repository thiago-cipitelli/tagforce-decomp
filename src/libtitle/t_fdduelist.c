// @range 0x08818A88-0x088195B4
// @rodata 0x0882B118-0x0882B1D0
// @data 0x0882F238-0x0882F290
/* Free duel setup: choosing the duelists (sub 8) and the screen that leads into the extra data / ad hoc steps. */
#include "libtitle.h"

#define TFD_AT(t, f, off) typedef char t##_##f##_at[(int)&((t *)0)->f == (off) ? 1 : -1]
TFD_AT(Title, u874, 0x874);
TFD_AT(TFdTail, u002, 0x2);
TFD_AT(TDuelCfg, ready, 0xEB0);

void tfd_rule_icon(int on);
void tfd_rule_sprite(int n);
void tpt_anim_start(int on, int len);
int tpartner_update_net(void);
void *tpartner_draw(void *list);
void tpt_set_name(short i, int kind, int type2, int src, int deck, void *name);
void *tframe_cursor(void *list, unsigned int color, int x, int y, int w, int h);
void *extdata_draw();
void *tfd_show_duelinfo(void *list, int i, int x, int y, int w, int h);
void *tfd_sprite_anim(void *list, int i, int x, int y, int w, int h);
void tfd_adhoctop_draw(int step);
void tfd_adhoctop_icon2(void);
int func_088237D4(int a);
int func_0882527C(int id);
void *func_08825BBC(void *list);
void tfd_duelist_draw(void);
int tfd_duelist_enter_init(void);

void tfd_duelist_update(void)
{
    Title *t = &title;
    TDuelCfg *d = &title.duel;
    TFdTail *q = &title.duel.tail;
    int f;

    switch (t->step) {
    case 0:
    case 10:
    case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
    case 26: case 28: case 30: case 31: case 32: case 33: case 42: case 43: case 44:
        break;
    case 1:
        tfd_rule_icon(0);
        tfd_rule_sprite(5);
        title_fade_out(0);
        t->step = 2;
        break;
    case 2:
    case 3:
        if (title_fade_state() != 0)
            return;
        t->step = 4;
        break;
    case 4:
        title_ehf_load(0xF, 2, 0, 0x11);
        title_ehf_load(0xC, 2, 0, 0x1D);
        t->step = 5;
        break;
    case 5:
        if (title_ehf_frame(0xF) == 0x11)
            title_ehf_anim(0xF, 2, 0x12, 0x35);
        f = title_ehf_frame(0xC);
        switch (f) {
        case 0x1D:
            title_ehf_anim(0xC, 2, 0x1E, 0x3B);
        }
        if (f >= 0x1D)
            t->step = 6;
        break;
    case 6:
        if (d->u005 == 4) {
            tfd_duelist_draw();
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
        title_ehf_anim(0xC, 2, 0x1E, 0x3B);
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
        switch (tfd_duelist_enter_init()) {
        case -1:
            break;
        case -2:
            break;
        case 0:
            t->step = 0x1D;
            q->u002 = t->arg;
            t->next_sub = 0xFFF;
            t->next_step = 0x22;
            break;
        case 1:
            t->step = 0x16;
            break;
        }
        break;
    case 0x18:
    case 0x19:
        if (d->u005 == 0xF) {
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
        tfd_rule_icon(1);
        t->u86d = 2;
        t->u86c = 0;
        t->arg = 0;
        t->step2 = 0;
        title_fade_out(0);
        t->step = 8;
        break;
    }
}

void tfd_duelist_draw(void)
{
    TDuelCfg *d = &title.duel;
    TFdList *l = &d->list;
    int other = (d->u001 + 1) % 2;
    TFdDeck *k;

    k = &l->deck[d->u001];
    k->side = 0;
    k->type = 1;
    k->u76 = 0;
    k->kind = 1;
    tpt_set_name(k->side, 1, d->u001, 3, k->deck, k->name);
    k = &l->deck[other];
    k->side = 1;
    k->type = 1;
    k->u76 = 0;
    k->kind = 1;
    tpt_set_name(k->side, 1, other, 3, k->deck, k->name);
}

int tfd_duelist_enter_init(void)
{
    return func_088237D4(0);
}

void *tfd_duelist_draw2(void *list)
{
    Title *t = &title;
    TDuelCfg *d = &title.duel;
    int sel = title.u86c;

    if (t->step < 0x22) {
        list = title_ehf_draw(list, 10);
        if (t->step >= 5) {
            list = title_ehf_draw(list, 0xF);
            list = title_screen_label(list);
        }
        list = title_ehf_draw(list, 0xC);
        if (title_ehf_frame(0xC) == 0x17)
            se_play(0x628C);
        else if (title_ehf_frame(0xC) == 0x4B)
            se_play(0x628D);
        if (t->step >= 11) {
            list = tbox_header(list, tfd_show_duelinfo);
            list = tbox_corner(list, tfd_sprite_anim);
        }
        if (t->step >= 7)
            list = tpartner_draw(list);
        if (t->step >= 11) {
        int cur[2][4] = { { 0x6E, 0xCE, 0x94, 0x17 }, { 0x147, 0xF2, 0x97, 0x17 } };
            list = tframe_cursor(list, -1, cur[sel][0], cur[sel][1], cur[sel][2], cur[sel][3]);
        }
        if (t->step >= 11 && t->step < 0x22) {
            int pos[2][2] = { { 0x25, 0x9E }, { 0x175, 0x3D } };
            int off[4][2] = { { -14, 38 }, { -14, 38 }, { 50, 38 }, { 50, 38 } };
            int ids[2] = { 0x6A, 0x6B };
            for (sel = 0; sel < 2; sel++) {
                if (d->ready[sel].on)
                    list = title_sprite(list, 1, ids[sel], pos[sel][0] + off[sel][0], pos[sel][1] + off[sel][1]);
            }
        }
    } else if (t->step >= 0x23 && t->step < 0x26) {
        list = extdata_draw();
    } else if (t->step > 0x26) {
        list = func_08825BBC(title_ehf_draw(list, 10));
    }
    return list;
}

void tfd_duelist_icon(int on)
{
    Title *t = &title;
    TDuelCfg *d = &title.duel;

    memset(d->ready, 0, sizeof(d->ready));
    title.step = 0;
    title.step2 = 0;
    title.frame = 0;
    title.u86c = 0;
    title.u86d = 2;
    d->u004b6 = 0;
    if (on)
        d->u004b6 = 1;
    d->u000 = d->u001;
    d->u004b5 = 0;
    if (title.duel.list.n == 2) {
        t->arg = 1;
        d->u004b5 = 1;
        d->u004 = d->u001 + 2;
        d->u004b4 = 1;
    } else {
        switch (d->u001) {

        case 0:
            t->arg = 1;
            break;
        case 1:
            t->arg = 3;
            break;
        case 2:
            t->arg = 0;
            break;
        case 3:
            t->arg = 2;
            break;
        }
    }
    t->u874 = 4;
    if (!on) {
        title_ehf_load(0xF, 2, 0, 0x11);
        title_ehf_load(0xC, 2, 0, 0);
    } else {
        title_ehf_anim(0xF, 2, 0x12, 0x35);
        title_ehf_anim(0xC, 2, 0x1E, 0x3B);
    }
    title_ehf_load(10, 2, 0, 0x1DF);
    title_ehf_load(0xB, 2, 0, 0);
}

void tfd_duelist_sprite(int *id, int *costume, int *x, int *y)
{
    Title *t = &title;

    if (title_any_deck()) {
        charalist_random(t->heap);
        if (id)
            *id = charalist_result_id();
        if (costume)
            *costume = charalist_result_costume();
        if (x)
            *x = func_0882527C(*id);
        if (id && y) {
            if (*id == 1 || *id == 0x29)
                *y = 0;
            else
                *y = 1;
        }
    } else {
        if (id)
            *id = 2;
        if (costume)
            *costume = 0;
        if (x)
            *x = func_0882527C(*id);
        if (y)
            *y = 1;
    }
}
