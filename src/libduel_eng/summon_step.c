// @range 0x088DE360-0x088DEB2C
// @asm 0x088DE360-0x088DEB2C
/* Steps of the requests that put a card from the hand on the field (summon, set). */
#include "duel.h"

int duel_req_flag(int i);
int duel_slot_has_effect(unsigned int p, int z, int id);
void duel_088DF554(unsigned int p, int z, unsigned int bit, int on);

extern unsigned int duel_turn_player;
extern unsigned int duel_view_player;
extern int duel_busy_0891523C;

typedef struct DuelLoc {               /* 16-bit card location in host messages */
    unsigned short player : 1;
    unsigned short zone : 5;
    unsigned short idx : 8;
    unsigned short b14 : 1;
    unsigned short b15 : 1;
} DuelLoc;

typedef struct DuelReq {               /* 0x08914A30: request being carried out */
    unsigned short id;                 /* bit 15: player */
    unsigned short arg;
    DuelLoc loc;
    unsigned short w;                  /* uid << 1 | flag */
} DuelReq;

extern DuelReq duel_req;
extern int duel_req_step;                  /* 0x08915240 */
extern DuelCard duel_card_tmp;         /* 0x08915248 */

int duel_hand_find_uid(unsigned int p, int uid);
int duel_host_call(int a, int b, int c, int d);
void duel_card_copy(DuelCard *dst, DuelCard *src);
void duel_card_take(unsigned int p, int zone, int i);
void duel_place_monster(unsigned int p, int z, DuelCard *c, int a, int b);
void duel_place_st(unsigned int p, int z, DuelCard *c, int a);
int duel_slot_counter(unsigned int p, int z, int id);
void duel_slot_effect_add(unsigned int p, int z, int a, int id, int b);
void duel_0884DB38(int a, int b);

#ifdef NON_MATCHING /* draft: the s registers and the order of the bitfield stores of the two locations differ */
/* Steps of summoning a monster from the hand: ask, move it to its zone, effects, wait. */
void duel_step_summon(void)
{
    int zone;
    int arg = duel_req.arg;
    DuelLoc loc;
    unsigned int w;
    int uid;
    int flag;
    DuelLoc from;
    int set;
    int def;
    int i;
    DuelLoc to;
    unsigned int p = (duel_req.id & 0x8000) != 0;

    *(short *)&loc = *(short *)&duel_req.loc;
    w = duel_req.w;
    uid = (int)w >> 1;
    flag = w & 1;
    zone = loc.zone;
    set = loc.b14;
    def = loc.b15;
    i = duel_hand_find_uid(p, uid);
    switch (duel_req_step) {
    case 0:
        duel_host_call(0x44, p, arg, set);
        duel_req_step++;
        break;
    case 1:
        if (duel_req_flag(0x44) != 0) {
            break;
        }
        duel_card_copy(&duel_card_tmp, &duel.players[p & 1].hand[i]);
        from.player = p;
        from.zone = 11;
        from.idx = i;
        from.b15 = 0;
        to.player = p;
        to.zone = zone;
        to.idx = 0;
        to.b15 = def;
        from.b14 = set;
        to.b14 = set;
        duel_card_take(from.player, from.zone, from.idx);
        duel_place_monster(p, zone, &duel_card_tmp, def, set);
        if (set) {
            duel_088DF554(p, zone, 1, 1);
        }
        duel_host_call(0x18, (unsigned short)uid | (unsigned short)(set ? 1 : 4) << 16, *(unsigned short *)&from, *(unsigned short *)&to);
        duel_req_step++;
        break;
    case 2:
        if (duel_slot_has_effect(p, 11, 0x15C7)) {
            duel_slot_effect_add(p, zone, 1, 0x15C7, duel_slot_counter(p, 11, 0x15C7));
        }
        if (flag) {
            duel.players[p & 1].flags |= 0x20000;
        }
        if (duel.phase == 2) {
            duel.players[duel_turn_player & 1].flags |= 0x100000;
        }
        if (!set) {
            if (p == duel_view_player) {
                duel_0884DB38(0x15, 1);
                duel_0884DB38(0x1C, 1);
            }
            duel_host_call(0x1C, p, zone, def);
        }
        duel_req_step++;
        break;
    case 3:
        if (duel_req_flag(0x1C) == 0) {
            duel_busy_0891523C = 0;
        }
        break;
    }
}


/* Steps of setting a spell or trap from the hand. */
void duel_step_set_st(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    int arg = duel_req.arg;
    DuelLoc loc;
    unsigned int uid;
    int zone;
    int set;
    int i;
    DuelLoc from;
    DuelLoc to;

    *(short *)&loc = *(short *)&duel_req.loc;
    uid = duel_req.w;
    zone = loc.zone;
    set = loc.b14;
    i = duel_hand_find_uid(p, uid);
    switch (duel_req_step) {
    case 0:
        duel_host_call(0x44, p, arg, set);
        duel_req_step++;
        break;
    case 1:
        if (duel_req_flag(0x44) != 0) {
            break;
        }
        duel_card_copy(&duel_card_tmp, &duel.players[p & 1].hand[i]);
        from.player = p;
        from.zone = 11;
        from.idx = i;
        from.b15 = 0;
        to.player = p;
        to.zone = zone;
        to.idx = 0;
        to.b15 = 0;
        from.b14 = set;
        to.b14 = set;
        duel_card_take(from.player, from.zone, from.idx);
        duel_place_st(p, zone, &duel_card_tmp, set);
        if (set) {
            duel_088DF554(p, zone, 1, 1);
        }
        duel_host_call(0x18, uid | (set ? 3 : 4) << 16, *(unsigned short *)&from, *(unsigned short *)&to);
        duel_req_step++;
        break;
    case 2:
        if (!set) {
            if (duel.phase == 2) {
                duel.players[duel_turn_player & 1].flags |= 0x100000;
            }
            duel_host_call(0x1C, p, zone, 0);
            if (p == duel_view_player) {
                duel_0884DB38(0x1D, 1);
            }
        } else {
            duel_host_call(0x43, p, arg, 0);
        }
        duel_req_step++;
        break;
    case 3:
        if (duel_req_flag(0x1C) == 0 && duel_req_flag(0x43) == 0) {
            duel_busy_0891523C = 0;
        }
        break;
    }
}
#endif
