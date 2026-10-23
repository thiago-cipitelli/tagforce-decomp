// @range 0x0880B448-0x0880FDF4
// @asm 0x0880C15C-0x0880C50C
// @asm 0x0880C60C-0x0880D288
// @asm 0x0880DC1C-0x0880E388
// @asm 0x0880E690-0x0880EA18
// @asm 0x0880EC5C-0x0880EFAC
// @asm 0x0880F1D0-0x0880F278
// @asm 0x0880F3B4-0x0880F8C4
/* Computer player: values it declares for cards that ask for a monster type, an attribute or a
   number. */
#include "duel.h"
#include "card_db.h"

int duel_0880A2B8(unsigned int p, unsigned int q, int skip, int fa, int fd);
unsigned int duel_field_type(unsigned int p, int z);
int duel_rand(int n);
int field_get_card(unsigned int p, int z);
int duel_attack_check(unsigned int p, int z, int arg);
int duel_can_direct_attack(unsigned int p, int z);
int duel_direct_attack_kind(unsigned int p, int z, int arg);
int duel_can_attack_target(unsigned int p, int z, unsigned int q, int i, int arg);
int duel_monster_can_act(unsigned int p, int z);
int duel_08805810(unsigned int p, int z);
void duel_memcpy(void *dst, void *src, int n);
int duel_can_attack_any(unsigned int p, int z, int arg);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_count_monsters(unsigned int p);
int duel_08836868(unsigned int p);
void duel_0880546C(unsigned int p, int z, DuelStats *st);
unsigned int duel_08827D98(unsigned int p, int uid);
int duel_08806730(unsigned int p, int n, unsigned short id);
int duel_08805DA4(unsigned int p, int id, int set, int tribute);
int duel_088078AC(unsigned int p, int uid, int a);
int duel_0880750C(unsigned int p, int id, int a, int n, unsigned short *buf);
int duel_double_tribute(unsigned int p, int id, unsigned int q, int z);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_field_def(unsigned int p, int z);
int duel_088055A0(unsigned int p, int atk, int def);
int duel_088070A8(int id);
int duel_can_activate(unsigned int p);
int duel_08809F2C(unsigned int p);
int duel_count_grave_type(unsigned int p, unsigned int type);
int duel_08807434(unsigned int p, int a);
void duel_08809FC4(unsigned int p, int *atk, int *def, int f);
int duel_count_free_monster_zones(unsigned int p);
int duel_088154D8(unsigned int p);
int duel_list_card_level(unsigned int p, int i);
int duel_0884E948(DuelAct *a, unsigned int p);
int duel_card_fits_zone(unsigned int p, int z, unsigned short id);
unsigned int duel_card_locate(unsigned int uid);
DuelCard *duel_field_card_deb2c(unsigned int p, int zone, int i);
int duel_0880B07C(unsigned int p);

extern unsigned int duel_turn_player;
extern int duel_phase;

/* the two halfwords at +0x14 of the card being activated, as the AI conditions read them */
#define ACT_H14(a) (*(unsigned short *)((char *)(a) + 0x14))
#define ACT_H16(a) (*(unsigned short *)((char *)(a) + 0x16))
int duel_phase_ok(unsigned int p);
int duel_pick_monster_zone(unsigned int p);
int duel_field_base_atk(unsigned int p, int z);
void duel_bzero(void *p, int n);

typedef struct DuelAiView {            /* 0x08913CF4 */
    unsigned int player;               /* player the computer is choosing for (duel_w_08913CF4) */
    int unk4[5];
    DuelStats st[2][5];                /* +0x18 monsters as the computer sees them */
} DuelAiView;

extern DuelAiView duel_w_08913CF4;

typedef struct DuelAtkGuess {          /* result of duel_0880C60C */
    int direct;                        /* the attack goes to the player */
    int ok;                            /* the computer should attack */
    int z;                             /* attacker zone */
    int tz;                            /* target zone */
    int lost;                          /* the attacker would be destroyed */
    int won;                           /* the target would be destroyed */
    int dmg_self;                      /* +0x18 damage to the attacking player */
    int dmg_opp;                       /* +0x1C damage to the opponent */
} DuelAtkGuess;

void duel_0880C60C(unsigned int p, int z, int tz, int force, DuelAtkGuess *g);

extern DuelAtkGuess duel_w_08913EF0;   /* attack chosen by the computer */
extern int duel_w_08913EC4;            /* duel_w_08913ED8 is up to date */
extern int duel_w_08913EC8;            /* own monsters lost in the simulated battles */
extern int duel_w_08913ECC;            /* opponent monsters destroyed in them */
extern int duel_w_08913ED0;
extern int duel_w_08913ED4;
extern int duel_w_08913ED8;            /* damage the computer expects to deal this turn */
extern int duel_w_08913EDC[5];
extern unsigned short duel_w_08914A18;  /* duel_chain + 0x6C8 */
extern int duel_w_089149F0;            /* duel_chain + 0x6A0 */
extern unsigned int duel_w_08914A00;   /* duel_chain + 0x6B0 */
extern unsigned short duel_w_08914A24;  /* duel_chain + 0x6D4 */         /* per attacker: damage + 1 of its planned attack */

/* Monster type (1-20) the computer declares for card id. */
int duel_0880B448(unsigned int p, int id)
{
    int r = 0;
    int i;
    int z;
    unsigned int q;

    switch (id) {
    case 0x1357:
        /* the type of the first card of p in the duel that is known for one */
        for (i = 1; i < 512; i++) {
            if ((i & 1) != p) {
                continue;
            }
            switch (duel.list[i].id) {
            case 0x119A:
            case 0x128B:
            case 0x1377:
            case 0x14CE:
            case 0x14DE:
            case 0x14DF:
            case 0x14E0:
            case 0x14E1:
            case 0x14E2:
            case 0x14E3:
            case 0x14E4:
            case 0x157E:
            case 0x15FC:
            case 0x16EC:
            case 0x1879:
            case 0x1921:
                return 1;
            case 0xFF9:
            case 0x10E8:
            case 0x153E:
            case 0x1744:
            case 0x1893:
                return 2;
            case 0x14B5:
            case 0x14B9:
            case 0x14BC:
            case 0x14BE:
            case 0x14C1:
            case 0x14C2:
            case 0x14CD:
            case 0x1755:
            case 0x1985:
            case 0x19A5:
            case 0x170D:
                return 3;
            case 0x1756:
            case 0x1835:
            case 0x1950:
                return 4;
            case 0x10EB:
            case 0x113D:
            case 0x12C6:
            case 0x13B0:
            case 0x1409:
            case 0x157A:
            case 0x16AC:
            case 0x1825:
            case 0x184B:
            case 0x18FE:
            case 0x191C:
            case 0x19BE:
            case 0x1A07:
            case 0x1A4E:
            case 0x1A51:
            case 0x1A5B:
            case 0x1A7F:
                return 7;
            case 0x10EC:
            case 0x1AA3:
            case 0x1AA6:
                return 9;
            case 0x10D7:
            case 0x10D8:
            case 0x12E0:
            case 0x13A9:
            case 0x13FC:
            case 0x1702:
            case 0x1703:
            case 0x1704:
            case 0x1941:
                return 10;
            case 0x10DA:
            case 0x1862:
            case 0x187B:
            case 0x1882:
            case 0x1889:
            case 0x16FC:
            case 0x16CE:
            case 0x16F9:
                return 11;
            case 0x10E5:
            case 0x1729:
                return 12;
            case 0x1091:
            case 0x10DC:
            case 0x13A0:
            case 0x1753:
                return 13;
            case 0x10EE:
                return 14;
            case 0x10D3:
            case 0x1399:
            case 0x13AA:
            case 0x13F6:
            case 0x149F:
            case 0x14C5:
            case 0x14C6:
            case 0x14C9:
            case 0x14CA:
            case 0x14CF:
            case 0x150C:
            case 0x18FB:
            case 0x1938:
            case 0x193A:
            case 0x1980:
            case 0x170E:
            case 0x175D:
            case 0x14D3:
                return 15;
            case 0x10ED:
            case 0x1725:
            case 0x183F:
            case 0x1A35:
                return 16;
            case 0x10DE:
            case 0x14ED:
            case 0x175E:
            case 0x1986:
            case 0x1A28:
                return 17;
            case 0x10E9:
            case 0x15FF:
            case 0x1678:
            case 0x16ED:
            case 0x17C6:
            case 0x1818:
            case 0x1819:
            case 0x1984:
                return 18;
            case 0x10E2:
            case 0x1475:
            case 0x1A2F:
                return 19;
            case 0x1715:
                return 20;
            }
        }
        break;
    case 0x1358:
        z = duel_0880A2B8(p, 1 - p, -1, 1, 0);
        if (z >= 0) {
            r = card_get_type(duel.players[(1 - p) & 1].field[z].card.id);
        }
        break;
    case 0x161C:
        q = 1 - p;
        z = duel_0880A2B8(p, q, -1, 1, 1);
        if (z >= 0 && duel.players[q & 1].field[z].faceup != 0) {
            r = (unsigned short)duel_field_type(q, z);
        }
        if (r == 0) {
            for (z = 0; z <= 4; z++) {
                if (duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].faceup != 0) {
                    r = (unsigned short)duel_field_type(q, z);
                    break;
                }
            }
        }
        if (r == 0) {
            for (z = 0; z <= 4; z++) {
                if (duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup != 0) {
                    r = (unsigned short)duel_field_type(p, z);
                    break;
                }
            }
        }
        break;
    }
    if (r == 0 || r > 20) {
        r = duel_rand(20) + 1;
    }
    return r;
}

/* Attributes (bit mask) the computer declares for card id: x = count | mask of allowed bits << 8. */
unsigned int duel_0880BD00(unsigned int p, int id, unsigned short x)
{
    int n = x & 0xFF;
    unsigned int mask = x >> 8 & 0xFF;
    int i;
    int j;
    int z = -1;
    unsigned int r = 0;

    int k;
    int a;
    unsigned int bit;

    switch (id) {
    case 0x171F:
        for (i = 1; i < 512; i++) {
            if ((i & 1) != p) {
                continue;
            }
            switch (duel.list[i].id) {
            case 0x10D9:
            case 0x1242:
            case 0x13A4:
            case 0x16A8:
            case 0x1767:
            case 0x17ED:
                    return 2;
            case 0x10D4:
            case 0x1140:
            case 0x123B:
            case 0x1397:
            case 0x188C:
                    return 4;
            case 0x10DB:
            case 0x16AB:
            case 0x18C0:
            case 0x1928:
            case 0x196C:
            case 0x1A2C:
                    return 8;
            case 0x1234:
            case 0x12EE:
            case 0x17E1:
            case 0x18C1:
            case 0x1929:
            case 0x196D:
            case 0x1A2D:
            case 0x17E3:
            case 0x1827:
            case 0x1860:
                    return 16;
            case 0x10EA:
            case 0x18BF:
            case 0x1927:
            case 0x196B:
            case 0x19F0:
            case 0x19F1:
            case 0x1A2B:
            case 0x17E4:
            case 0x1861:
            case 0x138B:
                    return 32;
            case 0x1184:
            case 0x12EF:
            case 0x182A:
            case 0x18C2:
            case 0x192A:
            case 0x196E:
            case 0x1A2E:
            case 0x1A30:
            case 0x1A40:
            case 0x1826:
                    return 64;
            }
        }
        break;
    }
    /* the attributes of the opponent's strongest monsters */
    for (j = 0; j < 2; j++) {
        z = duel_0880A2B8(p, 1 - p, z, 1, 0);
        if (z >= 0) {
            a = card_get_kind(duel.players[(1 - p) & 1].field[z].card.id);
            if (a > 0 && a < 7) {
                bit = 1 << a;
                if ((mask & bit) && n != 0) {
                    r |= bit;
                    mask ^= bit;
                    n--;
                }
            }
        }
    }
    /* random ones for the rest */
    for (z = 0; z < n; z++) {
        k = duel_rand(6);
        for (i = 0; i < 6; i++) {
            bit = 1 << ((i + k) % 6 + 1);
            if (!(mask & bit)) {
                continue;
            }
            r |= bit;
            mask ^= bit;
            break;
        }
    }
    return r;
}

#ifdef NON_MATCHING /* no caso 0x19C2 o original calcula (1 - p) * 0xDC antes de tz * 0x2C ao indexar duel_w_08913CF4.st; a ordem dos índices não se reproduz */
/* Does the effect of the monster at (p, z) work well against the opponent's monster in zone tz? */
int duel_0880C15C(unsigned int p, int z, int tz)
{
    switch (field_get_card(p, z)) {
    case 0x1524:
    case 0x15E0:
    case 0x17D7:
    case 0x17D8:
    case 0x1823:
    case 0x1829:
        return !duel.players[(1 - p) & 1].field[tz].faceup && duel.players[(1 - p) & 1].field[tz].pos != 0;
    case 0x17DF:
        return duel.players[(1 - p) & 1].field[tz].faceup != 0 && duel.players[(1 - p) & 1].field[tz].pos != 0;
    case 0x1963:
        return duel.players[(1 - p) & 1].field[tz].faceup != 0 && duel_w_08913CF4.st[1 - p][tz].level < 4;
    case 0x1752:
        return duel.players[(1 - p) & 1].field[tz].pos != 0 && duel_w_08913CF4.st[1 - p][tz].def >= 2000;
    case 0x18F3:
        return duel.players[(1 - p) & 1].field[tz].pos;
    case 0x19C2:
        return duel.players[(1 - p) & 1].field[tz].faceup != 0 &&
               (duel.players[p & 1].field[z].link & duel_w_08913CF4.st[1 - p][tz].mask) != 0;
    }
    return 0;
}
#endif

/* Highest ATK the computer knows of p's monsters (except skip); only those that can attack if check. */
int duel_0880C50C(unsigned int p, int skip, int check)
{
    int z;
    int best = -1;
    int v;

    for (z = 0; z <= 4; z++) {
        if (z != skip && duel.players[p & 1].field[z].card.id != 0) {
            if (check != 0) {
                if (duel_attack_check(p, z, 1) == 0) {
                    continue;
                }
            }
            v = duel_w_08913CF4.st[p][z].atk;
            if (best < v) {
                best = v;
            }
        }
    }
    return best;
}

#ifdef NON_MATCHING /* 3 KB; o original salva duel_special membro a membro (parte com lwc1/swc1); mantida em asm */
int duel_08846584();
int duel_088482F8(int a, int b);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_count_monsters_pos_is(unsigned int p, unsigned int pos);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_count_active(int id);
int duel_slot_affected(unsigned int p, int z, int id);
int duel_slot_counter_1439(unsigned int p, int z);

/* Simulate the attack of p's monster at z on the opponent's zone tz (>= 5: direct attack) and judge it. */
void duel_0880C60C(unsigned int p, int z, int tz, int force, DuelAtkGuess *g)
{
    DuelSpecial saved;
    unsigned int q;
    int forced = 0;
    int n;
    int i;
    DuelStats *a;

    g->direct = tz >= 5;
    g->z = z;
    g->tz = tz;
    g->dmg_self = 0;
    g->dmg_opp = 0;
    g->lost = 0;
    g->won = 0;
    if (g->direct == 0 && SLOT_ACTIVE(&duel.players[p & 1].field[z]) && duel_0880C15C(p, z, tz) != 0) {
        g->ok = 1;
        g->won = 1;
        return;
    }
    saved = duel_special;
    q = 1 - p;
    duel_special.unk8 = g->direct;
    duel_special.player[0] = p;
    duel_special.player[1] = q;
    duel_special.zone[0] = z;
    duel_special.zone[1] = tz;
    duel_08846584(0);
    n = duel_find_effect_card(q, 0x1381, -1);
    for (; n != 0; n--) {
        duel_special.act[p].pad14[2] /= 2;
    }
    duel_088482F8(0, 1);
    for (i = 0; i < 2; i++) {
        if (duel_special.recs[i].b[0] == p) {
            g->dmg_self += duel_special.recs[i].w0;
        } else {
            g->dmg_opp += duel_special.recs[i].w0;
        }
    }
    if (g->direct != 0) {
        g->ok = 1;
    } else {
        if (!(duel.players[p & 1].field[z].flags >> 5 & 1)) {
            switch (field_get_card(p, z)) {
            case 0x16BF:
                forced = 1;
                break;
            case 0x13CB:
                if (duel.players[q & 1].field[tz].pos == 0 && duel_count_monsters_pos_is(p, 0) > 1) {
                    g->dmg_opp += 500;
                }
                break;
            case 0x172C:
            case 0x1657:
                if (g->lost != 0) {
                    g->won = 1;
                }
                break;
            case 0x10A6:
                g->won = 1;
                g->lost = 1;
                break;
            }
        }
        if (SLOT_ACTIVE(&duel.players[q & 1].field[tz])) {
            switch (field_get_card(q, tz)) {
            case 0x129A:
                if (duel.players[q & 1].field[tz].pos == 0) {
                    g->dmg_self += duel_w_08913CF4.st[p][z].atk;
                    g->won = 1;
                }
                break;
            case 0x172C:
                if (g->won != 0) {
                    g->lost = 1;
                }
                break;
            case 0x1657:
            case 0x10A6:
                g->won = 1;
                g->lost = 1;
                break;
            }
        }
        if (duel_slot_has_effect(p, 0xB, 0x1669) != 0) {
            forced = 1;
        }
        if (duel_slot_has_effect(p, 0xB, 0x14A6) != 0) {
            forced = 1;
        }
        g->lost |= duel_special.act[p].pad14[6] | duel_special.act[p].pad14[7];
        g->won |= duel_special.act[q].pad14[6] | duel_special.act[q].pad14[7];
        if (g->dmg_opp > 0) {
            g->ok = 1;
        } else {
            g->ok = g->lost < g->won;
        }
        if (g->won != 0 && g->lost != 0) {
            /* both destroyed: still attack with a strong monster when the opponent has nothing as strong */
            a = &duel_w_08913CF4.st[p][z];
            if (a->def < a->atk && (a->level < 5 || a->level <= duel_w_08913CF4.st[q][tz].level)) {
                n = duel_0880C50C(p, z, 1);
                if (n <= a->atk && n >= 1600 && duel_0880C50C(q, tz, 1) < n) {
                    g->ok = 1;
                }
            }
        }
        if (g->lost == 0 && g->won == 0 && duel_count_active(0x166C) == 0) {
            if (duel_slot_affected(p, z, 0x143A) != 0 && duel_slot_counter_1439(q, tz) != 0) {
                g->ok = 1;
            }
            switch (field_get_card(q, tz)) {
            case 0x152E:
                if (duel.players[q & 1].field[tz].faceup == 0 && duel.players[p & 1].lp > 1000 && g->dmg_self <= 1000) {
                    g->ok = 1;
                }
                break;
            case 0x129C:
                if (duel_0880C50C(p, z, 1) >= 1000 && duel.players[p & 1].lp > 1000 && g->dmg_self <= 1000) {
                    g->ok = 1;
                }
                break;
            }
        }
        if (duel.players[p & 1].lp <= g->dmg_self) {
            g->ok = 0;
        }
        if (force != 0 && forced) {
            g->ok = 1;
        }
    }
    duel_special = saved;
}
#endif

/* Is the attack of p's monster at z on zone tz (>= 5: direct) worth trying at all? */
int duel_0880D288(unsigned int p, int z, int tz, int careful)
{
    unsigned int q = 1 - p;
    int direct = tz >= 5;
    int id;
    int v;
    int i;
    int any;
    int zz;

    if (!(duel.players[p & 1].field[z].flags >> 5 & 1)) {
        switch (field_get_card(p, z)) {
        case 0x1115:
            if ((duel_can_direct_attack(p, z) == 0 || duel_w_08913CF4.st[p][z].atk < duel.players[q & 1].lp) &&
                careful != 0 && duel.players[p & 1].lp > 4000) {
                return 0;
            }
            break;
        case 0x128F:
        case 0x12A0:
            if (careful != 0) {
                v = duel_w_08913CF4.st[p][z].atk;
                any = 0;
                for (i = 0; i < 5; i++) {
                    if (duel.players[p & 1].field[i].card.id != 0 && duel_w_08913CF4.st[p][i].atk < v) {
                        any = 1;
                    }
                }
                if (!any) {
                    return 0;
                }
            }
            break;
        case 0x1419:
        case 0x1566:
        case 0x15D2:
        case 0x1912:
        case 0x1915:
            if (!direct) {
                if (duel.players[q & 1].field[tz].pos != 0) {
                    if (duel_w_08913CF4.st[q][tz].def < 1900) {
                        return 0;
                    }
                } else {
                    v = duel_w_08913CF4.st[q][tz].atk;
                    if (v > 1000 && v < 1900) {
                        return 0;
                    }
                }
            }
            break;
        case 0x13A7:
            if (duel.players[q & 1].field[tz].pos != 0) {
                return 0;
            }
            if (duel.players[p & 1].lp <= 2000) {
                return 0;
            }
            break;
        case 0x14D7:
            if (duel.players[q & 1].field[tz].pos != 0) {
                return 0;
            }
            break;
        case 0x182D:
            if (direct) {
                return 1;
            }
            break;
        case 0x16BF:
            return 1;
        }
    }
    if (!direct) {
        switch (field_get_card(q, tz)) {
        case 0x148A:
            /* it cannot be attacked while all the monsters of q are face-up copies of it */
            if (duel.players[q & 1].field[tz].faceup != 0) {
                for (zz = 0; zz <= 4; zz++) {
                    id = field_get_card(q, zz);
                    if (id != 0 && (duel.players[q & 1].field[zz].faceup == 0 || id != 0x148A)) {
                        return 0;
                    }
                }
            }
        }
        if (duel_monster_can_act(p, z) != 0 && duel_08805810(p, z) == 1 &&
            duel_w_08913CF4.st[p][z].atk + 1000 <= duel_0880C50C(q, tz, 1)) {
            return 0;
        }
    } else {
        if (duel_w_08913CF4.st[p][z].atk >= duel.players[q & 1].lp) {
            return 1;
        }
        switch (field_get_card(p, z)) {
        case 0x14D6:
        case 0x1993:
        case 0x1419:
        case 0x1566:
        case 0x15D2:
        case 0x1912:
        case 0x1915:
        case 0x1A61:
        case 0x1A52:
        case 0x1A78:
            break;
        default:
            if (duel_monster_can_act(p, z) != 0) {
                v = duel_w_08913CF4.st[p][z].atk;
                if (v < 300 || v + 1000 <= duel_0880C50C(q, -1, 1)) {
                    return 0;
                }
            }
            break;
        }
    }
    return 1;
}

/* Best target for p's monster at z: fills g, returns whether to attack. */
int duel_0880D924(unsigned int p, int z, int careful, DuelAtkGuess *g, int force)
{
    unsigned int q;
    int i;
    int better;
    int r;
    DuelAtkGuess t;

    if (duel_can_direct_attack(p, z) != 0 ||
        (duel_direct_attack_kind(p, z, 1) != 0 && duel_find_effect_card(1 - p, 0x17FE, -1) == 0)) {
        if (force != 0 || duel_0880D288(p, z, 5, careful) != 0) {
            duel_0880C60C(p, z, 5, force, g);
            r = force != 0;
            if (!r) {
                r = g->dmg_opp > 0;
            }
            return r;
        }
        return 0;
    }
    g->ok = 0;
    g->direct = 0;
    g->dmg_self = 0;
    g->dmg_opp = 0xFFFF0000;
    q = 1 - p;
    for (i = 0; i < 5; i++) {
        if (duel_can_attack_target(p, z, q, i, 1) != 0 && (force != 0 || duel_0880D288(p, z, i, careful) != 0)) {
            better = 0;
            duel_0880C60C(p, z, i, force, &t);
            if (force != 0 && g->ok == 0) {
                better = 1;
            }
            if (t.ok > g->ok) {
                better = 1;
            }
            if (t.ok == g->ok) {
                if (t.dmg_opp > g->dmg_opp) {
                    better = 1;
                } else if (t.dmg_opp == g->dmg_opp) {
                    if (t.won > g->won) {
                        better = 1;
                    } else if (duel.players[q & 1].field[t.tz].faceup == 0) {
                        better = 1;
                    } else if (duel_w_08913CF4.st[q][t.tz].def > duel_w_08913CF4.st[q][g->tz].def) {
                        better = 1;
                    }
                }
            }
            if (better) {
                duel_memcpy(g, &t, sizeof(t));
            }
        }
    }
    r = force != 0;
    if (!r) {
        r = g->ok != 0;
    }
    return r;
}

#ifdef NON_MATCHING /* o original faz a ordenação inline com o cálculo da prioridade dentro do laço; mantida em asm */
/* Order of p's attackers: low priority and low ATK first. */
static unsigned int duel_attacker_key(unsigned int p, int z)
{
    unsigned int atk = duel_w_08913CF4.st[p][z].atk;
    int prio = 10;
    int i;

    if (!(duel.players[p & 1].field[z].flags >> 5 & 1)) {
        switch (field_get_card(p, z)) {
        case 0x18A4:
        case 0x11CD:
        case 0x128F:
        case 0x12A0:
            prio = 11;
            break;
        case 0x147D:
            prio = 9;
            break;
        case 0x1524:
        case 0x15E0:
        case 0x1823:
        case 0x1829:
        case 0x17DF:
        case 0x1963:
        case 0x1752:
        case 0x18F3:
        case 0x19C2:
            for (i = 0; i < 5; i++) {
                if (duel.players[(1 - p) & 1].field[i].card.id != 0 && duel_0880C15C(p, z, i) != 0) {
                    prio = 9;
                    break;
                }
            }
            break;
        case 0x13B1: case 0x16CB: case 0x17E3: case 0x1826: case 0x1861: case 0x19D4: case 0x14D5:
        case 0x19F8: case 0x1A93: case 0x1529: case 0x17D2: case 0x17D3: case 0x17D7: case 0x17D8:
        case 0x17DA: case 0x147A: case 0x1520: case 0x158F: case 0x1592: case 0x15DC: case 0x164D:
        case 0x1641: case 0x16C5: case 0x1704: case 0x170B: case 0x172A: case 0x172B: case 0x174A:
        case 0x174B: case 0x1791: case 0x1792: case 0x17C7: case 0x17C8: case 0x18AE: case 0x18C8:
        case 0x193B: case 0x1943: case 0x194F: case 0x1987: case 0x19A4: case 0x19D3: case 0x1A23:
        case 0x1A95:
            if (duel_count_monsters(1 - p) != 0) {
                prio = 8;
            }
            break;
        case 0x1A4E:
        case 0x1505:
        case 0x1644:
        case 0x1958:
        case 0x186C:
        case 0x19C1:
            prio = 7;
            break;
        }
    }
    return (atk & 0xFFFF) | prio << 16;
}

/* Pick the computer's next attack into duel_w_08913EF0; mode < 0 skips monsters that attack twice. */
int duel_0880DC1C(unsigned int p, int mode)
{
    int list[5];
    int n = 0;
    int z;
    int i;
    int j;
    int k;
    int t;
    unsigned int key;
    unsigned int min;
    DuelAtkGuess g;

    duel_w_08913EF0.direct = 0;
    duel_w_08913EF0.dmg_self = 0;
    duel_w_08913EF0.dmg_opp = 0;
    duel_w_08913EF0.lost = 0;
    duel_w_08913EF0.won = 0;
    for (z = 0; z < 5; z++) {
        if (duel_can_attack_any(p, z, 1) == 0) {
            continue;
        }
        if (mode < 0 && duel.players[p & 1].field[z].flags >> 22 & 1) {
            if (duel_slot_has_effect(p, 0xB, 0x15FF) != 0 || duel_slot_has_effect(p, 0xB, 0x195B) != 0) {
                continue;
            }
            if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
                switch (field_get_card(p, z)) {
                case 0x1A4E:
                case 0x19C1:
                case 0x186C:
                case 0x1958:
                case 0x1644:
                case 0x1505:
                    continue;
                }
            }
        }
        list[n++] = z;
    }
    if (n == 0) {
        return 0;
    }
    for (i = 0; i < n - 1; i++) {
        k = i;
        min = 0xFFFFFFFF;
        for (j = i; j < n; j++) {
            key = duel_attacker_key(p, list[j]);
            if (key < min) {
                min = key;
                k = j;
            }
        }
        t = list[i];
        list[i] = list[k];
        list[k] = t;
    }
    for (i = 0; i < n; i++) {
        if (duel_0880D924(p, list[i], 1, &g, 0) != 0) {
            duel_memcpy(&duel_w_08913EF0, &g, sizeof(g));
            return 1;
        }
    }
    if (mode > 0) {
        for (i = n - 1; i >= 0; i--) {
            if (duel_0880D924(p, list[i], 1, &g, 1) != 0) {
                duel_memcpy(&duel_w_08913EF0, &g, sizeof(g));
                return 1;
            }
        }
    }
    return 0;
}
#endif

/* Save the monster zones of both players into buf (200 bytes). */
void duel_0880E388(DuelSlot *buf, unsigned int p)
{
    duel_memcpy(buf, duel.players[0].field, sizeof(DuelSlot) * 5);
    duel_memcpy(buf + 5, duel.players[1].field, sizeof(DuelSlot) * 5);
}

void duel_0880E3CC(DuelSlot *buf, unsigned int p)
{
    duel_memcpy(duel.players[0].field, buf, sizeof(DuelSlot) * 5);
    duel_memcpy(duel.players[1].field, buf + 5, sizeof(DuelSlot) * 5);
}

/* Put p's monsters in attack position as the computer would (simulation). */
void duel_0880E414(unsigned int p)
{
    int z;
    int id;

    for (z = 0; z < 5; z++) {
        id = duel.players[p & 1].field[z].card.id;
        if (id == 0) {
            continue;
        }
        if (duel.players[p & 1].field[z].faceup == 0 && duel_phase_ok(p) == 0) {
            continue;
        }
        if (duel.players[p & 1].field[z].faceup == 0 && card_get_special_group(id) != 0) {
            continue;
        }
        if (duel.players[p & 1].field[z].pos != 0 && duel_monster_can_act(p, z) != 0) {
            duel.players[p & 1].field[z].pos = 0;
            duel.players[p & 1].field[z].faceup = 1;
        }
    }
}

/* Simulate the tribute summon of card id in position pos with the n tributes in list. */
int duel_0880E4F0(unsigned int p, int id, unsigned short pos, int n, unsigned short *list)
{
    int i;
    int sum = 0;
    int z;
    unsigned int tp;
    unsigned int tz;

    for (i = 0; i < n; i++) {
        tp = list[i] & 0xFF;
        tz = list[i] >> 8 & 0xFF;
        sum += duel_field_base_atk(tp, tz);
        duel_bzero(&duel.players[tp & 1].field[tz], sizeof(DuelSlot));
    }
    z = duel_pick_monster_zone(p);
    if (z >= 0) {
        duel.players[p & 1].field[z].card.id = id;
        duel.players[p & 1].field[z].faceup = 1;
        duel.players[p & 1].field[z].pos = pos;
        switch (id) {
        case 0x1688:
            sum *= 2;
        case 5000:
        case 0x1654:
            duel.players[p & 1].field[z].link = sum;
            break;
        }
    }
    return z;
}

/* Refresh duel_w_08913CF4.st with what the computer knows of every monster. */
static inline void duel_ai_view_refresh(void)
{
    int q;
    int z;

    for (q = 0; q < 2; q++) {
        for (z = 0; z < 5; z++) {
            if (duel.players[q & 1].field[z].card.id != 0) {
                duel_0880546C(q, z, &duel_w_08913CF4.st[q][z]);
            }
        }
    }
}

#ifdef NON_MATCHING /* laços de 5 zonas desenrolados pelo compilador e acessos a duel_w_08913EF0 pela base; não reproduzido */
/* Simulate all the attacks the computer would make this turn; returns the total damage dealt. */
int duel_0880E690(unsigned int p)
{
    int total = 0;
    int z;

    duel_w_08913EC8 = 0;
    duel_w_08913ECC = 0;
    for (z = 0; z < 5; z++) {
        duel.players[p & 1].field[z].flags &= ~0x400000;
        duel.players[p & 1].field[z].flags &= ~0x800000;
        duel_w_08913EDC[z] = 0;
    }
    if (duel_08836868(p) == 0) {
        return 0;
    }
    duel_ai_view_refresh();
    if (duel_0880DC1C(p, -1) != 0) {
        do {
            z = duel_w_08913EF0.z;
            duel.players[p & 1].field[z].flags |= 1 << (duel.players[p & 1].field[z].flags >> 22 & 1 ? 23 : 22);
            if (duel_w_08913EF0.lost != 0) {
                duel.players[p & 1].field[z].card.id = 0;
                duel_w_08913EC8++;
            }
            if (duel_w_08913EF0.won != 0) {
                duel.players[(1 - p) & 1].field[duel_w_08913EF0.tz].card.id = 0;
                duel_w_08913ECC++;
            }
            duel_w_08913EDC[z] = duel_w_08913EF0.dmg_opp + 1;
            total += duel_w_08913EF0.dmg_opp;
            duel_ai_view_refresh();
        } while (duel_0880DC1C(p, -1) != 0);
    }
    return total;
}
#endif

/* Damage the computer expects to deal this turn (cached until duel_w_08913EC4 is cleared). */
int duel_0880EA18(unsigned int p)
{
    DuelSlot buf[10];

    if (duel_w_08913EC4 == 0) {
        duel_0880E388(buf, p);
        duel_0880E414(p);
        duel_w_08913ED8 = duel_0880E690(p);
        duel_w_08913EC4 = 1;
        duel_0880E3CC(buf, p);
    }
    return duel_w_08913ED8;
}

int duel_0880EA88(unsigned int p, int mode)
{
    duel_w_08913CF4.player = p;
    duel_ai_view_refresh();
    return duel_0880DC1C(p, mode);
}

int duel_0880EB64(unsigned int p, int z, int force)
{
    duel_w_08913CF4.player = p;
    duel_ai_view_refresh();
    return duel_0880D924(p, z, 1, &duel_w_08913EF0, force);
}

#ifdef NON_MATCHING /* função grande com a simulação de cada invocação; mantida em asm */
/* Hand index of the monster whose tribute summon (no tributes if notrib) lets the computer deal most damage. */
int duel_0880EC5C(unsigned int p, int notrib)
{
    int best = -1;
    int base = duel_0880EA18(p);
    int best_ecc = duel_w_08913ECC;
    unsigned int i;
    int id;
    int uid;
    int n;
    int k;
    int ok;
    int z;
    int v;
    unsigned int f;
    unsigned int r;
    int better;
    unsigned short trib[4];
    DuelSlot buf[10];
    DuelStats st;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        id = duel.players[p & 1].hand[i].id;
        uid = CARD_UID(duel.players[p & 1].hand[i]);
        if (!(duel_08827D98(p, uid) & 0x10) || duel_08806730(p, 2, id) != 0 || duel_08805DA4(p, id, 0, 0) == 1) {
            continue;
        }
        n = duel_088078AC(p, uid, 0);
        ok = 1;
        if (notrib == (n == 0)) {
            continue;
        }
        for (k = 0; k < n; k++) {
            r = duel_0880750C(p, id, 0, k, trib);
            if ((int)r < 0) {
                ok = 0;
                break;
            }
            if (duel_double_tribute(p, id, r & 0xFF, (unsigned short)r >> 8 & 0xFF)) {
                trib[k] = r;
                k++;
            }
            trib[k] = r;
        }
        if (!ok) {
            continue;
        }
        duel_0880E388(buf, p);
        duel_0880E414(p);
        z = duel_0880E4F0(p, id, 0, n, trib);
        if (z >= 0) {
            duel_field_stats(p, z, &st);
        }
        v = duel_0880E690(p);
        duel_w_08913ED0 = duel_count_monsters(p);
        duel_w_08913ED4 = duel_count_monsters(1 - p);
        f = duel.players[p & 1].field[z].flags;
        duel_0880E3CC(buf, p);
        if (z >= 0 && st.atk >= 900 && f >> 22 & 1) {
            better = base < v;
            if (v == base && best_ecc < duel_w_08913ECC && duel_w_08913ED0 != 0) {
                better = 1;
            }
            if (better) {
                best_ecc = duel_w_08913ECC;
                base = v;
                best = i;
            }
        }
    }
    return best;
}
#endif

/* Stats card id would have after its tribute summon with the n tributes in list (or from the database). */
void duel_0880EFAC(unsigned int p, int id, int n, unsigned short *list, DuelStats *st)
{
    DuelSlot buf[10];
    int z;
    unsigned int v;

    duel_0880E388(buf, p);
    z = duel_0880E4F0(p, id, 0, n, list);
    if (z >= 0) {
        if (duel_w_08914A18 & 1) {
            switch (id) {
            case 0x18B4:
                duel.players[p & 1].field[z].link = duel_w_08914A24 * 700;
                break;
            case 0x18FB:
                v = duel_w_08914A00;
                duel.players[p & 1].field[z].link = v > 0xFFFF ? 0xFFFF : v;
                break;
            case 0x1A4E:
                duel.players[p & 1].field[z].link = duel_w_089149F0;
                break;
            }
        }
        duel_field_stats(p, z, st);
        duel.players[p & 1].field[z].pos = 1;
        st->def = duel_field_def(p, z);
    } else {
        st->id = id;
        st->unk8 = card_get_kind(id);
        st->level = card_get_level(id);
        st->attr = card_get_type(id);
        st->atk = card_get_atk(id);
        st->def = card_get_def(id);
    }
    duel_0880E3CC(buf, p);
}

#ifdef NON_MATCHING /* só o primeiro teste r == -1 difere: o original usa bne com nop, o mwcc gera bnel com o lw ra no atraso */
/* Position for the summon of card id with the n tributes in list: 1 attack, 0 defense. */
int duel_0880F1D0(unsigned int p, int id, int set, int n, unsigned short *list, int tribute)
{
    int r;
    DuelStats st;

    duel_w_08913CF4.player = p;
    r = duel_08805DA4(p, id, set, tribute);
    if (r == -1) {
        duel_0880EFAC(p, id, n, list, &st);
        r = duel_088055A0(p, st.atk, st.def);
        switch (r) {
        case -1:
            r = !(st.atk > st.def);
        }
    }
    return r;
}
#endif

/* Cards the computer summons as soon as it can. */
int duel_0880F278(unsigned int p, int id)
{
    if (duel_088070A8(id) != 0) {
        return 1;
    }
    if (duel_can_activate(p) != 0) {
        switch (id) {
        case 0x179A:
        case 0x119B:
        case 0x11C3:
            return duel_count_monsters(1 - p) > 0;
        case 0x1152:
            return duel_count_grave_type(p, 0x16) > 0;
        case 0x100C:
            return duel_count_grave_type(p, 0x17) > 0;
        case 0x11C2:
        case 0x11F5:
            return duel.players[p & 1].n_hand < 3;
        case 0x1369:
        case 0x1331:
            return duel_08809F2C(p) < 0;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* função grande (escolha da invocação com oferendas); mantida em asm */
/* Hand index of the monster the computer summons (pos: 1 attack, 0 defense), or -1. */
int duel_0880F3B4(unsigned int p, int *pos)
{
    int best = -1;
    int trib = duel_08807434(p, 1);
    int oatk;
    int odef;
    int matk;
    int mdef;
    int mmax;
    unsigned int i;
    int id;
    int uid;
    unsigned int f;
    int n;
    int k;
    int ok;
    int r;
    int v;
    int atk;
    int a;
    unsigned short buf[4];
    DuelStats st;

    duel_08809FC4(1 - p, &oatk, &odef, 0);
    duel_08809FC4(p, &matk, &mdef, 0);
    mmax = MAX(matk, mdef);
    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        id = duel.players[p & 1].hand[i].id;
        uid = CARD_UID(duel.players[p & 1].hand[i]);
        if (card_is_monster(id) == 0 || duel_08806730(p, 2, id) != 0) {
            continue;
        }
        f = duel_08827D98(p, uid);
        if (f == 0) {
            continue;
        }
        n = duel_088078AC(p, uid, 1);
        ok = 1;
        for (k = 0; k < n; k++) {
            r = duel_0880750C(p, id, 0, k, buf);
            if (r < 0) {
                ok = 0;
                break;
            }
            if (duel_double_tribute(p, id, r & 0xFF, (unsigned short)r >> 8 & 0xFF)) {
                buf[k] = r;
                k++;
            }
            buf[k] = r;
        }
        if (!ok) {
            continue;
        }
        r = duel_08805DA4(p, id, 0, 0);
        duel_0880EFAC(p, id, n, buf, &st);
        if (r == -1) {
            r = duel_088055A0(p, st.atk, st.def);
            if (r == -1) {
                r = !(st.atk > st.def);
            }
        }
        a = r == 0;
        if ((f == 0x10 && !a) || (f == 0x40 && a)) {
            continue;
        }
        atk = MAX(st.atk, st.def);
        if (trib != -1 && n != 0) {
            *pos = !a;
            best = i;
        }
        if (oatk < atk) {
            *pos = !a;
            best = i;
        }
        if (duel_0880F278(p, id) != 0) {
            *pos = !a;
            best = i;
        }
        if (mmax < atk) {
            *pos = !a;
            best = i;
        }
        if (n == 0) {
            *pos = !a;
            best = i;
        }
    }
    if (best < 0) {
        if (duel_count_free_monster_zones(p) == 0) {
            return -1;
        }
        v = duel_088154D8(p) != 0 ? 1 : 2;
        for (i = 0; i < duel.players[p & 1].n_hand; i++) {
            id = duel.players[p & 1].hand[i].id;
            uid = CARD_UID(duel.players[p & 1].hand[i]);
            if (card_is_monster(id) != 0 && duel_list_card_level(p, uid) < 5 && duel_08806730(p, v, id) == 0) {
                f = duel_08827D98(p, uid);
                if (f != 0) {
                    *pos = (f & 0x40) != 0;
                    return i;
                }
            }
        }
        best = -1;
    }
    return best;
}
#endif

/* Conditions of the computer player for activating cards (DuelAct of the card). */
int duel_0880F8C4(DuelAct *a)
{
    return duel_turn_player != a->player && duel_phase == 5;
}

int duel_0880F8FC(DuelAct *a)
{
    return duel_turn_player != a->player;
}

int duel_0880F918(DuelAct *a)
{
    unsigned short v = ACT_H16(a);
    unsigned short *w = &ACT_H14(a);

    if (v >= duel.players[(1 - a->player) & 1].lp) {
        return 1;
    }
    return !(*w < v);
}

/* The attacked monster is in attack position and the attacker in defense... of different players. */
int duel_0880F974(void)
{
    if (duel_special.player[0] == duel_special.player[1]) {
        return 0;
    }
    if (duel.players[duel_special.player[0] & 1].field[duel_special.zone[0]].pos != 0) {
        return 0;
    }
    return !duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].pos;
}

int duel_0880FA10(DuelAct *a)
{
    unsigned short v = ACT_H14(a);

    if (v >= duel.players[a->player & 1].lp) {
        return 1;
    }
    return !(v < 1500);
}

int duel_0880FA5C(DuelAct *a)
{
    unsigned int q;
    int z;
    int id;

    if (duel_0884E948(a, a->player) == 0) {
        return 0;
    }
    for (z = 5; z < 10; z++) {
        q = 1 - a->player;
        id = duel.players[q & 1].field[z].card.id;
        if (id != 0 && duel.players[q & 1].field[z].faceup != 0 && duel_card_fits_zone(q, z, id) != 0) {
            return 1;
        }
    }
    return 0;
}

/* A pending summon (kind 0x16) of a strong monster of a's player, or of 0x11E4/0xFD6 of its own. */
int duel_0880FB28(DuelAct *a)
{
    unsigned int i;
    unsigned int loc;
    DuelCard *c;
    struct DuelActX *x;

    for (i = 0; i < duel_chain.n_kinds; i++) {
        if (duel_chain.kinds[i] == 0x16) {
            x = &duel_chain.srcs[i];
            loc = duel_card_locate(x->s_uid);
            if (loc != 0x1000) {
                c = duel_field_card_deb2c(loc & 0xFF, (unsigned short)loc >> 8 & 0xFF, loc >> 16 & 0xFFFF);
                if (x->s_player == a->player && (int)card_get_level(c->id) > 4) {
                    return 1;
                }
                switch (c->id) {
                case 0xFD6:
                case 0x11E4:
                    if (c->owner == a->player) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/* The opponent is summoning a monster p's side may not handle (few cards in hand). */
int duel_0880FC78(DuelAct *a)
{
    unsigned int i;
    int id;
    int atk;
    int def;

    if (duel.players[a->player & 1].n_hand > 2) {
        return 0;
    }
    if (duel_0880B07C(a->player) == 0) {
        return 0;
    }
    duel_08809FC4(1 - a->player, &atk, &def, 1);
    for (i = 0; i < duel_chain.n_kinds; i++) {
        if (duel_chain.kinds[i] == 0x16) {
            id = duel.list[duel_chain.srcs[i].s_uid].id;
            if ((int)card_get_level(id) > 4) {
                return 1;
            }
            if (card_attribute_icon_in_2_3(id) != 0) {
                return 1;
            }
            if (card_get_atk(id) > 1400 && !(card_get_atk(id) + 800 < atk)) {
                return 1;
            }
        }
    }
    return 0;
}
