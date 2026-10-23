// @range 0x08830E94-0x088324F0
// @asm 0x08830E94-0x0883136C
// @asm 0x08831590-0x08831C2C
// @asm 0x08831D68-0x08831FE8
// @data 0x089079C4-0x08907A18
/* Questions asked during an event, continued (see ai_0882FF4C.c), and duel_ev_run, which runs the
   handler of the pending event (duel_ev_kind). */
#include "duel.h"

extern int duel_player_type[2];
extern unsigned int duel_turn_player;
extern unsigned int duel_w_08919670;   /* player of the event */
extern int duel_ev_unk;                /* 0x08919678 step */
extern unsigned short duel_ev_data[8]; /* 0x0891967C */
extern unsigned short duel_w_0891968C; /* answer */
extern unsigned short duel_h_0891968E[2]; /* zones picked by each player (bit per zone) */
extern int duel_w_08919624;            /* answer of the last question */
extern int duel_w_0891964C;            /* picked player */
extern int duel_w_08919650;            /* picked zone */
extern int duel_w_08919654;

int duel_08835AC0(void);
int duel_host_call(int a, int b, unsigned int c, int d);
void duel_0881956C(unsigned int p, int id, int (*fn)());
void duel_08835978(int (*fn)());
int duel_0884ACD8(unsigned int p, int msg);
int duel_0884DF2C(int (*fn)());
void duel_088D1B20(unsigned int p, unsigned int mask);
int duel_tributable(unsigned int p, unsigned int q, int z);
unsigned int duel_field_type(unsigned int p, int z);
int duel_slot_ef098(unsigned int p, int z, int id);
int duel_slot_immune(unsigned int p, int z, int opp);
int duel_slot_ref_immune(unsigned int p, int z, int opp);
int card_is_monster(int id);
int card_is_attribute9(int id);
int card_is_in_group_1a58(int id);
int duel_can_attack_target(unsigned int p, int z, unsigned int q, int i, int arg);
extern int duel_atk_zone;              /* 0x08913F2C */
int duel_zone_card_uid(unsigned int p, int z, int i);
int duel_08809B50(unsigned int p, unsigned short id, unsigned short a);
DuelCard *duel_0884C7CC(void);
void duel_0884C588(unsigned int p, int a, int id, int b);
int duel_0880B448(unsigned int p, int id);
int duel_0880BD00(void);
int duel_0884AF34(int a, int b);
int duel_0884AF48(int a, int b, int c);
int duel_088302BC(unsigned int p, int z, int i);
int duel_08830358(unsigned int p, int z, int i);
int duel_0883042C(unsigned int p, int z, int i);
int duel_088304A0(unsigned int p, int z, int i);
int duel_08830590(unsigned int p, int z, int i);
int duel_08830664(unsigned int p, int z, int i);
int duel_08830728(unsigned int p, int z, int i);

int duel_0884AFD8(int a, int b, int c, int d);
int duel_088054A8(unsigned int p, int a, int b);
extern unsigned int duel_w_08913CF4;   /* player the computer is choosing for [?] */
int duel_rand(int n);
int duel_088BFFD8(unsigned int p, int id, int i);
int duel_count_free_monster_zones(unsigned int p);
DuelCard *duel_0884CCBC(int i);
int duel_0884AD18(int msg);
void duel_wfmt_s();
void duel_wfmt_d();
unsigned short *card_get_name(int id);
unsigned short *duel_08861480(int id, int i);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
void duel_088C448C(int p, int a);
void duel_088C44E4(unsigned int p, int id);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
extern unsigned int duel_view_player;
void duel_0884DB38(int a, int b);
void duel_0884DAF0(int a, int b);
void duel_0884DA24(int a, int b);
int duel_stat_get(int i);
int duel_count_monsters(unsigned int p);
int duel_0880B17C(unsigned int p, int i);
int duel_0880B0FC(void);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_0884AD40(int msg, int uid);
int duel_list_find_id(unsigned int p, unsigned int id);
unsigned short *dlg_table_sel(int i);
unsigned short *card_db_table_sel(int i);
typedef struct DuelEvFlags {
    unsigned char b0 : 1;
    unsigned char b1 : 1;              /* answer received from the other console */
    unsigned char b2 : 6;
} DuelEvFlags;
extern DuelEvFlags duel_ev_flags;      /* 0x08914341 */
extern int duel_ev_sent;               /* 0x08919668 */
extern int duel_ev_pending;            /* 0x0891966C */
extern int duel_ev_kind;               /* 0x08919674 */
void duel_msg_send(unsigned short cmd, int arg, void *data, int len);
int duel_08832D14();
int duel_088330E8();
int duel_088332E0();
int duel_08833638();
int duel_08833A88();
int duel_0883F1DC();
int duel_08830DC0();
int duel_08830030();
int duel_0883078C();
int duel_08830B7C();
int duel_08830CDC();
int duel_088309F8(unsigned int p, unsigned short id, unsigned short arg);
int duel_08831590();

#ifdef NON_MATCHING /* o original mantém o id em s1 (o mwcc o deixa em a2) e muda toda a alocação */
/* Pick cards of the hand or the deck (duel_0884C588) for the card of the event; the answers are
   the uids (duel.unk3832) or the card picked. */
int duel_08830E94(unsigned int p, int id)
{
    int arg = duel_ev_data[1];
    int n;
    int k;

    switch (duel_ev_unk) {
    case 0:
        switch (id) {
        case 0x132F:
        case 0x13ED:
        case 0x1717:
        case 0x17F4:
            duel_0884ACD8(p, 0xF6);
            break;
        case 0x14EE:
        case 0x1522:
        case 0x1531:
        case 0x166B:
        case 0x1746:
            if (duel_088BFFD8(p, id, arg) == 0) {
                duel_0884ACD8(p, 0xD);
                duel_w_0891968C = 0;
                duel_h_0891968E[0] = 0;
                return 1;
            }
            duel_0884ACD8(p, 0x121);
            break;
        case 0x1616:
            duel_0884ACD8(p, 0x5E);
            break;
        case 0x1712:
        case 0x1818:
        case 0x198E:
            duel_0884ACD8(p, 0x11D);
            break;
        case 0x1796:
            duel_0884ACD8(p, 0x16);
            break;
        case 0x14C2:
        case 0x19C0:
            duel_0884ACD8(p, 0x56);
            break;
        }
        duel_ev_unk++;
        return 0;
    case 1:
        switch (id) {
        case 0x132F:
        case 0x14C2:
        case 0x1717:
        case 0x1796:
        case 0x1818:
        case 0x19C0:
            duel_0884C588(p, 6, id, 1);
            break;
        case 0x13ED:
            duel_0884C588(p, 0x4A, 0x13ED, 0);
            break;
        case 0x14EE:
        case 0x1712:
            duel_0884C588(p, MIN(arg, 0x20) + 6, id, 0);
            break;
        case 0x15DD:
            duel_0884C588(p, MIN(arg, 0x20) + 0x27, 0x15DD, 0);
            break;
        case 0x1522:
        case 0x1531:
        case 0x1616:
        case 0x166B:
        case 0x1746:
        case 0x17F4:
            duel_0884C588(p, 6, id, arg);
            break;
        case 0x198E:
            n = duel_088BFFD8(p, 0x198E, arg);
            n = MIN(n, duel_count_free_monster_zones(p));
            n = MIN(n, 0x20);
            duel_0884C588(p, n + 6, 0x198E, arg);
            break;
        }
        duel_ev_unk++;
        return 0;
    case 2:
        switch (id) {
        case 0x14EE:
        case 0x15DD:
        case 0x1712:
        case 0x198E:
            n = duel_w_08919624;
            if ((unsigned int)n > 7) {
                n = 7;
            }
            duel_w_0891968C = n;
            for (k = 0; k < duel_w_0891968C; k++) {
                duel.unk3832[k] = duel_0884CCBC(k + 1)->owner + (duel_0884CCBC(k + 1)->uid_hi << 1);
            }
            return 1;
        }
        duel_w_0891968C = duel_0884C7CC()->id;
        duel_h_0891968E[0] = duel_0884C7CC()->owner + (duel_0884C7CC()->uid_hi << 1);
        return 1;
    }
    return 0;
}

#endif

int duel_0883136C(unsigned int p)
{
    return 1;
}

/* Pick a number: LP to pay (0x1766, in thousands), 0x159A (4 to 7), 0x14BE (the argument). */
int duel_08831374(unsigned int p, int id, int arg)
{
    int a;
    int b;

    switch (duel_ev_unk) {
    case 0:
        duel_ev_unk++;
        return 0;
    case 1:
        switch (id) {
        case 0x14BE:
            if (duel_player_type[p] == 1) {
                duel_w_08919624 = arg;
            } else {
                duel_0884AFD8(0x12D, 0x12F, 1, arg);
            }
            break;
        case 0x159A:
            if (duel_player_type[p] == 1) {
                duel_w_08919624 = duel_rand(4) + 4;
            } else {
                duel_0884AFD8(0x118, 0x130, 1, 0xC);
            }
            break;
        case 0x1766:
            if (duel_player_type[p] == 1) {
                a = (duel.players[p & 1].lp - 1) / 1000;
                duel_w_08913CF4 = p;
                b = (duel_088054A8(1 - p, -1, 0) + 999) / 1000;
                b = MAX(b, 3);
                a = MIN(a, b);
                duel_w_08919624 = a;
            } else {
                duel_0884AFD8(0x12D, 0x12E, 1, (duel.players[p & 1].lp - 1) / 1000);
            }
            break;
        }
        duel_ev_unk++;
        return 0;
    }
    duel_w_0891968C = duel_w_08919624;
    return 1;
}

#ifdef NON_MATCHING /* switch de ~30 casos com argumentos herdados dos registradores (a0 = jogador, a1 = id); não tentado a fundo */
/* Yes/no questions: the computer answers at once (duel_w_0891968C), the player gets the message of
   the card. */
int duel_08831590(unsigned int p, int id, unsigned int arg)
{
    unsigned short tmp[256];
    int vals[2];
    unsigned short msg[256];
    int r;
    int k;
    int *v;
    int bits;

    switch (duel_ev_unk) {
    case 0:
        if (duel_player_type[p] == 1) {
            switch (id) {
            case 0x1932:
                duel_w_0891968C = duel_turn_player != p;
                break;
            case 0x12FD:
                r = duel_turn_player != p;
                if (r) {
                    r = duel.players[p & 1].lp < 3000;
                }
                duel_w_0891968C = r;
                break;
            case 0x1300:
                r = duel_turn_player != p;
                if (r) {
                    r = duel_count_monsters(1 - p) < duel_count_monsters(p);
                }
                duel_w_0891968C = r;
                break;
            case 0x167A:
                duel_w_0891968C = duel.players[p & 1].lp < duel.players[(1 - p) & 1].lp;
                break;
            case 0x153F:
                duel_w_0891968C = duel_0880B17C(1 - p, arg);
                break;
            case 0x1544:
                duel_w_0891968C = !duel.players[p & 1].n_hand;
                break;
            case 0x140D:
                duel_w_0891968C = duel_0880B0FC();
                break;
            case 0xFE0:
            case 0x1488:
            case 0x15DD:
                duel_w_0891968C = 1;
                break;
            default:
                duel_w_0891968C = 0;
                break;
            }
            return 1;
        }
        duel_ev_unk = 1;
        return 0;
    case 1:
        switch (id) {
        case 0x198E:
            duel_0884ACD8(p, arg == 0 ? 0x122 : 0x141);
            break;
        case 0x1968:
            duel_0884ACD8(p, 0x1F);
            break;
        case 0x17C3:
            duel_0884AD18(0x2F);
            break;
        case 0x1739:
            v = vals;
            for (k = 1; k < 7; k++) {
                if (arg & 1 << k) {
                    *v++ = k;
                }
            }
            duel_wfmt_d(tmp, 0x129, vals[0]);
            duel_wfmt_d(msg, tmp, vals[1]);
            duel_0884ACD8(p, (int)msg);
            break;
        case 0x171F:
        case 0x19C2:
            duel_wfmt_s(msg, 0x108, dlg_table_sel(arg));
            duel_0884ACD8(p, (int)msg);
            break;
        case 0x1717:
            duel_0884ACD8(p, 0xD);
            break;
        case 0x1516:
        case 0x16DF:
            duel_0884ACD8(p, 0x3A);
            break;
        case 0x167A:
            duel_0884AD18(0x12A);
            break;
        case 0x15F3:
            duel_0884ACD8(p, 0x7B);
            break;
        case 0x15DD:
            duel_0884AD18(0x55);
            break;
        case 0x1544:
            bits = duel.players[p & 1].n_hand != 0;
            if (duel_count_monsters(p) != 0) {
                bits |= 2;
            }
            duel_0884ADAC(0x103, (int)duel_08861480(0x1544, 0), (int)duel_08861480(0x1544, 1), 0, 0, bits);
            break;
        case 0x153F:
            duel_0884ADAC(0x104, (int)duel_08861480(0x153F, 0), (int)duel_08861480(0x153F, 1), (int)duel_08861480(0x153F, 2), 0, 0xF);
            break;
        case 0x1488:
            duel_0884AD40(0xE, duel_list_find_id(p, id));
            break;
        case 0x140D:
            duel_0884AD18(0xB);
            break;
        case 0x139C:
            duel_0884AD18(0xE1);
            break;
        case 0x1357:
            duel_wfmt_s(msg, 0x108, card_db_table_sel(arg));
            duel_0884ACD8(p, (int)msg);
            break;
        case 0x135B:
            duel_0884ACD8(p, arg == 0 ? 0xED : 0x11);
            break;
        case 0x12F3:
        case 0x14B3:
        case 0x1A19:
        case 0x1A50:
        case 0x1A67:
            duel_0884ACD8(p, 0x11);
            break;
        case 0x10D6:
            duel_wfmt_s(msg, 0x108, 0x1AC);
            duel_0884ACD8(p, (int)msg);
            break;
        case 0xFE0:
            duel_0884AD40(0xE3, duel_list_find_id(p, id));
            break;
        default:
            duel_0884AD40(0xFA, duel_list_find_id(p, id));
            break;
        }
        duel_ev_unk++;
        return 0;
    }
    duel_w_0891968C = duel_w_08919624;
    return 1;
}
#endif

/* Pick one of the names of the card (or a card name) for the event. */
int duel_08831C2C(unsigned int p, int id, int arg)
{
    unsigned short msg[256];

    if (duel_player_type[p] == 1) {
        return 1;
    }
    switch (duel_ev_unk) {
    case 0:
        switch (id) {
        case 0x1368:
        case 0x1568:
        case 0x1590:
        case 0x1593:
        case 0x15DE:
        case 0x169D:
        case 0x16D3:
        case 0x1803:
        case 0x1A7E:
            duel_wfmt_s(msg, 0x108, card_get_name(arg));
            break;
        default:
            duel_wfmt_s(msg, 0x108, duel_08861480(id, arg));
            break;
        }
        duel_0884ACD8(p, (int)msg);
        duel_ev_unk++;
        return 0;
    }
    duel_w_0891968C = duel_w_08919624;
    return 1;
}

#ifdef NON_MATCHING /* o original relê duel_w_0891968C depois do laço; o mwcc o mantém em a3 */
/* Coin tosses of 0x150F: the argument has the count (low byte) and the side called (high byte);
   0x150F may toss again. */
int duel_08831D68(void)
{
    unsigned int p = duel_w_08919670;
    int id = duel_ev_data[0];
    int n = duel_ev_data[1] & 0xFF;
    int want = (unsigned char)(duel_ev_data[1] >> 8);
    int k;
    int r;

    switch (duel_ev_unk) {
    case 0:
        duel_w_0891968C = 0;
        duel_h_0891968E[0] = 0;
        for (k = 0; k < n; k++) {
            r = duel_rand(2);
            duel_w_0891968C = duel_w_0891968C * 2 + r;
            duel_h_0891968E[0] += r == want;
        }
        duel_cmd((p ? 0x8000 : 0) | 0x5C, id, (unsigned char)n | (unsigned char)want << 8, duel_w_0891968C);
        duel_ev_unk++;
        return 0;
    case 1:
        if (duel_find_effect_card(p, 0x150F, -1) != 0 && duel_slot_has_effect(p, 0xB, 0x150F) == 0) {
            if (duel_player_type[p] == 1) {
                duel_w_08919624 = n / 2 >= duel_h_0891968E[0];
            } else {
                duel_0884AD18(0x11F);
            }
            duel_ev_unk++;
            return 0;
        }
        break;
    case 2:
        if (duel_w_08919624 != 0) {
            duel_088C448C(p, 0x150F);
            duel_088C77D4(p, 0x150F, 1, 0);
            duel_ev_unk = 0;
            return 0;
        }
        break;
    }
    if (p == duel_view_player) {
        if (n / 2 < duel_h_0891968E[0]) {
            duel_0884DB38(0x33, 1);
            duel_0884DAF0(0x34, duel_stat_get(0x33));
        } else {
            duel_0884DA24(0x33, 0);
        }
    }
    return 1;
}

#endif

/* Dice rolls of 0x16A5 (argument + 1 dice, in duel.unk3830); 0x16A5 may roll again. */
int duel_08831FE8(void)
{
    int k;
    unsigned int p = duel_w_08919670;
    int id = duel_ev_data[0];
    int n = duel_ev_data[1] + 1;

    switch (duel_ev_unk) {
    case 0:
        for (k = 0; k < n; k++) {
            (&duel.unk3830)[k] = duel_rand(6) + 1;
            duel_cmd((p ? 0x8000 : 0) | 0x5D, id, (&duel.unk3830)[k], 0);
        }
        duel_ev_unk++;
        return 0;
    case 1:
        if (duel_slot_effect_kind(p, 0xB, 0x16A5, 1) != 0 && duel_slot_effect_kind(p, 0xB, 0x16A5, 2) == 0) {
            if (duel_player_type[p] == 1) {
                duel_w_08919624 = duel_w_0891968C < 4;
            } else {
                duel_0884AD18(0x120);
            }
            duel_ev_unk++;
            return 0;
        }
        return 1;
    case 2:
        if (duel_w_08919624 != 0) {
            duel_088C44E4(p, 0x16A5);
            duel_088C77D4(p, 0x16A5, 2, 0);
            duel_ev_unk = 0;
            return 0;
        }
        return 1;
    }
    return 0;
}

/* Runs one step of the question of the pending event (duel_ev_kind); when it is answered, the answer
   is sent to the other console and the event is done. Returns 0 when no event is pending. */
int duel_ev_run(void)
{
    int r;

    if (duel_ev_pending == 0) {
        return 0;
    }
    if (duel_ev_sent != 0 && duel_w_08919670 == (duel_view_player ^ 1)) {
        r = duel_ev_flags.b1;
    } else {
        switch (duel_ev_kind) {
        case 1:
        case 2:
            r = duel_08832D14(duel_w_08919670, duel_ev_data[0], duel_ev_data[1]);
            break;
        case 3:
            r = duel_088330E8(duel_w_08919670, duel_ev_data[0], duel_ev_data[1]);
            break;
        case 4:
            r = duel_088332E0(duel_w_08919670, duel_ev_data[0], duel_ev_data[1]);
            break;
        case 5:
            r = duel_08833638(duel_w_08919670, duel_ev_data[0], duel_ev_data[1]);
            break;
        case 6:
            r = duel_08833A88(duel_w_08919670, duel_ev_data[0]);
            break;
        case 7:
            r = duel_08830DC0(duel_w_08919670);
            break;
        case 8:
            r = duel_08830030(duel_w_08919670, duel_ev_data[0], duel_ev_data[1]);
            break;
        case 9:
            r = duel_0883078C(duel_w_08919670, duel_ev_data[0], duel_ev_data[1]);
            break;
        case 10:
            r = duel_08830B7C(duel_w_08919670, duel_ev_data[0]);
            break;
        case 11:
            r = duel_08830CDC(duel_w_08919670, duel_ev_data[0], duel_ev_data[1]);
            break;
        case 12:
            r = duel_08830E94(duel_w_08919670, duel_ev_data[0]);
            break;
        case 13:
            r = duel_0883136C(duel_w_08919670);
            break;
        case 14:
            r = duel_088309F8(duel_w_08919670, duel_ev_data[0], duel_ev_data[1]);
            break;
        case 15:
            r = duel_08831590(duel_w_08919670, duel_ev_data[0], duel_ev_data[1]);
            break;
        case 16:
            r = duel_08831C2C(duel_w_08919670, duel_ev_data[0], duel_ev_data[1]);
            break;
        case 17:
            r = duel_0883F1DC(duel_w_08919670, duel_ev_data[0]);
            break;
        case 18:
            r = duel_08831D68();
            break;
        case 19:
            r = duel_08831FE8();
            break;
        case 20:
            r = duel_08831374(duel_w_08919670, duel_ev_data[0], duel_ev_data[1]);
            break;
        case 0:
        default:
            return 1;
        }
    }
    if (r != 0) {
        if (duel_ev_sent != 0 && duel_w_08919670 == duel_view_player) {
            duel_msg_send(0x1F, -1, &duel_w_0891968C, 0x10);
        }
        duel_ev_pending = 0;
    }
    return 1;
}
