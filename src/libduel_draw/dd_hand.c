// @range 0x0880D31C-0x0880E45C
// @asm 0x0880D31C-0x0880E45C
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

#ifdef NON_MATCHING /* laço desenrolado com alocação de registradores diferente */
/* empties the pool, the zone lists and the free list */
void dd_cobjs_init(void)
{
    DdCobjs *b = &dd_cobjs;
    int p;
    int z;
    int i;
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
    for (i = 0; i < 460; i++) {
        o = &b->obj[i];
        o->next = b->free.next;
        o->prev = &b->free;
        b->free.next = o;
        o->next->prev = o;
        o->x94 = -1;
        o->x90 = -1;
    }
}
#endif

#ifdef NON_MATCHING /* funções grandes (layout das mãos); forma não reproduzida */
extern DdPos dd_hand_pos0;             /* 0x08874E24 zone 11 of player 0 */
extern DdXyzw dd_hand_tilt0;           /* 0x08874E30 */
extern DdXyzw dd_hand_down0;           /* 0x08874E40 */
extern DdXyzw dd_hand_zaxis0;          /* 0x08874E50 */
extern DdPos dd_hand_pos1;             /* 0x08874E60 zone 11 of player 1 */
extern DdXyzw dd_hand_tilt1;           /* 0x08874E70 */
extern DdXyzw dd_hand_down1;           /* 0x08874E80 */
extern DdXyzw dd_hand_zaxis1;          /* 0x08874E90 */

/* rotation facing the camera from the hand slot */
#define HAND_BASE(pos, rot, at, tilt, down)                                 \
    {                                                                       \
        DdXyzw view;                                                        \
        DdXyzw axis;                                                        \
        DdXyzw q;                                                           \
        dd_slot_xform(&pos, &rot, at);                                      \
        quat_mul(&rot, &tilt, &rot);                                        \
        mtx_get_view_dir(&view);                                            \
        vec3_cross(&axis, &view, &down);                                    \
        quat_from_axis_angle(&q, &axis, acosf(vec3_dot2(&view, &down)));    \
        quat_mul(&rot, &q, &rot);                                           \
    }

/* moves the cards of the hand of player `p` to their place (instant: no animation) */
#define HAND_PLACE(o, pos, rot, i, force, instant, zaxis)                                        \
    {                                                                                            \
        if (o->a6 || o->a7 || (o->b3 && o->home.player == 1)) {                                  \
            DdXyzw q;                                                                            \
            quat_from_axis_angle(&q, &zaxis, 3.14159274f);                                       \
            quat_mul(&rot, &rot, &q);                                                            \
        }                                                                                        \
        if (instant == 0) {                                                                      \
            if (force || memcmp2(&o->pos1, &pos, 16) || memcmp2(&o->rot1, &rot, 16)) {           \
                float dur = dd_move_time();                                                      \
                int quick = 0;                                                                   \
                int snd = 0;                                                                     \
                if (force == 0 && memcmp2(&o->rot1, &rot, 16) == 0) {                            \
                    if (o->pos.x == pos.x) {                                                     \
                        dur = 0.1f;                                                              \
                        quick = 1;                                                               \
                    }                                                                            \
                }                                                                                \
                if (o->a6 || o->a7 || o->b0)                                                     \
                    quick = 1;                                                                   \
                if (memcmp2(&o->rot1, &rot, 16) || o->a5)                                        \
                    snd = 1;                                                                     \
                dd_cobj_move(dur, o, &pos, &rot, snd);                                           \
                o->b6 = quick;                                                                   \
                if (memcmp2(&o->rot1, &rot, 16) == 0 && memcmp2(&o->rot1, &o->rot, 16) == 0)     \
                    o->c0 = 1;                                                                   \
            }                                                                                    \
        } else {                                                                                 \
            o->pos = pos;                                                                        \
            o->pos1 = pos;                                                                       \
            o->rot = rot;                                                                        \
            o->rot1 = rot;                                                                       \
            o->t = 0.0f;                                                                         \
            o->dur = 0.0f;                                                                       \
        }                                                                                        \
    }

void dd_hand_layout0(int force, int instant)
{
    DdXyzw base;
    DdXyzw base_rot;
    DdXyzw start;
    DdXyzw step;
    DdXyzw pos;
    DdXyzw rot;
    DdLoc cur;
    int n;
    int i;
    DdCobj *o;

    HAND_BASE(base, base_rot, dd_hand_pos0, dd_hand_tilt0, dd_hand_down0)
    n = dd_zone_count(0, 11);
    start = base;
    memset(&step, 0, sizeof(step));
    if (n < 8) {
        if ((n & 1) == 0)
            start.x = base.x - (float)(n / 2 * 65 - 32);
        else
            start.x = base.x - (float)(n / 2 * 65);
        step.x = 65.0f;
    } else {
        start.x = base.x - 200.0f;
        step.x = 400.0f / (float)(n - 1);
    }
    o = dd_zone_list(0, 11)->next;
    for (i = 0; i < n; i++, o = o->next) {
        if (o->a5) {
            pos = base;
            rot = base_rot;
        } else {
            vec4_scale(&pos, &step, (float)i);
            vec4_add(&pos, &pos, &start);
            rot = base_rot;
            dd_cursor_get(&cur);
            if (cur.player == 0 && cur.zone == 11 && cur.idx == i) {
                pos.z -= 10.0f;
                pos.y += 15.0f;
            }
        }
        HAND_PLACE(o, pos, rot, i, force, instant, dd_hand_zaxis0)
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
    DdLoc cur;
    int n;
    int i;
    DdCobj *o;

    HAND_BASE(base, base_rot, dd_hand_pos1, dd_hand_tilt1, dd_hand_down1)
    n = dd_zone_count(1, 11);
    start = base;
    memset(&step, 0, sizeof(step));
    if (n < 8) {
        if ((n & 1) == 0)
            start.x = base.x + (float)(n / 2 * 70 - 35);
        else
            start.x = base.x + (float)(n / 2 * 70);
        step.x = 70.0f;
    } else {
        start.x = base.x + 210.0f;
        step.x = 420.0f / (float)n;
    }
    o = dd_zone_list(1, 11)->next;
    for (i = 0; i < n; i++, o = o->next) {
        if (o->a5) {
            pos = base;
            rot = base_rot;
        } else {
            vec4_scale(&pos, &step, -(float)i);
            vec4_add(&pos, &pos, &start);
            rot = base_rot;
            dd_cursor_get(&cur);
            if (cur.player == 1 && cur.zone == 11 && cur.idx == i)
                pos.y += 10.0f;
        }
        HAND_PLACE(o, pos, rot, i, force, instant, dd_hand_zaxis1)
    }
}
#endif
