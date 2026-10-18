// @range 0x088FB5E4-0x088FBAC8
/* Card effect handlers (continued). */
#include "duel.h"
#include "card_db.h"

extern int duel_step;

int field_get_card(unsigned int p, int z);
int duel_field_e5ae0(unsigned int p, int z);
int duel_hand_count_monsters(unsigned int p);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_088A9008(DuelAct *a, int b, int c);
int duel_08887ED8(DuelAct *a, int b);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088C625C(unsigned int p, int a, int b);
int duel_088DFCC8(unsigned int uid);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
int duel_0887DE04(DuelAct *a, int b);
void duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
void duel_088C7810(unsigned int p, int a);
void duel_08832950(unsigned int p, unsigned short id);
void duel_08832894(unsigned int p, unsigned short id);
void duel_088CACF0(unsigned int p, DuelCard *c, int a);
void duel_088CC178(unsigned int p, int z, int a);
int duel_08881160(DuelAct *a, int b);

int duel_eff_fb5e4(DuelAct *a, int b)
{
    int r = duel_08887ED8(a, b);
    int i;

    if (r == 0) {
        for (i = 0; i <= 4; i++) {
            int id = field_get_card(a->player, i);

            if (id != 0 && duel.players[a->player & 1].field[i].faceup != 0 && card_is_in_group_1a6f(id) &&
                duel_field_e5ae0(a->player, i)) {
                duel_088C87D8(a, a->player, i, a->id, 2, 0);
            }
        }
    }
    return r;
}

int duel_eff_fb6dc(DuelAct *a)
{
    unsigned int i;
    int n;
    unsigned int p;

    duel_088C7BA4();
    duel_088C625C(a->player, 3, 0);
    n = 0;
    for (i = 0; p = a->player, i < MIN((unsigned int)duel.players[p & 1].n_d, 3); i++) {
        if (card_is_monster(duel.players[p & 1].zone_d[i].id) == 0 &&
            duel_088DFCC8(CARD_UID(duel.players[a->player & 1].zone_d[i]))) {
            n++;
        }
    }
    duel_088C4604(a, p, n * 1000);
    duel_088C7CF0();
    return 0;
}

int duel_eff_fb81c(DuelAct *a, int b)
{
    switch (duel_step) {
    case 0x80:
        if (a->b4_2 == 0 && duel_088A9008(a, 0, 0)) {
            unsigned int t = duel_act_target(a, 0);
            unsigned int p = t & 0xFF;
            unsigned int z = (unsigned char)((t & 0xFFFF) >> 8);

            if (duel.players[p & 1].field[z].faceup != 0 && duel.players[p & 1].field[z].pos == 0) {
                duel_088C97F0(a, p, z, 0, 1);
            }
        }
        return 0x7F;
    case 0x7F:
        return duel_0887DE04(a, b);
    }
    return 0;
}

int duel_eff_fb914(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_hand_count_monsters(a->player) == 0) {
            duel_088C7810(a->player, 1);
            return 0x78;
        }
        duel_08832894(a->player, a->id);
        return 0x7F;
    case 0x7F: {
        unsigned int i = duel.unk3830;
        int kind = card_get_kind(duel.players[a->player & 1].hand[i].id);

        duel_088CACF0(a->player, &duel.players[a->player & 1].hand[i], 0);
        duel_088CC178(a->player, a->zone, kind);
        return 0;
    }
    case 0x78:
        duel_08832950(1 - a->player, a->id);
        return 0x77;
    case 0x77:
        duel_088C7810(a->player, 0);
        return 0;
    }
    return 0;
}

int duel_eff_fba80(DuelAct *a, int b)
{
    if (duel.mode & 1) {
        return 0;
    }
    if (a->w[2] != 0) {
        return duel_08881160(a, b);
    }
    return 0;
}
