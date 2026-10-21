// @range 0x088966D8-0x08898B10
// @asm 0x088977BC-0x08897A5C
// @asm 0x08898608-0x08898810
/* Card effect handlers (DuelAct *a), after effects_zi.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern unsigned short duel_w_0891968C;
extern unsigned short duel_h_0891968E;
extern int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern unsigned int duel_view_player;
extern int duel_w_08919624;
extern int duel_w_08914900;
extern int duel_player_type[2];
extern int duel_turn_player;

int duel_0884E9E8(DuelAct *a, int b);
void duel_088C77EC(unsigned int p, int uid);
int duel_list_card_level(unsigned int p, int i);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
DuelCard *duel_card_by_uid(int uid);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
unsigned int duel_0884E4E4(DuelAct *a);
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
int duel_count_monsters(unsigned int p);
void duel_088328E8(unsigned int p, unsigned short id, int a);
void duel_08832894(unsigned int p, unsigned short id);
int duel_088559C0();
int duel_d_count_monsters(unsigned int p);
void duel_08832B40(unsigned int p, unsigned short id, int b);
void duel_wfmt_d(char *dst, char *fmt, int n);
void duel_wfmt_s(char *dst, char *fmt, char *arg);
void duel_0884ACD8(unsigned int p, void *msg);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088C4508(unsigned int p, int id);
int duel_zone_faceup(unsigned int p, int z, int i);
void duel_088C7880(unsigned int p);
void duel_088CB980(unsigned int p);
int card_is_monster(int id);
int card_is_group_b(int id);
int card_get_level(int id);
void duel_088C625C(unsigned int p, int a, int b);
int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2(unsigned int p);
int duel_card_dfac8(unsigned int p, int id, int field);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);
DuelCard *duel_0884C7CC(void);
void duel_088C69EC(unsigned int p, int uid, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_0884C588(unsigned int p, int a, int id, int b);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
void duel_088C7810(unsigned int p, int a);
void duel_088CAB04(unsigned int p, int i, int b);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_08832974(unsigned int p, unsigned short id, int a);
int card_get_type(int id);
void duel_088C452C(unsigned int p, int n);
void duel_088CA384(unsigned int p, int i, int f);
void duel_088C5E70(unsigned int p, int a);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_pick_st_zone(unsigned int p);
int duel_act_check_08853CE8(DuelAct *a, int x, int y);
void duel_088D1DA4(DuelAct *a, unsigned int p, int z, unsigned int q, int y);
void duel_088C8B78(unsigned int p, unsigned short loc, unsigned short t, unsigned short k);
int duel_pick_monster_zone(unsigned int p);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_0884DB38(int a, int b);
int duel_088D16C4(DuelAct *a, unsigned int p, int z, int f);
int card_is_attribute9(int id);
unsigned int duel_field_controller(unsigned int p, int z, int arg);
unsigned int duel_card_locate(unsigned int uid);
void duel_088C7E68(unsigned int p, int z, int a, int b);
void duel_088CB490(unsigned int p, int uid);
void duel_088CB778(unsigned int p, int z, int uid, int a, int b, int c, int d);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int card_get_kind(int id);
int duel_08897198(DuelAct *a);
int duel_08880D54(DuelAct *a, int b);
int duel_0887FD20(DuelAct *a, int b);
unsigned short duel_slot_e1854(unsigned int p, int z);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
int duel_088DFE24(unsigned int p, unsigned int q);
void duel_08832950(unsigned int p, unsigned short id);
void duel_088329D0(unsigned int p, int a, int b);
int duel_grave_find_uid(unsigned int p, int uid);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
int duel_08856248();
int duel_0884E760(DuelAct *a, unsigned int p, int (*fn)());
int duel_0884E690(DuelAct *a, int (*fn)());
void duel_088359E8(unsigned int p, int id, int (*fn)());
int duel_0884AFB8(int a, int b, int c);
void duel_08832B28(unsigned int p, int a, int b);
int duel_08881160(DuelAct *a, int b);
unsigned short duel_slot_src_attr8(unsigned int p, int z, int active);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_0886CE38();
int duel_08835AC0(void);
void duel_088D147C(DuelAct *a, int b, int c);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);

int duel_088966D8(DuelAct *a)
{
    char buf2[512];
    char buf[512];
    DuelCard *c;
    int owner;
    unsigned int p;
    int id;

    switch (duel_step) {
    case 0x80:
        if (duel.players[a->player & 1].n_d == 0) {
            return 0;
        }
        if (duel_d_count_monsters(a->player) == 0) {
            return 0x64;
        }
        duel_08832B40(1 - a->player, a->id, 0);
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel_w_0891968C;
        duel_wfmt_d(buf, (char *)0x130, duel_w_089148FC);
        duel_wfmt_s(buf2, (char *)0x108, buf);
        duel_0884ACD8(a->player, buf2);
        return 0x7E;
    case 0x7E:
        duel_088C7BA4();
        return 0x7D;
    case 0x7D:
        p = a->player;
        if (duel.players[p & 1].n_d != 0) {
            c = &duel.players[p & 1].zone_d[0];
            owner = c->owner;
            id = c->id;
            duel_088C4508(p, id);
            if (duel_zone_faceup(p, 0xD, 0) == 0) {
                duel_088C7880(p);
            }
            duel_088CB980(p);
            if (card_is_monster(id) && card_is_group_b(id)) {
                if (duel_w_089148FC == card_get_level(id)) {
                    duel_088C625C(p, 1, 0);
                } else if (duel_count_free_monster_zones(a->player) && duel_phase_ok2(a->player) &&
                           duel_card_dfac8(a->player, id, 0)) {
                    duel_088263A8(owner, c, 1, 0);
                } else {
                    duel_088C625C(p, 1, 0);
                }
            } else {
                duel_088C625C(p, 1, 0);
                return 0x7D;
            }
        }
        return 0x7C;
    case 0x7C:
        duel_088C7CF0();
        return 0;
    case 0x64:
        duel_0884ACD8(a->player, (void *)0xD);
        return 0x63;
    case 0x63:
        duel_088328FC(1 - a->player);
        return 0x62;
    case 0x62:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_08896A04(DuelAct *a)
{
    int uid;

    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            a->b4_3 = 1;
            return 0;
        }
        duel_0884ACD8(a->player, (void *)0x5E);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7E;
    case 0x7E:
        uid = CARD_UID(*duel_0884C7CC());
        duel_088C69EC(a->player, uid, 0x80);
        if (a->uid == CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
            duel_088C87A8(a->player, a->zone, a->id, 3, uid);
        }
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_08896B98(DuelAct *a)
{
    duel_088C77D4(a->player, a->id, 5, 0);
    return 0;
}

int duel_08896BC8(DuelAct *a)
{
    unsigned int i;
    unsigned int q;

    switch (duel_step) {
    case 0x80:
        duel_088C7810(a->player, 1);
        duel_088C7810(1 - a->player, 1);
        duel_w_089148FC = 0;
        return 0x7F;
    case 0x7F:
        q = duel_turn_player ^ duel_w_089148FC;
        for (i = 0; i < duel.players[q & 1].n_hand; i++) {
            if (card_is_monster(duel.players[q & 1].hand[i].id)) {
                duel_088CAB04(q, i, q != a->player);
            }
        }
        if (++duel_w_089148FC < 2) {
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_088C7810(a->player, 0);
        duel_088C7810(1 - a->player, 0);
        return 0;
    }
    return 0;
}

int duel_08896D50(DuelAct *a)
{
    int id;

    switch (duel_step) {
    case 0x80:
        if (duel.players[a->player & 1].n_d == 0) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 0;
        } else {
            duel_0884ADAC(0x104, 0x1B8, 0x1B9, 0x1BA, 0, 0xF);
        }
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel_w_08919624;
        duel_08832974(1 - a->player, a->id, duel_w_089148FC);
        return 0x7E;
    case 0x7E:
        if (duel_zone_faceup(a->player, 0xD, 0) == 0) {
            duel_088C7880(a->player);
            duel_088CB980(a->player);
            duel_088C7880(a->player);
        }
        return 0x7D;
    case 0x7D:
        id = duel.players[a->player & 1].zone_d[0].id;
        switch (duel_w_089148FC) {
        case 0:
            if (card_is_monster(id) == 0) {
                return 0x64;
            }
            break;
        case 1:
            if (card_get_type(id) != 0x16) {
                return 0x64;
            }
            break;
        case 2:
            if (card_get_type(id) != 0x17) {
                return 0x64;
            }
            break;
        }
        duel_088C452C(a->player, duel.players[(1 - a->player) & 1].lp);
        duel_088C452C(1 - a->player, duel.players[a->player & 1].lp);
        return 0x64;
    case 0x64:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_08896FCC(DuelAct *a)
{
    unsigned int p;

    switch (a->id) {
    case 0x15AA:
    case 0x17F3:
        p = 1 - a->player;
        break;
    case 0x16D9:
        p = a->player;
        break;
    }
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        duel_w_08914900 = 0;
    case 0x7F:
        if (duel.players[p & 1].n_hand != 0) {
            if (p != duel.players[p & 1].hand[0].owner) {
                duel_w_08914900 = 1;
                if (a->id != 0x17F3) {
                    duel_w_089148FC++;
                }
            } else {
                duel_w_089148FC++;
            }
            duel_088CA384(p, 0, 1);
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_088C78E0(p);
        if (duel_w_08914900 != 0) {
            duel_088C78E0(1 - p);
        }
        return 0x7D;
    case 0x7D:
        duel_088C5E70(p, duel_w_089148FC);
        return 0x7C;
    }
    return 0;
}

int duel_08897160(DuelAct *a)
{
    duel_088C77D4(1 - a->player, a->id, 0x11, 0);
    return 0;
}

int duel_08897198(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    switch (duel_step) {
    case 0x80:
        if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
            return 0;
        }
        if (a->zone < 5) {
            if (duel_088A9008(a, 0, 0) != 0) {
                t = duel_act_target(a, 0);
                p = t & 0xFF;
                z = (unsigned short)t >> 8 & 0xFF;
                duel_w_089148FC = duel_pick_st_zone(a->player);
                if (duel_act_check_08853CE8(a, p, z) == 1 && duel_w_089148FC >= 0) {
                    duel_088D1DA4(a, a->player, a->zone, a->player, duel_w_089148FC);
                    duel_088C8B78(a->player, (unsigned short)((unsigned char)a->player | (unsigned char)duel_w_089148FC << 8),
                                  (unsigned short)((unsigned char)p | (unsigned char)z << 8), 0xB);
                    return 0x7F;
                }
            }
        } else {
            duel_w_089148FC = duel_pick_monster_zone(a->player);
            if (duel_phase_ok2(a->player)) {
                if (duel_w_089148FC >= 0) {
                    duel_0882619C(a->player, &duel.players[a->player & 1].field[a->zone].card, 1, 0, 0);
                } else {
                    duel_088D1438(a->player, a->zone, 0, 0);
                }
                return 0x7F;
            }
        }
        return 0;
    case 0x7F:
        duel_088C87A8(a->player, duel_w_089148FC, 0xFB6, 2, 0);
        if (duel.list[a->uid].id != a->id) {
            duel_088D1438(a->player, duel_w_089148FC, 0, 0);
        }
        if (a->player == duel_view_player) {
            if (duel_w_089148FC >= 5) {
                duel_0884DB38(0x30, 1);
            }
            duel_0884DB38(0x31, 1);
        }
        return 0;
    }
    return 0;
}

int duel_088974A4(DuelAct *a)
{
    unsigned int q;
    unsigned int u;
    unsigned int b1;
    unsigned int b0;

    unsigned int p;
    unsigned int z;
    unsigned int t;
    int uid;
    int pos;
    int id;
    unsigned int loc;
    unsigned int lp;

    if (a->b3_4 == 0) {
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel_0884E440(a, p, z) != 0) {
                uid = CARD_UID(duel.players[p & 1].field[z].card);
                if (duel_088D16C4(a, p, z, 0x2000) == 0) {
                    return 0;
                }
                if (!card_is_attribute9(duel.players[p & 1].field[z].card.id)) {
                    pos = duel.players[p & 1].field[z].pos;
                    t = duel_field_controller(p, z, 1);
                    duel_088C77D4(a->player, a->id, 1, pos * 2 + (uid * 8 + t * 4) + 1);
                }
            }
        }
        return 0;
    } else {
        q = a->player ^ (a->id == 0x1A02);
        u = (unsigned int)a->idx >> 3;
        uid = (unsigned int)a->idx >> 2 & 1;
        b1 = (unsigned int)a->idx >> 1 & 1;
        b0 = a->idx & 1;
        loc = duel_card_locate(u);
        id = duel.list[u].id;
        switch (duel_step) {
        case 0x80:
            lp = loc & 0xFF;
            duel_w_089148FC = duel_pick_monster_zone(q);
            if (((unsigned short)loc >> 8 & 0xFF) != 0xF) {
                return 0;
            }
            if (duel_w_089148FC < 0) {
                duel_088CB490(lp, u);
                return 0;
            }
            duel_088CB778(lp, 0xF, u, q, duel_w_089148FC, b0, b1);
            return 0x7F;
        case 0x7F:
            if (card_is_monster(id) == 0) {
                duel_088D1438(q, duel_w_089148FC, 0, a->id);
                return 0;
            }
            if (q != uid) {
                duel_088C7E68(q, duel_w_089148FC, 0, !(duel.players[q & 1].field[duel_w_089148FC].flags & 1));
            }
            break;
        }
        return 0;
    }
}

int duel_0889778C(DuelAct *a)
{
    duel_088C625C(1 - a->player, 1, 1);
    return 0;
}

#ifdef NON_MATCHING /* clamps: o original carrega duel_w_089148FC antes do teste de 0xFFFF */
int duel_088977BC(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    unsigned int x;
    int v;

    DuelCard c;
    DuelStats st;
    char *q;
    int n;

    switch (duel_step) {
    case 0x80:
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel.players[p & 1].field[z].faceup != 0 && duel_act_immune(a, p, z) == 0) {
                q = (char *)&c;
                n = 4;
                if (q != 0) {
                    do {
                        *q++ = 0;
                    } while (--n);
                }
                c.id = 0x1602;
                c.owner = a->player;
                duel_w_089148FC = duel_pick_monster_zone(a->player);
                duel_088263A8(a->player, &c, 1, a->id << 16 | 1);
                return 0x7F;
            }
        }
        break;
    case 0x7F:
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        duel_field_stats(p, z, &st);
        duel_088C87A8(a->player, duel_w_089148FC, 0x1602, 5,
                      (card_get_type(st.id) & 0xFF) << 8 | (unsigned char)((card_get_kind(st.id) & 0xF) | (card_get_level(st.id) & 0xF) << 4));
        duel_088C87A8(a->player, duel_w_089148FC, 0x1602, 5, st.batk >= 0x10000 ? 0xFFFF : st.batk);
        duel_088C87A8(a->player, duel_w_089148FC, 0x1602, 5, st.bdef >= 0x10000 ? 0xFFFF : st.bdef);
        duel_088C87A8(p, z, (unsigned short)((unsigned char)a->player | (unsigned char)duel_w_089148FC << 8), 7, 0);
        break;
    }
    return 0;
}
#endif

int duel_08897A5C(DuelAct *a)
{
    unsigned int loc;
    int z;
    unsigned int p;

    if (a->w[2] == 2) {
        loc = duel_card_locate(a->w[3]);
        z = (unsigned short)loc >> 8 & 0xFF;
        if (z < 5) {
            p = loc & 0xFF;
            if (duel.players[p & 1].field[z].faceup != 0) {
                duel_088C87A8(p, z, a->id, 2, 0);
            }
        }
        return 0;
    }
    return duel_08897198(a);
}

int duel_08897B04(DuelAct *a, int b)
{
    if (a->kind == 0xF) {
        return duel_08880D54(a, b);
    }
    return duel_0887FD20(a, b);
}

int duel_08897B48(DuelAct *a)
{
    unsigned int t;

    if (a->w[2] == 2) {
        if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
            return 0;
        }
        t = duel_slot_e1854(a->player, a->zone);
        duel_088C97F0(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF, 0, 0);
        return 0;
    }
    return duel_08897198(a);
}

int duel_08897C24(DuelAct *a)
{
    duel_088C77D4(a->player, a->id, 4, a->uid);
    return 0;
}

int duel_08897C5C(DuelAct *a)
{
    unsigned int q;
    int i;

    q = duel_turn_player ^ duel_w_089148FC;
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        duel_088C7BA4();
    case 0x7F:
        if (duel_088DFE24(a->player, 1 - q) == 0) {
            return 0x64;
        }
        if (duel_088BFF78(q, a->id, 0) == 0) {
            return 0x64;
        }
        duel_08832950(q, a->id);
        return 0x7E;
    case 0x7E:
        if (duel_w_0891968C == 0) {
            return 0x64;
        }
        duel_088329D0(q, a->id, 3);
        return 0x7D;
    case 0x7D:
        for (i = 0; i < duel_w_0891968C; i++) {
            duel_088CB400(1 - q, &duel.players[(1 - q) & 1].grave[duel_grave_find_uid(1 - q, duel.unk3832[i])], 1);
        }
        return 0x64;
    case 0x64:
        if (++duel_w_089148FC < 2) {
            return 0x7F;
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_08897E0C(DuelAct *a)
{
    int i;
    int m;

    switch (duel_step) {
    case 0x80:
        duel_08832B28(a->player, a->zone, 2);
        return 0x7F;
    case 0x7F:
        if (duel_w_0891968C != duel_h_0891968E) {
            if (duel_player_type[a->player] == 1) {
                m = a->w[2] = MAX(duel_w_0891968C, duel_h_0891968E);
                if (duel_0884E760(a, 1 - a->player, duel_08856248) != 0) {
                    duel_w_08919624 = 1 << m;
                } else {
                    duel_w_08919624 = 1 << MIN(duel_w_0891968C, duel_h_0891968E);
                }
            } else {
                duel_0884AFB8(0x140, 1, (1 << duel_w_0891968C) + (1 << duel_h_0891968E));
            }
        } else {
            duel_w_08919624 = 1 << duel_w_0891968C;
        }
        return 0x7E;
    case 0x7E:
        for (i = 1; i <= 6; i++) {
            if (duel_w_08919624 & 1 << i) {
                a->w[2] = i;
            }
        }
        if (duel_0884E690(a, duel_08856248) == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, (void *)0x6D);
        duel_088359E8(a->player, a->id, duel_08856248);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            duel_088D147C(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return 0;
        }
        return 0x7E;
    }
    return 0;
}

int duel_0889802C(DuelAct *a, int b)
{
    switch (duel_step) {
    case 0x80:
        duel_088C77D4(a->player, a->id, 1, 0);
    }
    return duel_08881160(a, b);
}

int duel_08898094(DuelAct *a, int b)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    t = duel_act_target(a, 0);
    p = t & 0xFF;
    z = (unsigned short)t >> 8 & 0xFF;
    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) == 0) {
            return 0;
        }
        a->w[1] = 0;
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        if (duel_0884E440(a, p, z) != 0) {
            t = duel_slot_src_attr8(p, z, 0);
            z = (unsigned short)t >> 8 & 0xFF;
            p = t & 0xFF;
            if (duel_0886CE38(a, b) != 0) {
                duel_w_08914900 = duel_slot_has_effect(p, z, 0xFB6);
                duel_w_089148FC = duel_pick_monster_zone(a->player);
                duel_088263A8(p, &duel.players[p & 1].field[z].card, 1, 0);
                return 0x7E;
            }
        }
        return 0x7D;
    case 0x7E:
        if (duel_w_08914900 != 0) {
            duel_088C87A8(a->player, duel_w_089148FC, 0xFB6, 2, 0);
        }
        return 0x7F;
    case 0x7D:
        if (duel.players[p & 1].field[z].card.id != 0) {
            duel_cmd((p ? 0x8000 : 0) | 0x1B, z, 0, 0);
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_088982B4(DuelAct *a, int b)
{
    int l0;
    int l1;

    switch (duel_step) {
    case 0x80:
        if (duel_0884E9E8(a, b) == 0) {
            return 0;
        }
        duel_08832894(a->player, a->id);
        return 0x7F;
    case 0x7F:
        a->w[0] = duel_h_0891968E;
        duel_08832894(1 - a->player, a->id);
        return 0x7E;
    case 0x7E:
        a->w[1] = duel_h_0891968E;
        duel_088C77EC(a->player, a->w[0]);
        duel_088C77EC(1 - a->player, a->w[1]);
        l0 = duel_list_card_level(a->player, a->w[0]);
        l1 = duel_list_card_level(1 - a->player, a->w[1]);
        duel_088C7BA4();
        if (l1 < l0) {
            duel_088C4604(a, 1 - a->player, 1000);
            duel_088CAE24(1 - a->player, duel_card_by_uid(a->w[1]), 0);
        } else if (l0 < l1) {
            duel_088C4604(a, a->player, 1000);
            duel_088CAE24(a->player, duel_card_by_uid(a->w[0]), 0);
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_08898468(DuelAct *a)
{
    int i;

    for (i = duel.players[(1 - a->player) & 1].n_hand - 1; i >= 0; i--) {
        duel_088CAB04(1 - a->player, i, 1);
    }
    duel_088D13CC(a, duel_0884E4E4(a));
    return 0;
}

int duel_0889850C(DuelAct *a)
{
    int n;

    n = duel_count_monsters(1 - a->player);
    if (n <= duel_count_monsters(a->player)) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        return 0x7F;
    case 0x7F:
        if (duel_0884E690(a, duel_088559C0) == 0) {
            return 0;
        }
        duel_088328E8(1 - a->player, a->id, 0);
        return 0x7E;
    case 0x7E:
        duel_088D147C(a, duel_w_0891968C & 0xFF, duel_w_0891968C >> 8 & 0xFF);
        return 0x7F;
    }
    return 0;
}

#ifdef NON_MATCHING /* clamps: como em duel_088977BC */
int duel_08898608(DuelAct *a)
{
    unsigned int t;
    DuelCard c;
    DuelStats st;
    char *q;
    int n;

    switch (duel_step) {
    case 0x80:
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            if (duel_0884E440(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF) != 0) {
                q = (char *)&c;
                n = 4;
                if (q != 0) {
                    do {
                        *q++ = 0;
                    } while (--n);
                }
                c.id = 0x1608;
                c.owner = a->player;
                duel_w_089148FC = duel_pick_monster_zone(a->player);
                duel_088263A8(a->player, &c, 1, a->id << 16 | 1);
                return 0x7F;
            }
        }
        break;
    case 0x7F:
        t = duel_act_target(a, 0);
        duel_field_stats(t & 0xFF, (unsigned short)t >> 8 & 0xFF, &st);
        duel_088C87A8(a->player, duel_w_089148FC, 0x1608, 5,
                      (unsigned char)st.attr << 8 | (unsigned char)((st.unk8 & 0xF) | (st.level & 0xF) << 4));
        duel_088C87A8(a->player, duel_w_089148FC, 0x1608, 5, st.atk > 0xFFFF ? 0xFFFF : st.atk);
        duel_088C87A8(a->player, duel_w_089148FC, 0x1608, 5, st.def > 0xFFFF ? 0xFFFF : st.def);
        break;
    }
    return 0;
}
#endif

int duel_08898810(DuelAct *a)
{
    unsigned int p;
    unsigned int z;
    unsigned int t;
    unsigned int p0;
    unsigned int z0;
    unsigned int p1;
    unsigned int z1;

    switch (duel_step) {
    case 0x80:
        switch (a->w[2]) {
        case 1:
        case 3:
            if (duel_088A9008(a, 0, 0) && duel_088A9008(a, 1, 0)) {
                t = duel_act_target(a, 0);
                p0 = t & 0xFF;
                z0 = (unsigned short)t >> 8 & 0xFF;
                t = duel_act_target(a, 1);
                p1 = t & 0xFF;
                z1 = (unsigned short)t >> 8 & 0xFF;
                a->w[2] = 1;
                if (duel_0884E440(a, p0, z0) == 0) {
                    return 0;
                }
                a->w[2] = 3;
                if (duel_0884E440(a, p1, z1) == 0) {
                    return 0;
                }
                duel_w_08914900 = duel_slot_has_effect(p0, z0, 0xFB6);
                duel_w_089148FC = duel_pick_st_zone(a->player);
                if (duel_w_089148FC >= 0) {
                    duel_088D1DA4(a, p0, z0, p0, duel_w_089148FC);
                    duel_088C8B78(a->player, (unsigned short)((unsigned char)p0 | (unsigned char)duel_w_089148FC << 8),
                                  (unsigned short)((unsigned char)p1 | (unsigned char)z1 << 8), 0xB);
                    if (a->player == duel_view_player) {
                        duel_0884DB38(0x30, 1);
                    }
                    return 0x7F;
                }
            }
            break;
        case 2:
            if (duel_088A9008(a, 0, 0)) {
                t = duel_act_target(a, 0);
                p = t & 0xFF;
                z = (unsigned short)t >> 8 & 0xFF;
                if (duel_0884E440(a, p, z)) {
                    duel_w_08914900 = duel_slot_has_effect(p, z, 0xFB6);
                    duel_w_089148FC = duel_pick_monster_zone(a->player);
                    if (duel_w_089148FC >= 0 && duel_phase_ok2(a->player)) {
                        duel_0882619C(a->player, &duel.players[p & 1].field[z].card, 1, 0, 0);
                        return 0x7F;
                    }
                }
            }
            break;
        }
        return 0;
    case 0x7F:
        if (duel_w_08914900 != 0) {
            duel_088C87A8(a->player, duel_w_089148FC, 0xFB6, 2, 0);
        }
        return 0;
    }
    return 0;
}
