// @range 0x088FA920-0x088FAEFC
/* Card effect handlers (continued). */
#include "duel.h"

extern int duel_player_type[2];
extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08919624;
extern unsigned int duel_turn_player;

int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2(unsigned int p);
void duel_088C7CF0(void);
void duel_088C7BA4(void);
void duel_088C78E0(unsigned int p);
void duel_088328FC(unsigned int p);
DuelCard *duel_0884C7CC(void);
void duel_088CB554(unsigned int p, DuelCard *c);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_0886CE38();
int duel_card_dfac8(unsigned int p, int id, int a);
void duel_0884AD40(int msg, int uid);
int duel_088AA224(DuelAct *a);
void duel_0884C588(unsigned int p, int a, unsigned short id, unsigned int b);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_0884ACD8(unsigned int p, int msg);
void duel_088CB4BC(unsigned int p, int uid);
int duel_0887F728(DuelAct *a, int b);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, unsigned int d);
void duel_0884AF68(int msg, int uid, int a);
int duel_0887DB24(void);

int duel_eff_fa920(DuelAct *a, int b)
{
    unsigned int p = a->player ^ a->opp;

    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(p, a->id, duel_088AA224(a)) == 0) {
            duel_0884ACD8(p, 0x15);
            return 0x78;
        }
        duel_0884ACD8(p, 0x5E);
        return 0x7E;
    case 0x7E:
        duel_0884C588(p, 6, a->id, duel_088AA224(a));
        return 0x7D;
    case 0x7D:
        if (duel_0886CE38(a, b) && duel_card_dfac8(p, duel_0884C7CC()->id, 1)) {
            if (duel_player_type[p] == 1) {
                duel_w_08919624 = duel_0884C7CC()->id == 0x18AA;
            } else {
                duel_0884AD40(0x35, CARD_UID(*duel_0884C7CC()));
            }
            return 0x7C;
        }
        duel_w_08919624 = 0;
        return 0x7C;
    case 0x7C:
        if (duel_w_08919624 != 0) {
            duel_088263A8(p, duel_0884C7CC(), 1, a->id << 16);
        } else {
            duel_088CB554(p, duel_0884C7CC());
        }
        return 100;
    case 0x78:
        duel_088328FC(1 - p);
        return 100;
    case 100:
        duel_088C78E0(p);
        return 0;
    }
    return 0;
}

int duel_eff_fab50(unsigned int p)
{
    p = duel_turn_player;
    p ^= duel_w_089148FC;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        if (duel.players[p & 1].n_f == 0) {
            return 0x7D;
        }
    case 0x7E:
        if (duel.players[p & 1].n_f != 0) {
            duel_088CB4BC(p, CARD_UID(duel.players[p & 1].zone_f[duel.players[p & 1].n_f - 1]));
            return 0x7E;
        }
        duel_088C78E0(p);
        return 0x7D;
    case 0x7D:
        if (++duel_w_089148FC < 2) {
            return 0x7F;
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_eff_fac94(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return duel_0887F728(a, b);
    }
    return 0;
}

int duel_eff_fad48(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_0887DB24() != 0) {
            return 0x7F;
        }
        return 0;
    case 0x7F:
        if (duel_count_free_monster_zones(1 - a->player) == 0) {
            return 0;
        }
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, 0xF0);
        return 0x7E;
    case 0x7E:
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7D;
    case 0x7D:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 0;
        } else {
            duel_0884AF68(0xF7, CARD_UID(*duel_0884C7CC()), 1);
        }
        return 0x7C;
    case 0x7C:
        duel_0882619C(1 - a->player, duel_0884C7CC(), 1, duel_w_08919624, a->id << 16 | 0x20);
        return 0;
    }
    return 0;
}
