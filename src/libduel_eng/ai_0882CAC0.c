// @range 0x0882CAC0-0x0882D398
// @data 0x08907968-0x08907974
/* Summoning a monster (duel_summon), continued: summons with special costs. */
#include "duel.h"

extern int duel_player_type[2];
extern int duel_w_08919624;            /* answer of the last question */
extern int duel_w_0891964C;            /* picked player */
extern int duel_w_08919650;            /* picked zone */
extern int duel_w_08919654;

int duel_08835AC0(void);
int duel_host_call(int a, int b, unsigned int c, int d);
int duel_0884AD18(int msg);
void duel_wfmt_d(unsigned short *dst, int fmt, int n);
int duel_list_card_level(unsigned int p, int i);
int duel_double_tribute(unsigned int p, int id, unsigned int q, int z);
void duel_0881956C(unsigned int p, int id, int (*fn)());
void duel_08835978(int (*fn)());
int duel_0882926C(unsigned int p, int z, int i);
int duel_0882AAA0(unsigned int p, int z, int i);
int duel_08827DA4(unsigned int p, int uid, int a, int b);
void duel_0884AD40(int msg, int uid);
int duel_0884ACD8(unsigned int p, int msg);
int duel_list_find_id(unsigned int p, unsigned int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_088C448C(int p, int a);
void duel_088D1AC8(unsigned int p, int z);
int card_is_in_group_19bb(int id);
int duel_count_free_monster_zones(unsigned int p);
unsigned short *card_get_name(int id);
void duel_wfmt_s();
int duel_summon_tributes(void);
int duel_0882A680(unsigned int p, int z, int i);
int duel_0882AA10(unsigned int p, int z, int i);
int card_get_type(int id);
void duel_088C4564(unsigned int p, int n);
void duel_088CABA8(int p, unsigned int i);
extern int duel_w_0891962C;            /* picked card (uid) */
int duel_088297A0(unsigned int p, int z, int i);
int duel_08829D74(unsigned int p, int z, int i);
int duel_08829EB8(unsigned int p, int z, int i);
int duel_0882A018(unsigned int p, int z, int i);
int duel_0882A23C(unsigned int p, int z, int i);
int duel_0882ABB4(unsigned int p, int z, int i);
int duel_08826710(int id);
int duel_hand_find_name(unsigned int p, int id);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088D1BEC(unsigned int p, int z, int a, int b);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
int duel_088D1608(int p, int z, int flags);
unsigned int duel_card_locate(unsigned int uid);
void duel_088CABF4(unsigned int p, int i, int a);
void duel_088D1B20(unsigned int p, unsigned int mask);

int duel_ritual_ids2[3] = { 0x1288, 0x129B, 0x12B8 };   /* same as duel_ritual_ids */

/* Summon of 0x1288: the three monsters of duel_0882A23C, one after the other. */
int duel_0882CAC0(void)
{
    unsigned short msg[256];
    int k;
    unsigned int mask;

    switch (duel_summon.step) {
    case 0:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_0882A23C);
        } else {
            duel_wfmt_s(msg, 0xD3, card_get_name(duel_ritual_ids2[duel_summon.n]));
            duel_0884ACD8(duel_summon.player, (int)msg);
            duel_08835978(duel_0882A23C);
        }
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            duel_summon.uids[duel_summon.n] = duel_w_0891962C;
            duel_host_call(0x28, duel_w_0891964C, duel_w_08919650, duel_w_0891962C);
            if (++duel_summon.n < 3) {
                duel_summon.step--;
                return 0;
            }
            duel_summon.step++;
            return 0;
        }
        duel_summon.step--;
        return 0;
    case 2:
        mask = 0;
        for (k = 0; k < duel_summon.n; k++) {
            unsigned int loc = duel_card_locate(duel_summon.uids[k]);
            int z = (unsigned char)((loc & 0xFFFF) >> 8);

            if (z < 0xB) {
                mask |= 1 << (z + (loc & 0xFF) * 16);
            } else {
                duel_088CABF4(loc & 0xFF, (unsigned short)(loc >> 16), 0);
            }
        }
        duel_088D1B20(duel_summon.player, mask);
        duel_summon.step++;
        return 0;
    case 3:
        duel_summon.n = 0;
        return 1;
    }
    return 0;
}

/* Summon with the two tributes 0x13E5 and 0x13E7. */
int duel_0882CD40(void)
{
    unsigned short msg[256];
    unsigned short tmp[256];
    unsigned int p;
    int z;

    switch (duel_summon.step) {
    case 0:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_0882A680);
        } else {
            duel_wfmt_s(tmp, 0xDA, card_get_name(0x13E5));
            duel_wfmt_s(msg, tmp, card_get_name(0x13E7));
            duel_0884ACD8(duel_summon.player, (int)msg);
            duel_08835978(duel_0882A680);
        }
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_summon.n = 1;
            duel_summon.locs[0] = (unsigned char)p | (unsigned char)z << 8;
            duel_summon.step++;
            return 0;
        }
        return -1;
    case 2:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_0882A680);
        } else {
            duel_0884ACD8(duel_summon.player, 4);
            duel_08835978(duel_0882A680);
        }
        duel_summon.step++;
        return 0;
    case 3:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_summon.n = 2;
            duel_summon.locs[1] = (unsigned char)p | (unsigned char)z << 8;
            return 1;
        }
        duel_summon.step = 0;
        duel_host_call(0x29, 0, 0, 0);
        return 0;
    }
    return 0;
}

/* Summon by an effect: done at once; cards of type 9 may use 0x1AA3. */
int duel_0882D068(void)
{
    if (duel_summon.f1) {
        return 1;
    }
    if (card_get_type(duel_summon.id) == 9) {
        return duel_0882BF2C();
    }
    return duel_summon_tributes();
}

/* Summon by an effect in face-down defense. */
int duel_0882D0D0(void)
{
    if (duel_summon.f1) {
        duel_summon.kind = 0;
        return 1;
    }
    return duel_summon_tributes();
}

/* Summon by an effect that pays 500 LP. */
int duel_0882D130(void)
{
    if (duel_summon.f1) {
        duel_088C4564(duel_summon.player, 500);
        return 1;
    }
    return duel_summon_tributes();
}

/* Summon by an effect that discards a card of the hand (duel_0882AA10). */
int duel_0882D180(void)
{
    if (duel_summon.f1) {
        switch (duel_summon.step) {
        case 0:
            if (duel_player_type[duel_summon.player] == 1) {
                duel_0881956C(duel_summon.player, duel_summon.id, duel_0882AA10);
            } else {
                duel_0884ACD8(duel_summon.player, 0xEB);
                duel_08835978(duel_0882AA10);
            }
            duel_summon.step++;
            return 0;
        case 1:
            if (duel_08835AC0() != 0) {
                duel_088CABA8(duel_w_0891964C, duel_w_08919654);
            } else {
                duel_summon.step--;
                return 0;
            }
            break;
        }
        return 1;
    }
    return duel_summon_tributes();
}

/* Normal summon that afterwards discards the other cards of the hand. */
int duel_0882D2A0(void)
{
    int r;

    switch (duel_summon.step) {
    case 0:
        duel_summon.step = 0xA;
    default:
        r = duel_summon_tributes();
        if (r == 1) {
            duel_summon.step = 0xF;
            return 0;
        }
        return r;
    case 0xF:
        if (duel.players[duel_summon.player & 1].n_hand > 1) {
            duel_088CABA8(duel_summon.player, duel_summon.uid == CARD_UID(duel.players[duel_summon.player & 1].hand[0]));
            return 0;
        }
        return 1;
    }
}
