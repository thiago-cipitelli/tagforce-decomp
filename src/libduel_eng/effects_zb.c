// @range 0x0888E5D4-0x0888EB8C
// @asm 0x0888E5D4-0x0888E8F8
/* Card effect handlers (DuelAct *a), after effects_za.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

void duel_088CC178();
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
unsigned int duel_0884E4E4(DuelAct *a);
int duel_0884E5E0(DuelAct *a);
void duel_wfmt_d(unsigned short *dst, int fmt, int n);
void duel_0884ACD8(unsigned int p, void *msg);
int duel_0884E690(DuelAct *a, int (*fn)(int id));
int duel_088537B0(int id);
void duel_088359E8(unsigned int p, int id, int (*fn)(int id));
int duel_08835AC0(void);
void duel_088CB914(void);
int duel_0887DE04(DuelAct *a, int b);
int duel_field_atk(unsigned int p, int z);
int duel_0886CE90(DuelAct *a, int b);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884C7CC(void);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_0887DF94(DuelAct *a, int b);
void duel_088C5424(unsigned int p, int n);
int duel_088C4604(DuelAct *a, unsigned int p, int n);

#ifdef NON_MATCHING /* ordem das constantes no laço da máscara; endereço de a->w antes do deslocamento */
int duel_0888E5D4(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;
    unsigned int mask;
    int i;
    int k;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    mask = 0;
    for (i = 0; i < a->n7; i++) {
        k = a->w[i] & 0xFF;
        if (a->id == 0x1AA9) {
            k = 1;
        }
        mask |= 1 << ((a->w[i] >> 8 & 0xFF) + k * 16);
    }
    duel_088CC178(p, z, mask);
    duel_088C77D4(a->player, a->player | a->zone << 8, 6, 0);
    return 0;
}

int duel_0888E6F4(DuelAct *a)
{
    unsigned short buf[256];

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = duel.players[0].n_grave + duel.players[1].n_grave;
        duel_088D13CC(a, duel_0884E4E4(a));
        duel_088C77D4(a->player, a->id, 1, 0);
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel.players[0].n_grave + duel.players[1].n_grave - duel_w_089148FC;
        a->w[1] = 0;
        a->w[0] = 0;
        a->w[2] = 1;
        if (duel_w_089148FC <= 0) {
            return 0;
        }
        if (duel_0884E5E0(a) == 0) {
            return 0;
        }
        duel_wfmt_d(buf, 0x119, duel_w_089148FC);
        duel_0884ACD8(a->player, buf);
        return 0x7E;
    case 0x7E:
        if (duel_0884E690(a, duel_088537B0) == 0) {
            return 0x7C;
        }
        duel_088359E8(a->player, a->id, duel_088537B0);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            a->w[duel_w_0891964C] |= 1 << (duel_w_08919650 + duel_w_08919654);
            duel_088CB914();
            return --duel_w_089148FC != 0 ? 0x7E : 0x7C;
        }
        return 0x7E;
    case 0x7C:
        duel_088D13CC(a, a->w[0] | a->w[1] << 16);
        return 0;
    }
    return 0;
}
#endif

int duel_0888E8F8(DuelAct *a, int b)
{
    int z;
    int sum = 0;

    duel_0887DE04(a, b);
    for (z = 0; z <= 4; z++) {
        if (duel.players[a->player & 1].field[z].card.id != 0 && duel.players[a->player & 1].field[z].faceup != 0 &&
            a->player != duel.players[a->player & 1].field[z].card.owner) {
            sum += duel_field_atk(a->player, z);
        }
    }
    duel_088CC178(a->player, a->zone, sum > 0xFFFF ? (sum = 0xFFFF) : sum);
    return 0;
}

int duel_0888E9E0(DuelAct *a, int b)
{
    switch (duel_step) {
    case 0x80:
        if (duel_088D13CC(a, duel_0884E4E4(a)) != 0) {
            return 0x7F;
        }
        return 100;
    case 0x7F:
        if (duel_0886CE90(a, b) == 0) {
            return 100;
        }
        duel_0884ACD8(a->player, (void *)0x5E);
        return 0x7E;
    case 0x7E:
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7D;
    case 0x7D:
        duel_088263A8(a->player, duel_0884C7CC(), 1, 0x20);
        return 100;
    case 100:
        duel_088C77D4(a->player, a->id, 1, 0);
        return 0;
    }
    return 0;
}

int duel_0888EAFC(DuelAct *a, int b)
{
    if (a->b3_4 == 0) {
        duel_0887DF94(a, b);
        duel_088C5424(a->player, 600);
        duel_088C5424(1 - a->player, 600);
    } else {
        duel_088C4604(a, 1 - (a->player ^ a->opp), 100);
    }
    return 0;
}
