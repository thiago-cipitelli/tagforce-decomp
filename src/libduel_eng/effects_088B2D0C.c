// @range 0x088B2D0C-0x088B3360
/* Candidate lists: cards of a pile offered for selection (duel_088BFC64(p, zone, i)). */
#include "duel.h"

void duel_088BFC64(unsigned int p, int z, int i);
int card_is_monster(int id);
int card_get_atk_q(int id);
unsigned int card_get_type(int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int card_same_name(int a, int b);
int card_id_remap_1a6c(int id, int *out);

void duel_088B2D0C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_is_monster(duel.players[p & 1].zone_d[i].id) &&
            card_get_atk_q(duel.players[p & 1].zone_d[i].id) < 0x5DD &&
            duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i])) == 0) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B2E04(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0x17) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B2EB0(unsigned int p)
{
    int i;

    for (i = 0; i < (unsigned int)duel.players[p & 1].n_d && i < 5; i++) {
        duel_088BFC64(p, 0xD, i);
    }
}


/* cards of the hand and of zone 0xD whose name matches the one(s) searched by card `id` (arg: a second
   card id that some of them depend on) */
void duel_088B2F34(unsigned int p, int id, int arg)
{
    int names[2] = { 0 };
    unsigned int i;
    int c;

    switch (id) {
    case 0xFB6:
        names[0] = 0x146E;
        break;
    case 0x10E4:
        names[0] = 0xFE4;
        names[1] = 0xFE5;
        break;
    case 0x1529:
        names[0] = 0x1534;
        break;
    case 0x15E0:
        names[0] = 0xFA7;
        break;
    case 0x1642:
        names[0] = 0x1643;
        break;
    case 0x165A:
        names[0] = 0x1644;
        break;
    case 0x167E:
        names[0] = 0xFC9;
        break;
    case 0x1741:
        names[0] = 0x173D;
        break;
    case 0x17AF:
        names[0] = 0x1788;
        break;
    case 0x17D1:
        names[0] = 0x1822;
        break;
    case 0x1822:
        names[0] = 0x185E;
        break;
    case 0x185E:
        names[0] = 0x18AF;
        break;
    case 0x17C5:
        names[0] = 0x17C5;
        break;
    case 0x17C9:
        names[0] = 0x17C9;
        break;
    case 0x17D2:
        names[0] = 0x17D3;
        break;
    case 0x17D3:
        names[0] = 0x17D4;
        break;
    case 0x17D5:
        names[0] = 0x17D6;
        break;
    case 0x17D7:
        names[0] = 0x17D8;
        break;
    case 0x17D8:
        names[0] = 0x1823;
        break;
    case 0x17D9:
        names[0] = 0x17DA;
        break;
    case 0x17DA:
        names[0] = 0x17DB;
        break;
    case 0x17F5:
        card_id_remap_1a6c(arg, names);
        break;
    case 0x1812:
        names[0] = 0x1814;
        break;
    case 0x1814:
        names[0] = 0x1816;
        break;
    case 0x1817:
        names[0] = 0x181A;
        break;
    case 0x185D:
        names[0] = 0x185C;
        break;
    case 0x187E:
        names[0] = 0x186B;
        break;
    case 0x18A3:
        names[0] = 0x18A4;
        break;
    case 0x1907:
        names[0] = 0x1906;
        break;
    case 0x1918:
        names[0] = 0x1918;
        break;
    case 0x19B5:
        names[0] = 0x19A8;
        break;
    case 0x19D8:
        switch (arg) {
        case 0x1757:
            names[0] = 0x19CD;
            break;
        case 0x191D:
            names[0] = 0x19CE;
            break;
        }
        break;
    case 0x1A0C:
        names[0] = 0x1A05;
        break;
    case 0x1A1A:
        names[0] = 0x19F6;
        break;
    case 0x1A6C:
        names[0] = 0x1A70;
        break;
    case 0x1A76:
        names[0] = 0x1A70;
        break;
    }
    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        c = duel.players[p & 1].hand[i].id;
        if (card_same_name(names[0], c) || card_same_name(names[1], c)) {
            duel_088BFC64(p, 0xB, i);
        }
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        c = duel.players[p & 1].zone_d[i].id;
        if (card_same_name(names[0], c) || card_same_name(names[1], c)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}
