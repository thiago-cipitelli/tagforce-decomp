// @range 0x088B1EAC-0x088B2460
// @asm 0x088B20A0-0x088B23AC
/* Ritual summon checks (after duel_088B1854): level sums and the ritual table. */
#include "duel.h"

typedef struct DuelRitual {             /* 4 */
    unsigned int monster : 13;
    unsigned int spell : 13;
    unsigned int level : 6;
} DuelRitual;

extern DuelRitual duel_rituals[];       /* 0x0891291C */
extern DuelAct *duel_w_089147DC;
extern int duel_w_089148FC;
extern int duel_w_08914900;

int duel_088B1854(DuelAct *a, unsigned int p, int z, unsigned int i);
int duel_088B153C(unsigned int id, int i);
int duel_088B15B8(unsigned int p, int i);
int duel_088B167C(unsigned int p, unsigned int id);
int duel_088B1774(unsigned int p);
int duel_pick_monster_zone(unsigned int p);
int duel_zone_usable(unsigned int p, unsigned int z);
int card_is_monster(int id);
int duel_phase_ok2(unsigned int p);
int duel_count_active(int id);

int duel_088B1EAC(int *v, int n, int need, int max)
{
    int x;
    int i;

    for (i = n; i >= 0; i--) {
        x = v[i];
        if (x < need) {
            if (i != 0 && duel_088B1EAC(v, i - 1, need - x, max < x ? max : x) != 0) {
                return 1;
            }
        } else if (x < need + max) {
            return 1;
        }
    }
    return 0;
}

int duel_088B20A0(DuelAct *a, int i);

int duel_088B1F74(unsigned int p, int z, int i)
{
    int k;

    if (p == duel_w_089147DC->player && z == 0xB) {
        for (k = 0; duel_rituals[k].monster != 0; k++) {
            if (duel_rituals[k].monster == duel.players[p & 1].hand[i].id && duel_088B20A0(duel_w_089147DC, k) != 0) {
                return 0x800;
            }
        }
    }
    return 0;
}

int duel_088B2058(unsigned int p, int z, unsigned int i)
{
    if (duel_088B1854(duel_w_089147DC, p, z, i) != 0) {
        return 0x800;
    }
    return 0;
}

int duel_088B2094(void)
{
    return duel_w_089148FC;
}

#ifdef NON_MATCHING /* alocação de registradores (a/i trocados entre $s0 e $s1) */
int duel_088B20A0(DuelAct *a, int i)
{
    DuelRitual *r;
    unsigned int q;
    unsigned int m;
    int z;
    unsigned int k;

    if (duel_088B153C(a->id, i) == 0) {
        return 0;
    }
    if (duel_088B15B8(a->player, i) == 0) {
        return 0;
    }
    r = &duel_rituals[i];
    a->w[0] = i;
    duel_w_089148FC = duel_rituals[i].level;
    duel_w_08914900 = 0xFF;
    if (duel_pick_monster_zone(a->player) < 0) {
        for (z = 0; z < 5; z++) {
            if (duel.players[a->player & 1].field[z].card.id != 0 && duel_088B1854(a, a->player, 0, z) != 0 &&
                duel_zone_usable(a->player, z) != 0) {
                return 1;
            }
        }
        return 0;
    }
    switch (a->id) {
    case 0x1698:
    case 0x1716:
    case 0x19D9:
        m = r->monster;
        for (q = 0; (int)q < 2; q++) {
            for (z = 0; z < 5; z++) {
                if (duel.players[q & 1].field[z].card.id != 0 && duel_088B1854(a, q, 0, z) != 0) {
                    return 1;
                }
            }
        }
        for (k = 0; k < duel.players[a->player & 1].n_hand; k++) {
            unsigned int c = duel.players[a->player & 1].hand[k].id;

            if (card_is_monster(c)) {
                if (c == m) {
                    m = -1;
                } else if (duel_088B1854(a, a->player, 0xB, k) != 0) {
                    return 1;
                }
            }
        }
        return 0;
    }
    return duel_rituals[i].level <= duel_088B167C(a->player, r->monster) + duel_088B1774(a->player);
}
#endif

int duel_088B23AC(DuelAct *a)
{
    int k;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (duel_count_active(0x13F2) != 0) {
        return 0;
    }
    for (k = 0; duel_rituals[k].monster != 0; k++) {
        if (!duel_088B20A0(a, k)) {
            continue;
        }
        return 1;
    }
    return 0;
}

