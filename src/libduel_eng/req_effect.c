// @range 0x088D86B8-0x088D9080
// @asm 0x088D86B8-0x088D8A24
// @asm 0x088D8AB8-0x088D8B1C
// @asm 0x088D8C50-0x088D8CF8
// @asm 0x088D8DD0-0x088D9080
/* Requests that change effect entries and slot values directly (the effect half of the request steps
   in req_step.c). */
#include "duel.h"

typedef struct DuelLoc {
    unsigned short player : 1;
    unsigned short zone : 5;
    unsigned short idx : 8;
    unsigned short b14 : 1;
    unsigned short b15 : 1;
} DuelLoc;

typedef struct DuelReq {               /* 0x08914A30 */
    unsigned short id;                 /* bit 15: player */
    unsigned short arg;
    DuelLoc loc;
    unsigned short w;
} DuelReq;

extern DuelReq duel_req;
extern unsigned int duel_req_step;
extern int duel_busy_0891523C;
extern int duel_order_ctr;
extern unsigned int duel_view_player;

int duel_req_flag(int i);
int duel_host_call(int a, int b, int c, int d);
void duel_slot_effect_add(unsigned int p, int z, int kind, int src, int arg);
void duel_list_effect_add(int uid, int kind, int src, int arg);
void duel_effect_set(unsigned int p, int z, int i, int kind, unsigned short arg);
void duel_effects_free_card(unsigned int e, unsigned short id, int all);
void duel_effects_free_src(unsigned int e, unsigned short src, int all);
void duel_effects_free_kind(unsigned int e, unsigned short id, unsigned int kind, int all);
void duel_088DF554(unsigned int p, int z, unsigned int bit, int on);
int field_get_card(unsigned int p, int z);
int card_get_group_value_1983(int id);
int duel_slot_counter_kind(unsigned int p, int z, int id, int kind);
void duel_0884DB38(int a, int b);

void duel_slot_effects_reset(unsigned int p, unsigned int z);

#ifdef NON_MATCHING /* draft: the original keeps the zone address parts in s registers */
/* Turn the card in zone arg face up or down (w: 1 reveal first, 2 set, 4 announce, 8 only show). */
void duel_req_flip(void)
{
    unsigned int z = duel_req.arg;
    unsigned int p = (duel_req.id & 0x8000) != 0;
    DuelSlot *s = &duel.players[p & 1].field[z];
    unsigned int pos = *(unsigned short *)&duel_req.loc;
    unsigned int w = duel_req.w;
    unsigned int show = w & 1;
    unsigned int set = w & 2;
    unsigned int ann = w & 4;
    unsigned int only = w & 8;
    unsigned int id = s->card.id;

    if (id == 0) {
        duel_busy_0891523C = 0;
        return;
    }
    switch (duel_req_step) {
    case 0:
        if (show) {
            duel_special.unk24 = id;
            duel_host_call(0x47, p, id, 0);
        }
        if (ann) {
            duel_host_call(0x47, p, id, 1);
        }
        duel_req_step++;
        break;
    case 1:
        if (duel_req_flag(0x47) != 0) {
            break;
        }
        if (ann || set) {
            duel_088DF554(p, z, 1, 1);
        }
        if (only == 0) {
            duel_host_call(0x1A, (s->pos & 1) << 15 | (1 - s->faceup & 1) << 14 | p | (z & 0x1F) << 1, set == 0, 0);
        } else {
            duel_host_call(0x1A, (s->pos & 1) << 15 | (pos & 1) << 14 | p | (z & 0x1F) << 1, 1, 0);
        }
        duel_req_step++;
        break;
    case 2:
        if (duel_req_flag(0x1A) != 0) {
            break;
        }
        if (only == 0) {
            s->faceup = !(s->faceup != 0);
            if (s->faceup == 0) {
                duel_slot_effects_reset(p, z);
                if (p == duel_view_player && z < 5) {
                    duel_0884DB38(0x1C, 1);
                }
            } else {
                duel_order_ctr++;
                s->unk4 = duel_order_ctr;
            }
            if (z < 5) {
                duel_088DF554(p, z, 0x13, s->faceup);
            }
        }
        if (ann) {
            duel_host_call(0x43, p, id, 1);
        }
        s->flags |= 0x80;
        duel_req_step++;
        break;
    case 3:
        if (duel_req_flag(0x43) == 0) {
            duel_busy_0891523C = 0;
        }
        break;
    }
}
#endif

/* Add an effect entry (loc bit 15: to the list card arg instead of the zone). */
void duel_req_effect_add(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int z = duel_req.arg & 0xFF;
    unsigned int k = duel_req.arg >> 8 & 0xFF;
    unsigned int loc = *(unsigned short *)&duel_req.loc;
    unsigned int w = duel_req.w;
    unsigned int kind = loc & 0x7FFF;

    if (loc & 0x8000) {
        duel_list_effect_add(p + z * 2, k, kind, w);
    } else {
        duel_slot_effect_add(p, z, k, kind, w);
    }
    duel_host_call(0x24, 0, 0, 0);
    duel_busy_0891523C = 0;
}

#ifdef NON_MATCHING /* the player test is scheduled after the loads of the other fields */
void duel_req_effect_set(void)
{
    unsigned int arg = duel_req.arg;

    duel_effect_set((duel_req.id & 0x8000) != 0, arg & 0xFF, arg >> 8 & 0xFF, *(unsigned short *)&duel_req.loc, duel_req.w);
    duel_host_call(0x24, 0, 0, 0);
    duel_busy_0891523C = 0;
}
#endif

/* Free entries of zone arg & 0xFF by card (arg >> 8 below 6), by source, or by kind (w >> 8). */
void duel_req_effect_free(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int z = duel_req.arg & 0xFF;
    int k = duel_req.arg >> 8 & 0xFF;
    unsigned int id = *(unsigned short *)&duel_req.loc;
    unsigned int all = duel_req.w & 0xFF;
    unsigned int mode = duel_req.w >> 8 & 0xFF;

    if (mode != 0) {
        duel_effects_free_kind(duel.players[p & 1].field[z].effects, id, k, all);
    } else if (k >= 6) {
        duel_effects_free_src(duel.players[p & 1].field[z].effects, id, all);
    } else {
        duel_effects_free_card(duel.players[p & 1].field[z].effects, id, all);
    }
    duel_host_call(0x24, 0, 0, 0);
    duel_busy_0891523C = 0;
}

#ifdef NON_MATCHING /* the original stores the activation counter before the zone field */
/* Give zone arg its activation order. */
void duel_req_order(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int z = (unsigned char)duel_req.arg;
    char *b = (char *)(z * sizeof(DuelSlot) + (int)&duel.players[p & 1]);

    if (((DuelSlot *)(b + 0x30))->card.id != 0) {
        duel_order_ctr++;
        ((DuelSlot *)(b + 0x30))->unk4 = duel_order_ctr;
        duel_088DF554(p, z, 1, 0);
        duel_host_call(0x24, 0, 0, 0);
    }
    duel_busy_0891523C = 0;
}
#endif

/* Set (arg >> 8 == 0) or add the value loc | w << 16 of zone arg & 0xFF. */
void duel_req_slot_value(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int z = duel_req.arg & 0xFF;
    unsigned int add = duel_req.arg >> 8 & 0xFF;
    unsigned int v = *(unsigned short *)&duel_req.loc | duel_req.w << 16;

    if (add != 0) {
        duel.players[p & 1].field[z].link += v;
    } else {
        duel.players[p & 1].field[z].link = v;
    }
    duel_host_call(0x24, 0, 0, 0);
    duel_busy_0891523C = 0;
}

#ifdef NON_MATCHING /* the s registers and the clamping branches differ */
/* Change the counters of zone arg by loc (w 0: the zone value up to the card's maximum, w 1: the zone
   value, else entries of card w). */
void duel_req_counters(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int z = duel_req.arg;
    int k;
    int i;
    int n = (short)*(unsigned short *)&duel_req.loc;
    int max;
    unsigned int id = duel_req.w;
    int v;
    int old;
    int card = field_get_card(p, z);

    switch (duel_req_step) {
    case 0:
        if (card != 0) {
            if (id == 0) {
                max = card_get_group_value_1983(card);
                old = duel.players[p & 1].field[z].link;
                v = MIN(old + n, max);
                v = MAX(v, 0);
                duel.players[p & 1].field[z].link = v;
                if (p == duel_view_player) {
                    if (n < 0) {
                        duel_0884DB38(0x2F, old - v);
                    } else {
                        duel_0884DB38(0x2E, v - old);
                    }
                }
            } else if (id == 1) {
                v = duel.players[p & 1].field[z].link + n;
                v = MAX(v, 0);
                duel.players[p & 1].field[z].link = v;
            } else if (n > 0) {
                for (i = 0; i < n; i++) {
                    duel_slot_effect_add(p, z, 3, id, 0);
                }
            } else {
                max = duel_slot_counter_kind(p, z, id, 3);
                k = -n;
                if (max < k) {
                    k = max;
                }
                for (i = 0; i < k; i++) {
                    duel_effects_free_card(duel.players[p & 1].field[z].effects, id, 0);
                }
            }
            duel_host_call(0x25, (unsigned char)p | (unsigned char)z << 8, n, id);
        }
        duel_req_step++;
        break;
    case 1:
        if (duel_req_flag(0x25) == 0) {
            duel_host_call(0x24, 0, 0, 0);
            duel_busy_0891523C = 0;
        }
        break;
    }
}
#endif
