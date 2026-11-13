// @range 0x0880D31C-0x0880E45C
// @data 0x08874E24-0x08874EA0
/* Card objects: the setup of the pool and of the zone lists, and the layout of the two hands
   (cards fanned towards the camera, the one under the cursor raised, turned over when hidden). */
#include "dd_d1.h"

int memcmp2(const void *a, const void *b, int n);
float acosf(float x);
DdXyzw *vec4_add(DdXyzw *out, DdXyzw *a, DdXyzw *b);
DdXyzw *vec4_scale(DdXyzw *out, DdXyzw *v, float s);
float vec3_dot2(DdXyzw *a, DdXyzw *b);
DdXyzw *vec3_cross(DdXyzw *out, DdXyzw *a, DdXyzw *b);
DdXyzw *quat_mul(DdXyzw *out, DdXyzw *a, DdXyzw *b);
DdXyzw *quat_from_axis_angle(DdXyzw *out, DdXyzw *axis, float angle);
void mtx_get_view_dir(DdXyzw *out);
void dd_cursor_get(DdLoc *cur);

/* empties the pool, the zone lists and the free list */
void dd_cobjs_init(void)
{
    DdCobjs *b = &dd_cobjs;
    int p;
    int z;
    DdCobj *o;

    memset(b, 0, sizeof(DdCobjs));
    dd_cobjs.en[0] = 1;
    dd_cobjs.en[1] = 1;
    dd_cobjs.free.prev = &dd_cobjs.free;
    dd_cobjs.free.next = &dd_cobjs.free;
    for (p = 0; p < 4; p++) {
        for (z = 0; z < 16; z++) {
            b->zone[p][z].prev = &b->zone[p][z];
            b->zone[p][z].next = &b->zone[p][z];
        }
    }
    for (p = 0; p < 460; p++) {
        o = &b->obj[p];
        o->next = b->free.next;
        o->prev = &b->free;
        o->prev->next = o;
        o->next->prev = o;
        o->x94 = -1;
        o->x90 = -1;
    }
}

void dd_hand_layout0(int force, int instant)
{
    DdXyzw base;
    DdXyzw base_rot;
    DdXyzw start;
    DdXyzw step;
    DdXyzw pos;
    DdXyzw rot;
    int n;
    DdCobj *o;
    int i;
    float w = 65.0f;

    {
        DdPos at = { 0, 11 };
        DdXyzw tilt = { 0.0f, 0.0f, 1.0f, 0.0f };
        DdXyzw q;
        DdXyzw down = { 0.0f, -1.0f, 0.0f, 0.0f };
        DdXyzw view;
        DdXyzw axis;

        dd_slot_xform(&base, &base_rot, at);
        quat_mul(&base_rot, &tilt, &base_rot);
        mtx_get_view_dir(&view);
        vec3_cross(&axis, &view, &down);
        quat_from_axis_angle(&q, &axis, acosf(vec3_dot2(&view, &down)));
        quat_mul(&base_rot, &q, &base_rot);
    }
    n = dd_zone_count(0, 11);
    if (n < 8) {
        if (n & 1) {
            start = base;
            start.x -= (float)(n / 2 * 65);
            step = (DdXyzw){ w };
        } else {
            start = base;
            start.x -= (float)(n / 2 * 65 - 32);
            step = (DdXyzw){ w };
        }
    } else {
        start = base;
        start.x -= 200.0f;
        step = (DdXyzw){ 400.0f / (float)(n - 1) };
    }
    o = dd_zone_list(0, 11)->next;
    for (i = 0; i < n; i++, o = o->next) {
        if (o->a5) {
            pos = base;
            rot = base_rot;
        } else {
            DdLoc cur;
            DdLoc t;

            vec4_scale(&pos, &step, (float)i);
            vec4_add(&pos, &pos, &start);
            rot = base_rot;
            dd_cursor_get(&t);
            *(short *)&cur = *(short *)&t;
            if (cur.player == 0 && cur.zone == 11 && cur.idx == i) {
                pos.z -= 10.0f;
                pos.y += 15.0f;
            }
        }
        if (o->a6 || o->a7 || (o->b3 && o->home.player == 1)) {
            DdXyzw q;
            DdXyzw zaxis = { 0.0f, 0.0f, 1.0f, 0.0f };

            quat_from_axis_angle(&q, &zaxis, 3.14159274f);
            quat_mul(&rot, &rot, &q);
        }
        if (instant) {
            o->pos = pos;
            o->pos1 = pos;
            o->rot = rot;
            o->rot1 = rot;
            o->t = 0.0f;
            o->dur = 0.0f;
        } else if (force || memcmp2(&o->pos1, &pos, 16) || memcmp2(&o->rot1, &rot, 16)) {
            float dur = dd_move_time();
            int quick = 0;
            int snd = 0;

            if (force == 0 && memcmp2(&o->rot1, &rot, 16) == 0 && o->pos.x == pos.x) {
                dur = 0.1f;
                quick = 1;
            }
            if (o->a6 || o->a7 || o->b0)
                quick = 1;
            if (memcmp2(&o->rot1, &rot, 16) || o->a5)
                snd = 1;
            dd_cobj_move(dur, o, &pos, &rot, snd);
            o->b6 = (signed char)quick;
            if (memcmp2(&o->rot1, &rot, 16) == 0 && memcmp2(&o->rot1, &o->rot, 16) == 0)
                o->c0 = 1;
        }
    }
}

void dd_hand_layout1(int force, int instant)
{
    DdXyzw base;
    DdXyzw base_rot;
    DdXyzw start;
    DdXyzw step;
    DdXyzw pos;
    DdXyzw rot;
    int n;
    DdCobj *o;
    int i;
    float w = 70.0f;

    {
        DdPos at = { 1, 11 };
        DdXyzw tilt = { 0.0f, 1.0f, 0.0f, 0.0f };
        DdXyzw q;
        DdXyzw down = { 0.0f, -1.0f, 0.0f, 0.0f };
        DdXyzw view;
        DdXyzw axis;

        dd_slot_xform(&base, &base_rot, at);
        quat_mul(&base_rot, &tilt, &base_rot);
        mtx_get_view_dir(&view);
        vec3_cross(&axis, &view, &down);
        quat_from_axis_angle(&q, &axis, acosf(vec3_dot2(&view, &down)));
        quat_mul(&base_rot, &q, &base_rot);
    }
    n = dd_zone_count(1, 11);
    if (n < 8) {
        if (n & 1) {
            start = base;
            start.x += (float)(n / 2 * 70);
            step = (DdXyzw){ w };
        } else {
            start = base;
            start.x += (float)(n / 2 * 70 - 35);
            step = (DdXyzw){ w };
        }
    } else {
        start = base;
        start.x += 210.0f;
        step = (DdXyzw){ 420.0f / (float)n };
    }
    o = dd_zone_list(1, 11)->next;
    for (i = 0; i < n; i++, o = o->next) {
        if (o->a5) {
            pos = base;
            rot = base_rot;
        } else {
            DdLoc cur;
            DdLoc t;

            vec4_scale(&pos, &step, -(float)i);
            vec4_add(&pos, &pos, &start);
            rot = base_rot;
            dd_cursor_get(&t);
            *(short *)&cur = *(short *)&t;
            if (cur.player == 1 && cur.zone == 11 && cur.idx == i)
                pos.y += 10.0f;
        }
        if (o->a6 || o->a7 || (o->b3 && o->home.player == 1)) {
            DdXyzw q;
            DdXyzw zaxis = { 0.0f, 0.0f, 1.0f, 0.0f };

            quat_from_axis_angle(&q, &zaxis, 3.14159274f);
            quat_mul(&rot, &rot, &q);
        }
        if (instant) {
            o->pos = pos;
            o->pos1 = pos;
            o->rot = rot;
            o->rot1 = rot;
            o->t = 0.0f;
            o->dur = 0.0f;
        } else if (force || memcmp2(&o->pos1, &pos, 16) || memcmp2(&o->rot1, &rot, 16)) {
            float dur = dd_move_time();
            int quick = 0;

            if (force == 0 && memcmp2(&o->rot1, &rot, 16) == 0 && o->pos.x == pos.x) {
                dur = 0.1f;
                quick = 1;
            }
            if (o->a6 || o->a7 || o->b0)
                quick = 1;
            memcmp2(&o->rot1, &rot, 16); /* the original compares and passes 0 anyway */
            dd_cobj_move(dur, o, &pos, &rot, 0);
            o->b6 = (signed char)quick;
            if (memcmp2(&o->rot1, &rot, 16) == 0 && memcmp2(&o->rot1, &o->rot, 16) == 0)
                o->c0 = 1;
        }
    }
}

