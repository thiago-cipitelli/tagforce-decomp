// @range 0x0880A8E0-0x0880C48C
// @data 0x08874BB0-0x08874E10
// @asm 0x0880BB70-0x0880BEB0
/* Where a card sits on the field: position and rotation of each zone (by player, face up, defense
   position, stack height), the lookups in the zone lists and the rebuild of the lists from the
   duel engine (card uids) after the engine moves cards around. */
#include "dd_d1.h"

typedef int (*DdUidFn)(int player, int i);

void quat_mul(DdXyzw *out, DdXyzw *a, DdXyzw *b);
float dd_stack_y(int n);
void dd_hand_layout0(int a, int b);
void dd_hand_layout1(int a, int b);
int dv_hand_uid(int player, int i);
int dv_zone_d_uid(int player, int i);
int dv_zone_c_uid(int player, int i);
int dv_grave_uid(int player, int i);
int dv_zone_f_uid(int player, int i);
int dv_n_hand(int player);
int dv_n_d(int player);
int dv_n_18(int player);
int dv_n_grave(int player);
int dv_n_f(int player);
int dv_slot_uid(int player, int zone);
int dv_slot_faceup(int player, int zone);
int dv_slot_pos(int player, int zone);
int dv_zone_faceup(int player, int zone, int index);
int dv_list_id(int uid);

/* position and rotation of a card at `at`; zones 16-31 fall back to slot 0 of player 0 */
void dd_slot_xform(DdXyzw *pos, DdXyzw *rot, DdPos at)
{
    int zone = at.zone;
    int player = at.player;
    DdXyzw qf;
    DdXyzw qd;
    DdXyzw r;

    switch ((player << 5) | zone) {
    default: { DdXyzw t = { -210.0f, 0.0f, 85.0f, 1.0f }; *pos = t; } break;
    case 1: { DdXyzw t = { -105.0f, 0.0f, 85.0f, 1.0f }; *pos = t; } break;
    case 2: { DdXyzw t = { 0.0f, 0.0f, 85.0f, 1.0f }; *pos = t; } break;
    case 3: { DdXyzw t = { 105.0f, 0.0f, 85.0f, 1.0f }; *pos = t; } break;
    case 4: { DdXyzw t = { 210.0f, 0.0f, 85.0f, 1.0f }; *pos = t; } break;
    case 5: { DdXyzw t = { -210.0f, 0.0f, 190.0f, 1.0f }; *pos = t; } break;
    case 6: { DdXyzw t = { -105.0f, 0.0f, 190.0f, 1.0f }; *pos = t; } break;
    case 7: { DdXyzw t = { 0.0f, 0.0f, 190.0f, 1.0f }; *pos = t; } break;
    case 8: { DdXyzw t = { 105.0f, 0.0f, 190.0f, 1.0f }; *pos = t; } break;
    case 9: { DdXyzw t = { 210.0f, 0.0f, 190.0f, 1.0f }; *pos = t; } break;
    case 10: { DdXyzw t = { -315.0f, 0.0f, 135.0f, 1.0f }; *pos = t; } break;
    case 11: { DdXyzw t = { 0.0f, 200.0f, 440.0f, 1.0f }; *pos = t; } break;
    case 12: { DdXyzw t = { -315.0f, 0.0f, 240.0f, 1.0f }; *pos = t; } break;
    case 13: { DdXyzw t = { 315.0f, 0.0f, 240.0f, 1.0f }; *pos = t; } break;
    case 14: { DdXyzw t = { 315.0f, 0.0f, 135.0f, 1.0f }; *pos = t; } break;
    case 15: { DdXyzw t = { 315.0f, 0.0f, 30.0f, 1.0f }; *pos = t; } break;
    case 32: { DdXyzw t = { 210.0f, 0.0f, -85.0f, 1.0f }; *pos = t; } break;
    case 33: { DdXyzw t = { 105.0f, 0.0f, -85.0f, 1.0f }; *pos = t; } break;
    case 34: { DdXyzw t = { 0.0f, 0.0f, -85.0f, 1.0f }; *pos = t; } break;
    case 35: { DdXyzw t = { -105.0f, 0.0f, -85.0f, 1.0f }; *pos = t; } break;
    case 36: { DdXyzw t = { -210.0f, 0.0f, -85.0f, 1.0f }; *pos = t; } break;
    case 37: { DdXyzw t = { 210.0f, 0.0f, -190.0f, 1.0f }; *pos = t; } break;
    case 38: { DdXyzw t = { 105.0f, 0.0f, -190.0f, 1.0f }; *pos = t; } break;
    case 39: { DdXyzw t = { 0.0f, 0.0f, -190.0f, 1.0f }; *pos = t; } break;
    case 40: { DdXyzw t = { -105.0f, 0.0f, -190.0f, 1.0f }; *pos = t; } break;
    case 41: { DdXyzw t = { -210.0f, 0.0f, -190.0f, 1.0f }; *pos = t; } break;
    case 42: { DdXyzw t = { 315.0f, 0.0f, -135.0f, 1.0f }; *pos = t; } break;
    case 43: { DdXyzw t = { 0.0f, 50.0f, -300.0f, 1.0f }; *pos = t; } break;
    case 44: { DdXyzw t = { 315.0f, 0.0f, -240.0f, 1.0f }; *pos = t; } break;
    case 45: { DdXyzw t = { -315.0f, 0.0f, -240.0f, 1.0f }; *pos = t; } break;
    case 46: { DdXyzw t = { -315.0f, 0.0f, -135.0f, 1.0f }; *pos = t; } break;
    case 47: { DdXyzw t = { -315.0f, 0.0f, -30.0f, 1.0f }; *pos = t; } break;
    }
    switch (zone) {
    case 14:
    case 15:
        pos->y += dd_stack_y(at.idx);
        break;
    case 13:
        if (at.idx == 0)
            pos->y += dd_stack_y(dd_zone_count(player, zone) - at.idx);
        break;
    }
    { DdXyzw t = { 0.0f, 0.0f, 0.0f, 1.0f }; qf = *(DdXyzw *)&t; }
    { DdXyzw t = { 0.0f, 0.0f, 0.0f, 1.0f }; qd = *(DdXyzw *)&t; }
    { DdXyzw t = { 0.0f, 0.0f, 0.0f, 1.0f }; r = *(DdXyzw *)&t; }
    if (at.f14) { DdXyzw t = { 0.0f, 0.0f, 1.0f, 0.0f }; qf = *(DdXyzw *)&t; }
    if (at.f15) { DdXyzw t = { 0.0f, 0.707107f, 0.0f, 0.707107f }; qd = *(DdXyzw *)&t; }
    if (at.player) { DdXyzw t = { 0.0f, 1.0f, 0.0f, 0.0f }; r = *(DdXyzw *)&t; }
    *rot = r;
    quat_mul(rot, rot, &qd);
    quat_mul(rot, rot, &qf);
}

/* number of card objects in a zone */
int dd_zone_count(int player, int zone)
{
    DdCobj *head = dd_zone_list(player, zone);
    DdCobj *o = head->next;
    int n;

    if (o == head)
        return 0;
    n = 0;
    while (o != head) {
        o = o->next;
        n++;
    }
    return n;
}

/* card object `index` of a zone (0xFF/0xFFFF: the first one) */
DdCobj *dd_cobj_at(int player, int zone, int index)
{
    DdCobj *head = dd_zone_list(player, zone);
    DdCobj *o = head->next;
    int i;

    if (o == head)
        return 0;
    if (index == 0xFF || index == 0xFFFF)
        return o;
    for (i = 0; i < index && o != head; i++)
        o = o->next;
    if (o == head)
        return 0;
    return o;
}

/* the search loop alone in an inline helper: the back edge gets the likely branch with the copy of the loop head */
static inline DdCobj *dd_cobj_scan(DdCobj *head, DdCobj *o, int uid)
{
    while (o->uid != uid && (o = o->next) != head)
        ;
    return o;
}

/* card object of card `uid` in a zone */
DdCobj *dd_cobj_find(int player, int zone, int uid)
{
    DdCobj *head = dd_zone_list(player, zone);
    DdCobj *o = head->next;

    if (o == head)
        return 0;
    o = dd_cobj_scan(head, o, uid);
    if (o == head)
        return 0;
    return o;
}

void dd_zone_rebuild(int player, int keep);

/* matches the objects of a pile (deck, extra, graveyard...) with the engine's list of uids */
void dd_zone_sync(int player, int zone)
{
    DdCobjs *b = &dd_cobjs;
    DdUidFn uid_of;
    int n;
    DdCobj *head;
    DdCobj *o;
    DdCobj *p = 0;
    int i;
    int uid;
    int id;

    switch (zone) {
    default: return;
    case 13: uid_of = dv_zone_d_uid; n = dv_n_d(player); break;
    case 12: uid_of = dv_zone_c_uid; n = dv_n_18(player); break;
    case 14: uid_of = dv_grave_uid; n = dv_n_grave(player); break;
    case 11: uid_of = dv_hand_uid; n = dv_n_hand(player); break;
    case 15: uid_of = dv_zone_f_uid; n = dv_n_f(player); break;
    }
    head = dd_zone_list(player, zone);
    o = head->next;
    for (i = 0; i < n; i++) {
        uid = uid_of(player, i) & 0xFFFF;
        id = dv_list_id(uid);
        if (o != head) {
            if (o->uid == uid) {
                o = o->next;
                continue;
            }
            if (o->uid == 0 && o->card == id) {
                o->uid = uid;
                o = o->next;
                continue;
            }
            p = o->next;
            do {
                if (p->uid == uid)
                    break;
                if (p->uid == 0 && p->card == id) {
                    p->uid = uid;
                    break;
                }
            } while ((p = p->next) != head);
        }
        if (o == head || p == head) {
            if (b->sync == 0) {
                dd_zone_rebuild(player, 0);
                return;
            }
            p = dd_cobj_alloc();
            memset(p, 0, sizeof(DdCobj));
            p->card = id;
            p->uid = uid;
            p->home.player = player;
            p->home.zone = zone;
            p->home.idx = i;
            p->prev = o->prev;
            p->next = o;
            p->prev->next = p;
            p->next->prev = p;
        } else {
            dd_list_unlink(p, head);
            p->prev = o->prev;
            p->next = o;
            p->prev->next = p;
            p->next->prev = p;
        }
    }
    if (o != head)
        dd_zone_rebuild(player, 0);
}

extern int dd_pile_zones[5];           /* 0x08874E10 13, 12, 11, 14, 15 */

#ifdef NON_MATCHING /* 24 palavras: escalonamento do unlink/add_tail inline (a zona carregada depois de o->next/o->prev) */
/* rebuilds the piles of a player from the engine's lists, reusing the objects by uid; the unlink and
   add_tail are inline here. Reads tmp.uid when every pile was empty (as the original). */
void dd_zone_rebuild(int player, int redraw)
{
    DdCobj tmp;
    int k;
    DdCobj *last;
    DdUidFn uid_of;
    int n;
    int i;
    DdCobj *o;
    unsigned short uid;
    int id;
    int zone;

    tmp.next = tmp.prev = &tmp;
    for (k = 0; k < 5; k++) {
        DdCobj *h = dd_zone_list(player, dd_pile_zones[k]);
        DdCobj *first = h->next;
        if (first != h) {
            last = h->prev;
            h->next = h->prev = h;
            last->next = &tmp;
            first->prev = tmp.prev;
            tmp.prev->next = first;
            tmp.prev = last;
        }
    }
    for (k = 0; k < 5; k++) {
        switch (dd_pile_zones[k]) {
        case 13: uid_of = dv_zone_d_uid; n = dv_n_d(player); break;
        case 12: uid_of = dv_zone_c_uid; n = dv_n_18(player); break;
        case 11: uid_of = dv_hand_uid; n = dv_n_hand(player); break;
        case 14: uid_of = dv_grave_uid; n = dv_n_grave(player); break;
        case 15: uid_of = dv_zone_f_uid; n = dv_n_f(player); break;
        default: continue;
        }
        {
        DdCobj *head = dd_zone_list(player, dd_pile_zones[k]);
        for (i = 0; i < n; i++) {
            uid = uid_of(player, i);
            id = dv_list_id(uid);
            o = tmp.next;
            do {
                if (o->uid == uid)
                    break;
                if (o->uid == 0 && o->card == id) {
                    o->uid = uid;
                    break;
                }
            } while ((o = o->next) != &tmp);
            if (o == &tmp)
                continue;
            o->prev->next = o->next;
            o->next->prev = o->prev;
            o->prev = o->next = 0;
            zone = dd_pile_zones[k];
            o->prev = head->prev;
            o->next = head;
            o->prev->next = o;
            o->next->prev = o;
            o->home.zone = zone;
            o->home.idx = i;
            o->loc.zone = zone;
            o->loc.idx = i;
            if (zone == 11) {
                if (dv_zone_faceup(player, 11, i))
                    o->b3 = 1;
                else
                    o->b3 = 0;
            }
        }
    }
    }
    if (redraw) {
        if (player == 0)
            dd_hand_layout0(0, 1);
        else
            dd_hand_layout1(0, 1);
    }
}
#endif

/* builds every card object from the engine's state (after a load) */
void dd_cobjs_rebuild(void)
{
    DdCobjs *b = &dd_cobjs;
    int p;
    int z;
    int i;
    int n;
    DdCobj *o;
    int uid;
    DdCobj *head;
    DdUidFn uid_of;
    DdPos at;
    int k;
    DdCobj *o2;

    dd_cobjs.free.next = dd_cobjs.free.prev = &dd_cobjs.free;
    for (z = 0; z < 4; z++)
        for (i = 0; i < 16; i++)
            b->zone[z][i].next = b->zone[z][i].prev = &b->zone[z][i];
    for (i = 0; i < 460U; i++) {
        o = &b->obj[i];
        o->next = b->free.next;
        o->prev = &b->free;
        o->prev->next = o;
        o->next->prev = o;
    }
    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 10; z++) {
            uid = dv_slot_uid(p, z);
            if (uid > 0) {
                head = dd_zone_list(p, z);
                o = dd_cobj_alloc();
                memset(o, 0, sizeof(DdCobj));
                o->uid = uid;
                o->card = dv_list_id(uid);
                o->home.player = p;
                o->home.zone = z;
                o->home.idx = 0;
                at = *(DdPos *)&o->home;
                at.f14 = dv_slot_faceup(p, z);
                at.f15 = dv_slot_pos(p, z);
                dd_slot_xform(&o->pos, &o->rot, at);
                o->pos0 = o->pos;
                o->pos1 = o->pos;
                o->rot0 = o->rot;
                o->rot1 = o->rot;
                dd_list_add_tail(o, head);
            }
        }
        for (z = 11; z <= 15; z++) {
            switch (z) {
            default:
            case 11: n = dv_n_hand(p); uid_of = dv_hand_uid; break;
            case 12: n = dv_n_18(p); uid_of = dv_zone_c_uid; break;
            case 13: n = dv_n_d(p); uid_of = dv_zone_d_uid; break;
            case 14: n = dv_n_grave(p); uid_of = dv_grave_uid; break;
            case 15: n = dv_n_f(p); uid_of = dv_zone_f_uid; break;
            }
            head = dd_zone_list(p, z);
            for (k = 0; k < n; k++) {
                uid = uid_of(p, k);
                o2 = dd_cobj_alloc();
                memset(o2, 0, sizeof(DdCobj));
                o2->uid = uid;
                o2->card = dv_list_id(uid);
                o2->home.player = p;
                o2->home.zone = z;
                o2->home.idx = 0;
                at = *(DdPos *)&o2->home;
                at.f14 = dv_slot_faceup(p, z);
                at.f15 = dv_slot_pos(p, z);
                dd_slot_xform(&o2->pos, &o2->rot, at);
                o2->pos0 = o2->pos;
                o2->pos1 = o2->pos;
                o2->rot0 = o2->rot;
                o2->rot1 = o2->rot;
                dd_list_add_tail(o2, head);
            }
        }
        dd_hand_layout0(0, 1);
        dd_hand_layout1(0, 1);
    }
}
