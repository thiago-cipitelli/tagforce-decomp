// @range 0x088D28F8-0x088D43F8
// @asm 0x088D28F8-0x088D2C14
// @asm 0x088D2C9C-0x088D2D40
// @asm 0x088D2D40-0x088D2FD8
// @asm 0x088D31F0-0x088D36C8
// @asm 0x088D3744-0x088D392C
// @asm 0x088D392C-0x088D3E5C
// @asm 0x088D3E5C-0x088D416C
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

extern DuelReq duel_req;               /* 0x08914A30 */
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

#ifdef NON_MATCHING /* o original guarda z * 20 e p * 0xFAC em registradores s além do ponteiro da zona; não reproduzido */
/* Reveal the card at (p, arg) (loc: bit 0 known, bit 2 flipped face up; w: low byte, high byte). */
void duel_req_088D28F8(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int z = duel_req.arg;
    unsigned int flags = *(unsigned short *)&duel_req.loc;
    unsigned int v = p;
    unsigned int w = duel_req.w;
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
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned short z = duel_req.arg;

    duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
    duel_busy_0891523C = 0;
}

#ifdef NON_MATCHING /* p e arg trocam s1/s2 */
/* Copy the card with uid w into entry loc of the pick list. */
void duel_req_088D2C9C(void)
{
    int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.loc;
    unsigned int w = duel_req.w;

    duel_w_0891524C.c[loc] = *duel_card_by_uid(w);
    duel_host_call(0x2B, p, arg, (unsigned short)w | (unsigned short)loc << 16);
    duel_busy_0891523C = 0;
}
#endif

#ifdef NON_MATCHING /* a montagem do DuelCard temporário e a alocação dos campos do pedido não reproduzidas */
/* Show the card with uid w (taken from zone arg) as entry loc of the pick list. */
void duel_req_088D2D40(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int kind = duel_req.arg;
    unsigned int i = *(unsigned short *)&duel_req.loc;
    unsigned int uid = duel_req.w;
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
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.loc;
    unsigned int n = duel_req.w;
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
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.loc;
    unsigned int w = duel_req.w;

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
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int mask = duel_req.w;
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
        duel_089195C8 = duel_req.arg | *(unsigned short *)&duel_req.loc << 16;
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
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.loc;

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
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int z = *(unsigned short *)&duel_req.loc;
    short kind = duel_req.w;

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
    unsigned int arg = duel_req.arg;
    unsigned int uid = *(unsigned short *)&duel_req.loc;
    unsigned int p = arg & 0xFF;
    unsigned int z = arg >> 8;
    unsigned int w = duel_req.w;
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
    unsigned int id = *(unsigned short *)&duel_req.loc;
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int b = duel_req.arg >> 8;
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
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int z = duel_req.arg;
    unsigned int n = duel_slot_min_1130(p, z);

    if (n == 0 || (int)*(unsigned short *)&duel_req.loc < (int)n) {
        n = *(unsigned short *)&duel_req.loc;
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
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int reason = duel_req.arg;

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
