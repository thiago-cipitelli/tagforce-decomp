// @range 0x088FCA90-0x088FD2DC
/* Card effect handlers (continued). */
#include "duel.h"
#include "card_db.h"

extern int duel_player_type[2];
extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08914900;
extern int duel_w_08919638;
extern int duel_w_08919640;
extern unsigned short duel_w_0891968C;
extern struct {
    unsigned char b0 : 3;
    unsigned char b3 : 1;
    unsigned char b4 : 2;
    unsigned char b6 : 2;
} duel_w_08914A2A;

int duel_phase_ok2(unsigned int p);
int duel_count_free_monster_zones(unsigned int p);
int duel_hand_count_monsters(unsigned int p);
int duel_field_atk(unsigned int p, int z);
int duel_088BFF78(unsigned int p, unsigned short id, int a);
void duel_0884ACD8(unsigned int p, int msg);
void duel_0884C588(unsigned int p, int a, unsigned short id, int b);
DuelCard *duel_0884CCBC(int i);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_088C7CF0(void);
void duel_088C7BA4(void);
void duel_088C7810(unsigned int p, int a);
void duel_08832950(unsigned int p, unsigned short id);
int duel_08827DA4(unsigned int p, int uid, int a, int b);
void duel_08819B94(unsigned int p);
void duel_08835944(int a);
void duel_0882AF5C(int a, int b, int c);
int duel_088E9150(int id);
int duel_08881160(DuelAct *a, int b);
void duel_088C7478(unsigned int p, int i, DuelCard *c, int a);
void duel_08832870(unsigned int p, int a);
void duel_088D192C(DuelAct *a, unsigned int p, int z);
void duel_08832880(unsigned int p, unsigned short id, int a);
int duel_088C4604(DuelAct *a, unsigned int p, int n);

int duel_eff_fca90(DuelAct *a)
{
    unsigned int q = a->player ^ a->opp;

    switch (duel_step) {
    case 0x80: {
        unsigned int i;

        if (a->w[0] == 0) {
            return 0;
        }
        for (i = 0; i < duel.players[q & 1].n_hand; i++) {
            if (!!duel_08827DA4(q, CARD_UID(duel.players[q & 1].hand[i]), 1, a->w[0])) {
                duel_0884ACD8(q, 0x27);
                return 0x7F;
            }
        }
        duel_088C7810(q, 1);
        return 0x78;
    }
    case 0x7F:
        duel_w_08914A2A.b4 = a->w[0];
        if (duel_player_type[q] == 1) {
            duel_08819B94(q);
        } else {
            duel_08835944(6);
        }
        return 0x7E;
    case 0x7E:
        if (duel_w_08919638 == 0) {
            return 0x80;
        }
        switch (duel_w_08919640) {
        default:
            return 0x7F;
        case 6:
        case 4:
            duel_0882AF5C(duel_w_08919640 == 4, 0, a->w[0] + 1);
            return 0x7D;
        }
    case 0x7D:
        if (duel_w_08914A2A.b3) {
            return 0x7F;
        }
        return 0;
    case 0x78:
        duel_08832950(1 - q, a->id);
        return 0x77;
    case 0x77:
        duel_088C7810(q, 0);
        return 0;
    }
    return 0;
}

int duel_eff_fccfc(DuelAct *a)
{
    switch (duel_step) {
    case 0x80: {
        int n;

        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        n = duel_088BFF78(a->player, a->id, 0);
        if (n == 0) {
            return 0;
        }
        n = MIN(n, duel_count_free_monster_zones(a->player));
        duel_0884ACD8(a->player, 0x11D);
        duel_w_089148FC = n;
        duel_w_08914900 = 0;
        return 0x7F;
    }
    case 0x7F:
        duel_0884C588(a->player, MIN(duel_w_089148FC, 32) + 6, a->id, 0);
        duel_088C7BA4();
        return 0x7E;
    case 0x7E:
        if (duel_w_08914900 < duel_w_089148FC) {
            duel_088263A8(a->player, duel_0884CCBC(duel_w_08914900 + 1), 1, a->id << 16);
            duel_w_08914900++;
            return 0x7E;
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_eff_fce88(DuelAct *a, int b)
{
    if (duel_088E9150(0x1A77) != 0) {
        return duel_08881160(a, b);
    }
    return 0;
}

int duel_eff_fced0(DuelAct *a)
{
    switch (duel_step) {
    case 0x80: {
        unsigned int q = 1 - a->player;

        if (duel.players[q & 1].n_hand == 0) {
            return 0;
        }
        duel_08832870(q, 0);
        return 0x7F;
    }
    case 0x7F: {
        unsigned int i = duel_w_0891968C;
        unsigned int q = 1 - a->player;

        duel_088C7478(q, i, &duel.players[q & 1].hand[i], 1);
        return 0;
    }
    }
    return 0;
}

int duel_eff_fcf9c(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    duel_088D192C(a, p, z);
    return 0;
}

int duel_eff_fd058(DuelAct *a)
{
    switch (duel_step) {
    case 0x80: {
        unsigned int q = 1 - a->player;

        if (duel.players[q & 1].n_hand == 0) {
            return 0;
        }
        if (duel_hand_count_monsters(q) == 0) {
            duel_088C7810(1 - a->player, 1);
            return 0x6E;
        }
        duel_08832880(a->player, a->id, 1);
        return 0x7F;
    }
    case 0x7F: {
        int z;
        unsigned int i = duel_w_0891968C;
        int atk = card_get_atk_q(duel.players[(1 - a->player) & 1].hand[i].id);

        if (atk != 0xFFFF) {
            for (z = 0; z <= 4; z++) {
                if (duel.players[a->player & 1].field[z].card.id != 0 && duel.players[a->player & 1].field[z].faceup != 0 &&
                    duel_field_atk(a->player, z) >= atk) {
                    unsigned int q;

                    duel_088C7BA4();
                    q = 1 - a->player;
                    duel_088C7478(q, i, &duel.players[q & 1].hand[i], 1);
                    duel_088C4604(a, 1 - a->player, 500);
                    duel_088C7CF0();
                    return 0;
                }
            }
        }
        duel_088C4604(a, a->player, 500);
        return 0;
    }
    case 0x6E:
        duel_0884ACD8(a->player, 0x11);
        return 100;
    case 100:
        duel_088C7810(1 - a->player, 0);
        return 0;
    }
    return 0;
}
