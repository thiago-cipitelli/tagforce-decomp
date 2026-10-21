// @range 0x088F9668-0x088F9C00
// @asm 0x088F98D8-0x088F9AFC
/* Card effect handlers (continued). */
#include "duel.h"

extern int duel_player_type[2];
extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08919624;

int duel_rand(int n);
int duel_count_field_all(unsigned int p);
int duel_grave_count_id(unsigned int p, unsigned int id);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C77D4(unsigned int p, int id, int a, int b);
void duel_088CC4B4(unsigned int p, int z, int a);
void duel_08832AF4(unsigned int p, int z, int a, int b);
void duel_0884AF88(int msg);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_08885F68(DuelAct *a, int b);
int duel_08894A84(DuelAct *a, int b);
void duel_08832960(unsigned int p, unsigned short id);
void duel_088CC178(unsigned int p, int z, unsigned short a);
void duel_08832988(unsigned int p, unsigned short id, unsigned short n, int a);
int duel_088998D8(DuelAct *a, int b);
int duel_0888229C(DuelAct *a, int b);

int duel_eff_f9668(DuelAct *a)
{
    if (duel_088C4604(a, 1 - a->player, 500)) {
        duel_088C77D4(0, a->id, 1, 0);
    }
    return 0;
}

int duel_eff_f96c0(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_player_type[p] == 1) {
            duel_w_08919624 = duel_rand(2);
        } else {
            duel_0884AF88(0x38);
        }
        return 0x7F;
    case 0x7F:
        duel_08832AF4(p, z, 1, duel_w_08919624);
        return 0x7E;
    case 0x7E:
        if (duel.unk3832[0] == 0) {
            duel_088CC4B4(p, z, -1);
        }
        return 0;
    }
    return 0;
}

int duel_eff_f980c(DuelAct *a, int b)
{
    if (a->kind == 6) {
        unsigned char z = a->zone;
        unsigned int p = a->player;

        if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        duel_088C87A8(p, z, a->id, 5, 1 - p);
        return 0;
    }
    return duel_08885F68(a, b);
}

#ifdef NON_MATCHING /* same steps; register allocation of the 0x7E loop and the count argument differ */
int duel_eff_f98d8(DuelAct *a, int b)
{
    if (a->kind == 20) {
        return duel_08894A84(a, b);
    }
    switch (duel_step) {
    case 0x80: {
        unsigned char z = a->zone;
        unsigned int p = a->player;

        if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        if (duel.players[p & 1].field[z].faceup == 0) {
            return 0;
        }
        a->w[0] = duel_grave_count_id(p, a->id);
        if (a->w[0] == 0) {
            return 0;
        }
        duel_08832988(a->player, a->id, a->w[0], 0xFF);
        return 0x7F;
    }
    case 0x7F:
        duel_088CC178(a->player, a->zone, duel.unk3830);
        duel_w_089148FC = 1;
        return 0x7E;
    case 0x7E:
        while (duel_w_089148FC < 6) {
            unsigned int p = a->player;

            if (1 << duel_w_089148FC & duel.players[p & 1].field[a->zone].link) {
                duel_08832960(1 - p, a->id);
                duel_w_089148FC++;
                return 0x7E;
            }
            duel_w_089148FC++;
        }
        return 0;
    }
    return 0;
}
#endif

int duel_eff_f9afc(DuelAct *a, int b)
{
    if (duel_count_field_all(a->player) != 0) {
        return 0;
    }
    return duel_088998D8(a, b);
}

int duel_eff_f9b4c(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return duel_0888229C(a, b);
    }
    return 0;
}
