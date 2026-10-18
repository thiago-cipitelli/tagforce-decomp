// @range 0x088F9024-0x088F9668
// @asm 0x088F943C-0x088F9668
// @data 0x0891205C-0x08912078
/* Card effect handlers (continued). */
#include "duel.h"
#include "card_db.h"

extern int duel_player_type[2];
extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08919654;

int duel_pick_st_zone(unsigned int p);
unsigned int duel_act_target2(DuelAct *a, int i);
unsigned int duel_card_locate(unsigned int uid);
DuelCard *duel_field_card_deb2c(unsigned int p, int zone, int i);
int duel_list_card_level(unsigned int p, int i);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088C78E0(unsigned int p);
void duel_088328FC(unsigned int p);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_088DFE24(unsigned int p, unsigned int q);
void duel_088CB400(unsigned int p, void *c, int a);
void duel_088C77D4(unsigned int p, int id, int a, int b);
DuelCard *duel_0884C7CC(void);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_0884ACD8(unsigned int p, int msg);
int duel_08835AC0(void);
void duel_088C77EC(unsigned int p, int i);
int duel_0887FCA8(int id);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(int id));
void duel_08835978(int (*fn)(int id));
int duel_0884E9E8(void);
void duel_088C96EC(unsigned int p, int z, int a);
void duel_088C4508(unsigned int p, int id);
void duel_088C7B90(int n);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088CB728(unsigned int p, int a, int b, unsigned int c, int d, int e, int f);
int duel_088BFF78(unsigned int p, unsigned short id, int b);

int duel_eff_f9024(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_088DFE24(a->player, 1 - a->player) == 0) {
            return 0;
        }
        duel_w_089148FC = 0;
        if (a->n7 != 0) {
            int i;

            duel_088C7BA4();
            for (i = a->n7 - 1; i >= 0; i--) {
                unsigned int r = duel_card_locate(duel_act_target2(a, i));
                unsigned int zone = (unsigned char)((r & 0xFFFF) >> 8);

                if (zone == 14) {
                    DuelCard *c = duel_field_card_deb2c(r & 0xFF, zone, r >> 16 & 0xFFFF);
                    unsigned int o = c->owner;

                    duel_088CB400(o, c, a->player != o);
                    duel_w_089148FC++;
                }
            }
            duel_088C7CF0();
        }
        return 0x7F;
    case 0x7F: {
        unsigned char z = a->zone;
        unsigned int p = a->player;

        if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        if (duel.players[p & 1].field[z].faceup == 0) {
            return 0;
        }
        duel_088C87A8(p, z, a->id, 3, duel_w_089148FC * 300);
        return 0;
    }
    }
    return 0;
}

int duel_eff_f920c(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_0884E9E8() == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, 0x49);
        return 0x7F;
    case 0x7F:
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, a->id, duel_0887FCA8);
        } else {
            duel_08835978(duel_0887FCA8);
        }
        return 0x7E;
    case 0x7E:
        if (duel_08835AC0() != 0) {
            a->w[0] = CARD_UID(duel.players[a->player & 1].hand[duel_w_08919654]);
            a->w[1] = duel_list_card_level(a->player, a->w[0]) << 1;
            duel_088C77EC(a->player, a->w[0]);
            return 0x7D;
        }
        return 0x80;
    case 0x7D:
        duel_0884ACD8(a->player, 0x56);
        duel_088C7BA4();
        return 0x7C;
    case 0x7C:
        duel_0884C588(a->player, 6, a->id, a->w[1] << 16);
        return 0x7B;
    case 0x7B:
        duel_088CB400(a->player, duel_0884C7CC(), 0);
        a->w[1] -= card_get_level(duel_0884C7CC()->id);
        if (a->w[1] != 0) {
            return 0x7C;
        }
        return 0x7A;
    case 0x7A:
        duel_088C77D4(a->player, a->id, 1, a->w[0]);
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* the uid argument of the 0x7E step is scheduled differently */
int duel_eff_f943c(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            duel_0884ACD8(a->player, 0xD);
            return 0x78;
        }
        duel_0884ACD8(a->player, 0x5E);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7E;
    case 0x7E:
        if (card_get_prop5_hi(duel_0884C7CC()->id) == 2) {
            duel_088D1438(a->player, 10, 0, 0);
            duel_w_089148FC = 10;
        } else {
            duel_w_089148FC = duel_pick_st_zone(a->player);
        }
        duel_088CB728(a->player, 0xD, CARD_UID(*duel_0884C7CC()), a->player, duel_w_089148FC, 0, 0);
        return 0x7D;
    case 0x7D:
        duel_088C87A8(a->player, duel_w_089148FC, a->id, 1, 0);
        duel_088C96EC(a->player, duel_w_089148FC, 1);
        duel_088C4508(a->player, duel_0884C7CC()->id);
        duel_088C7B90(0x1E);
        duel_088C96EC(a->player, duel_w_089148FC, 0);
        return 100;
    case 0x78:
        duel_088328FC(1 - a->player);
        return 100;
    case 100:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}
#endif
