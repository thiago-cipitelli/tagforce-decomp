// @range 0x088F85D4-0x088F8B34
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
unsigned int duel_act_target2(DuelAct *a, int i);
int duel_grave_find_uid(unsigned int p, unsigned int uid);
int duel_0884E5E0(DuelAct *a);
void duel_088FF1F4(void *dst, int fmt, void *arg);
void duel_0884ACD8(unsigned int p, void *msg);
int duel_0884E440(int id);
void duel_088359E8(unsigned int p, int id, int (*fn)(int id));
int duel_08835AC0(void);
void duel_088CB728(unsigned int p, int a, int b, unsigned int c, int d, int e, int f);
void duel_088C8B78(unsigned int p, unsigned int a, unsigned int b, int c);
void duel_0884DB38(int a, int b);
void duel_088C77D4(unsigned int p, int id, int a, int b);
int duel_08899BF0(DuelAct *a, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088D13CC(DuelAct *a, unsigned int mask);
void duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C5E70(unsigned int p, int a);
void duel_088C78E0(unsigned int p);
void duel_088CA384(unsigned int p, int id, int a);
int duel_0887FCA8(int id);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(int id));
void duel_08835978(int (*fn)(int id));

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
        *(short *)((char *)a + 10) = i;
        if (duel_0884E5E0(a) != 0) {
            return 0x7F;
        }
        return 0;
    case 0x7F:
        duel_088FF1F4(buf, 0x45, card_get_name(duel.list[i].id));
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
