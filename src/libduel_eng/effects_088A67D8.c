// @range 0x088A67D8-0x088A8B94
// @asm 0x088A6F00-0x088A77A8
// @asm 0x088A7C88-0x088A8058
// @asm 0x088A83B4-0x088A8B94
/* Card effect handlers after effects_088A6464.c. */
#include "duel.h"

extern DuelAct *duel_w_089147DC;
extern unsigned int duel_w_089147E4;
extern unsigned int duel_w_089149F8;
extern int duel_w_089149E8;
extern int duel_w_089149F0;
extern int duel_w_089149F4;
extern int duel_w_089149FC;
extern int duel_w_08914A00;
extern unsigned int duel_view_player;
extern int duel_err;
extern int duel_w_089149EC;
extern int duel_w_08914A04;
extern unsigned short duel_w_089147E8[];
extern int duel_step;
extern int duel_player_type[2];
extern DuelCard duel_w_0891525C[];
extern unsigned int duel_w_0891964C;
extern unsigned int duel_w_08919650;
extern int duel_w_08919654;
extern int duel_w_08919624;
extern int duel_w_089148FC;

/* duel_w_089149F8 reread after a store into duel_chain.uids (the original reloads it there) */
#define CHAIN_F8 (*(volatile unsigned int *)&duel_w_089149F8)
#define CHAIN_F0 (*(volatile int *)&duel_w_089149F0)

int field_get_card(unsigned int p, int z);
int duel_field_protected(unsigned int p, int z);
int duel_088A65EC(unsigned int p, int z, int f);
int duel_slot_immune(unsigned int p, int z, int opp);
int duel_card_protected(int id, int uid);
int duel_088A6464(int id, int f);
int duel_pick_monster_zone(unsigned int p);
int duel_zone_usable(unsigned int p, unsigned int z);
int card_get_attribute_icon(int id);
int card_not_in_group_1a6f(int id);
int card_not_in_group_1a53(int id);
int duel_088A6300(int id);
void duel_088A63F8(void);
int duel_hand_find_uid(unsigned int p, int uid);
unsigned int duel_card_locate(unsigned int uid);
int duel_phase_ok2(unsigned int p);
int duel_count_active(int id);
int duel_088BFFD8();
int card_get_atk(int id);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_08835978(void *fn);
int duel_08835AC0(void);
void duel_0884ACD8(unsigned int p, void *msg);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884C7CC(void);
void duel_088CACF0(unsigned int p, DuelCard *c, int a);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
void duel_088CB44C(unsigned int p, DuelCard *c, int a);
int duel_088D1684(DuelAct *a, unsigned int p, int z, int flags);
int duel_088D1CB4(DuelAct *a, unsigned int p, int z, int flags);
int duel_088DFCC8(unsigned int uid);
DuelCard *duel_card_by_uid(int uid);
unsigned int duel_c_find_uid(unsigned int p, int uid);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_field_base_atk(unsigned int p, int z);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
int duel_host_call(int a, int b, int c, int d);
int duel_slot_efb84(unsigned int p, int z);
void duel_wfmt_d(unsigned short *dst, int fmt, int n);
int duel_zone_card_id(unsigned int p, int z, int i);
void duel_088C1618(unsigned int p);
int duel_slot_efacc(unsigned int p, int z, int id);
int duel_088DFE24(unsigned int p, unsigned int q);
void duel_0884CC68(int k);
int duel_0884C804(void);
int duel_count_free_monster_zones(unsigned int p);
int duel_0884E9E8(DuelAct *a, int b);
int duel_slot_has_effect2(unsigned int p, int z, int id);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
unsigned int duel_slot_effect12_arg(unsigned int p, int z, int id);
int duel_c_find_name(unsigned int p, int id);
int duel_0886CE38(DuelAct *a, int b);
void duel_088C6508(unsigned int p, int id, int a, int b);
void duel_088C78E0(unsigned int p);
void duel_088328FC(unsigned int p);
void duel_08832974(unsigned int p, unsigned short id, int a);
void duel_088C7BA4(void);
void duel_088C6728(unsigned int p, int uid, int a);
void duel_088C7CF0(void);

int duel_088A67D8(unsigned int p, int zone, int i)
{
    int uid;
    int z = zone + i;
    int id;
    unsigned int k;
    unsigned int n;
    unsigned int head;

    if (z < 5) {
        if (field_get_card(p, z) == 0) {
            return 0;
        }
        uid = CARD_UID(duel.players[p & 1].field[z].card);
        if (duel_field_protected(p, z)) {
            return 0;
        }
        if (duel_088A65EC(p, z, 0) == 0) {
            return 0;
        }
        if (duel_slot_immune(p, z, 0)) {
            return 0;
        }
    } else if (zone == 0xB) {
        id = duel.players[p & 1].hand[i].id;
        uid = CARD_UID(duel.players[p & 1].hand[i]);
        if (duel_card_protected(id, uid)) {
            return 0;
        }
        if (duel_088A6464(id, 0) == 0) {
            return 0;
        }
    } else {
        return 0;
    }
    n = duel_w_089149F8;
    k = 0;
    if (n != 0) {
        head = duel_w_089147E4;
        do {
            if (duel_chain.uids[(head + k) % 128] == uid) {
                return 0;
            }
            k++;
        } while (k < n);
    }
    if (duel_pick_monster_zone(p) < 0 && duel_w_089149F8 == 0) {
        if (z > 4) {
            return 0;
        }
        if (duel_zone_usable(p, z) == 0) {
            return 0;
        }
    }
    return 0x800;
}

int duel_088A69FC(unsigned int p, int zone, int i)
{
    if (p != (duel_w_089147DC->player ^ duel_w_089147DC->opp)) {
        return 0;
    }
    return duel_088A67D8(p, zone, i);
}

void *duel_088A6A48(unsigned int p)
{
    unsigned int i;
    int z;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (!duel_088A67D8(p, 0xB, i) != 0) {
            continue;
        }
        return &duel.players[p & 1].hand[i];
    }
    for (z = 0; z <= 4; z++) {
        if (!duel_088A67D8(p, z, 0) != 0) {
            continue;
        }
        return &duel.players[p & 1].field[z];
    }
    return 0;
}

int duel_088A6B5C(unsigned int p, int id)
{
    DuelCard *c;
    int i;
    unsigned int loc;
    unsigned int i2;

    if (card_get_attribute_icon(id) != 3) {
        return 0;
    }
    if (duel_card_protected(id, -1)) {
        return 0;
    }
    if (card_not_in_group_1a6f(id) == 0) {
        return 0;
    }
    duel_w_089149E8 = id;
    duel_w_089149F0 = duel_088A6300(id);
    duel_w_089149F8 = 0;
    duel_w_089149FC = !card_not_in_group_1a53(id);
    duel_088A63F8();
    for (i = 0; i < duel_w_089149F0; i++) {
        c = duel_088A6A48(p);
        if (c == 0) {
            return 0;
        }
        loc = CARD_UID(*c);
        i2 = duel_w_089149F8;
        duel_w_089149F8 = i2 + 1;
        duel_chain.uids[(duel_w_089147E4 + i2) & 0x7F] = loc;
        if (duel_hand_find_uid(p, CARD_UID(*c)) >= 0) {
            duel_088A6464(c->id, 1);
        } else {
            loc = duel_card_locate(CARD_UID(*c));
            duel_088A65EC(loc & 0xFF, (unsigned short)loc >> 8 & 0xFF, 1);
        }
    }
    return 1;
}

int duel_088A6D2C(int id, int n, DuelCard *c)
{
    int i;

    if (card_not_in_group_1a6f(id) == 0) {
        return 0;
    }
    duel_w_089149E8 = id;
    duel_w_089149F0 = n;
    duel_w_089149FC = !card_not_in_group_1a53(id);
    duel_w_089149F4 = 0;
    duel_088A63F8();
    for (i = 0; i < duel_w_089149F0; i++, c++) {
        if (duel_088A6464(duel.list[CARD_UID(*c)].id, 2) == 0) {
            return 0;
        }
    }
    if (duel_w_089149E8 == 0x1A4E && duel_w_08914A00 != 0 && duel_w_089149F4 == 0) {
        return 0;
    }
    return 1;
}

int duel_088A6E5C(DuelAct *a, int b)
{
    unsigned int p = a->player ^ a->opp;

    if (duel_phase_ok2(p) == 0) {
        return 0;
    }
    if (duel_count_active(0x197A)) {
        return 0;
    }
    if (p == duel_view_player) {
        duel_err = 0xE;
    }
    if (!duel_088BFFD8(p, a->id, 0)) {
        return 0;
    }
    return 1;
}

#ifdef NON_MATCHING /* registradores: p em $s0 e a em $s1 (original: $s1/$s2, temporários em $s0) */
int duel_088A6F00(DuelAct *a, int b)
{
    unsigned short buf[256];
    unsigned int p = a->player ^ a->opp;
    unsigned int loc;
    int k;
    int id;
    int x;
    int y;
    DuelCard *c;

    switch (duel_step) {
    case 0x80:
        if (!duel_088A6E5C(a, b)) {
            return 0;
        }
        duel_0884ACD8(a->player, (void *)0x5B);
        return 0x7F;
    case 0x7F:
        duel_0884C588(p, 6, a->id, 0);
        return 0x7E;
    case 0x7E:
        duel_w_089149E8 = duel_0884C7CC()->id;
        duel_w_089149EC = CARD_UID(*duel_0884C7CC());
        if (duel_088A6B5C(p, duel_w_089149E8) == 0) {
            return 0;
        }
        duel_w_089149F0 = duel_088A6300(duel_w_089149E8);
        duel_w_089149FC = !card_not_in_group_1a53(duel_w_089149E8);
        duel_w_089149F8 = 0;
        if (duel_player_type[p] == 1) {
            return 100;
        }
        duel_088A63F8();
        if (duel_w_089149E8 == 0x1A4E) {
            duel_0884ACD8(p, (void *)0x5C);
        } else {
            duel_wfmt_d(buf, 0xC, duel_w_089149F0);
            duel_0884ACD8(p, buf);
        }
        return 0x7D;
    case 0x7D:
        duel_08835978(duel_088A69FC);
        return 0x7C;
    case 0x7C:
        if (duel_08835AC0() != 0) {
            if (duel_w_08919650 < 5) {
                id = field_get_card(duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            } else {
                id = duel_zone_card_id(duel_w_0891964C, duel_w_08919650, duel_w_08919654);
            }
            k = CARD_UID(*duel_field_card_deb2c(duel_w_0891964C, duel_w_08919650, duel_w_08919654));
            if (duel_hand_find_uid(p, k) >= 0) {
                duel_088A6464(id, 1);
            } else {
                duel_088A65EC(p, duel_w_08919650 + duel_w_08919654, 1);
            }
            duel_w_089147E8[(duel_w_089147E4 + duel_w_089149F8) & 0x7F] = k;
            id = CHAIN_F8;
            duel_w_0891525C[id] = *duel_card_by_uid(k);
            duel_host_call(0x2B, duel_w_0891964C, duel_w_08919650, (unsigned short)k | (unsigned short)id << 16);
            if (++duel_w_089149F8 < duel_w_089149F0) {
                return 0x7D;
            }
            if (duel_w_089149E8 == 0x1A4E) {
                duel_w_08914A04 = 7;
                if (duel_088A6A48(p)) {
                    return 0x7D;
                }
                duel_w_089149F0 = duel_w_089149F8;
            }
            duel_w_089149F8 = 0;
            return 100;
        }
        if (duel_w_089149E8 == 0x1A4E && duel_w_089149F8 >= 2) {
            duel_w_089149F0 = duel_w_089149F8;
            duel_w_089149F8 = 0;
            return 100;
        }
        duel_host_call(0x2C, 0, 0, 0);
        return 0x80;
    case 100:
        if (duel_w_089149F8 == 0) {
            for (k = 0; k < duel_w_089149F0; k++) {
                loc = duel_card_locate(duel_chain.uids[(duel_w_089147E4 + k) & 0x7F]);
                duel_cmd((p ? 0x8000 : 0) | 0x4B, (unsigned short)loc >> 8 & 0xFF, k,
                         duel_chain.uids[(duel_w_089147E4 + k) & 0x7F]);
            }
            duel_w_08914A00 = 0;
        }
        if (duel_w_089149F8 < duel_w_089149F0) {
            k = duel_w_089147E8[(duel_w_089147E4 + duel_w_089149F8) & 0x7F];
            loc = duel_card_locate(k);
            y = (unsigned short)loc >> 8 & 0xFF;
            if (y == 0xB) {
                x = loc >> 16 & 0xFFFF;
                if (duel_088DFCC8(k)) {
                    y = 0x20;
                } else {
                    y = 0x10;
                }
                duel_cmd((p ? 0x8000 : 0) | 0x4C, 0xB, duel_w_089149F8, k);
                duel_088C1618(p);
                switch (a->id) {
                case 0x149C:
                    c = &duel.players[p & 1].hand[x];
                    duel_088CACF0(p, c, y);
                    break;
                default:
                    c = &duel.players[p & 1].hand[x];
                    duel_088CAE24(p, c, y);
                    break;
                }
                if (duel_w_089149E8 == 0x18FB) {
                    duel_w_08914A00 += card_get_atk(c->id);
                }
            } else if (y == 0xE) {
                id = loc >> 16 & 0xFFFF;
                duel_cmd((p ? 0x8000 : 0) | 0x4C, 0xE, duel_w_089149F8, k);
                c = &duel.players[p & 1].grave[id];
                duel_088CB44C(p, c, 0x10);
                if (duel_w_089149E8 == 0x18FB) {
                    duel_w_08914A00 += card_get_atk(c->id);
                }
            } else {
                y += loc >> 16 & 0xFFFF;
                if (duel_slot_efb84(p, y)) {
                    x = 0x20;
                } else {
                    x = 0x10;
                }
                duel_cmd((p ? 0x8000 : 0) | 0x4C, y, duel_w_089149F8, k);
                switch (a->id) {
                case 0x149C:
                case 0x1920:
                case 0x1921:
                case 0x1A7F:
                    duel_088D1684(a, p, y, x);
                    break;
                default:
                    duel_088D1CB4(a, p, y, x);
                    break;
                }
                if (duel_w_089149E8 == 0x18FB) {
                    duel_w_08914A00 += duel_field_base_atk(p, y);
                }
            }
            duel_w_089149F8++;
            return 100;
        }
        return 99;
    case 99:
        duel_cmd((p ? 0x8000 : 0) | 0x4D, duel_w_089149E8, duel_w_089149EC, duel_w_089149F0);
        duel_088263A8(p, &duel.players[p & 1].zone_c[duel_c_find_uid(p, duel_w_089149EC)], 1, a->id << 16 | 1);
        return 0;
    }
    return 0;
}
#endif

int duel_088A77A8(unsigned int p, int zone, int i)
{
    int uid;
    int z = zone + i;
    int id;
    unsigned int k;
    unsigned int n;
    unsigned int head;

    if (z < 5) {
        if (field_get_card(p, z) == 0) {
            return 0;
        }
        uid = CARD_UID(duel.players[p & 1].field[z].card);
        if (duel_field_protected(p, z)) {
            return 0;
        }
        if (duel_088A65EC(p, z, 0) == 0) {
            return 0;
        }
        if (duel_slot_immune(p, z, 0)) {
            return 0;
        }
        if (duel_slot_efacc(p, z, duel_w_089147DC->id) == 0) {
            return 0;
        }
    } else if (zone == 0xE) {
        id = duel.players[p & 1].grave[i].id;
        uid = CARD_UID(duel.players[p & 1].grave[i]);
        if (duel_card_protected(id, uid)) {
            return 0;
        }
        if (duel_088A6464(id, 0) == 0) {
            return 0;
        }
    } else {
        return 0;
    }
    n = duel_w_089149F8;
    k = 0;
    if (n != 0) {
        head = duel_w_089147E4;
        do {
            if (duel_chain.uids[(head + k) % 128] == uid) {
                return 0;
            }
            k++;
        } while (k < n);
    }
    if (duel_pick_monster_zone(p) < 0 && duel_w_089149F8 == 0) {
        if (z > 4) {
            return 0;
        }
        if (duel_zone_usable(p, z) == 0) {
            return 0;
        }
    }
    return 0x800;
}

int duel_088A79F0(unsigned int p, int id)
{
    int z;
    DuelPlayer *pl;
    unsigned int i;
    int uid;
    unsigned int k;

    if (card_get_attribute_icon(id) != 3) {
        return 0;
    }
    if (duel_card_protected(id, -1)) {
        return 0;
    }
    if (card_not_in_group_1a6f(id) == 0) {
        return 0;
    }
    duel_w_089149E8 = id;
    duel_w_089149F0 = duel_088A6300(id);
    duel_w_089149F8 = 0;
    duel_w_089149FC = !card_not_in_group_1a53(id);
    duel_088A63F8();
    pl = &duel.players[p & 1];
    for (z = 0; z <= 4; z++) {
        if (duel_088A77A8(p, z, 0)) {
            duel_088A65EC(p, z, 1);
            uid = CARD_UID(pl->field[z].card);
            k = CHAIN_F8;
            CHAIN_F8 = k + 1;
            duel_chain.uids[(duel_w_089147E4 + k) & 0x7F] = uid;
            if (CHAIN_F8 >= duel_w_089149F0) {
                return 1;
            }
        }
    }
    if (duel_088DFE24(p, p)) {
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            if (duel_088A77A8(p, 0xE, i)) {
                duel_088A6464(pl->grave[i].id, 1);
                uid = CARD_UID(pl->grave[i]);
                k = CHAIN_F8;
                CHAIN_F8 = k + 1;
                duel_chain.uids[(duel_w_089147E4 + k) & 0x7F] = uid;
                if (CHAIN_F8 >= duel_w_089149F0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* após o laço 0x7C: duel_w_08919624 em $a1 e duel_w_089149F8 em $a0 (original: $a0/$v1) */
int duel_088A7C88(DuelAct *a, int b)
{
    unsigned short buf[256];
    unsigned int i;
    unsigned int p = a->player ^ a->opp;
    int id;

    switch (duel_step) {
    case 0x80:
        if (!duel_0884E9E8(a, b)) {
            return 0;
        }
        duel_0884ACD8(p, (void *)0x5B);
        return 0x7F;
    case 0x7F:
        duel_0884C588(p, 6, a->id, 0);
        return 0x7E;
    case 0x7E:
        duel_w_089149E8 = duel_0884C7CC()->id;
        duel_w_089149EC = CARD_UID(*duel_0884C7CC());
        duel_w_089149F0 = duel_088A6300(duel_w_089149E8);
        duel_w_089149FC = !card_not_in_group_1a53(duel_w_089149E8);
        duel_w_089149F8 = 0;
        duel_088A63F8();
        if (duel_w_089149E8 == 0x1A4E) {
            duel_0884ACD8(p, (void *)0x5C);
        } else {
            duel_wfmt_d(buf, 0xC, duel_w_089149F0);
            duel_0884ACD8(p, buf);
        }
        return 0x7D;
    case 0x7D:
        if (duel_w_089149E8 == 0x157E && duel_count_free_monster_zones(p)) {
            duel_0884C588(p, 0xB, a->id, duel_w_089149E8);
        } else if (duel_w_089149E8 == 0x1A4E && duel_w_089149F8 >= 2) {
            duel_0884C588(p, 0x48, a->id, duel_w_089149E8);
        } else {
            duel_0884C588(p, 6, a->id, duel_w_089149E8);
        }
        return 0x7C;
    case 0x7C:
        for (i = 0; i < duel_w_08919624; i++) {
            duel_0884CC68(i + 1);
            id = duel_0884C7CC()->id;
            if (duel_0884C804() == 0xE) {
                duel_088A6464(id, 1);
            } else {
                duel_088A65EC(p, duel_0884C804(), 1);
            }
            duel_chain.uids[(duel_w_089147E4 + duel_w_089149F8) & 0x7F] = CARD_UID(*duel_0884C7CC());
            CHAIN_F8++;
        }
        if (duel_w_089149F8 < duel_w_089149F0) {
            return 0x7D;
        }
        if (duel_w_089149E8 == 0x1A4E) {
            if (duel_w_089149F8 == 2 && duel_w_089149F8 - duel_w_08919624 < 2) {
                duel_w_08914A04 = 7;
                if (duel_088BFFD8(p, a->id)) {
                    return 0x7D;
                }
            }
            duel_w_089149F0 = duel_w_089149F8;
        }
        duel_w_089149F8 = 0;
        return 100;
    default:
        return duel_088A6F00(a, b);
    }
}
#endif

int duel_088A8058(unsigned int p, int zone, int i)
{
    int id;
    int uid;
    unsigned int k;
    unsigned int n;
    unsigned int head;

    if (zone == 0xD) {
        id = duel.players[p & 1].zone_d[i].id;
        uid = CARD_UID(duel.players[p & 1].zone_d[i]);
        if (duel_card_protected(id, uid)) {
            return 0;
        }
        if (duel_088A6464(id, 0) == 0) {
            return 0;
        }
    } else {
        return 0;
    }
    n = duel_w_089149F8;
    k = 0;
    if (n != 0) {
        head = duel_w_089147E4;
        do {
            if (duel_chain.uids[(head + k) % 128] == uid) {
                return 0;
            }
            k++;
        } while (k < n);
    }
    return 0x800;
}

int duel_088A815C(unsigned int p, int id)
{
    unsigned int i;
    int uid;
    unsigned int k;

    if (card_get_attribute_icon(id) != 3) {
        return 0;
    }
    if (duel_card_protected(id, -1)) {
        return 0;
    }
    if (card_not_in_group_1a6f(id) == 0) {
        return 0;
    }
    duel_w_089149E8 = id;
    duel_w_089149F0 = duel_088A6300(id);
    duel_w_089149F8 = 0;
    duel_w_089149FC = 1;
    duel_088A63F8();
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (duel_088A8058(p, 0xD, i)) {
            duel_088A6464(duel.players[p & 1].zone_d[i].id, 1);
            uid = CARD_UID(duel.players[p & 1].zone_d[i]);
            k = CHAIN_F8;
            CHAIN_F8 = k + 1;
            duel_chain.uids[(duel_w_089147E4 + k) & 0x7F] = uid;
            if (CHAIN_F8 >= duel_w_089149F0) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_088A8304(DuelAct *a)
{
    int r;

    if (a->b3_4 == 0) {
        if (duel_count_free_monster_zones(a->player) == 0) {
            r = 0;
        } else if (duel.players[a->player & 1].n_18 == 0) {
            r = 0;
        } else {
            r = duel.players[a->player & 1].n_d != 0;
        }
    } else {
        r = duel_slot_has_effect2(a->player, a->zone, a->id) != 0;
    }
    return r;
}

#ifdef NON_MATCHING /* passo 0x7B: duel_w_08919624 em $a1 e duel_w_089149F8 em $a0 (original: $a0/$v1), como em duel_088A7C88 */
int duel_088A83B4(DuelAct *a, int b)
{
    unsigned short buf[256];
    unsigned int p = a->player ^ a->opp;
    unsigned int i;
    int k;
    DuelCard *c;
    DuelEffect *ef;
    unsigned int e;
    unsigned int u;

    if (a->b3_4 == 0) {
        switch (duel_step) {
        case 0x80:
            if (duel.players[a->player & 1].n_18 == 0) {
                return 0;
            }
            if (duel.players[a->player & 1].n_d == 0) {
                return 0;
            }
            if (duel_088BFFD8(p, a->id, 0) == 0) {
                duel_0884ACD8(p, (void *)0x122);
                return 0x6E;
            }
            duel_0884ACD8(a->player, (void *)0x5B);
            return 0x7F;
        case 0x7F:
            duel_0884C588(p, 6, a->id, 0);
            return 0x7E;
        case 0x7E:
            duel_w_089149E8 = duel_0884C7CC()->id;
            duel_w_089149EC = CARD_UID(*duel_0884C7CC());
            duel_w_089149F0 = duel_088A6300(duel_w_089149E8);
            duel_w_089149FC = 1;
            duel_w_089149F8 = 0;
            duel_088A63F8();
            duel_08832974(1 - p, a->id, duel_w_089149E8);
            return 0x7D;
        case 0x7D:
            if (duel_w_089149E8 == 0x1A4E) {
                duel_0884ACD8(p, (void *)0x5C);
            } else {
                duel_wfmt_d(buf, 0xC, duel_w_089149F0);
                duel_0884ACD8(p, buf);
            }
            return 0x7C;
        case 0x7C:
            if (duel_w_089149E8 == 0x157E) {
                duel_0884C588(p, 0xB, a->id, duel_w_089149E8);
            } else if (duel_w_089149E8 == 0x1A4E && duel_w_089149F8 >= 2) {
                duel_0884C588(p, 0x48, a->id, duel_w_089149E8);
            } else {
                duel_0884C588(p, 6, a->id, duel_w_089149E8);
            }
            return 0x7B;
        case 0x7B:
            for (i = 0; i < duel_w_08919624; i++) {
                duel_0884CC68(i + 1);
                duel_088A6464(duel_0884C7CC()->id, 1);
                duel_chain.uids[(duel_w_089147E4 + duel_w_089149F8) & 0x7F] = CARD_UID(*duel_0884C7CC());
                CHAIN_F8++;
            }
            if (duel_w_089149F8 < duel_w_089149F0) {
                return 0x7C;
            }
            if (duel_w_089149E8 == 0x1A4E) {
                if (duel_w_089149F8 == 2 && duel_w_089149F8 - duel_w_08919624 < 2) {
                    duel_w_08914A04 = 7;
                    if (duel_088BFFD8(p, a->id)) {
                        return 0x7C;
                    }
                }
                duel_w_089149F0 = duel_w_089149F8;
            }
            return 0x7A;
        case 0x7A:
            duel_w_089149F0 = duel_w_089149F8;
            duel_w_089149F8 = 0;
            duel_088C7BA4();
        case 0x79:
            if (duel_w_089149F8 < duel_w_089149F0) {
                k = duel_w_089147E8[(duel_w_089147E4 + duel_w_089149F8) & 0x7F];
                duel_088C6728(p, k, 0);
                duel_088C87A8(a->player, a->zone, a->id, 3, k);
                duel_w_089149F8++;
                return 0x79;
            }
            duel_088C7CF0();
            duel_088C87A8(a->player, a->zone, a->id, 0x41, duel_w_089149E8);
            return 0x6D;
        case 0x6E:
            duel_088328FC(1 - p);
            return 0x6D;
        case 0x6D:
            duel_088C78E0(p);
            return 0;
        }
    } else {
    switch (duel_step) {
    case 0x80:
        duel_w_089149E8 = duel_slot_effect12_arg(a->player, a->zone, a->id);
        k = duel_c_find_name(p, duel_w_089149E8);
        if (k < 0) {
            return 0;
        }
        duel_w_089149EC = CARD_UID(duel.players[p & 1].zone_c[k]);
        duel_w_089149F0 = 0;
        duel_w_08914A00 = 0;
        if (!duel_0886CE38(a, b) || duel_count_active(0x197A)) {
            duel_088C6508(a->player, duel_w_089149E8, 0, 0);
            return 0;
        }
        e = duel.players[a->player & 1].field[a->zone].effects;
        while (e != 0) {
            ef = &duel.effects[e];
            e = ef->next;
            if ((ef->kind & 0xF) == 3 && ef->src == a->id) {
                u = ef->arg;
                if (duel_w_089149E8 == 0x18FB) {
                    duel_w_08914A00 += card_get_atk(duel.list[u].id);
                }
                duel_chain.uids[(duel_w_089147E4 + duel_w_089149F0) & 0x7F] = u;
                duel_cmd((p ? 0x8000 : 0) | 0x4C, 0x10, CHAIN_F0, u);
                duel_w_089149F0++;
            }
        }
        duel_cmd((p ? 0x8000 : 0) | 0x4D, duel_w_089149E8, duel_w_089149EC, duel_w_089149F0);
        duel_w_089148FC = duel_pick_monster_zone(p);
        duel_088263A8(p, &duel.players[p & 1].zone_c[k], 1, a->id << 16 | 1);
        return 0x7F;
    case 0x7F:
        duel_088C87A8(p, duel_w_089148FC, a->player | a->zone << 8, 7, 0);
        return 0;
    }
    }
    return 0;
}
#endif
