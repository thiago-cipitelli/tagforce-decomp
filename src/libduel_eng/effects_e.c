// @range 0x088F9C00-0x088FA288
// @asm 0x088F9C00-0x088F9ED0
// @asm 0x088FA080-0x088FA288
/* Card effect handlers (continued). */
#include "duel.h"
#include "card_db.h"

extern int duel_player_type[2];
extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08914900;
extern int duel_w_08919624;

int duel_grave_count_id(unsigned int p, unsigned int id);
int duel_grave_find_name(unsigned int p, int id);
int duel_count_owned_name(unsigned int p, int id);
unsigned int duel_act_target(DuelAct *a, int i);
int field_get_card(unsigned int p, int z);
void duel_088CC178(unsigned int p, int z, int a);
void duel_08832974(unsigned int p, unsigned short id, int a);
int duel_08861480(unsigned short id, int i);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
int duel_0889BD04(DuelAct *a, int b);
int duel_0887F728(DuelAct *a, int b);
void duel_088C7CF0(void);
void duel_088C7BA4(void);
void duel_088C78E0(unsigned int p);
void duel_088328FC(unsigned int p);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
int duel_088DFE24(unsigned int p, unsigned int q);
int duel_088A9008(DuelAct *a, int b, int c);
int duel_0884E440();
int duel_088D1684(DuelAct *a, unsigned int p, int z, int b);
DuelCard *duel_0884C7CC(void);
void duel_088C6728(unsigned int p, int uid, int a);
int duel_088AA224(DuelAct *a);
void duel_0884C588(unsigned int p, int a, unsigned short id, unsigned int b);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_0884ACD8(unsigned int p, int msg);

#ifdef NON_MATCHING /* register allocation of the mask update and the zone argument */
int duel_eff_f9c00(DuelAct *a, int b)
{
    unsigned int k = a->kind;

    if (k == 6) {
        int n = duel_grave_count_id(a->player, a->id);

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
            if (n == 0) {
                return 0;
            }
            duel_w_089148FC = 0;
            duel_w_08914900 = 0;
            return 0x7F;
        }
        case 0x7F:
            if (duel_player_type[a->player] == 1) {
                int i;

                for (i = 0; i < 3; i++) {
                    if (!(duel_w_08914900 & 1 << i)) {
                        duel_w_08919624 = i;
                        return 0x7E;
                    }
                }
            } else {
                duel_0884ADAC(0x103, duel_08861480(a->id, 0), duel_08861480(a->id, 1), duel_08861480(a->id, 2), 0,
                              ~duel_w_08914900);
            }
            return 0x7E;
        case 0x7E:
            duel_08832974(1 - a->player, a->id, duel_w_08919624);
            return 0x7D;
        case 0x7D:
            duel_w_08914900 |= 1 << duel_w_08919624;
            if (n > 3) {
                n = 3;
            }
            if (++duel_w_089148FC < n) {
                return 0x7F;
            }
            duel_088CC178(a->player, a->zone, duel_w_08914900);
            return 0;
        }
        return 0;
    }
    if (k == 15) {
        return duel_0889BD04(a, b);
    }
    if (k == 22) {
        return duel_0887F728(a, b);
    }
    return 0;
}
#endif

int duel_eff_f9ed0(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) != 0) {
            unsigned int t = duel_act_target(a, 0);
            unsigned int q = t & 0xFF;
            unsigned int z = (unsigned char)((t & 0xFFFF) >> 8);

            if (duel_0884E440(a, q, z) != 0 && duel_088D1684(a, q, z, 0) != 0) {
                a->w[2] = duel.players[q & 1].field[z].card.id;
                return 0x7F;
            }
        }
        return 0;
    case 0x7F: {
        unsigned int p = a->player;

        if (duel_088DFE24(p, 1 - p) == 0) {
            return 0;
        }
        duel_088C7BA4();
        return 0x7E;
    }
    case 0x7E: {
        int i = duel_grave_find_name(1 - a->player, a->w[2]);

        if (i >= 0) {
            unsigned int q = 1 - a->player;

            duel_088CB400(q, &duel.players[q & 1].grave[i], 1);
            return 0x7E;
        }
        return 0x7D;
    }
    case 0x7D:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* branch layout of the 0x80 step */
int duel_eff_fa080(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) != 0) {
            unsigned int t = duel_act_target(a, 0);
            unsigned int q = t & 0xFF;
            unsigned int z = (unsigned char)((t & 0xFFFF) >> 8);

            if (duel_0884E440(a, q, z) == 0) {
                return 0;
            }
            a->w[1] = field_get_card(q, z);
            if (duel_088BFF78(a->player, a->id, duel_088AA224(a)) == 0) {
                if (card_attribute_icon_is3(a->w[1]) == 0) {
                    duel_0884ACD8(a->player, 0xD);
                    return 0x78;
                }
                return 0;
            }
            duel_0884ACD8(a->player, 0x3A);
            return 0x7E;
        }
        return 0;
    case 0x7E:
        duel_0884C588(a->player, 6, a->id, duel_088AA224(a));
        return 0x7D;
    case 0x7D:
        duel_088C6728(a->player, CARD_UID(*duel_0884C7CC()), 0);
        return 100;
    case 0x78:
        if (duel_count_owned_name(a->player, a->w[1]) < card_get_limit(a->w[1])) {
            duel_088328FC(1 - a->player);
        }
        return 100;
    case 100:
        if (card_attribute_icon_is3(a->w[1]) == 0) {
            duel_088C78E0(a->player);
        }
        return 0;
    }
    return 0;
}
#endif
