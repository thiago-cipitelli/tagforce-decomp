// @range 0x088FBAC8-0x088FC114
// @asm 0x088FBAC8-0x088FBCD4
/* Card effect handlers (continued). */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;

int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2(unsigned int p);
int duel_pick_monster_zone(void);
int duel_hand_find_name(unsigned int p, int id);
int duel_0886CE38();
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088CC178();
void duel_088C8874(unsigned int uid, unsigned short id, int a, unsigned short b);
void duel_088D1BEC(unsigned int p, int z, int a, int b);
int duel_0887DB24(DuelAct *a, int b);
int duel_0887DDB0(DuelAct *a, int b);
int duel_0887F8E4(DuelAct *a, int b);
int duel_08890730(DuelAct *a, int b);
int duel_eff_fbed8(DuelAct *a, int b);

#ifdef NON_MATCHING
/* 5 words: the original computes the zone before the player in case 0x80 */
int duel_eff_fbac8(DuelAct *a, int b)
{
    switch (a->w[2]) {
    case 1:
        return duel_08890730(a, b);
    case 2:
        duel_0887DB24(a, b);
        return 0;
    case 3:
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
            if (duel_count_free_monster_zones(a->player) == 0) {
                return 0;
            }
            if (duel_phase_ok2(a->player) == 0) {
                return 0;
            }
            {
                DuelCard c = {0};

                c.id = 0x19ED;
                c.owner = a->player;
                duel_w_089148FC = duel_pick_monster_zone();
                duel_088263A8(a->player, &c, 1, a->id << 16 | 1);
            }
            return 0x7F;
        }
        case 0x7F:
            duel_088C87A8(a->player, a->zone, a->player | (duel_w_089148FC & 0xFF) << 8, 6, 0);
            break;
        }
    }
    return 0;
}
#endif

int duel_eff_fbcd4(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    switch (duel.players[p & 1].field[z].link / 10) {
    case 0x18C3:
        duel_088C87A8(p, z, a->id, 3, 0);
        break;
    default:
        duel_088CC178(p, z);
        break;
    }
    return 0;
}

int duel_eff_fbde0(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return duel_0887DDB0(a, b);
    }
    return 0;
}

int duel_eff_fbe94(DuelAct *a, int b)
{
    if (a->kind == 5) {
        return duel_eff_fbed8(a, b);
    }
    return duel_0887F8E4(a, b);
}

int duel_eff_fbed8(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    duel_088D1BEC(p, z, 0, 0);
    return 0;
}

int duel_eff_fbf94(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return duel_0887F8E4(a, b);
    }
    return 0;
}

int duel_eff_fc048(DuelAct *a, int b)
{
    int i = duel_hand_find_name(a->player, a->id);
    unsigned int p;

    if (i < 0) {
        return 0;
    }
    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    duel_088C8874(a->uid, a->id, 1, a->w[0]);
    p = a->player;
    duel_088263A8(p, &duel.players[p & 1].hand[i], 1, a->id << 16);
    return 0;
}
