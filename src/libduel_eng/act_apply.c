// @range 0x088D13CC-0x088D1DF4
// @asm 0x088D16C4-0x088D184C
/* Applying an effect to a set of zones: duel_act_apply (0x088D1094, kept in asm) takes the card
   acting, a mask of zones (bit player * 16 + zone), the kind of effect and flags, and returns the
   zones it affected; the wrappers below test one zone or build the acting card. */
#include "duel.h"

unsigned int duel_act_apply();
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
int duel_act_can_affect();
int card_attribute_icon_is3(int id);

unsigned int duel_088D13CC(DuelAct *a, unsigned int mask)
{
    return duel_act_apply(a, mask, 0xE, 2);
}

unsigned int duel_088D13D8(unsigned int p, unsigned short id, unsigned int mask)
{
    DuelAct t = {0};

    t.id = id;
    t.player = p;
    return duel_act_apply(&t, mask, 0xE, 2);
}

int duel_088D1438(unsigned int p, int z, unsigned int q, unsigned short id)
{
    unsigned int m = 1 << (z + p * 16);

    return (m & duel_088D13D8(p ^ q, id, m)) != 0;
}

int duel_088D147C(DuelAct *a, unsigned int p, int z)
{
    unsigned int m = 1 << (z + p * 16);

    return (m & duel_088D13CC(a, m)) != 0;
}

int duel_088D14B8(DuelAct *a, unsigned int p, int z)
{
    unsigned int m;

    if (duel_act_can_affect(a, p, z, 1) != 0) {
        m = 1 << (z + p * 16);
        return (m & duel_088D13CC(a, m)) != 0;
    }
    return 0;
}

int duel_088D1524(DuelAct *a, unsigned int p, int z)
{
    unsigned int m = 1 << (z + p * 16);
    int kind = card_attribute_icon_is3(duel.players[p & 1].field[z].card.id) != 0 ? 0xC : 0xD;

    return duel_act_apply(a, m, kind, 2) != 0;
}

int duel_088D15B4(DuelAct *a, unsigned int p, int z)
{
    unsigned int m = 1 << (z + p * 16);

    if (z > 10) {
        return 0;
    }
    return (m & duel_act_apply(a, m, 0xE, 10)) != 0;
}

int duel_088D1608(unsigned int p, int z, int flags)
{
    unsigned int m = 1 << (z + p * 16);
    DuelAct t = {0};

    t.player = p;
    return (m & duel_act_apply(&t, m, 0xF, flags)) != 0;
}

int duel_088D1684(DuelAct *a, unsigned int p, int z, int flags)
{
    unsigned int m = 1 << (z + p * 16);

    return (m & duel_act_apply(a, m, 0xF, flags)) != 0;
}

#ifdef NON_MATCHING /* loop registers differ */
int duel_088D16C4(DuelAct *a, unsigned int p, int z)
{
    unsigned int m = 1 << (z + p * 16);
    unsigned int e;

    if ((m & duel_act_apply(a, m, 0xF)) == 0) {
        return 0;
    }
    e = duel.players[p & 1].field[z].effects;
    if (e == 0) {
        return 1;
    }
    do {
        DuelEffect *ef = &duel_effects[e];

        e = ef->next;
        if ((ef->kind & 0xF) < 6) {
            switch (ef->src) {
            case 0x17D2:
            case 0x17D5:
            case 0x1814:
            case 0x1A1F:
            case 0x1A20:
            case 0x1A21:
                duel_088C77D4(p, ef->src, (ef->kind & 0xF) | (ef->kind & 0xF0),
                              CARD_UID(duel.players[p & 1].field[z].card));
                break;
            }
        }
    } while (e != 0);
    return 1;
}
#endif

void duel_088D184C(DuelAct *a, unsigned int mask, int flags)
{
    duel_act_apply(a, mask, 0xF, flags);
}

int duel_088D1858(DuelAct *a, unsigned int p, int z)
{
    unsigned int m = 1 << (z + p * 16);

    return (m & duel_act_apply(a, m, 0xB, 0)) != 0;
}

int duel_088D189C(unsigned int p, int z, unsigned int q, unsigned short id)
{
    unsigned int m = 1 << (z + p * 16);
    DuelAct t = {0};

    t.player = p ^ q;
    t.id = id;
    return (m & duel_act_apply(&t, m, 0xB, 0)) != 0;
}

void duel_088D1920(DuelAct *a, unsigned int mask)
{
    duel_act_apply(a, mask, 0xB, 0);
}

int duel_088D192C(DuelAct *a, unsigned int p, int z)
{
    unsigned int m = 1 << (z + p * 16);

    return (m & duel_act_apply(a, m, 0xD, 0)) != 0;
}

int duel_088D1970(DuelAct *a, unsigned int p, int z)
{
    unsigned int m = 1 << (z + p * 16);

    return (m & duel_act_apply(a, m, 0xD, 0x800000)) != 0;
}

void duel_088D19B4(DuelAct *a, unsigned int mask)
{
    duel_act_apply(a, mask, 0xD, 0);
}

int duel_088D19C0(unsigned int p, int z, unsigned int q, unsigned short id)
{
    unsigned int m = 1 << (z + p * 16);
    DuelAct t = {0};

    t.player = p ^ q;
    t.id = id;
    return (m & duel_act_apply(&t, m, 0xD, 0)) != 0;
}

int duel_088D1A44(DuelAct *a, unsigned int p, int z)
{
    unsigned int m = 1 << (z + p * 16);
    unsigned int o = duel.players[p & 1].field[z].card.owner;

    return (m & duel_act_apply(a, m, 0xD, o == p ? 0x40000 : 0)) != 0;
}

int duel_088D1B78(DuelAct *a, unsigned int p, int z);

void duel_088D1AC8(unsigned int p, int z)
{
    DuelAct t = {0};

    t.player = p;
    duel_088D1B78(&t, p, z);
}

void duel_088D1B20(unsigned int p, unsigned int mask)
{
    DuelAct t = {0};

    t.player = p;
    duel_act_apply(&t, mask, 0xE, 0x1000);
}

int duel_088D1B78(DuelAct *a, unsigned int p, int z)
{
    unsigned int m = 1 << (z + p * 16);

    return (m & duel_act_apply(a, m, 0xE, 0x1000)) != 0;
}

unsigned int duel_088D1BBC(DuelAct *a, unsigned int mask)
{
    return mask & duel_act_apply(a, mask, 0xE, 0x1000);
}

int duel_088D1BEC(unsigned int p, int z, unsigned int q, unsigned short id)
{
    unsigned int m = 1 << (z + p * 16);
    DuelAct t = {0};

    t.player = p ^ q;
    t.id = id;
    return (m & duel_act_apply(&t, m, 0xE, 0)) != 0;
}

int duel_088D1C70(DuelAct *a, unsigned int p, int z)
{
    unsigned int m = 1 << (z + p * 16);

    return (m & duel_act_apply(a, m, 0xE, 0)) != 0;
}

int duel_088D1CB4(DuelAct *a, unsigned int p, int z, int flags)
{
    unsigned int m = 1 << (z + p * 16);

    return (m & duel_act_apply(a, m, 0xE, flags)) != 0;
}

int duel_088D1CF4(DuelAct *a, unsigned int p, int z)
{
    unsigned int m = 1 << (z + p * 16);
    unsigned short id = a->id;
    unsigned int r;

    a->id = 0;
    r = duel_act_apply(a, m, 0xE, 0);
    a->id = id;
    return (m & r) != 0;
}

void duel_088D1D58(DuelAct *a, unsigned int mask)
{
    duel_act_apply(a, mask, 0xE, 0);
}

void duel_088D1D64(DuelAct *a, unsigned int mask)
{
    unsigned short id = a->id;

    a->id = 0;
    duel_act_apply(a, mask, 0xE, 0);
    a->id = id;
}

int duel_088D1DA4(DuelAct *a, unsigned int p, int z, unsigned int q, int kind)
{
    unsigned int m = 1 << (z + p * 16);

    return (m & duel_act_apply(a, m, kind, p != q ? 0x40000 : 0)) != 0;
}
