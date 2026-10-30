// @range 0x088B1EAC-0x088B2D0C
// @asm 0x088B20A0-0x088B23AC
/* Ritual summon checks (after duel_088B1854): level sums, the ritual table and the ritual summon step machine. */
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
extern int duel_step;
extern int duel_player_type[2];
extern unsigned int duel_view_player;
extern int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
int duel_08835AC0(void);
void duel_088359B0(int (*fn)());
void duel_0881956C(unsigned int p, int id, int (*fn)());
void duel_0884ACD8(unsigned int p, int msg);
int duel_0884B078(int a, int b);
int duel_host_call(int a, int b, int c, int d);
int duel_list_card_level(unsigned int p, int i);
int duel_field_level(unsigned int p, int z);
int card_is_attribute9(int id);
unsigned int duel_card_locate(unsigned int uid);
void duel_088CABF4(unsigned int p, int i, int a);
unsigned int duel_088D1BBC(DuelAct *a, unsigned int mask);
int duel_hand_find_name(unsigned int p, int id);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_0884DB38(int a, int b);
void duel_088C78E0(unsigned int p);

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
int duel_088B23AC(DuelAct *a, int b);

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

int duel_088B23AC(DuelAct *a, int b)
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


/* ritual summon (a->w[0]: ritual table entry, w[1]: tributes picked, w[2]: owners of the tributes,
   duel_w_089148FC: level still to pay, duel_w_08914900: lowest level paid) */
int duel_088B2460(DuelAct *a)
{
    unsigned int i;
    int lv;
    int n;
    unsigned int q;
    int z;
    int k;
    int id;
    unsigned int loc;
    unsigned int mask;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80: /* find the ritual monster: in hand for the spells that summon any of them */
        duel_w_089148FC = 0;
        a->w[2] = 0;
        if (!duel_088B23AC(a, 0)) {
            return 0;
        }
        switch (a->id) {
        case 0x1698:
        case 0x1716:
        case 0x19D9:
            if (duel_player_type[a->player] == 1) {
                for (i = 0; i < duel.players[a->player & 1].n_hand; i++) {
                    if (!duel_088B1F74((int)a->player, 0xB, i)) {
                        continue;
                    }
                    id = duel.players[a->player & 1].hand[i].id;
                    for (k = 0; duel_rituals[k].monster != 0; k++) {
                        if (id == duel_rituals[k].monster) {
                            a->w[0] = k;
                        }
                    }
                    return 0x7E;
                }
            } else {
                duel_0884ACD8(a->player, 0x11E);
                duel_088359B0(duel_088B1F74);
            }
            return 0x7F;
        }
        for (k = 0; duel_rituals[k].spell != 0; k++) {
            if (a->id == duel_rituals[k].spell) {
                a->w[0] = k;
                break;
            }
        }
        return 0x7E;
    case 0x7F: /* monster picked by the player */
        if (duel_08835AC0()) {
            id = duel.players[a->player & 1].hand[duel_w_08919654].id;
            for (k = 0; duel_rituals[k].monster != 0; k++) {
                if (id == duel_rituals[k].monster) {
                    a->w[0] = k;
                }
            }
            return 0x7E;
        }
        return 0x80;
    case 0x7E: /* level to pay */
        duel_w_089148FC = duel_rituals[a->w[0]].level;
        duel_w_08914900 = 0xFF;
        a->w[1] = 0;
        if (duel_player_type[a->player] == 0) {
            duel_0884B078(0xCF, duel_w_089148FC);
        }
        return 0x7D;
    case 0x7D: /* ask for a tribute */
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, a->id, duel_088B2058);
        } else {
            duel_088359B0(duel_088B2058);
        }
        return 0x7C;
    case 0x7C: /* tribute picked: from the hand or from the field */
        if (duel_08835AC0()) {
            if (duel_w_08919650 == 0xB) {
                q = duel_w_0891964C;
                i = duel_w_08919654;
                lv = duel_list_card_level(a->player, CARD_UID(duel.players[a->player & 1].hand[i]));
                duel_host_call(0x28, q, 0xB, CARD_UID(duel.players[q & 1].hand[i]));
                duel_chain.uids[(duel_chain.uids_head + a->w[1]) & 0x7F] = CARD_UID(duel.players[q & 1].hand[i]);
                a->w[1]++;
                a->w[2] |= 1 << duel.players[q & 1].hand[i].owner;
            } else {
                unsigned int fq = duel_w_0891964C;
                int fz = duel_w_08919650 + duel_w_08919654;

                lv = duel_field_level(fq, fz);
                duel_host_call(0x28, fq, fz, CARD_UID(duel.players[fq & 1].field[fz].card));
                duel_chain.uids[(duel_chain.uids_head + a->w[1]) & 0x7F] = CARD_UID(duel.players[fq & 1].field[fz].card);
                a->w[1]++;
                if (!card_is_attribute9(duel.players[fq & 1].field[fz].card.id)) {
                    a->w[2] |= 1 << duel.players[fq & 1].field[fz].card.owner;
                }
            }
            n = duel_w_089148FC - lv;
            if (n < 0) {
                n = 0;
            }
            duel_w_089148FC = n;
            duel_w_08914900 = MIN(lv, duel_w_08914900);
            if (*(volatile int *)&duel_w_089148FC == 0) { /* re-read, as the original does */
                return 0x64;
            }
            return 0x7D;
        } else {
            duel_host_call(0x29, 0, 0, 0);
            duel_w_089148FC = duel_rituals[a->w[0]].level;
            duel_w_08914900 = 0xFF;
            a->w[1] = 0;
            return 0x7D;
        }
    case 0x64: /* send the tributes */
        mask = 0;
        for (k = 0; k < a->w[1]; k++) {
            loc = duel_card_locate(duel_chain.uids[(duel_chain.uids_head + k) & 0x7F]);
            z = (unsigned char)((loc & 0xFFFF) >> 8);
            if (z < 0xB) {
                mask |= 1 << (z + (loc & 0xFF) * 16);
            } else {
                duel_088CABF4(loc & 0xFF, (unsigned short)(loc >> 16), 1);
            }
        }
        duel_088D1BBC(a, mask);
        return 0x63;
    case 0x63: /* summon the ritual monster from the hand */
        k = duel_hand_find_name(a->player, duel_rituals[a->w[0]].monster);
        duel_088263A8(a->player, &duel.players[a->player & 1].hand[k], 1, 1);
        if (a->player == duel_view_player) {
            duel_0884DB38(0x24, 1);
        }
        return 0x62;
    case 0x62: /* 0x19DA: each player who gave a tribute [?] */
        if (duel_count_active(0x19DA)) {
            if (a->w[2] & 1 << a->player) {
                duel_088C78E0(a->player);
            }
            if (a->w[2] & 1 << (1 - a->player)) {
                duel_088C78E0(1 - a->player);
            }
        }
        return 0;
    }
    return 0;
}
