// @range 0x0887DB24-0x088807F0
// @asm 0x0887DB24-0x0887DBB4
// @asm 0x0887DC94-0x0887DDB0
// @asm 0x0887E140-0x0887F728
// @asm 0x0887F8E4-0x0887FCA8
// @asm 0x0887FD20-0x088800E0
// @asm 0x08880204-0x08880624
// @asm 0x08880624-0x088807F0
/* Card effect handlers (DuelAct *a): what a's card does when it resolves. */
#include "duel.h"

int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440();
int duel_088D14B8(DuelAct *a, unsigned int p, int z);
void duel_088D13CC(DuelAct *a, unsigned int mask);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_count_free_monster_zones();
int duel_phase_ok2();
int duel_act_target2(DuelAct *a, int i);
unsigned int duel_card_locate(unsigned int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_0884E4E4(DuelAct *a);
int duel_act_check_08853CE8(DuelAct *a, int x, int y);
int card_get_type(int id);
void duel_088C8B70(unsigned int p, unsigned short loc, unsigned short t);
void duel_088CC178();
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088CC718(int id);
int duel_0887DB24(DuelAct *a);
void duel_act_0887E140(DuelAct *a, int d[2]);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C5424(unsigned int p, int n);
void duel_088CBAA4(unsigned int p, int z, unsigned int uid);
void duel_088D15B4(DuelAct *a, unsigned int p, int z);
void duel_088C5E70(unsigned int p, int n);
int duel_count_field(unsigned int p);
int duel_grave_count_id(unsigned int p, int id);
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
void duel_0884ACD8(unsigned int p, int msg);
void duel_wfmt_s(char *dst, char *fmt, unsigned short *arg);
unsigned short *card_db_table_sel(int i);
unsigned short *dlg_table_sel(int i);
int card_get_kind(int id);
int duel_0886CF1C(DuelAct *a, int b);
int field_get_card(unsigned int p, int z);
int card_same_name(int id, int id2);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_zone_usable(unsigned int p, unsigned int z);
int duel_0887FCA8(unsigned int p, int z, int k);
extern int duel_w_0891964C;
extern int duel_w_08919650;
extern unsigned short duel_h_0891968E;
extern unsigned int duel_turn_player;
DuelCard *duel_0884C7CC(void);
int duel_088CB554(unsigned int p, DuelCard *c);
void duel_088C78E0(unsigned int p);
void duel_0884C588(unsigned int p, int a, int id, int b);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_088D1AC8(int a, int b);
unsigned short *card_get_name(int id);
void duel_088359E8(unsigned int p, int id, int (*fn)(int id));
void duel_08832AF4(unsigned int p, int z, int a, int b);
int duel_rand(int n);
void duel_0884AF88(int msg);
void duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
int duel_field_atk(unsigned int p, int z);
unsigned int duel_field_type(unsigned int p, int z);
int duel_0884E690(DuelAct *a, int (*fn)(int id));
int duel_088800E0(DuelAct *a, unsigned int p, int z);

#ifdef NON_MATCHING /* branch layout differs */
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
        return 0;
    }
    return 0;
}
#endif

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

#ifdef NON_MATCHING /* branch layout differs */
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

        if ((loc & 0xFFFF) >> 8 == 0xE && duel_grave_card_usable(q, loc & 0xFF, loc >> 16) != 0) {
            duel_088263A8(q, &duel.players[loc & 1].grave[loc >> 16], 1, a->id << 16);
            return 1;
        }
        return 0;
    }
    return 0;
}
#endif

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

/* duel_act_0887E140 (0x0887E140, 5.6 KB, kept in asm): LP change that the effect of a's card
   makes for each player, by card id, into d[2]. */

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

#ifdef NON_MATCHING /* case layout differs */
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
        n = 1;
        break;
    case 0x1A90:
        n = 1;
        break;
    case 0x1175:
    case 0x12EC:
    case 0x1353:
    case 0x1662:
    case 0x16D2:
        n = 2;
        break;
    case 0x1A5B:
        n = 2;
        break;
    case 0x1748:
        n = 3;
        break;
    case 0x1790:
        n = 3;
        break;
    case 0x1567:
        n = 2 - duel.players[p].n_hand;
        break;
    case 0x1501:
        n = 5 - duel.players[p].n_hand;
        break;
    case 0x1776:
        n = 1;
        if (a->w[2] != 1) {
            n = 2;
        }
        break;
    case 0x17D6:
        n = duel_slot_has_effect(p, 11) + 1;
        break;
    case 0x1888:
        n = duel_grave_count_id(p, 0x1888) + 1;
        break;
    case 0x18F9:
        if (duel.players[p].n_hand == 0 &&
            !(a->uid != CARD_UID(duel.players[p].field[a->zone].card)) == duel_count_field(p)) {
            n = 2;
        }
        break;
    case 0x1966:
        if (p == a->y.y15_3) {
            n = 1;
        } else {
            n = (p == a->y.y15_1) + 1;
        }
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

#ifdef NON_MATCHING /* switch layout differs */
int duel_0887FD20(DuelAct *a, int b)
{
    char buf[512];
    unsigned short id = a->id;
    unsigned int q = a->player ^ a->opp;
    int n = 1;
    int r;

    if (id == 0x16FD) {
        n = 5;
    } else if (id == 0x12CA) {
        n = 2;
    }
    r = 100;
    switch (duel_step) {
    case 100:
        duel_088C7CF0();
        return 0;
    case 0x7D:
        if (duel_08835AC0() == 0) {
            return 0x7F;
        } else {
            int i = duel_w_08919654;
            unsigned short x = a->id;

            duel_w_08914900 = duel_pick_monster_zone(q);
            duel_088263A8(q, &duel.players[q].hand[i], 1, (x == 0x165B) | a->id << 16);
            duel_w_089148FC++;
            if (duel_w_089148FC < n) {
                return 0x7F;
            }
            return 100;
        }
    case 0x7E:
        if (duel_w_08919624 != 0) {
            if (duel_player_type[q] == 1) {
                duel_0881956C(q, id, (int (*)(int))duel_0887FCA8);
            } else {
                duel_08835978((int (*)(int))duel_0887FCA8);
            }
            r = 0x7D;
        }
        return r;
    case 0x7F:
        if (duel_0886CF1C(a, b) == 0) {
            return 100;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else if (duel_w_089148FC == 0) {
            int kind = 0;
            int tab = 0;

            switch (a->id) {
            case 0x1A2B:
            case 0x1A2C:
            case 0x1A2D:
            case 0x1A2E:
                kind = card_get_kind(a->id);
                break;
            case 0x1AA1:
                tab = 9;
                break;
            case 0x1572:
                kind = 2;
                break;
            case 0x19AC:
                tab = 7;
                break;
            case 0x1879:
                tab = 1;
                break;
            case 0x153B:
                tab = 2;
                break;
            case 0x1715:
                tab = 9;
                break;
            case 0x16F6:
                tab = 4;
                break;
            case 0x12CA:
                tab = 1;
                break;
            case 0x12A4:
                tab = 10;
                break;
            }
            if (tab == 0 && kind == 0) {
                duel_0884ACD8(q, 0x11E);
            } else {
                if (tab != 0) {
                    duel_wfmt_s(buf, (char *)0x24, card_db_table_sel(tab));
                }
                if (kind != 0) {
                    duel_wfmt_s(buf, (char *)0x24, dlg_table_sel(kind));
                }
                duel_0884ACD8(a->player, (int)buf);
            }
            duel_w_08919624 = 1;
        } else {
            duel_0884AD18(0x23);
        }
        return 0x7E;
    case 0x80:
        duel_w_089148FC = 0;
        if (duel_0886CF1C(a, b) == 0) {
            return 0;
        }
        duel_088C7BA4();
        return 0x7F;
    }
    return 0;
}
#endif

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

#ifdef NON_MATCHING /* switch layout differs */
int duel_08880204(DuelAct *a)
{
    char m0[512];
    char m1[512];
    char m2[512];

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x5F:
        if (duel_088CB554(a->player, duel_0884C7CC()) != 0) {
            duel_088C78E0(a->player);
        }
        return 10;
    case 0x60:
        duel_0884C588(a->player, 6, 0x146E, 0);
        return 0x5F;
    case 0x61:
        if (duel_088BFF78(a->player, 0x146E, 0) <= 0) {
            return 10;
        }
        duel_0884ACD8(a->player, 0x3C);
        return 0x60;
    case 99:
        duel_088263A8(a->player, duel_0884C7CC(), 1, 1);
        return 0x61;
    case 100:
        duel_0884C588(a->player, 6, a->id, 0);
        return 99;
    case 0x6D:
        if (duel_08835AC0() == 0) {
            return 0x77;
        }
        duel_088D1AC8(duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
        return 100;
    case 0x77:
        if (duel_w_08919624 == 0) {
            return 10;
        }
        duel_wfmt_s(m0, (char *)0x13, card_get_name(0xFC9));
        duel_0884ACD8(a->player, (int)m0);
        duel_088359E8(a->player, 1, (int (*)(int))duel_088800E0);
        return 0x6D;
    case 0x78:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 0;
        } else {
            duel_wfmt_s(m1, (char *)0x39, card_get_name(0xFC9));
            duel_wfmt_s(m2, m1, card_get_name(0x146E));
            duel_0884AD18((int)m2);
        }
        return 0x77;
    case 0x7E:
        if (duel_h_0891968E == 0) {
            int atk = 0;
            int z;

            for (z = 0; z < 5; z++) {
                if (duel.players[a->player].field[z].card.id != 0) {
                    atk += duel_field_atk(a->player, z);
                }
            }
            duel_088D13CC(a, 0x1F << (a->player << 4));
            duel_088C4604(a, a->player, atk >> 1);
        } else {
            duel_088D13CC(a, 0x1F << (((1 - a->player) * 16) & 0x1F));
            if (duel_088BFFD8(a->player, a->id, 0) <= 0) {
                return 10;
            }
            if (duel_phase_ok2(a->player) != 0 && duel_0884E690(a, (int (*)(int))duel_088800E0) != 0) {
                return 0x78;
            }
        }
        return 10;
    case 0x7F:
        duel_08832AF4(a->player, a->zone, 1, duel_w_08919624);
        return 0x7E;
    case 0x80:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = duel_rand(2);
        } else {
            duel_0884AF88(0x38);
        }
        return 0x7F;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* loop layout differs */
int duel_08880624(DuelAct *a)
{
    unsigned int mask;
    int i;
    int z;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x7F:
        for (i = 0; i < 2; i++) {
            unsigned int p = duel_turn_player ^ i;

            for (z = 0; z < 5; z++) {
                DuelSlot *sl = &duel.players[p & 1].field[z];

                if (sl->card.id != 0 && sl->faceup != 0 && sl->pos != 0 && (short)duel_field_type(p, z) == 1) {
                    duel_088C97F0(a, p, z, 0, 0);
                }
            }
        }
        return 1;
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
        if (mask == 0) {
            return 1;
        }
        return 0x7F;
    }
    return 0;
}
#endif
