// @range 0x088FA288-0x088FA920
// @asm 0x088FA288-0x088FA488
// @asm 0x088FA5C8-0x088FA920
/* Card effect handlers (continued). */
#include "duel.h"
#include "card_db.h"

extern int duel_step;
extern int duel_w_089148FC;

int duel_count_owned_name(unsigned int p, int id);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_rand(int n);
void duel_088C78E0(unsigned int p);
void duel_088328FC(unsigned int p);
DuelCard *duel_0884C7CC(void);
void duel_088CB554(unsigned int p, DuelCard *c);
int duel_088AA224(DuelAct *a);
void duel_0884C588(unsigned int p, int a, unsigned short id, unsigned int b);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_0884ACD8(unsigned int p, int msg);
int duel_0887DB24(void);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
int duel_088DFCC8(unsigned int uid);
void duel_088C7CF0(void);
void duel_088C7BA4(void);
void duel_088C6508(unsigned int p, int id, int a);
void duel_088CBAA4(void);
void duel_088D1858(int x, unsigned int p, int z);
void duel_088C9344(int x, unsigned int p, int z, int a);

#ifdef NON_MATCHING /* the original computes the target zone before the player */
int duel_eff_fa288(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_0887DB24() == 0) {
            return 0;
        } else {
            unsigned int t = duel_act_target(a, 0);
            unsigned int z = (unsigned char)((t & 0xFFFF) >> 8);
            unsigned char q = t;

            a->w[1] = duel.players[q & 1].field[z].card.id;
            if (card_attribute_icon_is3(a->w[1])) {
                return 0;
            }
            if (card_is_attribute9(a->w[1])) {
                return 0;
            }
            return 0x7F;
        }
    case 0x7F:
        if (duel_088BFF78(a->player, a->id, duel_088AA224(a)) == 0) {
            duel_0884ACD8(a->player, 0x15);
            return 0x78;
        }
        duel_0884ACD8(a->player, 0x33);
        return 0x7E;
    case 0x7E:
        duel_0884C588(a->player, 6, a->id, duel_088AA224(a));
        return 0x7D;
    case 0x7D:
        duel_088CB554(a->player, duel_0884C7CC());
        return 100;
    case 0x78:
        if (duel_count_owned_name(a->player, a->w[1]) < card_get_limit(a->w[1])) {
            duel_088328FC(1 - a->player);
        }
        return 100;
    case 100:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}
#endif

int duel_eff_fa488(DuelAct *a)
{
    unsigned int p;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
    case 0x7F:
        p = a->player;
        if (duel.players[p & 1].n_hand != 0) {
            duel_088CAE24(p, &duel.players[p & 1].hand[0], 0);
            p = a->player;
            if (duel_088DFCC8(CARD_UID(duel.players[p & 1].hand[0]))) {
                duel_w_089148FC++;
            }
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_088C4604(a, 1 - a->player, duel_w_089148FC * 200);
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* the constant 1 of 1 - player goes to a3 in the original */
int duel_eff_fa5c8(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_088C7BA4();
        duel_w_089148FC = 0;
        return 0x7F;
    case 0x7F:
        if (duel_w_089148FC++ < 2) {
            int r = duel_rand(duel.players[(1 - a->player) & 1].n_18);
            unsigned int q = 1 - a->player;

            duel_088C6508(q, duel.players[q & 1].zone_c[r].id, 0);
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* the original computes the zone before the player */
int duel_eff_fa6e4(int x, DuelAct *a)
{
    unsigned int z;

    a->b4_3 = a->b4_2 = a->b4_1 = 1;
    z = a->zone & 0xFF;
    if (a->uid == CARD_UID(duel.players[a->player & 1].field[z].card)) {
        duel_088CBAA4();
        duel_088D1858(x, a->player, a->zone);
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* same as duel_eff_fa6e4 */
int duel_eff_fa800(int x, DuelAct *a)
{
    unsigned int z;

    a->b4_3 = a->b4_2 = a->b4_1 = 1;
    z = a->zone & 0xFF;
    if (a->uid == CARD_UID(duel.players[a->player & 1].field[z].card)) {
        duel_088CBAA4();
        duel_088C9344(x, a->player, a->zone, 0);
    }
    return 0;
}
#endif
