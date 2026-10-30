// @range 0x08809B1C-0x0880A8E0
// @asm 0x08809CFC-0x08809E14
// @asm 0x08809EF0-0x0880A8E0
/* The 3D card objects of the field: a pool of 460 nodes and one list per player and zone (deck,
   extra, graveyard...), filled from the decks at the start; the moves of a card between two
   places as the duel engine reports them. */
#include "dd_d1.h"

/* other ranges of libduel_draw */
void dd_cobjs_init(void);
void func_0880E45C(void);
void func_0880E4D4(void);
void func_0880E61C(void);
void func_0880E65C(void);
void func_0880E6C4(void);
void func_0880E788(void);
void func_0880EB70(void);
void func_0880F4D4(void);
void dd_hand_layout0(int a, int b);
void dd_hand_layout1(int a, int b);
void func_08811210(float dt);
void func_08815670(void);
void func_08815678(void);
void func_088157A0(void);
void func_088115CC(int player);
void func_0881E3B8(DdCobj *o);
void func_0881FAF8(DdCobj *o);
int func_0882092C(void);
int func_088667AC(void);
int func_088667BC(int uid);
int func_08866BC0(int player, int zone, int index);
int func_08866BF0(int player, int zone, int index);
float dd_card_drop_y(int n);
void req_flag_set(int i);
void func_08814DFC(void);

void dd_cobjs_setup(void)
{
    func_08814DFC();
    dd_cobjs_init();
}

/* card objects of the main deck (zone 13) and side deck (zone 12) of player p */
void dd_cobjs_add_deck(int p, RecipeData *deck)
{
    DdCobjs *b = &dd_cobjs;
    DdCobj *at;
    unsigned int k;
    DdCobj *o;

    if (deck == 0)
        return;
    at = &b->zone[p][13];
    for (k = 0; k < deck->n_main; k++) {
        o = dd_cobj_alloc();
        o->card = deck->main_ids[k];
        o->home.player = 0;
        o->home.zone = 13;
        o->home.idx = 0;
        dd_list_add_tail(o, at);
        at = o;
    }
    at = &b->zone[p][12];
    for (k = 0; k < deck->n_side; k++) {
        o = dd_cobj_alloc();
        o->card = deck->side_ids[k];
        o->home.player = 0;
        o->home.zone = 12;
        o->home.idx = 0;
        dd_list_add_tail(o, at);
        at = o;
    }
}

void dd_cobjs_clear(void)
{
    memset(&dd_cobjs, 0, sizeof(DdCobjs));
}

#ifdef NON_MATCHING /* o original endereça map pelo símbolo e as listas por base+deslocamento; ordem dos cálculos diferente */
/* deck of player `deck` shown on screen side `side` */
void dd_cobjs_map(int side, int deck)
{
    DdCobj *head;
    DdCobj *o;

    dd_cobjs.map[side] = deck;
    head = &dd_cobjs.zone[deck][13];
    for (o = head->next; o != head; o = o->next)
        o->home.player = side;
    head = &dd_cobjs.zone[deck][12];
    for (o = head->next; o != head; o = o->next)
        o->home.player = side;
    head = &dd_cobjs.zone[deck][11];
    for (o = head->next; o != head; o = o->next)
        o->home.player = side;
}
#endif

#ifdef NON_MATCHING /* ordem do lui/sll no cálculo do endereço */
int dd_cobjs_map_get(int side)
{
    DdCobjs *b = &dd_cobjs;

    return b->map[side != 0];
}
#endif

int dd_cobjs_map_other(int side)
{
    int v = dd_cobjs_map_get(side);

    return v / 2 != 0 ? v - 2 : v + 2;
}

/* draws the card objects (flag 2 of the scene) */
void dd_cobjs_draw(void)
{
    func_0880E45C();
    func_0880E4D4();
    func_0880E65C();
    func_0880E6C4();
    func_0880E788();
    dd_hand_layout0(dd_cobjs.x168f0, 0);
    dd_hand_layout1(dd_cobjs.x168f0, 0);
    dd_cobjs.x168f0 = 0;
    func_0880EB70();
    if (func_088667AC() == 3 && !func_0882092C())
        func_0880F4D4();
    func_08815678();
    func_0880E61C();
}

#ifdef NON_MATCHING /* o laço de zonas mantém o teste no topo a cada volta do laço externo */
/* advances every card object */
void dd_cobjs_update(float dt)
{
    int p;
    int z;
    DdCobj *head;
    DdCobj *o;
    DdCobj *next;

    func_08815670();
    for (p = 0; p < 2; p++) {
        for (z = 0; z < 16; z++) {
            head = dd_zone_list(p, z);
            for (o = head->next; o != head; o = next) {
                next = o->next;
                dd_cobj_tick(o, dt);
            }
        }
    }
    func_08811210(dt);
}
#endif

#ifdef NON_MATCHING /* funções grandes com campos de bits; forma não reproduzida */
extern DdPos dd_pos_up;                /* 0x08874BA0 templates {up = 1} */
extern DdPos dd_pos_up2;
/* moves the object of card `uid` from `from` to `to` (kind: how it moves) */
void dd_cobj_goto(DdPos from, DdPos to, int uid, int kind)
{
    DdCobj *o;
    DdCobj *head;
    DdXyzw pos;
    DdXyzw rot;

    func_088667BC(uid);
    switch (to.zone) {
    case 0:
        if (to.idx < 5)
            to.zone += to.idx;
        to.idx = 0;
        break;
    case 5:
        break;
    }
    switch (from.zone) {
    case 0:
        if (from.idx < 5)
            from.zone += from.idx;
        from.idx = 0;
        break;
    case 5:
        break;
    }
    o = dd_cobj_at(to.player, to.zone, 0);
    if (o == 0)
        return;
    if (uid >= 0) {
        head = o->prev;
        for (; o != head && o->uid != uid; o = o->next)
            ;
        if (o == head)
            o = o->next;
    }
    if (o->uid == 0)
        o->uid = uid;
    if (o->t < o->dur) {
        if (!o->b7 && !o->c0) {
            o->dest = from;
            goto later;
        }
    }
    if (o->a1 || o->a6 || o->a7 || o->b0) {
        o->dest = from;
    later:
        o->dest_kind = kind;
        o->a0 = 1;
        req_flag_set(0);
        return;
    }
    if (!o->b5 && to.zone - 12 < 4) {
        dd_slot_xform(&pos, &rot, to);
        o->pos = pos;
        o->rot = rot;
    }
    dd_list_unlink(o, dd_zone_list(to.player, to.zone));
    dd_list_add_tail(o, dd_zone_list(from.player, from.zone));
    *(DdPos *)&o->home = from;
    o->loc = from;
    o->b5 = 0;
    dd_slot_xform(&pos, &rot, from);
    if (from.zone == 11 && from.f14)
        o->b3 = 1;
    else
        o->b3 = 0;
    if (kind != 7 && kind != 5) {
        if (kind != 8 && !o->b4) {
            dd_cobj_move(dd_move_time(), o, &pos, &rot, 1);
            goto done;
        }
        o->b4 = 0;
    }
    dd_cobj_move(0.0001f, o, &pos, &rot, 0);
    dd_cobj_tick(o, 0.0001f);
    func_0881FAF8(o);
done:
    o->a2 = 0;
    func_088157A0();
    if (func_088667AC() == 3 && kind != 6 && from.zone < 5)
        func_088115CC(from.player);
}

/* turns the card `zone` of player p face up/down in place */
void dd_cobj_flip(int p, int zone)
{
    DdCobj *o = dd_cobj_find(p, zone, func_08866BF0(p, zone, 0));
    DdPos at;
    DdPos to;
    DdXyzw pos;
    DdXyzw rot;

    if (o == 0)
        return;
    dd_list_unlink(o, dd_zone_list(p, zone));
    dd_list_add_head(o, dd_zone_list(p, zone));
    if (func_08866BC0(p, zone, 0))
        return;
    memset(&at, 0, sizeof(at));
    at.player = p;
    at.zone = zone;
    at.f14 = o->b5;
    dd_slot_xform(&o->pos, &o->rot, at);
    memset(&to, 0, sizeof(to));
    to.player = p;
    to.zone = zone;
    to.f14 = !o->b5;
    dd_slot_xform(&pos, &rot, to);
    dd_cobj_move(dd_move_time(), o, &pos, &rot, 1);
    o->b2 = 1;
    o->a2 = 0;
    o->b5 = !o->b5;
}

/* puts the card `uid` on top of the pile p/zone */
void dd_cobj_to_top(int p, int zone, int uid)
{
    int n = dd_zone_count(p, zone);
    DdCobj *o = dd_cobj_at(p, zone, n - 1);
    DdPos at;
    DdPos to;
    DdXyzw pos;
    DdXyzw rot;

    if (o == 0)
        return;
    if (o->uid == uid) {
        func_0881E3B8(o);
        return;
    }
    for (; o != o->next && o->uid != uid; o = o->prev)
        ;
    if (o == o->next)
        return;
    dd_list_unlink(o, dd_zone_list(p, zone));
    dd_list_add_tail(o, dd_zone_list(p, zone));
    at = dd_pos_up;
    at.player = p;
    at.zone = zone;
    dd_slot_xform(&o->pos, &o->rot, at);
    to = dd_pos_up2;
    to.player = p;
    to.zone = zone;
    to.idx = n;
    dd_slot_xform(&pos, &rot, to);
    pos.y += dd_card_drop_y(1);
    o->pos.y = 0.0f;
    dd_cobj_move(dd_move_time(), o, &pos, &rot, 1);
    o->b1 = 1;
    o->c1 = 1;
    o->a2 = 1;
}
#endif
