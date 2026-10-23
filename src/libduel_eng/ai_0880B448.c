// @range 0x0880B448-0x0880C60C
// @asm 0x0880C15C-0x0880C50C
/* Computer player: values it declares for cards that ask for a monster type, an attribute or a
   number. */
#include "duel.h"
#include "card_db.h"

int duel_0880A2B8(unsigned int p, unsigned int q, int skip, int fa, int fd);
unsigned int duel_field_type(unsigned int p, int z);
int duel_rand(int n);
int field_get_card(unsigned int p, int z);
int duel_attack_check(unsigned int p, int z, int arg);

typedef struct DuelAiView {            /* 0x08913CF4 */
    unsigned int player;               /* player the computer is choosing for (duel_w_08913CF4) */
    int unk4[5];
    DuelStats st[2][5];                /* +0x18 monsters as the computer sees them */
} DuelAiView;

extern DuelAiView duel_w_08913CF4;

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
