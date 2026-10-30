// @range 0x0880C48C-0x0880D31C
// @asm 0x0880C48C-0x0880C640
// @asm 0x0880C758-0x0880C9BC
// @asm 0x0880CB44-0x0880CBF4
// @asm 0x0880CC64-0x0880CD38
// @asm 0x0880CE60-0x0880D028
// @asm 0x0880D028-0x0880D0B4
// @asm 0x0880D16C-0x0880D31C
/* Card objects, continued: fusion gathering (the cards fly to the middle), state bits and the
   per-object effect links, the queries "is anything still moving", the stack height of a pile,
   the counters of the monster zones and the stack of highlighted locations. */
#include "dd_d1.h"

void dd_cobj_goto(DdPos from, DdPos to, int uid, int kind);
void vec3_lerp_w(DdXyzw *out, DdXyzw *a, DdXyzw *b, float t);
void req_flag_set(int i);
void dd_card_fx_start(DdCobj *o);
void dd_banner_summon(DdCobj *o);
void dd_banner_set(DdCobj *o);
void dd_counter_reset(int player, int zone);
int dv_list_id(int uid);
int dv_slot_uid(int player, int zone);
int dv_side_to_player(int side);
void dv_field_stats(int player, int zone, int *out);
char *dv_special(void);
unsigned int cfg_get_bits_1_3_4(void);
void dd_cobjs_dirty(void);

#ifdef NON_MATCHING /* alocação, laços com teste no topo ou cadeias de comparação não reproduzidos */
/* gathers the monsters of `mask` (zones 0..4 of player p) in the middle of them, lifted */
void dd_cobj_gather(int p, unsigned int mask)
{
    DdCobj *first = 0;
    DdCobj *last = 0;
    int z;
    DdCobj *o;
    DdXyzw rot;
    DdXyzw mid;

    for (z = 0; z < 5; z++) {
        if ((mask & 1 << z) && (o = dd_cobj_at(p, z, 0)) != 0) {
            o->uid = dv_slot_uid(p, z);
            o->card = dv_list_id(o->uid);
            last = o;
            if (first == 0)
                first = o;
        }
    }
    if (first == 0 || last == 0)
        return;
    rot = first->rot;
    vec3_lerp_w(&mid, &first->pos, &last->pos, 0.5f);
    mid.y += 30.0f;
    for (z = 0; z < 5; z++) {
        if ((mask & 1 << z) && (o = dd_cobj_at(p, z, 0)) != 0) {
            dd_cobj_move(dd_move_time() * 2.0f, o, &mid, &rot, 1);
            o->a5 = 1;
            dd_cobj_goto(o->loc, o->loc, -1, 0);
        }
    }
}
#endif

void dd_cobj_end_a2(DdCobj *o)
{
    if (o->t < o->dur)
        o->a2 = 1;
    else
        dd_card_fx_start(o);
}

void dd_cobj_end_a3(DdCobj *o)
{
    if (o->t < o->dur) {
        o->a3 = 1;
        req_flag_set(0x33);
    } else {
        dd_banner_summon(o);
    }
}

void dd_cobj_end_a4(DdCobj *o)
{
    if (o->t < o->dur) {
        o->a4 = 1;
        req_flag_set(0x34);
    } else {
        dd_banner_set(o);
    }
}

void dd_cobj_set_card(DdCobj *o, unsigned short card, unsigned short uid)
{
    o->uid = uid;
    o->card = card;
}

#ifdef NON_MATCHING /* alocação, laços com teste no topo ou cadeias de comparação não reproduzidos */
/* new card object at `at` (token...) */
void dd_cobj_spawn(DdPos at, unsigned short card, unsigned short uid)
{
    DdCobj *o = dd_cobj_alloc();
    DdXyzw rot;
    DdXyzw pos;

    if (o == 0)
        return;
    o->card = card;
    o->uid = uid;
    o->home.player = at.player;
    o->home.zone = at.zone;
    o->home.idx = at.idx;
    o->loc = at;
    dd_slot_xform(&pos, &rot, at);
    o->pos = pos;
    o->pos1 = pos;
    o->pos0 = pos;
    o->rot = rot;
    o->rot1 = rot;
    o->rot0 = rot;
    o->t = 0.0f;
    o->dur = 0.0f;
    dd_list_add_tail(o, dd_zone_list(at.player, at.zone));
}
#endif

#ifdef NON_MATCHING /* alocação, laços com teste no topo ou cadeias de comparação não reproduzidos */
/* gives the object at `at` back to the free list */
void dd_cobj_remove(DdPos at)
{
    DdCobjs *b = &dd_cobjs;
    int player = at.player;
    int zone = at.zone;
    DdCobj *o = dd_cobj_at(player, zone, at.idx);

    if (o == 0)
        return;
    dd_list_unlink(o, dd_zone_list(player, zone));
    dd_list_add_tail(o, &b->free);
}
#endif

/* resyncs the extra deck of player p */
void dd_cobj_sync_extra(int p)
{
    DdCobj *head = dd_zone_list(p, 11);
    DdCobj *o;

    for (o = head->next; o != head; o = o->next)
        o->a5 = 1;
    dd_zone_sync(p, 11);
    dd_cobjs_dirty();
}

void dd_cobj_hide(DdCobj *o)
{
    if (!o->b3) {
        o->a6 = 1;
        o->t = 0.0f;
    }
}

void dd_cobj_extra_up(int p, int up)
{
    DdCobj *head = dd_zone_list(p, 11);
    DdCobj *o;

    for (o = head->next; head != o; o = o->next)
        o->b3 = up;
}

/* 1 while the object has an action pending */
int dd_cobj_busy(DdCobj *o)
{
    if (o->a2 || o->a3 || o->a4 || o->a5 || o->a6 || o->a7 || o->b0)
        return 1;
    return 0;
}

#ifdef NON_MATCHING /* alocação, laços com teste no topo ou cadeias de comparação não reproduzidos */
/* 1 while any card object has an action pending */
int dd_cobjs_busy(void)
{
    int p;
    int z;
    DdCobj *head;
    DdCobj *o;

    for (p = 0; p < 2; p++) {
        for (z = 0; z < 16; z++) {
            head = dd_zone_list(p, z);
            for (o = head->next; o != head; o = o->next) {
                if (dd_cobj_busy(o))
                    return 1;
            }
        }
    }
    return 0;
}
#endif

void dd_cobjs_dirty(void)
{
    dd_cobjs.x168f0 = 1;
}

/* height of a pile of n cards */
float dd_stack_y(int n)
{
    if (n <= 0)
        return 0.0f;
    if (n >= 120)
        return 46.026f;
    return (float)n * 45.822002f / 120.0f + 0.204f;
}

#ifdef NON_MATCHING /* alocação, laços com teste no topo ou cadeias de comparação não reproduzidos */
/* 1 when no card object moves */
int dd_cobjs_idle(void)
{
    int p;
    int z;
    DdCobj *head;
    DdCobj *o;

    for (p = 0; p < 2; p++) {
        for (z = 0; z < 15; z++) {
            head = dd_zone_list(p, z);
            o = head->next;
            if (o != head && o != head) {
                do {
                    if (o->t < o->dur)
                        return 0;
                    if (o->a7 || o->b0 || o->a6)
                        return 0;
                    o = o->next;
                } while (o != head);
            }
        }
    }
    return 1;
}
#endif

/* 1 when no card object is busy with an effect */
int dd_cobjs_free(void)
{
    int p;
    int z;
    DdCobj *head;
    DdCobj *o;

    for (p = 0; p < 2; p++) {
        for (z = 0; z < 15; z++) {
            head = dd_zone_list(p, z);
            o = head->next;
            if (o != head && o != head) {
                do {
                    if (o->busy)
                        return 0;
                    o = o->next;
                } while (o != head);
            }
        }
    }
    return 1;
}

void dd_cobjs_sync_set(int on)
{
    dd_cobjs.sync = on;
}

void dd_cobj_link_add(DdCobj *o, int fx)
{
    int i;

    for (i = 0; i < 4; i++) {
        if (o->link[i] == 0) {
            o->link[i] = fx;
            return;
        }
    }
}

void dd_cobj_link_del(DdCobj *o, int fx)
{
    int i;

    for (i = 0; i < 4; i++) {
        if (fx == o->link[i]) {
            o->link[i] = 0;
            return;
        }
    }
}

void dd_cobjs_en_set(int i, int on)
{
    dd_cobjs.en[i] = on;
}

#ifdef NON_MATCHING /* alocação, laços com teste no topo ou cadeias de comparação não reproduzidos */
/* refreshes the counters of the monster zones */
void dd_cobjs_counters(void)
{
    int p;
    int z;

    for (p = 0; p < 2; p++)
        for (z = 0; z < 5; z++)
            dd_counter_reset(p, z);
}
#endif

#ifdef NON_MATCHING /* alocação, laços com teste no topo ou cadeias de comparação não reproduzidos */
/* starts the counter animation of the monster zones whose values changed */
void dd_zone_anim_update(int live)
{
    DdCobjs *b = &dd_cobjs;
    int p;
    int z;
    DdZoneAnim *za;
    char *e;
    int a;
    int bv;
    int st[11];

    for (p = 0; p < 2; p++) {
        for (z = 0; z < 5; z++) {
            za = &b->za[p][z];
            a = -1;
            if (live) {
                e = dv_special() + dv_side_to_player(p) * 0x38;
                if (*(int *)(e + 0x30) == z) {
                    a = *(int *)(e + 0x48);
                    bv = *(int *)(e + 0x4C);
                }
            }
            if (a < 0) {
                dv_field_stats(p, z, st);
                a = st[5];
                bv = st[6];
            }
            if (za->a != a || za->b != bv) {
                za->a0 = za->a;
                za->b0 = za->b;
                za->a1 = a;
                za->b1 = bv;
                za->t = 0.0f;
                za->dur = dd_move_time();
                if (cfg_get_bits_1_3_4() > 2) {
                    za->a = a;
                    za->b = bv;
                    za->t = za->dur;
                }
            }
        }
    }
}
#endif

/* 1 while a counter animation of a monster zone is running (wait callback of the event commands) */
#ifdef NON_MATCHING /* o original mantém o teste de entrada do laço interno e não reduz o índice z a ponteiro; 33 palavras */
int dd_zone_anim_busy(void)
{
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 0; z < 5; z++) {
            if (dd_cobjs.za[p][z].t < dd_cobjs.za[p][z].dur)
                return 1;
        }
    }
    return 0;
}
#endif

/* scale of a counter value v of kind k (digits) */
float dd_count_scale(int k, int v)
{
    float r = 0.0f;

    switch (v + k * 16) {
    case 5 ... 9:
        r = 1.0f;
        break;
    case 0 ... 4:
        r = 2.0f;
        break;
    case 0x10 ... 0x100:
        r = 4.0f;
        break;
    case 0x200 ... 0x2000:
        r = 6.0f;
        break;
    case 0x8000:
        r = 1.0f;
        break;
    }
    return r;
}

#ifdef NON_MATCHING /* alocação, laços com teste no topo ou cadeias de comparação não reproduzidos */
/* pushes location (p, zone) on the highlight stack with value v */
void dd_hl_push(int p, unsigned int zone, int v)
{
    DdLoc l = { 0 };

    dd_cobjs.sel[p][zone] = v;
    l.player = p;
    l.zone = zone;
    dd_cobjs.stack[dd_cobjs.nstack] = l;
    dd_cobjs.nstack++;
}
#endif

#ifdef NON_MATCHING /* alocação, laços com teste no topo ou cadeias de comparação não reproduzidos */
/* pops it again; the location keeps the depth of its previous entry */
void dd_hl_pop(unsigned int p, unsigned int zone)
{
    DdCobjs *b = &dd_cobjs;
    DdLoc l = { 0 };
    int i;
    DdLoc *s;

    dd_cobjs.nstack--;
    dd_cobjs.stack[dd_cobjs.nstack] = l;
    dd_cobjs.sel[p][zone] = 0;
    for (i = dd_cobjs.nstack - 1; i >= 0; i--) {
        s = &b->stack[i];
        if (s->player == p && s->zone == zone) {
            dd_cobjs.sel[p][zone] = i + 1;
            return;
        }
    }
}
#endif
