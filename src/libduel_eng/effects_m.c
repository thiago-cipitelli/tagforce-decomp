// @range 0x088FD2DC-0x088FD690
/* Card effect handlers (continued). */
#include "duel.h"
#include "card_db.h"

extern int duel_step;

int field_get_card(unsigned int p, int z);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_088A9008(DuelAct *a, int b, int c);
int duel_0884E440();
void duel_088C89BC(unsigned int p, int z, int a, int b);
void duel_088CC178(unsigned int p, int z, unsigned short a);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_phase_ok2(unsigned int p);
int duel_0884E4A0(DuelAct *a);
int duel_088D1D58(DuelAct *a, int b);
int duel_08881160(DuelAct *a, int b);
int duel_088D16C4(DuelAct *a, unsigned int p, int z, int f);
int duel_field_controller(unsigned int p, int z, int a);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);

int duel_eff_fd2dc(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        unsigned int t = duel_act_target(a, 0);
        unsigned int q = t & 0xFF;
        unsigned int qz = (unsigned char)((t & 0xFFFF) >> 8);

        if (duel_0884E440(a, q, qz) != 0 && card_is_attribute9(duel.players[q & 1].field[qz].card.id) == 0 &&
            a->id != field_get_card(q, qz)) {
            int id;

            duel_088C89BC(a->player, a->zone, 0xFB6, 1);
            duel_088CC178(a->player, a->zone, 0);
            id = duel.players[q & 1].field[qz].card.id;
            if (id == a->id) {
                id = field_get_card(q, qz);
            }
            duel_088C87A8(a->player, a->zone, a->id, 2, id);
        }
    }
    return 0;
}

int duel_eff_fd4c8(DuelAct *a, int b)
{
    int r;

    switch (duel_step) {
    case 0x80:
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_088D1D58(a, duel_0884E4A0(a)) != 0) {
            return 0x7F;
        }
        return 0;
    default:
        duel_step++;
        r = duel_08881160(a, b);
        if (r != 0) {
            return r - 1;
        }
        return 0;
    }
}

int duel_eff_fd560(DuelAct *a)
{
    if (a->b3_4 == 1) {
        unsigned char z = a->zone;
        unsigned int p = a->player;

        if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        if (duel.players[p & 1].field[z].faceup == 0) {
            return 0;
        }
        if (duel_088D16C4(a, p, z, 0x2000) != 0) {
            int c = duel_field_controller(a->player, a->zone, 1);

            duel_088C77D4(a->player, a->id, 0x21, a->uid * 8 + c * 4 + 1);
        }
    }
    return 0;
}
