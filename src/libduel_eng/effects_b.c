// @range 0x088F85D4-0x088F9024
/* Card effect handlers (continued). */
#include "duel.h"
#include "card_db.h"

extern unsigned int duel_view_player;
extern int duel_player_type[2];
extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08914900;
extern int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

int duel_pick_st_zone(unsigned int p);
int duel_slot_has_effect(unsigned int p, int z, int id);
unsigned int duel_act_target2(DuelAct*, int);
int duel_grave_find_uid(unsigned int p, unsigned int uid);
int duel_0884E5E0(DuelAct *a);
void duel_wfmt_s(void *dst, int fmt, void *arg);
void duel_0884ACD8(unsigned int p, void *msg);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088359E8(unsigned int p, int id, int (*fn)());
int duel_08835AC0(void);
void duel_088CB728(int, int, int, unsigned int, int, int, int);
void duel_088C8B78(unsigned int, unsigned short, unsigned short, int);
void duel_0884DB38(int a, int b);
void duel_088C77D4(unsigned int p, unsigned short id, unsigned short a, unsigned short b);
int duel_08899BF0(DuelAct *a, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
unsigned int duel_088D13CC(DuelAct*, unsigned int);
int duel_088C4604(DuelAct*, unsigned int, int);
int duel_088C5E70(unsigned int, int);
void duel_088C78E0(unsigned int p);
void duel_088CA384(unsigned int p, int id, int a);
int duel_0887FCA8(unsigned int p, int z, int k);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(unsigned int q, int z, int i));
void duel_08835978(int (*fn)());

int duel_eff_f85d4(DuelAct *a)
{
    unsigned short buf[256];
    int i = duel_act_target2(a, 0);

    switch (duel_step) {
    case 0x80:
        if (duel_pick_st_zone(a->player) < 0) {
            return 0;
        }
        if (duel_grave_find_uid(a->player, i) < 0) {
            return 0;
        }
        a->w[1] = i;
        if (duel_0884E5E0(a) != 0) {
            return 0x7F;
        }
        return 0;
    case 0x7F:
        duel_wfmt_s(buf, 0x45, card_get_name(duel.list[i].id));
        duel_0884ACD8(a->player, buf);
        duel_088359E8(a->player, 0x10D3, duel_0884E440);
        return 0x7E;
    case 0x7E:
        if (duel_08835AC0() != 0) {
            int c = duel_w_0891964C;
            int d = duel_w_08919650 + duel_w_08919654;
            int z = duel_pick_st_zone(a->player);

            duel_088CB728(a->player, 0xE, i, a->player, z, 1, 0);
            duel_088C8B78(a->player, (unsigned short)(a->player | (z & 0xFF) << 8),
                          (unsigned short)(c & 0xFF | (d & 0xFF) << 8), 0xB);
            if (a->player == duel_view_player) {
                duel_0884DB38(0x30, 1);
            }
            return 0;
        }
        return 0x7F;
    }
    return 0;
}

int duel_eff_f87b8(DuelAct *a, int b)
{
    if (duel_step == 0x80) {
        if (duel_slot_has_effect(a->player, 11, 0x14C4)) {
            return 0;
        }
        duel_088C77D4(a->player, 0x14C4, 1, 0);
    }
    return duel_08899BF0(a, b);
}

int duel_eff_f8838(DuelAct *a)
{
    unsigned int m = 0;

    if (duel.mode & 1) {
        return 0;
    }
    if (!(duel.mode >> 1 & 1)) {
        return 0;
    }
    duel_088C7BA4();
    m |= 1 << (duel_special.zone[0] + duel_special.player[0] * 16);
    m |= 1 << (duel_special.zone[1] + duel_special.player[1] * 16);
    duel_088D13CC(a, m);
    duel_088C4604(a, 1 - a->player, 800);
    duel_088C7CF0();
    return 0;
}

int duel_eff_f890c(DuelAct *a)
{
    unsigned int p = a->player;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        duel_w_08914900 = 0;
        if (duel.players[p & 1].n_hand == 0) {
            return 0;
        }
        duel_0884ACD8(p, (void *)0x1F);
        return 0x7F;
    case 0x7F:
        if (duel.players[p & 1].n_hand == 0) {
            return 0x7D;
        }
        if (duel_player_type[p] == 1) {
            duel_0881956C(p, a->id, duel_0887FCA8);
        } else {
            duel_08835978(duel_0887FCA8);
        }
        return 0x7E;
    case 0x7E:
        if (duel_08835AC0() != 0) {
            if (p != duel.players[a->player & 1].hand[duel_w_08919654].owner) {
                duel_w_08914900 = 1;
            }
            duel_w_089148FC++;
            duel_088CA384(a->player, duel_w_08919654, 1);
            return 0x7F;
        }
        if (duel_w_089148FC != 0) {
            return 0x7D;
        }
        return 0x80;
    case 0x7D:
        duel_088C78E0(p);
        if (duel_w_08914900 != 0) {
            duel_088C78E0(1 - p);
        }
        return 0x7C;
    case 0x7C:
        duel_088C5E70(p, duel_w_089148FC);
        return 0;
    }
    return 0;
}

typedef struct DuelList2 {             /* 0x0891524C */
    unsigned char pad0[0x10];
    DuelCard cards[64];                /* +0x10 */
} DuelList2;

extern DuelList2 duel_w_0891524C;
extern unsigned int duel_w_08919670;

int duel_count_owned_name(unsigned int p, int id);
int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2();
unsigned int duel_act_target(DuelAct *a, int i);
int field_get_card(unsigned int, int);
void duel_088C7810(unsigned int, unsigned int);
void duel_088328FC(unsigned int p);
unsigned int duel_card_locate(unsigned int);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
void duel_088C6508(unsigned int p, int id, int a, int b);
void duel_0882619C(unsigned int, DuelCard*, int, int, unsigned int);
int duel_088BFF78(unsigned int, int, int);
void duel_08832960(unsigned int p, int a, int b);
int duel_088A9008(DuelAct*, int, unsigned int*);
void duel_088CB914(unsigned int p, int z);
int duel_088304A0(unsigned int p, int z, int a);
void duel_088328E8(unsigned int, unsigned short, int);

int duel_eff_f8b34(DuelAct *a)
{
    unsigned int pl = a->player;
    unsigned int p = pl ^ duel_w_089148FC;
    int i;

    switch (duel_step) {
    case 0x80:
        a->w[2] = 0;
        a->w[3] = 0;
        if (duel_phase_ok2(1 - a->player) == 0 || duel_count_free_monster_zones(1 - a->player) == 0) {
            goto none;
        }
        duel_w_08919670 = 1 - a->player;
        for (i = 0; i <= 4; i++) {
            if (!duel_088304A0(1 - a->player, i, 0)) {
                continue;
            }
            duel_088328E8(1 - a->player, a->id, 0);
            return 0x7F;
        }
    none:
        return 0x7E;
    case 0x7F:
        duel_088CB914(duel.unk3830 & 0xFF, (unsigned char)((duel.unk3830 & 0xFFFF) >> 8));
        a->w[3] = field_get_card(duel.unk3830 & 0xFF, (unsigned char)((duel.unk3830 & 0xFFFF) >> 8));
        return 0x7E;
    case 0x7E:
        if (duel_phase_ok2(pl) && duel_count_free_monster_zones(a->player) && duel_088A9008(a, 0, 0)) {
            unsigned int t = duel_act_target(a, 0);
            unsigned int q = t & 0xFF;
            int z = (unsigned short)t >> 8 & 0xFF;
            int id = field_get_card(q, z);

            if (*(unsigned short *)(z * 20 + ((q & 1) * sizeof(DuelPlayer) + (unsigned int)&duel.players[0].field[0].faceup)) != 0 &&
                card_is_monster(id) && card_is_in_group_1a58(id)) {
                a->w[2] = field_get_card(q, z);
            }
        }
        duel_088C7BA4();
        duel_w_089148FC = 0;
        return 0x7D;
    case 0x7D:
        a->w[1] = a->w[duel_w_089148FC + 2];
        if (a->w[1] == 0) {
            return 100;
        }
        a->w[4] = duel_088BFF78(p, a->id, a->w[1] & 0xFFFF);
        duel_w_08914900 = 0;
        if (a->w[4] == 0) {
            duel_08832960(p, a->id, 0);
            return 0x78;
        }
        if (duel_count_free_monster_zones(p) < a->w[4]) {
            duel_08832960(p, a->id, 1);
            return 0x7A;
        }
        return 0x7C;
    case 0x7C:
        if (a->w[4] > duel_w_08914900) {
            {
            DuelCard *c = &duel_w_0891524C.cards[duel_w_08914900];
            duel_0882619C(p, c, 1, p != pl ? 2 : 0, 0);
            }
            duel_w_08914900++;
            return 0x7C;
        }
        return 0x78;
    case 0x7A:
        if (duel_w_08914900 < a->w[4]) {
            DuelCard *c = &duel_w_0891524C.cards[duel_w_08914900];

            if (((unsigned short)duel_card_locate(CARD_UID(*c)) >> 8 & 0xFF) == 11) {
                duel_088CAE24(p, c, 0);
            }
            duel_w_08914900++;
            return 0x7A;
        }
        duel_088C6508(p, a->w[1], 1, p != pl);
        return 0x78;
    case 0x78:
        i = duel_count_owned_name(p, a->w[1]);
        if (!(i < card_get_limit(a->w[1]))) {
            return 0x75;
        }
        if (card_attribute_icon_in_2_3(a->w[1]) != 0) {
            return 0x75;
        }
        duel_088C7810(p, 1);
        return 0x77;
    case 0x77:
        duel_088328FC(1 - p);
        return 0x76;
    case 0x76:
        duel_088C7810(p, 0);
        return 0x75;
    case 0x75:
        duel_088C78E0(p);
        return 100;
    case 100:
        if (++duel_w_089148FC < 2) {
            return 0x7D;
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}
