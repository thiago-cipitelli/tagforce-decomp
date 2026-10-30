// @range 0x0880FCFC-0x08810758
// @data 0x08874F70-0x08874F80
// @asm 0x0880FEF0-0x08810598
// @asm 0x088106F4-0x08810758
/* Card objects: the list primitives, the pool allocator, the start of a move (with the card
   slide sound) and the per-frame move with its arc, the end-of-move callbacks, the zone list
   lookup and the move duration of the animation speed option. */
#include "dd_d1.h"

unsigned int rand_next(void);
void se_play(int id);
int memcmp2(const void *a, const void *b, int n);
void vec3_lerp_w(DdXyzw *out, DdXyzw *a, DdXyzw *b, float t);
void quat_slerp(DdXyzw *out, DdXyzw *a, DdXyzw *b, float t);
unsigned int cfg_get_bits_1_3_4(void);
void req_flag_set(int i);
void dd_cobj_goto(DdPos from, DdPos to, int uid, int kind);
void dd_cobjs_dirty(void);
int func_088667AC(void);
void func_0881E3B8(DdCobj *o);
void func_0881E610(DdCobj *o);
void func_0881E7EC(DdCobj *o);
void func_0881F9B4(DdCobj *o);

int dd_list_unlink(DdCobj *o, DdCobj *list)
{
    o->prev->next = o->next;
    o->next->prev = o->prev;
    o->next = 0;
    o->prev = 0;
    return 1;
}

int dd_list_add_tail(DdCobj *o, DdCobj *list)
{
    o->prev = list->prev;
    o->next = list;
    o->prev->next = o;
    o->next->prev = o;
    return 1;
}

int dd_list_add_head(DdCobj *o, DdCobj *list)
{
    o->next = list->next;
    o->prev = list;
    list->next = o;
    o->next->prev = o;
    return 1;
}

/* takes a card object from the free list (0 when exhausted) */
DdCobj *dd_cobj_alloc(void)
{
    DdCobjs *b = &dd_cobjs;
    DdCobj *o;

    if (dd_cobjs.free.next == &dd_cobjs.free)
        return 0;
    o = b->free.next;
    o->prev->next = o->next;
    o->next->prev = o->prev;
    o->next = 0;
    o->prev = 0;
    return o;
}

/* starts a move to pos/rot lasting dur; the slide sound when something changes */
void dd_cobj_move(float dur, DdCobj *o, DdXyzw *pos, DdXyzw *rot, int sound)
{
    o->pos0 = o->pos;
    o->rot0 = o->rot;
    o->pos1 = *pos;
    o->rot1 = *rot;
    o->t = 0.0f;
    o->dur = dur;
    o->b6 = 0;
    o->c0 = 0;
    if (sound && (memcmp2(&o->pos0, &o->pos1, 16) || memcmp2(&o->rot0, &o->rot1, 16)))
        se_play((int)rand_next() % 5 + 0x623D);
}

void dd_cobj_done(DdCobj *o);

#ifdef NON_MATCHING /* máquina de estados grande / registrador do slti */
/* advances a card object by dt: the move, its arc, then the actions queued at its end */
void dd_cobj_tick(DdCobj *o, float dt)
{
    float t;
    float dur;
    float arc;
    int z;

    if (o->t >= o->dur) {
        if (o->a7) {
            if (o->t < dd_move_time() * 2.0f) {
                t = o->t;
                goto run;
            }
        }
        if (!o->a1)
            return;
        o->a1 = 0;
        z = o->home.zone;
        if (z == 15 || z == 14 || z == 12 || z == 13)
            dd_zone_sync(o->home.player, z);
        if (o->b0)
            o->b0 = 0;
        if (o->a7) {
            o->a7 = 0;
            o->b0 = 1;
            o->a1 = 1;
            o->t = 0.0f;
            if (o->b7) {
                o->b7 = 0;
                o->b0 = 0;
                o->dur = 0.0f;
                o->a1 = 0;
                func_0881F9B4(o);
            }
        }
        if (o->a6) {
            o->a6 = 0;
            o->a7 = 1;
            o->a1 = 1;
            o->t = 0.0f;
            dd_cobjs_dirty();
        }
        if (o->a0) {
            if (!o->a7 && !o->a6 && !o->b0) {
                req_flag_set(0);
                o->a0 = 0;
                dd_cobj_goto(o->dest, *(DdPos *)&o->home, o->uid, o->dest_kind);
                return;
            }
        }
        if (o->a2) {
            o->a2 = 0;
            func_0881E3B8(o);
        }
        if (o->a3) {
            o->a3 = 0;
            func_0881E610(o);
        }
        if (o->a4) {
            o->a4 = 0;
            func_0881E7EC(o);
        }
        if (o->a5) {
            o->a5 = 0;
            se_play((int)rand_next() % 5 + 0x623D);
        }
        return;
    }
    t = o->t;
run:
    o->t = t + dt;
    if (o->b6) {
        if (!o->c1)
            req_flag_set(0);
    } else if (o->t >= o->dur * 0.5f && !o->a0 && cfg_get_bits_1_3_4() > 1) {
        if (!o->c1)
            req_flag_set(0);
    } else {
        req_flag_set(0);
    }
    if (o->b7)
        req_flag_set(0x1D);
    if (o->a3)
        req_flag_set(0x33);
    if (o->a4)
        req_flag_set(0x34);
    t = o->t;
    if (t >= o->dur && !(o->a7 && t < dd_move_time() * 2.0f)) {
        o->pos = o->pos1;
        o->rot = o->rot1;
        o->b1 = 0;
        o->b2 = 0;
        o->a1 = 1;
        dd_cobj_done(o);
        return;
    }
    dur = o->dur;
    if (o->a5)
        dur *= 0.5f;
    if (t >= dur) {
        o->pos = o->pos1;
        o->rot = o->rot1;
    } else {
        vec3_lerp_w(&o->pos, &o->pos0, &o->pos1, t / dur);
        quat_slerp(&o->rot, &o->rot0, &o->rot1, o->t / dur);
        t = o->t;
        arc = t * -360.0f * t / dur / dur + t * 360.0f / dur;
        if (o->b1) {
            if (o->home.player == 0)
                o->pos.x -= arc;
            else
                o->pos.x += arc;
        }
        if (o->b2)
            o->pos.y += arc;
    }
    dd_cobj_done(o);
}
#endif

/* runs the end callbacks of a card object (cleared when they return non-zero) */
void dd_cobj_done(DdCobj *o)
{
    int i;
    DdCobj *c;

    for (i = 0, c = o; i < 4; i++, c = (DdCobj *)((int *)c + 1)) {
        if (c->link[0] && ((int (*)(DdCobj *))c->link[0])(o))
            c->link[0] = 0;
    }
}

/* head of the list of a zone; the decks of the tag partners for zones 11..13 */
DdCobj *dd_zone_list(int player, int zone)
{
    DdCobjs *b = &dd_cobjs;

    switch (zone) {
    case 13:
    case 11:
    case 12:
        return &b->zone[b->map[player]][zone];
    }
    return &b->zone[player][zone];
}

float dd_speed_moves[4] = { 0.5f, 0.375f, 0.25f, 0.1f };

/* duration of a card move for the animation speed option */
float dd_move_time(void)
{
    return dd_speed_moves[cfg_get_bits_1_3_4()];
}

#ifdef NON_MATCHING /* máquina de estados grande / registrador do slti */
/* 1 when the monster at `at` shows its counter (main phase, face up) */
int dd_cobj_counter_shown(DdCobj *o, DdPos at)
{
    int zone = at.zone;

    if (zone < 0 || !(zone < 5))
        return 0;
    if (at.f14 == 0)
        return 0;
    return func_088667AC() == 3;
}
#endif
