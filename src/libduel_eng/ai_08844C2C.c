// @range 0x08844C2C-0x088451E8
// @asm 0x08844C2C-0x08844E24
/* Computer player: last responses of the zone tables, value of some cards and selection filters. */
#include "duel.h"

extern unsigned int duel_turn_player;
extern unsigned int duel_w_08919604;
extern unsigned int duel_w_08919608;
extern unsigned short duel_w_0891968C;

int duel_08843858(unsigned int p, int id);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_0886B0E4(unsigned int a, unsigned short uid, int c);
void duel_0884F91C(unsigned int a, unsigned short b);
void duel_08832950(unsigned int p, unsigned short id);
int field_get_card(unsigned int p, int z);
int duel_088D1438(unsigned int p, int z, int a, int b);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_slot_e68a0(unsigned int p, int z);

#ifdef NON_MATCHING /* registradores e o teste 5 < 10 que o original não dobra */
int duel_08844C2C(unsigned int p, int id)
{
    unsigned int z;

    switch (duel_w_08919608) {
    case 0:
        duel_w_08919608 = 5;
        for (; (z = duel_w_08919608) < 10; duel_w_08919608++) {
            if (duel_slot_e8170(p, z, id) != 0
                && duel_0886B0E4((p << 31 & 0x80000000) | 0x4200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                                 CARD_UID(duel.players[p & 1].field[z].card), 0) != 0) {
                duel_08832950(p, id);
                return 0;
            }
        }
        return 1;
    default:
        if (duel_w_0891968C == 0) {
            return 1;
        }
        z = duel_w_08919608;
        duel_0884F91C((p << 31 & 0x80000000) | 0x4200000 | (z << 16 & 0x1F0000) | (field_get_card(p, z) & 0xFFFF),
                      CARD_UID(duel.players[p & 1].field[z].card));
        duel_w_08919608 = 0;
        return 0;
    }
}
#endif

int duel_08844E24(unsigned int p)
{
    return duel_08844C2C(0x1 - p, 0x1367);
}

int duel_08844E34(unsigned int p)
{
    return duel_08844C2C(p, 0x16D5);
}

int duel_08844E3C(unsigned int p)
{
    return duel_08843858(0x1 - p, 0x1485);
}

int duel_08844E4C(unsigned int p)
{
    unsigned int i;
    unsigned int q;
    int z;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = p ^ i / 5;
        z = i % 5 + 5;
        if (!duel_slot_e8170(q, z, 0x1539)) {
            continue;
        }
        duel_0884F30C((q << 31 & 0x80000000) | 0x4200000 | (z << 16 & 0x1F0000) | (field_get_card(q, z) & 0xFFFF),
                      CARD_UID(duel.players[q & 1].field[z].card), 0);
        duel_w_08919608++;
        return 0;
    }
    return 1;
}

int duel_08844FA4(unsigned int p)
{
    int z;

    if (duel.players[p & 1].n_hand < 5) {
        for (z = 5; z < 10; z++) {
            if (!duel_slot_e8170(p, z, 0x1540)) {
                continue;
            }
            duel_088D1438(p, z, 0, 0);
            return 0;
        }
    }
    return 1;
}

int duel_08845048(unsigned int p)
{
    return duel_08843858(p, 0x1282);
}

int duel_08845050(unsigned int p)
{
    return duel_08843858(p, 0x11EA);
}

int duel_08845058(unsigned int p)
{
    return duel_08843858(p, 0x158E);
}

int duel_08845060(unsigned int p)
{
    return duel_08843858(p, 0x1147);
}

int duel_08845068(unsigned int p)
{
    return duel_08843858(p, 0x1A03);
}

int duel_08845070(int id)
{
    switch (id) {
    case 0x1381:
        return 2000;
    case 0x134A:
        return 100;
    case 0x1360:
        return 700;
    case 0x13F4:
        return 1000;
    case 0x13F9:
        return 500;
    case 0x151A:
        return 500;
    case 0x1639:
        return 1000;
    case 0x168C:
        return 500;
    case 0x168D:
        return 900;
    case 0x168E:
        return 500;
    case 0x168F:
        return 500;
    case 0x1690:
        return 500;
    case 0x1691:
        return 800;
    case 0x1692:
        return 500;
    }
    return 0;
}

int duel_08845124(unsigned int p, int z, int i)
{
    int n = z + i;

    if (n < 5 && p == duel_turn_player && duel_tributable(p, p, n) != 0 && duel_w_08919608 != n) {
        return 0x800;
    }
    return 0;
}

int duel_08845190(unsigned int p, int z, int i)
{
    z += i;
    if (z < 11 && p == (duel_turn_player ^ duel_w_08919604) && duel_slot_e68a0(p, z) != 0) {
        return 0x800;
    }
    return 0;
}
