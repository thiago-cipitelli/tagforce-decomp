// @range 0x08820F88-0x08822B0C
// @asm 0x08820F88-0x088212B0
// @asm 0x08821460-0x088215A0
// @asm 0x088215A0-0x08821868
// @asm 0x08821954-0x08821A3C
// @asm 0x08821A3C-0x08821BAC
// @asm 0x08821DBC-0x08821F38
// @asm 0x08821FA8-0x0882214C
// @asm 0x0882228C-0x08822520
// @asm 0x08822520-0x088226C0
// @asm 0x08822914-0x08822B0C
// @data 0x08876110-0x08876200
/* The action window over a field location (summon, set, activate, attack... as icons of
   dd_act_tab, placed next to the card on screen) and the message windows of the duel's other
   questions: the battle position choice, the chain confirmation with the cards involved, yes/no and
   the "use this card?" choice between a card and its back. */
#include "dd_field_ui.h"

typedef struct DdLocSrc {              /* where the action window's location comes from */
    void (*get)(DdLoc *out);
} DdLocSrc;

typedef struct DdActMenu {             /* 0x08C582B8, 0x9C */
    DdLocSrc src;                      /* +0x00 */
    int n;                             /* +0x04 */
    int act[32];                       /* +0x08 rows of dd_act_tab */
    int text_w;                        /* +0x88 widest action name */
    MenuWidget *w;                     /* +0x8C */
    int sel;                           /* +0x90 menu_update's answer */
    unsigned int bits;                 /* +0x94 actions allowed (bit = DdActIcon.act), 0x2000 card shown */
    int card;                          /* +0x98 */
} DdActMenu;

typedef struct DdActIcon {             /* 0x08876110: icon (u, v in the sheet), name and action */
    int u, v;
    int text;
    int act;
} DdActIcon;

typedef struct DdPosSel {              /* 0x08C58354: battle position choice */
    unsigned int mask;                 /* positions offered */
    int cur;                           /* +0x04 */
    MenuWidget *w;                     /* +0x08 */
} DdPosSel;

typedef struct DdChainSel {            /* 0x08C58360: chain confirmation, 0x4C */
    MenuWidget *w;
    int uid[16];                       /* +0x04 cards involved */
    int n;                             /* +0x44 */
    int more;                          /* +0x48 more than shown */
} DdChainSel;

extern int (*dd_act_hook)(DdLoc loc, int *acts, int n); /* 0x08C582B4 >0: action (+1) chosen at once */
extern DdActMenu dd_act_menu;
extern DdPosSel dd_pos_sel;
extern DdChainSel dd_chain_sel;
extern int dd_yesno_default;           /* 0x08C583AC open on "no" */
extern int dd_sel_flag;                /* 0x08C583B0 */
extern int dd_sel_card;                /* 0x08C583B4 */

DdActIcon dd_act_tab[15] = {
    { 91, 66, 7, 0 },   { 65, 79, 2, 1 },   { 26, 79, 8, 2 },  { 78, 66, 9, 3 },
    { 65, 66, 1, 4 },   { 39, 66, 10, 5 },  { 104, 92, 65, 6 }, { 52, 66, 64, 7 },
    { 26, 66, 5, 8 },   { 13, 66, 6, 9 },   { 0, 79, 11, 10 }, { 65, 92, 12, 11 },
    { 104, 66, 13, 12 }, { 0, 66, 4, 13 },  { 0, 66, 47, 14 },
};

int dv_w624_set();
int dv_list_id(int i);
char *dv_chain(void);
int dv_088695B8(int side, int other, int z, void *a);
int dv_08869728(int side, int i, void *a);
int dv_088BFFD8(int side, int id, int i);
unsigned int dv_n_hand(int side);
int dv_hand_uid(int side, int i);
int dv_slot_uid(int side, int z);
int dv_zone_c_uid(int side, int i);
int dv_zone_d_uid(int side, int i);
unsigned int dv_08835AEC(int side, int z, unsigned int i);
int dv_zone_card_id(int side, int z, int i);
int dv_n_grave(int side);
int dv_n_f(int side);
int dd_loc_visible(DdLoc loc);
void dd_info_select_uid(int uid);
void dd_act(int player, int zone, int idx, int act);
int dd_is_tag_duel(void);
int card_is_monster(int id);
void *dd_card_texture(void *list, int id);
unsigned short *dd_text(int id);
void *dd_gim_get(int id);
int dd_input_on(void);
unsigned int *dd_pad(void);
void dd_mode_push(int m);
void dd_mode_set_flag(int f);
void dd_mode_set_card(int card);
DdCobj *dd_cobj_at(int player, int zone, int index);
void dd_slot_xform(float *pos, float *rot, DdLoc at);
float *mat4_transform_vec4(float *out, float *m, float *v);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
int font_draw_wtext_ex(int x, int y, int w, int h, unsigned short *text, int max_glyphs);
unsigned int font_measure_wtext2(int w, int h, unsigned short *text);
void func_08825824(unsigned short *title, unsigned short *text, int kind, void (*setup)(MenuWidget *w, int open));
int func_08825728(void);
void func_088259E4(void *(*fn)(void *dl));
void func_088259F0(void (*fn)(MenuWidget *w, int old, int cur));
void func_088259FC(void (*fn)(void));
void func_08825A08(int (*fn)(void));
int func_08825A14(void);
void func_08825AFC(unsigned short *text);
void func_08825B7C(int v);
void func_088212B0(void);
void func_088212B8(int (*hook)(DdLoc loc, int *acts, int n));
unsigned int *func_088212C4(unsigned int *dl, int x, int y, int act);
void func_0882136C(MenuWidget *w, int open);
void func_08821460(void);
void func_088215A0(void);
void func_08821868(void);
void func_088218CC(DdLoc loc, int act);
void *func_0882192C(void *list);
void func_08821954(unsigned int *dl, unsigned short i, int x, int y);
unsigned int func_08821A3C(DdLoc loc);
int func_08821BAC(void);
unsigned short *func_08821C90(int text);
void func_08821DBC(MenuWidget *w, int open);
void func_08821F38(MenuWidget *w, int old, int i);
void func_08821F70(void);
void *func_08821FA0(void *list);
unsigned int *func_08821FA8(unsigned int *dl, unsigned short i, int x, int y);
void func_088221C4(MenuWidget *w, int open);
void func_08822240(void);
void func_0882228C(unsigned int *dl);
void func_08822520(void);
void func_08822740(MenuWidget *w, int open);
void func_088227EC(void);
void func_088228B8(MenuWidget *w, int open);
void func_088228F0(void);
unsigned int *func_08822914(unsigned int *dl, unsigned short i, int x, int y);

#ifdef NON_MATCHING /* alocação de registradores ou forma não reproduzida */
/* opens the action window of the location given by src; a single plain action (or the hook) acts
   at once */
void func_08820F88(DdLocSrc *src)
{
    DdActMenu *m = &dd_act_menu;
    DdLoc loc;
    DdCobj *o;
    int r;
    int i;

    src->get(&loc);
    dd_act_menu.n = 0;
    dd_act_menu.text_w = 0;
    dd_act_menu.src = *src;
    dd_act_menu.w = 0;
    dd_act_menu.card = dv_zone_card_id(loc.player, loc.zone, loc.idx);
    func_088212B0();
    if (dd_act_hook && (r = dd_act_hook(loc, m->act, m->n)) > 0) {
        func_088218CC(loc, r - 1);
        return;
    }
    if (m->n == 0) {
        se_play(0x6215);
        return;
    }
    for (i = 0; i < m->n; i++) {
        if (m->act[i] != 11 && m->act[i] != 13)
            break;
    }
    if (i == m->n && m->act[0] == 11) {
        func_088218CC(loc, m->act[0]);
        se_play(0x6213);
        return;
    }
    func_08825824(0, 0, 0x20, func_0882136C);
    dd_mode_push(4);
    if (dd_is_tag_duel() == 0)
        dd_mode_set_flag(0x31);
    func_088259FC(func_08821868);
    func_088259E4(func_0882192C);
    func_08825A08(func_08821BAC);
    func_08825B7C(1);
    se_play(0x6216);
    if (loc.zone == 12)
        loc.idx = 0;
    else if (loc.zone == 13)
        loc.idx = 0;
    else if (loc.zone == 15)
        loc.idx = dv_n_f(loc.player) - 1;
    else if (loc.zone == 14)
        loc.idx = dv_n_grave(loc.player) - 1;
    if ((short)loc.idx < 0)
        loc.idx = 0;
    o = dd_cobj_at(loc.player, loc.zone, loc.idx);
    if (o != 0 && (dd_loc_visible(loc) || o->b5))
        dd_info_select_uid(o->uid);
}
#endif

void func_088212B0(void)
{
    func_08821460();
}

void func_088212B8(int (*hook)(DdLoc loc, int *acts, int n))
{
    dd_act_hook = hook;
}

/* icon of action row act */
unsigned int *func_088212C4(unsigned int *dl, int x, int y, int act)
{
    DdActIcon *row = &dd_act_tab[act];

    dl = gim_set_texture(dl, dd_gim_get(0x40005));
    dl = draw_set_offsets(dl, 13, 13, 13, 13);
    return draw_sprite(dl, x, y, 0, row->u, row->v, 0, 0);
}

/* window setup: a row per action; after the layout, sized to the widest name */
void func_0882136C(MenuWidget *w, int open)
{
    DdActMenu *m = &dd_act_menu;
    MenuItemDef *d = &w->def;

    if (open) {
        func_088212B0();
        w->cancel_blocked = 0;
        w->count = m->n;
        w->rows = m->n;
        w->item_w = 100;
        w->item_h = 15;
        w->draw_item = (MenuDrawItemFn)func_08821954;
        d->tab = 1;
        d->pad_t = 2;
        d->pad_b = 2;
        d->pad_l = 2;
        d->pad_r = 2;
    } else {
        w->item_w = m->text_w + 18;
        w->cursor_w = w->item_w;
        d->w = d->pad_r + (w->item_w + 8 + d->pad_l);
        m->w = w;
        if (d->y < 0)
            d->y = 0;
    }
}

#ifdef NON_MATCHING /* alocação de registradores ou forma não reproduzida */
/* the actions allowed at the location and the widest of their names */
void func_08821460(void)
{
    DdActMenu *m = &dd_act_menu;
    DdActIcon *row;
    DdLoc loc;
    unsigned int bits;
    unsigned int i;
    int n;
    int maxw;
    int w;
    int k;

    dd_act_menu.src.get(&loc);
    bits = func_08821A3C(loc);
    dd_act_menu.bits = bits;
    n = 0;
    maxw = 0;
    font_set_size(12, 12);
    row = dd_act_tab;
    for (i = 0; i < 15 && n <= 32; i++, row++) {
        if (bits & (1 << row->act)) {
            m->act[n] = i;
            n++;
        }
    }
    for (k = 0; k < n; k++) {
        w = font_measure_text(func_08821C90(dd_act_tab[m->act[k]].text)) >> 6;
        if (maxw < w)
            maxw = w;
    }
    m->text_w = maxw;
    m->n = n;
    if (n > 32)
        m->n = 32;
}
#endif

#ifdef NON_MATCHING /* alocação de registradores ou forma não reproduzida */
/* places the window above the card on screen, its tab pointing at the card */
void func_088215A0(void)
{
    DdActMenu *m = &dd_act_menu;
    FxMat4 model, view, proj, mvp;
    FxVec4 rot, scr, pos;
    DdLoc loc;
    DdCobj *o;
    MenuItemDef *d;

    if (dd_act_menu.w == 0)
        return;
    m->src.get(&loc);
    o = dd_cobj_at(loc.player, loc.zone, loc.idx);
    if (o != 0 && loc.zone == 11) {
        pos = *(FxVec4 *)&o->pos;
        rot = *(FxVec4 *)&o->rot;
    } else {
        dd_slot_xform(&pos.x, &rot.x, loc);
    }
    mtx_set_mode(2);
    mtx_get34(proj.m);
    mtx_set_mode(1);
    mtx_get34(view.m);
    mtx_set_mode(0);
    mtx_get34(model.m);
    mat4_mul(mvp.m, view.m, proj.m);
    mat4_mul(mvp.m, model.m, mvp.m);
    mtx_set_mode(2);
    pos.w = 1.0f;
    mat4_transform_vec4(&scr.x, mvp.m, &pos.x);
    scr.x = 480.0f * scr.x / scr.w;
    scr.y = -272.0f * scr.y / scr.w;
    scr.x += 2122.0f;
    scr.y += 2044.0f;
    scr.x -= 1808.0f;
    scr.y -= 1912.0f;
    scr.y -= 20.0f;
    d = &m->w->def;
    d->x = scr.x - (float)(d->w / 2);
    d->y = scr.y - (float)d->h;
    if (d->x + d->w > 480)
        d->x = 480 - d->w;
    if (d->x < 148)
        d->x = 148;
    if (d->y < 0)
        d->y = 0;
    d->tab_x = scr.x - (float)d->x;
    if (d->tab_x < 4)
        d->tab_x = 4;
    if (d->tab_x > d->w - 4)
        d->tab_x = d->w - 4;
}
#endif

/* close hook: does the chosen action */
void func_08821868(void)
{
    DdActMenu *m = &dd_act_menu;
    DdActIcon *row;
    DdLoc loc;

    if (dd_act_menu.sel >= 0) {
        row = &dd_act_tab[m->act[dd_act_menu.sel]];
        m->src.get(&loc);
        func_088218CC(loc, row->act);
    }
}

/* action act at loc: the duel's actions, or 13 = card detail mode */
void func_088218CC(DdLoc loc, int act)
{
    if (act < 13)
        dd_act(loc.player, loc.zone, loc.idx, act);
    else if (act == 13)
        dd_mode_push(5);
}

/* draw hook: keeps the window over the card */
void *func_0882192C(void *list)
{
    func_088215A0();
    return list;
}

#ifdef NON_MATCHING /* alocação de registradores ou forma não reproduzida */
/* row i: icon and name */
void func_08821954(unsigned int *dl, unsigned short i, int x, int y)
{
    int *a = &dd_act_menu.act[i];
    DdActIcon *row = &dd_act_tab[*a];
    unsigned short *text;

    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 1);
    dl = func_088212C4(dl, x, y + 1, *a);
    font_set_size(12, 12);
    text = func_08821C90(row->text);
    font_text_set(dl);
    font_set_color(0xFF000000);
    font_draw_text((x + 15) << 6, (y + 1) << 6, (480 - (x + 15)) << 6, text);
    font_text_clear();
}
#endif

#ifdef NON_MATCHING /* alocação de registradores ou forma não reproduzida */
/* allowed actions of a location; a visible card also becomes the detail card */
unsigned int func_08821A3C(DdLoc loc)
{
    int idx = loc.idx;
    int zone = loc.zone;
    int player = loc.player;
    unsigned int bits = dv_08835AEC(player, zone, idx);
    int uid;

    if (zone == 12) {
        uid = dv_zone_c_uid(player, 0);
        loc.idx = 0;
    } else if (zone == 13) {
        uid = dv_zone_d_uid(player, 0);
        loc.idx = 0;
    } else if (zone == 15) {
        uid = 0;
    } else if (zone == 14) {
        uid = 0;
    } else if (zone == 11) {
        uid = dv_hand_uid(player, idx);
    } else {
        uid = dv_slot_uid(player, zone);
    }
    if (uid > 0 && dd_loc_visible(loc)) {
        bits |= 0x2000;
        dd_mode_set_card(dv_list_id(uid));
    }
    return bits;
}
#endif

/* window update; R jumps to the attack row (when attacking is allowed) */
int func_08821BAC(void)
{
    DdActMenu *m = &dd_act_menu;
    unsigned int *pad = dd_pad();
    int k;
    int i;

    if (dd_input_on()) {
        if ((m->bits & 0x1000) && (pad[1] & 0x200) && m->w->state == 3) {
            /* k stays unset when there is no attack row (as in the original) */
            for (i = 0; i < m->n; i++) {
                if (m->act[i] == 12) {
                    k = i;
                    break;
                }
            }
            menu_set_cursor(m->w, k);
            se_play(0x6213);
        }
        m->sel = menu_update(m->w);
    }
    if (!func_08825728())
        return 1;
    return 0;
}

/* name of an action; "activate" texts fall back to text 3 unless the card allows both */
unsigned short *func_08821C90(int text)
{
    DdActMenu *m = &dd_act_menu;

    if (text == 0x41 && (m->bits & 0xC0) != 0xC0)
        text = 3;
    if (text == 0x40 && card_is_monster(m->card) == 0 && (m->bits & 0xC0) != 0xC0)
        text = 3;
    return dd_text(text);
}

/* battle position choice among mask (6 icons) */
void func_08821D20(unsigned short *text, unsigned int mask)
{
    memset(&dd_pos_sel, 0, sizeof(DdPosSel));
    dd_pos_sel.mask = mask;
    func_08825824(text, text, 0x30, func_08821DBC);
    dd_mode_push(4);
    dd_mode_set_flag(0x31);
    func_088259FC(func_08821F70);
    func_088259E4(func_08821FA0);
    func_088259F0(func_08821F38);
    se_play(0x6216);
}

#ifdef NON_MATCHING /* alocação de registradores ou forma não reproduzida */
void func_08821DBC(MenuWidget *w, int open)
{
    DdPosSel *p = &dd_pos_sel;
    unsigned short *t;
    int tw;
    int i;

    if (open) {
        for (i = 0; i <= 5; i++) {
            if (p->mask & (1 << i))
                break;
        }
        w->cancel_blocked = 0;
        w->cancel_resets = 1;
        w->initial = i;
        w->count = 6;
        w->rows = 6;
        w->disabled = ~p->mask;
        w->skip_disabled = 1;
        w->item_w = 40;
        w->item_h = 40;
        w->draw_item = (MenuDrawItemFn)func_08821FA8;
        w->item_x = 5;
        w->cursor_w = 30;
        w->cursor_h = 40;
        p->w = w;
    } else {
        font_set_size(w->def.font_size, w->def.font_size);
        tw = 0;
        for (i = 0; i < 5; i++) {
            t = dd_text(i + 14);
            if (t)
                font_measure_text(t);  /* the width is not kept (tw stays 0) */
        }
        if (w->def.w < tw + (w->def.pad_l + w->def.pad_r + 8))
            w->def.w = tw + (w->def.pad_l + w->def.pad_r + 8);
        func_08821F38(w, 0, w->scroll + w->cursor);
    }
}
#endif

/* cursor hook: the name of the position as the window text */
void func_08821F38(MenuWidget *w, int old, int i)
{
    func_08825AFC(dd_text(i + 0xE));
    dd_pos_sel.cur = i;
}

void func_08821F70(void)
{
    int r = func_08825A14();

    if (r >= 0)
        dv_w624_set(r);
}

void *func_08821FA0(void *list)
{
    return list;
}

#ifdef NON_MATCHING /* alocação de registradores ou forma não reproduzida */
/* position icon i (dimmed when not offered); the arrow over the initial one blinks */
unsigned int *func_08821FA8(unsigned int *dl, unsigned short i, int x, int y)
{
    unsigned int on = dd_pos_sel.mask & (1 << i);
    void *gim;
    int v;
    int blink;

    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 1);
    gim = dd_gim_get(0x40025);
    if (gim)
        dl = gim_set_texture(dl, gim);
    if (on)
        v = 0;
    else
        v = 32;
    dl = draw_set_offsets(dl, 20, 32, 20, 32);
    dl = draw_sprite(dl, x + 10, y + 4, 0, i * 20, v, 0, 0);
    if (i > 0) {
        if (i - 1 == dd_pos_sel.w->initial)
            blink = *(unsigned int *)gfx_get_state() / 15 >> 3 & 1;
        else
            blink = 1;
        dl = draw_set_offsets(dl, 7, 11, 7, 11);
        dl = draw_sprite(dl, x - 3, y + 15, 0, 120, blink << 5, 0, 0);
    }
    return dl;
}
#endif

/* chain confirmation (yes/no) showing the cards involved */
void func_0882214C(unsigned short *text)
{
    memset(&dd_chain_sel, 0, sizeof(DdChainSel));
    func_08822520();
    func_08825824(0, text, 3, func_088221C4);
    dd_mode_push(4);
    dd_mode_set_flag(0x31);
    func_088259FC(func_08822240);
    se_play(0x621A);
}

void func_088221C4(MenuWidget *w, int open)
{
    DdChainSel *c = &dd_chain_sel;

    if (open) {
        c->w = w;
        w->cancel_blocked = 0;
        w->cancel_resets = 1;
        w->initial = 3;
        w->draw_before = (MenuDrawRectFn)func_0882228C;
        if (c->n > 0)
            w->def.pad_b = 40;
        else
            w->def.pad_b = 2;
    }
}

/* answer: yes (1 or 2) */
void func_08822240(void)
{
    int v;

    switch (func_08825A14()) {
    case 3:
    default:
        v = 0;
        break;
    case 1:
    case 2:
        v = 1;
        break;
    }
    dv_w624_set(v);
}

#ifdef NON_MATCHING /* alocação de registradores ou forma não reproduzida */
/* the cards of the chain in a row under the text ("..." when they don't fit) */
void func_0882228C(unsigned int *dl)
{
    DdChainSel *c = &dd_chain_sel;
    MenuWidget *w = dd_chain_sel.w;
    int cx = w->def.x + w->def.w / 2;
    int fit = (w->def.w - 9) / 22;
    int by = w->def.y + w->def.h - 25;
    int more = dd_chain_sel.more;
    int n = dd_chain_sel.n;
    int cnt;
    int i;
    int x;

    if (fit < n) {
        n = fit;
        more = 1;
    }
    dl = dl_enable(dl, 9);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_disable(dl, 4);
    if (more)
        cnt = n - 1;
    else
        cnt = n;
    for (i = 0; i < cnt; i++) {
        dl = dd_card_texture(dl, dv_list_id(c->uid[i]));
        dl = draw_set_offsets(dl, 64, 64, 20, 30);
        dl = draw_sprite(dl, 1.0f + ((float)cx + 22.0f * ((float)i - (float)n / 2.0f)), by - 15, 0, 0, 0, 0, 0);
    }
    dl = dl_enable(dl, 4);
    if (more) {
        unsigned short dots[4] = { 0x30FB, 0x30FB, 0x30FB, 0 };

        x = 1.0f + ((float)cx + 22.0f * ((float)i - (float)n / 2.0f));
        font_text_set(dl);
        font_set_size(6, 12);
        font_set_color(0xFF000000);
        font_draw_text(x << 6, (by - 6) << 6, (480 - x) << 6, dots);
        font_text_clear();
    }
}
#endif

#ifdef NON_MATCHING /* alocação de registradores ou forma não reproduzida */
/* collects the cards the current chain link involves (16 at most) */
void func_08822520(void)
{
    DdChainSel *c = &dd_chain_sel;
    char *chain;
    void *link;
    int p, z, i, n;

    dd_chain_sel.n = 0;
    dd_chain_sel.more = 0;
    chain = dv_chain();
    if (*(int *)(chain + 0x484) != 0)
        link = chain + (*(int *)(chain + 0x484) - 1) * 0x18 + 0x300;
    else
        link = 0;
    for (p = 0; p < 2; p++) {
        for (z = 0; z < 11; z++) {
            if (dv_088695B8(0, p, z, link)) {
                c->uid[c->n] = dv_slot_uid(p, z);
                c->n++;
                if (c->n >= 16) {
                    c->more = 1;
                    return;
                }
            }
        }
    }
    n = dv_n_hand(0);
    for (i = 0; i < n; i++) {
        if (dv_08869728(0, i, link)) {
            c->uid[c->n] = dv_hand_uid(0, i);
            c->n++;
            if (c->n >= 16) {
                c->more = 1;
                return;
            }
        }
    }
    if (dv_088BFFD8(0, 0xFFFE, 14))
        c->more = 1;
}
#endif

/* yes/no (with_no) or OK question; def_no opens on "no" */
void func_088226C0(unsigned short *text, int with_no, int def_no)
{
    dd_yesno_default = def_no;
    if (with_no)
        func_08825824(0, text, 3, func_08822740);
    else
        func_08825824(0, text, 1, func_08822740);
    dd_mode_push(4);
    dd_mode_set_flag(0x31);
    func_088259FC(func_088227EC);
    se_play(0x621A);
}

void func_08822740(MenuWidget *w, int open)
{
    MenuItemDef *d = &w->def;

    if (open) {
        w->cancel_blocked = 0;
        w->cancel_resets = 1;
        if (w->buttons == 1)
            w->initial = 1;
        else
            w->initial = 3;
        d->pad_b = 0;
    } else if (dd_yesno_default) {
        w->cursor = 1;
        w->button = 3;
    }
}

void func_088227EC(void)
{
    int v;

    switch (func_08825A14()) {
    case 3:
    default:
        v = 0;
        break;
    case 1:
    case 2:
        v = 1;
        break;
    }
    dv_w624_set(v);
}

/* "use the card?" between the card face and its back (flag 0: back shown as no card) */
void func_08822838(unsigned short *text, unsigned short card, int flag)
{
    dd_sel_card = dv_list_id(card);
    dd_sel_flag = flag;
    func_08825824(0, text, 0x30, func_088228B8);
    dd_mode_push(4);
    dd_mode_set_flag(0x31);
    func_088259FC(func_088228F0);
    se_play(0x621A);
}

void func_088228B8(MenuWidget *w, int open)
{
    if (open == 0)
        return;
    w->count = 2;
    w->rows = 2;
    w->item_w = 0x50;
    w->item_h = 0x5A;
    w->draw_item = (MenuDrawItemFn)func_08822914;
}

void func_088228F0(void)
{
    dv_w624_set(func_08825A14() != 0);
}

#ifdef NON_MATCHING /* alocação de registradores ou forma não reproduzida */
/* choice i: the card upright (0) or sideways (1) with its caption */
unsigned int *func_08822914(unsigned int *dl, unsigned short i, int x, int y)
{
    int flip, dx, dy, tw, th, msg;
    int flags;
    unsigned short *text;
    unsigned int m;
    int tx, ty;

    if (i == 0) {
        flip = 0;
        dx = 25;
        dy = 0;
        tw = 31;
        th = 44;
        msg = 23;
    } else {
        flip = 1;
        dx = 18;
        dy = 7;
        tw = 44;
        th = 31;
        msg = 24;
    }
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 1);
    dl = dd_card_texture(dl, (i != 0 && dd_sel_flag == 0) ? 0 : dd_sel_card);
    dl = draw_set_offsets(dl, 64, 64, tw, th);
    dl = draw_sprite(dl, x + dx, y + 12 + dy, 0, 0, 0, 0, flip);
    flags = font_get_flags();
    font_text_set(dl);
    font_set_size(12, 12);
    if (cfg_get_language())
        font_set_flag8(1);
    text = dd_text(msg);
    m = font_measure_wtext2(0x1400, 0x600, text);
    tx = 40 - (int)(m >> 22) / 2;
    ty = -(int)((m & 0xFFFF) >> 6) / 2;
    font_set_color(0xFF000000);
    font_draw_wtext_ex((x + tx) << 6, (y + 74 + ty) << 6, 0x1400, 0x600, text, -1);
    dl = font_text_clear();
    font_set_flags(flags);
    return dl;
}
#endif
