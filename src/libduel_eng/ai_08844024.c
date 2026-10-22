// @range 0x08844024-0x0884451C
// @asm 0x088441D0-0x08844324
/* Computer player: more responses from the spell/trap zones (tail calls into duel_08843858/duel_08843998). */
#include "duel.h"

extern unsigned int duel_w_08919608;

int duel_08843858(unsigned int p, int id);
int duel_08843998(unsigned int p, int id);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int field_get_card(unsigned int p, int z);
int duel_zone_f_count_monsters(unsigned int p);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
int duel_088C4730(unsigned int p, int n, int f, int id);

int duel_08844024(unsigned int p)
{
    return duel_08843998(p, 0x14C0);
}

int duel_0884402C(unsigned int p)
{
    return duel_08843998(p, 0x1628);
}

int duel_08844034(unsigned int p)
{
    return duel_08843998(0x1 - p, 0x1A0C);
}

int duel_08844044(unsigned int p)
{
    return duel_08843858(p, 0x16FE);
}

int duel_0884404C(unsigned int p)
{
    return duel_08843858(p, 0x18F8);
}

int duel_08844054(unsigned int p)
{
    return duel_08843858(p, 0x1757);
}

int duel_0884405C(unsigned int p)
{
    return duel_08843858(p, 0x191D);
}

int duel_08844064(unsigned int p)
{
    return duel_08843858(p, 0x19CD);
}

int duel_0884406C(unsigned int p)
{
    return duel_08843858(p, 0x19CE);
}

int duel_08844074(unsigned int p)
{
    return duel_08843858(p, 0x1637);
}

int duel_0884407C(unsigned int p)
{
    int z;
    unsigned int i;
    unsigned int q;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = p ^ i / 5;
        z = i % 5;
        if (!duel_slot_e8170(q, z, 0x1690)) {
            continue;
        }
        duel_0884F30C((q << 31 & 0x80000000) | 0x4200000 | (z << 16 & 0x1F0000) | (field_get_card(q, z) & 0xFFFF),
                      CARD_UID(duel.players[q & 1].field[z].card), 0);
        duel_w_08919608++;
        return 0;
    }
    return 1;
}

#ifdef NON_MATCHING /* o original relê duel_w_08919608 depois do if (g == 0) g = 5 */
int duel_088441D0(unsigned int p)
{
    int z;
    int n;

    if (duel_w_08919608 == 0) {
        duel_w_08919608 = 5;
    }
    for (; duel_w_08919608 < 10; duel_w_08919608++) {
        z = duel_w_08919608;
        if (duel_slot_e8170(p, z, 0x1491) != 0 && (n = duel_zone_f_count_monsters(1 - p)) != 0) {
            duel_088C4448(p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_088C4730(1 - p, n * 100, 1, 0x1491);
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}
#endif

int duel_08844324(unsigned int p)
{
    unsigned int i;
    unsigned int q;
    int z;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = p ^ i / 5;
        z = i % 5 + 5;
        if (!duel_slot_e8170(q, z, 0x1406)) {
            continue;
        }
        duel_088C4448(q, z, CARD_UID(duel.players[q & 1].field[z].card));
        duel_088C4730(p, 500, p != q, 0x1406);
        duel_w_08919608++;
        return 0;
    }
    return 1;
}

int duel_0884445C(unsigned int p)
{
    return duel_08843998(p, 0x13F0);
}

int duel_08844464(unsigned int p)
{
    return duel_08843998(p, 0x13F3);
}

int duel_0884446C(unsigned int p)
{
    return duel_08843998(p, 0x14B2);
}

int duel_08844474(unsigned int p)
{
    return duel_08843998(p, 0x175D);
}

int duel_0884447C(unsigned int p)
{
    return duel_08843998(0x1 - p, 0x1322);
}

int duel_0884448C(unsigned int p)
{
    return duel_08843998(0x1 - p, 0x1877);
}

int duel_0884449C(unsigned int p)
{
    return duel_08843998(0x1 - p, 0x169A);
}

int duel_088444AC(unsigned int p)
{
    return duel_08843998(0x1 - p, 0x137B);
}

int duel_088444BC(unsigned int p)
{
    return duel_08843998(0x1 - p, 0x1355);
}

int duel_088444CC(unsigned int p)
{
    return duel_08843998(0x1 - p, 0x1370);
}

int duel_088444DC(unsigned int p)
{
    return duel_08843998(0x1 - p, 0x1286);
}

int duel_088444EC(unsigned int p)
{
    return duel_08843998(0x1 - p, 0x19BD);
}

int duel_088444FC(unsigned int p)
{
    return duel_08843858(0x1 - p, 0x19D0);
}

int duel_0884450C(unsigned int p)
{
    return duel_08843858(0x1 - p, 0x1A63);
}
