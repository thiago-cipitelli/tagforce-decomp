// @range 0x08890094-0x08890820
// @asm 0x088905D0-0x08890820
/* Card effect handlers (DuelAct *a), after effects_zc.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2();
int duel_088BFF78(unsigned int p, int id, int b);
void duel_0884ACD8(unsigned int p, void *msg);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884C7CC(void);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_088C7810(unsigned int p, unsigned int on);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);
int duel_0887F80C(DuelAct *a, DuelAct *b);
int duel_0884E690(DuelAct *a, int (*fn)(int id));
int duel_088587EC(int id);
void duel_0884AD18(int msg);
void duel_08835A54(unsigned int p, unsigned short id, int (*fn)(int id));
int duel_08835AC0(void);
void duel_088C7BA4(void);
void duel_088D147C(DuelAct *a, unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C7CF0(void);
int duel_first_free_zone(unsigned int p);
void duel_088C6728(unsigned int p, int uid, int a);
void duel_088AFD5C(int kind, unsigned int a);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_field_def(unsigned int p, int z);
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);

int duel_08890094(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            duel_0884ACD8(a->player, (void *)0xD);
            return 0x6E;
        }
        duel_0884ACD8(a->player, (void *)0x11D);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7E;
    case 0x7E:
        duel_088263A8(a->player, duel_0884C7CC(), 1, 0);
        return 100;
    case 0x6E:
        duel_088C7810(a->player, 1);
        return 0x6D;
    case 0x6D:
        duel_088328FC(1 - a->player);
        return 0x6C;
    case 0x6C:
        duel_088C7810(a->player, 0);
        return 100;
    case 100:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_0889021C(DuelAct *a, DuelAct *b)
{
    unsigned int q;
    int z;

    switch (duel_step) {
    case 0x80:
        duel_0887F80C(a, b);
        return 0x7F;
    case 0x7F:
        if (duel_0884E690(a, duel_088587EC) == 0) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_0884AD18(0x68);
        }
        return 0x7E;
    case 0x7E:
        if (duel_w_08919624 == 0) {
            return 0;
        }
        duel_08835A54(a->player, a->id, duel_088587EC);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            q = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_088C7BA4();
            duel_088D147C(a, q, z);
            duel_088C4604(a, 1 - a->player, duel_field_atk(q, z));
            duel_088C7CF0();
            return 0;
        }
        return 0x7F;
    }
    return 0;
}

int duel_08890394(DuelAct *a)
{
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel_first_free_zone(a->player) < 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    duel_088263A8(a->player, &duel.players[a->player & 1].field[a->zone].card, 1, 1);
    return 0;
}

int duel_088904A8(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            duel_0884ACD8(a->player, (void *)0xD);
            return 0x7E;
        }
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7F;
    case 0x7F:
        duel_088C6728(a->player, CARD_UID(*duel_0884C7CC()), 0);
        return 0x7D;
    case 0x7E:
        duel_088328FC(1 - a->player);
        return 0x7D;
    case 0x7D:
        duel_088C78E0(a->player);
        duel_088AFD5C(0x24, 0);
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* alocação dos $s no laço; ramo do id 0x1835 */
int duel_088905D0(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;
    int atk;
    unsigned int mask;
    int i;
    int b;

    if (a->kind != 0x23) {
        duel_088C77D4(a->player, a->id, 1, 0);
    }
    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            atk = duel_field_atk(p, z);
            p = 1 - p;
            mask = 0;
            for (i = 0, b = p * 16; i < 5; i++, b++) {
                if (duel.players[p & 1].field[i].card.id != 0 && duel.players[p & 1].field[i].faceup != 0 &&
                    duel_field_def(p, i) <= atk) {
                    mask |= 1 << b;
                }
            }
            duel_088D13CC(a, mask);
        }
    }
    return 0;
}

int duel_08890730(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        duel_088C87A8(p, z, a->id, 3, a->id == 0x1835 ? a->w[2] * 1000 : 0);
    }
    return 0;
}
#endif
