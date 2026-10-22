// @range 0x0884451C-0x08844C2C
/* Computer player: responses with cards in the graveyard and the spell/trap zones. */
#include "duel.h"

extern unsigned int duel_w_08919608;

int duel_08843858(unsigned int p, int id);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
unsigned int duel_slot_effect12_arg(unsigned int p, int z, int id);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_0886C448(DuelAct *a);
int field_get_card(unsigned int p, int z);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088C8AD0(unsigned int p, int z, int id, int v);

int duel_0884451C(unsigned int p, int id)
{
    unsigned int i;
    DuelCard *c;
    int cid;

    if (duel_slot_has_effect(p, 0xB, id) == 0) {
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            c = &duel.players[p & 1].grave[i];
            cid = c->id;
            if (cid == id && !c->b21
                && duel_0884F30C((p << 31 & 0x80000000) | 0x44E0000 | (cid & 0xFFFF), CARD_UID(*c), 0) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

int duel_08844640(unsigned int p)
{
    return duel_0884451C(p, 0x1181);
}

int duel_08844648(unsigned int p)
{
    return duel_0884451C(p, 0x19CB);
}

int duel_08844650(unsigned int p)
{
    unsigned int i;
    DuelCard *c;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        c = &duel.players[p & 1].grave[i];
        if (c->id == 0x1775 && !c->b21) {
            DuelAct t = {0};

            t.id = 0x1775;
            t.player = p;
            t.b3_4 = 2;
            t.zone = 0xE;
            t.kind = 2;
            t.uid = CARD_UID(*c);
            if (duel_0886C448(&t) == 0
                && duel_0884F30C((p << 31 & 0x80000000) | 0x44E0000 | c->id, CARD_UID(*c), 0) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

int duel_0884483C(unsigned int p)
{
    int z;

    for (; duel_w_08919608 <= 4; duel_w_08919608++) {
        z = duel_w_08919608;
        if (duel_slot_e8170(p, z, 0x13B2) != 0 && duel_slot_effect_kind(p, z, 0x13B2, 2) != 0) {
            duel_088C4448(p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_088C87A8(p, z, field_get_card(p, z), 3, 1);
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_08844960(unsigned int p)
{
    return duel_08843858(p, 0x154D);
}

int duel_08844968(unsigned int p)
{
    return duel_08843858(p, 0x1645);
}

int duel_08844970(unsigned int p)
{
    return duel_08843858(p, 0x160E);
}

int duel_08844978(unsigned int p)
{
    return duel_08843858(0x1 - p, 0x1A65);
}

int duel_08844988(unsigned int p)
{
    return duel_08843858(p, 0x1740);
}

int duel_08844990(unsigned int p)
{
    int z;
    int id;

    for (; duel_w_08919608 < 5; duel_w_08919608++) {
        z = duel_w_08919608;
        id = field_get_card(p, z);
        switch (id) {
        case 0x17D1:
        case 0x1822:
        case 0x185E:
        case 0x17D5:
        case 0x17D9:
        case 0x1812:
        case 0x1814:
        case 0x1817:
            if (duel.players[p & 1].field[z].faceup != 0) {
                duel_0884F30C((p << 31 & 0x80000000) | 0x4200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                              CARD_UID(duel.players[p & 1].field[z].card), 0);
                duel_w_08919608++;
                return 0;
            }
        }
    }
    return 1;
}

int duel_08844B14(unsigned int p, int z, int id)
{
    int v = duel_slot_effect12_arg(p, 0xB, id);

    if (v >= 0) {
        duel_088C8AD0(p, 0xB, id, v);
        duel_0884F30C((p << 31 & 0x80000000) | 0x4400000 | (z << 16 & 0x1F0000) | (id & 0xFFFF), v, 0);
        return 0;
    }
    return 1;
}

int duel_08844BB8(unsigned int p)
{
    return duel_08844B14(p, 0xE, 0x13C7);
}

int duel_08844BC4(unsigned int p)
{
    return duel_08844B14(p, 0xE, 0x156B);
}

int duel_08844BD0(unsigned int p)
{
    return duel_08844B14(p, 0xE, 0x1522);
}

int duel_08844BDC(unsigned int p)
{
    return duel_08844B14(p, 0xE, 0x185C);
}

int duel_08844BE8(unsigned int p)
{
    return duel_08844B14(p, 0xE, 0x188F);
}

int duel_08844BF4(unsigned int p)
{
    return duel_08844B14(0x1 - p, 0xE, 0x188F);
}

int duel_08844C08(unsigned int p)
{
    return duel_08844B14(p, 0xE, 0x17DE);
}

int duel_08844C14(unsigned int p)
{
    return duel_08844B14(p, 0xE, 0x1A64);
}

int duel_08844C20(unsigned int p)
{
    return duel_08844B14(p, 0xE, 0x1A62);
}
