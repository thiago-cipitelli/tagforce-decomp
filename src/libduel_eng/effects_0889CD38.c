// @range 0x0889CD38-0x088A07C4
// @data 0x08911974-0x08911990
/* Card effect handlers (DuelAct *a), after effects_0889C150.c. */
#include "duel.h"

extern int duel_step;
extern int duel_turn_player;
extern int duel_w_089148FC;
extern unsigned short duel_h_0891968E;
extern unsigned short duel_w_0891968C;
extern unsigned int duel_turn;
extern DuelAct *duel_w_089147DC;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

void duel_088CC4B4(unsigned int p, int z, int n);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
void duel_088D192C(DuelAct *a, unsigned int p, int z);
void duel_088D1858(DuelAct *a, unsigned int p, int z);
void duel_088D147C(DuelAct *a, unsigned int p, int z);
DuelCard *duel_card_by_uid(int uid);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_088329D0(unsigned int p, int a, int b);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
int duel_phase_ok2(unsigned int p);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
int duel_count_free_monster_zones(unsigned int p);
void duel_088C78E0(unsigned int p);
void duel_088C7880(unsigned int p);
void duel_088CB980(unsigned int p);
void duel_088328FC(unsigned int p);
void duel_08832950();
int card_is_monster(int id);
int duel_card_dfac8(unsigned int p, int id, int field);
void duel_08832880(unsigned int p, unsigned short id, int a);
void duel_088CAB04(unsigned int p, int i, int b);
void duel_088C452C(unsigned int p, int n);
unsigned int duel_card_locate(unsigned int uid);
int duel_field_atk(unsigned int p, int z);
int card_get_atk(int id);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_08832B18(unsigned int p, int z);
int duel_08835AC0(void);
void duel_0884ACD8(unsigned int p, int msg);
void duel_088359E8(unsigned int p, int id, int (*fn)());
int duel_088559C0();
void duel_08832624(unsigned int p, int a, int b, int c);
void duel_088C5E70(unsigned int p, int a);
int duel_0884E690(DuelAct *a, int (*fn)());
void duel_088C625C(unsigned int p, int a, int b);
void duel_08832960(unsigned int p, unsigned short id, int a);
int duel_0884AF9C(int a, unsigned short b);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
int duel_0887F728(DuelAct *a, int b);
int duel_0887DDD0();
int field_get_card(unsigned int p, int z);
int card_same_name(int id, int id2);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
void duel_088C4508(unsigned int p, int id);
int duel_zone_faceup(unsigned int p, int z, int i);
int card_is_group_b(int id);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
void duel_088D1C70(DuelAct *a, unsigned int p, int z);
void duel_088C7810(unsigned int p, int a);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)());
void duel_08835978(int (*fn)());
int duel_0884E5E0(DuelAct *a);
int duel_08881160(DuelAct *a, int b);
DuelCard *duel_0884CCBC(int k);
void duel_088CAF24(DuelAct *a, unsigned int p, DuelCard *c);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_088327F8(unsigned int p, int a, int b);
int duel_088A2130(DuelAct *a, DuelAct *b);
void duel_088CBAA4(unsigned int p, int z, int uid);
int duel_088D15B4(DuelAct *a, unsigned int p, int z);
int duel_0886CE38(DuelAct *a, int b);
int duel_act_target2(DuelAct *a, int i);
int duel_grave_find_uid(unsigned int p, int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
void duel_088CA428(unsigned int p, unsigned int uid, int a);
void duel_088CA384(unsigned int p, int i, int f);
void duel_08832870(unsigned int p, int a);
unsigned int duel_0884E4E4(DuelAct *a);
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
unsigned int duel_0884E538(DuelAct *a, int (*fn)());
int duel_0885CE00();
void duel_08832AF4(unsigned int p, int z, int a, int b);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
void duel_088CB554(unsigned int p, DuelCard *c);
int duel_count_faceup_skip(unsigned int p, int id, int skip);
int card_get_attribute_icon(int id);
int card_get_level(int id);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);
int duel_pick_st_zone(unsigned int p);
void duel_088CC178(unsigned int p, int z, unsigned int v);
int duel_find_st(unsigned int p, int id);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_0884C804(void);
void duel_088C6A10(unsigned int p, unsigned short id, int z);
DuelCard *duel_0884C7CC(void);
void duel_088CB728(unsigned int p, int z, int uid, unsigned int q, int qz, int a, int b);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
void duel_088C44E4(unsigned int p, int id);
int duel_slot_counter(unsigned int p, int z, int id);
int duel_d_find_uid(unsigned int p, int uid);
void duel_088C7490(unsigned int p, int i, int a);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_field_level(unsigned int p, int z);
void duel_088C9344(DuelAct *a, unsigned int p, int z, int b);
int duel_08858704();
int duel_088D1B78(DuelAct *a, unsigned int p, int z);
int duel_eff_fbed8(DuelAct *a, int b);
unsigned int duel_088D184C(DuelAct *a, unsigned int mask, int b);
int duel_bit_count(unsigned int m);
int duel_0886CE90(DuelAct *a, int b);
int duel_0886CF1C(DuelAct *a, int b);
void duel_08832894(unsigned int p, unsigned short id);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
void duel_088C67F0(unsigned int p, int id, int a, int b);
int duel_0884E9E8(DuelAct *a, int b);
int duel_hand_find_uid(unsigned int p, int uid);
int duel_0887F80C();
int duel_0887F8E4(DuelAct *a, int b);
int duel_08894A84(DuelAct *a, int b);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);

int duel_0889CD38(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    duel_088CC4B4(p, z, 1);
    return 0;
}

int duel_0889CDF4(DuelAct *a)
{
    unsigned int t = a->idx;
    unsigned int p = t & 0xFF;
    int z = (unsigned char)((t & 0xFFFF) >> 8);

    if (duel_slot_has_effect(p, z, a->id)) {
        switch (a->id) {
        case 0x1700:
            duel_088D1858(a, p, z);
            break;
        case 0x1749:
            duel_088D192C(a, p, z);
            break;
        case 0x1703:
            if (duel.players[p & 1].field[z].pos == 0) {
                duel_088C97F0(a, p, z, 0, 1);
            }
            break;
        }
    }
    return 0;
}

int duel_0889CEF4(DuelAct *a)
{
    unsigned int t = a->idx;
    unsigned int p = t & 0xFF;
    int z = (unsigned char)((t & 0xFFFF) >> 8);

    if (duel_slot_has_effect(p, z, a->id)) {
        duel_088D147C(a, p, z);
    }
    return 0;
}

int duel_0889CF60(DuelAct *a)
{
    int p = duel_w_089148FC;
    int r = 0x78;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = duel_turn_player;
        a->w[2] = duel_phase_ok2(0);
        a->w[3] = duel_phase_ok2(1);
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        if (a->w[p + 2] == 0) {
            return r;
        }
        a->w[1] = MIN(duel_count_free_monster_zones(p), duel_088BFF78(p, a->id, 0));
        if (a->w[1] == 0) {
            return 0x78;
        }
        return 0x7E;
    case 0x7E:
        duel_088329D0(p, a->id, a->w[1]);
        a->w[0] = 0;
        return 0x7D;
    case 0x7D:
        if (a->w[0] < a->w[1]) {
            duel_088263A8(p, duel_card_by_uid((&duel_h_0891968E)[a->w[0]]), 1, 0);
            a->w[0]++;
            return 0x7D;
        }
        return r;
    case 0x78:
        duel_w_089148FC = 1 - p;
        if (duel_w_089148FC != duel_turn_player) {
            return 0x7F;
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_0889D150(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(1 - a->player, a->id, 0) == 0) {
            return 0x78;
        }
        duel_088329D0(1 - a->player, a->id, 0);
        return 0x7F;
    case 0x7F:
        duel_088C78E0(1 - a->player);
        duel_cmd(((1 - a->player) ? 0x8000 : 0) | 0x57, 0xD, duel_h_0891968E, 0);
        duel_088C7880(1 - a->player);
        duel_088CB980(1 - a->player);
        duel_088C7880(1 - a->player);
        return 0;
    case 0x78:
        duel_08832950(1 - a->player, a->id);
        return 0x77;
    case 0x77:
        duel_088328FC(a->player);
        return 0x76;
    case 0x76:
        duel_088C78E0(1 - a->player);
        break;
    }
    return 0;
}

int duel_0889D2D0(DuelAct *a)
{
    int i;
    int id;

    switch (duel_step) {
    case 0x80:
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel.players[a->player & 1].n_hand == 0) {
            return 0;
        }
        duel_08832880(1 - a->player, a->id, 0);
        return 0x7F;
    case 0x7F:
        i = duel_w_0891968C;
        id = duel.players[a->player & 1].hand[i].id;
        if (card_is_monster(id) && duel_card_dfac8(a->player, id, 0)) {
            duel_088263A8(a->player, &duel.players[a->player & 1].hand[i], 1, 0);
            return 0;
        }
        duel_088CAB04(a->player, i, 0);
        return 0;
    }
    return 0;
}

int duel_0889D468(DuelAct *a)
{
    duel_088C452C(a->player, 0);
    duel_088C452C(1 - a->player, 0);
    return 0;
}

int duel_0889D4B4(DuelAct *a)
{
    int uid;
    unsigned int loc;
    int z;
    int n;

    if (a->kind == 0x16) {
        uid = duel_special.act[duel_special.act[0].uid == a->src.uid].uid;
        loc = duel_card_locate(uid);
        z = (unsigned char)((loc & 0xFFFF) >> 8);
        if (z < 5) {
            n = duel_field_atk(loc & 0xFF, z);
        } else {
            n = card_get_atk(duel.list[uid].id);
        }
        duel_088C4604(a, 1 - a->player, n);
    }
    return 0;
}

int duel_0889D588(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        duel_08832B18(p, z);
        return 0x7F;
    case 0x7F:
        duel_088C87A8(p, z, a->id, 3, duel_w_0891968C * 200);
        break;
    }
    return 0;
}

int duel_0889D69C(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_08832B18(a->player, a->zone);
        return 0x7F;
    case 0x7F:
        switch (duel_w_0891968C) {
        case 1:
            duel_08832624(a->player, 1, 0, 0);
            return 0;
        case 2:
            duel_08832624(1 - a->player, 1, 0, 1);
            return 0;
        case 3:
            duel_088C5E70(a->player, 1);
            return 0;
        case 4:
            duel_088C5E70(1 - a->player, 1);
            return 0;
        case 5:
            return duel_0884E690(a, duel_088559C0) != 0 ? 0x7E : 0;
        case 6:
            duel_088D147C(a, a->player, a->zone);
            return 0;
        }
        return 0;
    case 0x7E:
        duel_0884ACD8(a->player, 0x62);
        duel_088359E8(a->player, a->id, duel_088559C0);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            duel_088D147C(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            break;
        }
        return 0x7E;
    }
    return 0;
}

int duel_0889D860(DuelAct *a)
{
    int i;

    switch (duel_step) {
    case 0x80:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 0x60;
        } else {
            duel_0884AF9C(0x128, 2);
        }
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel_w_08919624;
        duel_08832960(1 - a->player, a->id, duel_w_089148FC);
        return 0x7E;
    case 0x7E:
        duel_08832B18(1 - a->player, -1);
        return 0x7D;
    case 0x7D:
        i = duel_w_0891968C;
        if (duel_w_089148FC & 1 << i) {
            duel_088C5E70(a->player, i);
            return 0;
        }
        duel_088C625C(a->player, i, 0);
        break;
    }
    return 0;
}

int duel_0889D984(DuelAct *a, int b)
{
    duel_088C77D4(a->player, a->id, 1, 0);
    if (a->b4_2 == 0) {
        if (duel.players[a->player & 1].field[10].faceup == 0) {
            return 0;
        }
        if (duel.players[a->player & 1].field[10].card.id != 0x175E) {
            return 0;
        }
        return duel_0887F728(a, b);
    }
    return 0;
}

int duel_0889DA40(DuelAct *a)
{
    int u;

    if (duel_special.padc[2] != 0) {
        return 0;
    }
    if (duel_special.unk24 != 0) {
        u = duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].faceup;
        duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].faceup = 0;
        duel_088D147C(a, duel_special.player[1], duel_special.zone[1]);
        duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].faceup = u;
    } else {
        duel_088D147C(a, duel_special.player[1], duel_special.zone[1]);
    }
    return 0;
}

int duel_0889DB20(DuelAct *a, int b)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].field[10].faceup == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[10].card.id != 0x175E) {
        return 0;
    }
    return duel_0887DDD0(a, b);
}

int duel_0889DB98(DuelAct *a, int b)
{
    int z;

    if (a->b4_2) {
        return 0;
    }
    if (a->kind != 0x23) {
        for (z = 0; z <= 4; z++) {
            if (card_same_name(field_get_card(a->player, z), 0xFA7) && duel_act_immune(a, a->player, z)) {
                duel_088C87A8(a->player, z, a->id, 1, 0);
            }
        }
        duel_088C77D4(a->player, a->id, 1, 0);
    }
    return duel_0887DDD0(a, b);
}

int duel_0889DC90(DuelAct *a)
{
    DuelCard *c;
    int owner;
    unsigned int p;
    int id;

    switch (duel_step) {
    case 0x80:
        if (duel.players[a->player & 1].n_d == 0) {
            return 0;
        }
        return duel_088BFF78(a->player, a->id, 0) != 0 ? 0x7E : 0x64;
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
            if (!duel_zone_faceup(p, 0xD, 0)) {
                duel_088C7880(p);
            }
            duel_088CB980(p);
            if (card_is_monster(id) && card_is_group_b(id)) {
                if (duel_count_free_monster_zones(a->player) && duel_phase_ok2(a->player)
                    && duel_card_dfac8(a->player, id, 0)) {
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
        duel_0884ACD8(a->player, 0xD);
        return 0x63;
    case 0x63:
        duel_088328FC(1 - a->player);
        return 0x62;
    case 0x62:
        duel_088C78E0(a->player);
        break;
    }
    return 0;
}

int duel_0889DF20(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    int z;
    DuelStats st;

    if (duel_088A9008(a, 0, &t)) {
        p = t & 0xFF;
        z = (unsigned char)((t & 0xFFFF) >> 8);
        if (duel_0884E440(a, p, z)) {
            duel_field_stats(p, z, &st);
            duel_088C87A8(p, z, a->player | a->zone << 8, 7, duel_turn);
            duel_088C87A8(p, z, a->id, 0x12, st.def > 0xFFFF ? 0xFFFF : st.def);
            duel_088C87A8(p, z, a->id, 0x12, st.atk > 0xFFFF ? 0xFFFF : st.atk);
        }
    }
    return 0;
}

int duel_0889E034(DuelAct *a)
{
    unsigned int p;
    int z;
    int i;

    for (i = 0; i < 2; i++) {
        p = duel_turn_player;
        p ^= i;
        for (z = 0; z <= 4; z++) {
            if (duel_0884E440(a, p, z)) {
                duel_088C97F0(a, p, z, 0, 1);
            }
        }
    }
    return 0;
}

int duel_0889E0D4(unsigned int p, int z, int n)
{
    if (p != duel_w_089147DC->player) {
        if (z == 0xB) {
            return 0x800;
        }
        if (z < 0xB && duel_0884E440(duel_w_089147DC, p, z + n)) {
            return 0x800;
        }
    }
    return 0;
}

int duel_0889E140(DuelAct *a)
{
    unsigned int p;

    switch (duel_step) {
    case 0x80:
        if (duel.players[(1 - a->player) & 1].n_hand == 0 && duel_0884E5E0(a) == 0) {
            return 0;
        }
        duel_088C7810(1 - a->player, 1);
        return 0x7F;
    case 0x7F:
        p = a->player;
        if (duel_player_type[p] == 1) {
            duel_0881956C(p, a->id, duel_0889E0D4);
        } else {
            duel_0884ACD8(p, 0x3A);
            duel_08835978(duel_0889E0D4);
        }
        return 0x7E;
    case 0x7E:
        if (duel_08835AC0() != 0) {
            if (duel_w_08919650 == 0xB) {
                p = 1 - a->player;
                duel_088CAE24(p, &duel.players[p & 1].hand[duel_w_08919654], 1);
            } else {
                duel_088D1C70(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            }
            duel_088C7810(1 - a->player, 0);
            break;
        }
        return 0x7F;
    }
    return 0;
}

int duel_0889E2F8(DuelAct *a, int b)
{
    unsigned int p;

    switch (duel_step) {
    default:
        return duel_08881160(a, b);
    case 1:
        p = a->player;
        duel_088C87A8(p, duel_w_089148FC, p | a->zone << 8, 0x17, 0);
        return 0;
    }
}

int duel_0889E360(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = duel.players[a->player & 1].n_hand;
        return 0x7F;
    case 0x7F:
        if (duel.players[a->player & 1].n_hand != 0) {
            duel_088CAB04(a->player, 0, 0);
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_0884C588(a->player, MIN(duel_w_089148FC, 0x20) + 6, a->id, 0);
        duel_w_089148FC = 0;
        duel_088C7BA4();
        return 0x7D;
    case 0x7D:
        if ((unsigned int)duel_w_089148FC < duel_w_08919624) {
            duel_088CAF24(a, a->player, duel_0884CCBC(duel_w_089148FC + 1));
            duel_w_089148FC++;
            return 0x7D;
        }
        duel_088C7CF0();
        break;
    }
    return 0;
}

int duel_0889E4D0(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].field[10].faceup == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[10].card.id != 0x169F) {
        return 0;
    }
    duel_088327F8(1 - p, 1, 3);
    return 0;
}

int duel_0889E550(DuelAct *a, DuelAct *b)
{
    while (a->idx-- != 0) {
        b--;
    }
    return duel_088A2130(a, b);
}

int duel_0889E57C(DuelAct *a, DuelAct *b)
{
    switch (duel_step) {
    case 0x80:
        b->b4_3 = b->b4_2 = b->b4_1 = 1;
        duel_088CBAA4(b->player, b->zone, b->uid);
        if (b->zone < 0xB && duel_088D15B4(a, b->player, b->zone)) {
            return 0x7F;
        }
        break;
    case 0x7F:
        duel_088C5E70(a->player, 1);
        break;
    }
    return 0;
}

int duel_0889E688(DuelAct *a, int b)
{
    int i;
    int n;
    int g;

    switch (duel_step) {
    case 0x80:
        if (duel_0886CE38(a, b) == 0) {
            return 0;
        }
        if (a->id == 0x1951) {
            for (n = 0, i = 0; i < a->n7; i++) {
                g = duel_grave_find_uid(a->player, duel_act_target2(a, i));
                if (g >= 0 && duel_grave_card_usable(a->player, a->player, g)) {
                    n++;
                }
            }
            if (duel_count_free_monster_zones(a->player) < n) {
                return 0;
            }
        }
        duel_088C7BA4();
        duel_w_089148FC = 0;
        return 0x7F;
    case 0x7F:
        if (duel_w_089148FC < a->n7) {
            if (duel_count_free_monster_zones(a->player) != 0) {
                g = duel_grave_find_uid(a->player, duel_act_target2(a, duel_w_089148FC));
                if (g >= 0 && duel_grave_card_usable(a->player, a->player, g)) {
                    duel_088263A8(a->player, &duel.players[a->player & 1].grave[g], 1, a->id << 16);
                }
                duel_w_089148FC++;
                return 0x7F;
            }
        }
        return 0x7E;
    case 0x7E:
        duel_088C7CF0();
        break;
    }
    return 0;
}

int duel_0889E890(DuelAct *a)
{
    unsigned int uid;

    if (a->n7 == 1) {
        uid = duel_act_target2(a, 0);
        if (duel_grave_find_uid(a->player, uid) >= 0) {
            switch (a->id) {
            case 0x16D5:
                duel_088CA428(a->player, uid, 0);
                break;
            default:
                duel_088CA428(a->player, uid, 1);
                break;
            }
        }
    }
    return 0;
}

int duel_0889E92C(DuelAct *a)
{
    unsigned int q;
    int i;
    int owner;

    switch (duel_step) {
    case 0x80:
        q = 1 - a->player;
        if (duel.players[q & 1].n_hand == 0) {
            return 0;
        }
        duel_08832870(q, 0);
        return 0x7F;
    case 0x7F:
        i = duel_w_0891968C;
        q = 1 - a->player;
        owner = duel.players[q & 1].hand[i].owner;
        duel_088CA384(q, i, 1);
        duel_088C78E0(owner);
        break;
    }
    return 0;
}

int duel_0889EA10(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_08832AF4(a->player, a->zone, 3, 1);
        return 0x7F;
    case 0x7F:
        if (duel_h_0891968E == 3) {
            duel_088D13CC(a, duel_0884E4E4(a));
            return 0;
        }
        if (duel_h_0891968E == 0) {
            duel_088D13CC(a, duel_0884E538(a, duel_0885CE00));
        }
        break;
    }
    return 0;
}

int duel_0889EAC8(DuelAct *a, int b)
{
    unsigned int loc;
    unsigned int p;
    unsigned int i;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    loc = duel_card_locate(a->src.uid);
    p = loc & 0xFF;
    i = (unsigned short)(loc >> 16);
    if ((unsigned char)((loc & 0xFFFF) >> 8) == 0xE && duel_grave_card_usable(a->player, p, i)) {
        duel_0882619C(a->player, &duel.players[p & 1].grave[i], 1, 1, 0);
    }
    return 0;
}

int duel_0889EB90(DuelAct *a)
{
    unsigned int p = duel_turn_player ^ duel_w_089148FC;
    int u;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        a->w[0] = 0;
        a->w[1] = 0;
        return 0x7F;
    case 0x7F:
        if (duel_088BFF78(p, a->id, 0) == 0) {
            return 0x79;
        }
        duel_088329D0(p, a->id, 0);
        return 0x7E;
    case 0x7E:
        a->w[p] = duel_h_0891968E;
        return 0x78;
    case 0x79:
        duel_088328FC(1 - p);
        return 0x78;
    case 0x78:
        duel_w_089148FC++;
        if (duel_w_089148FC < 2) {
            return 0x7F;
        }
        duel_w_089148FC = 0;
        return 0x77;
    case 0x77:
        u = a->w[p];
        if (u != 0) {
            duel_088CB554(p, duel_card_by_uid((unsigned short)u));
        }
        duel_w_089148FC++;
        if (duel_w_089148FC < 2) {
            return 0x77;
        }
        return 0x6E;
    case 0x6E:
        duel_088C78E0(duel_turn_player);
        duel_088C78E0(1 - duel_turn_player);
        break;
    }
    return 0;
}

int duel_0889ED28(DuelAct *a)
{
    int i = duel_grave_find_uid(a->player, a->uid);
    unsigned int p;

    if (i >= 0) {
        p = a->player;
        duel_088CAF24(a, p, &duel.players[p & 1].grave[i]);
    }
    return 0;
}

int duel_0889EDA4(DuelAct *a, int b)
{
    if (a->b3_4 == 2) {
        return duel_0887F728(a, b);
    }
    if (a->b4_2 != 0) {
        return 0;
    }
    if (duel_count_faceup_skip(a->player, a->id, a->zone) >= 2) {
        duel_088C5E70(a->player, 3);
        duel_088D13CC(a, duel_0884E4E4(a));
    }
    return 0;
}

int duel_0889EE4C(DuelAct *a)
{
    int i;

    switch (duel_step) {
    case 0x80:
        for (i = 0; i < 4; i++) {
            if (i >= (unsigned int)duel.players[a->player & 1].n_d) {
                break;
            }
            a->w[i] = CARD_UID(duel.players[a->player & 1].zone_d[i]);
        }
        a->w[4] = duel_phase_ok2(a->player);
        duel_w_089148FC = 0;
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        if (duel_w_089148FC == 4) {
            return 100;
        }
        if (duel.players[a->player & 1].n_d == 0) {
            return 100;
        }
        duel_cmd((a->player ? 0x8000 : 0) | 0x57, 0xD, a->w[duel_w_089148FC], 0);
        return 0x7E;
    case 0x7E:
        if (!duel_zone_faceup(a->player, 0xD, 0)) {
            duel_088C7880(a->player);
        }
        duel_088CB980(a->player);
        if (card_get_attribute_icon(duel.players[a->player & 1].zone_d[0].id) == 0
            && card_get_level(duel.players[a->player & 1].zone_d[0].id) < 4) {
            if (a->w[4] && duel_count_free_monster_zones(a->player)) {
                duel_088263A8(a->player, &duel.players[a->player & 1].zone_d[0], 1, a->id << 16);
            } else {
                duel_088C625C(a->player, 1, 0);
            }
        } else {
            if (!duel_zone_faceup(a->player, 0xD, 0)) {
                duel_088C7880(a->player);
            }
        }
        duel_w_089148FC++;
        return 0x7F;
    case 100:
        duel_088C7CF0();
        duel_088C78E0(a->player);
        break;
    }
    return 0;
}

int duel_0889F144(DuelAct *a)
{
    unsigned int p;

    switch (duel_step) {
    case 0x80:
        p = a->player;
        if (duel.players[p & 1].n_hand != 0) {
            duel_088CAB04(p, 0, 0);
            return 0x80;
        }
        return 0x7F;
    case 0x7F:
        p = 1 - a->player;
        if (duel.players[p & 1].n_hand != 0) {
            duel_088CAB04(p, 0, 1);
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        p = ~duel_0884E4E4(a);
        duel_088D13CC(a, p ^ 1 << (a->zone + a->player * 16));
        return 0;
    }
    return 0;
}

int duel_0889F270(DuelAct *a)
{
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 0; z < 5; z++) {
            if (duel_0884E440(a, p, z) && !duel_slot_has_effect(p, z, a->id)) {
                duel_088C87D8(a, p, z, a->id, 2, 0);
            }
        }
    }
    duel_088C77D4(a->player, a->id, 1, 0);
    return 0;
}

int duel_0889F330(DuelAct *a, int b)
{
    unsigned int p;
    int z;
    int n;
    int id;
    int st;
    int u;
    unsigned int q;

    if (a->b4_2) {
        return 0;
    }
    if (a->kind == 5) {
        p = a->player;
        z = a->zone;
        n = duel.players[p & 1].field[z].link;
        id = 1;
        st = duel_pick_st_zone(p);
        q = a->player;
        if (a->uid != CARD_UID(duel.players[q & 1].field[a->zone].card)) {
            return 0;
        }
        switch (n) {
        case 0:
            id = 0x17AE;
            break;
        case 1:
            id = 0x17AD;
            break;
        }
        switch (duel_step) {
        case 0x80:
            if (duel_088BFF78(q, a->id, id) == 0) {
                duel_0884ACD8(a->player, 0xD);
                return 0;
            }
            duel_0884ACD8(p, 0xF6);
            duel_w_089148FC = 1;
            return 0x7F;
        case 0x7F:
            duel_0884C588(q, 6, a->id, id);
            return 0x7E;
        case 0x7E:
            switch (duel_0884C804()) {
            case 0xB:
                u = CARD_UID(*duel_0884C7CC());
                duel_088CB728(p, 0xB, u, p, st, 1, 0);
                if (duel_slot_effect_with(p, 0xB, 0x1379, u)) {
                    duel_088C44E4(p, 0x1379);
                    duel_088C4604(a, p, 2000);
                }
                if (duel_w_089148FC != 0) {
                    duel_088C78E0(p);
                }
                break;
            case 0xD:
                duel_088C6A10(p, id, st);
                duel_088C78E0(p);
                break;
            }
            return 0x78;
        case 0x78:
            if (n < 2) {
                duel_088CC178(p, z, n + 1);
            }
            if (duel_find_st(p, 0x17AE) >= 0 && duel_find_st(p, 0x17AD) >= 0) {
                duel_0884F30C(a->id | ((p << 31 & 0x80000000) | 0x200000 | (z << 16 & 0x1F0000)),
                              CARD_UID(duel.players[p & 1].field[z].card), 0);
            }
            return 0;
        case 0x6E:
            duel_088C7810(p, 1);
            return 0x6D;
        case 0x6D:
            duel_088328FC(1 - p);
            return 0x6C;
        case 0x6C:
            duel_088C7810(p, 0);
            return 100;
        case 100:
            duel_088C78E0(p);
            return 0;
        }
    } else if (a->w[2] != 0) {
        return duel_08881160(a, b);
    }
    return 0;
}

int duel_0889F754(DuelAct *a, int b)
{
    int n = duel_slot_counter(a->player, 0xB, a->id);

    switch (duel_step) {
    case 0x80:
        if (duel_0886CE38(a, b) == 0) {
            return 0;
        }
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            duel_0884ACD8(a->player, 0xD);
            return 100;
        }
        duel_0884ACD8(a->player, 0x30);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, MIN(n, 0x20) + 6, a->id, 0);
        return 0x7E;
    case 0x7E:
        duel_w_089148FC = 0;
        a->w[0] = duel_w_08919624;
        duel_088C7BA4();
        return 0x7D;
    case 0x7D:
        if (duel_w_089148FC < a->w[0]) {
            if (duel_count_free_monster_zones(a->player)) {
                duel_088263A8(a->player, duel_0884CCBC(duel_w_089148FC + 1), 1, a->id << 16);
            } else {
                duel_088C7490(a->player, duel_d_find_uid(a->player, CARD_UID(*duel_0884CCBC(duel_w_089148FC + 1))), 0);
            }
            duel_w_089148FC++;
            return 0x7D;
        }
        duel_088C7CF0();
        return a->w[0] < n ? 100 : 99;
    case 100:
        duel_088328FC(1 - a->player);
        return 99;
    case 99:
        duel_088C78E0(a->player);
        break;
    }
    return 0;
}

int duel_0889F9BC(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    int z;

    if (duel_088A9008(a, 0, 0)) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned char)((t & 0xFFFF) >> 8);
        if (duel_0884E440(a, p, z)) {
            duel_088C77D4(a->player, a->id, 0x11, duel_field_level(p, z));
        }
    }
    return 0;
}

int duel_0889FA58(DuelAct *a)
{
    unsigned int p;
    int z;
    int i;

    duel_088C7BA4();
    for (i = 0; i < 2; i++) {
        p = duel_turn_player;
        p ^= i;
        for (z = 0; z < 5; z++) {
            if (duel_0884E440(a, p, z)) {
                duel_088C97F0(a, p, z, 0, 0);
                duel_088C87D8(a, p, z, a->id, 2, 0);
            }
        }
    }
    duel_088C7CF0();
    return 0;
}

int duel_0889FB20(DuelAct *a)
{
    int z;
    unsigned int p;

    switch (duel_step) {
    case 0x80:
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        for (z = 0; z <= 4; z++) {
            if (duel_0884E440(a, a->player, z)) {
                p = a->player;
                if (duel.players[p & 1].field[z].faceup == 0) {
                    if (duel.players[p & 1].field[z].pos == 0) {
                        duel_088C97F0(a, p, z, 1, 1);
                    } else {
                        duel_088C9344(a, p, z, 1);
                    }
                } else if (duel.players[p & 1].field[z].pos == 0) {
                    duel_088C97F0(a, p, z, 0, 1);
                }
            }
        }
        return 0x7E;
    case 0x7E:
        duel_088C7CF0();
        break;
    }
    return 0;
}

int duel_0889FC60(DuelAct *a, int b)
{
    int (*fn)();
    int n;
    unsigned int t;
    unsigned int p;
    int z;

    switch (a->id) {
    case 0x179F:
        fn = duel_088559C0;
        b = 0x62;
        n = 1;
        break;
    case 0x17B8:
        fn = duel_08858704;
        b = 0x8E;
        n = 2;
        break;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0)) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned char)((t & 0xFFFF) >> 8);
            if (duel_0884E440(a, p, z) && duel_088D1B78(a, p, z)) {
                return duel_0884E690(a, fn) != 0 ? 0x7F : 0;
            }
        }
        break;
    case 0x7F:
        if (duel_0884E690(a, fn) == 0) {
            return 0;
        }
        duel_088C7BA4();
        duel_w_089148FC = 0;
        return 0x7E;
    case 0x7E:
        if (duel_w_089148FC == 0) {
            duel_0884ACD8(a->player, b);
        }
        duel_088359E8(a->player, a->id, fn);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0()) {
            duel_088D147C(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            duel_w_089148FC++;
            return duel_w_089148FC < n ? 0x7E : 0x7C;
        }
        return duel_w_089148FC != 0 ? 0x7C : 0x7E;
    case 0x7C:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_0889FE94(DuelAct *a, int b)
{
    unsigned char z;
    unsigned int p;

    if (a->b3_4 == 1) {
        return duel_eff_fbed8(a, b);
    }
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = duel_bit_count(duel_088D184C(a, duel_0884E4E4(a), 2));
        return 0x7F;
    case 0x7F:
        z = a->zone;
        p = a->player;
        if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        if (duel.players[p & 1].field[z].faceup == 0) {
            return 0;
        }
        duel_088C87A8(p, z, a->id, 3, duel_w_089148FC * 300);
        break;
    }
    return 0;
}

int duel_0889FFF4(DuelAct *a, int b)
{
    unsigned int p = a->player ^ a->opp;

    switch (duel_step) {
    case 0x80:
        if (duel_0886CE90(a, b) == 0) {
            return 0;
        }
        a->w[1] = MIN(duel_count_free_monster_zones(p), duel_088BFF78(p, a->id, 0));
        if (a->w[1] == 0) {
            return 0;
        }
        duel_0884ACD8(p, 0x11D);
        return 0x7E;
    case 0x7E:
        duel_0884C588(p, MIN(a->w[1], 0x20) + 6, a->id, 0);
        a->w[0] = 0;
        duel_088C7BA4();
        return 0x7D;
    case 0x7D:
        if (a->w[0] < a->w[1]) {
            duel_088263A8(p, duel_0884CCBC(a->w[0] + 1), 1, a->id << 16);
            a->w[0]++;
            return 0x7D;
        }
        duel_088C7CF0();
        break;
    }
    return 0;
}

int duel_088A0174(DuelAct *a, int b)
{
    unsigned int p = duel_turn_player ^ duel_w_089148FC;
    unsigned int q;
    int r;

    switch (duel_step) {
    case 0x80:
        duel_088C7BA4();
        duel_w_089148FC = 0;
        return 0x7F;
    case 0x7F:
        q = a->player;
        a->player = p;
        r = duel_0886CF1C(a, b);
        a->player = q;
        if (r == 0) {
            return 100;
        }
        if (duel_player_type[p] == 1) {
            duel_w_0891968C = 1;
        } else {
            duel_08832950(p, a->id);
        }
        return 0x7E;
    case 0x7E:
        if (duel_w_0891968C == 0) {
            return 100;
        }
        duel_08832894(p, a->id);
        return 0x7D;
    case 0x7D:
        duel_088263A8(p, &duel.players[p & 1].hand[duel_w_0891968C], 1, a->id << 16);
        return 100;
    case 100:
        duel_w_089148FC++;
        if (duel_w_089148FC < 2) {
            return 0x7F;
        }
        duel_088C7CF0();
        break;
    }
    return 0;
}

int duel_088A0328(DuelAct *a, int b)
{
    unsigned int p = duel_w_089148FC;
    unsigned int i;
    int id;

    switch (duel_step) {
    case 0x80:
        if (duel_0884E9E8(a, b) == 0) {
            return 0;
        }
        duel_w_089148FC = duel_turn_player;
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        for (i = 0; i < duel.players[p & 1].n_d; i++) {
            id = duel.players[p & 1].zone_d[i].id;
            if (!card_is_monster(id)) {
                continue;
            }
            duel_088C67F0(p, id, 0, p != a->player);
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        for (id = duel.players[p & 1].n_grave - 1; id >= 0; id--) {
            if (!card_is_monster(duel.players[p & 1].grave[id].id)) {
                continue;
            }
            duel_088CB400(p, &duel.players[p & 1].grave[id], p != a->player);
            return 0x7E;
        }
        return 0x78;
    case 0x78:
        duel_w_089148FC = 1 - p;
        if (duel_w_089148FC != duel_turn_player) {
            return 0x7F;
        }
        duel_088C7CF0();
        return 100;
    case 100:
        duel_088328FC(1 - duel_turn_player);
        return 99;
    case 99:
        duel_088328FC(duel_turn_player);
        return 0x62;
    case 0x62:
        duel_088C78E0(duel_turn_player);
        return 0x61;
    case 0x61:
        duel_088C78E0(1 - duel_turn_player);
        break;
    }
    return 0;
}

int duel_088A05C4(DuelAct *a)
{
    int i = duel_hand_find_uid(a->player, a->uid);

    if (i < 0) {
        return 0;
    }
    if (duel_count_free_monster_zones(a->player) == 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    duel_088263A8(a->player, &duel.players[a->player & 1].hand[i], 1, a->id << 16);
    return 0;
}

int duel_088A0688(DuelAct *a, int b)
{
    int z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return duel_0887F80C(a, b);
    }
    return 0;
}

void duel_088A073C(DuelAct *a, int b)
{
    if (a->kind == 2) {
        duel_08881160(a, b);
    } else {
        duel_0887F8E4(a, b);
    }
}

void duel_088A0780(DuelAct *a, int b)
{
    if (a->kind == 5) {
        duel_08881160(a, b);
    } else {
        duel_08894A84(a, b);
    }
}
