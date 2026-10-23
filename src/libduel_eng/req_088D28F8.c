// @range 0x088D28F8-0x088D5BA0
// @data 0x08911B8C-0x08911D2C
// @asm 0x088D28F8-0x088D2C14
// @asm 0x088D2C9C-0x088D2D40
// @asm 0x088D2D40-0x088D2FD8
// @asm 0x088D31F0-0x088D36C8
// @asm 0x088D3744-0x088D392C
// @asm 0x088D392C-0x088D3E5C
// @asm 0x088D3E5C-0x088D416C
// @asm 0x088D43F8-0x088D4A04
// @asm 0x088D5830-0x088D5BA0
// @asm 0x088D416C-0x088D42C8
// @asm 0x088D42C8-0x088D43F8
/* Requests carried out one step per frame (duel_req, duel_req_step): revealing and flipping a card,
   showing a card of a list, sorting the zones, ... */
#include "duel.h"
#include "card_db.h"

typedef struct DuelListCard {
    unsigned short id : 13;
    unsigned short placed : 2;
    unsigned short b15 : 1;
    unsigned short effects;
} DuelListCard;

extern DuelListCard duel_cards[512];

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

typedef struct DuelLogEnt {
    unsigned short v[4];
} DuelLogEnt;

typedef struct DuelReqQueue {          /* 0x08914A30 */
    DuelReq req;                       /* request being carried out */
    DuelLogEnt q[256];                 /* commands for the presentation queued after it */
    unsigned int n;                    /* +0x808 (duel_req.n) commands in q */
} DuelReqQueue;

extern DuelReqQueue duel_req;
extern unsigned int duel_req_step;     /* 0x08915240 */
extern unsigned int duel_view_player;  /* 0x089139F4 */
extern int duel_busy_0891523C;
extern unsigned int duel_089195C8;     /* random seed */

typedef struct DuelPick {              /* 0x0891524C, as in duel_pick.c */
    int unk0;
    int kind;
    int cur;
    unsigned int n;
    DuelCard c[512];
} DuelPick;

extern DuelPick duel_w_0891524C;

int duel_req_flag(int i);
int duel_host_call(int a, int b, int c, int d);
void duel_0884DB38(int a, int b);
void duel_0884DB7C();
void duel_088DF554(unsigned int p, int z, unsigned int bit, int on);
DuelCard *duel_card_by_uid(int uid);
unsigned int duel_card_locate(unsigned int uid);
int duel_grave_find_uid(unsigned int p, int uid);
void duel_grave_to_top(unsigned int p, unsigned short uid);
DuelCard *duel_0884C7EC(int i);
void duel_list_effect_add(int uid, int kind, int src, int arg);
int duel_rand(int n);
void duel_effects_move_src(unsigned short from, unsigned short to);
void duel_memcpy(void *dst, void *src, int n);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_counter(unsigned int p, int z, int id);
void duel_slot_effect_add(unsigned int p, int z, int a, int id, int b);
void duel_card_copy(DuelCard *dst, DuelCard *src);
void duel_card_take(unsigned int p, int zone, int i);
void duel_card_put(unsigned int p, int zone, unsigned int arg, DuelCard *c);
unsigned short duel_zone_pos(unsigned int p, int z, int i);
int duel_zone_faceup(unsigned int p, int z, int i);
unsigned short duel_uid_next();
void duel_place_monster(unsigned int p, int z, DuelCard *c, unsigned short pos, int up);
unsigned int duel_slot_min_1130(unsigned int p, int z);
void duel_effects_free_card();
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);

extern unsigned int duel_req_count;    /* 0x08915244 */
extern DuelCard duel_card_tmp;         /* 0x08915248 */
extern unsigned int duel_turn_player;
extern int duel_phase;
extern int duel_w_08917DC4;
extern int duel_w_089195E4;
extern int duel_atk_direct;            /* 0x08913F18 */
extern int duel_player_type[2];        /* 0x089139F8 */
extern struct {
    unsigned char lo : 2;
    unsigned char b2 : 1;              /* a message sent outside the queue is being carried out */
    unsigned char b3 : 1;              /* a queued command is being carried out */
    unsigned char hi : 4;
} duel_msg_state;                      /* 0x08914340, first byte of the DuelMsgState */
extern DuelLogEnt duel_msg_last;       /* 0x08914344 */
extern unsigned char duel_w_0891434C;

void duel_0884DA24(int a, int b);
void duel_0884DAF0(int a, int b);
int duel_stat_get(int i);
void duel_log_push(DuelLogEnt *v);
void duel_req_088D57E0(void);
void duel_req_088D5830(void);
void duel_req_088D5BA0(void);
void duel_req_088D5D2C(void);
void duel_req_088D5DEC(void);
void duel_req_088D5FC8(void);
void duel_req_088D613C(void);
void duel_req_088D6168(void);
void duel_req_088D624C(void);
void duel_req_088D6294(void);
void duel_req_088D6310(void);
void duel_req_088D6348(void);
void duel_req_088D63FC(void);
void duel_req_088D6708(void);
void duel_req_088D9080(void);
void duel_req_088D9248(void);
void duel_req_088D9504(void);
void duel_req_088D9DBC(void);
void duel_req_activate(void);
void duel_req_add_effect(void);
void duel_req_announce(void);
void duel_req_announce_field(void);
void duel_req_banish(void);
void duel_req_change_pos(void);
void duel_req_counter(void);
void duel_req_counters(void);
void duel_req_cur_card(void);
void duel_req_d_return(void);
void duel_req_d_reverse(void);
void duel_req_d_shuffle(void);
void duel_req_done(void);
void duel_req_draw(void);
void duel_req_effect_add(void);
void duel_req_effect_free(void);
void duel_req_effect_set(void);
void duel_req_end_turn(void);
void duel_req_flip(void);
void duel_req_grave_top(void);
void duel_req_hand_flash(void);
void duel_req_hand_open(void);
void duel_req_hand_show(void);
void duel_req_hand_shuffle(void);
void duel_req_host_1(void);
void duel_req_host_15(void);
void duel_req_host_2(void);
void duel_req_host_2e(void);
void duel_req_host_2f(void);
void duel_req_host_36(void);
void duel_req_host_37(void);
void duel_req_host_38(void);
void duel_req_host_39(void);
void duel_req_host_3a(void);
void duel_req_host_3b(void);
void duel_req_host_3b_table(void);
void duel_req_host_3c(void);
void duel_req_host_3d(void);
void duel_req_host_3e(void);
void duel_req_host_42(void);
void duel_req_log_seek(void);
void duel_req_log_seek_if(void);
void duel_req_message(void);
void duel_req_mill(void);
void duel_req_mode_bit(void);
void duel_req_move(void);
void duel_req_move_card(void);
void duel_req_move_zone(void);
void duel_req_next_turn(void);
void duel_req_order(void);
void duel_req_phase_0(void);
void duel_req_phase_1(void);
void duel_req_phase_2(void);
void duel_req_phase_3(void);
void duel_req_phase_4(void);
void duel_req_phase_5(void);
void duel_req_player_bit(void);
void duel_req_point(void);
void duel_req_send(void);
void duel_req_set_card(void);
void duel_req_slot_bit(void);
void duel_req_slot_value(void);
void duel_req_swap_d_grave(void);
void duel_req_swap_zones(void);
void duel_req_wait_5(void);
void duel_req_wait_frames(void);
void duel_step_set_st(void);
void duel_step_summon(void);

extern int duel_w_0891961C;
extern int duel_w_089195EC;
extern int duel_req_queue_step;             /* 0x08919664 */
extern int duel_atk_zone;              /* 0x08913F2C */
extern unsigned int duel_atk_tp;       /* 0x08913F14 */
extern int duel_atk_tz;                /* 0x08913F30 */

void duel_msg_send(unsigned short cmd, int arg, void *data, int len);
void duel_msg_send3(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_0884D618(int all);
int duel_slot_count_src(unsigned int p, int z, int id);
void duel_player_flag_set(unsigned int p, unsigned int bit, int on);
void duel_mode_set(unsigned int bit, int on);
int field_get_card(unsigned int p, int z);

/* commands for the presentation, queued after the request itself */
#define DUEL_CMDQ (duel_req.q)

#ifdef NON_MATCHING /* o original guarda z * 20 e p * 0xFAC em registradores s além do ponteiro da zona; não reproduzido */
/* Reveal the card at (p, arg) (loc: bit 0 known, bit 2 flipped face up; w: low byte, high byte). */
void duel_req_088D28F8(void)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int z = duel_req.req.arg;
    unsigned int flags = *(unsigned short *)&duel_req.req.loc;
    unsigned int v = p;
    unsigned int w = duel_req.req.w;
    unsigned int a = w & 0xFF;
    unsigned int b = w >> 8 & 0xFF;
    DuelSlot *s = &duel.players[p & 1].field[z];

    if ((flags & 1) && (flags & 4)) {
        switch (s->card.id) {
        case 0x1578:
        case 0x1670:
        case 0x1671:
        case 0x1672:
        case 0x1A92:
            v = 1 - p;
            break;
        }
    }
    if (flags & 1) {
        s->card.b14 = 1;
        if (v == duel_view_player) {
            duel.list[CARD_UID(s->card)].b15 = 1;
            duel_0884DB7C(s->card.id);
        }
    }
    if (!(flags & 4)) {
        if (b == 0) {
            if (v == duel_view_player) {
                duel_0884DB38(0x15, 1);
            }
            s->flags |= 0x10000;
        } else {
            if (v == duel_view_player) {
                duel_0884DB38(0x16, 1);
            }
            s->flags |= 0x20000;
        }
        if (v == duel_view_player && a != 0) {
            duel_0884DB38(0x18, 1);
        }
    } else {
        s->flags |= 8;
        s->flags |= 0x40000;
        if (v == duel_view_player) {
            duel_0884DB38(0x19, 1);
            if (s->faceup == 0) {
                duel_0884DB38(0x1C, 1);
            }
            if (card_is_attribute9(s->card.id) != 0) {
                duel_0884DB38(0x17, 1);
            }
        }
    }
    duel_088DF554(p, z, 1, 0);
    duel_host_call(0x24, 0, 0, 0);
    duel_busy_0891523C = 0;
}
#endif

void duel_req_088D2C14(void)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned short z = duel_req.req.arg;

    duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
    duel_busy_0891523C = 0;
}

#ifdef NON_MATCHING /* p e arg trocam s1/s2 */
/* Copy the card with uid w into entry loc of the pick list. */
void duel_req_088D2C9C(void)
{
    int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int arg = duel_req.req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.req.loc;
    unsigned int w = duel_req.req.w;

    duel_w_0891524C.c[loc] = *duel_card_by_uid(w);
    duel_host_call(0x2B, p, arg, (unsigned short)w | (unsigned short)loc << 16);
    duel_busy_0891523C = 0;
}
#endif

#ifdef NON_MATCHING /* a montagem do DuelCard temporário e a alocação dos campos do pedido não reproduzidas */
/* Show the card with uid w (taken from zone arg) as entry loc of the pick list. */
void duel_req_088D2D40(void)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int kind = duel_req.req.arg;
    unsigned int i = *(unsigned short *)&duel_req.req.loc;
    unsigned int uid = duel_req.req.w;
    DuelCard *c;

    switch (duel_req_step) {
    case 0:
        c = duel_card_by_uid(uid);
        if (c != 0) {
            duel_w_0891524C.c[i] = *c;
        } else {
            DuelCard t = {0};

            t.id = duel_cards[uid].id;
            t.owner = uid & 1;
            t.uid_hi = (uid & 0x1FF) >> 1;
            duel_w_0891524C.c[i] = t;
        }
        if (kind == 0x10) {
            if (((duel_card_locate(uid) & 0xFFFF) >> 8) - 0xE < 2) {
                c = duel_card_by_uid(uid);
                c->b15 = 1;
                c->b16 = 0;
            }
            duel_busy_0891523C = 0;
            return;
        }
        duel_req_step++;
        if (kind == 0xE && duel.players[p & 1].n_grave - 1 != duel_grave_find_uid(p, uid)) {
            duel_grave_to_top(p, uid);
            duel_host_call(0x16, p, 0xE, uid);
            return;
        }
    case 1:
        if (duel_req_flag(0x16) == 0) {
            duel_host_call(0x2D, p, kind, i << 16 | uid);
            duel_req_step++;
        }
        return;
    case 2:
        if (duel_req_flag(0x2D) != 0) {
            return;
        }
        duel_busy_0891523C = 0;
        return;
    }
}
#endif

/* Show the w cards of the list to player p (zone loc) and mark them. */
void duel_req_088D2FD8(void)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int arg = duel_req.req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.req.loc;
    unsigned int n = duel_req.req.w;
    int i;

    switch (duel_req_step) {
    case 0:
        duel_host_call(0x41, p, arg, 0);
        duel_req_step++;
        break;
    case 1:
        if (duel_req_flag(0x41) == 0) {
            duel_host_call(0x35, p, loc, n);
            for (i = 0; i < (int)n; i++) {
                duel_list_effect_add(loc, 3, 0x12E5, CARD_UID(*duel_0884C7EC(i)));
            }
            duel_req_step++;
        }
        break;
    default:
        if (duel_req_flag(0x35) == 0) {
            if (p == duel_view_player) {
                duel_0884DB38(0x23, 1);
            }
            duel_busy_0891523C = 0;
        }
        break;
    }
}

void duel_req_088D3144(void)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int arg = duel_req.req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.req.loc;
    unsigned int w = duel_req.req.w;

    if (duel_req_step == 0) {
        duel_host_call(0x40, p, arg, (unsigned short)((w & 0xFF) | (loc & 0xFF) << 8));
        duel_req_step++;
    }
    if (duel_req_flag(0x40) == 0) {
        duel_host_call(8, 0, 0, 0);
        duel_busy_0891523C = 0;
    }
}

#ifdef NON_MATCHING /* laços com o desenrolamento do compilador e cópias por duel_memcpy; mantida em asm */
/* Shuffle p's monsters among the zones of the mask w (the seed is arg | loc << 16). */
void duel_req_088D31F0(void)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int mask = duel_req.req.w;
    int to[5];
    int from[5];
    DuelSlot tmp[5];
    int n = 0;
    int z;
    int i;
    int j;
    int t;

    switch (duel_req_step) {
    case 0:
        for (z = 0; z < 5; z++) {
            if ((int)mask >> z & 1) {
                to[n] = z;
                from[n] = z;
                n++;
            }
        }
        if (n < 2) {
            duel_busy_0891523C = 0;
            return;
        }
        duel_089195C8 = duel_req.req.arg | *(unsigned short *)&duel_req.req.loc << 16;
        for (i = n - 1; i > 0; i--) {
            j = duel_rand(i + 1);
            t = to[i];
            to[i] = to[j];
            to[j] = t;
        }
        for (i = 0; i < n; i++) {
            duel_effects_move_src(p | from[i] << 8, (p + 2) | from[i] << 8);
        }
        for (i = 0; i < n; i++) {
            duel_memcpy(&tmp[from[i]], &duel.players[p].field[from[i]], sizeof(DuelSlot));
        }
        for (i = 0; i < n; i++) {
            duel_memcpy(&duel.players[p].field[to[i]], &tmp[from[i]], sizeof(DuelSlot));
        }
        for (i = 0; i < n; i++) {
            duel_effects_move_src((p + 2) | from[i] << 8, p | to[i] << 8);
        }
        for (i = 0; i < n; i++) {
            duel.players[p].field[to[i]].flags &= ~0x80;
        }
        duel_host_call(0x27, p, mask, 0);
        duel_req_step++;
        break;
    case 1:
        if (duel_req_flag(0x27) != 0) {
            return;
        }
        duel_busy_0891523C = 0;
        break;
    }
}
#endif

void duel_req_088D36C8(void)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int arg = duel_req.req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.req.loc;

    if (duel_req_step == 0) {
        duel_host_call(0x45, p, arg, loc);
        duel_req_step++;
    }
    if (duel_req_flag(0x45) == 0) {
        duel_busy_0891523C = 0;
    }
}

#ifdef NON_MATCHING /* ordem dos loads dos campos do pedido e dos stores não reproduzida */
/* Show the monster at (p, loc) changing position (w == 2: with its position and face) or being flipped. */
void duel_req_088D3744(void)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int z = *(unsigned short *)&duel_req.req.loc;
    short kind = duel_req.req.w;

    switch (duel_req_step) {
    case 0:
        duel_req_step = 1;
    case 1:
        if (duel_req_flag(-1) == 0) {
            if (kind == 2) {
                duel_host_call(0x34, (duel.players[p & 1].field[z].pos & 1) << 15 | (duel.players[p & 1].field[z].faceup & 1) << 14 |
                               p | (z & 0x1F) << 1,
                               CARD_UID(duel.players[p & 1].field[z].card), 0);
            } else {
                duel_host_call(0x33, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            }
            duel_req_step++;
        }
        break;
    case 2:
        if (duel_req_flag(kind == 2 ? 0x34 : 0x33) == 0) {
            duel_busy_0891523C = 0;
        }
        break;
    }
}
#endif

#ifdef NON_MATCHING /* montagem dos DuelLoc de origem e destino; mantida em asm */
/* Move the card with uid loc to the monster zone (arg: player | zone << 8; w: face | pos << 8). */
void duel_req_088D392C(void)
{
    unsigned int arg = duel_req.req.arg;
    unsigned int uid = *(unsigned short *)&duel_req.req.loc;
    unsigned int p = arg & 0xFF;
    unsigned int z = arg >> 8;
    unsigned int w = duel_req.req.w;
    unsigned int pos = w >> 8;
    unsigned int at;
    unsigned int zone;
    unsigned int i;
    DuelCard *c;
    DuelLoc from;
    DuelLoc to;

    switch (duel_req_step) {
    case 0:
        if (duel_req_flag(-1) != 0) {
            break;
        }
        at = duel_card_locate(uid);
        zone = (at & 0xFFFF) >> 8;
        duel_req_count = zone;
        c = duel_field_card_deb2c(at & 0xFF, zone, at >> 16);
        c->b19 = 0;
        c->b17 = 0;
        c->b20 = 0;
        c->b21 = 0;
        duel_card_copy(&duel_card_tmp, c);
        if (p == duel_view_player && zone == 0xE) {
            duel_0884DB38(0x1B, 1);
        }
        i = at >> 16 & 0xFF;
        *(unsigned short *)&from = 0;
        from.player = at & 1;
        from.zone = zone;
        from.idx = i;
        from.b15 = duel_zone_pos(from.player, from.zone, i);
        from.b14 = duel_zone_faceup(from.player, from.zone, from.idx);
        *(unsigned short *)&to = 0;
        to.player = arg & 1;
        to.zone = z;
        to.b15 = pos;
        to.b14 = w;
        if (from.zone < 11) {
            duel.list[uid].b14 = duel.list[uid].b14 - 1;
        }
        duel_card_take(from.player, from.zone, from.idx);
        duel_card_put(to.player, to.zone, (w & 0xFF) | pos << 8, &duel_card_tmp);
        duel_088DF554(p, z, 1, 1);
        duel_host_call(0x18, CARD_UID(duel_card_tmp) | 0x20000, *(unsigned short *)&from, *(unsigned short *)&to);
        duel_088DF554(p, z, 7, 1);
        duel_req_step++;
        break;
    case 1:
        if (duel_phase == 2) {
            duel.players[duel_turn_player & 1].flags |= 0x100000;
        }
        if (duel_req_count == 0xB && duel_slot_has_effect(p, 0xB, 0x15C7) != 0) {
            duel_slot_effect_add(p, z, 1, 0x15C7, duel_slot_counter(p, 0xB, 0x15C7));
        }
        duel_host_call(0x34, (pos & 1) << 15 | (w & 1) << 14 | (arg & 1) | (z & 0x1F) << 1,
                       CARD_UID(duel.players[arg & 1].field[z].card), 0);
        duel_req_step++;
        break;
    case 2:
        if (duel_req_flag(0x34) == 0) {
            duel_busy_0891523C = 0;
        }
        break;
    }
}
#endif

#ifdef NON_MATCHING /* montagem do DuelCard temporário; mantida em asm */
/* Put a new token (card id loc) in p's monster zone (arg >> 8: up | pos << 1 | zone << 4). */
void duel_req_088D3E5C(void)
{
    unsigned int id = *(unsigned short *)&duel_req.req.loc;
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int b = duel_req.req.arg >> 8;
    int z = (int)(b & 0xF0) >> 4;
    int pos = (int)(b & 2) >> 1;
    unsigned int up = b & 1;
    unsigned int owner;
    DuelCard c;

    switch (duel_req_step) {
    case 0:
        owner = p;
        if (id == 0x1672 || id == 0x1671 || id == 0x1670) {
            owner = 1 - p;
        }
        *(unsigned int *)&c = 0;
        c.id = id;
        c.owner = owner;
        c.uid_hi = duel_uid_next();
        duel_cards[CARD_UID(c)].id = id;
        duel_place_monster(p, z, &c, pos, up);
        duel.players[p].field[z].flags |= 0x200000;
        duel.players[p].field[z].flags |= 0x200008;
        duel_host_call(0x1B, pos << 15 | up << 14 | p | z << 1, id, CARD_UID(duel.players[p].field[z].card));
        duel_req_step++;
        break;
    case 1:
        duel_host_call(0x34, pos << 15 | up << 14 | p | z << 1, CARD_UID(duel.players[p].field[z].card), 0);
        duel_req_step++;
        break;
    case 2:
        if (duel_req_flag(0x34) == 0) {
            duel_busy_0891523C = 0;
        }
        break;
    }
}
#endif

#ifdef NON_MATCHING /* ordem dos loads dos campos do pedido e dos stores não reproduzida */
/* Put loc (at most the current minimum) 0x1130 counters on the card at (p, arg). */
void duel_req_088D416C(void)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int z = duel_req.req.arg;
    unsigned int n = duel_slot_min_1130(p, z);

    if (n == 0 || (int)*(unsigned short *)&duel_req.req.loc < (int)n) {
        n = *(unsigned short *)&duel_req.req.loc;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        duel_busy_0891523C = 0;
    } else if (duel_req_step == 1) {
        if (duel_req_flag(0x26) == 0) {
            duel_busy_0891523C = 0;
        }
    } else if (duel_req_step == 0) {
        duel_effects_free_card(duel.players[p & 1].field[z].effects, 0x1130);
        duel_slot_effect_add(p, z, 3, 0x1130, n);
        duel_host_call(0x26, p, z, n);
        duel_req_step++;
    }
}
#endif

#ifdef NON_MATCHING /* ordem dos loads dos campos do pedido e dos stores não reproduzida */
/* Player p loses the duel (arg: reason, 0xB without a message). */
void duel_req_088D42C8(void)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int reason = duel_req.req.arg;

    switch (duel_req_step) {
    case 0:
        if (reason != 0xB) {
            duel_host_call(0x30, p, reason, 0);
        }
        duel_host_call(0xD, p, 0, 0);
        duel_req_step++;
        break;
    case 1:
        if (reason == 0xB || duel_req_flag(0x30) == 0) {
            duel_w_089195E4 = 1;
            duel.players[p & 1].lp = 0;
            duel.players[(1 - p) & 1].unk20[3] = reason;
            duel_w_08917DC4 = 1;
            if (p == duel_view_player) {
                duel_w_089195E4 = 2;
            }
            duel_busy_0891523C = 0;
        }
        break;
    }
}
#endif

#ifdef NON_MATCHING /* passos de pedido com muitos registros de estatística; mantida em asm */
/* Change the LP of p by arg (gain when gain != 0, else loss; loc: 0 battle, 1 effect, 2 cost;
   w: card causing it), with the statistics of the duel record. */
void duel_req_088D43F8(int gain)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    int d = gain == 0 ? -(int)duel_req.req.arg : duel_req.req.arg;
    unsigned int how = *(unsigned short *)&duel_req.req.loc;
    unsigned int id = duel_req.req.w;
    int lp;
    int n;

    switch (duel_req_step) {
    case 0:
        duel_host_call(0xE, p, d, (how & 0xFF) << 8 | 1);
        if (p == (duel_view_player ^ 1) && d < 0) {
            n = -d;
            if (how == 1) {
                duel_0884DAF0(3, n);
                duel_0884DB38(4, n);
                duel_0884DB38(10, n);
                duel_0884DAF0(9, duel_stat_get(10));
                if (duel_atk_direct != 0) {
                    duel_0884DB38(7, n);
                    if (card_is_attribute6(id) != 0) {
                        duel_0884DB38(0xB, n);
                    }
                }
                if (duel_turn_player == (duel_view_player ^ 1)) {
                    duel_0884DAF0(5, n);
                    duel_0884DB38(6, n);
                }
                if (duel.players[p].lp > 0 && duel.players[p].lp <= n) {
                    if (p == (duel_view_player ^ 1)) {
                        duel_0884DA24(8, n);
                    }
                    switch (id) {
                    case 0x16EC:
                    case 0x1725:
                    case 0x1729:
                    case 0x1985:
                    case 0x1986:
                    case 0x1B2E:
                    case 0x1B2F:
                        if (duel_atk_direct != 0) {
                            duel.players[(1 - p) & 1].unk20[2] = id;
                        }
                        break;
                    case 0x1386:
                    case 4999:
                    case 5000:
                        duel_0884DA24(0x3E, 1);
                        break;
                    case 0xFBE:
                        duel_0884DA24(0x3F, 1);
                        break;
                    }
                }
            } else if (how == 0) {
                duel_0884DAF0(1, n);
                duel_0884DB38(2, n);
                duel_0884DB38(10, n);
                duel_0884DAF0(9, duel_stat_get(10));
                if (duel.players[p].lp > 0 && duel.players[p].lp <= n && p == (duel_view_player ^ 1)) {
                    duel_0884DA24(8, n);
                }
            }
            duel_0884DA24(0x37, -d == duel.players[p].lp);
        }
        if (p == duel_view_player && d < 0) {
            duel_0884DA24(0x36, 1);
        }
        if (p == duel_view_player && d > 0) {
            duel_0884DB38(0x12, d);
        }
        duel_req_step++;
        break;
    case 1:
        if (duel_req_flag(0xE) == 0) {
            duel_host_call(0x49, p, d, how);
            duel_req_count = 0;
            duel_req_step++;
        }
        break;
    case 2:
        if (duel_req_flag(0x49) == 0 && ++duel_req_count > 10) {
            duel_req_step++;
        }
        break;
    case 3:
        duel_host_call(0xE, p, d, (how & 0xFF) << 8);
        duel_req_step++;
        break;
    case 4:
        if (duel_req_flag(0xE) == 0) {
            if (d < 0) {
                lp = duel.players[p].lp + d;
                if (lp < 0) {
                    lp = 0;
                }
                duel.players[p].lp = lp;
            } else if (duel.players[p].lp != 0) {
                duel.players[p].lp += d;
            }
            if (how == 2 && p == duel_turn_player) {
                duel_slot_effect_add(p, 0xB, 0x21, 0x14C0, -d);
            }
            duel_0884DAF0(0x13, duel.players[duel_view_player & 1].lp);
            duel_0884DAF0(0x14, duel.players[(duel_view_player ^ 1) & 1].lp - duel.players[duel_view_player & 1].lp);
            duel_host_call(8, 0, 0, 0);
            duel_busy_0891523C = 0;
        }
        break;
    }
}
#endif

void duel_req_088D43F8(int gain);

void duel_req_lp_gain(void)
{
    duel_req_088D43F8(1);
}

void duel_req_lp_loss(void)
{
    duel_req_088D43F8(0);
}

/* Set the LP of p to arg. */
void duel_req_088D4A14(void)
{
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int v = duel_req.req.arg;
    int old;
    int me;

    switch (duel_req_step) {
    case 0:
        duel_host_call(0xD, p, v, 0);
        duel_req_step++;
        break;
    case 1:
        if (duel_req_flag(0xD) == 0) {
            old = duel.players[p & 1].lp;
            me = p == duel_view_player;
            duel.players[p & 1].lp = v;
            if (me && duel.players[p & 1].lp < old) {
                duel_0884DA24(0x36, 1);
            }
            duel_0884DAF0(0x13, duel.players[duel_view_player & 1].lp);
            duel_0884DAF0(0x14, duel.players[(duel_view_player ^ 1) & 1].lp - duel.players[duel_view_player & 1].lp);
            duel_busy_0891523C = 0;
        }
        break;
    }
}

/* Clear the damage record of the requesting player. */
void duel_req_088D4B7C(void)
{
    int i;
    unsigned int p = (duel_req.req.id & 0x8000) != 0;

    for (i = 0; i < 2; i++) {
        if (duel_special.recs[i].b[0] == p) {
            duel_special.recs[i].w0 = 0;
            break;
        }
    }
    duel_host_call(0xF, 0, 0, 0);
    duel_busy_0891523C = 0;
}

/* Queue a command for the presentation (and the replay log). */
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c)
{
    if (duel_player_type[0] == 3) {
        return;
    }
    if (duel_req.n < 256) {
        DUEL_CMDQ[duel_req.n].v[0] = cmd;
        DUEL_CMDQ[duel_req.n].v[1] = a;
        DUEL_CMDQ[duel_req.n].v[2] = b;
        DUEL_CMDQ[duel_req.n].v[3] = c;
        duel_log_push(&DUEL_CMDQ[duel_req.n]);
        duel_req.n++;
    }
}

void duel_088D4CD8(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c)
{
    if (duel_req.n < 256) {
        DUEL_CMDQ[duel_req.n].v[0] = cmd;
        DUEL_CMDQ[duel_req.n].v[1] = a;
        DUEL_CMDQ[duel_req.n].v[2] = b;
        DUEL_CMDQ[duel_req.n].v[3] = c;
        duel_req.n++;
    }
}

/* Send a message to the presentation outside the queue. */
void duel_088D4D70(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c)
{
    duel_msg_last.v[0] = cmd;
    duel_msg_last.v[1] = a;
    duel_msg_last.v[2] = b;
    duel_msg_last.v[3] = c;
    duel_msg_state.b2 = 1;
    duel_w_0891434C = 0;
    duel_log_push(&duel_msg_last);
}

/* Carry out one step of the request in duel_req (id & 0xFFF selects the handler). */
void duel_req_dispatch(void)
{
    duel_w_0891961C = 1;
    switch (duel_req.req.id & 0xFFF) {
    case 0x1:
        duel_req_host_1();
        break;
    case 0x2:
        duel_req_wait_5();
        break;
    case 0x3:
        duel_req_end_turn();
        break;
    case 0x4:
        duel_req_next_turn();
        break;
    case 0x5:
        duel_req_host_2();
        break;
    case 0x6:
        duel_req_host_3a();
        break;
    case 0x7:
        duel_req_host_3b_table();
        break;
    case 0x8:
        duel_req_host_3c();
        break;
    case 0x9:
        duel_req_mode_bit();
        break;
    case 0xA:
        duel_req_wait_frames();
        break;
    case 0xB:
        duel_req_host_3d();
        break;
    case 0xC:
        duel_req_phase_0();
        break;
    case 0xD:
        duel_req_phase_1();
        break;
    case 0xE:
        duel_req_phase_2();
        break;
    case 0xF:
        duel_req_phase_3();
        break;
    case 0x10:
        duel_req_phase_4();
        break;
    case 0x11:
        duel_req_phase_5();
        break;
    case 0x12:
        duel_req_host_2e();
        break;
    case 0x13:
        duel_req_host_2f();
        break;
    case 0x14:
        duel_req_cur_card();
        break;
    case 0x15:
        duel_req_088D57E0();
        break;
    case 0x16:
        duel_req_088D613C();
        break;
    case 0x17:
        duel_req_088D5D2C();
        break;
    case 0x18:
        duel_req_088D5DEC();
        break;
    case 0x19:
        duel_req_088D5830();
        break;
    case 0x1A:
        duel_req_088D5BA0();
        break;
    case 0x1B:
        duel_req_088D5FC8();
        break;
    case 0x1C:
        duel_req_088D6168();
        break;
    case 0x1D:
        duel_req_088D624C();
        break;
    case 0x1E:
        duel_req_088D6294();
        break;
    case 0x1F:
        duel_req_088D6310();
        break;
    case 0x20:
        duel_req_088D6348();
        break;
    case 0x21:
        duel_req_088D63FC();
        break;
    case 0x22:
        duel_req_player_bit();
        break;
    case 0x23:
        duel_req_088D42C8();
        break;
    case 0x24:
        duel_req_lp_gain();
        break;
    case 0x25:
        duel_req_lp_loss();
        break;
    case 0x26:
        duel_req_088D4A14();
        break;
    case 0x27:
        duel_req_088D4B7C();
        break;
    case 0x28:
        duel_req_088D9080();
        break;
    case 0x29:
        duel_req_088D9248();
        break;
    case 0x2A:
        duel_req_slot_bit();
        break;
    case 0x2B:
        duel_req_host_36();
        break;
    case 0x2C:
        duel_req_088D3744();
        break;
    case 0x2D:
        duel_req_088D392C();
        break;
    case 0x2E:
        duel_req_activate();
        break;
    case 0x2F:
        duel_req_announce();
        break;
    case 0x30:
        duel_req_announce_field();
        break;
    case 0x31:
        duel_req_088D9504();
        break;
    case 0x32:
        duel_req_088D9DBC();
        break;
    case 0x33:
        duel_req_banish();
        break;
    case 0x34:
        duel_req_change_pos();
        break;
    case 0x35:
        duel_req_flip();
        break;
    case 0x36:
        duel_req_effect_add();
        break;
    case 0x37:
        duel_req_effect_free();
        break;
    case 0x38:
        duel_req_effect_set();
        break;
    case 0x39:
        duel_req_order();
        break;
    case 0x3A:
        duel_req_slot_value();
        break;
    case 0x3B:
        duel_req_counter();
        break;
    case 0x3C:
        duel_req_counters();
        break;
    case 0x3D:
        duel_req_move_zone();
        break;
    case 0x3E:
        duel_req_swap_zones();
        break;
    case 0x3F:
        duel_req_088D6708();
        break;
    case 0x40:
        duel_req_move();
        break;
    case 0x41:
        duel_req_move_card();
        break;
    case 0x42:
        duel_req_add_effect();
        break;
    case 0x43:
        duel_req_point();
        break;
    case 0x44:
        duel_req_set_card();
        break;
    case 0x45:
        duel_req_088D28F8();
        break;
    case 0x46:
        duel_req_send();
        break;
    case 0x47:
        duel_req_088D416C();
        break;
    case 0x48:
        duel_req_088D3E5C();
        break;
    case 0x49:
        duel_req_088D31F0();
        break;
    case 0x4A:
        duel_req_088D2C14();
        break;
    case 0x4B:
        duel_req_088D2C9C();
        break;
    case 0x4C:
        duel_req_088D2D40();
        break;
    case 0x4D:
        duel_req_088D2FD8();
        break;
    case 0x4E:
        duel_req_hand_shuffle();
        break;
    case 0x4F:
        duel_step_summon();
        break;
    case 0x50:
        duel_step_set_st();
        break;
    case 0x51:
        duel_req_hand_show();
        break;
    case 0x52:
        duel_req_hand_open();
        break;
    case 0x53:
        duel_req_hand_flash();
        break;
    case 0x54:
        duel_req_d_shuffle();
        break;
    case 0x55:
        duel_req_draw();
        break;
    case 0x56:
        duel_req_mill();
        break;
    case 0x57:
        duel_req_d_return();
        break;
    case 0x58:
        duel_req_host_15();
        break;
    case 0x59:
        duel_req_d_reverse();
        break;
    case 0x5A:
        duel_req_grave_top();
        break;
    case 0x5B:
        duel_req_swap_d_grave();
        break;
    case 0x5C:
        duel_req_host_37();
        break;
    case 0x5D:
        duel_req_host_38();
        break;
    case 0x5E:
        duel_req_host_39();
        break;
    case 0x5F:
        duel_req_host_3b();
        break;
    case 0x60:
        duel_req_088D3144();
        break;
    case 0x61:
        duel_req_088D36C8();
        break;
    case 0x62:
        duel_req_host_42();
        break;
    case 0x63:
        duel_req_done();
        break;
    case 0x64:
        duel_req_log_seek();
        break;
    case 0x65:
        duel_req_log_seek_if();
        break;
    case 0x66:
        duel_req_message();
        break;
    case 0x67:
        duel_req_host_3e();
        break;
    default:
        duel_busy_0891523C = 0;
        break;
    }
}

/* Carry out the message sent outside the queue (duel_msg_last); 1 while it is busy. */
int duel_req_088D5490(void)
{
    switch (duel_w_0891434C) {
    case 0:
        duel_memcpy(&duel_req.req, &duel_msg_last, 8);
        duel_busy_0891523C = 1;
        duel_req_step = 0;
        duel_w_0891434C++;
    case 1:
        duel_req_dispatch();
        if (duel_busy_0891523C != 0) {
            return 1;
        }
        duel_w_0891434C++;
    case 2:
        duel_msg_send3(8, 0, 0, 0);
        duel_msg_state.b2 = 0;
        return 1;
    }
    return 1;
}

/* Carry out the queued commands one by one; 1 while busy. */
int duel_req_088D557C(void)
{
    unsigned int i;

    if (duel_req_flag(0) != 0) {
        return 1;
    }
    if (duel_w_089195EC != 0 && duel_msg_state.b2) {
        return duel_req_088D5490();
    }
    switch (duel_req_queue_step) {
    case 0:
        if (duel_req.n == 0) {
            return 0;
        }
        duel_memcpy(&duel_req.req, &duel_req.q[0], 8);
        duel_req.n--;
        for (i = 0; i < duel_req.n; i++) {
            duel_memcpy(&duel_req.q[i], &duel_req.q[i + 1], 8);
        }
        duel_msg_state.b3 = 1;
        duel_busy_0891523C = 1;
        if (duel_w_089195EC != 0) {
            duel_msg_send(7, -1, &duel_req.req, 8);
            duel_msg_state.b3 = 0;
        }
        duel_req_step = 0;
        duel_req_count = 0;
        duel_req_queue_step++;
    case 1:
        duel_req_dispatch();
        if (duel_busy_0891523C != 0) {
            return 1;
        }
        duel_req_queue_step++;
    case 2:
        if (!duel_msg_state.b3) {
            return 1;
        }
        duel_req_queue_step = 0;
        if (duel_req.n != 0) {
            return 1;
        }
        if (duel_player_type[0] == 3) {
            if (duel_busy_0891523C == 0) {
                duel_0884D618(0);
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

void duel_req_088D57E0(void)
{
    duel_host_call(9, (duel_req.req.id & 0x8000) != 0, 0, 0);
    duel_host_call(0x24, 0, 0, 0);
    duel_busy_0891523C = 0;
}

#ifdef NON_MATCHING /* início de ataque com muitos campos de duel_special; mantida em asm */
/* An attack begins: the monster at (p, arg) attacks zone loc of player w. */
void duel_req_088D5830(void)
{
    unsigned int z = duel_req.req.arg;
    unsigned int p = (duel_req.req.id & 0x8000) != 0;
    unsigned int tz = *(unsigned short *)&duel_req.req.loc;
    unsigned int tp = duel_req.req.w;
    unsigned int q;
    int i;

    if (duel_req_step == 0) {
        duel_atk_direct = 0;
        duel_special.uid = CARD_UID(duel.players[p].field[z].card);
        duel_special.unk24 = 0;
        q = tp & 1;
        duel_special.player[0] = p;
        duel_atk_tp = tp;
        duel_atk_zone = z;
        duel_atk_tz = tz;
        duel_special.act[1 - p].uid = CARD_UID(duel.players[q].field[tz].card);
        duel_effects_free_card(duel.players[p].effects, 0x1846);
        if (duel.players[q].field[tz].faceup == 0 && duel.players[q].field[tz].pos != 0) {
            if (duel_slot_count_src(p, z, 0x1846) != 0) {
                duel_slot_effect_add(p, 0xB, 4, 0x1846, 0);
            }
            switch (field_get_card(p, z)) {
            case 0x178D:
                duel_slot_effect_add(p, 0xB, 4, 0x1846, 0);
                break;
            case 0x152D:
                if (p != tp) {
                    duel_slot_effect_add(p, 0xB, 4, 0x1846, 0);
                }
                break;
            case 0x1433:
                if (SLOT_ACTIVE(&duel.players[p].field[z]) &&
                    card_get_special_group(duel.players[q].field[tz].card.id) == 1) {
                    duel_slot_effect_add(tp, tz, 3, 0x1433, 0);
                }
                break;
            }
        }
        for (i = 0; i < 5; i++) {
            if (i != (int)z && field_get_card(p, i) == 0x1A79) {
                duel.players[p].field[i].link = 0;
            }
        }
        duel_player_flag_set(p, 0x12, 1);
        duel_mode_set(1, 1);
        duel_host_call(0xB, z, tp, tz);
        duel_req_step++;
    }
    if (duel_req_flag(0xB) == 0) {
        duel_busy_0891523C = 0;
    }
}
#endif
