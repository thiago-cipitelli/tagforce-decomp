// @range 0x0880A8E0-0x0880C48C
// @data 0x08874BB0-0x08874E10
// @asm 0x0880B834-0x0880C48C
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

#ifdef NON_MATCHING /* 4 palavras: o primeiro teste e o laço saem com o likely trocado (do/while, while com break, for com e sem retorno antecipado testados) */
/* card object of card `uid` in a zone */
DdCobj *dd_cobj_find(int player, int zone, int uid)
{
    DdCobj *head = dd_zone_list(player, zone);
    DdCobj *o = head->next;

    if (o == head)
        return 0;
    while (o->uid != uid && (o = o->next) != head)
        ;
    if (o == head)
        return 0;
    return o;
}
#endif

#ifdef NON_MATCHING /* laços de busca e campos de bits; não reproduzido */
void dd_zone_rebuild(int player, int keep);

/* matches the objects of a pile (deck, extra, graveyard...) with the engine's list of uids */
void dd_zone_sync(int player, int zone)
{
    DdUidFn uid_of;
    int n;
    DdCobj *head;
    DdCobj *o;
    DdCobj *p = 0;
    int i;
    int uid;
    unsigned short id;

    switch (zone) {
    case 15: uid_of = dv_zone_f_uid; n = dv_n_f(player); break;
    case 11: uid_of = dv_hand_uid; n = dv_n_hand(player); break;
    case 14: uid_of = dv_grave_uid; n = dv_n_grave(player); break;
    case 12: uid_of = dv_zone_c_uid; n = dv_n_18(player); break;
    case 13: uid_of = dv_zone_d_uid; n = dv_n_d(player); break;
    default: return;
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
            for (p = o->next; p->uid != uid; p = p->next) {
                if (p->uid == 0 && p->card == id) {
                    p->uid = uid;
                    break;
                }
                if (p == head)
                    break;
            }
        }
        if (o == head || p == head) {
            if (dd_cobjs.sync == 0) {
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
        } else {
            dd_list_unlink(p, head);
        }
        p->prev = o->prev;
        p->next = o;
        p->prev->next = p;
        p->next->prev = p;
    }
    if (o != head)
        dd_zone_rebuild(player, 0);
}

extern int dd_pile_zones[5];           /* 0x08874E10 13, 12, 11, 14, 15 */

/* rebuilds the piles of a player from the engine's lists, reusing the objects by uid */
void dd_zone_rebuild(int player, int redraw)
{
    DdCobj tmp;
    int k;
    DdCobj *head;
    DdCobj *o;
    DdUidFn uid_of;
    int n;
    int i;
    int uid;
    unsigned short id;
    int zone;

    tmp.prev = tmp.next = &tmp;
    for (k = 0; k < 5; k++) {
        head = dd_zone_list(player, dd_pile_zones[k]);
        if (head->next != head) {
            head->prev->next = &tmp;
            head->next->prev = tmp.prev;
            tmp.prev->next = head->next;
            tmp.prev = head->prev;
            head->prev = head->next = head;
        }
    }
    for (k = 0; k < 5; k++) {
        zone = dd_pile_zones[k];
        switch (zone) {
        case 15: uid_of = dv_zone_f_uid; n = dv_n_f(player); break;
        case 14: uid_of = dv_grave_uid; n = dv_n_grave(player); break;
        case 11: uid_of = dv_hand_uid; n = dv_n_hand(player); break;
        case 12: uid_of = dv_zone_c_uid; n = dv_n_18(player); break;
        case 13: uid_of = dv_zone_d_uid; n = dv_n_d(player); break;
        default: continue;
        }
        head = dd_zone_list(player, zone);
        for (i = 0; i < n; i++) {
            uid = uid_of(player, i) & 0xFFFF;
            id = dv_list_id(uid);
            for (o = tmp.next; o != &tmp && o->uid != uid; o = o->next) {
                if (o->uid == 0 && o->card == id) {
                    o->uid = uid;
                    break;
                }
            }
            if (o == &tmp)
                continue;
            dd_list_unlink(o, &tmp);
            dd_list_add_tail(o, head);
            o->home.zone = zone;
            o->home.idx = i;
            o->loc.zone = zone;
            o->loc.idx = i;
            if (zone == 11)
                o->b3 = dv_zone_faceup(player, 11, i) != 0;
        }
    }
    if (redraw) {
        if (player == 0)
            dd_hand_layout0(0, 1);
        else
            dd_hand_layout1(0, 1);
    }
}

/* builds every card object from the engine's state (after a load) */
void dd_cobjs_rebuild(void)
{
    DdCobjs *b = &dd_cobjs;
    int p;
    int z;
    int i;
    int n;
    int uid;
    DdCobj *head;
    DdCobj *o;
    DdUidFn uid_of;
    DdPos at;

    b->free.prev = b->free.next = &b->free;
    for (p = 0; p < 4; p++)
        for (z = 0; z < 16; z++)
            b->zone[p][z].prev = b->zone[p][z].next = &b->zone[p][z];
    for (i = 0; i < 460; i++)
        dd_list_add_tail(&b->obj[i], &b->free);
    for (p = 0; p < 2; p++) {
        for (z = 0; z < 11; z++) {
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
        for (z = 11; z < 16; z++) {
            switch (z) {
            case 15: n = dv_n_f(p); uid_of = dv_zone_f_uid; break;
            case 14: n = dv_n_grave(p); uid_of = dv_grave_uid; break;
            case 13: n = dv_n_d(p); uid_of = dv_zone_d_uid; break;
            case 12: n = dv_n_18(p); uid_of = dv_zone_c_uid; break;
            default: n = dv_n_hand(p); uid_of = dv_hand_uid; break;
            }
            head = dd_zone_list(p, z);
            for (i = 0; i < n; i++) {
                uid = uid_of(p, i);
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
        dd_hand_layout0(0, 1);
        dd_hand_layout1(0, 1);
    }
}
#endif
