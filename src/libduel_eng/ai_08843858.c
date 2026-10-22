// @range 0x08843858-0x08844024
// @asm 0x08843AE0-0x08843C74
/* Computer player: responses from the spell/trap zones and the graveyard. */
#include "duel.h"

extern unsigned int duel_w_08919608;

int duel_slot_e8170(unsigned int p, int z, int id);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int field_get_card(unsigned int p, int z);
void duel_088CC1D0(unsigned int p, int z, int a);
int duel_slot_effect_arg(unsigned int p, int z, int id);
unsigned int duel_slot_effect12_arg(unsigned int p, int z, int id);
void duel_088C89BC(unsigned int p, int z, int id, int c);
void duel_088C8AD0(unsigned int p, int z, int id, int v);
int duel_count_monsters(unsigned int p);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
int duel_088C9878(unsigned int p, int z, int x, int y, int id, int opp);
void duel_088C7E68(unsigned int p, int z, int a, int b);

int duel_08843858(unsigned int p, int id)
{
    int z;

    for (; duel_w_08919608 < 5; duel_w_08919608++) {
        z = duel_w_08919608;
        if (duel_slot_e8170(p, z, id) != 0
            && duel_0884F30C((p << 31 & 0x80000000) | 0x4200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                             CARD_UID(duel.players[p & 1].field[z].card), 0) != 0) {
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_08843998(unsigned int p, int id)
{
    int z;

    for (; duel_w_08919608 <= 5; duel_w_08919608++) {
        z = duel_w_08919608 + 5;
        if (duel_slot_e8170(p, z, id) != 0
            && duel_0884F30C((p << 31 & 0x80000000) | 0x4200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                             CARD_UID(duel.players[p & 1].field[z].card), 0) != 0) {
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_08843AD8(unsigned int p)
{
    return duel_08843998(p, 0x13FF);
}

#ifdef NON_MATCHING /* registradores: o original não mantém pl->field[z] num registrador salvo */
int duel_08843AE0(unsigned int p)
{
    int z;
    DuelPlayer *pl;

    for (; duel_w_08919608 <= 5; duel_w_08919608++) {
        z = duel_w_08919608 + 5;
        pl = &duel.players[p & 1];
        if (field_get_card(p, z) == 0x1A7E && pl->field[z].faceup != 0) {
            duel_088CC1D0(p, z, 1);
            if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
                duel_0884F30C((p << 31 & 0x80000000) | 0x4200000 | (z << 16 & 0x1F0000) | 0x1A7E,
                              CARD_UID(pl->field[z].card), 0);
            }
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}
#endif

int duel_08843C74(unsigned int p)
{
    return duel_08843998(p, 0x1494);
}

int duel_08843C7C(unsigned int p)
{
    return duel_08843998(p, 0x1519);
}

int duel_08843C84(unsigned int p)
{
    return duel_08843998(p, 0x1545);
}

int duel_08843C8C(unsigned int p)
{
    return duel_08843998(p, 0x1738);
}

int duel_08843C94(unsigned int p)
{
    int v = duel_slot_effect_arg(p, 0xB, 0x140C);

    if (v >= 0) {
        duel_088C89BC(p, 0xB, 0x140C, 0);
        duel_0884F30C((p << 31 & 0x80000000) | 0x450140C, 0, v);
        return 0;
    }
    return 1;
}

int duel_08843D10(unsigned int p)
{
    int v = duel_slot_effect12_arg(p, 0xB, 0x1A02);

    if (v >= 0) {
        duel_088C8AD0(p, 0xB, 0x1A02, v);
        duel_0884F30C((p << 31 & 0x80000000) | 0x4501A02, 0, v);
        return 0;
    }
    return 1;
}

int duel_08843D8C(unsigned int p)
{
    return duel_08843858(p, 0x11CF);
}

int duel_08843D94(unsigned int p)
{
    return duel_08843858(p, 0x1578);
}

int duel_08843D9C(unsigned int p)
{
    int z;

    for (; duel_w_08919608 <= 4; duel_w_08919608++) {
        z = duel_w_08919608;
        if (duel_slot_e8170(p, z, 0x1338) != 0 && duel_count_monsters(p) == 1) {
            duel_088C4448(p, z, CARD_UID(duel.players[p & 1].field[z].card));
            if (duel.players[p & 1].field[z].pos == 0) {
                duel_088C9878(p, z, 0, 0, 0x1338, 0);
            }
            duel_088C7E68(p, z, 0x15, 1);
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_08843EF0(unsigned int p)
{
    return duel_08843858(p, 0x1450);
}

int duel_08843EF8(unsigned int p)
{
    return duel_08843858(p, 0x1451);
}

int duel_08843F00(unsigned int p)
{
    return duel_08843858(p, 0x1454);
}

int duel_08843F08(unsigned int p)
{
    unsigned int i;
    DuelCard *c;
    int id;

    if (duel_w_08919608 != 0) {
        return 1;
    }
    {
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            c = &duel.players[p & 1].grave[i];
            id = c->id;
            if (id == 0x1459 && !c->b21) {
                duel_0884F30C((p << 31 & 0x80000000) | 0x44E0000 | (id & 0xFFFF), CARD_UID(*c), 0);
            }
        }
        duel_w_08919608++;
        return 0;
    }
}
