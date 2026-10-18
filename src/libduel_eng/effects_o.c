// @range 0x088FDE68-0x088FE160
// @asm 0x088FDE68-0x088FE09C
/* Card effect handlers (continued). */
#include "duel.h"

extern int duel_player_type[2];
extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08919624;
extern int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

unsigned int duel_act_target(DuelAct *a, int i);
int duel_088A9008(DuelAct *a, int b, int c);
int duel_0884E440();
int duel_field_level(unsigned int p, int z);
int duel_088537B0(int id);
int duel_0884E690(DuelAct *a, int (*fn)(int id));
void duel_0884ACD8(unsigned int p, int msg);
void duel_0884AD18(int msg);
void duel_088359E8(unsigned int p, int id, int (*fn)(int id));
int duel_08835AC0(void);
void duel_088CB914(void);
void duel_088D1920(DuelAct *a, unsigned int mask);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);

#ifdef NON_MATCHING /* 10 words: the original loads duel_w_0891964C before the shift operands in the 0x7D step */
int duel_eff_fde68(DuelAct *a)
{
    int r = 0x7C;
    unsigned int t;
    unsigned int q;
    unsigned int z;

    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) != 0 &&
            (t = duel_act_target(a, 0), q = t & 0xFF, z = (unsigned char)((t & 0xFFFF) >> 8), duel_0884E440(a, q, z) != 0)) {
            a->w[1] = 0;
            a->w[0] = 0;
            a->w[2] = duel_field_level(q, z);
            duel_w_089148FC = 0;
            return 0x7F;
        }
        return 0;
    case 0x7F:
        if (duel_0884E690(a, duel_088537B0) == 0) {
            return 0x7C;
        }
        if (duel_w_089148FC == 0) {
            duel_0884ACD8(a->player, 0x61);
            duel_w_08919624 = 1;
        } else if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_0884AD18(0x75);
        }
        return 0x7E;
    case 0x7E:
        if (duel_w_08919624 == 0) {
            return r;
        }
        duel_088359E8(a->player, a->id, duel_088537B0);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            a->w[duel_w_0891964C] |= 1 << (duel_w_08919650 + duel_w_08919654);
            duel_088CB914();
            if (--duel_w_089148FC != 0) {
                r = 0x7F;
            } else {
                r = 0x7C;
            }
            return r;
        }
        return 0x7F;
    case 0x7C:
        duel_088D1920(a, a->w[0] | a->w[1] << 16);
        return 0;
    default:
        return 0;
    }
    return r;
}
#endif

int duel_eff_fe09c(DuelAct *a)
{
    if (duel.mode & 1) {
        return 0;
    }
    if ((duel.mode >> 1 & 1) == 0) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        unsigned int t = duel_act_target(a, 0);
        unsigned int q = t & 0xFF;
        unsigned int z = (unsigned char)((t & 0xFFFF) >> 8);

        if (duel.players[q & 1].field[z].faceup != 0) {
            duel_088C87D8(a, q, z, a->id, 4, 0);
        }
    }
    return 0;
}
