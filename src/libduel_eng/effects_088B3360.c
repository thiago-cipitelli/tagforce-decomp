// @range 0x088B3360-0x088B3F18
// @asm 0x088B3A10-0x088B3AEC
/* Candidate lists: cards of a pile offered for selection (duel_088BFC64(p, zone, i)), after effects_088B2D0C.c. */
#include "duel.h"

void duel_088BFC64(unsigned int p, int z, int i);
int card_is_monster(int id);
int card_get_def_q(int id);
unsigned int card_get_type(int id);
int card_same_name(int id, int name);
int card_is_in_group_1a58(int id);
int card_not_in_group_1a6f(int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_card_dfac8(unsigned int p, int id, int field);
int duel_088A6B5C(unsigned int p, int id);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);

void duel_088B3360(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_same_name(duel.players[p & 1].grave[i].id, 0x12E5)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B340C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        int id = duel.players[p & 1].zone_c[i].id;

        if (card_is_in_group_1a58(id) && card_not_in_group_1a6f(id)) {
            duel_088BFC64(p, 0xC, i);
        }
    }
}

void duel_088B34D0(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        duel_088BFC64(p, 0xC, i);
    }
}

void duel_088B354C(unsigned int p, int id)
{
    unsigned int i;
    int name;

    switch (id) {
    case 0x1308:
        name = 0x12E5;
        break;
    case 0x1293:
        name = 0x1291;
        break;
    case 0x139D:
        name = 0xFE4;
        break;
    case 0x1807:
        name = 0x180B;
        break;
    case 0x180B:
        name = 0x180C;
        break;
    case 0x180C:
        name = 0x1807;
        break;
    case 0x189F:
        name = 0x18A3;
        break;
    default:
        name = id;
        break;
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_same_name(duel.players[p & 1].zone_d[i].id, name)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B36A0(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0x16) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B374C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_is_monster(duel.players[p & 1].zone_d[i].id) &&
            card_get_def_q(duel.players[p & 1].zone_d[i].id) < 0x5DD &&
            duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i])) == 0) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B3844(unsigned int p, int id)
{
    int names[2];
    unsigned int i;
    int k;

    switch (id) {
    case 0x1294:
        names[0] = 0x1291;
        names[1] = 0x1293;
        break;
    case 0x1631:
        names[0] = 0xFC9;
        names[1] = 0x1377;
        break;
    }
    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        for (k = 0; k < 2; k++) {
            if (card_same_name(duel.players[p & 1].grave[i].id, names[k])) {
                duel_088BFC64(p, 0xE, i);
                break;
            }
        }
    }
}

void duel_088B3960(unsigned int p, int id, int i)
{
    int c = duel.players[p & 1].hand[i].id;

    if (card_get_type(c) == card_get_type(id) && duel_card_dfac8(p, c, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

#ifdef NON_MATCHING /* agendamento: o original calcula &zone_d[i] no delay slot do bne */
void duel_088B3A10(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (duel.players[p & 1].zone_d[i].id != 0x12A1) {
            continue;
        }
        if (duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i])) != 0) {
            continue;
        }
        duel_088BFC64(p, 0xD, i);
    }
}
#endif

void duel_088B3AEC(unsigned int p, int id)
{
    unsigned int i;
    int name;

    switch (id) {
    case 0x1232:
        name = 0x1117;
        break;
    case 0x12DE:
        name = 0xFC9;
        break;
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_same_name(duel.players[p & 1].zone_d[i].id, name)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B3BC0(unsigned int p, int id, int i)
{
    int c = duel.players[p & 1].hand[i].id;

    if (card_get_type(c) == 1 && duel_card_dfac8(p, c, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B3C5C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        if (duel_088A6B5C(p, duel.players[p & 1].zone_c[i].id)) {
            duel_088BFC64(p, 0xC, i);
        }
    }
}

void duel_088B3D08(unsigned int p)
{
    unsigned int i;
    int k;
    unsigned int q;

    for (k = 0; k < 2; k++) {
        q = p ^ k;
        for (i = 0; i < duel.players[q & 1].n_grave; i++) {
            if (card_is_monster(duel.players[q & 1].grave[i].id) && duel_grave_card_usable(p, q, i)) {
                duel_088BFC64(q, 0xE, i);
            }
        }
    }
}

void duel_088B3DE4(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[(1 - p) & 1].n_grave; i++) {
        if (card_is_monster(duel.players[(1 - p) & 1].grave[i].id)) {
            duel_088BFC64(1 - p, 0xE, i);
        }
    }
}

void duel_088B3E90(unsigned int p)
{
    int i;

    for (i = 0; i < (unsigned int)duel.players[(1 - p) & 1].n_d && i < 5; i++) {
        duel_088BFC64(1 - p, 0xD, i);
    }
}
