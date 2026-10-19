// @range 0x088D7AB8-0x088D9080
// @asm 0x088D7AB8-0x088D8A24
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
extern DuelCard duel_card_tmp;

void duel_effects_move_src(unsigned short from, unsigned short to);
void *memcpy(void *dst, const void *src, unsigned int n);
extern unsigned int duel_turn_player;
extern int duel_phase;

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


void duel_bzero(void *p, int n);
void duel_slot_effects_free_keep(DuelSlot *s);
void duel_card_copy(DuelCard *dst, DuelCard *src);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);

#ifdef NON_MATCHING /* draft */
/* Move the card of zone arg to zone loc (player | zone << 8); into a spell/trap zone it loses its
   position and value. */
void duel_req_move_zone(void)
{
    unsigned int a = duel_req.arg;
    unsigned int b = *(unsigned short *)&duel_req.loc;
    unsigned int ap = a & 0xFF;
    unsigned int az = a >> 8;
    unsigned int bp = b & 0xFF;
    unsigned int bz = b >> 8;
    DuelSlot *dst;
    DuelLoc from;
    DuelLoc to;

    switch (duel_req_step) {
    case 0:
        if (az < 11 && bz < 11) {
            duel_req_step = 1;
        } else {
            duel_busy_0891523C = 0;
        }
        break;
    case 1:
        duel_card_copy(&duel_card_tmp, duel_field_card_deb2c(ap, az, 0));
        if (bp == duel_view_player && az < 5 && bz < 5) {
            duel_0884DB38(0x2D, 1);
        }
        from.player = a;
        from.zone = az;
        from.idx = 0;
        from.b15 = duel.players[a & 1].field[az].pos;
        from.b14 = duel.players[a & 1].field[az].faceup;
        to = from;
        to.player = b;
        to.zone = bz;
        if (bz > 4) {
            to.b15 = 0;
        }
        dst = &duel.players[b & 1].field[bz];
        memcpy(dst, &duel.players[a & 1].field[az], sizeof(DuelSlot));
        duel_card_copy(&dst->card, &duel_card_tmp);
        duel_bzero(&duel.players[a & 1].field[az], sizeof(DuelSlot));
        duel.zone_off &= ~(1 << (az + ap * 16));
        dst->flags &= ~0x200000;
        duel_effects_move_src(ap | az << 8, bp | bz << 8);
        if (bz > 4) {
            duel_slot_effects_free_keep(dst);
            dst->pos = 0;
            dst->link = 0;
        }
        duel_host_call(0x18, CARD_UID(duel_card_tmp), *(unsigned short *)&from, *(unsigned short *)&to);
        duel_req_step++;
        break;
    default:
        duel_busy_0891523C = 0;
        break;
    }
}
#endif

#ifdef NON_MATCHING /* draft */
/* Swap the cards of zones arg and loc (player | zone << 8): exchange the slots, hide the ids while
   the host animates the swap, then put them back. */
void duel_req_swap_zones(void)
{
    unsigned int a = duel_req.arg;
    unsigned int b = *(unsigned short *)&duel_req.loc;
    unsigned int az = a >> 8;
    unsigned int bz = b >> 8;
    DuelSlot tmp;
    DuelSlot *sa;
    DuelSlot *sb;
    DuelLoc la;
    DuelLoc lb;
    unsigned int fa;
    unsigned int fb;

    switch (duel_req_step) {
    case 0:
        if (duel.players[a & 1].field[az].card.id == 0 || duel.players[b & 1].field[bz].card.id == 0) {
            duel_busy_0891523C = 0;
            return;
        }
        sb = &duel.players[b & 1].field[bz];
        memcpy(&tmp, sb, sizeof(DuelSlot));
        sa = &duel.players[a & 1].field[az];
        memcpy(sb, sa, sizeof(DuelSlot));
        memcpy(sa, &tmp, sizeof(DuelSlot));
        fa = (a & 0xFF) | az << 8;
        duel.players[a & 1].field[az].flags &= ~0x200000;
        duel.players[b & 1].field[bz].flags &= ~0x200000;
        fb = ((b & 0xFF) + 2 & 0xFF) | bz << 8;
        duel_effects_move_src(fa, fb);
        duel_effects_move_src((b & 0xFF) | bz << 8, fa);
        duel_effects_move_src(fb, (b & 0xFF) | bz << 8);
        duel_req.w = sa->card.id;
        duel_card_tmp.id = sb->card.id;
        sa->card.id = 0;
        sb->card.id = 0;
        duel_req_step++;
        return;
    case 1:
        if (az < 5 && bz < 5) {
            duel_0884DB38(0x2D, 1);
        }
        la.player = a;
        la.zone = az;
        la.b15 = duel.players[a & 1].field[az].pos;
        la.b14 = duel.players[a & 1].field[az].faceup;
        lb.player = b;
        lb.zone = bz;
        lb.b15 = duel.players[b & 1].field[bz].pos;
        lb.b14 = duel.players[b & 1].field[bz].faceup;
        duel_host_call(0x19, 0, *(unsigned short *)&la, *(unsigned short *)&lb);
        duel_req_step++;
        return;
    case 2:
        duel.players[a & 1].field[az].card.id = duel_req.w;
        duel.players[b & 1].field[bz].card.id = duel_card_tmp.id;
        break;
    }
    duel_busy_0891523C = 0;
}
#endif

#ifdef NON_MATCHING /* draft */
/* Change the position of the monster in zone arg (w: by an effect; loc: also flip it). */
void duel_req_change_pos(void)
{
    short by_effect = duel_req.w;
    unsigned int flip = *(unsigned short *)&duel_req.loc;
    unsigned int z = duel_req.arg;
    unsigned int p = (duel_req.id & 0x8000) != 0;
    DuelSlot *s = &duel.players[p & 1].field[z];
    unsigned int q;
    int k;

    if (s->card.id == 0) {
        duel_busy_0891523C = 0;
        return;
    }
    switch (duel_req_step) {
    case 0:
        if (duel_req.w != 0 && *(unsigned short *)&duel_req.loc == 0) {
            duel_host_call(0x46, p, 1 - s->pos, 0);
        }
        duel_req_step++;
        break;
    case 1:
        if (duel_req_flag(0x46) != 0) {
            break;
        }
        if (by_effect != 0 && flip != 0) {
            duel_088DF554(p, z, 1, 1);
        }
        duel_host_call(0x1A, (1 - s->pos & 1) << 15 | ((flip ^ s->faceup) & 1) << 14 | p | (z & 0x1F) << 1,
                       by_effect == 0 && flip != 0, 0);
        duel_req_step++;
        break;
    case 2:
        if (duel_req_flag(0x1A) != 0) {
            break;
        }
        s->pos = !(s->pos != 0);
        s->faceup = flip ^ s->faceup;
        if (s->faceup == 0 && flip != 0) {
            duel_slot_effects_reset(p, z);
            if (p == duel_view_player) {
                duel_0884DB38(0x1C, 1);
            }
        }
        if (s->pos != 0 && field_get_card(p, z) == 0x1743) {
            for (q = 0; (int)q < 2; q++) {
                for (k = 0; k < 5; k++) {
                    duel_effects_free_src(duel.players[q & 1].field[k].effects, p | (z & 0xFF) << 8, 1);
                }
            }
        }
        if (duel_phase == 2) {
            duel.players[duel_turn_player & 1].flags |= 0x100000;
        }
        if (flip != 0) {
            duel_088DF554(p, z, 0x13, s->faceup);
            s->flags |= 0x80;
        }
        duel_busy_0891523C = 0;
        break;
    }
}
#endif

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
