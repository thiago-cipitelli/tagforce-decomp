// @range 0x088DC1A0-0x088DEB2C
// @asm 0x088DC4E0-0x088DCA4C
// @asm 0x088DD038-0x088DD1A4
// @asm 0x088DD660-0x088DD708
// @asm 0x088DD88C-0x088DD98C
// @asm 0x088DE360-0x088DEB2C
/* Steps of the requests the engine carries out one per frame (duel_req, duel_req_step): input log, messages,
   graveyard and hand moves, summoning and setting a card from the hand. */
#include "duel.h"

int duel_req_flag(int i);
int duel_slot_has_effect(unsigned int p, int z, int id);
void duel_088DF554(unsigned int p, int z, unsigned int bit, int on);

extern unsigned int duel_turn_player;
extern unsigned int duel_view_player;
extern int duel_busy_0891523C;

/* a field of duel.players[p & 1] addressed from the field of player 0 (offset folded into the address) */
#define PL_FIELD(T, f, p) (*(T *)((char *)&duel.players[0].f + ((p) & 1) * sizeof(DuelPlayer)))

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
extern unsigned int duel_req_step;                 /* 0x08915240 */
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

typedef struct DuelLogEnt {
    unsigned short v[4];
} DuelLogEnt;

void duel_log_rewind(void);
int duel_log_at_end(void);
void duel_log_next(void);
DuelLogEnt *duel_log_cur(void);
void duel_wfmt_s(unsigned short *dst, int fmt, unsigned short *arg);
void duel_0884AD18(unsigned short *msg);
void duel_0884ACD8(unsigned int p, int msg);
void duel_0884AD2C(unsigned short *msg, int kind);
int duel_0884B08C(void);
void duel_wcscpy(unsigned short *dst, unsigned short *src);
void duel_wcscat(unsigned short *dst, unsigned short *src);
unsigned short *card_get_name(int id);
int card_attribute_icon_is3(int id);
void duel_swap_d_grave(unsigned int p);
int duel_hand_shuffle(unsigned int p);
void duel_card_put(unsigned int p, int zone, unsigned int arg, DuelCard *c);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
void duel_player_flag_set(unsigned int p, unsigned int bit, int on);

extern int duel_w_08919624;
extern unsigned short duel_str_08913528[];
extern unsigned int duel_089195C8;
extern unsigned int duel_turn;         /* 0x089195D4 = duel.turn */
extern int duel_phase;                 /* 0x089195DC = duel.phase */
extern int duel_w_089195FC;
extern int duel_w_08919600;

void duel_0884CD24(void);
void duel_0884DA24(int a, int b);

extern int duel_ids_08911D2C[5];

void duel_effects_end_turn(void);
void duel_effects_tick(void);
void duel_0884DAF0(int a, int b);

void duel_d_shuffle(unsigned int p);
void duel_d_reverse(unsigned int p);
unsigned int duel_card_locate(unsigned int uid);
int duel_d_remove(unsigned int p, DuelCard *c);
void duel_d_push_top(unsigned int p, DuelCard *c);
void duel_d_push(unsigned int p, DuelCard *c);
void duel_mode_set(unsigned int bit, int on);

void duel_draw(unsigned int p);
unsigned int duel_zone_faceup(unsigned int p, int z, int i);

/* Draw loc cards (arg: show the draw even in phase 7). */
void duel_req_draw(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;
    DuelLoc from;
    DuelLoc to;

    switch (duel_req_step) {
    case 0:
        if (*(unsigned short *)&duel_req.loc == 0) {
            duel_busy_0891523C = 0;
            break;
        }
        if (PL_FIELD(int, n_d, p) == 0) {
            PL_FIELD(int, unk20[0], p) = 1;
            duel_busy_0891523C = 0;
            break;
        }
        duel_req_step++;
        if (duel_phase != 7 || arg != 0) {
            duel_host_call(0x3F, p, *(unsigned short *)&duel_req.loc, arg);
        }
        break;
    case 1:
        if (duel_req_flag(0x3F) != 0) {
            break;
        }
        if (PL_FIELD(int, n_d, p) == 0) {
            PL_FIELD(int, unk20[0], p) = 1;
            duel_busy_0891523C = 0;
            break;
        }
        duel_card_copy(&duel_card_tmp, &duel.players[p & 1].zone_d[0]);
        duel_draw(p);
        from.player = p;
        from.zone = 0xD;
        from.idx = 0;
        from.b15 = 0;
        from.b14 = 0;
        to.player = p;
        to.zone = 0xB;
        to.idx = PL_FIELD(unsigned int, n_hand, p) - 1;
        to.b15 = 0;
        to.b14 = duel_zone_faceup(to.player, to.zone, to.idx);
        duel_host_call(0x18, (unsigned short)CARD_UID(duel_card_tmp), *(unsigned short *)&from, *(unsigned short *)&to);
        duel_req_step++;
        break;
    case 2:
        (*(unsigned short *)&duel_req.loc)--;
        if ((int)*(unsigned short *)&duel_req.loc > 0) {
            duel_req_step--;
            break;
        }
        if (duel.mode >> 2 & 1) {
            duel_player_flag_set(p, 2, 1);
        }
        duel_busy_0891523C = 0;
        break;
    }
}

void duel_add_counters(unsigned int p, int id, int n);
void duel_set_flag4(unsigned int p, int id);

extern unsigned int duel_req_count;    /* 0x08915244 */

#ifdef NON_MATCHING /* draft: the s registers of the flags and the order of the location stores differ */
/* Send arg cards from the top of the deck to the graveyard, or banish them (loc bits: 1 always,
   2 monsters, 4 cards of player 0, 8 cards of player 1); w: show them. */
void duel_req_mill(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int n = duel_req.arg;
    unsigned int how = *(unsigned short *)&duel_req.loc;
    unsigned int show = duel_req.w;
    DuelCard *top = &duel.players[p & 1].zone_d[0];
    int uid = CARD_UID(*top);
    int banish = 0;
    DuelCard *c;
    DuelLoc from;
    DuelLoc to;

    if (how & 1) {
        banish = 1;
    }
    if ((how & 2) && card_is_monster(top->id)) {
        banish = 1;
    }
    if ((how & 4) && !(uid & 1)) {
        banish = 1;
    }
    if ((how & 8) && (uid & 1) == 1) {
        banish = 1;
    }
    switch (duel_req_step) {
    case 0:
        duel_req_count = n;
        if (n != 0) {
            duel_req_step++;
        } else {
            duel_busy_0891523C = 0;
        }
        break;
    case 1:
        if (duel.players[p & 1].n_d == 0) {
            duel_busy_0891523C = 0;
            break;
        }
        if (banish) {
            duel_host_call(0x1F, p, 0xD, CARD_UID(*top));
        }
        duel_req_step++;
        break;
    case 2:
        if (duel_req_flag(0x1F) != 0) {
            break;
        }
        c = duel_field_card_deb2c(p, 0xD, 0);
        c->b20 = 0;
        c->b18 = show;
        if (!banish) {
            duel_add_counters(0, 0x1361, 1 << (c->owner << 3));
            duel_add_counters(1, 0x1361, 1 << (c->owner << 3));
            if (card_is_monster(c->id)) {
                duel_set_flag4(1 - c->owner, 0x1574);
            }
        }
        if (banish) {
            duel_add_counters(0, 0x16DA, 1);
            duel_add_counters(1, 0x16DA, 1);
        }
        duel_card_copy(&duel_card_tmp, c);
        if (p == (duel_view_player ^ 1) && show != 0) {
            duel_0884DB38(0x29, 1);
            if (banish) {
                duel_0884DB38(0x26, 1);
                if (card_is_monster(duel_card_tmp.id)) {
                    duel_0884DB38(0x27, 1);
                }
            }
        }
        from.player = p;
        from.zone = 0xD;
        from.idx = 0;
        from.b15 = 0;
        from.b14 = 1;
        to.player = duel_card_tmp.owner;
        to.zone = banish ? 0xF : 0xE;
        to.idx = 1;
        to.b15 = 0;
        to.b14 = 1;
        duel_card_take(from.player, from.zone, from.idx);
        duel_card_put(to.player, to.zone, banish ? 0 : to.idx, &duel_card_tmp);
        duel_host_call(0x18, (unsigned short)CARD_UID(duel_card_tmp), *(unsigned short *)&from, *(unsigned short *)&to);
        duel_req_step++;
        break;
    case 3:
        if (--duel_req_count != 0) {
            duel_req_step--;
        } else {
            duel_req_step++;
        }
        break;
    default:
        duel_busy_0891523C = 0;
        break;
    }
}
#endif

void duel_req_d_shuffle(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = (duel_req.arg & 0xFFFF) | *(unsigned short *)&duel_req.loc << 16;

    switch (duel_req_step) {
    case 0:
        if ((unsigned int)duel.players[p & 1].n_d <= 1) {
            duel_busy_0891523C = 0;
        } else {
            duel_host_call(0x13, p, 0, 0);
            duel_req_step++;
        }
        break;
    case 1:
        if (duel_req_flag(0x13) == 0) {
            duel_089195C8 = arg;
            duel_d_shuffle(p);
            duel_host_call(0x14, p, 0xD, 0);
            duel_busy_0891523C = 0;
        }
        break;
    }
}

void duel_req_d_reverse(void)
{
    unsigned int arg = duel_req.arg;

    switch (duel_req_step) {
    case 0:
        duel_mode_set(2, arg);
        duel_d_reverse(0);
        duel_d_reverse(1);
        duel_host_call(0x14, 0, 0xD, 0);
        duel_host_call(0x14, 1, 0xD, 0);
        duel_busy_0891523C = 0;
        break;
    }
}

/* Put the card with uid back on top (w 0) or at the bottom of player p's deck. */
void duel_req_d_return(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int zone = duel_req.arg;
    unsigned int w = duel_req.w;
    unsigned int t = duel_card_locate(*(unsigned short *)&duel_req.loc);
    unsigned int z = (unsigned char)((t & 0xFFFF) >> 8);

    if (z == zone) {
        DuelCard c = *duel_field_card_deb2c(t & 0xFF, z, (unsigned short)(t >> 16));

        duel_d_remove(p, &c);
        if (w != 0) {
            duel_d_push(p, &c);
        } else {
            duel_d_push_top(p, &c);
        }
        duel_host_call(0x14, p, 0xD, 0);
    }
    duel_busy_0891523C = 0;
}

void duel_req_host_15(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;

    switch (duel_req_step) {
    case 0:
        duel_host_call(0x15, p, arg, 0);
        duel_req_step++;
        break;
    case 1:
        if (duel_req_flag(0x15) == 0) {
            duel_busy_0891523C = 0;
        }
        break;
    }
}

void duel_req_host_1(void)
{
    if (duel_req_step == 0) {
        duel_host_call(1, 0, 0, 0);
        duel_req_step++;
    } else if (duel_req_flag(1) == 0) {
        duel_busy_0891523C = 0;
    }
}

/* Coin or dice result arg (seen from the viewer when the request is the opponent's). */
void duel_req_host_2(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int r = duel_req.arg;

    if (duel_req_step == 0) {
        if (p == (duel_view_player ^ 1)) {
            switch (r) {
            case 1:
                r = 2;
                break;
            case 2:
                r = 1;
                break;
            }
            duel.unk3788 = r;
            duel.players[0].unk20[3] = *(unsigned short *)&duel_req.loc;
            duel.players[1].unk20[3] = duel_req.w;
        }
        duel_host_call(2, r, 0, 0);
        duel_req_step++;
    } else if (duel_req_flag(2) == 0) {
        duel_busy_0891523C = 0;
    }
}

/* Next turn. */
void duel_req_next_turn(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;

    duel_turn_player ^= 1;
    duel_turn++;
    duel_w_089195FC = 0;
    duel_w_08919600 = 0;
    duel_phase = 7;
    duel_0884DA24(0x38, duel.players[duel_view_player & 1].lp < duel.players[(duel_view_player ^ 1) & 1].lp);
    duel_host_call(5, 1 - p, 0, 0);
    duel_busy_0891523C = 0;
}

/* Enter phase ph. */
void duel_req_phase(int ph)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;

    if (duel_req_step == 0) {
        duel_host_call(4, p, ph, 0);
        duel.phase = ph;
        duel_effects_end_turn();
        duel_req_step++;
    } else if (duel_req_flag(4) == 0) {
        duel_host_call(0x24, 0, 0, 0);
        duel_busy_0891523C = 0;
    }
}

void duel_req_phase_0(void)
{
    duel_req_phase(0);
}

void duel_req_phase_1(void)
{
    duel_req_phase(1);
}

void duel_req_phase_2(void)
{
    duel_req_phase(2);
}

void duel_req_phase_3(void)
{
    duel_req_phase(3);
}

void duel_req_phase_4(void)
{
    duel_req_phase(4);
}

void duel_req_phase_5(void)
{
    duel_req_phase(5);
}

void duel_req_wait_5(void)
{
    if (duel_req_flag(5) == 0) {
        duel_0884DA24(10, 0);
        duel_busy_0891523C = 0;
    }
}

#ifdef NON_MATCHING /* the pointer and counter registers of the two graveyard loops are swapped */
/* End of turn: clear the high flags of every zone and of the mode, mark the graveyard and zone F
   cards as old, count down the timed effects. */
void duel_req_end_turn(void)
{
    unsigned int p;
    int z;
    unsigned int i;
    unsigned int j;

    duel_host_call(0x48, (duel_req.id & 0x8000) != 0, 0, 0);
    for (p = 0; (int)p < 2; p++) {
        for (z = 0; z <= 11; z++) {
            duel.players[p & 1].field[z].flags = (unsigned short)duel.players[p & 1].field[z].flags;
        }
    }
    duel.mode &= 0xFFFF;
    for (p = 0; (int)p < 2; p++) {
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            duel.players[p & 1].grave[i].b20 = 1;
        }
        for (j = 0; j < duel.players[p & 1].n_f; j++) {
            duel.players[p & 1].zone_f[j].b20 = 1;
        }
    }
    duel_effects_tick();
    duel_busy_0891523C = 0;
}
#endif

void duel_req_wait_frames(void)
{
    if (duel_req_step++ >= duel_req.arg) {
        duel_busy_0891523C = 0;
    }
}

void duel_req_mode_bit(void)
{
    unsigned int bit = duel_req.arg;
    unsigned int on = *(unsigned short *)&duel_req.loc;

    if (on != 0) {
        duel.mode |= 1 << bit;
    } else {
        duel.mode &= ~(1 << bit);
    }
    duel_host_call(0x24, 0, 0, 0);
    duel_busy_0891523C = 0;
}

void duel_req_player_bit(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int bit = duel_req.arg;
    unsigned int on = *(unsigned short *)&duel_req.loc;

    if (on != 0) {
        duel.players[p & 1].flags |= 1 << bit;
    } else {
        duel.players[p & 1].flags &= ~(1 << bit);
    }
    duel_host_call(0x24, 0, 0, 0);
    duel_busy_0891523C = 0;
}

void duel_req_slot_bit(void)
{
    duel_088DF554((duel_req.id & 0x8000) != 0, duel_req.arg, *(unsigned short *)&duel_req.loc, duel_req.w);
    duel_host_call(0x24, 0, 0, 0);
    duel_busy_0891523C = 0;
}

void duel_req_cur_card(void)
{
    duel.cur_card = duel_req.arg;
    if (duel_req_step == 0) {
        duel_req_step++;
        duel_host_call(6, duel.cur_card, 0, 0);
    } else if (duel_req_flag(6) == 0) {
        duel_host_call(0x24, 0, 0, 0);
        duel_busy_0891523C = 0;
    }
}

void duel_req_host_42(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;

    if (duel_req_step == 0) {
        duel_host_call(0x42, p, arg, 0);
        duel_req_step++;
    } else if (duel_req_flag(0x42) == 0) {
        duel_busy_0891523C = 0;
    }
}

void duel_req_host_2e(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.loc;

    if (duel_req_step == 0) {
        duel_host_call(0x2E, p, arg, loc);
        duel_0884DAF0(0x25, loc);
        duel_req_step++;
    } else if (duel_req_flag(0x2E) == 0) {
        duel_busy_0891523C = 0;
    }
}

void duel_req_host_2f(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.loc;

    if (duel_req_step == 0) {
        duel_host_call(0x2F, p, arg, loc);
        duel_req_step++;
    } else if (duel_req_flag(0x2F) == 0) {
        duel_busy_0891523C = 0;
    }
}

void duel_req_host_37(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;
    unsigned int lo = *(unsigned short *)&duel_req.loc & 0xFF;
    unsigned int hi = *(unsigned short *)&duel_req.loc >> 8 & 0xFF;
    unsigned int w = duel_req.w;

    if (duel_req_step == 0) {
        duel_0884CD24();
        duel_host_call(0x37, (unsigned short)((p & 0xFF) | (arg & 0xFF) << 8), (unsigned short)((lo & 0xFF) | (hi & 0xFF) << 8), w);
        duel_req_step++;
    } else if (duel_req_flag(0x37) == 0) {
        duel_busy_0891523C = 0;
    }
}

#ifdef NON_MATCHING /* the registers of the player and of the location are swapped */
void duel_req_host_38(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.loc;

    if (duel_req_step == 0) {
        duel_0884CD24();
        duel_host_call(0x38, p, arg, loc);
        duel_req_step++;
    } else if (duel_req_flag(0x38) == 0) {
        duel_busy_0891523C = 0;
    }
}
#endif

void duel_req_host_3a(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;

    if (duel_req_step == 0) {
        duel_host_call(0x3A, p, 0, 0);
        duel_req_step++;
    } else if (duel_req_flag(0x3A) == 0) {
        duel_busy_0891523C = 0;
    }
}

void duel_req_host_39(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;

    if (duel_req_step == 0) {
        duel_host_call(0x39, p, arg, 0);
        duel_req_step++;
    } else if (duel_req_flag(0x39) == 0) {
        if (arg != 0) {
            duel_0884DA24(0x40, 1);
        }
        duel_busy_0891523C = 0;
    }
}

void duel_req_host_3b(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = duel_req.arg;
    unsigned int loc = *(unsigned short *)&duel_req.loc;

    if (duel_req_step == 0) {
        duel_host_call(0x3B, p, loc, arg);
        duel_req_step++;
    } else if (duel_req_flag(0x3B) == 0) {
        duel_busy_0891523C = 0;
    }
}

#ifdef NON_MATCHING /* the registers of the player and of the table pointer are swapped */
/* Point at the spell/trap zones holding the five cards of the table. */
void duel_req_host_3b_table(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int k;
    int *id;
    int z;

    if (duel_req_step == 0) {
        for (k = 0, id = duel_ids_08911D2C; (int)k < 5; k++, id++) {
            for (z = 5; z < 10; z++) {
                int c = duel.players[p & 1].field[z].card.id;

                if (c == *id) {
                    duel_host_call(0x3B, p, z, (unsigned short)((k & 0xFF) | 0x100));
                    break;
                }
            }
        }
        duel_req_step++;
    } else if (duel_req_flag(0x3B) == 0) {
        duel_busy_0891523C = 0;
    }
}
#endif

void duel_req_host_3c(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;

    if (duel_req_step == 0) {
        duel_host_call(0x3C, p, 0, 0);
        duel_req_step++;
    } else if (duel_req_flag(0x3C) == 0) {
        duel_busy_0891523C = 0;
    }
}

void duel_req_host_3d(void)
{
    unsigned int arg = duel_req.arg;

    if (duel_req_step == 0) {
        if (arg == 0) {
            duel.unk377c = duel.turn;
        }
        duel_host_call(0x3D, arg, 0, 0);
        duel_req_step++;
    } else if (duel_req_flag(0x3D) == 0) {
        duel_busy_0891523C = 0;
    }
}

void duel_req_host_36(void)
{
    duel_host_call(0x36, duel_req.arg, 0, 0);
    duel_busy_0891523C = 0;
}

void duel_req_done(void)
{
    duel_busy_0891523C = 0;
}

/* Rewind the input log to the entry 99 with the request's argument. */
void duel_req_log_seek(void)
{
    int arg = duel_req.arg;

    duel_log_rewind();
    while (!duel_log_at_end()) {
        duel_log_next();
        if (duel_log_cur()->v[0] == 99 && arg == duel_log_cur()->v[1]) {
            duel_busy_0891523C = 0;
            return;
        }
    }
    duel_log_rewind();
    duel_busy_0891523C = 0;
}

void duel_req_log_seek_if(void)
{
    if (duel_w_08919624 == *(unsigned short *)&duel_req.loc) {
        duel_req_log_seek();
    }
    duel_busy_0891523C = 0;
}

/* Show message loc (0xA9-0xCB: a text built with card name w). */
void duel_req_message(void)
{
    unsigned short buf[256];
    unsigned short buf2[256];
    int msg = *(unsigned short *)&duel_req.loc;
    int w = duel_req.w;

    if (duel_req_step == 0) {
        duel_req_step++;
        if (msg >= 0xA9 && msg <= 0xCB) {
            duel_wcscpy(buf, (unsigned short *)msg);
            duel_wcscat(buf, duel_str_08913528);
            duel_wcscat(buf, (unsigned short *)0xCC);
            duel_0884AD2C(buf, 3);
        } else if (msg == 0x8C) {
            duel_wfmt_s(buf2, msg, card_get_name(w));
            duel_0884AD18(buf2);
        } else {
            duel_0884ACD8(duel_view_player, msg);
        }
    }
    if (duel_0884B08C() == 0) {
        duel_busy_0891523C = 0;
    }
}

void duel_req_host_3e(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;

    if (duel_req_step == 0) {
        duel_host_call(0x3E, p, duel_req.arg, *(unsigned short *)&duel_req.loc | duel_req.w << 16);
        duel_req_step++;
    }
    if (duel_req_flag(0x3E) == 0) {
        duel_busy_0891523C = 0;
    }
}

/* Move the top card of player p's graveyard to zone 0xC or 0xD by its icon. */
void duel_req_grave_top(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    DuelLoc from;
    DuelLoc to;

    switch (duel_req_step) {
    case 0: {
        unsigned int *n = (unsigned int *)((char *)&duel.players[0].n_grave + (p & 1) * sizeof(DuelPlayer));

        if (*n != 0) {
            duel_card_copy(&duel_card_tmp, duel_field_card_deb2c(p, 0xE, *n - 1));
            from.player = p;
            from.zone = 0xE;
            from.idx = *n - 1;
            from.b15 = 0;
            from.b14 = 1;
            to.player = p;
            to.zone = card_attribute_icon_is3(duel_card_tmp.id) != 0 ? 0xC : 0xD;
            to.idx = 0;
            to.b15 = 0;
            to.b14 = 0;
            duel_card_take(p, 0xE, *n - 1);
            duel_card_put(to.player, to.zone, to.idx, &duel_card_tmp);
            duel_host_call(0x18, (unsigned short)CARD_UID(duel_card_tmp), *(unsigned short *)&from, *(unsigned short *)&to);
        } else {
            duel_busy_0891523C = 0;
        }
        break;
    }
    }
}

void duel_req_swap_d_grave(void)
{
    int p;

    switch (duel_req_step) {
    case 0:
        duel_swap_d_grave(0);
        duel_swap_d_grave(1);
        duel_req_step++;
        break;
    case 1:
        for (p = 0; p < 2; p++) {
            duel_host_call(0x14, p, 0xD, 0);
            duel_host_call(0x14, p, 0xE, 0);
        }
        duel_busy_0891523C = 0;
        break;
    }
}

void duel_req_hand_shuffle(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    unsigned int arg = (duel_req.arg & 0xFFFF) | *(unsigned short *)&duel_req.loc << 16;

    switch (duel_req_step) {
    case 0:
        if (duel_req_flag(-1) == 0) {
            duel_089195C8 = arg;
            if (duel_hand_shuffle(p)) {
                duel_host_call(0x10, p, 0, 0);
            }
            duel.players[p & 1].flags &= ~4;
            duel_req_step++;
        }
        break;
    case 1:
        if (duel_req_flag(0x10) == 0) {
            duel_busy_0891523C = 0;
        }
        break;
    }
}

void duel_req_hand_flash(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;

    if (duel.players[p & 1].n_hand <= 1) {
        duel_busy_0891523C = 0;
    } else if (duel_req_step <= 0x20) {
        unsigned int k = duel_req_step >> 1;

        duel_req_step++;
        duel_host_call(7, p, 0xB, k % duel.players[p & 1].n_hand);
    } else {
        duel_busy_0891523C = 0;
    }
}

void duel_req_hand_show(void)
{
    unsigned int p = (duel_req.id & 0x8000) != 0;
    int i = duel_hand_find_uid(p, duel_req.arg);

    switch (duel_req_step) {
    case 0:
        duel_req_step++;
        break;
    case 1:
        if (i >= 0) {
            duel_host_call(0x11, p, i, 0);
            duel.players[p & 1].flags |= 4;
        }
        duel_req_step++;
        break;
    case 2:
        if (duel_req_flag(0x11) == 0) {
            duel_busy_0891523C = 0;
        }
        break;
    }
}

void duel_req_hand_open(void)
{
    int arg;
    unsigned int p;

    p = (duel_req.id & 0x8000) != 0;
    arg = duel_req.arg;

    switch (duel_req_step) {
    case 0:
        duel_player_flag_set(p, 1, arg);
        duel_host_call(0x12, p, arg, 0);
        if (arg != 0) {
            duel.players[p & 1].flags |= 4;
        }
        duel_req_step++;
        break;
    case 1:
        if (duel_req_flag(0x12) == 0) {
            duel_busy_0891523C = 0;
        }
        break;
    }
}

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
