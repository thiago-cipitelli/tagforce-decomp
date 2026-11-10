// @range 0x0887DB24-0x0888146C
// @data 0x08911770-0x089117A0
// @asm 0x0887F8E4-0x0887FCA8
/* Card effect handlers (DuelAct *a): what a's card does when it resolves. */
#include "duel.h"

int duel_088A9008(DuelAct*, int, unsigned int*);
int duel_0884E440();
int duel_088D14B8(DuelAct *a, unsigned int p, int z);
unsigned int duel_088D13CC(DuelAct*, unsigned int);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_count_free_monster_zones();
int duel_phase_ok2();
unsigned int duel_act_target2(DuelAct*, int);
unsigned int duel_card_locate(unsigned int);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_0884E4E4(DuelAct *a);
int duel_act_check_08853CE8(DuelAct*, unsigned int, int);
unsigned int card_get_type(int);
void duel_088C8B70(unsigned int p, unsigned short loc, unsigned short t);
void duel_088CC178();
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, unsigned short a, unsigned short b);
void duel_088CC718(int id);
int duel_0887DB24(DuelAct *a);
void duel_act_0887E140(DuelAct *a, int d[2]);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
int duel_088C4604(DuelAct*, unsigned int, int);
void duel_088C5424(unsigned int p, int n);
void duel_088CBAA4(unsigned int, int, int);
void duel_088D15B4(DuelAct *a, unsigned int p, int z);
int duel_088C5E70(unsigned int, int);
int duel_count_field(unsigned int p);
int duel_grave_count_id(unsigned int, unsigned int);
int duel_slot_has_effect();
int duel_088BFFD8();
extern DuelAct *duel_w_089147DC;
extern int duel_step;
extern int duel_player_type[2];
extern int duel_w_089148FC;
extern int duel_w_08914900;
extern int duel_w_08919624;
extern int duel_w_08919654;
int duel_08835AC0(void);
int duel_pick_monster_zone();
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(int id));
void duel_08835978(int (*fn)(int id));
void duel_0884AD18(int msg);
extern int duel_atk_zone;
int card_is_in_group_1a6f(int id);
int card_is_attribute9(int);
int card_is_monster(int id);
int card_get_def(int id);
int card_get_atk(int);
int card_get_level(int);
int duel_count_grave_type(unsigned int, unsigned int);
int duel_count_field_all(unsigned int p);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
int duel_count_monsters(unsigned int p);
int duel_count_attr(unsigned int p, int attr);
int duel_count_type(unsigned int, unsigned int);
int duel_slot_effect_sum(unsigned int p, int z, unsigned int id);
unsigned short duel_slot_e1854(unsigned int, int);
void duel_0884ACD8(unsigned int p, int msg);
void duel_wfmt_s(char *dst, char *fmt, unsigned short *arg);
unsigned short *card_db_table_sel(int i);
unsigned short *dlg_table_sel(int i);
int card_get_kind(int id);
int duel_0886CF1C(DuelAct *a, int b);
int field_get_card(unsigned int, int);
int card_same_name(int id, int id2);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_zone_usable(unsigned int p, unsigned int z);
int duel_0887FCA8(unsigned int p, int z, int k);
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern unsigned short duel_h_0891968E;
extern unsigned int duel_turn_player;
DuelCard *duel_0884C7CC(void);
int duel_088CB554(unsigned int p, DuelCard *c);
void duel_088C78E0(unsigned int p);
void duel_0884C588(unsigned int, int, int, int);
int duel_088BFF78(unsigned int, int, int);
void duel_088D1AC8(unsigned int, int);
unsigned short *card_get_name(int id);
void duel_088359E8(unsigned int p, int id, int (*fn)(int id));
void duel_08832AF4(unsigned int, int, int, int);
int duel_rand(int n);
int duel_0884AF88(int);
int duel_088C97F0(DuelAct*, unsigned int, int, int, int);
int duel_field_atk(unsigned int p, int z);
unsigned int duel_field_type(unsigned int p, int z);
int duel_0884E690(DuelAct *a, int (*fn)(int id));
int duel_088800E0(DuelAct *a, unsigned int p, int z);
void duel_088328FC(unsigned int p);
int duel_088AA224(DuelAct *a);
int duel_cmd();
void duel_088C7E68(unsigned int, int, int, int);
void duel_088C96EC(unsigned int p, int z, int a);
void duel_088C4508(unsigned int p, int id);
void duel_088C7B90(int n);
void duel_088CAF24(DuelAct *a, unsigned int p, DuelCard *c);
unsigned int duel_grave_find_uid(unsigned int p, int uid);
void duel_088327F8(unsigned int p, int a, int b);
void duel_088C6B24(unsigned int p);
int duel_088D1684(DuelAct *a, unsigned int p, int z, int b);
void duel_088D2288(DuelAct *a, int i, int b);
void duel_088CA428(unsigned int p, unsigned int uid, int a);
void duel_088C7810(unsigned int, unsigned int);
void duel_0884DB38(int a, int b);
int card_is_in_group_198c(int id);
int duel_0886CE38();
extern unsigned int duel_view_player;

int duel_0887DB24(DuelAct *a)
{
    int t;
    int p;
    int z;

    if (duel_088A9008(a, 0, (unsigned int *)&t) != 0) {
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0 && duel_088D14B8(a, p, z) != 0) {
            return 1;
        }
    }
    return 0;
}

void duel_0887DBB4(DuelAct *a)
{
    unsigned int mask = 0;
    int i;
    int t;

    for (i = 0; i < a->n_targets; i++) {
        if (duel_088A9008(a, i, (unsigned int *)&t) != 0) {
            int p = t & 0xFF;
            int z = (unsigned short)t >> 8 & 0xFF;

            if (duel_0884E440(a, p, z) != 0 && duel_act_can_affect(a, p, z, 1) != 0) {
                mask |= 1 << (z + p * 16);
            }
        }
    }
    duel_088D13CC(a, mask);
}

int duel_0887DC94(DuelAct *a)
{
    unsigned int q = a->player ^ a->opp;

    if (duel_count_free_monster_zones(q) == 0) {
        return 0;
    }
    if (duel_phase_ok2(q) == 0) {
        return 0;
    }
    if (a->n7 == 1) {
        unsigned int loc = duel_card_locate(duel_act_target2(a, 0));
        unsigned int p = loc & 0xFF;
        unsigned int i = (loc >> 16) & 0xFFFF;

        if ((unsigned char)((unsigned short)loc >> 8) == 0xE && duel_grave_card_usable(q, p, i) != 0) {
            duel_088263A8(q, &duel.players[p & 1].grave[i], 1, a->id << 16);
            return 1;
        }
    }
    return 0;
}

int duel_0887DDB0(DuelAct *a)
{
    duel_0887DB24(a);
    return 0;
}

int duel_0887DDD0(DuelAct *a)
{
    duel_088D13CC(a, duel_0884E4E4(a));
    return 0;
}

int duel_0887DE04(DuelAct *a)
{
    int t;
    unsigned int p;

    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel_088A9008(a, 0, (unsigned int *)&t) == 0 ||
        duel_act_check_08853CE8(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF) == 0) {
        a->b4_3 = 1;
        return 0;
    }
    if (card_get_type(a->id) == 0x17 && a->b4_2) {
        a->b4_3 = 1;
        return 0;
    }
    p = a->player;
    duel_088C8B70(p, p | a->zone << 8, (unsigned short)t);
    switch (a->id) {
    case 0x12C6:
    case 0x145B:
        duel_088CC178(a->player, a->zone, a->w[1]);
        break;
    }
    return 0;
}

int duel_0887DF94(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int q = 1 - p;
    unsigned int id = duel.players[q & 1].field[10].card.id;
    int i;
    int z;

    if (a->uid != CARD_UID(duel.players[p & 1].field[a->zone].card)) {
        return 0;
    }
    if (id != 0 && duel.players[q & 1].field[10].faceup != 0) {
        duel_088D1438(q, 10, 0, 0);
    }
    for (i = 0; i < 2; i++) {
        for (z = 0; z <= 4; z++) {
            if (duel.players[i & 1].field[z].card.id != 0 && duel.players[i & 1].field[z].faceup != 0) {
                duel_088C87A8(i, z, a->player | a->zone << 8, 9, 0);
            }
        }
    }
    if (a->id != id) {
        duel_088CC718(a->id);
    }
    return 0;
}

/* LP change that the effect of a's card makes for each player, by card id, into d[2] */
void duel_act_0887E140(DuelAct *a, int d[2])
{
    unsigned int me = a->player ^ a->opp;
    unsigned int k = a->w[0] & 1;
    unsigned short src;
    int t;
    unsigned int q;
    int n;

    d[1] = 0;
    d[0] = 0;
    switch (a->id) {
        case 0x10F8:
            d[k] = 200;
            break;
        case 0x10F9:
            d[k] = 500;
            break;
        case 0x10FA:
            d[k] = 600;
            break;
        case 0x10FB:
            d[k] = 800;
            break;
        case 0x10FC:
            d[k] = 1000;
            break;
        case 0x12E8:
            d[k] = 400;
            break;
        case 0x1459:
            d[me] = 200;
            break;
        case 0x104C:
        case 0x142C:
            d[me] = 500;
            break;
        case 0x17A5:
            d[me] = 700;
            break;
        case 0x1454:
            d[me] = 800;
            break;
        case 0x12A2:
        case 0x1450:
        case 0x1451:
        case 0x1455:
        case 0x1467:
        case 0x17A4:
            d[me] = 1000;
            break;
        case 0x14F3:
        case 0x1793:
            d[me] = 2000;
            break;
        case 0x151B:
            d[me] = 4000;
            break;
        case 0x137A:
            d[me] = (duel_count_monsters(0) + duel_count_monsters(1)) * 300;
            break;
        case 0x1757:
            d[me] = duel_count_monsters(me) * 400;
            break;
        case 0x19CD:
            d[me] = duel_count_monsters(me) * 800;
            break;
        case 0x16DA:
            d[me] = a->idx * 500;
            break;
        case 0x1A14:
            d[me] = duel_count_monsters_fn(me, card_is_in_group_1a6f) * 1000;
            break;
        case 0x1370:
            t = duel_slot_e1854(a->player, a->zone);
            if ((t & 0xFFFF) != 0xFFFF) {
                d[me] = (duel_field_atk(t & 0xFF, (unsigned short)t >> 8 & 0xFF) + 1) >> 1;
            }
            break;
        case 0x1877:
            d[1 - me] = 500;
            break;
        case 0x1322:
        case 0x137B:
        case 0x16F5:
            d[1 - me] = 1000;
            break;
        case 0x141F:
            d[me] = 1000;
            d[1 - me] = 1000;
            break;
        case 0x14E6:
            d[me] = a->w[0] * 1000;
            break;
        case 0x15E7:
            switch (a->w[2]) {
            case 1:
                d[me] = 1200;
                break;
            case 2:
                d[1 - me] = -800;
                break;
            }
            break;
        case 0x16A0:
            if (!card_is_attribute9(a->w[1]) && card_is_monster(a->w[1])) {
                d[me] = card_get_atk(a->w[1]);
            }
            break;
        case 0x175A:
            d[me] = a->w[2];
            break;
        case 0x14ED:
        case 0x1506:
            d[me] = a->h[0];
            break;
        case 0x1AA2:
            d[me] = a->w[0] * 400;
            break;
        case 0x1101:
            d[1 - me] = -1000;
            d[me] = -500;
            break;
        case 0xFFF:
        case 0x1855:
        case 0x1929:
            d[1 - me] = -a->w[0];
            break;
        case 0x10B0:
            d[1 - me] = duel_count_field_all(1 - me) * -500;
            break;
        case 0x191D:
            d[1 - me] = duel_count_monsters(1 - me) * -300;
            break;
        case 0x19CE:
            d[1 - me] = duel_count_monsters(1 - me) * -600;
            break;
        case 0x1574:
        case 0x18B6:
        case 0x1A51:
            d[1 - me] = -300;
            break;
        case 0x1627:
            d[1 - me] = -400;
            break;
        case 0x10DD:
        case 0x11A6:
        case 0x1355:
        case 0x13A8:
        case 0x1435:
        case 0x14B2:
        case 0x1520:
        case 0x1525:
        case 0x1565:
        case 0x1586:
        case 0x1756:
        case 0x1762:
        case 0x17E1:
        case 0x187B:
        case 0x18B7:
        case 0x1A69:
            d[1 - me] = -500;
            break;
        case 0x1637:
            d[1 - me] = -600;
            break;
        case 0x158C:
            d[1 - me] = -700;
            break;
        case 0x14E0:
        case 0x1702:
        case 0x144E:
        case 0x144F:
        case 0x179B:
            d[1 - me] = -800;
            break;
        case 0x1190:
        case 0x11BC:
        case 0x1460:
        case 0x16FA:
        case 0x19A4:
        case 0x19F0:
        case 0x1AA0:
            d[1 - me] = -1000;
            break;
        case 0x144B:
            d[1 - me] = -1200;
            break;
        case 0x190A:
            d[1 - me] = -1500;
            break;
        case 0x10FD:
            d[k] = -200;
            break;
        case 0x10FE:
            d[k] = -500;
            break;
        case 0x10FF:
            d[k] = -600;
            break;
        case 0x1100:
            d[k] = -800;
            break;
        case 0x13F0:
        case 0x13F3:
            q = duel_slot_e1854(a->player, a->zone) & 0xFF;
            d[q] = -500;
            break;
        case 0x11CF:
            d[me] = -300;
            break;
        case 0x1578:
            d[me] = -1000;
            break;
        case 0x118A:
            d[me] = -2000;
            break;
        case 0x169A:
            d[me] = -800;
            break;
        case 0x11C9:
            d[1 - me] = 3000;
            break;
        case 0x12E9:
            d[k] = -300;
            break;
        case 0x1301:
            d[1 - me] = duel_count_monsters(1 - me) * -500;
            break;
        case 0x1307:
            d[k] = (int)duel.players[k & 1].n_hand * -200;
            break;
        case 0x161D:
            d[1 - me] = (int)duel.players[1 - me & 1].n_hand * -400;
            break;
        case 0x1421:
            if (a->w[0] != 0) {
                d[1 - me] = -500;
            }
            break;
        case 0x18DA:
            d[1 - me] = -a->w[0];
            break;
        case 0x159B:
            d[1 - me] = a->w[0] * -300;
            break;
        case 0x15EE:
            d[1 - me] = a->w[1] * -1000;
            break;
        case 0x15F4:
            q = 1 - me;
            d[q] = (duel.players[q & 1].n_hand + duel_count_field(q)) * -200;
            break;
        case 0x129A:
            d[1 - me] = -duel_field_atk(duel_special.player[0], duel_atk_zone);
            break;
        case 0x1641:
        case 0x18C8:
        case 0x18D1:
        case 0x193B:
        case 0x1943:
        case 0x19D3:
            d[1 - me] = -card_get_atk(duel.list[a->src.uid].id);
            break;
        case 0x170B:
        case 0x1987:
        case 0x19E0:
            d[me] = card_get_atk(duel.list[a->src.uid].id);
            break;
        case 0x1A23:
            d[me] = card_get_def(duel.list[a->src.uid].id);
            break;
        case 0x17C7:
            d[1 - me] = -card_get_atk(duel.list[a->src.uid].id) / 2;
            break;
        case 0x17C8:
            d[1 - me] = -card_get_def(duel.list[a->src.uid].id) / 2;
            break;
        case 0x18D0:
            d[1 - me] = -card_get_def(duel.list[a->src.uid].id);
            break;
        case 0x174A:
        case 0x174B:
            d[1 - me] = card_get_level(duel.list[a->src.uid].id) * -500;
            break;
        case 0x1792:
            d[me] = card_get_level(duel.list[a->src.uid].id) * 300;
            break;
        case 0x16C5:
            d[1 - me] = -1500;
            break;
        case 0x16CC:
            d[1 - me] = -700;
            break;
        case 0x16DC:
            d[1 - me] = -3000;
            break;
        case 0x16FE:
            d[a->idx & 0xFFFF] = (a->idx >> 16 & 0xFFFF) * -400;
            break;
        case 0x1794:
            d[1 - me] = -2000;
            break;
        case 0x1804:
            d[k] = duel.players[k & 1].n_grave * -100;
            break;
        case 0x188B:
            d[k] = duel.players[k & 1].n_f * -300;
            break;
        case 0x1761:
            d[1 - me] = -500;
            d[me] = 500;
            break;
        case 0x163F:
            d[me] = a->b3_4 == 1 ? 1000 : -2000;
            break;
        case 0x173F:
            q = 1 - me;
            n = duel.players[me & 1].lp - duel.players[q & 1].lp;
            d[q] = -(n < 0 ? 0 : n);
            break;
        case 0x1767:
            d[1 - me] = duel_count_attr(me, 1) * -600;
            break;
        case 0x1770:
            d[duel_special.player[0]] = -1000;
            break;
        case 0x18D7:
        case 0x174E:
            d[a->y.y16_2] = -1000;
            break;
        case 0x1802:
            t = duel_slot_effect_sum(a->player, a->zone, a->id);
            d[0] = (t & 0xFF) * -500;
            d[1] = ((unsigned short)t >> 8 & 0xFF) * -500;
            break;
        case 0x1869:
            d[1 - me] = -1000;
            if (a->kind == 0x16) {
                d[me] = -1000;
            }
            break;
        case 0x1914:
            d[a->b15_1] = -card_get_atk(a->id);
            break;
        case 0x1950:
            d[1 - me] = -800;
            d[me] = -800;
            break;
        case 0x192C:
            d[1 - me] = duel_count_grave_type(me, 0x16) * -200;
            break;
        case 0x194A:
            if (duel_special.recs[0].b[0] == me) {
                d[1 - me] -= duel_special.recs[0].w0;
            }
            if (duel_special.recs[1].b[0] == me) {
                d[1 - me] -= duel_special.recs[1].w0;
            }
            break;
        case 0x1984:
            d[1 - me] = duel_count_type(me, 0x12) * -200;
            break;
        case 0x19CF:
            d[1 - me] = (unsigned int)duel.players[1 - me & 1].n_18 * -100;
            break;
        case 0x1A40:
            n = duel_count_field_all(0) * 500 - 1000;
            d[0] = n;
            if (n > 0) {
                n = 0;
            }
            d[0] = n;
            n = duel_count_field_all(1) * 500 - 1000;
            d[1] = n;
            if (n > 0) {
                n = 0;
            }
            d[1] = n;
            break;
        case 0x1AAD:
            if (a->b3_4 == 2) {
                d[1 - me] = -1000;
            } else if (a->kind == 5) {
                d[me] = -800;
            } else {
                d[1 - me] = -300;
            }
            break;
        case 0x1B2D:
            if (a->b3_4 == 2) {
                d[1 - me] = -2000;
            } else {
                d[1 - me] = -500;
            }
            break;
    }
}

int duel_0887F728(DuelAct *a)
{
    int d[2];
    unsigned int q;

    duel_act_0887E140(a, d);
    duel_088C7BA4();
    q = 1 - a->player;
    if (d[q] < 0) {
        duel_088C4604(a, q, -d[q]);
    }
    if (d[a->player] < 0) {
        duel_088C4604(a, a->player, -d[a->player]);
    }
    if (d[a->player] > 0) {
        duel_088C5424(a->player, d[a->player]);
    }
    if (d[1 - a->player] > 0) {
        duel_088C5424(1 - a->player, d[1 - a->player]);
    }
    duel_088C7CF0();
    return 0;
}

int duel_0887F80C(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    b->b4_3 = b->b4_2 = b->b4_1 = 1;
    duel_088CBAA4(b->player, b->zone, b->uid);
    {
        int z = b->zone;

        if (z <= 10) {
            duel_088D15B4(a, b->player, z);
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* 132 palavras, só registradores: forma dos case e dos grupos confere; o original deixa p em v0, a->id em a2 e as constantes dos testes em v1 (aqui id em v0, p em v1) */
int duel_0887F8E4(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int q = p ^ a->opp;
    int n = 0;

    switch (a->id) {
    case 0x1082:
    case 0x11C2:
    case 0x139F:
    case 0x1403:
    case 0x145A:
    case 0x1533:
    case 0x1563:
    case 0x15DC:
    case 0x161A:
    case 0x16BB:
    case 0x16C1:
    case 0x16CD:
    case 0x16F7:
    case 0x16FB:
    case 0x1740:
    case 0x17D5:
    case 0x1911:
    case 0x191A:
    case 0x1A07:
    case 0x1A0B:
    case 0x1A1F:
    case 0x1A22:
    case 0x1A90:
        n = 1;
        break;
    case 0x1175:
    case 0x12EC:
    case 0x1353:
    case 0x1662:
    case 0x16D2:
    case 0x1A5B:
        n = 2;
        break;
    case 0x1748:
    case 0x1790:
        n = 3;
        break;
    case 0x1567:
        n = 2 - duel.players[p & 1].n_hand;
        break;
    case 0x1501:
        n = 5 - duel.players[p & 1].n_hand;
        break;
    case 0x1776:
        n = 1;
        if (a->w[2] != n) {
            n = 2;
        }
        break;
    case 0x17D6:
        n = duel_slot_has_effect(p, 11, 0x17D6) + 1;
        break;
    case 0x1888:
        n = duel_grave_count_id(p, 0x1888) + 1;
        break;
    case 0x18F9:
        if (duel.players[p & 1].n_hand == 0 &&
            !(a->uid != CARD_UID(duel.players[p & 1].field[a->zone].card)) == duel_count_field(p)) {
            n = 2;
        }
        break;
    case 0x1966:
        n = (p != a->y.y15_3 && p == a->y.y15_1) + 1;
        break;
    case 0x19C7:
    case 0x1A63:
        n = 1;
        q = 1 - p;
        break;
    }
    if (n > 0) {
        duel_088C5E70(q, n);
    }
    return 0;
}
#endif

int duel_0887FCA8(unsigned int p, int z, int k)
{
    DuelAct *c = duel_w_089147DC;

    if (p == (c->player ^ c->opp) && z == 11) {
        int id = c->id;

        if (id == 0x183E) {
            id = c[-1].id;
        }
        if (duel_088BFFD8(p, id, k) != 0) {
            return 0x800;
        }
    }
    return 0;
}

int duel_0887FD20(DuelAct *a, int b)
{
    char buf[512];
    unsigned int q = a->player ^ a->opp;
    unsigned short id = a->id;
    int n = 1;
    int r;

    switch (id) {
    case 0x12CA:
        n = 2;
        break;
    case 0x16FD:
        n = 5;
        break;
    }
    r = 100;
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        if (duel_0886CF1C(a, b) == 0) {
            return 0;
        }
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        if (duel_0886CF1C(a, b) == 0) {
            return 100;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else if (duel_w_089148FC == 0) {
            int tab = 0;
            int kind = 0;

            switch (a->id) {
            case 0x12A4:
                tab = 10;
                break;
            case 0x12CA:
                tab = 1;
                break;
            case 0x16F6:
                tab = 4;
                break;
            case 0x1715:
                tab = 9;
                break;
            case 0x153B:
                tab = 2;
                break;
            case 0x1879:
                tab = 1;
                break;
            case 0x19AC:
                tab = 7;
                break;
            case 0x1572:
                kind = 2;
                break;
            case 0x1AA1:
                tab = 9;
                break;
            case 0x1A2B:
            case 0x1A2C:
            case 0x1A2D:
            case 0x1A2E:
                kind = card_get_kind(a->id);
                break;
            }
            if (tab != 0 || kind != 0) {
                if (tab != 0) {
                    duel_wfmt_s(buf, (char *)0x24, card_db_table_sel(tab));
                }
                if (kind != 0) {
                    duel_wfmt_s(buf, (char *)0x24, dlg_table_sel(kind));
                }
                duel_0884ACD8(a->player, (int)buf);
            } else {
                duel_0884ACD8(q, 0x11E);
            }
            duel_w_08919624 = 1;
        } else {
            duel_0884AD18(0x23);
        }
        return 0x7E;
    case 0x7E:
        if (duel_w_08919624 == 0) {
            return r;
        }
        if (duel_player_type[q] == 1) {
            duel_0881956C(q, id, (int (*)(int))duel_0887FCA8);
        } else {
            duel_08835978((int (*)(int))duel_0887FCA8);
        }
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            int i = duel_w_08919654;
            unsigned short k = 0;

            switch (a->id) {
            case 0x165B:
                k = 1;
                break;
            }
            duel_w_08914900 = duel_pick_monster_zone(q);
            duel_088263A8(q, &duel.players[q & 1].hand[i], 1, k | (unsigned int)a->id << 16);
            duel_w_089148FC++;
            if (duel_w_089148FC >= n) {
                return 100;
            }
        }
        return 0x7F;
    case 100:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_088800E0(DuelAct *a, unsigned int p, int z)
{
    if (a->player != p) {
        return 0;
    }
    if (card_same_name(field_get_card(p, z), 0xFC9) == 0) {
        return 0;
    }
    if (duel_tributable(a->player, p, z) == 0) {
        return 0;
    }
    if (p != a->player && duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_count_free_monster_zones(a->player) == 0) {
        if (a->player != p) {
            return 0;
        }
        if (duel_zone_usable(p, z) == 0) {
            return 0;
        }
    }
    return 1;
}

int duel_08880204(DuelAct *a)
{
    char m2[512];
    char m1[512];
    char m0[512];

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = duel_rand(2);
        } else {
            duel_0884AF88(0x38);
        }
        return 0x7F;
    case 0x7F:
        duel_08832AF4(a->player, a->zone, 1, duel_w_08919624);
        return 0x7E;
    case 0x7E:
        if (duel_h_0891968E != 0) {
            duel_088D13CC(a, 0x1F << ((1 - a->player) << 4));
            if (duel_088BFFD8(a->player, a->id, 0) > 0) {
                if (duel_phase_ok2(a->player) != 0 && duel_0884E690(a, (int (*)(int))duel_088800E0) != 0) {
                    return 0x78;
                }
            }
        } else {
            int z;
            int atk = 0;

            for (z = 0; z < 5; z++) {
                if (duel.players[a->player & 1].field[z].card.id != 0) {
                    atk += duel_field_atk(a->player, z);
                }
            }
            duel_088D13CC(a, 0x1F << (a->player << 4));
            duel_088C4604(a, a->player, atk >> 1);
        }
        return 10;
    case 0x78:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 0;
        } else {
            duel_wfmt_s(m1, (char *)0x39, card_get_name(0xFC9));
            duel_wfmt_s(m2, m1, card_get_name(0x146E));
            duel_0884AD18((int)m2);
        }
        return 0x77;
    case 0x77:
        if (duel_w_08919624 != 0) {
            duel_wfmt_s(m0, (char *)0x13, card_get_name(0xFC9));
            duel_0884ACD8(a->player, (int)m0);
            duel_088359E8(a->player, 1, (int (*)(int))duel_088800E0);
            return 0x6D;
        } else {
            return 10;
        }
    case 0x6D:
        if (duel_08835AC0() != 0) {
            duel_088D1AC8(duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return 100;
        } else {
            return 0x77;
        }
    case 100:
        duel_0884C588(a->player, 6, a->id, 0);
        return 99;
    case 99:
        duel_088263A8(a->player, duel_0884C7CC(), 1, 1);
        return 0x61;
    case 0x61:
        if (duel_088BFF78(a->player, 0x146E, 0) > 0) {
            duel_0884ACD8(a->player, 0x3C);
            return 0x60;
        } else {
            return 10;
        }
    case 0x60:
        duel_0884C588(a->player, 6, 0x146E, 0);
        return 0x5F;
    case 0x5F:
        if (duel_088CB554(a->player, duel_0884C7CC()) != 0) {
            duel_088C78E0(a->player);
        }
        return 10;
    }
    return 0;
}

int duel_08880624(DuelAct *a)
{
    unsigned int mask;
    int i;
    int z;
    int p;
    int j;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        mask = 0;
        for (i = 0; i < 2; i++) {
            for (z = 5; z < 10; z++) {
                if (duel.players[i & 1].field[z].card.id == 0x10EF && duel.players[i & 1].field[z].faceup != 0) {
                    mask |= 1 << (z + i * 16);
                }
            }
        }
        duel_088D13CC(a, mask);
        return mask != 0 ? 0x7F : 1;
    case 0x7F:
        for (j = 0; j < 2; j++) {
            p = duel_turn_player ^ j;
            for (z = 0; z < 5; z++) {
                if (duel.players[p & 1].field[z].card.id > 0 && duel.players[p & 1].field[z].faceup != 0 &&
                    duel.players[p & 1].field[z].pos != 0 && (unsigned short)duel_field_type(p, z) == 1) {
                    duel_088C97F0(a, p, z, 0, 0);
                }
            }
        }
        return 1;
    }
    return 0;
}

int duel_088807F0(DuelAct *a)
{
    unsigned int q = a->player ^ a->opp;

    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(q, a->id, duel_088AA224(a)) == 0) {
            duel_0884ACD8(q, 0x15);
            return 0x78;
        }
        duel_0884ACD8(q, 0x33);
        return 0x7E;
    case 0x7E:
        duel_0884C588(q, 6, a->id, duel_088AA224(a));
        return 0x7D;
    case 0x7D:
        duel_088CB554(q, duel_0884C7CC());
        return 100;
    case 0x78:
        duel_088328FC(1 - q);
        return 100;
    case 100:
        duel_088C78E0(q);
        return 0;
    }
    return 0;
}

int duel_08880918(DuelAct *a)
{
    duel_cmd((unsigned short)((a->player ? 0x8000 : 0) | 0x27), 0, 0, 0);
    return 0;
}

int duel_0888095C(DuelAct *a)
{
    unsigned char z;
    unsigned int p;
    int i;
    int k;

    if (a->b4_2) {
        return 0;
    }
    z = a->zone;
    p = a->player;
    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    duel_088C7E68(p, z, 4, 1);
    for (i = 0; i < 2; i++) {
        for (k = 0; k < 5; k++) {
            if (duel.players[i & 1].field[k].card.id != 0 && duel.players[i & 1].field[k].faceup != 0 &&
                (unsigned short)duel_field_type(i, k) == 2) {
                duel_088C87A8(i, k, a->player | a->zone << 8, 7, 0);
            }
        }
    }
    return 0;
}

int duel_08880B00(DuelAct *a)
{
    unsigned int t;

    if (duel_088A9008(a, 0, &t) != 0) {
        unsigned int p = t & 0xFF;
        unsigned int z = ((unsigned short)t >> 8) & 0xFF;
        int id = duel.players[p & 1].field[z].card.id;
        int r;

        if (id != 0) {
            if (duel.players[p & 1].field[z].faceup != 0) {
                if (duel_0884E440(a, p, z) != 0) {
                    duel_088D14B8(a, p, z);
                }
                return 0;
            } else {
                duel_088C96EC(p, z, 1);
                duel_088C4508(p, id);
                duel_088C7B90(0x1E);
                duel.players[p & 1].field[z].faceup = 1;
                r = duel_0884E440(a, p, z);
                duel.players[p & 1].field[z].faceup = 0;
                if (r != 0 && duel_088D14B8(a, p, z) != 0) {
                    return 0;
                }
                duel_088C96EC(p, z, 0);
            }
        }
    }
    return 0;
}

int duel_08880C7C(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_088C7BA4();
    case 0x7F:
        if (a->n_targets != 0) {
            int i = duel_grave_find_uid(a->player, duel_act_target2(a, (unsigned char)(int)a->n_targets - 1));

            if (i >= 0) {
                unsigned int p = a->player;

                duel_088CAF24(a, p, &duel.players[p & 1].grave[i]);
            }
            a->n_targets--;
            return 0x7F;
        } else {
            duel_088C7CF0();
            return 0;
        }
    }
    return 0;
}

int duel_08880D54(DuelAct *a)
{
    duel_088327F8(1 - a->player, 1, 1);
    return 0;
}

int duel_08880D84(DuelAct *a)
{
    duel_088C78E0(a->player);
    return 0;
}

int duel_08880DA8(DuelAct *a)
{
    int id = a->id;
    int n = 0;

    switch (id) {
    case 0x1051:
        n = 5;
        break;
    case 0x162C:
        n = 3;
        break;
    case 0x184C:
        n = 3;
        break;
    }
    switch (duel_step) {
    case 0x80: {
        unsigned int p = a->player;

        if (duel.players[p & 1].n_d == 0) {
            return 0;
        }
        if (duel_player_type[p] == 1) {
            return 0;
        }
        duel_0884ACD8(p, 0xF5);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, MIN(n, 32) + 6, id, 0);
        return 0x7E;
    case 0x7E:
        duel_088C6B24(a->player);
        return 0;
    }
    }
    return 0;
}

int duel_08880EC4(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        return 0x7F;
    case 0x7F:
        duel_cmd((unsigned short)((a->player ? 0x8000 : 0) | 0x5A), 1, 0, 0);
        return 0x7E;
    case 0x7E:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_08880F60(DuelAct *a)
{
    int i;

    for (i = 0; i < 2; i++) {
        unsigned int p = duel_turn_player ^ i;
        DuelSpecAct *s = &duel_special.act[p];

        if (duel_special.act[p].pad14[6] != 0) {
            duel_088D2288(a, i, 0);
        } else if (s->uid == CARD_UID(duel.players[s->unk0 & 1].field[s->unk4].card)) {
            duel_088D1684(a, s->unk0, s->unk4, 0);
        }
    }
    return 0;
}

int duel_08881054(DuelAct *a)
{
    int n = 0;
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    switch (a->id) {
    case 0x19A5:
        n = a->w[0];
    }
    duel_088C87A8(p, z, a->id, 2, n);
    return 0;
}

int duel_0888112C(DuelAct *a)
{
    duel_088CA428(a->player, a->uid, 1);
    return 0;
}

int duel_08881160(DuelAct *a, int b)
{
    unsigned int q = a->player ^ a->opp;
    int r;

    switch (duel_step) {
    case 0x80:
        if (duel_0886CE38(a, b) == 0) {
            r = 0;
            break;
        }
        switch (a->id) {
        case 0x17F5:
            if (duel_088AA224(a) == 0) {
                r = 0;
                goto end;
            }
            break;
        }
        if (duel_088BFF78(q, a->id, duel_088AA224(a)) == 0) {
            duel_0884ACD8(q, 0x122);
            r = 0x78;
        } else {
            duel_0884ACD8(q, 0x11D);
            r = 0x7F;
        }
        break;
    case 0x7F:
        duel_0884C588(q, 6, a->id, duel_088AA224(a));
        r = 0x7E;
        break;
    case 0x7E: {
        unsigned short k = 0;

        switch (a->id) {
        case 0x10E4:
            if (duel_0884C7CC()->id == 0xFE5) {
                k = 1;
            }
            break;
        case 0x1529:
        case 0x1642:
        case 0x165A:
        case 0x167D:
        case 0x17AF:
        case 0x17C9:
        case 0x17D3:
        case 0x17DA:
        case 0x1814:
        case 0x1817:
        case 0x187E:
        case 0x18A3:
        case 0x1907:
        case 0x195C:
        case 0x19B1:
        case 0x19B5:
        case 0x19D8:
            k = 1;
            break;
        }
        if (q == duel_view_player && card_is_in_group_198c(duel_0884C7CC()->id) != 0) {
            duel_0884DB38(0x32, 1);
        }
        duel_w_089148FC = duel_pick_monster_zone(q);
        duel_088263A8(q, duel_0884C7CC(), 1, k | (unsigned int)a->id << 16);
        r = 0x7D;
        break;
    }
    case 0x7D:
        duel_088C78E0(q);
        r = 1;
        break;
    case 0x78:
        duel_088C7810(q, 1);
        r = 0x77;
        break;
    case 0x77:
        duel_088328FC(1 - q);
        r = 0x76;
        break;
    case 0x76:
        duel_088C7810(q, 0);
        r = 0x75;
        break;
    case 0x75:
        duel_088C78E0(q);
        r = 0;
        break;
    default:
        r = 0;
        break;
    }
end:
    return r;
}
