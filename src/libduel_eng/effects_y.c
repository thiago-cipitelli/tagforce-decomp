// @range 0x08889BE4-0x0888B118
// @asm 0x08889BE4-0x0888A020
// @asm 0x0888A88C-0x0888AB18
// @asm 0x0888AF04-0x0888B078
/* Card effect handlers (DuelAct *a), after effects_x.c. */
#include "duel.h"

extern int duel_step;
extern unsigned int duel_turn_player;
extern int duel_w_089148FC;
extern int duel_w_08914900;
extern DuelCard duel_w_0891525C[];
extern int duel_w_08919624;
extern int duel_phase;
extern unsigned short duel_h_0891968E;
extern int duel_player_type[2];
extern unsigned short duel_w_0891968C;
extern struct {
    unsigned char b0 : 1;
    unsigned char zone : 4;
} duel_b_08914A14;

int card_is_monster(int id);
int card_is_attribute9(int id);
int card_attribute_icon_is3(int id);
int card_attribute_icon_in_2_3(int id);
int card_get_level(int id);
void duel_088D19B4(DuelAct *a, unsigned int mask);
void duel_088C78E0(unsigned int p);
int duel_phase_ok2();
void duel_088C7BA4(void);
void duel_088C4508(unsigned int p, int a);
int duel_zone_faceup(unsigned int p, int z, int i);
void duel_088C7880(unsigned int p);
void duel_088CB980(unsigned int p);
int duel_card_dfac8(unsigned int p, int id, int field);
int duel_no_1a1b(unsigned int p);
int duel_count_free_monster_zones(unsigned int p);
void duel_card_copy(DuelCard *dst, DuelCard *src);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
int duel_pick_monster_zone(unsigned int p);
void duel_088C625C(unsigned int p, int n, int x);
void duel_088CB9E0(unsigned int p, unsigned int mask);
void duel_088C7CF0(void);
int duel_0887F728(DuelAct *a, int b);
int duel_pick_st_zone(unsigned int p);
void duel_088CA184(unsigned short loc, unsigned short to);
void duel_088C8B70(unsigned int p, unsigned short loc, unsigned short t);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_088C7DD4(unsigned int p, int a, int b);
int duel_act_target2(DuelAct *a, int i);
int duel_grave_find_uid(unsigned int p, int uid);
void duel_088CB2AC(DuelAct *a, unsigned int p, DuelCard *c);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_088C7810(unsigned int p, unsigned int on);
void duel_088C7B90(int a);
int duel_0887DB24(DuelAct *a);
int duel_df664(unsigned int p);
int duel_088BFFD8();
void duel_0884AD18(int msg);
void duel_08832894(unsigned int p, int a);
void duel_088C7548(unsigned int p, int i, int x);
int duel_0886CE38();
unsigned int duel_card_locate(int uid);
int duel_zone_f_card_usable(unsigned int p, unsigned int q, int i);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
DuelCard *duel_card_by_uid(int uid);
int card_get_prop5_hi(int id);
unsigned short *dlg_table_sel(int i);
void duel_08832988(unsigned int p, unsigned short id, unsigned short n, int a);
void duel_wfmt_s(unsigned short *dst, int fmt, unsigned short *arg);
void duel_0884ACD8(unsigned int p, void *msg);
int duel_field_kind(unsigned int p, int z);
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C5424(unsigned int p, int n);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);
int duel_0884E398(DuelAct *a, int b);
void duel_088D147C(DuelAct *a, unsigned int p, int z);
void duel_088CC178();
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect();
int duel_088D1684(DuelAct *a, unsigned int p, int z, int flags);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);

#ifdef NON_MATCHING /* &a->w[p] guardado num $s; endereço de duel_w_0891525C num $s no caso 0x7D */
int duel_08889BE4(DuelAct *a)
{
    int i;
    int z;
    int id;
    unsigned int p;
    DuelCard *c;
    unsigned int o;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        a->w[0] = 0;
        a->w[1] = 0;
        for (i = 0; i < 2; i++) {
            for (z = 0; z < 5; z++) {
                id = duel.players[i & 1].field[z].card.id;
                if (card_is_monster(id) != 0 && card_is_attribute9(id) == 0 && card_attribute_icon_is3(id) == 0) {
                    a->w[duel.players[i & 1].field[z].card.owner]++;
                }
            }
        }
        duel_088D19B4(a, 0x1F001F);
        for (i = 0; i < 2; i++) {
            if (a->w[duel_turn_player ^ i] != 0) {
                duel_088C78E0(duel_turn_player ^ i);
            }
        }
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel_turn_player;
        a->w[2] = 0;
        a->w[3] = 0;
        duel_w_08914900 = duel_phase_ok2(0) | duel_phase_ok2(1) << 1;
        duel_088C7BA4();
        return 0x7E;
    case 0x7E:
        p = duel_w_089148FC;
        if (duel.players[p & 1].n_d != 0 && a->w[p] != 0) {
            c = &duel.players[p & 1].zone_d[0];
            o = c->owner;
            id = c->id;
            duel_088C4508(p, id);
            if (duel_zone_faceup(p, 0xD, 0) == 0) {
                duel_088C7880(p);
            }
            duel_088CB980(p);
            if (card_is_monster(id) == 0) {
                return 0x7C;
            }
            a->w[p]--;
            if (card_get_level(id) < 5 && duel_card_dfac8(p, id, 1) != 0 && duel_no_1a1b(p) != 0 &&
                duel_count_free_monster_zones(o) > 0 && (duel_w_08914900 >> o & 1)) {
                duel_card_copy(duel_w_0891525C, c);
                return 0x7D;
            }
            return 0x7C;
        }
        return 0x7B;
    case 0x7D:
        c = duel_w_0891525C;
        o = c->owner;
        duel_0882619C(o, c, card_attribute_icon_in_2_3(c->id), 1, 0);
        a->w[2 + o] |= 1 << duel_pick_monster_zone(o);
        return 0x7B;
    case 0x7C:
        duel_088C625C(duel_w_089148FC, 1, duel_w_089148FC != a->player);
    case 0x7B:
        if (a->w[duel_w_089148FC] > 0) {
            return 0x7E;
        }
        duel_w_089148FC = 1 - duel_w_089148FC;
        if (duel_w_089148FC != duel_turn_player) {
            return 0x7E;
        }
        return 0x78;
    case 0x78:
        for (i = 0; i < 2; i++) {
            duel_088CB9E0(duel_turn_player ^ i, a->w[2 + (duel_turn_player ^ i)]);
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}
#endif

int duel_0888A020(DuelAct *a, int b)
{
    int s;

    if (a->kind == 2) {
        if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
            return 0;
        }
        return duel_0887F728(a, b);
    }
    s = duel_pick_st_zone(a->player);
    if (s >= 0) {
        duel_088CA184(a->player | a->zone << 8, (unsigned char)duel_special.player[1] | (s & 0xFF) << 8);
        duel_088C8B70(a->player, (unsigned char)duel_special.player[1] | (s & 0xFF) << 8,
                      (unsigned char)duel_special.player[0] | (unsigned char)duel_special.zone[0] << 8);
        duel_088C87A8(duel_special.player[1], s, a->id, 0x21, 0);
    } else {
        duel_088D1438(a->player, a->zone, 0, a->id);
    }
    duel_cmd(0x1F, 0, 0, 0);
    return 0;
}

int duel_0888A1C8(DuelAct *a)
{
    duel_088C7DD4(1 - a->player, 0, 1);
    return 0;
}

int duel_0888A1F8(DuelAct *a)
{
    int i;
    DuelCard *c;

    if (a->b4_2) {
        return 0;
    }
    if (a->n7 == 1) {
        i = duel_grave_find_uid(1 - a->player, duel_act_target2(a, 0));
        if (i >= 0) {
            c = &duel.players[1 - a->player & 1].grave[i];
            duel_088CB2AC(a, 1 - a->player, c);
            duel_088C77D4(a->player, a->id, 1, CARD_UID(*c));
        }
    }
    return 0;
}

int duel_0888A2F0(DuelAct *a)
{
    duel_088C7810(1 - a->player, 1);
    duel_088C7B90(0x3C);
    return 0;
}

int duel_0888A324(DuelAct *a)
{
    int i;
    int found;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_0887DB24(a) != 0) {
            return 0x7F;
        }
        return 0;
    case 0x7F:
        if (duel.players[a->player & 1].n_hand == 0) {
            return 0;
        }
        if (duel_df664(a->player) == 0) {
            return 0;
        }
        found = 0;
        for (i = 0; i < duel.players[a->player & 1].n_hand; i++) {
            if (!(!duel_088BFFD8(a->player, a->id, i))) {
                found = 1;
                break;
            }
        }
        if (found == 0) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 0;
        } else {
            duel_0884AD18(0x2D);
        }
        return 0x7E;
    case 0x7E:
        if (duel_w_08919624 == 0) {
            return 0;
        }
        duel_08832894(a->player, a->id);
        return 0x7D;
    case 0x7D:
        duel_088C7548(a->player, duel_w_0891968C, 0);
        return 0;
    }
    return 0;
}

int duel_0888A52C(DuelAct *a, int b)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;
    int uid;
    unsigned int loc;
    unsigned int q;
    int n;
    int ok;
    int k;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (!a->b4_2 && duel_0886CE38(a, b) != 0 && a->n7 == 1) {
            uid = duel_act_target2(a, 0);
            loc = duel_card_locate(uid);
            q = loc & 0xFF;
            n = loc >> 16 & 0xFFFF;
            if (a->id != 0x1A36) {
                ok = ((unsigned short)loc >> 8 & 0xFF) == 0xE && duel_grave_card_usable(a->player, q, n) != 0;
            } else {
                ok = ((unsigned short)loc >> 8 & 0xFF) == 0xF && duel_zone_f_card_usable(a->player, q, n) != 0;
            }
            if (ok) {
                k = 2;
                duel_088C7BA4();
                switch (a->id) {
                case 0x137D:
                case 0x1366:
                case 0x15E6:
                    k = 0;
                    break;
                case 0x17B7:
                    k = 1;
                    break;
                }
                duel_0882619C(a->player, duel_card_by_uid(uid), 1, k, 0x20);
                return 0x7F;
            }
        }
        if (card_get_prop5_hi(a->id) == 3) {
            a->b4_3 = 1;
        }
        return 0;
    case 0x7F:
        switch (a->id) {
        case 0x137D:
        case 0x17B7:
        case 0x1A36:
        case 0x1A9B:
            duel_088C87A8(p, duel_b_08914A14.zone, p | z << 8, 0x17, 0);
            break;
        case 0x1366:
        case 0x15E6:
        case 0x1880:
        case 0x1881:
        case 0x19D7:
            duel_088C8B70(p, p | z << 8, p | duel_b_08914A14.zone << 8);
            break;
        }
        return 100;
    case 100:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* desvio do caso 0x137E para o corpo de 0x14FD */
int duel_0888A88C(DuelAct *a)
{
    int n;

    switch (a->id) {
    case 0x137E:
        if (a->player == duel_turn_player) {
            n = (duel_phase == 0 ^ 1) << 1;
        } else {
            n = 1;
        }
        break;
    case 0x14FD:
        n = (a->player == duel_turn_player) + 1;
        break;
    }
    duel_088C77D4(a->player, a->id, (n & 0xF) << 4 | 1, a->uid);
    return 0;
}
#endif

#ifdef NON_MATCHING /* $s1/$s2 trocados no laço das zonas */
int duel_0888A938(DuelAct *a)
{
    unsigned short buf[256];
    unsigned int k;
    int i;
    int z;
    unsigned int mask;
    unsigned int q;
    int b;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        duel_08832988(a->player, a->id, 2, 0xFF);
        return 0x7F;
    case 0x7F:
        duel_08832988(1 - a->player, a->id, 1, duel_w_0891968C);
        return 0x7E;
    case 0x7E:
        duel_wfmt_s(buf, 0x108, dlg_table_sel(duel_h_0891968E));
        duel_0884ACD8(a->player, buf);
        return 0x7D;
    case 0x7D:
        k = duel_h_0891968E;
        mask = 0;
        for (i = 0; i < 2; i++) {
            q = i != 0 ? a->player : 1 - a->player;
            for (z = 0, b = q * 16; z < 5; z++, b++) {
                if (duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].faceup != 0 && duel_field_kind(q, z) == k) {
                    mask |= 1 << b;
                }
            }
        }
        duel_088D13CC(a, mask);
        return 0;
    }
    return 0;
}
#endif

int duel_0888AB18(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    int z;

    if (duel.mode & 1) {
        return 0;
    }
    if (!(duel.mode >> 1 & 1)) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel.players[p & 1].field[z].faceup != 0) {
            switch (a->id) {
            case 0x1380:
                duel_088C5424(a->player, duel_field_atk(p, z));
                break;
            case 0x1404:
                if (duel_act_immune(a, p, z) == 0) {
                    duel_cmd((p ? 0x8000 : 0) | 0x20, z, 0, 0);
                    duel_088C4604(a, p, duel_field_atk(p, z));
                }
                break;
            case 0x176A:
                if (duel_act_immune(a, p, z) == 0) {
                    duel_cmd((p ? 0x8000 : 0) | 0x20, z, 0, 0);
                    duel_088C5424(a->player, duel_field_atk(p, z));
                }
                break;
            }
        }
    }
    return 0;
}

int duel_0888ACD8(DuelAct *a)
{
    unsigned int sp = duel_special.player[0];
    int sz = duel_special.zone[0];
    unsigned int p;
    unsigned char z;

    if (a->b4_2) {
        return 0;
    }
    z = a->zone;
    p = a->player;
    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (a->kind - 0x12U < 3) {
        if (sp != p && duel_st_affects(sp, sz, p, z) == 0) {
            duel_088C87D8(a, sp, sz, a->player | a->zone << 8, 7, 0);
        }
    }
    return 0;
}

int duel_0888AE18(DuelAct *a, int b)
{
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    if (duel_0884E398(a, b) == 0) {
        return 0;
    }
    duel_088D147C(a, a->x.s_player, a->x.s_zone);
    return 0;
}

#ifdef NON_MATCHING /* beqz/beqzl na falha de duel_0884E440 */
int duel_0888AF04(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    unsigned char y;

    switch (a->w[2]) {
    case 1:
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel_0884E440(a, p, z) != 0) {
                if (duel_act_can_affect(a, p, z, 1) != 0) {
                    duel_088D1684(a, p, z, 2);
                }
            }
            return 0;
        }
        return 0;
    case 2:
        y = a->zone;
        p = a->player;
        if (a->uid != CARD_UID(duel.players[p & 1].field[y].card)) {
            return 0;
        }
        if (duel.players[p & 1].field[y].faceup == 0) {
            return 0;
        }
        duel_088CC178(p, y, a->w[0]);
        break;
    }
    return 0;
}
#endif

int duel_0888B078(DuelAct *a)
{
    unsigned int q;
    int z;
    int i;

    for (i = 0; i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z < 5; z++) {
            if (duel_0884E440(a, q, z) != 0) {
                duel_088C97F0(a, q, z, 0, 0);
            }
        }
    }
    return 0;
}
