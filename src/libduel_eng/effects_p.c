// @range 0x0887DB24-0x0887FD20
// @asm 0x0887DB24-0x0887DBB4
// @asm 0x0887DC94-0x0887DDB0
// @asm 0x0887E140-0x0887F728
// @asm 0x0887F8E4-0x0887FCA8
/* Card effect handlers (DuelAct *a): what a's card does when it resolves. */
#include "duel.h"

int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440();
int duel_088D14B8(DuelAct *a, unsigned int p, int z);
void duel_088D13CC(DuelAct *a, unsigned int mask);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_count_free_monster_zones(unsigned int p);
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
